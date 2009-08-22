/***************************************************************************
                          inkvedit.cpp -  description
                             -------------------
    begin                : Wed May 14 2008
    copyright            : (C) 2008 by Antonio Nastasi
                           (C) 2009 by Sjors Gielen
    email                : sifcenter@gmail.com
                           dazjorz@kmess.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "isfinkedit.h"
#include "isfqtdrawing.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QDebug>

namespace Isf
{
  
/**
 * Create a new InkEdit widget that allows you to draw Ink on a canvas.
 *
 * To retrieve a QImage that contains the image drawn, use the function
 * getImage().
 *
 * To get the Isf::Drawing instance for your ink, use getDrawing().
 *
 * @param parent The parent widget.
 */
InkEdit::InkEdit( QWidget *parent )
: QWidget( parent )
, erasingImage_( false )
, drawing_( 0 )
, scribbling_( false )
, currentStroke_( 0 )
, drawingDirty_( true )
{
  
  setPenColor( Qt::black );
  setPenSize( 4 );
  
  drawing_ = new Isf::Drawing();
  //drawing_->setBoundingRect( rect() );

  // prepare the buffer
  clearBuffer();

  setAttribute( Qt::WA_StaticContents );
}




/**
 * By default the size should be around 300x300 - of course, this is flexible.
 */
QSize InkEdit::sizeHint() const
{
  if ( drawing_->isNull() )
  {
    return QSize(300, 300);
  }
  else
  {
    qDebug() << "NEW SIZE HINT:"<<drawing_->getSize();
    return drawing_->getSize();
  }
}




/**
 * Clears the current image. All Ink data is discarded.
 */
void InkEdit::clear()
{
  if ( drawing_ != 0 )
  {
    delete drawing_;
    drawing_ = 0;
  }
  
  drawing_ = new Isf::Drawing();

  drawingDirty_ = true;
  
  update();

  emit inkChanged();
}



/**
 * Change the color of the pen that draws on the Ink canvas.
 *
 * @param newColor A QColor object for the new color.
 */
void InkEdit::setPenColor( QColor newColor )
{
  if ( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }
  
  color_ = newColor;
}




/**
 * Change the size of the pen that draws in the Ink canvas.
 *
 * @param pixels The size of the pen, in pixels.
 */
void InkEdit::setPenSize( uint pixels )
{
  penSize_ = pixels;
}



// Draw a line from start point to last point
void InkEdit::drawLineTo( const QPoint &endPoint )
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }

  // draw the "in progress" strokes on the buffer.
  QPainter painter( &(bufferPixmap_) );
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true );

  QColor color = color_;
  if( erasingImage_ )
  {
    color = QColor( "white" );
  }

  painter.setPen( QPen( color, penSize_, Qt::SolidLine, Qt::RoundCap,
                  Qt::RoundJoin ) );

  // QPainter::drawLine() doesn't draw anything if the two points are the same
  if( lastPoint_ == endPoint )
  {
    painter.drawPoint( endPoint );
  }
  else
  {
    painter.drawLine( lastPoint_, endPoint );
  }

  int rad = (1 / 2) + 2;
  update( QRect( lastPoint_, endPoint ).normalized()
          .adjusted( -rad, -rad, +rad, +rad ) );

  lastPoint_ = endPoint;

  // Refresh the widget with the new stroke
  update();
}




// Returns true if the image is empty
bool InkEdit::isEmpty()
{
  return drawing_->isNull();
}



void InkEdit::mousePressEvent( QMouseEvent *event )
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }

  if( event->button() != Qt::LeftButton )
  {
    return;
  }

  lastPoint_ = event->pos();
  //cropImage( lastPoint_ );
  scribbling_ = true;

  // Search if there already is an attributes set compatible with the current one
  Isf::AttributeSet *reusableAttributeSet = 0;
  foreach( Isf::AttributeSet *set, drawing_->getAttributeSets() )
  {
    if( abs( (qreal)penSize_ - set->penSize.width() ) < 1.0f
    && color_ == set->color )
    {
      reusableAttributeSet = set;
      qDebug() << "Found an usable ISF attribute set";
      break;
    }
  }

  // If none is found, create a new one
  if( ! reusableAttributeSet )
  {
    qDebug() << "Creating new attribute set";
    reusableAttributeSet = new Isf::AttributeSet;
    reusableAttributeSet->color = color_;
    reusableAttributeSet->penSize.setWidth ( (qreal)penSize_ );
    reusableAttributeSet->penSize.setHeight( (qreal)penSize_ );

    drawing_->addAttributeSet( reusableAttributeSet );
  }

  // Use it for the next strokes
  drawing_->setCurrentAttributeSet( reusableAttributeSet );

  // If there already is a stroke, add it
  if( currentStroke_ )
  {
    drawing_->addStroke( currentStroke_ );
    currentStroke_ = 0;
  }
  
  currentStroke_ = new Isf::Stroke;
  currentStroke_->points.append( Isf::Point( lastPoint_ ) );
}



