/***************************************************************************
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

#include <IsfQtDrawing>

#include <QPainter>
#include <QPixmap>

#include <cmath>

using namespace Isf;
using namespace Compress;



/**
 * Construct a new NULL Drawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
Drawing::Drawing()
: currentMetrics_( 0 )
, currentAttributeSet_( 0 )
, currentStrokeInfo_( 0 )
, currentTransform_( 0 )
, error_( ISF_ERROR_NONE )
, hasXData_( true )
, hasYData_( true )
, isNull_( true )
, maxGuid_( 0 )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Created new ISF drawing **";
#endif
  clear();
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
  qDebug() << "** Destroyed ISF drawing object **";
#endif
}



/**
 * Add new a attribute set to the drawing
 *
 * @param attributes The attribute set to add
 * @return Index of the new attribute set or -1 on failure
 */
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
}



/**
 * Add a new stroke to the drawing
 *
 * @param attributes The stroke to add
 * @return Index of the new stroke or -1 on failure
 */
qint32 Drawing::addStroke( Stroke *newStroke )
{
  if( newStroke == 0 )
  {
    return -1;
  }

  quint64 numPoints = newStroke->points.count();

  // The polygon is used to determine the stroke's bounding rect
  QPolygon polygon( numPoints );
  for( quint64 index = 0; index < numPoints; ++index )
  {
    polygon.setPoint( index, newStroke->points.at( index ).position );
  }
  
  // assign the attributes to the stroke.
  newStroke->attributes = currentAttributeSet_;
  
  // add FitToCurve (Windows will use Bezier smoothing to make the ink look nice)
  newStroke->attributes->flags |= FitToCurve;

  // set the bounding rectangle.
  if ( polygon.boundingRect().size() == QSize(1, 1) )
  {
    // can't have a 1px by 1px bounding rect - the eraser will never hit it.
    // make the bounding rectange completely cover the drawn stroke.
    float penSize = newStroke->attributes->penSize.width();
    QPoint point = newStroke->points.at( 0 ).position;
    newStroke->boundingRect = QRect( point.x() - penSize / 2, point.y() - penSize / 2, penSize, penSize );
  }
  else
  {
    newStroke->boundingRect = polygon.boundingRect();
  }

  isNull_ = false;
  strokes_.append( newStroke );

  // force a bounding rectangle update
  boundingRect_ = QRect();
  
  return ( strokes_.count() - 1 );
}



/**
 * Add a new transformation to the drawing
 *
 * @param attributes The transform to add
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
 * Clean up the drawing
 *
 * This restores the drawing to its initial state, an empty drawing.
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

  // Invalidate the current item pointers
  currentMetrics_      = 0;
  currentAttributeSet_ = 0;
  currentStrokeInfo_   = 0;
  currentTransform_    = 0;

  // Nullify the other properties
  boundingRect_ = QRect();
  canvas_       = QRect();
  error_        = ISF_ERROR_NONE;
  hasXData_     = true;
  hasYData_     = true;
  isNull_       = true;
  maxGuid_      = 0;
  maxPenSize_   = QSizeF();
  size_         = QSize();
  
  // add a default transform.
  QMatrix *transform = new QMatrix();
  transform->scale( 1.f, 1.f );
  transform->translate( .0f, .0f );

  transforms_.append( transform );
}



/**
 * Remove some attribute set from the drawing
 *
 * @param index Index of the attribute set to delete
 * @return bool
 */
bool Drawing::deleteAttributeSet( quint32 index )
{
  if( (qint64)index >= attributeSets_.count() )
  {
    return false;
  }

  delete attributeSets_.takeAt( index );
  
  // re-calculate max pen size
  foreach( AttributeSet *set, attributeSets_ )
  {
    // This value is used to adjust the drawing borders to include thick strokes
    if( set->penSize.width() > maxPenSize_.width() )
    {
      maxPenSize_.setWidth( set->penSize.width() );
    }
    if( set->penSize.height() > maxPenSize_.height() )
    {
      maxPenSize_.setHeight( set->penSize.height() );
    }
  }

  qDebug() << "Max Pen Size:"<<maxPenSize_;
  return true;
}



/**
 * Remove a stroke from the drawing
 *
 * @param index Index of the stroke to delete
 * @return bool
 */
bool Drawing::deleteStroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return false;
  }

  delete strokes_.takeAt( index );
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
 * Remove a transformation from the drawing
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
 * Return the last error that has occurred
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
 * Retrieve an attribute set to manipulate it
 *
 * @param index Index of the attribute set to get
 * @return AttributeSet or 0 if not found
 */
AttributeSet *Drawing::getAttributeSet( quint32 index )
{
  if( (qint64)index >= attributeSets_.count() )
  {
    return 0;
  }

  return attributeSets_.at( index );
}



/**
 * Retrieve the attribute sets
 *
 * @return The list of existing attribute sets
 */
const QList<AttributeSet*> Drawing::getAttributeSets()
{
  return attributeSets_;
}




/**
 * Return a QRect that will hold all of the strokes in this Drawing instance.
 * @return A QRect object that is just large enough to hold all strokes.
 */
