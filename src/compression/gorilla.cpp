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
    bool deflateGorilla( IsfData &source, quint32 length, quint8 blockSize, QByteArray &encodedData )
    {
      Q_UNUSED( length );
      Q_UNUSED( encodedData );
      return true;
    }



    // Decompress data using the Gorilla algorithm
    bool inflateGorilla( IsfData &source, quint32 length, quint8 blockSize, QByteArray &decodedData )
    {
      Q_UNUSED( source );
      Q_UNUSED( length );
      Q_UNUSED( decodedData );

      if( blockSize > 64 )
      {
        qWarning() << "A block size of" << blockSize << "is too high!";
        blockSize = 64; // Fuck it!
      }

      quint32 pos = 0;
      quint64 signMask = 0XFFFFFFFFFFFFFFFFLL << ( blockSize - 1 );

      while( ! source.atEnd() && pos < length )
      {
        quint8 valuePos = 0;
        quint64 value = 0;

        while( ! source.atEnd() && valuePos < blockSize )
        {
          value |= ( source.getBit() << valuePos++ );
        }

        // If the maks matches, ths sign bit is set, so ORing value and mask will
        // set all leftmost bits to 1, making it a real 64bit signed integer
        if( value & signMask )
        {
          value |= signMask;
        }

        source.append( QByteArray( (char*)&value, sizeof(quint64) ) );
        pos++;
      }

      return true;
    }


  }
};
