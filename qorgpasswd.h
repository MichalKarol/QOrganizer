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

#ifndef QORGPASSWD_H_
#define QORGPASSWD_H_
#include <qorgtools.h>
#include <QtWidgets>
#include <vector>

using std::vector;
class Password {
public:
    Password();
    ~Password();
    QString Login;
    QString Passwd;
};

class Program {
public:
    Program();
    ~Program();
    QString Name;
    vector  <Password*>  Passwordv;
};

class qorgPasswd : public QWidget {
    Q_OBJECT
public:
    explicit qorgPasswd(QWidget*);
    ~qorgPasswd();
    QString output();
    void input(QString);
private:
    vector  <Program>  Programv;
    QGridLayout* La;
    QTreeWidgetItem* lastitem;
    QPoint Last;
    QTreeWidget* Tree;
    QLabel* L[3];
    QLineEdit* ProgramL;
    QLineEdit* LoginL;
    QLineEdit* PasswordL;
    QCompleter* C;
    QPushButton* Change;
    QPushButton* Add;
    QPushButton* OKB;
    QPushButton* Cancel;
    void UpdateTree();
private slots:
    void change();
    void AddB();
    void row(QString);
    void clicked(QTreeWidgetItem*);
    void Edit(QPoint);
    void Delete(QPoint);
    void OK();
    void Can();
};

#endif  // QORGPASSWD_H_
