/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
 *   Copyright (C) 2009 by Adam Goossens                                   *
 *   adam@kmess.org                                                        *
 ***************************************************************************/

/***************************************************************************
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
#include <QUuid>


// Forward declarations
class QByteArray;



namespace Isf
{



  /**
   * @class Drawing
   * @brief This is a manipulable representation of an ISF stream.
   *
   * This class is a container for the data of ISF (Ink Serialized Format)
   * drawings. You can manipulate its contents, or create a new one and
   * fill it, then save it back to ISF format with the Isf::Stream methods.
   *
   * @see Isf::Stream
   * @author Adam Goossens (adam@kmess.org)
   * @author Valerio Pilo (valerio@kmess.org)
   */
  class Drawing
  {
    friend class Stream;
    friend class TagsParser;
    friend class TagsWriter;

    public: // public constructors
                                 Drawing();
                                 Drawing( const Drawing &other );
                                ~Drawing();

    public: // public state retrieval methods
      AttributeSet              *attributeSet( quint32 index );
      const QList<AttributeSet*> attributeSets();
      QRect                      boundingRect();
      void                       clear();
      IsfError                   error() const;
      bool                       isNull() const;
      QPixmap                    pixmap( const QColor backgroundColor = Qt::white );
      QSize                      size();
      Stroke                    *stroke( quint32 index );
      Stroke                    *strokeAtPoint( const QPoint &point );
      const QList<Stroke*>       strokes();
      QMatrix                   *transform( quint32 index );
      const QList<QMatrix*>      transforms();

    public: // public manipulation methods
      qint32                     addAttributeSet( AttributeSet *newAttributeSet );
      qint32                     addStroke( Stroke *newStroke );
      qint32                     addTransform( QMatrix *newTransform );
      bool                       deleteAttributeSet( quint32 index );
      bool                       deleteStroke( quint32 index );
      bool                       deleteStroke( Stroke *stroke ) ;
      bool                       deleteTransform( quint32 index );
      bool                       setCurrentAttributeSet( AttributeSet *attributeSet );
      bool                       setCurrentTransform( QMatrix *transform );

    public: // Public static utility methods
      /**
       * Converts a value from himetric units to pixels.
       *
       * The paint device is required to retrieve the device size.
       * Himetric is a device-dependent unit.
       *
       * @param himetric The value to convert
       * @param device The device used to detect the size of the drawing context
       */
      inline static float        himetricToPixels( float himetric, QPaintDevice &device )
      {
        return ( ( (float)device.width() / (float)device.widthMM() ) * ( himetric * 0.01 ) );
      }
      /**
       * Converts a value from pixels to himetric units.
       *
       * The paint device is required to retrieve the device size.
       * Himetric is a device-dependent unit.
       *
       * @param pixels The value to convert
       * @param device The device used to detect the size of the drawing context
       */
      inline static float        pixelsToHimetric( float pixels, QPaintDevice &device )
      {
        return ( pixels / ( (float)device.width() / (float)device.widthMM() ) / 0.01 );
      }

    private: // Private properties
      /// List of attributes of the points in the drawing
      QList<AttributeSet*>       attributeSets_;
      /// Bounding rectangle of the drawing
      QRect                      boundingRect_;
      /// Virtual drawing canvas dimensions
      QRect                      canvas_;
      /// Link to the currently used metric data
      Metrics                   *currentMetrics_;
      /// Link to the currently used point info data
      AttributeSet              *currentAttributeSet_;
      /// Link to the currently used stroke info data
      StrokeInfo                *currentStrokeInfo_;
      /// Link to the currently used transformation
      QMatrix                   *currentTransform_;
      /// Link to the default metric data
      Metrics                    defaultMetrics_;
      /// Link to the default point info data
      AttributeSet               defaultAttributeSet_;
      /// Link to the default stroke info data
      StrokeInfo                 defaultStrokeInfo_;
      /// Link to the default transformation
      QMatrix                    defaultTransform_;
      /// Last parsing error (if there is one)
      IsfError                   error_;
      /// List of registered GUIDs
      QList<QUuid>               guids_;
      /// Whether the drawing contains X coordinates or not
      bool                       hasXData_;
      /// Whether the drawing contains Y coordinates or not
      bool                       hasYData_;
      /// Whether the drawing is invalid or valid
      bool                       isNull_;
      /// Maximum GUID available in the drawing
      quint64                    maxGuid_;
      /// Maximum thickness of the strokes
      QSizeF                     maxPenSize_;
      /// List of metrics used in the drawing
      QList<Metrics*>            metrics_;
      /// Pixel size of the drawing
      QSize                      size_;
      /// List of information about the drawing's strokes
      QList<StrokeInfo*>         strokeInfo_;
      /// List of strokes composing this drawing
      QList<Stroke*>             strokes_;
      /// Transformation matrices
      QList<QMatrix*>            transforms_;

  };



}; // namespace Isf



#endif // ISFQTDRAWING_H
