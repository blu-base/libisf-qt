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
, defaultMetrics_( other.defaultMetrics_ )
, defaultAttributeSet_( other.defaultAttributeSet_ )
, defaultStrokeInfo_( other.defaultStrokeInfo_ )
, defaultTransform_( other.defaultTransform_ )
, error_( other.error_ )
, guids_( other.guids_ )
, hasXData_( other.hasXData_ )
, hasYData_( other.hasYData_ )
, isNull_( other.isNull_ )
, maxGuid_( other.maxGuid_ )
, maxPenSize_( other.maxPenSize_ )
, size_( other.size_ )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Copying ISF drawing:" << (void*)&other << "into new:" << this << "**";
#endif

  // First: create copies of all the local lists

  foreach( Metrics *metrics, other.metrics_ )
  {
    Metrics *newMetrics = new Metrics( *metrics );
    metrics_.append( newMetrics );
  }

  foreach( StrokeInfo *strokeInfo, other.strokeInfo_ )
  {
    StrokeInfo *newStrokeInfo = new StrokeInfo( *strokeInfo );
    strokeInfo_.append( newStrokeInfo );
  }

  foreach( QMatrix *transform, other.transforms_ )
  {
    QMatrix *newTransform = new QMatrix( *transform );
    transforms_.append( newTransform );
  }

  foreach( AttributeSet *attributeSet, other.attributeSets_ )
  {
    AttributeSet *newAttributeSet = new AttributeSet( *attributeSet );
    attributeSets_.append( newAttributeSet );
  }

  // Then: clone the strokes. Each stroke is linked to a certain
  // attribute set, metrics set, etc, so we need to associate the
  // original stroke's properties to the current one's
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
  qDeleteAll( metrics_ );
  qDeleteAll( strokeInfo_ );
  qDeleteAll( strokes_ );
  qDeleteAll( transforms_ );
  qDeleteAll( attributeSets_ );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Destroyed ISF drawing:" << this << "**";
#endif
}



/**
 * Add new a attribute set to the drawing.
 *
 * @param newAttributeSet The attribute set to add
 * @return Index of the new attribute set or -1 on failure
 *
qint32 Drawing::addAttributeSet( AttributeSet *newAttributeSet )
{
  if( newAttributeSet == 0 )
  {
    return -1;
  }

  isNull_ = false;

  attributeSets_.append( newAttributeSet );

  // This value is used to adjust the drawing borders to include thick strokes
  if( newAttributeSet->penSize.width() > maxPenSize_.width() )
  {
    maxPenSize_.setWidth( newAttributeSet->penSize.width() );
  }
  if( newAttributeSet->penSize.height() > maxPenSize_.height() )
  {
    maxPenSize_.setHeight( newAttributeSet->penSize.height() );
  }

  return ( attributeSets_.count() - 1 );
}*/



/**
 * Add a new stroke to the drawing.
 *
 * @param newStroke The stroke to add
 * @return Index of the new stroke or -1 on failure
 */
qint32 Drawing::addStroke( QList<Point> points )
{
  return addStroke( new Stroke( points ) );
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

  boundingRect_ = boundingRect_.united( newStroke->boundingRect() );

  // this stroke needs to be repainted.
  changedStrokes_.append( newStroke );

  dirty_ = true;

  return ( strokes_.count() - 1 );
}



/**
 * Add a new transformation to the drawing.
 *
 * @param newTransform The transform to add
 * @return Index of the new transform or -1 on failure
 */
qint32 Drawing::addTransform( QMatrix *newTransform )
{
  if( newTransform == 0 )
  {
    return -1;
  }

  isNull_ = false;
  transforms_.append( newTransform );

  return ( transforms_.count() - 1 );
}



/**
 * Clean up the drawing.
 *
 * This restores the drawing to its initial state: an empty (null) drawing.
 */
