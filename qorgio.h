#ifndef QORGIO_H
#define QORGIO_H
#include <QtWidgets>
#include "qorganizer.h"
namespace qorgIO
{
   bool ReadFile(QString*,QString*,QOrganizer*,QString);
   void SaveFile(QString*,QString*,QOrganizer*,QString);
}
#endif // QORGIO_H
