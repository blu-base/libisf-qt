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
#include "compression/isfdata.h"
#include "multibytecoding.h"

#include <QtDebug>


/**
 * Uncomment to get a ton of debugging messages about
 * whatever was found in the data
 */
#define PARSER_DEBUG_VERBOSE



namespace Isf
{



/**
 * Construct a new NULL Drawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
Drawing::Drawing()
  : isNull_( true ),
    parserError_( ISF_ERROR_NONE )
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
ParserError Drawing::parserError() const
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
#ifdef PARSER_DEBUG_VERBOSE
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

        quint8 tagIndex = Compress::decodeUInt( isfData );

        // Thanks, thanks a lot to the TclISF authors!
        switch( tagIndex )
        {
          case TAG_INK_SPACE_RECT:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_INK_SPACE_RECT";
#endif
              // TODO
              break;

          case TAG_GUID_TABLE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_GUID_TABLE";
#endif
              // err =  getGUIDTable ();
              break;

          case TAG_DRAW_ATTRS_TABLE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_DRAW_ATTRS_TABLE";
#endif
              // err = getDrawAttrsTable ();
              break;

          case TAG_DRAW_ATTRS_BLOCK:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
              // err = getDrawAttrsBlock ();
              break;

          case TAG_STROKE_DESC_TABLE:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_STROKE_DESC_TABLE";
#endif
              // TODO
              break;

          case TAG_STROKE_DESC_BLOCK:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_STROKE_DESC_BLOCK";
#endif
              // err = getStrokeDescBlock ();
              break;

          case TAG_BUTTONS:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_BUTTONS";
#endif
              // TODO
              break;

          case TAG_NO_X:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_NO_X";
#endif
              // TODO
              break;

          case TAG_NO_Y:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_NO_Y";
#endif
              // TODO
              break;

          case TAG_DIDX:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_DIDX";
#endif
              // err = getDIDX ();
              break;

          case TAG_STROKE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_STROKE";
#endif
              // err = getStroke ();
              break;

          case TAG_STROKE_PROPERTY_LIST:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_STROKE_PROPERTY_LIST";
#endif
              // TODO
              break;

          case TAG_POINT_PROPERTY:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_POINT_PROPERTY";
#endif
              // TODO
              break;

          case TAG_SIDX:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_SIDX";
#endif
              // TODO
              break;

          case TAG_COMPRESSION_HEADER:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_COMPRESSION_HEADER";
#endif
              // TODO
              break;

          case TAG_TRANSFORM_TABLE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_TABLE";
#endif
              // err = getTransformTable ();
              break;

          case TAG_TRANSFORM:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM";
#endif
              // err = getTransform ();
              break;

          case TAG_TRANSFORM_ISOTROPIC_SCALE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
              // err = getTransformIsotropicScale ();
              break;

          case TAG_TRANSFORM_ANISOTROPIC_SCALE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
              // err = getTransformAnisotropicScale ();
              break;

          case TAG_TRANSFORM_ROTATE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_ROTATE";
#endif
              // err = getTransformRotate ();
              break;

          case TAG_TRANSFORM_TRANSLATE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_TRANSLATE";
#endif
              // err = getTransformTranslate ();
              break;

          case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
              // err = getTransformScaleAndTranslate ();
              break;

          case TAG_TRANSFORM_QUAD:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_TRANSFORM_QUAD";
#endif
              // TODO
              break;

          case TAG_TIDX:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_TIDX";
#endif
              // err = getTIDX ();
              break;

          case TAG_METRIC_TABLE:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_METRIC_TABLE";
#endif
              // TODO
              break;

          case TAG_METRIC_BLOCK:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_METRIC_BLOCK";
#endif
              // err = getMetricBlock ();
              break;

          case TAG_MIDX:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_MIDX";
#endif
              // TODO
              break;

          case TAG_MANTISSA:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "Got tag: TAG_MANTISSA";
#endif
              // TODO
              break;

          case TAG_PERSISTENT_FORMAT:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_PERSISTENT_FORMAT";
#endif
              // err = getPersistentFormat ();
              break;

          case TAG_HIMETRIC_SIZE:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_HIMETRIC_SIZE";
#endif
              // err = getHimetricSize ();
              break;

          case TAG_STROKE_IDS:
#ifdef PARSER_DEBUG_VERBOSE
              qDebug() << "Got tag: TAG_STROKE_IDS";
#endif
              // err = getStrokeIds ();
              break;

          default:
#ifdef PARSER_DEBUG_VERBOSE
              qWarning() << "got unknown tag" << tagIndex;
#endif
              break;
        }

      }

      break;
    }
  }

  return drawing;
}

}
