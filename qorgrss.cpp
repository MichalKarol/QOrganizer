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

#include <qorgrss.h>
#include <algorithm>
#include <vector>

uint RSSthreadCounter = 0;
QMutex RSSaccessToCounter;

RSSItem::RSSItem() {
    Title.clear();
    PubDate.setMSecsSinceEpoch(0);
    Link.clear();
    Description.clear();
    GUID.clear();
    New = true;
}
RSSChannel::RSSChannel() {
    Title.clear();
    Link.clear();
    Itemv.clear();
}

class Download :public QThread {
    Q_OBJECT
public:
    explicit Download(qorgRSS*, RSSChannel*);
    ~Download();
    RSSChannel* ChannelRSSChannel;
private:
    uint counter;
    void get();
protected:
    void run();
signals:
    void Downloaded(QString);
};
Download::Download(qorgRSS* parent, RSSChannel* Channel) :QThread(parent) {
    counter = 0;
    QMutexLocker L(&RSSaccessToCounter);
    RSSthreadCounter++;
    this->ChannelRSSChannel = Channel;
}
Download::~Download() {
    QMutexLocker L(&RSSaccessToCounter);
    RSSthreadCounter--;
}
void Download::run() {
    this->get();
    this->deleteLater();
}
void Download::get() {
    QNetworkRequest Request(QUrl::fromUserInput(ChannelRSSChannel->Link));
    QNetworkAccessManager* QNAM = new QNetworkAccessManager();
    QNetworkReply* QNRe = QNAM->get(Request);

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(QNRe, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(30000);   // 30 secs. timeout
    loop.exec();
    if (timer.isActive()) {
        timer.stop();
        QVariant possibleRedirectUrl = QNRe->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (!possibleRedirectUrl.toUrl().isEmpty()
                && QNRe->error() == QNetworkReply::NoError
                && counter < 2) {
            counter++;
            ChannelRSSChannel->Link = possibleRedirectUrl.toString();
           this->get();
        } else if (QNRe->error() != QNetworkReply::NoError
                   && counter < 2) {
            counter++;
            this->get();
        } else {
            QString Rep =QNRe->readAll();
            emit Downloaded(Rep);
        }
    } else {
       disconnect(QNRe, SIGNAL(finished()), &loop, SLOT(quit()));
       QNRe->abort();
       emit Downloaded(QString());
    }
    delete QNRe;
    delete QNAM;
}

QString stringBetween(QString Tag, QString Text) {
    QString Output;
    if (Text.contains("<" + Tag) && Text.contains(Tag + ">")) {
        Text = Text.mid(Text.indexOf("<" + Tag, 0, Qt::CaseInsensitive) + Tag.length() + 1,
                        Text.indexOf(Tag + ">", Text.indexOf("<" + Tag, 0, Qt::CaseInsensitive) + Tag.length() + 1, Qt::CaseInsensitive)
                        - Text.indexOf("<" + Tag, 0, Qt::CaseInsensitive) - Tag.length()-1);
        Text.remove("<![CDATA[");
        Text.remove("]]>");
        if (Text.contains(">")) {
            Text = Text.mid(Text.indexOf(">")+1, Text.length()-Text.indexOf(">")-1);
        }
        if (Text.contains("<")) {
            Text = Text.mid(0, Text.lastIndexOf("<"));
        }
        Text.replace("<", "&lt;");
        Text.replace(">", "&gt;");
        Text.replace("&amp;", "&");
        QTextDocument D;
        D.setHtml(Text);
        Text = D.toPlainText();
        for (int i = 0; i < Text.length()-4; i++) {
            if (Text[i] == '&' && Text[i+1] == '#' && Text[i+4] == ';') {
                QChar C(Text.mid(i+2, 2).toInt());
                Output.append(C);
                i+=4;
            } else {
                Output.append(Text[i]);
            }
        }
        Output.append(Text.mid(Text.length()-4, 4));
    }
    return Output;
}

qorgRSS::qorgRSS(QWidget* parent) :QWidget(parent) {
    currentC = -1;
    Layout = new QGridLayout(this);
    Layout->setMargin(0);
    List = new QTreeWidget(this);
    List->header()->hide();
    List->setColumnCount(2);
    List->header()->setStretchLastSection(false);
    List->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    List->setColumnWidth(1, 50);
    connect(List, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(chooseChannel(QModelIndex)));
    Lab = new QLabel("URL: ", this);
    URL = new QLineEdit(this);
    URL->setMinimumWidth(100);
    Add = new QPushButton(QIcon(":/main/Add.png"), "", this);
    Add->setShortcut(Qt::Key_Return);
    connect(Add, SIGNAL(clicked()), this, SLOT(AddS()));
    Split = new QSplitter(Qt::Vertical, this);
    Titles = new QTreeWidget(this);
    Titles->setColumnCount(2);
    Titles->header()->hide();
    Titles->header()->setStretchLastSection(false);
    Titles->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Titles->setColumnWidth(1, 120);
    connect(Titles, SIGNAL(clicked(QModelIndex)), this, SLOT(chooseItem(QModelIndex)));
    View = new QWebView(this);
    View->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(View->page()->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*, QList <QSslError>)),
            this, SLOT(HTTPSS(QNetworkReply*, QList <QSslError>)));
    connect(View->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    Link = new QLabel(this);
    Link->setTextFormat(Qt::RichText);
    Link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    Link->setOpenExternalLinks(true);
    UpdateQuene = 0;
    Refresh = new QPushButton(QIcon(":/main/Refresh.png"), "", this);
    Refresh->setMaximumWidth(30);
    connect(Refresh, SIGNAL(clicked()), this, SLOT(RefreshS()));
    QWidget* W = new QWidget(this);
    QVBoxLayout* V = new QVBoxLayout(W);
    V->setMargin(0);
    V->addWidget(View);
    QHBoxLayout* H = new QHBoxLayout();
    H->addWidget(Link);
    H->addWidget(Refresh);
    V->addLayout(H);
    Split->addWidget(Titles);
    Split->addWidget(W);
    setChannel(-1);
}
qorgRSS::~qorgRSS() {
    for (uint i = 0; i < RSSv.size(); i++) {
        for (uint j = 0; j < RSSv[i].Itemv.size(); j++) {
            delete RSSv[i].Itemv[j];
        }
    }
}
void qorgRSS::setPointer(qorgOptions* Options) {
    this->Options = Options;
}

