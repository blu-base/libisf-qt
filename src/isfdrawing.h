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

#include <QMap>
#include <QTransform>


// Forward declarations
class QByteArray;



namespace Isf
{



  // Forward declarations
  namespace Compress
  {
    class IsfData;
  }
  using Compress::IsfData;



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
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class Drawing
  {

    public:

      /// Supported ISF version number
      static const unsigned short SUPPORTED_ISF_VERSION = 0;

    public:

      /// Constructor
      Drawing();
      /// Convert the ISF drawing into a pixmap
      QPixmap              getPixmap();


    public: // Public static methods

      /// Return whether this is a null Drawing
      bool                 isNull() const;
      /// Return the last parser error
      IsfError             parserError() const;


    public: // Public static methods

      /// builds an ISF drawing object from raw ISF data.
      static Drawing       fromIsfData( const QByteArray &rawData );


    private: // Private static methods

      /// Parse a single ISF tag
      static IsfError      parseTag( Drawing &drawing, IsfData &isfData, DataTag tag );


    private: // Private properties

      // List of attributes of the points in the drawing
      QList<PointInfo>      attributes_;
      // Virtual drawing canvas dimensions
      QRect                 canvas_;
      // Whether the drawing contains X coordinates or not
      bool                  hasXData_;
      // Whether the drawing contains Y coordinates or not
      bool                  hasYData_;
      // Whether the drawing is invalid or valid
      bool                  isNull_;
      // List of strokes composing this drawing
      QList<Stroke>         strokes_;
      // Transformation matrices
      QMap<DataTag,QTransform> transforms_;
      // Maximum GUID available in the drawing
      quint64               maxGuid_;
      // Parser error (if there is one)
      IsfError              parserError_;
      // Pixel size of the drawing
      QSize                 size_;

  };



}



#endif // ISFDRAWING_H
