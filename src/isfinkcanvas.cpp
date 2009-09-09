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

#include "isfinkcanvas.h"
#include "isfqtdrawing.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QDebug>


using namespace Isf;



/**
 * Create a new InkCanvas widget that allows you to draw Ink on a canvas.
 *
 * To retrieve a QImage that contains the image drawn, use the function
 * image().
 *
 * To get the Isf::Drawing instance for your ink, use drawing().
 *
 * @param parent The parent widget.
 */
InkCanvas:: InkCanvas( QWidget *parent )
: QWidget( parent )
, erasingImage_( false )
, drawing_( 0 )
, scribbling_( false )
, currentStroke_( 0 )
, drawingDirty_( true )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Created new InkCanvas:" << this << "**";
#endif

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



/**
 * Destructor.
 *
 * There are no members on stack, so it's currently not used.
 */
InkCanvas::~InkCanvas()
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Destroyed InkCanvas:" << this << "**";
#endif
}



/**
 * Creates a QCursor displayed when the mouse pointer moves over the widget.
 *
 * The cursor becomes a point, drawn with the current stroke colour and pen size.
 */
void InkCanvas::updateCursor()
{
  if ( cursorPixmap_.isNull() )
  {
    cursorPixmap_ = QPixmap( QSize( 32, 32 ) );
  }

  if ( penType_ == EraserPen )
  {
    cursorPixmap_ = QPixmap( ":pics/draw-eraser.png" );
    cursor_ = QCursor( cursorPixmap_, 0, cursorPixmap_.height() );
  }
  else
  {
    cursorPixmap_.fill( Qt::transparent );

    QPainter painter( &cursorPixmap_ );

    painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true );
    painter.setPen( QPen( color_, penSize_, Qt::SolidLine, Qt::RoundCap,
                          Qt::RoundJoin ) );

    // now draw a point.
    painter.drawPoint( QPoint( cursorPixmap_.size().width() / 2, cursorPixmap_.size().height() / 2 ) );

    cursor_ = QCursor( cursorPixmap_ );
  }

  // create our cursor.
  setCursor( cursor_ );
}



/**
 * Return the current suggested size for the canvas.
 *
 * The actual suggested size is one which can fit the current drawing well.
 * By default the size should be around 300x300 - of course, this is flexible.
 */
QSize InkCanvas::sizeHint() const
{
  if ( drawing_->isNull() )
  {
    return QSize( 300, 300 );
  }
  else
  {
    QRect boundingRect = drawing_->boundingRect();
    boundingRect.setTopLeft( QPoint( 0, 0 ) );
    return boundingRect.size();
  }
}



/**
 * Clears the current image.
 *
 * All the Ink data is discarded, but the status is not: for example,
 * the stroke size and color are kept.
 */
void InkCanvas::clear()
{
  if ( drawing_ == 0 )
  {
    drawing_ = &initialDrawing_;
  }

  drawing_->clear();
  drawingDirty_ = true;

  update();

  emit inkChanged();
}



/**
 * Change the color of the pen used to draw on the Ink canvas.
 *
 * @param newColor The new color
 */
void InkCanvas::setPenColor( QColor newColor )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen color:" << newColor.name();
#endif

  color_ = newColor;

  updateCursor();
}



/**
 * Change the size of the pen used to draw on the Ink canvas.
 *
 * @param pixels The size of the pen, in pixels
 */
void InkCanvas::setPenSize( int pixels )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen size:" << pixels;
#endif
  penSize_ = pixels;

  updateCursor();
}



/**
 * Change the pen type.
 *
 * See the PenType enum documentation for more information.
 *
 * @see PenType
 * @param type The new pen type
 */
void InkCanvas::setPenType( PenType type )
{
#ifdef ISFQT_DEBUG
  qDebug() << "Setting new pen type:" << type;
#endif

  penType_ = type;

  updateCursor();
}



