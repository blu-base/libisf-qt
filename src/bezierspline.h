/***************************************************************************
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

/*
 * This algorithm is a C++ translation of the C# algorithm by Oleg V. Polikarpotchkin at 
 * (http://www.codeproject.com/KB/graphics/BezierSpline.aspx). Copyright Oleg V. Polikarpotchkin 2008.
 *
 * C++ translation Copyright (C) 2010 Adam Goossens (adam@kmess.org)
 *
 */

#ifndef BEZIERSPLINE_H
#define BEZIERSPLINE_H

#include <QList>
#include <QPointF>

/**
 * Given a series of QPointF instances that describe a curve, this class
 * generates the appropriate bezier control points that can be used to draw a bezier
 * curve that smoothly approximates the parent curve.
 *
 * This class cannot be instantiated.
 */
class BezierSpline
{
  private:
    BezierSpline();

  public:
    //! Calculate the Bezier control points for a curve passing through the points in knots.
    static void calculateControlPoints( const QList<QPointF> &knots, QList<QPointF> *c1, QList<QPointF> *c2 );
    
  private:
    //! Solves the system for the first control points.
    static void firstControlPoints( double rhs[], double *xOut, int n );
};

#endif
