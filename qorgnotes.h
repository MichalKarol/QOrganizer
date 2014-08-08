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

#ifndef QORGNOTES_H_
#define QORGNOTES_H_
#include <qorgtools.h>
#include <QtWidgets>
#include <vector>

using std::vector;
using std::swap;
struct Note {
    QString Topic;
    unsigned char Priority;
    QString Text;
};

class qorgNotes : public QWidget {
    Q_OBJECT
public:
    explicit qorgNotes(QWidget*);
    QString output();
    void input(QString);
    vector  <Note>  Notes;
private:
    void updateList();
    QGridLayout *Layout;
    QLabel* TLabel;
    QLineEdit *Topic;
    QSlider *Priority;
    QTextEdit *Text;
    QPushButton *OK;
    QPushButton *Cancel;
    QTreeWidget *List;
    QPushButton *AddB;
    int edited;
private slots:
    void Add();
    void Edit(uint);
    void Delete(uint);
    void EOK();
    void EC();
    void EClicked(QModelIndex);
    void ChangeT();
    void ChangeX();
};

#endif  // QORGNOTES_H_
