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
#include "bezierspline.h"

#include <IsfQtDrawing>

#include <QPainter>
#include <QPixmap>

#include <cmath>


using namespace Isf;
using namespace Compress;



/**
 * Construct a new empty (null) Drawing instance.
 *
 * As soon as you add data, the instance becomes non-NULL.
 */
Drawing::Drawing()
: error_( ISF_ERROR_NONE )
, hasXData_( true )
, hasYData_( true )
, isNull_( true )
, maxGuid_( 0 )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Created new ISF drawing:" << this << "**";
#endif
  clear();
}



/**
 * Construct a copy of an existing Drawing instance.
 *
 * @param other The instance to duplicate.
 */
Drawing::Drawing( const Drawing &other )
: boundingRect_( other.boundingRect_ )
, canvas_( other.canvas_ )
, error_( other.error_ )
, guids_( other.guids_ )
, hasXData_( other.hasXData_ )
, hasYData_( other.hasYData_ )
, isNull_( other.isNull_ )
, maxGuid_( other.maxGuid_ )
, maxPenSize_( other.maxPenSize_ )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Copying ISF drawing:" << (void*)&other << "into new:" << this << "**";
#endif

  // Clone the stroke objects
  foreach( Stroke *stroke, other.strokes_ )
  {
    Stroke *newStroke = new Stroke( *stroke );
    strokes_.append( newStroke );
  }
}


/**
 * Destructor
 */
Drawing::~Drawing()
{
  qDeleteAll( strokes_ );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Destroyed ISF drawing:" << this << "**";
#endif
}



/**
 * Add a new stroke to the drawing.
 *
 * @param newStroke The stroke to add
 * @return Index of the new stroke or -1 on failure
 */
qint32 Drawing::addStroke( PointList points )
{
  Stroke* newStroke = new Stroke();

  newStroke->addPoints( points );
  newStroke->finalize();

  return addStroke( newStroke );
}



/**
 * Add a new stroke to the drawing.
 *
 * @param newStroke The stroke to add
 * @return Index of the new stroke or -1 on failure
 */
qint32 Drawing::addStroke( Stroke *newStroke )
{
  if( newStroke == 0 )
  {
    return -1;
  }

  isNull_ = false;
  strokes_.append( newStroke );
  maxPenSize_ = maxPenSize_.width() < newStroke->penSize().width() ? newStroke->penSize() : maxPenSize_;

  // This stroke needs to be painted
  changedStrokes_.append( newStroke );

  dirty_ = true;

  updateBoundingRect();

  return ( strokes_.count() - 1 );
}




/**
 * Return the current bounding rectangle of the drawing.
 *
 * The bounding rectangle (or bounding box) is a QRect large enough (and as
 * small as) to hold all of the strokes in this Drawing instance.
 *
 * @return The current bounding box.
 */
QRect Drawing::boundingRect() const
{
  return boundingRect_;
}



/**
 * Clean up the drawing.
 *
 * This restores the drawing to its initial state: an empty (null) drawing.
 */
void Drawing::clear()
{
  // Clean up the internal property lists
  qDeleteAll( strokes_ );
  strokes_       .clear();
  changedStrokes_.clear();

  // Nullify the other properties
  boundingRect_ = QRect();
  canvas_       = QRect();
  cacheRect_    = QRect();
  error_        = ISF_ERROR_NONE;
  hasXData_     = true;
  hasYData_     = true;
  isNull_       = true;
  maxGuid_      = 0;
  maxPenSize_   = QSizeF();
  dirty_        = false;
  cachePixmap_  = QPixmap();
}



/**
 * Remove a stroke from the drawing.
 *
 * Delete a stroke object from the drawing.
 * @return bool
 */
bool Drawing::deleteStroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return false;
  }

  Stroke *victim = strokes_.takeAt( index );

  // make sure this goes from the changedStrokes_ list too.
  changedStrokes_.removeAll( victim );

  delete victim;

  dirty_ = true;

  updateBoundingRect();

  return true;
}



