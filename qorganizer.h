#ifndef QORGANIZER_H
#define QORGANIZER_H
#include  <QtWidgets>
#include "qorgcalendar.h"
#include "qorgmail.h"
#include "qorgnotes.h"
#include "qorgab.h"
#include "qorgrss.h"
#include "qorgpasswd.h"
#include "qorgtools.h"
class QOrganizer : public QWidget
{
    Q_OBJECT
public:
    QOrganizer();
    ~QOrganizer();
    qorgCalendar *Calendar;
    qorgMail *Mail;
    qorgNotes *Notes;
    qorgAB *AdressBook;
    qorgRSS *RSS;
    qorgPasswd *PasswordManager;
    void setUser(QString,QString*,QString*);
    uint UInterval;
    uint BInterval;
private:
    QTimer *UTimer;
    QSystemTrayIcon *Tray;
    QTreeWidget *TreeWidget;
    QStackedWidget *Stacked;
    QString Updates[3];
    void setTree();
    void closeEvent(QCloseEvent *);
    bool closing;
    //Settings
    QLineEdit *CP;
    QLineEdit *NP;
    QPushButton *Change;
    QSpinBox *UpdateInterval;
    QSpinBox *BlockInterval;
    QPushButton *ChangeInterval;
    QTimer *BTimer;
    QLineEdit *BlockL;
    QPushButton *OKB;
    bool shown;
protected:
    QString user;
    QString *hash;
    QString *hashed;
private slots:
    void launchFunction(QTreeWidgetItem*);
    void updateCalendar();
    void Notification(QString);
    void updateMail();
    void updateRSS();
    void doubleClick(QString);
    void TrayClick(QSystemTrayIcon::ActivationReason);
    void MailNews(QString);
    void RSSNews(QString);
    void updateTime();
    void Block();

    //Settings
    void Validator(QString);
    void NewPassword();
    void SetInterval();

    //Block
    void Unlock();
};
#endif //QORGANIZER_H
