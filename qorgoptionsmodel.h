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

#ifndef QORGOPTIONSMODEL_H
#define QORGOPTIONSMODEL_H

#include <qorgtools.h>
#include <QObject>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QNetworkProxy>
#include <QSslCipher>
using std::vector;

class qorgOptionsModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgOptionsModel(QObject* parent = 0);
    ~qorgOptionsModel();

    // IO functions
    void input(QString input, uint version);
    QString output();

    // Functions to change model by controller
    void addAcceptedSslCertificate(QSslCertificate certificate);
    void addBlacklistedSslCertificate(QSslCertificate certificate);
    void addAcceptedSslCipher(QSslCipher cipher);
    void addBlacklistedSslCipher(QSslCipher cipher);
    void setVirusTotalApiKey(QString apiKey);
    void setProxy(QNetworkProxy proxy);
    void setTimes(uint update, uint block);
    void clear();

    // Functions to send data to view
    int checkCertificate(QSslCertificate certificate);
    QString getVirusTotalApiKey();

  private:
    vector <QSslCertificate> acceptedSslCertificates;
    vector <QSslCertificate> blacklistedSslCertificates;

    QList <QSslCipher> acceptedSslCiphers;
    QList <QSslCipher> blacklistedSslCiphers;

    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();

    QString virusTotalApiKey = QString();

    QNetworkProxy currentProxy = QNetworkProxy();

    uint updateTime = 30;
    uint blockTime = 30;

  signals:
    void timesChanged();
};

#endif // QORGOPTIONSMODEL_H
