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

#ifndef ISFTAGS_H
#define ISFTAGS_H

#include "libisftypes.h"

#include <QMap>
#include <QTransform>
#include <QString>



namespace Isf
{


  // Forward declarations
  namespace Compress
  {
    class IsfData;
  }
  using Compress::IsfData;



  /**
   * The methods in this namespace can parse the ISF tags data.
   *
   * @author Valerio Pilo (valerio@kmess.org)
   */
  namespace Tags
  {


    /// Read the table of GUIDs from the data
    IsfError parseGuidTable( IsfData &source, quint64 &maxGuid );
    /// Read the drawing dimensions
    IsfError parseHiMetricSize( IsfData &source, QSize &size );
    /// Read the ink canvas dimensions
    IsfError parseInkSpaceRectangle( IsfData &source, QRect &rect );
    /// Read a block of points attributes
    IsfError parseAttributeBlock( IsfData &source, QList<PointInfo> &attributes, int blockIndex = 0 );
    /// Read a table of points attributes
    IsfError parseAttributeTable( IsfData &source, QList<PointInfo> &attributes );
    /// Read payload: Persistent Format
    IsfError parsePersistentFormat( IsfData &source );
    /// Read payload: Metric Block
    IsfError parseMetricBlock( IsfData &source );
    /// Read payload: Metric Table
    IsfError parseMetricTable( IsfData &source );
    /// Read a drawing transformation matrix
    IsfError parseTransformation( IsfData &source, QMap<DataTag,QTransform> transforms, DataTag transformType );
    /// Read a table of transformation matrices
    IsfError parseTransformationTable( IsfData &source, QMap<DataTag,QTransform> transforms );
    /// Read a stroke
    IsfError parseStroke( IsfData &source, QList<Stroke> &strokes, QList<PointInfo> &attributes, bool hasPressureData );
    /// Read a stroke description block
    IsfError parseStrokeDescBlock( IsfData &source, QList<Stroke> &strokes, bool &hasXData, bool &hasYData, bool &hasPressureData );
    /// Read a stroke description table
    IsfError parseStrokeDescTable( IsfData &source, QList<Stroke> &strokes, bool &hasXData, bool &hasYData, bool &hasPressureData );


    // Debugging methods

    /// Read away an unsupported tag
    IsfError parseUnsupported( IsfData &source, const QString &tagName );
    // Print the payload of an unknown tag
    void     analyzePayload( IsfData &source, const QString &tagName );
    // Print the payload of an unknown tag
    void     analyzePayload( IsfData &source, const quint64 payloadSize, const QString &message );

  }
}



#endif
