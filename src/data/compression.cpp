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

#include "compression.h"

#include "isfqt-internal.h"
#include "algorithms/bitpacking.h"
#include "algorithms/bitpacking_byte.h"
#include "algorithms/huffman.h"
#include "algorithms/deltatransform.h"


using namespace Isf;



// Decompress packet data autodetecting the algorithm to use
bool Compress::inflatePacketData( DataSource &source, quint64 length, QList<qint64> &decodedData )
{
  if( source.atEnd() )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Source was at end!";
#endif
    return false;
  }

  bool    result;
  uchar   byte          = source.getByte();
  qint16  algorithm     = ( byte & AlgorithmMask );
  quint8  algorithmData = 0;

  // Detect which algorithm to use

  // Use the "best compression" algorithm
  if( ( algorithm & BestCompression ) == BestCompression )
  {
    /* TODO: Not implemented yet. We don't know how ISF actually works here.
      * According to the specs, both bit packing and Huffman may be used; but
      * where are the rest of the bits stored? In the next byte?
      * We need an ISF stream with such a characteristic to be sure.
      */
    qWarning() << "Unknown decompression method!";
    qDebug()   << "[Information - type: best, byte:" << QString::number( source.getByte(), 16 ) << "hex]";
    return false;
  }
  // Use the "default compression" algorithm, which is Huffman
  else if( ( algorithm & DefaultCompression ) == DefaultCompression
  ||       ( algorithm & Huffman            ) == Huffman )
  {
    algorithm = Huffman;
    algorithmData = ( HuffmanMask & byte );
  }
  // According to the specs, the only other algorithm for packet data
  // is generic bit packing.
  else if( algorithm == BitPacking )
  {
    algorithm = BitPacking;
    algorithmData = ( BitPackingMask & byte );
  }
  else
  {
    // Unknown algorithm. Throw an error below.
    algorithm = -1;
  }

  // Apply the inflation algorithm

  switch( algorithm )
  {
    case BitPacking:
    {
      // Detect if the data is encoded with delta-delta transformation
      bool requiresDeltaDeltaTransform = ( BitPackingTransformMask & byte );
      algorithmData ^= BitPackingTransformMask;

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "items using the Bit Packing algorithm and a block size of" << algorithmData;
#endif

      result = BitPackingAlgorithm::inflate( source, length, algorithmData, decodedData );

      if( result && requiresDeltaDeltaTransform )
      {
        result = Delta::inverseTransform( decodedData );
      }
      break;
    }

    case Huffman:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "items using the Huffman algorithm and index" << algorithmData;
#endif
      result = HuffmanAlgorithm::inflate( source, length, algorithmData, decodedData );

      // Always delta-inverse-transform data compressed with Huffman
      if( result )
      {
        result = Delta::inverseTransform( decodedData );
      }
      break;
    }

    default:
      qWarning() << "Unknown decompression method!";
      qDebug()   << "[Information - type: unknown, byte:" << QString::number( byte, 16 ) << "hex]";

      // Go back to the previous read position
      source.seekRelative( -1 );

      break;
  }

  // Discard any partially read bytes
  if( ! source.atEnd() )
  {
    source.skipToNextByte();
  }

  return result;
}



// Compress packet data autodetecting the algorithm to use
bool Compress::deflatePacketData( QByteArray &encodedData, const QList<qint64> &source )
{
  bool result;

  // TODO Is this really how to detect the algorithm to use?
  int algorithm;
  if( source.count() == 1 )
  {
    algorithm = BitPacking;
  }
  else
  {
    algorithm = Huffman;
  }


  switch( algorithm )
  {
    case BitPacking:
    {
      quint8 blockSize = BitPackingAlgorithm::blockSize( source );

      // Write the data encoding algorithm and block size
      encodedData.append( blockSize | BitPacking );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Deflating" << source.count() << "items using the bit packing algorithm and a block size of" << blockSize;
#endif

      // Deflate the data
      result = BitPackingAlgorithm::deflate( encodedData, blockSize, source );
      break;
    }


    case Huffman:
    {
      quint8 index = HuffmanAlgorithm::index( source );

      // Write the data encoding algorithm and index
      encodedData.append( index | Huffman );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Deflating" << source.count() << "items using the Huffman algorithm and index" << index;
#endif

      // Deflate the data
      QList<qint64> list( source );
      result = Delta::transform( list );

      if( result )
      {
        result = HuffmanAlgorithm::deflate( encodedData, index, list );
      }
      break;
    }


    default:
#ifdef ISFQT_DEBUG
      qDebug() << "Encoding algorithm not implemented! (algorithm:" << QString::number( algorithm, 16 ) << ")";
#endif
      break;
  }

  return result;
}