/**
 * Draw a line from the current point to an ending point.
 *
 * The current point is initially set in mousePressEvent();
 * then it's set here.
 *
 * @see mousePressEvent()
 * @param endPoint Final point of the line
 */
void InkCanvas::drawLineTo( const QPoint &endPoint )
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkCanvas!";
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
 * Get whether the drawing is empty.
 *
 * @return True if the Ink image is empty (i.e., no strokes), false otherwise
 */
bool InkCanvas::isEmpty()
{
  return drawing_->isNull();
}



/**
 * Start drawing a new stroke.
 *
 * Look at mouseMoveEvent() and mouseReleaseEvent() for more
 * information on how drawing is done.
 *
 * This method is usually called by Qt.
 *
 * @see mouseMoveEvent()
 * @see mouseReleaseEvent()
 * @param event The mouse button press event
 */
void InkCanvas::mousePressEvent( QMouseEvent *event )
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkCanvas!";
    return;
  }

  if( event->button() != Qt::LeftButton )
  {
    return;
  }

  scribbling_ = true;

  if ( penType_ == EraserPen )
  {
    // is there a stroke here?
    QPoint point = event->pos();
    Stroke *s = drawing_->strokeAtPoint( point );
    if ( s != 0 )
    {
      drawing_->deleteStroke( s );
      drawingDirty_ = true;
      update();
    }

    return;
  }

  lastPoint_ = event->pos();

  // Search if there already is an attributes set compatible with the current one
  Isf::AttributeSet *reusableAttributeSet = 0;
  foreach( Isf::AttributeSet *set, drawing_->attributeSets() )
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
 * Keep on drawing the current stroke.
 *
 * While the mouse is moving, we continue drawing the stroke we started
 * in mousePressEvent(), by drawing a line for each event we receive.
 * Drawing ends when the mouse button is depressed.
 *
 * This method is usually called by Qt.
 *
 * @see mousePressEvent()
 * @see mouseReleaseEvent()
 * @param event The mouse move event
 */
void InkCanvas::mouseMoveEvent( QMouseEvent *event )
{
  if( ! ( event->buttons() & Qt::LeftButton ) || ! scribbling_ )
  {
    return;
  }

  if ( penType_ == EraserPen )
  {
    // is there a stroke here?
    QPoint point = event->pos();
    Stroke *s = drawing_->strokeAtPoint( point );
    if ( s != 0 )
    {
      drawing_->deleteStroke( s );
      drawingDirty_ = true;
      update();
    }

    return;
  }

  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkCanvas!";
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
 * End drawing the current stroke.
 *
 * This method is usually called by Qt.
 *
 * @see mousePressEvent()
 * @see mouseMoveEvent()
 * @param event The mouse release event
 */
void InkCanvas::mouseReleaseEvent( QMouseEvent *event )
{
  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkCanvas!";
    return;
  }

  if( event->button() != Qt::LeftButton || ! scribbling_ )
  {
    return;
  }

  if ( penType_ == EraserPen )
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
 * Clear the internal pixmap buffer.
 *
 * The buffer used to quickly render on the widget painting area
 * the new and old strokes: every new stroke is saved here.
 * This allows to avoid repainting all strokes at every paint event.
 */
void InkCanvas::clearBuffer()
{
  bufferPixmap_ = QPixmap( size() );
  bufferPixmap_.fill( Qt::transparent );
}



/**
 * Repaint the widget.
 *
 * For performance, painting is done using an internal buffer instead
 * of redrawing all strokes everytime.
 *
 * @param event The painting request.
 */
void InkCanvas::paintEvent( QPaintEvent *event )
{
  Q_UNUSED( event );

  QPainter painter( this );

  painter.save();
  painter.setBrush( QBrush( canvasColor_ ) );
  painter.drawRect( QRect(-1, -1, width() + 1 , height() + 1 ) );
  painter.restore();

  if( drawing_ == 0 )
  {
    qWarning() << "Uninitialized usage of InkCanvas!";
    return;
  }

  // draw the ISF first, then the buffer over the top.
  // buffer has a transparent background.
  QPixmap isfPixmap( drawingDirty_
                       ? drawing_->pixmap( Qt::transparent )
                       : isfCachePixmap_ );

  if ( drawingDirty_ )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "ISF pixmap cache dirty; re-caching.";
#endif
    isfCachePixmap_ = isfPixmap;
    drawingDirty_ = false;
  }

  QRect boundingRect = drawing_->boundingRect();

  // draw the pixmap starting at the boundingRect_ top left corner.
  painter.drawPixmap( boundingRect.topLeft(), isfPixmap );

  // draw the buffer from 0,0.
  painter.drawPixmap( QPoint(0, 0), bufferPixmap_ );

  QWidget::paintEvent( event );
}



