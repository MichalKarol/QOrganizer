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

#include "qorgcalendarmodel.h"

// IO functions
bool Event::input(QString line, uint version) {
    QList <QString> list = line.split(" ");
    bool output = false;
    int control = (version < 105 ? list.size() - 1 : list.size());

    if (control > 0) {
        if (version < 104 && control >= 5) {
            name = inputString(list[0]);
            category = inputString(list[1]);
            priority = inputUchar(list[2]);

            if (control == 7) {
                occurance = static_cast<Event::Type>(inputUchar(list[3]));
                dateTimeStart = inputDateTime(list[4]);
                dateTimeEnd = inputDateTime(list[5]);
                dateEnd = inputDate(list[6]);
            } else {
                dateTimeStart = inputDateTime(list[3]);
                dateTimeEnd = inputDateTime(list[4]);
            }

            output = true;
        } else if (control >= 5) {
            name = inputString(list[0]);
            category = inputString(list[1]);
            priority = inputUchar(list[2]);
            dateTimeStart = inputDateTime(list[3]);
            dateTimeEnd = inputDateTime(list[4]);

            if (control == 7) {
                occurance = static_cast<Event::Type>(inputUchar(list[5]));
                dateEnd = inputDate(list[6]);
            }

            output = true;
        }
    }

    return output;
}
QString Event::output() {
    QString output;
    output.append(outputFilter(name) + " ");
    output.append(outputFilter(category) + " ");
    output.append(outputFilter(priority) + " ");
    output.append(outputFilter(dateTimeStart) + " ");
    output.append(outputFilter(dateTimeEnd) + " ");

    if (occurance != Event::NoOccurance) {
        output.append(outputFilter(static_cast<uchar>(occurance)) + " ");
        output.append(outputFilter(dateEnd) + "\n");
    }

    output.append("\n");
    return output;
}
QString Event::exportToVEvent() {
    QString vevent;
    vevent.append("BEGIN:VEVENT\n");
    vevent.append("UID: " + QUuid::createUuidV5(QUuid(), QString(name + category
                  + dateTimeStart.toString(Qt::ISODate)
                  + dateTimeEnd.toString(Qt::ISODate)).toUtf8()).toString().mid(1, 36) + "\n");
    vevent.append("DTSTART:" + dateTimeStart.toUTC().toString("yyyyMMddThhmmss") + "\n");
    vevent.append("DTEND:" + dateTimeEnd.toUTC().toString("yyyyMMddThhmmss") + "\n");
    QString priorityString;

    switch (priority) {
        case 5: {
            priorityString = "1";
        };

        break;
        case 4: {
            priorityString = "3";
        };

        break;
        case 3: {
            priorityString = "5";
        };

        break;
        case 2: {
            priorityString = "7";
        };

        break;
        case 1: {
            priorityString = "9";
        };

        break;
    }

    vevent.append("PRIORITY:" + priorityString + "\n");
    vevent.append("SUMMARY:" + name + "\n");
    vevent.append("DESCRIPTION:" + category + "\n");

    if (occurance != Event::NoOccurance) {
        vevent.append("RRULE:FREQ=");

        switch (occurance) {
            case Event::NoOccurance: {
            };

            break;
            case Event::Daily: {
                vevent.append("DAILY");
            };

            break;
            case Event::Weekly: {
                vevent.append("WEEKLY");
            };

            break;
            case Event::Monthly: {
                vevent.append("MONTHLY");
            };

            break;
            case Event::Yearly: {
                vevent.append("YEARLY");
            };

            break;
        }

        if (dateEnd.year() != 2099) {
            vevent.append(";UNTIL=" + QDateTime(dateEnd).toUTC().toString("yyyyMMddThhmmss"));
        }

        vevent.append("\n");
    }

    QString valarm;
    valarm.append("BEGIN:VALARM\n");
    valarm.append("TRIGGER:-PT30M\n");
    valarm.append("ACTION:AUDIO\n");
    valarm.append("END:VALARM\n");

    vevent.append(valarm);
    vevent.append("END:VEVENT\n");
    return vevent;
}

// QDebug + operators
QDebug operator<<(QDebug d, const Event* event) {
    d << "Event";
    d << event->name;
    d << event->category;
    d << event->priority;
    d << event->dateTimeStart;
    d << event->dateTimeEnd;

    if (event->occurance != Event::NoOccurance) {
        d << event->occurance;
        d << event->dateEnd;
    }

    d << "\n";
    return d;
}
bool Event::operator <(const Event* event) {
    bool output = false;

    if (this->dateTimeStart == event->dateTimeStart) {
        if (this->priority == event->priority) {
            output = (this->category < event->category);
        } else {
            output = (this->priority < event->priority);
        }
    } else {
        output = (this->dateTimeStart < event->dateTimeStart);
    }

    return output;
}
bool Event::operator ==(const Event* event) {
    return (name == event->name
            && category == event->category
            && priority == event->priority
            && occurance == event->occurance
            && dateTimeStart == event->dateTimeStart
            && dateTimeEnd == event->dateTimeEnd
            && dateEnd == dateEnd);
}


qorgCalendarModel::qorgCalendarModel(QObject* parent) : QObject(parent) {
}
qorgCalendarModel::~qorgCalendarModel() {
    for (uint i = 0; i < data.size(); i++) {
        delete data[i];
        data[i] = NULL;
    }

    data.clear();
}

// IO functions
void qorgCalendarModel::input(QString input, uint version) {
    if (!input.isEmpty()) {
        QStringList line = input.split("\n");

        for (int i = 0; i < line.size(); i++) {
            Event* event = new Event;

            if (event->input(line[i], version)) {
                data.push_back(event);
            } else {
                delete event;
            }
        }
    }
}
QString qorgCalendarModel::output() {
    QString output;

    for (uint i = 0; i < data.size(); i++) {
        output.append(data[i]->output());
    }

    output.append("\n\n");
    return output;
}
QString qorgCalendarModel::exportToICalendar() {
    QString output;
    output.append("BEGIN:VCALENDAR\nVERSION:2.0\n");

    for (uint i = 0; i < data.size(); i++) {
        output.append(data[i]->exportToVEvent());
    }

    output.append("END:VCALENDAR");
    return output;
}

// Functions to change model by controller
void qorgCalendarModel::setDate(QDate date) {
    if (this->currentDate != date) {
        this->currentDate = date;
        emit dateChanged(date);
    }
}
void qorgCalendarModel::setCategory(QString category) {
    if (this->currentCategory != category) {
        this->currentCategory = category;
        emit categoryChanged();
    }
}
void qorgCalendarModel::addEvent(Event* event) {

}
void qorgCalendarModel::deleteEvent(uint uid) {

}
void qorgCalendarModel::clear() {
    currentDate = QDate::currentDate();
    currentCategory = QString();

    for (uint i = 0; i < data.size(); i++) {
        delete data[i];
    }

    data.clear();
    emit modelChanged();
}

// Functions to send data to view
QList <QTreeWidgetItem*> qorgCalendarModel::getIncoming() {
    QList <QTreeWidgetItem*> output;

    for (uint i = 0; i < data.size(); i++) {

    }

    return output;
}
QList <QTreeWidgetItem*> qorgCalendarModel::getDayView() {
    QList <QTreeWidgetItem*> output;

    for (uint i = 0; i < data.size(); i++) {

    }

    return output;
}

