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

#include <qorgpasswd.h>
#include <algorithm>

Program::Program() {
    Name.clear();
    Passwordv.clear();
}
Program::~Program() {
    Name.clear();
    Passwordv.clear();
}
Password::Password() {
    Login.clear();
    Passwd.clear();
}
Password::~Password() {
    Login.clear();
    Passwd.clear();
}

class TreeWidget :public QWidget {
    Q_OBJECT
public:
    TreeWidget(Password* I, QPoint P, QWidget *parent) :QWidget(parent) {
        this->P = P;
        Edit = new QPushButton(QIcon(":/main/Edit.png"), "", this);
        Edit->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Edit, SIGNAL(clicked()), this, SLOT(EditIN()));
        Delete = new QPushButton(QIcon(":/main/Delete.png"), "", this);
        Delete->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Delete, SIGNAL(clicked()), this, SLOT(DeleteIN()));
        QGraphicsScene *Scene = new QGraphicsScene(this);
        Scene->addItem(new QGraphicsTextItem("Login: "+I->Login+"\nPassword: "+QString(calculateXOR(QByteArray::fromBase64(I->Passwd.toUtf8()), QCryptographicHash::hash(I->Login.toUtf8(), QCryptographicHash::Sha3_512)))));
        View = new QGraphicsView(Scene, this);
        QGridLayout *G = new QGridLayout(this);
        G->addWidget(View, 0, 0, 2, 1);
        G->addWidget(Edit, 0, 1);
        G->addWidget(Delete, 1, 1);
        activate(false);
        I = NULL;
    }
    void activate(bool T) {
        if (T) {
            View->show();
            Edit->show();
            Delete->show();
        } else {
            View->hide();
            Edit->hide();
            Delete->hide();
        }
    }
    QPoint P;
private:
    QGraphicsView *View;
    QPushButton *Edit;
    QPushButton *Delete;
private slots:
    void EditIN() {
        emit EditOUT(P);
    }
    void DeleteIN() {
        emit DeleteOUT(P);
    }
signals:
    void EditOUT(QPoint);
    void DeleteOUT(QPoint);
};

qorgPasswd::qorgPasswd(QWidget *parent) :QWidget(parent) {
    Tree = new QTreeWidget(this);
    Tree->header()->hide();
    connect(Tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(clicked(QTreeWidgetItem*)));
    C = new QCompleter(this);
    L[0]=new QLabel("Program: ", this);
    ProgramL = new QLineEdit(this);
    L[1]=new QLabel("Login: ", this);
    LoginL = new QLineEdit(this);
    L[2]=new QLabel("Password: ", this);
    PasswordL = new QLineEdit(this);
    PasswordL->setEchoMode(QLineEdit::Password);
    Change = new QPushButton(QIcon(":/passwd/Light.png"), "", this);
    connect(Change, SIGNAL(clicked()), this, SLOT(change()));
    Add = new QPushButton(QIcon(":/main/Add.png"), "", this);
    Add->setShortcut(Qt::Key_Return);
    connect(Add, SIGNAL(clicked()), this, SLOT(AddB()));
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
    Layout->addWidget(Tree, 0, 0, 2, 1);
    Layout->setMargin(0);
    La = new QGridLayout();
    La->addWidget(L[0], 0, 0);
    La->addWidget(ProgramL, 0, 1, 1, 2);
    La->addWidget(L[1], 1, 0);
    La->addWidget(LoginL, 1, 1, 1, 2);
    La->addWidget(L[2], 2, 0);
    La->addWidget(PasswordL, 2, 1);
    La->addWidget(Change, 2, 2);
    La->addWidget(Add, 3, 0, 1, 3);
    Layout->addLayout(La, 1, 1);
}
qorgPasswd::~qorgPasswd() {
    for (uint i = 0; i < Programv.size(); i++) {
        for (uint j = 0; j < Programv[i].Passwordv.size(); j++) {
            delete Programv[i].Passwordv[j];
        }
    }
}
QString qorgPasswd::output() {
    QString out;
    for (uint i = 0; i < Programv.size(); i++) {
        out.append(Output(Programv[i].Name)+" \n");
        for (uint j = 0; j < Programv[i].Passwordv.size(); j++) {
            out.append(Output(Programv[i].Passwordv[j]->Login)+" ");
            out.append(Output(Programv[i].Passwordv[j]->Passwd)+" \n");
        }
    }
    out.append("\n\n");
    return out;
}
void qorgPasswd::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        Program *cProgram;
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 1: {
                Programv.push_back(Program());
                cProgram = &Programv.back();
                cProgram->Name = InputS(B[0]);
            }break;
            case 2: {
                Password *cPassword = new Password();
                cProgram->Passwordv.push_back(cPassword);
                cPassword->Login = InputS(B[0]);
                cPassword->Passwd = InputS(B[1]);
            }break;
            }
        }
    }
    UpdateTree();
}

