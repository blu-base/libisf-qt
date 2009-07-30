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



namespace Isf
{

/**
 * Construct a new NULL IsfDrawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
IsfDrawing::IsfDrawing()
  : isNull_( true ),
    parserError_( ISF_ERROR_NONE )
{
}

/**
 * Return True if this instance of IsfDrawing is invalid (NULL), False otherwise.
 *
 * @return True if this is a NULL IsfDrawing, FALSE otherwise.
 */
bool IsfDrawing::isNull() const
{
  return isNull_;
}

/**
 * Return the error from the ISF parser if this is a NULL IsfDrawing.
 *
 * If nothing went wrong, this returns ISF_ERROR_NONE.
 *
 * @return The last ISF parser error.
 */
IsfParserError IsfDrawing::parserError() const
{
  return parserError_;
}

/**
 * Creates and returns a new IsfDrawing object from some raw ISF input data.
 *
 * If the ISF data is invalid, a NULL IsfDrawing is returned.
 *
 * @param isfData Raw ISF data to interpret.
 * @return A new IsfDrawing object representing the data.
 */
IsfDrawing IsfDrawing::fromIsfData( const QByteArray &rawData )
{
  IsfDrawing drawing;
  Compress::IsfData isfData( rawData );

  int size = isfData.size();

  if ( size == 0 )
  {
    drawing.isNull_ = true;
    return drawing;
  }

  // let's start by assuming that the data is valid.
  drawing.isNull_ = false;

  IsfParserState state = ISF_PARSER_START;

  while ( ( ! isfData.atEnd() ) && ( state != ISF_PARSER_FINISH ) )
  {
    switch ( state )
    {
      case ISF_PARSER_START:
      {
        // step 1: read ISF version.
        quint8 version = Compress::decodeUInt( isfData );
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
          drawing.isNull_ = true;
          state = ISF_PARSER_FINISH;
        }
        else
        {
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
        quint8 tagIdx = Compress::decodeUInt( isfData );
        if ( tagIdx == TAG_GUID_TABLE )
        {
          qDebug() << "FOUND GUID TABLE";
        }
        state = ISF_PARSER_FINISH;
      }

      break;
    }
  }

  return drawing;
}

}
