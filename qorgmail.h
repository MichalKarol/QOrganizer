#ifndef QORGMAIL_H
#define QORGMAIL_H
#include  <QtWidgets>
#include  <QWebView>
#include  <QSslSocket>
#include "qorgtools.h"
#include  "qorgab.h"
using namespace std;

class Structure
{
public:
    Structure();
    ~Structure();
    struct Attrybutes
    {
        QString Charset;
        QString Name;
    };
    QString Structure_Number;
    QString Structure_Type;
    QString Structure_Subtype;
    Attrybutes Structure_Attrybutes;
    QString Structure_CID;
    QString Structure_Descryption;
    QString Structure_Encoding;
    uint Structure_Size;
    uint Structure_Lines;
    QString Structure_MD5;
    QString Structure_Disposition;
    QString Structure_Language;
    QString Structure_Location;
};
class Email
{
public:
    Email();
    ~Email();
    enum Flags
    {
        Answered=1,
        Flagged=2,
        Draft=4,
        Deleted=8,
        Seen=16,
    };
    struct EUser
    {
        QString Name;
        QString EMailA;
    };
    QString Email_Subject;
    EUser Email_From;
    QDateTime Email_Date;
    QString Email_Body[2];
    uint Email_UID;
    char Email_Flags;
    vector  < Structure* > Structurev;
};
class Mailbox
{
public:
    Mailbox();
    ~Mailbox();
    enum Att
    {
        HasChildren=1,
        HasNoChildren=2,
        Noselect=4,
        Trash=8,
        Flagged=16,
        Important=32,
        Drafts=64,
        All=128,
        Sent=256,
        INBOX=512
    };
    QString Mbox_Name;
    QString Mbox_Showname;
    unsigned char Mbox_Attrybutes;
    vector  < Mailbox* >  Mbox_Children;
    vector  < Email* >  Emailv;
    bool Mbox_Refresh;
    bool Mbox_Top;
};
class Mail
{
public:
    Mail();
    ~Mail();
    QString Name;
    QString IMAPserver;
    QString SMTPserver;
    QString User;
    QString Password;
    vector  < Mailbox* >  Mboxv;
};

class qorgMail: public QWidget
{
    Q_OBJECT
public:
    qorgMail(QWidget*,qorgAB*);
    ~qorgMail();
    QString output();
    void input(QString);
    void setMail(QString);
    QStringList getCategories();
    QString getCurrent()
    {
     return MailCat;
    }
    void getUpdate();
private:
    qorgAB *AB;
    vector  < Mail >  Mailv;
    QString MailCat;
    QGridLayout *Layout;
    void setMailbox(int);

    uint currentMail;
    uint currentMailbox;
    int currentEmail;
    void setLayoutF();
    void addChildren(Mailbox* I,QTreeWidgetItem* W);
    QTreeWidget *Mailboxes;
    QTreeWidget *MailView;
    QSplitter *Split;
    QWebView *ReadMail;
    int Quene;
    uint UpdateQuene;
    QListWidget *AttachmentList;
    QPushButton *Refresh;
    QPushButton *Send;
    QPushButton *Delete;
    QPushButton *Forward;
    QPushButton *Reply;
    QList  < QWidget* >  F;

    void setLayoutC();
    QTreeWidget *List;
    QLabel *Labels[5];
    QLineEdit *Username;
    QLineEdit *Passwd;
    QLineEdit *IMAPS;
    QLineEdit *SMTPS;
    QComboBox *Choose;
    QPushButton *AddB;
    QList  <QWidget*>  C;
private slots:
    //CAT
    void testInput();
    void row(QString);
    void change(int);
    void Click(QModelIndex);
    void EditMail(uint);
    void EditMailS(bool);
    void DeleteMail(uint);

    //MAIN
    void chooseMbox(QTreeWidgetItem*);
    void chooseEmail(QModelIndex);
    void LoginS(bool);
    void MailboxesS(bool);
    void EmailS(bool);
    void downloadAttachment(QModelIndex);
    void downloadAttachment(uint,QString);
    void AttachmentS(bool);
    void RefreshS();
    void RefreshS(bool);
    void SendMail();
    void SendEmailS(bool);
    void DeleteEmail();
    void DeleteEmailS(bool);
    void UpdateEmail(bool);
    void UpdateS();
    void HTTPSS(QNetworkReply*,QList <QSslError>);
signals:
    void updateTree();
    void doubleClick(QString);
    void sendUpdate(QString);
};

#endif // QORGMAIL_H
