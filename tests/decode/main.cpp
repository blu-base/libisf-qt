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

#include <IsfQtDrawing>

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QMainWindow>

#include "ui_testdecode.h"



class TestDecode : public QMainWindow, private Ui::TestDecode
{
  public:

    void test()
    {
      bool rewrite = false;
      QStringList arguments( qApp->arguments() );
      arguments.removeAt( 0 );

      if( arguments.count() < 1 )
      {
        label_->setText( "You must specify the name of an ISF file to test!" );
        return;
      }

      int rewriteIndex = arguments.indexOf( "--rewrite" );
      if( rewriteIndex >= 0 )
      {
        rewrite = true;
        arguments.removeAt( rewriteIndex );
      }

      const QString fileName( arguments.at( 0 ) );
      const QByteArray rawData( readTestIsfData( fileName ) );
      Isf::Drawing drawing;

      // Extract the ISF data from the GIF image
      if( fileName.contains( ".gif" ) )
      {
        qDebug() << "------------------------- Creating drawing from GIF -------------------------";
        drawing = Isf::Stream::readerGif( rawData );
      }
      // Extract the ISF data from the ISF file
      else
      {
        qDebug() << "------------------------- Creating drawing from ISF -------------------------";
        drawing = Isf::Stream::reader( rawData );
      }

      if( rewrite )
      {
        // change to test GIF or regular streams
        bool testGif = false;

        if( testGif )
        {
          // Test Fortified-GIF r/w
          qDebug() << "------------------------- Writing drawing to GIF file -------------------------";
          QByteArray data( Isf::Stream::writerGif( drawing ) );

          qDebug() << "------------------------- Reading GIF back -------------------------";
          drawing = Isf::Stream::readerGif( data );
        }
        else
        {
          // Test regular streams r/w
          qDebug() << "------------------------- Writing drawing to ISF file -------------------------";
          QByteArray data2( Isf::Stream::writer( drawing ) );

          qDebug() << "------------------------- Reading ISF back -------------------------";
          drawing = Isf::Stream::reader( data2 );
        }
      }

      if( drawing.isNull() )
      {
        label_->setText( "Invalid file contents!" );
      }
      else
      {
        qDebug() << "------------------------- Showing it -------------------------";
        label_->setPixmap( drawing.pixmap( Qt::transparent ) );
      }
    }


    TestDecode()
    {
      QWidget *mainWidget = new QWidget( this );
      setupUi( mainWidget );
      setCentralWidget( mainWidget );
      setWindowTitle( "ISF decoding test" );

      connect( closeButton_, SIGNAL( clicked(QAbstractButton*) ),
               this,         SLOT  (   close()                 ) );

      show();

      test();
    }

    ~TestDecode()
    {}



    // read some test raw ISF data from a file on the filesystem and
    // return it as a QByteArray.
    QByteArray readTestIsfData( const QString &filename )
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

      QByteArray data = file.readAll();
      return data;
    }



    // read some test raw ISF data from a file on the filesystem and
    // return it as a QByteArray.
    void saveTestIsfData( const QByteArray &data )
    {
      static int number = 1;
      QString fileName( "test" + QString::number( number ) + "." );

      // autodetect if it's a gif or an isf
      if( data[0] == '\0' )
      {
        fileName.append( "isf" );
      }
      else
      {
        fileName.append( "gif" );
      }

      QFile file( fileName );
      file.open( QIODevice::WriteOnly );
      file.write( data );
      file.close();
    }

};



int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  TestDecode test;

  return app.exec();
}



