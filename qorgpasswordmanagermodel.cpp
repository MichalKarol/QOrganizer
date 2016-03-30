//    Copyright (C) 2015 Michał Karol <michal.p.karol@gmail.com>

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

#include "qorgpasswordmanagermodel.h"
bool Container::operator <(const Container* container) {
    return (this->name < container->name);
}

QDebug operator <<(QDebug d, const Container* container) {
    d << "Container";
    d << container->name;
    d << " Size";
    d << container->items.size();
    return d;
}


qorgPasswordManagerModel::qorgPasswordManagerModel(QObject* parent) : QObject(parent) {

}

