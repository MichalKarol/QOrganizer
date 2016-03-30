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

#include "qorgnotesmodel.h"

// IO functions
bool Note::input(QString line, uint version) {
    bool output = false;
    QStringList list = line.split(" ");
    int control = (version < 105 ? list.size() - 1 : list.size());

    if (control > 0 && version < 105) {
        if (control == 3) {
            subject = inputString(list[0]);
            priority = inputUchar(list[1]);
            body = inputString(list[2]);
            output = true;
        }
    }

    return output;
}
QString Note::output() {
    QString output;
    output.append(outputFilter(subject) + " ");
    output.append(outputFilter(priority) + " ");
    output.append(outputFilter(body) + "\n");
    return output;
}
QString Note::exportToVNote() {
    QString vnote;
    vnote.append("BEGIN:VNOTE\nVERSION:1.1\n");
    vnote.append("BODY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:"
                 + quotedPrintableEncode(body.toUtf8())
                 + "\n");
    vnote.append("CLASS:PUBLIC\nEND:VNOTE\n");
    return vnote;
}

qorgNotesModel::qorgNotesModel(QObject* parent) : QObject(parent) {

}
qorgNotesModel::~qorgNotesModel() {
    for (uint i = 0; i < data.size(); i++) {
        delete data[i];
        data[i] = NULL;
    }

    data.clear();
}

// IO functions
void qorgNotesModel::input(QString input, uint version) {
    if (!input.isEmpty()) {
        QStringList lines = input.split("\n");

        for (int i = 0; i < lines.size(); i++) {
            Note* note = new Note;

            if (note->input(lines[i], version)) {
                data.push_back(note);
            } else {
                delete note;
                note = NULL;
            }
        }
    }
}
QString qorgNotesModel::output() {
    QString output;

    for (uint i = 0; i < data.size(); i++) {
        output.append(data[i]->output());
    }

    output.append("\n\n");
    return output;
}
QString qorgNotesModel::exportToVNote() {
    QString output;

    for (uint i = 0; i < data.size(); i++) {
        output.append(data[i]->exportToVNote());
    }

    return output;
}

// Functions to change model by controller
void qorgNotesModel::addNote(Note* note) {

}
void qorgNotesModel::deleteNote(uint uid) {

}
void qorgNotesModel::clear() {
    for (uint i = 0; i < data.size(); i++) {
        delete data[i];
        data[i] = NULL;
    }

    data.clear();
    emit modelChanged();
}

// Functions to send data to view
QList <QTreeWidgetItem*> qorgNotesModel::getList() {
    QList<QTreeWidgetItem*> output;

    return output;
}
