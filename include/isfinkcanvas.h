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

#ifndef INKCANVAS_H
#define INKCANVAS_H

#include <QWidget>

#include "isfqt.h"
#include "isfqtdrawing.h"

namespace Isf
{

/**
 * This is an Ink-editing control.
 *
 * It manipulates data in ISF format and is capable of
 * drawing strokes, erasing strokes and setting pen size and color. It can also return the
 * drawn image as a QImage.
 *
 * @author Adam Goossens (adam@kmess.org)
 */
class InkCanvas : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The various types of pens supported by InkCanvas.
     *
     * Currently only two are supported; DrawingPen (for drawing Ink)
     * and EraserPen, for erasing ink stroke-by-stroke.
     */
    enum PenType
    {
      DrawingPen,
      EraserPen
    };

  public:
     /**
      * Create a new InkCanvas widget that allows you to draw Ink on a canvas.
      *
      * To retrieve a QImage that contains the image drawn, use the function
      * image().
      *
      * To get the Isf::Drawing instance for your ink, use drawing().
      *
      * @param parent The parent widget.
      */
                        InkCanvas( QWidget *parent = 0 );
    // Destructor
                        ~InkCanvas();
    // Get a QByteArray representing the Drawing instance.
    QByteArray          bytes();
    // Return the drawn ISF image
    Isf::Drawing       *drawing();
    // Return the Ink image as a QImage
    QImage              image();
    // Returns true if the image is empty
    bool                isEmpty();
    // Get current pen color
    QColor              penColor();
    // Get current pen size
    int                 penSize();
    // Get the current pen type
    PenType             penType();
    // Save the drawing to a QIODevice, optionally base64 encoded.
    void                save( QIODevice &device, bool base64 = false );
    // Change the current ink drawing.
    void                setDrawing( Isf::Drawing *drawing );
    // Provide a sensible default size
    virtual QSize       sizeHint() const;

  public slots:
    // Clear the current image
    void                clear();
    // Set the canvas background color
    void                setCanvasColor( QColor newColor );
    // Change the colour for the pen
    void                setPenColor( QColor newColor );
    // Change the size of the pen, in pixels
    void                setPenSize( int pixels );
    // Change the pen to an eraser that erases strokes
    void                setPenType( PenType type );

  protected: // protected methods
    // Implements all events to grep the mouse pointer
    void                mousePressEvent( QMouseEvent *event );
    void                mouseMoveEvent( QMouseEvent *event );
    void                mouseReleaseEvent( QMouseEvent *event );
    void                paintEvent( QPaintEvent *event );
    void                resizeEvent( QResizeEvent *event );

  private: // private methods
    // Draw a line from start point to last point
    void                drawLineTo( const QPoint &endPoint );
    // Clear the buffer pixmap
    void                clearBuffer();
    // Update the cursor
    void                updateCursor();

  private: // private attribute
    // Color of the canvas background
    QColor              canvasColor_;
    // Color of current pen
    QColor              color_;
    // Cursor pixmap
    QPixmap             cursorPixmap_;
    // Current cursor
    QCursor             cursor_;
    // It's true if the erase brush was selected.
    bool                erasingImage_;
    // Current drawing being manipulated.
    Isf::Drawing       *drawing_;
    Isf::Drawing        initialDrawing_;
    // Last point where the mouse pointer was released
    QPoint              lastPoint_;
    // True if the user is painting
    bool                scribbling_;
    // Pen size, in pixels.
    int                 penSize_;
    // Pen type
    PenType             penType_;
    // The current stroke being drawn
    Isf::Stroke         *currentStroke_;
    // The pixmap buffer where in progress strokes are drawn.
    QPixmap             bufferPixmap_;
    // Cache pixmap so the Ink isn't redrawn on every mouse move.
    QPixmap             isfCachePixmap_;
    // Dirty value used to update isf cache pixmap.
    bool                drawingDirty_;
  signals:
    // Emitted when the ink was modified
    void                inkChanged();
};

}; // namespace Isf
#endif
