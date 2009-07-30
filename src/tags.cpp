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



    /// Read away an unsupported tag
    IsfError parseUnsupported( IsfData &source, const QString &tagName )
    {
      // Unsupported content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, tagName );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read the table of GUIDs from the data
    IsfError parseGuidTable( IsfData &source, quint64 &maxGuid )
    {
      quint64 value = Isf::Compress::decodeUInt( source );

      // GUIDs are 16 bytes long
      maxGuid = 99 + value / 16;

      // The rest of the payload is unknown
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, "GUID Table" );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read payload: Persistent Format
    IsfError parsePersistentFormat( IsfData &source )
    {
      // Unknown content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, "Persistent Format" );
#endif
      return ISF_ERROR_NONE;
    }



    // Print the payload of an unknown tag
    void analyzePayload( IsfData &source, const QString &tagName )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
        return;
      }


      quint64 pos = 0;
      QByteArray output;

      qDebug() << "Got tag: " << tagName << "with" << payloadSize << "bytes of payload";
      qDebug() << "--------------------------------------------------------------------";
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

      qDebug() << "--------------------------------------------------------------------";
    }



  }
}


