/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
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

#include "isfdata.h"

#include <QDebug>



namespace Isf
{
  namespace Compress
  {
    // Constructor
    IsfData::IsfData( QByteArray &data )
    : currentBitIndex_( 0 )
    {
      buffer_.setBuffer( &data );

      currentByte_.resize( 8 );

      // Move the first byte of data in the bit array
      moveByteToBitArray();
    }



    // Destructor
    IsfData::~IsfData()
    {
    }



    /// Retrieve the index of the current bit
    quint8 IsfData::getBitIndex()
    {
      return currentBitIndex_;
    }



    /// Retrieve the next bit from the data
    bool IsfData::getBit()
    {
      if( currentBitIndex_ >= 8 )
      {
        moveByteToBitArray();
      }

      return currentByte_.at( currentBitIndex_++ );
    }



    /// Retrieve the next <amount> bits from the data
    quint32 IsfData::getBits( quint8 amount )
    {
      quint8 pos = 0;
      quint32 value = 0;

      while( pos < amount )
      {
        value |= ( getBit() << pos++ );
      }

      return value;
    }



    /// Retrieve the next byte from the data
    char IsfData::getByte()
    {
      quint8 pos = 0;
      qint8 byte = 0;

      while( pos < 8 )
      {
        byte |= ( getBit() << pos++ );
      }

      return byte;
    }



    // Move a byte into the bit array
    void IsfData::moveByteToBitArray()
    {
      char byte = 0;

      if( buffer_.read( &byte, 1 ) != 1 )
      {
        qWarning() << "IsfData::moveByteToBitArray() - Read failed at buffer position" << buffer_.pos();
        return;
      }

      for( qint8 i = 0; i < 8; i++ )
      {
        currentByte_.setBit( i, byte & ( 1 << i ) );
      }

      currentBitIndex_ = 0;
    }



    /// Go back one byte in the stream
    void IsfData::seekByteBack()
    {
      buffer_.seek( -1 );
    }



  }



};


