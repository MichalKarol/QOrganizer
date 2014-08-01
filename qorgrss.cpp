#include "qorgrss.h"
#include "qorgtools.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
RSSItem::RSSItem()
{
    Title.clear();
    PubDate.setMSecsSinceEpoch(0);
    Link.clear();
    Description.clear();
    GUID.clear();
    New=true;
}
RSSItem::~RSSItem()
{
    Title.clear();
    PubDate.setMSecsSinceEpoch(0);
    Link.clear();
    Description.clear();
    GUID.clear();
    New=true;
}
RSSChannel::RSSChannel()
{
    Title.clear();
    Link.clear();
    Itemv.clear();
}
RSSChannel::~RSSChannel()
{
    Title.clear();
    Link.clear();
    Itemv.clear();
}

class Download :public QThread
{
    Q_OBJECT
public:
    Download(RSSChannel*);
    RSSChannel *Ch;
    void run();
private:
    QString URL;
    void Downloading();
    QByteArray SubDownload(QString);
signals:
    void Downloaded(QString);
};
Download::Download(RSSChannel *C)
{
    URL=C->Link;
    Ch=C;
    this->start();
    connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}
void Download::run()
{
    Downloading();
}
void Download::Downloading()
{
    URL.remove("http://");
    QString server=URL.mid(0,URL.indexOf("/"));
    QTcpSocket *S=new QTcpSocket();
    S->connectToHost(server,80);
    QString Output;
    if(S->waitForConnected())
    {
        QString Req=URL.mid(URL.indexOf("/"),URL.length()-URL.indexOf("/"));
        if(Req==server)
        {
            Req="/";
        }
        S->write(QString("GET "+Req+" HTTP/1.1\nUser-Agent: QOrganizer\nHost: "+server+"\nConnection:close\n\n").toUtf8());
        if(S->waitForReadyRead())
        {
            QByteArray Reply=S->readAll();
            while(S->state()==QTcpSocket::ConnectedState)
            {
                if(S->waitForReadyRead())
                {
                    Reply.append(S->readAll());
                }
            }
            if(Reply.contains("HTTP/1.1 301"))
            {
                QString NUrl=Reply.mid(Reply.indexOf("The document has moved ")+32,Reply.indexOf("\">here")-Reply.indexOf("The document has moved ")-32);
                Reply=SubDownload(NUrl);
            }
            else if(Reply.contains("HTTP/1.1 302"))
            {
                QString NUrl=Reply.mid(Reply.indexOf("Location: ")+10,Reply.indexOf("\r\n",Reply.indexOf("Location: "))-Reply.indexOf("Location: ")-10);
                Reply=SubDownload(server+NUrl);
            }
            if(Reply.contains("encoding=\""))
            {
                QString encoding=Reply.mid(Reply.indexOf("encoding=\"")+10,Reply.indexOf("\"?>")-Reply.indexOf("encoding=\"")-10);
                if(encoding.toUpper()!="UTF-8")
                {
                    QTextCodec *C=QTextCodec::codecForName(encoding.toUtf8());
                    Output=C->toUnicode(Reply);
                }
                else
                {
                    Output=QString(Reply);
                }
            }
            else
            {
                Output=QString(Reply);
            }
        }
    }
    S->close();
    delete S;
    emit Downloaded(Output);
}
QByteArray Download::SubDownload(QString I)
{
    I.remove("http://");
    QString server=I.mid(0,I.indexOf("/"));
    QTcpSocket *S=new QTcpSocket();
    S->connectToHost(server,80);
    QByteArray Reply;
    if(S->waitForConnected())
    {
        QString Req=I.mid(I.indexOf("/"),I.length()-I.indexOf("/"));
        if(Req==server)
        {
            Req="/";
        }
        S->write(QString("GET "+Req+" HTTP/1.1\nUser-Agent: QOrganizer\nHost: "+server+"\nConnection:close\n\n").toUtf8());
        if(S->waitForReadyRead())
        {
            Reply=S->readAll();
            while(S->state()==QTcpSocket::ConnectedState)
            {
                if(S->waitForReadyRead())
                {
                    Reply.append(S->readAll());
                }
            }
            if(Reply.contains("HTTP/1.1 301"))
            {
                QString NUrl=Reply.mid(Reply.indexOf("The document has moved ")+32,Reply.indexOf("\">here")-Reply.indexOf("The document has moved ")-32);
                Reply=SubDownload(NUrl);
            }
            else if(Reply.contains("HTTP/1.1 302"))
            {
                QString NUrl=Reply.mid(Reply.indexOf("Location: ")+10,Reply.indexOf("\n",Reply.indexOf("Location: "))-Reply.indexOf("Location: ")-10);
                Reply=SubDownload(NUrl);
            }
        }
    }
    delete S;
    return Reply;
}

