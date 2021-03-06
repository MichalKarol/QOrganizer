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
#include <QNetworkProxy>
#include <vector>

bool VectorSearch(vector <QSslCertificate>* A, QSslCertificate B) {
    for (uint i = 0; i < A->size(); i++) {
        if ((*A)[i] == B) {
            return true;
        }
    }
    return false;
}

qorgOptions::qorgOptions(QWidget* parent) :QWidget(parent) {
    UpdateInterval = 30;
    BlockInterval = 30;
    UpdateQTimer = new QTimer(this);
    connect(UpdateQTimer, SIGNAL(timeout()), this, SLOT(UTimeout()));
    BlockQTimer = new QTimer(this);
    connect(BlockQTimer, SIGNAL(timeout()), this, SLOT(BTimeout()));
    currentWidget = -2;

    WidgetsQWidget[0] = new QWidget(this);
    CurrentPasswordQLabel = new QLabel("Current password", WidgetsQWidget[0]);
    CurrentPasswordQLineEdit = new QLineEdit(WidgetsQWidget[0]);
    CurrentPasswordQLineEdit->setEchoMode(QLineEdit::Password);
    connect(CurrentPasswordQLineEdit, SIGNAL(textChanged(QString)), this, SLOT(Validator(QString)));
    NewPasswordQLabel = new QLabel("New password", WidgetsQWidget[0]);
    NewPasswordQLineEdit = new QLineEdit(WidgetsQWidget[0]);
    NewPasswordQLineEdit->setEchoMode(QLineEdit::Password);
    connect(NewPasswordQLineEdit, SIGNAL(textChanged(QString)), this, SLOT(Validator(QString)));
    PasswordQPushButton = new QPushButton("Change password", WidgetsQWidget[0]);
    connect(PasswordQPushButton, SIGNAL(clicked()), this, SLOT(ChangePassword()));
    QGridLayout* W0QGridLayout = new QGridLayout(WidgetsQWidget[0]);
    W0QGridLayout->addWidget(CurrentPasswordQLabel, 0, 0);
    W0QGridLayout->addWidget(CurrentPasswordQLineEdit, 1, 0);
    W0QGridLayout->addWidget(NewPasswordQLabel, 2, 0);
    W0QGridLayout->addWidget(NewPasswordQLineEdit, 3, 0);
    W0QGridLayout->addWidget(PasswordQPushButton, 4, 0);
    BoxesQGroupBox[0] = new QGroupBox("Changing password", this);
    BoxesQGroupBox[0]->setLayout(W0QGridLayout);
    BoxesQGroupBox[0]->hide();

    WidgetsQWidget[1] = new QWidget(this);
    UpdateQLabel = new QLabel("Update interval", WidgetsQWidget[1]);
    BlockQLabel = new QLabel("Block interval", WidgetsQWidget[1]);
    UpdateIntervalQSpinBox = new QSpinBox(WidgetsQWidget[1]);
    BlockIntervalQSpinBox = new QSpinBox(WidgetsQWidget[1]);
    IntervalQPushButton = new QPushButton("Change intervals", WidgetsQWidget[1]);
    connect(IntervalQPushButton, SIGNAL(clicked()), this, SLOT(ChangeInterval()));
    QGridLayout* W1QGridLayout = new QGridLayout(WidgetsQWidget[1]);
    W1QGridLayout->addWidget(UpdateQLabel, 0, 0);
    W1QGridLayout->addWidget(BlockQLabel, 0, 1);
    W1QGridLayout->addWidget(UpdateIntervalQSpinBox, 1, 0);
    W1QGridLayout->addWidget(BlockIntervalQSpinBox, 1, 1);
    W1QGridLayout->addWidget(IntervalQPushButton, 2, 0, 1, 2);
    BoxesQGroupBox[1] = new QGroupBox("Changing intervals", this);
    BoxesQGroupBox[1]->setLayout(W1QGridLayout);
    BoxesQGroupBox[1]->hide();

    WidgetsQWidget[2] = new QWidget(this);
    iCalendarQLabel = new QLabel("Export calendar to iCalendar", WidgetsQWidget[2]);
    iCalendarQPushButton = new QPushButton("Export", WidgetsQWidget[2]);
    connect(iCalendarQPushButton, SIGNAL(clicked()), this, SLOT(ExportToVSth()));
    vNoteQLabel = new QLabel("Export notes to vNote", WidgetsQWidget[2]);
    vNoteQPushButton = new QPushButton("Export", WidgetsQWidget[2]);
    connect(vNoteQPushButton, SIGNAL(clicked()), this, SLOT(ExportToVSth()));
    vCardQLabel = new QLabel("Export address book to vCard", WidgetsQWidget[2]);
    vCardQPushButton = new QPushButton("Export", WidgetsQWidget[2]);
    connect(vCardQPushButton, SIGNAL(clicked()), this, SLOT(ExportToVSth()));
    QGridLayout* W2QGridLayout = new QGridLayout(WidgetsQWidget[2]);
    W2QGridLayout->addWidget(iCalendarQLabel, 0, 0);
    W2QGridLayout->addWidget(iCalendarQPushButton, 0, 1);
    W2QGridLayout->addWidget(vNoteQLabel, 1, 0);
    W2QGridLayout->addWidget(vNoteQPushButton, 1, 1);
    W2QGridLayout->addWidget(vCardQLabel, 2, 0);
    W2QGridLayout->addWidget(vCardQPushButton, 2, 1);
    BoxesQGroupBox[2] = new QGroupBox("Exporting", this);
    BoxesQGroupBox[2]->setLayout(W2QGridLayout);
    BoxesQGroupBox[2]->hide();

    WidgetsQWidget[3] = new QWidget(this);
    ProxyTypeQLabel = new QLabel("Proxy type", this);
    ProxyTypeQComboBox = new QComboBox(this);
    ProxyTypeQComboBox->addItem("None");
    ProxyTypeQComboBox->addItem("SOCKS5");
    ProxyTypeQComboBox->addItem("HTTP");
    ProxyTypeQComboBox->setCurrentIndex(0);
    connect(ProxyTypeQComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(proxyTypeChanged(int)));
    ProxyServerQLabel = new QLabel("SOCKS server", this);
    ProxyServerQLabel->setDisabled(true);
    ProxyServerQLineEdit = new QLineEdit(this);
    ProxyServerQLineEdit->setDisabled(true);
    ProxyPortQLabel = new QLabel("SOCKS port", this);
    ProxyPortQLabel->setDisabled(true);
    ProxyPortQSpinBox = new QSpinBox(this);
    ProxyPortQSpinBox->setRange(0, 65535);
    ProxyPortQSpinBox->setValue(1080);
    ProxyPortQSpinBox->setDisabled(true);
    ProxyUsernameQLabel = new QLabel("SOCKS username", this);
    ProxyUsernameQLabel->setDisabled(true);
    ProxyUsernameQLineEdit = new QLineEdit(this);
    ProxyUsernameQLineEdit->setDisabled(true);
    ProxyPasswordQLabel = new QLabel("SOCKS password", this);
    ProxyPasswordQLabel->setDisabled(true);
    ProxyPasswordQLineEdit = new QLineEdit(this);
    ProxyPasswordQLineEdit->setEchoMode(QLineEdit::Password);
    ProxyPasswordQLineEdit->setDisabled(true);
    ProxySetupQPushButton = new QPushButton("Setup", this);
    ProxySetupQPushButton->setDisabled(true);

    connect(ProxySetupQPushButton, SIGNAL(clicked()), this, SLOT(setupProxy()));
    QGridLayout* W3QGridLayout = new QGridLayout(WidgetsQWidget[3]);
    W3QGridLayout->addWidget(ProxyTypeQLabel, 0, 0);
    W3QGridLayout->addWidget(ProxyTypeQComboBox, 0, 1);
    W3QGridLayout->addWidget(ProxyServerQLabel, 1, 0);
    W3QGridLayout->addWidget(ProxyServerQLineEdit, 1, 1);
    W3QGridLayout->addWidget(ProxyPortQLabel, 2, 0);
    W3QGridLayout->addWidget(ProxyPortQSpinBox, 2, 1);
    W3QGridLayout->addWidget(ProxyUsernameQLabel, 3, 0);
    W3QGridLayout->addWidget(ProxyUsernameQLineEdit, 3, 1);
    W3QGridLayout->addWidget(ProxyPasswordQLabel, 4, 0);
    W3QGridLayout->addWidget(ProxyPasswordQLineEdit, 4, 1);
    W3QGridLayout->addWidget(ProxySetupQPushButton, 5, 0, 1, 2);
    BoxesQGroupBox[3] = new QGroupBox("Proxy", this);
    BoxesQGroupBox[3]->setLayout(W3QGridLayout);
    BoxesQGroupBox[3]->hide();

    WidgetsQWidget[4] = new QWidget(this);
    AcceptedQLabel = new QLabel("Accepted SSL Certificates.", WidgetsQWidget[3]);
    AcceptedQListWidget = new QListWidget(WidgetsQWidget[3]);
    connect(AcceptedQListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(CertificateClicked(QModelIndex)));
    BlacklistedQLabel = new QLabel("Blacklisted SSL Certificates.", WidgetsQWidget[3]);
    BlacklistedQListWidget = new QListWidget(WidgetsQWidget[3]);
    connect(BlacklistedQListWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(CertificateClicked(QModelIndex)));
    QGridLayout* W4QGridLayout = new QGridLayout(WidgetsQWidget[4]);
    W4QGridLayout->addWidget(AcceptedQLabel, 0, 0);
    W4QGridLayout->addWidget(BlacklistedQLabel, 0, 1);
    W4QGridLayout->addWidget(AcceptedQListWidget, 1, 0);
    W4QGridLayout->addWidget(BlacklistedQListWidget, 1, 1);
    WidgetsQWidget[4]->hide();

    LayoutQGridLayout = new QGridLayout(this);
}
int qorgOptions::checkCertificate(QSslCertificate I) {
    if (VectorSearch(&SSLCertA, I)) {
        return 1;
    } else if (VectorSearch(&SSLCertB, I)) {
        return -1;
    }
    return 0;
}
void qorgOptions::setPointers(qorgCalendar* C, qorgNotes* N, qorgAB* A) {
    this->Calendar = C;
    this->Notes = N;
    this->AddressBook = A;
}

