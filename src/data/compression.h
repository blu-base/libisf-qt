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

#ifndef ISFCOMPRESSION_H
#define ISFCOMPRESSION_H

#include "datasource.h"



namespace Isf
{
  /**
   * Available compression and decompressions algorithms
   */
  enum Algorithm
  {
    MaskByte = 0xC0   /// Mask byte used to identify the used algorithm
  , Huffman  = 0x80   /// Adaptive Huffman compression
  , Gorilla  = 0x00   /// Gorilla compression
  };



  /// Mask byte used to identify the used algorithm
  enum Flags
  {
    BlockSizeFlag      = 0x1F   /// Find the block size of a packet
  , TransformationFlag = 0x20   /// Gorilla compressed data requires an extra transformation
  };



  /**
   * Types of storable data.
   *
   * These enum values are given to deflate() to compress different
   * kinds of data with the right algorithm.
   */
  enum DataType
  {
    Points          /// We need to deflate points data
  , Properties      /// We need to deflate packet properties
  };



  /**
   * Methods to compression and decompress ISF images
   */
  namespace Compress
  {
    /// Decompress data autodetecting the algorithm to use
    bool inflate( DataSource &source, quint64 length, QList<qint64> &decodedData );
    /// Compress data autodetecting the algorithm to use
    bool deflate( QByteArray &destination, quint64 length, const QList<qint64> &decodedData, DataType dataType );
  }
}



#endif
