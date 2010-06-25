/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2009 by Adam Goossens                                   *
 *   adam@kmess.org                                                        *
 ***************************************************************************/

/***************************************************************************
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
    /**
     * @class DataSource
     * @brief Class to handle a QBuffer at the bit level.
     *
     * You can use this class in two ways: first, reading and writing bytes
     * normally. Second, reading and writing individual bits.
     *
     * Warning: Mixing up the usage modes is not very well tested!
     */
    class DataSource
    {

      public: // Public constructors
                          DataSource();
                          DataSource( const QByteArray& data );
                         ~DataSource();

      public: // Public status retrieval methods
        bool              atEnd( bool considerBits = false ) const;
        const QByteArray& data() const;
        qint64            pos() const;
        qint64            size() const;

      public: // Public data manipulation methods
        void              append( char byte );
        void              append( const QBitArray& bits );
        void              append( const QByteArray& bytes );
        void              clear();
        void              flush();
        bool              getBit( bool* ok = 0 );
        quint64           getBits( quint8 amount, bool* ok = 0 );
        quint8            getBitIndex();
        char              getByte( bool* ok = 0 );
        QByteArray        getBytes( quint8 amount, bool* ok = 0 );
        void              prepend( char byte );
        void              prepend( const QByteArray& bytes );
        void              reset();
        void              setData( const QByteArray& data );
        void              seekRelative( int pos );
        void              skipToNextByte();
        void              skipToPrevByte();

      private: // Private methods
        bool              moveByteToBitArray();

      private: // Private properties
        /// Main data buffer
        QBuffer           buffer_;
        /// Current bit within the bit array
        quint8            currentBitIndex_;
        /// Current byte
        QBitArray         currentByte_;

    };



  }
}



#endif
