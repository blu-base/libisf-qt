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

#include "isfqt-internal.h"

#include "data/datasource.h"
#include "data/multibytecoding.h"
#include "tagsparser.h"

#include <IsfQtDrawing>

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
, currentAttributeSet_( 0 )
, currentStrokeInfo_( 0 )
, currentTransform_( 0 )
, error_( ISF_ERROR_NONE )
, hasXData_( true )
, hasYData_( true )
, isNull_( true )
, maxGuid_( 0 )
{
#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Created new ISF drawing **";
#endif
}



/**
 * Destructor
 */
Drawing::~Drawing()
{
  qDeleteAll( metrics_ );
  qDeleteAll( strokeInfo_ );
  qDeleteAll( strokes_ );
  qDeleteAll( transforms_ );
  qDeleteAll( attributeSets_ );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "** Destroyed ISF drawing object **";
#endif
}



/**
 * Add new a attribute set to the drawing
 *
 * @param attributes The attribute set to add
 * @return Index of the new attribute set or -1 on failure
 */
qint32 Drawing::addAttributeSet( AttributeSet *newAttributeSet )
{
  if( newAttributeSet == 0 )
  {
    return -1;
  }

  attributeSets_.append( newAttributeSet );

  return ( attributeSets_.count() - 1 );
}



/**
 * Add a new stroke to the drawing
 *
 * @param attributes The stroke to add
 * @return Index of the new stroke or -1 on failure
 */
qint32 Drawing::addStroke( Stroke *newStroke )
{
  if( newStroke == 0 )
  {
    return -1;
  }

  strokes_.append( newStroke );

  return ( strokes_.count() - 1 );
}



/**
 * Add a new transformation to the drawing
 *
 * @param attributes The transform to add
 * @return Index of the new transform or -1 on failure
 */
qint32 Drawing::addTransform( QMatrix *newTransform )
{
  if( newTransform == 0 )
  {
    return -1;
  }

  transforms_.append( newTransform );

  return ( transforms_.count() - 1 );
}



/**
 * Clean up the drawing
 *
 * This restores the drawing to its initial state, an empty drawing.
 */
void Drawing::clear()
{
  // Clean up the internal property lists
  qDeleteAll( metrics_ );
  qDeleteAll( strokeInfo_ );
  qDeleteAll( strokes_ );
  qDeleteAll( transforms_ );
  qDeleteAll( attributeSets_ );
  metrics_      .clear();
  strokeInfo_   .clear();
  strokes_      .clear();
  transforms_   .clear();
  attributeSets_.clear();

  // Invalidate the current item pointers
  currentMetrics_      = 0;
  currentAttributeSet_ = 0;
  currentStrokeInfo_   = 0;
  currentTransform_    = 0;

  // Nullify the other properties
  boundingRect_ = QRect();
  canvas_       = QRect();
  error_        = ISF_ERROR_NONE;
  hasXData_     = true;
  hasYData_     = true;
  isNull_       = true;
  maxGuid_      = 0;
  maxPenSize_   = QSizeF();
  size_         = QSize();
}



/**
 * Remove some attribute set from the drawing
 *
 * @param index Index of the attribute set to delete
 * @return bool
 */
bool Drawing::deleteAttributeSet( quint32 index )
{
  if( (qint64)index >= attributeSets_.count() )
  {
    return false;
  }

  delete attributeSets_.takeAt( index );
  return true;
}



/**
 * Remove a stroke from the drawing
 *
 * @param index Index of the stroke to delete
 * @return bool
 */
bool Drawing::deleteStroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return false;
  }

  delete strokes_.takeAt( index );
  return true;
}



/**
 * Remove a transformation from the drawing
 *
 * @param index Index of the transform to delete
 * @return bool
 */
