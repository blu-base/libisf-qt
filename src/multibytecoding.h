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

#include <QByteArray>



namespace Isf
{
  /**
   * Encodes a multibyte unsigned integer into a 64-bit value.
   */
  QByteArray encodeUInt( quint64 value );



  /**
   * Decodes a multibyte unsigned integer into a quint64.
   */
  quint64 decodeUInt( const QByteArray &bytes, int &pos );



  /**
   * Decodes a multibyte signed integer into a qint64.
   */
  qint64 decodeInt( const QByteArray &bytes, int &pos );


  
  /**
   * Encode a signed integer into a multibyte representation.
   */
  QByteArray encodeInt( qint64 value );

}

#endif
