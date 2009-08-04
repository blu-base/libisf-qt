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

#include "tags.h"

#include "compression/compression.h"
#include "compression/isfdata.h"
#include "multibytecoding.h"

#include <QDebug>



namespace Isf
{
  namespace Tags
  {



    /// Read away an unsupported tag
    IsfError parseUnsupported( IsfData &source, const QString &tagName )
    {
      // Unsupported content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, tagName + " (Unsupported)" );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read the table of GUIDs from the data
    IsfError parseGuidTable( IsfData &source, quint64 &maxGuid )
    {
      quint64 guidTableSize = Isf::Compress::decodeUInt( source );

      // GUIDs are 16 bytes long
      quint8 numGuids = guidTableSize / 16;
      // Maximum GUID present in the file
      maxGuid = 99 + numGuids;

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- GUID table has" << numGuids << "entries for total" << guidTableSize << "bytes:";
#endif

      quint8 index = 0;
      while( ! source.atEnd() && index < numGuids )
      {
        qDebug() << "  - Index" << Isf::Compress::decodeUInt( source ) << " -> " << source.getBytes( 16 ).toHex();
        ++index;
      }

      return ISF_ERROR_NONE;
    }



    /// Read payload: Persistent Format
    IsfError parsePersistentFormat( IsfData &source )
    {
      // Unknown content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, "Persistent Format" );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read the drawing dimensions
    IsfError parseHiMetricSize( IsfData &source, QSize &size )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_HIMETRIC_SIZE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      size.setWidth ( Isf::Compress::decodeInt( source ) );
      size.setHeight( Isf::Compress::decodeInt( source ) );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Drawing dimensions:" << size << "("
               << (size.width()  / HiMetricToPixel) << "x"
               << (size.height() / HiMetricToPixel) << "pixels )";
#endif

      return ISF_ERROR_NONE;
    }



    /// Read a block of points attributes
    IsfError parseAttributeBlock( IsfData &source, QList<PointInfo> &attributes, int blockIndex )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_DRAW_ATTRS_BLOCK";
#endif
      return ISF_ERROR_INVALID_PAYLOAD;
      }

      if( attributes.count() >= blockIndex )
      {
        attributes.append( PointInfo() );
      }
      PointInfo &attrs = attributes[ blockIndex ];

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Drawing attributes size:" << payloadSize;
#endif

      quint64 payloadEnd = source.pos() + payloadSize;
      while( source.pos() < payloadEnd && ! source.atEnd() )
      {
        // Read the tag and its value
        quint64 tag   = Isf::Compress::decodeUInt( source );
        quint64 value = Isf::Compress::decodeUInt( source );

        switch( tag )
        {
          case PEN_STYLE:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got style" << value << "- Unable to handle it, skipping.";
#endif
            break;

          case PEN_COLOR:
          {
            QRgb invertedColor = value & 0xFFFFFF;
            // The color value is stored in BGR order, so we need to read it back inverted,
            // as QRgb stores the value in BGR order: QRgb(RRGGBB) <-- value(BBGGRR).
            // TODO: It also contains an alpha value, ignored here for now because it's unknown if
            // it is needed or not
            attrs.color = QColor( qBlue ( invertedColor ),
                                  qGreen( invertedColor ),
                                  qRed  ( invertedColor ) );
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen color" << attrs.color;
#endif
            break;
          }

          case PEN_WIDTH:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen width" << QString::number( (float)value, 'g', 16 )
                     << "(" << (value/HiMetricToPixel) << "pixels )";
#endif
            attrs.penSize.setWidth( (float)value );

            // In square/round pens the width will be the only value present.
            attrs.penSize.setHeight( (float)value );
            break;

          case PEN_HEIGHT:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen height" << QString::number( (float)value, 'g', 16 );
#endif
            attrs.penSize.setHeight( (float)value );
            break;

          case PEN_TIP:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen shape: is rectangular?" << (bool)value;
#endif
            if( value )
            {
              attrs.flags |= IsRectangle;
            }
            break;

          case PEN_FLAGS:
            attrs.flags = (StrokeFlags)( ( 0XFF00 & attrs.flags ) | (ushort) value );
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got flags value:" << value;
            if( value & FitToCurve )
            {
              qDebug() << "  - Got FitToCurve pen flag";
              value ^= FitToCurve;
            }
            if( value & IgnorePressure )
            {
              qDebug() << "  - Got IgnorePressure pen flag";
              value ^= IgnorePressure;
            }
            if( value & IsHighlighter )
            {
              qDebug() << "  - Got IsHighlighter pen flag";
              value ^= IsHighlighter;
            }
            if( value & IsRectangle )
            {
              qDebug() << "  - Got IsRectangle pen flag";
              value ^= IsRectangle;
            }
            qDebug() << "  - Remaining unknown flags value:" << value;
#endif
            break;

          case PEN_TRANSPARENCY:
            value = ( (uchar)value ) << 24;
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen transparency" << value;
#endif
            attrs.color.setAlpha( value );
            break;

          case PEN_ISHIGHLIGHTER:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen highlighting flag";
#endif
            attrs.flags |= IsHighlighter;
            // TODO There seems to be a total of 4 payload bytes, unknown.
            // We already read one of the 4 bytes, it's in the 'value' variable
            source.seekRelative( +3 );
            break;

          default:
            qWarning() << "- Unknown tag" << tag;

            // If the tag *should* be known, record it differently
            // FIXME have the maxGuid value here
            if( /*drawing.maxGuid_ > 0 &&*/ tag >= 100 /*&& tag <= drawing.maxGuid_*/ )
            {
              analyzePayload( source, "TAG_PROPERTY_" + QString::number( tag ) );
            }
            else
            {
              analyzePayload( source, "Unknown property " + QString::number( tag ) );
            }
            break;
        }
      }

      return ISF_ERROR_NONE;
    }


