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
class Event {
public:
    Event();
    enum Type {
        NoOccurance,
        Daily,
        Weekly,
        Monthly,
        Yearly
    };
    QString Name;
    QString Category;
    uchar Priority;
    QDateTime Datet;
    QDateTime Edatet;
    Type OccuranceType;
    QDate Edate;
    bool occursOnDate(QDate);
    QDateTime starts(QDate);
    QDateTime starts(QDateTime);
    QDateTime ends(QDate);
    QDateTime ends(QDateTime);

};

class qorgCalendar: public QWidget {
    Q_OBJECT
public:
    explicit qorgCalendar(QWidget*);
    void setPointer(qorgAB*);
    QString output();
    void input(QString);
    QStringList getCategories();
    void setCategory(QString);
    QString getUpdate();
    QString exportToICalendar();
    QString category;
    vector <Event> Eventv;
private:
    qorgAB* AB;
    void setCalendar();
    void updateAll();
    void sort();
    QList <Event> checkBd(QDate);
    QDate currentDate;
    QGridLayout* Layout;
    QComboBox* Month;
    QPushButton* Yminus;
    QComboBox* Year;
    QPushButton* Yplus;
    QTableWidget* Calendar;
    QTreeWidget* DayView;
    QTreeWidget* Incoming;

    QTimer* MidnightTester;
    QTimer* Midnight;
    QTimer* NTimer;
    // TODO(mkarol) Import from vCalendar
    // TODO(mkarol) whole day events
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
    void Notification(QString, QString);
    void TimeChangeBlock();
};

#endif  // QORGCALENDAR_H_
