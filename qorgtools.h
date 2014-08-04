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
QByteArray calculateXOR(QByteArray A, QByteArray B);
QString Bit7ToBit8(QString);
QString QPEncode(QByteArray);
QByteArray QPDecode(QByteArray);
QString OutputTools(QString, QString);
QString OutputToolsS(QString, QString);
QString OutputTools(int, QString);
QString OutputTools(bool, QString);
QString InputS(QString, QString);
QString InputSS(QString, QString);
int InputI(QString, QString);
bool InputB(QString, QString);
void colorItem(QTreeWidgetItem*, char);
QString salting(QString);

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

#endif  // QORGTOOLS_H_
