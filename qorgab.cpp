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
    ListItem(Person* P, uint IID, QWidget* parent) :QWidget(parent) {
        ItemID = IID;
        QGraphicsScene* Scene = new QGraphicsScene(this);
        if (!P->Photo.isEmpty()) {
            QPixmap tmpPixmap;
            tmpPixmap.loadFromData(P->Photo);
            Scene->addPixmap(tmpPixmap);
        } else {
            Scene->addPixmap(QPixmap::fromImage(QImage(":/ad/Default.png")));
        }
        Photo = new QGraphicsView(Scene, this);
        Photo->setFixedSize(130,130);
        L[0]=new QLabel(P->Name+" "+P->Surname, this);
        L[1]=new QLabel(P->Category, this);
        L[2]=new QLabel(P->Town, this);
        L[3]=new QLabel(P->Street
                        + (P->HouseNumber != 0 ? " "+QString::number(P->HouseNumber) : "")
                        + (P->Apartment != 0 ? "/"+QString::number(P->Apartment) : ""), this);
        L[4]=new QLabel(P->Mobile, this);
        L[5]=new QLabel(P->Email, this);
        L[6]=new QLabel(this);
        if (!P->Birthday.isEmpty()) {
            L[6]->setText("Birthday: "+QDate::fromString(P->Birthday,Qt::ISODate).toString("dd.MM.yyyy"));
        }
        Edit = new QPushButton(QIcon(":/main/Edit.png"), "", this);
        Edit->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Edit, SIGNAL(clicked()), this, SLOT(EditIN()));
        Delete = new QPushButton(QIcon(":/main/Delete.png"), "", this);
        Delete->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Delete, SIGNAL(clicked()), this, SLOT(DeleteIN()));
        QGridLayout* La = new QGridLayout(this);
        La->setMargin(5);
        La->addWidget(Photo, 0, 0);
        La->addWidget(L[0], 0, 1);
        La->addWidget(L[1], 0, 2);
        La->addWidget(L[2], 1, 1);
        La->addWidget(L[3], 1, 2);
        La->addWidget(L[4], 2, 1);
        La->addWidget(L[5], 2, 2);
        La->addWidget(L[6], 3, 1);
        QHBoxLayout* H = new QHBoxLayout();
        H->addWidget(Edit);
        H->addWidget(Delete);
        La->addLayout(H, 3, 2);
        P = NULL;
    }
    uint ItemID;
private:
    QGraphicsView* Photo;
    QLabel* L[7];
    QPushButton* Edit;
    QPushButton* Delete;
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

