/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                 *
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

#ifndef ISFQT_INTERNAL_H
#define ISFQT_INTERNAL_H

#include "isfqtconfig.h"

#include <IsfQt>

#include <QtDebug>


/**
 * Uncomment this define to enable very verbose debugging output
 */
#ifdef ISFQT_DEBUG
  #define ISFQT_DEBUG_VERBOSE
#endif



namespace Isf
{



  /**
   * A list of all known ISF tag indexes into the GUID
   * table
   */
  enum DataTag
  {
    TAG_INK_SPACE_RECT                      =   0
  , TAG_GUID_TABLE
  , TAG_DRAW_ATTRS_TABLE
  , TAG_DRAW_ATTRS_BLOCK
  , TAG_STROKE_DESC_TABLE
  , TAG_STROKE_DESC_BLOCK
  , TAG_BUTTONS
  , TAG_NO_X
  , TAG_NO_Y
  , TAG_DIDX
  , TAG_STROKE                           // =  10
  , TAG_STROKE_PROPERTY_LIST
  , TAG_POINT_PROPERTY
  , TAG_SIDX
  , TAG_COMPRESSION_HEADER
  , TAG_TRANSFORM_TABLE
  , TAG_TRANSFORM
  , TAG_TRANSFORM_ISOTROPIC_SCALE
  , TAG_TRANSFORM_ANISOTROPIC_SCALE
  , TAG_TRANSFORM_ROTATE
  , TAG_TRANSFORM_TRANSLATE              // =  20
  , TAG_TRANSFORM_SCALE_AND_TRANSLATE
  , TAG_TRANSFORM_QUAD
  , TAG_TIDX
  , TAG_METRIC_TABLE
  , TAG_METRIC_BLOCK
  , TAG_MIDX
  , TAG_MANTISSA
  , TAG_PERSISTENT_FORMAT
  , TAG_HIMETRIC_SIZE
  , TAG_STROKE_IDS                       // =  30
  , DEFAULT_TAGS_NUMBER                     = 100
  };
  Q_DECLARE_FLAGS( DataTags, DataTag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( DataTags )



  /**
   * ISF parser state machine states
   */
  enum ParserState
  {
    ISF_PARSER_START = 0,        /// The parser has not read anything from the data yet
    ISF_PARSER_FINISH,           /// The parser is done
    ISF_PARSER_STREAMSIZE,       /// The parser is reading the ISF stream size
    ISF_PARSER_TAG               /// The parser is reading tags from the stream
  };



  /**
   * Persistent Format version
   *
   * See TagsParser::parsePersistentFormat() for more info.
   */
  const uint ISF_PERSISTENT_FORMAT_VERSION = 65536;


}



#endif
