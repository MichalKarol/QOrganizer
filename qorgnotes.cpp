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

#include <qorgnotes.h>
#include <algorithm>

qorgNotes::qorgNotes(QWidget *parent) :QWidget(parent) {
    Layout = new QGridLayout(this);
    Layout->setMargin(0);
    TLabel = new QLabel("Topic: ", this);
    Topic = new QLineEdit(this);
    Topic->setReadOnly(true);
    Priority = new QSlider(Qt::Horizontal, this);
    Priority->setDisabled(true);
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
    Text = new QTextEdit(this);
    Text->setReadOnly(true);
    OK = new QPushButton("OK", this);
    OK->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    OK->hide();
    connect(OK, SIGNAL(clicked()), this, SLOT(EOK()));
    Cancel = new QPushButton("Cancel", this);
    Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    Cancel->hide();
    connect(Cancel, SIGNAL(clicked()), this, SLOT(EC()));
    List = new QTreeWidget(this);
    List->header()->hide();
    List->setColumnCount(3);
    List->header()->setStretchLastSection(false);
    List->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    List->setColumnWidth(1, 50);
    List->setColumnWidth(2, 50);
    connect(List, SIGNAL(clicked(QModelIndex)), this, SLOT(EClicked(QModelIndex)));
    AddB = new QPushButton(QIcon(":/main/Add.png"), "Add", this);
    AddB->setShortcut(Qt::Key_Return);
    connect(AddB, SIGNAL(clicked()), this, SLOT(Add()));
    Layout->addWidget(TLabel, 0, 0);
    Layout->addWidget(Topic, 0, 1);
    Layout->addWidget(Priority, 1, 0, 1, 2);
    Layout->addWidget(Text, 2, 0, 2, 2);
    Layout->addWidget(List, 0, 2, 3, 1);
    Layout->addWidget(AddB, 3, 2);
}
QString qorgNotes::output() {
    QString out;
    for (uint i = 0; i < Notes.size(); i++) {
        QString work;
        work.append(OutputTools(Notes[i].topic, "TOPIC"));
        work.append(OutputTools(Notes[i].priority, "PRIORITY"));
        work.append(OutputTools(Notes[i].text, "TEXT"));
        out.append(OutputToolsS(work, "NOTE"));
    }
    out = OutputToolsS(out, "NOTES");
    return out;
}
void qorgNotes::input(QString IN) {
    while (IN.contains("<NOTE>")) {
        QString NS = InputSS(IN, "NOTE");
        Note tmp;
        tmp.topic = InputS(NS, "TOPIC");
        tmp.priority = InputI(NS, "PRIORITY");
        tmp.text = InputS(NS, "TEXT");
        Notes.push_back(tmp);
       IN.remove(IN.indexOf("<NOTE>"), IN.indexOf("</NOTE>")-IN.indexOf("<NOTE>")+7);
    }
    updateList();
}
void qorgNotes::updateList() {
    if (Notes.size() > 1) {
        while (1) {
            bool Sorted = true;
            for (uint i = 0; i < Notes.size()-1; i++) {
                if (Notes[i].priority < Notes[i+1].priority) {
                    swap(Notes[i], Notes[i+1]);
                    Sorted = false;
                } else {
                    if (Notes[i].priority == Notes[i+1].priority&&Notes[i].topic > Notes[i+1].topic) {
                        swap(Notes[i], Notes[i+1]);
                        Sorted = false;
                    }
                }
            }
            if (Sorted) {
                break;
            }
        }
    }
    List->clear();
    for (uint i = 0; i < Notes.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(List);
        Itm->setText(0, Notes[i].topic);
        Itm->setToolTip(0, Notes[i].topic);
        Itm->setText(1, "");
        Itm->setText(2, "");
        QItemPushButton *Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, i);
        connect(Edit, SIGNAL(clicked(uint)), this, SLOT(Edit(uint)));
        List->setItemWidget(Itm, 1, Edit);
        QItemPushButton *Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, i);
        connect(Delete, SIGNAL(clicked(uint)), this, SLOT(Delete(uint)));
        List->setItemWidget(Itm, 2, Delete);
        colorItem(Itm, Notes[i].priority);
    }
}
void qorgNotes::Add() {
    if (Topic->isReadOnly()) {
        Topic->setReadOnly(false);
        Priority->setEnabled(true);
        Text->setReadOnly(false);
        QHBoxLayout *L1 = new QHBoxLayout();
        L1->addWidget(Cancel);
        L1->addWidget(OK);
        OK->show();
        Cancel->show();
        Layout->addWidget(TLabel, 0, 0);
        Layout->addWidget(Topic, 0, 1);
        Layout->addWidget(Priority, 1, 0, 1, 2);
        Layout->addWidget(Text, 2, 0, 1, 2);
        Layout->addLayout(L1, 3, 0, 1, 2);
        Layout->addWidget(List, 0, 2, 3, 1);
        Layout->addWidget(AddB, 3, 2);
    }
    Topic->clear();
    Priority->setSliderPosition(3);
    Text->clear();
    edited=-1;
}
void qorgNotes::Edit(uint IID) {
    if (Topic->isReadOnly()) {
        Topic->setReadOnly(false);
        Priority->setEnabled(true);
        Text->setReadOnly(false);
        QHBoxLayout *L1 = new QHBoxLayout();
         L1->addWidget(Cancel);
        L1->addWidget(OK);
        OK->show();
        Cancel->show();
        Layout->addWidget(TLabel, 0, 0);
        Layout->addWidget(Topic, 0, 1);
        Layout->addWidget(Priority, 1, 0, 1, 2);
        Layout->addWidget(Text, 2, 0, 1, 2);
        Layout->addLayout(L1, 3, 0, 1, 2);
        Layout->addWidget(List, 0, 2, 3, 1);
        Layout->addWidget(AddB, 3, 2);
    }
    Topic->setText(Notes[IID].topic);
    Priority->setSliderPosition(Notes[IID].priority);
    Text->setText(Notes[IID].text);
    edited = IID;
}
void qorgNotes::Delete(uint IID) {
    Notes.erase(Notes.begin()+IID);
    EC();
    updateList();
}
void qorgNotes::EOK() {
    if (Text->toPlainText().isEmpty()||Topic->text().isEmpty()) {
        if (Topic->text().isEmpty()&&Topic->styleSheet() != "QLineEdit{background: #FF8888;}") {
            Topic->setStyleSheet("QLineEdit{background: #FF8888;}");
            connect(Topic, SIGNAL(textChanged(QString)), this, SLOT(ChangeT()));
        }
        if (Text->toPlainText().isEmpty()&&Text->styleSheet() != "QTextEdit{background: #FF8888;}") {
            Text->setStyleSheet("QTextEdit{background: #FF8888;}");
            connect(Text, SIGNAL(textChanged()), this, SLOT(ChangeX()));
        }
        return;
    }
    if (edited == -1) {
        Note str;
        str.topic = Topic->text();
        str.priority = Priority->value();
        str.text = Text->document()->toPlainText();
        Notes.push_back(str);
    } else {
        Notes[edited].topic = Topic->text();
        Notes[edited].priority = Priority->value();
        Notes[edited].text = Text->document()->toPlainText();
    }
    EC();
    updateList();
}
void qorgNotes::EC() {
    OK->hide();
    Cancel->hide();
    Layout->addWidget(TLabel, 0, 0);
    Layout->addWidget(Topic, 0, 1);
    Layout->addWidget(Priority, 1, 0, 1, 2);
    Layout->addWidget(Text, 2, 0, 2, 2);
    Layout->addWidget(List, 0, 2, 3, 1);
    Layout->addWidget(AddB, 3, 2);
    Topic->setStyleSheet("QLineEdit{background: white;}");
    disconnect(Topic, SIGNAL(textChanged(QString)), this, SLOT(ChangeT()));
    Topic->clear();
    Topic->setReadOnly(true);
    Priority->setSliderPosition(3);
    Priority->setDisabled(true);
    Text->setStyleSheet("QTextEdit{background: white;}");
    disconnect(Text, SIGNAL(textChanged()), this, SLOT(ChangeX()));
    Text->clear();
    Text->setReadOnly(true);
}
void qorgNotes::EClicked(QModelIndex IN) {
    Topic->setText(Notes[IN.row()].topic);
    Priority->setValue(Notes[IN.row()].priority);
    Text->setText(Notes[IN.row()].text);
}
void qorgNotes::ChangeT() {
    if (!Topic->text().isEmpty()) {
        Topic->setStyleSheet("QLineEdit{background: white;}");
    }
}
void qorgNotes::ChangeX() {
    if (!Text->toPlainText().isEmpty()) {
        Text->setStyleSheet("QTextEdit{background: white;}");
    }
}
