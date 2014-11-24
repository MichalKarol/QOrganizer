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

#include <qorgmail.h>
#include <algorithm>
#include <vector>

uint MAILthreadCounter = 0;
QMutex MAILaccessToCounter;

Structure::Structure() {
    this->Structure_Number.clear();
    this->Structure_Type.clear();
    this->Structure_Subtype.clear();
    this->Structure_Attrybutes.Charset.clear();
    this->Structure_Attrybutes.Name.clear();
    this->Structure_CID.clear();
    this->Structure_Descryption.clear();
    this->Structure_Encoding.clear();
    this->Structure_Size = 0;
    this->Structure_Lines = 0;
    this->Structure_MD5.clear();
    this->Structure_Disposition.clear();
    this->Structure_Language.clear();
    this->Structure_Location.clear();
}
Email::Email() {
    this->Email_Subject.clear();
    this->Email_From.clear();
    this->Email_Date.setMSecsSinceEpoch(0);
    this->Email_Body[0].clear();
    this->Email_Body[1].clear();
    this->Email_UID = 0;
    this->Email_Flags = 0;
    this->Email_MessageID.clear();
    this->Email_ReplyTo.clear();
    this->Structurev.clear();
    this->Email_Quene = 0;
}
Mailbox::Mailbox() {
    this->Mbox_Name.clear();
    this->Mbox_Showname.clear();
    this->Mbox_Attrybutes = 0;
    this->Mbox_Children.clear();
    this->Emailv.clear();
    this->Mbox_Refresh = false;
    this->Mbox_Top = false;
}
Mail::Mail() {
    this->Name.clear();
    this->IMAPserver.clear();
    this->SMTPserver.clear();
    this->User.clear();
    this->Password.clear();
    this->Mboxv.clear();
}

class SSLCON : public QThread {
    Q_OBJECT
public:
    enum Method {
        Login,
        Mailboxes,
        Emails,

        Attachment,
        Send,

        CopyE,
        DeleteE,
        MoveE,

        CreateM,
        DeleteM,
        RenameM,

        Sleep,
        Stop,
    };
    explicit SSLCON(qorgMail*, Mail*);
    ~SSLCON();
    void setMethod(Method I) {
        Current = I;
    }
    void setMailbox(Mailbox* B) {
        this->B = B;
    }
    void setEmail(Email* E) {
        this->E = E;
    }
    void setAttachment(Structure* A) {
        this->A = A;
    }
    void setData(QList <QString> D) {
        this->Data = D;
    }

    Mail* M;
    Mailbox* B;
    Email* E;
    Structure* A;
    QList <QString> Data;
    bool Cancelled;
private:
    Method Current;
    void SecureLogin();
    void DownloadMBoxVector();
    void DownloadEmails();

    void DownloadAttachment();
    void SendEmail();

    void CopyEmail();
    void DeleteEmail();
    void MoveEmail();
    void AddFlagEmail();
    void DelFlagEmail();

    void CreateMailbox();
    void DeleteMailbox();
    void RenameMailbox();
    QByteArray readAll(QSslSocket*);
    QByteArray readIMAP(QSslSocket*);
    QByteArray readIMAPBODY(QSslSocket*);
    QByteArray readSMTP(QSslSocket*);
    void closeIMAP(QSslSocket*);
    QList <QString> splitBS(QString);
    QString NILCleaner(QString);
protected:
    void run();
public slots:
    void Canceled();
signals:
    void changeValue(int);
    void end();

    void LoginS(bool, QString);
    void MailboxesS(bool, QString);
    void EmailS(bool, QString);

    void AttachmentS(bool, QString);
    void SendEmailS(bool, QString);

    void CopyES(bool, QString);
    void DeleteES(bool, QString);
    void MoveES(bool, QString);

    void CreateMS(bool, QString);
    void DeleteMS(bool, QString);
    void RenameMS(bool, QString);
};
SSLCON::SSLCON(qorgMail* parent, Mail* I) :QThread(parent) {
    QMutexLocker L(&MAILaccessToCounter);
    MAILthreadCounter++;
    M = I;
    Cancelled = false;
    Current = Sleep;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}
SSLCON::~SSLCON() {
    QMutexLocker L(&MAILaccessToCounter);
    MAILthreadCounter--;
}

void SSLCON::run() {
    while (Current != Stop) {
        if (Current != Sleep) {
            if (Current == Login) {
                Current = Sleep;
                SecureLogin();
            } else if (Current == Mailboxes) {
                Current = Sleep;
                DownloadMBoxVector();
            } else if (Current == Emails) {
                DownloadEmails();
                break;
            } else if (Current == Attachment) {
                DownloadAttachment();
                break;
            } else if (Current == Send) {
                SendEmail();
                break;
            } else if (Current == CopyE) {
                CopyEmail();
                break;
            } else if (Current == DeleteE) {
                DeleteEmail();
                break;
            } else if (Current == MoveE) {
                MoveEmail();
                break;
            } else if (Current == CreateM) {
                CreateMailbox();
                break;
            } else if (Current == DeleteM) {
                DeleteMailbox();
                break;
            } else if (Current == RenameM) {
                RenameMailbox();
                break;
            }
        } else if (Current != Stop) {
            this->msleep(100);
        }
    }
    if (Current == Stop) {
        delete M;
    }
    emit end();
}
QByteArray SSLCON::readAll(QSslSocket* S) {
    QByteArray Reply;
    if (S->state() == QAbstractSocket::ConnectingState) {
        S->waitForConnected();
    }
    if (!Cancelled && (S->state() == QAbstractSocket::ConnectedState)) {
        Reply.append(S->readAll());
        while (S->waitForReadyRead(500)&&S->bytesAvailable() != 0) {
            Reply.append(S->readAll());
        }
    }
    return Reply;
}
QByteArray SSLCON::readIMAP(QSslSocket* S) {
    QByteArray Reply;
    if (!Cancelled && (S->state() == QAbstractSocket::ConnectedState) && S->waitForReadyRead()) {
        Reply.append(S->readAll());
        while (!(Reply.contains("TAG OK") || Reply.contains("TAG NO") || Reply.contains("TAG BAD"))) {
            if (S->waitForReadyRead() && S->bytesAvailable() != 0) {
                Reply.append(S->readAll());
            }
        }
    }
    return Reply;
}
QByteArray SSLCON::readIMAPBODY(QSslSocket *S) {
    QByteArray Reply;
    if (!Cancelled && (S->state() == QAbstractSocket::ConnectedState) && S->waitForReadyRead()) {
        Reply.append(S->readAll());
        if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
            return QByteArray("ERROR");
        }
        while (!(Reply.contains("{") || Reply.contains("}"))) {
            if (S->waitForReadyRead() && S->bytesAvailable() != 0) {
                Reply.append(S->readAll());
            }
        }
        int charachters = Reply.mid(Reply.indexOf("{")+1,Reply.indexOf("}")-Reply.indexOf("{")-1).toInt();
        Reply.remove(0,Reply.indexOf("}\r\n")+3);
        while (Reply.size() < charachters || !Reply.contains("TAG OK")) {
            if (S->waitForReadyRead() && S->bytesAvailable() != 0) {
                Reply.append(S->readAll());
            }
        }
        Reply.remove(Reply.lastIndexOf(")"), Reply.length()-Reply.lastIndexOf(")"));
    }
    return Reply;
}

QByteArray SSLCON::readSMTP(QSslSocket* S) {
    QByteArray Reply;
    if (!Cancelled && (S->state() == QAbstractSocket::ConnectedState) && S->waitForReadyRead()) {
        Reply.append(S->readAll());
        while (S->waitForReadyRead(100)&&S->bytesAvailable() != 0) {
            Reply.append(S->readAll());
        }
    }
    return Reply;
}
void SSLCON::closeIMAP(QSslSocket* S) {
    S->write(QString("TAG LOGOUT\r\n").toUtf8());
    readIMAP(S);
    S->close();
}
QList <QString> SSLCON::splitBS(QString I) {
    QList <QString> Output;
    if (I[0] != '\"') {
        QString P = "";
        uint V = 0;
        bool SPB = false;
        if (I[0] != '(') {
            P = I.mid(0, I.indexOf(" "));
            I = I.mid(I.indexOf(" ")+1, I.length()-I.indexOf(" ")-1);
            SPB = true;
        }
        uint S = 0;
        uint L = 0;
        for (int i = 0; i < I.length(); i++) {
            if (I[i] == '(') {
                if (L == 0) {
                    S = i;
                }
                L++;
            } else if (I[i] == ')') {
                L--;
                if (L == 0) {
                    if (SPB) {
                        V++;
                        QString O = P+"."+QString::number(V)+" "+I.mid(S+1, i-S-1);
                        if (I.mid(S+1, i-S-1)[0] != '\"') {
                            Output.append(splitBS(O));
                        } else {
                            Output.append(O);
                        }
                    } else {
                        V++;
                        QString O = QString::number(V)+" "+I.mid(S+1, i-S-1);
                        if (I.mid(S+1, i-S-1)[0] != '\"') {
                            Output.append(splitBS(O));
                        } else {
                            Output.append(O);
                        }
                    }
                    if (I[i+1] == ' ') {
                        break;
                    }
                }
            }
        }
    } else {
        Output.append("1 "+I);
    }
    return Output;
}
QString SSLCON::NILCleaner(QString I) {
    if (I.simplified() == "NIL") {
        return QString();
    } else {
        return I;
    }
}

