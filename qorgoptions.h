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

#ifndef QORGOPTIONS_H_
#define QORGOPTIONS_H_

#include <qorgtools.h>
#include <QSslCertificate>
#include <QtWidgets>
#include <vector>

using std::vector;
class qorgOptions : public QWidget {
    Q_OBJECT
public:
    explicit qorgOptions(QWidget* parent);
    ~qorgOptions();
    int checkCertificate(QSslCertificate);
    void setWidget(uint);
    QString output();
    void input(QString);
    void acceptSSLCert(QSslCertificate);
    void blacklistSSLCert(QSslCertificate);
    void addForVeryfication(QSslCertificate);
    uint UInterval;
    uint BInterval;
    QMutex CertMutex;
private:
    vector <QSslCertificate> SSLCertA;
    vector <QSslCertificate> SSLCertB;
    QList <QSslCertificate> SSLCertTmp;
    QTimer* UTimer;
    QTimer* BTimer;
    uint currentW;
    QGridLayout* Layout;

    QLabel* A[4];
    QLineEdit* CPassword;
    QLineEdit* NPassword;
    QSpinBox* UInt;
    QSpinBox* BInt;
    QPushButton* Passwd;
    QPushButton* Interval;
    QSpacerItem* Spacer;
    QList <QWidget*> W1;

    QLabel* B[2];
    QListWidget* Accepted;
    QListWidget* Blacklisted;
    QList <QWidget*> W2;
public slots:
    void start(bool);
    void stop(bool);
private slots:
    void UTimeout();
    void BTimeout();
    void ChangeInterval();
    void ChangePassword();
    void Validator(QString);
    void DClicked(QModelIndex);
signals:
    void CNPassword(QString*, QString*, QString*, QString*);
    void Update();
    void Block();
};

#endif  // QORGOPTIONS_H_
