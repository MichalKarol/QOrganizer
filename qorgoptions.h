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

#ifndef QORGOPTIONSH
#define QORGOPTIONSH

#include <qorgtools.h>
#include <qorgcalendar.h>
#include <qorgnotes.h>
#include <qorgab.h>
#include <QSslCertificate>
#include <QtWidgets>
#include <vector>

using std::vector;
class qorgOptions : public QWidget {
    Q_OBJECT
public:
    explicit qorgOptions(QWidget*);

    uint UpdateInterval;
    uint BlockInterval;
    QMutex CertificateQMutex;

    void setPointers(qorgCalendar*, qorgNotes*, qorgAB*);
    void setWidget(int);

    QString output();
    void input(QString);

    int checkCertificate(QSslCertificate);
    void acceptSSLCert(QSslCertificate);
    void blacklistSSLCert(QSslCertificate);
    void addForVerification(QSslCertificate);
private:
    vector <QSslCertificate> SSLCertA;
    vector <QSslCertificate> SSLCertB;
    QList <QSslCertificate> SSLCertTmp;

    qorgCalendar* Calendar;
    qorgNotes* Notes;
    qorgAB* AddressBook;


    QTimer* UpdateQTimer;
    QTimer* BlockQTimer;
    int currentWidget;
    QGridLayout* LayoutQGridLayout;
    QWidget* WidgetsQWidget[4];
    QGroupBox* BoxesQGroupBox[3];

    //Password changing widget
    QLabel* CurrentPasswordQLabel;
    QLabel* NewPasswordQLabel;
    QLineEdit* CurrentPasswordQLineEdit;
    QLineEdit* NewPasswordQLineEdit;
    QPushButton* PasswordQPushButton;

    //Interval changing widget
    QLabel* UpdateQLabel;
    QLabel* BlockQLabel;
    QSpinBox* UpdateIntervalQSpinBox;
    QSpinBox* BlockIntervalQSpinBox;
    QPushButton* IntervalQPushButton;

    //Export widget
    QLabel* vCalendarQLabel;
    QLabel* vNoteQLabel;
    QLabel* vCardQLabel;
    QPushButton* vCalendarQPushButton;
    QPushButton* vNoteQPushButton;
    QPushButton* vCardQPushButton;

    //SSL manager widget
    QLabel* AcceptedQLabel;
    QLabel* BlacklistedQLabel;
    QListWidget* AcceptedQListWidget;
    QListWidget* BlacklistedQListWidget;

public slots:
    void start(bool);
    void stop(bool);
private slots:
    void UTimeout();
    void BTimeout();
    void ChangePassword();
    void ChangeInterval();
    void ExportToVSth();
    void Validator(QString);
    void CertificateClicked(QModelIndex);
signals:
    void CNPassword(QByteArray, QByteArray, QByteArray, QByteArray);
    void Update();
    void Block();
};

#endif  // QORGOPTIONSH
