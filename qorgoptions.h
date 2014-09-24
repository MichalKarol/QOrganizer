#ifndef QORGOPTIONS_H
#define QORGOPTIONS_H

#include <qorganizer.h>
#include <QtWidgets>
#include <algorithm>

using std::vector;
using std::find;
class qorgOptions : public QWidget
{
    Q_OBJECT
public:
    explicit qorgOptions(QOrganizer *parent);
    int checkCertificate(QSslCertificate);
    void setWidget(uint);
    QString output();
    void input(QString);
    QDialog *SSLCertYOR(QSslCertificate);
private:
    vector <QSslCertificate> SSLCertA;
    vector <QSslCertificate> SSLCertB;
    uint UInterval;
    uint BInterval;
    QTimer *UTimer;
    QTimer *BTimer;
    uint currentW;
    QListWidget *Accepted;
    QListWidget *Blacklisted;
signals:
    void ChangePassword(QString*,QString*);
    void Update();
    void Block();
public slots:
    void startUT();
    void startBT();
    void acceptSSLCert(QSslCertificate);
    void blockSSLCert(QSslCertificate);
private slots:

};

#endif // QORGOPTIONS_H