QString stringBetween(QString Tag,QString Text)
{
    Text=Text.mid(Text.indexOf(Tag,0,Qt::CaseInsensitive)+Tag.length(),Text.indexOf(Tag,Text.indexOf(Tag,0,Qt::CaseInsensitive)+1,Qt::CaseInsensitive)-Text.indexOf(Tag,0,Qt::CaseInsensitive)-Tag.length());
    Text.remove("<![CDATA[");
    Text.remove("]]>");
    if(Text.contains(">"))
    {
        Text=Text.mid(Text.indexOf(">")+1,Text.length()-Text.indexOf(">")-1);
    }
    if(Text.contains("<"))
    {
        Text=Text.mid(0,Text.lastIndexOf("<"));
    }
    return Text;
}

qorgRSS::qorgRSS(QWidget *parent) :QWidget(parent)
{
    Layout=new QGridLayout(this);
    List=new QTreeWidget(this);
    List->header()->hide();
    List->setColumnCount(2);
    List->header()->setStretchLastSection(false);
    List->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    List->setColumnWidth(1,50);
    connect(List,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chooseChannel(QModelIndex)));
    Lab=new QLabel("URL: ",this);
    URL=new QLineEdit(this);
    URL->setMinimumWidth(100);
    Add=new QPushButton(QIcon(":/main/Add.png"),"",this);
    Add->setShortcut(Qt::Key_Return);
    connect(Add,SIGNAL(clicked()),this,SLOT(AddS()));
    Split=new QSplitter(Qt::Vertical,this);
    Titles=new QTreeWidget(this);
    Titles->setColumnCount(2);
    Titles->header()->hide();
    Titles->header()->setStretchLastSection(false);
    Titles->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Titles->setColumnWidth(1,120);
    connect(Titles,SIGNAL(clicked(QModelIndex)),this,SLOT(chooseItem(QModelIndex)));
    View=new QWebView(this);
    Link=new QLabel(this);
    Link->setTextFormat(Qt::RichText);
    UpdateQuene=0;
    Refresh=new QPushButton(QIcon(":/main/Refresh.png"),"",this);
    Refresh->setMaximumWidth(30);
    connect(Refresh,SIGNAL(clicked()),this,SLOT(RefreshS()));
    QWidget *W=new QWidget(this);
    QVBoxLayout *V=new QVBoxLayout(W);
    V->addWidget(View);
    QHBoxLayout *H=new QHBoxLayout();
    H->addWidget(Link);
    H->addWidget(Refresh);
    V->addLayout(H);
    Split->addWidget(Titles);
    Split->addWidget(W);
    setLayoutC();
}
qorgRSS::~qorgRSS()
{
    for(uint i=0;i<RSSv.size();i++)
    {
        for(uint j=0;j<RSSv[i].Itemv.size();j++)
        {
            delete RSSv[i].Itemv[j];
        }
    }
}
QString qorgRSS::output()
{
    QString out;
    for(uint i=0;i<RSSv.size();i++)
    {
        QString work;
        work.append(OutputTools(RSSv[i].Title,"TITLE"));
        work.append(OutputTools(RSSv[i].Link,"LINK"));
        QString Items;
        for(uint j=0;j<RSSv[i].Itemv.size();j++)
        {
            QString work2;
            work2.append(OutputTools(RSSv[i].Itemv[j]->Title,"ITEM_TITLE"));
            work2.append(OutputTools(RSSv[i].Itemv[j]->PubDate.toString(Qt::ISODate),"ITEM_PUBDATE"));
            work2.append(OutputTools(RSSv[i].Itemv[j]->Link,"ITEM_LINK"));
            work2.append(OutputTools(RSSv[i].Itemv[j]->Description,"ITEM_DESCRIPTION"));
            work2.append(OutputTools(RSSv[i].Itemv[j]->GUID,"ITEM_GUID"));
            work2.append(OutputTools(RSSv[i].Itemv[j]->New,"ITEM_NEW"));
            Items.append(OutputToolsS(work2,"ITEM"));
        }
        work.append(OutputToolsS(Items,"ITEMV"));
        out.append(OutputToolsS(work,"CHANNEL"));
    }
    out=OutputToolsS(out,"CHANNELV");
    return out;
}
void qorgRSS::input(QString IN)
{
    while(IN.contains("<CHANNEL>"))
    {
        QString CS=InputSS(IN,"CHANNEL");
        RSSChannel C;
        C.Title=InputS(CS,"TITLE");
        C.Link=InputS(CS,"LINK");
        QString Items=InputSS(CS,"ITEMV");
        while(Items.contains("<ITEM>"))
        {
            QString IS=InputSS(Items,"ITEM");
            RSSItem *I=new RSSItem();
            I->Title=InputS(IS,"ITEM_TITLE");
            I->PubDate=QDateTime::fromString(InputS(IS,"ITEM_PUBDATE"),Qt::ISODate);
            I->Link=InputS(IS,"ITEM_LINK");
            I->Description=InputS(IS,"ITEM_DESCRIPTION");
            I->GUID=InputS(IS,"ITEM_GUID");
            I->New=InputB(IS,"ITEM_NEW");
            C.Itemv.push_back(I);
            Items.remove(Items.indexOf("<ITEM>"),Items.indexOf("</ITEM>")-Items.indexOf("<ITEM>")+7);
        }
        RSSv.push_back(C);
        IN.remove(IN.indexOf("<CHANNEL>"),IN.indexOf("</CHANNEL>")-IN.indexOf("<CHANNEL>")+10);
    }
    setLayoutC();
}
void qorgRSS::setChannel(QString I)
{
    if(I!=currentChannel)
    {
        if(I=="")
        {
            setLayoutC();
            currentChannel=I;
        }
        else
        {
            Titles->clear();
            View->setHtml("");
            Link->clear();
            for(uint i=0;i<RSSv.size();i++)
            {
                if(I==RSSv[i].Title)
                {
                    currentC=i;
                    for(uint j=0;j<RSSv[i].Itemv.size();j++)
                    {
                        QTreeWidgetItem *Itm=new QTreeWidgetItem(Titles);
                        if(RSSv[i].Itemv[j]->New)
                        {
                            Itm->setFont(0,QFont("",Itm->font(0).pixelSize(),QFont::Bold));
                        }
                        Itm->setText(0,RSSv[i].Itemv[j]->Title);
                        Itm->setToolTip(0,RSSv[i].Itemv[j]->Title);
                        Itm->setText(1,RSSv[i].Itemv[j]->PubDate.toString("dd/MM/yyyy hh:mm"));
                    }
                    break;
                }
            }
            setLayoutF();
            currentChannel=I;
        }
    }
}
QList <QString> qorgRSS::getChannels()
{
    QList <QString> List;
    for(uint i=0;i<RSSv.size();i++)
    {
        List.append(RSSv[i].Title);
    }
    return List;
}
void qorgRSS::getUpdate()
{
    if(RSSv.size()!=0)
    {
        for(uint i=0;i<RSSv.size();i++)
        {
            Download *D=new Download(&RSSv[i]);
            connect(D,SIGNAL(Downloaded(QString)),this,SLOT(DownloadedS(QString)));
            connect(D,SIGNAL(Downloaded(QString)),this,SLOT(UpdateS()));
            UpdateQuene++;
        }
    }
    else
    {
        emit sendUpdate("RSS: No channels.");
    }
}
void qorgRSS::setLayoutC()
{
    Split->hide();
    Layout->removeWidget(Split);
    List->clear();
    for(uint i=0;i<RSSv.size();i++)
    {
        QTreeWidgetItem *Itm=new QTreeWidgetItem(List);
        Itm->setText(0,RSSv[i].Title);
        Itm->setToolTip(0,RSSv[i].Title);
        QItemPushButton *Delete=new QItemPushButton(QIcon(":/main/Delete.png"),this,i);
        connect(Delete,SIGNAL(clicked(uint)),this,SLOT(DeleteS(uint)));
        List->setItemWidget(Itm,1,Delete);
        List->resizeColumnToContents(0);
        List->resizeColumnToContents(1);
    }
    List->show();
    Layout->addWidget(List,0,0,2,1);
    QGridLayout *G=new QGridLayout();
    Lab->show();
    G->addWidget(Lab,0,0);
    URL->show();
    G->addWidget(URL,0,1);
    Add->show();
    G->addWidget(Add,1,0,1,2);
    Layout->addLayout(G,1,1);
}
void qorgRSS::setLayoutF()
{
    List->hide();
    Layout->removeWidget(List);
    Lab->hide();
    Layout->removeWidget(Lab);
    URL->hide();
    Layout->removeWidget(URL);
    Add->hide();
    Layout->removeWidget(Add);
    Split->show();
    Layout->addWidget(Split,0,0);
}
void qorgRSS::AddS()
{
    if(URL->text().isEmpty())
    {
        if(URL->styleSheet()!="QLineEdit{background: #FF8888;}")
        {
            connect(URL,SIGNAL(textChanged(QString)),this,SLOT(row(QString)));
        }
        URL->setStyleSheet("QLineEdit{background: #FF8888;}");
    }
    else
    {
        disconnect(URL,SIGNAL(textChanged(QString)),this,SLOT(row(QString)));
        for(uint i=0;i<RSSv.size();i++)
        {
            if(RSSv[i].Link==URL->text())
            {
                QMessageBox::critical(this,"Error","Channel already exist.");
                return;
            }
        }
        RSSChannel *Channel=new RSSChannel();
        Channel->Link=URL->text();
        URL->clear();
        QEventLoop *eventLoop=new QEventLoop(this);
        Download *D=new Download(Channel);
        connect(D,SIGNAL(Downloaded(QString)),this,SLOT(DownloadedS(QString)));
        connect(D,SIGNAL(finished()),eventLoop,SLOT(quit()));
        eventLoop->exec();
        URL->clear();
    }
}
void qorgRSS::DownloadedS(QString Rep)
{
    vector <RSSItem*> Itm;
    Download *D=qobject_cast<Download*>(QObject::sender());
    RSSChannel *Channel=D->Ch;
    if(Rep.contains("<rss ")&&Rep.contains("</rss>"))
    {
        if(Rep.contains("<channel>")&&Rep.contains("</channel>"))
        {
            Channel->Title=stringBetween("title",Rep);
            Rep=Rep.remove(0,Rep.indexOf("<item>"));
            while(Rep.contains("</item>"))
            {
                RSSItem *item=new RSSItem();
                item->GUID=stringBetween("guid",Rep);
                QString Title=stringBetween("title",Rep);
                Title.remove("\r\n");
                Title.remove("<![CDATA[");
                Title.remove("]]>");
                Title.replace("&lt;","<");
                Title.replace("&gt;",">");
                Title.replace("&amp;","&");
                Title.replace("&quot;","\"");
                Title.remove("<br>");
                item->Title=Title;
                item->Link=stringBetween("link",Rep);
                if(item->GUID.isEmpty())
                {
                    item->GUID=item->Link;
                }
                QString Des=stringBetween("description",Rep);
                Des.replace("&lt;","<");
                Des.replace("&gt;",">");
                Des.replace("&amp;","&");
                Des.remove("<![CDATA[");
                Des.remove("]]>");
                item->Description=Des;
                QString PB=stringBetween("pubDate",Rep);
                PB=PB.remove(0,5);
                QStringList D=PB.simplified().split(" ");
                QList <QString> Mon;
                QDateTime Tmp;
                Mon<<"Jan"<<"Feb"<<"Mar"<<"Apr"<<"May"<<"Jun"<<"Jul"<<"Aug"<<"Sep"<<"Oct"<<"Nov"<<"Dec";
                Tmp.setTimeSpec(Qt::UTC);
                if(D[2].toInt()<100)
                {
                    D[2]="20"+D[2];
                }
                Tmp.setDate(QDate(D[2].toInt(),Mon.indexOf(D[1])+1,D[0].toInt()));
                QStringList H=D[3].split(":");
                Tmp.setTime(QTime(H[0].toInt(),H[1].toInt(),H[2].toInt()));
                if(D[4]!="GMT")
                {
                    short H=D[4].mid(1,2).toShort();
                    short M=D[4].mid(3,2).toShort();
                    int Sec=H*3600+M*60;
                    if(D[4][0]=='+')
                    {
                        Tmp=Tmp.addSecs(-Sec);
                    }
                    else
                    {
                        Tmp=Tmp.addSecs(Sec);
                    }
                }
                item->PubDate=Tmp.toLocalTime();
                Itm.push_back(item);
                Rep.remove(0,Rep.indexOf("</item>")+7);
            }
        }
    }
    else if(Rep.contains("<feed xmlns=\"http://www.w3.org/2005/Atom\"")&&Rep.contains("</feed>"))
    {
        Channel->Title=stringBetween("title",Rep);
        Rep=Rep.remove(0,Rep.indexOf("<entry>"));
        while(Rep.contains("</entry>"))
        {
            RSSItem *item=new RSSItem();
            item->GUID=stringBetween("id",Rep);
            QString Title=stringBetween("title",Rep);
            Title.remove("\r\n");
            Title.replace("&lt;","<");
            Title.replace("&gt;",">");
            Title.replace("&amp;","&");
            Title.replace("&quot;","\"");
            Title.remove("<br>");
            item->Title=Title;
            QString Link=Rep.mid(Rep.indexOf("<link",0,Qt::CaseInsensitive),Rep.indexOf("/>",Rep.indexOf("<link",0,Qt::CaseInsensitive),Qt::CaseInsensitive)-Rep.indexOf("<link",0,Qt::CaseInsensitive));
            Link=Link.mid(Link.indexOf("href=\"")+6,Link.indexOf("\"",Link.indexOf("href=\"")+6)-6);
            item->Link=Link;
            QString Des=stringBetween("summary",Rep);
            Des.replace("&lt;","<");
            Des.replace("&gt;",">");
            Des.replace("&amp;","&");
            item->Description=Des;
            QString PB=stringBetween("updated",Rep);
            item->PubDate=QDateTime::fromString(PB,Qt::ISODate).toLocalTime();
            Itm.push_back(item);
            Rep.remove(0,Rep.indexOf("</entry>")+8);
        }
    }
    else
    {
        QMessageBox::critical(this,"Error","Error during reading feed.");
    }
    if(Itm.size()>0)
    {
        for(uint i=0;i<Channel->Itemv.size();i++)
        {
            for(uint j=0;j<Itm.size();j++)
            {
                if(Channel->Itemv[i]->GUID==Itm[j]->GUID)
                {
                    Itm[j]->New=Channel->Itemv[i]->New;
                    break;
                }
            }
        }
        for(uint i=0;i<Itm.size();i++)
        {
            for(uint j=0;j<Itm.size();j++)
            {
                if(i!=j&&Itm[i]->GUID==Itm[j]->GUID)
                {
                    delete Itm[j];
                    Itm.erase(Itm.begin()+j);
                    j--;
                }
            }
        }
        Channel->Itemv.swap(Itm);
        for(uint i=0;i<Itm.size();i++)
        {
            delete Itm[i];
        }
        if(UpdateQuene==0)
        {
            setChannel("");
            setChannel(Channel->Title);
        }
    }
}
void qorgRSS::DeleteS(uint IID)
{
    for(uint j=0;j<RSSv[IID].Itemv.size();j++)
    {
        delete RSSv[IID].Itemv[j];
    }
    RSSv.erase(RSSv.begin()+IID);
    setLayoutC();
    emit updateTree();
}
void qorgRSS::row(QString IN)
{
    if(IN.isEmpty())
    {
        URL->setStyleSheet("QLineEdit{background: #FF8888;}");
    }
    else
    {
        URL->setStyleSheet("QQLineEdit{background: white;}");
    }
}
void qorgRSS::chooseChannel(QModelIndex I)
{
    setChannel(RSSv[I.row()].Title);
    emit doubleClick(RSSv[I.row()].Title);
}
void qorgRSS::chooseItem(QModelIndex I)
{
    if(RSSv[currentC].Itemv[I.row()]->New)
    {
        RSSv[currentC].Itemv[I.row()]->New=false;
        Titles->selectedItems().first()->setFont(0,QFont());
    }
    View->setHtml(RSSv[currentC].Itemv[I.row()]->Description);
    Link->setText("<a href='"+RSSv[currentC].Itemv[I.row()]->Link+"'>"+RSSv[currentC].Itemv[I.row()]->Link);
}
void qorgRSS::RefreshS()
{
    Download *D=new Download(&RSSv[currentC]);
    connect(D,SIGNAL(Downloaded(QString)),this,SLOT(DownloadedS(QString)));
}
void qorgRSS::UpdateS()
{
    if(UpdateQuene==1)
    {
        uint unread=0;
        for(uint i=0;i<RSSv.size();i++)
        {
            for(uint j=0;j<RSSv[i].Itemv.size();j++)
            {
                if(RSSv[i].Itemv[j]->New)
                {
                    unread++;
                }
            }
        }
        if(unread==1)
        {
            emit sendUpdate("RSS: 1 unreaded message.");
        }
        else
        {
            emit sendUpdate("RSS: "+QString::number(unread)+" unreaded messages.");
        }
        UpdateQuene--;
    }
    else
    {
        UpdateQuene--;
    }
}
#include "qorgrss.moc"
