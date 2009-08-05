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

#ifndef ISFQT_H
#define ISFQT_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QtGlobal>
#include <QTransform>



namespace Isf
{



  /**
   * Possible errors values returned by the ISF library
   */
  enum IsfError
  {
    ISF_ERROR_NONE = 0           /// No error

    /// Stream reader errors
  , ISF_ERROR_BAD_VERSION        /// Incompatible ISF version
  , ISF_ERROR_BAD_STREAMSIZE     /// Stream size of ISF data is too small
  , ISF_ERROR_INVALID_PAYLOAD    /// A tag's payload was empty
  , ISF_ERROR_INVALID_BLOCK      /// A block had invalid contents
  };



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
