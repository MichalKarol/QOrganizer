#ifndef QORGAB_H
#define QORGAB_H
#include <QtWidgets>
using namespace std;

struct Person
{
    QString Name;
    QString Surname;
    QString Town;
    QString Street;
    uint HouseNumber;
    uint Apartment;
    QString Email;
    QString Mobile;
    QDate Birthday;
};
class qorgAB : public QWidget
{
    Q_OBJECT
public:
    qorgAB(QWidget*);
    QString output();
    void input(QString);
    QList <QString> getBirthdays(QDate);
    QList <QString> getEmails();
private:
    vector <Person> Personv;
    QListWidgetItem *lastitem;
    uint lastIID;
    QGridLayout *La;
    QListWidget *List;
    QLabel *L[8];
    QLineEdit *E[8];
    QDateEdit *D;
    QPushButton *Add;
    QPushButton *OKB;
    QPushButton *Cancel;
private slots:
    void AddS();
    void row(QString);
    void Click(QModelIndex);
    void Edit(uint);
    void Delete(uint);
    void OK();
    void Can();
    void UpdateList();



};

#endif // QORGAB_H
