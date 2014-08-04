//    Copyright (C) 2014 Michał Karol <mkarol@linux.pl>

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QORGIO_H_
#define QORGIO_H_
#include <qorganizer.h>
#include <QtWidgets>
#include <openssl/aes.h>
#include <openssl/rand.h>

using std::vector;
class QOrganizer;

namespace qorgIO {
   bool ReadFile(QString*, QString*, QOrganizer*, QString);
   void SaveFile(QString*, QString*, QOrganizer*, QString);
}
#endif  // QORGIO_H_
