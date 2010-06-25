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

#include "../datasource.h"



namespace Isf
{
  namespace Compress
  {
    namespace HuffmanAlgorithm
    {

      bool   deflate( QByteArray& encodedData, quint8 index, const QList<qint64>& source );
      bool   inflate( DataSource* source, quint64 length, quint8 index, QList<qint64>& decodedData );
      quint8 index( const QList<qint64>& data );

      // Internal use methods

      bool   deflateValue( DataSource* output, quint8 index, qint64 value );

    }
  }
}



#endif