void InkEdit::mouseMoveEvent(QMouseEvent *event)
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }

  if( ! ( event->buttons() & Qt::LeftButton ) || ! scribbling_ )
  {
    return;
  }

  QPoint position( event->pos() );
  //cropImage ( position );
  drawLineTo( position );

  if( currentStroke_ == 0 )
  {
#ifdef KMESSDEBUG_INKEDIT_GENERAL
    qWarning() << "The stroke isn't ready!";
#endif
    return;
  }

  // Add the new point to the stroke
  currentStroke_->points.append( Isf::Point( lastPoint_ ) );
}



void InkEdit::mouseReleaseEvent(QMouseEvent *event)
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }

  if( event->button() != Qt::LeftButton || ! scribbling_ )
  {
    return;
  }

  QPoint position = event->pos();
  //cropImage(  position );
  drawLineTo( position );


  scribbling_ = false;
  emit inkChanged();

  if( ! currentStroke_ )
  {
#ifdef KMESSDEBUG_INKEDIT_GENERAL
    qWarning() << "The stroke isn't ready!";
#endif
    return;
  }

#ifdef KMESSDEBUG_INKEDIT_GENERAL
  qDebug() << "Finishing up stroke";
#endif

  currentStroke_->points.append( Isf::Point( lastPoint_ ) );
  drawing_->addStroke( currentStroke_ );
  currentStroke_ = 0;
  
  drawingDirty_ = true;

  // clear the buffer.
  clearBuffer();
}



void InkEdit::clearBuffer()
{
  bufferPixmap_ = QPixmap( size() );
  bufferPixmap_.fill( Qt::transparent );
}



