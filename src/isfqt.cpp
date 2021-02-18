/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2009 by Adam Goossens                                   *
 *   adam@kmess.org                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "isfqt-internal.h"

#include "data/datasource.h"
#include "data/multibytecoding.h"
#include "tagsparser.h"
#include "tagswriter.h"

#include <IsfQtDrawing>

#include <QPainter>
#include <QPixmap>

#if ISFQT_GIF_ENABLED == 1
  #include "gif-support.h"
#endif


using namespace Isf;
using namespace Compress;


/// Supported ISF version number
#define SUPPORTED_ISF_VERSION       0


// Initialization of static properties
StreamData* Stream::streamData_( 0 );



/**
 * Convert a raw ISF data stream into a drawing.
 *
 * If the ISF data is invalid, a null Drawing is returned.
 *
 * @param rawData Source byte array with an ISF stream
 * @param decodeFromBase64 Whether the bytes are in the Base64 format and
 *                         need to be decoded first
 * @return an Isf::Drawing, with null contents on error
 */
Drawing& Stream::reader( const QByteArray& rawData, bool decodeFromBase64 )
{
  // Create a new drawing on the heap to ensure it will keep
  // living after this method returns
  Drawing* drawing = new Drawing();

  ParserState state = ISF_PARSER_START;

  streamData_ = new StreamData();
  streamData_->dataSource = new DataSource( decodeFromBase64
                                            ? QByteArray::fromBase64( rawData )
                                            : rawData );

  int size = streamData_->dataSource->size();
  if( size == 0 )
  {
    state = ISF_PARSER_FINISH;
    drawing->error_ = ISF_ERROR_BAD_STREAMSIZE;
  }

  while( state != ISF_PARSER_FINISH )
  {
    switch( state )
    {
      case ISF_PARSER_START:
      {
        // step 1: read ISF version.
        quint8 version = decodeUInt( streamData_->dataSource );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "Version:" << version;
#endif
        if ( version != SUPPORTED_ISF_VERSION )
        {
          drawing->error_ = ISF_ERROR_BAD_VERSION;
          drawing->isNull_ = true;
          state = ISF_PARSER_FINISH;
        }
        else
        {
          // version is OK. find ISF stream size next.
          state = ISF_PARSER_STREAMSIZE;
        }

        break;
      }

      case ISF_PARSER_STREAMSIZE:
      {
        // read ISF stream size.
        // check it matches the length of the data array.
        quint64 streamSize = decodeUInt( streamData_->dataSource );

        if ( streamSize != (quint64)( streamData_->dataSource->size() - streamData_->dataSource->pos() ) )
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Invalid stream size" << streamSize
                   << ", expected" << ( streamData_->dataSource->size() - streamData_->dataSource->pos() );
#endif
          // streamsize is bad.
          drawing->error_ = ISF_ERROR_BAD_STREAMSIZE;
          state = ISF_PARSER_FINISH;
        }
        else
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Reading ISF stream of size:" << streamSize << "...";
#endif
          // Validate the drawing
          drawing->isNull_ = false;

          // start looking for ISF tags.
          state = ISF_PARSER_TAG;
        }

        break;
      }

      case ISF_PARSER_TAG:
      {
        if( streamData_->dataSource->atEnd() )
        {
          state = ISF_PARSER_FINISH;
          break;
        }

        // Read the next tag from the stream
        drawing->error_ = TagsParser::nextTag( streamData_, drawing );

        if( drawing->error_ != ISF_ERROR_NONE )
        {
#ifdef ISFQT_DEBUG_VERBOSE
          qWarning() << "Error in last operation, stopping.";
#endif
          state = ISF_PARSER_FINISH;
        }

        break;
      }

      // Should never arrive here! It's here only to avoid compiler warnings.
      case ISF_PARSER_FINISH:
        break;

    }
  }

#ifdef ISFQT_DEBUG
  qDebug() << "Finished with" << ( drawing->error_ == ISF_ERROR_NONE ? "success" : "error" );
  qDebug();
#endif

  delete streamData_->dataSource;

  if( drawing->error_ != ISF_ERROR_NONE )
  {
    delete streamData_;
    streamData_ = 0;
    return *drawing;
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Drawing bounding rectangle:" << drawing->boundingRect();
  qDebug() << "Maximum thickness:" << drawing->maxPenSize_;
#endif

  delete streamData_;
  streamData_ = 0;
  return *drawing;
}



/**
 * Convert a Fortified-GIF image into a drawing.
 *
 * If the GIF image or the ISF data within it are invalid, or if the GIF did not
 * have any ISF stream within, then a null Drawing is returned.
 *
 * Please note that this method does nothing if Isf-Qt is compiled
 * withous GIF support. Use Stream::supportsGif() to verify whether
 * GIF was compiled in or not.
 *
 * @see supportsGif()
 * @param gifRawBytes Source byte array with a Fortified GIF image
 * @param decodeFromBase64 True if the bytes are in the Base64 format and
 *                         need to be decoded first
 * @return an Isf::Drawing, with null contents on error
 */
