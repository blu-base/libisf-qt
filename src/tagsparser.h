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

#ifndef TAGSPARSER_H
#define TAGSPARSER_H

#include <QMap>
#include <QMatrix>
#include <QString>

#include <IsfQt>



namespace Isf
{


  // Forward declarations
  class StreamData;
  class Drawing;



  /**
   * The methods in this class can parse the ISF tags data.
   *
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class TagsParser
  {

    public: // Static public methods
      static IsfError parseCustomTag( StreamData* streamData, Drawing& drawing, quint64 tagIndex );
      static IsfError parseGuidTable( StreamData* streamData, Drawing& drawing );
      static IsfError parseHiMetricSize( StreamData* streamData, Drawing& drawing );
      static IsfError parseInkSpaceRectangle( StreamData* streamData, Drawing& drawing );
      static IsfError parseAttributeBlock( StreamData* streamData, Drawing& drawing );
      static IsfError parseAttributeTable( StreamData* streamData, Drawing& drawing );
      static IsfError parsePersistentFormat( StreamData* streamData, Drawing& drawing );
      static IsfError parseMetricBlock( StreamData* streamData, Drawing& drawing );
      static IsfError parseMetricTable( StreamData* streamData, Drawing& drawing );
      static IsfError parseTransformation( StreamData* streamData, Drawing& drawing, quint64 transformType );
      static IsfError parseTransformationTable( StreamData* streamData, Drawing& drawing );
      static IsfError parseStroke( StreamData* streamData, Drawing& drawing );
      static IsfError parseStrokeDescBlock( StreamData* streamData, Drawing& drawing );
      static IsfError parseStrokeDescTable( StreamData* streamData, Drawing& drawing );

    public: // Static public debugging methods
      static IsfError parseUnsupported( StreamData* streamData, const QString& tagName );

    private: // Static private debugging methods
      static QByteArray  analyzePayload( StreamData* streamData, const QString& tagName );
      static QByteArray  analyzePayload( StreamData* streamData, const quint64 payloadSize, const QString& message = QString() );

  };
}



#endif
