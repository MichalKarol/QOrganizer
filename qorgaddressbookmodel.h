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

#ifndef QORGADDRESSBOOKMODEL_H
#define QORGADDRESSBOOKMODEL_H

#include <QObject>
#include <QDateTime>

class Person {
  private:
    QString name = QString();
    QString surname = QString();
    QString city = QString();
    QString street = QString();
    uint house = 0;
    uint apartment = 0;
    QString email = QString();
    QString mobile = QString();
    QDateTime birthday = QDateTime();
    QByteArray photo = QByteArray();
    QString extraInformation = QString();

    bool operator < (const Person* person);
    friend QDebug operator << (QDebug d, const Person* person);

};

class qorgAddressBookModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgAddressBookModel(QObject* parent = NULL);

  signals:

  public slots:
};

#endif // QORGADDRESSBOOKMODEL_H
