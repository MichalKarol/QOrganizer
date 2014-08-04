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

#include <qorganizer.h>
#include <qorglogin.h>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QOrganizer *w = new QOrganizer();
    w->setAttribute(Qt::WA_DeleteOnClose);
    if ((new qorgLogin(w))->exec() == QDialog::Accepted) {
       w->show();
     return a.exec();
    } else {
        return 0;
    }
}
