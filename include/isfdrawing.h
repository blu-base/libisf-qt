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


#include "isfqt.h"

#include <QMap>
#include <QTransform>
#include <QPaintDevice>


// Forward declarations
class QByteArray;



namespace Isf
{



  /**
   * This class loads ISF (Ink Serialized Format) drawings.
   *
   * @author Adam Goossens (adam@kmess.org)
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class Drawing
  {
    friend class TagsParser;

    public:

      /// Constructor
                                Drawing();
      /// Convert the ISF drawing into a pixmap
      QPixmap                   getPixmap();


    public: // Public static methods

      /// Return whether this is a null Drawing
      bool                      isNull() const;
      /// Return the last parser error
      IsfError                  parserError() const;


    public: // Public static methods

      /// Builds an ISF drawing object from raw ISF data
      static Drawing            fromIsfData( const QByteArray &rawData );
      /// Convert a value in himetric units to pixels, given a paint device
      inline static float       himetricToPixels( float himetric, QPaintDevice &device )
      {
        return ( ( (float)device.width() / (float)device.widthMM() ) * ( himetric * 0.01 ) );
      }
      /// Convert a value in pixels to himetric units, given a paint device
      inline static float       pixelsToHimetric( float pixels, QPaintDevice &device )
      {
        return ( pixels / ( (float)device.width() / (float)device.widthMM() ) / 0.01 );
      }


    private: // Private static properties

      /// Supported ISF version number
      static const ushort       SUPPORTED_ISF_VERSION = 0;


    private: // Private properties

      // List of attributes of the points in the drawing
      QList<PointInfo>          attributes_;
      // Bounding rectangle of the drawing
      QRect                     boundingRect_;
      // Virtual drawing canvas dimensions
      QRect                     canvas_;
      // Link to the currently used metric data
      Metrics                  *currentMetrics_;
      // Link to the currently used point info data
      PointInfo                *currentPointInfo_;
      // Link to the currently used stroke info data
      StrokeInfo               *currentStrokeInfo_;
      // Link to the currently used transformation
      QTransform               *currentTransform_;
      // Link to the default metric data
      Metrics                   defaultMetrics_;
      // Link to the default point info data
      PointInfo                 defaultPointInfo_;
      // Link to the default stroke info data
      StrokeInfo                defaultStrokeInfo_;
      // Link to the default transformation
      QTransform                defaultTransform_;
      // Whether the drawing contains X coordinates or not
      bool                      hasXData_;
      // Whether the drawing contains Y coordinates or not
      bool                      hasYData_;
      // Whether the drawing is invalid or valid
      bool                      isNull_;
      // Maximum GUID available in the drawing
      quint64                   maxGuid_;
      // Maximum thickness of the strokes
      QSizeF                    maxPenSize_;
      // List of metrics used in the drawing
      QList<Metrics>            metrics_;
      // Parser error (if there is one)
      IsfError                  parserError_;
      // Pixel size of the drawing
      QSize                     size_;
      // List of information about the drawing's strokes
      QList<StrokeInfo>         strokeInfo_;
      // List of strokes composing this drawing
      QList<Stroke>             strokes_;
      // Transformation matrices
      QList<QTransform>         transforms_;

  };



}



#endif // ISFDRAWING_H
