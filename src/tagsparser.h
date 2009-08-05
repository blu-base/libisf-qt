/***************************************************************************
 *   Copyright (C) 2008-2009 by Valerio Pilo                               *
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

#ifndef TAGSPARSER_H
#define TAGSPARSER_H

#include "isfqt.h"

#include <QMap>
#include <QTransform>
#include <QString>



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

      /// Read the table of GUIDs from the data
      static IsfError parseGuidTable( DataSource &source, Drawing &drawing );
      /// Read the drawing dimensions
      static IsfError parseHiMetricSize( DataSource &source, Drawing &drawing );
      /// Read the ink canvas dimensions
      static IsfError parseInkSpaceRectangle( DataSource &source, Drawing &drawing );
      /// Read a block of points attributes
      static IsfError parseAttributeBlock( DataSource &source, Drawing &drawing );
      /// Read a table of points attributes
      static IsfError parseAttributeTable( DataSource &source, Drawing &drawing );
      /// Read payload: Persistent Format
      static IsfError parsePersistentFormat( DataSource &source, Drawing &drawing );
      /// Read payload: Metric Block
      static IsfError parseMetricBlock( DataSource &source, Drawing &drawing );
      /// Read payload: Metric Table
      static IsfError parseMetricTable( DataSource &source, Drawing &drawing );
      /// Read a drawing transformation matrix
      static IsfError parseTransformation( DataSource &source, Drawing &drawing, quint64 transformType );
      /// Read a table of transformation matrices
      static IsfError parseTransformationTable( DataSource &source, Drawing &drawing );
      /// Read a stroke
      static IsfError parseStroke( DataSource &source, Drawing &drawing );
      /// Read a stroke description block
      static IsfError parseStrokeDescBlock( DataSource &source, Drawing &drawing );
      /// Read a stroke description table
      static IsfError parseStrokeDescTable( DataSource &source, Drawing &drawing );


    public: // Static public debugging methods

      /// Read away an unsupported tag
      static IsfError parseUnsupported( DataSource &source, const QString &tagName );


    private: // Static private debugging methods

      // Print the payload of an unknown tag
      static void     analyzePayload( DataSource &source, const QString &tagName );
      // Print the payload of an unknown tag
      static void     analyzePayload( DataSource &source, const quint64 payloadSize, const QString &message );

  };
}



#endif
