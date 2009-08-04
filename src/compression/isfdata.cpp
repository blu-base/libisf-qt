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


/**
 * Uncomment to get a ton of debugging messages about
 * data reading and writing
 */
#define ISFDATA_DEBUG_VERBOSE



namespace Isf
{
  namespace Compress
  {


    // Constructor with no initial data
    IsfData::IsfData()
    : currentBitIndex_( 0 )
    {
      buffer_.open( QBuffer::ReadWrite );

      currentByte_.resize( 8 );
    }

    // Constructor
    IsfData::IsfData( const QByteArray &data )
    : currentBitIndex_( 0 )
    {
      buffer_.setData( data );
      buffer_.open( QBuffer::ReadWrite );
      buffer_.seek( 0 );

      currentByte_.resize( 8 );

      // Move the first byte of data in the bit array
      moveByteToBitArray();
    }



    // Destructor
    IsfData::~IsfData()
    {
    }



    // Insert a byte at the end of the data
    void IsfData::append( char byte )
    {
      bool wasEmpty = ( buffer_.size() == 0 );
      qint64 oldPosition = wasEmpty ? 0 : buffer_.pos();

      if( buffer_.write( &byte, 1 ) != 1 )
      {
        qWarning() << "IsfData::append() - Write failed at buffer position" << buffer_.pos();

        buffer_.seek( oldPosition );
        return;
      }

      buffer_.seek( oldPosition );

      // Prepare the first byte to be read
      if( wasEmpty )
      {
        moveByteToBitArray();
      }
    }



    // Insert bytes at the end of the data
    void IsfData::append( const QByteArray &bytes )
    {
      bool wasEmpty = ( buffer_.size() == 0 );
      qint64 oldPosition = wasEmpty ? 0 : buffer_.pos();

      if( buffer_.write( bytes ) != bytes.size() )
      {
        qWarning() << "IsfData::append() - Write of" << bytes.size() << "bytes failed at buffer position" << buffer_.pos();

        buffer_.seek( oldPosition );
        return;
      }

      buffer_.seek( oldPosition );

      // Prepare the first byte to be read
      if( wasEmpty )
      {
        moveByteToBitArray();
      }

    }



    // Get whether the buffer is finished
    bool IsfData::atEnd( bool considerBits ) const
    {
      if( considerBits )
      {
        // There's no more data only when both the buffer AND all the bits
        // have been read
        return buffer_.atEnd() && ( currentBitIndex_ >= 7 );
      }

      return buffer_.atEnd();
    }



    // Clear the data buffer
    void IsfData::clear()
    {
      buffer_.close();
      buffer_.setData( QByteArray() );
      buffer_.open( QIODevice::ReadWrite );
      buffer_.seek( 0 );

      currentBitIndex_ = 0;

      currentByte_.clear();
      currentByte_.resize( 8 );
    }



    // Retrieve the index of the current bit
    quint8 IsfData::getBitIndex()
    {
      return currentBitIndex_;
    }



    // Retrieve the next bit from the data
    bool IsfData::getBit()
    {
      if( currentBitIndex_ >= 8 )
      {
        moveByteToBitArray();
      }

      return currentByte_.at( currentBitIndex_++ );
    }



    // Retrieve the next <amount> bits from the data
    quint64 IsfData::getBits( quint8 amount )
    {
      if( amount > 64 )
      {
        qWarning() << "IsfData:getBits() - Cannot retrieve" << amount << "bits, the maximum is 64 bits!";
        return 0;
      }

      quint8 bitIndex = 0;
      quint64 value = 0;

      while( ! atEnd( true ) && bitIndex < amount )
      {
        if( getBit() )
        {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
          value |= 1 << bitIndex;
#else
          value |= 1 << ( amount - bitIndex - 1 );
#endif
        }

        ++bitIndex;
      }

      return value;
    }



    // Retrieve the next byte from the data
    char IsfData::getByte()
    {
      quint8 pos = 0;
      qint8 byte = 0;

      while( pos < 8 )
      {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        byte |= ( getBit() << pos++ );
#else
        byte |= ( getBit() << ( 7 - pos++ ) );
#endif
      }

      return byte;
    }



    // Retrieve the next <amount> bytes from the data
    QByteArray IsfData::getBytes( quint8 amount )
    {
      QByteArray bytes;
      quint8     index = 0;

      while( ! atEnd() && index < amount )
      {
        bytes.append( getByte() );
        ++index;
      }

      return bytes;
    }



    // Move a byte from the buffer into the bit array
    void IsfData::moveByteToBitArray()
    {
      uchar byte = 0;

      if( buffer_.size() == 0 )
      {
        qWarning() << "IsfData:moveByteToBitArray() - The buffer is empty!";
        currentByte_.clear();
        currentBitIndex_ = 0;
        return;
      }

      if( buffer_.atEnd() )
      {
        qWarning() << "IsfData:moveByteToBitArray() - The buffer was completely parsed!";
        currentByte_.clear();
        currentBitIndex_ = 0;
        return;
      }

      if( buffer_.read( (char*)&byte, 1 ) != 1 )
      {
        qWarning() << "IsfData::moveByteToBitArray() - Read failed at buffer position" << buffer_.pos();
        return;
      }


      for( qint8 i = 0; i < 8; i++ )
      {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        currentByte_.setBit( i, byte & ( 1 << i ) );
#else
        currentByte_.setBit( 7 - i, byte & ( 1 << i ) );
#endif
      }

      currentBitIndex_ = 0;
    }



    // Get the current position within the data
    qint64 IsfData::pos() const
    {
      return buffer_.pos();
    }



    // Seek back and forth in the stream
    void IsfData::seekRelative( int pos )
    {
      if( pos < 0 && buffer_.pos() <= 0 )
      {
#ifdef ISFDATA_DEBUG_VERBOSE
        qWarning() << "Cannot seek back!";
#endif
        return;
      }
      if( pos > 0 && ( buffer_.size() - buffer_.pos() ) < pos )
      {
#ifdef ISFDATA_DEBUG_VERBOSE
        qWarning() << "Cannot seek forward!";
#endif
        return;
      }

      buffer_.seek( buffer_.pos() + pos );
    }



    // Replace the data array with another
    void IsfData::setData( const QByteArray &data )
    {
      buffer_.close();
      buffer_.setData( data );
      buffer_.open( QIODevice::ReadWrite );
      buffer_.seek( 0 );

      currentBitIndex_ = 0;

      // Move the first byte from the new data in the bit array
      moveByteToBitArray();
    }



    // Skip the rest of the current byte
    void IsfData::skipToNextByte()
    {
      if( currentBitIndex_ == 0 )
      {
        return;
      }

      moveByteToBitArray();
    }



    // Get the size of the data
    qint64 IsfData::size() const
    {
      return buffer_.size();
    }



  }



};


