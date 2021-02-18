/***************************************************************************
 *   Copyright (C) 2010 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2010 by Adam Goossens                                   *
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

#ifndef ISFQTSTROKE_H
#define ISFQTSTROKE_H

#include <IsfQt>

#include <QColor>
#include <QList>
#include <QMatrix>
#include <QRect>
#include <QSizeF>


// Forward declarations
class QPainterPath;



namespace Isf
{



  // Forward declarations
  class AttributeSet;



  /**
  * A pen stroke.
  */
  class Stroke
  {
    public:
      Stroke();
      Stroke( const Stroke& );
     ~Stroke();

      void          addPoint( Point );
      void          addPoints( PointList );
      QRect         boundingRect() const;
      QColor        color() const;
      void          finalize();
      StrokeFlags   flags() const;
      bool          hasPressureData() const;
      Metrics*      metrics();
      QPainterPath  painterPath();
      QSizeF        penSize() const;
      PointList&    points();
      void          setColor( QColor );
      void          setFlag( StrokeFlag, bool = true );
      void          setFlags( StrokeFlags );
      void          setMetrics( Metrics* );
      void          setPenSize( QSizeF );
      void          setTransform( QMatrix* );
      QMatrix*      transform();

    private:
      void          bezierCalculateControlPoints();
      void          bezierGetFirstControlPoints( double[], double*, int );

    private:
      /// Bezier data
      QList<QPointF> bezierControlPoints1_;
      QList<QPointF> bezierControlPoints2_;
      QList<QPointF> bezierKnots_;
      /// Bounding rectangle of this stroke
      QRect          boundingRect_;
      /// The stroke color, optionally with alpha channel
      QColor         color_;
      /// Whether the stroke data needs to be analyzed or not
      bool           finalized_;
      /// Mask of StrokeFlags, @see StrokeFlags
      StrokeFlags    flags_;
      /// Whether the stroke contains pressure information or not
      bool           hasPressureData_;
      /// Link to this stroke's metrics, if any
      Metrics*       metrics_;
      /// Dimensions of the pencil in pixels
      QSizeF         penSize_;
      /// List of points
      PointList      points_;
      /// Link to this stroke's transformation, if any
      QMatrix*       transform_;


  };



} // namespace Isf



#endif // ISFQTSTROKE_H
