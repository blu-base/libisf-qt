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

#ifndef MULTIBYTECODING_H
#define MULTIBYTECODING_H

#include "datasource.h"

#include <QByteArray>



namespace Isf
{
  namespace Compress
  {

    quint64    decodeUInt( DataSource &source );
    qint64     decodeInt( DataSource &source );
    float      decodeFloat( DataSource &source );

    QByteArray encodeUInt( quint64 value );
    void       encodeUInt( DataSource &source, quint64 value, bool prepend = false );

    QByteArray encodeInt( qint64 value );
    void       encodeInt( DataSource &source, qint64 value, bool prepend = false );

    QByteArray encodeFloat( float value );
    void       encodeFloat( DataSource &source, float value, bool prepend = false );

    quint8     getMultiByteSize( quint64 value );
    quint8     getMultiByteSize( qint64 value );

  }
}

#endif
