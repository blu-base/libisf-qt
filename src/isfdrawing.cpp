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

#include <isfdrawing.h>

#include "isfqt-internal.h"

#include "data/datasource.h"
#include "data/multibytecoding.h"
#include "tagsparser.h"

#include <QPainter>
#include <QPixmap>


using namespace Isf;
using namespace Compress;



/**
 * Construct a new NULL Drawing instance.
 *
 * When you add a new stroke to the drawing the instance becomes non-NULL.
 */
Drawing::Drawing()
: currentMetrics_( 0 )
, currentPointInfo_( 0 )
, currentStrokeInfo_( 0 )
, currentTransform_( 0 )
, error_( ISF_ERROR_NONE )
, hasXData_( true )
, hasYData_( true )
, isNull_( true )
, maxGuid_( 0 )
{
}



/**
 * Return True if this instance of Drawing is invalid (NULL), False otherwise.
 *
 * @return True if this is a NULL Drawing, FALSE otherwise.
 */
bool Drawing::isNull() const
{
  return isNull_;
}



/**
 * Return the last error that has occurred
 *
 * If nothing went wrong, this returns ISF_ERROR_NONE.
 *
 * @return The last ISF error status
 */
IsfError Drawing::error() const
{
  return error_;
}



QPixmap Drawing::getPixmap()
{
  QPixmap pixmap( size_ );
  pixmap.fill( Qt::white );
  QPainter painter( &pixmap );

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering a drawing of size" << size_;
#endif

  painter.setWindow( boundingRect_ );
  painter.setWorldMatrixEnabled( true );
  painter.setRenderHints(   QPainter::Antialiasing
                          | QPainter::SmoothPixmapTransform
                          | QPainter::TextAntialiasing,
                          true );

  QPen pen;
  pen.setStyle    ( Qt::SolidLine );
  pen.setCapStyle ( Qt::RoundCap  );
  pen.setJoinStyle( Qt::RoundJoin );


#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "The drawing contains" << strokes_.count() << "strokes.";
#endif

  // Keep record of the currently used properties, to avoid re-setting them for each stroke
  currentMetrics_    = 0;
  currentPointInfo_  = 0;
  currentStrokeInfo_ = 0;
  currentTransform_  = 0;

  int index = 0;
  foreach( const Stroke &stroke, strokes_ )
  {
    if( currentMetrics_ != stroke.metrics )
    {
      currentMetrics_ = stroke.metrics;
      // TODO need to convert all units somehow?
//       painter.setSomething( currentMetrics );
    }
    if( currentPointInfo_ != stroke.attributes )
    {
      currentPointInfo_ = stroke.attributes;

      float penSizePixels = Drawing::himetricToPixels( currentPointInfo_->penSize.width(), pixmap );

      pen.setColor( currentPointInfo_->color );
      pen.setWidthF( penSizePixels );
      painter.setPen( pen );
    }
    if( currentStrokeInfo_ != stroke.info )
    {
      currentStrokeInfo_ = stroke.info;
    }
    if( currentTransform_ != stroke.transform )
    {
      currentTransform_ = stroke.transform;
      painter.setWorldTransform( *currentTransform_, true );
    }

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke.points.count() << "points";
    qDebug() << "- Stroke color:" << currentPointInfo_->color.name() << "Pen size:" << pen.widthF();
#endif

    if( stroke.points.count() > 1 )
    {
      Point lastPoint;
      foreach( const Point &point, stroke.points )
      {
//       qDebug() << "Point:" << point.position;

        if( lastPoint.position.isNull() )
        {
          lastPoint = point;
          continue;
        }

        if( currentStrokeInfo_->hasPressureData )
        {
          // FIXME Ignoring pressure data - need to find out how pressure must be applied
//           pen.setWidth( pen.widthF() + point.pressureLevel );
//           painter.setPen( pen );
        }

        // How nice of QPainter! Lines drawn from and to the same point
        // won't be drawn at all
        if( point.position == lastPoint.position )
        {
          painter.drawPoint( point.position );
        }
        else
        {
          painter.drawLine( lastPoint.position, point.position );
        }

        lastPoint = point;
      }
    }
    else
    {
      Point point = stroke.points.first();
      if( currentStrokeInfo_->hasPressureData )
      {
        // FIXME Ignoring pressure data - need to find out how pressure must be applied
//         pen.setWidth( pen.widthF() + point.pressureLevel );
//         painter.setPen( pen );
      }

//       qDebug() << "Point:" << point.position;
      painter.drawPoint( point.position );
    }

/*
#ifdef ISFQT_DEBUG_VERBOSE
    // Draw the stroke number next to each one, for debugging purposes
    pen.setColor( QColor( Qt::red ) );
    painter.setPen( pen );
    painter.drawText( stroke.points.first().position, QString::number( index ) );
    pen.setColor( currentPointInfo_->color );
    painter.setPen( pen );
#endif
*/

    ++index;
  }

  painter.end();

#ifdef ISFQT_DEBUG
  qDebug() << "Rendering complete.";
#endif

  return pixmap;
}