// Decompress property data autodetecting the algorithm to use
bool Compress::inflatePropertyData( DataSource &source, quint64 length, QList<qint64> &decodedData )
{
  if( source.atEnd() )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Source was at end!";
#endif
    return false;
  }

  bool    result;
  quint8  byte          = source.getByte();
  qint16  algorithm     = ( byte & AlgorithmMask );
  quint8  algorithmData = 0;

  // Detect which algorithm to use

  // Use the "default compression" algorithm
  if( ( algorithm & DefaultCompression ) == DefaultCompression )
  {
    /* TODO: Not implemented yet. We don't know how ISF actually works here.
      * According to the specs, any of the 3 bit packing techniques may be used;
      * but where are the rest of the bits stored? In the next byte?
      * We need an ISF stream with such a characteristic to be sure.
      */
    qWarning() << "Unknown decompression method!";
    qDebug()   << "[Information - type: default, byte:" << QString::number( source.getByte(), 16 ) << "hex]";
    return false;
  }
  // Use the "best compression" algorithm, which is Huffman
  else if( ( algorithm & BestCompression ) == BestCompression
  ||       ( algorithm & Huffman         ) == Huffman )
  {
    algorithm = Huffman;
    algorithmData = ( HuffmanMask & byte );
  }
  else if( ( algorithm & LimpelZiv ) == LimpelZiv )
  {
    algorithm = LimpelZiv;
  }
  else if( ( algorithm & BitPackingByte ) == BitPackingByte )
  {
    algorithm = BitPackingByte;
    algorithmData = ( BitPackingByteMask & byte );
  }
  else if( ( algorithm & BitPackingWord ) == BitPackingWord )
  {
    algorithm = BitPackingWord;
    algorithmData = ( BitPackingWordMask & byte );
  }
  else if( ( algorithm & BitPackingLong ) == BitPackingLong )
  {
    algorithm = BitPackingLong;
    algorithmData = ( BitPackingLongMask & byte );
  }
  else
  {
    // Unknown algorithm. Throw an error below.
    algorithm = -1;
  }

  // Apply the deflation algorithm

  switch( algorithm )
  {
    case BitPackingByte:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "bytes using the Bit Packing Byte algorithm and an index of" << algorithmData;
#endif

      result = BitPackingByteAlgorithm::inflate( source, length, algorithmData, decodedData );
      break;
    }

    case BitPackingWord:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "bytes using the Bit Packing Word algorithm and an index of" << algorithmData;
#endif

      qWarning() << "Stream data is compressed with non implemented algorithm!";
      qDebug()   << "[Information - algorithm: Bit Packing Word]";
      return false;

      break;
    }

    case BitPackingLong:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "bytes using the Bit Packing Long algorithm and an index of" << algorithmData;
#endif

      qWarning() << "Stream data is compressed with non implemented algorithm!";
      qDebug()   << "[Information - algorithm: Bit Packing Long]";
      return false;

      break;
    }

    case LimpelZiv:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "items using the Limpel-Ziv algorithm and a block size of" << algorithmData;
#endif

      qWarning() << "Stream data is compressed with non implemented algorithm!";
      qDebug()   << "[Information - algorithm: Limpel-Ziv]";
      return false;

      break;
    }

    case Huffman:
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Inflating" << length << "items using the Huffman algorithm and index" << algorithmData;
#endif
      result = HuffmanAlgorithm::inflate( source, length, algorithmData, decodedData );

      // Always delta-inverse-transform data compressed with Huffman
      if( result )
      {
        result = Delta::inverseTransform( decodedData );
      }

      break;
    }

    default:
    qWarning() << "Unknown decompression method!";
    qDebug()   << "[Information - type: unknown, byte:" << QString::number( byte, 16 ) << "hex]";

      // Go back to the previous read position
      source.seekRelative( -1 );

      break;
  }

  // Discard any partially read bytes
  if( ! source.atEnd() )
  {
    source.skipToNextByte();
  }

  return result;
}



// Compress property data autodetecting the algorithm to use
bool Compress::deflatePropertyData( QByteArray &encodedData, const QList<qint64> &source )
{
  bool result = false;

  // TODO Everything here.

  qWarning() << "Property data saving is not implemented at the moment!";

  return result;
}


