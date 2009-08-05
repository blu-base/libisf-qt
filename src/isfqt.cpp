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

#include <isfdrawing.h>

#include "isfqt-internal.h"

#include "data/datasource.h"
#include "data/multibytecoding.h"
#include "tagsparser.h"

#include <QPainter>
#include <QPixmap>


using namespace Isf;
using namespace Compress;


/// Supported ISF version number
#define SUPPORTED_ISF_VERSION       0



/**
 * Creates and returns a new Drawing object from some raw ISF input data.
 *
 * If the ISF data is invalid, a NULL Drawing is returned.
 *
 * @param isfData Raw ISF data to interpret.
 * @return A new Drawing object representing the data.
 */
Drawing Parser::isfToDrawing( const QByteArray &rawData )
{
  Drawing drawing;
  DataSource isfData( rawData );
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
        quint8 version = decodeUInt( isfData );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "Version:" << version;
#endif
        if ( version != SUPPORTED_ISF_VERSION )
        {
          drawing.error_ = ISF_ERROR_BAD_VERSION;
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
        quint64 streamSize = decodeUInt( isfData );

        if ( streamSize != (quint64)( isfData.size() - isfData.pos() ) )
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Invalid stream size" << streamSize
                   << ", expected" << ( isfData.size() - isfData.pos() );
#endif
          // streamsize is bad.
          drawing.error_ = ISF_ERROR_BAD_STREAMSIZE;
          state = ISF_PARSER_FINISH;
        }
        else
        {
#ifdef ISFQT_DEBUG
          qDebug() << "Reading ISF stream of size:" << streamSize << "...";
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

        quint64 tagIndex = decodeUInt( isfData );
        switch( tagIndex )
        {
          case TAG_INK_SPACE_RECT:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_INK_SPACE_RECT";
#endif
            drawing.error_ = TagsParser::parseInkSpaceRectangle( isfData, drawing );
            break;

          case TAG_GUID_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_GUID_TABLE";
#endif
            drawing.error_ = TagsParser::parseGuidTable( isfData, drawing );
            break;

          case TAG_DRAW_ATTRS_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DRAW_ATTRS_TABLE";
#endif
            drawing.error_ = TagsParser::parseAttributeTable( isfData, drawing );
            break;

          case TAG_DRAW_ATTRS_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
            drawing.error_ = TagsParser::parseAttributeBlock( isfData, drawing );
            break;

          case TAG_STROKE_DESC_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_DESC_TABLE";
#endif
            drawing.error_ = TagsParser::parseStrokeDescTable( isfData, drawing );
            break;

          case TAG_STROKE_DESC_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_DESC_BLOCK";
#endif
            drawing.error_ = TagsParser::parseStrokeDescBlock( isfData, drawing );
            break;

          case TAG_BUTTONS:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_BUTTONS";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_BUTTONS" );
            break;

          case TAG_NO_X:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_NO_X";
#endif
            drawing.error_ = ISF_ERROR_NONE;

            drawing.hasXData_ = false;
            break;

          case TAG_NO_Y:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_NO_Y";
#endif
            drawing.hasYData_ = false;
            break;

          case TAG_DIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DIDX";
#endif

            quint64 value = decodeUInt( isfData );

            if( value < (uint)drawing.attributes_.count() )
            {
              drawing.currentPointInfo_ = &drawing.attributes_[ value ];
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use drawing attributes#" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid drawing attribute ID!";
#endif
            }
            break;
          }

          case TAG_STROKE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE";
#endif
            drawing.error_ = TagsParser::parseStroke( isfData, drawing );
            break;

          case TAG_STROKE_PROPERTY_LIST:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_PROPERTY_LIST";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_STROKE_PROPERTY_LIST" );
            break;

          case TAG_POINT_PROPERTY:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_POINT_PROPERTY";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_POINT_PROPERTY" );
            break;

          case TAG_SIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_SIDX";
#endif

            quint64 value = decodeUInt( isfData );

            if( value < (uint)drawing.strokeInfo_.count() )
            {
              drawing.currentStrokeInfo_ = &drawing.strokeInfo_[ value ];
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use stroke info#" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid stroke ID!";
#endif
            }
            break;
          }

          case TAG_COMPRESSION_HEADER:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_COMPRESSION_HEADER";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_COMPRESSION_HEADER" );
            break;

          case TAG_TRANSFORM_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_TABLE";
