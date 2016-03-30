//    Copyright (C) 2015 Michał Karol <michal.p.karol@gmail.com>

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

#ifndef QORGFEEDSREADERMODEL_H
#define QORGFEEDSREADERMODEL_H

#include <QObject>
#include <QDateTime>
#include <QUrl>
using std::vector;

class Channel {
  private:
    class Item {
      private:
        QString title = QString();
        QDateTime publicationDateTime = QDateTime::currentDateTime();
        QUrl link = QUrl();
        QString descryption = QString();
        QString GUID = QString();
        bool readed = false;

        bool operator < (const Item* item);
        friend QDebug operator << (QDebug d, const Item* item);
    };
    QString title = QString();
    QUrl link = QUrl();
    vector <Item*> items;

    bool operator < (const Channel* channel);
    friend QDebug operator << (QDebug d, const Channel* channel);
};


class qorgFeedsReaderModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgFeedsReaderModel(QObject* parent = 0);
  signals:

  public slots:
};

#endif // QORGFEEDSREADERMODEL_H
