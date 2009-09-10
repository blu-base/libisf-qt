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

#include "tagswriter.h"

#include "isfqt-internal.h"

#include "data/compression.h"
#include "data/datasource.h"
#include "data/multibytecoding.h"

#include <IsfQtDrawing>


using namespace Isf;
using namespace Isf::Compress;



/**
 * Write the persistent format tag.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
IsfError TagsWriter::addPersistentFormat( DataSource &source, const Drawing &drawing )
{
  Q_UNUSED( drawing );

  QByteArray tagContents;

  tagContents.append( encodeUInt( ISF_PERSISTENT_FORMAT_VERSION ) );

  tagContents.prepend( encodeUInt( tagContents.size() ) );
  tagContents.prepend( encodeUInt( TAG_PERSISTENT_FORMAT ) );

  source.append( tagContents );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Persistent Format version";
#endif

  return ISF_ERROR_NONE;
}



/**
 * Write the drawing dimensions.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
IsfError TagsWriter::addHiMetricSize( DataSource &source, const Drawing &drawing )
{
  QByteArray tagContents;

  tagContents.append( encodeInt( drawing.size_.width () ) );
  tagContents.append( encodeInt( drawing.size_.height() ) );

  encodeUInt( source, TAG_HIMETRIC_SIZE );
  encodeUInt( source, tagContents.size() );
  source.append( tagContents );

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Added drawing dimensions:" << drawing.size_;
#endif

  return ISF_ERROR_NONE;
}



/**
 * Write a table (or a block only) of points attributes.
 *
 * If there is only one block, no table is outputted into the stream, just
 * that block.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
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

      // Prepare the color value, it needs to be stored in BGR format,
      // in the 24 least significant bits
      quint64 value = ( info->color.blue () << 16 )
                    | ( info->color.green() <<  8 )
                    | ( info->color.red  () <<  0 );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Color:" << info->color.name();
#endif
      blockData.append( encodeUInt( value ) );

      // Add the transparency if needed
      if( info->color.alpha() < 255 )
      {
        blockData.append( encodeUInt( GUID_TRANSPARENCY ) );
        blockData.append( encodeUInt( info->color.alpha() ) );
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Alpha:" << info->color.alpha();
#endif
      }
    }

    // Add the pen size
    if( info->penSize != defaultAttributeSet.penSize )
    {
      blockData.append( encodeUInt( GUID_PEN_WIDTH ) );
      blockData.append( encodeUInt( info->penSize.width() * HiMetricToPixel ) );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Pen width:" << ( info->penSize.width() * HiMetricToPixel );
#endif
      if( info->penSize.width() != info->penSize.height() )
      {
        blockData.append( encodeUInt( GUID_PEN_HEIGHT ) );
        blockData.append( encodeUInt( info->penSize.height() * HiMetricToPixel ) );
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Pen height:" << ( info->penSize.height() * HiMetricToPixel );
#endif
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
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - isRectangle flag";
#endif
      }
/*
      if( flags & IsHighlighter )
      {
        blockData.append( encodeUInt( GUID_ROP ) );
        blockData.append( QByteArray( 4, '\0' ) ); // Value unknown
        flags ^= IsHighlighter;
      }
*/

      // Add FitToCurve too for now, as a test
      flags |= FitToCurve;

      // Copy the other flags as they are
      blockData.append( encodeUInt( GUID_DRAWING_FLAGS ) );
      blockData.append( encodeUInt( flags ) );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Flags bitfield:" << flags;
#endif
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



