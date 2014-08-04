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

#include <qorgab.h>
#include <algorithm>

class ListItem :public QWidget {
    Q_OBJECT
public:
    ListItem(Person* P, uint IID, QWidget *parent) :QWidget(parent) {
        ItemID = IID;
        L[0]=new QLabel(P->Name+" "+P->Surname, this);
        QString N[2];
        if (P->HouseNumber == 0) {
            N[0]="-";
        } else {
            N[0]=QString::number(P->HouseNumber);
        }
        if (P->Apartment == 0) {
            N[1]="-";
        } else {
            N[1]=QString::number(P->Apartment);
        }
        L[1]=new QLabel(P->Street+" "+N[0]+"/"+N[1], this);
        L[2]=new QLabel(P->Town, this);
        L[3]=new QLabel(P->Email, this);
        L[4]=new QLabel(P->Mobile, this);
        L[5]=new QLabel("Birthday: "+P->Birthday.toString("dd/MM"), this);
        Edit = new QPushButton(QIcon(":/main/Edit.png"), "", this);
        Edit->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Edit, SIGNAL(clicked()), this, SLOT(EditIN()));
        Delete = new QPushButton(QIcon(":/main/Delete.png"), "", this);
        Delete->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Delete, SIGNAL(clicked()), this, SLOT(DeleteIN()));
        QGridLayout *La = new QGridLayout(this);
        La->addWidget(L[0], 0, 0);
        QHBoxLayout *H = new QHBoxLayout();
        H->addWidget(Edit);
        H->addWidget(Delete);
        La->addLayout(H, 0, 1);
        La->addWidget(L[1], 1, 0);
        La->addWidget(L[2], 1, 1);
        La->addWidget(L[3], 2, 0);
        La->addWidget(L[4], 2, 1);
        La->addWidget(L[5], 3, 0);
        P = NULL;
    }

private:
    uint ItemID;
    QLabel *L[6];
    QPushButton *Edit;
    QPushButton *Delete;
private slots:
    void EditIN() {
        emit EditOUT(ItemID);
    }
    void DeleteIN() {
        emit DeleteOUT(ItemID);
    }
signals:
    void EditOUT(uint);
    void DeleteOUT(uint);
};

