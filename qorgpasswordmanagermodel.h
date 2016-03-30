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

#ifndef QORGPASSWORDMANAGERMODEL_H
#define QORGPASSWORDMANAGERMODEL_H

#include <QObject>
#include <QDebug>
using std::vector;

class Container {
  public:

  private:
    class Item {
        QString login = QString();
        QString password = QString(); // TODO(mkarol) Create QSecureString to securly handle passwords

        friend QDebug operator << (QDebug d, const Item* item);
    };
    QString name = QString();
    vector <Item*> items;

    bool operator <(const Container* container);
    friend QDebug operator <<(QDebug d, const Container* container);
};

class qorgPasswordManagerModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgPasswordManagerModel(QObject* parent = 0);
  private:
    vector <Container*> data;
  public slots:

  signals:
    void refresh();
    void refreshFullView();
};

#endif // QORGPASSWORDMANAGERMODEL_H
