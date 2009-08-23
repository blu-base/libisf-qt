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

#include "isfqt-internal.h"

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
  setCanvasColor( Qt::white );
  setPenColor( Qt::black );
  setPenSize( 4 );
  
  clear();

  // prepare the buffer
  clearBuffer();

  // create a custom cursor
  updateCursor();
  
  // start with a drawing pen by default.
  setPenType( DrawingPen );

  setAttribute( Qt::WA_StaticContents );
}




InkEdit::~InkEdit()
{
  if ( drawing_ != 0 )
  {
    delete drawing_;
    drawing_ = 0;
  }
}




/**
 * Creates a QCursor that will be displayed when the mouse pointer moves over the widget.
 *
 * The cursor becomes a point, drawn with the current stroke colour and pen size.
 */
void InkEdit::updateCursor()
{
  if ( cursorPixmap_.isNull() )
  {
    cursorPixmap_ = QPixmap( QSize( 32, 32 ) );
  }
  
  cursorPixmap_.fill( Qt::transparent );
  
  QPainter painter( &cursorPixmap_ );
  
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true );
  painter.setPen( QPen( color_, penSize_, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin ) );

  // now draw a point.
  painter.drawPoint( QPoint( cursorPixmap_.size().width() / 2, cursorPixmap_.size().height() / 2 ) );
  
  cursor_ = QCursor( cursorPixmap_ );
  
  // create our cursor.
  setCursor( cursor_ );
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
    QRect boundingRect = drawing_->getBoundingRect();
    boundingRect.setTopLeft( QPoint( 0, 0 ) );
    return boundingRect.size();
  }
}




/**
 * Clears the current image. All Ink data is discarded.
 */
void InkEdit::clear()
{
  if ( drawing_ == 0 )
  {
    drawing_ = new Isf::Drawing();
  }
  drawing_->clear();
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
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen color:" << newColor.name();
#endif

  color_ = newColor;
  
  updateCursor();
}




/**
 * Change the size of the pen that draws in the Ink canvas.
 *
 * @param pixels The size of the pen, in pixels.
 */
void InkEdit::setPenSize( int pixels )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen size:" << pixels;
#endif
  penSize_ = pixels;
  
  updateCursor();
}




/**
 * Change the pen type. See the PenType enum documentation for more information.
 *
 * @param type The new pen type.
 */
void InkEdit::setPenType( PenType type )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen type:" << type;
#endif

  penType_ = type;
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




/**
 * Returns true if the Ink image is empty (i.e., no strokes). False otherwise.
 */
bool InkEdit::isEmpty()
{
  return drawing_->isNull();
}



/**
 * Start drawing the stroke; save any attribute data if necessary.
 */
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
  scribbling_ = true;

  // Search if there already is an attributes set compatible with the current one
  Isf::AttributeSet *reusableAttributeSet = 0;
  foreach( Isf::AttributeSet *set, drawing_->getAttributeSets() )
  {
    if( abs( (qreal)penSize_ - set->penSize.width() ) < 1.0f
    && color_ == set->color )
    {
      reusableAttributeSet = set;
#ifdef ISFQT_DEBUG
      qDebug() << "Found an usable ISF attribute set";
#endif
      break;
    }
  }

  // If none is found, create a new one
  if( ! reusableAttributeSet )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Creating new attribute set";
#endif
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


/**
 * The mouse is moving; continue drawing the stroke.
 */
void InkEdit::mouseMoveEvent(QMouseEvent *event)
{
  if( ! ( event->buttons() & Qt::LeftButton ) || ! scribbling_ )
  {
    return;
  }

  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }

  QPoint position( event->pos() );

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



/**
 * Handle drawing and saving of a stroke.
 */
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


/**
 * Clear the pixmap buffer.
 */
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
  painter.setBrush( QBrush( canvasColor_ ) );  
  painter.drawRect( QRect(0, 0, width(), height() ) );
  painter.restore();
  
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkEdit!";
    return;
  }
  
  // draw the ISF first, then the buffer over the top.
  // buffer has a transparent background.
  QPixmap isfPixmap = drawingDirty_ ? drawing_->getPixmap( Qt::transparent ) : isfCachePixmap_;

  if ( drawingDirty_ )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "ISF pixmap cache dirty; re-caching.";
#endif
    isfCachePixmap_ = isfPixmap;
    drawingDirty_ = false;
  }

  QRect boundingRect = drawing_->getBoundingRect();
  
  // draw the pixmap starting at the boundingRect_ top left corner.
  painter.drawPixmap( boundingRect.topLeft(), isfPixmap );
  
  // draw the buffer from 0,0.
  painter.drawPixmap( QPoint(0, 0), bufferPixmap_ );
  
}



// when resized, re-draw everything.
void InkEdit::resizeEvent( QResizeEvent *event )
{ 
  // need to resize the buffer pixmap.
  clearBuffer();

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
  return drawing_->getPixmap().toImage();
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
 * Save the current drawing to a give QIODevice, optionally base64 encoded (default no)
 *
 * If the base64 param is True, then the drawing will be written base64-encoded. This is
 * helpful for transmission over mediums which are not binary-friendly.
 *
 * @param dev The QIODevice to save to.
 * @param base64 If true, the drawing is written encoded with base64.
 */

void InkEdit::save( QIODevice &dev, bool base64 )
{
  if ( base64 )
  {
    dev.write( Isf::Stream::writer( *drawing_ ).toBase64() );
  }
  else
  {
    dev.write( Isf::Stream::writer( *drawing_ ) );
  }
}




/**
 * Returns a QByteArray filled with appropriate ISF data.
 * @return A QByteArra filled with ISF data.
 */
QByteArray InkEdit::getBytes()
{
  return Isf::Stream::writer( *drawing_ );
}




/**
 * Set the canvas colour (i.e., the background colour of the InkEdit control)
 *
 * Note that this is not saved with the Ink drawing.
 *
 * @param newColor The new canvas color.
 */
void InkEdit::setCanvasColor( QColor newColor )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new canvas color:" << newColor.name();
#endif
  canvasColor_ = newColor;
  update();
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

  // try to resize of the widget to accommodate the
  // drawing.
  QRect boundingRect = drawing_->getBoundingRect();
  boundingRect.setTopLeft( QPoint( 0, 0 ) );
  
  resize( boundingRect.width(), boundingRect.height() );
  
  drawingDirty_ = true;

  update();
}

}; // namespace Isf
