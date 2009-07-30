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

#ifndef ISFDRAWING_H
#define ISFDRAWING_H


#include "libisftypes.h"


// Forward declarations
class QByteArray;



namespace Isf
{

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
   * This class loads ISF (Ink Serialized Format) drawings.
   *
   * @author Adam Goossens (adam@kmess.org)
   */
  class Drawing
  {

    public:

      /// Supported ISF version number
      static const unsigned short SUPPORTED_ISF_VERSION = 0;

    public:

      /// Constructor
      Drawing();


    public: // Public static methods

      /// Return whether this is a null Drawing
      bool                 isNull() const;
      /// Return the last parser error
      IsfError             parserError() const;


    public: // Public static methods

      /// builds an ISF drawing object from raw ISF data.
      static Drawing       fromIsfData( const QByteArray &rawData );


    private: // Private properties

      // Whether the drawing is invalid or valid
      bool                 isNull_;
      // Parser error (if there is one)
      IsfError             parserError_;

  };



}



#endif // ISFDRAWING_H
