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

#include <qorgcalendar.h>
#include <algorithm>

Event::Event() {
    this->Name.clear();
    this->Category.clear();
    this->Priority = 0;
    this->OccuranceType = Event::NoOccurance;
    this->Datet = QDateTime::fromMSecsSinceEpoch(0);
    this->Edatet = QDateTime::fromMSecsSinceEpoch(0);
    this->Edate = QDateTime::fromMSecsSinceEpoch(0).date();
}
bool Event::occursOnDate(QDate D) {
    if (D >= Datet.date()
            && ((OccuranceType == NoOccurance && D <= Edatet.date())
                ||  (OccuranceType != NoOccurance && D <= Edate))) {
        if (!QDate::isLeapYear(D.year())
                && OccuranceType == Yearly
                && Datet.date().day() == 29
                && Datet.date().month() == 2) {
            if (D .month() == 2
                 && D.day() == 28) {
                return true;
            } else {
                return false;
            }
        }
        switch (OccuranceType) {
        case NoOccurance: {
            return true;
        }; break;
        case Daily: {
            return true;
        }; break;
        case Weekly: {
            if  (Datet.daysTo(Edatet) < 7) {
                if (Edatet.date().dayOfWeek() -  Datet.date().dayOfWeek() >= 0)  {
                    if (Datet.date().dayOfWeek() <= D.dayOfWeek()
                            && D.dayOfWeek() <= Edatet.date().dayOfWeek()) {
                        return true;
                    }
                } else {
                    if (Datet.date().dayOfWeek() <= D.dayOfWeek()
                            || D.dayOfWeek() <= Edatet.date().dayOfWeek()) {
                        return true;
                    }
                }
            } else {
                return true;
            }
        }; break;
        case Monthly: {
            if (Datet.date().month() == Edatet.date().month()) {
                if  (Datet.date().day() <= D.day()
                     && D.day() <= Edatet.date().day()) {
                    return true;
                }
            } else {
                if (Datet.date().day() > Edatet.date().day()) {
                    if (D.day() <= Edatet.date().day()
                            ||  Datet.date().day() <= D.day()) {
                        return true;
                    }
                } else {
                    return true;
                }
            }
        }; break;
        case Yearly: {
            if (Datet.date().year() == Edatet.date().year()) {
                if (Datet.date().month() == Edatet.date().month()
                        && D.month() == Datet.date().month()) {
                    if (Datet.date().day() <= D.day()
                            && D.day() <= Edatet.date().day()) {
                        return true;
                    }
                } else {
                    if (Datet.date().month() == D.month()
                            && Datet.date().day() <= D.day()) {
                        return true;
                    } else if (D.month() == Edatet.date().month()
                               && D.day() <= Edatet.date().day()) {
                        return true;
                    } else if (Datet.date().month() < D.month()
                               && D.month() < Edatet.date().month()) {
                        return true;
                    }
                }
            } else {
                if (Datet.date().month() == Edatet.date().month()
                        && D.month() == Datet.date().month()) {
                    if (Datet.date().day() > Edatet.date().day()) {
                        if (Datet.date().day() <= D.day()
                                || D.day() <= Edatet.date().day()) {
                            return true;
                        }
                    } else {
                        return true;
                    }
                } else if (Datet.date().month() > Edatet.date().month()) {
                    if (Edatet.date().month() == D.month()
                            && D.day() <= Edatet.date().day()) {
                        return true;
                    } else if (D.month() <Edatet.date().month()
                               ||  Datet.date().month() < D.month()) {
                        return true;
                    }  else if  (Datet.date().month() == D.month()
                                 && Datet.date().day() <= D.day()) {
                        return true;
                    }
                } else {
                    return true;
                }
            }
        }; break;
        }

    }

    return false;
}
QDateTime Event::starts(QDateTime D) {
    if (Datet <= D) {
        if (!QDate::isLeapYear(D.date().year())
                && D.date().month() == 2
                && D.date().day() == 28
                && OccuranceType == Yearly
                && Datet.date().day() == 29
                && Datet.date().month() == 2) {
            return QDateTime(D.date(), QTime(8, 0));
        }
        if (OccuranceType != NoOccurance) {
            switch (OccuranceType) {
            case NoOccurance: {
            }; break;
            case Daily: {
                if (Datet.date() != D.date()) {
                    if (D.time() < Datet.time()) {
                        return QDateTime(D.date().addDays(-1), Datet.time());
                    } else {
                        return QDateTime(D.date(), Datet.time());
                    }
                }
            }; break;
            case Weekly: {
                if (Datet.date() != D.date()) {
                    if (Datet.date().dayOfWeek() == D.date().dayOfWeek()) {
                        if (D.time() < Datet.time()) {
                            return QDateTime(D.date().addDays(-7), Datet.time());
                        } else {
                            return QDateTime(D.date(), Datet.time());
                        }
                    } else {
                        return QDateTime(D.date().addDays(-((D.date().dayOfWeek()+7-Datet.date().dayOfWeek())%7)), Datet.time());
                    }
                }
            }; break;
            case Monthly: {
                if (Datet.date() != D.date()) {
                    if (Datet.date().day() == D.date().day()) {
                        if (D.time() < Datet.time()) {
                            return QDateTime(D.date().addMonths(-1), Datet.time());
                        } else {
                            return QDateTime(D.date(), Datet.time());
                        }
                    } else if (Datet.date().day() < D.date().day()) {
                        return QDateTime(QDate(D.date().year(), D.date().month(), Datet.date().day()), Datet.time());
                    } else {
                        return QDateTime(QDate(D.date().addMonths(-1).year(), D.date().addMonths(-1).month(), Datet.date().day()), Datet.time());
                    }
                }
            }; break;
            case Yearly: {
                if (Datet.date() != D.date()) {
                    if (Datet.date().month() == D.date().month()) {
                        if (Datet.date().day() == D.date().day()) {
                            if (D.time() < Datet.time()) {
                                return QDateTime(D.date().addYears(-1), Datet.time());
                            } else {
                                return QDateTime(D.date(), Datet.time());
                            }
                        } else if (Datet.date().day() < D.date().day()) {
                            return QDateTime(QDate(D.date().year(), Datet.date().month(), Datet.date().day()), Datet.time());
                        } else {
                            return QDateTime(QDate(D.date().addYears(-1).year(), Datet.date().month(), Datet.date().day()), Datet.time());
                        }
                    } else if (Datet.date().month() < D.date().month()) {
                        return QDateTime(QDate(D.date().year(), Datet.date().month(), Datet.date().day()), Datet.time());
                    } else {
                        return QDateTime(QDate(D.date().addYears(-1).year(), Datet.date().month(), Datet.date().day()), Datet.time());
                    }
                }
            }; break;
            }
        }
    }
    return Datet;
}
QDateTime Event::ends(QDateTime D) {
    QDateTime S = starts(D);
    if (Datet <= S) {
        if (!QDate::isLeapYear(D.date().year())
                && D.date().month() == 2
                && D.date().day() == 28
                && OccuranceType == Yearly
                && Datet.date().day() == 29
                && Datet.date().month() == 2) {
            return QDateTime(D.date(), QTime(12, 0));
        }
        if (OccuranceType != NoOccurance) {
            switch (OccuranceType) {
            case NoOccurance: {
            }; break;
            case Daily: {
                if (Edatet >= Datet.addDays(1)) {
                    return S.addDays(1).addSecs(-1*60);
                } else {
                    return QDateTime(S.addDays(Datet.daysTo(Edatet)).date(), Edatet.time());
                }
            }; break;
            case Weekly: {
                if (Edatet >= Datet.addDays(7)) {
                    return S.addDays(7).addSecs(-1*60);
                } else {
                    return QDateTime(S.addDays(Datet.daysTo(Edatet)).date(), Edatet.time());
                }
            }; break;
            case Monthly: {
                if (Edatet >= Datet.addMonths(1)) {
                    return S.addMonths(1).addSecs(-1*60);
                } else {
                    if (Datet.date().year() == Edatet.date().year()) {
                        if (Datet.date().month() == Edatet.date().month()) {
                            return QDateTime(QDate(S.date().year(), S.date().month(), Edatet.date().day()), Edatet.time());
                        } else {
                            return QDateTime(QDate(S.date().year(), S.date().addMonths(1).month(), Edatet.date().day()), Edatet.time());
                        }
                    } else {
                        return QDateTime(QDate(S.date().addYears(1).year(), Edatet.date().month(), Edatet.date().day()), Edatet.time());
                    }
                }
            }; break;
            case Yearly: {
                if (Edatet >= Datet.addYears(1)) {
                    return S.addYears(1).addSecs(-1*60);
                } else {
                    if (Datet.date().year() == Edatet.date().year()) {
                        return QDateTime(QDate(S.date().year(), Edatet.date().month(), Edatet.date().day()), Edatet.time());
                    } else {
                        return QDateTime(QDate(S.date().addYears(1).year(), S.date().month(), Edatet.date().day()), Edatet.time());
                    }
                }
            }; break;
            }
        }
        return Edatet;
    }
    return QDateTime();
}


