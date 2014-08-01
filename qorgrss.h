#ifndef QORGRSS_H
#define QORGRSS_H

#include  <QtWidgets>
#include  <QNetworkReply>
#include  <QWebView>
using namespace std;

class RSSItem
{
public:
    RSSItem();
    ~RSSItem();
    QString Title;
    QDateTime PubDate;
    QString Link;
    QString Description;
    QString GUID;
    bool New;
};
class RSSChannel
{
public:
    RSSChannel();
    ~RSSChannel();
    QString Title;
    QString Link;
    vector  < RSSItem* >  Itemv;
};

class qorgRSS : public QWidget
{
    Q_OBJECT
public:
    qorgRSS(QWidget*);
    ~qorgRSS();
    QString output();
    void input(QString);
    void setChannel(QString);
    QList  < QString >  getChannels();
    QString getCurrent()
    {
        return currentChannel;
    }
    void getUpdate();
private:
    QString currentChannel;
    int currentC;
    vector  < RSSChannel >  RSSv;
    QGridLayout *Layout;
    QTreeWidget *List;
    QLabel *Lab;
    QLineEdit *URL;
    QPushButton *Add;

    QSplitter *Split;
    QTreeWidget *Titles;
    QWebView *View;
    QLabel *Link;
    QPushButton *Refresh;
    uint UpdateQuene;
    void setLayoutC();
    void setLayoutF();
private slots:
    void AddS();
    void DownloadedS(QString);
    void DeleteS(uint);
    void row(QString);
    void chooseChannel(QModelIndex);
    void chooseItem(QModelIndex);
    void RefreshS();
    void UpdateS();
signals:
    void updateTree();
    void doubleClick(QString);
    void sendUpdate(QString);
};

#endif // QORGRSS_H
