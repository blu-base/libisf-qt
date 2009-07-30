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

#include <QtDebug>



namespace Isf
{



/**
 * Construct a new NULL Drawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
Drawing::Drawing()
  : isNull_( true )
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

  while( ( ! isfData.atEnd() ) && ( state != ISF_PARSER_FINISH ) )
  {
    switch( state )
    {
      case ISF_PARSER_START:
      {
        // step 1: read ISF version.
        quint8 version = Compress::decodeUInt( isfData );
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Got version" << version;
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

        if ( streamSize != ( isfData.size() - isfData.pos() ) )
        {
          // streamsize is bad.
          drawing.parserError_ = ISF_ERROR_BAD_STREAMSIZE;
          state = ISF_PARSER_FINISH;
        }
        else
        {
          // Validate the drawing
          drawing.isNull_ = false;
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
//         state = ISF_PARSER_FINISH;

        IsfError result = ISF_ERROR_NONE;
        quint8 tagIndex = Compress::decodeUInt( isfData );

        // Thanks, thanks a lot to the TclISF authors!
        switch( tagIndex )
        {
          case TAG_INK_SPACE_RECT:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_INK_SPACE_RECT";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_INK_SPACE_RECT" );
            break;

          case TAG_GUID_TABLE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_GUID_TABLE";
#endif
            result = Tags::parseGuidTable( isfData, drawing.maxGuid_ );
            break;

          case TAG_DRAW_ATTRS_TABLE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DRAW_ATTRS_TABLE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_DRAW_ATTRS_TABLE" );
            break;

          case TAG_DRAW_ATTRS_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_DRAW_ATTRS_BLOCK" );
            break;

          case TAG_STROKE_DESC_TABLE:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_STROKE_DESC_TABLE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_STROKE_DESC_TABLE" );
            break;

          case TAG_STROKE_DESC_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_DESC_BLOCK";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_STROKE_DESC_BLOCK" );
            break;

          case TAG_BUTTONS:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_BUTTONS";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_BUTTONS" );
            break;

          case TAG_NO_X:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_NO_X";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_NO_X" );
            break;

          case TAG_NO_Y:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_NO_Y";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_NO_Y" );
            break;

          case TAG_DIDX:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_DIDX";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_DIDX" );
            break;

          case TAG_STROKE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_STROKE" );
            break;

          case TAG_STROKE_PROPERTY_LIST:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_STROKE_PROPERTY_LIST";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_STROKE_PROPERTY_LIST" );
            break;

          case TAG_POINT_PROPERTY:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_POINT_PROPERTY";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_POINT_PROPERTY" );
            break;

          case TAG_SIDX:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_SIDX";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_SIDX" );
            break;

          case TAG_COMPRESSION_HEADER:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_COMPRESSION_HEADER";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_COMPRESSION_HEADER" );
            break;

          case TAG_TRANSFORM_TABLE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_TABLE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_TABLE" );
            break;

          case TAG_TRANSFORM:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM" );
            break;

          case TAG_TRANSFORM_ISOTROPIC_SCALE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_ISOTROPIC_SCALE" );
            break;

          case TAG_TRANSFORM_ANISOTROPIC_SCALE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_ANISOTROPIC_SCALE" );
            break;

          case TAG_TRANSFORM_ROTATE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_ROTATE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_ROTATE" );
            break;

          case TAG_TRANSFORM_TRANSLATE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_TRANSLATE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_TRANSLATE" );
            break;

          case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_SCALE_AND_TRANSLATE" );
            break;

          case TAG_TRANSFORM_QUAD:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_TRANSFORM_QUAD";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TRANSFORM_QUAD" );
            break;

          case TAG_TIDX:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_TIDX";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_TIDX" );
            break;

          case TAG_METRIC_TABLE:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_METRIC_TABLE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_METRIC_TABLE" );
            break;

          case TAG_METRIC_BLOCK:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_METRIC_BLOCK";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_METRIC_BLOCK" );
            break;

          case TAG_MIDX:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_MIDX";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_MIDX" );
            break;

          case TAG_MANTISSA:
#ifdef ISF_DEBUG_VERBOSE
            qWarning() << "Got tag: TAG_MANTISSA";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_MANTISSA" );
            break;

          case TAG_PERSISTENT_FORMAT:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_PERSISTENT_FORMAT";
#endif
            result = Tags::parsePersistentFormat( isfData );
            break;

          case TAG_HIMETRIC_SIZE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_HIMETRIC_SIZE";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_HIMETRIC_SIZE" );
            break;

          case TAG_STROKE_IDS:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "Got tag: TAG_STROKE_IDS";
#endif
            result = Tags::parseUnsupported( isfData, "TAG_STROKE_IDS" );
            break;

          default:
            // If the tag *should* be known, record it differently
            if( drawing.maxGuid_ > 0 && tagIndex >= 100 && tagIndex <= drawing.maxGuid_ )
            {
              Tags::analyzePayload( isfData, "TAG_GUID_" + QString::number( tagIndex ) );
              //  err = getProperty (pDecISF, tag);
            }
            else
            {
              Tags::analyzePayload( isfData, "Unknown " + QString::number( tagIndex ) );
            }
            break;
        }

        if( result != ISF_ERROR_NONE )
        {
#ifdef ISF_DEBUG_VERBOSE
          qWarning() << "Error in last operation, stopping";
#endif
          state = ISF_PARSER_FINISH;
        }

      }

      break;
    }
  }

  return drawing;
}

}
