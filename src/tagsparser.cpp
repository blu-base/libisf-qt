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

#include "tagsparser.h"

#include "data/compression.h"
#include "data/datasource.h"
#include "data/multibytecoding.h"

#include "isfqt-internal.h"

#include <IsfQt>
#include <IsfQtDrawing>

#include <QPolygon>
#include <QUuid>


using namespace Isf;
using namespace Isf::Compress;



/**
 * Read away an unsupported tag.
 *
 * @param streamData StreamData object with the stream data
 * @param tagName Name of the tag if known, index number if not
 * @return IsfError
 */
IsfError TagsParser::parseUnsupported( StreamData* streamData, const QString &tagName )
{
  // Unsupported content
  analyzePayload( streamData, tagName + " (Unsupported)" );

  return ISF_ERROR_NONE;
}



/**
 * Read a custom tag.
 *
 * 99.9% of custom tags is currently unknown.
 * As soon as new tags will be identified, this method will recognize them and
 * delegate their parsing to another method in this class.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @param tagIndex Index number of the custom tag
 * @return IsfError
 */
IsfError TagsParser::parseCustomTag( StreamData* streamData, Drawing& drawing, quint64 tagIndex )
{
  quint64 tag = tagIndex - FIRST_CUSTOM_TAG_ID;

  // Find if we have this tag registered in the GUID table
  if( tag >= (quint64)drawing.guids_.count() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Custom tag" << QString::number( tagIndex ) << "was not registered!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }

  QList<qint64> data;
  QUuid         guid        = drawing.guids_.at( tag );
  quint64       payloadSize = decodeUInt( streamData->dataSource ) + 1;

  // Decompress the property data
  DataSource payload( streamData->dataSource->getBytes( payloadSize ) );
  Compress::inflatePropertyData( &payload, payloadSize-1, data );

  // String values
  if(
      guid == "{96E9B229-B657-DA4F-BFFD-F54DBA4C35F9}" // Unknown meaning, name?
  ||  guid == "{7C8E448A-390F-D94C-BB52-71FDA3221674}" // Unknown meaning, surname?
     )
  {
    // TODO: WTF is the first char for? Its value is always "0x08"
    data.takeAt( 0 );

    QString string;

    foreach( quint64 item, data )
    {
      string.append( (quint8)item );
    }

#ifdef ISFQT_DEBUG
    qDebug() << "- String value:" << string;
#endif
  }
  else
  {
#ifdef ISFQT_DEBUG
    qDebug() << "- Unknown property data:" << data;
#endif
  }

/*
 * Other observed custom tags:
 * Tag GUID                               | Encoding           | Property data sample
 * {634BF9C9-E4DC-4842-995D-5AA84CDB887F} | Bit Packing Byte   | (3, 0, 27, 253, 255, 255)
 * {43D8C905-2CC8-C34A-BCDA-2E74E78721AD} | Bit Packing Byte   | (3, 132, 0)
 */

  return ISF_ERROR_NONE;
}



