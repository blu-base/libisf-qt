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

#ifndef ISFDRAWING_H
#define ISFDRAWING_H

#include <QtCore/QObject>

namespace Isf
{
  
/**
 * This class loads ISF (Ink Serialized Format) drawings.
 *
 * @author Adam Goossens (adam@kmess.org)
 */
class IsfDrawing : public QObject
{
  public:
    IsfDrawing();
    IsfDrawing(QByteArray &isfData);

    // true if this is a null IsfDocument, false otherwise.
    bool            isNull() const;
    // get the ISF version number
    quint16         getIsfVersion() const;

  private:
    void            parseIsfData(const QByteArray &data);

  private:
    // The raw ISF data.
    QByteArray      isfData_;
    // is this a null document?
    bool            isNull_;
    // isf version
    quint16         version_;

};

}

#endif // ISFDRAWING_H
