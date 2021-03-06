/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
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
#include <QMatrix>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QtGlobal>



namespace Isf
{
  // Forward declarations
  class Drawing;
  class StreamData;



  /**
   * Possible error return values returned by the ISF library.
   */
  enum IsfError
  {
    ISF_ERROR_NONE = 0           ///< No error

    /// Stream reader errors
  , ISF_ERROR_BAD_VERSION        ///< Incompatible ISF version
  , ISF_ERROR_BAD_STREAMSIZE     ///< Stream size of ISF data is too small
  , ISF_ERROR_INVALID_STREAM     ///< The stream contains wrong or duplicated tags
  , ISF_ERROR_INVALID_PAYLOAD    ///< A tag's payload was empty
  , ISF_ERROR_INVALID_BLOCK      ///< A block had invalid contents

    /// Other errors
  , ISF_ERROR_INTERNAL           ///< An error within the code has occurred
  };



  /**
   * List of predefined packet properties.
   *
   * These are used with strokes and their attributes.
   * The Metrics structure defines how to interpret these properties.
   */
  enum PacketProperty
  {
    KNOWN_GUID_BASE_INDEX          = 50
  , GUID_X                         = KNOWN_GUID_BASE_INDEX + 0
  , GUID_Y                         = KNOWN_GUID_BASE_INDEX + 1
  , GUID_Z                      // = et cetera
  , GUID_PACKET_STATUS
  , GUID_TIMER_TICK
  , GUID_SERIAL_NUMBER
  , GUID_NORMAL_PRESSURE
  , GUID_TANGENT_PRESSURE
  , GUID_BUTTON_PRESSURE
  , GUID_X_TILT_ORIENTATION
  , GUID_Y_TILT_ORIENTATION     // = KNOWN_GUID_BASE_INDEX + 10
  , GUID_AZIMUTH_ORIENTATION
  , GUID_ALTITUDE_ORIENTATION
  , GUID_TWIST_ORIENTATION
  , GUID_PITCH_ROTATION
  , GUID_ROLL_ROTATION
  , GUID_YAW_ROTATION
  , GUID_PEN_STYLE
  , GUID_COLORREF
  , GUID_PEN_WIDTH
  , GUID_PEN_HEIGHT             // = KNOWN_GUID_BASE_INDEX + 20
  , GUID_PEN_TIP
  , GUID_DRAWING_FLAGS
  , GUID_CURSORID
  , GUID_WORD_ALTERNATES
  , GUID_CHAR_ALTERNATES
  , GUID_INKMETRICS
  , GUID_GUIDE_STRUCTURE
  , GUID_TIME_STAMP
  , GUID_LANGUAGE
  , GUID_TRANSPARENCY           // = KNOWN_GUID_BASE_INDEX + 30
  , GUID_CURVE_FITTING_ERROR
  , GUID_RECO_LATTICE
  , GUID_CURSORDOWN
  , GUID_SECONDARYTIPSWITCH
  , GUID_BARRELDOWN
  , GUID_TABLETPICK
  , GUID_ROP                    // = KNOWN_GUID_BASE_INDEX + 37
  , KNOWN_GUID_LAST_INDEX
  };



  /**
   * Available pen tips.
   */
  enum PenTip
  {
    Ball      = 0  ///< The pen tip is a circle
  , Rectangle = 1  ///< The pen tip is a square or a rectangle
  };