    /// Read a table of points attributes
    IsfError parseAttributeTable( IsfData &source, QList<PointInfo> &attributes )
    {
      IsfError result = ISF_ERROR_NONE;
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_DRAW_ATTRS_TABLE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      quint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        result = parseAttributeBlock( source, attributes );
      }

      return result;
    }



    /// Read the ink canvas dimensions
    IsfError parseInkSpaceRectangle( IsfData &source, QRect &rect )
    {
      // This tag has a fixed 4-byte size
      rect.setLeft  ( Isf::Compress::decodeInt( source ) );
      rect.setTop   ( Isf::Compress::decodeInt( source ) );
      rect.setRight ( Isf::Compress::decodeInt( source ) );
      rect.setBottom( Isf::Compress::decodeInt( source ) );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got drawing canvas:" << rect;
#endif

      return ISF_ERROR_NONE;
    }



    /// Read payload: Metric Table
    IsfError parseMetricTable( IsfData &source )
    {
      IsfError result = ISF_ERROR_NONE;
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_METRIC_TABLE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      quint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        result = parseMetricBlock( source );
      }

      return result;
    }



    /// Read payload: Metric Block
    IsfError parseMetricBlock( IsfData &source )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_METRIC_BLOCK";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      // Skip it, its usefulness is lesser than making the parser to actually work :)
      source.seekRelative( payloadSize );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- Skipped metrics block";
#endif

      return ISF_ERROR_NONE;
    }



    /// Read a table of transformation matrices
    IsfError parseTransformationTable( IsfData &source, QMap<DataTag,QTransform> transforms )
    {
      IsfError result = ISF_ERROR_NONE;
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_TRANSFORM_TABLE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      quint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        // Read the type of the next transformation
        DataTag tagIndex = (DataTag) Compress::decodeUInt( source );

        result = parseTransformation( source, transforms, tagIndex );
      }

      return result;
    }



    /// Read a drawing transformation matrix
    IsfError parseTransformation( IsfData &source, QMap<DataTag,QTransform> transforms, DataTag transformType )
    {
      QTransform transform;

      switch( transformType )
      {
        case TAG_TRANSFORM:
          transform.setMatrix( Compress::decodeFloat( source )
                             , Compress::decodeFloat( source )
                             , .0f
                             , Compress::decodeFloat( source )
                             , Compress::decodeFloat( source )
                             , .0f
                             , Compress::decodeFloat( source )
                             , Compress::decodeFloat( source )
                             , 1.f );
          break;

        case TAG_TRANSFORM_ISOTROPIC_SCALE:
        {
          float scaleAmount = Compress::decodeFloat( source );
          transform.scale( scaleAmount, scaleAmount );
          break;
        }

        case TAG_TRANSFORM_ANISOTROPIC_SCALE:
          transform.scale( Compress::decodeFloat( source )
                         , Compress::decodeFloat( source ) );
          break;

        case TAG_TRANSFORM_ROTATE:
          transform.rotate( Compress::decodeUInt( source ) / 100.0f );
          break;

        case TAG_TRANSFORM_TRANSLATE:
          transform.translate( Compress::decodeFloat( source )
                             , Compress::decodeFloat( source ) );
          break;

        case TAG_TRANSFORM_SCALE_AND_TRANSLATE:
          transform.translate( Compress::decodeFloat( source )
                             , Compress::decodeFloat( source ) );
          transform.scale    ( Compress::decodeFloat( source )
                             , Compress::decodeFloat( source ) );
          break;

        default:
#ifdef ISF_DEBUG_VERBOSE
          qDebug() << "Got unknown transformation type:" << transformType;
#endif
          return ISF_ERROR_INVALID_BLOCK;
      }

      transforms[ transformType ] = transform;

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "Got transformation matrix:" << transform;
#endif

      return ISF_ERROR_NONE;
    }



    /// Read a stroke
    IsfError parseStroke( IsfData &source, QList<Stroke> &strokes, QList<PointInfo> &attributes, bool hasPressureData )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );
      quint64 initialPos = source.pos();

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_STROKE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      // Get the number of points which comprise this stroke
      quint64 numPoints = Isf::Compress::decodeUInt( source );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- Tag size:" << payloadSize << "Points stored:" << numPoints;
