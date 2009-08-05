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
#include "multibytecoding.h"

#include <QDebug>
#include <math.h>



namespace Isf
{
  namespace Compress
  {



    // Decodes a multibyte unsigned integer into a quint64.
    quint64 decodeUInt( IsfData &source )
    {
      quint8 byte;       // Current byte
      quint8 flag;       // Used to find out if there are more bytes to convert
      quint8 count = 0;  // Count of bits read so far
      quint64 value = 0; // Destination 64-bit value

      do
      {
        byte   = source.getByte();
        flag   = byte & 0x80;
        value |= (byte & 0x7F) << count;
        count += 7;
      }
      while( flag );

      return value;
    }



    // Decodes a multibyte signed integer into a qint64.
    qint64 decodeInt( IsfData &source )
    {
      qint64 value = decodeUInt( source );

      if( value & 1 )
      {
        value *= -1;
      }

      value >>= 1;

      return value;
    }



    // Decodes a float.
    float decodeFloat( IsfData &source )
    {
      qint8 index;

      // Union used to decode the float transparently
      union
      {
        float value;
        uchar bytes[4];
      } data;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
      index = 3;
      do
      {
        data.bytes[ index-- ] = source.getByte();
      }
      while( index >= 0 );
#else
      index = 0;
      do
      {
        data.bytes[ index++ ] = source.getByte();
      }
      while( index <= 3 );
#endif

      return data.value;
    }



    // Encodes a multibyte unsigned integer into a 64-bit value.
    QByteArray encodeUInt( quint64 value )
    {
      quint8 byte;
      quint8 flag;
      QByteArray output;

      do
      {
        byte = value & 0x7F;
        value >>= 7;

        flag = ( value == 0 ) ? 0 : 0x80;

        output.append( byte | flag );
      }
      while( flag );

      return output;
    }



    /**
     * Encodes a signed 64-bit integer into a QByteArray with
     * multibyte encoding.
     * @param val The value to encode.
     * @return a QByteArray instance where each index corresponds to a byte in the encoding.
     */
    QByteArray encodeInt( qint64 val )
    {
      // encoding process:
      // get absolute value.
      // shift left by 1.
      // encode via multibyte encoding.
      // set sign bit.
      bool isNegative = ( val < 0 );
      if ( isNegative ) val *= -1;

      val = val << 1;

      QByteArray result = encodeUInt( val );

      if ( isNegative ) result[0] = result[0] | 0x1;

      return result;
    }



    /**
     * Encodes a floating-point value into a QByteArray with
     * multibyte encoding.
     * @param val The value to encode.
     * @return a QByteArray instance where each index corresponds to a byte in the encoding.
     */
    QByteArray encodeFloat( float val )
    {
      return QByteArray();
    }



  }
}


