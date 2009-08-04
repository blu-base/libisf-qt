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
#include "isfdrawing.h"
#include "multibytecoding.h"

#include <QDebug>

using namespace Isf;




    /// Read away an unsupported tag
    IsfError Tags::parseUnsupported( IsfData &source, const QString &tagName )
    {
      // Unsupported content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, tagName + " (Unsupported)" );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read the table of GUIDs from the data
    IsfError Tags::parseGuidTable( IsfData &source, Drawing &drawing )
    {
      quint64 guidTableSize = Isf::Compress::decodeUInt( source );

      // GUIDs are 16 bytes long
      quint8 numGuids = guidTableSize / 16;
      // Maximum GUID present in the file
      drawing.maxGuid_ = 99 + numGuids;

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- GUID table has" << numGuids << "entries for total" << guidTableSize << "bytes:";
#endif

      quint8 index = 0;

      while( ! source.atEnd() && index < numGuids )
      {
        // 100 is the first index available for custom GUIDs
        quint8 guidIndex = index + 100;

#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "  - Index" << QString::number( guidIndex ).rightJustified( ' ', 5 )
                 << "->" << source.getBytes( 16 ).toHex();
#endif

        ++index;
      }

      return ISF_ERROR_NONE;
    }



    /// Read payload: Persistent Format
    IsfError Tags::parsePersistentFormat( IsfData &source, Drawing &drawing )
    {
      Q_UNUSED( drawing )

      // Unknown content
#ifdef ISF_DEBUG_VERBOSE
      analyzePayload( source, "Persistent Format" );
#endif
      return ISF_ERROR_NONE;
    }



    /// Read the drawing dimensions
    IsfError Tags::parseHiMetricSize( IsfData &source, Drawing &drawing )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_HIMETRIC_SIZE";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      drawing.size_.setWidth ( Isf::Compress::decodeInt( source ) );
      drawing.size_.setHeight( Isf::Compress::decodeInt( source ) );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- Drawing dimensions:" << drawing.size_ << "("
               << (drawing.size_.width()  / HiMetricToPixel) << "x"
               << (drawing.size_.height() / HiMetricToPixel) << "pixels )";
#endif

      return ISF_ERROR_NONE;
    }



    /// Read a block of points attributes
    IsfError Tags::parseAttributeBlock( IsfData &source, Drawing &drawing )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      if( payloadSize == 0 )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Invalid payload for TAG_DRAW_ATTRS_BLOCK";
#endif
        return ISF_ERROR_INVALID_PAYLOAD;
      }

      drawing.attributes_.append( PointInfo() );
      PointInfo &info = drawing.attributes_.last();
      drawing.currentPointInfo_ = &info;

      qint64 payloadEnd = source.pos() + payloadSize;
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
            info.color = QColor( qBlue ( invertedColor ),
                                 qGreen( invertedColor ),
                                 qRed  ( invertedColor ) );
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen color" << info.color;
#endif
            break;
          }

          case PEN_WIDTH:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen width" << QString::number( (float)value, 'g', 16 )
                     << "(" << (value/HiMetricToPixel) << "pixels )";
#endif
            info.penSize.setWidth( (float)value );

            // In square/round pens the width will be the only value present.
            info.penSize.setHeight( (float)value );
            break;

          case PEN_HEIGHT:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen height" << QString::number( (float)value, 'g', 16 );
#endif
            info.penSize.setHeight( (float)value );
            break;

          case PEN_TIP:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen shape: is rectangular?" << (bool)value;
#endif
            if( value )
            {
              info.flags |= IsRectangle;
            }
            break;

          case PEN_FLAGS:
            info.flags = (StrokeFlags)( ( 0XFF00 & info.flags ) | (ushort) value );
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
            info.color.setAlpha( value );
            break;

          case PEN_ISHIGHLIGHTER:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Got pen highlighting flag";
#endif
            info.flags |= IsHighlighter;
            // TODO There seems to be a total of 4 payload bytes, unknown.
            // We already read one of the 4 bytes, it's in the 'value' variable
            source.seekRelative( +3 );
            break;

          default:
            qWarning() << "- Unknown tag" << tag;

            // If the tag *should* be known, record it differently
            if( drawing.maxGuid_ > 0 && tag >= 100 && tag <= drawing.maxGuid_ )
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
    IsfError Tags::parseAttributeTable( IsfData &source, Drawing &drawing )
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

      qint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
#ifdef ISF_DEBUG_VERBOSE
        qDebug() << "Got tag: TAG_DRAW_ATTRS_BLOCK";
