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

#include <QPainterPath>


using namespace Isf;



/**
 * Constructor
 */
Stroke::Stroke()
: finalized_( true )
, metrics_( 0 )
, transform_( 0 )
{
}



/**
 * Copy constructor
 *
 * @param other The object to clone
 */
Stroke::Stroke( const Stroke& other )
{
  boundingRect_ = other.boundingRect_;
  finalized_ = other.finalized_;
  points_ = other.points_;
  metrics_ = other.metrics_;
  transform_ = other.transform_;
}



/**
 * Destructor
 */
Stroke::~Stroke()
{
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
 * Given a series of known (knot) points, knots, calculates the control points (c1, c2) to approximate a series of
 * bezier curves passing smoothly through the knot points.
 */
void Stroke::bezierCalculateControlPoints()
{
//   bezierKnots_.clear();
//   bezierControlPoints1_.clear();
//   bezierControlPoints2_.clear();

  int numPoints = points_.size();

  // For a better curve, don't pass through all of points:
  // skip about 70% of them
  int toSkip = 0.70 * numPoints;
  int step = numPoints / ( numPoints - toSkip );
  step = ( step < 1 ) ? 1 : step; // Sanity check

  for( int i = 0; i < numPoints; i += step )
  {
    bezierKnots_.append( points_.at(i).position );
  }

  // always pass through the last point.
  bezierKnots_.append( points_.last().position );

  /////////////////////////////////////////////////////////////////////////////

  if( bezierKnots_.size() == 0 )
  {
    return;
  }

  int n = bezierKnots_.size() - 1;
  if( n < 1 )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Require at least two knot points to generate Bezier control points; ignoring.";
#endif
    return;
  }

  // special case: Bezier curve should be a straight line.
  if( n == 1 )
  {
    QPointF cp1, cp2;
    QPointF k1 = bezierKnots_.at(0);   // knot point 1
    QPointF k2 = bezierKnots_.at(1);   // knot point 2

    cp1.setX( ( 2 * k1.x() + k2.x() ) / 3.0 );
    cp1.setY( ( 2 * k1.y() + k2.y() ) / 3.0 );

    cp2.setX( ( 2 * cp1.x() - k1.x() ) );
    cp2.setY( ( 2 * cp1.y() - k1.y() ) );

    bezierControlPoints1_.append( cp1 );
    bezierControlPoints2_.append( cp2 );

    return; // done!
  }

  // right hand side vector.
  double rhs[n];

  // set RHS x values
  for( int i = 1; i < n-1; i++ )
  {
    rhs[i] = 4 * bezierKnots_[i].x() + 2 * bezierKnots_[i+1].x();
  }

  rhs[0] = bezierKnots_[0].x() + 2 * bezierKnots_[1].x();
  rhs[n-1] = ( 8 * bezierKnots_[n-1].x() + bezierKnots_[n].x() ) / 2.0;

  // get the first ctl points x values.
  double x[n];
  bezierGetFirstControlPoints( rhs, x, n );


  // now set RHS y-values.
  for(int i = 1; i < n-1; i++ )
  {
    rhs[i] = 4 * bezierKnots_[i].y() + 2 * bezierKnots_[i+1].y();
  }

  rhs[0] = bezierKnots_[0].y() + 2 * bezierKnots_[1].y();
  rhs[n-1] = ( 8 * bezierKnots_[n-1].y() + bezierKnots_[n].y() ) / 2.0;

  double y[n];
  bezierGetFirstControlPoints( rhs, y, n );

  // now fill the output QList.
  for( int i = 0; i < n; i++ )
  {
    QPointF cp1( x[i], y[i] );
    QPointF cp2;

    // second ctl point
    if ( i < n-1 )
    {
      cp2 = QPointF( 2 * bezierKnots_[i+1].x() - x[i+1],
                     2 * bezierKnots_[i+1].y() - y[i+1] );
    }
    else
    {
      cp2 = QPointF( ( bezierKnots_[n].x() + x[n-1] ) / 2,
                     ( bezierKnots_[n].y() + y[n-1] ) / 2 );
    }

    bezierControlPoints1_.append( cp1 );
    bezierControlPoints2_.append( cp2 );
  }
}



// Solves the system for the first control points.
void Stroke::bezierGetFirstControlPoints( double rhs[], double* xOut, int n )
{
  double* x = xOut;   // solution vector.
  double tmp[n]; // temp workspace.

  double b = 2.0;
  x[0] = rhs[0] / b;
  for( int i = 1; i < n; i++ ) // decomposition and forward substitution
  {
    tmp[i] = 1 / b;
    b = ( i < n - 1 ? 4.0 : 3.5 ) - tmp[i];
    x[i] = (rhs[i] - x[i-1]) / b;
  }

  for( int i = 1; i < n; i++ )
  {
    x[n-i-1] -= tmp[n-i] * x[n-i]; // back substitution.
  }

  // results are in xOut.
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

  // Finally, pre-calculate the stroke paths
  painterPath();

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
 * Get a painter path to draw this stroke.
 *
 * The path is adjusted according to the drawing flags.
 * Given a list of knot points, generates a QPainterPath that describes the stroke.
 *
 * If the FitToCurve flag is present, the stroke path is generated using bezier curves
 * to approximate the stroke, giving a much smoother appearance.
 *
 * @see calculateControlPoints()
 * @return QPainterPath
 */
QPainterPath Stroke::painterPath()
{
  int numPoints = points_.size();

  if( numPoints == 0 )
  {
    return QPainterPath();
  }

  QPointF startPos( points_.first().position );

  QPainterPath path( startPos );

  if( ( flags_ & FitToCurve ) == false )
  {
    foreach( Point point, points_ )
    {
      path.lineTo( point.position );
    }
    return path;
  }

  // don't calculate control points if they've
  // already been calculated.
  if( bezierKnots_.isEmpty() )
  {
    bezierCalculateControlPoints();
  }

  for( int i = 0; i < bezierControlPoints1_.size(); i++ )
  {
    // draw the bezier curve!
    path.cubicTo( bezierControlPoints1_[ i ], bezierControlPoints2_[ i ], bezierKnots_[ i + 1 ] );
  }

  return path;
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


