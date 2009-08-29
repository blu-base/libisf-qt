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

#include "tagsparser.h"

#include "isfqt-internal.h"

#include "data/compression.h"
#include "data/datasource.h"
#include "data/multibytecoding.h"

#include <IsfQtDrawing>

#include <QPolygon>


using namespace Isf;
using namespace Isf::Compress;



/// Read away an unsupported tag
IsfError TagsParser::parseUnsupported( DataSource &source, const QString &tagName )
{
  // Unsupported content
  analyzePayload( source, tagName + " (Unsupported)" );

  return ISF_ERROR_NONE;
}



/// Read away an unsupported custom tag
IsfError TagsParser::parseCustomTag( DataSource &source, const QString &tagName )
{
  // Unsupported custom tag. The payload size does not include the algorithm byte,
  // so we need to analyze it too

  quint64 payloadSize = decodeUInt( source ) + 1;

  analyzePayload( source,
                  payloadSize,
                  "Got unsupported custom tag: " + tagName +
                  " with " + QString::number( payloadSize ) + " bytes of payload" );

  return ISF_ERROR_NONE;
}



/// Read the table of GUIDs from the data
IsfError TagsParser::parseGuidTable( DataSource &source, Drawing &drawing )
{
  quint64 guidTableSize = decodeUInt( source );

  // GUIDs are 16 bytes long
  quint8 numGuids = guidTableSize / 16;
  // Maximum GUID present in the file
  drawing.maxGuid_ = 99 + numGuids;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- New maximum GUID index:" << drawing.maxGuid_;
  qDebug() << "- GUID table has" << numGuids << "entries for total" << guidTableSize << "bytes:";
#endif

  quint8 index = 0;

  while( ! source.atEnd() && index < numGuids )
  {
    // 100 is the first index available for custom GUIDs
    quint8 guidIndex = index + 100;

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "  - Index" << QString::number( guidIndex ).rightJustified( 5 , ' ' )
            << "->" << source.getBytes( 16 ).toHex();
#else
    Q_UNUSED( guidIndex )
#endif

    ++index;
  }

  return ISF_ERROR_NONE;
}



/// Read the Persistent Format data
IsfError TagsParser::parsePersistentFormat( DataSource &source, Drawing &drawing )
{
  Q_UNUSED( drawing )

  // This tag probably contains a version number or ID; it's made of a tag
  // and a payload.
  // The only observed value so far is 65536, multibyte-encoded as 0x808004,
  // with a payload size of 3 bytes.

  quint64 payloadSize    = decodeUInt( source );
  quint64 sourcePosition = source.pos();

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_PERSISTENT_FORMAT";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  // Try reading one multibyte value
  quint64 version = decodeUInt( source );

  // If we read some unexpected contents, dump them to help improving the library
  quint32 actualPayloadSize = source.pos() - sourcePosition;
  if( actualPayloadSize != payloadSize )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload contents for TAG_PERSISTENT_FORMAT!"
             << "Read" << actualPayloadSize << "bytes instead of" << payloadSize;
    source.seekRelative( - actualPayloadSize );
    analyzePayload( source, payloadSize, "TAG_PERSISTENT_FORMAT unknown contents" );
#endif
  }

  // Check the version, if it really is a version :)
  if( version != ISF_PERSISTENT_FORMAT_VERSION )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "- Invalid Persistent Format version" << version;
#endif
    return ISF_ERROR_BAD_VERSION;
  }
#ifdef ISFQT_DEBUG
  else
  {
    qDebug() << "- Persistent Format version ok.";
  }
#endif

  return ISF_ERROR_NONE;
}



/// Read the drawing dimensions
IsfError TagsParser::parseHiMetricSize( DataSource &source, Drawing &drawing )
{
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_HIMETRIC_SIZE:" << payloadSize;
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  if( drawing.size_.isValid() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Duplicated TAG_HIMETRIC_SIZE!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }

  drawing.size_.setWidth ( decodeInt( source ) );
  drawing.size_.setHeight( decodeInt( source ) );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Drawing dimensions:" << drawing.size_;
#endif

  return ISF_ERROR_NONE;
}



