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

#include "gorilla.h"

#include <QDebug>



namespace Isf
{
  namespace Compress
  {



    // Compress data using the Gorilla algorithm
    bool deflateGorilla( IsfData &source, quint8 length, QByteArray &encodedData )
    {
      Q_UNUSED( length );
      Q_UNUSED( encodedData );
      return true;
    }



    // Decompress data using the Gorilla algorithm
    bool inflateGorilla( IsfData &source, quint8 length, QByteArray &decodedData )
    {
      Q_UNUSED( source );
      Q_UNUSED( length );
      Q_UNUSED( decodedData );
/*
 * \param length number of packets to read                              *
 * \param blockSize    size of each data block                                *
 * \param arr array    where we store the decoded integers                    *
 * \param buffer       pointer to a buffer we store the current Byte read     *
 * \param offset       offset of the current bit to be read in #buffer.       *
    int err = OK;* the error code *
    INT64 i=0,
          tmp,
          signMask;

     * \b Algorithm:
     * -# Read width bits from the stream into value.
     * -# Construct a sign-mask by taking the value 0xFFFFFFFFFFFFFFFF and
     *    left-shift it by width - 1.
     * -# If value ANDed with the mask is non-zero, OR the value with the mask.
     * What this means is that if the mask matched, the sign bit is set, and by
     * ORing the value with the mask we effectively fill all the bits to the
     * left of the sign bit with 1s, turning it into a true 64-bit signed integer.
     *

    signMask = 0XFFFFFFFFFFFFFFFF << (blockSize-1);

    while (err == OK && i < length)
    {
        err = readNBits (pDecISF, blockSize, buffer, offset, &tmp);
        *(arr+i) = (tmp & signMask)?tmp|signMask:tmp;
        i++;
    }
*/
      return true;
    }


  }
};
