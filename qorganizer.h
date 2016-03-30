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

#ifndef QORGANIZER_H_
#define QORGANIZER_H_

#include <qorgtools.h>
#include <qorgmodel.h>
#include <QtWidgets>
#include <QSslCertificate>

class QOrganizer : public QWidget {
    Q_OBJECT

  public:
    QOrganizer(qorgModel* model, QWidget* parent = NULL);
    ~QOrganizer();

  private:
    qorgModel* model = NULL;

    // Main widget
    QStackedWidget* stacked = NULL;
    QTreeWidget* mainTree = NULL;
    QSystemTrayIcon* trayIcon = NULL;

    void mainWidgetGenerator();


    // Login widget
    QWidget* loginWidget = NULL;
    ValidatingLineEdit* loginUsername = NULL;
    ValidatingLineEdit* loginPassword = NULL;
    QLabel* loginInformation = NULL;
    QPushButton* registrationButton = NULL;
    QPushButton* loginButton = NULL;
    bool loggedIn = false;

    QWidget* loginWidgetGenerator();

    // Block widget
    QWidget* blockWidget = NULL;
    ValidatingLineEdit* blockPassword = NULL;
    QPushButton* unlockButton = NULL;

    QWidget* blockWidgetGenerator();

    // Other widgets



  private slots:
    // void logged(bool);
};

// TODO(mkarol) Check all loops!
// TODO(mkarol) AutoUpdate after hibernation, time of last unlocking to prevent unlicking and locking again
// FIXME(mkarol) QThread rebuild; https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
// FIXME(mkarol) 1 minute after suspension/hibernation access
// TODO(mkarol) Bold TreeWidget's children which have: event today or unreded email or unreded post.
// TODO(mkarol) Language support
// NOTE(mkarol) Remove all QtWidget includes
#endif  // QORGANIZER_H_
