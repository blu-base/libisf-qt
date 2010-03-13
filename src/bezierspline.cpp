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

#include "bezierspline.h"

#include <QList>
#include <QPointF>

/**
 * Given a series of known (knot) points, knots, calculates the control points (c1, c2) to approximate a series of
 * bezier curves passing smoothly through the knot points.
 *
 * @param knots The list of knot points to approximate control points to.
 * @param c1 Output parameter - a pointer to a QList where the first control points for the curve will be placed.
 * @param c2 Output parameter - a pointer to a QList where the second control points for the curve will be placed.
 */
void BezierSpline::calculateControlPoints( const QList<QPointF> &knots, QList<QPointF> *c1, QList<QPointF> *c2 )
{
  if ( knots.size() == 0 )
  {
    return;
  }
  
  if ( ! c1 || ! c2 )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Control point QList was null.";
#endif
    return;
  }
  
  
  int n = knots.size() - 1;
  if ( n < 1 )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Require at least two knot points to generate Bezier control points; ignoring.";
#endif    
    return;
  }
 
  c1->clear();
  c2->clear();
  
  // special case: Bezier curve should be a straight line.
  if ( n == 1 )
  {
    QPointF cp1, cp2;
    QPointF k1 = knots.at(0);   // knot point 1
    QPointF k2 = knots.at(1);   // knot point 2
    
    cp1.setX( ( 2 * k1.x() + k2.x() ) / 3.0 );
    cp1.setY( ( 2 * k1.y() + k2.y() ) / 3.0 );
    
    cp2.setX( ( 2 * cp1.x() - k1.x() ) );
    cp2.setY( ( 2 * cp1.y() - k1.y() ) );
    
    c1->append(cp1);
    c2->append(cp2);
    
    return; // done!
  }
  
  // right hand side vector.
  double rhs[n];
  
  // set RHS x values
  for( int i = 1; i < n-1; i++ )
  {
    rhs[i] = 4 * knots[i].x() + 2 * knots[i+1].x();
  }
  
  rhs[0] = knots[0].x() + 2 * knots[1].x();
  rhs[n-1] = ( 8 * knots[n-1].x() + knots[n].x() ) / 2.0;
  
  // get the first ctl points x values.
  double x[n];
  firstControlPoints( rhs, x, n );
  
  
  // now set RHS y-values.
  for(int i = 1; i < n-1; i++ )
  {
    rhs[i] = 4 * knots[i].y() + 2 * knots[i+1].y();
  }
  
  rhs[0] = knots[0].y() + 2 * knots[1].y();
  rhs[n-1] = ( 8 * knots[n-1].y() + knots[n].y() ) / 2.0;
  
  double y[n];
  firstControlPoints( rhs, y, n );
  
  // now fill the output QList.
  for( int i = 0; i < n; i++ )
  {
    QPointF cp1( x[i], y[i] );
    QPointF cp2;
    
    // second ctl point
    if ( i < n-1 )
    {
      cp2 = QPointF( 2 * knots[i+1].x() - x[i+1],
                    2 * knots[i+1].y() - y[i+1] );
    }
    else
    {
      cp2 = QPointF( ( knots[n].x() + x[n-1] ) / 2,
                    ( knots[n].y() + y[n-1] ) / 2 );
    }
    
    c1->append(cp1);
    c2->append(cp2);
  }
}



// Solves the system for the first control points.
void BezierSpline::firstControlPoints( double rhs[], double *xOut, int n )
{
  double *x = xOut;   // solution vector.
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
