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

#ifndef QORGNOTESMODEL_H
#define QORGNOTESMODEL_H

#include <qorgtools.h>
#include <QObject>
using std::vector;

class Note {
  public:
    // IO functions
    bool input(QString line, uint version);
    QString output();
    QString exportToVNote();

  private:
    // Data fields
    QString subject = QString();
    uchar priority = 0;
    QString body = QString();

    // QDebug + operators
    bool operator <(const Note* note);
    bool operator ==(const Note* note);
    friend QDebug operator << (QDebug d, const Note* note);
};

class qorgNotesModel : public QObject {
    Q_OBJECT
  public:
    explicit qorgNotesModel(QObject* parent = 0);
    ~qorgNotesModel();

    // IO functions
    void input(QString input, uint version);
    QString output();
    QString exportToVNote();

    // Functions to change model by controller
    void addNote(Note* note);
    void deleteNote(uint uid);
    void clear();

    // Functions to send data to view
    QList <QTreeWidgetItem*> getList();

  private:
    vector <Note*> data;




  signals:
    void modelChanged();
};

// TODO(mkarol) Import from vNote


#endif // QORGNOTESMODEL_H