QString qorgRSS::output() {
    QString out;
    for (uint i = 0; i < RSSv.size(); i++) {
        out.append(Output(RSSv[i].Title)+" ");
        out.append(Output(RSSv[i].Link)+" \n");
        for (uint j = 0; j < RSSv[i].Itemv.size(); j++) {
            out.append(Output(RSSv[i].Itemv[j]->Title)+" ");
            out.append(Output(RSSv[i].Itemv[j]->PubDate)+" ");
            out.append(Output(RSSv[i].Itemv[j]->Link)+" ");
            out.append(Output(RSSv[i].Itemv[j]->Description)+" ");
            out.append(Output(RSSv[i].Itemv[j]->GUID)+" ");
            out.append(Output(RSSv[i].Itemv[j]->New)+" \n");
        }
    }
    out.append("\n\n");
    return out;
}
void qorgRSS::input(QString Input) {
    if (!Input.isEmpty()) {
        QStringList A = Input.split("\n");
        RSSChannel* cChannel;
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 2: {
                RSSv.push_back(RSSChannel());
                cChannel = &RSSv.back();
                cChannel->Title = InputS(B[0]);
                cChannel->Link = InputS(B[1]);
            }break;
            case 6: {
                RSSItem* cItem = new RSSItem();
                cChannel->Itemv.push_back(cItem);
                cItem->Title = InputS(B[0]);
                cItem->PubDate = InputDT(B[1]);
                cItem->Link = InputS(B[2]);
                cItem->Description = InputS(B[3]);
                cItem->GUID = InputS(B[4]);
                cItem->New = InputB(B[5]);
            }break;
            }
        }
    }
    setLayoutC();
}
void qorgRSS::setChannel(int I) {
    if (I != currentC) {
        currentC = I;
        if (I == -1) {
            setLayoutC();
        } else {
            Titles->clear();
            View->setHtml("");
            Link->clear();
            for (uint j = 0; j < RSSv[currentC].Itemv.size(); j++) {
                QTreeWidgetItem* Itm = new QTreeWidgetItem(Titles);
                if (RSSv[currentC].Itemv[j]->New) {
                    Itm->setFont(0, QFont("", Itm->font(0).pixelSize(), QFont::Bold));
                }
                Itm->setText(0, RSSv[currentC].Itemv[j]->Title);
                Itm->setToolTip(0, RSSv[currentC].Itemv[j]->Title);
                Itm->setText(1, RSSv[currentC].Itemv[j]->PubDate.toString("HH:mm dd.MM.yyyy"));
            }
            Titles->scrollToTop();
            setLayoutF();
        }
    }
}
QStringList qorgRSS::getChannels() {
    QStringList  List;
    for (uint i = 0; i < RSSv.size(); i++) {
        List.append(RSSv[i].Title);
    }
    return List;
}
void qorgRSS::getUpdate() {
    if (RSSv.size() != 0) {
        for (uint i = 0; i < RSSv.size(); i++) {
            UpdateQuene++;
            Download* D = new Download(this, &RSSv[i]);
            connect(D, SIGNAL(Downloaded(QString)), this, SLOT(DownloadedS(QString)));
            connect(D, SIGNAL(Downloaded(QString)), this, SLOT(UpdateS()));
            D->start();
        }
    } else {
        emit sendUpdate("RSS: No channels.");
    }
}
uint qorgRSS::threadNumber() {
    QMutexLocker L(&RSSaccessToCounter);
    return RSSthreadCounter;
}
void qorgRSS::setLayoutC() {
    Split->hide();
    Layout->removeWidget(Split);
    List->clear();
    for (uint i = 0; i < RSSv.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(List);
        Itm->setText(0, RSSv[i].Title);
        Itm->setToolTip(0, RSSv[i].Title);
        QItemPushButton* Delete = new QItemPushButton(QIcon(":/main/Delete.png"), this, i);
        connect(Delete, SIGNAL(clicked(uint)), this, SLOT(DeleteS(uint)));
        List->setItemWidget(Itm, 1, Delete);
        List->resizeColumnToContents(0);
        List->resizeColumnToContents(1);
    }
    List->show();
    Layout->addWidget(List, 0, 0, 2, 1);
    QGridLayout* G = new QGridLayout();
    Lab->show();
    G->addWidget(Lab, 0, 0);
    URL->show();
    G->addWidget(URL, 0, 1);
    Add->show();
    G->addWidget(Add, 1, 0, 1, 2);
    Layout->addLayout(G, 1, 1);
}
void qorgRSS::setLayoutF() {
    List->hide();
    Layout->removeWidget(List);
    Lab->hide();
    Layout->removeWidget(Lab);
    URL->hide();
    Layout->removeWidget(URL);
    Add->hide();
    Layout->removeWidget(Add);
    Split->show();
    Layout->addWidget(Split, 0, 0);
}
void qorgRSS::AddS() {
    if (URL->text().isEmpty()) {
        if (URL->styleSheet() != "QLineEdit{background: #FF8888;}") {
            connect(URL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        }
        URL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        disconnect(URL, SIGNAL(textChanged(QString)), this, SLOT(row(QString)));
        for (uint i = 0; i < RSSv.size(); i++) {
            if (RSSv[i].Link == URL->text()) {
                QMessageBox::critical(this, "Error", "Channel already exist.");
                return;
            }
        }
        RSSChannel* Channel = new RSSChannel();
        Channel->Link = URL->text();
        URL->clear();
        QEventLoop eventLoop;
        Download* D = new Download(this, Channel);
        connect(D, SIGNAL(Downloaded(QString)), this, SLOT(DownloadedS(QString)));
        connect(D, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(SSLSocketError(QNetworkReply*, QList<QSslError>)));
        connect(D, SIGNAL(Downloaded(QString)), &eventLoop, SLOT(quit()));
        D->start();
        eventLoop.exec();
        URL->clear();
    }
}
void qorgRSS::DownloadedS(QString Rep) {
    vector  <RSSItem*>  Itm;
    Download* D = qobject_cast<Download*>(QObject::sender());
    RSSChannel* Channel = D->ChannelRSSChannel;
    if (Rep.contains("<rss ")
            && Rep.contains("</rss>")) {
        if (Rep.contains("<channel>")
                && Rep.contains("</channel>")) {
            QList <QString> Parts = Rep.split("<item>");
            Channel->Title = stringBetween("title", Parts[0]);
            for (int i = 1; i < Parts.size()-1; i++) {
                RSSItem* item = new RSSItem();
                item->GUID = stringBetween("guid", Parts[i]);
                QString Title = stringBetween("title", Parts[i]);
                Title.remove("\r\n");
                Title.remove("<br>");
                item->Title = Title;
                item->Link = stringBetween("link", Parts[i]);
                if (item->GUID.isEmpty()) {
                    item->GUID = item->Link;
                }
                QString Des = stringBetween("description", Parts[i]);
                item->Description = Des;
                QString PB = stringBetween("pubDate", Parts[i]);
                PB = PB.remove(0, 5);
                QStringList D = PB.simplified().split(" ");
                if (D.size() == 5) {
                    QList  <QString>  Mon;
                    QDateTime Tmp;
                    Mon << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
                    Tmp.setTimeSpec(Qt::UTC);
                    if (D[2].toInt() < 100) {
                        D[2]="20"+D[2];
                    }
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
                    item->PubDate = Tmp.toLocalTime();
                }
                Itm.push_back(item);
            }
        }
    } else if (Rep.contains("<feed xmlns=\"http://www.w3.org/2005/Atom\"")
               && Rep.contains("</feed>")) {
        Channel->Title = stringBetween("title", Rep);
        Rep = Rep.remove(0, Rep.indexOf("<entry>"));
        while (Rep.contains("</entry>")) {
            RSSItem* item = new RSSItem();
            item->GUID = stringBetween("id", Rep);
            QString Title = stringBetween("title", Rep);
            Title.remove("\r\n");
            Title.remove("<br>");
            item->Title = Title;
            int S = Rep.indexOf("<link", 0, Qt::CaseInsensitive);
            int B = Rep.indexOf("href=\"", S+5, Qt::CaseInsensitive);
            int E = Rep.indexOf("\"", B+6);
            item->Link = Rep.mid(B+6, E-B-6);
            QString Des;
            if (Rep.contains("summary")) {
                Des = stringBetween("summary", Rep);
            } else {
                Des = stringBetween("content", Rep);
            }
            item->Description = Des;
            QString PB = stringBetween("updated", Rep);
            item->PubDate = QDateTime::fromString(PB, Qt::ISODate).toLocalTime();
            Itm.push_back(item);
            Rep.remove(0, Rep.indexOf("</entry>")+8);
        }
    } else {
        if (UpdateQuene == 0) {
            QMessageBox::critical(this, "Error", "Error during reading feed.");
        }
    }
    if (Itm.size() > 0) {
        bool NewChannel = true;
        if (Channel->Itemv.size() > 0) {
            NewChannel = false;
        }
        for (uint i = 0; i < Itm.size(); i++) {
            for (uint j = 0; j < Itm.size(); j++) {
                if (i != j&&Itm[i]->GUID == Itm[j]->GUID) {
                    delete Itm[j];
                    Itm.erase(Itm.begin()+j);
                    j--;
                }
            }
        }
        if (!NewChannel) {
            for (uint i = 0; i < Channel->Itemv.size(); i++) {
                for (uint j = 0; j < Itm.size(); j++) {
                    if (Channel->Itemv[i]->GUID == Itm[j]->GUID) {
                        Itm[j]->New = Channel->Itemv[i]->New;
                        break;
                    }
                }
            }
            Channel->Itemv.swap(Itm);
            if (UpdateQuene == 0) {
                int tmp = currentC;
                currentC = -1;
                setChannel(tmp);
            }
        } else {
            Channel->Itemv.swap(Itm);
            RSSv.push_back((*Channel));
            if (UpdateQuene == 0) {
                currentC = RSSv.size()-1;
            }
            delete Channel;
            sortRSS();
        }
        for (uint i = 0; i < Itm.size(); i++) {
            delete Itm[i];
        }
    }
}
void qorgRSS::DeleteS(uint IID) {
    for (uint j = 0; j < RSSv[IID].Itemv.size(); j++) {
        delete RSSv[IID].Itemv[j];
    }
    RSSv.erase(RSSv.begin()+IID);
    currentC = -1;
    sortRSS();
}
void qorgRSS::row(QString Input) {
    if (Input.isEmpty()) {
        URL->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        URL->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgRSS::chooseChannel(QModelIndex I) {
    setChannel(I.row());
    emit doubleClick(RSSv[I.row()].Title);
}
void qorgRSS::chooseItem(QModelIndex I) {
    if (RSSv[currentC].Itemv[I.row()]->New) {
        RSSv[currentC].Itemv[I.row()]->New = false;
        Titles->selectedItems().first()->setFont(0, QFont());
    }
    View->setHtml(RSSv[currentC].Itemv[I.row()]->Description);
    Link->setText("<a href='"+RSSv[currentC].Itemv[I.row()]->Link+"'>"+RSSv[currentC].Itemv[I.row()]->Link);
}
void qorgRSS::RefreshS() {
    Download* D = new Download(this, &RSSv[currentC]);
    connect(D, SIGNAL(Downloaded(QString)), this, SLOT(DownloadedS(QString)));
    D->start();
}
void qorgRSS::UpdateS() {
    if (UpdateQuene == 1) {
        uint unread = 0;
        for (uint i = 0; i < RSSv.size(); i++) {
            for (uint j = 0; j < RSSv[i].Itemv.size(); j++) {
                if (RSSv[i].Itemv[j]->New) {
                    unread++;
                }
            }
        }
        emit sendUpdate("RSS: "+QString::number(unread)+" unreaded "+(unread == 1 ? "message.": "messages."));
        UpdateQuene--;
    } else {
        UpdateQuene--;
    }
}
void qorgRSS::linkClicked(QUrl link) {
    QDesktopServices::openUrl(link);
}
void qorgRSS::HTTPSS(QNetworkReply* QNR, QList<QSslError> I) {
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
void qorgRSS::SSLSocketError(QNetworkReply* QNR, QList<QSslError> I) {
    int response = Options->checkCertificate(I.first().certificate());
    if (response == 0) {
        Options->addForVerification(I.first().certificate());
        return;
    } else if (response == 1) {
        QNR->ignoreSslErrors(I);
    }
}
void qorgRSS::sortRSS() {
    if (RSSv.size() > 1) {
        while (true) {
            bool OKL = true;
            for (int i = 0; i < static_cast<int>(RSSv.size()-1); i++) {
                if (RSSv[i].Title > RSSv[i+1].Title) {
                    if (i == currentC) {
                        currentC++;
                    } else if (i+1 == currentC) {
                        currentC--;
                    }
                    std::swap(RSSv[i], RSSv[i+1]);
                    OKL = false;
                }
            }
            if (OKL) {
                break;
            }
        }
    }
    int tmp = currentC;
    currentC = -2;
    setChannel(tmp);
    emit updateTree();
}
#include "qorgrss.moc"