/// Read a block of points attributes
IsfError TagsParser::parseAttributeBlock( DataSource &source, Drawing &drawing )
{
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_DRAW_ATTRS_BLOCK";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  drawing.attributeSets_.append( new AttributeSet() );
  AttributeSet *set = drawing.attributeSets_.last();

  // set this once when we get the first DRAW_ATTRS_BLOCK. then,
  // everytime we get a DIDX we can update it. if we don't do this
  // then the first stroke will have the same colour as the last stroke.
  if ( drawing.currentAttributeSet_ == &drawing.defaultAttributeSet_ )
  {
    drawing.currentAttributeSet_ = set;
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added drawing attribute block #" << ( drawing.attributeSets_.count() - 1 );
#endif

  qint64 payloadEnd = source.pos() + payloadSize;
  while( source.pos() < payloadEnd && ! source.atEnd() )
  {
    // Read the property and its value
    quint64 property = decodeUInt( source ); // contains a PacketProperty value
    quint64 value    = decodeUInt( source );

    switch( property )
    {
      case GUID_PEN_STYLE:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got style" << value << "- Unable to handle it, skipping.";
#endif
        break;

      case GUID_COLORREF:
      {
        QRgb invertedColor = value & 0xFFFFFF;
        // The color value is stored in BGR order, so we need to read it back inverted,
        // as QRgb stores the value in BGR order: QRgb(RRGGBB) <-- value(BBGGRR).
        // TODO: It also contains an alpha value, ignored here for now because it's unknown if
        // it is needed or not
        set->color = QColor( qBlue ( invertedColor ),
                             qGreen( invertedColor ),
                             qRed  ( invertedColor ) );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen color" << set->color.name().toUpper();
#endif
        break;
      }

      case GUID_PEN_WIDTH:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen width" << QString::number( (float)value, 'g', 16 )
                 << "(" << (value/HiMetricToPixel) << "pixels )";
#endif
        // In square/round pens the width will be the only value present.
        set->penSize.setWidth ( (float)value / HiMetricToPixel );
        set->penSize.setHeight( (float)value / HiMetricToPixel );
        break;

      case GUID_PEN_HEIGHT:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen height" << QString::number( (float)value, 'g', 16 );
#endif
        set->penSize.setHeight( (float)value / HiMetricToPixel );
        break;

      case GUID_PEN_TIP:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen shape: is rectangular?" << (bool)value << "full value:" << value;
#endif
        if( value )
        {
          set->flags |= IsRectangle;
        }
        break;

      case GUID_DRAWING_FLAGS:
        set->flags = (StrokeFlags)( ( 0XFF00 & set->flags ) | (ushort) value );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got flags value:" << value;
        if( value & FitToCurve )
        {
          qDebug() << "  - Got FitToCurve pen flag";
          value ^= FitToCurve;
        }
        if( value & IgnorePressure )
        {
          qDebug() << "  - Got IgnorePressure pen flag";
          value ^= IgnorePressure;
        }
        if( value & IsHighlighter )
        {
          qDebug() << "  - Got IsHighlighter pen flag";
          value ^= IsHighlighter;
        }
        if( value & IsRectangle )
        {
          qDebug() << "  - Got IsRectangle pen flag";
          value ^= IsRectangle;
        }
        qDebug() << "  - Remaining unknown flags value:" << value;
#endif
        break;

      case GUID_TRANSPARENCY:
        value = ( (uchar)value ) << 24;
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen transparency" << value;
#endif
        set->color.setAlpha( value );
        break;

      case GUID_ROP:
#ifdef ISFQT_DEBUG
        qDebug() << "- Got ROP property (Run Of Press? Reject Occasional Partners?)";
        // We already read the first value, go back
//         source.seekRelative( getMultiByteSize( value ) );
        analyzePayload( source, 3 );
#endif
//         info.flags |= IsHighlighter;
        break;

      default:
#ifdef ISFQT_DEBUG
        qDebug() << "- Unknown property:" << property;
#endif

        // If the tag *should* be known, record it differently
        if( drawing.maxGuid_ > 0 && property >= 100 && property <= drawing.maxGuid_ )
        {
          analyzePayload( source, "TAG_PROPERTY_" + QString::number( property ) );
        }
        else
        {
          analyzePayload( source, "Unknown property " + QString::number( property ) );
        }
        break;
    }
  }

  // Update the drawing's maximum pen size.
  // This value is used to adjust the drawing borders to include thick strokes
  if( set->penSize.width() > drawing.maxPenSize_.width() )
  {
    drawing.maxPenSize_.setWidth( set->penSize.width() );
  }
  if( set->penSize.height() > drawing.maxPenSize_.height() )
  {
    drawing.maxPenSize_.setHeight( set->penSize.height() );
  }


  return ISF_ERROR_NONE;
}



/// Read a table of points attributes
IsfError TagsParser::parseAttributeTable( DataSource &source, Drawing &drawing )
{
  IsfError result = ISF_ERROR_NONE;
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_DRAW_ATTRS_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = source.pos() + payloadSize;
  while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
  {
#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
    result = parseAttributeBlock( source, drawing );
  }

  return result;
}



/// Read the ink canvas dimensions
IsfError TagsParser::parseInkSpaceRectangle( DataSource &source, Drawing &drawing )
{
  if( drawing.canvas_.isValid() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Duplicated TAG_INK_SPACE_RECT!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }

  // This tag has a fixed 4-byte size
  drawing.canvas_.setLeft  ( decodeInt( source ) );
  drawing.canvas_.setTop   ( decodeInt( source ) );
  drawing.canvas_.setRight ( decodeInt( source ) );
  drawing.canvas_.setBottom( decodeInt( source ) );

#ifdef ISFQT_DEBUG
  qDebug() << "- Got drawing canvas:" << drawing.canvas_;
#endif

  return ISF_ERROR_NONE;
}



/// Read payload: Metric Table
IsfError TagsParser::parseMetricTable( DataSource &source, Drawing &drawing )
{
  IsfError result = ISF_ERROR_NONE;
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_METRIC_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = source.pos() + payloadSize;
  while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
  {
    result = parseMetricBlock( source, drawing );
  }

  return result;
}



/// Read payload: Metric Block
IsfError TagsParser::parseMetricBlock( DataSource &source, Drawing &drawing )
{
  Q_UNUSED( drawing )

  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_METRIC_BLOCK";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  Metrics *metricsList = new Metrics();
  qint64 payloadEnd = source.pos() + payloadSize;
  while( source.pos() < payloadEnd && ! source.atEnd() )
  {
    quint64 property = Isf::Compress::decodeUInt( source );

    qint64 initialPos = source.pos();
    payloadSize = Isf::Compress::decodeUInt( source );

    // Two multibyte signed ints, one byte, one float: minimum 7 bytes
    if( payloadSize < 7 )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Metric id" << property << "has an invalid payload size:" << payloadSize;
#endif
      analyzePayload( source, payloadSize );
      continue;
    }

    // Get the metric values
    Metric metric;
    metric.min        = Isf::Compress::decodeInt( source );
    metric.max        = Isf::Compress::decodeInt( source );
    metric.units      = (MetricScale) source.getByte();
    metric.resolution = Isf::Compress::decodeFloat( source );

    // Identify the metric
    switch( property )
    {
      case GUID_X:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- X resolution";
#endif
        break;

      case GUID_Y:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Y resolution";
#endif
        break;

      case GUID_NORMAL_PRESSURE:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Pressure";
#endif
        break;

      default:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Unknown metric, id:" << property << "size:" << payloadSize;
#endif
        analyzePayload( source, payloadSize );
        continue;
    }

#ifdef ISFQT_DEBUG
    // If there's extra data, show it
    quint64 parsedPayload = ( source.pos() - initialPos );
    if( parsedPayload < payloadSize )
    {
      qDebug() << "- Extra data:";
      analyzePayload( source, payloadSize - parsedPayload );
    }
#endif

#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - min:" << metric.min << " max:" << metric.max
                 << " resolution:" << metric.resolution
                 << " units:" << metric.units;
#endif

    metricsList->items[ property ] = metric;
  }

  // Save the obtained metrics
  drawing.metrics_.append( metricsList );
  Metrics *savedMetrics = drawing.metrics_.last();

  // set this once when we get the first METRIC_BLOCK. then,
  // everytime we get a MIDX we can update it. if we don't do this
  // then the first stroke will have the same metrics as the last stroke.
  if ( drawing.currentMetrics_ == &drawing.defaultMetrics_ )
  {
    drawing.currentMetrics_ = savedMetrics;
  }

  return ISF_ERROR_NONE;
}



