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

class QTableCalendarWidget: public QWidget {
    Q_OBJECT
public:
    QTableCalendarWidget(QString ToolTip, bool Ev, bool BDay, QWidget* parent) :QWidget(parent) {
        Date = QDate::fromString(ToolTip, "d/MM/yyyy");
        Event = new QLabel(this);
        Birth = new QLabel(this);
        Label = new QLabel(QString::number(Date.day()), this);
        Label->setAlignment(Qt::AlignHCenter);
        QPixmap w = QPixmap(12, 12);
        w.fill(Qt::transparent);
        if (Ev) {
            Event->setPixmap(QIcon(":/cal/Event.png").pixmap(12));
        } else {
            Event->setPixmap(w);
        }
        if (BDay) {
            Birth->setPixmap(QIcon(":/cal/Birthday.png").pixmap(12));
        } else {
            Birth->setPixmap(w);
        }
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
class QCalDialogAdd: public QDialog {
    Q_OBJECT
public:
    QCalDialogAdd(QDate Date, qorgCalendar* Cal) :QDialog(Cal) {
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
        Starts1 = new QDateEdit(Date, this);
        Starts2 = new QTimeEdit(QTime(12, 00), this);
        connect(Starts2, SIGNAL(timeChanged(QTime)), this, SLOT(time(QTime)));
        Finish1 = new QDateEdit(Date, this);
        Finish2 = new QTimeEdit(QTime(13, 00), this);
        OccuraneEndDate = new QDateEdit(Date, this);
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
        Lay->addWidget(Name, 0, 1, 1, 2);
        Lay->addWidget(Category, 1, 1, 1, 2);
        Lay->addWidget(Priority, 2, 1, 1, 2);
        Lay->addWidget(Occurance, 3, 1, 1, 2);
        Lay->addWidget(Starts1, 4, 1);
        Lay->addWidget(Starts2, 4, 2);
        Lay->addWidget(Finish1, 5, 1);
        Lay->addWidget(Finish2, 5, 2);
        Lay->addWidget(OccuraneEndDate, 6, 1, 1, 2);
        Lay->addWidget(ActionButtons[1], 7, 1, 1, 2);
        Lay->addWidget(ActionButtons[0], 7, 0);
        if (Calendar->category == "Birthdays") {
            Category->setDisabled(true);
            Finish1->setDisabled(true);
            Finish2->setDisabled(true);
            Occurance->setCurrentIndex(4);
            Occurance->setDisabled(true);
        }
        setWindowTitle("Add event");
    }
private:
    QLabel* Labels[7];
    QLineEdit* Name;
    QLineEdit* Category;
    QSlider* Priority;
    QComboBox* Occurance;
    QDateEdit* Starts1;
    QTimeEdit* Starts2;
    QDateEdit* Finish1;
    QTimeEdit* Finish2;
    QDateEdit* OccuraneEndDate;
    QPushButton* ActionButtons[2];
    qorgCalendar* Calendar;
private slots:
    void lock(int i) {
        if (i == 0) {
            Labels[6]->setDisabled(true);
            OccuraneEndDate->setDisabled(true);
            Finish1->setDisabled(false);
        } else {
            Labels[6]->setDisabled(false);
            OccuraneEndDate->setDisabled(false);
            if (i < 3) {
                OccuraneEndDate->setDate(Starts1->date().addDays(1));
            } else {
                OccuraneEndDate->setDate(QDate(2099, Starts1->date().month(), Starts1->date().day()));
            }
            Finish1->setDisabled(true);
        }
    }
    void add() {
        if (Occurance->currentIndex() == 0) {
            CalNor str;
            if (Name->text().isEmpty()) {
                if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                    Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                    connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                }
                return;
            }
            str.name = Name->text();
            str.category = Category->text();
            str.priority = Priority->value();
            str.datet = QDateTime(Starts1->date(), Starts2->time());
            str.edatet = QDateTime(Finish1->date(), Finish2->time());
            if (str.datet >= str.edatet) {
                if (Finish1->styleSheet() != "QDateEdit{background: #FF8888;}") {
                    Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Finish1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    Finish2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                }
                return;
            }
            Calendar->Normal.push_back(str);
        } else {
            CalRec str;
            if (Name->text().isEmpty()||QDateTime(Starts1->date(), Starts2->time()) >= QDateTime(Starts1->date(), Finish2->time())||OccuraneEndDate->date() < Starts1->date()) {
                if (Name->text().isEmpty()) {
                    if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                        Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                        connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                    }
                }
                if (QDateTime(Starts1->date(), Starts2->time()) >= QDateTime(Starts1->date(), Finish2->time())) {
                    Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    Finish2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                } else {
                    Starts1->setStyleSheet("QDateEdit{background: white;}");
                    Starts2->setStyleSheet("QTimeEdit{background: white;}");
                    Finish2->setStyleSheet("QTimeEdit{background: white;}");
                }
                if (OccuraneEndDate->date() < Starts1->date()) {
                    Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    OccuraneEndDate->setStyleSheet("QDateEdit{background: #FF8888;}");
                } else {
                    OccuraneEndDate->setStyleSheet("QDateEdit{background: white;}");
                }
                return;
            }
            str.name = Name->text();
            str.category = Category->text();
            str.priority = Priority->value();
            str.type = Occurance->currentIndex();
            str.datet = QDateTime(Starts1->date(), Starts2->time());
            str.edatet = QDateTime(Finish1->date(), Finish2->time());
            str.edate = OccuraneEndDate->date();
            Calendar->Recurrent.push_back(str);
        }
        this->done(1);
    }
    void changed(QString Input) {
        if (!Input.isEmpty()) {
            Name->setStyleSheet("QLineEdit{background: white;}");
        }
    }
    void cancelled() {
        this->done(0);
    }
    void time(QTime T) {
        if (T.hour() == 23) {
            if (Occurance->currentIndex() == 0) {
                Finish1->setDate(Starts1->date().addDays(1));
                Finish2->setTime(T.addSecs(3600));
            } else {
                Finish2->setTime(QTime(23, 59));
            }
        } else {
            Finish2->setTime(T.addSecs(3600));
        }
    }
};
class QCalDialogEdit: public QDialog {
    Q_OBJECT
public:
    QCalDialogEdit(uint IID, qorgCalendar* Cal) :QDialog(Cal) {
        Calendar = Cal;
        ItemID = IID;
        Labels[0]=new QLabel("Name: ", this);
        Labels[1]=new QLabel("Category: ", this);
        Labels[2]=new QLabel("Priority: ", this);
        Labels[3]=new QLabel("Occurance: ", this);
        Labels[4]=new QLabel("Start: ", this);
        Labels[5]=new QLabel("Finish: ", this);
        Labels[6]=new QLabel("Date of occurane ending: ", this);
        Labels[6]->setDisabled(true);
        Name = new QLineEdit(this);
        Category = new QLineEdit(this);
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
        Occurance = new QComboBox(this);
        Occurance->addItems(QStringList() << "No occurance" << "Daily" << "Weekly" << "Monthy" << "Yearly");
        connect(Occurance, SIGNAL(currentIndexChanged(int)), this, SLOT(lock(int)));
        Starts1 = new QDateEdit(this);
        Starts2 = new QTimeEdit(this);
        Finish1 = new QDateEdit(this);
        Finish2 = new QTimeEdit(this);
        OccuraneEndDate = new QDateEdit(this);
        OccuraneEndDate->setDisabled(true);
        ActionButtons[0]=new QPushButton(this);
        ActionButtons[0]->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(ActionButtons[0], SIGNAL(clicked()), this, SLOT(cancelled()));
        ActionButtons[1]=new QPushButton(this);
        ActionButtons[1]->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        ActionButtons[1]->setDefault(true);
        connect(ActionButtons[1], SIGNAL(clicked()), this, SLOT(edit()));
        if (!(IID&0xF0000000)) {
            Name->setText(Calendar->Normal[ItemID].name);
            Category->setText(Calendar->Normal[ItemID].category);
            Priority->setSliderPosition(Calendar->Normal[ItemID].priority);
            Starts1->setDate(Calendar->Normal[ItemID].datet.date());
            Starts2->setTime(Calendar->Normal[ItemID].datet.time());
            Finish1->setDate(Calendar->Normal[ItemID].edatet.date());
            Finish2->setTime(Calendar->Normal[ItemID].edatet.time());
            OccuraneEndDate->setDate(QDate::currentDate());
        } else {
            Name->setText(Calendar->Recurrent[ItemID%0xF0000000].name);
            Category->setText(Calendar->Recurrent[ItemID%0xF0000000].category);
            Priority->setSliderPosition(Calendar->Recurrent[ItemID%0xF0000000].priority);
            Occurance->setCurrentIndex(Calendar->Recurrent[ItemID%0xF0000000].type);
            Starts1->setDate(Calendar->Recurrent[ItemID%0xF0000000].datet.date());
            Starts2->setTime(Calendar->Recurrent[ItemID%0xF0000000].datet.time());
            Finish1->setDate(Calendar->Recurrent[ItemID%0xF0000000].edatet.date());
            Finish2->setTime(Calendar->Recurrent[ItemID%0xF0000000].edatet.time());
            OccuraneEndDate->setDate(Calendar->Recurrent[ItemID%0xF0000000].edate);
        }
        connect(Starts2, SIGNAL(timeChanged(QTime)), this, SLOT(time(QTime)));
        QGridLayout* Lay = new QGridLayout(this);
        for (int i = 0; i < 7; i++) {
            Lay->addWidget(Labels[i], i, 0);
        }
        Lay->addWidget(Name, 0, 1, 1, 2);
        Lay->addWidget(Category, 1, 1, 1, 2);
        Lay->addWidget(Priority, 2, 1, 1, 2);
        Lay->addWidget(Occurance, 3, 1, 1, 2);
        Lay->addWidget(Starts1, 4, 1);
        Lay->addWidget(Starts2, 4, 2);
        Lay->addWidget(Finish1, 5, 1);
        Lay->addWidget(Finish2, 5, 2);
        Lay->addWidget(OccuraneEndDate, 6, 1, 1, 2);
        Lay->addWidget(ActionButtons[1], 7, 1, 1, 2);
        Lay->addWidget(ActionButtons[0], 7, 0);
        if (Calendar->category == "Birthdays"||((!ItemID&0xF0000000)&&Calendar->Normal[ItemID].category == "Birthdays")||(ItemID&0xF0000000&&Calendar->Recurrent[ItemID%0xF0000000].category == "Birthdays")) {
            Category->setDisabled(true);
            Finish1->setDisabled(true);
            Finish2->setDisabled(true);
            Occurance->setCurrentIndex(4);
            Occurance->setDisabled(true);
        }
        setWindowTitle("Edit event");
    }
private:
    QLabel* Labels[7];
    QLineEdit* Name;
    QLineEdit* Category;
    QSlider* Priority;
    QComboBox* Occurance;
    QDateEdit* Starts1;
    QTimeEdit* Starts2;
    QDateEdit* Finish1;
    QTimeEdit* Finish2;
    QDateEdit* OccuraneEndDate;
    QPushButton* ActionButtons[2];
    qorgCalendar* Calendar;
    uint ItemID;
private slots:
    void lock(int i) {
        if (i == 0) {
            Labels[6]->setDisabled(true);
            OccuraneEndDate->setDisabled(true);
            Finish1->setDisabled(false);
        } else {
            Labels[6]->setDisabled(false);
            OccuraneEndDate->setDisabled(false);
            if (i < 3) {
                OccuraneEndDate->setDate(Starts1->date().addDays(1));
            } else {
                OccuraneEndDate->setDate(QDate(2099, Starts1->date().month(), Starts1->date().day()));
            }
            Finish1->setDisabled(true);
        }
    }
    void edit() {
        if (Occurance->currentIndex() == 0) {
            CalNor str;
            if (Name->text().isEmpty()) {
                if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                    Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                    connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                }
            } else {
                str.name = Name->text();
                str.category = Category->text();
                str.priority = Priority->value();
                str.datet = QDateTime(Starts1->date(), Starts2->time());
                str.edatet = QDateTime(Finish1->date(), Finish2->time());
                if (str.datet >= str.edatet) {
                    if (Finish1->styleSheet() != "QDateEdit{background: #FF8888;}") {
                        Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                        Finish1->setStyleSheet("QDateEdit{background: #FF8888;}");
                        Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                        Finish2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                        connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                    }
                } else {
                    Calendar->Normal.push_back(str);
                }
            }
        } else {
            CalRec str;
            if (Name->text().isEmpty()||QDateTime(Starts1->date(), Starts2->time()) >= QDateTime(Starts1->date(), Finish2->time())||OccuraneEndDate->date() < Starts1->date()) {
                if (Name->text().isEmpty()) {
                    if (Name->styleSheet() != "QLineEdit{background: #FF8888;}") {
                        connect(Name, SIGNAL(textChanged(QString)), this, SLOT(changed(QString)));
                    }
                    Name->setStyleSheet("QLineEdit{background: #FF8888;}");
                }
                if (QDateTime(Starts1->date(), Starts2->time()) >= QDateTime(Starts1->date(), Finish2->time())) {
                    Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    Finish2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                } else {
                    Starts1->setStyleSheet("QDateEdit{background: white;}");
                    Starts2->setStyleSheet("QTimeEdit{background: white;}");
                    Finish2->setStyleSheet("QTimeEdit{background: white;}");
                }
                if (OccuraneEndDate->date() < Starts1->date()) {
                    Starts1->setStyleSheet("QDateEdit{background: #FF8888;}");
                    Starts2->setStyleSheet("QTimeEdit{background: #FF8888;}");
                    OccuraneEndDate->setStyleSheet("QDateEdit{background: #FF8888;}");
                } else {
                    OccuraneEndDate->setStyleSheet("QDateEdit{background: white;}");
                }
                return;
            }
            str.name = Name->text();
            str.category = Category->text();
            str.priority = Priority->value();
            str.type = Occurance->currentIndex();
            str.datet = QDateTime(Starts1->date(), Starts2->time());
            str.edatet = QDateTime(Finish1->date(), Finish2->time());
            str.edate = OccuraneEndDate->date();
            Calendar->Recurrent.push_back(str);
        }
        if (!(ItemID&0xF0000000)) {
            Calendar->Normal.erase(Calendar->Normal.begin()+ItemID);
        } else {
            Calendar->Recurrent.erase(Calendar->Recurrent.begin()+ItemID%0xF0000000);
        }
        this->accept();
    }
    void changed(QString Input) {
        if (!Input.isEmpty()) {
            Name->setStyleSheet("QLineEdit{background: white;}");
        } else {
            Name->setStyleSheet("QLineEdit{background: #FF8888;}");
        }
    }
    void cancelled() {
        this->reject();
    }
    void time(QTime T) {
        if (T.hour() == 23) {
            if (Occurance->currentIndex() == 0) {
                Finish1->setDate(Starts1->date().addDays(1));
                Finish2->setTime(T.addSecs(3600));
            } else {
                Finish2->setTime(QTime(23, 59));
            }
        } else {
            Finish2->setTime(T.addSecs(3600));
        }
    }
};

