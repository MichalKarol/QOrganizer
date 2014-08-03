#ifndef QORGCALENDAR_H
#define QORGCALENDAR_H
#include  <QtWidgets>
#include  "qorgtools.h"
#include  "qorgab.h"
using namespace std;
struct CalNor
{
    QString name;
    QString category;
    unsigned char priority;
    QDateTime datet;
    QDateTime edatet;
};
struct CalRec
{
    QString name;
    QString category;
    unsigned char priority;
    unsigned char type;
    QDateTime datet;
    QDateTime edatet;
    QDate edate;
};

class qorgCalendar: public QWidget
{
    Q_OBJECT
public:
    qorgCalendar(QWidget*,qorgAB*);
    QString output();
    void input(QString);
    QStringList getCategories();
    void setCategory(QString);
    QString getUpdate();
    vector <CalNor> Normal;
    vector <CalRec> Recurrent;
    QString category;
private:
    qorgAB *AB;
    //Calendar
    void setCalendar();
      void updateAll();
      void sort();
    QList <uint> checkEvN(QDate,char);
    QList <uint> checkEvR(QDate,char);
    QList <QString> checkBd(QDate);
    //Widget elements
    QDate currentDate;
    QGridLayout* Layout;

    QComboBox *Month;
    QPushButton *Yminus;
    QComboBox *Year;
    QPushButton *Yplus;
    QTableWidget *Calendar;
    QTreeWidget *DayView;
    QTreeWidget *Incoming;
    QList <uint> NotifiedN;
    QList <uint> NotifiedR;
    QTimer *NTimer;
private slots:

    void Add(QDate);
    void Edit(uint);
    void Delete(uint);
    void dayChanged(QTableWidgetItem*);
    void monthChanged(int);
    void yearChanged(QString IN="");
    void MidnightChange();
    void setNotification(bool first=false);
signals:
    void updateTree();
    void Notification(QString);
};

#endif // QORGCALENDAR_H
