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
#include <vector>

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
Structure::~Structure() {
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
    this->Email_From.Name.clear();
    this->Email_From.EMailA.clear();
    this->Email_Date.setMSecsSinceEpoch(0);
    this->Email_Body[0].clear();
    this->Email_Body[1].clear();
    this->Email_UID = 0;
    this->Email_Flags = 0;
    this->Structurev.clear();
}
Email::~Email() {
    this->Email_Subject.clear();
    this->Email_From.Name.clear();
    this->Email_From.EMailA.clear();
    this->Email_Date.setMSecsSinceEpoch(0);
    this->Email_Body[0].clear();
    this->Email_Body[1].clear();
    this->Email_UID = 0;
    this->Email_Flags = 0;
    this->Structurev.clear();
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
Mailbox::~Mailbox() {
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
Mail::~Mail() {
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
        Sleep,
        Login,
        Mailboxes,
        Emails,
        Attachment,
        Send,
        Delete,
        Stop,
    };
    explicit SSLCON(Mail*);
    ~SSLCON() {
        M = NULL;
        To.clear();
        Bar->deleteLater();
        Downloading->deleteLater();
    }
    void setMethod(Method);
    void DownloadAttachmentData(int B, int E, int A, QString P) {
        this->M = M;
        this->B = B;
        this->E = E;
        this->A = A;
        this->Path = P;
    }
    void SetBE(int B, int E) {
        this->B = B;
        this->E = E;
    }
    void SendEmail(QString header, QList  <QString >  to, QString data, QList  < QString>  Att) {
        this->Header = header;
        this->To = to;
        this->Data = data;
        this->Att = Att;
    }
    QDialog *Downloading;
    QProgressBar *Bar;
    Mail *M;
private:
    QLabel *L;
    Method Current;
    int B;
    int E;
    int A;
    QString Path;
    QString Header;
    QList  <QString>  To;
    QString Data;
    QList  <QString>  Att;
    void SecureLogin();
    void DownloadMBoxVector();
    void DownloadEmails();
    void DownloadAttachment();
    void SendEmail();
    void DeleteEmail();
    void run();
    QByteArray readAll(QSslSocket*);
    QByteArray readIMAP(QSslSocket*);
signals:
    void LoginS(bool);
    void MailboxesS(bool);
    void EmailS(bool);
    void AttachmentS(bool);
    void SendEmailS(bool);
    void DeleteS(bool);
};
SSLCON::SSLCON(Mail* I) {
    M = I;
    Downloading = new QDialog();
    Bar = new QProgressBar();
    QHBoxLayout *L = new QHBoxLayout(Downloading);
    L->addWidget(Bar);
    Downloading->setLayout(L);
    Downloading->setWindowTitle("Progress bar");
    Current = Sleep;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}
void SSLCON::setMethod(Method I) {
    Current = I;
}
void SSLCON::run() {
    while (Current != Stop) {
        if (M != NULL&&Current != Sleep) {
            bool kill = false;
            if (Current == Login) {
                SecureLogin();
            }
            if (Current == Mailboxes) {
                DownloadMBoxVector();
            }
            if (Current == Emails) {
                DownloadEmails();
                kill = true;
            }
            if (Current == Attachment) {
                DownloadAttachment();
                kill = true;
            }
            if (Current == Send) {
                SendEmail();
                kill = true;
            }
            if (Current == Delete) {
                DeleteEmail();
            }
            if (!kill) {
                Current = Sleep;
            } else {
                Current = Stop;
            }
        } else {
            this->msleep(100);
        }
    }
    Bar->hide();
}
QByteArray SSLCON::readAll(QSslSocket *S) {
    QByteArray Reply;
    if (S->waitForReadyRead()) {
        Reply.append(S->readAll());
        while (S->waitForReadyRead(10)&&S->bytesAvailable() != 0) {
            Reply.append(S->readAll());
        }
    }
    return Reply;
}
QByteArray SSLCON::readIMAP(QSslSocket *S) {
    QByteArray Reply;
    if (S->waitForReadyRead()) {
        Reply.append(S->readAll());
        while (!Reply.contains("TAG OK") && !Reply.contains("TAG NO")) {
            if (S->waitForReadyRead()) {
                Reply.append(S->readAll());
            }
        }
    }
    return Reply;
}
void SSLCON::SecureLogin() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    S.ignoreSslErrors();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit LoginS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit LoginS(false);
        return;
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    S.close();
    if (Reply.contains("TAG OK")) {
        Bar->setValue(12);
        emit LoginS(true);
    } else {
        emit LoginS(false);
    }
}
void SSLCON::DownloadMBoxVector() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    S.ignoreSslErrors();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit MailboxesS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MailboxesS(false);
        return;
    }
    if (!Reply.contains("TAG OK")) {
        emit MailboxesS(false);
        return;
    }
    S.write("TAG LIST \"\" \"*\"\r\n");
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit MailboxesS(false);
        return;
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    S.close();
    QStringList A = Reply.split("\n", QString::SkipEmptyParts);
    A.removeLast();
    QString Split;
    for (int i = 0; i < A.size(); i++) {
        Mailbox *temp = new Mailbox();
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
        int pos = 0;
        pos = A[i].indexOf(")")+2;
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
        temp->Mbox_Name.lastIndexOf(Split);
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
    emit MailboxesS(true);
    Bar->setValue(25);
}
void SSLCON::DownloadEmails() {
    Bar->show();
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    S.ignoreSslErrors();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit EmailS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit EmailS(false);
        return;
    }
    if (!Reply.contains("TAG OK")) {
        emit EmailS(false);
        return;
    }
    for (uint i = 0; i < M->Mboxv.size(); i++) {
        if (M->Mboxv[i]->Mbox_Refresh) {
            S.write(QString("TAG SELECT \""+M->Mboxv[i]->Mbox_Name+"\"\r\n").toUtf8());
            Reply = readIMAP(&S);
            if (Reply.isEmpty()) {
                emit EmailS(false);
                return;
            }
            QStringList A = Reply.split("*", QString::SkipEmptyParts);
            for (int j = 0; j < A.size(); j++) {
                if (A[j].contains("EXISTS")) {
                    Bar->setValue(30);
                    vector  <Email*> *Vec=&M->Mboxv[i]->Emailv;
                    QString Fnu="1";
                    uint Fn = 1;
                    QString Enu = A[j].mid(1, A[j].indexOf(" EXISTS")-1);
                    uint En = Enu.toInt();
                    if (En > 0) {
                        QString UIDS;
                        {
                            S.write(QString("TAG FETCH 1:"+Enu+" UID\r\n").toUtf8());
                            UIDS = readIMAP(&S);
                            if (UIDS.isEmpty()) {
                                emit EmailS(false);
                                return;
                            }
                            QStringList UIDL = UIDS.split("UID ", QString::SkipEmptyParts);
                            QList  <uint>  UIDLi;
                            UIDL.removeFirst();
                            for (int k = 0; k < UIDL.size(); k++) {
                                UIDLi.append(UIDL[k].mid(0, UIDL[k].indexOf(")")).toInt());
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
                                        Fnu = QString::number(k+2);
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
                            Bar->setValue(35);
                            QString Subjects;
                            {
                                S.write(QString("TAG FETCH "+Fnu+":"+Enu+" BODY.PEEK[HEADER.FIELDS (Subject)]\r\n").toUtf8());
                                Subjects = readIMAP(&S);
                                if (Subjects.isEmpty()) {
                                    emit EmailS(false);
                                    return;
                                }
                                QStringList SubL = Subjects.split("Subject: ", QString::SkipEmptyParts);
                                SubL.removeFirst();
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    QString Sub = SubL[k].mid(0, SubL[k].indexOf("\r\n)"));
                                    Sub.remove("\r\n");
                                    Sub = NameFilter(Sub);
                                    if (Sub.simplified().isEmpty()) {
                                        Sub="(no subject)";
                                    }
                                    (*Vec)[Fn+k-1]->Email_Subject = Sub.simplified();
                                }
                            }
                            Bar->setValue(40);
                            QString Dates;
                            {
                                S.write(QString("TAG FETCH "+Fnu+":"+Enu+" BODY.PEEK[HEADER.FIELDS (Date)]\r\n").toUtf8());
                                Dates = readIMAP(&S);
                                if (Dates.isEmpty()) {
                                    emit EmailS(false);
                                    return;
                                }
                                QStringList DateL = Dates.split("Date: ");
                                DateL.removeFirst();
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    QString Dat = DateL[k].mid(0, DateL[k].indexOf("\r\n)"));
                                    if (Dat.contains(", ")) {
                                        Dat = Dat.mid(Dat.indexOf(", ")+2, Dat.length()-Dat.indexOf(", ")-2);
                                    }
                                    QStringList D = Dat.simplified().split(" ");
                                    QList  <QString>  Mon;
                                    QDateTime Tmp;
                                    Mon << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
                                    Tmp.setTimeSpec(Qt::UTC);
                                    Tmp.setDate(QDate(D[2].toInt(), Mon.indexOf(D[1])+1, D[0].toInt()));
                                    QStringList H = D[3].split(":");
                                    Tmp.setTime(QTime(H[0].toInt(), H[1].toInt(), H[2].toInt()));
                                    if (D[4] != "GMT") {
                                        char H = D[4].mid(1, 2).toShort();
                                        char M = D[4].mid(3, 2).toShort();
                                        int Sec = H*3600+M*60;
                                        if (D[4][0] == '+') {
                                            Tmp = Tmp.addSecs(-Sec);
                                        } else {
                                            Tmp = Tmp.addSecs(Sec);
                                        }
                                    }
                                    (*Vec)[Fn+k-1]->Email_Date = Tmp.toLocalTime();
                                }
                            }
                            Bar->setValue(45);
                            QString Froms;
                            {
                                S.write(QString("TAG FETCH "+Fnu+":"+Enu+" BODY.PEEK[HEADER.FIELDS (From)]\r\n").toUtf8());
                                Froms = readIMAP(&S);
                                if (Froms.isEmpty()) {
                                    emit EmailS(false);
                                    return;
                                }
                                QStringList FromsL = Froms.split("From: ", QString::SkipEmptyParts);
                                FromsL.removeFirst();
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    QString From = FromsL[k].mid(0, FromsL[k].indexOf("\r\n)"));
                                    QString Name = From.mid(0, From.indexOf("<"));
                                    Name = NameFilter(Name);
                                    QString EMA = From.mid(From.indexOf("<")+1, From.indexOf(">")-From.indexOf("<")-1).simplified();
                                    (*Vec)[Fn+k-1]->Email_From.Name = Name.simplified();
                                    (*Vec)[Fn+k-1]->Email_From.EMailA = EMA;
                                }
                            }
                            Bar->setValue(50);
                            QString Flags;
                            {
                                S.write(QString("TAG FETCH "+Fnu+":"+Enu+" FLAGS\r\n").toUtf8());
                                Flags = readIMAP(&S);
                                if (Flags.isEmpty()) {
                                    emit EmailS(false);
                                    return;
                                }
                                QStringList FlagsL = Flags.split("FLAGS (", QString::SkipEmptyParts);
                                FlagsL.removeFirst();
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    QString Fla = FlagsL[k].mid(0, FlagsL[k].indexOf(")"));
                                    char Flag = 0;
                                    if (Fla.contains("\\Answered")) {
                                        Flag|=Email::Answered;
                                    }
                                    if (Fla.contains("\\Flagged")) {
                                        Flag|=Email::Flagged;
                                    }
                                    if (Fla.contains("\\Draft")) {
                                        Flag|=Email::Draft;
                                    }
                                    if (Fla.contains("\\Deleted")) {
                                        Flag|=Email::Deleted;
                                    }
                                    if (Fla.contains("\\Seen")) {
                                        Flag|=Email::Seen;
                                    }
                                    (*Vec)[Fn+k-1]->Email_Flags = Flag;
                                }
                            }
                            Bar->setValue(55);
                            QString BS;
                            {
                                S.write(QString("TAG FETCH "+Fnu+":"+Enu+" BODYSTRUCTURE\r\n").toUtf8());
                                BS = readIMAP(&S);
                                if (BS.isEmpty()) {
                                    emit EmailS(false);
                                    return;
                                }
                                QList  <QString>  BSL = BS.split("BODYSTRUCTURE (");
                                BSL.removeFirst();
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    QString I = BSL[k].mid(0, BSL[k].lastIndexOf(")")-1);
                                    int part = 1;
                                    if (I[0] != '"') {
                                        int S = 0;
                                        int F = 0;
                                        int L = 0;
                                        for (int l = 0; l < I.length(); l++) {
                                            if (I[l] == '(') {
                                                if (L == 0) {
                                                    S = l;
                                                }
                                                L++;
                                            }
                                            if (I[l] == ')') {
                                                L--;
                                                if (L == 0) {
                                                    F = l;
                                                    if (I.mid(S+1, F-S-1)[0] == '(') {
                                                        QString P = I.mid(S+1, F-S-1);
                                                        int S = 0;
                                                        int F = 0;
                                                        int L = 0;
                                                        int subpart = 1;
                                                        for (int m = 0; m < P.length(); m++) {
                                                            if (P[m] == '(') {
                                                                if (L == 0) {
                                                                    S = m;
                                                                }
                                                                L++;
                                                            }
                                                            if (P[m] == ')') {
                                                                L--;
                                                                if (L == 0) {
                                                                    F = m;
                                                                    QString I = P.mid(S+1, F-S-1);
                                                                    if (!I.toUpper().contains("\"BOUNDARY\"")) {
                                                                        Structure *Tmp = new Structure();
                                                                        Tmp->Structure_Number = QString::number(part)+"."+QString::number(subpart);
                                                                        QList <QString> IL;
                                                                        int L = 0;
                                                                        QString Data;
                                                                        for (int n = 0; n < I.length(); n++) {
                                                                            if (I[n] == ' '&&L == 0) {
                                                                                IL.append(Data);
                                                                                Data.clear();
                                                                            } else {
                                                                                if (I[n] == '(') {
                                                                                    L++;
                                                                                }
                                                                                if (I[n] == ')') {
                                                                                    L--;
                                                                                }
                                                                                Data.append(I[n]);
                                                                            }
                                                                        }
                                                                        IL.append(Data);
                                                                        for (int n = 0; n < IL.size(); n++) {
                                                                            IL[n].remove("\"");
                                                                            IL[n].remove("(");
                                                                            IL[n].remove(")");
                                                                        }
                                                                        Tmp->Structure_Type = IL[0].toUpper();
                                                                        Tmp->Structure_Subtype = IL[1].toUpper();
                                                                        QList  <QString>  ILL = IL[2].toUpper().split(" ");
                                                                        QList  <QString>  ILLD = IL[2].split(" ");
                                                                        if (ILL.indexOf("CHARSET") != -1) {
                                                                            Tmp->Structure_Attrybutes.Charset = ILL[ILL.indexOf("CHARSET")+1];
                                                                        }
                                                                        if (ILL.indexOf("NAME") != -1) {
                                                                            Tmp->Structure_Attrybutes.Name = ILLD[ILL.indexOf("NAME")+1];
                                                                        }
                                                                        if (IL[3].contains("<")&&IL[3].contains(">")) {
                                                                            IL[3]=IL[3].mid(1, IL[3].length()-2);
                                                                        }
                                                                        Tmp->Structure_CID = IL[3];
                                                                        Tmp->Structure_Descryption = IL[4].toUpper();
                                                                        Tmp->Structure_Encoding = IL[5].toUpper();
                                                                        Tmp->Structure_Size = IL[6].toInt();
                                                                        if (Tmp->Structure_Type == "TEXT") {
                                                                            Tmp->Structure_Lines = IL[7].toInt();
                                                                            Tmp->Structure_MD5 = IL[8];
                                                                            Tmp->Structure_Disposition = IL[9];
                                                                            QList  <QString>  ILL = IL[9].toUpper().split(" ");
                                                                            QList  <QString>  ILLD = IL[9].split(" ");
                                                                            if (ILL.indexOf("FILENAME") != -1) {
                                                                                QString N = ILLD[ILL.indexOf("FILENAME")+1];
                                                                                uint i = 2;
                                                                                while (!N.contains(".")) {
                                                                                    N.append(ILLD[ILL.indexOf("FILENAME")+i]);
                                                                                }
                                                                                Tmp->Structure_Attrybutes.Name = N;
                                                                            }
                                                                            Tmp->Structure_Language = IL[10];
                                                                            if (IL.size() == 12) {
                                                                                Tmp->Structure_Location = IL[11];
                                                                            }
                                                                        } else {
                                                                            Tmp->Structure_MD5 = IL[7];
                                                                            Tmp->Structure_Disposition = IL[8];
                                                                            QList  <QString>  ILL = IL[8].toUpper().split(" ");
                                                                            QList  <QString>  ILLD = IL[8].split(" ");
                                                                            if (ILL.indexOf("FILENAME") != -1) {
                                                                                QString N = ILLD[ILL.indexOf("FILENAME")+1];
                                                                                uint i = 2;
                                                                                while (!N.contains(".")) {
                                                                                    N.append(ILLD[ILL.indexOf("FILENAME")+i]);
                                                                                }
                                                                                Tmp->Structure_Attrybutes.Name = N;
                                                                            }
                                                                            Tmp->Structure_Language = IL[9];
                                                                            if (IL.size() == 11) {
                                                                                Tmp->Structure_Location = IL[10];
                                                                            }
                                                                        }
                                                                        (*Vec)[Fn+k-1]->Structurev.push_back(Tmp);
                                                                        subpart++;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        part++;
                                                    } else {
                                                        if (!I.mid(S+1, F-S-1).toUpper().contains("\"BOUNDARY\"")) {
                                                            QString P = I.mid(S+1, F-S-1);
                                                            QString I = P;
                                                            Structure *Tmp = new Structure();
                                                            Tmp->Structure_Number = QString::number(part);
                                                            QList <QString> IL;
                                                            int L = 0;
                                                            QString Data;
                                                            for (int n = 0; n < I.length(); n++) {
                                                                if (I[n] == ' '&&L == 0) {
                                                                    IL.append(Data);
                                                                    Data.clear();
                                                                } else {
                                                                    if (I[n] == '(') {
                                                                        L++;
                                                                    }
                                                                    if (I[n] == ')') {
                                                                        L--;
                                                                    }
                                                                    Data.append(I[n]);
                                                                }
                                                            }
                                                            IL.append(Data);
                                                            for (int n = 0; n < IL.size(); n++) {
                                                                IL[n].remove("\"");
                                                                IL[n].remove("(");
                                                                IL[n].remove(")");
                                                            }
                                                            Tmp->Structure_Type = IL[0].toUpper();
                                                            Tmp->Structure_Subtype = IL[1].toUpper();
                                                            QList  <QString>  ILL = IL[2].toUpper().split(" ");
                                                            QList  <QString>  ILLD = IL[2].split(" ");
                                                            if (ILL.indexOf("CHARSET") != -1) {
                                                                Tmp->Structure_Attrybutes.Charset = ILL[ILL.indexOf("CHARSET")+1];
                                                            }
                                                            if (ILL.indexOf("NAME") != -1) {
                                                                Tmp->Structure_Attrybutes.Name = ILLD[ILL.indexOf("NAME")+1];
                                                            }
                                                            if (IL[3].contains("<")&&IL[3].contains(">")) {
                                                                IL[3]=IL[3].mid(1, IL[3].length()-2);
                                                            }
                                                            Tmp->Structure_CID = IL[3];
                                                            Tmp->Structure_Descryption = IL[4].toUpper();
                                                            Tmp->Structure_Encoding = IL[5].toUpper();
                                                            Tmp->Structure_Size = IL[6].toInt();
                                                            if (Tmp->Structure_Type == "TEXT") {
                                                                Tmp->Structure_Lines = IL[7].toInt();
                                                                Tmp->Structure_MD5 = IL[8];
                                                                Tmp->Structure_Disposition = IL[9];
                                                                QList  <QString>  ILL = IL[9].toUpper().split(" ");
                                                                QList  <QString>  ILLD = IL[9].split(" ");
                                                                if (ILL.indexOf("FILENAME") != -1) {
                                                                    QString N = ILLD[ILL.indexOf("FILENAME")+1];
                                                                    uint i = 2;
                                                                    while (!N.contains(".")) {
                                                                        N.append(ILLD[ILL.indexOf("FILENAME")+i]);
                                                                    }
                                                                    Tmp->Structure_Attrybutes.Name = N;
                                                                }
                                                                Tmp->Structure_Language = IL[10];
                                                                if (IL.size() == 12) {
                                                                    Tmp->Structure_Location = IL[11];
                                                                }
                                                            } else {
                                                                Tmp->Structure_MD5 = IL[7];
                                                                Tmp->Structure_Disposition = IL[8];
                                                                QList  <QString>  ILL = IL[8].toUpper().split(" ");
                                                                QList  <QString>  ILLD = IL[8].split(" ");
                                                                if (ILL.indexOf("FILENAME") != -1) {
                                                                    QString N = ILLD[ILL.indexOf("FILENAME")+1];
                                                                    uint i = 2;
                                                                    while (!N.contains(".")) {
                                                                        N.append(ILLD[ILL.indexOf("FILENAME")+i]);
                                                                    }
                                                                    Tmp->Structure_Attrybutes.Name = N;
                                                                }
                                                                Tmp->Structure_Language = IL[9];
                                                                if (IL.size() == 11) {
                                                                    Tmp->Structure_Location = IL[10];
                                                                }
                                                            }
                                                            (*Vec)[Fn+k-1]->Structurev.push_back(Tmp);
                                                            part++;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        Structure *Tmp = new Structure();
                                        Tmp->Structure_Number = QString::number(part);
                                        QList <QString> IL;
                                        int L = 0;
                                        QString Data;
                                        for (int n = 0; n < I.length(); n++) {
                                            if (I[n] == ' '&&L == 0) {
                                                IL.append(Data);
                                                Data.clear();
                                            } else {
                                                if (I[n] == '(') {
                                                    L++;
                                                }
                                                if (I[n] == ')') {
                                                    L--;
                                                }
                                                Data.append(I[n]);
                                            }
                                        }
                                        IL.append(Data);
                                        for (int n = 0; n < IL.size(); n++) {
                                            IL[n].remove("\"");
                                            IL[n].remove("(");
                                            IL[n].remove(")");
                                        }
                                        Tmp->Structure_Type = IL[0].toUpper();
                                        Tmp->Structure_Subtype = IL[1].toUpper();
                                        QList  <QString>  ILL = IL[2].toUpper().split(" ");
                                        QList  <QString>  ILLD = IL[2].split(" ");
                                        if (ILL.indexOf("CHARSET") != -1) {
                                            Tmp->Structure_Attrybutes.Charset = ILL[ILL.indexOf("CHARSET")+1];
                                        }
                                        if (ILL.indexOf("NAME") != -1) {
                                            Tmp->Structure_Attrybutes.Name = ILLD[ILL.indexOf("NAME")+1];
                                        }
                                        if (IL[3].contains("<")&&IL[3].contains(">")) {
                                            IL[3]=IL[3].mid(1, IL[3].length()-2);
                                        }
                                        Tmp->Structure_CID = IL[3];
                                        Tmp->Structure_Descryption = IL[4].toUpper();
                                        Tmp->Structure_Encoding = IL[5].toUpper();
                                        Tmp->Structure_Size = IL[6].toInt();
                                        if (Tmp->Structure_Type == "TEXT") {
                                            Tmp->Structure_Lines = IL[7].toInt();
                                            Tmp->Structure_MD5 = IL[8];
                                            Tmp->Structure_Disposition = IL[9];
                                            QList  <QString>  ILL = IL[9].toUpper().split(" ");
                                            QList  <QString>  ILLD = IL[9].split(" ");
                                            if (ILL.indexOf("FILENAME") != -1) {
                                                QString N = ILLD[ILL.indexOf("FILENAME")+1];
                                                uint i = 2;
                                                while (!N.contains(".")) {
                                                    N.append(ILLD[ILL.indexOf("FILENAME")+i]);
                                                }
                                                Tmp->Structure_Attrybutes.Name = N;
                                            }
                                            Tmp->Structure_Language = IL[10];
                                            if (IL.size() == 12) {
                                                Tmp->Structure_Location = IL[11];
                                            }
                                        } else {
                                            Tmp->Structure_MD5 = IL[7];
                                            Tmp->Structure_Disposition = IL[8];
                                            QList  <QString>  ILL = IL[8].toUpper().split(" ");
                                            QList  <QString>  ILLD = IL[8].split(" ");
                                            if (ILL.indexOf("FILENAME") != -1) {
                                                Tmp->Structure_Attrybutes.Name = ILLD[ILL.indexOf("FILENAME")+1];
                                            }
                                            Tmp->Structure_Language = IL[9];
                                            if (IL.size() == 11) {
                                                Tmp->Structure_Location = IL[10];
                                            }
                                        }
                                        (*Vec)[Fn+k-1]->Structurev.push_back(Tmp);
                                    }
                                }
                            }
                            Bar->setValue(60);
                            QByteArray Text;
                            {
                                double VA=(static_cast<double>(40)/static_cast<double>(En-Fn+1));
                                for (uint k = 0; k < En-Fn+1; k++) {
                                    for (uint l = 0; l < (*Vec)[Fn+k-1]->Structurev.size()&&l < 2; l++) {
                                        int t = 0;
                                        if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Type == "TEXT"&&(*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Name.isEmpty()) {
                                            if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Subtype == "HTML") {
                                                t = 1;
                                            }
                                            S.write(QString("TAG UID FETCH "+QString::number((*Vec)[Fn+k-1]->Email_UID)+" BODY["+(*Vec)[Fn+k-1]->Structurev[l]->Structure_Number+"]\r\n").toUtf8());
                                        } else {
                                            break;
                                        }
                                        Text = readIMAP(&S);
                                        if (Text.isEmpty()) {
                                            emit EmailS(false);
                                            return;
                                        }
                                        Text = Text.mid(Text.indexOf("}")+1, Text.indexOf(")\r\nTAG OK")-Text.indexOf("}")-1);
                                        QByteArray Tmp;
                                        if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Encoding == "BASE64") {
                                            Tmp = QByteArray::fromBase64(Text);
                                        } else if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Encoding == "QUOTED-PRINTABLE") {
                                            Tmp = QPDecode(Text);
                                        } else {
                                            Tmp = Text;
                                        }
                                        if ((*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset != "UTF-8" && (!(*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset.isEmpty())) {
                                            QTextCodec *C = QTextCodec::codecForName((*Vec)[Fn+k-1]->Structurev[l]->Structure_Attrybutes.Charset.toUtf8());
                                            (*Vec)[Fn+k-1]->Email_Body[t]=C->toUnicode(Tmp);
                                        } else {
                                            (*Vec)[Fn+k-1]->Email_Body[t]=Tmp;
                                        }
                                    }
                                    Bar->setValue(static_cast<int>(60+(k+1)*VA));
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    S.close();
    emit EmailS(true);
    Bar->hide();
}
void SSLCON::DownloadAttachment() {
    L = new QLabel("Downloading...");
    L->setStyleSheet("background-color: #FF8888;");
    L->setWindowTitle(M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Attrybutes.Name);
    L->show();
    if (M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Attrybutes.Name.isEmpty()) {
        if (M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_CID == "NIL") {
            L->setWindowTitle("text."+M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Subtype.toLower());
        } else {
            L->setWindowTitle(M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_CID+"."+M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Subtype.toLower());
        }
    }
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    S.ignoreSslErrors();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit AttachmentS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit AttachmentS(false);
        return;
    }
    if (!Reply.contains("TAG OK")) {
        emit AttachmentS(false);
        return;
    }
    S.write(QString("TAG SELECT "+M->Mboxv[B]->Mbox_Name+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit AttachmentS(false);
        return;
    }
    S.write(QString("TAG UID FETCH "+QString::number(M->Mboxv[B]->Emailv[E]->Email_UID)+" BODY["+M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Number+"]\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit AttachmentS(false);
        return;
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    S.close();
    Reply = Reply.mid(Reply.indexOf("}")+1, Reply.indexOf(")\r\nTAG OK")-Reply.indexOf("}")-1);
    QByteArray Array;
    if (M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Encoding == "BASE64") {
        Array = QByteArray::fromBase64(Reply.toUtf8());
    } else if (M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Encoding == "QUOTED-PRINTABLE") {
        Array = QPDecode(Reply.toUtf8());
    }
    if (M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Attrybutes.Charset != "UTF-8"&&!(M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Attrybutes.Charset.isEmpty())) {
        QTextCodec *C = QTextCodec::codecForName(M->Mboxv[B]->Emailv[E]->Structurev[A]->Structure_Attrybutes.Charset.toUtf8());
        Array = C->toUnicode(Array).toUtf8();
    }
    QFile F(Path);
    F.open(QIODevice::WriteOnly);
    F.write(Array);
    F.close();
    L->deleteLater();
    emit AttachmentS(true);
}
void SSLCON::SendEmail() {
    QByteArray P = calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512));
    QString Reply;
    QSslSocket S;
    S.ignoreSslErrors();
    Email* EM;
    if (E == -1) {
        EM = new Email;
    } else {
        EM = M->Mboxv[B]->Emailv[E];
    }
    QString Output;
    QString boundry = QCryptographicHash::hash(Data.toUtf8(), QCryptographicHash::Md5).toHex();
    Output.append(Header+"\r\n");
    Output.append("Content-Type: multipart/mixed; boundary=\"------"+boundry+"\"\nThis is a multi-part message in MIME format.\n\n");
    Output.append("--------"+boundry+"\n");
    Output.append("Content-Type: text/plain; charset=UTF-8\n");
    Output.append("Content-Transfer-Encoding: base64\n\n");
    Output.append(Data.toUtf8().toBase64()+"\n\n");
    if (Att.size() != 0&&Att[0][0] != '<' && EM->Structurev[Att[0].toInt()]->Structure_Type == "TEXT" && EM->Structurev[Att[0].toInt()]->Structure_Subtype == "HTML") {
        Output.append("--------"+boundry+"\n");
        Output.append("Content-Type: text/html; charset=UTF-8\n");
        Output.append("Content-Transfer-Encoding: base64\n\n");
        Output.append(EM->Email_Body[1].toUtf8().toBase64()+"\n\n");
        Att.removeFirst();
    }
    S.connectToHostEncrypted(M->IMAPserver, 993);
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    if (!Reply.contains("TAG OK")) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("TAG SELECT "+M->Mboxv[B]->Mbox_Name+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    for (int i = 0; i < Att.size(); i++) {
        Output.append("--------"+boundry+"\n");
        if (Att[i][0] == '<') {
            QMimeDatabase db;
            QString Path = Att[i].mid(1, Att[i].length()-2);
            QFile *F = new QFile(Path);
            F->open(QFile::ReadOnly);
            QByteArray Array = F->readAll();
            Output.append("Content-Type: "+db.mimeTypeForData(Array).name());
            Output.append("; name=\""+Path.mid(Path.lastIndexOf("/")+1, Path.length()-Path.lastIndexOf("/")-1)+"\"\n");
            Output.append("Content-Disposition: attachment; filename=\""+Path.mid(Path.lastIndexOf("/")+1, Path.length()-Path.lastIndexOf("/")-1)+"\"\n");
            Output.append("Content-Transfer-Encoding: base64\n\n");
            Output.append(Array.toBase64()+"\n\n");
        } else {
            S.write(QString("TAG UID FETCH "+QString::number(M->Mboxv[B]->Emailv[E]->Email_UID)+" BODY["+M->Mboxv[B]->Emailv[E]->Structurev[Att[i].toInt()]->Structure_Number+"]\r\n").toUtf8());
            Reply = readIMAP(&S);
            if (Reply.isEmpty()) {
                emit SendEmailS(false);
                return;
            }
            Reply = Reply.mid(Reply.indexOf("}")+1, Reply.indexOf(")\r\nTAG OK")-Reply.indexOf("}")-1);
            if (EM->Structurev[Att[i].toInt()]->Structure_CID != "NIL") {
                Output.append("Content-Type: "+EM->Structurev[Att[i].toInt()]->Structure_Type.toLower()+"/"+EM->Structurev[Att[i].toInt()]->Structure_Subtype.toLower());
                Output.append("; name=\""+EM->Structurev[Att[i].toInt()]->Structure_CID+"\"\n");
                Output.append("Content-Transfer-Encoding: "+EM->Structurev[Att[i].toInt()]->Structure_Encoding+"\n");
                Output.append("Content-ID: <"+EM->Structurev[Att[i].toInt()]->Structure_CID+">\n\n");
            } else {
                Output.append("Content-Type: "+EM->Structurev[Att[i].toInt()]->Structure_Type.toLower()+"/"+EM->Structurev[Att[i].toInt()]->Structure_Subtype.toLower());
                Output.append("; name=\""+EM->Structurev[Att[i].toInt()]->Structure_Attrybutes.Name+"\"\n");
                Output.append("Content-Disposition: attachment; filename=\""+EM->Structurev[Att[i].toInt()]->Structure_Attrybutes.Name+"\"\n");
                Output.append("Content-Transfer-Encoding: "+EM->Structurev[Att[i].toInt()]->Structure_Encoding+"\n");
            }
            Output.append(Reply+"\n\n");
        }
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    Output.append("--------"+boundry+"--");
    S.close();
    S.connectToHostEncrypted(M->SMTPserver, 465);
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("EHLO "+M->SMTPserver.mid(5, M->SMTPserver.length()-5)+"\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("AUTH LOGIN\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString(M->User.toUtf8().toBase64()+"\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString(P.toBase64()+"\r\n").toUtf8());
    P.clear();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    if (!Reply.contains("235")) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("MAIL FROM: <"+M->User+"@"+M->IMAPserver.mid(5, M->IMAPserver.length()-5)+">\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    for (int i = 0; i < To.size(); i++) {
        S.write(QString("RCPT TO: <"+To[i]+">\r\n").toUtf8());
        Reply = readAll(&S);
        if (Reply.isEmpty()) {
            emit SendEmailS(false);
            return;
        }
    }
    S.write(QString("DATA\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString(Output+"\r\n.\r\n").toUtf8());
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit SendEmailS(false);
        return;
    }
    S.write(QString("QUIT\r\n").toUtf8());
    S.close();
    emit SendEmailS(true);
}
void SSLCON::DeleteEmail() {
    QString P = QString(calculateXOR(QByteArray::fromBase64(M->Password.toUtf8()), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)));
    QString Reply;
    QSslSocket S;
    S.connectToHostEncrypted(M->IMAPserver, 993);
    S.ignoreSslErrors();
    Reply = readAll(&S);
    if (Reply.isEmpty()) {
        emit DeleteS(false);
        return;
    }
    S.write(QString("TAG LOGIN "+M->User+" "+P+"\r\n").toUtf8());
    P.clear();
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteS(false);
        return;
    }
    if (!Reply.contains("TAG OK")) {
        emit DeleteS(false);
        return;
    }
    S.write(QString("TAG SELECT "+M->Mboxv[B]->Mbox_Name+"\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteS(false);
        return;
    }
    for (uint i = 0; i < M->Mboxv.size(); i++) {
        if (M->Mboxv[i]->Mbox_Attrybutes&Mailbox::Trash) {
            S.write(QString("TAG UID COPY "+QString::number(M->Mboxv[B]->Emailv[E]->Email_UID)+" \""+M->Mboxv[i]->Mbox_Name+"\"\r\n").toUtf8());
            Reply = readIMAP(&S);
            if (Reply.isEmpty()) {
                emit DeleteS(false);
                return;
            }
            break;
        }
    }
    S.write(QString("TAG UID STORE "+QString::number(M->Mboxv[B]->Emailv[E]->Email_UID)+" +FLAGS (\\Deleted)\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteS(false);
        return;
    }
    S.write(QString("TAG EXPUNGE\r\n").toUtf8());
    Reply = readIMAP(&S);
    if (Reply.isEmpty()) {
        emit DeleteS(false);
        return;
    }
    S.write(QString("TAG LOGOUT\r\n").toUtf8());
    S.close();
    emit DeleteS(true);
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
    QItemCheckBox(QWidget *parent, uint IID) :QCheckBox(parent) {
        ItemID = IID;
    }
    uint ItemID;
};
class MailboxTree : public QDialog {
    Q_OBJECT
public:
    MailboxTree(Mail *M, QWidget *parent) :QDialog(parent) {
        vec=&M->Mboxv;
        setWindowTitle("Choose mailboxes to use.");
        QGridLayout *Layout = new QGridLayout(this);
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
                QTreeWidgetItem *Item = new QTreeWidgetItem(Tree);
                Item->setText(0, (*vec)[i]->Mbox_Showname);
                QItemCheckBox *W = new QItemCheckBox(this, i);
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
    void AddChildren(Mailbox *I, QTreeWidgetItem* T) {
        for (uint i = 0; i < I->Mbox_Children.size(); i++) {
            QTreeWidgetItem *ItemC = new QTreeWidgetItem(T);
            ItemC->setText(0, I->Mbox_Children[i]->Mbox_Showname);
            QItemCheckBox *W = new QItemCheckBox(this, std::find(vec->begin(), vec->end(), I->Mbox_Children[i])-vec->begin());
            W->setChecked(I->Mbox_Children[i]->Mbox_Refresh);
            if (I->Mbox_Children[i]->Mbox_Attrybutes & Mailbox::Noselect) {
                W->setDisabled(true);
            } else if (I->Mbox_Children[i]->Mbox_Attrybutes & Mailbox::INBOX) {
                W->setChecked(true);
            }
            Tree->setItemWidget(ItemC, 1, W);
            QItemCheckBox *ParentCheckBox = qobject_cast  < QItemCheckBox* > (Tree->itemWidget(T, 1));
            connect(ParentCheckBox, SIGNAL(toggled(bool)), W, SLOT(setChecked(bool)));
            AddChildren(I->Mbox_Children[i], ItemC);
        }
    }
private:
    QTreeWidget *Tree;
    vector  <Mailbox*>  *vec;
    QPushButton *Cancel;
    QPushButton *OK;
    void OKChildren(QTreeWidgetItem *Itm) {
        for (int i = 0; i < Itm->childCount(); i++) {
            OKChildren(Itm->child(i));
        }
        QItemCheckBox *W = qobject_cast<QItemCheckBox*>(Tree->itemWidget(Itm, 1));
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
    EditDialog(Mail *A, QWidget *parent) :QDialog(parent) {
        M = A;
        setWindowTitle("Edit mail settings.");
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
        QGridLayout *L = new QGridLayout(this);
        L->addWidget(User, 0, 0);
        L->addWidget(U, 0, 1);
        L->addWidget(Passwd, 1, 0);
        L->addWidget(P, 1, 1);
        L->addWidget(IMAP, 2, 0);
        L->addWidget(I, 2, 1);
        L->addWidget(SMTP, 3, 0);
        L->addWidget(S, 3, 1);
        QHBoxLayout *H = new QHBoxLayout();
        H->addWidget(Cancel);
        H->addWidget(OKB);
        L->addLayout(H, 4, 1, 1, 2);
    }
    ~EditDialog() {
        M = NULL;
    }
private:
    Mail *M;
    QLabel *User;
    QLabel *Passwd;
    QLabel *IMAP;
    QLabel *SMTP;
    QLineEdit *U;
    QLineEdit *P;
    QLineEdit *I;
    QLineEdit *S;
    QPushButton *OKB;
    QPushButton *Cancel;
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
        QLineEdit *I = qobject_cast<QLineEdit*>(QObject::sender());
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
        Forward,
        Reply
    };
    Sender(Email *E, Type I, QCompleter *C, SSLCON *SSL, QWidget *parent) :QDialog(parent) {
        setWindowTitle("Send Mail");
        setMinimumWidth(600);
        this->SSL = SSL;
        S = new QLabel("Subject: ", this);
        T = new QLabel("To: ", this);
        A = new QLabel("Attachments", this);
        Subject = new QLineEdit(this);
        To = new QLineEdit(this);
        To->setCompleter(C);
        Text = new QTextBrowser(this);
        Text->setReadOnly(false);
        Receivers = new QListWidget(this);
        connect(Receivers, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DelTo(QModelIndex)));
        Attachments = new QListWidget(this);
        connect(Attachments, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DelAt(QModelIndex)));
        AddTo = new QPushButton(this);
        AddTo->setIcon(QIcon(":/main/Add.png"));
        connect(AddTo, SIGNAL(clicked()), this, SLOT(ATo()));
        AddAttchment = new QPushButton(this);
        AddAttchment->setIcon(QIcon(":/main/Add.png"));
        connect(AddAttchment, SIGNAL(clicked()), this, SLOT(AAt()));
        OKB = new QPushButton(this);
        OKB->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        connect(OKB, SIGNAL(clicked()), this, SLOT(OK()));
        Cancel = new QPushButton(this);
        Cancel->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
        connect(Cancel, SIGNAL(clicked()), this, SLOT(Can()));
        if (I == Forward) {
            Subject->setText("Fwd: "+E->Email_Subject);
            Text->setPlainText("Message forwarded:\n----------"+E->Email_Body[0]+"----------");
        }
        if (I == Reply) {
            Subject->setText("Re: "+E->Email_Subject);
            if (!E->Email_From.EMailA.contains("noreply")) {
                QListWidgetItem *Itm = new QListWidgetItem(E->Email_From.EMailA+" "+E->Email_From.Name);
                Receivers->addItem(Itm);
            }
            Text->setPlainText("Message replied:\n----------\n"+E->Email_Body[0]+"\n----------\n");
        }
        for (uint i = 0; i < E->Structurev.size(); i++) {
            if (E->Structurev[i]->Structure_Type == "TEXT"&&E->Structurev[i]->Structure_Subtype == "PLAIN") {
                i++;
            }
            if (i == E->Structurev.size()) {
                break;
            }
            QListWidgetItem *Itm;
            if (E->Structurev[i]->Structure_Attrybutes.Name.isEmpty()) {
                if (E->Structurev[i]->Structure_CID == "NIL") {
                    Itm = new QListWidgetItem("text."+E->Structurev[i]->Structure_Subtype.toLower(), Attachments);
                } else {
                    Itm = new QListWidgetItem(E->Structurev[i]->Structure_CID+"."+E->Structurev[i]->Structure_Subtype.toLower(), Attachments);
                }
            } else {
                Itm = new QListWidgetItem(NameFilter(E->Structurev[i]->Structure_Attrybutes.Name), Attachments);
            }
            Itm->setToolTip(QString::number(i));
        }
        QGridLayout *L = new QGridLayout(this);
        L->addWidget(S, 0, 0);
        L->addWidget(Subject, 0, 1);
        L->addWidget(T, 1, 0);
        L->addWidget(Receivers, 1, 1);
        L->addWidget(AddTo, 2, 0);
        L->addWidget(To, 2, 1);
        L->addWidget(Text, 3, 0, 1, 2);
        L->addWidget(A, 4, 0);
        L->addWidget(Attachments, 4, 1, 2, 1);
        L->addWidget(AddAttchment, 5, 0);
        QHBoxLayout *B = new QHBoxLayout();
        B->addWidget(Cancel);
        B->addWidget(OKB);
        L->addLayout(B, 6, 0, 1, 2);
    }
private:
    SSLCON *SSL;
    QLabel *S;
    QLabel *T;
    QLabel *A;
    QLineEdit *Subject;
    QLineEdit *To;
    QTextBrowser *Text;
    QListWidget *Receivers;
    QListWidget *Attachments;
    QPushButton *AddTo;
    QPushButton *AddAttchment;
    QPushButton *OKB;
    QPushButton *Cancel;
private slots:
    void OK() {
        if (Subject->text().isEmpty()) {
            if (Subject->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(Subject, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            Subject->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else if (Receivers->count() == 0) {
            if (To->styleSheet() != "QLineEdit{background: #FF8888;}") {
                connect(To, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
            To->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            QString Months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            QString Header="Subject: =?UTF-8?B?"+Subject->text().toUtf8().toBase64()+"?=\r\nDate: "+QDateTime::currentDateTime().toString("dd ")+Months[QDateTime::currentDateTime().date().month()-1]+QDateTime::currentDateTime().toString(" MM yyyy hh:mm:ss");
            QList  <QString>  To;
            for (int i = 0; i < Receivers->count(); i++) {
                QString M = Receivers->item(i)->text();
                To.append(M.mid(0, M.indexOf(" ")));
            }
            QList  <QString>  Att;
            for (int i = 0; i < Attachments->count(); i++) {
                if (QFile(Attachments->item(i)->toolTip()).exists()) {
                    Att.append("<"+Attachments->item(i)->toolTip()+">");
                } else {
                    Att.append(Attachments->item(i)->toolTip());
                }
            }
            SSL->SendEmail(Header, To, Text->document()->toPlainText(), Att);
            SSL->setMethod(SSLCON::Send);
            SSL->start();
            this->accept();
        }
    }
    void Can() {
        this->reject();
    }
    void ATo() {
        if (!To->text().isEmpty()) {
            bool OK = true;
            for (int i = 0; i < Receivers->count(); i++) {
                if (Receivers->item(i)->toolTip() == To->text()) {
                    OK = false;
                    break;
                }
            }
            if (OK) {
                QListWidgetItem *Itm = new QListWidgetItem(To->text());
                Itm->setToolTip(To->text());
                Receivers->addItem(Itm);
                disconnect(To, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
                To->clear();
            }
        }
    }
    void AAt() {
        QString Path = QFileDialog::getOpenFileName(this, "Add attachment", QDir::homePath());
        if (!Path.isEmpty()) {
            bool OK = true;
            for (int i = 0; i < Attachments->count(); i++) {
                if (Attachments->item(i)->toolTip() == Path) {
                    OK = false;
                    break;
                }
            }
            if (OK) {
                QString Name = Path.mid(Path.lastIndexOf("/")+1, Path.length()-Path.lastIndexOf("/")-1);
                QListWidgetItem *Itm = new QListWidgetItem(Name);
                Itm->setToolTip(Path);
                Attachments->addItem(Itm);
                disconnect(Subject, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
            }
        }
    }
    void DelTo(QModelIndex I) {
        delete Receivers->item(I.row());
    }
    void DelAt(QModelIndex I) {
        delete Attachments->item(I.row());
    }
    void row(QString Input) {
        QLineEdit *I = qobject_cast<QLineEdit*>(QObject::sender());
        if (Input.isEmpty()) {
            I->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            I->setStyleSheet("QQLineEdit{background: white;}");
        }
    }
};

qorgMail::qorgMail(QWidget *parent, qorgAB *AB) :QWidget(parent) {
    this->AB = AB;
    AvailableServices.append(Services("Gmail", "imap.gmail.com", "smtp.gmail.com"));
    AvailableServices.append(Services("Wp.pl", "imap.wp.pl", "smtp.wp.pl"));
    AvailableServices.append(Services("Linux.pl", "imap.linux.pl", "smtp.linux.pl"));
    Layout = new QGridLayout(this);
    Layout->setMargin(0);
    Mailboxes = new QTreeWidget(this);
    Mailboxes->header()->hide();
    Mailboxes->setFixedWidth(110);
    Mailboxes->setIndentation(10);
    connect(Mailboxes, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(chooseMbox(QTreeWidgetItem*)));
    MailView = new QTreeWidget(this);
    MailView->setColumnCount(3);
    MailView->setHeaderLabels(QStringList() << "Topic" << "From" << "Date");
    MailView->header()->setStretchLastSection(false);
    MailView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    MailView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    MailView->setColumnWidth(2, 120);
    connect(MailView, SIGNAL(clicked(QModelIndex)), this, SLOT(chooseEmail(QModelIndex)));
    ReadMail = new QWebView(this);
    connect(ReadMail->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, QList <QSslError>)),
            this, SLOT(HTTPSS(QNetworkReply*, QList <QSslError>)));
    Quene=-1;
    AttachmentList = new QListWidget(this);
    connect(AttachmentList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(downloadAttachment(QModelIndex)));
    QWidget *Wi = new QWidget(this);
    QVBoxLayout *V = new QVBoxLayout(Wi);
    V->setMargin(0);
    V->addWidget(ReadMail);
    V->addWidget(AttachmentList);
    Split = new QSplitter(Qt::Vertical, this);
    Split->addWidget(MailView);
    Split->addWidget(Wi);
    Refresh = new QPushButton(QIcon(":/main/Refresh.png"), "", this);
    Refresh->setToolTip("Refresh mail");
    connect(Refresh, SIGNAL(clicked()), this, SLOT(RefreshS()));
    Send = new QPushButton(QIcon(":/main/Mail.png"), "", this);
    Send->setToolTip("Send email");
    connect(Send, SIGNAL(clicked()), this, SLOT(SendMail()));
    Delete = new QPushButton(QIcon(":/main/Delete.png"), "", this);
    Delete->setToolTip("Delete email");
    connect(Delete, SIGNAL(clicked()), this, SLOT(DeleteEmail()));
    Forward = new QPushButton(QIcon(":/mail/Forward.png"), "", this);
    Forward->setToolTip("Forward email");
    connect(Forward, SIGNAL(clicked()), this, SLOT(SendMail()));
    Reply = new QPushButton(QIcon(":/mail/Reply.png"), "", this);
    Reply->setToolTip("Reply to email");
    connect(Reply, SIGNAL(clicked()), this, SLOT(SendMail()));
    F << Mailboxes << MailView << Split << Refresh << Send << Delete << Forward << Reply;
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
    setLayoutC();
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
    ReadMail->page()->deleteLater();
}
QString qorgMail::output() {
    QString out;
    for (uint i = 0; i < Mailv.size(); i++) {
        QString work;
        work.append(OutputTools(Mailv[i].Name, "NAME"));
        work.append(OutputTools(Mailv[i].IMAPserver, "IMAP"));
        work.append(OutputTools(Mailv[i].SMTPserver, "SMTP"));
        work.append(OutputTools(Mailv[i].User, "USER"));
        work.append(OutputTools(Mailv[i].Password, "PWD"));
        QString Mailboxes;
        for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
            QString work1;
            work1.append(OutputTools(Mailv[i].Mboxv[j]->Mbox_Name, "MBOX_NAME"));
            work1.append(OutputTools(Mailv[i].Mboxv[j]->Mbox_Showname, "MBOX_SHOWNAME"));
            work1.append(OutputTools(static_cast<int>(Mailv[i].Mboxv[j]->Mbox_Attrybutes), "MBOX_ATTRYBUTES"));
            work1.append(OutputTools(Mailv[i].Mboxv[j]->Mbox_Refresh, "MBOX_REFRESH"));
            work1.append(OutputTools(Mailv[i].Mboxv[j]->Mbox_Top, "MBOX_TOP"));
            QString Childrens;
            for (uint k = 0; k < Mailv[i].Mboxv[j]->Mbox_Children.size(); k++) {
                for (uint l = 0; l < Mailv[i].Mboxv.size(); l++) {
                    if (Mailv[i].Mboxv[j]->Mbox_Children[k] == Mailv[i].Mboxv[l]) {
                        Childrens.append(OutputTools(static_cast<int>(l), "MBOX_CHILDINT"));
                        break;
                    }
                }
            }
            work1.append(OutputToolsS(Childrens, "MBOX_CHILDREN"));
            QString Emails;
            for (uint k = 0; k < Mailv[i].Mboxv[j]->Emailv.size(); k++) {
                QString work2;
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_Subject, "EMAIL_SUBJECT"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_From.Name, "EMAIL_FROM_NAME"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_From.EMailA, "EMAIL_FROM_EMAILA"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_Date.toString(Qt::ISODate), "EMAIL_DATE"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_Body[0], "EMAIL_BODY0"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_Body[1], "EMAIL_BODY1"));
                work2.append(OutputTools(static_cast<int>(Mailv[i].Mboxv[j]->Emailv[k]->Email_UID), "EMAIL_UID"));
                work2.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Email_Flags, "EMAIL_FLAGS"));
                QString Structures;
                for (uint l = 0; l < Mailv[i].Mboxv[j]->Emailv[k]->Structurev.size(); l++) {
                    QString work3;
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Number, "STRUCTURE_NUMBER"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Type, "STRUCTURE_TYPE"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Subtype, "STRUCTURE_SUBTYPE"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Attrybutes.Charset, "STRUCTURE_ATTRYBUTES_CHARSET"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Attrybutes.Name, "STRUCTURE_ATTRYBUTES_NAME"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_CID, "STRUCTURE_CID"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Descryption, "STRUCTURE_DESRIPTION"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Encoding, "STRUCTURE_ENCODING"));
                    work3.append(OutputTools(static_cast<int>(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Size), "STRUCTURE_SIZE"));
                    work3.append(OutputTools(static_cast<int>(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Lines), "STRUCTURE_LINES"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_MD5, "STRUCTURE_MD5"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Disposition, "STRUCTURE_DISPOSITION"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Language, "STRUCTURE_LANGUAGE"));
                    work3.append(OutputTools(Mailv[i].Mboxv[j]->Emailv[k]->Structurev[l]->Structure_Location, "STRUCTURE_LOCATION"));
                    Structures.append(OutputToolsS(work3, "STRUCTURE"));
                }
                work2.append(OutputToolsS(Structures, "STRUCTUREV"));
                Emails.append(OutputToolsS(work2, "EMAIL"));
            }
            work1.append(OutputToolsS(Emails, "EMAILV"));
            Mailboxes.append(OutputToolsS(work1, "MAILBOX"));
        }
        work.append(OutputToolsS(Mailboxes, "MBOXV"));
        out.append(OutputToolsS(work, "MAIL"));
    }
    out = OutputToolsS(out, "MAILV");
    return out;
}
void qorgMail::input(QString Input) {
    while (Input.contains("<MAIL>")) {
        QString MS = InputSS(Input, "MAIL");
        Mail M;
        M.Name = InputS(MS, "NAME");
        M.IMAPserver = InputS(MS, "IMAP");
        M.SMTPserver = InputS(MS, "SMTP");
        M.User = InputS(MS, "USER");
        M.Password = InputS(MS, "PWD");
        QString Mailboxes = InputSS(MS, "MBOXV");
        QList  <vector <int> > ConnectPointer;
        while (Mailboxes.contains("<MAILBOX>")) {
            QString MBS = InputSS(Mailboxes, "MAILBOX");
            Mailbox* MB = new Mailbox();
            MB->Mbox_Name = InputS(MBS, "MBOX_NAME");
            MB->Mbox_Showname = InputS(MBS, "MBOX_SHOWNAME");
            MB->Mbox_Attrybutes = InputI(MBS, "MBOX_ATTRYBUTES");
            MB->Mbox_Refresh = InputB(MBS, "MBOX_REFRESH");
            MB->Mbox_Top = InputB(MBS, "MBOX_TOP");
            QString MBCS = InputSS(MBS, "MBOX_CHILDREN");
            vector  <int>  Pointers;
            while (MBCS.contains("<MBOX_CHILDINT>")) {
                Pointers.push_back(InputI(MBCS, "MBOX_CHILDINT"));
                MBCS.remove(MBCS.indexOf("<MBOX_CHILDINT>"), MBCS.indexOf("</MBOX_CHILDINT>")-MBCS.indexOf("<MBOX_CHILDINT>")+16);
            }
            ConnectPointer.append(Pointers);
            QString Emails = InputSS(MBS, "EMAILV");
            while (Emails.contains("<EMAIL>")) {
                QString ES = InputSS(Emails, "EMAIL");
                Email *E = new Email();
                E->Email_Subject = InputS(ES, "EMAIL_SUBJECT");
                E->Email_From.Name = InputS(ES, "EMAIL_FROM_NAME");
                E->Email_From.EMailA = InputS(ES, "EMAIL_FROM_EMAILA");
                E->Email_Date = QDateTime::fromString(InputS(ES, "EMAIL_DATE"), Qt::ISODate);
                E->Email_Body[0]=InputS(ES, "EMAIL_BODY0");
                E->Email_Body[1]=InputS(ES, "EMAIL_BODY1");
                E->Email_UID = InputI(ES, "EMAIL_UID");
                E->Email_Flags = InputI(ES, "EMAIL_FLAGS");
                QString Structures = InputSS(ES, "STRUCTUREV");
                while (Structures.contains("<STRUCTURE>")) {
                    QString SS = InputSS(Structures, "STRUCTURE");
                    Structure *S = new Structure();
                    S->Structure_Number = InputS(SS, "STRUCTURE_NUMBER");
                    S->Structure_Type = InputS(SS, "STRUCTURE_TYPE");
                    S->Structure_Subtype = InputS(SS, "STRUCTURE_SUBTYPE");
                    S->Structure_Attrybutes.Charset = InputS(SS, "STRUCTURE_ATTRYBUTES_CHARSET");
                    S->Structure_Attrybutes.Name = InputS(SS, "STRUCTURE_ATTRYBUTES_NAME");
                    S->Structure_CID = InputS(SS, "STRUCTURE_CID");
                    S->Structure_Descryption = InputS(SS, "STRUCTURE_DESRIPTION");
                    S->Structure_Encoding = InputS(SS, "STRUCTURE_ENCODING");
                    S->Structure_Size = InputI(SS, "STRUCTURE_SIZE");
                    S->Structure_Lines = InputI(SS, "STRUCTURE_LINES");
                    S->Structure_MD5 = InputS(SS, "STRUCTURE_MD5");
                    S->Structure_Disposition = InputS(SS, "STRUCTURE_DISPOSITION");
                    S->Structure_Language = InputS(SS, "STRUCTURE_LANGUAGE");
                    S->Structure_Location = InputS(SS, "STRUCTURE_LOCATION");
                    E->Structurev.push_back(S);
                    Structures.remove(Structures.indexOf("<STRUCTURE>"), Structures.indexOf("</STRUCTURE>")-Structures.indexOf("<STRUCTURE>")+12);
                }
                MB->Emailv.push_back(E);
                Emails.remove(Emails.indexOf("<EMAIL>"), Emails.indexOf("</EMAIL>")-Emails.indexOf("<EMAIL>")+8);
            }
            M.Mboxv.push_back(MB);
            Mailboxes.remove(Mailboxes.indexOf("<MAILBOX>"), Mailboxes.indexOf("</MAILBOX>")-Mailboxes.indexOf("<MAILBOX>")+10);
        }
        for (int i = 0; i < ConnectPointer.size(); i++) {
            for (uint j = 0; j < ConnectPointer[i].size(); j++) {
                M.Mboxv[i]->Mbox_Children.push_back(M.Mboxv[ConnectPointer[i][j]]);
            }
        }
        Mailv.push_back(M);
        Input.remove(Input.indexOf("<MAIL>"), Input.indexOf("</MAIL>")-Input.indexOf("<MAIL>")+7);
    }
    setLayoutC();
}
void qorgMail::setMail(QString currentMbox) {
    if (currentMbox != MailCat) {
        MailCat = currentMbox;
        if (MailCat.isEmpty()) {
            setLayoutC();
        } else {
            currentEmail=-1;
            Mailboxes->clear();
            setLayoutF();
            for (uint i = 0; i < Mailv.size(); i++) {
                if (Mailv[i].Name == MailCat) {
                    currentMail = i;
                    break;
                }
            }
            int I = 0;
            for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
                if (Mailv[currentMail].Mboxv[i]->Mbox_Top) {
                    QTreeWidgetItem* Itm = new QTreeWidgetItem();
                    Itm->setText(0, Mailv[currentMail].Mboxv[i]->Mbox_Showname);
                    Itm->setToolTip(0, Bit7ToBit8(Mailv[currentMail].Mboxv[i]->Mbox_Name));
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
void qorgMail::setMailbox(int I) {
    currentMailbox = I;
    MailView->clear();
    ReadMail->setHtml("");
    AttachmentList->clear();
    for (int i = Mailv[currentMail].Mboxv[I]->Emailv.size()-1; i > -1; i--) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem();
        if (!(Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_Flags&Email::Seen)) {
            Itm->setFont(0, QFont("", Itm->font(0).pixelSize(), QFont::Bold));
        }
        Itm->setText(0, Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_Subject);
        Itm->setToolTip(0, Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_Subject);
        Itm->setText(1, Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_From.EMailA);
        Itm->setToolTip(1, Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_From.EMailA);
        Itm->setText(2, Mailv[currentMail].Mboxv[I]->Emailv[i]->Email_Date.toString("dd/MM/yyyy hh:mm"));
        MailView->addTopLevelItem(Itm);
    }
    MailView->scrollToTop();
}
QStringList qorgMail::getCategories() {
    QStringList list;
    for (uint i = 0; i < Mailv.size(); i++) {
        list.append(Mailv[i].Name);
    }
    list.sort();
    return list;
}
void qorgMail::setLayoutC() {
    List->clear();
    for (uint i = 0; i < Mailv.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(List);
        Itm->setText(0, Mailv[i].Name);
        QItemPushButton *Edit = new QItemPushButton(QIcon(":/main/Edit.png"), this, i);
        Edit->setStyleSheet("QPushButton {border: 0px solid white;}");
        connect(Edit, SIGNAL(clicked(uint)), this, SLOT(EditMail(uint)));
        List->setItemWidget(Itm, 1, Edit);
        QItemPushButton *Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, i);
        connect(Delete, SIGNAL(clicked(uint)), this, SLOT(DeleteMail(uint)));
        List->setItemWidget(Itm, 2, Delete);
    }
    for (int i = 0; i < F.size(); i++) {
        Layout->removeWidget(F[i]);
        F[i]->hide();
    }
    Layout->addWidget(List, 0, 0, 2, 1);
    QGridLayout *ADD = new QGridLayout();
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
void qorgMail::setLayoutF() {
    for (int i = 0; i < C.size(); i++) {
        Layout->removeWidget(C[i]);
        C[i]->hide();
    }
    Layout->addWidget(Refresh, 0, 0);
    Layout->addWidget(Send, 0, 1);
    Layout->addWidget(Delete, 0, 2);
    Layout->addWidget(Forward, 0, 3);
    Layout->addWidget(Reply, 0, 4);
    Layout->addWidget(Mailboxes, 0, 6, 2, 1);
    Layout->addWidget(Split, 1, 0, 1, 5);
    for (int i = 0; i < F.size(); i++) {
        F[i]->show();
    }
}
void qorgMail::addChildren(Mailbox *I, QTreeWidgetItem *W) {
    for (uint i = 0; i < I->Mbox_Children.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem();
        Itm->setText(0, I->Mbox_Children[i]->Mbox_Showname);
        Itm->setToolTip(0, Bit7ToBit8(I->Mbox_Children[i]->Mbox_Name));
        if (I->Mbox_Children[i]->Mbox_Children.size() != 0) {
            addChildren(I->Mbox_Children[i], Itm);
        }
        W->addChild(Itm);
    }
}
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
        Mail *M = new Mail;
        bool EX = false;
        for (uint i = 0; i < Mailv.size(); i++) {
            if (Mailv[i].IMAPserver == IMAPS->text()&&Mailv[i].User == Username->text()) {
                EX = true;
                break;
            }
        }
        if (!EX) {
            M->User = Username->text();
            M->Password = QString(calculateXOR(Passwd->text().toUtf8(), QCryptographicHash::hash(M->User.toUtf8(), QCryptographicHash::Sha3_512)).toBase64());
            M->IMAPserver = IMAPS->text();
            M->SMTPserver = SMTPS->text();
            M->Name = M->User;
            SSLCON *T = new SSLCON(M);
            connect(T, SIGNAL(LoginS(bool)), this, SLOT(LoginS(bool)));
            connect(T, SIGNAL(MailboxesS(bool)), this, SLOT(MailboxesS(bool)));
            connect(T, SIGNAL(EmailS(bool)), this, SLOT(EmailS(bool)));
            T->start();
            T->setMethod(SSLCON::Login);
            T->Downloading->exec();
        } else {
            QMessageBox::critical(this, "Mail error", "Mail already exist.");
        }
    }
    Username->clear();
    Passwd->clear();
}
void qorgMail::row(QString Input) {
    QLineEdit *I = qobject_cast<QLineEdit*>(QObject::sender());
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
    setMail(Mailv[I.row()].Name);
    emit doubleClick(Mailv[I.row()].Name);
}
void qorgMail::EditMail(uint IID) {
    Mail *I=&Mailv[IID];
    Mail *A = new Mail;
    A->User = I->User;
    A->IMAPserver = I->IMAPserver;
    A->SMTPserver = I->SMTPserver;
    if ((new EditDialog(A, this))->exec() == QDialog::Accepted) {
        currentMail = IID;
        SSLCON *T = new SSLCON(A);
        connect(T, SIGNAL(MailboxesS(bool)), this, SLOT(EditMailS(bool)));
        T->start();
        T->setMethod(SSLCON::Mailboxes);
    }
}
void qorgMail::EditMailS(bool I) {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        Mailv[currentMail].Name = T->M->Name;
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
        delete T->M;
        if((new MailboxTree(&Mailv[currentMail], this))->exec() == QDialog::Accepted) {
            T = new SSLCON(&Mailv[currentMail]);
            connect(T, SIGNAL(EmailS(bool)), this, SLOT(UpdateEmail(bool)));
            T->Bar->deleteLater();
            T->Bar = new QProgressBar();
            T->start();
            T->setMethod(SSLCON::Emails);
        }
    } else {
        QMessageBox::critical(this, "Error", "Wrong settings.");
    }
    List->setEnabled(true);
    Username->setEnabled(true);
    Passwd->setEnabled(true);
    Choose->setEnabled(true);
    IMAPS->setEnabled(true);
    SMTPS->setEnabled(true);
    AddB->setEnabled(true);
}
void qorgMail::DeleteMail(uint IID) {
    Mail *I=&Mailv[IID];
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
    setLayoutC();
    emit updateTree();
}

void qorgMail::chooseMbox(QTreeWidgetItem *I) {
    int Int = 0;
    currentEmail=-1;
    for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
        if (Bit7ToBit8(Mailv[currentMail].Mboxv[i]->Mbox_Name) == I->toolTip(0)) {
            Int = i;
        }
    }
    if (!(Mailv[currentMail].Mboxv[Int]->Mbox_Attrybutes&Mailbox::Noselect)) {
        setMailbox(Int);
    }
}
void qorgMail::chooseEmail(QModelIndex I) {
    currentEmail = Mailv[currentMail].Mboxv[currentMailbox]->Emailv.size()-I.row()-1;
    Email *E = Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail];
    if (E->Email_Body[1].isEmpty()) {
        QString Text = E->Email_Body[0];
        Text="<html><body>"+Text+"</body ></html>";
        Text.replace("\n", "<br/>");
        ReadMail->setHtml(Text);
    } else {
        QString HTML = E->Email_Body[1];
        if (HTML.contains("\"cid:")) {
            bool Downloaded = true;
            while (HTML.contains("\"cid:")) {
                int s = HTML.indexOf("\"cid:");
                int e = HTML.indexOf("\"", s+5);
                QString CID = HTML.mid(s+5, e-s-5);
                for (uint i = 1; i < E->Structurev.size(); i++) {
                    if (E->Structurev[i]->Structure_CID == CID) {
                        if (!QFile(QDir::tempPath()+"/"+CID+"."+E->Structurev[i]->Structure_Subtype.toLower()).exists()) {
                            downloadAttachment(i, QDir::tempPath()+"/"+CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                            Downloaded = false;
                        }
                        QUrl A = QUrl::fromLocalFile(QDir::tempPath()+"/"+CID+"."+E->Structurev[i]->Structure_Subtype.toLower());
                        HTML = HTML.mid(0, s)+"\""+A.toString()+HTML.mid(e, HTML.length()-e);
                        break;
                    }
                }
            }
            if (Downloaded) {
                ReadMail->setHtml(HTML);
            }
        } else {
            ReadMail->setHtml(HTML);
        }
    }
    if (!(E->Email_Flags&Email::Seen)) {
        E->Email_Flags|=Email::Seen;
        MailView->selectedItems().first()->setFont(0, QFont());
    }
    AttachmentList->clear();
    for (uint i = 0; i < E->Structurev.size(); i++) {
        if (E->Structurev[i]->Structure_Type == "TEXT"&&E->Structurev[i]->Structure_Subtype == "PLAIN"&&i < E->Structurev.size()-1) {
            i++;
        } else if (E->Structurev[i]->Structure_Type == "TEXT"&&E->Structurev[i]->Structure_Subtype == "PLAIN"&&i == E->Structurev.size()-1) {
            break;
        }
        QListWidgetItem *Itm = new QListWidgetItem(AttachmentList);
        if (E->Structurev[i]->Structure_Attrybutes.Name.isEmpty()) {
            if (E->Structurev[i]->Structure_CID == "NIL") {
                Itm->setText("text."+E->Structurev[i]->Structure_Subtype.toLower());
            }
        } else {
            Itm->setText(NameFilter(E->Structurev[i]->Structure_Attrybutes.Name));
        }
    }
}
void qorgMail::LoginS(bool I) {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        T->setMethod(SSLCON::Mailboxes);
    } else {
        T->Downloading->reject();
        delete T->M;
        T->setMethod(SSLCON::Stop);
        QMessageBox::critical(this, "Login error", "Wrong username or password.");
    }
}
void qorgMail::MailboxesS(bool I) {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        if ((new MailboxTree(T->M, this))->exec() == QDialog::Accepted) {
            T->setMethod(SSLCON::Emails);
        } else {
            T->Downloading->reject();
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                delete T->M->Mboxv[i];
            }
            delete T->M;
            T->setMethod(SSLCON::Stop);
        }
    } else {
        T->Downloading->reject();
        T->setMethod(SSLCON::Stop);
    }
}
void qorgMail::EmailS(bool I) {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
        T->Downloading->accept();
        Mailv.push_back((*T->M));
        setMail(T->M->Name);
        delete T->M;
        emit updateTree();
    } else {
        T->Downloading->reject();
    }
}
void qorgMail::downloadAttachment(QModelIndex I) {
    QString name = NameFilter(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I.row()+1]->Structure_Attrybutes.Name);
    if (name.isEmpty()) {
        if (Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I.row()+1]->Structure_CID == "NIL") {
            name="text."+Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[I.row()+1]->Structure_Subtype.toLower();
        }
    }
    QString path = QFileDialog::getSaveFileName(this, "Save attachment to:", QDir::homePath()+"/"+name);
    if (!path.isEmpty()) {
        SSLCON *T = new SSLCON(&Mailv[currentMail]);
        T->start();
        T->DownloadAttachmentData(currentMailbox, currentEmail, I.row()+1, path);
        T->setMethod(SSLCON::Attachment);
    }
}
void qorgMail::downloadAttachment(uint I, QString path) {
    SSLCON *T = new SSLCON(&Mailv[currentMail]);
    connect(T, SIGNAL(AttachmentS(bool)), this, SLOT(AttachmentS(bool)));
    T->start();
    T->DownloadAttachmentData(currentMailbox, currentEmail, I, path);
    T->setMethod(SSLCON::Attachment);
    Quene++;
}
void qorgMail::AttachmentS(bool I) {
    if (I) {
        if (Quene != -1) {
            if (Quene == 0) {
                QList  <QString>  ToClear;
                QString HTML = Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Email_Body[1];
                while (HTML.contains("\"cid:")) {
                    int s = HTML.indexOf("\"cid:");
                    int e = HTML.indexOf("\"", s+5);
                    QString CID = HTML.mid(s+5, e-s-5);
                    for (uint i = 1; i < Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev.size(); i++) {
                        if (Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[i]->Structure_CID == CID) {
                            QUrl A = QUrl::fromLocalFile(QDir::tempPath()+"/"+CID+"."+Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[i]->Structure_Subtype.toLower());
                            ToClear.append(QDir::tempPath()+"/"+CID+"."+Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[i]->Structure_Subtype.toLower());
                            HTML = HTML.mid(0, s)+"\""+A.toString()+HTML.mid(e, HTML.length()-e);
                            break;
                        }
                    }
                }
                ReadMail->setHtml(HTML);
                for (int i = 0; i < ToClear.size(); i++) {
                    QFile::remove(ToClear[i]);
                }
            }
            Quene--;
        }
    }
}
void qorgMail::RefreshS() {
    Mail *M = new Mail();
    M->IMAPserver = Mailv[currentMail].IMAPserver;
    M->User = Mailv[currentMail].User;
    M->Password = Mailv[currentMail].Password;
    SSLCON *T = new SSLCON(M);
    T->setMethod(SSLCON::Mailboxes);
    T->start();
    connect(T, SIGNAL(MailboxesS(bool)), this, SLOT(RefreshS(bool)));
}
void qorgMail::RefreshS(bool I) {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
    if (I) {
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
            while ((new MailboxTree(&Mailv[currentMail], this))->exec() != QDialog::Accepted) {
                QMessageBox::information(this, "Mailbox tree changed.", "Mailbox structure is different, please update.");
            }
        } else {
            for (uint i = 0; i < T->M->Mboxv.size(); i++) {
                delete T->M->Mboxv[i];
            }
        }
        T->setMethod(SSLCON::Stop);
        delete T->M;
        T = new SSLCON(&Mailv[currentMail]);
        T->Bar->deleteLater();
        T->Bar = new QProgressBar();
        T->start();
        T->setMethod(SSLCON::Emails);
        connect(T, SIGNAL(EmailS(bool)), this, SLOT(UpdateEmail(bool)));
    } else {
        T->setMethod(SSLCON::Stop);
    }
}
void qorgMail::SendMail() {
    QPushButton *T = qobject_cast<QPushButton*>(QObject::sender());
    SSLCON *S = new SSLCON(&Mailv[currentMail]);
    S->SetBE(currentMailbox, currentEmail);
    connect(S, SIGNAL(SendEmailS(bool)), this, SLOT(SendEmailS(bool)));
    QStringList Z;
    Z.append(AB->getEmails());
    for (uint i = 0; i < Mailv[currentMail].Mboxv.size(); i++) {
        for (uint j = 0; j < Mailv[currentMail].Mboxv[i]->Emailv.size(); j++) {
            if (!Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From.EMailA.contains("noreply")) {
                QString S = Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From.EMailA+" "+Mailv[currentMail].Mboxv[i]->Emailv[j]->Email_From.Name;
                if (!Z.contains(S)) {
                    Z.append(S);
                }
            }
        }
    }
    QCompleter *Completer = new QCompleter(Z);
    Completer->setCaseSensitivity(Qt::CaseInsensitive);
    Z.clear();
    if (T == Send) {
        (new Sender((new Email), Sender::Normal, Completer, S, this))->exec();
    } else if (T == Forward) {
        if (currentEmail != -1) {
            (new Sender(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail], Sender::Forward, Completer, S, this))->exec();
        }
    } else {
        if (currentEmail != -1) {
            (new Sender(Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail], Sender::Reply, Completer, S, this))->exec();
        }
    }
    Completer->deleteLater();
}
void qorgMail::SendEmailS(bool I) {
    if (I) {
        QMessageBox::information(this, "Sending", "Mail sent.");
    } else {
        QMessageBox::critical(this, "Error", "Error during sending mail");
    }
}
void qorgMail::DeleteEmail() {
    if (currentEmail != -1) {
        SSLCON *T = new SSLCON(&Mailv[currentMail]);
        T->SetBE(currentMailbox, currentEmail);
        T->setMethod(SSLCON::Delete);
        connect(T, SIGNAL(DeleteS(bool)), this, SLOT(DeleteEmailS(bool)));
        T->start();
    }
}
void qorgMail::DeleteEmailS(bool I) {
    if (I) {
        for (uint i = 0; i < Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev.size(); i++) {
            delete Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail]->Structurev[i];
        }
        delete Mailv[currentMail].Mboxv[currentMailbox]->Emailv[currentEmail];
        Mailv[currentMail].Mboxv[currentMailbox]->Emailv.erase(Mailv[currentMail].Mboxv[currentMailbox]->Emailv.begin()+currentEmail);
        setMailbox(currentMailbox);
    } else {
        QMessageBox::critical(this, "Error", "Error during deleting email.");
    }
}
void qorgMail::UpdateEmail(bool I) {
    if (I) {
        setMail("");
        setMail(Mailv[currentMail].Name);
        emit updateTree();
    }
}
void qorgMail::UpdateS() {
    SSLCON *T = qobject_cast<SSLCON*>(QObject::sender());
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
void qorgMail::getUpdate() {
    UpdateQuene = 0;
    if (Mailv.size() != 0) {
        for (uint i = 0; i < Mailv.size(); i++) {
            Mail *M = new Mail();
            M->User = Mailv[i].User;
            M->Password = Mailv[i].Password;
            M->IMAPserver = Mailv[i].IMAPserver;
            for (uint j = 0; j < Mailv[i].Mboxv.size(); j++) {
                if (Mailv[i].Mboxv[j]->Mbox_Attrybutes&Mailbox::INBOX) {
                    M->Mboxv.push_back(Mailv[i].Mboxv[j]);
                    break;
                }
            }
            SSLCON *T = new SSLCON(M);
            T->setMethod(SSLCON::Emails);
            connect(T, SIGNAL(EmailS(bool)), this, SLOT(UpdateS()));
            T->start();
            ++UpdateQuene;
        }
    } else {
        emit sendUpdate("Mail: No mails.");
    }
}
void qorgMail::HTTPSS(QNetworkReply *QNR, QList<QSslError> I) {
    I.clear();
    QNR->ignoreSslErrors();
    QNR->deleteLater();
}
#include "qorgmail.moc"