/**
 * Read the table of custom GUIDs.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseGuidTable( StreamData* streamData, Drawing& drawing )
{
  if( ! drawing.guids_.isEmpty() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Duplicated TAG_GUID_TABLE!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }

  quint64 guidTableSize = decodeUInt( streamData->dataSource );

  // GUIDs are 16 bytes long
  quint8 numGuids = guidTableSize / 16;
  // Maximum GUID present in the file
  drawing.maxGuid_ = 99 + numGuids;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- New maximum GUID index:" << drawing.maxGuid_;
  qDebug() << "- GUID table has" << numGuids << "entries for total" << guidTableSize << "bytes:";
#endif

  bool ok = true;
  quint8 index = 0;

  while( ! streamData->dataSource->atEnd() && index < numGuids )
  {
    int block1;
    short block2;
    short block3;
    char  block4[ 8 ];

    block1 = streamData->dataSource->getBytes( 4, &ok ).toHex().toUInt( &ok, 16 );
    if( ! ok )
    {
      break;
    }

    block2 = streamData->dataSource->getBytes( 2, &ok ).toHex().toUShort( &ok, 16 );
    if( ! ok )
    {
      break;
    }

     block3 = streamData->dataSource->getBytes( 2, &ok ).toHex().toUShort( &ok, 16 );
    if( ! ok )
    {
      break;
    }

    for( int i = 0; ok && i < 8; ++i )
    {
      block4[ i ] = streamData->dataSource->getBytes( 1, &ok ).at( 0 );
    }
    if( ! ok )
    {
      break;
    }

    QUuid guid( block1, block2, block3,
                block4[0], block4[1], block4[2], block4[3],
                block4[4], block4[5], block4[6], block4[7] );

    drawing.guids_.append( guid );

    if( index != ( drawing.guids_.count() - 1 ) )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Tag/index mismatch!";
#endif
      return ISF_ERROR_INTERNAL;
    }

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "  - Index" << ( index + FIRST_CUSTOM_TAG_ID )
             << "->"        << QString( guid ).toUpper();
#endif

    ++index;
  }

  if( ! ok )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid GUID table!";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  return ISF_ERROR_NONE;
}



/**
 * Read the Persistent Format data.
 *
 * This tag probably contains a version number or ID; it's made of a tag
 * and a payload.
 * The only observed value so far is 65536, multibyte-encoded as 0x808004,
 * with a payload size of 3 bytes.
 *
 * @see ISF_PERSISTENT_FORMAT_VERSION
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parsePersistentFormat( StreamData* streamData, Drawing& drawing )
{
  Q_UNUSED( drawing )

  quint64 payloadSize    = decodeUInt( streamData->dataSource );
  quint64 sourcePosition = streamData->dataSource->pos();

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_PERSISTENT_FORMAT";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  // Try reading one multibyte value
  quint64 version = decodeUInt( streamData->dataSource );

  // If we read some unexpected contents, dump them to help improving the library
  quint32 actualPayloadSize = streamData->dataSource->pos() - sourcePosition;
  if( actualPayloadSize != payloadSize )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload contents for TAG_PERSISTENT_FORMAT!"
             << "Read" << actualPayloadSize << "bytes instead of" << payloadSize;
    streamData->dataSource->seekRelative( - actualPayloadSize );
    analyzePayload( streamData, payloadSize, "TAG_PERSISTENT_FORMAT unknown contents" );
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



/**
 * Read the drawing dimensions.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseHiMetricSize( StreamData* streamData, Drawing& drawing )
{
  quint64 payloadSize = decodeUInt( streamData->dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_HIMETRIC_SIZE:" << payloadSize;
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  if( streamData->boundingRect.isValid() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Duplicated TAG_HIMETRIC_SIZE!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }


  streamData->boundingRect = QRect( 0, 0, 0, 0 );
  streamData->boundingRect.setWidth ( decodeInt( streamData->dataSource ) );
  streamData->boundingRect.setHeight( decodeInt( streamData->dataSource ) );


#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Drawing dimensions:" << drawing.size();
#endif

  return ISF_ERROR_NONE;
}



/**
 * Read a block of points attributes.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseAttributeBlock( StreamData* streamData, Drawing& drawing )
{
  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_DRAW_ATTRS_BLOCK";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  AttributeSet set;

  // set this once when we get the first DRAW_ATTRS_BLOCK. then,
  // everytime we get a DIDX we can update it. if we don't do this
  // then the first stroke will have the same colour as the last stroke.
//   if ( drawing.currentAttributeSetIndex == &drawing.defaultAttributeSet_ )
//   {
//     streamData->currentAttributeSetIndex = streamData->attributeSets.indexOf( set );
//   }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added drawing attribute block #" << ( streamData->attributeSets.count() - 1 );
#endif

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    // Read the property and its value
    quint64 property = decodeUInt( dataSource ); // contains a PacketProperty value
    quint64 value    = decodeUInt( dataSource );

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
        set.color = QColor( qBlue ( invertedColor ),
                            qGreen( invertedColor ),
                            qRed  ( invertedColor ) );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen color" << set.color.name().toUpper();
#endif
        break;
      }

      case GUID_PEN_WIDTH:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen width" << QString::number( (float)value, 'g', 16 )
                 << "(" << (value/HiMetricToPixel) << "pixels )";
#endif
        // In square/round pens the width will be the only value present.
        set.penSize.setWidth ( (float)value / HiMetricToPixel );
        set.penSize.setHeight( (float)value / HiMetricToPixel );
        break;

      case GUID_PEN_HEIGHT:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen height" << QString::number( (float)value, 'g', 16 );
#endif
        set.penSize.setHeight( (float)value / HiMetricToPixel );
        break;

      case GUID_PEN_TIP:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Got pen shape: is rectangular?" << (bool)value << "full value:" << value;
#endif
        if( value )
        {
          set.flags |= IsRectangle;
        }
        break;

      case GUID_DRAWING_FLAGS:
        set.flags = (StrokeFlags)( ( 0XFF00 & set.flags ) | (ushort) value );
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
        set.color.setAlpha( value );
        break;

      case GUID_ROP:
#ifdef ISFQT_DEBUG
        qDebug() << "- Got ROP property (Run Of Press? Reject Occasional Partners?)";
        // We already read the first value, go back
//         source.seekRelative( getMultiByteSize( value ) );
        analyzePayload( streamData, 3 );
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
          analyzePayload( streamData, "TAG_PROPERTY_" + QString::number( property ) );
        }
        else
        {
          analyzePayload( streamData, "Unknown property " + QString::number( property ) );
        }
        break;
    }
  }

  // Update the drawing's maximum pen size.
  // This value is used to adjust the drawing borders to include thick strokes
  if( set.penSize.width() > drawing.maxPenSize_.width() )
  {
    drawing.maxPenSize_.setWidth( set.penSize.width() );
  }
  if( set.penSize.height() > drawing.maxPenSize_.height() )
  {
    drawing.maxPenSize_.setHeight( set.penSize.height() );
  }

  streamData->attributeSets.append( set );

  return ISF_ERROR_NONE;
}



/**
 * Read a table of points attributes.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseAttributeTable( StreamData* streamData, Drawing& drawing )
{
  IsfError result = ISF_ERROR_NONE;
  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_DRAW_ATTRS_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( result == ISF_ERROR_NONE && dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
    result = parseAttributeBlock( streamData, drawing );
  }

  return result;
}



/**
 * Read the ink canvas dimensions.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseInkSpaceRectangle( StreamData* streamData, Drawing& drawing )
{
  if( drawing.canvas_.isValid() )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Duplicated TAG_INK_SPACE_RECT!";
#endif
    return ISF_ERROR_INVALID_STREAM;
  }

  // This tag has a fixed 4-byte size
  DataSource* dataSource = streamData->dataSource;
  drawing.canvas_.setLeft  ( decodeInt( dataSource ) );
  drawing.canvas_.setTop   ( decodeInt( dataSource ) );
  drawing.canvas_.setRight ( decodeInt( dataSource ) );
  drawing.canvas_.setBottom( decodeInt( dataSource ) );

#ifdef ISFQT_DEBUG
  qDebug() << "- Got drawing canvas:" << drawing.canvas_;
#endif

  return ISF_ERROR_NONE;
}



/**
 * Read a Metrics table.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseMetricTable( StreamData* streamData, Drawing& drawing )
{
  IsfError result = ISF_ERROR_NONE;
  DataSource* dataSource = streamData->dataSource;

  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_METRIC_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( result == ISF_ERROR_NONE && dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    result = parseMetricBlock( streamData, drawing );
  }

  return result;
}



/**
 * Read a Metrics block.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseMetricBlock( StreamData* streamData, Drawing& drawing )
{
  Q_UNUSED( drawing )

  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_METRIC_BLOCK";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  Metrics *metricsList = new Metrics();
  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    quint64 property = Isf::Compress::decodeUInt( dataSource );

    qint64 initialPos = dataSource->pos();
    payloadSize = Isf::Compress::decodeUInt( dataSource );

    // Two multibyte signed ints, one byte, one float: minimum 7 bytes
    if( payloadSize < 7 )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Metric id" << property << "has an invalid payload size:" << payloadSize;
#endif
      analyzePayload( streamData, payloadSize );
      continue;
    }

    // Get the metric values
    Metric metric;
    metric.min        = Isf::Compress::decodeInt( dataSource );
    metric.max        = Isf::Compress::decodeInt( dataSource );
    metric.units      = (MetricScale) dataSource->getByte();
    metric.resolution = Isf::Compress::decodeFloat( dataSource );

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
        analyzePayload( streamData, payloadSize );
        continue;
    }

#ifdef ISFQT_DEBUG
    // If there's extra data, show it
    quint64 parsedPayload = ( dataSource->pos() - initialPos );
    if( parsedPayload < payloadSize )
    {
      qDebug() << "- Extra data:";
      analyzePayload( streamData, payloadSize - parsedPayload );
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
  streamData->metrics.append( metricsList );

  // set this once when we get the first METRIC_BLOCK. then,
  // everytime we get a MIDX we can update it. if we don't do this
  // then the first stroke will have the same metrics as the last stroke.
//   Metrics *savedMetrics = streamData->metrics.last();
//   if ( drawing.currentMetrics_ == &drawing.defaultMetrics_ )
//   {
//     drawing.currentMetrics_ = savedMetrics;
//   }

  return ISF_ERROR_NONE;
}



/**
 * Read a table of transformation matrices.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseTransformationTable( StreamData* streamData, Drawing& drawing )
{
  IsfError result = ISF_ERROR_NONE;
  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_TRANSFORM_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( result == ISF_ERROR_NONE && dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    // Read the type of the next transformation
    DataTag tagIndex = (DataTag) Compress::decodeUInt( dataSource );

    result = parseTransformation( streamData, drawing, tagIndex );
  }

  return result;
}



/**
 * Read a drawing transformation matrix.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @param transformType Type of transform to read
 * @return IsfError
 */
