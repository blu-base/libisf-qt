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

#include "libisf.h"
#include "format/types.h"

#include <QDebug>



namespace Isf
{



  bool decodeIsf( const QByteArray &bytes, Image &image )
  {
    int pos = 0;
//     quint64 tagNumber = 0;

    // Check for the header, by reading the first multibyte int
    if( decodeUInt( bytes, pos ) != 0 )
    {
#ifdef LIBISF_DEBUG
      qDebug() << "Header was not valid! decoded byte:" << decodeUInt( bytes, pos );
#endif

      return false;
    }

    return true;
  }



  bool encodeIsf( const Image &image, QByteArray &bytes )
  {
    return true;
  }



};