#endif
        result = parseAttributeBlock( source, drawing );
      }

      return result;
    }



    /// Read the ink canvas dimensions
    IsfError Tags::parseInkSpaceRectangle( IsfData &source, Drawing &drawing )
    {
      // This tag has a fixed 4-byte size
      drawing.canvas_.setLeft  ( Isf::Compress::decodeInt( source ) );
      drawing.canvas_.setTop   ( Isf::Compress::decodeInt( source ) );
      drawing.canvas_.setRight ( Isf::Compress::decodeInt( source ) );
      drawing.canvas_.setBottom( Isf::Compress::decodeInt( source ) );

#ifdef ISF_DEBUG_VERBOSE
      qDebug() << "- Got drawing canvas:" << drawing.canvas_;
#endif

      return ISF_ERROR_NONE;
    }



    /// Read payload: Metric Table
    IsfError Tags::parseMetricTable( IsfData &source, Drawing &drawing )
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

      qint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        result = parseMetricBlock( source, drawing );
      }

      return result;
    }



    /// Read payload: Metric Block
    IsfError Tags::parseMetricBlock( IsfData &source, Drawing &drawing )
    {
      Q_UNUSED( drawing )

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
    IsfError Tags::parseTransformationTable( IsfData &source, Drawing &drawing )
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

      qint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        // Read the type of the next transformation
        DataTag tagIndex = (DataTag) Compress::decodeUInt( source );

        result = parseTransformation( source, drawing, tagIndex );
      }

      return result;
    }



    /// Read a drawing transformation matrix
    IsfError Tags::parseTransformation( IsfData &source, Drawing &drawing, quint64 transformType )
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

      drawing.transforms_.append( transform );
      drawing.currentTransform_ = &( drawing.transforms_.last() );

      return ISF_ERROR_NONE;
    }



    /// Read a stroke
    IsfError Tags::parseStroke( IsfData &source, Drawing &drawing )
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

      if(   drawing.currentStrokeInfo_->hasPressureData
      &&  ! Isf::Compress::inflate( source, numPoints, pressureData ) )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "Decompression failure while extracting pressure data!";
        return ISF_ERROR_INVALID_PAYLOAD;
#endif
      }

      if( (uint)xPointsData.size() != numPoints || (uint)yPointsData.size() != numPoints )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "The points arrays have sizes x=" << xPointsData.size() << "y=" << yPointsData.size()
                   << "which do not match with the advertised size of" << numPoints;
#endif
      }

      if( drawing.currentStrokeInfo_->hasPressureData
      &&  (uint)pressureData.size() != numPoints )
      {
#ifdef ISF_DEBUG_VERBOSE
        qWarning() << "The pressure data has a size of" << pressureData.size()
                   << "which does not match with the advertised size of" << numPoints;
#endif
      }

      // Add a new stroke
      drawing.strokes_.append( Stroke() );
      Stroke &stroke = drawing.strokes_[ drawing.strokes_.size() - 1 ];

      for( quint64 i = 0; i < numPoints; ++i )
      {
        stroke.points.append( Point() );
        Point &point = stroke.points[ stroke.points.size() - 1 ];

        point.position.setX( xPointsData[ i ] );
        point.position.setY( yPointsData[ i ] );

        if( drawing.currentStrokeInfo_->hasPressureData )
        {
          point.pressureLevel = pressureData[ i ];
        }
      }

      stroke.attributes = drawing.currentPointInfo_;
      stroke.info       = drawing.currentStrokeInfo_;
      stroke.metrics    = drawing.currentMetrics_;
      stroke.transform  = drawing.currentTransform_;

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
    IsfError Tags::parseStrokeDescBlock( IsfData &source, Drawing &drawing )
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
      qDebug() << "- Finding stroke description properties in the next" << payloadSize << "bytes";
#endif

      drawing.strokeInfo_.append( StrokeInfo() );
      StrokeInfo &info = drawing.strokeInfo_.last();
      drawing.currentStrokeInfo_ = &info;

      qint64 payloadEnd = source.pos() + payloadSize;
      while( source.pos() < payloadEnd && ! source.atEnd() )
      {
        quint64 tag = Isf::Compress::decodeUInt( source );

        switch( tag )
        {
          case TAG_NO_X:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Stroke contains no X coordinates";
#endif
            info.hasXData = false;
            break;

          case TAG_NO_Y:
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Stroke contains no Y coordinates";
#endif
            info.hasYData = false;
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

          default: // List of Stroke packet properties
#ifdef ISF_DEBUG_VERBOSE
            qDebug() << "- Packet properties list:" << QString::number( tag, 10 );
#endif
            info.hasPressureData = true;
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
    IsfError Tags::parseStrokeDescTable( IsfData &source, Drawing &drawing )
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

      qint64 payloadEnd = source.pos() + payloadSize;
      while( result == ISF_ERROR_NONE && source.pos() < payloadEnd && ! source.atEnd() )
      {
        result = parseStrokeDescBlock( source, drawing );
      }

      return result;
    }



    // Print the payload of an unknown tag
    void Tags::analyzePayload( IsfData &source, const QString &tagName )
    {
      quint64 payloadSize = Isf::Compress::decodeUInt( source );

      analyzePayload( source,
                      payloadSize,
                      "Got tag: " + tagName + " with " + QString::number( payloadSize ) + " bytes of payload" );
    }



    // Print the payload of an unknown tag
    void Tags::analyzePayload( IsfData &source, const quint64 payloadSize, const QString &message )
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
          qDebug() << output.trimmed();
          output.clear();
        }

        ++pos;
      }

      if( ! output.isEmpty() )
      {
        qDebug() << output.trimmed();
      }

      qDebug() << "--------------------------------------------------------------------";
    }