class QTableCalendarWidget: public QWidget {
    Q_OBJECT
public:
    QTableCalendarWidget(QString ToolTip, bool Ev, bool BDay, QWidget* parent) :QWidget(parent) {
        Date = QDate::fromString(ToolTip, "dd.MM.yyyy");
        Event = new QLabel(this);
        Birth = new QLabel(this);
        Label = new QLabel(QString::number(Date.day()), this);
        Label->setAlignment(Qt::AlignHCenter);
        QPixmap w = QPixmap(12, 12);
        w.fill(Qt::transparent);
        Event->setPixmap((Ev ? QIcon(":/cal/Event.png").pixmap(12) : w));
        Birth->setPixmap((BDay ? QIcon(":/cal/Birthday.png").pixmap(12) : w));
        Add = new QPushButton(QIcon(QIcon(w).pixmap(12, 12)), "", this);
        Add->setStyleSheet("QPushButton {border: 0px solid white;}");
        Add->setEnabled(false);
        connect(Add, SIGNAL(clicked()), this, SLOT(addIN()));
        QHBoxLayout* Layout = new QHBoxLayout(this);
        Layout->setSpacing(1);
        Layout->addWidget(Event);
        Layout->addWidget(Birth);
        Layout->addWidget(Label, Qt::AlignHCenter);
        Layout->addWidget(Add);
    }
    void AddB() {
        Add->setIcon(QIcon(QIcon(":/main/Add.png").pixmap(15, 15)));
        Add->setEnabled(true);
    }
private:
    QDate Date;
    QLabel* Event;
    QLabel* Birth;
    QLabel* Label;
    QPushButton* Add;
private slots:
    void addIN() {
        emit addOUT(Date);
    }
signals:
    void addOUT(QDate);
};
class QCalDialog: public QDialog {
    Q_OBJECT
public:
    QCalDialog(QDate Date, qorgCalendar* Cal) :QDialog(Cal) {
        Calendar = Cal;
        Labels[0]=new QLabel("Name: ", this);
        Labels[1]=new QLabel("Category: ", this);
        Labels[2]=new QLabel("Priority: ", this);
        Labels[3]=new QLabel("Occurance: ", this);
        Labels[4]=new QLabel("Start: ", this);
        Labels[5]=new QLabel("Finish: ", this);
        Labels[6]=new QLabel("Date of occurane ending: ", this);
        Labels[6]->setDisabled(true);
        Name = new QLineEdit(this);
        Category = new QLineEdit(Calendar->category, this);
        QCompleter* completer = new QCompleter(Calendar->getCategories(), Category);
        Category->setCompleter(completer);
        Priority = new QSlider(Qt::Horizontal, this);
        Priority->setRange(1, 5);
        Priority->setPageStep(1);
        Priority->setSliderPosition(3);
        Priority->setStyleSheet("QSlider::groove:horizontal"
                                "{ "
                                "border: 1px solid #999999;"
                                "height: 8px;"
                                "background: qlineargradient(spread:pad, x1:0,  y1:0, x2:1, y2:0, stop:0 blue, stop:0.25 purple, stop:0.5 white, stop:0.75 orange, stop:1 red);"
                                "margin: 2px 0;"
                                "}"
                                "QSlider::handle:horizontal {"
                                "background: grey;"
                                "border: 1px solid #5c5c5c;"
                                "width: 18px;"
                                "margin: -2px 0;"
                                "border-radius: 3px;"
                                "}");
        Occurance = new QComboBox(this);
        Occurance->addItems(QStringList() << "No occurance" << "Daily" << "Weekly" << "Monthy" << "Yearly");
        connect(Occurance, SIGNAL(currentIndexChanged(int)), this, SLOT(lock(int)));
        CWidgetS = new QCalendarWidget(this);
        CWidgetF = new QCalendarWidget(this);
        CWidgetE = new QCalendarWidget(this);
        Start = new QDateTimeEdit(QDateTime(Date, QTime(12, 00)), this);
        Start->setCalendarPopup(true);
        Start->setCalendarWidget(CWidgetS);
        connect(Start, SIGNAL(timeChanged(QTime)), this, SLOT(time(QTime)));
        Finish = new QDateTimeEdit(QDateTime(Date, QTime(13, 00)), this);
        Finish->setCalendarPopup(true);
        Finish->setCalendarWidget(CWidgetF);
        OccuraneEndDate = new QDateEdit(Date, this);
        OccuraneEndDate->setCalendarPopup(true);
        OccuraneEndDate->setCalendarWidget(CWidgetE);
        OccuraneEndDate->setDisabled(true);
        ActionButtons[0]=new QPushButton(this);
        ActionButtons[0]->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(ActionButtons[0], SIGNAL(clicked()), this, SLOT(cancelled()));
        ActionButtons[1]=new QPushButton(this);
        ActionButtons[1]->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        ActionButtons[1]->setDefault(true);
        connect(ActionButtons[1], SIGNAL(clicked()), this, SLOT(add()));
        QGridLayout* Lay = new QGridLayout(this);
        for (int i = 0; i < 7; i++) {
            Lay->addWidget(Labels[i], i, 0);
        }
        Lay->addWidget(Name, 0, 1);
        Lay->addWidget(Category, 1, 1);
        Lay->addWidget(Priority, 2, 1);
        Lay->addWidget(Occurance, 3, 1);
        Lay->addWidget(Start, 4, 1);
        Lay->addWidget(Finish, 5, 1);
        Lay->addWidget(OccuraneEndDate, 6, 1);
        Lay->addWidget(ActionButtons[0], 7, 0);
        Lay->addWidget(ActionButtons[1], 7, 1);
        setWindowTitle("Add event");
    }
    QCalDialog(uint IID, qorgCalendar* Cal) :QDialog(Cal) {
        Calendar = Cal;
        ItemID = IID;
        Event Tmp = Calendar->Eventv[IID];
        Labels[0]=new QLabel("Name: ", this);
        Labels[1]=new QLabel("Category: ", this);
        Labels[2]=new QLabel("Priority: ", this);
        Labels[3]=new QLabel("Occurance: ", this);
        Labels[4]=new QLabel("Start: ", this);
        Labels[5]=new QLabel("Finish: ", this);
        Labels[6]=new QLabel("Date of occurane ending: ", this);
        Labels[6]->setDisabled(true);
        Name = new QLineEdit(Tmp.Name, this);
        Category = new QLineEdit(Tmp.Category, this);
        QCompleter* completer = new QCompleter(Calendar->getCategories(), Category);
        Category->setCompleter(completer);
        Priority = new QSlider(Qt::Horizontal, this);
        Priority->setRange(1, 5);
        Priority->setPageStep(1);
        Priority->setStyleSheet("QSlider::groove:horizontal"
                                "{ "
                                "border: 1px solid #999999;"
                                "height: 8px;"
                                "background: qlineargradient(spread:pad, x1:0,  y1:0, x2:1, y2:0, stop:0 blue, stop:0.25 purple, stop:0.5 white, stop:0.75 orange, stop:1 red);"
                                "margin: 2px 0;"
                                "}"
                                "QSlider::handle:horizontal {"
                                "background: grey;"
                                "border: 1px solid #5c5c5c;"
                                "width: 18px;"
                                "margin: -2px 0;"
                                "border-radius: 3px;"
                                "}");
        Priority->setSliderPosition(Tmp.Priority);
        Occurance = new QComboBox(this);
        Occurance->addItems(QStringList() << "No occurance" << "Daily" << "Weekly" << "Monthy" << "Yearly");
        connect(Occurance, SIGNAL(currentIndexChanged(int)), this, SLOT(lock(int)));
        CWidgetS = new QCalendarWidget(this);
        CWidgetF = new QCalendarWidget(this);
        CWidgetE = new QCalendarWidget(this);
        Start = new QDateTimeEdit(Tmp.Datet, this);
        Start->setCalendarPopup(true);
        Start->setCalendarWidget(CWidgetS);
        connect(Start, SIGNAL(timeChanged(QTime)), this, SLOT(time(QTime)));
        Finish = new QDateTimeEdit(Tmp.Edatet, this);
        Finish->setCalendarPopup(true);
        Finish->setCalendarWidget(CWidgetF);
        OccuraneEndDate = new QDateEdit(Tmp.Edate, this);
        OccuraneEndDate->setCalendarPopup(true);
        OccuraneEndDate->setCalendarWidget(CWidgetE);
        OccuraneEndDate->setDisabled(true);
        if (Tmp.OccuranceType == Event::NoOccurance) {
            OccuraneEndDate->setDate(QDate::currentDate());
        } else {
            Occurance->setCurrentIndex(Tmp.OccuranceType);
            OccuraneEndDate->setDate(Tmp.Edate);
        }
        ActionButtons[0]=new QPushButton(this);
        ActionButtons[0]->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(ActionButtons[0], SIGNAL(clicked()), this, SLOT(cancelled()));
        ActionButtons[1]=new QPushButton(this);
        ActionButtons[1]->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        ActionButtons[1]->setDefault(true);
        connect(ActionButtons[1], SIGNAL(clicked()), this, SLOT(edit()));
        QGridLayout* Lay = new QGridLayout(this);
        for (int i = 0; i < 7; i++) {
            Lay->addWidget(Labels[i], i, 0);
        }
        Lay->addWidget(Name, 0, 1);
        Lay->addWidget(Category, 1, 1);
        Lay->addWidget(Priority, 2, 1);
        Lay->addWidget(Occurance, 3, 1);
        Lay->addWidget(Start, 4, 1);
        Lay->addWidget(Finish, 5, 1);
        Lay->addWidget(OccuraneEndDate, 6, 1);
        Lay->addWidget(ActionButtons[0], 7, 0);
        Lay->addWidget(ActionButtons[1], 7, 1);
        setWindowTitle("Edit event");
    }
private:
    QLabel* Labels[7];
    QLineEdit* Name;
    QLineEdit* Category;
    QSlider* Priority;
    QComboBox* Occurance;
    QCalendarWidget* CWidgetS;
    QCalendarWidget* CWidgetF;
    QCalendarWidget* CWidgetE;
    QDateTimeEdit* Start;
    QDateTimeEdit* Finish;
    QDateEdit* OccuraneEndDate;
    QPushButton* ActionButtons[2];
    qorgCalendar* Calendar;
    uint ItemID;
private slots:
    void lock(int i) {
        if (i == 0) {
            OccuraneEndDate->setDisabled(true);
        } else {
            Labels[6]->setDisabled(false);
            OccuraneEndDate->setDisabled(false);
            if (i < 3) {
                OccuraneEndDate->setDate(Start->date().addDays(1));
            } else {
                if (Start->date().month() == 2
                        && Start->date().day() == 29) {
                    OccuraneEndDate->setDate(QDate(2099, 2, 28));
                } else {
                    OccuraneEndDate->setDate(QDate(2099, Start->date().month(), Start->date().day()));
                }
            }
        }
    }
    void add() {
        if (Name->text().isEmpty()) {
            if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
            }
            return;
        }
        if (Start->dateTime() >= Finish->dateTime()) {
            if (Finish->styleSheet() != "QDateTimeEdit{background: #FF8888;}") {
                Start->setStyleSheet("QDateTimeEdit{background: #FF8888;}");
                Finish->setStyleSheet("QDateTimeEdit{background: #FF8888;}");
                connect(Start, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
                connect(Finish, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
            }
            return;
        }
        Event::Type OccurancecI = static_cast<Event::Type>(Occurance->currentIndex());
        Event event;
        event.Name = Name->text();
        event.Category = Category->text();
        event.Priority = Priority->value();
        event.Datet = Start->dateTime();
        event.Edatet = Finish->dateTime();
        event.OccuranceType = OccurancecI;
        event.Edate = OccuraneEndDate->date();
        Calendar->Eventv.push_back(event);
        this->accept();
    }
    void edit() {
        if (Name->text().isEmpty()) {
            if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
            }
            return;
        }
        if (Start->dateTime() >= Finish->dateTime()) {
            if (Finish->styleSheet() != "QDateTimeEdit{background: #FF8888;}") {
                Start->setStyleSheet("QDateTimeEdit{background: #FF8888;}");
                Finish->setStyleSheet("QDateTimeEdit{background: #FF8888;}");
                connect(Start, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
                connect(Finish, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
            }
            return;
        }
        Event event;
        event.Name = Name->text();
        event.Category = Category->text();
        event.Priority = Priority->value();
        event.Datet = Start->dateTime();
        event.Edatet = Finish->dateTime();
        event.OccuranceType = static_cast<Event::Type>(Occurance->currentIndex());
        event.Edate = OccuraneEndDate->date();
        Calendar->Eventv.push_back(event);
        Calendar->Eventv.erase(Calendar->Eventv.begin()+ItemID);
        this->accept();
    }
    void changed(QString Input) {
        if (!Input.isEmpty()) {
            Name->setStyleSheet("QLineEdit{background: white;}");
        }
    }
    void cancelled() {
        this->reject();
    }
    void time(QTime T) {
        if (T.hour() == 23) {
            Finish->setDate(Start->date().addDays(1));
            Finish->setTime(T.addSecs(3600));
        } else {
            Finish->setTime(T.addSecs(3600));
        }
    }
    void row() {
        if (!(Start->dateTime() <= Finish->dateTime())) {
            Start->setStyleSheet("QDateTimeEdit{background: white;}");
            Finish->setStyleSheet("QDateTimeEdit{background: white;}");
            disconnect(Start, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
            disconnect(Finish, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(row()));
        }
    }
};

qorgCalendar::qorgCalendar(QWidget* parent) :QWidget(parent) {
    category="";
    currentDate = QDate::currentDate();
    Layout = new QGridLayout(this);
    Month = new QComboBox(this);
    Month->setMinimumWidth(300);
    Month->addItems(QStringList() << "January" << "February" << "March" << "April" << "May" << "June" << "July" << "August" << "September" << "October" << "November" << "December");
    Month->setCurrentIndex(currentDate.month()-1);
    connect(Month, SIGNAL(currentIndexChanged(int)), this, SLOT(monthChanged(int)));
    Yminus = new QPushButton("<", this);
    Yminus->setMinimumWidth(30);
    connect(Yminus, SIGNAL(clicked()), this, SLOT(yearChanged()));
    Year = new QComboBox(this);
    Year->setMinimumWidth(100);
    for (int i = currentDate.year()-20; i < currentDate.year()+21; i++) {
        Year->addItem(QString::number(i));
    }
    Year->setCurrentIndex(20);
    connect(Year, SIGNAL(activated(QString)), this, SLOT(yearChanged(QString)));
    Yplus = new QPushButton(">", this);
    Yplus->setMinimumWidth(30);
    connect(Yplus, SIGNAL(clicked()), this, SLOT(yearChanged()));
    Calendar = new QTableWidget(6, 7, this);
    Calendar->setHorizontalHeaderLabels(QStringList() << "Monday" << "Tuesday" << "Wednesday" << "Thusday" << "Friday" << "Saturday" << "Sunday");
    Calendar->horizontalHeader()->setDefaultSectionSize(78);
    Calendar->verticalHeader()->setDefaultSectionSize(34);
    Calendar->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    Calendar->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    Calendar->setSelectionMode(QAbstractItemView::NoSelection);
    Calendar->setMinimumWidth(400);
    Calendar->setFixedHeight(225);
    connect(Calendar, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(dayChanged(QTableWidgetItem*)));

    DayView = new QTreeWidget(this);
    DayView->setMaximumWidth(350);
    DayView->header()->hide();
    DayView->setColumnCount(5);
    DayView->header()->setStretchLastSection(false);
    DayView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    DayView->setColumnWidth(1, 45);
    DayView->setColumnWidth(2, 45);
    DayView->setColumnWidth(3, 20);
    DayView->setColumnWidth(4, 20);

    Incoming = new QTreeWidget(this);
    Incoming->header()->hide();
    Incoming->setColumnCount(6);
    Incoming->header()->setStretchLastSection(false);
    Incoming->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Incoming->setColumnWidth(1, 120);
    Incoming->setColumnWidth(2, 120);
    Incoming->header()->setSectionResizeMode(3, QHeaderView::Stretch);
    Incoming->setColumnWidth(4, 50);
    Incoming->setColumnWidth(5, 50);
    connect(Incoming, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClick(QModelIndex)));

    QGridLayout* LayoutFx = new QGridLayout();
    LayoutFx->addWidget(Month, 0, 0);
    LayoutFx->addWidget(Yminus, 0, 1);
    LayoutFx->addWidget(Year, 0, 2);
    LayoutFx->addWidget(Yplus, 0, 3);
    LayoutFx->addWidget(Calendar, 1, 0, 1, 4);
    Layout->addLayout(LayoutFx, 0, 0);
    Layout->addWidget(Incoming, 1, 0, 1, 1);
    Layout->addWidget(DayView, 0, 1, 2, 1);
    Layout->setMargin(0);
    NTimer = new QTimer(this);
    connect(NTimer, SIGNAL(timeout()), this, SLOT(setNotification()));
    Midnight = new QTimer(this);
    MidnightTester = new QTimer(this);
    MidnightTester->setInterval(60000);
    MidnightTester->start();
    connect(MidnightTester, SIGNAL(timeout()), this, SLOT(checkMidnight()));
}
void qorgCalendar::setPointer(qorgAB* AB) {
    this->AB = AB;
    MidnightChange();
}
QString qorgCalendar::output() {
    QString out;
    for (uint i = 0; i < Eventv.size(); i++) {
        out.append(Output(Eventv[i].Name)+" ");
        out.append(Output(Eventv[i].Category)+" ");
        out.append(Output(Eventv[i].Priority)+" ");
        out.append(Output(Eventv[i].Datet)+" ");
        out.append(Output(Eventv[i].Edatet)+" ");
        if (Eventv[i].OccuranceType == Event::NoOccurance) {
            out.append("\n");
        } else {
            out.append(Output(static_cast<uchar>(Eventv[i].OccuranceType))+" ");
            out.append(Output(Eventv[i].Edate)+" \n");
        }
    }
    out.append("\n\n");
    return out;
}
void qorgCalendar::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            if (B.size() > 1) {
                Event event;
                event.Name = InputS(B[0]);
                event.Category = InputS(B[1]);
                event.Priority = InputC(B[2]);
                event.Datet = InputDT(B[3]);
                event.Edatet = InputDT(B[4]);
                if (event.Datet > event.Edatet) {
                    event.Edatet = QDateTime(event.Datet.date(), event.Edatet.time());
                }
                if (B.size()-1 == 7) {
                    event.OccuranceType = static_cast<Event::Type>(InputC(B[5]));
                    event.Edate = InputD(B[6]);
                }
                Eventv.push_back(event);
            }
        }
    }
    sort();
    updateAll();
    setNotification(true);
}
QStringList qorgCalendar::getCategories() {
    QStringList list;
    list.append("Birthdays");
    for (uint i = 0; i < Eventv.size(); i++) {
        if (Eventv[i].Category != "" && !(list.contains(Eventv[i].Category))) {
            list.append(Eventv[i].Category);
        }
    }
    list.sort();
    return list;
}
void qorgCalendar::setCategory(QString cat) {
    category = cat;
    Month->blockSignals(true);
    Month->setCurrentIndex(QDate::currentDate().month()-1);
    Month->blockSignals(false);
    Year->blockSignals(true);
    Year->setCurrentIndex(Year->currentIndex()-(currentDate.year()-QDate::currentDate().year()));
    Year->blockSignals(false);
    currentDate = QDate::currentDate();
    updateAll();
}
QString qorgCalendar::getUpdate() {
    int birthdays = checkBd(QDate::currentDate()).size();
    if (Eventv.size()+birthdays != 0) {
        uint hour = 0;
        uint today = 0;
        uint tomorrow = 0;
        for (uint i = 0; i < Eventv.size(); i++) {
            if (Eventv[i].occursOnDate(QDate::currentDate())) {
                if (Eventv[i].starts(QDateTime::currentDateTime()).time() < QTime::currentTime().addSecs(3600)
                        && Eventv[i].starts(QDateTime::currentDateTime()).time() >= QTime::currentTime()) {
                    hour++;
                }
                today++;
            }
            if (Eventv[i].occursOnDate(QDate::currentDate().addDays(1))) {
                tomorrow++;
            }
        }
        QString M1;
        QString M2;
        QString M3;
        QString M4;
        M1 = QString::number(hour)+" event"+(hour == 1? "" : "s")+" during hour.\n";
        M2 = QString::number(today)+" event"+(today == 1? "" : "s")+" today.\n";
        M3 = QString::number(tomorrow)+" event"+(tomorrow == 1? "" : "s")+" tomorrow.\n";
        M4 = QString::number(birthdays)+(birthdays == 1? " person has" : " people have")+" birthday today.";
        return "Calendar: "+M1+QString(16, ' ')+M2+QString(16, ' ')+M3+QString(16, ' ')+M4;
    } else {
        return "Calendar: No event.";
    }
}
QString qorgCalendar::exportToICalendar() {
    QString Output;
    Output.append("BEGIN:VCALENDAR\nVERSION:2.0\n");
    for (uint i = 0; i < Eventv.size(); i++) {
        QString vcalendar;
        vcalendar.append("BEGIN:VEVENT\n");
        vcalendar.append("UID: "+QUuid::createUuidV5(QUuid(), QString(Eventv[i].Name+Eventv[i].Category+Eventv[i].Datet.toString(Qt::ISODate)+Eventv[i].Edatet.toString(Qt::ISODate)).toUtf8()).toString().mid(1, 36)+"\n");
        vcalendar.append("DTSTART:" + Eventv[i].Datet.toUTC().toString("yyyyMMddThhmmss")+"\n");
        vcalendar.append("DTEND:"+Eventv[i].Edatet.toUTC().toString("yyyyMMddThhmmss")+"\n");
        QString priority;
        switch (Eventv[i].Priority) {
        case 5: {
            priority="1";
        }; break;
        case 4: {
            priority="3";
        }; break;
        case 3: {
            priority="5";
        }; break;
        case 2: {
            priority="7";
        }; break;
        case 1: {
            priority="9";
        }; break;
        }
        vcalendar.append("PRIORITY:"+priority+"\n");
        vcalendar.append("SUMMARY:"+Eventv[i].Name+"\n");
        vcalendar.append("DESCRIPTION:"+Eventv[i].Category+"\n");
        switch (Eventv[i].OccuranceType) {
        case Event::NoOccurance: {
        }; break;
        case Event::Daily: {
            if (Eventv[i].Edate.year() != 2099) {
                vcalendar.append("RRULE:FREQ=DAILY;UNTIL="+QDateTime(Eventv[i].Edate).toUTC().toString("yyyyMMddThhmmss")+"\n");
            } else {
                vcalendar.append("RRULE:FREQ=DAILY\n");
            }
        }; break;
        case Event::Weekly: {
            if (Eventv[i].Edate.year() != 2099) {
                vcalendar.append("RRULE:FREQ=WEEKLY;UNTIL= "+QDateTime(Eventv[i].Edate).toUTC().toString("yyyyMMddThhmmss")+"\n");
            } else {
                vcalendar.append("RRULE:FREQ=WEEKLY\n");
            }
        }; break;
        case Event::Monthly: {
            if (Eventv[i].Edate.year() != 2099) {
                vcalendar.append("RRULE:FREQ=MONTHLY;UNTIL= "+QDateTime(Eventv[i].Edate).toUTC().toString("yyyyMMddThhmmss")+"\n");
            } else {
                vcalendar.append("RRULE:FREQ=MONTHLY\n");
            }
        }; break;
        case Event::Yearly: {
            if (Eventv[i].Edate.year() != 2099) {
                vcalendar.append("RRULE:FREQ=YEARLY ;UNTIL="+QDateTime(Eventv[i].Edate).toUTC().toString("yyyyMMddThhmmss")+"\n");
            } else {
                vcalendar.append("RRULE:FREQ=YEARLY\n");
            }
        }; break;
        }
        QString valarm;
        valarm.append("BEGIN:VALARM\n");
        valarm.append("TRIGGER:-PT30M\n");
        valarm.append("ACTION:AUDIO\n");
        valarm.append("END:VALARM\n");

        vcalendar.append(valarm);
        vcalendar.append("END:VEVENT\n");
        Output.append(vcalendar);
    }
    Output.append("END:VCALENDAR");
    return Output;
}

