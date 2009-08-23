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

#ifndef ISFQTDRAWING_H
#define ISFQTDRAWING_H

#include <IsfQt>

#include <QMap>
#include <QMatrix>
#include <QPaintDevice>


// Forward declarations
class QByteArray;



namespace Isf
{



  /**
   * This class is a container for the data of ISF (Ink Serialized Format) drawings.
   *
   * @author Adam Goossens (adam@kmess.org)
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class Drawing
  {
    friend class Stream;
    friend class TagsParser;
    friend class TagsWriter;

    public:

      /// Constructor
                                Drawing();
      /// Destructor
                               ~Drawing();
      /// Clean up the drawing
      void                      clear();
      /// Return the last error
      IsfError                  error() const;
      /// Convert the ISF drawing into a pixmap
      QPixmap                   getPixmap( const QColor backgroundColor = Qt::white );
      /// Return whether this is a null Drawing
      bool                      isNull() const;


    public: // Manipulation methods

      /// Add new attribute set to the drawing
      qint32                    addAttributeSet( AttributeSet *newAttributeSet );
      /// Add a new stroke to the drawing
      qint32                    addStroke( Stroke *newStroke );
      /// Add a new transformation to the drawing
      qint32                    addTransform( QMatrix *newTransform );
      /// Remove an attribute set from the drawing
      bool                      deleteAttributeSet( quint32 index );
      /// Remove a stroke from the drawing
      bool                      deleteStroke( quint32 index );
      bool                      deleteStroke( Stroke *stroke ) ;
      /// Remove a transformation from the drawing
      bool                      deleteTransform( quint32 index );
      /// Retrieve an attribute set to manipulate it
      AttributeSet             *getAttributeSet( quint32 index );
      /// Retrieve the attribute sets
      const QList<AttributeSet*>getAttributeSets();
      /// Return the QRect bounding rectangle
      QRect                     getBoundingRect();
      /// Return the pixel size of the drawing
      QSize                     getSize();
      /// Return the stroke that passes through a given point.
      Stroke                   *getStrokeAtPoint( QPoint point );
      /// Retrieve a stroke to manipulate it
      Stroke                   *getStroke( quint32 index );
      /// Retrieve the strokes
      const QList<Stroke*>      getStrokes();
      /// Retrieve a transformation to manipulate it
      QMatrix                  *getTransform( quint32 index );
      /// Retrieve the transformations
      const QList<QMatrix*>     getTransforms();
      /// Change the current attribute set
      bool                      setCurrentAttributeSet( AttributeSet *attributeSet );
      /// Change the new current attribute set
      bool                      setCurrentTransform( QMatrix *transform );

    public: // Public static methods

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


    private: // Private properties

      // List of attributes of the points in the drawing
      QList<AttributeSet*>      attributeSets_;
      // Bounding rectangle of the drawing
      QRect                     boundingRect_;
      // Virtual drawing canvas dimensions
      QRect                     canvas_;
      // Link to the currently used metric data
      Metrics                  *currentMetrics_;
      // Link to the currently used point info data
      AttributeSet             *currentAttributeSet_;
      // Link to the currently used stroke info data
      StrokeInfo               *currentStrokeInfo_;
      // Link to the currently used transformation
      QMatrix                  *currentTransform_;
      // Link to the default metric data
      Metrics                   defaultMetrics_;
      // Link to the default point info data
      AttributeSet              defaultAttributeSet_;
      // Link to the default stroke info data
      StrokeInfo                defaultStrokeInfo_;
      // Link to the default transformation
      QMatrix                   defaultTransform_;
      // Last parsing error (if there is one)
      IsfError                  error_;
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
      QList<Metrics*>           metrics_;
      // Pixel size of the drawing
      QSize                     size_;
      // List of information about the drawing's strokes
      QList<StrokeInfo*>        strokeInfo_;
      // List of strokes composing this drawing
      QList<Stroke*>            strokes_;
      // Transformation matrices
      QList<QMatrix*>           transforms_;

  };



}



#endif // ISFQTDRAWING_H
