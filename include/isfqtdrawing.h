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
#include <IsfQtStroke>

#include <QMap>
#include <QMatrix>
#include <QPaintDevice>
#include <QUuid>
#include <QPixmap>


// Forward declarations
class QByteArray;

class AttributeSet;



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
                                 Drawing( const Drawing & );
                                ~Drawing();

    public: // public state retrieval methods
      QRect                      boundingRect() const;
      void                       clear();
      IsfError                   error() const;
      qint32                     indexOfStroke( const Stroke* ) const;
      bool                       isNull() const;
      QPixmap                    pixmap( const QColor = Qt::transparent );
      void                       setBoundingRect( QRect );
      QSize                      size() const;
      Stroke*                    stroke( quint32 );
      Stroke*                    strokeAtPoint( const QPoint& );
      const QList<Stroke*>       strokes();

    public: // public manipulation methods
      qint32                     addStroke( Stroke* );
      qint32                     addStroke( PointList = PointList() );
      bool                       deleteStroke( quint32 );
      bool                       deleteStroke( Stroke* );

    private:
      void                       updateBoundingRect();

    private: // Private properties
      /// Bounding rectangle of the drawing
      QRect                      boundingRect_;
      /// Cached bounding rectangle
      QRect                      cacheRect_;
      /// The cached pixmap
      QPixmap                    cachePixmap_;
      /// Virtual drawing canvas dimensions
      QRect                      canvas_;
      /// A list of strokes that need to be repainted.
      QList<Stroke*>             changedStrokes_;
      /// Is the drawing dirty? i.e, requires repainting?
      bool                       dirty_;
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
      /// List of strokes composing this drawing
      QList<Stroke*>             strokes_;

  };



}; // namespace Isf



#endif // ISFQTDRAWING_H
