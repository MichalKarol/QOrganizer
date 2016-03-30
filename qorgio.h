//    Copyright (C) 2014 Michał Karol <michal.p.karol@gmail.com>

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
#include <qorgmodel.h>
#include <QtWidgets>

using std::vector;
class QOrganizer;

namespace qorgIO {
    bool readFile(QPair<QByteArray, QByteArray> passwordPair, qorgModel* mainModel);
    bool saveFile(QPair<QByteArray, QByteArray> passwordPair, qorgModel* mainModel);
    // TODO(mkarol) Test input from 1.02 and 1.03 and 1.04
    // TODO(mkarol) Compression and GCM

}
#endif  // QORGIO_H_
