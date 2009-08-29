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
  namespace Compress
  {
    /**
     * Algorithm extra data masks.
     *
     * These masks are used to obtain the least significant bits which
     * are useful for the specific algorithm. If an algo takes no
     * useful bits, the mask is 0.
     */
    enum AlgorithmMasks
    {
      AlgorithmMask            = 0xF0   /// Mask used to identify the algorithms
    , BitPackingMask           = 0x3F   /// Mask for bit packing of arbitrary-sized integers
    , BitPackingTransformMask  = 0x20   /// Mask for delta-delta transformed bit packed data
    , HuffmanMask              = 0x3F   /// Mask for Huffman
    , BitPackingLongMask       = 0x3F   /// Mask for bit packing of 32bit signed integers
    , BitPackingWordMask       = 0x1F   /// Mask for bit packing of 16bit unsigned integers
    , BitPackingByteMask       = 0x1F   /// Mask for bit packing of 8bit unsigned integers
    , LimpelZivMask            = 0x00   /// Mask for Limpel-Ziv
    , BestCompressionMask      = 0x00   /// Mask for the best compression algo
    , DefaultCompressionMask   = 0x00   /// Mask for the default compression algo
    };

    /**
     * Compression and decompressions algorithms.
     *
     * Bit assignment table for packets (strokes):
     * ---------------------------
     * | Algorithm             |  7 6 5 4 3 2 1 0 (Bit assignment) |
     *   Bit packing              0 0 D D D D D D
     *   Huffman                  1 0 D D D D D D
     *   Default compression      1 1 0 0 X X X X
     *   Best compression         1 1 1 1 X X X X
     *
     * Bit assignment table for properties (tags):
     * ---------------------------
     * | Algorithm             |  7 6 5 4 3 2 1 0 (Bit assignment) |
     *   Bit packing of bytes     0 0 0 D D D D D
     *   Bit packing of words     0 0 1 D D D D D
     *   Bit packing of longs     0 1 D D D D D D
     *   Limpel-Ziv               1 0 1 0 X X X X
     *   Default compression      1 1 0 0 X X X X
     *   Best compression         1 1 1 1 X X X X
     *
     * D = Algorithm-specific data
     * X = Ignored bit
     *
     * We use the two most significant bits to identify the algorithms,
     * because they are guaranteed to not contain algorithm data.
     *
     * Also, note that these tables are not identical to the ones in the
     * specs: we changed them because they differ from our factual
     * observations (in the specs Huffman is missing, for example).
     */
    enum CommonDataAlgorithms
    {
      BestCompression          = 0xF0   /// Best compression algorithm
    , DefaultCompression       = 0xC0   /// Default compression algorithm (Huffman)
    , Huffman                  = 0x80   /// Huffman encoding
    };
    enum PacketDataAlgorithms
    {
      BitPacking               = 0x00   /// Bit packing
    };
    enum PropertyDataAlgorithms
    {
      LimpelZiv                = 0xA0   /// Limpel-Ziv compressionrs
    , BitPackingLong           = 0x40   /// Bit packing of 32bit signed integers
    , BitPackingWord           = 0x20   /// Bit packing of 16bit unsigned integers
    , BitPackingByte           = 0x00   /// Bit packing of 8bit unsigned integers
    };

    /// Decompress packet data autodetecting the algorithm to use
    bool inflatePacketData( DataSource &source, quint64 length, QList<qint64> &decodedData );
    /// Compress packet data autodetecting the algorithm to use
    bool deflatePacketData( QByteArray &destination, const QList<qint64> &decodedData );

    /// Decompress property data autodetecting the algorithm to use
    bool inflatePropertyData( DataSource &source, quint64 length, QList<qint64> &decodedData );
    /// Compress property data autodetecting the algorithm to use
    bool deflatePropertyData( QByteArray &destination, const QList<qint64> &decodedData );

  }
}



#endif
