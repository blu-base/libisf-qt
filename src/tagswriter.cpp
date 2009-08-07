/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
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

#include "tagswriter.h"

#include "isfqt-internal.h"

#include "data/compression.h"
#include "data/datasource.h"
#include "data/multibytecoding.h"

#include <IsfQtDrawing>


using namespace Isf;
using namespace Isf::Compress;



/// Write the drawing dimensions
IsfError TagsWriter::addHiMetricSize( DataSource &source, const Drawing &drawing )
{
  encodeUInt( source, TAG_HIMETRIC_SIZE );

  // This tag has a fixed size of 2 bytes. Nevertheless, it must have a payload size field. wtf.
  encodeUInt( source, 2 );

  encodeInt( source, drawing.size_.width () );
  encodeInt( source, drawing.size_.height() );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added drawing dimensions:" << drawing.size_;
#endif

  return ISF_ERROR_NONE;
}



/// Write a table of points attributes
IsfError TagsWriter::addAttributeTable( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;
  PointInfo defaultPoint;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.attributes_.count() << "attributes...";
  quint8 counter = 0;
#endif

  foreach( const PointInfo *info, drawing.attributes_ )
  {
    // Add the color to the attribute block
    if( info->color != defaultPoint.color )
    {
      blockData.append( encodeUInt( GUID_COLORREF ) );

      // Prepare the color value, it needs to be stored in BGR format
      quint64 value = (info->color.blue()  << 24)
                    | (info->color.green() << 16)
                    | (info->color.red()   <<  8);
      blockData.append( encodeUInt( value ) );

      // Add the transparency if needed
      if( info->color.alpha() < 255 )
      {
        blockData.append( encodeUInt( GUID_TRANSPARENCY ) );
        blockData.append( encodeUInt( info->color.alpha() ) );
      }
    }

    // Add the pen size
    if( info->penSize != defaultPoint.penSize )
    {
      blockData.append( encodeUInt( GUID_PEN_WIDTH ) );
      blockData.append( encodeUInt( info->penSize.width() ) );

      if( info->penSize.width() != info->penSize.height() )
      {
        blockData.append( encodeUInt( GUID_PEN_HEIGHT ) );
        blockData.append( encodeUInt( info->penSize.height() ) );
      }
    }

    // Add the other drawing flags
    if( info->flags != defaultPoint.flags )
    {
      StrokeFlags flags = info->flags;
      if( flags & IsRectangle )
      {
        blockData.append( encodeUInt( GUID_PEN_TIP ) );
        blockData.append( encodeUInt( 0 ) ); // Value unknown, is the tag enough?
        flags ^= IsRectangle;
      }
/*
      if( flags & IsHighlighter )
      {
        blockData.append( encodeUInt( GUID_ROP ) );
        blockData.append( QByteArray( 4, '\0' ) ); // Value unknown
        flags ^= IsHighlighter;
      }
*/

      // Copy the other flags as they are
      blockData.append( encodeUInt( GUID_DRAWING_FLAGS ) );
      blockData.append( encodeUInt( flags ) );
    }


    blockData.prepend( encodeUInt( blockData.size() ) );
    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added attribute block #" << ++counter;
#endif
  }

  if( drawing.attributes_.count() > 1 )
  {
    tagContents.prepend( encodeUInt( tagContents.size() ) );
    tagContents.prepend( encodeUInt( TAG_DRAW_ATTRS_TABLE ) );
  }
  else
  {
    tagContents.prepend( encodeUInt( TAG_DRAW_ATTRS_BLOCK ) );
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}



/// Write a table of transformation matrices
IsfError TagsWriter::addTransformationTable( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;
  quint64 transformTag;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.transforms_.count() << "transformations...";
  quint8 counter = 0;
#endif

  foreach( const QTransform *trans, drawing.transforms_ )
  {
    if( trans->isRotating() )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM_ROTATE";
#endif
      transformTag = TAG_TRANSFORM_ROTATE;
      blockData.append( encodeFloat( trans->m11() * 100.0f ) );
    }
    else
    if( trans->isScaling() && trans->isTranslating() )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
      transformTag = TAG_TRANSFORM_SCALE_AND_TRANSLATE;
      blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      blockData.append( encodeFloat( trans->dx () ) );
      blockData.append( encodeFloat( trans->dy () ) );
    }
    else
    if( ! trans->isScaling() && trans->isTranslating() )
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM_TRANSLATE";
#endif
      transformTag = TAG_TRANSFORM_TRANSLATE;
      blockData.append( encodeFloat( trans->dx() ) );
      blockData.append( encodeFloat( trans->dy() ) );
    }
    else
    if( trans->isScaling() && ! trans->isTranslating() )
    {
      if( trans->m11() == trans->m22() )
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
        transformTag = TAG_TRANSFORM_ISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      }
      else
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
        transformTag = TAG_TRANSFORM_ANISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      }
    }
    else
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM";
#endif
      transformTag = TAG_TRANSFORM;
      blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      blockData.append( encodeFloat( trans->m12() ) );
      blockData.append( encodeFloat( trans->m21() ) );
      blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      blockData.append( encodeFloat( trans->dx () ) );
      blockData.append( encodeFloat( trans->dy () ) );
    }

    blockData.prepend( encodeUInt( transformTag ) );

    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added transform #" << ++counter;