// Repaint the widget.
void InkEdit::paintEvent(QPaintEvent *event)
{
  Q_UNUSED( event );
  
  
  QPainter painter( this );
  
  painter.save();
  painter.setBrush( QBrush( Qt::white ) );  
  painter.drawRect( QRect(0, 0, width(), height() ) );
  painter.restore();
  
  if( drawing_ == 0 || drawing_->isNull() )
  {
    //qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }
 
  drawing_->finalizeChanges();
  
  // draw the ISF first, then the buffer over the top.
  // buffer has a transparent background.
  QPixmap isfPixmap = drawingDirty_ ? drawing_->getPixmap( Qt::transparent ) : isfCachePixmap_;

  if ( drawingDirty_ )
  {
    isfCachePixmap_ = isfPixmap;
    drawingDirty_ = false;
  }

  painter.drawPixmap( QPoint(0, 0), isfPixmap );
  painter.drawPixmap( QPoint(0, 0), bufferPixmap_ );
  
}



// when the widget is resized, also resize the size of the
// buffer pixmap
void InkEdit::resizeEvent( QResizeEvent *event )
{
  update();

  QWidget::resizeEvent( event );
}



/**
 * Return the Ink drawn as a QImage.
 *
 * @return A QImage instance containing the rendered Ink.
 */
QImage InkEdit::getImage()
{
  return drawing_->getPixmap(/* cropped */).toImage();
}




/**
 * Return the Isf::Drawing instance that the InkEdit is currently manipulating.
 * @return The current Isf::Drawing instance.
 */
Isf::Drawing *InkEdit::getDrawing()
{
  return drawing_;
}



/**
 * Changes the currently displayed ink with the Isf::Drawing supplied.
 *
 * Note: InkEdit will take ownership of this Isf::Drawing instance. That means when
 * InkEdit is deleted, or setDrawing is called again, this Isf::Drawing instance will be
 * deleted.
 *
 * @param drawing The new Ink drawing to display.
 */
void InkEdit::setDrawing( Isf::Drawing *drawing )
{
  if ( drawing_ != 0 )
  {
    delete drawing_;
    drawing_ = 0;
  }
  
  drawing_ = drawing;
  
  // don't crop displayed images or size data.
  //drawing_->setCropping( false );

  // try to resize of the widget to accommodate the
  // drawing.
  resize( drawing_->getSize() );
  
  drawingDirty_ = true;

  update();
}


#if 0
// Return the bytes representing the image in the requested format
QByteArray InkEdit::getImageBytes( InkFormat format ) const
{
  QByteArray imageBytes;

#if KMESS_ENABLE_INK == 1
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return imageBytes;
  }
#else
  // Neither of the ink formats is supported, abort
  return imageBytes;
#endif


#ifdef KMESSDEBUG_INKEDIT_GENERAL
  switch( format )
  {
    case FORMAT_ISF: qDebug() << "Obtaining ISF ink data"; break;
    case FORMAT_GIF: qDebug() << "Obtaining GIF ink data"; break;
  }
#endif


  // Verify if the requested ink format is supported by KMess or not
  switch( format )
  {
    case FORMAT_ISF:
#if KMESS_ENABLE_INK_ISF == 0
      // ISF is not supported, why were we called for ISF data?
      qWarning() << "ISF is not supported, but was requested!";
#else
      // Overwrite the image bytes with the ISF data stream
      image_->isfData->finalizeChanges();
      imageBytes = Isf::Stream::writer( *image_->isfData );
      image_->isfData->clear();
      image_->stroke = 0;
#endif
      break;


    case FORMAT_GIF:
#if KMESS_ENABLE_INK_GIF == 0
      // GIF is not supported, why were we called for GIF data?
      qWarning() << "GIF is not supported, but was requested!";
#else
      // Copy the interesting piece of the image over to a new image
      QImage image( image_->pixels.copy( image_->area ).convertToFormat( QImage::Format_Indexed8, Qt::ThresholdDither ) );

      // Initialise the gif variables
      GifFileType    *gft      = NULL;
      ColorMapObject *cmap     = NULL;
      int             height   = image.height();
      int             width    = image.width();
      bool            gifError = false;

      // Open a temporary file
      QTemporaryFile tempFile( "kmess-ink-XXXXXX.gif" );
      QFile tempFile2;
      if( ! tempFile.open() )
      {
        qWarning() << "Couldn't open temporary file for GIF conversion.";
        goto error;
      }

#ifdef KMESSDEBUG_INKEDIT_GENERAL
      qDebug() << "Created temporary file to convert ink to GIF image.";
#endif

      // Convert the image to GIF using libgif
      // This is needed because Windows Live Messenger (at least 2009) does not display inks
      // which are not in GIF format (it doesn't even give an error).
      gifError             = true;

      // Open the gif file
      gft = EGifOpenFileHandle( tempFile.handle() );
      if( gft == 0 )
      {
        qWarning() << "Couldn't initialize gif library.";
        goto error;
      }

      // Create the color map
      cmap = MakeMapObject( image.numColors(), NULL );
      if( cmap == 0 )
      {
        qWarning() << "Couldn't create map object for gif conversion (num colors = " << image.numColors() << ")";
        goto error;
      }

      // Fill in the color map with the colors in the image color table
      for( int i = 0; i < image.numColors(); i++ )
      {
        QRgb color = image.color( i );
        cmap->Colors[i].Red = qRed( color );
        cmap->Colors[i].Green = qGreen( color );
        cmap->Colors[i].Blue = qBlue( color );
      }

      // Initialize the GIF file
      if( EGifPutScreenDesc( gft, width, height, 8, 0, cmap) == GIF_ERROR )
      {
        qWarning() << "EGifPutScreenDesc failed.";
        goto error;
      }

      // Initialize the GIF image
      if( EGifPutImageDesc( gft, 0, 0, width, height, 0, NULL) == GIF_ERROR )
      {
        qWarning() << "EGifPutImageDesc failed.";
        goto error;
      }


      // FIXME: This is the point where the artifacts are made.
      // I don't know why, but if you run:
      // EGifPutLine( gft, image.bits(), image.width() * image.height() )
      // i.e. convert the complete image in one call, then the resulting image is mangled.
      // Something is wrong with the width or so, it seems to be off by about two pixels. (Try it!)

      // Write every line
      for( int j = 0; j < height; j++ )
      {
        if( EGifPutLine( gft, image.scanLine( j ), width ) == GIF_ERROR )
        {
          qWarning() << "EGifPutLine failed for scanline" << j << "(height=" << image.height() << ";width=" << image.width() << ";bytesperline=" << image.bytesPerLine() << ")";
          goto error;
        }
      }

      // Clean up the GIF converter etc
      EGifCloseFile( gft );
      FreeMapObject( cmap );
      gifError = false;

      // Read the file back in
      // Because the QTemporaryFile is opened in unbuffered mode, we have to flush it then re-open it with QFile.
      tempFile.flush();
      tempFile2.setFileName( tempFile.fileName() );
      if( ! tempFile2.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
      {
        qWarning() << "Couldn't reopen temporary file: " << tempFile2.errorString();
        goto error;
      }

      imageBytes = tempFile2.readAll();

      tempFile2.close();
      tempFile.close();

#ifdef KMESSDEBUG_INKEDIT_GENERAL
      qDebug() << "Converted ink to GIF (" << image.height() << "x" << image.width() << "), sending " << imageBytes.size() << " bytes of image data.";
#endif

error:
      if( gifError )
      {
        qWarning() << "A GIF error occured in getImageBytes, returning empty byte array. The error was: ";
        PrintGifError();
      }
      else
      {
        qWarning() << "A non-GIF error occured in getImageBytes, returning empty byte array.";
      }
#endif  // KMESS_ENABLE_INK_GIF == 0
      break;
  }

  return imageBytes;
}
#endif

}; // namespace Isf
