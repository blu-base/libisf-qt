/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.*/

#include "test_png_fortification.h"

#include <IsfQtDrawing>

#include "isfqt-internal.h"

#include <QtTest/QtTest>
#include <QPixmap>


using namespace Isf;
using namespace Isf::Compress;




void TestPngFortification::testEncode()
{
  QPixmap pix( QSize( 135, 114 ) );
  pix.fill( Qt::transparent );
  qDebug() << "pix size:" << pix.size();

  qDebug() << "#################### ENCODER TEST ####################";

  QFile file( "../../tests/test2.isf" );
  QVERIFY( file.exists() );

  // read file into qbytearray.
  QVERIFY( file.open( QIODevice::ReadOnly ) );

  Drawing drawing = Stream::reader( file.readAll(), false );

  QVERIFY( ! drawing.isNull() );

  QByteArray byteArray( Stream::writerPng( drawing, false ) );

  QVERIFY( ! byteArray.isEmpty() );

  QImage img = QImage::fromData( byteArray, "PNG" );

  QVERIFY( ! img.isNull() );
  qDebug() << "Image size:" << img.size();

  QVERIFY( tempFile.open() );

  tempFile.write( byteArray );
  tempFile.close();
}


void TestPngFortification::testDecode()
{
  qDebug() << "#################### DECODER TEST ####################";

  QVERIFY( tempFile.exists() );

  // read file and convert to qbytearray.
  QVERIFY( tempFile.open() );

  QByteArray byteArray( tempFile.readAll() );

  Drawing drawing = Stream::readerPng( byteArray, false );

  QVERIFY( ! drawing.isNull() );

  qDebug() << "Drawing size:" << drawing.size();
}



QTEST_MAIN(TestPngFortification)



#include "test_png_fortification.moc"