IsfError TagsParser::parseTransformation( StreamData* streamData, Drawing& drawing, quint64 transformType )
{
  QMatrix *transform = new QMatrix();
  DataSource* dataSource = streamData->dataSource;

  /*
   * Unlike the other transformations, scale is expressed in HiMetric units,
   * so we must convert it to pixels for rendering
   *
   * WARNING: The order in which parameters are evaluated is *platform-dependent*. thus,
   *          you cannot read data like so:
   *            transform->setMatrix( Compress::decodeFloat( source ), Compress::decodeFloat( source ), ... )
   *          and expect it to work since you have no guarantee as to which of those
   *          decodeFloat() methods will be called first.
   *          You have to read each transform value the long way - assign to variable, then
   *          set the variable as the parameter.
   */

  switch( transformType )
  {
    case TAG_TRANSFORM:
    {
      float scaleX = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      float shearX = Compress::decodeFloat( dataSource );
      float shearY = Compress::decodeFloat( dataSource );
      float dx     = Compress::decodeFloat( dataSource );
      float dy     = Compress::decodeFloat( dataSource );

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
      float scaleAmount = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      transform->scale( scaleAmount, scaleAmount );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale:" << scaleAmount;
#endif
      break;
    }

    case TAG_TRANSFORM_ANISOTROPIC_SCALE:
    {
      float scaleX = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
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
      float rotateAmount = Compress::decodeFloat( dataSource ) / 100.0f;
      transform->rotate( rotateAmount );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Rotate:" << rotateAmount;
#endif
      break;
    }

    case TAG_TRANSFORM_TRANSLATE:
    {
      float dx = Compress::decodeFloat( dataSource );
      float dy = Compress::decodeFloat( dataSource );
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
      float scaleX = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      float scaleY = Compress::decodeFloat( dataSource ) / HiMetricToPixel;
      float dx = Compress::decodeFloat( dataSource );
      float dy = Compress::decodeFloat( dataSource );

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

  streamData->transforms.append( transform );

  // set this once when we get the first transformation. then,
  // everytime we get a TIDX we can update it. if we don't do this
  // then the first stroke will have the same transform as the last stroke.
//   if ( drawing.currentTransform_ == &drawing.defaultTransform_ )
//   {
//     drawing.currentTransform_ = drawing.transforms_.last();
//   }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added transform block #" << ( streamData->transforms.count() - 1 );
#endif

  return ISF_ERROR_NONE;
}



/**
 * Read a stroke.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseStroke( StreamData* streamData, Drawing& drawing )
{
  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );
  quint64 initialPos = dataSource->pos();

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_STROKE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  // Get the number of points which comprise this stroke
  quint64 numPoints = decodeUInt( dataSource );

  QList<qint64> xPointsData, yPointsData, pressureData;

  // If the stream has pressure info, parse it
  bool streamHasPressureData = streamData->strokeInfos.count() && streamData->strokeInfos.last()->hasPressureData;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Tag size:" << payloadSize << "Points stored:" << numPoints;
#endif

  if( ! Compress::inflatePacketData( dataSource, numPoints, xPointsData ) )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Decompression failure while extracting X points data!";
    return ISF_ERROR_INVALID_PAYLOAD;
#endif
  }

  if( ! Compress::inflatePacketData( dataSource, numPoints, yPointsData ) )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "Decompression failure while extracting Y points data!";
    return ISF_ERROR_INVALID_PAYLOAD;
#endif
  }

  if(   streamHasPressureData
  &&  ! Compress::inflatePacketData( dataSource, numPoints, pressureData ) )
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

  if( streamHasPressureData
  &&  (uint)pressureData.size() != numPoints )
  {
#ifdef ISFQT_DEBUG
    qWarning() << "The pressure data has a size of" << pressureData.size()
              << "which does not match with the advertised size of" << numPoints;
#endif
  }

  // Add a new stroke
  drawing.strokes_.append( new Stroke() );
  Stroke *stroke = drawing.strokes_.last();

  // Apply the stroke style, transforms and metrics
  if( streamData->attributeSets.count() )
  {
    AttributeSet set = streamData->attributeSets.at( streamData->currentAttributeSetIndex );

    stroke->setColor  ( set.color   );
    stroke->setFlags  ( set.flags   );
    stroke->setPenSize( set.penSize );
  }
  if( streamData->metrics.count() )
  {
    Metrics* metrics = streamData->metrics.at( streamData->currentMetricsIndex );

    if( ! metrics )
    {
      qWarning() << "Invalid reference to metrics" << streamData->currentMetricsIndex;
    }
    else
    {
      stroke->setMetrics( metrics );
    }
  }
  if( streamData->transforms.count() )
  {
    QMatrix* transform = streamData->transforms.at( streamData->currentTransformsIndex );

    if( ! transform )
    {
      qWarning() << "Invalid reference to transform" << streamData->currentTransformsIndex;
    }
    else
    {
      stroke->setTransform( transform );
    }
  }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added stroke #" << ( drawing.strokes_.count() - 1 );
#endif

  // Add the points to the stroke
  PointList list;
  for( quint64 i = 0; i < numPoints; ++i )
  {
    list.append( Point() );
    Point &point = list.last();

    point.position.setX( xPointsData[ i ] );
    point.position.setY( yPointsData[ i ] );

    if( streamHasPressureData )
    {
      point.pressureLevel = pressureData[ i ];
    }
  }
  // And finish it out
  stroke->addPoints( list );
  stroke->finalize();

  // Update the entire drawing's bounding rect
  drawing.boundingRect_ = drawing.boundingRect_.united( stroke->boundingRect() );

  qint64 remainingPayloadSize = payloadSize - ( dataSource->pos() - initialPos );
  if( remainingPayloadSize > 0 )
  {
    analyzePayload( streamData,
                    remainingPayloadSize,
                    "Remaining stroke data: " + QString::number( remainingPayloadSize ) +
                    " bytes" );
  }

  return ISF_ERROR_NONE;
}



/**
 * Read a stroke description block.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseStrokeDescBlock( StreamData* streamData, Drawing& drawing )
{
  DataSource* dataSource = streamData->dataSource;
  quint64 payloadSize = decodeUInt( dataSource );

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

  streamData->strokeInfos.append( new StrokeInfo() );
  StrokeInfo *info = streamData->strokeInfos.last();

  // set this once when we get the first TAG_STROKE_DESC_BLOCK. then,
  // everytime we get a SIDX we can update it. if we don't do this
  // then the first stroke will have the same stroke info as the last stroke.
//   if ( drawing.currentStrokeInfo_ == &drawing.defaultStrokeInfo_ )
//   {
//     drawing.currentStrokeInfo_ = info;
//   }

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added stroke info block #" << ( streamData->strokeInfos.count() - 1 );
#endif

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    quint64 tag = decodeUInt( dataSource );

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



/**
 * Read a stroke description table.
 *
 * @param streamData StreamData object with the stream data
 * @param drawing Drawing into which the data obtained from the tag should be read
 * @return IsfError
 */
IsfError TagsParser::parseStrokeDescTable( StreamData* streamData, Drawing& drawing )
{
  IsfError result = ISF_ERROR_NONE;
  DataSource* dataSource = streamData->dataSource;

  quint64 payloadSize = decodeUInt( dataSource );

  if( payloadSize == 0 )
  {
#ifdef ISFQT_DEBUG
    qDebug() << "Invalid payload for TAG_STROKE_DESC_TABLE";
#endif
    return ISF_ERROR_INVALID_PAYLOAD;
  }

  qint64 payloadEnd = dataSource->pos() + payloadSize;
  while( result == ISF_ERROR_NONE && dataSource->pos() < payloadEnd && ! dataSource->atEnd() )
  {
    result = parseStrokeDescBlock( streamData, drawing );
  }

  return result;
}



/**
 * Print the payload of an unknown tag.
 *
 * This method can be used to decode tags which are formed by:
 * - a TAG byte
 * - a payload size (multibyte-encoded)
 * - the actual tag data
 *
 * @param streamData StreamData object with the stream data
 * @param tagName Name of the tag if known, index number if not
 * @return Byte array with the contents of the tag
 */
QByteArray TagsParser::analyzePayload( StreamData* streamData, const QString &tagName )
{
  quint64 payloadSize = decodeUInt( streamData->dataSource );

  return analyzePayload( streamData,
                         payloadSize,
                         "Got tag: " + tagName + " with " + QString::number( payloadSize ) + " bytes of payload" );
}



/**
 * Print the payload of an unknown tag.
 *
 * This variant can be used to print out an arbitrarily sized data block.
 *
 * @param streamData StreamData object with the stream data
 * @param payloadSize Size of the payload to print out
 * @param message Message to show as label for the printed out data
 * @return Byte array with the contents of the tag
 */
QByteArray TagsParser::analyzePayload( StreamData* streamData, const quint64 payloadSize, const QString &message )
{
  QByteArray result;
  QByteArray output;

  if( payloadSize == 0 )
  {
    return output;
  }

#ifndef ISFQT_DEBUG_VERBOSE
  Q_UNUSED( message )
  streamData->dataSource->seekRelative( +payloadSize );
  return output;
#endif

  quint64 pos = 0;

  if( ! message.isEmpty() )
  {
    qDebug() << message;
  }
  qDebug() << "--------------------------------------------------------------------";
  while( ! streamData->dataSource->atEnd() && pos < payloadSize )
  {
    quint8 byte = streamData->dataSource->getByte();
    result.append( byte );
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

  return result;
}


