#ifndef QORGNOTES_H
#define QORGNOTES_H
#include <QtWidgets>
using namespace std;
struct Note
{
    QString topic;
    short int priority;
    QString text;
};

class qorgNotes : public QWidget
{
    Q_OBJECT
public:
    qorgNotes(QWidget*);
    QString output();
    void input(QString);
    vector <Note> Notes;
private:
    void updateList();
    QGridLayout *Layout;
    QLabel* TLabel;
    QLineEdit *Topic;
    QSlider *Priority;
    QTextEdit *Text;
    QPushButton *OK;
    QPushButton *Cancel;
    QTreeWidget *List;
    QPushButton *AddB;
    int edited;
private slots:
    void Add();
    void Edit(uint);
    void Delete(uint);
    void EOK();
    void EC();
    void EClicked(QModelIndex);
    void ChangeT();
    void ChangeX();
};

#endif // QORGNOTES_H
