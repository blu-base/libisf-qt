/***************************************************************************
 *   Copyright (C) 2009 by Valerio Pilo                                    *
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

#include "tagswriter.h"

#include "isfqt-internal.h"

#include "data/compression.h"
#include "data/datasource.h"
#include "data/multibytecoding.h"

#include <IsfQtDrawing>


using namespace Isf;
using namespace Isf::Compress;



/// Write the drawing dimensions
IsfError TagsWriter::addHiMetricSize( DataSource &source, const Drawing &drawing )
{
  encodeUInt( source, TAG_HIMETRIC_SIZE );

  // This tag has a fixed size of 2 bytes. Nevertheless, it must have a payload size field. wtf.
  encodeUInt( source, 2 );

  encodeInt( source, drawing.size_.width () );
  encodeInt( source, drawing.size_.height() );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added drawing dimensions:" << drawing.size_;
#endif

  return ISF_ERROR_NONE;
}



/// Write a table of points attributes
IsfError TagsWriter::addAttributeTable( DataSource &source, const Drawing &drawing )
{
  QByteArray   blockData;
  QByteArray   tagContents;
  AttributeSet defaultAttributeSet;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.attributeSets_.count() << "attributes...";
  quint8 counter = 0;
#endif

  foreach( const AttributeSet *info, drawing.attributeSets_ )
  {
    // Add the color to the attribute block
    if( info->color != defaultAttributeSet.color )
    {
      blockData.append( encodeUInt( GUID_COLORREF ) );

      // Prepare the color value, it needs to be stored in BGR format
      quint64 value = (info->color.blue()  << 24)
                    | (info->color.green() << 16)
                    | (info->color.red()   <<  8);
      blockData.append( encodeUInt( value ) );

      // Add the transparency if needed
      if( info->color.alpha() < 255 )
      {
        blockData.append( encodeUInt( GUID_TRANSPARENCY ) );
        blockData.append( encodeUInt( info->color.alpha() ) );
      }
    }

    // Add the pen size
    if( info->penSize != defaultAttributeSet.penSize )
    {
      blockData.append( encodeUInt( GUID_PEN_WIDTH ) );
      blockData.append( encodeUInt( info->penSize.width() ) );

      if( info->penSize.width() != info->penSize.height() )
      {
        blockData.append( encodeUInt( GUID_PEN_HEIGHT ) );
        blockData.append( encodeUInt( info->penSize.height() ) );
      }
    }

    // Add the other drawing flags
    if( info->flags != defaultAttributeSet.flags )
    {
      StrokeFlags flags = info->flags;
      if( flags & IsRectangle )
      {
        blockData.append( encodeUInt( GUID_PEN_TIP ) );
        blockData.append( encodeUInt( 0 ) ); // Value unknown, is the tag enough?
        flags ^= IsRectangle;
      }
/*
      if( flags & IsHighlighter )
      {
        blockData.append( encodeUInt( GUID_ROP ) );
        blockData.append( QByteArray( 4, '\0' ) ); // Value unknown
        flags ^= IsHighlighter;
      }
*/

      // Copy the other flags as they are
      blockData.append( encodeUInt( GUID_DRAWING_FLAGS ) );
      blockData.append( encodeUInt( flags ) );
    }


    blockData.prepend( encodeUInt( blockData.size() ) );
    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added attribute block #" << ++counter;
#endif
  }

  if( drawing.attributeSets_.count() > 1 )
  {
    tagContents.prepend( encodeUInt( tagContents.size() ) );
    tagContents.prepend( encodeUInt( TAG_DRAW_ATTRS_TABLE ) );
  }
  else
  {
    tagContents.prepend( encodeUInt( TAG_DRAW_ATTRS_BLOCK ) );
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}



