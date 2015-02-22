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

#ifndef QORGMAIL_H_
#define QORGMAIL_H_
#include <qorgtools.h>
#include <qorgab.h>
#include <qorgoptions.h>
#include <QtWidgets>
#include <QWebView>
#include <QWebFrame>
#include <QNetworkReply>
#include <QtConcurrent>
#include <vector>

using std::vector;
class Structure {
public:
    Structure();
    struct Attrybutes    {
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
class Email {
public:
    Email();
    enum Flags {
        Seen = 1,
        Answered = 2,
        Flagged = 4,
        Deleted = 8,
        Draft = 16,
        Recent = 32,
    };
    QString Email_Subject;
    QString Email_From;
    QDateTime Email_Date;
    QString Email_Body[2];
    uint Email_UID;
    uchar Email_Flags;
    QString Email_MessageID;
    QString Email_ReplyTo;
    vector <QString> Email_RecipientsTo;
    vector <QString> Email_RecipientsCC;
    vector  <Structure*> Structurev;
    // Created during loading
    uint Email_Quene;

};
class Mailbox {
public:
    Mailbox();
    enum Att    {
        HasChildren = 1,
        HasNoChildren = 2,
        Noselect = 4,
        Trash = 8,
        Flagged = 16,
        Important = 32,
        Drafts = 64,
        All = 128,
        Sent = 256,
        INBOX = 512
    };
    QString Mbox_Name;
    QString Mbox_Showname;
    uint Mbox_Attrybutes;
    vector  <Mailbox*>  Mbox_Children;
    vector  <Email*>  Emailv;
    bool Mbox_Refresh;
    bool Mbox_Top;
};
class Mail {
public:
    Mail();
    QString Name;
    QString IMAPserver;
    QString SMTPserver;
    QString User;
    QString Password;
    vector  <Mailbox*>  Mboxv;
};

class qorgMail: public QWidget {
    Q_OBJECT
public:
    explicit qorgMail(QWidget*);
    ~qorgMail();
    void setPointers(qorgAB*, qorgOptions*);
    QString output();
    void input(QString);
    void setMail(int);
    QStringList getCategories();
    int getCurrent()    {
        return currentMail;
    }
    uint threadNumber();
    bool SSLSocketError(QList<QSslError>);
    void getUpdate();
private:
    vector <Mail> Mailv;
    QGridLayout* Layout;
    void setMailbox(int);
    qorgAB* AddressBook;
    qorgOptions* Options;

    int currentMail;
    uint currentMailbox;
    int currentEmail;
    bool mailboxAction;
    bool emailAction;

    void setLayoutF();
    void addChildren(Mailbox*, QTreeWidgetItem*);
    QTreeWidget* Mailboxes;
    QTreeWidget* MailView;
    QSplitter* Split;
    QSplitter* InternalSplitter;
    QTabWidget* Tabs;
    QTextBrowser* MailText;
    QWebView* MailHtml;
    uint UpdateQuene;
    QListWidget* AttachmentList;
    QPushButton* Refresh;
    QPushButton* Send;
    QPushButton* Delete;
    QPushButton* Reply;
    QPushButton* ReplyAll;
    QPushButton* Forward;
    QList  <QWidget*>  F;

    void setLayoutC();
    QTreeWidget* List;
    QLabel* Labels[5];
    QLineEdit* Username;
    QLineEdit* Passwd;
    QLineEdit* IMAPS;
    QLineEdit* SMTPS;
    QComboBox* Choose;
    QPushButton* AddB;
    QList  <QWidget*>  C;
private slots:
    // Categories
    void testInput();
    void row(QString);
    void change(int);
    void Click(QModelIndex);
    void DeleteMail(uint);
    void EditMail(uint);
    void EditMailS(bool, QString);
    void EditMailSS(bool, QString);

    // Main
    // SSLCON signals slots
    void LoginS(bool, QString);
    void MailboxesS(bool, QString);
    void RefreshS(bool, QString);
    void RefreshSS(bool, QString);
    void EmailS(bool, QString);
    void UpdateS(bool, QString);

    void AttachmentS(bool, QString);
    void AttachmentSUser(bool, QString);

    void CopyES(bool, QString);
    void DeleteES(bool, QString);
    void MoveES(bool, QString);

    void CreateMS(bool, QString);
    void DeleteMS(bool, QString);
    void RenameMS(bool, QString);

    // Other slots
    void chooseMbox(QTreeWidgetItem*);
    void chooseEmail(QModelIndex);

    void downloadAttachment(QModelIndex);
    void downloadAttachment(uint, QString);
    void RefreshS();
    void SendEmail();
    void DeleteEmail();

    void linkClicked(QUrl);
    void HTTPSS(QNetworkReply*, QList <QSslError>);
    void sortMail();
    // Menus and actions
    void MailViewMenu(QPoint);
    void MailboxesMenu(QPoint);
signals:
    void updateTree();
    void doubleClick(QString);
    void sendUpdate(QString);
};

#endif  // QORGMAIL_H_
