/***************************************************************************
 *   Copyright (C) 2008 by Valerio Pilo                                    *
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

#ifndef ISFTAGS_H
#define ISFTAGS_H

#include "libisftypes.h"

#include <QString>



namespace Isf
{


  // Forward declarations
  namespace Compress
  {
    class IsfData;
  }
  using Compress::IsfData;



  namespace Tags
  {


    /// Read the table of GUIDs from the data
    IsfError parseGuidTable( IsfData &source );
    /// Read payload: Persistent Format
    IsfError parsePersistentFormat( IsfData &source );


    // Debugging methods

    // Print the payload of an unknown tag
    void analyzePayload( IsfData &source, const QString &tagName );

  }
}



#endif
