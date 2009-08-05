/***************************************************************************
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

#include "isfdrawing.h"
#include "tags.h"
#include "compression/isfdata.h"
#include "multibytecoding.h"

#include <QPainter>
#include <QPixmap>
#include <QtDebug>



namespace Isf
{



/**
 * Construct a new NULL Drawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
Drawing::Drawing()
: currentMetrics_( 0 )
, currentPointInfo_( 0 )
, currentStrokeInfo_( 0 )
, currentTransform_( 0 )
, hasXData_( true )
, hasYData_( true )
, isNull_( true )
, maxGuid_( 0 )
, parserError_( ISF_ERROR_NONE )
{
}



/**
 * Return True if this instance of Drawing is invalid (NULL), False otherwise.
 *
 * @return True if this is a NULL Drawing, FALSE otherwise.
 */
bool Drawing::isNull() const
{
  return isNull_;
}



/**
 * Return the error from the ISF parser if this is a NULL Drawing.
 *
 * If nothing went wrong, this returns ISF_ERROR_NONE.
 *
 * @return The last ISF parser error.
 */
IsfError Drawing::parserError() const
{
  return parserError_;
}



/**
 * Creates and returns a new Drawing object from some raw ISF input data.
 *
 * If the ISF data is invalid, a NULL Drawing is returned.
 *
 * @param isfData Raw ISF data to interpret.
 * @return A new Drawing object representing the data.
 */
Drawing Drawing::fromIsfData( const QByteArray &rawData )
{
  Drawing drawing;
  Compress::IsfData isfData( rawData );
  int size = isfData.size();

  if ( size == 0 )
  {
    return drawing;
  }

  ParserState state = ISF_PARSER_START;

  while( state != ISF_PARSER_FINISH )
  {
    switch( state )
    {
      case ISF_PARSER_START:
      {
        // step 1: read ISF version.
        quint8 version = Compress::decodeUInt( isfData );
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Version:" << version;
#endif
        if ( version != SUPPORTED_ISF_VERSION )
        {
          drawing.parserError_ = ISF_ERROR_BAD_VERSION;
          drawing.isNull_ = true;
          state = ISF_PARSER_FINISH;
        }
        else
        {
          // version is OK. find ISF stream size next.
          state = ISF_PARSER_STREAMSIZE;
        }

        break;
      }

      case ISF_PARSER_STREAMSIZE:
      {
        // read ISF stream size.
        // check it matches the length of the data array.
        quint64 streamSize = Compress::decodeUInt( isfData );

        if ( streamSize != (quint64)( isfData.size() - isfData.pos() ) )
        {
#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Invalid stream size" << streamSize
                   << "expected:" << ( isfData.size() - isfData.pos() );
#endif
          // streamsize is bad.
          drawing.parserError_ = ISF_ERROR_BAD_STREAMSIZE;
          state = ISF_PARSER_FINISH;
        }
        else
        {
#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Stream size:" << streamSize;
#endif
          // Validate the drawing
          drawing.isNull_ = false;

          // Fill up the default properties
          drawing.currentMetrics_    = &drawing.defaultMetrics_;
          drawing.currentPointInfo_  = &drawing.defaultPointInfo_;
          drawing.currentStrokeInfo_ = &drawing.defaultStrokeInfo_;
          drawing.currentTransform_  = &drawing.defaultTransform_;

          // start looking for ISF tags.
          state = ISF_PARSER_TAG;
        }

        break;
      }

      // ******************
      // This is the key point of the state machine. This will continually loop looking for ISF
      // tags and farming off to the appropriate method.
      // *******************
      case ISF_PARSER_TAG:
      {
        if( isfData.atEnd() )
        {
          state = ISF_PARSER_FINISH;
          break;
        }

        quint64 tagIndex = Compress::decodeUInt( isfData );
        drawing.parserError_ = parseTag( drawing, isfData, tagIndex );

        if( drawing.parserError_ != ISF_ERROR_NONE )
        {
#ifdef ISF_DEBUG_VERBOSE
          qWarning() << "Error in last operation, stopping";
#endif
          state = ISF_PARSER_FINISH;
        }

        break;
      }

      // Should never arrive here! It's here only to avoid compiler warnings.
      case ISF_PARSER_FINISH:
        break;

      break;
    }
  }

  // Perform the last operations on the drawing
  if( drawing.parserError_ == ISF_ERROR_NONE )
  {
    // Convert the maximum pen size to pixels
    drawing.maxPenSize_ /= HiMetricToPixel;

    // Adjust the bounding rectangle to include the strokes borders
    QSize size( drawing.maxPenSize_.toSize() );
    drawing.boundingRect_.adjust( -size.width(), -size.height(),
                                  +size.width(), +size.height() );
  }


#ifdef ISF_DEBUG_VERBOSE
    qDebug() << "Drawing bounding rectangle:" << drawing.boundingRect_;
#endif
#ifdef ISF_DEBUG_VERBOSE
    qDebug() << "Maximum thickness:" << drawing.maxPenSize_;
#endif

#ifdef ISF_DEBUG_VERBOSE
  qDebug() << "Finished with" << ( drawing.parserError_ == ISF_ERROR_NONE ? "success" : "error" );
  qDebug();
#endif

  return drawing;
}



