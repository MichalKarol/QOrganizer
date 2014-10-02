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

#include <qorgoptions.h>
#include <vector>

bool VectorSearch(vector <QSslCertificate>* A, QSslCertificate B) {
    for (uint i = 0; i < A->size(); i++) {
        if ((*A)[i] == B) {
            return true;
        }
    }
    return false;
}

qorgOptions::qorgOptions(QWidget *parent) :QWidget(parent) {
    UInterval = 30;
    BInterval = 30;
    UTimer = new QTimer(this);
    connect(UTimer, SIGNAL(timeout()), this, SLOT(UTimeout()));
    BTimer = new QTimer(this);
    connect(BTimer, SIGNAL(timeout()), this, SLOT(BTimeout()));

    currentW = 2;
    A[0] = new QLabel("Current password", this);
    A[1] = new QLabel("New password", this);
    A[2] = new QLabel("Update interval", this);
    A[3] = new QLabel("Block interval", this);
    CPassword = new QLineEdit(this);
    CPassword->setEchoMode(QLineEdit::Password);
    connect(CPassword, SIGNAL(textChanged(QString)), this, SLOT(Validator(QString)));
    NPassword = new QLineEdit(this);
    NPassword->setEchoMode(QLineEdit::Password);
    connect(NPassword, SIGNAL(textChanged(QString)), this, SLOT(Validator(QString)));
    UInt = new QSpinBox(this);
    BInt = new QSpinBox(this);
    Passwd = new QPushButton("Change\npassword", this);
    Passwd->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    connect(Passwd, SIGNAL(clicked()), this, SLOT(ChangePassword()));
    Interval = new QPushButton("Change\ninterval", this);
    Interval->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    connect(Interval, SIGNAL(clicked()), this, SLOT(ChangeInterval()));
    Spacer = new QSpacerItem(250, 250);
    W1 << A[0] << A[1] << A[2] << A[3] << CPassword << NPassword << UInt << BInt << Passwd << Interval;

    B[0] = new QLabel("Accepted SSL Certificates.", this);
    B[1] = new QLabel("Blacklisted SSL Certificates.", this);
    Accepted = new QListWidget(this);
    connect(Accepted, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DClicked(QModelIndex)));
    Blacklisted = new QListWidget(this);
    connect(Blacklisted, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DClicked(QModelIndex)));
    W2 << B[0] << B[1] << Accepted << Blacklisted;

    Layout = new QGridLayout(this);
}
int qorgOptions::checkCertificate(QSslCertificate I) {
    if (VectorSearch(&SSLCertA, I)) {
        return 1;
    } else if (VectorSearch(&SSLCertB, I)) {
        return -1;
    }
    return 0;
}
void qorgOptions::setWidget(uint W) {
    if (W != currentW) {
        if (W == 0) {
            for (int i = 0; i < W1.size(); i++) {
                W1[i]->show();
            }
            for (int i = 0; i < W2.size(); i++) {
                W2[i]->hide();
                Layout->removeWidget(W2[i]);
            }
            Layout->addWidget(A[0], 0, 0, 1, 2);
            Layout->addWidget(CPassword, 1, 0, 1, 2);
            Layout->addWidget(A[1], 2, 0, 1, 2);
            Layout->addWidget(NPassword, 3, 0, 1, 2);
            Layout->addWidget(Passwd, 0, 2, 4, 1);
            Layout->addWidget(A[2], 4, 0);
            Layout->addWidget(A[3], 4, 1);
            Layout->addWidget(UInt, 5, 0);
            Layout->addWidget(BInt, 5, 1);
            Layout->addWidget(Interval, 4, 2, 2, 1);
            Layout->addItem(Spacer, 6, 0, 5, 3);
            currentW = 0;
        } else {
            for (int i = 0; i < W2.size(); i++) {
                W2[i]->show();
            }
            for (int i = 0; i < W1.size(); i++) {
                W1[i]->hide();
                Layout->removeWidget(W1[i]);
            }
            Layout->removeItem(Spacer);
            Layout->addWidget(B[0], 0, 0);
            Layout->addWidget(B[1], 0, 1);
            Layout->addWidget(Accepted, 1, 0);
            Layout->addWidget(Blacklisted, 1, 1);
            currentW = 1;
            for (int i = SSLCertTmp.size(); i > 0; i--) {
                if (checkCertificate(SSLCertTmp[i-1]) == 0) {
                    if ((new CertAccept(SSLCertTmp[i-1]))->exec() == QDialog::Accepted) {
                        acceptSSLCert(SSLCertTmp[i-1]);
                    } else {
                        blacklistSSLCert(SSLCertTmp[i-1]);
                    }
                }
            }
        }
    }
}
QString qorgOptions::output() {
    QString Out;
    Out.append(Output(UInterval)+" ");
    Out.append(Output(BInterval)+" \n");
    for (uint i = 0; i < SSLCertA.size(); i++) {
        Out.append(Output(QString(SSLCertA[i].toPem()))+" \n");
    }
    for (uint i = 0; i < SSLCertB.size(); i++) {
        Out.append(Output(QString(SSLCertB[i].toPem()))+" B B \n");
    }
    Out.append("\n\n");
    return Out;
}
void qorgOptions::input(QString Input) {
    QStringList A = Input.split("\n");
    for (int i = 0; i < A.size(); i++) {
        QStringList B = A[i].split(" ");
        switch (B.size()-1) {
        case 1: {
            SSLCertA.push_back(QSslCertificate::fromData(InputS(B[0]).toUtf8()).first());
            Accepted->addItem(SSLCertA.back().serialNumber());
        }break;
        case 2: {
            UInterval = InputI(B[0]);
            BInterval = InputI(B[1]);
        }break;
        case 3: {
            SSLCertB.push_back(QSslCertificate::fromData(InputS(B[0]).toUtf8()).first());
            Blacklisted->addItem(SSLCertB.back().serialNumber());
        }break;
        }
    }
    UInt->setValue(UInterval);
    BInt->setValue(BInterval);
}
void qorgOptions::acceptSSLCert(QSslCertificate C) {
    if (!VectorSearch(&SSLCertA, C)) {
        SSLCertA.push_back(C);
        Accepted->addItem(C.serialNumber());
    }
}
void qorgOptions::blacklistSSLCert(QSslCertificate C) {
    if (!VectorSearch(&SSLCertB, C)) {
        SSLCertB.push_back(C);
        Blacklisted->addItem(C.serialNumber());
    }
}
void qorgOptions::addForVeryfication(QSslCertificate C) {
    SSLCertTmp.append(C);
}
void qorgOptions::start(bool I) {
    if (!I) {
        UTimer->setInterval(UInterval*60*1000);
        UTimer->start();
    } else {
        BTimer->setInterval(BInterval*60*1000);
        BTimer->start();
    }
}
void qorgOptions::stop(bool I) {
    if (!I) {
        UTimer->stop();
    } else {
        BTimer->stop();
    }
}
void qorgOptions::UTimeout() {
    emit Update();
}
void qorgOptions::BTimeout() {
    emit Block();
}
void qorgOptions::ChangeInterval() {
    if (UInt->value() != 0&&BInt->value() != 0) {
        UInterval = UInt->value();
        BInterval = BInt->value();
        stop(0);
        stop(1);
        start(0);
        start(1);
        QMessageBox::information(this, "Changing intervals", "Intervals changed successfully.");
    }
}
void qorgOptions::ChangePassword() {
    if (CPassword->styleSheet() == "QLineEdit{background: white;}" && !CPassword->text().isEmpty() &&
            NPassword->styleSheet() == "QLineEdit{background: white;}" && !NPassword->text().isEmpty()) {
        QString* CA = new QString(QCryptographicHash::hash(salting(CPassword->text()).toUtf8(), QCryptographicHash::Sha3_512));
        QString* CB = new QString(calculateXOR(CPassword->text().toUtf8(), CA->toUtf8()).toBase64());
        QString* NA = new QString(QCryptographicHash::hash(salting(NPassword->text()).toUtf8(), QCryptographicHash::Sha3_512));
        QString* NB = new QString(calculateXOR(NPassword->text().toUtf8(), NA->toUtf8()).toBase64());
        CPassword->clear();
        NPassword->clear();
        CPassword->setStyleSheet("QLineEdit{background: white;}");
        NPassword->setStyleSheet("QLineEdit{background: white;}");
        emit CNPassword(CA, CB, NA, NB);
    }
}
void qorgOptions::Validator(QString Input) {
    QLineEdit* L = qobject_cast<QLineEdit*>(QObject::sender());
    if (Input.length() < 8 || Input.isEmpty()) {
        L->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        bool whitespaces = false;
        for (int i = 0; i < Input.length(); i++) {
            if (Input[i].isSpace()) {
                whitespaces = true;
                break;
            }
        }
        if (whitespaces) {
            L->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            L->setStyleSheet("QLineEdit{background: white;}");
        }
    }
}
void qorgOptions::DClicked(QModelIndex I) {
    if (QObject::sender() == Accepted) {
        if ((new CertAccept(SSLCertA[I.row()]))->exec() == QDialog::Rejected) {
            SSLCertB.push_back(SSLCertA[I.row()]);
            Blacklisted->addItem(SSLCertB.back().serialNumber());
            SSLCertA.erase(SSLCertA.begin()+I.row());
            delete Accepted->item(I.row());
        }
    } else {
        if ((new CertAccept(SSLCertB[I.row()]))->exec() == QDialog::Accepted) {
            SSLCertA.push_back(SSLCertB[I.row()]);
            Accepted->addItem(SSLCertA.back().serialNumber());
            SSLCertB.erase(SSLCertB.begin()+I.row());
            delete Blacklisted->item(I.row());
        }
    }
}
