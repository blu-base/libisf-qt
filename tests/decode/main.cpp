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

#include "isfdrawing.h"

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

      if( qApp->arguments().count() < 2 )
      {
        label_->setText( "You must specify the name of an ISF file to test!" );
        return;
      }

      QByteArray data = readTestIsfData( qApp->arguments().at( 1 ) );
      Isf::Drawing drawing( Isf::Parser::isfToDrawing( data ) );

      if( drawing.isNull() )
      {
        label_->setText( "Invalid file contents!" );
      }
      else
      {
        label_->setPixmap( drawing.getPixmap() );
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

};



int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  TestDecode test;

  return app.exec();
}



