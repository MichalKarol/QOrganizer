#ifndef QORGTOOLS_H
#define QORGTOOLS_H
#include  <QtWidgets>
QByteArray calculateXOR(QByteArray A,QByteArray B);
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
void colorItem(QTreeWidgetItem*,short);
QString salting(QString);

class QItemPushButton :public QPushButton
{
    Q_OBJECT
public:
    QItemPushButton(QIcon icon,QWidget *parent,uint IID) :QPushButton(icon,"",parent)
    {
        ItemID=IID;
        setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(this,SIGNAL(clicked()),this,SLOT(Emits()));
    }
private:
    uint ItemID;
private slots:
    void Emits()
    {
        emit clicked(ItemID);
    }
signals:
    void clicked(uint);
};

#endif // QORGTOOLS_H