/// Read a table of transformation matrices
IsfError TagsParser::parseTransformationTable( DataSource &source, Drawing &drawing )
{
  IsfError result = ISF_ERROR_NONE;
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_TRANSFORM_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = source.pos() + payloadSize;
  while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
  {
    // Read the type of the next transformation
    DataTag tagIndex = (DataTag) Compress::decodeUInt( source );

    result = parseTransformation( source, drawing, tagIndex );
  }

  return result;
}



/// Read a drawing transformation matrix
IsfError TagsParser::parseTransformation( DataSource &source, Drawing &drawing, quint64 transformType )
{
  QMatrix *transform = new QMatrix();

  // Unlike the other transformations, scale is expressed in HiMetric units,
  // so we must convert it to pixels for rendering

  // WARNING: The order in which parameters are evaluated is *platform-dependent*. thus, you cannot read data like so:
  //
  // transform->setMatrix ( Compress::decodeFloat( source ), Compress::decodeFloat( source ), ... )
  //
  // and expect it to work since you have no guarantee as to which of those decodeFloat() methods will be called first.
  //
  // you have to read each transform value the long way - assign to variable, then set the variable as the parameter.
  switch( transformType )
  {
    case TAG_TRANSFORM:
    {
      float scaleX = Compress::decodeFloat( source ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( source ) / HiMetricToPixel;
      float shearX = Compress::decodeFloat( source );
      float shearY = Compress::decodeFloat( source );
      float dx     = Compress::decodeFloat( source );
      float dy     = Compress::decodeFloat( source );

      transform->setMatrix( scaleX, scaleY, shearX, shearY, dx, dy );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale X:" << transform->m11()
               << "Shear X:" << transform->m21()
               << "Scale Y:" << transform->m22()
               << "Shear Y:" << transform->m12()
               << "Translate X:" << transform->dx()
               << "Translate Y:" << transform->dy();
#endif
      break;
    }

    case TAG_TRANSFORM_ISOTROPIC_SCALE:
    {
      float scaleAmount = Compress::decodeFloat( source ) / HiMetricToPixel;
      transform->scale( scaleAmount, scaleAmount );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale:" << scaleAmount;
#endif
      break;
    }

    case TAG_TRANSFORM_ANISOTROPIC_SCALE:
    {
      float scaleX = Compress::decodeFloat( source ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( source ) / HiMetricToPixel;
      transform->scale( scaleX, scaleY );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale X:" << transform->m11()
               << "Scale Y:" << transform->m22();
#endif
      break;
    }

    case TAG_TRANSFORM_ROTATE:
    {
      float rotateAmount = Compress::decodeFloat( source ) / 100.0f;
      transform->rotate( rotateAmount );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Rotate:" << rotateAmount;
#endif
      break;
    }

    case TAG_TRANSFORM_TRANSLATE:
    {
      float dx = Compress::decodeFloat( source );
      float dy = Compress::decodeFloat( source );
      transform->translate( dx, dy );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
              << "Translate X:" << transform->dx()
              << "Translate Y:" << transform->dy();
#endif
      break;
    }

    case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
    {
      float scaleX = Compress::decodeFloat( source ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( source ) / HiMetricToPixel;
      float dx = Compress::decodeFloat( source );
      float dy = Compress::decodeFloat( source );

      transform->scale    ( scaleX, scaleY );
      transform->translate( dx, dy );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale X:" << transform->m11()
               << "Scale Y:" << transform->m22()
               << "Translate X:" << transform->dx()
               << "Translate Y:" << transform->dy();
#endif
      break;
    }

    default:
#ifdef ISFQT_DEBUG
      qDebug() << "Got unknown transformation type:" << transformType;
#endif
      return ISF_ERROR_INVALID_BLOCK;
  }

  drawing.transforms_.append( transform );

  // set this once when we get the first transformation. then,
  // everytime we get a TIDX we can update it. if we don't do this
  // then the first stroke will have the same transform as the last stroke.
  if ( drawing.currentTransform_ == &drawing.defaultTransform_ )
  {
    drawing.currentTransform_ = drawing.transforms_.last();
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added transform block #" << ( drawing.transforms_.count() - 1 );
#endif

  return ISF_ERROR_NONE;
}



/// Read a stroke
IsfError TagsParser::parseStroke( DataSource &source, Drawing &drawing )
{
  quint64 payloadSize = decodeUInt( source );
  quint64 initialPos = source.pos();

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_STROKE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  // Get the number of points which comprise this stroke
  quint64 numPoints = decodeUInt( source );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Tag size:" << payloadSize << "Points stored:" << numPoints;
#endif

  QList<qint64> xPointsData, yPointsData, pressureData;
  if( ! inflate( source, numPoints, xPointsData ) )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Decompression failure while extracting X points data!";
    return ISF_ERROR_INVALID_PAYLOAD;
#endif
  }

  if( ! inflate( source, numPoints, yPointsData ) )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Decompression failure while extracting Y points data!";
    return ISF_ERROR_INVALID_PAYLOAD;
#endif
  }

  if(   drawing.currentStrokeInfo_->hasPressureData
  &&  ! inflate( source, numPoints, pressureData ) )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Decompression failure while extracting pressure data!";
    return ISF_ERROR_INVALID_PAYLOAD;
#endif
  }

  if( (uint)xPointsData.size() != numPoints || (uint)yPointsData.size() != numPoints )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "The points arrays have sizes x=" << xPointsData.size() << "y=" << yPointsData.size()
              << "which do not match with the advertised size of" << numPoints;
#endif
  }

  if( drawing.currentStrokeInfo_->hasPressureData
  &&  (uint)pressureData.size() != numPoints )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "The pressure data has a size of" << pressureData.size()
              << "which does not match with the advertised size of" << numPoints;
#endif
  }

  // Add a new stroke
  drawing.strokes_.append( new Stroke() );
  Stroke *stroke = drawing.strokes_[ drawing.strokes_.size() - 1 ];

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added stroke #" << ( drawing.strokes_.count() - 1 );
#endif

  // The polygon is used to obtain the stroke's bounding rect
  QPolygon polygon( numPoints );
  for( quint64 i = 0; i < numPoints; ++i )
  {
    stroke->points.append( Point() );
    Point &point = stroke->points[ stroke->points.size() - 1 ];

    point.position.setX( xPointsData[ i ] );
    point.position.setY( yPointsData[ i ] );

    polygon.setPoint( i, xPointsData[ i ], yPointsData[ i ] );

    if( drawing.currentStrokeInfo_->hasPressureData )
    {
      point.pressureLevel = pressureData[ i ];
    }
  }

  if ( drawing.currentTransform_ != 0 )
  {
    polygon = drawing.currentTransform_->map( polygon );
  }

  stroke->boundingRect = polygon.boundingRect();

  // set the bounding rectangle.
  if ( stroke->boundingRect.size() == QSize(1, 1) )
  {
    // can't have a 1px by 1px bounding rect - the eraser will never hit it.
    // make the bounding rectange completely cover the drawn stroke.
    float penSize = drawing.currentAttributeSet_->penSize.width();
    stroke->boundingRect.setSize( QSize( penSize, penSize ) );
    stroke->boundingRect.translate( -( penSize / 2 ), -( penSize / 2 ) );
  }

  stroke->attributes   = drawing.currentAttributeSet_;
  stroke->info         = drawing.currentStrokeInfo_;
  stroke->metrics      = drawing.currentMetrics_;
  stroke->transform    = drawing.currentTransform_;

  // Update the entire drawing's bounding rect
  drawing.boundingRect_ = drawing.boundingRect_.united( stroke->boundingRect );

  qint64 remainingPayloadSize = payloadSize - ( source.pos() - initialPos );
  if( remainingPayloadSize > 0 )
  {
    analyzePayload( source,
                    remainingPayloadSize,
                    "Remaining stroke data: " + QString::number(remainingPayloadSize) +
                    " bytes" );
  }

  return ISF_ERROR_NONE;
}



