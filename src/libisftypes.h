/***************************************************************************
 *   Copyright (C) 2008 by Valerio Pilo                                    *
 *   valerio@kmess.org                                                     *
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

#ifndef LIBISFTYPES_H
#define LIBISFTYPES_H

#include <QList>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QtGlobal>



namespace Isf
{
  /**
   * Stroke drawing flags
   */
  enum StrokeFlag
  {
    FitToCurve     = 0x0001
  , IgnorePressure = 0x0004
  , IsHighlighter  = 0x0100
  , IsRectangle    = 0x0200
  };
  Q_DECLARE_FLAGS( StrokeFlags, StrokeFlag )
  Q_DECLARE_OPERATORS_FOR_FLAGS( StrokeFlags )



  /**
   * Drawing attributes for points
   */
  struct PointAttributes
  {
    /// dimensions of the pencil in Himetric units
    QSizeF penSize;
    /// color in AABBGGRR format (Alpha channel: 00 is solid, FF is transparent)
    quint32 color;
    /// mask of StrokeFlags
    StrokeFlags flags;
    /// number of strokes using those attributes
    quint32 nStrokes;
  };



  /**
   * A single point within a stroke
   */
  struct Point
  {
    /// coordinates
    QPointF position;
    /// Pressure information
    qint64  pressureLevel;
    /// Drawing attributes structure used to display the stroke
    PointAttributes drawAttrs;
  };



  /**
   * A pen stroke
   */
  struct Stroke
  {
    /// List of points contained in this stroke
    QList<Point> points;
    /// Its bounding box, ie the minimum sized rectangle which contains all of the stroke's points
    QRectF boundingBox;
  };



  /**
   * A complete ISF image
   */
  struct Image
  {
    /// Bounding box, the minimum sized rectangle which contains all the strokes
    QRectF boundingBox;
    /// Dimensions of the image, or of the area used to draw the ISF
    QSize size;
    /// highest pencil dimension used
    QSizeF maxPenSize;

    /// collection of strokes
    QList<Stroke> strokes;
  };



}



#endif
