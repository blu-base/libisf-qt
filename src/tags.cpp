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

#include "tags.h"

#include "compression/isfdata.h"
#include "multibytecoding.h"

#include <QDebug>



namespace Isf
{
  namespace Tags
  {



    /// Read the table of GUIDs from the data
    IsfError parseGuidTable( IsfData &source )
    {
      // Unknown content
      analyzePayload( source, "GUID Table" );
      return ISF_ERROR_NONE;
    }



    /// Read payload: Persistent Format
    IsfError parsePersistentFormat( IsfData &source )
    {
      // Unknown content
      analyzePayload( source, "Persistent Format" );
      return ISF_ERROR_NONE;
    }



    // Print the payload of an unknown tag
    void analyzePayload( IsfData &source, const QString &tagName )
    {
      qint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
        qWarning() << "Got empty payload for tag" << tagName << "!";
        return;
      }

      qint64 pos = 0;
      QByteArray output;

      qDebug() << "------------ Payload contents for tag" << tagName << "------------";
      while( ! source.atEnd() && pos < payloadSize )
      {
        quint8 byte = source.getByte();
        output.append( QByteArray::number( byte, 16 ).rightJustified( 2, '0').toUpper() + " " );

        if( ( ( pos + 1 ) % 24 ) == 0 )
        {
          qDebug() << output;
          output.clear();
        }

        ++pos;
      }

      if( ! output.isEmpty() )
      {
        qDebug() << output;
      }

      qDebug() << "------------ Payload contents for tag" << tagName << "------------";
    }

  }
}