/// Read a stroke description block
IsfError TagsParser::parseStrokeDescBlock( DataSource &source, Drawing &drawing )
{
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_STROKE_DESC_BLOCK";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Finding stroke description properties in the next" << payloadSize << "bytes";
#endif

  drawing.strokeInfo_.append( new StrokeInfo() );
  StrokeInfo *info = drawing.strokeInfo_.last();

  // set this once when we get the first TAG_STROKE_DESC_BLOCK. then,
  // everytime we get a SIDX we can update it. if we don't do this
  // then the first stroke will have the same stroke info as the last stroke.
  if ( drawing.currentStrokeInfo_ == &drawing.defaultStrokeInfo_ )
  {
    drawing.currentStrokeInfo_ = info;
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added stroke info block #" << ( drawing.strokeInfo_.count() - 1 );
#endif

  qint64 payloadEnd = source.pos() + payloadSize;
  while( source.pos() < payloadEnd && ! source.atEnd() )
  {
    quint64 tag = decodeUInt( source );

    switch( tag )
    {
      case TAG_NO_X:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Stroke contains no X coordinates";
#endif
        info->hasXData = false;
        break;

      case TAG_NO_Y:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Stroke contains no Y coordinates";
#endif
        info->hasYData = false;
        break;

      case TAG_BUTTONS:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Buttons...";
#endif
        break;

      case TAG_STROKE_PROPERTY_LIST:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Property list...";
#endif
        break;

      default: // List of Stroke packet properties
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Packet properties list:" << QString::number( tag, 10 );
#endif
        info->hasPressureData = true;
        // TODO How is this list made?
/*
        for( quint64 i = 0; i < GUID_NUM; ++i )
        {
          if( tag & ( 1 << i ) )
          {
            qDebug() << "match:" << i;
          }
        }
*/
        break;
    }
  }

  return ISF_ERROR_NONE;
}



/// Read a stroke description table
IsfError TagsParser::parseStrokeDescTable( DataSource &source, Drawing &drawing )
{
  IsfError result = ISF_ERROR_NONE;
  quint64 payloadSize = decodeUInt( source );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_STROKE_DESC_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = source.pos() + payloadSize;
  while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
  {
    result = parseStrokeDescBlock( source, drawing );
  }

  return result;
}



// Print the payload of an unknown tag
void TagsParser::analyzePayload( DataSource &source, const QString &tagName )
{
  quint64 payloadSize = decodeUInt( source );

  analyzePayload( source,
                  payloadSize,
                  "Got tag: " + tagName + " with " + QString::number( payloadSize ) + " bytes of payload" );
}



// Print the payload of an unknown tag
void TagsParser::analyzePayload( DataSource &source, const quint64 payloadSize, const QString &message )
{
  if( payloadSize == 0 )
  {
    return;
  }

#ifdef ISFQT_DEBUG_VERBOSE
  quint64 pos = 0;
  QByteArray output;

  if( ! message.isEmpty() )
  {
    qDebug() << message;
  }
  qDebug() << "--------------------------------------------------------------------";
  while( ! source.atEnd() && pos < payloadSize )
  {
    quint8 byte = source.getByte();
    output.append( QByteArray::number( byte, 16 ).rightJustified( 2, '0').toUpper() + " " );

    if( ( ( pos + 1 ) % 24 ) == 0 )
    {
      qDebug() << output.trimmed();
      output.clear();
    }

    ++pos;
  }

  if( ! output.isEmpty() )
  {
    qDebug() << output.trimmed();
  }

  qDebug() << "--------------------------------------------------------------------";
#else
  Q_UNUSED( message )
  source.seekRelative( +payloadSize );
#endif
}


