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

#ifndef ISFQTSTROKE_H
#define ISFQTSTROKE_H

#include "isfqt-internal.h"

// Forward declarations
class Flag;
class Flags;


namespace Isf
{



  /**
  * A pen stroke.
  */
  class Stroke
  {
    public:
      Stroke();
      Stroke( const Stroke& );

      void          addPoint( Point );
      void          addPoints( QList<Point> );
      BezierData*   bezierInfo();
      QRect         boundingRect() const;
      QColor        color() const;
      void          finalize();
      QColor        flags() const;
      Metrics       metrics();
      QSize         penSize() const;
      QList<Point>& points();
      void          setColor( QColor );
      void          setFlag( Flag, bool = true );
      void          setFlags( Flags );
      void          setMetrics( Metrics* );
      void          setPenSize( QSize );
      void          setTransform( QMatrix* );
      QMatrix*      transform();

    private:
      /// Attributes of this stroke's points
      AttributeSet   attributes_;
      /// Bezier data
      BezierData*    bezierInfo_;
      /// Bounding rectangle of this stroke
      QRect          boundingRect_;
      /// Whether the stroke data needs to be analyzed or not
      bool           finalized_;
      /// Link to this stroke's attributes, if any
      StrokeInfo*    info_;
      /// Link to this stroke's metrics, if any
      Metrics*       metrics_;
      /// List of points
      QList<Point>   points_;
      /// Link to this stroke's transformation, if any
      QMatrix*       transform_;

  };



}; // namespace Isf



#endif // ISFQTSTROKE_H