#endif
  }

  if( drawing.attributes_.count() > 1 )
  {
    tagContents.prepend( encodeUInt( tagContents.size() ) );
    tagContents.prepend( encodeUInt( TAG_TRANSFORM_TABLE ) );
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}



/// Write the strokes
IsfError TagsWriter::addStrokes( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.strokes_.count() << "strokes...";
  quint8 counter = 0;
#endif

  // Last set of attibutes applied to a stroke
  Metrics    *currentMetrics    = 0;
  PointInfo  *currentPointInfo  = 0;
  QTransform *currentTransform  = 0;

  foreach( const Stroke *stroke, drawing.strokes_ )
  {
    // There is more than one set of metrics, assign each stroke to its own
    if( drawing.metrics_.count() > 1 )
    {
      // Only write a MIDX if this stroke needs different metrics than the last stroke
      if( currentMetrics != stroke->metrics )
      {
        currentMetrics = stroke->metrics;
        blockData.append( encodeUInt( TAG_MIDX ) );
        blockData.append( encodeUInt( drawing.metrics_.indexOf( stroke->metrics ) ) );
      }
    }
    // There is more than one set of attributes, assign each stroke to its own
    if( drawing.attributes_.count() > 1 )
    {
      // Only write a DIDX if this stroke needs a different attribute set than the last stroke
      if( currentPointInfo != stroke->attributes )
      {
        currentPointInfo = stroke->attributes;
        blockData.append( encodeUInt( TAG_DIDX ) );
        blockData.append( encodeUInt( drawing.attributes_.indexOf( stroke->attributes ) ) );
      }
    }
    // There is more than one set of attributes, assign each stroke to its own
    if( drawing.metrics_.count() > 1 )
    {
      // Only write a TIDX if this stroke needs a different transform than the last stroke
      if( currentTransform != stroke->transform )
      {
        currentTransform = stroke->transform;
        blockData.append( encodeUInt( TAG_TIDX ) );
        blockData.append( encodeUInt( drawing.transforms_.indexOf( stroke->transform ) ) );
      }
    }

    // Write this stroke in the stream
    blockData.append( encodeUInt( stroke->points.count() ) );

    QList<qint64> xPoints, yPoints;
    foreach( const Point &point, stroke->points )
    {
      xPoints.append( point.position.x() );
      yPoints.append( point.position.y() );
    }
qDebug() << "deflating..";
    deflate( blockData, stroke->points.count(), xPoints, Points );
    deflate( blockData, stroke->points.count(), yPoints, Points );

    blockData.prepend( encodeUInt( blockData.size() ) );
    blockData.prepend( encodeUInt( TAG_STROKE ) );

    tagContents.append( blockData );
    blockData.clear();
    source.append( tagContents );

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added stroke #" << ++counter;
#endif
  }
/*
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
  drawing.strokes_.append( Stroke() );
  Stroke &stroke = drawing.strokes_[ drawing.strokes_.size() - 1 ];

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added stroke #" << ( drawing.strokes_.count() - 1 );
#endif

  // The polygon is used to obtain the stroke's bounding rect
  QPolygon polygon( numPoints );

  for( quint64 i = 0; i < numPoints; ++i )
  {
    stroke.points.append( Point() );
    Point &point = stroke.points[ stroke.points.size() - 1 ];

    point.position.setX( xPointsData[ i ] );
    point.position.setY( yPointsData[ i ] );

    polygon.setPoint( i, xPointsData[ i ], yPointsData[ i ] );

    if( drawing.currentStrokeInfo_->hasPressureData )
    {
      point.pressureLevel = pressureData[ i ];
    }
  }

  stroke.boundingRect = polygon.boundingRect();
  stroke.attributes   = drawing.currentPointInfo_;
  stroke.info         = drawing.currentStrokeInfo_;
  stroke.metrics      = drawing.currentMetrics_;
  stroke.transform    = drawing.currentTransform_;

  // Update the entire drawing's bounding rect
  drawing.boundingRect_ = drawing.boundingRect_.united( stroke.boundingRect );

  qint64 remainingPayloadSize = payloadSize - ( source.pos() - initialPos );
  if( remainingPayloadSize > 0 )
  {
    analyzePayload( source,
                    remainingPayloadSize,
                    "Remaining stroke data: " + QString::number(remainingPayloadSize) +
                    " bytes" );
  }
*/
  return ISF_ERROR_NONE;
}



