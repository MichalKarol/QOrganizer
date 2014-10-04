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

#ifndef QORGTOOLS_H_
#define QORGTOOLS_H_
#include <QtWidgets>
#include <QSslCertificate>

QByteArray calculateXOR(QByteArray A, QByteArray B);
QString Bit7ToBit8(QString);
QString QPEncode(QByteArray);
QByteArray QPDecode(QByteArray);
QString Output(QString);
QString Output(uint);
QString Output(uchar);
QString Output(bool);
QString Output(QDateTime);
QString Output(QDate);

QString InputS(QString);
uint InputI(QString);
uchar InputC(QString);
bool InputB(QString);
QDateTime InputDT(QString);
QDate InputD(QString);

void colorItem(QTreeWidgetItem*, char);
QString salting(QString);
QString NameFilter(QString);

class QItemPushButton :public QPushButton {
    Q_OBJECT
public:
    QItemPushButton(QIcon icon, QWidget *parent, uint IID) :QPushButton(icon, "", parent)    {
        ItemID = IID;
        setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(this, SIGNAL(clicked()), this, SLOT(Emits()));
    }
private:
    uint ItemID;
private slots:
    void Emits()    {
        emit clicked(ItemID);
    }
signals:
    void clicked(uint);
};

class CertAccept :public QDialog {
public:
    explicit CertAccept(QSslCertificate cert) {
        setWindowTitle("SSL certificate error.");
        setWindowIcon(QIcon(":/main/QOrganizer.png"));
        QLabel *La = new QLabel("Do you trust this certificate?", this);
        QTextBrowser* Text = new QTextBrowser(this);
        Text->setText(cert.toText());
        QPushButton* No = new QPushButton(this);
        No->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
        connect(No, SIGNAL(clicked()), this, SLOT(reject()));
        QPushButton* Yes = new QPushButton(this);
        Yes->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        connect(Yes, SIGNAL(clicked()), this, SLOT(accept()));
        QGridLayout *L = new QGridLayout(this);
        L->addWidget(La, 0, 0, 1, 2);
        L->addWidget(Text, 1, 0, 1, 2);
        L->addWidget(No, 2, 0);
        L->addWidget(Yes, 2, 1);
    }
};

#endif  // QORGTOOLS_H_
