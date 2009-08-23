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

#include <IsfQtDrawing>

#include <QColorDialog>
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDataStream>
#include <QTextStream>
#include <QFile>

#include <IsfInkEdit>

#include "main.h"

using namespace Isf;

TestInkEdit::TestInkEdit()
{
  setupUi( this );
  connect( cmdSave_, SIGNAL(clicked()), this, SLOT( saveInk() ) );
  connect( cmdLoad_, SIGNAL(clicked()), this, SLOT( loadInk() ) );
  connect( cmdClear_, SIGNAL(clicked()), this, SLOT( clearInk() )  );
  
  connect( cmdStrokeColor_, SIGNAL(clicked()), this, SLOT( chooseColor() )  );
  connect( cmdCanvasColor_, SIGNAL(clicked()), this, SLOT( chooseColor() )  );
  
  QButtonGroup *grp = new QButtonGroup( this );
  grp->addButton( rbDrawing_ );
  grp->addButton( rbEraser_ );
  
  connect( grp, SIGNAL( buttonClicked( QAbstractButton * ) ), this, SLOT( penTypeChanged( QAbstractButton * ) ) );
  
  connect( spinWidth_, SIGNAL( valueChanged( int ) ), editor_, SLOT( setPenSize( int ) ) );

  editor_->setPenSize( spinWidth_->value() );
  /*
  setWindowTitle("Ink Edit Test");
  
  editor_ = new Isf::InkEdit();
  editor_->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

  sayLabel_ = new QLabel();
  sayLabel_->setText( "Draw something!" );
  sayLabel_->setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  cmdSave_ = new QPushButton();
  cmdSave_->setText( "Save Ink" );
  connect( cmdSave_, SIGNAL(clicked()), this, SLOT( saveInk() ) );
  
  cmdLoad_ = new QPushButton();
  cmdLoad_->setText( "Load Ink" );
  connect( cmdLoad_, SIGNAL(clicked()), this, SLOT( loadInk() ) );
  
  QPushButton *cmdClear_ = new QPushButton();
  cmdClear_->setText( "Clear Ink" );
  connect( cmdClear_, SIGNAL(clicked()), this, SLOT( clearInk() )  );

  QVBoxLayout *layout = new QVBoxLayout();
  QHBoxLayout *strokeBtnLayout = new QHBoxLayout();
  
  layout->addWidget( sayLabel_ );

  layout->addWidget( editor_ );
  
  QHBoxLayout *ctlBtnLayout = new QHBoxLayout();
  ctlBtnLayout->addWidget(cmdSave_);
  ctlBtnLayout->addWidget(cmdLoad_);
  ctlBtnLayout->addWidget(cmdClear_);

  layout->addLayout( ctlBtnLayout );
  
  setLayout( layout );
  */
}

TestInkEdit::~TestInkEdit()
{
}


void TestInkEdit::chooseColor()
{
  QColor color = QColorDialog::getColor();
  if ( sender() == cmdStrokeColor_ )
  {
    editor_->setPenColor( color );
  }
  else
  {
    editor_->setCanvasColor( color );
  }
}


void TestInkEdit::penTypeChanged( QAbstractButton *button )
{
  if( button == rbDrawing_ )
  {
    editor_->setPenType( InkEdit::DrawingPen );
  }
  else
  {
    editor_->setPenType( InkEdit::EraserPen );
  }
}


void TestInkEdit::saveInk()
{
  QString filter;
  QString saveFile = QFileDialog::getSaveFileName( this, "Save Ink", QString(), "Raw ISF (*.isf);;base64-encoded ISF (*.isf64)", &filter );
  if ( saveFile != QString() ) 
  {
    QFile file(saveFile);
    file.open(QIODevice::WriteOnly);

    if ( filter.contains("isf64") )
    {
      editor_->save( file, true );
    }
    else
    {
      editor_->save( file );
    }
    
    file.close();
    
    QMessageBox::information(0, "Save complete", "Saved to " + saveFile );
  }
}

void TestInkEdit::loadInk() 
{
  QString filter;
  QString filename = QFileDialog::getOpenFileName( this, "Open Ink", QString(), "Raw ISF (*.isf);;base64-encoded ISF (*.isf64)", &filter );
  if ( ! filename.isEmpty() ) 
  {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QByteArray data = filter.contains("*.isf64") ? QByteArray::fromBase64( file.readAll() ) : file.readAll();

    file.close();

    // got some ink.
    Isf::Drawing *drawing = &Isf::Stream::reader(data);
    
    editor_->setDrawing( drawing );

    editor_->updateGeometry();
  }
}

void TestInkEdit::clearInk()
{
  editor_->clear();
}

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  TestInkEdit edit;
  edit.show();
  
  return app.exec();
}
