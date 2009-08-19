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
#include "gorilla.h"
#include "huffman.h"



namespace Isf
{
  namespace Compress
  {



    // Decompress data autodetecting the algorithm to use
    bool inflate( DataSource &source, quint64 length, QList<qint64> &decodedData )
    {
      if( source.atEnd() )
      {
#ifdef ISFQT_DEBUG
        qWarning() << "Source was at end!";
#endif
        return false;
      }

      bool       result;
      uchar      byte           = source.getByte();
      uchar      algorithm      = ( byte & MaskByte );
      uchar      needsTransform = ( byte & TransformationFlag );
      quint8     blockSize      = ( byte & BlockSizeFlag );

      if( blockSize == 0 )
      {
#ifdef ISFQT_DEBUG
        qWarning() << "Block size invalid, was reset to 32";
#endif
        blockSize = 32;
      }


      switch( algorithm )
      {
        case Gorilla:
          if( needsTransform )
          {
#ifdef ISFQT_DEBUG
            qWarning() << "Gorilla transformation is required, aborting!";
#endif
            return false;
          }

#ifdef ISFQT_DEBUG_VERBOSE
          qDebug() << "- Inflating" << length << "items using the Gorilla algorithm and a block size of" << blockSize;
#endif

          result = inflateGorilla( source, length, blockSize, decodedData );
          break;

        case Huffman:
#ifdef ISFQT_DEBUG_VERBOSE
          qDebug() << "- Inflating" << length << "items using the Huffman algorithm and index" << blockSize;
#endif
          result = inflateHuffman( source, length, blockSize, decodedData );
          break;

        default:
#ifdef ISFQT_DEBUG
          qDebug() << "Decoding algorithm not recognized! (byte:" << QString::number( algorithm, 2 ) << ")";
#endif
          // Go back to the previous read position
          source.seekRelative( -1 );
          return false;
      }

      // Discard any partially read bytes
      if( ! source.atEnd() )
      {
        source.skipToNextByte();
      }

      return result;
    }



    // Compress data autodetecting the algorithm to use
    bool deflate( QByteArray &encodedData, const QList<qint64> &source, DataType dataType )
    {
      bool result;

      // TODO Is this really how to detect the algorithm to use?
      Algorithm algorithm;
      if( source.count() == 1 )
      {
        algorithm = Gorilla;
      }
      else
      {
        algorithm = Huffman;
      }


      switch( algorithm )
      {
        case Gorilla:
        {
          quint8 blockSize = getBlockSizeGorilla( source );

          // Write the data encoding algorithm and block size
          encodedData.append( blockSize | Gorilla );

#ifdef ISFQT_DEBUG_VERBOSE
          qDebug() << "- Deflating" << source.count() << "items using the Gorilla algorithm and a block size of" << blockSize;
#endif

          // Deflate the data
          result = deflateGorilla( encodedData, blockSize, source );
          break;
        }


        case Huffman:
        {
          quint8 index = getIndexHuffman( source );

          // Write the data encoding algorithm and index
          encodedData.append( index | Huffman );

#ifdef ISFQT_DEBUG_VERBOSE
          qDebug() << "- Deflating" << source.count() << "items using the Huffman algorithm and index" << index;
#endif

          // Deflate the data
          result = deflateHuffman( encodedData, index, source );
          break;
        }


        default:
#ifdef ISFQT_DEBUG
          qDebug() << "Encoding algorithm not implemented! (data type:" << dataType
                   << ", algorithm:" << QString::number( algorithm, 16 ) << ")";
#endif
          break;
      }

      return result;
    }



  }
};