/**
 * Delete a stroke object from the drawing.
 *
 * @param stroke The stroke to remove.
 * @return bool
 */
bool Drawing::deleteStroke( Stroke *stroke )
{
  if ( ! strokes_.contains( stroke ) )
  {
    return false;
  }

  return deleteStroke( strokes_.indexOf( stroke ) );
}



/**
 * Return the last error that has occurred.
 *
 * If nothing went wrong, this returns ISF_ERROR_NONE.
 *
 * @return The last ISF error status
 */
IsfError Drawing::error() const
{
  return error_;
}



/**
 * Given a list of knot points, generates a QPainterPath that describes the stroke.
 *
 * If fitToCurve is true, uses bezier curves to approximate the stroke, giving a much smoother appearance.
 * See the comments in BezierSpline::calculateControlPoints.
 *
 * @param knotPoints The known points of the curve (the "knot" points).
 * @param fitToCurve If true, bezier approximation is used to smooth the resulting curve.
 */
QPainterPath Drawing::generatePainterPath( Stroke *stroke, bool fitToCurve )
{
  QList<Point>& strokePoints = stroke->points();

  if ( strokePoints.size() == 0 )
  {
    return QPainterPath();
  }

  QPoint startPos( strokePoints.at(0).position );

  QPainterPath path( QPointF( startPos.x(), startPos.y() ) );

  if ( ! fitToCurve )
  {
    foreach( Point point, strokePoints )
    {
      path.lineTo( point.position );
    }

  }
  else
  {
    BezierData* bezier = stroke->bezierInfo();

    // don't calculate control points if they've
    // already been calculated.
    if( bezier->knotPoints.isEmpty() )
    {
      // for a better curve, don't pass through all of points.
      // skip about 70% of them.
      int toSkip = 0.70 * strokePoints.size();
      int step = strokePoints.size() / ( strokePoints.size() - toSkip );

      step = ( step < 1 ) ? 1 : step;   // sanity check.

      QList<QPointF> points;

      for( int i = 0; i < strokePoints.size(); i += step )
      {
        points.append( strokePoints.at(i).position );
      }

      // always pass through the last point.
      points.append( strokePoints.last().position );

      QList<QPointF> c1;
      QList<QPointF> c2;

      // generate the bezier control points.
      BezierSpline::calculateControlPoints( points, &c1, &c2 );

      bezier->c1 = c1;
      bezier->c2 = c2;
      bezier->knotPoints = points;
    }

    for( int i = 0; i < bezier->c1.size(); i++ )
    {
      // draw the bezier curve!
      path.cubicTo( bezier->c1[ i ], bezier->c2[ i ], bezier->knotPoints[ i + 1 ] );
    }
  }

  return path;
}



/**
 * Return the index of a certain stroke.
 *
 * @param stroke Stroke to search
 * @return Index or -1 if not found
 */
qint32 Drawing::indexOfStroke( const Stroke* stroke ) const
{
  return strokes_.indexOf( const_cast<Stroke*>( stroke ) );
}



/**
 * Return whether this drawing is empty.
 *
 * @return True if this is an empty (null) Drawing, false otherwise.
 */
bool Drawing::isNull() const
{
  return isNull_;
}



/**
 * Render the drawing into an image.
 *
 * If the drawing is empty, the pixmap will be null.
 * Also note that the maximum renderable size is 2000x2000 pixels: if the strokes
 * exceed by even one dimension, the pixmap will not be rendered.
 * This is a security measure, since the pixmaps are allocated by the graphics
 * system (X on *nix, for example) and a machine could be DoSed with a very large
 * drawing.
 *
 * @param backgroundColor The color used as background in the returned image.
 *                        Default is transparent.
 * @return The rendered drawing, or a null one on error.
 */