void qorgCalendar::setCalendar() {
    Calendar->clearContents();
    DayView->clear();

    QDateTime Tmp = QDateTime(currentDate, QTime(0, 0));
    QTreeWidgetItem* Hours[24];
    for (int i = 0; i < 24; i++) {
        Hours[i] = new QTreeWidgetItem(DayView);
        Hours[i]->setText(0, Tmp.toString("HH"));
        Hours[i]->setIcon(0, QIcon(":/cal/Clock"));

        QList <uint> IIDs;
        QList <Event> Events;
        if (category.isEmpty()
                || category == "Birthdays") {
            Events.append(checkBd(currentDate));
        }
        uint BDSize = Events.size();
        for (char j = 5; j > 0; j--) {
            for (uint k = 0; k < Eventv.size(); k++) {
                if ((Eventv[k].Category == category
                     || category.isEmpty())
                        && Eventv[k].Priority == j
                        && Eventv[k].occursOnDate(currentDate)) {
                    Events.append(Eventv[k]);
                    IIDs.append(k);
                }
            }
        }
        for (int j = 0; j < Events.size(); j++) {
            if (Events[j].starts(Tmp) <= Tmp
                    && Events[j].ends(Tmp) >= Tmp) {
                QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                Itm->setToolTip(0, Events[j].Name);
                Itm->setText(0, Events[j].Name);
                Itm->setText(1, "");
                Itm->setText(2, "");
                Itm->setText(3, "");
                Itm->setText(4, "");
                if (Tmp.date() == Events[j].starts(Tmp).date()
                        && Tmp.time().hour() ==  Events[j].starts(Tmp).time().hour()) {
                    Itm->setText(1, Events[j].starts(Tmp).time().toString("HH:mm"));
                }
                if (Tmp.date() == Events[j].ends(Tmp).date()
                        && Tmp.time().hour() == Events[j].ends(Tmp).time().hour()) {
                    Itm->setText(2, Events[j].ends(Tmp).time().toString("HH:mm"));
                }
                if (Events[j].Priority != 0) {
                    QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, IIDs[j-BDSize]);
                    DayView->setItemWidget(Itm, 3, Edit);
                    connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                    QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, IIDs[j-BDSize]);
                    DayView->setItemWidget(Itm, 4, Delete);
                    connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                }
                colorItem(Itm, Events[j].Priority);
            }
        }
        Tmp = Tmp.addSecs(60*60);
    }

    DayView->expandAll();
    QDate temp = QDate(currentDate.year(), currentDate.month(), 1);
    QDate temp1 = QDate(currentDate.year(), currentDate.month(), currentDate.daysInMonth());

    QList  <QTableWidgetItem*>  Items;

    int ItmSel = 0;

    // Days before month
    for (int i = temp.dayOfWeek(); i > 1; i--) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        Itm->setText(QString::number(temp.addDays(1-i).day()));
        Itm->setTextAlignment(Qt::AlignCenter);
        Itm->setTextColor(Qt::gray);
        Itm->setToolTip(temp.addDays(1-i).toString("dd.MM.yyyy"));
        Items.append(Itm);
    }
    // Month days
    for (int i = 0; i < temp.daysInMonth(); i++) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        if (temp.addDays(i) == currentDate) {
            ItmSel = Items.size();
        }
        Itm->setToolTip(temp.addDays(i).toString("dd.MM.yyyy"));
        Items.append(Itm);
        if (temp.addDays(i) == QDate::currentDate()) {
            Itm->setBackground(QColor("#EE7777"));
        }
    }
    // Days after month
    for (int i = 1; Items.size() < 42; i++) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        Itm->setText(QString::number(temp1.addDays(i).day()));
        Itm->setTextAlignment(Qt::AlignCenter);
        Itm->setToolTip(temp1.addDays(i).toString("dd.MM.yyyy"));
        Itm->setTextColor(Qt::gray);
        Items.append(Itm);
    }

    // Setting flags and cakes ... and calendar
    for (int i = 0; i < 42; i++) {
        Calendar->setItem((i-(i%7))/7, i%7, Items[i]);
        QDate Date = QDate::fromString(Items[i]->toolTip(), "dd.MM.yyyy");
        if (Date.month() == currentDate.month()) {
            bool Ev = false;
            bool Bd = false;
            if (category.isEmpty()
                    || category == "Birthdays") {
                Bd = (checkBd(Date).size() != 0);
            }
            for (uint j = 0; j < Eventv.size(); j++) {
                if ((Eventv[j].Category == category
                     || category.isEmpty())
                        && Eventv[j].occursOnDate(Date)) {
                    Ev = true;
                    break;
                }
            }
            QTableCalendarWidget* Day = new QTableCalendarWidget(Items[i]->toolTip(), Ev, Bd, this);
            Calendar->setCellWidget(Items[i]->row(), Items[i]->column(), Day);
            if (i == ItmSel) {
                Day->AddB();
                connect(Day, SIGNAL(addOUT(QDate)), this, SLOT(Add(QDate)));
            }
        }
    }

    // Setting week labels
    QStringList labels;
    for (int i = 0; i < 6; i++) {
        int w=(temp.addDays(7*i).dayOfYear()-temp.addDays(7*i).dayOfWeek()+10)/7;
        if (w == 53) {
            int p=(QDate(temp.year()+1, 1, 1).dayOfYear()-QDate(temp.year()+1, 1, 1).dayOfWeek()+10)/7;
            if (p != 0) {
                w = 1;
            }
        }
        if (w == 0) {
            w=(QDate(temp.year()-1, 12, 31).dayOfYear()-QDate(temp.year()-1, 12, 31).dayOfWeek()+10)/7;;
        }
        labels.append(QString::number(w));
    }
    Calendar->setVerticalHeaderLabels(labels);
}
QList <Event> qorgCalendar::checkBd(QDate D) {
    QList <Event> BdEvent;
    QList <QString> BName = AB->getBirthdays(D);
    for (int i = 0; i < BName.size(); i++) {
        Event tmp;
        tmp.Name = BName[i];
        tmp.Datet = QDateTime(D, QTime(8, 0));
        tmp.Edatet = QDateTime(D, QTime(12, 0));
        tmp.Priority = 0;
        BdEvent.append(tmp);
    }
    return BdEvent;
}
void qorgCalendar::updateAll() {
    Incoming->clear();
    QList <uint> DuplicateLock;
    QList <Event> Events;
    for (uint i = 0; i < 30 && Events.size() < 25; i++) {
        if (category.isEmpty()
                || category == "Birthdays") {
            Events.append(checkBd(QDate::currentDate().addDays(i)));
        }
        uint BDSize = Events.size();
        QList <uint> IIDs;
        for (char j = 5; j > 0; j--) {
            for (uint k = 0; k < Eventv.size(); k++) {
                if ((Eventv[k].Category == category
                     || category.isEmpty())
                        && Eventv[k].Priority == j
                        && Eventv[k].occursOnDate(QDate::currentDate().addDays(i))
                        && !DuplicateLock.contains(k)) {
                    Events.append(Eventv[k]);
                    DuplicateLock.append(k);
                    IIDs.append(k);
                }
            }
        }
        for (int j = 0; j < Events.size(); j++) {
            QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
            Itm->setText(0, Events[j].Name);
            Itm->setToolTip(0, Events[j].Name);
            Itm->setText(1, Events[j].starts(QDateTime(QDate::currentDate().addDays(i), QTime(00, 00))).toString("HH:mm dd.MM.yyyy"));
            Itm->setText(2, Events[j].ends(QDateTime(QDate::currentDate().addDays(i), QTime(00, 00))).toString("HH:mm dd.MM.yyyy"));
            Itm->setText(3, Events[j].Category);
            Itm->setText(4, "");
            Itm->setText(5, "");
            if (Events[j].Priority != 0) {
                QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, IIDs[j-BDSize]);
                Incoming->setItemWidget(Itm, 4, Edit);
                connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, IIDs[j-BDSize]);
                Incoming->setItemWidget(Itm, 5, Delete);
                connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
            }
            colorItem(Itm, Events[j].Priority);
        }
        Events.clear();
    }
    setCalendar();
}
void qorgCalendar::sort() {
    if (Eventv.size() > 1) {
        while (true) {
            bool Sorted = true;
            for (uint i = 0; i < Eventv.size()-1; i++) {
                if (Eventv[i].Datet > Eventv[i+1].Datet) {
                    std::swap(Eventv[i], Eventv[i+1]);
                    Sorted = false;
                } else if (Eventv[i].Datet == Eventv[i+1].Datet) {
                    if (Eventv[i].Priority < Eventv[i+1].Priority) {
                        std::swap(Eventv[i], Eventv[i+1]);
                        Sorted = false;
                    }
                }
            }
            if (Sorted) {
                break;
            }
        }
    }
}
void qorgCalendar::Add(QDate Input) {
    if ((new QCalDialog(Input, this))->exec() == QDialog::Accepted) {
        sort();
        updateAll();
        setNotification();
        emit updateTree();
    }
}
void qorgCalendar::Edit(uint IID) {
    if ((new QCalDialog(IID, this))->exec() == QDialog::Accepted) {
        sort();
        updateAll();
        setNotification();
        emit updateTree();
    }
}
void qorgCalendar::Delete(uint IID) {
    Eventv.erase(Eventv.begin()+IID);
    sort();
    updateAll();
    setNotification();
    emit updateTree();
}
void qorgCalendar::dayChanged(QTableWidgetItem* Input) {
    QDate picked = QDate::fromString(Input->toolTip(), "dd.MM.yyyy");
    if (picked.year() >= QDate::currentDate().year()-20
            && picked.year() <= QDate::currentDate().year()+20) {
        Month->blockSignals(true);
        Month->setCurrentIndex(picked.month()-1);
        Month->blockSignals(false);
        Year->blockSignals(true);
        Year->setCurrentIndex(Year->currentIndex()-(currentDate.year()-picked.year()));
        Year->blockSignals(false);
        currentDate = picked;
        setCalendar();
    }
}
void qorgCalendar::doubleClick(QModelIndex I) {
    QDate D = QDateTime::fromString(Incoming->topLevelItem(I.row())->text(1), "HH:mm dd.MM.yyyy").date();
    Month->blockSignals(true);
    Month->setCurrentIndex(D.month()-1);
    Month->blockSignals(false);
    Year->blockSignals(true);
    Year->setCurrentIndex(Year->currentIndex()-(currentDate.year()-D.year()));
    Year->blockSignals(false);
    currentDate = D;
    setCalendar();
}

