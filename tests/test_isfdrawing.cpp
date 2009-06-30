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

#include "test_isfdrawing.h"

#include <QtTest/QtTest>

#include "isfdrawing.h"

using namespace Isf;

void TestIsfDrawing::constructEmptyConstructor()
{
  IsfDrawing *doc = new IsfDrawing();
  QVERIFY(doc != NULL);
  QCOMPARE(doc->isNull(), true);
  delete doc;
}

void TestIsfDrawing::constructEmptyData()
{
  QByteArray data;
  IsfDrawing *doc = new IsfDrawing(data);
  QCOMPARE(doc->isNull(), true);
  delete doc;
}

void TestIsfDrawing::constructNonEmptyData()
{
  QByteArray data = QByteArray::fromBase64("AP8CHAOAgAQdBLAC7gICDQRIEUVkB0gRRP8BRWQZFDIIAIAyAhzHsUIzCADgEgIcxzFCFauq00GrqtNBAF"
                                           "jVPgCAlT4eBwaC/HH43AAJAQp3vAGC/gDz+APbZq0iJcVLEsAksssBcqCVNmy2UAAAEC5T"
                                           "LJZLls0oVNSrZSpO4VKiwlgXNgJYsWAAgv4EC/gQO1bZ2ZsXNS2REpSLiEWRYMWCxYsShZZNlmy1"
                                           "ZSxZKJUQIZYuVYM0TWaZ3LKzUspYXNkNhUVKlsUJAAosPYL9yfuUAAEVKlibKAbCBJQWAIL+AIv4"
                                           "Ajlkqbl0KlyiVLJZYsJRZZZKKgAKJiuC/gAj+ACUWLJRaiwsWVKWJZKAgv4Aw/gDGWUDYkM2BsKK"
                                           "lSwACiUugv4A2/gDmWblgElKSgBLJuUsgv4Aw/gDEEAUVKWTLNllSpQACiElgv4BW/gFcAEAJUsq"
                                           "aACC/gC7+ALZSVdiy4BlJublWaAKFhKC/dn7rJJ6VdtroIL+AZP4BlsWAKQ=");
 
  IsfDrawing *doc = new IsfDrawing(data);
  QCOMPARE(doc->isNull(), false);
  delete doc;
}

QTEST_MAIN(TestIsfDrawing)

#include "test_isfdrawing.moc"