void Drawing::clear()
{
  // Clean up the internal property lists
  qDeleteAll( metrics_ );
  qDeleteAll( strokeInfo_ );
  qDeleteAll( strokes_ );
  qDeleteAll( transforms_ );
  qDeleteAll( attributeSets_ );
  metrics_      .clear();
  strokeInfo_   .clear();
  strokes_      .clear();
  transforms_   .clear();
  attributeSets_.clear();
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
  size_         = QSize();
  dirty_        = false;
  cachePixmap_  = QPixmap();

  // set the default transform
  defaultTransform_.scale( 1.f, 1.f );
  defaultTransform_.translate( .0f, .0f );
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

  boundingRect_ = QRect(); // force a recalculation.

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
 * Remove a transformation from the drawing.
 *
 * @param index Index of the transform to delete
 * @return bool
 */
bool Drawing::deleteTransform( quint32 index )
{
  if( (qint64)index >= transforms_.count() )
  {
    return false;
  }

  delete transforms_.takeAt( index );
  return true;
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
 * Return the current bounding rectangle of the drawing.
 *
 * The bounding rectangle (or bounding box) is a QRect large enough (and as
 * small as) to hold all of the strokes in this Drawing instance.
 *
 * @return The current bounding box.
 */
QRect Drawing::boundingRect()
{
  // if the boundingRect_ is invalid, update it.
  // it becomes invalid after a stroke is deleted.
  if( boundingRect_ == QRect() )
  {
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
    }

    const QSize penSize( maxPenSize_.toSize() );
    boundingRect_.adjust( -penSize.width() - 1, -penSize.height() - 1,
                          +penSize.width() + 1, +penSize.height() + 1 );

    size_ = boundingRect_.size();
  }

  return boundingRect_;
}



/**
 * Return the size of this drawing.
 *
 * @return Size of the drawing, in pixels.
 */
QSize Drawing::size()
{
  return boundingRect().size();
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

  QSize size_ = size();

  if( size_.width() > 2000 || size_.height() > 2000 )
  {
    qWarning() << "Cannot render a drawing so big!";
    qDebug()   << "[Information - Size:" << size_ << "pixels]";

    return QPixmap();
  }

  // is the cache null, or are we repainting everything? if so, create a new pixmap.
  if( cachePixmap_.isNull() || changedStrokes_.isEmpty() )
  {
    cachePixmap_ = QPixmap( size_ );
    cachePixmap_.fill( backgroundColor );
    cacheRect_ = boundingRect();
  }
  else
  {
    // otherwise, resize and repaint the cache.

    QRect newRect = boundingRect();

    // has the size of the drawing changed? if so, resize the cachePixmap_.
    if( cacheRect_.size() != newRect.size() )
    {
//       qDebug() << "Cache pixmap needs resizing to" << size_;
//       qDebug() << "Cache rect:" << cacheRect_;
//       qDebug() << "New rect:" << newRect;

      QPixmap pixmap( size_ );
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
  qDebug() << "Rendering a drawing of size" << size_;
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

  painter.setWindow( boundingRect() );
  painter.setWorldMatrixEnabled( true );
  painter.setRenderHints(   QPainter::Antialiasing
                          | QPainter::SmoothPixmapTransform
                          | QPainter::TextAntialiasing,
                          true );

  QPen pen;
  pen.setStyle    ( Qt::SolidLine );
  pen.setCapStyle ( Qt::RoundCap  );
  pen.setJoinStyle( Qt::RoundJoin );

  // Keep record of the currently used properties, to avoid re-setting them for each stroke
  Metrics*    currentMetrics_    = 0;
  StrokeInfo* currentStrokeInfo_ = 0;
  QMatrix*    currentTransform_  = 0;

  int index = 0;
  foreach( Stroke *stroke, strokes )
  {
    if( currentMetrics_ != stroke->metrics() )
    {
      currentMetrics_ = stroke->metrics;
      // TODO need to convert all units somehow?
//       painter.setSomething( currentMetrics );
    }
    if( currentAttributeSet_ != stroke->attributes && stroke->attributes != 0)
    {
      currentAttributeSet_ = stroke->attributes;

      pen.setColor( currentAttributeSet_->color );
      pen.setWidthF( currentAttributeSet_->penSize.width() );
      painter.setPen( pen );
    }
    if( currentStrokeInfo_ != stroke->info )
    {
      currentStrokeInfo_ = stroke->info;
    }
    if( currentTransform_ != stroke->transform && stroke->transform != 0 )
    {
      currentTransform_ = stroke->transform;
      painter.setWorldTransform( QTransform( *currentTransform_ ), false );

      // the problem with setting the world transform is that it will scale the pen size too.
      // we don't want that. so we have to artificially beef up the pen size.
      QPen pen = painter.pen();
      pen.setWidthF( currentAttributeSet_->penSize.width() / currentTransform_->m22() );
      painter.setPen( pen );
      painter.setBrush( Qt::transparent );
    }

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke->points.count() << "points";
    qDebug() << "- Stroke color:" << currentAttributeSet_->color.name() << "Pen size:" << pen.widthF();
#endif

    if( stroke->points.count() > 1 )
    {

      bool fitToCurve = stroke->attributes->flags & FitToCurve;
      QPainterPath path = generatePainterPath( stroke, fitToCurve );

      // TODO: pressure data.
      painter.drawPath( path );
    }
    else
    {
      Point point = stroke->points.first();
      if( currentStrokeInfo_->hasPressureData )
      {
        // FIXME Ignoring pressure data - need to find out how pressure must be applied
//         pen.setWidth( pen.widthF() + point.pressureLevel );
//         painter.setPen( pen );
      }

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

    ++index;
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
    Stroke *s = i.previous();;

    // skip strokes where we're not near.
    if ( ! s->boundingRect.contains( point ) )
    {
      continue;
    }

    // what's the pen size of this stroke? That way we have a "fudge factor"
    AttributeSet *set = s->attributes;
    float penSize = set->penSize.width();
    float penHalfSize = penSize / 2;

    // only want points that fall near the cursor. prevents searching unnecessary points.
    QRect searchRect;

    // search rect must accommodate pen size.
    // the large the pen size, the bigger our search area has to be.
    //
    // minimum search area of 10x10 pixels.
    if ( penHalfSize > 5 )
    {
      // 25% extra room to move.
      searchRect = QRect( point.x() - penHalfSize * 1.25, point.y() - penHalfSize * 1.25, penHalfSize * 1.25, penHalfSize * 1.25 );
    }
    else
    {
      searchRect = QRect( point.x() - 5, point.y() - 5, 10, 10 );
    }

    // special case: a single point (sometimes it'll appear as a single point but
    // be made up of two).
    if ( s->points.size() == 1 || s->points.size() == 2 )
    {
      QLineF dist = QLineF( QPointF( s->points.at(0).position ), QPointF( point ) );
      if ( dist.length() <= penHalfSize * 1.25 )
      {
        return s;
      }
      continue;
    }

    // multiple points.
    for( int j = 0; j < s->points.size() - 1; j++)
    {
      QPoint p1 = s->points.at(j).position;
      QPoint p2 = s->points.at(j+1).position;

      if ( ! searchRect.contains( p1 ) && ! searchRect.contains( p2 ) )
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
      float sp = 0.5*( lineA.length() + base.length() + lineC.length() );
      float a = lineA.length();
      float b = base.length();
      float c = lineC.length();
      float area = sqrt( sp * (sp - a) * (sp - b) * (sp - c) );

      float height = ( 2 * area ) / b;

      if ( height <= penHalfSize * 1.25 )
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
 * Retrieve a transformation to manipulate.
 *
 * @param index Index of the transform to get
 * @return QMatrix or 0 if not found
 */
QMatrix *Drawing::transform( quint32 index )
{
  if( (qint64)index >= transforms_.count() )
  {
    return 0;
  }

  return transforms_.at( index );
}



/**
 * Retrieve the transformations.
 *
 * @return The list of existing transformations
 */
const QList<QMatrix*> Drawing::transforms()
{
  return transforms_;
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