void qorgCalendar::monthChanged(int Input) {
    currentDate.setDate(currentDate.year(), Input+1, currentDate.day());
    setCalendar();
}
void qorgCalendar::yearChanged(QString Input) {
    if (Input.isEmpty()) {
        Year->blockSignals(true);
        QPushButton* Ypom = qobject_cast<QPushButton*>(QObject::sender());
        if (Ypom == Yminus) {
            if (currentDate.year()-1 >= QDate::currentDate().year()-20) {
                currentDate = currentDate.addYears(-1);
                Year->setCurrentIndex(Year->currentIndex()-1);
            }
        } else {
            if (currentDate.year()+1 < QDate::currentDate().year()+21) {
                currentDate = currentDate.addYears(1);
                Year->setCurrentIndex(Year->currentIndex()+1);
            }
        }
        Year->blockSignals(false);
    } else {
        int difference = Input.toInt() - currentDate.year();
        currentDate = currentDate.addYears(difference);
    }
    setCalendar();
}
void qorgCalendar::MidnightChange() {
    currentDate = QDate::currentDate();
    updateAll();
    setNotification();
    Midnight->stop();
    Midnight->setInterval(QDateTime::currentDateTime().msecsTo(QDateTime(QDate::currentDate().addDays(1), QTime(00, 00))));
    Midnight->start();
}
void qorgCalendar::setNotification(bool first) {
    QList <Event> LT15Min;
    QList <Event> MT15LT30Min;
    int closest = 24*60*60;
    for (uint i = 0; i < Eventv.size(); i++) {
        if (Eventv[i].occursOnDate(QDate::currentDate())
                && QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) >= 0) {
            if (first
                    && QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) < 15*60) {
                LT15Min.append(Eventv[i]);
            }
            if (QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) >= 15*60
                    && QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) <= 30*60) {
                MT15LT30Min.append(Eventv[i]);
            }
            if (QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) > 30*60) {
                if (QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime())) < closest) {
                    closest = QDateTime::currentDateTime().secsTo(Eventv[i].starts(QDateTime::currentDateTime()));
                }
            }
        }
    }
    QString Message;
    if (first
            && LT15Min.size() > 0) {
        Message.append("Immediate notification for:");
        for (int i = 0; i < LT15Min.size(); i++) {
            Message.append("\n" + LT15Min[i].Name);
        }
    }
    if (MT15LT30Min.size() > 0) {
        if (first && LT15Min.size() > 0) {
            Message.append("\n");
        }
        Message.append("At least 15 minuts notification for:");
        for (int i = 0; i < MT15LT30Min.size(); i++) {
            Message.append("\n" + MT15LT30Min[i].Name);
        }
    }
    if (!Message.isEmpty()) {
        emit Notification("Event notification", Message);
    }
    NTimer->setInterval((closest-15*60)*1000);
    NTimer->setSingleShot(true);
    NTimer->start();
}
void qorgCalendar::checkMidnight() {
    int difference = Midnight->remainingTime()-QDateTime::currentDateTime().msecsTo(QDateTime(QDate::currentDate().addDays(1), QTime(00, 00)));
    if (abs(difference) > 500) {
        if (abs(difference) > 2000) {
            emit TimeChangeBlock();
        }
        Midnight->stop();
        Midnight->setInterval(QDateTime::currentDateTime().msecsTo(QDateTime(QDate::currentDate().addDays(1), QTime(00, 00))));
        Midnight->start();
        setNotification(true);
        updateAll();
    }
}
#include "qorgcalendar.moc"