void SSLCON::SecureLogin() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit LoginS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit LoginS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit LoginS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit LoginS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit LoginS(false, "ERROR: Authentication failed.");
        return;
    }
    closeIMAP(&S);
    emit changeValue(10);
    emit LoginS(true, "Logged in.");
}
void SSLCON::DownloadMBoxVector() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit MailboxesS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit MailboxesS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit MailboxesS(false, "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit MailboxesS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MailboxesS(false, "ERROR: Authentication failed.");
        return;
    }
    S.write("TAG LIST \"\" \"*\"\r\n");
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit MailboxesS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MailboxesS(false, "ERROR: Cannot list mailboxes.");
        return;
    }
    closeIMAP(&S);
    QStringList A = Reply.split("\n", QString::SkipEmptyParts);
    A.removeLast();
    QString Split;
    for (int i = 0; i < A.size(); i++) {
        Mailbox* temp = new Mailbox();
        QString Atts = A[i].mid(A[i].indexOf("(")+1, A[i].indexOf(")")-A[i].indexOf("(")-1);
        if (Atts.contains("\\HasChildren")) {
            temp->Mbox_Attrybutes|=Mailbox::HasChildren;
        }
        if (Atts.contains("\\HasNoChildren")) {
            temp->Mbox_Attrybutes|=Mailbox::HasNoChildren;
        }
        if (Atts.contains("\\Noselect")) {
            temp->Mbox_Attrybutes|=Mailbox::Noselect;
        }
        if (Atts.contains("\\Trash")) {
            temp->Mbox_Attrybutes|=Mailbox::Trash;
        }
        if (Atts.contains("\\Flagged")) {
            temp->Mbox_Attrybutes|=Mailbox::Flagged;
        }
        if (Atts.contains("\\Important")) {
            temp->Mbox_Attrybutes|=Mailbox::Important;
        }
        if (Atts.contains("\\Drafts")) {
            temp->Mbox_Attrybutes|=Mailbox::Drafts;
        }
        if (Atts.contains("\\All")) {
            temp->Mbox_Attrybutes|=Mailbox::All;
        }
        if (Atts.contains("\\Sent")) {
            temp->Mbox_Attrybutes|=Mailbox::Sent;
        }
        int pos = A[i].indexOf(")")+2;
        QString N = A[i].mid(pos+4, A[i].length()-pos-4).simplified();
        if (N[0] == '"'&&N[N.length()-1] == '"') {
            N = N.mid(1, N.length()-2);
        }
        temp->Mbox_Name = N;
        if (temp->Mbox_Name.toUpper() == "INBOX") {
            temp->Mbox_Attrybutes|=Mailbox::INBOX;
        }
        if (i == 0) {
            Split = A[i].mid(pos+1, 1);
        }
        temp->Mbox_Showname = temp->Mbox_Name.mid(temp->Mbox_Name.lastIndexOf(Split)+1, temp->Mbox_Name.length()-temp->Mbox_Name.lastIndexOf(Split)-1);
        temp->Mbox_Showname = Bit7ToBit8(temp->Mbox_Showname);
        M->Mboxv.push_back(temp);
    }
    for (uint i = 0; i < M->Mboxv.size(); i++) {
        if (M->Mboxv[i]->Mbox_Name.contains(Split)) {
            for (uint j = 0; j < M->Mboxv.size(); j++) {
                if (M->Mboxv[j]->Mbox_Name == M->Mboxv[i]->Mbox_Name.mid(0, M->Mboxv[i]->Mbox_Name.lastIndexOf(Split))&&i != j) {
                    M->Mboxv[j]->Mbox_Children.push_back(M->Mboxv[i]);
                    break;
                }
            }
        } else {
            M->Mboxv[i]->Mbox_Top = true;
        }
    }
    emit changeValue(15);
    emit MailboxesS(true, "Mailboxes tree downloaded.");
}
void SSLCON::DownloadEmails() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit EmailS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit EmailS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit EmailS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit EmailS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit EmailS(false, "ERROR: Authentication failed.");
        return;
    }
    for (uint i = 0; i < M->Mboxv.size(); i++) {
        if (M->Mboxv[i]->Mbox_Refresh) {
            S.write(QString("TAG SELECT \""+M->Mboxv[i]->Mbox_Name+"\"\r\n").toUtf8());
            Reply = readIMAP(&S);
            if (Reply.isEmpty() || Cancelled) {
                emit EmailS(false, "ERROR: No response from server.");
                return;
            } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
                closeIMAP(&S);
                emit EmailS(false, "ERROR: Cannot select mailbox.");
                return;
            }
            int existsIndex = Reply.indexOf(" EXISTS");
            int starIndex = Reply.mid(0, existsIndex).lastIndexOf("* ");
            emit changeValue(20);
            vector  <Email*>* Vec=&M->Mboxv[i]->Emailv;
            uint Fn = 1;
            QString Enu = Reply.mid(starIndex+2, existsIndex-starIndex-2);
            uint En = Enu.toInt();
            if (En > 0) {
                QString UIDS;
                {
                    for (uint k = Fn; k + 100 < En; k += 100) {
                        S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" UID\r\n").toUtf8());
                        QString Tmp = readIMAP(&S);
                        Tmp.remove(Tmp.indexOf("TAG OK"), Tmp.length()-Tmp.indexOf("TAG OK"));
                        UIDS.append(Tmp);
                    }
                    S.write(QString("TAG FETCH "+QString::number(En - En%100 + 1)+":"+Enu+" UID\r\n").toUtf8());
                    UIDS.append(readIMAP(&S));
                    if (UIDS.isEmpty() || Cancelled) {
                        emit EmailS(false, "ERROR: No response from server.");
                        return;
                    } else if (UIDS.contains("TAG NO") || UIDS.contains("TAG BAD")) {
                        closeIMAP(&S);
                        emit EmailS(false, "ERROR: Cannot fetch uids.");
                        return;
                    }
                    QList <QString> UIDL = UIDS.split("UID");
                    QList  <uint>  UIDLi;
                    UIDL.removeFirst();
                    for (int k = 0; k < UIDL.size(); k++) {
                        UIDLi.append(UIDL[k].mid(1, UIDL[k].indexOf(")")-1).toInt());
                    }
                    for (uint k = 0; k < Vec->size(); k++) {
                        if (UIDLi.size()-1 < static_cast<int>(k) || (*Vec)[k]->Email_UID != UIDLi[k]) {
                            Vec->erase(Vec->begin()+k);
                            k--;
                        }
                    }
                    if (Vec->size() != 0) {
                        for (int k = UIDLi.size()-1; k > -1; k--) {
                            if (UIDLi[k] <= Vec->back()->Email_UID) {
                                Fn = k+2;
                                break;
                            }
                        }
                    }
                    for (uint k = 0; k < En-Fn+1; k++) {
                        Vec->push_back(new Email());
                        (*Vec)[Fn+k-1]->Email_UID = UIDLi[Fn+k-1];
                    }
                }
                if (Fn <= En) {
                    emit changeValue(25);
                    QString Subjects;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (Subject)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            Subjects.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (Subject)]\r\n").toUtf8());
                        Subjects.append(readIMAPBODY(&S));
                        if (Subjects.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (Subjects == "ERROR") {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch subjects.");
                            return;
                        }
                        QList <QString> SubL = Subjects.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (SubL[k].contains("Subject:", Qt::CaseInsensitive)) {
                                QString Sub = SubL[k].mid(SubL[k].indexOf("Subject: ")+9, SubL[k].indexOf("\r\n)", SubL[k].indexOf("Subject: ")+9)-SubL[k].indexOf("Subject:")-9);
                                Sub.remove("\r\n");
                                Sub = NameFilter(Sub);
                                if (Sub.simplified().isEmpty()) {
                                    Sub="(no subject)";
                                }
                                Sub.replace("<", "&lt;");
                                Sub.replace(">", "&gt;");
                                QTextDocument D;
                                D.setHtml(Sub.simplified());
                                (*Vec)[Fn+k-1]->Email_Subject = D.toPlainText();
                            }
                        }
                    }
                    emit changeValue(30);
                    QString Dates;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" INTERNALDATE\r\n").toUtf8());
                            QString Tmp = readIMAP(&S);
                            Tmp.remove(Tmp.indexOf("TAG OK"), Tmp.length()-Tmp.indexOf("TAG OK"));
                            Dates.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" INTERNALDATE\r\n").toUtf8());
                        Dates.append(readIMAP(&S));
                        if (Dates.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (Dates.contains("TAG NO") || Dates.contains("TAG BAD")) {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch internaldates.");
                            return;
                        }
                        QList <QString> DateL = Dates.split("INTERNALDATE \"");
                        DateL.removeFirst();
                        for (uint k = 0; k < En-Fn+1; k++) {
                            QString Date = DateL[k].mid(0, DateL[k].indexOf("\""));
                            QList <QString> Mon;
                            Mon << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
                            QDateTime Tmp;
                            Tmp.setTimeSpec(Qt::UTC);
                            int Month = Mon.indexOf(Date.mid(3, 3));
                            if (Month < 9) {
                                Date.replace(Date.mid(3, 3), "0"+QString::number(Month+1));
                            } else {
                                Date.replace(Date.mid(3, 3), QString::number(Month+1));
                            }
                            Tmp.setDate(QDate::fromString(Date.mid(0, 10), "dd-MM-yyyy"));
                            Tmp.setTime(QTime::fromString(Date.mid(11, 8), "HH:mm:ss"));
                            if (Date[20] == '+') {
                                Tmp = Tmp.addSecs(-(Date.mid(21, 2).toInt()*3600+Date.mid(23, 2).toInt()*60));
                                Tmp.setTimeSpec(Qt::UTC);
                            } else {
                                Tmp = Tmp.addSecs(Date.mid(21, 2).toInt()*3600+Date.mid(23, 2).toInt()*60);
                                Tmp.setTimeSpec(Qt::UTC);
                            }
                            (*Vec)[Fn+k-1]->Email_Date = Tmp.toLocalTime();
                        }
                    }
                    emit changeValue(35);
                    QString Froms;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (From)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            Froms.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (From)]\r\n").toUtf8());
                        Froms.append(readIMAPBODY(&S));
                        if (Froms.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (Froms == "ERROR") {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch froms.");
                            return;
                        }
                        QList <QString> FromsL = Froms.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (FromsL[k].contains("From:", Qt::CaseInsensitive)) {
                                QString From = FromsL[k].mid(FromsL[k].indexOf("From: ", Qt::CaseInsensitive)+6, FromsL[k].indexOf("\r\n)", FromsL[k].indexOf("From: ", Qt::CaseInsensitive)+6)-FromsL[k].indexOf("From: ", Qt::CaseInsensitive)-6);
                                (*Vec)[Fn+k-1]->Email_From = From.mid(From.indexOf("<")+1, From.indexOf(">")-From.indexOf("<")-1).simplified();
                            }
                        }
                    }
                    emit changeValue(40);
                    QString Flags;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" FLAGS\r\n").toUtf8());
                            QString Tmp = readIMAP(&S);
                            Tmp.remove(Tmp.indexOf("TAG OK"), Tmp.length()-Tmp.indexOf("TAG OK"));
                            Flags.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" FLAGS\r\n").toUtf8());
                        Flags.append(readIMAP(&S));
                        if (Flags.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (Flags.contains("TAG NO") || Flags.contains("TAG BAD")) {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch flags.");
                            return;
                        }
                        QList <QString> FlagsL = Flags.split("FLAGS (");
                        FlagsL.removeFirst();
                        for (uint k = 0; k < En-Fn+1; k++) {
                            QString Fla = FlagsL[k].mid(0, FlagsL[k].indexOf(")"));
                            char Flag = 0;
                            if (Fla.contains("\\Seen")) {
                                Flag |= Email::Seen;
                            }
                            if (Fla.contains("\\Answered")) {
                                Flag |= Email::Answered;
                            }
                            if (Fla.contains("\\Flagged")) {
                                Flag |= Email::Flagged;
                            }
                            if (Fla.contains("\\Deleted")) {
                                Flag |= Email::Deleted;
                            }
                            if (Fla.contains("\\Draft")) {
                                Flag |= Email::Draft;
                            }
                            if (Fla.contains("\\Recent")) {
                                Flag |= Email::Recent;
                            }
                            (*Vec)[Fn+k-1]->Email_Flags = Flag;
                        }
                    }
                    emit changeValue(45);
                    QString MessageID;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (Message-ID)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            MessageID.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (Message-ID)]\r\n").toUtf8());
                        MessageID.append(readIMAPBODY(&S));
                        if (MessageID.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (MessageID == "ERROR") {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch message IDs.");
                            return;
                        }
                        QList <QString> MIDL = MessageID.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (MIDL[k].contains("Message-ID:", Qt::CaseInsensitive)) {
                                (*Vec)[Fn+k-1]->Email_MessageID = MIDL[k].mid(MIDL[k].indexOf("<")+1, MIDL[k].indexOf(">", MIDL[k].indexOf("<")+1)-MIDL[k].indexOf("<")-1);
                            }
                        }
                    }
                    emit changeValue(50);
                    QString ReplyTo;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (Reply-To)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            ReplyTo.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (Reply-To)]\r\n").toUtf8());
                        ReplyTo.append(readIMAPBODY(&S));
                        if (ReplyTo.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (ReplyTo == "ERROR") {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch reply-to.");
                            return;
                        }
                        QList <QString> RPLL = ReplyTo.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (RPLL[k].contains("Reply-to:", Qt::CaseInsensitive)) {
                                (*Vec)[Fn+k-1]->Email_ReplyTo = RPLL[k].mid(RPLL[k].indexOf("<")+1, RPLL[k].indexOf(">", RPLL[k].indexOf("<")+1)-RPLL[k].indexOf("<")-1);
                            }
                        }
                    }
                    emit changeValue(55);
                    QString FetchTo;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (To)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            FetchTo.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (To)]\r\n").toUtf8());
                        FetchTo.append(readIMAPBODY(&S));
                        if (FetchTo.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (FetchTo.contains("TAG NO") || FetchTo.contains("TAG BAD")) {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch to.");
                            return;
                        }
                        QList <QString> FTL = FetchTo.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (FTL[k].contains("To:", Qt::CaseInsensitive)) {
                                FTL[k] = FTL[k].mid(FTL[k].indexOf("To: ", Qt::CaseInsensitive)+4, FTL[k].indexOf("\r\n)", FTL[k].indexOf("To: ", Qt::CaseInsensitive)+4)-FTL[k].indexOf("To: ", Qt::CaseInsensitive)-4);
                                QList <QString> FTLL = FTL[k].split(",");
                                for (int l = 0; l < FTLL.size(); l++) {
                                    QString Email =  FTLL[l].mid(FTLL[l].indexOf("<")+1, FTLL[l].indexOf(">")-FTLL[l].indexOf("<")-1).simplified();
                                    if (find((*Vec)[Fn+k-1]->Email_RecipientsTo.begin(), (*Vec)[Fn+k-1]->Email_RecipientsTo.end(), Email) == (*Vec)[Fn+k-1]->Email_RecipientsTo.end() &&
                                            Email != M->User.mid(0, M->User.indexOf("@"))+"@"+M->IMAPserver.mid(M->IMAPserver.indexOf(".")+1, M->IMAPserver.length()-M->IMAPserver.indexOf(".")-1)) {
                                        (*Vec)[Fn+k-1]->Email_RecipientsTo.push_back(Email);
                                    }
                                }
                            }
                        }
                    }
                    emit changeValue(60);
                    QString FetchCC;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODY.PEEK[HEADER.FIELDS (CC)]\r\n").toUtf8());
                            QString Tmp = readIMAPBODY(&S);
                            FetchCC.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODY.PEEK[HEADER.FIELDS (CC)]\r\n").toUtf8());
                        FetchCC.append(readIMAPBODY(&S));
                        if (FetchCC.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (FetchCC == "ERROR") {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch CC.");
                            return;
                        }
                        QList <QString> FCL = FetchCC.split(")\r\n*");
                        for (uint k = 0; k < En-Fn+1; k++) {
                            if (FCL[k].contains("CC:", Qt::CaseInsensitive)) {
                                FCL[k] = FCL[k].mid(FCL[k].indexOf("Cc: ", Qt::CaseInsensitive)+4, FCL[k].indexOf("\r\n)", FCL[k].indexOf("Cc: ", Qt::CaseInsensitive)+4)-FCL[k].indexOf("Cc: ", Qt::CaseInsensitive)-4);
                                QList <QString> FTLL = FCL[k].split(",");
                                for (int l = 0; l < FTLL.size(); l++) {
                                    QString Email =  FTLL[l].mid(FTLL[l].indexOf("<")+1, FTLL[l].indexOf(">")-FTLL[l].indexOf("<")-1).simplified();
                                    if (find((*Vec)[Fn+k-1]->Email_RecipientsCC.begin(), (*Vec)[Fn+k-1]->Email_RecipientsCC.end(), Email) == (*Vec)[Fn+k-1]->Email_RecipientsCC.end() &&
                                            Email != M->User.mid(0, M->User.indexOf("@"))+"@"+M->IMAPserver.mid(M->IMAPserver.indexOf(".")+1, M->IMAPserver.length()-M->IMAPserver.indexOf(".")-1)) {
                                        (*Vec)[Fn+k-1]->Email_RecipientsCC.push_back(Email);
                                    }
                                }
                            }
                        }
                    }
                    emit changeValue(65);
                    QString BS;
                    {
                        for (uint k = Fn; k + 100 < En; k +=100) {
                            S.write(QString("TAG FETCH "+QString::number(k)+":"+QString::number(k + 99)+" BODYSTRUCTURE\r\n").toUtf8());
                            QString Tmp = readIMAP(&S);
                            Tmp.remove(Tmp.indexOf("TAG OK"), Tmp.length()-Tmp.indexOf("TAG OK"));
                            BS.append(Tmp);
                        }
                        S.write(QString("TAG FETCH "+QString::number(Fn+((En-Fn)/100)*100)+":"+Enu+" BODYSTRUCTURE\r\n").toUtf8());
                        BS.append(readIMAP(&S));
                        if (BS.isEmpty() || Cancelled) {
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: No response from server.");
                            return;
                        } else if (BS.contains("TAG NO") || BS.contains("TAG BAD")) {
                            closeIMAP(&S);
                            for (uint k = En; k >= Fn; k--) {
                                delete (*Vec)[k-1];
                                Vec->erase(Vec->begin()+k-1);
                            }
                            emit EmailS(false, "ERROR: Cannot fetch bodystructures.");
                            return;
                        }
                        QList  <QString>  BSL = BS.split("BODYSTRUCTURE (");
                        BSL.removeFirst();
                        for (uint k = 0; k < En-Fn+1; k++) {
                            QList <QString> RBS = splitBS(BSL[k].mid(0, BSL[k].lastIndexOf(")")-1));
                            for (int l = 0; l < RBS.size(); l++) {
                                Structure* Tmp = new Structure();
                                QList <QString> IL;
                                int L = 0;
                                QString Data;
                                for (int m = 0; m < RBS[l].length(); m++) {
                                    if (RBS[l][m] == ' '&&L == 0) {
                                        IL.append(Data);
                                        Data.clear();
                                    } else {
                                        if (RBS[l][m] == '(') {
                                            L++;
                                        }
                                        if (RBS[l][m] == ')') {
                                            L--;
                                        }
                                        Data.append(RBS[l][m]);
                                    }
                                }
                                IL.append(Data);
                                for (int m = 0; m < IL.size(); m++) {
                                    IL[m].remove("(");
                                    IL[m].remove(")");
                                    IL[m].remove("\"");
                                }
                                Tmp->Structure_Number = IL[0];
                                Tmp->Structure_Type = NILCleaner(IL[1]).toUpper();
                                Tmp->Structure_Subtype = NILCleaner(IL[2]).toUpper();
                                QList  <QString>  ILL = IL[3].split(" ");
                                QList <QString> BILL = IL[3].toUpper().split(" ");
                                if (BILL.indexOf("CHARSET", 0) >= 0) {
                                    Tmp->Structure_Attrybutes.Charset = ILL[BILL.indexOf("CHARSET", 0)+1].toUpper();
                                }
                                if (BILL.indexOf("NAME", 0) >= 0) {
                                    Tmp->Structure_Attrybutes.Name = ILL[BILL.indexOf("NAME", 0)+1];
                                }
                                if (IL[4].contains("<")&&IL[4].contains(">")) {
                                    IL[4]=IL[4].mid(1, IL[4].length()-2);
                                }
                                Tmp->Structure_CID = NILCleaner(IL[4]);
                                Tmp->Structure_Descryption = NILCleaner(IL[5]).toUpper();
                                Tmp->Structure_Encoding = NILCleaner(IL[6]).toUpper();
                                Tmp->Structure_Size = IL[7].toInt();
                                if (Tmp->Structure_Type == "TEXT") {
                                    Tmp->Structure_Lines = IL[8].toInt();
                                    Tmp->Structure_MD5 = NILCleaner(IL[9]);
                                    QList  <QString>  ILL = IL[10].split(" ");
                                    QList <QString> BILL = IL[10].toUpper().split(" ");
                                    Tmp->Structure_Disposition = NILCleaner(ILL[0].remove("\"").toUpper());
                                    if (BILL.indexOf("FILENAME", 0) >= 0) {
                                        Tmp->Structure_Attrybutes.Name = ILL[BILL.indexOf("FILENAME", 0)+1];
                                    }
                                    Tmp->Structure_Language = NILCleaner(IL[11]).toUpper();
                                    if (IL.size() == 13) {
                                        Tmp->Structure_Location = NILCleaner(IL[12]);
                                    }
                                } else {
                                    Tmp->Structure_MD5 = NILCleaner(IL[8]);
                                    QList <QString> ILL = IL[9].split(" ");
                                    QList <QString> BILL = IL[9].toUpper().split(" ");
                                    Tmp->Structure_Disposition = NILCleaner(ILL[0].remove("\"").toUpper());
                                    if (BILL.indexOf("FILENAME", 0) >= 0) {
                                        Tmp->Structure_Attrybutes.Name = ILL[BILL.indexOf("FILENAME", 0)+1];
                                    }
                                    Tmp->Structure_Language = NILCleaner(IL[10]).toUpper();
                                    if (IL.size() == 12) {
                                        Tmp->Structure_Location = NILCleaner(IL[11]);
                                    }
                                }
                                (*Vec)[Fn+k-1]->Structurev.push_back(Tmp);
                            }
                        }
                    }
                    emit changeValue(70);
                    {
                        double VA=(static_cast<double>(30)/static_cast<double>(En-Fn+1));
                        for (uint k = 0; k < En-Fn+1; k++) {
                            for (uint l = 0; l < (*Vec)[Fn+k-1]->Structurev.size()&&l < 2; l++) {
                                QByteArray Text;
                                int t = 0;
                                if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Type == "TEXT" &&
                                        (*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Name.isEmpty()) {
                                    if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Subtype == "HTML") {
                                        t = 1;
                                    }
                                    if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Size != 0) {
                                        for (uint m = 0; m < (*Vec)[Fn+k-1]->Structurev[l]->Structure_Size; m += 10000) {
                                            S.write(QString("TAG UID FETCH "+QString::number((*Vec)[Fn+k-1]->Email_UID)+" BODY["+(*Vec)[Fn+k-1]->Structurev[l]->Structure_Number+"]<"+QString::number(m)+".10000>\r\n").toUtf8());
                                            QByteArray Tmp = readIMAPBODY(&S);
                                            Text.append(Tmp);
                                        }
                                        if (Text.isEmpty() || Cancelled) {
                                            for (uint m = En; m >= Fn; m--) {
                                                for (uint n = (*Vec)[m-1]->Structurev.size(); n > 0; n--) {
                                                    delete (*Vec)[m-1]->Structurev[n-1];
                                                    (*Vec)[m-1]->Structurev.erase((*Vec)[m-1]->Structurev.begin()+n-1);
                                                }
                                                delete (*Vec)[m-1];
                                                Vec->erase(Vec->begin()+m-1);
                                            }
                                            emit EmailS(false, "ERROR: No response from server.");
                                            return;
                                        } else if (Text == "ERROR") {
                                            closeIMAP(&S);
                                            for (uint m = En; m >= Fn; m--) {
                                                for (uint n = (*Vec)[m-1]->Structurev.size(); n > 0; n--) {
                                                    delete (*Vec)[m-1]->Structurev[n-1];
                                                    (*Vec)[m-1]->Structurev.erase((*Vec)[m-1]->Structurev.begin()+n-1);
                                                }
                                                delete (*Vec)[m-1];
                                                Vec->erase(Vec->begin()+m-1);
                                            }
                                            emit EmailS(false, "ERROR: Cannot fetch bodies.");
                                            return;
                                        }
                                        if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Encoding == "BASE64") {
                                            Text = QByteArray::fromBase64(Text);
                                        } else if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Encoding == "QUOTED-PRINTABLE") {
                                            Text = QPDecode(Text);
                                        }
                                        if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset != "UTF-8" && (!(*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset.isEmpty())) {
                                            if (QTextCodec::availableCodecs().contains((*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset.toUtf8())) {
                                                QTextCodec* C = QTextCodec::codecForName((*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset.toUtf8());
                                                (*Vec)[Fn+k-1]->Email_Body[t] = C->toUnicode(Text);
                                            }
                                        } else {
                                            (*Vec)[Fn+k-1]->Email_Body[t] = Text;
                                        }
                                    }
                                } else {
                                    break;
                                }
                            }
                            emit changeValue(static_cast<int>(70+(k+1)*VA));
                        }
                    }
                }
            } else {
                Vec->clear();
            }
            S.write(QString("TAG CLOSE\r\n").toUtf8());
            readIMAP(&S);
        }
    }
    closeIMAP(&S);
    emit EmailS(true, "Emails header fields and bodies downloaded.");
}