Drawing& Stream::readerGif( const QByteArray& gifRawBytes, bool decodeFromBase64 )
{
  QByteArray isfData;

#if ISFQT_GIF_ENABLED == 1

  QByteArray gifBytes( decodeFromBase64
                        ? QByteArray::fromBase64( gifRawBytes )
                        : gifRawBytes );

/**
 * With the commented code below, it would all have been so easy, but no!
 * DGifGetComment is NOT PRESENT IN THE LIBRARY despite being in the giflib
 * header file. And EGifPutComment is present!
 * It doesn't work, but hey, at least it's there. (see below)
 */
/*
  QBuffer gifData( &gifBytes );
  gifData.open( QIODevice::ReadOnly );

  // Open the gif file
  GifFileType* gifImage = DGifOpen( (void*)&gifData, GifReadFromByteArray );
  if( gifImage != 0 )
  {
    DGifGetComment( gifImage, data?? );
  }
  else
  {
    qWarning() << "Couldn't initialize GIF library!";
  }

  DGifCloseFile( gifImage );
  gifData.close();
*/

  // Find the last 'comment' type tag: it should be the last thing in the file...
  qint32 size = 0;
  qint32 position = 0;
  qint32 maxDataPosition = gifBytes.size() - 2; // comment and gif stream ending bytes

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Searching for a stream. Last valid position:" << maxDataPosition;
#endif

  while( size == 0 && ( position = gifBytes.lastIndexOf( COMMENT_EXT_FUNC_CODE, position - 1 ) ) >= 0 )
  {
    // Skip the comment tag, to have the size byte as current char
    qint32 lastPosition = position + 1;

    // The next character after the tag can't be an ISF stream start, skip
    if( gifBytes[ position + 2 ] != '\0' )
    {
      continue;
    }

    // Try to read the stream
    quint8 sizeByte;
    do
    {
      sizeByte = gifBytes[ lastPosition ];

      // Skip the size byte
      lastPosition++;

      isfData.append( gifBytes.mid( lastPosition, sizeByte ) );
      lastPosition += sizeByte;
    }
    while( sizeByte == MAX_GIF_BYTE && ( lastPosition <= maxDataPosition ) );

    // We found the ISF stream!
    if( lastPosition == maxDataPosition )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "Found an ISF stream of size" << isfData.size();
#endif
      break;
    }
    else
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "Stream not found at position:" << position
               << "size:" << ( lastPosition - position );
#endif
      isfData.clear();
    }
  }

#endif // ISFQT_GIF_ENABLED == 1

  return reader( isfData );
}



/**
 * Convert a Fortified-PNG image into a drawing.
 *
 * If the PNG image or the ISF data within it are invalid, or if the PNG did not
 * have any ISF stream within, then a null Drawing is returned.
 *
 * @param gifRawBytes Source byte array with a Fortified PNG image
 * @param decodeFromBase64 True if the bytes are in the Base64 format and
 *                         need to be decoded first
 * @return an Isf::Drawing, with null contents on error
 */
Drawing& Stream::readerPng( const QByteArray& pngRawBytes, bool decodeFromBase64 )
{
  QByteArray isfData;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Reading a PNG-Fortified file";
#endif

  QByteArray pngBytes( decodeFromBase64
                        ? QByteArray::fromBase64( pngRawBytes )
                        : pngRawBytes );

  QImage imageData( QImage::fromData( pngBytes, "PNG" ) );
  if( ! imageData.isNull() )
  {
#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Picture data is valid: checking for the ISF data tag...";
#endif
    isfData = imageData.text( "application/x-ms-ink" ).toLatin1();

    if( ! isfData.isEmpty() )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "ISF data found! Decoding from Base64 and parsing it...";
#endif
      isfData = QByteArray::fromBase64( isfData );
    }
  }

  return reader( isfData );
}



/**
 * Return whether the library was built with Fortified GIF support or not.
 *
 * @return bool
 */
bool Stream::supportsGif()
{
  return ( ISFQT_GIF_ENABLED == true );
}



/**
 * Convert a drawing into a raw ISF data stream.
 *
 * The resulting byte array will be empty if the drawing is not valid.
 *
 * @param drawing Source drawing
 * @param encodeToBase64 Whether the converted ISF stream should be
 *                       encoded with Base64 or not
 * @return Byte array with an ISF data stream
 */