bool Drawing::deleteTransform( quint32 index )
{
  if( (qint64)index >= transforms_.count() )
  {
    return false;
  }

  delete transforms_.takeAt( index );
  return true;
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



/**
 * Retrieve an attribute set to manipulate it
 *
 * @param index Index of the attribute set to get
 * @return AttributeSet or 0 if not found
 */
AttributeSet *Drawing::getAttributeSet( quint32 index )
{
  if( (qint64)index >= attributeSets_.count() )
  {
    return 0;
  }

  return attributeSets_.at( index );
}



/**
 * Retrieve the attribute sets
 *
 * @return The list of existing attribute sets
 */
QList<AttributeSet*> Drawing::getAttributeSets()
{
  return attributeSets_;
}



QPixmap Drawing::getPixmap( const QColor backgroundColor )
{
  QPixmap pixmap( size_ );
  pixmap.fill( backgroundColor );
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
  currentMetrics_       = 0;
  currentAttributeSet_  = 0;
  currentStrokeInfo_    = 0;
  currentTransform_     = 0;

  int index = 0;
  foreach( const Stroke *stroke, strokes_ )
  {
    if( currentMetrics_ != stroke->metrics )
    {
      currentMetrics_ = stroke->metrics;
      // TODO need to convert all units somehow?
//       painter.setSomething( currentMetrics );
    }
    if( currentAttributeSet_ != stroke->attributes )
    {
      currentAttributeSet_ = stroke->attributes;

      float penSizePixels = Drawing::himetricToPixels( currentAttributeSet_->penSize.width(), pixmap );

      pen.setColor( currentAttributeSet_->color );
      pen.setWidthF( penSizePixels );
      painter.setPen( pen );
    }
    if( currentStrokeInfo_ != stroke->info )
    {
      currentStrokeInfo_ = stroke->info;
    }
    if( currentTransform_ != stroke->transform )
    {
      currentTransform_ = stroke->transform;
      painter.setWorldTransform( QTransform( *currentTransform_ ), true );
    }

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "Rendering stroke" << index << "containing" << stroke->points.count() << "points";
    qDebug() << "- Stroke color:" << currentAttributeSet_->color.name() << "Pen size:" << pen.widthF();
#endif

    if( stroke->points.count() > 1 )
    {
      Point lastPoint;
      foreach( const Point &point, stroke->points )
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
      Point point = stroke->points.first();
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
    pen.setColor( currentAttributeSet_->color );
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



/**
 * Retrieve a stroke to manipulate it
 *
 * @param index Index of the stroke to get
 * @return Stroke or 0 if not found
 */
Stroke *Drawing::getStroke( quint32 index )
{
  if( (qint64)index >= strokes_.count() )
  {
    return 0;
  }

  return strokes_.at( index );
}



/**
 * Retrieve the strokes
 *
 * @return The list of existing strokes
 */
QList<Stroke*> Drawing::getStrokes()
{
  return strokes_;
}



/**
 * Retrieve a transformation to manipulate it
 *
 * @param index Index of the transform to get
 * @return QMatrix or 0 if not found
 */
QMatrix *Drawing::getTransform( quint32 index )
{
  if( (qint64)index >= transforms_.count() )
  {
    return 0;
  }

  return transforms_.at( index );
}



/**
 * Retrieve the transformations
 *
 * @return The list of existing transformations
 */
QList<QMatrix*> Drawing::getTransforms()
{
  return transforms_;
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
 * Change the current attribute set
 *
 * This will change the attribute set which will be applied to the next strokes.
 *
 * @param attributeSet the new attribute set
 * @return bool
 */
bool Drawing::setCurrentAttributeSet( AttributeSet *attributeSet )
{
  if( ! attributeSets_.contains( attributeSet ) )
  {
    return false;
  }

  currentAttributeSet_ = attributeSet;

  return true;
}



/**
 * Change the current transformation
 *
 * This will change the transformation which will be applied to the next strokes.
 *
 * @param attributeSet the new transformation
 * @return bool
 */
bool Drawing::setCurrentTransform( QMatrix *transform )
{
  if( ! transforms_.contains( transform ) )
  {
    return false;
  }

  currentTransform_ = transform;

  return true;
}


