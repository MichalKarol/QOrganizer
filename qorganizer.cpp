//    Copyright (C) 2014 Michał Karol <michal.p.karol@gmail.com>

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

#include <qorganizer.h>

// Main class
QOrganizer::QOrganizer(qorgModel* model, QWidget* parent) : QWidget(parent) {
    this->model = model;

    mainWidgetGenerator();
    stacked->addWidget(loginWidgetGenerator());
    //stacked->addWidget(blockWidgetGenerator());
}
QOrganizer::~QOrganizer() {
    mainTree->clear();
    //model->logout();
}

void QOrganizer::mainWidgetGenerator() {
    this->setWindowIcon(QIcon(":/main/QOrganizer.png"));
    this->setWindowTitle("QOrganizer");

    mainTree = new QTreeWidget();
    mainTree->setSelectionMode(QAbstractItemView::SingleSelection);

    stacked = new QStackedWidget();

    trayIcon = new QSystemTrayIcon(QIcon(":/main/QOrganizer.png"), this);
    trayIcon->show();

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(mainTree, 0, 0, Qt::AlignLeft);
    mainLayout->addWidget(stacked, 0, 1, Qt::AlignVCenter);
}

QWidget* QOrganizer::loginWidgetGenerator() {
    if (loginWidget == NULL) {
        loginWidget = new QWidget(this);
        loginUsername = new ValidatingLineEdit("\\S*", "Username cannot contain whitespaces.", loginWidget);
        loginPassword = new ValidatingLineEdit(".{8,}", "Password must contain at least 8 characters.", loginWidget);
        loginPassword->setEchoMode(QLineEdit::Password);

        registrationButton = new QPushButton("Register", loginWidget);
        connect(registrationButton, &QPushButton::clicked, [&]() {
            if (loginUsername->hasAcceptableInput() && loginPassword->hasAcceptableInput()) {

                // Checking if home directory exists
                if (!QDir(QDir::homePath() + QDir::separator() + ".qorganizer").exists()) {
                    QDir directory(QDir::homePath());
                    directory.mkdir(".qorganizer");
                }

                // Setting path
                QString path = QDir::homePath() + QDir::separator() + ".qorganizer" + QDir::separator() +
                               QString::fromUtf8(QCryptographicHash::hash(loginUsername->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/") + ".org";

                // Testing if user alresy exists (There is low probability of collision)
                if (QFile::exists(path)) {
                    QMessageBox::critical(this, "Error", "User already exists");
                } else {

                    //  Making user repeat the password in order to prevent typo
                    QString repeatedPassword = QInputDialog::getText(this, "Passsword", "Repeat password", QLineEdit::Password);

                    // All in working order, now create the pair and register new user
                    if (repeatedPassword == loginPassword->text()) {
                        QByteArray password = loginPassword->text().toUtf8();
                        QPair <QByteArray, QByteArray> pair;

                        try {
                            pair = createNewPair(password);
                        } catch (QString ex) {
                            QMessageBox::critical(this, tr("Critical error"), tr("Exeption occured: ") + ex);
                            exit(1);
                        }

                        model->registerFunction(pair, path);
                    } else {
                        QMessageBox::critical(this, "Error", "Passwords are different.");
                    }

                    repeatedPassword.clear();
                }
            }

            loginPassword->clear();
        });

        loginButton = new QPushButton("Log in", loginWidget);
        loginButton->setDefault(true);
        connect(loginButton, &QPushButton::clicked, [&]() {
            if (loginUsername->hasAcceptableInput() && loginPassword->hasAcceptableInput()) {

                // Setting path
                QString path = QDir::homePath() + QDir::separator() + ".qorganizer" + QDir::separator() +
                               QString::fromUtf8(QCryptographicHash::hash(loginUsername->text().toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/") + ".org";

                QFile file(path);

                if (file.exists()) {

                    // Due to the lots of changes, some backward compatibility
                    if (file.open(QFile::ReadOnly)) {
                        QByteArray password = loginPassword->text().toUtf8();
                        QPair <QByteArray, QByteArray> pair;
                        QByteArray newHash;

                        try {
                            pair = createNewPair(password);
                        } catch (QString ex) {
                            QMessageBox::critical(this, tr("Critical error"), tr("Exeption occured: ") + ex);
                            exit(1);
                        }

                        QByteArray header = file.read(15);
                        uint version = header.mid(11, 1).toUInt() * 100 + header.mid(13, 2).toUInt();

                        if (version <= 103) {
                            password = loginPassword->text().toUtf8();
                            QByteArray input = password + QByteArray(32 - password.length(), '\0');

                            newHash = pair.second;
                            pair.second = calculateXOR(input, pair.first);

                            input.clear();
                        } else if (version == 104 || version == 105) { // FIXME(mkarol) Only for input fuctions changes. Delete later.
                            password = loginPassword->text().toUtf8();
                            QByteArray input = QCryptographicHash::hash(
                                                   QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), password).toByteArray(), QCryptographicHash::Sha3_512)
                                                   + password
                                                   + QCryptographicHash::hash(password, QCryptographicHash::Sha3_512)
                                                   , QCryptographicHash::Sha3_256);

                            newHash = pair.second;
                            pair.second = calculateXOR(input, pair.first);

                            input.clear();
                        }

                        password.clear();
                        file.close();

                        model->loginFunction(pair, path, version, newHash);
                    } else {
                        QMessageBox::critical(this, "Error", "Cannot open file to read.");
                    }
                } else {
                    QMessageBox::critical(this, "Error", "Wrong usename or password.");
                }
            }

            loginPassword->clear();
        });

        loginInformation = new QLabel(tr("<html>QOrganizer 1.05<p>Created by: Michał Karol (michal.p.karol@gmail.com) "
                                         "13.12.2015<p>You could help in developing this software by donating:<p>"
                                         "Bitcoins: <a href='bitcoin:17wTU13S31LMdVuVmxxXyBwnj7kJwm74wK'>17wTU13S31LMdVuVmxxXyBwnj7kJwm74wK</a></html>"), loginWidget);
        loginInformation->setOpenExternalLinks(true);

        QFormLayout* mainLayout = new QFormLayout(loginWidget);
        mainLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        mainLayout->setLabelAlignment(Qt::AlignLeft);
        mainLayout->addRow("Username: ", loginUsername);
        mainLayout->addRow("Password:", loginPassword);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(registrationButton);
        buttonLayout->addWidget(loginButton);
        mainLayout->addRow(buttonLayout);
        mainLayout->addRow(loginInformation);
    }

    return loginWidget;
}
QWidget* QOrganizer::blockWidgetGenerator() {
    if (blockWidget == NULL) {
        blockWidget = new QWidget(this);

        blockPassword = new ValidatingLineEdit(".{8,}", "Password must contain at least 8 characters.", blockWidget);
        blockPassword->setEchoMode(QLineEdit::Password);

        unlockButton = new QPushButton(QIcon(":/main/Lock.png"), "", blockWidget);
        unlockButton->setShortcut(Qt::Key_Return);
        //connect(unlockButton, &QPushButton::clicked, this, &QOrganizer::unlock);

        QGridLayout* mainLayout = new QGridLayout(blockWidget);
        mainLayout->addWidget(blockPassword, 0, 0, 1, 3);
        mainLayout->addWidget(unlockButton, 1, 1, Qt::AlignHCenter);
    }

    return blockWidget;
}




