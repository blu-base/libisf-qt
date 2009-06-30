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

#include "isfdrawing.h"
#include "multibytecoding.h"

#include <QtDebug>

namespace Isf
{
  
IsfDrawing::IsfDrawing()
  : isNull_( true )
{
}

IsfDrawing::IsfDrawing(QByteArray &isfData)
  : isNull_(false),
    isfData_( isfData )
{
  parseIsfData(isfData);
}

bool IsfDrawing::isNull() const
{
  return isNull_;
}



/**
 * Return the ISF version number. "0" means "ISF 1.0", "1" means "ISF 1.1", etc.
 */
quint16 IsfDrawing::getIsfVersion() const
{
  return version_;
}



/**
 * Parse ISF data held in the given byte array.
 */
void IsfDrawing::parseIsfData(const QByteArray &isfData)
{
  int byteIndex = 0;
  
  if ( isfData.size() == 0 )
  {
    isNull_ = true;
    return;
  }

  // start by reading the ISF version.
  version_ = (quint32)decodeUInt(isfData, byteIndex);
}

}