QByteArray Stream::writer( const Drawing& drawing, bool encodeToBase64 )
{
  if( &drawing == 0 || drawing.isNull() || drawing.error() != ISF_ERROR_NONE )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "The drawing was not valid!";
#endif
    return QByteArray();
  }

  streamData_ = new StreamData();
  streamData_->dataSource = new DataSource();
  DataSource* dataSource = streamData_->dataSource;

  // Add the initial data
  TagsWriter::prepare( streamData_, &drawing );

  // Write the persistent format tag
  TagsWriter::addPersistentFormat( streamData_, &drawing );

  // Write the drawing size
  TagsWriter::addHiMetricSize( streamData_, &drawing );

  // Write the attributes
  TagsWriter::addAttributeTable( streamData_, &drawing );

  // Write the metrics
  TagsWriter::addMetricsTable( streamData_, &drawing );

  // Write the transforms
  TagsWriter::addTransformationTable( streamData_, &drawing );

  // Write the strokes
  TagsWriter::addStrokes( streamData_, &drawing );

  // Write the stream size (at the start of the stream)
  encodeUInt( dataSource, dataSource->size(), true/*prepend*/ );

  // Write the version number (at the start of the stream)
  encodeUInt( dataSource, SUPPORTED_ISF_VERSION, true/*prepend*/ );

  QByteArray data( dataSource->data() );

  delete streamData_->dataSource;
  delete streamData_;
  streamData_ = 0;

  // Convert to Base64 if needed
  if( encodeToBase64 )
  {
    return data.toBase64();
  }
  else
  {
    return data;
  }
}



/**
 * Convert a drawing into a Fortified-GIF image.
 *
 * The resulting byte array will be empty if the drawing is not valid.
 *
 * Please note that this method does nothing if Isf-Qt is compiled
 * withous GIF support. Use Stream::supportsGif() to verify whether
 * GIF was compiled in or not.
 *
 * The Fortified-GIF format is nothing more than a GIF image with the original
 * ISF drawing added as a GIF Comment field.
 *
 * @see supportsGif()
 * @param drawing Source drawing
 * @param encodeToBase64 Whether the converted GIF should be
 *                       encoded with Base64 or not
 * @return Byte array with a GIF data stream (optionally encoded with Base64)
 */