qorgCalendar::qorgCalendar(QWidget* parent, qorgAB* AB) :QWidget(parent) {
    this->AB = AB;
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
    connect(Calendar, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(dayChanged(QTableWidgetItem*)));

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
    setCalendar();
    NTimer = new QTimer(this);
    connect(NTimer, SIGNAL(timeout()), this, SLOT(setNotification()));
    Midnight = new QTimer(this);
    MidnightChange();
    MidnightTester = new QTimer(this);
    MidnightTester->setInterval(60000);
    MidnightTester->start();
    connect(MidnightTester, SIGNAL(timeout()), this, SLOT(checkMidnight()));
}
QString qorgCalendar::output() {
    QString out;
    for (uint i = 0; i < Normal.size(); i++) {
        out.append(Output(Normal[i].name)+" ");
        out.append(Output(Normal[i].category)+" ");
        out.append(Output(Normal[i].priority)+" ");
        out.append(Output(Normal[i].datet)+" ");
        out.append(Output(Normal[i].edatet)+" \n");
    }
    for (uint i = 0; i < Recurrent.size(); i++) {
        out.append(Output(Recurrent[i].name)+" ");
        out.append(Output(Recurrent[i].category)+" ");
        out.append(Output(Recurrent[i].priority)+" ");
        out.append(Output(Recurrent[i].type)+" ");
        out.append(Output(Recurrent[i].datet)+" ");
        out.append(Output(Recurrent[i].edatet)+" ");
        out.append(Output(Recurrent[i].edate)+" \n");
    }
    out.append("\n\n");
    return out;
}
void qorgCalendar::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 5: {
                CalNor Nor;
                Nor.name = InputS(B[0]);
                Nor.category = InputS(B[1]);
                Nor.priority = InputC(B[2]);
                Nor.datet = InputDT(B[3]);
                Nor.edatet = InputDT(B[4]);
                Normal.push_back(Nor);
            } break;
            case 7: {
                CalRec Rec;
                Rec.name = InputS(B[0]);
                Rec.category = InputS(B[1]);
                Rec.priority = InputC(B[2]);
                Rec.type = InputC(B[3]);
                Rec.datet = InputDT(B[4]);
                Rec.edatet = InputDT(B[5]);
                Rec.edate = InputD(B[6]);
                Recurrent.push_back(Rec);
            } break;
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
    for (uint i = 0; i < Normal.size(); i++) {
        if (Normal[i].category != ""&&!(list.contains(Normal[i].category))) {
            list.append(Normal[i].category);
        }
    }
    for (uint i = 0; i < Recurrent.size(); i++) {
        if (Recurrent[i].category != ""&&!(list.contains(Recurrent[i].category))) {
            list.append(Recurrent[i].category);
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
    if (Normal.size()+Recurrent.size()+checkBd(QDate::currentDate()).size() != 0) {
        uint hour = 0;
        uint today = 0;
        uint tomorrow = 0;
        for (char i = 5; i > 0; i--) {
            QList <uint> N = checkEvN(QDate::currentDate(), i);
            QList <uint> R = checkEvR(QDate::currentDate(), i);
            for (int j = 0; j < N.size(); j++) {
                if (QDateTime::currentDateTime().secsTo(Normal[N[j]].datet) < 3600&&QDateTime::currentDateTime().secsTo(Normal[N[j]].datet) >= 0) {
                    hour++;
                }
            }
            for (int j = 0; j < R.size(); j++) {
                if (QDateTime::currentDateTime().secsTo(QDateTime(QDate::currentDate(), Recurrent[R[j]].datet.time())) < 3600&&
                        QDateTime::currentDateTime().secsTo(QDateTime(QDate::currentDate(), Recurrent[R[j]].datet.time())) >= 0) {
                    hour++;
                }
            }
            today+=N.size();
            today+=R.size();
            QList <uint> Nt = checkEvN(QDate::currentDate().addDays(1), i);
            for (int j = 0; j < Nt.size(); j++) {
                if (N.contains(Nt[j])) {
                    Nt.removeAt(j);
                    j--;
                }
            }
            QList <uint> Rt = checkEvR(QDate::currentDate().addDays(1), i);
            for (int j = 0; j < Rt.size(); j++) {
                if (R.contains(Rt[j])) {
                    Rt.removeAt(j);
                    j--;
                }
            }
            tomorrow+=Nt.size();
            tomorrow+=Rt.size();
        }
        QString M1;
        QString M2;
        QString M3;
        QString M4;
        if (hour == 1) {
            M1="1 event during hour.\n";
        } else {
            M1 = QString::number(hour)+" events during hour.\n";
        }
        if (today == 1) {
            M2="1 event today.\n";
        } else {
            M2 = QString::number(today)+" events today.\n";
        }
        if (tomorrow == 1) {
            M3="1 event tommorow.\n";
        } else {
            M3 = QString::number(tomorrow)+" events tommorow.\n";
        }
        if (checkBd(QDate::currentDate()).size() == 1) {
            M4="1 person has birthday today.";
        } else {
            M4 = QString::number(checkBd(QDate::currentDate()).size())+" people have birthday today.";
        }
        return "Calendar: "+M1+QString(16, ' ')+M2+QString(16, ' ')+M3+QString(16, ' ')+M4;
    } else {
        return "Calendar: No event.";
    }
}
void qorgCalendar::setCalendar() {
    Calendar->clearContents();
    DayView->clear();
    QDateTime Tmp = QDateTime(currentDate, QTime(0, 0));
    QTreeWidgetItem* Hours[24];
    for (int i = 0; i < 24; i++) {
        Hours[i]=new QTreeWidgetItem(DayView);
        Hours[i]->setText(0, Tmp.toString("HH"));
        Hours[i]->setIcon(0, QIcon(":/cal/Clock"));
        if (category.isEmpty()||category == "Birthdays") {
            QList  <QString>  B = checkBd(currentDate);
            if (i == 8) {
                for (int j = 0; j < B.size(); j++) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                    Itm->setToolTip(0, B[j]);
                    Itm->setText(0, B[j]);
                    Itm->setText(1, "08:00");
                    Itm->setText(2, "");
                    Itm->setText(3, "");
                    Itm->setText(4, "");
                    colorItem(Itm, 0);
                }
            } else if (i > 8&&i < 12) {
                for (int j = 0; j < B.size(); j++) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                    Itm->setToolTip(0, B[j]);
                    Itm->setText(0, B[j]);
                    Itm->setText(1, "");
                    Itm->setText(2, "");
                    Itm->setText(3, "");
                    Itm->setText(4, "");
                    colorItem(Itm, 0);
                }
            } else if (i == 12) {
                for (int j = 0; j < B.size(); j++) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                    Itm->setToolTip(0, B[j]);
                    Itm->setText(0, B[j]);
                    Itm->setText(1, "");
                    Itm->setText(2, "12:00");
                    Itm->setText(3, "");
                    Itm->setText(4, "");
                    colorItem(Itm, 0);
                }
            }
        }
        for (char j = 5; j > 0; j--) {
            QList  <uint>  N = checkEvN(currentDate, j);
            QList  <uint>  R = checkEvR(currentDate, j);
            for (int k = 0; k < N.size(); k++) {
                if (Tmp >= QDateTime(Normal[N[k]].datet.date(), QTime(Normal[N[k]].datet.time().hour(), 0))&&Tmp <= QDateTime(Normal[N[k]].edatet.date(), QTime(Normal[N[k]].edatet.time().hour(), 0))) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                    Itm->setToolTip(0, Normal[N[k]].name);
                    Itm->setText(0, Normal[N[k]].name);
                    Itm->setText(1, "");
                    Itm->setText(2, "");
                    Itm->setText(3, "");
                    Itm->setText(4, "");
                    if (Tmp.date() == Normal[N[k]].datet.date()&&Tmp.time().hour() == Normal[N[k]].datet.time().hour()) {
                        Itm->setText(1, Normal[N[k]].datet.time().toString("HH:mm"));
                    }
                    if (Tmp.date() == Normal[N[k]].edatet.date()&&Tmp.time().hour() == Normal[N[k]].edatet.time().hour()) {
                        Itm->setText(2, Normal[N[k]].edatet.time().toString("HH:mm"));
                    }
                    QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, N[k]);
                    DayView->setItemWidget(Itm, 3, Edit);
                    connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                    QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, N[k]);
                    DayView->setItemWidget(Itm, 4, Delete);
                    connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                    colorItem(Itm, j);
                }
            }
            for (int k = 0; k < R.size(); k++) {
                if (Tmp >= QDateTime(currentDate, QTime(Recurrent[R[k]].datet.time().hour(), 0))&&Tmp <= QDateTime(currentDate, QTime(Recurrent[R[k]].edatet.time().hour(), 0))) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Hours[i]);
                    Itm->setToolTip(0, Recurrent[R[k]].name);
                    Itm->setText(0, Recurrent[R[k]].name);
                    Itm->setText(1, "");
                    Itm->setText(2, "");
                    Itm->setText(3, "");
                    Itm->setText(4, "");
                    if (Tmp.time().hour() == QDateTime(currentDate, Recurrent[R[k]].datet.time()).time().hour()) {
                        Itm->setText(1, Recurrent[R[k]].datet.time().toString("HH:mm"));
                    }
                    if (Tmp.time().hour() == QDateTime(currentDate, Recurrent[R[k]].edatet.time()).time().hour()) {
                        Itm->setText(2, Recurrent[R[k]].edatet.time().toString("HH:mm"));
                    }
                    QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, R[k]|0xF0000000);
                    DayView->setItemWidget(Itm, 3, Edit);
                    connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                    QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, R[k]|0xF0000000);
                    DayView->setItemWidget(Itm, 4, Delete);
                    connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                    colorItem(Itm, j);
                }
            }
        }
        Tmp.setTime(QTime(Tmp.time().hour()+1, 0));
    }
    DayView->expandAll();
    QDate temp = QDate(currentDate.year(), currentDate.month(), 1);
    QDate temp1 = QDate(currentDate.year(), currentDate.month(), currentDate.daysInMonth());
    QList  <QTableWidgetItem*>  Items;
    int ItmSel = 0;
    for (int i = temp.dayOfWeek(); i > 1; i--) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        Itm->setText(QString::number(temp.addDays(1-i).day()));
        Itm->setTextAlignment(Qt::AlignCenter);
        Itm->setTextColor(Qt::gray);
        Itm->setToolTip(temp.addDays(1-i).toString("d/MM/yyyy"));
        Items.append(Itm);
    }
    for (int i = 0; i < temp.daysInMonth(); i++) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        if (temp.addDays(i) == currentDate) {
            ItmSel = Items.size();
        }
        Itm->setToolTip(temp.addDays(i).toString("d/MM/yyyy"));
        Items.append(Itm);
        if (temp.addDays(i) == QDate::currentDate()) {
            Itm->setBackground(QColor("#EE7777"));
        }
    }
    for (int i = 1; Items.size() < 42; i++) {
        QTableWidgetItem* Itm = new QTableWidgetItem();
        Itm->setFlags(Itm->flags() ^ Qt::ItemIsEditable);
        Itm->setText(QString::number(temp1.addDays(i).day()));
        Itm->setTextAlignment(Qt::AlignCenter);
        Itm->setToolTip(temp1.addDays(i).toString("d/MM/yyyy"));
        Itm->setTextColor(Qt::gray);
        Items.append(Itm);
    }
    for (int i = 0; i < 42; i++) {
        Calendar->setItem((i-(i%7))/7, i%7, Items[i]);
        QDate Date = QDate::fromString(Items[i]->toolTip(), "d/MM/yyyy");
        if (Date.month() == currentDate.month()) {
            bool Ev = false;
            bool Bd = false;
            if (category.isEmpty()||category == "Birthdays") {
                Bd=(checkBd(Date).size() != 0);
            }
            for (char j = 5; j > 0; j--) {
                if (checkEvN(Date, j).size() != 0) {
                    Ev = true;
                    break;
                } else if (checkEvR(Date, j).size() != 0) {
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
QList <uint> qorgCalendar::checkEvN(QDate Input, char P) {
    QList  <uint>  Output;
    for (uint i = 0; i < Normal.size(); i++) {
        if (Input >= Normal[i].datet.date()&&Input <= Normal[i].edatet.date()) {
            if ((category == Normal[i].category||category.isEmpty())&&Normal[i].priority == P) {
                Output.append(i);
            }
        }
    }
    return Output;
}
QList <uint> qorgCalendar::checkEvR(QDate Input, char P) {
    QList  <uint>  Output;
    for (uint i = 0; i < Recurrent.size(); i++) {
        if (Input >= Recurrent[i].datet.date()&&Input <= Recurrent[i].edate) {
            if ((category == Recurrent[i].category||category.isEmpty())&&Recurrent[i].priority == P) {
                switch (Recurrent[i].type) {
                case 1:
                {
                    Output.append(i);
                }break;
                case 2:
                {
                    if (Input.dayOfWeek() == Recurrent[i].datet.date().dayOfWeek()) {
                        Output.append(i);
                    }
                }break;
                case 3:
                {
                    if (Input.day() == Recurrent[i].datet.date().day()) {
                        Output.append(i);
                    }
                }break;
                case 4:
                {
                    if (Input.day() == Recurrent[i].datet.date().day()&&Input.month() == Recurrent[i].datet.date().month()) {
                        Output.append(i);
                    }
                }break;
                }
            }
        }
    }
    return Output;
}
QList  <QString>  qorgCalendar::checkBd(QDate D) {
    return AB->getBirthdays(D);
}
void qorgCalendar::updateAll() {
    Incoming->clear();
    QList  <uint>  DUID;
    for (uint i = 0, j = 0; i < 30&&j < 25; i++) {
        if (category.isEmpty()||category == "Birthdays") {
            QList  <QString>  B = checkBd(QDate::currentDate().addDays(i));
            for (int k = 0; k < B.size(); k++) {
                QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
                Itm->setText(0, B[k]);
                Itm->setToolTip(0, B[k]);
                Itm->setText(1, "08:00 "+QDate::currentDate().addDays(i).toString("dd/MM/yyyy"));
                Itm->setText(2, "12:00 "+QDate::currentDate().addDays(i).toString("dd/MM/yyyy"));
                Itm->setText(3, "Birthdays");
                Itm->setText(4, "");
                Itm->setText(5, "");
                colorItem(Itm, 0);
            }
            j+=B.size();
        }
        for (char k = 5; k > 0; k--) {
            QList  <uint>  N = checkEvN(QDate::currentDate().addDays(i), k);
            QList  <uint>  R = checkEvR(QDate::currentDate().addDays(i), k);
            j+=(N.size()+R.size());
            for (int l = 0; l < N.size(); l++) {
                if (!(DUID.contains(N[l]))) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
                    Itm->setText(0, Normal[N[l]].name);
                    Itm->setToolTip(0, Normal[N[l]].name);
                    Itm->setText(1, Normal[N[l]].datet.toString("HH:mm dd/MM/yyyy"));
                    Itm->setText(2, Normal[N[l]].edatet.toString("HH:mm dd/MM/yyyy"));
                    Itm->setText(3, Normal[N[l]].category);
                    Itm->setText(4, "");
                    Itm->setText(5, "");
                    QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, N[l]);
                    Incoming->setItemWidget(Itm, 4, Edit);
                    connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                    QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, N[l]);
                    Incoming->setItemWidget(Itm, 5, Delete);
                    connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                    colorItem(Itm, k);
                    DUID.append(N[l]);
                }
            }
            for (int l = 0; l < R.size(); l++) {
                if (!DUID.contains(R[l]|0xF0000000)) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
                    Itm->setText(0, Recurrent[R[l]].name);
                    Itm->setToolTip(0, Recurrent[R[l]].name);
                    QString DateS;
                    QString DateF;
                    if (Recurrent[R[l]].type == 1) {
                        DateS = Recurrent[R[l]].datet.toString("HH:mm dd/MM/yyyy");
                        DateF = QDateTime(Recurrent[R[l]].edate, Recurrent[R[l]].edatet.time()).toString("HH:mm dd/MM/yyyy");
                    } else {
                        DateS = QDateTime(QDate::currentDate().addDays(i), Recurrent[R[l]].datet.time()).toString("HH:mm dd/MM/yyyy");
                        DateF = QDateTime(QDate::currentDate().addDays(i), Recurrent[R[l]].edatet.time()).toString("HH:mm dd/MM/yyyy");
                    }
                    Itm->setText(1, DateS);
                    Itm->setText(2, DateF);
                    Itm->setText(3, Recurrent[R[l]].category);
                    Itm->setText(4, "");
                    Itm->setText(5, "");
                    QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, R[l]|0xF0000000);
                    Incoming->setItemWidget(Itm, 4, Edit);
                    connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                    QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, R[l]|0xF0000000);
                    Incoming->setItemWidget(Itm, 5, Delete);
                    connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                    DUID.append(R[l]|0xF0000000);
                    colorItem(Itm, k);
                }
            }
        }
        if (QDate::currentDate().addDays(i).day() == 28&&QDate::currentDate().addDays(i).month() == 2&&!(QDate::isLeapYear(QDate::currentDate().year()))) {
            if (category.isEmpty()||category == "Birthdays") {
                QList  <QString>  B = checkBd(QDate(2012, 2, 29));
                for (int k = 0; k < B.size(); k++) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
                    Itm->setText(0, B[k]);
                    Itm->setToolTip(0, B[k]);
                    Itm->setText(1, "08:00 29/02/"+QDate::currentDate().toString("yyyy"));
                    Itm->setText(2, "12:00 29/02/"+QDate::currentDate().toString("yyyy"));
                    Itm->setText(3, "Birthdays");
                    Itm->setText(4, "");
                    Itm->setText(5, "");
                    colorItem(Itm, 0);
                }
                j+=B.size();
            }
            for (char k = 5; k > 0; k--) {
                for (uint l = 0; l < Recurrent.size(); l++) {
                    if (QDate::currentDate().addDays(i) >= Recurrent[l].datet.date()&&QDate::currentDate().addDays(i+1) <= Recurrent[l].edate) {
                        if ((category == Recurrent[l].category||category.isEmpty())&&Recurrent[l].priority == k&&Recurrent[l].type == 4) {
                            if (29 == Recurrent[l].datet.date().day()&&2 == Recurrent[l].datet.date().month()) {
                                QTreeWidgetItem* Itm = new QTreeWidgetItem(Incoming);
                                Itm->setText(0, Recurrent[l].name);
                                Itm->setToolTip(0, Recurrent[l].name);
                                Itm->setText(1, Recurrent[l].datet.time().toString("HH:mm")+" 29/02/"+QDate::currentDate().toString("yyyy"));
                                Itm->setText(2, Recurrent[l].edatet.time().toString("HH:mm")+" 29/02/"+QDate::currentDate().toString("yyyy"));
                                Itm->setText(3, Recurrent[l].category);
                                Itm->setText(4, "");
                                Itm->setText(5, "");
                                QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, i|0xF0000000);
                                Incoming->setItemWidget(Itm, 4, Edit);
                                connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
                                QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, i|0xF0000000);
                                Incoming->setItemWidget(Itm, 5, Delete);
                                connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
                                colorItem(Itm, k);
                                j++;
                            }
                        }
                    }
                }
            }
        }
    }
    setCalendar();
}
void qorgCalendar::sort() {
    if (Normal.size() > 1) {
        while (1) {
            bool Sorted = true;
            for (uint i = 0; i < Normal.size()-1; i++) {
                if (Normal[i].priority < Normal[i+1].priority) {
                    std::swap(Normal[i], Normal[i+1]);
                    Sorted = false;
                } else if (Normal[i].priority == Normal[i+1].priority) {
                    if (Normal[i].datet > Normal[i+1].datet) {
                        std::swap(Normal[i], Normal[i+1]);
                        Sorted = false;
                    }
                }
            }
            if (Sorted) {
                break;
            }
        }
    }
    if (Recurrent.size() > 1) {
        while (1) {
            bool Sorted = true;
            for (uint i = 0; i < Recurrent.size()-1; i++) {
                if (Recurrent[i].priority < Recurrent[i+1].priority) {
                    std::swap(Recurrent[i], Recurrent[i+1]);
                    Sorted = false;
                } else if (Recurrent[i].priority == Recurrent[i+1].priority) {
                    if (Recurrent[i].datet.time() > Recurrent[i+1].datet.time()) {
                        std::swap(Recurrent[i], Recurrent[i+1]);
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
    if ((new QCalDialogAdd(Input, this))->exec() == 1) {
        sort();
        updateAll();
        setNotification();
        emit updateTree();
    }
}
void qorgCalendar::Edit(uint IID) {
    if ((new QCalDialogEdit(IID, this))->exec() == QDialog::Accepted) {
        sort();
        updateAll();
        setNotification();
        emit updateTree();
    }
}
void qorgCalendar::Delete(uint IID) {
    if (!(IID&0xF0000000)) {
        Normal.erase(Normal.begin()+IID);
        NotifiedN.removeOne(IID);
    } else {
        Recurrent.erase(Recurrent.begin()+(IID&0xF0000000));
        NotifiedR.removeOne((IID&0xF0000000));
    }
    sort();
    updateAll();
    setNotification();
    emit updateTree();
}
void qorgCalendar::dayChanged(QTableWidgetItem* Input) {
    QDate picked = QDate::fromString(Input->toolTip(), "d/MM/yyyy");
    if (picked.year() >= QDate::currentDate().year()-20&&picked.year() <= QDate::currentDate().year()+20) {
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
    if (Incoming->topLevelItem(I.row()) != NULL) {
        QDate D = QDateTime::fromString(Incoming->topLevelItem(I.row())->text(1), "HH:mm dd/MM/yyyy").date();
        Month->blockSignals(true);
        Month->setCurrentIndex(D.month()-1);
        Month->blockSignals(false);
        Year->blockSignals(true);
        Year->setCurrentIndex(Year->currentIndex()-(currentDate.year()-D.year()));
        Year->blockSignals(false);
        currentDate = D;
        setCalendar();
    }
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
                currentDate.setDate(currentDate.year()-1, currentDate.month(), currentDate.day());
                Year->setCurrentIndex(Year->currentIndex()-1);
            }
        } else {
            if (currentDate.year()+1 < QDate::currentDate().year()+21) {
                currentDate.setDate(currentDate.year()+1, currentDate.month(), currentDate.day());
                Year->setCurrentIndex(Year->currentIndex()+1);
            }
        }
        Year->blockSignals(false);
    } else {
        currentDate.setDate(Input.toInt(), currentDate.month(), currentDate.day());
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
    QDateTime Tmp = QDateTime::currentDateTime();
    QString cur = category;
    category="";
    uint closest = 86400;
    if (first) {
        QList  <QString>  Immediate;
        for (char a = 0; a < 2; a++) {
            Tmp = Tmp.addDays(a);
            for (char i = 5; i > 0; i--) {
                QList  <uint>  NID = checkEvN(Tmp.date(), i);
                QList  <uint>  RID = checkEvR(Tmp.date(), i);
                for (int j = 0; j < NID.size(); j++) {
                    if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) <= 900
                            &&QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) > 0
                            &&!(NotifiedN.contains(NID[j]))) {
                        NotifiedN.append(NID[j]);
                        Immediate.append(Normal[NID[j]].name);
                    } else {
                        if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) < 0) {
                            NotifiedN.removeOne(NID[j]);
                        }
                    }
                }
                for (int j = 0; j < RID.size(); j++) {
                    QDateTime R = QDateTime(Tmp.date(), Recurrent[RID[j]].datet.time());
                    if (QDateTime::currentDateTime().secsTo(R) <= 900
                            &&QDateTime::currentDateTime().secsTo(R) > 0
                            &&!(NotifiedR.contains(RID[j]))) {
                        NotifiedR.append(RID[j]);
                        Immediate.append(Recurrent[RID[j]].name);
                    } else {
                        if (QDateTime::currentDateTime().time().secsTo(Recurrent[RID[j]].datet.time()) < 0) {
                            NotifiedR.removeOne(RID[j]);
                        }
                    }
                }
                for (int j = 0; j < NID.size(); j++) {
                    if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) > 900
                            &&!(NotifiedN.contains(NID[j]))
                            &&QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) < closest) {
                        closest = QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet);
                    }
                }
                for (int j = 0; j < RID.size(); j++) {
                    QDateTime R = QDateTime(Tmp.date(), Recurrent[RID[j]].datet.time());
                    if (QDateTime::currentDateTime().secsTo(R) > 900
                            &&!(NotifiedR.contains(RID[j]))
                            &&QDateTime::currentDateTime().secsTo(R) < closest) {
                        closest = QDateTime::currentDateTime().secsTo(R);
                    }
                }
            }
        }
        if (Immediate.size() > 0) {
            QString M="Immediate notification for:\n";
            for (int i = 0; i < Immediate.size(); i++) {
                M.append(Immediate[i]+"\n");
            }
            M = M.mid(0, M.length()-1);
            emit Notification(M);
        }
    } else {
        QList  <QString>  Notify;
        for (char a = 0; a < 2; a++) {
            Tmp = Tmp.addDays(a);
            for (char i = 5; i > 0; i--) {
                QList  <uint>  NID = checkEvN(Tmp.date(), i);
                QList  <uint>  RID = checkEvR(Tmp.date(), i);
                if (QObject::sender() == NTimer) {
                    for (int j = 0; j < NID.size(); j++) {
                        if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) <= 1800
                                &&QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) >= 900
                                &&!(NotifiedN.contains(NID[j]))) {
                            NotifiedN.append(NID[j]);
                            Notify.append(Normal[NID[j]].name);
                        } else {
                            if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) < 0) {
                                NotifiedN.removeOne(NID[j]);
                            }
                        }
                    }
                    for (int j = 0; j < RID.size(); j++) {
                        QDateTime R = QDateTime(Tmp.date(), Recurrent[RID[j]].datet.time());
                        if (QDateTime::currentDateTime().secsTo(R) <= 1800
                                &&QDateTime::currentDateTime().secsTo(R) >= 900
                                &&!(NotifiedR.contains(RID[j]))) {
                            NotifiedR.append(RID[j]);
                            Notify.append(Recurrent[RID[j]].name);
                        } else {
                            if (QDateTime::currentDateTime().time().secsTo(Recurrent[RID[j]].datet.time()) < 0) {
                                NotifiedR.removeOne(RID[j]);
                            }
                        }
                    }
                    for (int j = 0; j < NID.size(); j++) {
                        if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) > 1800
                                &&!(NotifiedN.contains(NID[j]))
                                &&QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) < closest) {
                            closest = QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet);
                        }
                    }
                    for (int j = 0; j < RID.size(); j++) {
                        QDateTime R = QDateTime(Tmp.date(), Recurrent[RID[j]].datet.time());
                        if (QDateTime::currentDateTime().secsTo(R) > 1800
                                &&!(NotifiedR.contains(RID[j]))
                                &&QDateTime::currentDateTime().secsTo(R) < closest) {
                            closest = QDateTime::currentDateTime().secsTo(R);
                        }
                    }
                } else {
                    for (int j = 0; j < NID.size(); j++) {
                        if (QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) > 900
                                &&!(NotifiedN.contains(NID[j]))
                                &&QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet) < closest) {
                            closest = QDateTime::currentDateTime().secsTo(Normal[NID[j]].datet);
                        }
                    }
                    for (int j = 0; j < RID.size(); j++) {
                        QDateTime R = QDateTime(Tmp.date(), Recurrent[RID[j]].datet.time());
                        if (QDateTime::currentDateTime().secsTo(R) > 900
                                &&!(NotifiedR.contains(RID[j]))
                                &&QDateTime::currentDateTime().secsTo(R) < closest) {
                            closest = QDateTime::currentDateTime().secsTo(R);
                        }
                    }
                }
            }
        }
        if (Notify.size() > 0) {
            QString M="At least 15 minutes notification for:\n";
            for (int i = 0; i < Notify.size(); i++) {
                M.append(Notify[i]+"\n");
            }
            M = M.mid(0, M.length()-1);
            emit Notification(M);
        }
    }
    category = cur;
    NTimer->setInterval((closest-900)*1000);
    NTimer->setSingleShot(true);
    NTimer->start();
}
void qorgCalendar::checkMidnight() {
    int difference = Midnight->remainingTime()-QDateTime::currentDateTime().msecsTo(QDateTime(QDate::currentDate().addDays(1), QTime(00, 00)));
    if (difference > 500 || difference < -500) {
        Midnight->stop();
        Midnight->setInterval(QDateTime::currentDateTime().msecsTo(QDateTime(QDate::currentDate().addDays(1), QTime(00, 00))));
        Midnight->start();
        setNotification(true);
        updateAll();
    }
}
#include "qorgcalendar.moc"