QRect Drawing::getBoundingRect()
{
  // if the boundingRect_ is invalid, update it.
  // it becomes invalid after a stroke is added or deleted.
  if ( boundingRect_ == QRect() )
  {
    foreach( Stroke *stroke, strokes_ )
    {
      boundingRect_ = boundingRect_.united( stroke->boundingRect );
    }

    QSize penSize( maxPenSize_.toSize() );
    boundingRect_.adjust( -penSize.width() - 1, -penSize.height() - 1,
                          +penSize.width() + 1, +penSize.height() + 1 );

    size_ = boundingRect_.size();
  }

  return boundingRect_;
}



/**
 * Return the size of this drawing, in pixels.
 * @return Size of the drawing, in pixels.
 */
QSize Drawing::getSize()
{
  return getBoundingRect().size();
}



QPixmap Drawing::getPixmap( const QColor backgroundColor )
{
  if ( isNull() )
  {
    return QPixmap();
  }

  QSize size_ = getSize();
  
  QPixmap pixmap( size_ );
  pixmap.fill( backgroundColor );
  QPainter painter( &pixmap );

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering a drawing of size" << size_;
#endif

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "The drawing contains" << strokes_.count() << "strokes.";
#endif

  // if there are no strokes there's no point going
  // through the rest of this logic.
  if ( strokes_.count() == 0 )
  {
    return pixmap;
  }

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
  
  // Keep record of the currently used properties, to avoid re-setting them for each stroke
  currentMetrics_       = 0;
  currentAttributeSet_  = 0;
  currentStrokeInfo_    = 0;
  currentTransform_     = 0;

  int index = 0;
  foreach( const Stroke *stroke, strokes_ )
  {
    if( currentMetrics_ != stroke->metrics )
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
      painter.setWorldTransform( QTransform( *currentTransform_ ), true );
    }

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke->points.count() << "points";
    qDebug() << "- Stroke color:" << currentAttributeSet_->color.name() << "Pen size:" << pen.widthF();
#endif

    if( stroke->points.count() > 1 )
    {
      Point lastPoint;
      foreach( const Point &point, stroke->points )
      {
//       qDebug() << "Point:" << point.position;

        if( lastPoint.position.isNull() )
        {
          lastPoint = point;
          continue;
        }

        if( currentStrokeInfo_->hasPressureData )
        {
          // FIXME Ignoring pressure data - need to find out how pressure must be applied
//           pen.setWidth( pen.widthF() + point.pressureLevel );
//           painter.setPen( pen );
        }

        // How nice of QPainter! Lines drawn from and to the same point
        // won't be drawn at all
        if( point.position == lastPoint.position )
        {
          painter.drawPoint( point.position );
        }
        else
        {
          painter.drawLine( lastPoint.position, point.position );
        }

        lastPoint = point;
      }
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

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering complete.";
#endif

  return pixmap;
}



/**
 * Retrieve a stroke to manipulate it
 *
 * @param index Index of the stroke to get
 * @return Stroke or 0 if not found
 */
Stroke *Drawing::getStroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return 0;
  }

  return strokes_.at( index );
}



/**
 * Given a QPoint, return the Stroke object that
 * passes through there. If no stroke passes through that point,
 * returns NULL.
 *
 * If multiple Strokes pass through that point the most recently drawn stroke
 * will be returned.
 *
 * @param point Point to check
 * @return A Stroke instance or NULL if no Stroke passes through that point.
 */
Stroke *Drawing::getStrokeAtPoint( QPoint point )
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
 * Retrieve the strokes
 *
 * @return The list of existing strokes
 */
const QList<Stroke*> Drawing::getStrokes()
{
  return strokes_;
}



/**
 * Retrieve a transformation to manipulate it
 *
 * @param index Index of the transform to get
 * @return QMatrix or 0 if not found
 */
QMatrix *Drawing::getTransform( quint32 index )
{
  if( (qint64)index >= transforms_.count() )
  {
    return 0;
  }

  return transforms_.at( index );
}



/**
 * Retrieve the transformations
 *
 * @return The list of existing transformations
 */
const QList<QMatrix*> Drawing::getTransforms()
{
  return transforms_;
}



/**
 * Return True if this instance of Drawing is invalid (NULL), False otherwise.
 *
 * @return True if this is a NULL Drawing, FALSE otherwise.
 */
bool Drawing::isNull() const
{
  return isNull_;
}




/**
 * Change the current attribute set
 *
 * This will change the attribute set which will be applied to the next strokes.
 *
 * @param attributeSet the new attribute set
 * @return bool
 */
bool Drawing::setCurrentAttributeSet( AttributeSet *attributeSet )
{
  if( ! attributeSets_.contains( attributeSet ) )
  {
    return false;
  }

  currentAttributeSet_ = attributeSet;

  return true;
}



/**
 * Change the current transformation
 *
 * This will change the transformation which will be applied to the next strokes.
 *
 * @param attributeSet the new transformation
 * @return bool
 */
bool Drawing::setCurrentTransform( QMatrix *transform )
{
  if( ! transforms_.contains( transform ) )
  {
    return false;
  }

  currentTransform_ = transform;

  return true;
}