/*
/// Write the table of GUIDs from the data
IsfError TagsWriter::addGuidTable( DataSource &source, const Drawing &drawing )
{
  quint64 guidTableSize = decodeUInt( source );

  // GUIDs are 16 bytes long
  quint8 numGuids = guidTableSize / 16;
  // Maximum GUID present in the file
  drawing.maxGuid_ = 99 + numGuids;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- GUID table has" << numGuids << "entries for total" << guidTableSize << "bytes:";
#endif

  quint8 index = 0;

  while( ! source.atEnd() && index < numGuids )
  {
    // 100 is the first index available for custom GUIDs
    quint8 guidIndex = index + 100;

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "  - Index" << QString::number( guidIndex ).rightJustified( ' ', 5 )
            << "->" << source.getBytes( 16 ).toHex();
#else
    Q_UNUSED( guidIndex )
#endif

    ++index;
  }

  return ISF_ERROR_NONE;
}



/// Write payload: Persistent Format
IsfError TagsWriter::addPersistentFormat( DataSource &source, const Drawing &drawing )
{
  Q_UNUSED( drawing )

  // Unknown content
  analyzePayload( source, "Persistent Format" );

  return ISF_ERROR_NONE;
}



/// Write the ink canvas dimensions
IsfError TagsWriter::addInkSpaceRectangle( DataSource &source, const Drawing &drawing )
{
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



/// Write payload: Metric Table
IsfError TagsWriter::addMetricTable( DataSource &source, const Drawing &drawing )
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



/// Write payload: Metric Block
IsfError TagsWriter::addMetricBlock( DataSource &source, const Drawing &drawing )
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

  // Skip it, its usefulness is lesser than making the parser to actually work :)
  source.seekRelative( payloadSize );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Skipped metrics block";
#endif

  return ISF_ERROR_NONE;
}



/// Write a stroke description block
IsfError TagsWriter::addStrokeDescBlock( DataSource &source, const Drawing &drawing )
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

  drawing.strokeInfo_.append( StrokeInfo() );
  StrokeInfo &info = drawing.strokeInfo_.last();

  // set this once when we get the first TAG_STROKE_DESC_BLOCK. then,
  // everytime we get a SIDX we can update it. if we don't do this
  // then the first stroke will have the same stroke info as the last stroke.
  if ( drawing.currentStrokeInfo_ == &drawing.defaultStrokeInfo_ )
  {
    drawing.currentStrokeInfo_ = &info;
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
        info.hasXData = false;
        break;

      case TAG_NO_Y:
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "- Stroke contains no Y coordinates";
#endif
        info.hasYData = false;
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
        info.hasPressureData = true;
        // TODO How is this list made?
        break;
    }
  }

  return ISF_ERROR_NONE;
}



/// Write a stroke description table
IsfError TagsWriter::addStrokeDescTable( DataSource &source, const Drawing &drawing )
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
*/


