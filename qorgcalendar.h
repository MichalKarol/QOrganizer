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

#ifndef QORGCALENDAR_H_
#define QORGCALENDAR_H_
#include <qorgtools.h>
#include <qorgab.h>
#include <QtWidgets>
#include <vector>

using std::vector;
struct CalNor {
    QString name;
    QString category;
    uchar priority;
    QDateTime datet;
    QDateTime edatet;
};
struct CalRec {
    QString name;
    QString category;
    uchar priority;
    uchar type;
    QDateTime datet;
    QDateTime edatet;
    QDate edate;
};

class qorgCalendar: public QWidget {
    Q_OBJECT
public:
    qorgCalendar(QWidget*, qorgAB*);
    QString output();
    void input(QString);
    QStringList getCategories();
    void setCategory(QString);
    QString getUpdate();
    vector <CalNor> Normal;
    vector <CalRec> Recurrent;
    QString category;
private:
    qorgAB* AB;
    void setCalendar();
    void updateAll();
    void sort();
    QList <uint> checkEvN(QDate, char);
    QList <uint> checkEvR(QDate, char);
    QList <QString> checkBd(QDate);
    QDate currentDate;
    QGridLayout* Layout;

    QComboBox* Month;
    QPushButton* Yminus;
    QComboBox* Year;
    QPushButton* Yplus;
    QTableWidget* Calendar;
    QTreeWidget* DayView;
    QTreeWidget* Incoming;
    QList <uint> NotifiedN;
    QList <uint> NotifiedR;
    QTimer* MidnightTester;
    QTimer* Midnight;
    QTimer* NTimer;
private slots:
    void Add(QDate);
    void Edit(uint);
    void Delete(uint);
    void dayChanged(QTableWidgetItem*);
    void doubleClick(QModelIndex);
    void monthChanged(int);
    void yearChanged(QString Input = "");
    void MidnightChange();
    void setNotification(bool first = false);
    void checkMidnight();

signals:
    void updateTree();
    void Notification(QString);
};

#endif  // QORGCALENDAR_H_