QPixmap Drawing::pixmap( const QColor backgroundColor )
{
  if( isNull() )
  {
    return QPixmap();
  }

  if( ! dirty_ && ! cachePixmap_.isNull() )
  {
    return cachePixmap_;
  }

  QSize drawingSize( size() );

  if( drawingSize.width() > 2000 || drawingSize.height() > 2000 )
  {
    qWarning() << "Cannot render a drawing so big!";
    qDebug()   << "[Information - Size:" << drawingSize << "pixels]";

    return QPixmap();
  }

  // is the cache null, or are we repainting everything? if so, create a new pixmap.
  if( cachePixmap_.isNull() || changedStrokes_.isEmpty() )
  {
    cachePixmap_ = QPixmap( drawingSize );
    cachePixmap_.fill( backgroundColor );
    cacheRect_ = boundingRect_;
  }
  else
  {
    // otherwise, resize and repaint the cache.

    QRect newRect = boundingRect_;

    // has the size of the drawing changed? if so, resize the cachePixmap_.
    if( cacheRect_.size() != newRect.size() )
    {
//       qDebug() << "Cache pixmap needs resizing to" << drawingSize;
//       qDebug() << "Cache rect:" << cacheRect_;
//       qDebug() << "New rect:" << newRect;

      QPixmap pixmap( drawingSize );
      pixmap.fill( backgroundColor );
      QPainter painter( &pixmap );

      int xOffset = ( newRect.x() - cacheRect_.x() ) * -1;
      int yOffset = ( newRect.y() - cacheRect_.y() ) * -1;

//       qDebug() << "x-offset:"<<xOffset<<", y-offset:"<<yOffset;
      painter.drawPixmap( xOffset, yOffset, cachePixmap_ );

      cachePixmap_ = pixmap;
      cacheRect_ = newRect;
    }
  }

  // if we're told specifically what strokes to paint, only paint those ones.
  // otherwise, paint all of them.

  QList<Stroke*> strokes = ( changedStrokes_.isEmpty() ? strokes_ : changedStrokes_ );

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering a drawing of size" << drawingSize;
#endif

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Rendering" << strokes.count() << "out of" << strokes_.count() << "strokes in the drawing.";
#endif

  // if there are no strokes there's no point going
  // through the rest of this logic.
  if ( strokes.count() == 0 )
  {
    return cachePixmap_;
  }

  QPainter painter( &cachePixmap_ );

  painter.setWindow( boundingRect_ );
  painter.setWorldMatrixEnabled( true );
  painter.setRenderHints(   QPainter::Antialiasing
                          | QPainter::SmoothPixmapTransform
                          | QPainter::TextAntialiasing,
                          true );

  QPen pen;
  pen.setStyle    ( Qt::SolidLine );
  pen.setCapStyle ( Qt::RoundCap  );
  pen.setJoinStyle( Qt::RoundJoin );

  // Keep record of the last used properties, to avoid re-setting them for each stroke
  AttributeSet currentAttributes;
  Metrics*     currentMetrics    = 0;
  QMatrix*     currentTransform  = 0;

  int index = 0;
  foreach( Stroke *stroke, strokes )
  {
    if( currentAttributes.color   != stroke->color()
    ||  currentAttributes.flags   != stroke->flags()
    ||  currentAttributes.penSize != stroke->penSize() )
    {
      currentAttributes.color   = stroke->color();
      currentAttributes.flags   = stroke->flags();
      currentAttributes.penSize = stroke->penSize();

      pen.setColor( stroke->color() );
      pen.setWidthF( stroke->penSize().width() );
      painter.setPen( pen );
    }
    if( stroke->metrics() && currentMetrics != stroke->metrics() )
    {
      currentMetrics = stroke->metrics();
      // TODO need to convert all units somehow?
//       painter.setSomething( currentMetrics );
    }
    if( stroke->transform() && currentTransform != stroke->transform() )
    {
      currentTransform = stroke->transform();
      painter.setWorldTransform( QTransform( *currentTransform ), false );

      // the problem with setting the world transform is that it will scale the pen size too.
      // we don't want that. so we have to artificially beef up the pen size.
      QPen pen = painter.pen();
      pen.setWidthF( stroke->penSize().width() / currentTransform->m22() );

/*
      // FIXME Ignoring pressure data - need to find out how pressure must be applied
      pen.setWidth( pen.widthF() + points??.pressureLevel );
*/

      painter.setPen( pen );
      painter.setBrush( Qt::transparent );
    }

    const PointList& points = stroke->points();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke->points().count() << "points";
    qDebug() << "- Stroke color:" << stroke->color().name() << "Pen size:" << pen.widthF();
#endif

    ++index;

    if( points.count() == 0 )
    {
      continue;
    }

    // TODO: pressure data.
    if( points.count() > 1 )
    {
      bool fitToCurve = stroke->flags() & FitToCurve;
      QPainterPath path = generatePainterPath( stroke, fitToCurve );

      painter.drawPath( path );
    }
    else
    {
      Point point = stroke->points().first();

//       qDebug() << "Point:" << point.position;
      painter.drawPoint( point.position );
    }

/*
#ifdef ISFQT_DEBUG_VERBOSE
    // Draw the stroke number next to each one, for debugging purposes
    pen.setColor( QColor( Qt::red ) );
    painter.setPen( pen );
    painter.drawText( stroke.points.first().position, QString::number( index ) );
    pen.setColor( currentAttributeSet_->color );
    painter.setPen( pen );
#endif
*/
  }

  painter.end();

  changedStrokes_.clear();

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering complete.";
#endif

  dirty_ = false;

  return cachePixmap_;
}