qorgAB::qorgAB(QWidget *parent) :QWidget(parent) {
    List = new QListWidget(this);
    connect(List, SIGNAL(clicked(QModelIndex)), this, SLOT(Click(QModelIndex)));
    QStringList A;
    A << "Name: " << "Surname: " << "Town: " << "Street: " << "/" << "Email: " << "Mobile: " << "Birthday: ";
    for (uint i = 0; i < 8; i++) {
        L[i]=new QLabel(A[i], this);
    }
    for (uint i = 0; i < 8; i++) {
        E[i]=new QLineEdit(this);
    }
    QIntValidator *V = new QIntValidator(this);
    E[4]->setValidator(V);
    E[5]->setValidator(V);
    E[7]->setInputMask("+00 000 000 000");
    E[7]->setText("00000000000");
    D = new QDateEdit(QDate(2012, QDate::currentDate().month(), QDate::currentDate().day()), this);
    D->setDisplayFormat("dd/MM");
    Add = new QPushButton(QIcon(":/main/Add.png"), "Add", this);
    Add->setShortcut(Qt::Key_Return);;
    connect(Add, SIGNAL(clicked()), this, SLOT(AddS()));
    OKB = new QPushButton(this);
    OKB->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    connect(OKB, SIGNAL(clicked()), this, SLOT(OK()));
    OKB->hide();
    Cancel = new QPushButton(this);
    Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(Cancel, SIGNAL(clicked()), this, SLOT(Can()));
    Cancel->hide();
    lastitem = NULL;
    QGridLayout *Layout = new QGridLayout(this);
    Layout->addWidget(List, 0, 0, 2, 1);
    Layout->setMargin(0);
    La = new QGridLayout();
    La->addWidget(L[0], 0, 0);
    La->addWidget(E[0], 0, 1);
    La->addWidget(L[1], 1, 0);
    La->addWidget(E[1], 1, 1);
    La->addWidget(L[2], 2, 0);
    La->addWidget(E[2], 2, 1);
    La->addWidget(L[3], 3, 0);
    QHBoxLayout *H = new QHBoxLayout();
    H->addWidget(E[3]);
    H->addSpacing(15);
    E[4]->setMaximumWidth(30);
    H->addWidget(E[4]);
    H->addWidget(L[4]);
    E[5]->setMaximumWidth(30);
    H->addWidget(E[5]);
    La->addLayout(H, 3, 1);
    La->addWidget(L[5], 4, 0);
    La->addWidget(E[6], 4, 1);
    La->addWidget(L[6], 5, 0);
    La->addWidget(E[7], 5, 1);
    La->addWidget(L[7], 6, 0);
    La->addWidget(D, 6, 1);
    La->addWidget(Add, 7, 0, 1, 2);
    Layout->addLayout(La, 1, 1);
}
QString qorgAB::output() {
    QString out;
    for (uint i = 0; i < Personv.size(); i++) {
        QString work;
        work.append(OutputTools(Personv[i].Name, "NAME"));
        work.append(OutputTools(Personv[i].Surname, "SURNAME"));
        work.append(OutputTools(Personv[i].Town, "TOWN"));
        work.append(OutputTools(Personv[i].Street, "STREET"));
        work.append(OutputTools(static_cast<int>(Personv[i].HouseNumber), "HOUSENUMBER"));
        work.append(OutputTools(static_cast<int>(Personv[i].Apartment), "APARTMENT"));
        work.append(OutputTools(Personv[i].Email, "EMAIL"));
        work.append(OutputTools(Personv[i].Mobile, "MOBILE"));
        work.append(OutputTools(Personv[i].Birthday.toString(Qt::ISODate), "BIRTHDAY"));
        out.append(OutputToolsS(work, "PERSON"));
    }
    out = OutputToolsS(out, "PERSONV");
    return out;
}
void qorgAB::input(QString IN) {
    while (IN.contains("<PERSON>")) {
        QString PS = InputSS(IN, "PERSON");
        Person tmp;
        tmp.Name = InputS(PS, "NAME");
        tmp.Surname = InputS(PS, "SURNAME");
        tmp.Town = InputS(PS, "TOWN");
        tmp.Street = InputS(PS, "STREET");
        tmp.HouseNumber = InputI(PS, "HOUSENUMBER");
        tmp.Apartment = InputI(PS, "APARTMENT");
        tmp.Email = InputS(PS, "EMAIL");
        tmp.Mobile = InputS(PS, "MOBILE");
        tmp.Birthday = QDate::fromString(InputS(PS, "BIRTHDAY"), Qt::ISODate);
        Personv.push_back(tmp);
        IN.remove(IN.indexOf("<PERSON>"), IN.indexOf("</PERSON>")-IN.indexOf("<PERSON>")+9);
    }
    UpdateList();
}
QList  <QString>  qorgAB::getBirthdays(QDate D) {
    QList  <QString>  BList;
    for (uint i = 0; i < Personv.size(); i++) {
        if (Personv[i].Birthday.day() == D.day()&&Personv[i].Birthday.month() == D.month()) {
            BList.append(Personv[i].Name+" "+Personv[i].Surname);
        }
    }
    return BList;
}
QList  <QString>  qorgAB::getEmails() {
    QList  <QString>  EList;
    for (uint i = 0; i < Personv.size(); i++) {
        if (!Personv[i].Email.isEmpty()) {
            EList.append(Personv[i].Email+" "+Personv[i].Name+" "+Personv[i].Surname);
        }
    }
    return EList;
}
void qorgAB::AddS() {
    if (E[0]->text().isEmpty()) {
        if (E[0]->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(E[0], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        E[0]->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (E[1]->text().isEmpty()) {
        if (E[1]->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(E[1], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        E[1]->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        disconnect(E[0], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(E[1], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        Person temp;
        temp.Name = E[0]->text().simplified();
        temp.Surname = E[1]->text().simplified();
        temp.Town = E[2]->text().simplified();
        temp.Street = E[3]->text().simplified();
        temp.HouseNumber = E[4]->text().toInt();
        temp.Apartment = E[5]->text().toInt();
        temp.Email = E[6]->text().simplified();
        temp.Mobile = E[7]->text();
        temp.Birthday = D->date();
        for (uint i = 0; i < 8; i++) {
            E[i]->clear();
        }
        E[7]->setText("00000000000");
        D->setDate(QDate(2012, QDate::currentDate().month(), QDate::currentDate().day()));
        Personv.push_back(temp);
        UpdateList();
    }
}
void qorgAB::row(QString IN) {
    QLineEdit *I = qobject_cast<QLineEdit*>(QObject::sender());
    if (IN.isEmpty()) {
        I->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        I->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgAB::Click(QModelIndex I) {
    QListWidgetItem *Itm = List->item(I.row());
    if (Itm != lastitem) {
        Itm->setText("");
        Itm->setSizeHint(QSize(Itm->sizeHint().width(), 100));
        ListItem *W = new ListItem(&Personv[I.row()], I.row(), this);
        connect(W, SIGNAL(EditOUT(uint)), this, SLOT(Edit(uint)));
        connect(W, SIGNAL(DeleteOUT(uint)), this, SLOT(Delete(uint)));
        List->setItemWidget(Itm, W);
        lastitem = Itm;
    } else {
        Itm->setSizeHint(QSize(Itm->sizeHint().width(), 20));
        List->removeItemWidget(Itm);
        Itm->setText(Personv[I.row()].Name+" "+Personv[I.row()].Surname);
        lastitem = NULL;
    }
}
void qorgAB::Edit(uint IID) {
    Person *I=&Personv[IID];
    E[0]->setText(I->Name);
    E[1]->setText(I->Surname);
    E[2]->setText(I->Town);
    E[3]->setText(I->Street);
    E[4]->setText(QString::number(I->HouseNumber));
    if (I->HouseNumber == 0) {
        E[4]->setText("");
    }
    E[5]->setText(QString::number(I->Apartment));
    if (I->Apartment == 0) {
        E[5]->setText("");
    }
    E[6]->setText(I->Email);
    E[7]->setText(I->Mobile);
    D->setDate(I->Birthday);
    Add->hide();
    QHBoxLayout *H = new QHBoxLayout();
    H->addWidget(Cancel);
    H->addWidget(OKB);
    La->addLayout(H, 7, 0, 1, 2);
    Cancel->show();
    OKB->show();
    lastIID = IID;
}
void qorgAB::Delete(uint IID) {
    Personv.erase(Personv.begin()+IID);
    Can();
    UpdateList();
}
void qorgAB::OK() {
    if (E[0]->text().isEmpty()) {
        if (E[0]->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(E[0], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        E[0]->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (E[1]->text().isEmpty()) {
        if (E[1]->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(E[1], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        E[1]->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        Personv[lastIID].Name = E[0]->text().simplified();
        Personv[lastIID].Surname = E[1]->text().simplified();
        Personv[lastIID].Town = E[2]->text().simplified();
        Personv[lastIID].Street = E[3]->text().simplified();
        Personv[lastIID].HouseNumber = E[4]->text().toInt();
        Personv[lastIID].Apartment = E[5]->text().toInt();
        Personv[lastIID].Email = E[6]->text().simplified();
        Personv[lastIID].Mobile = E[7]->text();
        Personv[lastIID].Birthday = D->date();
        UpdateList();
        Can();
    }
}
void qorgAB::Can() {
    disconnect(E[0], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
    disconnect(E[1], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
    for (uint i = 0; i < 8; i++) {
        E[i]->clear();
    }
    E[7]->setText("00000000000");
    D->setDate(QDate::currentDate());
    Cancel->hide();
    OKB->hide();
    La->addWidget(Add, 7, 0, 1, 2);
    Add->show();
}
void qorgAB::UpdateList() {
    List->clear();
    if (Personv.size() > 1) {
        while (true) {
            bool OKL = true;
            for (uint i = 0; i < Personv.size()-1; i++) {
                if (Personv[i].Surname > Personv[i+1].Surname) {
                    swap(Personv[i], Personv[i+1]);
                    OKL = false;
                } else if (Personv[i].Surname == Personv[i+1].Surname) {
                    if (Personv[i].Name > Personv[i+1].Name) {
                        swap(Personv[i], Personv[i+1]);
                        OKL = false;
                    }
                }
            }
            if (OKL) {
                break;
            }
        }
    }
    for (uint i = 0; i < Personv.size(); i++) {
        QListWidgetItem *Itm = new QListWidgetItem(List);
        Itm->setText(Personv[i].Name+" "+Personv[i].Surname);
    }
}

#include "qorgab.moc"
