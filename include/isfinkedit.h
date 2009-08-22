/***************************************************************************
                          inkedit.cpp -  description
                             -------------------
    begin                : Wed May 14 2008
    copyright            : (C) 2008 by Antonio Nastasi
                         : (c) 2009 by Adam Goossens
    email                : sifcenter@gmail.com
                         : adam@kmess.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INKEDIT_H
#define INKEDIT_H

#include <QWidget>

#include "isfqt.h"

namespace Isf
{
  
/**
 * This is an Ink-editing control. It manipulates data in ISF format and is capable of 
 * drawing strokes, erasing strokes and setting pen size and color. It can also return the
 * drawn image as a QImage.
 *
 * @author Adam Goossens (adam@kmess.org)
 */
class InkEdit : public QWidget
{
  Q_OBJECT

  public:
    // Constructor
                        InkEdit( QWidget *parent = 0 );
    // Return the drawn ISF image
    Isf::Drawing       *getDrawing();
    // Return the Ink image as a QImage
    QImage              getImage();
    // Change the colour for the pen
    void                setPenColor( QColor newColor );
    // Change the size of the pen, in pixels
    void                setPenSize( uint pixels );
    // Erase the stroke at a given point.
    void                eraseStroke( QPoint &strokePoint );
    // Returns true if the image is empty
    bool                isEmpty();
    // Change the current ink drawing.
    void                setDrawing( Isf::Drawing *drawing );
    // Provide a sensible default size
    virtual QSize       sizeHint() const;

  public slots:
    // Clear the current image
    void                clear();

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
    
  private: // private attribute
    // Color of current pen
    QColor              color_;
    // It's true if the erase brush was selected.
    bool                erasingImage_;
    // Current image data
    Isf::Drawing        *drawing_;
    // Last point where the mouse pointer was released
    QPoint              lastPoint_;
    // True if the user is painting
    bool                scribbling_;
    // Pen size, in pixels.
    uint                penSize_;
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
