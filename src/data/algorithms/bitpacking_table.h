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

#ifndef ISFCOMPRESSION_BITPACKING_H
#define ISFCOMPRESSION_BITPACKING_H

#include <QtGlobal>


#define CBITS   0
#define CPADS   1


namespace Isf {
  namespace Compress {
    namespace BitPackingAlgorithm {

      /// Table used to compute the length of bit-packed values and their remainders.
      quint8 bitLookUpTable_[ 48 ][ 2 ] =
      {
        // { cBits, cPads }
        {  8, 0 }, // index 0
        {  1, 0 },
        {  1, 1 },
        {  1, 2 },
        {  1, 3 },
        {  1, 4 },
        {  1, 5 },
        {  1, 6 },
        {  1, 7 },
        {  2, 0 },
        {  2, 1 }, // index 10
        {  2, 2 },
        {  2, 3 },
        {  3, 0 },
        {  3, 1 },
        {  3, 2 },
        {  4, 0 },
        {  4, 1 },
        {  5, 0 },
        {  5, 1 },
        {  6, 0 }, // index 20
        {  6, 1 },
        {  7, 0 },
        {  7, 1 },
        {  8, 0 },
        {  9, 0 },
        { 10, 0 },
        { 11, 0 },
        { 12, 0 },
        { 13, 0 },
        { 14, 0 }, // index 30
        { 15, 0 },
        { 16, 0 },
        { 17, 0 },
        { 18, 0 },
        { 19, 0 },
        { 20, 0 },
        { 21, 0 },
        { 22, 0 },
        { 23, 0 },
        { 24, 0 }, // index 40
        { 25, 0 },
        { 26, 0 },
        { 27, 0 },
        { 28, 0 },
        { 29, 0 },
        { 30, 0 },
        { 31, 0 }  // index 47
      };

    } // Namespace BitPackingAlgorithm
  } // Namespace Compress
} // Namespace Isf



#endif