/**
 * Change the bounding rectangle of the drawing.
 *
 * Adding, removing or changing the strokes will change it back to the real rectangle.
 *
 * @param newRect the new bounding rectangle
 */
void Drawing::setBoundingRect( QRect newRect )
{
  boundingRect_ = newRect;
}



/**
 * Return the size of this drawing.
 *
 * @return Size of the drawing, in pixels.
 */
QSize Drawing::size() const
{
  return boundingRect_.size();
}



/**
 * Retrieve a stroke to manipulate.
 *
 * @param index Index of the stroke to get
 * @return Stroke or 0 if not found
 */
Stroke *Drawing::stroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return 0;
  }

  return strokes_.at( index );
}



/**
 * Return the Stroke under a certain point.
 *
 * Given a QPoint, return the Stroke object that passes through there.
 * If no stroke passes through that point, returns NULL.
 *
 * If multiple Strokes pass through that point the most recently drawn stroke
 * will be returned.
 *
 * @param point Point to check
 * @return A Stroke instance or NULL if no Stroke passes through that point.
 */
Stroke *Drawing::strokeAtPoint( const QPoint &point )
{
  /*
  Here's how this algorithm works:

  1) Iterate through strokes in reverse order.
  2) For each stroke, check if the bounding rectangle contains the point where the cursor is.
     If not, continue to the next stroke. Prevents us checking all the strokes.
  3) Next, iterate through each pair of points in the stroke. If both of the points fall outside
     of the searching rectangle around the cursor, skip. Saves us checking hundreds of points when
     only a few will do.
  4) a) For each pair of strokes, form a triangle whose points are defined by the two points for the
        line segment, plus the cursor position.
     b) Calculate the height of this triangle. To do so, use Heron's Formula plus the formula for the
        area of any triangle.
     c) If the cursor is touching the drawn stroke, the height should be less than or equal to the
        half-width of the pen that drew the stroke.

    For reference (triangle sides a, b, c, height h)
    Heron's Formula: area = sqrt(s * (s - a) * (s - b) * (s - c) ), where s = semiperimeter = 0.5*(a+b+c)
                     area = 0.5*base*height;
                     thus height = ( 2 * area ) / base.
  */

  QListIterator<Stroke *> i( strokes_ );
  i.toBack();

  while( i.hasPrevious() )
  {
    Stroke *s = i.previous();

    // skip strokes where we're not near.
    if ( ! s->boundingRect().contains( point ) )
    {
      continue;
    }

    // what's the pen size of this stroke? That way we have a "fudge factor"
    float penSize = s->penSize().width();
    float penHalfSize = penSize / 2;
    float penHalfSizeFixed = penHalfSize * 1.25;

    // only want points that fall near the cursor. prevents searching unnecessary points.
    QRect searchRect;

    // search rect must accommodate pen size.
    // the large the pen size, the bigger our search area has to be.
    //
    // minimum search area of 10x10 pixels.
    if ( penHalfSize > 5 )
    {
      // 25% extra room to move.
      searchRect = QRect( point.x() - penHalfSizeFixed,
                          point.y() - penHalfSizeFixed,
                          penHalfSizeFixed, penHalfSizeFixed );
    }
    else
    {
      searchRect = QRect( point.x() - 5, point.y() - 5, 10, 10 );
    }

    // special case: a single point (sometimes it'll appear as a single point but
    // be made up of two).
    const PointList &points( s->points() );
    if( points.size() == 1 || points.size() == 2 )
    {
      QLineF dist = QLineF( QPointF( points.at(0).position ), QPointF( point ) );
      if( dist.length() <= penHalfSizeFixed )
      {
        return s;
      }

      continue;
    }

    // multiple points.
    for( int j = 0; j < points.size() - 1; j++)
    {
      QPoint p1 = points.at(j).position;
      QPoint p2 = points.at(j+1).position;

      if( ! searchRect.contains( p1 ) && ! searchRect.contains( p2 ) )
      {
        continue;
      }

      QPointF cursorPos = QPointF( point );
      QLineF base = QLineF( QPointF( p1 ), QPointF( p2 ) );
      QLineF lineA = QLineF( QPointF( p1 ), cursorPos );
      QLineF lineC = QLineF( QPointF( p2 ), cursorPos );

      // picture a triangle made up of the two points for the line segment, plus the
      // cursor position. The height of the triangle is the distance from the cursor point
      // to the line segment. If the cursor lies on the line, then the height should be less than
      // or equal to the half-width of the pen that drew the line.
      //
      // so, use Heron's Formula to get the area, plus A=0.5*base*height, re-arrange to get height.
      //
      // easy!
      float sp = 0.5 * ( lineA.length() + base.length() + lineC.length() );
      float a = lineA.length();
      float b = base.length();
      float c = lineC.length();
      float area = sqrt( sp * (sp - a) * (sp - b) * (sp - c) );

      float height = ( 2 * area ) / b;

      if ( height <= penHalfSizeFixed )
      {
        // got one
        return s;
      }
    }
  }

  return 0;
}



