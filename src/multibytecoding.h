/***************************************************************************
 *   Copyright (C) 2008 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
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

#ifndef MULTIBYTECODING_H
#define MULTIBYTECODING_H

#include "compression/isfdata.h"

#include <QByteArray>



namespace Isf
{
  namespace Compress
  {



    /**
     * Decodes a multibyte unsigned integer into a quint64.
     */
    quint64 decodeUInt( IsfData &source );



    /**
     * Decodes a multibyte signed integer into a qint64.
     */
    qint64 decodeInt( IsfData &source );



    /**
     * Decodes a float.
     */
    float decodeFloat( IsfData &source );



    /**
     * Encodes a multibyte unsigned integer into a 64-bit value.
     */
    QByteArray encodeUInt( quint64 value );



    /**
     * Encode a signed integer into a multibyte representation.
     */
    QByteArray encodeInt( qint64 value );



    /**
     * Encodes a float.
     */
    QByteArray encodeFloat( float val );



  }
}

#endif
