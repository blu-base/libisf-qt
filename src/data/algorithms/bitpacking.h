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

#ifndef ISFCOMPRESSION_BITPACKING_H
#define ISFCOMPRESSION_BITPACKING_H

#include "../datasource.h"



namespace Isf
{
  namespace Compress
  {
    namespace BitPackingAlgorithm
    {

      /// Get the most appropriate block size for the given data
      quint8 blockSize( const QList<qint64> &data );
      /// Compress data
      bool deflate( QByteArray &encodedData, quint8 blockSize, const QList<qint64> &source );
      /// Decompress data
      bool inflate( DataSource &source, quint64 length, quint8 blockSize, QList<qint64> &decodedData );

    }
  }
}



#endif
