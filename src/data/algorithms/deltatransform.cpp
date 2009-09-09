/***************************************************************************
 *   Copyright (C) 2008 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
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

#include "deltatransform.h"


using namespace Isf::Compress;



/**
 * Perform a delta-delta transformation on data to be deflated.
 *
 * @param data Data to transform - the list will be modified.
 * @return bool
 */
bool Delta::transform( QList<qint64> &data )
{
  qint64 currentDelta  = 0;
  qint64 previousDelta = 0;

  for( qint64 index = 0; index < data.size(); ++index )
  {
    qint64 delta = data[ index ] - previousDelta;

    previousDelta = currentDelta;
    currentDelta = previousDelta;

    data[ index ] = delta;
  }

  return true;
}



/**
 * Perform a delta-delta inverse transformation on just-inflated data.
 *
 * @param data Data to transform - the list will be modified.
 * @return bool
 */
bool Delta::inverseTransform( QList<qint64> &data )
{
  qint64 currentDelta  = 0;
  qint64 previousDelta = 0;

  for( qint64 index = 0; index < data.size(); ++index )
  {
    qint64 delta = ( currentDelta * 2 ) - previousDelta + data[ index ];

    previousDelta = currentDelta;
    currentDelta = delta;

    data[ index ] = delta;
  }

  return true;
}


