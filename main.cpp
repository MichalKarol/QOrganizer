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

#include <qorganizer.h>
#include <qorgmodel.h>
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    //app.setQuitOnLastWindowClosed(false); // FIXME (mkarol) Enable when QSystemTrayIcon is done

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);

    qorgModel* model = new qorgModel();
    QOrganizer* view = new QOrganizer(model);
    view->show();

    return app.exec();
}
