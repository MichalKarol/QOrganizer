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

#ifndef QORGRSS_H_
#define QORGRSS_H_
#include <qorgtools.h>
#include <qorgoptions.h>
#include <QtWidgets>
#include <QNetworkReply>
#include <QWebView>
#include <vector>

using std::vector;
class RSSItem {
public:
    RSSItem();
    QString Title;
    QDateTime PubDate;
    QString Link;
    QString Description;
    QString GUID;
    bool New;
};
class RSSChannel {
public:
    RSSChannel();
    QString Title;
    QString Link;
    vector <RSSItem*> Itemv;
};

class qorgRSS : public QWidget {
    Q_OBJECT
public:
    explicit qorgRSS(QWidget*, qorgOptions*);
    ~qorgRSS();
    QString output();
    void input(QString);
    void setChannel(int);
    QStringList getChannels();
    int getCurrent() {
        return currentC;
    }
    uint threadNumber();
    void getUpdate();
    bool SSLSocketError(QList<QSslError>);
private:
    qorgOptions* Options;
    int currentC;
    vector <RSSChannel> RSSv;
    QGridLayout* Layout;
    QTreeWidget* List;
    QLabel* Lab;
    QLineEdit* URL;
    QPushButton* Add;

    QSplitter* Split;
    QTreeWidget* Titles;
    QWebView* View;
    QLabel* Link;
    QPushButton* Refresh;
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
    void linkClicked(QUrl);
    void HTTPSS(QNetworkReply*, QList <QSslError>);
    void sortRSS();
signals:
    void updateTree();
    void doubleClick(QString);
    void sendUpdate(QString);
};

#endif  // QORGRSS_H_
