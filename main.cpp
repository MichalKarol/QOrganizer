#include "qorganizer.h"
#include "qorglogin.h"
#include <QApplication>
#include <QtWidgets>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QOrganizer *w=new QOrganizer();
    w->setAttribute(Qt::WA_DeleteOnClose);
   if((new qorgLogin(w))->exec()==QDialog::Accepted)
    {
       w->show();
     return a.exec();
    }
    else
    {
        return 0;
    }
}
