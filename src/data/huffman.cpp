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

#include "huffman.h"

#include "isfqt-internal.h"

#include <math.h>


#define HUFFMAN_BASES_NUM   8
#define HUFFMAN_BASE_SIZE  11



namespace Isf
{
  namespace Compress
  {
    const int bitAmounts_[HUFFMAN_BASES_NUM][HUFFMAN_BASE_SIZE] =
    {
        {0, 1, 2,  4,  6,  8, 12, 16, 24, 32, -1},
        {0, 1, 1,  2,  4,  8, 12, 16, 24, 32, -1},
        {0, 1, 1,  1,  2,  4,  8, 14, 22, 32, -1},
        {0, 2, 2,  3,  5,  8, 12, 16, 24, 32, -1},
        {0, 3, 4,  5,  8, 12, 16, 24, 32, -1, -1},
        {0, 4, 6,  8, 12, 16, 24, 32, -1, -1, -1},
        {0, 6, 8, 12, 16, 24, 32, -1, -1, -1, -1},
        {0, 7, 8, 12, 16, 24, 32, -1, -1, -1, -1},
    };

    const int huffmanBases_[HUFFMAN_BASES_NUM][HUFFMAN_BASE_SIZE] =
    {
        {0, 1,  2,   4,   12,    44,     172,    2220,   34988, 8423596, -1},
        {0, 1,  2,   3,    5,    13,     141,    2189,   34957, 8423565, -1},
        {0, 1,  2,   3,    4,     6,      14,     142,    8334, 2105486, -1},
        {0, 1,  3,   5,    9,    25,     153,    2201,   34969, 8423577, -1},
        {0, 1,  5,  13,   29,   157,    2205,   34973, 8423581,      -1, -1},
        {0, 1,  9,  41,  169,  2217,   34985, 8423593,      -1,      -1, -1},
        {0, 1, 33, 161, 2209, 34977, 8423585,      -1,      -1,      -1, -1},
        {0, 1, 65, 193, 2241, 35009, 8423617,      -1,      -1,      -1, -1},
    };



    // Analyze the data to find the most appropriate index
    quint8 getIndexHuffman( const QList<qint64> &data )
    {
      quint8 index = 0;

      // TODO: Find out what the Huffman index algorithm is
      Q_UNUSED( data )
      index = 2;

      return index;
    }



    // Compress data using the Adaptive-Huffman algorithm
    bool deflateHuffman( QByteArray &encodedData, quint8 index, const QList<qint64> &source )
    {
      DataSource output( encodedData );
      output.skipToNextByte();

      int previousDelta = 0, currentDelta = 0;
      for( int pos = 0; pos < source.size(); ++pos )
      {
        if( pos == 0 )
        {
          currentDelta = source[ pos ];
        }
        else
        {
          currentDelta = source[ pos ] - source[ pos - 1 ];
        }

        if( ! deflateHuffmanValue( output, index, currentDelta - previousDelta ) )
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Deflating failure for value:" << source[ pos ] << "- delta" << ( currentDelta - previousDelta );
#endif
          return false;
        }

        previousDelta = currentDelta;
      }

      // Flush any leftover bit to the byte array
      output.flush();

      encodedData = output.data();
      return true;
    }



    // Compress a single value using the Adaptive-Huffman algorithm
    bool deflateHuffmanValue( DataSource &output, quint8 index, qint64 value )
    {
      qint64 temp = value;
      quint8 requiredBits = 0;
      QBitArray valueBits;

      // Fill in a bitAmounts local vector
      QVector<int> bitAmounts;
      for( int i = 0; i < HUFFMAN_BASE_SIZE; ++i )
      {
        if( bitAmounts_[ index ][ i ] == -1 )
        {
          break;
        }

        bitAmounts.append( bitAmounts_[ index ][ i ] );
      }

      // Find the number of bits needed to store this value
      while( temp )
      {
        temp /= 2;
        ++requiredBits;
      }

      while( ! bitAmounts.count( requiredBits ) )
      {
        ++requiredBits;
      }

      qint64 offset = ( value < 0 ) ? -value : +value;

      quint8 prefixLength;
      for( prefixLength = 1;
           ( prefixLength < HUFFMAN_BASE_SIZE ) && ( offset >= huffmanBases_[ index ][ prefixLength ] ); prefixLength++ );


//       quint8 lengthIndex = bitAmounts.indexOf( requiredBits );

      // Write prefixLength 1s to the stream, then a 0
      QBitArray bits( prefixLength );
      bits.fill( true );
      bits.clearBit( bits.size() - 1 );

      // Write the number using the minimum possible number of bits
      qint32 size = bitAmounts_[ index ][ prefixLength - 1 ];
      qint64 mask = ( 1 << ( size - 1 ) ) - 1;

      valueBits.resize( size );
      offset = ( offset - huffmanBases_[ index ][ prefixLength - 1 ] ) & mask;
      offset <<= 1;
      offset |= ( value < 0 ) ? 1 : 0;

      // Copy the resulting offset
      for( qint64 i = 0; i < size; ++i )
      {
        valueBits.setBit( i, offset & ( 1 << ( size - i - 1 ) ) );
      }

      // Add the bits to the data source
      output.append( bits );
      output.append( valueBits );

      return true;
    }



    // Decompress data using the Huffman algorithm
    bool inflateHuffman( DataSource &source, quint64 length, quint8 index, QList<qint64> &decodedData )
    {
      QVector<int> huffmanBases;
      QVector<int> bitAmounts( HUFFMAN_BASE_SIZE );

      // Initialize the bit amounts vector
      memcpy( bitAmounts.data(), bitAmounts_[ index ], sizeof(int)*HUFFMAN_BASE_SIZE );

      int base = 1;
      huffmanBases.append( 0 );

      // Fill up the huffman bases vector
      for( quint8 i = 0; i < bitAmounts.size(); ++i )
      {
        int value = bitAmounts[ i ];

        // The bit amounts sequence ends in -1
        if( value == -1 )
        {
          bitAmounts.resize( i );
          break;
        }

        if( value == 0 )
        {
          continue;
        }

        huffmanBases.append( base );
        base += pow( 2, value - 1 );
      }


      quint32 count = 0;
      qint64 value = 0;
      bool bit;

      while( (uint)decodedData.length() < length )
      {
        bit = source.getBit();

        if( bit )
        {
          count++;
          continue;
        }

        if( count == 0 )
        {
          value = 0;
        }
        else if( count < (uint)bitAmounts.size() )
        {
          quint64 offset = source.getBits( bitAmounts[ count ] );
          bool sign = offset & 0x1;
          offset /= 2;
          value = huffmanBases[ count ] + offset;
          value *= ( sign ? -1 : +1 );
        }
        else if( count == (uint)bitAmounts.size() )
        {
          // TODO: Implement 64-bit data decompression :)
#ifdef ISFQT_DEBUG
          qDebug() << "Unsupported 64-bit value found!";
#endif
          value = 0;
        }
        else
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Decompression error!";
#endif
          value = 0;
        }

        decodedData.append( value );
        count = 0;
      }

      // Perform the delta-delta inverse transformation on the values
      int previousDelta = 0, currentDelta = 0;
      for( int i = 0; i < decodedData.size(); ++i )
      {
        int delta = ( currentDelta * 2 ) - previousDelta + decodedData[ i ];
        previousDelta = currentDelta;
        currentDelta = delta;

        decodedData[ i ] = delta;
      }

      return true;
    }


  }



};
