/***************************************************************************
 *   Copyright (C) 2008 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2009 by Adam Goossens                                   *
 *   adam@kmess.org                                                        *
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

#ifndef LIBISFTYPES_H
#define LIBISFTYPES_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QtGlobal>
#include <QTransform>


/**
 * Uncomment this define to enable debugging output
 */
#define ISF_DEBUG_VERBOSE



namespace Isf
{



  /**
   * A list of all known ISF tag indexes into the GUID
   * table
   */
  enum DataTag
  {
    TAG_INK_SPACE_RECT                  = 0
  , TAG_GUID_TABLE
  , TAG_DRAW_ATTRS_TABLE
  , TAG_DRAW_ATTRS_BLOCK
  , TAG_STROKE_DESC_TABLE
  , TAG_STROKE_DESC_BLOCK
  , TAG_BUTTONS
  , TAG_NO_X
  , TAG_NO_Y
  , TAG_DIDX
  , TAG_STROKE
  , TAG_STROKE_PROPERTY_LIST
  , TAG_POINT_PROPERTY
  , TAG_SIDX
  , TAG_COMPRESSION_HEADER
  , TAG_TRANSFORM_TABLE
  , TAG_TRANSFORM
  , TAG_TRANSFORM_ISOTROPIC_SCALE
  , TAG_TRANSFORM_ANISOTROPIC_SCALE
  , TAG_TRANSFORM_ROTATE
  , TAG_TRANSFORM_TRANSLATE
  , TAG_TRANSFORM_SCALE_AND_TRANSLATE
  , TAG_TRANSFORM_QUAD
  , TAG_TIDX
  , TAG_METRIC_TABLE
  , TAG_METRIC_BLOCK
  , TAG_MIDX
  , TAG_MANTISSA
  , TAG_PERSISTENT_FORMAT
  , TAG_HIMETRIC_SIZE
  , TAG_STROKE_IDS
  };
  Q_DECLARE_FLAGS( DataTags, DataTag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( DataTags )



  /**
   * List of predefined packet properties
   */
  enum StrokePacketProperty
  {
    GUID_X                      =  0
  , GUID_Y
  , GUID_Z
  , GUID_PACKET_STATUS
  , GUID_TIMER_TICK
  , GUID_SERIAL_NUMBER
  , GUID_NORMAL_PRESSURE
  , GUID_TANGENT_PRESSURE
  , GUID_BUTTON_PRESSURE        =  8
  , GUID_X_TILT_ORIENTATION
  , GUID_Y_TILT_ORIENTATION
  , GUID_AZIMUTH_ORIENTATION
  , GUID_ALTITUDE_ORIENTATION
  , GUID_TWIST_ORIENTATION
  , GUID_PITCH_ROTATION
  , GUID_ROLL_ROTATION
  , GUID_YAW_ROTATION           = 16
  , GUID_PEN_STYLE
  , GUID_COLORREF
  , GUID_PEN_WIDTH
  , GUID_PEN_HEIGHT
  , GUID_PEN_TIP
  , GUID_DRAWING_FLAGS
  , GUID_CURSORID
  , GUID_WORD_ALTERNATES        = 24
  , GUID_CHAR_ALTERNATES
  , GUID_INKMETRICS
  , GUID_GUIDE_STRUCTURE
  , GUID_TIME_STAMP
  , GUID_LANGUAGE
  , GUID_TRANSPARENCY
  , GUID_CURVE_FITTING_ERROR
  , GUID_RECO_LATTICE            = 32
  , GUID_CURSORDOWN
  , GUID_SECONDARYTIPSWITCH
  , GUID_BARRELDOWN
  , GUID_TABLETPICK
  , GUID_ROP
  , GUID_NUM
  };
  Q_DECLARE_FLAGS( StrokePacketProperties, StrokePacketProperty )
  Q_DECLARE_OPERATORS_FOR_FLAGS( StrokePacketProperties )

/*
  const GUID FAR KNOWN_GUIDS[38] =
  {
    { 0x598a6a8f, 0x52c0, 0x4ba0, { 0x93, 0xaf, 0xaf, 0x35, 0x74, 0x11, 0xa5, 0x61 } },
    { 0xb53f9f75, 0x04e0, 0x4498, { 0xa7, 0xee, 0xc3, 0x0d, 0xbb, 0x5a, 0x90, 0x11 } },
    { 0x735adb30, 0x0ebb, 0x4788, { 0xa0, 0xe4, 0x0f, 0x31, 0x64, 0x90, 0x05, 0x5d } },
    { 0x6e0e07bf, 0xafe7, 0x4cf7, { 0x87, 0xd1, 0xaf, 0x64, 0x46, 0x20, 0x84, 0x18 } },
    { 0x436510c5, 0xfed3, 0x45d1, { 0x8b, 0x76, 0x71, 0xd3, 0xea, 0x7a, 0x82, 0x9d } },
    { 0x78a81b56, 0x0935, 0x4493, { 0xba, 0xae, 0x00, 0x54, 0x1a, 0x8a, 0x16, 0xc4 } },
    { 0x7307502d, 0xf9f4, 0x4e18, { 0xb3, 0xf2, 0x2c, 0xe1, 0xb1, 0xa3, 0x61, 0x0c } },
    { 0x6da4488b, 0x5244, 0x41ec, { 0x90, 0x5b, 0x32, 0xd8, 0x9a, 0xb8, 0x08, 0x09 } },
    { 0x8b7fefc4, 0x96aa, 0x4bfe, { 0xac, 0x26, 0x8a, 0x5f, 0x0b, 0xe0, 0x7b, 0xf5 } },
    { 0xa8d07b3a, 0x8bf0, 0x40b0, { 0x95, 0xa9, 0xb8, 0x0a, 0x6b, 0xb7, 0x87, 0xbf } },
    { 0x0e932389, 0x1d77, 0x43af, { 0xac, 0x00, 0x5b, 0x95, 0x0d, 0x6d, 0x4b, 0x2d } },
    { 0x029123b4, 0x8828, 0x410b, { 0xb2, 0x50, 0xa0, 0x53, 0x65, 0x95, 0xe5, 0xdc } },
    { 0x82dec5c7, 0xf6ba, 0x4906, { 0x89, 0x4f, 0x66, 0xd6, 0x8d, 0xfc, 0x45, 0x6c } },
    { 0x0d324960, 0x13b2, 0x41e4, { 0xac, 0xe6, 0x7a, 0xe9, 0xd4, 0x3d, 0x2d, 0x3b } },
    { 0x7f7e57b7, 0xbe37, 0x4be1, { 0xa3, 0x56, 0x7a, 0x84, 0x16, 0x0e, 0x18, 0x93 } },
    { 0x5d5d5e56, 0x6ba9, 0x4c5b, { 0x9f, 0xb0, 0x85, 0x1c, 0x91, 0x71, 0x4e, 0x56 } },
    { 0x6a849980, 0x7c3a, 0x45b7, { 0xaa, 0x82, 0x90, 0xa2, 0x62, 0x95, 0x0e, 0x89 } },
    { 0x33c1df83, 0xecdb, 0x44f0, { 0xb9, 0x23, 0xdb, 0xd1, 0xa5, 0xb2, 0x13, 0x6e } },
    { 0x5329cda5, 0xfa5b, 0x4ed2, { 0xbb, 0x32, 0x83, 0x46, 0x01, 0x72, 0x44, 0x28 } },
    { 0x002df9af, 0xdd8c, 0x4949, { 0xba, 0x46, 0xd6, 0x5e, 0x10, 0x7d, 0x1a, 0x8a } },
    { 0x9d32b7ca, 0x1213, 0x4f54, { 0xb7, 0xe4, 0xc9, 0x05, 0x0e, 0xe1, 0x7a, 0x38 } },
    { 0xe71caab9, 0x8059, 0x4c0d, { 0xa2, 0xdb, 0x7c, 0x79, 0x54, 0x47, 0x8d, 0x82 } },
    { 0x5c0b730a, 0xf394, 0x4961, { 0xa9, 0x33, 0x37, 0xc4, 0x34, 0xf4, 0xb7, 0xeb } },
    { 0x2812210f, 0x871e, 0x4d91, { 0x86, 0x07, 0x49, 0x32, 0x7d, 0xdf, 0x0a, 0x9f } },
    { 0x8359a0fa, 0x2f44, 0x4de6, { 0x92, 0x81, 0xce, 0x5a, 0x89, 0x9c, 0xf5, 0x8f } },
    { 0x4c4642dd, 0x479e, 0x4c66, { 0xb4, 0x40, 0x1f, 0xcd, 0x83, 0x95, 0x8f, 0x00 } },
    { 0xce2d9a8a, 0xe58e, 0x40ba, { 0x93, 0xfa, 0x18, 0x9b, 0xb3, 0x90, 0x00, 0xae } },
    { 0xc3c7480f, 0x5839, 0x46ef, { 0xa5, 0x66, 0xd8, 0x48, 0x1c, 0x7a, 0xfe, 0xc1 } },
    { 0xea2278af, 0xc59d, 0x4ef4, { 0x98, 0x5b, 0xd4, 0xbe, 0x12, 0xdf, 0x22, 0x34 } },
    { 0xb8630dc9, 0xcc5c, 0x4c33, { 0x8d, 0xad, 0xb4, 0x7f, 0x62, 0x2b, 0x8c, 0x79 } },
    { 0x15e2f8e6, 0x6381, 0x4e8b, { 0xa9, 0x65, 0x01, 0x1f, 0x7d, 0x7f, 0xca, 0x38 } },
    { 0x7066fbe4, 0x473e, 0x4675, { 0x9c, 0x25, 0x00, 0x26, 0x82, 0x9b, 0x40, 0x1f } },
    { 0xbbc85b9a, 0xade6, 0x4093, { 0xb3, 0xbb, 0x64, 0x1f, 0xa1, 0xd3, 0x7a, 0x1a } },
    { 0x039143d3, 0x78cb, 0x449c, { 0xa8, 0xe7, 0x67, 0xd1, 0x88, 0x64, 0xc3, 0x32 } },
    { 0x67743782, 0x0ee5, 0x419a, { 0xa1, 0x2b, 0x27, 0x3a, 0x9e, 0xc0, 0x8f, 0x3d } },
    { 0xf0720328, 0x663b, 0x418f, { 0x85, 0xa6, 0x95, 0x31, 0xae, 0x3e, 0xcd, 0xfa } },
    { 0xa1718cdd, 0x0dac, 0x4095, { 0xa1, 0x81, 0x7b, 0x59, 0xcb, 0x10, 0x6b, 0xfb } },
    { 0x810a74d2, 0x6ee2, 0x4e39, { 0x82, 0x5e, 0x6d, 0xef, 0x82, 0x6a, 0xff, 0xc5 } },
  };
*/





  /**
   * A list of all known pen attribute tags
   */
  enum PenTags
  {
    PEN_STYLE                 = 67
  , PEN_COLOR                 = 68
  , PEN_WIDTH                 = 69
  , PEN_HEIGHT                = 70
  , PEN_TIP                   = 71
  , PEN_TIP_RECTANGLE         =  1
  , PEN_FLAGS                 = 72
  , PEN_TRANSPARENCY          = 80
  , PEN_ISHIGHLIGHTER         = 87
  };



  /**
   * Possible errors generated by the ISF parser
   */
  enum IsfError
  {
    ISF_ERROR_NONE = 0           /// No error
  , ISF_ERROR_BAD_VERSION        /// Incompatible ISF version
  , ISF_ERROR_BAD_STREAMSIZE     /// Stream size of ISF data is too small
  , ISF_ERROR_INVALID_PAYLOAD    /// A tag's payload was empty
  , ISF_ERROR_INVALID_BLOCK      /// A block had invalid contents
  };



  /**
   * Stroke drawing flags
   */
  enum StrokeFlag
  {
    FitToCurve     = 0x0001
  , IgnorePressure = 0x0004
  , IsHighlighter  = 0x0100
  , IsRectangle    = 0x0200
  };
  Q_DECLARE_FLAGS( StrokeFlags, StrokeFlag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( StrokeFlags )



  /**
   * Units used for metric measurements
   */
  enum MetricScale
  {
    CENTIMETERS = 1
  , DEFAULT = 0
  , DEGREES = 2
  , NOT_APPLICABLE = -1
  };



  /**
   * A metric: a set of values representing what kind of values some
   * type of measurement will assume.
   */
  struct Metric
  {
    /// Constructor for invalid metrics
    Metric()
    {
    }
    /// Constructor
    Metric( qint64 vMin, qint64 vMax, MetricScale vUnits, quint32 vResolution )
    : min( vMin )
    , max( vMax )
    , units( vUnits )
    , resolution( vResolution )
    {
    }

    /// Minimum value
    qint64       min;
    /// Maximum value
    qint64       max;
    /// Measurement unit
    MetricScale  units;
    /// Resolution
    quint32      resolution;
  };



  /**
   * A table of metrics.
   *
   * The default metric values are hard-coded here, different values read from the stream
   * will override these
   */
  struct Metrics
  {
    /// Constructor
    Metrics()
    {
      metrics[ GUID_X                    ] = Metric(     0, 12699, CENTIMETERS,  1000 );
      metrics[ GUID_Y                    ] = Metric(     0,  9649, CENTIMETERS,  1000 );
      metrics[ GUID_Z                    ] = Metric( -1023,  1023, CENTIMETERS,  1000 );
      metrics[ GUID_PACKET_STATUS        ] = Metric(     0,  1023, DEFAULT,         1 );
      metrics[ GUID_TIMER_TICK           ] = Metric(     0,  1023, DEFAULT,         1 );
      metrics[ GUID_SERIAL_NUMBER        ] = Metric(     0,  1023, DEFAULT,         1 );
      metrics[ GUID_NORMAL_PRESSURE      ] = Metric(     0,  3600, DEGREES,        10 );
      metrics[ GUID_TANGENT_PRESSURE     ] = Metric(     0,  3600, DEGREES,        10 );
      metrics[ GUID_BUTTON_PRESSURE      ] = Metric(     0,  3600, DEGREES,        10 );
      metrics[ GUID_X_TILT_ORIENTATION   ] = Metric(  -900,   900, DEGREES,        10 );
      metrics[ GUID_Y_TILT_ORIENTATION   ] = Metric(     0,  3600, DEGREES,        10 );
      metrics[ GUID_AZIMUTH_ORIENTATION  ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
      metrics[ GUID_ALTITUDE_ORIENTATION ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
      metrics[ GUID_TWIST_ORIENTATION    ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
      metrics[ GUID_PITCH_ROTATION       ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
      metrics[ GUID_ROLL_ROTATION        ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
      metrics[ GUID_YAW_ROTATION         ] = Metric(    -1,    -1, NOT_APPLICABLE, -1 );
    }

    /// The list of metrics defined in this table
    QMap<int,Metric> metrics;
  };



  /**
   * Drawing attributes for points
   */
  struct PointInfo
  {
    /// Constructor
    PointInfo()
    : color( Qt::black )
    , flags( 0 )
    , penSize( 20, 20 ) // +/- 8px in HiMetric units
    {
    }

    /// color in AABBGGRR format (Alpha channel: 00 is solid, FF is transparent)
    QColor       color;
    /// mask of StrokeFlags
    StrokeFlags  flags;
    /// dimensions of the pencil in Himetric units
    QSizeF       penSize;
  };



  /**
   * Drawing attributes for strokes
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
   * A single point within a stroke
   */
  struct Point
  {
    /// Constructor
    Point()
    : position( 0, 0 )
    , pressureLevel( 0 )
    {
    }

    /// coordinates
    QPoint      position;
    /// Pressure information
    qint64      pressureLevel;
  };



  /**
   * A pen stroke
   */
  struct Stroke
  {
    /// Constructor
    Stroke()
    : attributes( 0 )
    , info( 0 )
    , metrics( 0 )
    , transform( 0 )
    {
    }

    /// Bounding rectangle of this stroke
    QRect         boundingRect;
    /// Link to the attributes of this stroke's points, if any
    PointInfo    *attributes;
    /// Link to this stroke's attributes, if any
    StrokeInfo   *info;
    /// Link to this stroke's metrics, if any
    Metrics      *metrics;
    /// List of points
    QList<Point>  points;
    /// Link to this stroke's transformation, if any
    QTransform   *transform;
  };



  /**
   * Conversion unit for HiMetric -> pixels
   */
  const qreal HiMetricToPixel = 26.4572454037811;



}



#endif