QByteArray Stream::writerGif( const Drawing& drawing, bool encodeToBase64 )
{
  QByteArray imageBytes;

#if ISFQT_GIF_ENABLED == 1
  Drawing source( drawing );

  // Get the ISF data stream
  QByteArray isfData( writer( source ) );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "GIF-Fortifying an ISF stream of size" << isfData.size();
#endif

  // Get the ISF pixmap and copy the pixels to an 8bpp image
  QImage isfImage( source.pixmap().toImage()
                                  .convertToFormat( QImage::Format_Indexed8,
                                                    Qt::ThresholdDither ) );

  // Initialise the gif variables
  QBuffer         gifData;
  GifFileType*    gifImage  = nullptr;
  ColorMapObject* cmap      = nullptr;
  int             height    = isfImage.height();
  int             width     = isfImage.width();
  int             numColors = 0;
  bool            gifError  = true;
  int             errorCode = 0;


  auto writeError = [&]() {
    // Clean up the GIF converter etc
    EGifCloseFile(gifImage, &errorCode);
    GifFreeMapObject(cmap);
    gifData.close();

    if (gifError) {
      qWarning() << "GIF error code:" << GifErrorString(errorCode);
    }
    else {
      // Return the GIF data
      imageBytes = gifData.data();

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "Converted a" << isfData.size()
               << "bytes Ink to GIF:" << isfImage.height() << "x"
               << isfImage.width() << "->" << imageBytes.size() << "bytes";
#endif
    }
  };

  // Convert the image to GIF using libgif

  // Open the gif file
  gifData.open( QIODevice::WriteOnly );
  gifImage = EGifOpen( (void*)&gifData, GifWriteToByteArray, &errorCode );
  if( gifImage == 0 )
  {
    qWarning() << "Couldn't initialize gif library!";
    writeError();
  }

  // Create the color map
  numColors = ( isfImage.colorCount() << 2 );
  if( numColors > 256 )
  {
    numColors = 256;
  }

  cmap = GifMakeMapObject( numColors, nullptr );
  if( cmap == 0 && isfImage.colorCount() > 1 )
  {
    qWarning() << "Couldn't create map object for gif conversion (colors:" << isfImage.colorCount() << ")!";
    writeError();
  }

  // Fill in the color map with the colors in the image color table
  for( int i = 0; i < isfImage.colorCount(); ++i )
  {
    const QRgb &color( isfImage.color( i ) );
    cmap->Colors[i].Red   = qRed  ( color );
    cmap->Colors[i].Green = qGreen( color );
    cmap->Colors[i].Blue  = qBlue ( color );
  }

  // Save the file properties
  if( EGifPutScreenDesc( gifImage, width, height, 8, 0, cmap ) == GIF_ERROR )
  {
    qWarning() << "EGifPutScreenDesc() failed!";
    writeError();
  }

  // Save the image format
  if( EGifPutImageDesc( gifImage, 0, 0, width, height, 0, nullptr ) == GIF_ERROR )
  {
    qWarning() << "EGifPutImageDesc() failed!";
    writeError();
  }


  /**
   * FIXME: If to write the scanlines you use
   *   EGifPutLine( gifImage, isfImage.bits(), isfImage.width() * isfImage.height() )
   * i.e. convert the complete image in one call, then the resulting image is mangled.
   * Something is wrong with the width or so, it seems to be off by about two pixels.
   */
  // Write every scanline
  for( int line = 0; line < height; ++line )
  {
    if( EGifPutLine( gifImage, isfImage.scanLine( line ), width ) == GIF_ERROR )
    {
      qWarning() << "EGifPutLine failed at scanline" << line
                 << "(height:" << isfImage.height()
                 << ", width:" << isfImage.width()
                 << ", bytesperline:" << isfImage.bytesPerLine() << ")";
      writeError();
    }
  }


/**
 * Completing the funny theater that is giflib, EGifPutComment() doesn't
 * work, or I've overlooked something Extremely Obvious(tm).
 * Googling didn't help: I rewrote it (from the giflib source) with
 * Qt.
 */
/*
  if( EGifPutComment( gifImage, isfData.constData() ) == GIF_ERROR )
  {
    qWarning() << "EGifPutComment has failed!";
    goto writeError;
  }
*/

  // Write the ISF stream into the Comment Extension field
  if( isfData.size() < MAX_GIF_BYTE )
  {
    EGifPutExtension( gifImage, COMMENT_EXT_FUNC_CODE, isfData.size(), isfData.constData() );
  }
  else
  {
    // Write the extension
    if( EGifPutExtensionLeader( gifImage, COMMENT_EXT_FUNC_CODE ) == GIF_ERROR )
    {
      qWarning() << "EGifPutExtensionFirst failed!";
      writeError();
    }

    // The first MAX_GIF_BYTE bytes have been written already
    quint32 pos = MAX_GIF_BYTE;

    quint32 length = ( isfData.size() - pos );

    // Write all the full data blocks
    while( length >= MAX_GIF_BYTE )
    {
      if( EGifPutExtensionBlock( gifImage, MAX_GIF_BYTE, isfData.mid( pos, MAX_GIF_BYTE ).data() ) == GIF_ERROR )
      {
        qWarning() << "EGifPutExtensionNext failed!";
        writeError();
      }

      pos += MAX_GIF_BYTE;
      length -= MAX_GIF_BYTE;
    }

    // Write the last block
    if( EGifPutExtensionTrailer( gifImage ) == GIF_ERROR )
    {
      qWarning() << "EGifPutExtensionLast (n) failed!";
      writeError();
    }
  }

  gifError = false;

  writeError();

#endif // ISFQT_GIF_ENABLED == 1


  // Convert to Base64 if needed
  if( encodeToBase64 )
  {
    return imageBytes.toBase64();
  }
  else
  {
    return imageBytes;
  }
}



/**
 * Convert a drawing into a Fortified-PNG image.
 *
 * The resulting byte array will be empty if the drawing is not valid.
 *
 * The Fortified-PNG format is nothing more than a PNG image with the original
 * ISF drawing added as a PNG text field.
 *
 * @param drawing Source drawing
 * @param encodeToBase64 Whether the converted ISF stream should be
 *                       encoded with Base64 or not
 * @return Byte array with a PNG data stream (optionally encoded with Base64)
 */
QByteArray Stream::writerPng( const Drawing& drawing, bool encodeToBase64 )
{
  Drawing source( drawing );

  // Get the ISF data stream
  QByteArray isfData( writer( source ) );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "PNG-Fortifying an ISF stream of size" << isfData.size();
#endif

  // Get the actual image
  QImage isfImage( source.pixmap().toImage() );

  // Add to it the Base64 version of the ISF drawing, as a comment and in Base64 form
  // (that's because the PNG text fields are only meant to hold text)
  isfImage.setText( "application/x-ms-ink", isfData.toBase64() );

  // Save it as a PNG image
  QBuffer imageBytes;
  imageBytes.open( QIODevice::WriteOnly );
  isfImage.save( &imageBytes, "PNG" );
  imageBytes.close();

  // Convert to Base64 if needed
  if( encodeToBase64 )
  {
    return imageBytes.data().toBase64();
  }
  else
  {
    return imageBytes.data();
  }
}


