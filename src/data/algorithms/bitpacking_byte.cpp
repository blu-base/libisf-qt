/***************************************************************************
 *   Copyright (C) 2008-2009 by Valerio Pilo                               *
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

#include "isfqt-internal.h"
#include "bitpacking_byte.h"
#include "bitpacking_table.h"


using namespace Isf::Compress;



/**
 * Get the most appropriate index for the given data.
 *
 * @param data Data to analyze
 * @return Index value
 */
quint8 BitPackingByteAlgorithm::index( const QList<qint64> &data )
{
  Q_UNUSED( data );

  quint8 index = 0;
/*
  qint64 num;

  for( qint32 index = 0; index < data.size(); ++index )
  {
    num = data[ index ];

    if( num < 0 )
    {
      // We need the positive numbers: the +1 is for the sign bit
      num += - (num + 1);
    }

    // Shift right the value by index bits until it becomes zero:
    // we need to detect the maximum bitwise size required to store
    // the numbers in the data array.
    num >>= index;
    while( num )
    {
      ++index;
      num >>= 1;
    }
  }

  // The sign bit always takes up a bit
  ++index;
*/
  return index;
}



/**
 * Compress data with the Bit Packing algorithm.
 *
 * @param encodedData Byte array where to store the compressed data
 * @param index Index to use for compression
 * @param source List of values to compress
 * @return bool
 */
bool BitPackingByteAlgorithm::deflate( QByteArray &encodedData, quint8 index, const QList<qint64> &source )
{
  Q_UNUSED( encodedData );
  Q_UNUSED( index );
  Q_UNUSED( source );

/*
  if( blockSize > 64 )
  {
    qWarning() << "A block size of" << blockSize << "is too high!";
    blockSize = 64; // Fuck it :P
  }

  quint8 blockSizeLeft; // Remaining bits to encode of the current value
  quint8 freeBits = 8;  // Free bits of the current byte, in LSB order
  quint8 signMask = 1 << ( blockSize - 1 );  // Mask to add the sign bit

  quint8 byte = 0;          // Byte to add to the array
  qint64 currentValue;

  for( qint64 index = 0; index < source.count(); ++index )
  {
    currentValue = source[ index ];

    if( currentValue < 0 )
    {
      currentValue |= signMask;
    }

    blockSizeLeft = blockSize;

    if( freeBits >= blockSizeLeft )
    {
      freeBits -= blockSizeLeft;
      byte |= currentValue << freeBits;

      encodedData.append( byte );

      byte = 0;
      freeBits = 8;
    }
    else
    {
      // Fill the current byte
      quint64 mask = 0xFFFFFFFF >> ( 32 - blockSizeLeft );
      byte |= currentValue >> ( blockSizeLeft - freeBits );

      encodedData.append( byte );

      byte = 0;
      blockSizeLeft -= freeBits;
      mask >>= freeBits;
      currentValue &= mask;

      // Then fill all the next bytes required to encode the current value,
      // except the last one
      while( blockSizeLeft > 8 )
      {
        blockSizeLeft -= 8;
        encodedData.append( currentValue >> blockSizeLeft );

        mask >>= 8;
        currentValue &= mask;
      }

      // Finally, add the last byte
      freeBits = 8 - blockSizeLeft;
      encodedData.append( currentValue << freeBits );
    }
  }
*/
  return true;
}



/**
 * Decompress data with the Bit Packing Byte algorithm.
 *
 * @param source Data source where to read the compressed bytes
 * @param length Number of items to read
 * @param index Index to use for compression
 * @param decodedData List where to place decompressed values
 * @return bool
 */
bool BitPackingByteAlgorithm::inflate( DataSource &source, quint64 length, quint8 index, QList<qint64> &decodedData )
{
  if( index > 24 )
  {
    qWarning() << "An index of" << index << "is too high!";
    index = 24; // Fuck it :P
  }

  if( source.atEnd() )
  {
    qWarning() << "Cannot inflate: no more bits available!";
    return true;
  }

  quint8 cBits = BitPackingAlgorithm::bitLookUpTable_[ index ][ CBITS ];
//   quint8 cPads = BitPackingAlgorithm::bitLookUpTable_[ index ][ CPADS ];
  quint16 count = ( length * 8 / cBits ); // Number of items in the compressed array

  bool ok;
  quint64 pos = 0;
  while( pos++ < count )
  {
    quint8 byte = source.getBits( cBits, &ok );

    if( ! ok )
    {
      qWarning() << "Decompression failure!";
      return false;
    }

    decodedData.append( byte );
  }

  return true;
}