/**
 * Retrieve the strokes.
 *
 * @return The list of existing strokes
 */
const QList<Stroke*> Drawing::strokes()
{
  return strokes_;
}




/**
 * Update the bounding rectangle of the drawing.
 *
 * The bounding rectangle (or bounding box) is a QRect large enough (and as
 * small as) to hold all of the strokes in this Drawing instance.
 */
void Drawing::updateBoundingRect()
{
#ifdef ISFQT_DEBUG_VERBOSE
  QRect oldRect( boundingRect_ );
#endif

  boundingRect_ = QRect();
  foreach( Stroke *stroke, strokes_ )
  {
    const QRect rect( stroke->boundingRect() );
    QMatrix* transform = stroke->transform();
    if( transform != 0 )
    {
      boundingRect_ = boundingRect_.united( transform->mapRect( rect ) );
    }
    else
    {
      boundingRect_ = boundingRect_.united( rect );
    }

    maxPenSize_ = maxPenSize_.width() < stroke->penSize().width() ? stroke->penSize() : maxPenSize_;
  }

  const QSize penSize( maxPenSize_.toSize() );
  boundingRect_.adjust( -penSize.width() - 1, -penSize.height() - 1,
                        +penSize.width() + 1, +penSize.height() + 1 );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Bounding rectangle updated: from" << oldRect << "to" << boundingRect_;
#endif
}


