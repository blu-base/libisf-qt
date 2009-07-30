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

#include "test_multibyte_coding.h"

#include <QtTest/QtTest>
#include <QByteArray>


#include "compression/isfdata.h"
#include "libisf.h"


void TestMultibyteCoding::isfData()
{
  // test the IsfData class

  Isf::Compress::IsfData data;

  // Read one bit
  data.append( 1 );
  QVERIFY( data.getBit() == true );

  // Read seven bytes
  data.clear();
  data.append( 0xC0 );

  char byte = 0;
  for( int i = 0; i < 7; i++ )
  {
    byte |= ( data.getBit() << i );
  }
  QVERIFY( byte == Q_UINT64_C(0x40) );

  // Read a byte
  data.clear();
  data.append( 0x40 );

  QVERIFY( data.getByte() == Q_UINT64_C(0x40) );

  // Read a string (edge case)
  data.clear();

  QByteArray string1( "This is a test string." );
  data.append( string1 );

  QByteArray string2;
  for( int i = 0; i < string1.size(); ++i )
  {
    string2[ i ] = data.getByte();
  }

  QVERIFY( string1 == string2 );
}



void TestMultibyteCoding::unsignedEncode()
{
  // test encoding a value that will fit into a single byte.
  quint64 value = Q_UINT64_C(0x7d); // dec = 125
  QByteArray result(Isf::Compress::encodeUInt(value));

  // expect 1 byte in size
  QCOMPARE(result.size(), 1);

  // match the value
  QVERIFY(result.at(0) == value);

  // now test the edge case between 1 and 2 bytes for encoding:
  value = Q_UINT64_C(0x80); // dec= 128
  result = Isf::Compress::encodeUInt(value);
  QCOMPARE(result.size(), 2);
  QCOMPARE(result.at(0), (char)0x80);
  QCOMPARE(result.at(1), (char)0x01);

  // and finally test a definite multibyte value.
  value = Q_UINT64_C(0xFFFF); // dec = 65535
  result = Isf::Compress::encodeUInt(value);
  QCOMPARE(result.size(), 3);
  QCOMPARE(result.at(0), (char)0xFF);
  QCOMPARE(result.at(1), (char)0xFF);
  QCOMPARE(result.at(2), (char)0x03);
}

void TestMultibyteCoding::unsignedDecode()
{
  // test decoding a value that should only get encoded into
  // a single byte.
  Isf::Compress::IsfData data;
  data.append(0x7d); // 125.

  quint64 result = Isf::Compress::decodeUInt( data );

  QVERIFY(result == Q_UINT64_C(0x7d));

  // test the edge case...
  data.clear();
  data.append(0x80);
  data.append(0x01);
  result = Isf::Compress::decodeUInt( data );
  QVERIFY(result == Q_UINT64_C(0x80));

  // test a definite multibyte.
  data.clear();
  data.append(0xFF);
  data.append(0xFF);
  data.append(0x03);
  result = Isf::Compress::decodeUInt( data );
  QVERIFY(result == Q_UINT64_C(0xFFFF));
}

void TestMultibyteCoding::signedEncode()
{
  // test a value x such that -64 < x < 0.
  qint64 value = Q_INT64_C(-5);
  QByteArray result(Isf::Compress::encodeInt( value ));
  QCOMPARE(result.size(), 1);
  QCOMPARE(result.at(0), (char)0xB); // -5 encoded.
}

void TestMultibyteCoding::signedDecode()
{
  Isf::Compress::IsfData data;
  // -10 (0xA in hex, shift 1 to left, set sign bit)
  data.append((0xA << 1) | 0x01);
  qint64 result = Isf::Compress::decodeInt( data );
  QVERIFY(result == Q_INT64_C(-10));

  // edge case, -64.
  data.clear();
  data.append(0x81);
  data.append(0x01);
  result = Isf::Compress::decodeInt( data );
  QVERIFY(result == Q_INT64_C(-64));

  // a positive number, 100
  data.clear();
  data.append((char)0xC8);
  data.append(0x01);
  result = Isf::Compress::decodeInt( data );
  QVERIFY(result == Q_INT64_C(100));

  // a decent negative number, -500
  data.clear();
  data.append((char)0xE9);
  data.append(0x07);
  result = Isf::Compress::decodeInt( data );
  QVERIFY(result == Q_INT64_C(-500));
}

QTEST_MAIN(TestMultibyteCoding)

#include "test_multibyte_coding.moc"
