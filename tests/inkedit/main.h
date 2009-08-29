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

#ifndef MAIN_H
#define MAIN_H

#include "ui_testinkedit.h"

#include <IsfQtDrawing>

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QAbstractButton>

#include <IsfInkCanvas>


class TestInkEdit : public QWidget, public Ui::TestInkEdit
{
  Q_OBJECT
  
  public:
    
    TestInkEdit();
    ~TestInkEdit();
    
  public slots:
    
    void saveInk();
    void loadInk();
    void clearInk();
    void penTypeChanged( QAbstractButton *button );
    void chooseColor();

};

#endif
