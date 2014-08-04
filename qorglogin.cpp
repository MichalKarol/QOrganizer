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

#include <qorglogin.h>

qorgLogin::qorgLogin(QOrganizer* p) :QDialog(p) {
    pointer = p;
    setWindowIcon(QIcon(":/main/QOrganizer.png"));
    setWindowTitle("Login dialog");
    setMinimumWidth(300);
    Label[0]=new QLabel("Username: ", this);
    Label[1]=new QLabel("Password: ", this);
    PassLabel = new QLabel("", this);
    Label[0]->setFont(QFont("", 12));
    Label[1]->setFont(QFont("", 12));
    Line[0]=new QLineEdit(this);
    connect(Line[0], SIGNAL(textChanged(QString)), this, SLOT(UserInputValidation(QString)));
    Line[1]=new QLineEdit(this);
    Line[1]->setEchoMode(QLineEdit::Password);
    connect(Line[1], SIGNAL(textChanged(QString)), this, SLOT(PasswordInputValidation(QString)));
    Login = new QPushButton("Log in", this);
    connect(Login, SIGNAL(clicked()), this, SLOT(Authentication()));
    Reg = new QPushButton("Register", this);
    connect(Reg, SIGNAL(clicked()), this, SLOT(Register()));
    Validated[0]=false;
    Validated[1]=false;
    QGridLayout *L = new QGridLayout(this);
    L->addWidget(Label[0], 0, 0);
    L->addWidget(Line[0], 0, 1);
    L->addWidget(Label[1], 1, 0);
    L->addWidget(Line[1], 1, 1);
    QHBoxLayout *H = new QHBoxLayout();
    H->addWidget(Reg);
    H->addWidget(Login);
    L->addLayout(H, 2, 0, 1, 2);
    L->addWidget(PassLabel, 3, 0, 1, 2);
}
void qorgLogin::clear() {
    delete Line[0];
    delete Line[1];
}
void qorgLogin::UserInputValidation(QString input) {
    bool whitespaces = false;
    for (int i = 0; i < input.length(); i++) {
        if (input[i].isSpace()) {
            whitespaces = true;
            break;
        }
    }
    if (whitespaces) {
        Line[0]->setStyleSheet("QLineEdit{background: #FF8888;}");
        Validated[0]=false;
    } else {
        Line[0]->setStyleSheet("QLineEdit{background: white;}");
        Validated[0]=true;
    }
}
void qorgLogin::PasswordInputValidation(QString input) {
    if (input.length() < 8&&!input.isEmpty()) {
        Line[1]->setStyleSheet("QLineEdit{background: #FF8888;}");
        PassLabel->setText("Password should be at least 8 characters long!");
        Validated[1]=false;
    } else {
        PassLabel->setText("");
        bool whitespaces = false;
        for (int i = 0; i < input.length(); i++) {
            if (input[i].isSpace()) {
                whitespaces = true;
                break;
            }
        }
        if (whitespaces) {
            Line[1]->setStyleSheet("QLineEdit{background: #FF8888;}");
            PassLabel->setText("Password should not contain white spaces!");
            Validated[1]=false;
        } else {
            Line[1]->setStyleSheet("QLineEdit{background: white;}");
            Validated[1]=true;
        }
    }
}
void qorgLogin::Authentication() {
    if (Validated[0]&&Validated[1]) {
        QString path = QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(Line[0]->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org";
        QFileInfo UserFile(path);
        if (UserFile.exists()) {
            if (UserFile.permission(QFile::ReadUser)) {
                hash = new QString(QCryptographicHash::hash(salting(Line[1]->text()).toUtf8(), QCryptographicHash::Sha3_512));
                hashed = new QString(calculateXOR(Line[1]->text().toUtf8(), hash->toUtf8()));
                Line[1]->text().clear();
                if (qorgIO::ReadFile(hashed, hash, pointer, path)) {
                    pointer->setUser(Line[0]->text(), hashed, hash);
                    this->accept();
                }
            }
        } else {
            QMessageBox::critical(this, "Error", "Wrong usename or password.");
        }
        Line[1]->clear();
    }
}
void qorgLogin::Register() {
    if (Validated[0]&&Validated[1]) {
        if (!QDir(QDir::homePath()+"/.qorganizer/").exists()) {
            QDir Dir;
            Dir.mkdir(QDir::homePath()+"/.qorganizer/");
        }
        QString path = QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(Line[0]->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org";
        if (QFile::exists(path)) {
            QMessageBox::critical(this, "Error", "User already exists");
        } else {
            hash = new QString(QCryptographicHash::hash(salting(Line[1]->text()).toUtf8(), QCryptographicHash::Sha3_512));
            hashed = new QString(calculateXOR(Line[1]->text().toUtf8(), hash->toUtf8()));
            qorgIO::SaveFile(hashed, hash, pointer, path);
            pointer->setUser(Line[0]->text(), hashed, hash);
            this->accept();
        }
        Line[1]->clear();
    }
}
