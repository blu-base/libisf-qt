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

#ifndef ISFQT_INTERNAL_H
#define ISFQT_INTERNAL_H

#include "isfqtconfig.h"

#include <IsfQt>

#include <QtDebug>


/**
 * Uncomment this define to enable very verbose debugging output
 */
#ifdef ISFQT_DEBUG
  #define ISFQT_DEBUG_VERBOSE
#endif



namespace Isf
{
  // Forward declarations
  class Stroke;
  namespace Compress
  {
    class DataSource;
  }



  /**
   * A list of all known ISF tag indexes into the GUID
   * table
   */
  enum DataTag
  {
    TAG_INK_SPACE_RECT                      =   0
  , TAG_GUID_TABLE
  , TAG_DRAW_ATTRS_TABLE
  , TAG_DRAW_ATTRS_BLOCK
  , TAG_STROKE_DESC_TABLE
  , TAG_STROKE_DESC_BLOCK
  , TAG_BUTTONS
  , TAG_NO_X
  , TAG_NO_Y
  , TAG_DIDX
  , TAG_STROKE                           // =  10
  , TAG_STROKE_PROPERTY_LIST
  , TAG_POINT_PROPERTY
  , TAG_SIDX
  , TAG_COMPRESSION_HEADER
  , TAG_TRANSFORM_TABLE
  , TAG_TRANSFORM
  , TAG_TRANSFORM_ISOTROPIC_SCALE
  , TAG_TRANSFORM_ANISOTROPIC_SCALE
  , TAG_TRANSFORM_ROTATE
  , TAG_TRANSFORM_TRANSLATE              // =  20
  , TAG_TRANSFORM_SCALE_AND_TRANSLATE
  , TAG_TRANSFORM_QUAD
  , TAG_TIDX
  , TAG_METRIC_TABLE
  , TAG_METRIC_BLOCK
  , TAG_MIDX
  , TAG_MANTISSA
  , TAG_PERSISTENT_FORMAT
  , TAG_HIMETRIC_SIZE
  , TAG_STROKE_IDS                       // =  30
  , DEFAULT_TAGS_NUMBER                     = 100
  , FIRST_CUSTOM_TAG_ID                     = 100
  };
  Q_DECLARE_FLAGS( DataTags, DataTag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( DataTags )



  /**
   * ISF parser state machine states
   */
  enum ParserState
  {
    ISF_PARSER_START = 0,        ///< The parser has not read anything from the data yet
    ISF_PARSER_FINISH,           ///< The parser is done
    ISF_PARSER_STREAMSIZE,       ///< The parser is reading the ISF stream size
    ISF_PARSER_TAG               ///< The parser is reading tags from the stream
  };



  /**
   * Persistent Format version
   *
   * @see TagsParser::parsePersistentFormat() for more info.
   */
  const uint ISF_PERSISTENT_FORMAT_VERSION = 65536;



  /**
   * Drawing attributes for points.
   */
  struct AttributeSet
  {
    /// Constructor
    AttributeSet()
    : color( Qt::black )
    , flags( 0x10 )        // Meaning unknown
    , penSize( 8.f, 8.f )  // Default pen is 8.0 pixels wide
    {
    }
    /// Quick comparison operator
    bool operator ==( const AttributeSet& other )
    {
      return color   == other.color
          && flags   == other.flags
          && penSize == other.penSize;
    }
    /// Quick comparison operator
    bool operator !=( const AttributeSet& other )
    {
      return color   != other.color
          || flags   != other.flags
          || penSize != other.penSize;
    }

    /// The stroke color, optionally with alpha channel
    QColor       color;
    /// Mask of StrokeFlags, @see StrokeFlags
    StrokeFlags  flags;
    /// Dimensions of the pencil in pixels
    QSizeF       penSize;
  };



  /**
   * Drawing attributes for strokes.
   */
  struct StrokeInfo
  {
    /// Constructor
    StrokeInfo()
    : hasPressureData( false )
    , hasXData( true )
    , hasYData( true )
    {
    }

    /// Whether the stroke contains pressure info or not
    bool hasPressureData;
    /// Whether the stroke contains X coordinates or not
    bool hasXData;
    /// Whether the stroke contains Y coordinates or not
    bool hasYData;
  };



  /**
   * Internal parser data
   */
  struct StreamData
  {
    StreamData()
    : currentAttributeSetIndex( 0 )
    , currentMetricsIndex( 0 )
    , currentStrokeInfoIndex( 0 )
    , currentTransformsIndex( 0 )
    , dataSource( 0 )
    {
    }

    /// List of attributes of the points in the drawing
    QList<AttributeSet>        attributeSets;
    /// Drawing's bounding rectangle
    QRect                      boundingRect;
    /// Current attribute set
    quint64                    currentAttributeSetIndex;
    /// Current metrics set
    quint64                    currentMetricsIndex;
    /// Current stroke info
    quint64                    currentStrokeInfoIndex;
    /// Current transform
    quint64                    currentTransformsIndex;
    /// Raw bytes source used to read and write streams
    Compress::DataSource*      dataSource;
    /// List of metrics used in the drawing
    QList<Metrics*>            metrics;
    /// List of stroke info
    QList<StrokeInfo*>         strokeInfos;
    /// Transformation matrices
    QList<QMatrix*>            transforms;
  };



}



#endif
