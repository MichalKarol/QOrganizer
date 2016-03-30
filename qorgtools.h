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

#ifndef QORGTOOLS_H_
#define QORGTOOLS_H_
#include <QPair>
#include <QDateTime>
#include <QLayout>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QLabel>
#include <QDialog>
#include <QTextCodec>
#include <QGuiApplication>
#include <QTextDocument>
#include <QUuid>
#include <QTextBrowser>
#include <QDebug>
#include <QResizeEvent>
#include <QSslCertificate>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

// Input filters
QString inputString(QString);
uint inputUInt(QString);
uchar inputUchar(QString);
bool inputBool(QString);
QDateTime inputDateTime(QString);
QDate inputDate(QString);

// Output filters
QString outputFilter(QString);
QString outputFilter(uint);
QString outputFilter(uchar);
QString outputFilter(bool);
QString outputFilter(QDateTime);
QString outputFilter(QDate);

// Encryption
QByteArray encryptAesEcb(QByteArray& toEncrypt, QByteArray& password);
QByteArray encryptAesCbc(QByteArray& iv, QByteArray& toEncrypt, QByteArray& password);
QByteArray encryptAesGcm(QByteArray& iv, QByteArray& aad, QByteArray& toEncrypt, QByteArray& password);

// Decryption
QByteArray decryptAesEcb(QByteArray& toDecrypt, QByteArray& password);
QByteArray decryptAesCbc(QByteArray& iv, QByteArray& toDecrypt, QByteArray& password);
QByteArray decryptAesGcm(QByteArray& iv, QByteArray& aad, QByteArray& toDecrypt, QByteArray& password);
QByteArray calculateXOR(QByteArray& data, QByteArray& key);

// Other TODO(mkarol) Check if all of them is useful
QString bit7ToBit8(QString);
QString bit8ToBit7(QString);
QString quotedPrintableEncode(QByteArray);
QByteArray quotedPrintableDecode(QByteArray);
void changeLayoutWidgets(QLayout* layout);
void colorItem(QTreeWidgetItem*, char);
QString nameFilter(QString);
QString htmlCleaner(QString);
QList <QString> split(QString);
QPair <QByteArray, QByteArray> createNewPair(QByteArray& data);
QByteArray encryptUsingAES(QByteArray IV, QByteArray data, QByteArray password);
QByteArray decryptUsingAES(QByteArray IV, QByteArray data, QByteArray password);

class ItemPushButton : public QPushButton {
    Q_OBJECT
  public:
    ItemPushButton(QIcon icon, QWidget* parent, uint IID);
  private:
    uint itemID;
  private slots:
    void Emits();
  signals:
    void clicked(uint);
};

class ValidatingLineEdit : public QLineEdit {
    Q_OBJECT
  public:
    ValidatingLineEdit(QString regexp, QString reason, QWidget* parent = NULL, bool fading = false);
    void activate(QString reason, bool fading);

  private:
    bool fading;
    int height;
    QPixmap pixmap;
    QTimer timer;
    QString reason;

    QLabel* icon;
    QRegExpValidator* validator;

    void resizeEvent(QResizeEvent* event);
  public slots:
  private slots:
    void validateText();
    void deactivate();

  signals:

};

class CertAccept : public QDialog { // TODO(mkarol) Make it prettier. QGroupBox and QLabels ...
    Q_OBJECT
  public:
    explicit CertAccept(QSslCertificate cert);
};

#endif  // QORGTOOLS_H_
