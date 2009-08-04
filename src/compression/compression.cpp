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

#include "../libisftypes.h"
#include "compression.h"
#include "gorilla.h"
#include "huffman.h"

#include <QDebug>



namespace Isf
{
  namespace Compress
  {



    // Decompress data autodetecting the algorithm to use
    bool inflate( IsfData &source, quint64 length, QList<qint64> &decodedData )
    {
      uchar      byte           = source.getByte();
      uchar      algorithm      = ( byte & MaskByte );
      uchar      needsTransform = ( byte & TransformationFlag );
      quint8     blockSize      = ( byte & BlockSizeFlag );

      if( blockSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Block size reset";
#endif
        blockSize = 32;
      }


      switch( algorithm )
      {
        case Gorilla:
          if( needsTransform )
          {
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Required gorilla transformation!";
#endif
            return false;
          }

#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Inflating" << length << "items using the Gorilla algorithm and a block size of" << blockSize;
#endif

          return inflateGorilla( source, length, blockSize, decodedData );

        case Huffman:
#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Inflating" << length << "items using the Huffman algorithm and a block size of" << blockSize;
#endif

          return inflateHuffman( source, length, blockSize, decodedData );

        default:
#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Decoding algorithm not recognized! (byte:" << algorithm << ")";
#endif
          // Go back to the previous read position
          source.seekRelative( -1 );
          return false;
      }

      return false;
    }



    // Compress data autodetecting the algorithm to use
    bool deflate( IsfData &source, quint64 length, QList<qint64> &encodedData )
    {
      Q_UNUSED( source );
      Q_UNUSED( length );
      Q_UNUSED( encodedData );

      return true;
    }



  }
};