/// Write a table of transformation matrices
IsfError TagsWriter::addTransformationTable( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;
  quint64    transformTag;

  /**
   * QTransform is too complex for the 2D transformations we need to do;
   * but QMatrix is too simple: it doesn't 'remember' rotations. So we
   * save rotations as generic transformations.
   * Possible solutions:
   * 1) Create a struct containing the transform matrix data and use
   *    that instead of QMatrices;
   * 2) Switch (again!) to QTransform and fix the inconsistencies.
   */

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.transforms_.count() << "transformations...";
  quint8 counter = 0;
#endif

  foreach( const QMatrix *trans, drawing.transforms_ )
  {
    /**
     * All transforms are written backwards because they're stored my most
     * significant value first.
     */

    // Detect translation
    if( trans->dx() || trans->dy() )
    {
      // Detect scaling
      if( trans->m11() || trans->m22() )
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_SCALE_AND_TRANSLATE";
#endif
        transformTag = TAG_TRANSFORM_SCALE_AND_TRANSLATE;
        blockData.append( encodeFloat( trans->dy() ) );
        blockData.append( encodeFloat( trans->dx() ) );
        blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale X:" << trans->m11()
               << "Scale Y:" << trans->m22()
               << "Translate X:" << trans->dx()
               << "Translate Y:" << trans->dy();
#endif
      }
      else
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_TRANSLATE";
#endif
        transformTag = TAG_TRANSFORM_TRANSLATE;
        blockData.append( encodeFloat( trans->dy() ) );
        blockData.append( encodeFloat( trans->dx() ) );
      }
    }
    else
    // Detect scaling
    if( trans->m11() || trans->m22() )
    {
      if( trans->m11() == trans->m22() )
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_ISOTROPIC_SCALE";
#endif
        transformTag = TAG_TRANSFORM_ISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      }
      else
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_ANISOTROPIC_SCALE";
#endif
        transformTag = TAG_TRANSFORM_ANISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      }
    }
    else
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM";
#endif
      transformTag = TAG_TRANSFORM;
      blockData.append( encodeFloat( trans->dy () ) );
      blockData.append( encodeFloat( trans->dx () ) );
      blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      blockData.append( encodeFloat( .0f ) );
      blockData.append( encodeFloat( .0f ) );
      blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
    }

    blockData.prepend( encodeUInt( transformTag ) );

    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added transform #" << ++counter;
#endif
  }

  if( drawing.transforms_.count() > 1 )
  {
    tagContents.prepend( encodeUInt( tagContents.size() ) );
    tagContents.prepend( encodeUInt( TAG_TRANSFORM_TABLE ) );
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}



/// Write the strokes
IsfError TagsWriter::addStrokes( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.strokes_.count() << "strokes...";
  quint8 counter = 0;
#endif

  // Last set of attibutes applied to a stroke
  Metrics      *currentMetrics      = 0;
  AttributeSet *currentAttributeSet = 0;
  QMatrix      *currentTransform    = 0;

  foreach( const Stroke *stroke, drawing.strokes_ )
  {
    // There is more than one set of metrics, assign each stroke to its own
    if( drawing.metrics_.count() > 1 )
    {
      // Only write a MIDX if this stroke needs different metrics than the last stroke
      if( currentMetrics != stroke->metrics )
      {
        currentMetrics = stroke->metrics;
        blockData.append( encodeUInt( TAG_MIDX ) );
        blockData.append( encodeUInt( drawing.metrics_.indexOf( stroke->metrics ) ) );
      }
    }
    // There is more than one set of attributes, assign each stroke to its own
    if( drawing.attributeSets_.count() > 1 )
    {
      // Only write a DIDX if this stroke needs a different attribute set than the last stroke
      if( currentAttributeSet != stroke->attributes )
      {
        currentAttributeSet = stroke->attributes;
        blockData.append( encodeUInt( TAG_DIDX ) );
        blockData.append( encodeUInt( drawing.attributeSets_.indexOf( stroke->attributes ) ) );
      }
    }
    // There is more than one set of attributes, assign each stroke to its own
    if( drawing.metrics_.count() > 1 )
    {
      // Only write a TIDX if this stroke needs a different transform than the last stroke
      if( currentTransform != stroke->transform )
      {
        currentTransform = stroke->transform;
        blockData.append( encodeUInt( TAG_TIDX ) );
        blockData.append( encodeUInt( drawing.transforms_.indexOf( stroke->transform ) ) );
      }
    }

    // Flush the index tags
    if( ! blockData.isEmpty() )
    {
      tagContents.append( blockData );
      blockData.clear();
    }

    // Write this stroke in the stream

    QList<qint64> xPoints, yPoints;
    foreach( const Point &point, stroke->points )
    {
      xPoints.append( point.position.x() );
      yPoints.append( point.position.y() );
    }

    deflate( blockData, xPoints, Points );
    deflate( blockData, yPoints, Points );

    // The stroke is made by tag, then payload size, then number of points, then
    // the compressed points data
    blockData.prepend( encodeUInt( stroke->points.count() ) );
    blockData.prepend( encodeUInt( blockData.size() ) );
    blockData.prepend( encodeUInt( TAG_STROKE ) );

    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added stroke #" << ++counter;
#endif
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}