/**
 * The widget has changed size, re-draw everything.
 *
 * @param event The resizing event.
 */
void InkCanvas::resizeEvent( QResizeEvent *event )
{
  // need to resize the buffer pixmap.
  clearBuffer();

  update();

  QWidget::resizeEvent( event );
}



/**
 * Return the drawn Ink as a QImage.
 *
 * @return A QImage containing the rendered Ink.
 */
QImage InkCanvas::image()
{
  return drawing_->pixmap().toImage();
}



/**
 * Get the current pen color.
 *
 * @return The current pen color.
 */
QColor InkCanvas::penColor()
{
  return color_;
}



/**
 * Get the current pen type.
 *
 * @return The current pen type.
 */
int InkCanvas::penSize()
{
  return penSize_;
}



/**
 * Get the current pen type.
 *
 * See the PenType enum for details.
 *
 * @see PenType
 * @return The current pen type.
 */
InkCanvas::PenType InkCanvas::penType()
{
  return penType_;
}



/**
 * Return the Isf::Drawing instance that the InkCanvas is currently manipulating.
 *
 * Warning: If you call this method without having called setDrawing() previously, the
 *          returned Isf::Drawing pointer will be referencing an Isf::Drawing object
 *          *internal* to InkCanvas.
 *          Do not delete this initial reference or undefined behaviour (read: bad
 *          behaviour and/or crashes) will result.
 *
 * However, you can delete any references you get from here, AFTER you have call setDrawing()
 * for the first time.
 *
 * @see setDrawing()
 * @return The current Isf::Drawing instance.
 */
Isf::Drawing *InkCanvas::drawing()
{
  return drawing_;
}



/**
 * Save the current ISF drawing to a QIODevice.
 *
 * If the base64 param is True, then the drawing will be written base64-encoded.
 * This is helpful for transmission over mediums which are not binary-friendly.
 *
 * @param dev The QIODevice to save to.
 * @param base64 If true, the drawing is written encoded with base64.
 */

void InkCanvas::save( QIODevice &dev, bool base64 )
{
  dev.write( Isf::Stream::writer( *drawing_, base64 ) );
}



/**
 * Returns the drawing's ISF representation.
 *
 * @return A QByteArray filled with ISF data.
 */
QByteArray InkCanvas::bytes()
{
  return Isf::Stream::writer( *drawing_ );
}



/**
 * Set the canvas colour (i.e., the background colour of the InkCanvas control)
 *
 * Note that this is not saved with the Ink drawing, but
 * it is only used to paint the widget.
 *
 * @param newColor The new canvas color.
 */
void InkCanvas::setCanvasColor( QColor newColor )
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
 * Note: InkCanvas does not take ownership of this Isf::Drawing instance.
 * It is the caller's responsibility to delete the given instance.
 *
 * @param drawing The new Ink drawing to display.
 */
void InkCanvas::setDrawing( Isf::Drawing *drawing )
{
  drawing_ = drawing;

  // try to resize of the widget to accommodate the
  // drawing.
  QRect boundingRect = drawing_->boundingRect();
  boundingRect.setTopLeft( QPoint( 0, 0 ) );

  resize( boundingRect.width(), boundingRect.height() );

  drawingDirty_ = true;

  update();
}


