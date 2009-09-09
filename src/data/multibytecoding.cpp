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

#include "multibytecoding.h"

#include "isfqt-internal.h"

#include <math.h>


using namespace Isf;



/**
 * Decodes an unsigned integer into a quint64 using multibyte decoding.
 *
 * @param source Data Source where to read bytes from
 * @return quint64
 */
quint64 Compress::decodeUInt( DataSource &source )
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



/**
 * Decodes a signed integer into a qint64 using multibyte decoding.
 *
 * @param source Data Source where to read bytes from
 * @return qint64
 */
qint64 Compress::decodeInt( DataSource &source )
{
  bool isNegative = false;
  qint64 value = decodeUInt( source );

  // The least significant bit contains the sign
  if( value & 0x01 )
  {
    isNegative = true;
  }

  // Remove the sign bit from the raw value
  value >>= 1;

  // Apply the sign
  if( isNegative )
  {
    value *= -1;
  }

  return value;
}



/**
 * Decodes a float using multibyte decoding.
 *
 * @param source Data Source where to read bytes from
 * @return float
 */
float Compress::decodeFloat( DataSource &source )
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



/**
  * Encodes an unsigned 64-bit integer into a byte array using multibyte encoding.
  *
  * @param value The value to encode
  * @return byte array
  */
QByteArray Compress::encodeUInt( quint64 value )
{
  quint8 byte;
  quint8 flag;
  QByteArray result;

  do
  {
    byte = value & 0x7F;
    value >>= 7;

    flag = ( value == 0 ) ? 0 : 0x80;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    result.prepend( byte | flag );
#else
    result.append( byte | flag );
#endif
  }
  while( flag );

  return result;
}



/**
  * Encodes an unsigned 64-bit integer into the Data Source using multibyte encoding.
  *
  * @param source   The data source
  * @param value    The value to encode
  * @param prepend  False (default value) to append the encoded value to the existing data
  *                 True to prepend the new value to the existing data
  */
void Compress::encodeUInt( DataSource &source, quint64 value, bool prepend )
{
  QByteArray result( encodeUInt( value ) );

  if( prepend )
  {
    source.prepend( result );
  }
  else
  {
    source.append( result );
  }
}



/**
  * Encodes a signed 64-bit integer into a byte array using multibyte encoding.
  *
  * @param value The value to encode
  * @return byte array
  */
QByteArray Compress::encodeInt( qint64 value )
{
  bool isNegative = ( value < 0 );
  if ( isNegative ) value *= -1;

  value = value << 1;

  QByteArray result( encodeUInt( value ) );

  if ( isNegative )
  {
    int pos;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    pos = result.size() - 1;
#else
    pos = 0;
#endif

    result[ pos ] = result[ pos ] | 0x1;
  }

  return result;
}



/**
  * Encodes a signed 64-bit integer into the Data Source using multibyte encoding.
  *
  * @param source   The data source
  * @param value    The value to encode
  * @param prepend  False (default value) to append the encoded value to the existing data
  *                 True to prepend the new value to the existing data
  */
void Compress::encodeInt( DataSource &source, qint64 value, bool prepend )
{
  QByteArray result( encodeInt( value ) );

  if( prepend )
  {
    source.prepend( result );
  }
  else
  {
    source.append( result );
  }
}



/**
  * Encodes a floating-point value into a byte array using multibyte encoding.
  *
  * @param value The value to encode
  * @return byte array
  */
QByteArray Compress::encodeFloat( float value )
{
  qint8 index;
  QByteArray result;

  // Union used to encode the float transparently
  union
  {
    float number;
    uchar bytes[4];
  } data;

  data.number = value;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  index = 3;
  do
  {
    result.append( data.bytes[ index-- ] );
  }
  while( index >= 0 );
#else
  index = 0;
  do
  {
    result.append( data.bytes[ index++ ] );
  }
  while( index <= 3 );
#endif

  return result;
}



/**
  * Encodes a floating-point value into the Data Source using multibyte encoding.
  *
  * @param source   The data source
  * @param value    The value to encode
  * @param prepend  False (default value) to append the encoded value to the existing data
  *                 True to prepend the new value to the existing data
  */
void Compress::encodeFloat( DataSource &source, float value, bool prepend )
{
  QByteArray result( encodeFloat( value ) );

  if( prepend )
  {
    source.prepend( result );
  }
  else
  {
    source.append( result );
  }
}


    /**
     * Return how much bytes a certain unsigned value takes up using multi-byte encoding.
     *
     * @param  value The unsigned value to check
     * @return       Number of bytes required for multi-byte storage of that value
     */
quint8 Compress::getMultiByteSize( quint64 value )
{
  quint8 bytes = 1;

  while( value )
  {
    value >>= 7;

    if( value )
    {
      ++bytes;
    }
  }

  return bytes;
}




    /**
     * Return how much bytes a certain signed value takes up using multi-byte encoding.
     *
     * @param  value The signed value to check
     * @return       Number of bytes required for multi-byte storage of that value
     */
quint8 Compress::getMultiByteSize( qint64 value )
{
  quint8 bytes = 1;

  while( value )
  {
    value >>= 6;

    if( value )
    {
      ++bytes;
    }
  }

  return bytes;
}


