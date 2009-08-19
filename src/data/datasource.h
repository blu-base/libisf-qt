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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QBitArray>
#include <QBuffer>



namespace Isf
{
  namespace Compress
  {
    /// Class to handle a QBuffer at bit level
    class DataSource
    {



      public: // Constructors

        /// Constructor with no initial data
                          DataSource();
        /// Constructor with initial data buffer
                          DataSource( const QByteArray &data );
        /// Destructor
                         ~DataSource();


      public: // Public status retrieval methods

        /// Get whether the buffer is finished (use considerBits
        /// to also check whether there are more bits to read or not)
        bool              atEnd( bool considerBits = false ) const;
        /// Return a reference to the data array
        const QByteArray &data() const;
        /// Get the current position within the data
        qint64            pos() const;
        /// Get the size of the data
        qint64            size() const;


      public: // Public data manipulation methods

        /// Insert a byte at the end of the data
        void              append( char byte );
        /// Insert bits at the end of the data
        void              append( const QBitArray &bits );
        /// Insert bytes at the end of the data
        void              append( const QByteArray &bytes );
        /// Clear the data buffer
        void              clear();
        /// Flush the current byte to the buffer
        void              flush();
        /// Retrieve the next bit from the data
        bool              getBit( bool *ok = 0 );
        /// Retrieve the next <amount> bits from the data
        quint64           getBits( quint8 amount, bool *ok = 0 );
        /// Retrieve the index of the current bit
        quint8            getBitIndex();
        /// Retrieve the next byte from the data
        char              getByte( bool *ok = 0 );
        /// Retrieve the next <amount> bytes from the data
        QByteArray        getBytes( quint8 amount, bool *ok = 0 );
        /// Insert a byte at the beginning of the data
        void              prepend( char byte );
        /// Insert bytes at the beginning of the data
        void              prepend( const QByteArray &bytes );
        /// Return to the start of the buffer
        void              reset();
        /// Replace the data array with another
        void              setData( const QByteArray &data );
        /// Seek back and forth in the stream
        void              seekRelative( int pos );
        /// Skip the rest of the current byte
        void              skipToNextByte();
        /// Reset the current byte
        void              skipToPrevByte();


      private: // Private methods

        // Move a byte from the buffer into the bit array
        bool              moveByteToBitArray();


      private: // Private properties

        // Main data buffer
        QBuffer           buffer_;
        // Current bit within the bit array
        quint8            currentBitIndex_;
        // Current byte
        QBitArray         currentByte_;

    };



  }
}



#endif
