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
#include <QTimer>
#include <QButtonGroup>

#include <IsfInkCanvas>

#include "main.h"

using namespace Isf;

TestInkEdit::TestInkEdit()
{
  setupUi( this );
  connect(cmdSave_, &QPushButton::clicked, this, &TestInkEdit::saveInk);
  connect(cmdLoad_, &QPushButton::clicked, this, &TestInkEdit::loadInk);
  connect(cmdClear_, &QPushButton::clicked, this, &TestInkEdit::clearInk);

  connect(cmdStrokeColor_, &QPushButton::clicked, this, &TestInkEdit::chooseColor);
  connect(cmdCanvasColor_, &QPushButton::clicked, this, &TestInkEdit::chooseColor);

  connect(editor_, &InkCanvas::inkChanged, this, &TestInkEdit::inkChanged);

  auto *grp = new QButtonGroup( this );
  grp->addButton( rbDrawing_ );
  grp->addButton( rbEraser_ );

  connect(grp, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &TestInkEdit::penTypeChanged);

  connect(spinWidth_, QOverload<int>::of(&QSpinBox::valueChanged), editor_, &InkCanvas::setPenSize);

  editor_->setPenSize( spinWidth_->value() );
}

TestInkEdit::~TestInkEdit() = default;


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
    editor_->setPenType( InkCanvas::DrawingPen );
  }
  else
  {
    editor_->setPenType( InkCanvas::EraserPen );
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

    QMessageBox::information(nullptr, "Save complete", "Saved to " + saveFile );
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

void TestInkEdit::inkChanged()
{
  emptyDrawingCheckbox_->setChecked( editor_->isEmpty() );

  statusLabel_->setText( "Drawing changed!" );
  QTimer::singleShot( 3000, statusLabel_, &QLabel::clear );
}

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  TestInkEdit edit{};
  edit.show();

  return app.exec();
}