/**
 * Parse a single ISF tag
 *
 * If nothing went wrong, this returns ISF_ERROR_NONE.
 *
 * @return an ISF error.
 */
IsfError Drawing::parseTag( Drawing &drawing, IsfData &isfData, quint64 tag )
{
  IsfError result = ISF_ERROR_NONE;
  quint64 value;

  // Thanks, thanks a lot to the TclISF authors!
  switch( tag )
  {
    case TAG_INK_SPACE_RECT:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_INK_SPACE_RECT";
#endif
      result = Tags::parseInkSpaceRectangle( isfData, drawing );
      break;

    case TAG_GUID_TABLE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_GUID_TABLE";
#endif
      result = Tags::parseGuidTable( isfData, drawing );
      break;

    case TAG_DRAW_ATTRS_TABLE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_DRAW_ATTRS_TABLE";
#endif
      result = Tags::parseAttributeTable( isfData, drawing );
      break;

    case TAG_DRAW_ATTRS_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
      result = Tags::parseAttributeBlock( isfData, drawing );
      break;

    case TAG_STROKE_DESC_TABLE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_STROKE_DESC_TABLE";
#endif
      result = Tags::parseStrokeDescTable( isfData, drawing );
      break;

    case TAG_STROKE_DESC_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_STROKE_DESC_BLOCK";
#endif
      result = Tags::parseStrokeDescBlock( isfData, drawing );
      break;

    case TAG_BUTTONS:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_BUTTONS";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_BUTTONS" );
      break;

    case TAG_NO_X:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_NO_X";
#endif
      result = ISF_ERROR_NONE;

      drawing.hasXData_ = false;
      break;

    case TAG_NO_Y:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_NO_Y";
#endif
      drawing.hasYData_ = false;
      break;

    case TAG_DIDX:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_DIDX";
#endif

      value = Isf::Compress::decodeUInt( isfData );

      if( value < (uint)drawing.attributes_.count() )
      {
        drawing.currentPointInfo_ = &drawing.attributes_[ value ];
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "- Next strokes will use drawing attributes #" << value;
#endif
      }
      else
      {
        qWarning() << "Invalid drawing attribute ID!";
      }
      break;

    case TAG_STROKE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_STROKE";
#endif
      result = Tags::parseStroke( isfData, drawing );
      break;

    case TAG_STROKE_PROPERTY_LIST:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_STROKE_PROPERTY_LIST";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_STROKE_PROPERTY_LIST" );
      break;

    case TAG_POINT_PROPERTY:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_POINT_PROPERTY";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_POINT_PROPERTY" );
      break;

    case TAG_SIDX:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_SIDX";
#endif

      value = Isf::Compress::decodeUInt( isfData );

      if( value < (uint)drawing.strokeInfo_.count() )
      {
        drawing.currentStrokeInfo_ = &drawing.strokeInfo_[ value ];
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "- Next strokes will use stroke info #" << value;
#endif
      }
      else
      {
        qWarning() << "Invalid stroke ID!";
      }
      break;

    case TAG_COMPRESSION_HEADER:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_COMPRESSION_HEADER";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_COMPRESSION_HEADER" );
      break;

    case TAG_TRANSFORM_TABLE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_TABLE";
#endif
      result = Tags::parseTransformationTable( isfData, drawing );
      break;

    case TAG_TRANSFORM:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_ISOTROPIC_SCALE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_ANISOTROPIC_SCALE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_ROTATE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_ROTATE";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_TRANSLATE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_TRANSLATE";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TRANSFORM_QUAD:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TRANSFORM_QUAD";
#endif
      result = Tags::parseTransformation( isfData, drawing, tag );
      break;

    case TAG_TIDX:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_TIDX";
#endif

      value = Isf::Compress::decodeUInt( isfData );

      if( value < (uint)drawing.transforms_.count() )
      {
        drawing.currentTransform_ = &drawing.transforms_[ value ];
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "- Next strokes will use transform #" << value;
#endif
      }
      else
      {
        qWarning() << "Invalid transform ID!";
      }

      break;

    case TAG_METRIC_TABLE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_METRIC_TABLE";
#endif
      result = Tags::parseMetricTable( isfData, drawing );
      break;

    case TAG_METRIC_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_METRIC_BLOCK";
#endif
      result = Tags::parseMetricBlock( isfData, drawing );
      break;

    case TAG_MIDX:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_MIDX";
