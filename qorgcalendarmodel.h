//    Copyright (C) 2015 Michał Karol <michal.p.karol@gmail.com>

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

#ifndef QORGCALENDARMODEL_H
#define QORGCALENDARMODEL_H

#include <qorgtools.h>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <QObject>

using std::vector;

class Event {
  public:

    // IO functions
    bool input(QString line, uint version);
    QString output();
    QString exportToVEvent();

    bool occureOnDate(QDate);
    QDateTime starts(QDate);
    QDateTime starts(QDateTime);
    QDateTime ends(QDate);
    QDateTime ends(QDateTime);
  private:
    enum Type {
        NoOccurance,
        Daily,
        Weekly,
        Monthly,
        Yearly
    };

    // Data fields
    QString name = QString();
    QString category = QString();
    uchar priority = 0;
    Type occurance = Type::NoOccurance;
    QDateTime dateTimeStart = QDateTime::currentDateTime();
    QDateTime dateTimeEnd = QDateTime::currentDateTime();
    QDate dateEnd = QDate::currentDate();

    // QDebug + operators
    bool operator <(const Event* event);
    bool operator ==(const Event* event); // NOTE(mkarol) Too strict.
    friend QDebug operator<<(QDebug d, const Event* event);
};


class qorgCalendarModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgCalendarModel(QObject* parent = 0);
    ~qorgCalendarModel();

    // IO functions
    void input(QString input, uint version);
    QString output();
    QString exportToICalendar();
    QString statusRaport();

    // Functions to change model by controller
    void setDate(QDate date);
    void setCategory(QString category);
    void addEvent(Event* event);
    void deleteEvent(uint uid);
    void clear();

    // Functions to retreive needed information from model by controller
    QDate getDate();

    // Functions to send data to view
    void setCalendar(QTableWidget* widget); // FIXME(mkarol) MVC problem
    // TODO(mkarol) Setting icons to QTableWidgetItem
    QList <QTreeWidgetItem*> getIncoming();
    QList <QTreeWidgetItem*> getDayView();

  private:
    vector <Event*> data;
    QDate currentDate = QDate::currentDate();
    QString currentCategory = QString();

  signals:
    void dateChanged(QDate);
    void categoryChanged();
    void modelChanged();
};
// TODO(mkarol) Midnight date change
// TODO(mkarol) Fix notifications
// TODO(mkarol) Import from vCalendar
// TODO(mkarol) Whole day events
// TODO(mkarol) Fix negative birthdays
// TODO(mkarol) Function to get birthdays

#endif // QORGCALENDARMODEL_H
