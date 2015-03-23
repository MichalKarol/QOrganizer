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

#ifndef QORGAB_H_
#define QORGAB_H_
#include <qorgtools.h>
#include <QtWidgets>
#include <vector>

using std::vector;
struct Person {
    QString Name;
    QString Surname;
    QString Category;
    QString Town;
    QString Street;
    uint HouseNumber;
    uint Apartment;
    QString Email;
    QString Mobile;
    QString Birthday;
    QByteArray Photo;
    QString ExtraInformation;
};
class qorgAB : public QWidget {
    Q_OBJECT
public:
    explicit qorgAB(QWidget*);
    void setCategory(QString I) {
        if (I != currentCategory) {
         currentCategory = I;
         setList();
        }
    }
    QString getCurrent()    {
     return currentCategory;
    }
    QStringList getCategories();
    QString output();
    void input(QString);
    QString exportToVCard();
    QList  <QString>  getBirthdays(QDate);
    QList  <QString>  getEmails();
private:
    vector  <Person>  Personv;
    uint lastIID;
    QString currentCategory;
    QGridLayout* La;
    QListWidget* List;
    QPushButton* Photo;
    QLabel* L[9];
    QLineEdit* E[9];
    QCheckBox* BDayCheckBox;
    QDateEdit* D;
    QCalendarWidget* CalendarPopup;
    QTextBrowser* ExtraInformationField;
    QPushButton* Add;
    QPushButton* OKB;
    QPushButton* Cancel;
    QCompleter* C;
    // TODO(mkarol) Import from vCard
private slots:
    void SelectPhoto();
    void ActivateBirthdayField();
    void AddS();
    void row(QString);
    void Click(QModelIndex);
    void Edit(uint);
    void Delete(uint);
    void OK();
    void Can();
    void UpdateList();
    void setList();
signals:
    void updateTree();
};

#endif  // QORGAB_H_
