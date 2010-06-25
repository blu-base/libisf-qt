/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
 *                                                                         *
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

#ifndef ISFINKCANVAS_H
#define ISFINKCANVAS_H

#include <QWidget>

#include "isfqt.h"
#include "isfqtdrawing.h"



namespace Isf
{



  /**
   * @class InkCanvas
   * @brief This is a control designed for the drawing and display of Ink.
   *
   * InkCanvas is used for drawing and displaying Ink. The currently displayed Ink drawing can be retrieved
   * or set using the drawing() and setDrawing() methods.
   *
   * To set the properties of the current pen, use the setPenColor(), setPenSize() and setPenType() methods.
   * The supported pen types are outlined in the PenType enumeration documentation.
   *
   * Example:
   *
   * \code
   * InkCanvas* canvas = new Isf::InkCanvas( this );
   * canvas->setPenColor( Qt::blue );
   * canvas->setPenSize( 10 );
   *
   * // the pen is now 10 pixels thick and blue.
   *
   * canvas->setPenType( EraserPen );
   *
   * // now an eraser will be used and strokes can be erased individually.
   * \endcode
   *
   * To return the currently displayed Ink as a QImage, use image(). To return the raw ISF data, suitable
   * for saving to disk or sending over a network, use bytes().
   *
   * To write the ISF data directly to a QIODevice, such as a file, use the save() method.
   *
   * Connect to the inkChanged() signal to be notified when the drawing displayed on the canvas changes,
   * either through strokes being drawn or deleted, or the current drawing being changed.
   *
   * @author Adam Goossens (adam@kmess.org)
   */
  class InkCanvas : public QWidget
  {
    Q_OBJECT


    public: // public enumerations
      /**
       * The various types of pens supported by InkCanvas.
       *
       * The DrawingPen is a standard pen for drawing new strokes onto the canvas.
       * The pen size and color are controlled by setPenSize() and setPenColor().
       *
       * The EraserPen is used for erasing individual strokes. Pixel-by-pixel erase is not
       * yet supported.
       */
      enum PenType
      {
        DrawingPen,  ///< Used to draw Ink
        EraserPen    ///< Used to erase by stroke
      };

    public: // public constructors
                          InkCanvas( QWidget* = 0 );
                         ~InkCanvas();

    public: // public methods
      QByteArray          bytes();
      Isf::Drawing*       drawing();
      QImage              image();
      bool                isEmpty();
      QColor              penColor();
      int                 penSize();
      PenType             penType();
      void                save( QIODevice&, bool = false );
      void                setDrawing( Isf::Drawing* );
      virtual QSize       sizeHint() const;

    public slots:
      void                clear();
      void                setCanvasColor( QColor );
      void                setPenColor( QColor );
      void                setPenSize( int );
      void                setPenType( PenType );

    protected: // protected methods
      void                mousePressEvent( QMouseEvent* );
      void                mouseMoveEvent( QMouseEvent* );
      void                mouseReleaseEvent( QMouseEvent* );
      void                paintEvent( QPaintEvent* );
      void                resizeEvent( QResizeEvent* );

    private: // private methods
      void                drawLineTo( const QPoint& );
      void                clearBuffer();
      void                updateCursor();

    private: // private attributes
      /// Color of the canvas background
      QColor              canvasColor_;
      /// Color of current pen
      QColor              color_;
      /// Cursor pixmap
      QPixmap             cursorPixmap_;
      /// Current cursor
      QCursor             cursor_;
      /// It's true if the erase brush was selected
      bool                erasingImage_;
      /// Current drawing being manipulated
      Isf::Drawing*       drawing_;
      /// Initial drawing, can be overridden with drawing_
      Isf::Drawing        initialDrawing_;
      /// Last point where the mouse pointer was released
      QPoint              lastPoint_;
      /// True if the user is painting
      bool                scribbling_;
      /// Pen size, in pixels.
      int                 penSize_;
      /// Pen type
      PenType             penType_;
      /// The current stroke being drawn
      Isf::Stroke*        currentStroke_;
      /// The pixmap buffer where in progress strokes are drawn
      QPixmap             bufferPixmap_;
      /// Cache pixmap so the Ink isn't redrawn on every mouse move
      QPixmap             isfCachePixmap_;
      /// Dirty value used to update isf cache pixmap.
      bool                drawingDirty_;

    signals:
      /// Emitted when the ink representation is modified (stroke drawn,
      /// stroke deleted, current drawing changed).
      void                inkChanged();
  };



}; // namespace Isf



#endif // ISFINKCANVAS_H
