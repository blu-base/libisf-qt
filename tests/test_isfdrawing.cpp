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

#include "test_isfdrawing.h"

#include <QtTest/QtTest>

#include "isfdrawing.h"

using namespace Isf;



TestIsfDrawing::TestIsfDrawing()
{
}



void TestIsfDrawing::emptyConstructor_NullDrawing()
{
  Drawing doc;
  QCOMPARE(doc.isNull(), true);
}



// invalid ISF version numbers should return a null Drawing
void TestIsfDrawing::invalidVersion_NullDrawing()
{
  QByteArray data;
  data.append(0x0B);  // isf version 11 - invalid.

  Drawing drawing = Parser::isfToDrawing( data );
  QCOMPARE( drawing.isNull(), true );
  QVERIFY( drawing.error() == ISF_ERROR_BAD_VERSION );
}



// by default the parser error should be ISF_ERROR_NONE
void TestIsfDrawing::parserErrorNoneByDefault()
{
  Drawing drawing;
  QCOMPARE( drawing.error(), ISF_ERROR_NONE );
}



// an invalid stream size should give a null drawing.
void TestIsfDrawing::invalidStreamSize_NullDrawing()
{
  QByteArray data;
  data.append((char)0x00);  // ISF version 1.0.
  data.append(0x01);  // stream size of 1 byte, but only 3 bytes of data.

  Drawing drawing = Parser::isfToDrawing( data );
  QCOMPARE( drawing.isNull(), true );
  QCOMPARE( drawing.error(), ISF_ERROR_BAD_STREAMSIZE );
}



// valid ISF test data should generate a non-null
// drawing with the appropriate number of strokes.
void TestIsfDrawing::parseValidRawIsfData()
{
  QByteArray data = readTestIsfData("tests/test1.isf");
  Drawing drawing = Parser::isfToDrawing( data );
  QCOMPARE( drawing.isNull(), false );
}



// read some test raw ISF data from a file on the filesystem and
// return it as a QByteArray.
QByteArray TestIsfDrawing::readTestIsfData( const QString &filename )
{
  QFile file( filename );
  if ( !file.exists() )
  {
    qWarning() << "Test ISF file" << filename << "does not exist.";
    return QByteArray();
  }

  // read file and convert to qbytearray.
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    qWarning() << "Failed to open ISF data file" << filename;
    return QByteArray();
  }

  return file.readAll();
}



QTEST_MAIN(TestIsfDrawing)



#include "test_isfdrawing.moc"