void SSLCON::DownloadAttachment() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit AttachmentS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit AttachmentS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit AttachmentS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit AttachmentS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit AttachmentS(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG SELECT \""+B->Mbox_Name+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty() || Cancelled) {
        emit AttachmentS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit AttachmentS(false, "ERROR: Cannot select mailbox.");
        return;
    }
    Reply.clear();
    for (uint m = 0; m < A->Structure_Size; m += 10000) {
        S.write(QString("TAG UID FETCH "+QString::number(E->Email_UID)+" BODY["+A->Structure_Number+"]<"+QString::number(m)+".10000>\r\n").toUtf8());
        QByteArray Tmp = readIMAPBODY(&S);
        Tmp.remove(0, Tmp.indexOf("}\r\n")+3);
        Tmp.remove(Tmp.indexOf(")\r\nTAG OK"), Tmp.length()-Tmp.indexOf(")\r\nTAG OK"));
        Reply.append(Tmp);
    }
    if (Reply.isEmpty() || Cancelled) {
        emit AttachmentS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit AttachmentS(false, "ERROR: Cannot fetch attachment.");
        return;
    }
    closeIMAP(&S);
    QByteArray Array;
    if (A->Structure_Encoding == "BASE64") {
        Array = QByteArray::fromBase64(Reply.toUtf8());
    } else if (A->Structure_Encoding == "QUOTED-PRINTABLE") {
        Array = QPDecode(Reply.toUtf8());
    }
    if (A->Structure_Attrybutes.Charset != "UTF-8"&&!(A->Structure_Attrybutes.Charset.isEmpty())) {
        if (QTextCodec::availableCodecs().contains(A->Structure_Attrybutes.Charset.toUtf8())) {
            QTextCodec* C = QTextCodec::codecForName(A->Structure_Attrybutes.Charset.toUtf8());
            Array = C->toUnicode(Array).toUtf8();
        }
    }
    QFile F(Data.first());
    F.open(QIODevice::WriteOnly);
    F.write(Array);
    F.close();
    emit AttachmentS(true, "Attachment downloaded.");
}
void SSLCON::SendEmail() {
    QByteArray P = calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512));
    QString Reply;
    QSslSocket S;
    Email* EM;
    if (E == NULL) {
        EM = new Email;
    } else {
        EM = E;
    }
    QString Header;
    QList <QString> To;
    QList <QString> Bodies;
    QList <QString> Attachments;
    for (int i = 0, j = 0; i < Data.size(); i++) {
        if (Data[i].isEmpty()) {
            j++;
        } else {
            switch (j) {
            case 0: {
                Header = Data[i];
            }break;
            case 1: {
                To.append(Data[i]);
            }break;
            case 2: {
                Bodies.append(Data[i]);
            }break;
            case 3: {
                Attachments.append(Data[i]);
            }break;
            }
        }
    }
    QString Output;
    QString boundry = QCryptographicHash::hash(Header.toUtf8(), QCryptographicHash::Md5).toHex();
    Output.append(Header);
    Output.append("Content-Type: multipart/mixed; boundary=\"------"+boundry+"\"\nThis is a multi-part message in MIME format.\n\n");
    Output.append("--------"+boundry+"\n");
    Output.append("Content-Type: text/plain; charset=UTF-8\n");
    Output.append("Content-Transfer-Encoding: base64\n\n");
    Output.append(Bodies[0].toUtf8().toBase64()+"\n\n");
    if (Bodies.size() == 2) {
        Output.append("--------"+boundry+"\n");
        Output.append("Content-Type: text/html; charset=UTF-8\n");
        Output.append("Content-Transfer-Encoding: base64\n\n");
        Output.append(Bodies[1].toUtf8().toBase64()+"\n\n");
    }
    if (E != NULL) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            S.connectToHostEncrypted(M->IMAPserver, 993);
            if (!S.waitForConnected()) {
                emit SendEmailS(false, "ERROR: Problem with connection.");
                return;
            }
        }
        if (!S.waitForEncrypted()) {
            if (!S.sslErrors().isEmpty()) {
                qorgMail* P = qobject_cast<qorgMail*>(this->parent());
                if (P->SSLSocketError(S.sslErrors())) {
                    S.ignoreSslErrors(S.sslErrors());
                    S.connectToHostEncrypted(M->IMAPserver, 993);
                } else {
                    emit SendEmailS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                    return;
                }
            }
        }
        Reply = readAll(&S);
        if (Reply.isEmpty()) {
            emit SendEmailS(false, "ERROR(IMAP): No HELLO message.");
            return;
        }
        S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
        Reply = readIMAP(&S);
        if (Reply.isEmpty()) {
            emit SendEmailS(false, "ERROR: No response from server.");
            return;
        } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
            closeIMAP(&S);
            emit SendEmailS(false, "ERROR(IMAP): Authentication failed.");
            return;
        }
        S.write(QString("TAG SELECT \""+B->Mbox_Name+"\"\r\n").toUtf8());
        Reply = readIMAP(&S);
        if (Reply.isEmpty()) {
            emit SendEmailS(false, "ERROR: No response from server.");
            return;
        } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
            closeIMAP(&S);
            emit SendEmailS(false, "ERROR: Cannot select mailbox.");
            return;
        }
    }
    for (int i = 0; i < Attachments.size(); i++) {
        Output.append("--------"+boundry+"\n");
        if (Attachments[i][0] == '<') {
            QMimeDatabase db;
            QString Path = Attachments[i].mid(1, Attachments[i].length()-2);
            QFile* F = new QFile(Path);
            F->open(QFile::ReadOnly);
            QByteArray Array = F->readAll();
            Output.append("Content-Type: "+db.mimeTypeForData(Array).name());
            Output.append("; name=\""+Path.mid(Path.lastIndexOf(QDir::separator())+1, Path.length()-Path.lastIndexOf(QDir::separator())-1)+"\"\n");
            Output.append("Content-Disposition: attachment; filename=\""+Path.mid(Path.lastIndexOf(QDir::separator())+1, Path.length()-Path.lastIndexOf(QDir::separator())-1)+"\"\n");
            Output.append("Content-Transfer-Encoding: base64\n\n");
            Output.append(Array.toBase64()+"\n\n");
            F->close();
        } else {
            if (E != NULL) {
                S.write(QString("TAG UID FETCH "+QString::number(E->Email_UID)+" BODY["+E->Structurev[Attachments[i].toInt()]->Structure_Number+"]\r\n").toUtf8());
                Reply = readIMAP(&S);
                if (Reply.isEmpty()) {
                    emit SendEmailS(false, "ERROR: No response from server.");
                    return;
                } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
                    closeIMAP(&S);
                    emit SendEmailS(false, "ERROR: Cannot fetch attachment.");
                    return;
                }
                Reply = Reply.mid(Reply.indexOf("}")+1, Reply.indexOf(")\r\nTAG OK")-Reply.indexOf("}")-1);
                Output.append("Content-Type: "+EM->Structurev[Attachments[i].toInt()]->Structure_Type.toLower()+"/"+EM->Structurev[Attachments[i].toInt()]->Structure_Subtype.toLower())+"\n";
                Output.append("Content-Transfer-Encoding: "+EM->Structurev[Attachments[i].toInt()]->Structure_Encoding+"\n");
                if (!EM->Structurev[Attachments[i].toInt()]->Structure_Attrybutes.Name.isEmpty()) {
                    Output.append("Content-Disposition: attachment; filename=\""+EM->Structurev[Attachments[i].toInt()]->Structure_Attrybutes.Name+"\"\n");
                }
                if (!EM->Structurev[Attachments[i].toInt()]->Structure_CID.isEmpty()) {
                    Output.append("Content-ID: <"+EM->Structurev[Attachments[i].toInt()]->Structure_CID+">\n");
                }
                Output.append("Content-Transfer-Encoding: "+EM->Structurev[Attachments[i].toInt()]->Structure_Encoding+"\n\n");
                Output.append(Reply+"\n\n");
            }
        }
    }
    if (E != NULL) {
        closeIMAP(&S);
    }
    Output.append("--------"+boundry+"--");
    S.connectToHostEncrypted(M->SMTPserver, 465);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->SMTPserver, 465);
        if (!S.waitForConnected()) {
            emit SendEmailS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->SMTPserver, 465);
            } else {
                emit SendEmailS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR(SMTP): No HELLO message.");
        return;
    }
    S.write(QString("EHLO "+M->SMTPserver.mid(5, M->SMTPserver.length()-5)+"\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    S.write(QString("AUTH LOGIN\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    S.write(QString(M->User.toUtf8().toBase64()+"\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    S.write(QString(P.toBase64()+"\r\n").toUtf8());
    P.clear();
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    if (!Reply.contains("235")) {
        emit SendEmailS(false, "ERROR(SMTP): Authentication failed.");
        return;
    }
    S.write(QString("MAIL FROM: <"+M->User+"@"+M->IMAPserver.mid(M->IMAPserver.indexOf(".")+1, M->IMAPserver.length()-M->IMAPserver.indexOf(".")-1)+">\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    for (int i = 0; i < To.size(); i++) {
        S.write(QString("RCPT TO: <"+To[i]+">\r\n").toUtf8());
        Reply = readSMTP(&S);
        if (Reply.isEmpty()) {
            emit SendEmailS(false, "ERROR: No response from server.");
            return;
        }
    }
    S.write(QString("DATA\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    }
    S.write(QString(Output+"\r\n.\r\n").toUtf8());
    Reply = readSMTP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("552")) {
        emit SendEmailS(false, "ERROR: Data too large.");
        return;
    }
    S.write(QString("QUIT\r\n").toUtf8());
    S.close();
    emit SendEmailS(true, "Email sent.");
}

void SSLCON::CopyEmail() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit CopyES(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit CopyES(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit CopyES(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CopyES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CopyES(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG SELECT \""+B->Mbox_Name+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CopyES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CopyES(false, "ERROR: Cannot select mailbox.");
        return;
    }
    S.write(QString("TAG UID COPY "+QString::number(E->Email_UID)+" \""+Data.first()+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CopyES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CopyES(false, "ERROR: Cannot copy email.");
        return;
    }
    closeIMAP(&S);
    emit CopyES(true, "Copied.");
}
void SSLCON::DeleteEmail() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit DeleteES(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit DeleteES(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit DeleteES(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteES(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG SELECT \""+B->Mbox_Name+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteES(false, "ERROR: Cannot select mailbox.");
        return;
    }
    for (uint i = 0; i < M->Mboxv.size(); i++) {
        if (M->Mboxv[i]->Mbox_Attrybutes&Mailbox::Trash) {
            S.write(QString("TAG UID COPY "+QString::number(E->Email_UID)+" \""+M->Mboxv[i]->Mbox_Name+"\"\r\n").toUtf8());
            Reply = readIMAP(&S);
            if (Reply.isEmpty()) {
                emit DeleteES(false, "ERROR: No response from server.");
                return;
            } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
                closeIMAP(&S);
                emit DeleteES(false, "ERROR: Cannot copy email to trash.");
                return;
            }
            break;
        }
    }
    S.write(QString("TAG UID STORE "+QString::number(E->Email_UID)+" +FLAGS (\\Deleted)\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteES(false, "ERROR: Cannot set \\Deleted flag.");
        return;
    }
    S.write(QString("TAG EXPUNGE\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteES(false, "ERROR: Cannot expunge email.");
        return;
    }
    closeIMAP(&S);
    emit DeleteES(true, "Deleted");
}
void SSLCON::MoveEmail() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit MoveES(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit MoveES(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MoveES(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG SELECT \""+B->Mbox_Name+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MoveES(false, "ERROR: Cannot select mailbox.");
        return;
    }
    S.write(QString("TAG UID COPY "+QString::number(E->Email_UID)+" \""+Data.first()+"\"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MoveES(false, "ERROR: Cannot copy email.");
        return;
    }
    S.write(QString("TAG UID STORE "+QString::number(E->Email_UID)+" +FLAGS (\\Deleted)\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MoveES(false, "ERROR: Cannot set \\Deleted flag.");
        return;
    }
    S.write(QString("TAG EXPUNGE\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MoveES(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit MoveES(false, "ERROR: Cannot expunge email.");
        return;
    }
    closeIMAP(&S);
    emit MoveES(true, "Moved.");
}

void SSLCON::CreateMailbox() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit CreateMS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit CreateMS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit CreateMS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CreateMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CreateMS(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG LIST \"\" \"\"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CreateMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CreateMS(false, "ERROR: Cannot receive hierarchy separator.");
        return;
    }
    QString hierarchySeparator = Reply.mid(Reply.indexOf(")")+3, 1);
    if (Data[0].isEmpty()) {
        S.write(QString("TAG CREATE \""+Bit8ToBit7(Data[1]).remove(hierarchySeparator)+"\"\r\n").toUtf8());
    } else {
        S.write(QString("TAG CREATE \""+Data[0]+hierarchySeparator+Bit8ToBit7(Data[1]).remove(hierarchySeparator)+"\"\r\n").toUtf8());
    }
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CreateMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CreateMS(false, "ERROR: Cannot create mailbox.");
        return;
    }
    closeIMAP(&S);
    emit CreateMS(true, "Mailbox created.");
}
void SSLCON::DeleteMailbox() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit DeleteMS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit DeleteMS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit DeleteMS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteMS(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG DELETE \""+Data[0]+"\"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit DeleteMS(false, "ERROR: Cannot delete mailbox.");
        return;
    }
    closeIMAP(&S);
    emit DeleteMS(true, "Mailbox deleted.");
}
void SSLCON::RenameMailbox() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    if (!S.waitForConnected()) {
        S.connectToHostEncrypted(M->IMAPserver, 993);
        if (!S.waitForConnected()) {
            emit RenameMS(false, "ERROR: Problem with connection.");
            return;
        }
    }
    if (!S.waitForEncrypted()) {
        if (!S.sslErrors().isEmpty()) {
            qorgMail* P = qobject_cast<qorgMail*>(this->parent());
            if (P->SSLSocketError(S.sslErrors())) {
                S.ignoreSslErrors(S.sslErrors());
                S.connectToHostEncrypted(M->IMAPserver, 993);
            } else {
                emit RenameMS(false, "ERROR: Problem with SSL certificate. Check your SSL Manager.");
                return;
            }
        }
    }
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit RenameMS(false,  "ERROR: No HELLO message.");
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit RenameMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit RenameMS(false, "ERROR: Authentication failed.");
        return;
    }
    S.write(QString("TAG LIST \"\" \"\"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit CreateMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit CreateMS(false, "ERROR: Cannot receive hierarchy separator.");
        return;
    }
    QString hierarchySeparator = Reply.mid(Reply.indexOf(")")+3, 1);
    S.write(QString("TAG RENAME \""+Data[0]+"\" \""+Data[0].mid(0, Data[0].lastIndexOf(hierarchySeparator)+1)+Bit8ToBit7(Data[1]).remove(hierarchySeparator)+"\"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit RenameMS(false, "ERROR: No response from server.");
        return;
    } else if (Reply.contains("TAG NO") || Reply.contains("TAG BAD")) {
        closeIMAP(&S);
        emit RenameMS(false, "ERROR: Cannot rename mailbox.");
        return;
    }
    closeIMAP(&S);
    emit RenameMS(true, "Mailbox renamed.");
}

void SSLCON::Canceled() {
    Cancelled = true;
}


class Services {
public:
    Services(QString A, QString B, QString C) {
        Name = A;
        IMAP = B;
        SMTP = C;
    }
    QString Name;
    QString IMAP;
    QString SMTP;
};
QList  <Services>  AvailableServices;
class QItemCheckBox :public QCheckBox {
    Q_OBJECT
public:
    QItemCheckBox(QWidget* parent, uint IID) :QCheckBox(parent) {
        ItemID = IID;
    }
    uint ItemID;
};
class MailboxTree : public QDialog {
    Q_OBJECT
public:
    MailboxTree(Mail* M, QWidget* parent) :QDialog(parent) {
        vec=&M->Mboxv;
        setWindowTitle("Choose mailboxes to use.");
        setWindowIcon(QIcon(":/main/QOrganizer.png"));
        QGridLayout* Layout = new QGridLayout(this);
        Tree = new QTreeWidget(this);
        Tree->header()->hide();
        Tree->setColumnCount(2);
        Tree->header()->setStretchLastSection(false);
        Tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        Tree->setColumnWidth(1, 20);
        Cancel = new QPushButton(this);
        Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(Cancel, SIGNAL(clicked()), this, SLOT(CancelC()));
        OK = new QPushButton(this);
        OK->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        connect(OK, SIGNAL(clicked()), this, SLOT(OKC()));
        Layout->addWidget(Tree, 0, 0, 1, 2);
        Layout->addWidget(Cancel, 1, 0);
        Layout->addWidget(OK, 1, 1);
        for (uint i = 0; i < vec->size(); i++) {
            if ((*vec)[i]->Mbox_Top) {
                QTreeWidgetItem* Item = new QTreeWidgetItem(Tree);
                Item->setText(0, (*vec)[i]->Mbox_Showname);
                QItemCheckBox* W = new QItemCheckBox(this, i);
                W->setChecked((*vec)[i]->Mbox_Refresh);
                if ((*vec)[i]->Mbox_Attrybutes & Mailbox::Noselect) {
                    W->setDisabled(true);
                } else if ((*vec)[i]->Mbox_Attrybutes & Mailbox::INBOX) {
                    W->setChecked(true);
                }
                Tree->setItemWidget(Item, 1, W);
                AddChildren((*vec)[i], Item);
            }
        }
        Tree->expandAll();
    }
    ~MailboxTree() {
        vec = NULL;
    }
    void AddChildren(Mailbox* I, QTreeWidgetItem* T) {
        for (uint i = 0; i < I->Mbox_Children.size(); i++) {
            QTreeWidgetItem* ItemC = new QTreeWidgetItem(T);
            ItemC->setText(0, I->Mbox_Children[i]->Mbox_Showname);
            QItemCheckBox* W = new QItemCheckBox(this, std::find(vec->begin(), vec->end(), I->Mbox_Children[i])-vec->begin());
            W->setChecked(I->Mbox_Children[i]->Mbox_Refresh);
            if (I->Mbox_Children[i]->Mbox_Attrybutes & Mailbox::Noselect) {
                W->setDisabled(true);
            } else if (I->Mbox_Children[i]->Mbox_Attrybutes & Mailbox::INBOX) {
                W->setChecked(true);
            }
            Tree->setItemWidget(ItemC, 1, W);
            QItemCheckBox* ParentCheckBox = qobject_cast  < QItemCheckBox* > (Tree->itemWidget(T, 1));
            connect(ParentCheckBox, SIGNAL(toggled(bool)), W, SLOT(setChecked(bool)));
            AddChildren(I->Mbox_Children[i], ItemC);
        }
    }
private:
    QTreeWidget* Tree;
    vector  <Mailbox*>* vec;
    QPushButton* Cancel;
    QPushButton* OK;
    void OKChildren(QTreeWidgetItem* Itm) {
        for (int i = 0; i < Itm->childCount(); i++) {
            OKChildren(Itm->child(i));
        }
        QItemCheckBox* W = qobject_cast<QItemCheckBox*>(Tree->itemWidget(Itm, 1));
        (*vec)[W->ItemID]->Mbox_Refresh = W->isChecked();
    }
private slots:
    void OKC() {
        for (int i = 0; i < Tree->topLevelItemCount(); i++) {
            OKChildren(Tree->topLevelItem(i));
        }
        this->accept();
    }
    void CancelC() {
        this->reject();
    }
};
class EditDialog :public QDialog {
    Q_OBJECT
public:
    EditDialog(Mail* A, QWidget* parent) :QDialog(parent) {
        M = A;
        setWindowTitle("Edit mail settings.");
        setWindowIcon(QIcon(":/main/QOrganizer.png"));
        User = new QLabel("Username: ", this);
        Passwd = new QLabel("Password: ", this);
        IMAP = new QLabel("IMAP Server: ", this);
        SMTP = new QLabel("SMTP Server: ", this);
        U = new QLineEdit(A->User, this);
        P = new QLineEdit(this);
        P->setEchoMode(QLineEdit::Password);
        I = new QLineEdit(A->IMAPserver, this);
        S = new QLineEdit(A->SMTPserver, this);
        OKB = new QPushButton(this);
        OKB->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        connect(OKB, SIGNAL(clicked()), this, SLOT(OK()));
        Cancel = new QPushButton(this);
        Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(Cancel, SIGNAL(clicked()), this, SLOT(Can()));
        QGridLayout* L = new QGridLayout(this);
        L->addWidget(User, 0, 0);
        L->addWidget(U, 0, 1);
        L->addWidget(Passwd, 1, 0);
        L->addWidget(P, 1, 1);
        L->addWidget(IMAP, 2, 0);
        L->addWidget(I, 2, 1);
        L->addWidget(SMTP, 3, 0);
        L->addWidget(S, 3, 1);
        QHBoxLayout* H = new QHBoxLayout();
        H->addWidget(Cancel);
        H->addWidget(OKB);
        L->addLayout(H, 4, 1, 1, 2);
    }
private:
    Mail* M;
    QLabel* User;
    QLabel* Passwd;
    QLabel* IMAP;
    QLabel* SMTP;
    QLineEdit* U;
    QLineEdit* P;
    QLineEdit* I;
    QLineEdit* S;
    QPushButton* OKB;
    QPushButton* Cancel;
private slots:
    void OK() {
        if (U->text().isEmpty()) {
            if (U->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(U, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            U->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else if (P->text().isEmpty()) {
            if (P->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(P, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            P->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else if (I->text().isEmpty()) {
            if (I->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(I, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            I->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else if (S->text().isEmpty()) {
            if (S->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(S, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            S->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            M->Name = U->text();
            M->User = U->text();
            M->Password = QString(calculateXOR(P->text().toUtf8(), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)).toBase64());;
            P->clear();
            M->IMAPserver = I->text();
            M->SMTPserver = S->text();
            this->accept();
        }
    }
    void Can() {
        this->reject();
    }
    void row(QString Input) {
        QLineEdit* I = qobject_cast<QLineEdit*>(QObject::sender());
        if (Input.isEmpty()) {
            I->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            I->setStyleSheet("QQLineEdit{background: white;}");
        }
    }
};
class Sender :public QDialog {
    Q_OBJECT
public:
    enum Type {
        Normal,
        Reply,
        ReplyAll,
        Forward
    };
    Sender(Email* E, Type I, QStringList Z, SSLCON* SSL, QWidget* parent) :QDialog(parent) {
        setWindowTitle("Send Mail");
        setWindowIcon(QIcon(":/main/QOrganizer.png"));
        setMinimumWidth(600);
        this->SSL = SSL;
        this->Z = Z;
        Descriptions[0] = new QLabel("Subject: ", this);
        Descriptions[1] = new QLabel("To: ", this);
        Descriptions[2] = new QLabel("CC: ", this);
        Descriptions[3] = new QLabel("BCC: ", this);
        Header.append("From: "+SSL->M->User.mid(0, SSL->M->User.indexOf("@"))+"@"+SSL->M->IMAPserver.mid(SSL->M->IMAPserver.indexOf(".")+1, SSL->M->IMAPserver.length()-SSL->M->IMAPserver.indexOf(".")-1)+"\r\n");
        Header.append("Reply-To: "+SSL->M->User.mid(0, SSL->M->User.indexOf("@"))+"@"+SSL->M->IMAPserver.mid(SSL->M->IMAPserver.indexOf(".")+1, SSL->M->IMAPserver.length()-SSL->M->IMAPserver.indexOf(".")-1)+"\r\n");
        Header.append("Message-ID: <"+QCryptographicHash::hash(QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8(), QCryptographicHash::Md5).toHex()+".QOrganizer@localhost>\r\n");

        Subject = new QLineEdit(this);
        To = new QLineEdit(this);
        connect(To, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
        CC = new QLineEdit(this);
        connect(CC, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
        BCC = new QLineEdit(this);
        connect(BCC, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

        Text = new QTextBrowser(this);
        Text->setReadOnly(false);
        Text->setAcceptRichText(false);

        Html = new QWebView(this);
        Html->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        connect(Html->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
        Html->setHtml(E->Email_Body[1]);

        HtmlEdit = new QTextBrowser(this);
        HtmlEdit->setReadOnly(false);
        HtmlEdit->setAcceptRichText(false);
        HtmlEdit->setPlainText(E->Email_Body[1]);

        Attachments = new QListWidget(this);
        Attachments->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        connect(Attachments, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DelAt(QModelIndex)));
        AddAttchment = new QPushButton(this);
        AddAttchment->setIcon(QIcon(":/main/Add.png"));
        connect(AddAttchment, SIGNAL(clicked()), this, SLOT(AAt()));

        OKB = new QPushButton(this);
        OKB->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        connect(OKB, SIGNAL(clicked()), this, SLOT(OK()));
        Cancel = new QPushButton(this);
        Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(Cancel, SIGNAL(clicked()), this, SLOT(Can()));

        QTabWidget* Tab = new QTabWidget(this);
        connect(Tab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
        Tab->addTab(Text, "Text edit");
        Tab->addTab(Html, "HTML");
        Tab->addTab(HtmlEdit, "HTML edit");
        QGridLayout* L = new QGridLayout(this);
        L->addWidget(Descriptions[0], 0, 0);
        L->addWidget(Subject, 0, 1, 1, 2);
        L->addWidget(Descriptions[1], 1, 0);
        L->addWidget(To, 1, 1, 1, 2);
        L->addWidget(Descriptions[2], 2, 0);
        L->addWidget(CC, 2, 1, 1, 2);
        L->addWidget(Descriptions[3], 3, 0);
        L->addWidget(BCC, 3, 1, 1, 2);
        L->addWidget(Tab, 4, 0, 3, 3);
        QHBoxLayout* A = new QHBoxLayout();
        A->addWidget(Attachments);
        A->addWidget(AddAttchment);
        L->addLayout(A, 7, 0, 2, 3);
        QHBoxLayout* B = new QHBoxLayout();
        B->addWidget(Cancel);
        B->addWidget(OKB);
        L->addLayout(B, 9, 0, 1, 3);

        QString Str = E->Email_Body[0];
        Str.replace("<", "&lt;");
        Str.replace(">", "&gt;");
        Str.replace("&amp;", "&");
        QTextDocument D;
        D.setHtml(Str);
        Str = D.toPlainText();

        if (I == Reply) {
            Subject->setText("Re: "+E->Email_Subject);
            Header.append("In-Reply-To: <"+E->Email_MessageID+">\r\n");
            if (!E->Email_ReplyTo.isEmpty()) {
                To->setText(E->Email_ReplyTo);
            } else if (!(E->Email_From.contains("noreply") || E->Email_From.contains("no-reply"))) {
                To->setText(E->Email_From);
            }
            Text->setText("Message replied:\n----------\n"+Str+"\n----------\n");
        } else if (I == ReplyAll) {
            Subject->setText("Re: "+E->Email_Subject);
            Header.append("In-Reply-To: <"+E->Email_MessageID+">\r\n");
            QString StringTo;
            QString StringCC;
            if (!E->Email_ReplyTo.isEmpty()) {
                StringTo = E->Email_ReplyTo;
            } else if (!(E->Email_From.contains("noreply") || E->Email_From.contains("no-reply"))) {
                StringTo = E->Email_From;
            }
            for (uint i =  0; i < E->Email_RecipientsTo.size(); i++) {
                StringTo.append(","+E->Email_RecipientsTo[i]);
            }
            for (uint i =  0; i < E->Email_RecipientsCC.size(); i++) {
                StringCC.append(E->Email_RecipientsCC[i]);
                if (i != E->Email_RecipientsCC.size()-1) {
                    StringCC.append(",");
                }
            }
            To->setText(StringTo);
            CC->setText(StringCC);
            Text->setText("Message replied:\n----------\n"+Str+"\n----------\n");
        } else if (I == Forward) {
            Subject->setText("Fwd: "+E->Email_Subject);
            Text->setText("Message forwarded:\n----------\n"+Str+"\n----------\n");
        }
        for (uint i = 0; i < E->Structurev.size(); i++) {
            if (!(E->Structurev[i]->Structure_Type == "TEXT" && E->Structurev[i]->Structure_Disposition.isEmpty())) {
                QListWidgetItem* Itm = new QListWidgetItem(Attachments);
                if (E->Structurev[i]->Structure_Attrybutes.Name.isEmpty()) {
                    if (E->Structurev[i]->Structure_CID.isEmpty()) {
                        Itm->setText("attachment."+E->Structurev[i]->Structure_Subtype.toLower());
                    } else {
                        Itm->setText(E->Structurev[i]->Structure_CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                    }
                } else {
                    Itm->setText(NameFilter(E->Structurev[i]->Structure_Attrybutes.Name));
                }
                Itm->setData(Qt::UserRole, QString::number(i));
            }
        }


    }
private:
    SSLCON* SSL;
    QStringList Z;
    QLabel* Descriptions[4];
    QString Header;
    QLineEdit* Subject;
    QLineEdit* To;
    QCompleter* ToC;
    QLineEdit* CC;
    QCompleter* CCC;
    QLineEdit* BCC;
    QCompleter* BCCC;
    QTextBrowser* Text;
    QWebView* Html;
    QTextBrowser* HtmlEdit;
    QListWidget* Attachments;
    QPushButton* AddAttchment;
    QPushButton* OKB;
    QPushButton* Cancel;

private slots:
    void OK() {
        if (Subject->text().isEmpty()) {
            if (Subject->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(Subject, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            Subject->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else if (To->text().isEmpty() && CC->text().isEmpty() && BCC->text().isEmpty()) {
            if (To->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(To, SIGNAL(textChanged(QString)), this, SLOT(emailrow(QString)));
                connect(CC, SIGNAL(textChanged(QString)), this, SLOT(emailrow(QString)));
                connect(BCC, SIGNAL(textChanged(QString)), this, SLOT(emailrow(QString)));
            }
            To->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            QString Months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            Header.append("Subject: =?UTF-8?B?"+Subject->text().toUtf8().toBase64()+"?=\r\n");
            Header.append("Date: "+QDateTime::currentDateTime().toString("dd ")+Months[QDateTime::currentDateTime().date().month()-1]+QDateTime::currentDateTime().toString(" yyyy hh:mm:ss")+"\r\n");
            QList <QString> RcptL;
            QList <QString> ToL = To->text().split(",", QString::SkipEmptyParts);
            QList <QString> CcL = CC->text().split(",", QString::SkipEmptyParts);
            QList <QString> BccL = BCC->text().split(",", QString::SkipEmptyParts);
            if (!ToL.isEmpty()) {
                Header.append("To: ");
                for (int i = 0; i < ToL.size(); i++) {
                    RcptL.append(ToL[i]);
                    Header.append(ToL[i]);
                    if (i+1 != ToL.size()) {
                        Header.append(",");
                    } else {
                        Header.append("\r\n");
                    }
                }
            }
            if (!CcL.isEmpty()) {
                Header.append("CC: ");
                for (int i = 0; i < CcL.size(); i++) {
                    RcptL.append(CcL[i]);
                    Header.append(CcL[i]);
                    if (i+1 != CcL.size()) {
                        Header.append(",");
                    } else {
                        Header.append("\r\n");
                    }
                }
            }
            for (int i = 0; i < BccL.size(); i++) {
                RcptL.append(BccL[i]);
            }
            QList  <QString>  Att;
            for (int i = 0; i < Attachments->count(); i++) {
                Att.append(Attachments->item(i)->data(Qt::UserRole).toString());
            }
            if (Html->page()->mainFrame()->toHtml() != "<html><head></head><body></body></html>") {
                SSL->setData(QList <QString>() << Header << "" << RcptL << "" << Text->toPlainText() << Html->page()->mainFrame()->toHtml() << "" << Att);
            } else {
                SSL->setData(QList <QString>() << Header << "" << RcptL << "" << Text->toPlainText() << "" << Att);
            }
            SSL->setMethod(SSLCON::Send);
            SSL->start();
            this->accept();
        }
    }
    void Can() {
        this->reject();
    }
    void AAt() {
        QString Path = QFileDialog::getOpenFileName(this, "Add attachment", QDir::homePath());
        if (!Path.isEmpty()) {
            bool OK = true;
            for (int i = 0; i < Attachments->count(); i++) {
                if (Attachments->item(i)->data(Qt::UserRole) == Path) {
                    OK = false;
                    break;
                }
            }
            if (OK) {
                QString Name = Path.mid(Path.lastIndexOf(QDir::separator())+1, Path.length()-Path.lastIndexOf(QDir::separator())-1);
                QListWidgetItem* Itm = new QListWidgetItem(Name);
                Itm->setData(Qt::UserRole, "<"+Path+">");
                Attachments->addItem(Itm);
                disconnect(Subject, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
        }
    }
    void DelAt(QModelIndex I) {
        delete Attachments->item(I.row());
    }
    void row(QString Input) {
        QLineEdit* I = qobject_cast<QLineEdit*>(QObject::sender());
        if (Input.isEmpty()) {
            I->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            I->setStyleSheet("QQLineEdit{background: white;}");
        }
    }
    void emailrow(QString Input) {
        if (Input.isEmpty()) {
            To->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            To->setStyleSheet("QQLineEdit{background: white;}");
        }
    }
    void linkClicked(QUrl link) {
        QDesktopServices::openUrl(link);
    }
    void tabChanged(int I) {
        if (I == 1) {
            Html->setHtml(HtmlEdit->toPlainText());
        }
    }
    void textChanged(QString I) {
        if (!I.isEmpty()) {
            QLineEdit* E = qobject_cast<QLineEdit*>(QObject::sender());
            QString text = I.mid(0, E->cursorPosition());
            QString newWord;
            QString prefix;
            if (text.contains(",")) {
                newWord = text.mid(text.lastIndexOf(",")+1, text.length()-text.lastIndexOf(",")-1);
                prefix = text.mid(0, text.lastIndexOf(",")+1);
            } else {
                newWord = text;
            }
            if (!newWord.isEmpty()) {
                for (int i = 0; i < Z.size(); i++) {
                    if (Z[i].startsWith(newWord)) {
                        E->setText(prefix+newWord+Z[i].mid(newWord.length(), Z[i].length()-newWord.length()));
                        E->setCursorPosition(text.length());
                        return;
                    }
                }
                E->setText(prefix+newWord);
            } else {
                E->setText(prefix);
            }
        }
    }
};


qorgMail::qorgMail(QWidget* parent, qorgAB* AdressBook, qorgOptions* Options) :QWidget(parent) {
    this->AdressBook = AdressBook;
    this->Options = Options;
    currentMail = -1;
    mailboxAction = false;
    emailAction = false;
    AvailableServices.append(Services("AOL", "imap.aol.com", "smtp.aol.com"));
    AvailableServices.append(Services("Gmail", "imap.gmail.com", "smtp.gmail.com"));
    AvailableServices.append(Services("Interia.pl", "imap.interia.pl", "smtp.interia.pl"));
    AvailableServices.append(Services("Linux.pl", "imap.linux.pl", "smtp.linux.pl"));
    AvailableServices.append(Services("Mail.ru", "imap.mail.ru", "smtp.mail.ru"));
    AvailableServices.append(Services("o2.pl", "poczta.o2.pl", "poczta.o2.pl"));
    AvailableServices.append(Services("Outlook.com", "imap.outlook.com", "none"));
    AvailableServices.append(Services("Wp.pl", "imap.wp.pl", "smtp.wp.pl"));
    AvailableServices.append(Services("Yahoo!", "imap.mail.yahoo.com", "smtp.mail.yahoo.com"));
    Layout = new QGridLayout(this);
    Layout->setMargin(0);

    Mailboxes = new QTreeWidget(this);
    Mailboxes->header()->hide();
    Mailboxes->setFixedWidth(110);
    Mailboxes->setIndentation(10);
    Mailboxes->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Mailboxes, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(chooseMbox(QTreeWidgetItem*)));
    connect(Mailboxes, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(MailboxesMenu(QPoint)));

    MailView = new QTreeWidget(this);
    MailView->setColumnCount(3);
    MailView->setHeaderLabels(QStringList() << "Topic" << "From" << "Date");
    MailView->header()->setStretchLastSection(false);
    MailView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    MailView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    MailView->setColumnWidth(2, 120);
    MailView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(MailView, SIGNAL(clicked(QModelIndex)), this, SLOT(chooseEmail(QModelIndex)));
    connect(MailView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(MailViewMenu(QPoint)));

    Tabs = new QTabWidget(this);
    Tabs->setMinimumHeight(30);
    MailText = new QTextBrowser(this);
    MailText->hide();
    MailText->setReadOnly(true);
    MailText->setAcceptRichText(false);
    MailHtml = new QWebView(this);
    MailHtml->hide();
    MailHtml->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(MailHtml->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, QList <QSslError>)),
            this, SLOT(HTTPSS(QNetworkReply*, QList <QSslError>)));
    connect(MailHtml->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    AttachmentList = new QListWidget(this);
    AttachmentList->setMinimumHeight(30);
    connect(AttachmentList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(downloadAttachment(QModelIndex)));
    InternalSplitter = new QSplitter(Qt::Vertical, this);
    InternalSplitter->addWidget(Tabs);
    InternalSplitter->addWidget(AttachmentList);
    InternalSplitter->setSizes(QList<int>() << 177 <<30);
    Split = new QSplitter(Qt::Vertical, this);
    Split->addWidget(MailView);
    Split->addWidget(InternalSplitter);

    Refresh = new QPushButton(QIcon(":/main/Refresh.png"), "", this);
    Refresh->setToolTip("Refresh mail");
    connect(Refresh, SIGNAL(clicked()), this, SLOT(RefreshS()));
    Send = new QPushButton(QIcon(":/main/Mail.png"), "", this);
    Send->setToolTip("Send email");
    connect(Send, SIGNAL(clicked()), this, SLOT(SendEmail()));
    Delete = new QPushButton(QIcon(":/main/Delete.png"), "", this);
    Delete->setToolTip("Delete email");
    connect(Delete, SIGNAL(clicked()), this, SLOT(DeleteEmail()));
    Reply = new QPushButton(QIcon(":/mail/Reply.png"), "", this);
    Reply->setToolTip("Reply to email");
    connect(Reply, SIGNAL(clicked()), this, SLOT(SendEmail()));
    ReplyAll = new QPushButton(QIcon(":/mail/ReplyAll.png"), "", this);
    ReplyAll->setToolTip("Reply everyone to email");
    connect(ReplyAll, SIGNAL(clicked()), this, SLOT(SendEmail()));
    Forward = new QPushButton(QIcon(":/mail/Forward.png"), "", this);
    Forward->setToolTip("Forward email");
    connect(Forward, SIGNAL(clicked()), this, SLOT(SendEmail()));
    F << Mailboxes << MailView << Split << Refresh << Send << Delete << Reply << ReplyAll << Forward;
    List = new QTreeWidget(this);
    List->header()->hide();
    List->setColumnCount(3);
    List->header()->setStretchLastSection(false);
    List->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    List->setColumnWidth(1, 50);
    List->setColumnWidth(2, 50);
    connect(List, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(Click(QModelIndex)));
    Labels[0]=new QLabel("Username: ", this);
    Labels[1]=new QLabel("Password: ", this);
    Labels[2]=new QLabel("Serwers ", this);
    Labels[3]=new QLabel("IMAP: ", this);
    Labels[4]=new QLabel("SMTP: ", this);
    Username = new QLineEdit(this);
    Passwd = new QLineEdit(this);
    Passwd->setEchoMode(QLineEdit::Password);
    IMAPS = new QLineEdit(this);
    SMTPS = new QLineEdit(this);
    Choose = new QComboBox(this);
    connect(Choose, SIGNAL(currentIndexChanged(int)), this, SLOT(change(int)));
    for (int i = 0; i < AvailableServices.size(); i++) {
        Choose->addItem(AvailableServices[i].Name);
    }
    Choose->addItem("Other...");
    AddB = new QPushButton(QIcon(":/main/Add.png"), "Add", this);
    AddB->setShortcut(Qt::Key_Return);;
    connect(AddB, SIGNAL(clicked()), this, SLOT(testInput()));
    C << List << Labels[0] << Labels[1] << Labels[2] << Labels[3] << Labels[4] << Labels[5] << Username << Passwd << IMAPS << SMTPS << Choose << AddB;
    setMail(-1);
}
qorgMail::~qorgMail() {
    for (uint i = 0; i < Mailv.size(); i++) {
        for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
            for (uint k = 0; k < Mailv[i].Mboxv[j]->Emailv.size(); k++) {
                for (uint l = 0; l < Mailv[i].Mboxv[j]->Emailv[k]->Structurev.size(); l++) {
                    delete Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l];
                }
                delete Mailv[i].Mboxv[j]->Emailv[k];
            }
            delete Mailv[i].Mboxv[j];
        }
    }
    MailHtml->page()->deleteLater();
}
// Public
QString qorgMail::output() {
    QString out;
    for (uint i = 0; i < Mailv.size(); i++) {
        out.append(Output(Mailv[i].Name)+" ");
        out.append(Output(Mailv[i].IMAPserver)+" ");
        out.append(Output(Mailv[i].SMTPserver)+" ");
        out.append(Output(Mailv[i].User)+" ");
        out.append(Output(Mailv[i].Password)+" \n");
        for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
            out.append(Output(Mailv[i].Mboxv[j]->Mbox_Name)+" ");
            out.append(Output(Mailv[i].Mboxv[j]->Mbox_Showname)+" ");
            out.append(Output(Mailv[i].Mboxv[j]->Mbox_Attrybutes)+" ");
            out.append(Output(Mailv[i].Mboxv[j]->Mbox_Refresh)+" ");
            out.append(Output(Mailv[i].Mboxv[j]->Mbox_Top)+" ");
            QString Childrens;
            for (uint k = 0; k < Mailv[i].Mboxv[j]->Mbox_Children.size(); k++) {
                for (uint l = 0; l < Mailv[i].Mboxv.size(); l++) {
                    if (Mailv[i].Mboxv[j]->Mbox_Children[k] == Mailv[i].Mboxv[l]) {
                        Childrens.append(Output(l)+",");
                        break;
                    }
                }
            }
            out.append(Childrens+" \n");
            for (uint k = 0; k < Mailv[i].Mboxv[j]->Emailv.size(); k++) {
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_Subject)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_From)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_Date)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_Body[0])+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_Body[1])+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_UID)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_Flags)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_MessageID)+" ");
                out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_ReplyTo)+" ");
                QString To;
                for (uint l = 0; l < Mailv[i].Mboxv[j]->Emailv[k]->Email_RecipientsTo.size(); l++) {
                    To.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_RecipientsTo[l])+",");
                }
                out.append(To+" ");
                QString CC;
                for (uint l = 0; l < Mailv[i].Mboxv[j]->Emailv[k]->Email_RecipientsCC.size(); l++) {
                    CC.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Email_RecipientsCC[l])+",");
                }
                out.append(CC+" \n");
                for (uint l = 0; l < Mailv[i].Mboxv[j]->Emailv[k]->Structurev.size(); l++) {
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Number)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Type)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Subtype)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Attrybutes.Charset)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Attrybutes.Name)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_CID)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Descryption)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Encoding)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Size)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Lines)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_MD5)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Disposition)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Language)+" ");
                    out.append(Output(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Location)+" \n");
                }
            }
        }
    }
    out.append("\n\n");
    return out;
}
void qorgMail::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        vector < vector <uint> > MailConnections;
        Mail* cMail;
        Mailbox* cMailbox;
        Email* cEmail;
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 5: {
                if (Mailv.size() > 0) {
                    for (uint i = 0; i < Mailv.back().Mboxv.size(); i++) {
                        for (uint j = 0; j < MailConnections[i].size(); j++) {
                            Mailv.back().Mboxv[i]->Mbox_Children.push_back(Mailv.back().Mboxv[MailConnections[i][j]]);
                        }
                    }
                    MailConnections.clear();
                }
                Mailv.push_back(Mail());
                cMail = &Mailv.back();
                cMail->Name = InputS(B[0]);
                cMail->IMAPserver = InputS(B[1]);
                cMail->SMTPserver = InputS(B[2]);
                cMail->User = InputS(B[3]);
                cMail->Password = InputS(B[4]);
            }break;
            case 6: {
                cMailbox = new Mailbox();
                cMail->Mboxv.push_back(cMailbox);
                cMailbox->Mbox_Name = InputS(B[0]);
                cMailbox->Mbox_Showname = InputS(B[1]);
                cMailbox->Mbox_Attrybutes = InputI(B[2]);
                cMailbox->Mbox_Refresh = InputB(B[3]);
                cMailbox->Mbox_Top = InputB(B[4]);
                QStringList C = B[5].split(",", QString::SkipEmptyParts);
                vector <uint> Temp;
                for (int i = 0; i < C.size(); i++) {
                    Temp.push_back(static_cast<uint>(C[i].toInt()));
                }
                MailConnections.push_back(Temp);
            }break;
            case 11: {
                cEmail = new Email();
                cMailbox->Emailv.push_back(cEmail);
                cEmail->Email_Subject = InputS(B[0]);
                cEmail->Email_From = InputS(B[1]);
                cEmail->Email_Date = InputDT(B[2]);
                cEmail->Email_Body[0] = InputS(B[3]);
                cEmail->Email_Body[1] = InputS(B[4]);
                cEmail->Email_UID = InputI(B[5]);
                cEmail->Email_Flags = InputC(B[6]);
                cEmail->Email_MessageID = InputS(B[7]);
                cEmail->Email_ReplyTo = InputS(B[8]);
                QStringList C = B[9].split(",", QString::SkipEmptyParts);
                for (int i = 0; i < C.size(); i++) {
                    cEmail->Email_RecipientsTo.push_back(InputS(C[i]));
                }
                QStringList D = B[10].split(",", QString::SkipEmptyParts);
                for (int i = 0; i < D.size(); i++) {
                    cEmail->Email_RecipientsCC.push_back(InputS(D[i]));
                }
            }break;
            case 14: {
                Structure* cStruct = new Structure();
                cEmail->Structurev.push_back(cStruct);
                cStruct->Structure_Number = InputS(B[0]);
                cStruct->Structure_Type = InputS(B[1]);
                cStruct->Structure_Subtype = InputS(B[2]);
                cStruct->Structure_Attrybutes.Charset = InputS(B[3]);
                cStruct->Structure_Attrybutes.Name = InputS(B[4]);
                cStruct->Structure_CID = InputS(B[5]);
                cStruct->Structure_Descryption = InputS(B[6]);
                cStruct->Structure_Encoding = InputS(B[7]);
                cStruct->Structure_Size = InputI(B[8]);
                cStruct->Structure_Lines = InputI(B[9]);
                cStruct->Structure_MD5 = InputS(B[10]);
                cStruct->Structure_Disposition = InputS(B[11]);
                cStruct->Structure_Language = InputS(B[12]);
                cStruct->Structure_Location = InputS(B[13]);
            }break;
            }
        }
        for (uint i = 0; i < Mailv.back().Mboxv.size(); i++) {
            for (uint j = 0; j < MailConnections[i].size(); j++) {
                Mailv.back().Mboxv[i]->Mbox_Children.push_back(Mailv.back().Mboxv[MailConnections[i][j]]);
            }
        }
    }
    setLayoutC();
}
void qorgMail::setMail(int setMail) {
    if (setMail != currentMail) {
        currentMail = setMail;
        if (currentMail == -1) {
            setLayoutC();
        } else {
            currentMailbox = 0;
            currentEmail = -1;
            Mailboxes->clear();
            setLayoutF();
            int I = 0;
            for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
                if (Mailv[currentMail].Mboxv[i]->Mbox_Top) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem();
                    Itm->setText(0, Mailv[currentMail].Mboxv[i]->Mbox_Showname);
                    Itm->setToolTip(0, Mailv[currentMail].Mboxv[i]->Mbox_Showname);
                    Itm->setData(0, Qt::UserRole, i);
                    if (Mailv[currentMail].Mboxv[i]->Mbox_Attrybutes & Mailbox::Noselect) {
                        Itm->setFlags(Qt::ItemIsEnabled);
                    }
                    if (Mailv[currentMail].Mboxv[i]->Mbox_Children.size() != 0) {
                        addChildren(Mailv[currentMail].Mboxv[i], Itm);
                    }
                    Mailboxes->addTopLevelItem(Itm);
                }
                if (Mailv[currentMail].Mboxv[i]->Mbox_Attrybutes&Mailbox::INBOX) {
                    I = i;
                }
            }
            Mailboxes->expandAll();
            setMailbox(I);
        }
    }
}
QStringList qorgMail::getCategories() {
    QStringList list;
    for (uint i = 0; i < Mailv.size(); i++) {
        list.append(Mailv[i].Name);
    }
    return list;
}
uint qorgMail::threadNumber() {
    QMutexLocker L(&MAILaccessToCounter);
    return MAILthreadCounter;
}
bool qorgMail::SSLSocketError(QList<QSslError> I) {
    int response = Options->checkCertificate(I.first().certificate());
    if (response == 0) {
        Options->addForVeryfication(I.first().certificate());
        return false;
    } else if (response == 1) {
        return true;
    }
    return false;
}
void qorgMail::getUpdate() {
    UpdateQuene = 0;
    if (Mailv.size() != 0) {
        for (uint i = 0; i < Mailv.size(); i++) {
            Mail* M = new Mail();
            M->User = Mailv[i].User;
            M->Password = Mailv[i].Password;
            M->IMAPserver = Mailv[i].IMAPserver;
            for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
                if (Mailv[i].Mboxv[j]->Mbox_Attrybutes&Mailbox::INBOX) {
                    M->Mboxv.push_back(Mailv[i].Mboxv[j]);
                    break;
                }
            }
            SSLCON* T = new SSLCON(this, M);
            connect(T, SIGNAL(EmailS(bool, QString)), this, SLOT(UpdateS()));
            T->setMethod(SSLCON::Emails);
            T->start();
            ++UpdateQuene;
        }
    } else {
        emit sendUpdate("Mail: No mails.");
    }
}
// Private
void qorgMail::setMailbox(int I) {
    mailboxAction = false;
    emailAction = false;
    currentMailbox = I;
    MailView->clear();
    Tabs->clear();
    MailText->clear();
    MailHtml->setHtml("");
    AttachmentList->clear();
    for (int i = Mailv[currentMail].Mboxv[currentMailbox]->Emailv.size()-1; i > -1; i--) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem();
        if (!(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_Flags&Email::Seen)) {
            Itm->setFont(0, QFont("", Itm->font(0).pixelSize(), QFont::Bold));
        }
        Itm->setText(0, Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_Subject);
        Itm->setToolTip(0, Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_Subject);
        Itm->setText(1, Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_From);
        Itm->setToolTip(1, Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_From);
        Itm->setText(2, Mailv[currentMail].Mboxv[currentMailbox]->Emailv[i]->Email_Date.toString("dd/MM/yyyy hh:mm"));
        MailView->addTopLevelItem(Itm);
    }
    MailView->scrollToTop();
}
void qorgMail::setLayoutF() {
    for (int i = 0; i < C.size(); i++) {
        Layout->removeWidget(C[i]);
        C[i]->hide();
    }
    Layout->addWidget(Refresh, 0, 0);
    Layout->addWidget(Send, 0, 1);
    Layout->addWidget(Delete, 0, 2);
    Layout->addWidget(Reply, 0, 3);
    Layout->addWidget(ReplyAll, 0, 4);
    Layout->addWidget(Forward, 0, 5);
    Layout->addWidget(Mailboxes, 0, 6, 2, 1);
    Layout->addWidget(Split, 1, 0, 1, 6);
    for (int i = 0; i < F.size(); i++) {
        F[i]->show();
    }
}
void qorgMail::addChildren(Mailbox* I, QTreeWidgetItem* W) {
    for (uint i = 0; i < I->Mbox_Children.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem();
        Itm->setText(0, I->Mbox_Children[i]->Mbox_Showname);
        Itm->setToolTip(0, I->Mbox_Children[i]->Mbox_Showname);
        Itm->setData(0, Qt::UserRole, static_cast<uint>(std::find(Mailv[currentMail].Mboxv.begin(), Mailv[currentMail].Mboxv.end(), I->Mbox_Children[i])-Mailv[currentMail].Mboxv.begin()));
        if (I->Mbox_Children[i]->Mbox_Attrybutes & Mailbox::Noselect) {
            Itm->setFlags(Qt::ItemIsEnabled);
        }
        if (I->Mbox_Children[i]->Mbox_Children.size() != 0) {
            addChildren(I->Mbox_Children[i], Itm);
        }
        W->addChild(Itm);
    }
}
void qorgMail::setLayoutC() {
    List->clear();
    for (uint i = 0; i < Mailv.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(List);
        Itm->setText(0, Mailv[i].Name);
        QItemPushButton* Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, i);
        Edit->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Edit, SIGNAL(clicked(uint)), this, SLOT(EditMail(uint)));
        List->setItemWidget(Itm, 1, Edit);
        QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, i);
        connect(Delete, SIGNAL(clicked(uint)), this, SLOT(DeleteMail(uint)));
        List->setItemWidget(Itm, 2, Delete);
    }
    for (int i = 0; i < F.size(); i++) {
        Layout->removeWidget(F[i]);
        F[i]->hide();
    }
    Layout->addWidget(List, 0, 0, 2, 1);
    QGridLayout* ADD = new QGridLayout();
    ADD->addWidget(Labels[0], 0, 0);
    ADD->addWidget(Username, 0, 1, 1, 2);
    ADD->addWidget(Labels[1], 1, 0);
    ADD->addWidget(Passwd, 1, 1, 1, 2);
    ADD->addWidget(Labels[2], 2, 0);
    ADD->addWidget(Choose, 2, 1, 1, 2);
    ADD->addWidget(Labels[3], 3, 1);
    ADD->addWidget(IMAPS, 3, 2);
    ADD->addWidget(Labels[4], 4, 1);
    ADD->addWidget(SMTPS, 4, 2);
    ADD->addWidget(AddB, 5, 0, 1, 3);
    Layout->addLayout(ADD, 1, 1);
    for (int i = 0; i < C.size(); i++) {
        C[i]->show();
    }
}
// Private slots
// Categories
void qorgMail::testInput() {
    if (Username->text().isEmpty()) {
        if (Username->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(Username, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        Username->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (Passwd->text().isEmpty()) {
        if (Passwd->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(Passwd, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        Passwd->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (IMAPS->text().isEmpty()) {
        if (IMAPS->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(IMAPS, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        IMAPS->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else if (SMTPS->text().isEmpty()) {
        if (SMTPS->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(SMTPS, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        SMTPS->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        disconnect(Username, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(Passwd, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(IMAPS, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        disconnect(SMTPS, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        Mail* M = new Mail;
        bool EX = false;
        for (uint i = 0; i < Mailv.size(); i++) {
            if (Mailv[i].IMAPserver == IMAPS->text()&&Mailv[i].User == Username->text()) {
                EX = true;
                delete M;
                break;
            }
        }
        if (!EX) {
            M->User = Username->text();
            M->Password = QString(calculateXOR(Passwd->text().toUtf8(), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
            M->IMAPserver = IMAPS->text();
            M->SMTPserver = SMTPS->text();
            M->Name = M->User+" ("+M->IMAPserver+")";
            SSLCON* T = new SSLCON(this, M);
            connect(T, SIGNAL(LoginS(bool, QString)), this, SLOT(LoginS(bool, QString)));
            connect(T, SIGNAL(MailboxesS(bool, QString)), this, SLOT(MailboxesS(bool, QString)));
            connect(T, SIGNAL(EmailS(bool, QString)), this, SLOT(EmailS(bool, QString)));
            QProgressDialog* Bar = new QProgressDialog();
            Bar->setAutoReset(false);
            Bar->setWindowTitle("Add mail bar");
            Bar->setWindowIcon(QIcon(":/main/QOrganizer.png"));
            connect(T, SIGNAL(changeValue(int)), Bar, SLOT(setValue(int)));
            connect(T, SIGNAL(end()), Bar, SLOT(close()));
            connect(Bar, SIGNAL(canceled()), T, SLOT(Canceled()));
            Bar->setAttribute(Qt::WA_DeleteOnClose);
            T->setMethod(SSLCON::Login);
            T->start();
            Bar->exec();
        } else {
            QMessageBox::critical(this, "Mail error", "Mail already exist.");
        }
        Username->clear();
        Choose->setCurrentIndex(0);
    }
    Passwd->clear();
}
void qorgMail::row(QString Input) {
    QLineEdit* I = qobject_cast<QLineEdit*>(QObject::sender());
    if (Input.isEmpty()) {
        I->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        I->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgMail::change(int i) {
    if (i < AvailableServices.size()) {
        IMAPS->setText(AvailableServices[i].IMAP);
        SMTPS->setText(AvailableServices[i].SMTP);
    } else {
        IMAPS->clear();
        SMTPS->clear();
    }
}
void qorgMail::Click(QModelIndex I) {
    setMail(I.row());
    emit doubleClick(Mailv[I.row()].Name);
}
void qorgMail::DeleteMail(uint IID) {
    Mail* I=&Mailv[IID];
    for (uint j = 0; j < I->Mboxv.size(); j++) {
        for (uint k = 0; k < I->Mboxv[j]->Emailv.size(); k++) {
            for (uint l = 0; l < I->Mboxv[j]->Emailv[k]->Structurev.size(); l++) {
                delete I->Mboxv[j]->Emailv[k]->Structurev[l];
            }
            delete I->Mboxv[j]->Emailv[k];
        }
        delete I->Mboxv[j];
    }
    Mailv.erase(Mailv.begin()+IID);
    currentMail = -1;
    sortMail();
}

void qorgMail::EditMail(uint IID) {
    Mail* I = &Mailv[IID];
    Mail* A = new Mail;
    A->User = I->User;
    A->IMAPserver = I->IMAPserver;
    A->SMTPserver = I->SMTPserver;
    if ((new EditDialog(A, this))->exec() == QDialog::Accepted) {
        currentMail = IID;
        SSLCON* T = new SSLCON(this, A);
        connect(T, SIGNAL(MailboxesS(bool, QString)), this, SLOT(EditMailS(bool)));
        QProgressDialog* Bar = new QProgressDialog();
        Bar->setAutoReset(false);
        Bar->setWindowTitle("Edit mail bar");
        Bar->setWindowIcon(QIcon(":/main/QOrganizer.png"));
        Bar->setAttribute(Qt::WA_DeleteOnClose);
        connect(T, SIGNAL(changeValue(int)), Bar, SLOT(setValue(int)));
        connect(T, SIGNAL(end()), Bar, SLOT(close()));
        connect(Bar, SIGNAL(canceled()), T, SLOT(Canceled()));
        T->setMethod(SSLCON::Mailboxes);
        T->start();
        Bar->exec();
    }
}
void qorgMail::EditMailS(bool I) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        Mailv[currentMail].Name = T->M->User+" ("+T->M->IMAPserver+")";
        Mailv[currentMail].IMAPserver = T->M->IMAPserver;
        Mailv[currentMail].SMTPserver = T->M->SMTPserver;
        Mailv[currentMail].User = T->M->User;
        Mailv[currentMail].Password = T->M->Password;
        bool change = false;
        if (T->M->Mboxv.size() != Mailv[currentMail].Mboxv.size()) {
            change = true;
        } else {
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                if (T->M->Mboxv[i]->Mbox_Name != Mailv[currentMail].Mboxv[i]->Mbox_Name) {
                    change = true;
                    break;
                }
            }
        }
        if (change) {
            vector  <Mailbox*>  A = Mailv[currentMail].Mboxv;
            vector  <Mailbox*>  B = T->M->Mboxv;
            for (uint i = 0; i < A.size(); i++) {
                bool found = false;
                for (uint j = 0; j < B.size(); j++) {
                    if (A[i]->Mbox_Name == B[j]->Mbox_Name&&(A[i]->Mbox_Attrybutes/4) == (B[j]->Mbox_Attrybutes/4)) {
                        B[j]->Emailv = A[i]->Emailv;
                        B[j]->Mbox_Refresh = A[i]->Mbox_Refresh;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    for (uint j = 0; j < A[i]->Emailv.size(); j++) {
                        for (uint k = 0; k < A[i]->Emailv[j]->Structurev.size(); k++) {
                            delete A[i]->Emailv[j]->Structurev[k];
                        }
                        delete A[i]->Emailv[j];
                    }
                }
                delete A[i];
            }
            Mailv[currentMail].Mboxv = B;
        }
        T->setMethod(SSLCON::Stop);
        if ((new MailboxTree(&Mailv[currentMail], this))->exec() == QDialog::Accepted) {
            T = new SSLCON(this, &Mailv[currentMail]);
            connect(T, SIGNAL(EmailS(bool, QString)), this, SLOT(UpdateMail(bool, QString)));
            QProgressDialog* Bar = new QProgressDialog();
            Bar->setAutoReset(false);
            Bar->setWindowTitle("Edit mail bar");
            Bar->setWindowIcon(QIcon(":/main/QOrganizer.png"));
            Bar->setAttribute(Qt::WA_DeleteOnClose);
            connect(T, SIGNAL(changeValue(int)), Bar, SLOT(setValue(int)));
            connect(T, SIGNAL(end()), Bar, SLOT(close()));
            connect(Bar, SIGNAL(canceled()), T, SLOT(Canceled()));
            T->setMethod(SSLCON::Emails);
            T->start();
            Bar->exec();
        }
    } else {
        T->setMethod(SSLCON::Stop);
        QMessageBox::critical(this, "Error", "Wrong settings.");
    }
}
// Main
// SSLCON signals slots
void qorgMail::LoginS(bool I, QString R) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        T->setMethod(SSLCON::Mailboxes);
    } else {
        T->setMethod(SSLCON::Stop);
        if (!T->Cancelled) {
            QMessageBox::critical(this, "Login error", R);
        }
    }
}
void qorgMail::MailboxesS(bool I, QString R) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        if ((new MailboxTree(T->M, this))->exec() == QDialog::Accepted) {
            T->setMethod(SSLCON::Emails);
        } else {
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                delete T->M->Mboxv[i];
            }
            T->setMethod(SSLCON::Stop);
        }
    } else {
        T->setMethod(SSLCON::Stop);
        if (!T->Cancelled) {
            QMessageBox::critical(this, "Mailboxes error", R);
        }
    }
}
void qorgMail::RefreshS(bool I) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (I && mailboxAction && (currentMail != -1) && ((T->M->User == Mailv[currentMail].User) && (T->M->IMAPserver == Mailv[currentMail].IMAPserver))) {
        bool change = false;
        if (T->M->Mboxv.size() != Mailv[currentMail].Mboxv.size()) {
            change = true;
        } else {
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                if (T->M->Mboxv[i]->Mbox_Name != Mailv[currentMail].Mboxv[i]->Mbox_Name ||
                        T->M->Mboxv[i]->Mbox_Attrybutes != Mailv[currentMail].Mboxv[i]->Mbox_Attrybutes) {
                    change = true;
                    break;
                }
            }
        }
        if (change) {
            vector  <Mailbox*>  A = Mailv[currentMail].Mboxv;
            vector  <Mailbox*>  B = T->M->Mboxv;
            for (uint i = 0; i < A.size(); i++) {
                bool found = false;
                for (uint j = 0; j < B.size(); j++) {
                    if (A[i]->Mbox_Name == B[j]->Mbox_Name&&(A[i]->Mbox_Attrybutes/4) == (B[j]->Mbox_Attrybutes/4)) {
                        B[j]->Emailv = A[i]->Emailv;
                        B[j]->Mbox_Refresh = A[i]->Mbox_Refresh;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    for (uint j = 0; j < A[i]->Emailv.size(); j++) {
                        for (uint k = 0; k < A[i]->Emailv[j]->Structurev.size(); k++) {
                            delete A[i]->Emailv[j]->Structurev[k];
                        }
                        delete A[i]->Emailv[j];
                    }
                }
                delete A[i];
            }
            Mailv[currentMail].Mboxv = B;
            while ((new MailboxTree(&Mailv[currentMail], this))->exec() != QDialog::Accepted) {
                QMessageBox::information(this, "Mailbox tree changed.", "Mailbox structure is different, please update.");
            }
        } else {
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                delete T->M->Mboxv[i];
            }
        }
        T->setMethod(SSLCON::Stop);
        T = new SSLCON(this, &Mailv[currentMail]);
        connect(T, SIGNAL(EmailS(bool, QString)), this, SLOT(UpdateMail(bool, QString)));
        QProgressDialog* Bar = new QProgressDialog();
        Bar->setAutoReset(false);
        Bar->setWindowTitle("Refresh bar");
        Bar->setWindowIcon(QIcon(":/main/QOrganizer.png"));
        Bar->setAttribute(Qt::WA_DeleteOnClose);
        connect(T, SIGNAL(changeValue(int)), Bar, SLOT(setValue(int)));
        connect(T, SIGNAL(end()), Bar, SLOT(close()));
        connect(Bar, SIGNAL(canceled()), T, SLOT(Canceled()));
        T->setMethod(SSLCON::Emails);
        T->start();
        Bar->show();
    } else {
        T->setMethod(SSLCON::Stop);
        mailboxAction = false;
    }
}
void qorgMail::EmailS(bool I, QString R) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        Mailv.push_back((*T->M));
        delete T->M;
        currentMail = Mailv.size()-1;
        sortMail();
    } else {
        for (uint i = 0; i < T->M->Mboxv.size(); i++) {
            delete T->M->Mboxv[i];
        }
        delete T->M;
        if (!T->Cancelled) {
            QMessageBox::critical(this, "Email error", R);
        }
    }
}
void qorgMail::UpdateMail(bool I, QString R) {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    if (!I) {
        if (!T->Cancelled) {
            QMessageBox::critical(this, "Mailboxes error", R);
        }
    }
    mailboxAction = false;
    sortMail();
}
void qorgMail::UpdateS() {
    SSLCON* T = qobject_cast<SSLCON*>(QObject::sender());
    delete T->M;
    if (UpdateQuene == 1) {
        uint unread = 0;
        for (uint i = 0; i < Mailv.size(); i++) {
            for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
                if (Mailv[i].Mboxv[j]->Mbox_Attrybutes&Mailbox::INBOX) {
                    for (uint k = 0; k < Mailv[i].Mboxv[j]->Emailv.size(); k++) {
                        if (!(Mailv[i].Mboxv[j]->Emailv[k]->Email_Flags&Email::Seen)) {
                            unread++;
                        }
                    }
                    break;
                }
            }
        }
        if (unread == 1) {
            emit sendUpdate("Mail: 1 unreaded message.");
        } else {
            emit sendUpdate("Mail: "+QString::number(unread)+" unreaded messages.");
        }
    } else {
        UpdateQuene--;
    }
}

void qorgMail::AttachmentS(bool I, QString R) {
    SSLCON* S = qobject_cast<SSLCON*>(QObject::sender());
    Mailbox* B =S->B;
    Email* E = S->E;
    Structure* A = S->A;
    QList <QString> Data = S->Data;
    if (I) {
        if (E->Email_Quene == 1) {
            QList  <QString>  ToClear;
            QString HTML = E->Email_Body[1];
            uint position = 0;
            QList <QString> AlreadyDeleted;
            while (HTML.indexOf("\"cid:", position) != -1) {
                int s = HTML.indexOf("\"cid:", position);
                int e = HTML.indexOf("\"", s+5);
                position = e;
                QString CID = HTML.mid(s+5, e-s-5).trimmed();
                if (!AlreadyDeleted.contains(CID)) {
                    for (uint i = 1; i < E->Structurev.size(); i++) {
                        if (E->Structurev[i]->Structure_CID == CID) {
                            QUrl A = QUrl::fromLocalFile(QDir::tempPath()+QDir::separator()+QString::number(E->Email_UID)+CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                            ToClear.append(QDir::tempPath()+QDir::separator()+QString::number(E->Email_UID)+CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                            AlreadyDeleted.append(CID);
                            HTML = HTML.mid(0, s)+"\""+A.toString()+HTML.mid(e, HTML.length()-e);
                            break;
                        }
                    }
                }
            }
            if (E == Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail] && emailAction) {
                MailHtml->setHtml(HTML);
                QEventLoop loop;
                connect(MailHtml, SIGNAL(loadFinished(bool)), &loop, SLOT(quit()));
                connect(MailHtml, SIGNAL(urlChanged(QUrl)), &loop, SLOT(quit()));
                loop.exec();
                emailAction = false;
            }
            for (int i = 0; i < ToClear.size(); i++) {
                QFile::remove(ToClear[i]);
            }
        }
        if (E->Email_Quene != 0) {
            E->Email_Quene--;
        }
    } else {
        if (E->Email_Quene != 0) {
            if (E == Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail] && emailAction && !((R == "ERROR: No response from server.") || (R == "ERROR: Problem with connection."))) {
                SSLCON* T = new SSLCON(this, &Mailv[currentMail]);
                connect(T, SIGNAL(AttachmentS(bool, QString)), this, SLOT(AttachmentS(bool, QString)));
                T->setMailbox(B);
                T->setEmail(E);
                T->setAttachment(A);
                T->setData(Data);
                T->setMethod(SSLCON::Attachment);
                T->start();
            } else {
                E->Email_Quene--;
            }
        }
        if (E->Email_Quene == 0) {
            MailHtml->setHtml(E->Email_Body[1]);
        }
    }
}
void qorgMail::AttachmentSUser(bool I, QString R) {
    SSLCON* S = qobject_cast<SSLCON*>(QObject::sender());
    if (!I && !S->Cancelled && !((R == "ERROR: No response from server.") || (R == "ERROR: Problem with connection."))) {
        Mailbox* B =S->B;
        Email* E = S->E;
        Structure* A = S->A;
        QList <QString> Data = S->Data;
        QLabel* L = new QLabel("Downloading...");
        L->setAttribute(Qt::WA_DeleteOnClose);
        L->setStyleSheet("background-color: #FF8888;");
        L->setWindowTitle(S->Data.first().mid(S->Data.first().lastIndexOf(QDir::separator())+1, S->Data.first().length()-S->Data.first().lastIndexOf(QDir::separator())-1));
        L->show();
        SSLCON* T = new SSLCON(this, &Mailv[currentMail]);
        connect(T, SIGNAL(AttachmentS(bool, QString)), this, SLOT(AttachmentSUser(bool, QString)));
        connect(T, SIGNAL(end()), L, SLOT(close()));
        T->setMailbox(B);
        T->setEmail(E);
        T->setAttachment(A);
        T->setData(Data);
        T->setMethod(SSLCON::Attachment);
        T->start();
    }
}

void qorgMail::SendEmailS(bool I, QString R) {
    if (I) {
        QMessageBox::information(this, "Sending", R);
    } else {
        QMessageBox::critical(this, "Sending error", R);
    }
}

void qorgMail::CopyES(bool I, QString R) {
    mailboxAction = false;
    if (!I) {
        QMessageBox::critical(this, "Copying error", R);
    } else {
        RefreshS();
    }

}
void qorgMail::DeleteES(bool I, QString R) {
    if (I) {
        for (uint i = 0; i < Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev.size(); i++) {
            delete Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[i];
        }
        delete Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail];
        Mailv[currentMail].Mboxv[currentMailbox]->Emailv.erase(Mailv[currentMail].Mboxv[currentMailbox]->Emailv.begin()+currentEmail);
        if (mailboxAction) {
            setMailbox(currentMailbox);
        }
    } else {
        QMessageBox::critical(this, "Error", R);
    }
    mailboxAction = false;
}
void qorgMail::MoveES(bool I, QString R) {
    mailboxAction = false;
    if (!I) {
        QMessageBox::critical(this, "Moving error", R);
    } else {
        RefreshS();
    }
}

void qorgMail::CreateMS(bool I, QString R) {
    mailboxAction = false;
    if (!I) {
        QMessageBox::critical(this, "Creating error", R);
    } else {
        RefreshS();
    }
}
void qorgMail::DeleteMS(bool I, QString R) {
    mailboxAction = false;
    if (!I) {
        QMessageBox::critical(this, "Deleting error", R);
    } else {
        RefreshS();
    }
}
void qorgMail::RenameMS(bool I, QString R) {
    mailboxAction = false;
    if (!I) {
        QMessageBox::critical(this, "Renaming error", R);
    } else {
        RefreshS();
    }
}


// Other slots
void qorgMail::chooseMbox(QTreeWidgetItem* I) {
    currentEmail = -1;
    setMailbox(I->data(0, Qt::UserRole).toUInt());
}
void qorgMail::chooseEmail(QModelIndex I) {
    if (!mailboxAction) {
        emailAction = true;
        currentEmail = Mailv[currentMail].Mboxv[currentMailbox]->Emailv.size()-I.row()-1;
        Tabs->clear();
        MailText->hide();
        MailHtml->hide();
        MailHtml->setHtml("<html><body>Loading. Please wait.<body></html>");
        Email* E = Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail];
        if (!E->Email_Body[0].isEmpty()) {
            Tabs->addTab(MailText, "Text");
            MailText->show();
            MailText->setText(E->Email_Body[0]);
        }
        if (!E->Email_Body[1].isEmpty()) {
            Tabs->addTab(MailHtml, "HTML");
            Tabs->setCurrentIndex(1);
            MailHtml->show();
            QString HTML = E->Email_Body[1];
            if (HTML.contains("\"cid:")) {
                Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Email_Quene = 0;
                uint position = 0;
                QList <QString> AlreadyDownlading;
                while (HTML.indexOf("\"cid:", position) != -1) {
                    int s = HTML.indexOf("\"cid:", position);
                    int e = HTML.indexOf("\"", s+5);
                    position = e;
                    QString CID = HTML.mid(s+5, e-s-5).trimmed();
                    if (!AlreadyDownlading.contains(CID)) {
                        for (uint i = 1; i < E->Structurev.size(); i++) {
                            if (E->Structurev[i]->Structure_CID == CID) {
                                AlreadyDownlading.append(CID);
                                downloadAttachment(i, QDir::tempPath()+QDir::separator()+QString::number(E->Email_UID)+CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                                break;
                            }
                        }
                    }
                }
                if (AlreadyDownlading.isEmpty()) {
                    MailHtml->setHtml(HTML);
                    emailAction = false;
                }
            } else {
                MailHtml->setHtml(HTML);
                emailAction = false;
            }
        } else {
            emailAction = false;
        }
        if (!(E->Email_Flags&Email::Seen)) {
            E->Email_Flags |= Email::Seen;
            MailView->selectedItems().first()->setFont(0, QFont());
        }
        AttachmentList->clear();
        for (uint i = 0; i < E->Structurev.size(); i++) {
            if (E->Structurev[i]->Structure_Type == "TEXT"&&E->Structurev[i]->Structure_Subtype == "PLAIN"&&i < E->Structurev.size()-1) {
                i++;
            } else if (E->Structurev[i]->Structure_Type == "TEXT"&&E->Structurev[i]->Structure_Subtype == "PLAIN"&&i == E->Structurev.size()-1) {
                break;
            }
            QListWidgetItem* Itm = new QListWidgetItem(AttachmentList);
            if (E->Structurev[i]->Structure_Attrybutes.Name.isEmpty()) {
                if (E->Structurev[i]->Structure_CID.isEmpty()) {
                    Itm->setText("text."+E->Structurev[i]->Structure_Subtype.toLower());
                } else {
                    Itm->setText(E->Structurev[i]->Structure_CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                }
            } else {
                Itm->setText(NameFilter(E->Structurev[i]->Structure_Attrybutes.Name));
            }
        }
    }
}

void qorgMail::downloadAttachment(QModelIndex I) {
    QString name = AttachmentList->item(I.row())->text();
    QString path = QFileDialog::getSaveFileName(this, "Save attachment to:", QDir::homePath()+QDir::separator()+name);
    if (!path.isEmpty()) {
        SSLCON* T = new SSLCON(this, &Mailv[currentMail]);
        connect(T, SIGNAL(AttachmentS(bool, QString)), this, SLOT(AttachmentSUser(bool, QString)));
        QLabel* L = new QLabel("Downloading...");
        L->setAttribute(Qt::WA_DeleteOnClose);
        L->setStyleSheet("background-color: #FF8888;");
        L->setWindowTitle(path.mid(path.lastIndexOf(QDir::separator())+1, path.length()-path.lastIndexOf(QDir::separator())-1));
        connect(T, SIGNAL(end()), L, SLOT(close()));
        connect(L, SIGNAL(destroyed()), T, SLOT(Canceled()));
        L->show();
        if (Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[0]->Structure_Type == "TEXT" &&
                Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[0]->Structure_Subtype == "PLAIN") {
            T->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
            T->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]);
            T->setAttachment(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I.row()+1]);
            T->setData(QList <QString>() << path);
        } else {
            T->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
            T->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]);
            T->setAttachment(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I.row()]);
            T->setData(QList <QString>() << path);
        }
        T->setMethod(SSLCON::Attachment);
        T->start();
    }
}
void qorgMail::downloadAttachment(uint I, QString path) {
    SSLCON* T = new SSLCON(this, &Mailv[currentMail]);
    connect(T, SIGNAL(AttachmentS(bool, QString)), this, SLOT(AttachmentS(bool, QString)));
    T->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
    T->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]);
    T->setAttachment(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I]);
    T->setData(QList <QString>() << path);
    T->setMethod(SSLCON::Attachment);
    T->start();
    Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Email_Quene++;
}
void qorgMail::RefreshS() {
    if (!mailboxAction) {
        mailboxAction = true;
        Mail* M = new Mail();
        M->IMAPserver = Mailv[currentMail].IMAPserver;
        M->User = Mailv[currentMail].User;
        M->Password = Mailv[currentMail].Password;
        SSLCON* T = new SSLCON(this, M);
        connect(T, SIGNAL(MailboxesS(bool, QString)), this, SLOT(RefreshS(bool)));
        T->setMethod(SSLCON::Mailboxes);
        T->start();
    }
}
void qorgMail::SendEmail() {
    QPushButton* T = qobject_cast<QPushButton*>(QObject::sender());
    SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
    connect(S, SIGNAL(SendEmailS(bool, QString)), this, SLOT(SendEmailS(bool, QString)));
    if (currentEmail == -1) {
        S->setEmail(NULL);
    } else {
        S->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
        S->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]);
    }
    QStringList Z;
    Z.append(AdressBook->getEmails());
    for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
        for (uint j = 0; j < Mailv[currentMail].Mboxv[i]->Emailv.size(); j++) {
            if (!Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From.contains("noreply")) {
                if (!Z.contains(Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From)) {
                    Z.append(Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From);
                }
            }
        }
    }
    if (T == Send) {
        if ((new Sender((new Email), Sender::Normal, Z, S, this))->exec() != QDialog::Accepted) {
            delete S;
        }
    } else if (T == Reply) {
        if (currentEmail != -1) {
            if ((new Sender(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail], Sender::Reply, Z, S, this))->exec() != QDialog::Accepted) {
                delete S;
            }
        }
    } else if (T == ReplyAll) {
        if (currentEmail != -1) {
            if ((new Sender(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail], Sender::ReplyAll, Z, S, this))->exec() != QDialog::Accepted) {
                delete S;
            }
        }
    } else {
        if (currentEmail != -1) {
            if ((new Sender(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail], Sender::Forward, Z, S, this))->exec() != QDialog::Accepted) {
                delete S;
            }
        }
    }
}
void qorgMail::DeleteEmail() {
    if (currentEmail != -1 && !mailboxAction) {
        mailboxAction = true;
        if (Mailv[currentMail].Mboxv[currentMailbox]->Mbox_Refresh) {
            SSLCON* T = new SSLCON(this, &Mailv[currentMail]);
            connect(T, SIGNAL(DeleteES(bool, QString)), this, SLOT(DeleteES(bool, QString)));
            T->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
            T->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]);
            T->setMethod(SSLCON::DeleteE);
            T->start();
        } else {
            DeleteES(true, "Delete from vector");
            mailboxAction = false;
        }
    }
}


