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

#ifndef ISFCOMPRESSION_HUFFMAN_H
#define ISFCOMPRESSION_HUFFMAN_H

#include "isfdata.h"



namespace Isf
{
  namespace Compress
  {
    /// Compress data using the Huffman algorithm
    bool deflateHuffman( IsfData &source, quint8 length, QByteArray &encodedData );
    /// Decompress data using the Huffman algorithm
    bool inflateHuffman( IsfData &source, quint8 length, QByteArray &decodedData );
  }
}



#endif