#endif

      QList<qint64> xPointsData, yPointsData, pressureData;
      if( ! Isf::Compress::inflate( source, numPoints, xPointsData ) )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "Decompression failure while extracting X points data!";
        return ISF_ERROR_INVALID_PAYLOAD;
#endif
      }

      if( ! Isf::Compress::inflate( source, numPoints, yPointsData ) )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "Decompression failure while extracting Y points data!";
        return ISF_ERROR_INVALID_PAYLOAD;
#endif
      }

      if( hasPressureData && ! Isf::Compress::inflate( source, numPoints, pressureData ) )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "Decompression failure while extracting pressure data!";
        return ISF_ERROR_INVALID_PAYLOAD;
#endif
      }

      if( xPointsData.size() != numPoints || yPointsData.size() != numPoints )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "The points arrays have sizes x=" << xPointsData.size() << "y=" << yPointsData.size()
                   << "which do not match with the advertised size of" << numPoints;
#endif
      }

      if( hasPressureData && pressureData.size() != numPoints )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "The pressure data has a size of" << pressureData.size()
                   << "which does not match with the advertised size of" << numPoints;
#endif
      }

      // Add a new stroke
      strokes.append( Stroke() );
      Stroke &stroke = strokes[ strokes.size() - 1 ];

      for( int i = 0; i < numPoints; ++i )
      {
        stroke.points.append( Point() );
        Point &point = stroke.points[ stroke.points.size() - 1 ];

        point.position.setX( xPointsData[ i ] );
        point.position.setY( yPointsData[ i ] );

        if( hasPressureData )
        {
          point.pressureLevel = pressureData[ i ];
        }

        point.info = &attributes.last();
      }

      qint64 remainingPayloadSize = payloadSize - ( source.pos() - initialPos );
      if( remainingPayloadSize > 0 )
      {
        analyzePayload( source,
                        remainingPayloadSize,
                        "Remaining stroke data: " + QString::number(remainingPayloadSize) +
                        " bytes" );
      }

      return ISF_ERROR_NONE;
    }



    /// Read a stroke description block
    IsfError parseStrokeDescBlock( IsfData &source, QList<Stroke> &strokes, bool &hasXData, bool &hasYData, bool &hasPressureData )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_STROKE_DESC_BLOCK";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "  - Finding stroke description properties in the next" << payloadSize << "bytes";
#endif

      quint64 payloadEnd = source.pos() + payloadSize;
      while( source.pos() < payloadEnd && ! source.atEnd() )
      {
        quint64 tag = Isf::Compress::decodeUInt( source );

        switch( tag )
        {
          case TAG_NO_X:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Strokes contain no X coordinates";
#endif
            hasXData = false;
            break;

          case TAG_NO_Y:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Strokes contain no Y coordinates";
#endif
            hasYData = false;
            break;

          case TAG_BUTTONS:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Buttons...";
#endif
            break;

          case TAG_STROKE_PROPERTY_LIST:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Property list...";
#endif
            break;

          default: // List of Stroke packet properties present
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Packet properties list:" << QString::number( tag, 10 );
#endif
            hasPressureData = true;
            // TODO How is this list made?
/*
            for( quint64 i = 0; i < GUID_NUM; ++i )
            {
              if( tag & ( 1 << i ) )
              {
                qDebug() << "match:" << i;
              }
            }
*/
            break;
        }
      }

      return ISF_ERROR_NONE;
    }



    /// Read a stroke description table
    IsfError parseStrokeDescTable( IsfData &source, QList<Stroke> &strokes, bool &hasXData, bool &hasYData, bool &hasPressureData )
    {
      IsfError result = ISF_ERROR_NONE;
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_STROKE_DESC_TABLE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      quint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        result = parseStrokeDescBlock( source, strokes, hasXData, hasYData, hasPressureData );
      }

      return result;
    }



    // Print the payload of an unknown tag
    void analyzePayload( IsfData &source, const QString &tagName )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      analyzePayload( source,
                      payloadSize,
                      "Got tag: " + tagName + " with " + QString::number( payloadSize ) + " bytes of payload" );
    }



    // Print the payload of an unknown tag
    void analyzePayload( IsfData &source, const quint64 payloadSize, const QString &message )
    {
      if( payloadSize == 0 )
      {
        return;
      }

      quint64 pos = 0;
      QByteArray output;

      qDebug() << message;
      qDebug() << "--------------------------------------------------------------------";
      while( ! source.atEnd() && pos < payloadSize )
      {
        quint8 byte = source.getByte();
        output.append( QByteArray::number( byte, 16 ).rightJustified( 2, '0').toUpper() + " " );

        if( ( ( pos + 1 ) % 24 ) == 0 )
        {
          qDebug() << output;
          output.clear();
        }

        ++pos;
      }

      if( ! output.isEmpty() )
      {
        qDebug() << output;
      }

      qDebug() << "--------------------------------------------------------------------";
    }



  }
}


