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
    qorgCalendar *Calendar;
    qorgMail *Mail;
    qorgNotes *Notes;
    qorgAB *AdressBook;
    qorgRSS *RSS;
    qorgPasswd *PasswordManager;
    void setUser(QString, QString*, QString*);
    uint UInterval;
    uint BInterval;
private:
    QTimer *UTimer;
    QSystemTrayIcon *Tray;
    QTreeWidget *TreeWidget;
    QStackedWidget *Stacked;
    QString Updates[3];
    void setTree();
    void closeEvent(QCloseEvent *);
    bool closing;
    // Settings
    QLineEdit *CP;
    QLineEdit *NP;
    QPushButton *Change;
    QSpinBox *UpdateInterval;
    QSpinBox *BlockInterval;
    QPushButton *ChangeInterval;
    QTimer *BTimer;
    QLineEdit *BlockL;
    QPushButton *OKB;
    bool shown;
protected:
    QString user;
    QString *hash;
    QString *hashed;
private slots:
    void launchFunction(QTreeWidgetItem*);
    void updateCalendar();
    void Notification(QString);
    void updateMail();
    void updateAdressBook();
    void updateRSS();
    void doubleClick(QString);
    void TrayClick(QSystemTrayIcon::ActivationReason);
    void MailNews(QString);
    void RSSNews(QString);
    void updateTime();
    void Block();

    // Settings
    void Validator(QString);
    void NewPassword();
    void SetInterval();

    // Block
    void Unlock();
};
#endif  // QORGANIZER_H_
