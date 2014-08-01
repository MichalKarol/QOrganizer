#ifndef QORGPASSWD_H
#define QORGPASSWD_H
#include  <QtWidgets>
#include "qorgtools.h"
using namespace std;
class Password
{
public:
    Password();
    ~Password();
    QString Login;
    QString Passwd;
};

class Program
{
public:
    Program();
    ~Program();
    QString Name;
    vector  < Password* >  Passwordv;
};

class qorgPasswd : public QWidget
{
    Q_OBJECT
public:
    qorgPasswd(QWidget*);
    ~qorgPasswd();
    QString output();
    void input(QString);
private:
    vector  < Program >  Programv;
    QGridLayout *La;
    QTreeWidgetItem *lastitem;
    QPoint Last;
    QTreeWidget *Tree;
    QLabel *L[3];
    QLineEdit *ProgramL;
    QLineEdit *LoginL;
    QLineEdit *PasswordL;
    QCompleter *C;
    QPushButton *Change;
    QPushButton *Add;
    QPushButton *OKB;
    QPushButton *Cancel;
    void UpdateTree();
private slots:
    void change();
    void AddB();
    void row(QString);
    void clicked(QTreeWidgetItem*);
    void Edit(QPoint);
    void Delete(QPoint);
    void OK();
    void Can();

    
};

#endif // QORGPASSWD_H
