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

#include "data/datasource.h"
#include "data/multibytecoding.h"

#include "isfqt-internal.h"

#include <QtTest/QtTest>
#include <QByteArray>


void TestMultibyteCoding::testDataSource()
{
  // test the DataSource class

  bool ok;
  uchar byte;
  Isf::Compress::DataSource data;


  // Read one bit
  data.append( Q_UINT64_C(0x80) );
  data.reset(); // To re-read the appended byte
  QVERIFY( data.getBit() == true );

  data.clear();


  // Read seven bits
  data.append( Q_UINT64_C(0xC1) );
  data.reset(); // To re-read the appended byte

  byte = 0;
  for( int i = 0; i < 7; i++ )
  {
    bool bit = data.getBit( &ok );
    QVERIFY( ok );

    byte |= ( bit << ( 7 - i ) ); // Start writing from the 8th bit
  }
  QVERIFY( byte == Q_UINT64_C(0xC0) );

  data.clear();


  // Read seven bits, using the method to get multiple bits
  data.append( Q_UINT64_C(0xC9) );
  data.reset(); // To re-read the appended byte

// bits are for 0xC9:11001001 and for 0x64:01100100 (the least significant bit should be lost)
  byte = data.getBits( 7, &ok );
  QVERIFY( ok );

  QVERIFY( byte == Q_UINT64_C(0x64) );


  // Read a byte
  data.clear();
  data.append( Q_UINT64_C(0x40) );
  data.reset(); // To re-read the appended byte

  QVERIFY( data.getByte() == Q_UINT64_C(0x40) );

  data.clear();


  // Read a string (edge case)
  QByteArray string1( "This is a test string." );
  data.append( string1 );
  data.reset(); // To re-read the appended string

  QByteArray string2;
  for( int i = 0; i < string1.size(); ++i )
  {
    string2[ i ] = data.getByte( &ok );
    QVERIFY( ok );
  }

  QVERIFY( string1 == string2 );
}



void TestMultibyteCoding::unsignedEncode()
{
  // test encoding a value that will fit into a single byte.
  quint64 value = Q_UINT64_C(0x7D);
  QByteArray result( Isf::Compress::encodeUInt( value ) );

  // expect 1 byte in size
  QCOMPARE( result.size(), 1 );

  // match the value
  QVERIFY( (uint)result.at(0) == value );

  // now test the edge case between 1 and 2 bytes for encoding:
  value = Q_UINT64_C(0x80);
  // 1000000 = 128 decimal, should get encoded as 2 bytes:
  // 00000001|10000000
  result = Isf::Compress::encodeUInt( value );
  QCOMPARE( result.size(), 2 );
  QCOMPARE( result.at(0), (char)0x80 );
  QCOMPARE( result.at(1), (char)0x01 );

  // and finally test a definite multibyte value.
  value = Q_UINT64_C(0xFFFF); // 11111111 11111111, dec = 65535
  result = Isf::Compress::encodeUInt( value );
  QCOMPARE( result.size(), 3 );
  QCOMPARE( result.at(0), (char)0xFF );
  QCOMPARE( result.at(1), (char)0xFF );
  QCOMPARE( result.at(2), (char)0x03 );
}



void TestMultibyteCoding::unsignedDecode()
{
  quint64 result;

  // test decoding a value that should only get encoded into
  // a single byte.
  Isf::Compress::DataSource data;
  data.append( Q_UINT64_C(0x7D) ); // decimal 125
  data.reset(); // To re-read the appended bytes

  result = Isf::Compress::decodeUInt( &data );

  QVERIFY(result == Q_UINT64_C(0x7D) );

  data.clear();


  // test the edge case...
  data.append( Q_UINT64_C(0x80) );
  data.append( Q_UINT64_C(0x01) );
  data.reset(); // To re-read the appended bytes

  result = Isf::Compress::decodeUInt( &data );
  QVERIFY( result == Q_UINT64_C(0x80) );

  data.clear();


  // test a definite multibyte.
  data.append( Q_UINT64_C(0xFF) );
  data.append( Q_UINT64_C(0xFF) );
  data.append( Q_UINT64_C(0x03) );
  data.reset(); // To re-read the appended bytes

  result = Isf::Compress::decodeUInt( &data );
  QVERIFY( result == Q_UINT64_C(0xFFFF) );
}



void TestMultibyteCoding::signedEncode()
{
  // test a value x such that -64 < x < 0.
  qint64 value = Q_INT64_C(-5);
  QByteArray result( Isf::Compress::encodeInt( value ) );

  QCOMPARE( result.size(), 1 );
  QCOMPARE( result.at(0), (char)0x0B ); // -5 encoded.
}



void TestMultibyteCoding::signedDecode()
{
  qint64 result;
  Isf::Compress::DataSource data;

  // -10 (0xA in hex, shift 1 to left, set sign bit)
  data.append( (char)((0x0A << 1) | 0x01) );
  data.reset(); // To re-read the appended byte

  result = Isf::Compress::decodeInt( &data );
  QVERIFY( result == Q_INT64_C(-10) );

  data.clear();


  // edge case, -64.
  data.append( Q_INT64_C(0x81) );
  data.append( Q_INT64_C(0x01) );
  data.reset(); // To re-read the appended byte

  result = Isf::Compress::decodeInt( &data );
  QVERIFY( result == Q_INT64_C(-64) );

  data.clear();


  // a positive number, 100
  data.append( Q_INT64_C(0xC8) );
  data.append( Q_INT64_C(0x01) );
  data.reset(); // To re-read the appended byte

  result = Isf::Compress::decodeInt( &data );
  QVERIFY( result == Q_INT64_C(100) );

  data.clear();


  // a decent negative number, -500
  data.append( Q_INT64_C(0xE9) );
  data.append( Q_INT64_C(0x07) );
  data.reset(); // To re-read the appended byte

  result = Isf::Compress::decodeInt( &data );
  QVERIFY( result == Q_INT64_C(-500) );
}



void TestMultibyteCoding::floatEncode()
{
  QByteArray bytes;

  bytes = Isf::Compress::encodeFloat( -12.345678901f );

  QCOMPARE( bytes.size(), 4 );
  QCOMPARE( bytes.at(0), (char)0xE7 );
  QCOMPARE( bytes.at(1), (char)0x87 );
  QCOMPARE( bytes.at(2), (char)0x45 );
  QCOMPARE( bytes.at(3), (char)0xC1 );
}



void TestMultibyteCoding::floatDecode()
{
  float result;
  Isf::Compress::DataSource data;

  data.append( Q_INT64_C(0xE7) );
  data.append( Q_INT64_C(0x87) );
  data.append( Q_INT64_C(0x45) );
  data.append( Q_INT64_C(0xC1) );
  data.reset(); // To re-read the appended bytes

  result = Isf::Compress::decodeFloat( &data );
  QVERIFY( result == -12.345678901f );
}



QTEST_MAIN(TestMultibyteCoding)



#include "test_multibyte_coding.moc"