void qorgMail::linkClicked(QUrl link) {
    QDesktopServices::openUrl(link);
}
void qorgMail::HTTPSS(QNetworkReply* QNR, QList<QSslError> I) {
    int response =Options->checkCertificate(I.first().certificate());
    if (response == 0) {
        if ((new CertAccept(I.first().certificate()))->exec() == QDialog::Accepted) {
            Options->acceptSSLCert(I.first().certificate());
            QNR->ignoreSslErrors(I);
        } else {
            Options->blacklistSSLCert(I.first().certificate());
        }
    } else if (response == 1) {
        QNR->ignoreSslErrors(I);
    }
    connect(QNR, SIGNAL(finished()), QNR, SLOT(deleteLater()));
}
void qorgMail::sortMail() {
    if (Mailv.size() > 1) {
        while (true) {
            bool OKL = true;
            for (int i = 0; i < static_cast<int>(Mailv.size()-1); i++) {
                if (Mailv[i].Name > Mailv[i+1].Name) {
                    if (i == currentMail) {
                        currentMail++;
                    } else if (i+1 == currentMail) {
                        currentMail--;
                    }
                    std::swap(Mailv[i], Mailv[i+1]);
                    OKL = false;
                }
            }
            if (OKL) {
                break;
            }
        }
    }
    int tmp = currentMail;
    currentMail = -2;
    setMail(tmp);
    emit updateTree();
}
// Menus and actions
void qorgMail::MailViewMenu(QPoint P) {
    if (!mailboxAction) {
        mailboxAction = true;
        QMenu* main = new QMenu(this);
        QTreeWidgetItem* I = MailView->itemAt(P);
        if (I == NULL) {
            mailboxAction = false;
            return;
        }
        QList <QAction*> CMailboxes;
        QMenu* copy = new QMenu("Copy to: ", main);
        QList <QAction*> MMailboxes;
        QMenu* move = new QMenu("Move to: ", main);
        for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
            if (!(Mailv[currentMail].Mboxv[i]->Mbox_Attrybutes & Mailbox::Noselect) && i != currentMailbox) {
                QAction* C= new QAction(Mailv[currentMail].Mboxv[i]->Mbox_Showname, copy);
                QAction* M= new QAction(Mailv[currentMail].Mboxv[i]->Mbox_Showname, move);
                C->setData(i);
                M->setData(i);
                CMailboxes.append(C);
                MMailboxes.append(M);
            }
        }
        copy->addActions(CMailboxes);
        move->addActions(MMailboxes);
        main->addMenu(copy);
        main->addMenu(move);
        QAction* Exe = main->exec(MailView->mapToGlobal(P));
        if (Exe != NULL) {
            if (CMailboxes.contains(Exe)) {
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::CopyE);
                S->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
                S->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[Mailv[currentMail].Mboxv[currentMailbox]->Emailv.size()-MailView->indexOfTopLevelItem(I)-1]);
                S->setData(QList <QString>() << Mailv[currentMail].Mboxv[Exe->data().toUInt()]->Mbox_Name);
                connect(S, SIGNAL(CopyES(bool, QString)), this, SLOT(CopyES(bool, QString)));
                S->start();
            } else if (MMailboxes.contains(Exe)) {
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::MoveE);
                S->setMailbox(Mailv[currentMail].Mboxv[currentMailbox]);
                S->setEmail(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[Mailv[currentMail].Mboxv[currentMailbox]->Emailv.size()-MailView->indexOfTopLevelItem(I)-1]);
                S->setData(QList <QString>() << Mailv[currentMail].Mboxv[Exe->data().toUInt()]->Mbox_Name);
                connect(S, SIGNAL(MoveES(bool, QString)), this, SLOT(MoveES(bool, QString)));
                S->start();
            }
        } else {
            mailboxAction = false;
        }
    }
}
void qorgMail::MailboxesMenu(QPoint P) {
    if (!mailboxAction) {
        mailboxAction = true;
        QMenu* main = new QMenu(this);
        QTreeWidgetItem* I = Mailboxes->itemAt(P);
        QAction* C = new QAction("Create", main);
        QAction* CS = new QAction("Create submailbox", main);
        QAction* D = new QAction("Delete", main);
        QAction* R = new QAction("Rename", main);
        if (I != NULL) {
            main->addAction(CS);
            if (I->flags() & Qt::ItemIsSelectable) {
                main->addAction(D);
                main->addAction(R);
            }
        } else {
            main->addAction(C);
        }
        QAction* Exe = main->exec(Mailboxes->mapToGlobal(P));
        if (Exe != NULL) {
            if (Exe == C) {
                QString name = QInputDialog::getText(this, "Mailbox name", "Please enter name of new mailbox.\nMailbox name must be unique.\nDo not use / . % # -");
                name.remove("/");
                name.remove(".");
                name.remove("%");
                name.remove("#");
                name.remove("&");
                name.remove("-");
                if (name.isEmpty()) {
                    mailboxAction = false;
                    return;
                }
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::CreateM);
                S->setData(QList <QString>() << "" << name);
                connect(S, SIGNAL(CreateMS(bool, QString)), this, SLOT(CreateMS(bool, QString)));
                S->start();
            } else if (Exe == CS) {
                QString name = QInputDialog::getText(this, "Mailbox name", "Please enter name of new mailbox.\nMailbox name must be unique.\nDo not use / . % # -");
                name.remove("/");
                name.remove(".");
                name.remove("%");
                name.remove("#");
                name.remove("&");
                name.remove("-");
                if (name.isEmpty()) {
                    mailboxAction = false;
                    return;
                }
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::CreateM);
                S->setData(QList <QString>() << Mailv[currentMail].Mboxv[I->data(0, Qt::UserRole).toUInt()]->Mbox_Name << name);
                connect(S, SIGNAL(CreateMS(bool, QString)), this, SLOT(CreateMS(bool, QString)));
                S->start();
            } else if (Exe == D) {
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::DeleteM);
                S->setData(QList <QString>() << Mailv[currentMail].Mboxv[I->data(0, Qt::UserRole).toUInt()]->Mbox_Name);
                connect(S, SIGNAL(DeleteMS(bool, QString)), this, SLOT(DeleteMS(bool, QString)));
                S->start();
            } else if (Exe == R) {
                QString name = QInputDialog::getText(this, "Mailbox name", "Please enter name of new mailbox.\nMailbox name must be unique.\nDo not use / . % # -");
                name.remove("/");
                name.remove(".");
                name.remove("%");
                name.remove("#");
                name.remove("&");
                name.remove("-");
                if (name.isEmpty()) {
                    mailboxAction = false;
                    return;
                }
                SSLCON* S = new SSLCON(this, &Mailv[currentMail]);
                S->setMethod(SSLCON::RenameM);
                S->setData(QList <QString>() << Mailv[currentMail].Mboxv[I->data(0, Qt::UserRole).toUInt()]->Mbox_Name << name);
                connect(S, SIGNAL(RenameMS(bool, QString)), this, SLOT(RenameMS(bool, QString)));
                S->start();
            }
        } else {
            mailboxAction = false;
        }
    }
}

#include "qorgmail.moc"