  /**
   * Stroke drawing flags.
   */
  enum StrokeFlag
  {
    FitToCurve     = 0x0001  ///< Fit the lines between stroke points to Bezier curves
  , IgnorePressure = 0x0004  ///< Ignore the pressure level for this stroke
  , IsHighlighter  = 0x0100  ///< This stroke is an highlighter stroke
  , IsRectangle    = 0x0200  ///< Meaning unknown
  };
  Q_DECLARE_FLAGS( StrokeFlags, StrokeFlag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( StrokeFlags )



  /**
   * Units used for metric measurements.
   *
   * @see http://msdn.microsoft.com/en-us/library/ms840884.aspx
   */
  enum MetricScale
  {
    UNIT_UNUSED     = -1
  , UNIT_DEFAULT    =  0
  , UNIT_INCH       =  1
  , UNIT_CENTIMETER =  2
  , UNIT_DEGREE     =  3
  , UNIT_RADIAN     =  4
  , UNIT_SECOND     =  5
  , UNIT_POUND      =  6
  , UNIT_GRAM       =  7
  };




  /**
   * A single metric.
   *
   * It is a set of values representing what kind of values some
   * type of measurement will assume.
   */
  struct Metric
  {
    /// Constructor for invalid metrics
    Metric()
    {
    }
    /// Initialized constructor
    Metric( qint64 vMin, qint64 vMax, MetricScale vUnits, float vResolution )
    : min( vMin )
    , max( vMax )
    , units( vUnits )
    , resolution( vResolution )
    {
    }
    /// Quick comparison operator
    bool operator ==( const Metric& other )
    {
      return min        == other.min
          && max        == other.max
          && units      == other.units
          && resolution == other.resolution;
    }

    /// Minimum value
    qint64       min;
    /// Maximum value
    qint64       max;
    /// Measurement unit
    MetricScale  units;
    /// Resolution
    float        resolution;
  };



  /**
   * A table of metrics.
   *
   * The default metric values are hard-coded here, different values read from the stream
   * will override these.
   */
  struct Metrics
  {
    /// Constructor
    Metrics()
    {
      items[ GUID_X                    ] = Metric(     0, 12699, UNIT_CENTIMETER,  1000 );
      items[ GUID_Y                    ] = Metric(     0,  9649, UNIT_CENTIMETER,  1000 );
      items[ GUID_Z                    ] = Metric( -1023,  1023, UNIT_CENTIMETER,  1000 );
      items[ GUID_PACKET_STATUS        ] = Metric(     0,  1023, UNIT_DEFAULT,        1 );
      items[ GUID_TIMER_TICK           ] = Metric(     0,  1023, UNIT_DEFAULT,        1 );
      items[ GUID_SERIAL_NUMBER        ] = Metric(     0,  1023, UNIT_DEFAULT,        1 );
      items[ GUID_NORMAL_PRESSURE      ] = Metric(     0,  3600, UNIT_DEGREE,        10 );
      items[ GUID_TANGENT_PRESSURE     ] = Metric(     0,  3600, UNIT_DEGREE,        10 );
      items[ GUID_BUTTON_PRESSURE      ] = Metric(     0,  3600, UNIT_DEGREE,        10 );
      items[ GUID_X_TILT_ORIENTATION   ] = Metric(  -900,   900, UNIT_DEGREE,        10 );
      items[ GUID_Y_TILT_ORIENTATION   ] = Metric(     0,  3600, UNIT_DEGREE,        10 );
      items[ GUID_AZIMUTH_ORIENTATION  ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
      items[ GUID_ALTITUDE_ORIENTATION ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
      items[ GUID_TWIST_ORIENTATION    ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
      items[ GUID_PITCH_ROTATION       ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
      items[ GUID_ROLL_ROTATION        ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
      items[ GUID_YAW_ROTATION         ] = Metric(    -1,    -1, UNIT_UNUSED,        -1 );
    }

    /// The list of metrics defined in this table
    QMap<int,Metric> items;
  };



  /**
   * A single point within a stroke.
   */
  struct Point
  {
    /// Empty constructor
    Point()
    : position( 0, 0 )
    , pressureLevel( 0 )
    {
    }
    /// Constructor with point only
    Point( const QPoint& point )
    : position( point )
    , pressureLevel( 0 )
    {
    }
    /// Constructor with point and pressure
    Point( const QPoint& point, const qint64 pressure )
    : position( point )
    , pressureLevel( pressure )
    {
    }
    /// Quick comparison operator
    bool operator ==( const Point& other )
    {
      return position      == other.position
          && pressureLevel == other.pressureLevel;
    }

    /// coordinates
    QPoint      position;
    /// Pressure information
    qint64      pressureLevel;
  };



  /**
   * A list of Point items.
   */
  typedef QList<Point> PointList;



  /**
   * Conversion factor to convert from HiMetric units to pixels.
   */
  const qreal HiMetricToPixel = 26.4572454037811;



  /**
   * @class Stream
   * @brief The main class of the library.
   *
   * This class contains the only methods you need to transform ISF data
   * into strokes or pictures, and from strokes to ISF data.
   */
  class Stream
  {

    public: // Public static methods

      static Drawing    &reader( const QByteArray&, bool = false );
      static Drawing    &readerGif( const QByteArray&, bool = false );
      static Drawing    &readerPng( const QByteArray&, bool = false );
      static bool        supportsGif();
      static QByteArray  writer( const Drawing&, bool = false );
      static QByteArray  writerGif( const Drawing&, bool = false );
      static QByteArray  writerPng( const Drawing&, bool = false );

    private: // Private static properties
      static StreamData* streamData_;
  };

}



#endif
