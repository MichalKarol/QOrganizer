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

#ifndef QORGANIZER_H_
#define QORGANIZER_H_
#include <qorgio.h>
#include <qorglogin.h>
#include <qorgcalendar.h>
#include <qorgoptions.h>
#include <qorgmail.h>
#include <qorgnotes.h>
#include <qorgab.h>
#include <qorgrss.h>
#include <qorgpasswd.h>
#include <QtWidgets>

class QOrganizer : public QWidget {
    Q_OBJECT
public:
    QOrganizer();
    ~QOrganizer();
    qorgCalendar* Calendar;
    qorgMail* Mail;
    qorgNotes* Notes;
    qorgAB* AddressBook;
    qorgRSS* RSS;
    qorgPasswd* PasswordManager;
    qorgOptions* Options;
    void setUser(QString, QString*, QString*);
private:
    class VersionUpdater;
    VersionUpdater* VU;
    QSystemTrayIcon* Tray;
    QTreeWidget* TreeWidget;
    QStackedWidget* Stacked;
    QString Updates[3];
    void setTree();
    void closeEvent(QCloseEvent*);
    bool shown;
    bool closing;
    QLineEdit* BlockL;
    QPushButton* OKB;
protected:
    QString user;
    QString* hash;
    QString* hashed;
private slots:
    void launchFunction(QTreeWidgetItem*);
    void doubleClick(QString);

    void updateCalendar();

    void updateMail();
    void MailNews(QString);

    void updateAddressBook();

    void updateRSS();
    void RSSNews(QString);

    void TrayClick(QSystemTrayIcon::ActivationReason);
    void VersionUpdate(QString);

    void updateTime();
    void Block();
    void NewPassword(QString*, QString*, QString*, QString*);

    // Block
    void Unlock();
public slots:
    void Notification(QString,QString);
};
#endif  // QORGANIZER_H_