#endif
            drawing.error_ = TagsParser::parseTransformationTable( isfData, drawing );
            break;

          case TAG_TRANSFORM:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ISOTROPIC_SCALE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ANISOTROPIC_SCALE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_ROTATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ROTATE";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_TRANSLATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_TRANSLATE";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TRANSFORM_QUAD:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_QUAD";
#endif
            drawing.error_ = TagsParser::parseTransformation( isfData, drawing, tagIndex );
            break;

          case TAG_TIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TIDX";
#endif

            quint64 value = decodeUInt( isfData );

            if( value < (uint)drawing.transforms_.count() )
            {
              drawing.currentTransform_ = &drawing.transforms_[ value ];
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use transform#" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid transform ID!";
#endif
            }

            break;
          }

          case TAG_METRIC_TABLE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_METRIC_TABLE";
#endif
            drawing.error_ = TagsParser::parseMetricTable( isfData, drawing );
            break;

          case TAG_METRIC_BLOCK:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_METRIC_BLOCK";
#endif
            drawing.error_ = TagsParser::parseMetricBlock( isfData, drawing );
            break;

          case TAG_MIDX:
          {
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_MIDX";
#endif

            quint64 value = decodeUInt( isfData );

            if( value < (uint)drawing.metrics_.count() )
            {
              drawing.currentMetrics_ = &drawing.metrics_[ value ];
#ifdef ISFQT_DEBUG_VERBOSE
              qDebug() << "- Next strokes will use metrics#" << value;
#endif
            }
            else
            {
#ifdef ISFQT_DEBUG
              qWarning() << "Invalid metrics ID!";
#endif
            }

            break;
          }

          case TAG_MANTISSA:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_MANTISSA";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_MANTISSA" );
            break;

          case TAG_PERSISTENT_FORMAT:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_PERSISTENT_FORMAT";
#endif
            drawing.error_ = TagsParser::parsePersistentFormat( isfData, drawing );
            break;

          case TAG_HIMETRIC_SIZE:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_HIMETRIC_SIZE";
#endif
            drawing.error_ = TagsParser::parseHiMetricSize( isfData, drawing );
            break;

          case TAG_STROKE_IDS:
#ifdef ISFQT_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_IDS";
#endif
            drawing.error_ = TagsParser::parseUnsupported( isfData, "TAG_STROKE_IDS" );
            break;

          default:
            // If the tagIndex *should* be known, record it differently
            if( drawing.maxGuid_ > 0 && tagIndex >= 100 && tagIndex <= drawing.maxGuid_ )
            {
              TagsParser::parseUnsupported( isfData, "TAG_GUID_" + QString::number( tagIndex ) );
            }
            else
            {
              TagsParser::parseUnsupported( isfData, "Unknown " + QString::number( tagIndex ) );
            }
            break;

        } // End of tagIndex switch

        if( drawing.error_ != ISF_ERROR_NONE )
        {
#ifdef ISFQT_DEBUG_VERBOSE
          qWarning() << "Error in last operation, stopping.";
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
  if( drawing.error_ == ISF_ERROR_NONE )
  {
    // Convert the maximum pen size to pixels
    drawing.maxPenSize_ /= HiMetricToPixel;

    // Adjust the bounding rectangle to include the strokes borders
    QSize size( drawing.maxPenSize_.toSize() );
    drawing.boundingRect_.adjust( -size.width(), -size.height(),
                                  +size.width(), +size.height() );
  }


#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "Drawing bounding rectangle:" << drawing.boundingRect_;
  qDebug() << "Maximum thickness:" << drawing.maxPenSize_;
#endif

#ifdef ISFQT_DEBUG
  qDebug() << "Finished with" << ( drawing.error_ == ISF_ERROR_NONE ? "success" : "error" );
  qDebug();
#endif

  return drawing;
}



/**
  * Convert a drawing into raw data in ISF format
  */
QByteArray Parser::DrawingToIsf( const Drawing &drawing )
{
  Q_UNUSED( drawing )

  // TODO Write me!

  return QByteArray();
}