/**
 * Write a table (or a block only) of metrics.
 *
 * If there is only one block, no table is outputted into the stream, just
 * that block.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
IsfError TagsWriter::addMetricsTable( DataSource &source, const Drawing &drawing )
{
  QByteArray   metricData;
  QByteArray   metricBlockData;
  QByteArray   tagData;
  Metrics      defaultMetrics;
  Metric      *defaultMetric;

#ifdef ISFQT_DEBUG_VERBOSE
  qDebug() << "- Adding" << drawing.metrics_.count() << "metrics...";
  quint8 counter = 0;
#endif

  foreach( const Metrics *metrics, drawing.metrics_ )
  {
    QMapIterator<int,Metric> it( metrics->items );
    while( it.hasNext() )
    {
      it.next();
      const quint64 &property = it.key  ();
      const Metric  &metric   = it.value();

      // Skip metrics which are set to the default
      defaultMetric = &defaultMetrics.items[ property ];
      if( defaultMetric->min        == metric.min
      &&  defaultMetric->max        == metric.max
      &&  defaultMetric->units      == metric.units
      &&  defaultMetric->resolution == metric.resolution )
      {
        continue;
      }

      // Write the metric data
      metricData.append( encodeInt( metric.min ) );
      metricData.append( encodeInt( metric.max ) );
      metricData.append( (uchar) metric.units );
      metricData.append( encodeFloat( metric.resolution ) );

      // Write how much data is there
      metricData.prepend( encodeUInt( metricData.size() ) );

      // Write the property ID
      metricData.prepend( encodeUInt( property ) );

      // Flush the metric
      metricBlockData.append( metricData );
      metricData.clear();
    }

    // Flush the metrics block
    tagData.append( encodeUInt( metricBlockData.size() ) );
    tagData.append( metricBlockData );
    metricBlockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added metrics block #" << ++counter;
#endif
  }

  if( drawing.metrics_.count() > 1 )
  {
    tagData.prepend( encodeUInt( TAG_METRIC_TABLE ) );
  }
  else if ( drawing.metrics_.count() == 1 )
  {
    tagData.prepend( encodeUInt( TAG_METRIC_BLOCK ) );
  }
  // else: don't do anything.

  source.append( tagData );

  return ISF_ERROR_NONE;
}



/**
 * Write a table (or a block only) of transformation matrices.
 *
 * If there is only one block, no table is outputted into the stream, just
 * that block.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
IsfError TagsWriter::addTransformationTable( DataSource &source, const Drawing &drawing )
{
  QByteArray blockData;
  QByteArray tagContents;
  quint64    transformTag;

  /*
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
    /*
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
        qDebug() << "  - Transformation details - "
                 << "Scale X:" << trans->m11()
                 << "Scale Y:" << trans->m22()
                 << "Translate X:" << trans->dx()
                 << "Translate Y:" << trans->dy();
#endif
        transformTag = TAG_TRANSFORM_SCALE_AND_TRANSLATE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->dx()  / trans->m11() ) );
        blockData.append( encodeFloat( trans->dy()  / trans->m22() ) );
      }
      else
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_TRANSLATE";
        qDebug() << "  - Transformation details - "
                << "Translate X:" << trans->dx()
                << "Translate Y:" << trans->dy();
#endif
        transformTag = TAG_TRANSFORM_TRANSLATE;
        blockData.append( encodeFloat( trans->dx() ) );
        blockData.append( encodeFloat( trans->dy() ) );
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
        qDebug() << "  - Transformation details - "
                 << "Scale:" << (trans->m11() * HiMetricToPixel);
#endif
        transformTag = TAG_TRANSFORM_ISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      }
      else
      {
#ifdef ISFQT_DEBUG_VERBOSE
        qDebug() << "  - Transform: TAG_TRANSFORM_ANISOTROPIC_SCALE";
        qDebug() << "  - Transformation details - "
                 << "Scale X:" << trans->m11()
                 << "Scale Y:" << trans->m22();
#endif
        transformTag = TAG_TRANSFORM_ANISOTROPIC_SCALE;
        blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
        blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      }
    }
    else
    {
#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "  - Transform: TAG_TRANSFORM";
#endif
      transformTag = TAG_TRANSFORM;
      blockData.append( encodeFloat( trans->m11() * HiMetricToPixel ) );
      blockData.append( encodeFloat( .0f ) );
      blockData.append( encodeFloat( .0f ) );
      blockData.append( encodeFloat( trans->m22() * HiMetricToPixel ) );
      blockData.append( encodeFloat( trans->dx () / trans->m11() ) );
      blockData.append( encodeFloat( trans->dy () / trans->m22() ) );

#ifdef ISFQT_DEBUG_VERBOSE
      qDebug() << "- Transformation details - "
               << "Scale X:" << trans->m11()
               << "Shear X:" << trans->m21()
               << "Scale Y:" << trans->m22()
               << "Shear Y:" << trans->m12()
               << "Translate X:" << trans->dx()
               << "Translate Y:" << trans->dy();
#endif
    }

    blockData.prepend( encodeUInt( transformTag ) );

    tagContents.append( blockData );
    blockData.clear();

#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added transform #" << ++counter;
#endif
  }


  if ( drawing.transforms_.size() == 0 )
  {
#ifdef ISFQT_DEBUG_VERBOSE
    qDebug() << "- Added default transformation";
#endif

    // write the default transform.
    const QMatrix *transform = &drawing.defaultTransform_;
    tagContents.append( TAG_TRANSFORM_ISOTROPIC_SCALE );
    tagContents.append( encodeFloat( transform->m11() * HiMetricToPixel ) );
  }
  else if ( drawing.transforms_.size() > 1 )
  {
    tagContents.prepend( encodeUInt( tagContents.size() ) );
    tagContents.prepend( encodeUInt( TAG_TRANSFORM_TABLE ) );
  }

  source.append( tagContents );

  return ISF_ERROR_NONE;
}



/**
 * Write the strokes.
 *
 * @param source Data Source where to write bytes to
 * @param drawing Drawing from which to obtain the data to write
 * @return IsfError
 */
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
      // Make sure that the first strokes use the first metrics list (write a MIDX only
      // when needed)
      if( currentMetrics == 0 )
      {
        currentMetrics = drawing.metrics_.first();
      }

      // Only write a MIDX if this stroke needs different metrics than the last stroke
      if( currentMetrics != stroke->metrics && stroke->metrics != 0 )
      {
        currentMetrics = stroke->metrics;
        blockData.append( encodeUInt( TAG_MIDX ) );
        blockData.append( encodeUInt( drawing.metrics_.indexOf( stroke->metrics ) ) );
      }
    }

    // There is more than one set of attributes, assign each stroke to its own
    if( drawing.attributeSets_.count() > 1 )
    {
      // Make sure that the first strokes use the first attribute set (write a DIDX only
      // when needed)
      if( currentAttributeSet == 0 )
      {
        currentAttributeSet = drawing.attributeSets_.first();
      }

      // Only write a DIDX if this stroke needs a different attribute set than the last stroke
      if( currentAttributeSet != stroke->attributes && stroke->attributes != 0 )
      {
        currentAttributeSet = stroke->attributes;
        blockData.append( encodeUInt( TAG_DIDX ) );
        blockData.append( encodeUInt( drawing.attributeSets_.indexOf( stroke->attributes ) ) );
      }
    }

    // Make sure that the first strokes use the first transform (write a TIDX only
    // when needed)
    if ( drawing.transforms_.count() > 0 )
    {
      if( currentTransform == 0 )
      {
        currentTransform = drawing.transforms_.first();
      }

      // Only write a TIDX if this stroke needs a different transform than the last stroke
      if( currentTransform != stroke->transform && stroke->transform != 0 )
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

    deflatePacketData( blockData, xPoints );
    deflatePacketData( blockData, yPoints );

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


