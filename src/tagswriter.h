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

#ifndef TAGSWRITER_H
#define TAGSWRITER_H

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
   * The methods in this class can convert data to ISF tags.
   *
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class TagsWriter
  {
    public: // Static public methods

      /// Write the persistent format tag
      static IsfError addPersistentFormat( DataSource &source, const Drawing &drawing );
      /// Write the drawing dimensions
      static IsfError addHiMetricSize( DataSource &source, const Drawing &drawing );
      /// Write a table of points attributes
      static IsfError addAttributeTable( DataSource &source, const Drawing &drawing );
      /// Write a table of metrics
      static IsfError addMetricsTable( DataSource &source, const Drawing &drawing );
      /// Write a table of transformation matrices
      static IsfError addTransformationTable( DataSource &source, const Drawing &drawing );
      /// Write the strokes
      static IsfError addStrokes( DataSource &source, const Drawing &drawing );

/*
      /// Write the table of GUIDs from the data
      static IsfError addGuidTable( DataSource &source, const Drawing &drawing );
      /// Write the drawing dimensions
      static IsfError addHiMetricSize( DataSource &source, const Drawing &drawing );
      /// Write the ink canvas dimensions
      static IsfError addInkSpaceRectangle( DataSource &source, const Drawing &drawing );
      /// Write payload: Persistent Format
      static IsfError addPersistentFormat( DataSource &source, const Drawing &drawing );
      /// Write payload: Metric Block
      static IsfError addMetricBlock( DataSource &source, const Drawing &drawing );
      /// Write payload: Metric Table
      static IsfError addMetricTable( DataSource &source, const Drawing &drawing );
      /// Write a stroke description block
      static IsfError addStrokeDescBlock( DataSource &source, const Drawing &drawing );
      /// Write a stroke description table
      static IsfError addStrokeDescTable( DataSource &source, const Drawing &drawing );
*/

  };
}



#endif
