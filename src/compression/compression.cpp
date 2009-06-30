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
#include "gorilla.h"
#include "huffman.h"

#include <QDebug>

using namespace Isf;

namespace Isf
{
  namespace Compress
  {
    // Decompress data autodetecting the algorithm to use
    bool deflate( const QByteArray &source, quint32 &pos, QByteArray &decodedData )
    {
      uchar byte = source[ pos++ ];

      switch( byte )
      {
        case Gorilla:
          return deflateGorilla( source, pos, decodedData );

        case Huffman:
          return deflateHuffman( source, pos, decodedData );

        default:
#ifdef LIBISF_DEBUG
          qDebug() << "Encoding algorithm not recognized! (byte:" << byte << ")";
#endif
          // Go back to the previous read position
          --pos;
          return false;
      }

      return true;
    }



    // Compress data autodetecting the algorithm to use
    bool inflate( const QByteArray &source, quint64 &pos, QByteArray &encodedData )
    {
      return true;
    }



  }



};