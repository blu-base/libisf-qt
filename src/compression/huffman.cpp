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

#include <QDebug>



namespace Isf
{
  namespace Compress
  {
    // Compress data using the Huffman algorithm
    bool deflateHuffman( const QByteArray &source, quint32 &pos, QByteArray &decodedData )
    {
      return true;
    }



    // Decompress data using the Huffman algorithm
    bool inflateHuffman( const QByteArray &source, quint32 &pos, QByteArray &decodedData )
    {
      return true;
    }


  }



};
