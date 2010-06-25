/***************************************************************************
 *   Copyright (C) 2010 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2010 by Adam Goossens                                   *
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

#include "isfqtstroke.h"

#include "isfqt-internal.h"


using namespace Isf;



/**
 * Constructor
 */
Stroke::Stroke()
: finalized_( true )
, metrics_( 0 )
, transform_( 0 )
{
  bezierInfo_ = new BezierData;
}



/**
 * Copy constructor
 *
 * @param other The object to clone
 */
Stroke::Stroke( const Stroke& other )
: bezierInfo_( 0 )
, metrics_( 0 )
, transform_( 0 )
{
  boundingRect_ = other.boundingRect_;
  finalized_ = other.finalized_;
  points_ = other.points_;
  bezierInfo_ = new BezierData( *other.bezierInfo_ );

  if( other.metrics_ )
  {
    metrics_ = other.metrics_;
  }
  if( other.transform_ )
  {
    transform_ = other.transform_;
  }
}



/**
 * Destructor
 */
Stroke::~Stroke()
{
  delete bezierInfo_;
}



void Stroke::addPoint( Point point )
{
  // Avoid splitting up the logic
  addPoints( PointList() << point );
}



void Stroke::addPoints( PointList points )
{
  points_.append( points );

  // Hunt for pressure info
  if( ! hasPressureData_ )
  {
    foreach( Point point, points )
    {
      if( point.pressureLevel != 0 )
      {
        hasPressureData_ = true;
        break;
      }
    }
  }

  finalized_ = false;
}



/**
 * Returns the rectangle which contains this stroke.
 *
 * @return Rectangle
 */
QRect Stroke::boundingRect() const
{
  return boundingRect_;
}



/**
 * Returns the Bezier curve information
 *
 * Do not delete the object.
 */
BezierData* Stroke::bezierInfo()
{
  return bezierInfo_;
}



QColor Stroke::color() const
{
  return color_;
}



/**
 * Apply the changes to the stroke.
 *
 * Calculating the stroke bounds is complex, so a lazy approach was
 * chosen instead of recalculating them for every added point.
 */
void Stroke::finalize()
{
  if( finalized_ )
  {
    return;
  }

  // A polygon is used to determine the stroke's bounding rect
  quint64 numPoints = points_.count();
  QPolygon polygon( numPoints );
  for( quint64 index = 0; index < numPoints; ++index )
  {
    polygon.setPoint( index, points_.at( index ).position );
  }

  // Transform the bounding rect with the current transformation
  if( transform_ )
  {
    polygon = transform_->map( polygon );
  }

  // Set the bounding rectangle, expanded it to also accommodate pen size
  float halfPenSize = penSize_.width() / 2;
  boundingRect_ = polygon.boundingRect().adjusted( -( halfPenSize ), -( halfPenSize ),
                                                      halfPenSize,      halfPenSize );

  finalized_ = true;
}



/**
 * Get the stroke drawing flags.
 *
 * @return Stroke flags
 */
StrokeFlags Stroke::flags() const
{
  return flags_;
}



/**
 * Get whether the stroke contains pressure information.
 *
 * @return bool
 */
bool Stroke::hasPressureData() const
{
  return hasPressureData_;
}



/**
 * Get the stroke metrics.
 *
 * @return Metrics*
 */
Metrics* Stroke::metrics()
{
  return metrics_;
}



/**
 * Get the current pen size
 *
 * @return QSizeF of the stroke pen
 */
QSizeF Stroke::penSize() const
{
  return penSize_;
}



/**
 * Get the list of points
 *
 * @return List of stroke points
 */
PointList& Stroke::points()
{
  return points_;
}



/**
 * Change pen color
 *
 * @param newColor new pen color
 */
void Stroke::setColor( QColor newColor )
{
  color_ = newColor;
}




/**
 * Change a flag
 *
 * @param flag The flag to alter
 * @param set If true, the flag will be set. If false, it will be unset.
 */
void Stroke::setFlag( StrokeFlag flag, bool set )
{
  if( set )
  {
    flags_ |= flag;
  }
  else
  {
    flags_ ^= flag;
  }
}



/**
 * Change stroke flags all at once.
 *
 * @param newFlags Flags bitfield
 */
void Stroke::setFlags( StrokeFlags newFlags )
{
  flags_ = newFlags;
}



/**
 * Get the current pen size
 *
 * @param newMetrics The new metrics object
 */
void Stroke::setMetrics( Metrics* newMetrics )
{
  metrics_ = newMetrics;
}



/**
 * Get the current pen size
 *
 * @return QSizeF of the stroke pen
 */
void Stroke::setPenSize( QSizeF newSize )
{
  penSize_ = newSize;

  // The bounding box changes with pen size
  finalized_ = false;
}



/**
 * Apply a transformation to the stroke.
 *
 * The stroke doesn't take ownership of the transformation.
 *
 * @param newTransform Transformation to apply
 */
void Stroke::setTransform( QMatrix* newTransform )
{
  transform_ = newTransform;
}



/**
 * Get the stroke transformation, if any
 *
 * @return QMatrix, possibly null
 */
QMatrix* Stroke::transform()
{
  return transform_;
}


