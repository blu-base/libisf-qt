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

#ifndef ISFCOMPRESSION_ISFDATA_H
#define ISFCOMPRESSION_ISFDATA_H

#include <QBitArray>
#include <QBuffer>



namespace Isf
{
  namespace Compress
  {
    /// Class to handle a QBuffer at bit level
    class IsfData
    {

      public: // Public methods

        /// Constructor
                    IsfData( QByteArray &data );
        /// Destructor
                   ~IsfData();

        /// Retrieve the next bit from the data
        bool        getBit();
        /// Retrieve the next <amount> bits from the data
        quint32     getBits( quint8 amount );
        /// Retrieve the index of the current bit
        quint8      getBitIndex();
        /// Retrieve the next byte from the data
        char        getByte();
        /// Go back one byte in the stream
        void        seekByteBack();


      private: // Private methods

        // Move a byte into the bit array
        void       moveByteToBitArray();

      private: // Private properties

        // Main data buffer
        QBuffer     buffer_;
        // Current bit within the bit array
        quint8      currentBitIndex_;
        // Current byte
        QBitArray   currentByte_;

    };
  }
}



#endif