#endif

      value = Isf::Compress::decodeUInt( isfData );

      if( value < (uint)drawing.metrics_.count() )
      {
        drawing.currentMetrics_ = &drawing.metrics_[ value ];
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "- Next strokes will use metrics #" << value;
#endif
      }
      else
      {
        qWarning() << "Invalid metrics ID!";
      }

      break;

    case TAG_MANTISSA:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_MANTISSA";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_MANTISSA" );
      break;

    case TAG_PERSISTENT_FORMAT:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_PERSISTENT_FORMAT";
#endif
      result = Tags::parsePersistentFormat( isfData, drawing );
      break;

    case TAG_HIMETRIC_SIZE:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_HIMETRIC_SIZE";
#endif
      result = Tags::parseHiMetricSize( isfData, drawing );
      break;

    case TAG_STROKE_IDS:
#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got tag: TAG_STROKE_IDS";
#endif
      result = Tags::parseUnsupported( isfData, "TAG_STROKE_IDS" );
      break;

    default:
      // If the tag *should* be known, record it differently
      if( drawing.maxGuid_ > 0 && tag >= 100 && tag <= drawing.maxGuid_ )
      {
        Tags::parseUnsupported( isfData, "TAG_GUID_" + QString::number( tag ) );
      }
      else
      {
        Tags::parseUnsupported( isfData, "Unknown " + QString::number( tag ) );
      }
      break;
  }

  return result;
}



QPixmap Drawing::getPixmap()
{
  QPixmap pixmap( size_ );
  pixmap.fill( Qt::white );
  QPainter painter( &pixmap );

  painter.setWindow( boundingRect_ );
  painter.setWorldMatrixEnabled( true );
  painter.setRenderHints(   QPainter::Antialiasing
                          | QPainter::SmoothPixmapTransform
                          | QPainter::TextAntialiasing,
                          true );

  QPen pen;
  pen.setStyle    ( Qt::SolidLine );
  pen.setCapStyle ( Qt::RoundCap  );
  pen.setJoinStyle( Qt::RoundJoin );


#ifdef ISF_DEBUG_VERBOSE
  qDebug() << "The drawing contains" << strokes_.count() << "strokes.";
#endif

  // Keep record of the currently used properties, to avoid re-setting them for each stroke
  currentMetrics_    = 0;
  currentPointInfo_  = 0;
  currentStrokeInfo_ = 0;
  currentTransform_  = 0;

  int index = 0;
  foreach( const Stroke &stroke, strokes_ )
  {
    if( currentMetrics_ != stroke.metrics )
    {
      currentMetrics_ = stroke.metrics;
      // TODO need to convert all units somehow?
//       painter.setSomething( currentMetrics );
    }
    if( currentPointInfo_ != stroke.attributes )
    {
      currentPointInfo_ = stroke.attributes;

      float penSizePixels = Drawing::himetricToPixels( currentPointInfo_->penSize.width(), pixmap );

      pen.setColor( currentPointInfo_->color );
      pen.setWidthF( penSizePixels );
      painter.setPen( pen );
    }
    if( currentStrokeInfo_ != stroke.info )
    {
      currentStrokeInfo_ = stroke.info;
    }
    if( currentTransform_ != stroke.transform )
    {
      currentTransform_ = stroke.transform;
      painter.setWorldTransform( *currentTransform_, true );
    }

#ifdef ISF_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke.points.count() << "points...";
    qDebug() << "- Stroke color:" << currentPointInfo_->color.name() << "Pen size:" << pen.widthF();
#endif

    if( stroke.points.count() > 1 )
    {
      Point lastPoint;
      foreach( const Point &point, stroke.points )
      {
//       qDebug() << "Point:" << point.position;

        if( lastPoint.position.isNull() )
        {
          lastPoint = point;
          continue;
        }

        if( currentStrokeInfo_->hasPressureData )
        {
          // FIXME Ignoring pressure data - need to find out how pressure must be applied
//           pen.setWidth( pen.widthF() + point.pressureLevel );
//           painter.setPen( pen );
        }

        // How nice of QPainter! Lines drawn from and to the same point
        // won't be drawn at all
        if( point.position == lastPoint.position )
        {
          painter.drawPoint( point.position );
        }
        else
        {
          painter.drawLine( lastPoint.position, point.position );
        }

        lastPoint = point;
      }
    }
    else
    {
      Point point = stroke.points.first();
      if( currentStrokeInfo_->hasPressureData )
      {
        // FIXME Ignoring pressure data - need to find out how pressure must be applied
//         pen.setWidth( pen.widthF() + point.pressureLevel );
//         painter.setPen( pen );
      }

//       qDebug() << "Point:" << point.position;
      painter.drawPoint( point.position );
    }

/*
#ifdef ISF_DEBUG_VERBOSE
    // Draw the stroke number next to each one, for debugging purposes
    pen.setColor( QColor( Qt::red ) );
    painter.setPen( pen );
    painter.drawText( stroke.points.first().position, QString::number( index ) );
    pen.setColor( currentPointInfo_->color );
    painter.setPen( pen );
#endif
*/

    ++index;
  }

  painter.end();

  return pixmap;
}



}


