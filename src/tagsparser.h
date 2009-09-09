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
  namespace Compress
  {
    class DataSource;
  }
  using Compress::DataSource;
  class Drawing;



  /**
   * The methods in this class can parse the ISF tags data.
   *
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class TagsParser
  {

    public: // Static public methods
      static IsfError parseCustomTag( DataSource &source, Drawing &drawing, quint64 tagIndex );
      static IsfError parseGuidTable( DataSource &source, Drawing &drawing );
      static IsfError parseHiMetricSize( DataSource &source, Drawing &drawing );
      static IsfError parseInkSpaceRectangle( DataSource &source, Drawing &drawing );
      static IsfError parseAttributeBlock( DataSource &source, Drawing &drawing );
      static IsfError parseAttributeTable( DataSource &source, Drawing &drawing );
      static IsfError parsePersistentFormat( DataSource &source, Drawing &drawing );
      static IsfError parseMetricBlock( DataSource &source, Drawing &drawing );
      static IsfError parseMetricTable( DataSource &source, Drawing &drawing );
      static IsfError parseTransformation( DataSource &source, Drawing &drawing, quint64 transformType );
      static IsfError parseTransformationTable( DataSource &source, Drawing &drawing );
      static IsfError parseStroke( DataSource &source, Drawing &drawing );
      static IsfError parseStrokeDescBlock( DataSource &source, Drawing &drawing );
      static IsfError parseStrokeDescTable( DataSource &source, Drawing &drawing );

    public: // Static public debugging methods
      static IsfError parseUnsupported( DataSource &source, const QString &tagName );

    private: // Static private debugging methods
      static QByteArray  analyzePayload( DataSource &source, const QString &tagName );
      static QByteArray  analyzePayload( DataSource &source, const quint64 payloadSize, const QString &message = QString() );

  };
}



#endif