qorgAB::qorgAB(QWidget* parent) :QWidget(parent) {

    List = new QListWidget(this);
    connect(List, SIGNAL(clicked(QModelIndex)), this, SLOT(Click(QModelIndex)));

    QStringList A;
    A << "Name: " << "Surname: " << "Category: " << "Town: " << "Street: " << "/" << "Email: " << "Mobile: " << "Birthday: ";
    for (uint i = 0; i < 9; i++) {
        L[i]=new QLabel(A[i], this);
    }
    for (uint i = 0; i < 9; i++) {
        E[i]=new QLineEdit(this);
    }

    Photo = new QPushButton(QIcon(":/ad/Default.png"),"",this);
    Photo->setFixedSize(128,128);
    Photo->setIconSize(QSize(128,128));
    connect(Photo,SIGNAL(clicked()),this,SLOT(SelectPhoto()));

    C = new QCompleter(E[2]); //Set completer for category
    E[2]->setCompleter(C);

    QIntValidator* V = new QIntValidator(this); //House number and apartment must be int
    E[5]->setValidator(V);
    E[6]->setValidator(V);

    BDayCheckBox = new QCheckBox(this);
    connect(BDayCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ActivateBirthdayField()));
    CalendarPopup = new QCalendarWidget(this);
    D = new QDateEdit(QDate::currentDate(), this);
    D->setCalendarPopup(true);
    D->setCalendarWidget(CalendarPopup);
    D->setDisabled(true);

    ExtraInformationField = new QTextBrowser(this);
    ExtraInformationField->setReadOnly(false);
    ExtraInformationField->setAcceptRichText(false);

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

    QHBoxLayout* HLayout = new QHBoxLayout(this);
    HLayout->addWidget(List);
    La = new QGridLayout();
    QGridLayout* GLa = new QGridLayout();
    GLa->addWidget(Photo, 0, 0, 3,3);
    GLa->addWidget(L[0], 0, 3);
    GLa->addWidget(E[0], 0, 4);
    GLa->addWidget(L[1], 1, 3);
    GLa->addWidget(E[1], 1, 4);
    GLa->addWidget(L[2], 2, 3);
    GLa->addWidget(E[2], 2, 4);
    La->addLayout(GLa, 0, 0, 3, 2);
    La->addWidget(L[3], 3, 0);
    La->addWidget(E[3], 3, 1);
    La->addWidget(L[4], 4, 0);
    QHBoxLayout* H = new QHBoxLayout();
    H->addWidget(E[4]);
    H->addSpacing(15);
    E[5]->setMaximumWidth(30);
    H->addWidget(E[5]);
    H->addWidget(L[5]);
    E[6]->setMaximumWidth(30);
    H->addWidget(E[6]);
    La->addLayout(H, 4, 1);
    La->addWidget(L[6], 5, 0);
    La->addWidget(E[7], 5, 1);
    La->addWidget(L[7], 6, 0);
    La->addWidget(E[8], 6, 1);
    QHBoxLayout* Ha = new QHBoxLayout();
    Ha->addWidget(BDayCheckBox);
    Ha->addWidget(L[8]);
    Ha->addWidget(D);
    La->addLayout(Ha, 7, 0, 1, 2);
    La->addWidget(ExtraInformationField,8,0,4,2);
    La->addWidget(Add, 12, 0, 1, 2);
    HLayout->addLayout(La);
}
QStringList qorgAB::getCategories() {
    QStringList list;
    for (uint i = 0; i < Personv.size(); i++) {
        if (Personv[i].Category != ""&&!(list.contains(Personv[i].Category))) {
            list.append(Personv[i].Category);
        }
    }
    list.sort();
    return list;
}
QString qorgAB::output() {
    QString out;
    for (uint i = 0; i < Personv.size(); i++) {
        out.append(Output(Personv[i].Name)+" ");
        out.append(Output(Personv[i].Surname)+" ");
        out.append(Output(Personv[i].Category)+" ");
        out.append(Output(Personv[i].Town)+" ");
        out.append(Output(Personv[i].Street)+" ");
        out.append(Output(Personv[i].HouseNumber)+" ");
        out.append(Output(Personv[i].Apartment)+" ");
        out.append(Output(Personv[i].Email)+" ");
        out.append(Output(Personv[i].Mobile)+" ");
        out.append(Personv[i].Birthday+" ");
        out.append(Output(QString(Personv[i].Photo.toBase64()))+" ");
        out.append(Output(Personv[i].ExtraInformation)+" \n");
    }
    out.append("\n\n");
    return out;
}
void qorgAB::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            if (B.size()-1 == 12) {
                Person Per;
                Per.Name = InputS(B[0]);
                Per.Surname = InputS(B[1]);
                Per.Category = InputS(B[2]);
                Per.Town = InputS(B[3]);
                Per.Street = InputS(B[4]);
                Per.HouseNumber = InputI(B[5]);
                Per.Apartment = InputI(B[6]);
                Per.Email = InputS(B[7]);
                Per.Mobile = InputS(B[8]);
                Per.Birthday = B[9];
                Per.Photo = QByteArray::fromBase64(InputS(B[10]).toUtf8());
                Per.ExtraInformation = InputS(B[11]);
                Personv.push_back(Per);
            }
        }
    }
    UpdateList();
}
QList  <QString>  qorgAB::getBirthdays(QDate D) {
    QList  <QString>  BList;
    for (uint i = 0; i < Personv.size(); i++) {
        if (!Personv[i].Birthday.isEmpty()) {
            QDate BD = QDate::fromString(Personv[i].Birthday,Qt::ISODate);
            if (BD.day() == D.day() && BD.month() == D.month()) {
                BList.append(Personv[i].Name+" "+Personv[i].Surname+" ("+QString::number(D.year()-BD.year())+")");
            }
        }
    }
    if (!QDate::isLeapYear(D.year())
            && D .month() == 2
            && D.day() == 28) {
        return getBirthdays(QDate(D.year(),2,29));
    }

    return BList;
}
QList  <QString>  qorgAB::getEmails() {
    QList  <QString>  EList;
    for (uint i = 0; i < Personv.size(); i++) {
        if (!Personv[i].Email.isEmpty()) {
            EList.append(Personv[i].Email);
        }
    }
    return EList;
}
void qorgAB::SelectPhoto() {
    QString ImagePath = QFileDialog::getOpenFileName(this, "Open image", QDir::homePath());
    if (!ImagePath.isEmpty()) {
        QImage Image;
        if (Image.load(ImagePath)) {
            Image = Image.scaled(100,100,Qt::KeepAspectRatio);
            Photo->setIcon(QIcon(QPixmap::fromImage(Image)));
        } else {
            QMessageBox::critical(this,"Error","File is not an supported image.");
        }
    } else {
        Photo->setIcon(QIcon(":/ad/Default.png"));
    }
}
void qorgAB::ActivateBirthdayField() {
    if (BDayCheckBox->isChecked()) {
        D->setEnabled(true);
    } else {
        D->setDisabled(true);
    }
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
        temp.Category = E[2]->text().simplified();
        temp.Town = E[3]->text().simplified();
        temp.Street = E[4]->text().simplified();
        temp.HouseNumber = E[5]->text().toInt();
        temp.Apartment = E[6]->text().toInt();
        temp.Email = E[7]->text().simplified();
        temp.Mobile = E[8]->text();
        if (BDayCheckBox->isChecked()) {
            temp.Birthday = D->date().toString(Qt::ISODate);
        }
        if (Photo->icon().pixmap(100,100).toImage() != QIcon(":/ad/Default.png").pixmap(100,100).toImage()) {
            QPixmap tempPixmap = Photo->icon().pixmap(100,100);
            QBuffer buffer(&Personv[lastIID].Photo);
            buffer.open(QIODevice::WriteOnly);
            tempPixmap.save(&buffer,"JPG");
        }
        temp.ExtraInformation = ExtraInformationField->toPlainText();
        for (uint i = 0; i < 9; i++) {
            E[i]->clear();
        }
        D->setDate(QDate(2012, QDate::currentDate().month(), QDate::currentDate().day()));
        Personv.push_back(temp);
        emit updateTree();
        UpdateList();
    }
}
void qorgAB::row(QString Input) {
    QLineEdit* I = qobject_cast<QLineEdit*>(QObject::sender());
    if (Input.isEmpty()) {
        I->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        I->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgAB::Click(QModelIndex I) {
    QListWidgetItem* Itm = List->item(I.row());
    if (List->itemWidget(Itm) == NULL) {
        Itm->setSizeHint(QSize(Itm->sizeHint().width(), 140));
        for (uint i = 0; i < Personv.size(); i++) {
            if (Personv[i].Name+" "+Personv[i].Surname == Itm->text()) {
                ListItem* W = new ListItem(&Personv[i], i, this);
                connect(W, SIGNAL(EditOUT(uint)), this, SLOT(Edit(uint)));
                connect(W, SIGNAL(DeleteOUT(uint)), this, SLOT(Delete(uint)));
                List->setItemWidget(Itm, W);
                Itm->setToolTip(Personv[i].Name+" "+Personv[i].Surname+(!Personv[i].ExtraInformation.isEmpty() ? "\n\n"+Personv[i].ExtraInformation : ""));
                break;
            }
        }
        Itm->setText("");

    } else {
        Itm->setSizeHint(QSize(Itm->sizeHint().width(), 15));
        ListItem* W = qobject_cast <ListItem*>(List->itemWidget(Itm));
        Itm->setText(Personv[W->ItemID].Name+" "+Personv[W->ItemID].Surname);
        Itm->setToolTip(Personv[W->ItemID].Name+" "+Personv[W->ItemID].Surname);
        List->removeItemWidget(Itm);
    }
}
void qorgAB::Edit(uint IID) {
    Person* I=&Personv[IID];
    if (!I->Photo.isEmpty()) {
        QPixmap tmpPixmap;
        tmpPixmap.loadFromData(I->Photo);
        Photo->setIcon(QIcon(tmpPixmap));
    }
    E[0]->setText(I->Name);
    E[1]->setText(I->Surname);
    E[2]->setText(I->Category);
    E[3]->setText(I->Town);
    E[4]->setText(I->Street);
    E[5]->setText(QString::number(I->HouseNumber));
    if (I->HouseNumber == 0) {
        E[5]->setText("");
    }
    E[6]->setText(QString::number(I->Apartment));
    if (I->Apartment == 0) {
        E[6]->setText("");
    }
    E[7]->setText(I->Email);
    E[8]->setText(I->Mobile);
    if (!I->Birthday.isEmpty()) {
        D->setDate(QDate::fromString(I->Birthday,Qt::ISODate));
        BDayCheckBox->setChecked(true);
    }
    if (!I->Photo.isEmpty()) {
        QPixmap tmpPixmap;
        tmpPixmap.loadFromData(I->Photo);
        Photo->setIcon(QIcon(tmpPixmap));
    }
    ExtraInformationField->setText(I->ExtraInformation);
    Add->hide();
    QHBoxLayout* H = new QHBoxLayout();
    H->addWidget(Cancel);
    H->addWidget(OKB);
    La->addLayout(H, 12, 0, 1, 2);
    Cancel->show();
    OKB->show();
    lastIID = IID;
}
void qorgAB::Delete(uint IID) {
    Personv.erase(Personv.begin()+IID);
    emit updateTree();
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
        Personv[lastIID].Category = E[2]->text().simplified();
        Personv[lastIID].Town = E[3]->text().simplified();
        Personv[lastIID].Street = E[4]->text().simplified();
        Personv[lastIID].HouseNumber = E[5]->text().toInt();
        Personv[lastIID].Apartment = E[6]->text().toInt();
        Personv[lastIID].Email = E[7]->text().simplified();
        Personv[lastIID].Mobile = E[8]->text();
        if (BDayCheckBox->isChecked()) {
            Personv[lastIID].Birthday = D->date().toString(Qt::ISODate);
        } else {
            Personv[lastIID].Birthday.clear();
        }
        if (Photo->icon().pixmap(100,100).toImage() != QIcon(":/ad/Default.png").pixmap(100,100).toImage()) {
            QPixmap tempPixmap = Photo->icon().pixmap(100,100);
            QBuffer buffer(&Personv[lastIID].Photo);
            buffer.open(QIODevice::WriteOnly);
            tempPixmap.save(&buffer,"JPG");
        }
        Personv[lastIID].ExtraInformation = ExtraInformationField->toPlainText();
        emit updateTree();
        UpdateList();
        Can();
    }
}
void qorgAB::Can() {
    disconnect(E[0], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
    disconnect(E[1], SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
    for (uint i = 0; i < 9; i++) {
        E[i]->clear();
    }
    BDayCheckBox->setChecked(false);
    D->setDate(QDate::currentDate());
    Cancel->hide();
    OKB->hide();
    Photo->setIcon(QIcon(":/ad/Default.png"));
    La->addWidget(Add, 12, 0, 1, 2);
    Add->show();
}
void qorgAB::UpdateList() {
    if (Personv.size() > 1) {
        while (true) {
            bool OKL = true;
            for (uint i = 0; i < Personv.size()-1; i++) {
                if (Personv[i].Surname > Personv[i+1].Surname) {
                    std::swap(Personv[i], Personv[i+1]);
                    OKL = false;
                } else if (Personv[i].Surname == Personv[i+1].Surname) {
                    if (Personv[i].Name > Personv[i+1].Name) {
                        std::swap(Personv[i], Personv[i+1]);
                        OKL = false;
                    }
                }
            }
            if (OKL) {
                break;
            }
        }
    }
    setList();
    delete C;
    C = new QCompleter(getCategories(), E[2]);
    E[2]->setCompleter(C);
}
void qorgAB::setList() {
    List->clear();
    for (uint i = 0; i < Personv.size(); i++) {
        if (Personv[i].Category == currentCategory || currentCategory.isEmpty()) {
            QListWidgetItem* Itm = new QListWidgetItem(List);
            Itm->setText(Personv[i].Name+" "+Personv[i].Surname);
            Itm->setToolTip(Personv[i].Name+" "+Personv[i].Surname);
        }
    }
}

QString qorgAB::exportToVCard() {
    QString output;
    for (uint i = 0; i < Personv.size(); i++) {
        QString vcard;
        vcard.append("BEGIN:VCARD\nVERSION:2.1\n");
        vcard.append("N;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:"
                     + QPEncode(Personv[i].Surname.toUtf8())
                     + ";"
                     + QPEncode(Personv[i].Name.toUtf8())
                     + "\n");
        vcard.append("FN;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:"
                     + QPEncode(QString(Personv[i].Name + " " + Personv[i].Surname).toUtf8())
                     + "\n");
        if (!Personv[i].Mobile.isEmpty()) {
            vcard.append("TEL;CELL:"
                         + Personv[i].Mobile
                         + "\n");
        }
        if (!Personv[i].Email.isEmpty()) {
            vcard.append("EMAIL:"
                         + Personv[i].Email
                         + "\n");
        }
        if (!Personv[i].Town.isEmpty()) {
            vcard.append("ADR;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:;;"
                         + QPEncode(Personv[i].Street.toUtf8())
                         + (Personv[i].HouseNumber != 0 ? " "+QString::number(Personv[i].HouseNumber) : "")
                         + (Personv[i].Apartment != 0 ? "/"+QString::number(Personv[i].Apartment) : "")
                         + ";"
                         + QPEncode(Personv[i].Town.toUtf8())
                         + ";;;\n");
        }
        if (!Personv[i].ExtraInformation.isEmpty()) {
            vcard.append("NOTE;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:"
                         + QPEncode(Personv[i].ExtraInformation.toUtf8())
                         +"\n");
        }
        if (!Personv[i].Birthday.isEmpty()) {
            vcard.append("BDAY:"
                         + QDate::fromString(Personv[i].Birthday,Qt::ISODate).toString("yyyy-MM-dd")
                         +"\n");
        }
        if (!Personv[i].Category.isEmpty()) {
            vcard.append("CATEGORIES;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:"
                         + QPEncode(Personv[i].Category.toUtf8())
                         +"\n");
        }
        if (!Personv[i].Photo.isEmpty()) {
            vcard.append("PHOTO;ENCODING=BASE64;TYPE=JPG:"
                         + Personv[i].Photo.toBase64()
                         + "\n\n");
        }
        vcard.append("END:VCARD\n");
        output.append(vcard);
    }
    return output;
}
#include "qorgab.moc"
