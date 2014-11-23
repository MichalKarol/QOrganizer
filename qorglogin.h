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

#ifndef QORGLOGIN_H_
#define QORGLOGIN_H_
#include <qorganizer.h>
#include <qorgtools.h>
#include <qorgio.h>

class QOrganizer;

class qorgLogin :public QDialog {
    Q_OBJECT
public:
    explicit qorgLogin(QOrganizer*);
    void clear();
private:
    QOrganizer* pointer;
    bool Validated[2];
    QLineEdit* Line[2];
    QLabel* Label[2];
    QLabel* PassLabel;
    QPushButton* Login;
    QPushButton* Reg;
private slots:
    void UserInputValidation(QString);
    void PasswordInputValidation(QString);
    void Authentication();
    void Register();
protected:
    QString* hashed;
    QString* hash;
};

#endif  // QORGLOGIN_H_
