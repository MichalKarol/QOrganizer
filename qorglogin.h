#ifndef QORGLOGIN_H
#define QORGLOGIN_H
#include "qorganizer.h"
#include "qorgtools.h"
#include "qorgio.h"
//#include  <QObject>
class qorgLogin :public QDialog
{
    Q_OBJECT
public:
    qorgLogin(QOrganizer*);
    void clear();
private:
    QOrganizer *pointer;
    bool Validated[2];
    QLineEdit *Line[2];
    QLabel *Label[2];
    QLabel *PassLabel;
    QPushButton *Login;
    QPushButton *Reg;
private slots:
    void UserInputValidation(QString);
    void PasswordInputValidation(QString);
    void Authentication();
    void Register();
protected:
    QString *hashed;
    QString *hash;
};

#endif // QORGLOGIN_H
