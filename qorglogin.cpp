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
    QGridLayout* L = new QGridLayout(this);
    L->addWidget(Label[0], 0, 0);
    L->addWidget(Line[0], 0, 1);
    L->addWidget(Label[1], 1, 0);
    L->addWidget(Line[1], 1, 1);
    QHBoxLayout* H = new QHBoxLayout();
    H->addWidget(Reg);
    H->addWidget(Login);
    L->addLayout(H, 2, 0, 1, 2);
    L->addWidget(PassLabel, 3, 0, 1, 2);
}
void qorgLogin::clear() {
    Line[0]->deleteLater();
    Line[1]->deleteLater();
}
void qorgLogin::UserInputValidation(QString input) {
    if (input.isEmpty()) {
        Line[0]->setStyleSheet("QLineEdit{background: #FF8888;}");
        Validated[0]=false;
    } else {
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
}
void qorgLogin::PasswordInputValidation(QString input) {
    if (input.length() < 8 || input.isEmpty()) {
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
        QString path = QDir::homePath()+QDir::separator()+".qorganizer"+QDir::separator()+QString::fromUtf8(QCryptographicHash::hash(Line[0]->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org";
        QFile fileQFile(path);
        if (fileQFile.exists()) {
                if (fileQFile.open(QIODevice::ReadOnly)) {
                    hash = RandomQByteArray();
                    hashed = calculateXOR(QCryptographicHash::hash(
                                              QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), Line[1]->text().toUtf8()).toByteArray(), QCryptographicHash::Sha3_512)
                                              +Line[1]->text().toUtf8()
                                              +QCryptographicHash::hash(Line[1]->text().toUtf8(), QCryptographicHash::Sha3_512)
                                                                , QCryptographicHash::Sha3_256), hash);
                    QByteArray Header = fileQFile.read(15);
                    if (Header == "QOrganizer 1.02"
                            || Header == "QOrganizer 1.03") {
                        QByteArray hash1 = RandomQByteArray();
                        QByteArray hashed1 = calculateXOR(Line[1]->text().toUtf8()+QByteArray(32-Line[1]->text().length(), '\0'), hash1);
                        if (qorgIO::ReadFile(hash1, hashed1, pointer, path)) {
                            pointer->setUser(Line[0]->text(), hash, hashed);
                            this->accept();
                        }
                    } else {
                        if (qorgIO::ReadFile(hash, hashed, pointer, path)) {
                            pointer->setUser(Line[0]->text(), hash, hashed);
                            this->accept();
                        }
                    }
                    Line[1]->clear();
                    fileQFile.close();
                }
        } else {
            QMessageBox::critical(this, "Error", "Wrong usename or password.");
        }
        Line[1]->clear();
        Line[1]->setStyleSheet("QLineEdit{background: white;}");
        PassLabel->clear();
    }
}
void qorgLogin::Register() {
    if (Validated[0]&&Validated[1]) {
        if (!QDir(QDir::homePath()+QDir::separator()+".qorganizer").exists()) {
            QDir Dir;
            Dir.mkdir(QDir::homePath()+QDir::separator()+".qorganizer");
        }
        QString path = QDir::homePath()+QDir::separator()+".qorganizer"+QDir::separator()+QString::fromUtf8(QCryptographicHash::hash(Line[0]->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org";
        if (QFile::exists(path)) {
            QMessageBox::critical(this, "Error", "User already exists");
        } else {
            QString Repeat = QInputDialog::getText(this, "Passsword", "Repeat password", QLineEdit::Password);
            if (Repeat == Line[1]->text()) {
                hash = RandomQByteArray();
                hashed = calculateXOR(QCryptographicHash::hash(
                                          QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), Line[1]->text().toUtf8()).toByteArray(), QCryptographicHash::Sha3_512)
                                          +Line[1]->text().toUtf8()
                                          +QCryptographicHash::hash(Line[1]->text().toUtf8(), QCryptographicHash::Sha3_512)
                                                            , QCryptographicHash::Sha3_256), hash);
                Line[1]->clear();
                qorgIO::SaveFile(hash, hashed, pointer, path);
                if (qorgIO::ReadFile(hash, hashed, pointer, path)) {
                    pointer->setUser(Line[0]->text(), hashed, hash);
                    this->accept();
                }
            } else {
                QMessageBox::critical(this, "Error", "Passwords are different.");
            }
        }
        Line[1]->clear();
        Line[1]->setStyleSheet("QLineEdit{background: white;}");
        PassLabel->clear();
    }
}