void qorgOptions::setWidget(int W) {
    if (W != currentWidget || (W == 0 && !SSLCertTmp.isEmpty())) {
        if (W == -1) {
            LayoutQGridLayout->removeWidget(WidgetsQWidget[4]);
            WidgetsQWidget[4]->hide();
            LayoutQGridLayout->addWidget(BoxesQGroupBox[0], 0, 0);
            BoxesQGroupBox[0]->show();
            LayoutQGridLayout->addWidget(BoxesQGroupBox[1], 1, 0);
            BoxesQGroupBox[1]->show();
            LayoutQGridLayout->addWidget(BoxesQGroupBox[2], 2, 0);
            BoxesQGroupBox[2]->show();
            LayoutQGridLayout->addWidget(BoxesQGroupBox[3], 3, 0);
            BoxesQGroupBox[3]->show();
            currentWidget = -1;
        } else {
            LayoutQGridLayout->removeWidget(BoxesQGroupBox[0]);
            BoxesQGroupBox[0]->hide();
            LayoutQGridLayout->removeWidget(BoxesQGroupBox[1]);
            BoxesQGroupBox[1]->hide();
            LayoutQGridLayout->removeWidget(BoxesQGroupBox[2]);
            BoxesQGroupBox[2]->hide();
            LayoutQGridLayout->removeWidget(BoxesQGroupBox[3]);
            BoxesQGroupBox[3]->hide();
            LayoutQGridLayout->addWidget(WidgetsQWidget[4], 0, 0);
            WidgetsQWidget[4]->show();
            currentWidget = 0;
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
    Out.append(Output(UpdateInterval)+" ");
    Out.append(Output(BlockInterval)+" \n");
    for (uint i = 0; i < SSLCertA.size(); i++) {
        Out.append(Output(QString(SSLCertA[i].toPem()))+" \n");
    }
    for (uint i = 0; i < SSLCertB.size(); i++) {
        Out.append(Output(QString(SSLCertB[i].toPem()))+" B B \n");
    }
    if (!QNetworkProxy::applicationProxy().hostName().isEmpty()) {
        QNetworkProxy tmp = QNetworkProxy::applicationProxy();
        Out.append(Output(tmp.type() == QNetworkProxy::Socks5Proxy)+" "+Output(tmp.hostName())+" "+Output(static_cast<uint>(tmp.port()))
                   +" "+Output(tmp.user())+" "+Output(tmp.password())+" \n");
    }
    Out.append("\n\n");
    return Out;
}
void qorgOptions::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 1: {
                SSLCertA.push_back(QSslCertificate::fromData(InputS(B[0]).toUtf8()).first());
                AcceptedQListWidget->addItem(SSLCertA.back().serialNumber());
            }break;
            case 2: {
                UpdateInterval = InputI(B[0]);
                BlockInterval = InputI(B[1]);
            }break;
            case 3: {
                SSLCertB.push_back(QSslCertificate::fromData(InputS(B[0]).toUtf8()).first());
                BlacklistedQListWidget->addItem(SSLCertB.back().serialNumber());
            }break;
            case 5: {
                QNetworkProxy proxy;
                if (InputB(B[0])) {
                    proxy.setType(QNetworkProxy::Socks5Proxy);
                    ProxyTypeQComboBox->setCurrentIndex(1);
                } else {
                    proxy.setType(QNetworkProxy::HttpProxy);
                    ProxyTypeQComboBox->setCurrentIndex(2);
                }
                proxy.setHostName(InputS(B[1]));
                ProxyServerQLineEdit->setText(proxy.hostName());
                proxy.setPort(InputI(B[2]));
                ProxyPortQSpinBox->setValue(proxy.port());
                proxy.setUser(InputS(B[3]));
                ProxyUsernameQLineEdit->setText(proxy.user());
                proxy.setPassword(InputS(B[4]));
                QNetworkProxy::setApplicationProxy(proxy);
            }break;
            }
        }
    }
    UpdateIntervalQSpinBox->setValue(UpdateInterval);
    BlockIntervalQSpinBox->setValue(BlockInterval);
}
void qorgOptions::acceptSSLCert(QSslCertificate C) {
    if (!VectorSearch(&SSLCertA, C)) {
        SSLCertA.push_back(C);
        AcceptedQListWidget->addItem(C.serialNumber());
    }
}
void qorgOptions::blacklistSSLCert(QSslCertificate C) {
    if (!VectorSearch(&SSLCertB, C)) {
        SSLCertB.push_back(C);
        BlacklistedQListWidget->addItem(C.serialNumber());
    }
}
void qorgOptions::addForVerification(QSslCertificate C) {
    SSLCertTmp.append(C);
}
void qorgOptions::start(bool I) {
    if (!I) {
        UpdateQTimer->setInterval(UpdateInterval*60*1000);
        UpdateQTimer->start();
    } else {
        BlockQTimer->setInterval(BlockInterval*60*1000);
        BlockQTimer->start();
    }
}
void qorgOptions::stop(bool I) {
    if (!I) {
        UpdateQTimer->stop();
    } else {
        BlockQTimer->stop();
    }
}
void qorgOptions::UTimeout() {
    emit Update();
}
void qorgOptions::BTimeout() {
    emit Block();
}
void qorgOptions::ChangeInterval() {
    if (UpdateIntervalQSpinBox->value() != 0
            && BlockIntervalQSpinBox->value() != 0) {
        UpdateInterval = UpdateIntervalQSpinBox->value();
        BlockInterval = BlockIntervalQSpinBox->value();
        stop(0);
        stop(1);
        start(0);
        start(1);
        QMessageBox::information(this, "Changing intervals", "Intervals changed successfully.");
    }
}
void qorgOptions::ChangePassword() {
    if (CurrentPasswordQLineEdit->styleSheet() == "QLineEdit{background: white;}"
            && !CurrentPasswordQLineEdit->text().isEmpty()
            && NewPasswordQLineEdit->styleSheet() == "QLineEdit{background: white;}"
            && !NewPasswordQLineEdit->text().isEmpty()) {
        QByteArray CA = RandomQByteArray();
        QByteArray CB = calculateXOR(QCryptographicHash::hash(
                                         QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), CurrentPasswordQLineEdit->text().toUtf8()).toByteArray(), QCryptographicHash::Sha3_512)
                                         +CurrentPasswordQLineEdit->text().toUtf8()
                                         +QCryptographicHash::hash(CurrentPasswordQLineEdit->text().toUtf8(), QCryptographicHash::Sha3_512)
                                         , QCryptographicHash::Sha3_256), CA);
        QByteArray NA = RandomQByteArray();
        QByteArray NB = calculateXOR(QCryptographicHash::hash(
                                         QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), NewPasswordQLineEdit->text().toUtf8()).toByteArray(), QCryptographicHash::Sha3_512)
                                         +NewPasswordQLineEdit->text().toUtf8()
                                         +QCryptographicHash::hash(NewPasswordQLineEdit->text().toUtf8(), QCryptographicHash::Sha3_512)
                                         , QCryptographicHash::Sha3_256), NA);
        CurrentPasswordQLineEdit->clear();
        NewPasswordQLineEdit->clear();
        CurrentPasswordQLineEdit->setStyleSheet("QLineEdit{background: white;}");
        NewPasswordQLineEdit->setStyleSheet("QLineEdit{background: white;}");
        emit CNPassword(CA, CB, NA, NB);
    }
}
void qorgOptions::ExportToVSth() {
    QPushButton* ButtonQPushButton = qobject_cast<QPushButton*>(QObject::sender());

    QString Out;
    QString Extension;
    QString Suffix;
    if (ButtonQPushButton == iCalendarQPushButton) {
        Out = Calendar->exportToICalendar();
        Extension = "vCalendar (*.ics)";
        Suffix = ".ics";
    } else if (ButtonQPushButton == vNoteQPushButton) {
        Out = Notes->exportToVNote();
        Extension = "vNote (*.vnt)";
        Suffix = ".vnt";
    } else {
        Out = AddressBook->exportToVCard();
        Extension = "VCard (*.vcf)";
        Suffix = ".vcf";
    }
    QString path = QFileDialog::getSaveFileName(this, "Export to ...", QDir::homePath(), Extension);
    if (!path.isEmpty()) {
        if (!path.endsWith(Suffix)) {
            path = path + Suffix;
        }
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            stream << Out;
            file.close();
            Out.clear();
        } else {
            QMessageBox::critical(this, "Error", "Cannot save file due to"+file.errorString());
        }
    }
}
void qorgOptions::proxyTypeChanged(int i) {
    if (i != 0) {
        ProxyServerQLabel->setDisabled(false);
        ProxyServerQLineEdit->setDisabled(false);
        ProxyPortQLabel->setDisabled(false);
        ProxyPortQSpinBox->setDisabled(false);
        ProxyUsernameQLabel->setDisabled(false);
        ProxyUsernameQLineEdit->setDisabled(false);
        ProxyPasswordQLabel->setDisabled(false);
        ProxyPasswordQLineEdit->setDisabled(false);
        ProxySetupQPushButton->setDisabled(false);
        if (i == 1) {
            ProxyServerQLabel->setText("SOCKS server");
            ProxyPortQLabel->setText("SOCKS port");
            ProxyPortQSpinBox->setValue(1080);
            ProxyUsernameQLabel->setText("SOCKS username");
            ProxyPasswordQLabel->setText("SOCKS password");
        } else {
            ProxyServerQLabel->setText("HTTP server");
            ProxyPortQLabel->setText("HTTP port");
            ProxyPortQSpinBox->setValue(80);
            ProxyUsernameQLabel->setText("HTTP username");
            ProxyPasswordQLabel->setText("HTTP password");
        }
    } else {
        ProxyServerQLineEdit->clear();
        ProxyPortQSpinBox->setValue(0);
        ProxyUsernameQLineEdit->clear();
        ProxyPasswordQLineEdit->clear();
        ProxyServerQLabel->setDisabled(true);
        ProxyServerQLineEdit->setDisabled(true);
        ProxyPortQLabel->setDisabled(true);
        ProxyPortQSpinBox->setDisabled(true);
        ProxyUsernameQLabel->setDisabled(true);
        ProxyUsernameQLineEdit->setDisabled(true);
        ProxyPasswordQLabel->setDisabled(true);
        ProxyPasswordQLineEdit->setDisabled(true);
        ProxySetupQPushButton->setDisabled(true);
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));
    }
}

