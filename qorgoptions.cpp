#include "qorgoptions.h"

qorgOptions::qorgOptions(QOrganizer *parent) :QWidget(parent)
{
    Accepted = new QListWidget(this);
    Blacklisted = new QListWidget(this);
    for (uint i = 0;i < SSLCertA.size(); i++) {
        Accepted->addItem(SSLCertA[i].issuerInfo(QSslCertificate::CommonName));
    }
    for (uint i = 0;i < SSLCertB.size(); i++) {
        Blacklisted->addItem(SSLCertB[i].issuerInfo(QSslCertificate::CommonName));
    }
}
int qorgOptions::checkCertificate(QSslCertificate I) {
    if (find(SSLCertA.begin(), SSLCertA.end(), I) != SSLCertA.end()) {
        return 1;
    } else if (find(SSLCertB.begin(), SSLCertB.end(), I) != SSLCertB.end()) {
        return -1;
    } else {
        return 0;
    }
}
void qorgOptions::setWidget(uint W) {
    if (W != currentW) {
        if (W == 0) {
            //Account
        } else {
           //SSL MANAGER
        }
    }
}
QString qorgOptions::output() {
    QString Out;
    Out.append(Output(UInterval)+" ");
    Out.append(Output(BInterval)+" \n");
    for (uint i = 0;i < SSLCertA.size(); i++) {
        Out.append(SSLCertA[i].toPem())+" \n";
    }
    for (uint i = 0;i < SSLCertB.size(); i++) {
        Out.append(SSLCertB[i].toPem())+" B B \n";
    }
    Out.append("\n\n");
    return Out;
}
void qorgOptions::input(QString Input) {
    QStringList A = Input.split("\n");
    for (int i = 0; i < A.size(); i++) {
        QStringList B = A[i].split(" ");
        switch (B.size()-1) {
        case 2: {
            SSLCertA.push_back(QSslCertificate::fromData(B[0]));
        }break;
        case 3: {
            UInterval = InputI(B[0]);
            BInterval = InputI(B[1]);
        }break;
        case 4: {
            SSLCertB.push_back(QSslCertificate::fromData(B[0]));
        }break;
        }
    }
}
