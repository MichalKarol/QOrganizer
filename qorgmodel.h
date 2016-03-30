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

#ifndef QORGMODEL_H
#define QORGMODEL_H
#include <qorgcalendarmodel.h>
#include <qorgmailmodel.h>
#include <qorgnotesmodel.h>
#include <qorgaddressbookmodel.h>
#include <qorgfeedsreadermodel.h>
#include <qorgpasswordmanagermodel.h>
#include <qorgoptionsmodel.h>
#include <qorgtools.h>
#include <QMutexLocker>
#include <QMutex>
#include <QSslCertificate>
#include <QSslCipher>
#include <QMimeType>
#include <QUrl>
#include <QThread>
#include <vector>
using std::vector;



class qorgModel : public QObject {
    Q_OBJECT
  public:
    qorgModel(QObject* parent = 0);

    void loginFunction(QPair <QByteArray, QByteArray> pair, QString path, uint version, QByteArray newHash = QByteArray());
    void registerFunction(QPair <QByteArray, QByteArray> pair, QString path);
    void logout();
    void save();

    // Function for IO
    void inputData(QString parts, uint version, uint nr);
    QString outputData(uint nr);

    // Publicly availiable other models
    qorgCalendarModel* calendar = new qorgCalendarModel(this);
    // FIX(mkarol) qorgMailModel *mail = new qorgMailModel(this);
    qorgNotesModel* notes = new qorgNotesModel(this);
    qorgAddressBookModel* addressbook = new qorgAddressBookModel(this);
    qorgFeedsReaderModel* feedsreader = new qorgFeedsReaderModel(this);
    qorgPasswordManagerModel* passwordmanager = new qorgPasswordManagerModel(this);
    qorgOptionsModel* options = new qorgOptionsModel(this);

  private:
    // Basic user data
    QString user = QString();
    QByteArray hash = QByteArray();
    QByteArray hashed = QByteArray();
    uint version = 105;

    // IO functions
    void input(QString input, uint version);
    QString output();

  public slots:

};





#endif // QORGMODEL_H