void qorgOptions::setupProxy() {
    if (QUrl::fromUserInput(ProxyServerQLineEdit->text()).isValid()) {
        QNetworkProxy Proxy;
        if (ProxyTypeQComboBox->currentIndex() == 1) {
            Proxy.setType(QNetworkProxy::Socks5Proxy);
        } else {
            Proxy.setType(QNetworkProxy::HttpProxy);
        }
        Proxy.setHostName(ProxyServerQLineEdit->text());
        Proxy.setPort(ProxyPortQSpinBox->value());
        if (!ProxyUsernameQLineEdit->text().isEmpty()) {
            Proxy.setUser(ProxyUsernameQLineEdit->text());
        }
        if (!ProxyPasswordQLineEdit->text().isEmpty()) {
            Proxy.setPassword(ProxyPasswordQLineEdit->text());
            ProxyPasswordQLineEdit->clear();
        }
        QNetworkProxy::setApplicationProxy(Proxy);
    } else {
        ProxyServerQLineEdit->clear();
        ProxyServerQLineEdit->setPlaceholderText("Url is NOT valid. Enter valid Url.");
    }
}

void qorgOptions::Validator(QString Input) {
    QLineEdit* L = qobject_cast<QLineEdit*>(QObject::sender());
    if (Input.length() < 8
            || Input.isEmpty()) {
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
void qorgOptions::CertificateClicked(QModelIndex I) {
    if (QObject::sender() == AcceptedQListWidget) {
        if ((new CertAccept(SSLCertA[I.row()]))->exec() == QDialog::Rejected) {
            SSLCertB.push_back(SSLCertA[I.row()]);
            BlacklistedQListWidget->addItem(SSLCertB.back().serialNumber());
            SSLCertA.erase(SSLCertA.begin()+I.row());
            delete AcceptedQListWidget->item(I.row());
        }
    } else {
        if ((new CertAccept(SSLCertB[I.row()]))->exec() == QDialog::Accepted) {
            SSLCertA.push_back(SSLCertB[I.row()]);
            AcceptedQListWidget->addItem(SSLCertA.back().serialNumber());
            SSLCertB.erase(SSLCertB.begin()+I.row());
            delete BlacklistedQListWidget->item(I.row());
        }
    }
}