void qorgPasswd::UpdateTree() {
    if (Programv.size() > 0) {
        Tree->clear();
        while (true) {
            bool Sorted = true;
            for (uint i = 0; i < Programv.size()-1; i++) {
                if (Programv[i].Name > Programv[i+1].Name) {
                    swap(Programv[i], Programv[i+1]);
                    Sorted = false;
                }
            }
            if (Sorted) {
                break;
            }
        }
        QStringList A;
        for (uint i = 0; i < Programv.size(); i++) {
            if (Programv[i].Passwordv.size() > 0) {

                QTreeWidgetItem *Prog = new QTreeWidgetItem(Tree);
                Prog->setText(0, Programv[i].Name);
                while (true) {
                    bool OK = true;
                    for (uint j = 0; j < Programv[i].Passwordv.size()-1; j++) {
                        if (Programv[i].Passwordv[j]->Login > Programv[i].Passwordv[j+1]->Login) {
                            swap(Programv[i].Passwordv[j], Programv[i].Passwordv[j+1]);
                            OK = false;
                        }
                    }
                    if (OK) {
                        break;
                    }
                }
                for (uint j = 0; j < Programv[i].Passwordv.size(); j++) {
                    QTreeWidgetItem *Pass = new QTreeWidgetItem(Prog);
                    TreeWidget *W = new TreeWidget(Programv[i].Passwordv[j], QPoint(i, j), this);
                    connect(W, SIGNAL(EditOUT(QPoint)), this, SLOT(Edit(QPoint)));
                    connect(W, SIGNAL(DeleteOUT(QPoint)), this, SLOT(Delete(QPoint)));
                    Pass->setText(0, Programv[i].Passwordv[j]->Login);
                    Tree->setItemWidget(Pass, 0, W);
                }
                A << Programv[i].Name;
            } else {
                Programv.erase(Programv.begin()+i);
                i--;
            }
        }
        Tree->expandAll();
        C->deleteLater();
        C = new QCompleter(A, this);
        ProgramL->setCompleter(C);
    }
}
void qorgPasswd::change() {
    if (PasswordL->echoMode() == QLineEdit::Password) {
        PasswordL->setEchoMode(QLineEdit::Normal);
    } else {
        PasswordL->setEchoMode(QLineEdit::Password);
    }
}
void qorgPasswd::AddB() {
    if (LoginL->text().isEmpty()) {
        if (LoginL->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(LoginL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        LoginL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (PasswordL->text().isEmpty()) {
        if (PasswordL->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(PasswordL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        PasswordL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        disconnect(LoginL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(PasswordL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        QString P = ProgramL->text();
        if (P.isEmpty()) {
            P="Other";
        }
        bool nfound = true;
        for (uint i = 0; i < Programv.size(); i++) {
            if (P == Programv[i].Name) {
                bool OK = true;
                for (uint j = 0; j < Programv[i].Passwordv.size(); j++) {
                    if (Programv[i].Passwordv[j]->Login == LoginL->text()) {
                        OK = false;
                        break;
                    }
                }
                if (OK) {
                    Password *Pa = new Password();
                    Pa->Login = LoginL->text();
                    Pa->Passwd = QString(calculateXOR(PasswordL->text().toUtf8(), QCryptographicHash::hash(LoginL->text().toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
                    Programv[i].Passwordv.push_back(Pa);
                } else {
                    QMessageBox::critical(this, "Error", "Login alredy saved in program.");
                }
                nfound = false;
                break;
            }
        }
        if (nfound) {
            Program Pr;
            Pr.Name = P;
            Password *Pa = new Password();
            Pa->Login = LoginL->text();
            Pa->Passwd = QString(calculateXOR(PasswordL->text().toUtf8(), QCryptographicHash::hash(LoginL->text().toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
            Pr.Passwordv.push_back(Pa);
            Programv.push_back(Pr);
        }
        ProgramL->clear();
        LoginL->clear();
        PasswordL->clear();
        UpdateTree();
        PasswordL->setEchoMode(QLineEdit::Password);
    }
}
void qorgPasswd::row(QString Input) {
    QLineEdit *I = qobject_cast<QLineEdit*>(QObject::sender());
    if (Input.isEmpty()) {
        I->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        I->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgPasswd::clicked(QTreeWidgetItem* W) {
    if (W->parent() != NULL) {
        TreeWidget *Wi = qobject_cast  <TreeWidget*> (Tree->itemWidget(W, 0));
        if (W != lastitem) {
            W->setText(0, "");
            W->setSizeHint(0, QSize(W->sizeHint(0).width(), 130));
            Wi->activate(true);
            lastitem = W;
        } else {
            Wi->activate(false);
            W->setSizeHint(0, QSize(W->sizeHint(0).width(), 20));
            W->setText(0, Programv[Wi->P.x()].Passwordv[Wi->P.y()]->Login);
            lastitem = NULL;
        }
        Tree->collapseAll();
        Tree->expandAll();
    }
}
void qorgPasswd::Edit(QPoint P) {
    Add->hide();
    QHBoxLayout *H = new QHBoxLayout();
    H->addWidget(Cancel);
    H->addWidget(OKB);
    La->addLayout(H, 3, 0, 1, 2);
    OKB->show();
    Cancel->show();
    Last = P;
    ProgramL->setText(Programv[P.x()].Name);
    LoginL->setText(Programv[P.x()].Passwordv[P.y()]->Login);
}
void qorgPasswd::OK() {
    if (LoginL->text().isEmpty()) {
        if (LoginL->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(LoginL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        LoginL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (PasswordL->text().isEmpty()) {
        if (PasswordL->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(PasswordL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        PasswordL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        Password *Pass = Programv[Last.x()].Passwordv[Last.y()];
        Programv[Last.x()].Passwordv.erase(Programv[Last.x()].Passwordv.begin()+Last.y());
        disconnect(LoginL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(PasswordL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        QString P = ProgramL->text();
        if (P.isEmpty()) {
            P="Other";
        }
        bool nfound = true;
        for (uint i = 0; i < Programv.size(); i++) {
            if (P == Programv[i].Name) {
                bool OK = true;
                for (uint j = 0; j < Programv[i].Passwordv.size(); j++) {
                    if (Programv[i].Passwordv[j]->Login == LoginL->text()) {
                        OK = false;
                        break;
                    }
                }
                if (OK) {
                    delete Pass;
                    Password *Pa = new Password();
                    Pa->Login = LoginL->text();
                    Pa->Passwd = QString(calculateXOR(PasswordL->text().toUtf8(), QCryptographicHash::hash(LoginL->text().toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
                    Programv[i].Passwordv.push_back(Pa);
                } else {
                    QMessageBox::critical(this, "Error", "Login alredy saved in program.");
                    Programv[i].Passwordv.push_back(Pass);
                }
                nfound = false;
                break;
            }
        }
        if (nfound) {
            delete Pass;
            Program Pr;
            Pr.Name = P;
            Password *Pa = new Password();
            Pa->Login = LoginL->text();
            Pa->Passwd = QString(calculateXOR(PasswordL->text().toUtf8(), QCryptographicHash::hash(LoginL->text().toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
            Pr.Passwordv.push_back(Pa);
            Programv.push_back(Pr);
        }
        Can();
        UpdateTree();
    }
}
void qorgPasswd::Can() {
    ProgramL->clear();
    LoginL->clear();
    PasswordL->clear();
    Add->show();
    La->addWidget(Add, 3, 0, 1, 2);
    OKB->hide();
    Cancel->hide();
}
void qorgPasswd::Delete(QPoint P) {
    delete Programv[P.x()].Passwordv[P.y()];
    Programv[P.x()].Passwordv.erase(Programv[P.x()].Passwordv.begin()+P.y());
    Can();
    UpdateTree();
}

#include "qorgpasswd.moc"


