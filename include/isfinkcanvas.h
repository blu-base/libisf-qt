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

#ifndef ISFINKCANVAS_H
#define ISFINKCANVAS_H

#include <QWidget>

#include "isfqt.h"
#include "isfqtdrawing.h"



namespace Isf
{



  /**
   * @class InkCanvas
   * @brief This is an Ink-editing control.
   *
   * It manipulates data in ISF format and is capable of
   * drawing strokes, erasing strokes and setting pen size and color.
   * It can also return the drawn image as a QImage.
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
       * Erasing by pixel is not supported yet.
       */
      enum PenType
      {
        DrawingPen,  ///< Used to draw Ink
        EraserPen    ///< Used to erase by stroke
      };

    public: // public constructors
                          InkCanvas( QWidget *parent = 0 );
                        ~InkCanvas();

    public: // public methods
      QByteArray          bytes();
      Isf::Drawing       *drawing();
      QImage              image();
      bool                isEmpty();
      QColor              penColor();
      int                 penSize();
      PenType             penType();
      void                save( QIODevice &device, bool base64 = false );
      void                setDrawing( Isf::Drawing *drawing );
      virtual QSize       sizeHint() const;

    public slots:
      void                clear();
      void                setCanvasColor( QColor newColor );
      void                setPenColor( QColor newColor );
      void                setPenSize( int pixels );
      void                setPenType( PenType type );

    protected: // protected methods
      void                mousePressEvent( QMouseEvent *event );
      void                mouseMoveEvent( QMouseEvent *event );
      void                mouseReleaseEvent( QMouseEvent *event );
      void                paintEvent( QPaintEvent *event );
      void                resizeEvent( QResizeEvent *event );

    private: // private methods
      void                drawLineTo( const QPoint &endPoint );
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
      Isf::Drawing       *drawing_;
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
      Isf::Stroke         *currentStroke_;
      /// The pixmap buffer where in progress strokes are drawn
      QPixmap             bufferPixmap_;
      /// Cache pixmap so the Ink isn't redrawn on every mouse move
      QPixmap             isfCachePixmap_;
      /// Dirty value used to update isf cache pixmap.
      bool                drawingDirty_;

    signals:
      /// Emitted when the ink representation was modified
      void                inkChanged();
  };



}; // namespace Isf



#endif // ISFINKCANVAS_H
