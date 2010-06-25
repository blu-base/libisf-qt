/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2009 by Adam Goossens                                   *
 *   adam@kmess.org                                                        *
 ***************************************************************************/

/***************************************************************************
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

      // ******************
      // This is the key point of the state machine. This will continually loop looking for ISF
      // tags and farming off to the appropriate method.
      // *******************
      case ISF_PARSER_TAG:
      {
        if( streamData_->dataSource->atEnd() )
        {
          state = ISF_PARSER_FINISH;
          break;
        }

        QString place( "0x" + QString::number( streamData_->dataSource->pos(), 16 ).toUpper() );
        quint64 tagIndex = decodeUInt( streamData_->dataSource );

        switch( tagIndex )
        {
          case TAG_INK_SPACE_RECT:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_INK_SPACE_RECT";
#endif
            drawing->error_ = TagsParser::parseInkSpaceRectangle( streamData_, *drawing );
            break;

          case TAG_GUID_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_GUID_TABLE";
#endif
            drawing->error_ = TagsParser::parseGuidTable( streamData_, *drawing );
            break;

          case TAG_DRAW_ATTRS_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_DRAW_ATTRS_TABLE";
#endif
            drawing->error_ = TagsParser::parseAttributeTable( streamData_, *drawing );
            break;

          case TAG_DRAW_ATTRS_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_DRAW_ATTRS_BLOCK";
#endif
            drawing->error_ = TagsParser::parseAttributeBlock( streamData_, *drawing );
            break;

          case TAG_STROKE_DESC_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_STROKE_DESC_TABLE";
#endif
            drawing->error_ = TagsParser::parseStrokeDescTable( streamData_, *drawing );
            break;

          case TAG_STROKE_DESC_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_STROKE_DESC_BLOCK";
#endif
            drawing->error_ = TagsParser::parseStrokeDescBlock( streamData_, *drawing );
            break;

          case TAG_BUTTONS:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_BUTTONS";
#endif
            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_BUTTONS" );
            break;

          case TAG_NO_X:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_NO_X";
#endif
            drawing->error_ = ISF_ERROR_NONE;

            drawing->hasXData_ = false;
            break;

          case TAG_NO_Y:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_NO_Y";
#endif
            drawing->hasYData_ = false;
            break;

          case TAG_DIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_DIDX";
#endif

            quint64 value = decodeUInt( streamData_->dataSource );

            if( value < (uint)streamData_->attributeSets.count() )
            {
              streamData_->currentAttributeSetIndex = value;

#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use drawing attributes #" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid drawing attribute ID!";
#endif
            }
            break;
          }

          case TAG_STROKE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_STROKE";
#endif
            drawing->error_ = TagsParser::parseStroke( streamData_, *drawing );
            break;

          case TAG_STROKE_PROPERTY_LIST:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_STROKE_PROPERTY_LIST";
#endif
            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_STROKE_PROPERTY_LIST" );
            break;

          case TAG_POINT_PROPERTY:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_POINT_PROPERTY";
#endif
            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_POINT_PROPERTY" );
            break;

          case TAG_SIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_SIDX";
#endif

            quint64 value = decodeUInt( streamData_->dataSource );

            if( value < (uint)streamData_->strokeInfos.count() )
            {
              streamData_->currentStrokeInfoIndex = value
;
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use stroke info #" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid stroke ID!";
#endif
            }
            break;
          }

          case TAG_COMPRESSION_HEADER:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_COMPRESSION_HEADER";
#endif
            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_COMPRESSION_HEADER" );
            break;

          case TAG_TRANSFORM_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_TABLE";
#endif
            drawing->error_ = TagsParser::parseTransformationTable( streamData_, *drawing );
            break;

          case TAG_TRANSFORM:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ISOTROPIC_SCALE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ANISOTROPIC_SCALE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ROTATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_ROTATE";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_TRANSLATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_TRANSLATE";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TRANSFORM_QUAD:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TRANSFORM_QUAD";
#endif
            drawing->error_ = TagsParser::parseTransformation( streamData_, *drawing, tagIndex );
            break;

          case TAG_TIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_TIDX";
#endif

            quint64 value = decodeUInt( streamData_->dataSource );

            if( value < (uint)streamData_->transforms.count() )
            {
              streamData_->currentTransformsIndex = value;

#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use transform #" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid transform ID!";
#endif
            }

            break;
          }

          case TAG_METRIC_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_METRIC_TABLE";
#endif
            drawing->error_ = TagsParser::parseMetricTable( streamData_, *drawing );
            break;

          case TAG_METRIC_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_METRIC_BLOCK";
#endif
            drawing->error_ = TagsParser::parseMetricBlock( streamData_, *drawing );
            break;

          case TAG_MIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_MIDX";
#endif

            quint64 value = decodeUInt( streamData_->dataSource );

            if( value < (uint)streamData_->metrics.count() )
            {
              streamData_->currentMetricsIndex = value;
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use metrics #" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid metrics ID!";
#endif
            }

            break;
          }

          case TAG_MANTISSA:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_MANTISSA";
#endif
            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_MANTISSA" );
            break;

          case TAG_PERSISTENT_FORMAT:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_PERSISTENT_FORMAT";
#endif
            drawing->error_ = TagsParser::parsePersistentFormat( streamData_, *drawing );
            break;

          case TAG_HIMETRIC_SIZE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_HIMETRIC_SIZE";
#endif
            drawing->error_ = TagsParser::parseHiMetricSize( streamData_, *drawing );
            break;

          case TAG_STROKE_IDS:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag (@" << place << "): TAG_STROKE_IDS";
#endif

            drawing->error_ = TagsParser::parseUnsupported( streamData_, "TAG_STROKE_IDS" );
            break;

          default:

            // If the tagIndex is known from the GUID table, show it differently
            if( drawing->maxGuid_ > 0
            &&  tagIndex >= DEFAULT_TAGS_NUMBER && tagIndex <= drawing->maxGuid_ )
            {
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "Got tag (@" << place << "): TAG_CUSTOM:" << tagIndex;
#endif
              TagsParser::parseCustomTag( streamData_, *drawing, tagIndex );
            }
            else
            {
              TagsParser::parseUnsupported( streamData_, "Unknown " + QString::number( tagIndex ) );
            }
            break;

        } // End of tagIndex switch

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

  // Perform the last operations on the drawing

  // Adjust the bounding rectangle to include the strokes borders
  QSize penSize( drawing->maxPenSize_.toSize() );
  streamData_->boundingRect.adjust( -penSize.width() - 1, -penSize.height() - 1,
                                   +penSize.width() + 1, +penSize.height() + 1 );
  drawing->setBoundingRect( streamData_->boundingRect );

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
    isfData = imageData.text( "application/x-ms-ink" ).toAscii();

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
  TagsWriter::prepare( streamData_, drawing );

  // Write the persistent format tag
  TagsWriter::addPersistentFormat( streamData_, drawing );

  // Write the drawing size
  TagsWriter::addHiMetricSize( streamData_, drawing );

  // Write the attributes
  TagsWriter::addAttributeTable( streamData_, drawing );

  // Write the metrics
  TagsWriter::addMetricsTable( streamData_, drawing );

  // Write the transforms
  TagsWriter::addTransformationTable( streamData_, drawing );

  // Write the strokes
  TagsWriter::addStrokes( streamData_, drawing );

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
  GifFileType*    gifImage  = NULL;
  ColorMapObject* cmap      = NULL;
  int             height    = isfImage.height();
  int             width     = isfImage.width();
  int             numColors = 0;
  bool            gifError  = true;

  // Convert the image to GIF using libgif

  // Open the gif file
  gifData.open( QIODevice::WriteOnly );
  gifImage = EGifOpen( (void*)&gifData, GifWriteToByteArray );
  if( gifImage == 0 )
  {
    qWarning() << "Couldn't initialize gif library!";
    goto writeError;
  }

  // Create the color map
  numColors = ( isfImage.numColors() << 2 );
  if( numColors > 256 )
  {
    numColors = 256;
  }

  cmap = MakeMapObject( numColors, NULL );
  if( cmap == 0 && isfImage.numColors() > 1 )
  {
    qWarning() << "Couldn't create map object for gif conversion (colors:" << isfImage.numColors() << ")!";
    goto writeError;
  }

  // Fill in the color map with the colors in the image color table
  for( int i = 0; i < isfImage.numColors(); ++i )
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
    goto writeError;
  }

  // Save the image format
  if( EGifPutImageDesc( gifImage, 0, 0, width, height, 0, NULL ) == GIF_ERROR )
  {
    qWarning() << "EGifPutImageDesc() failed!";
    goto writeError;
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
      goto writeError;
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
    if( EGifPutExtensionFirst( gifImage, COMMENT_EXT_FUNC_CODE, MAX_GIF_BYTE, isfData.left( MAX_GIF_BYTE ).data() ) == GIF_ERROR )
    {
      qWarning() << "EGifPutExtensionFirst failed!";
      goto writeError;
    }

    // The first MAX_GIF_BYTE bytes have been written already
    quint32 pos = MAX_GIF_BYTE;

    quint32 length = ( isfData.size() - pos );

    // Write all the full data blocks
    while( length >= MAX_GIF_BYTE )
    {
      if( EGifPutExtensionNext( gifImage, 0, MAX_GIF_BYTE, isfData.mid( pos, MAX_GIF_BYTE ).data() ) == GIF_ERROR )
      {
        qWarning() << "EGifPutExtensionNext failed!";
        goto writeError;
      }

      pos += MAX_GIF_BYTE;
      length -= MAX_GIF_BYTE;
    }

    // Write the last block
    if( length > 0 )
    {
      if( EGifPutExtensionLast( gifImage, 0, length, isfData.mid( pos, MAX_GIF_BYTE ).data() ) == GIF_ERROR )
      {
        qWarning() << "EGifPutExtensionLast (n) failed!";
        goto writeError;
      }
    }
    else
    {
      if( EGifPutExtensionLast( gifImage, 0, 0, 0 ) == GIF_ERROR )
      {
        qWarning() << "EGifPutExtensionLast (0) failed!";
        goto writeError;
      }
    }
  }

  gifError = false;

writeError:
  // Clean up the GIF converter etc
  EGifCloseFile( gifImage );
  FreeMapObject( cmap );
  gifData.close();

  if( gifError )
  {
    qWarning() << "GIF error code:" << GifLastError();
  }
  else
  {
    // Return the GIF data
    imageBytes = gifData.data();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Converted a" << isfData.size() << "bytes Ink to GIF:" << isfImage.height() << "x" << isfImage.width() << "->" << imageBytes.size() << "bytes";
#endif
  }

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


