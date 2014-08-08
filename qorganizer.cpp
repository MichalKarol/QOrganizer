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

#include <qorganizer.h>

QOrganizer::QOrganizer() {
    setWindowIcon(QIcon(":/main/QOrganizer.png"));
    setWindowTitle("QOrganizer");
    this->setMinimumWidth(1024);
    AdressBook = new qorgAB(this);
    connect(AdressBook, SIGNAL(updateTree()), this, SLOT(updateAdressBook()));
    Calendar = new qorgCalendar(this, AdressBook);
    connect(Calendar, SIGNAL(updateTree()), this, SLOT(updateCalendar()));
    connect(Calendar, SIGNAL(Notification(QString)), this, SLOT(Notification(QString)));
    Mail = new qorgMail(this, AdressBook);
    connect(Mail, SIGNAL(updateTree()), this, SLOT(updateMail()));
    connect(Mail, SIGNAL(doubleClick(QString)), this, SLOT(doubleClick(QString)));
    connect(Mail, SIGNAL(sendUpdate(QString)), this, SLOT(MailNews(QString)));
    Notes = new qorgNotes(this);
    RSS = new qorgRSS(this);
    connect(RSS, SIGNAL(updateTree()), this, SLOT(updateRSS()));
    connect(RSS, SIGNAL(doubleClick(QString)), this, SLOT(doubleClick(QString)));
    connect(RSS, SIGNAL(sendUpdate(QString)), this, SLOT(RSSNews(QString)));
    PasswordManager = new qorgPasswd(this);
    QGridLayout *layout = new QGridLayout(this);
    TreeWidget = new QTreeWidget();
    connect(TreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(launchFunction(QTreeWidgetItem*)));
    layout->addWidget(TreeWidget, 0, 0, Qt::AlignLeft);
    Stacked = new QStackedWidget();
    // Home widget
    QTextBrowser *Label = new QTextBrowser(this);
    Label->setText("QOrganizer 1.01\nCreated by: Mkarol (mkarol@linux.pl)\n08.08.2014\nYou could help in developing this software by donating:"
                   "\nBitcoins: 17wTU13S31LMdVuVmxxXyBwnj7kJwm74wK\nLitecoins: LbDEkiQQJ8XzGoqf4oJE3UfJmW5qzPsK3i");
    // Timers and tray
    UInterval = 30;
    UTimer = new QTimer(this);
    connect(UTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
    BInterval = 30;
    BTimer = new QTimer(this);
    connect(BTimer, SIGNAL(timeout()), this, SLOT(Block()));
    Tray = new QSystemTrayIcon(QIcon(":/main/QOrganizer.png"), this);
    Tray->show();
    connect(Tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayClick(QSystemTrayIcon::ActivationReason)));
    closing = false;
    // Options widget
    QLabel *L[6];
    CP = new QLineEdit(this);
    CP->setEchoMode(QLineEdit::Password);
    L[0]=new QLabel("Current password: ", this);
    NP = new QLineEdit(this);
    NP->setEchoMode(QLineEdit::Password);
    connect(NP, SIGNAL(textChanged(QString)), this, SLOT(Validator(QString)));
    L[1]=new QLabel("New password: ", this);
    Change = new QPushButton("Change password.", this);
    connect(Change, SIGNAL(clicked()), this, SLOT(NewPassword()));
    L[2]=new QLabel("Updates interval: ", this);
    UpdateInterval = new QSpinBox(this);
    L[3]=new QLabel(" minutes", this);
    L[4]=new QLabel("Time to lock: ", this);
    BlockInterval = new QSpinBox(this);
    L[5]=new QLabel(" minutes", this);
    ChangeInterval = new QPushButton("Change", this);
    connect(ChangeInterval, SIGNAL(clicked()), this, SLOT(SetInterval()));
    shown = false;
    QWidget *W = new QWidget(this);
    QGridLayout *G = new QGridLayout(W);
    QHBoxLayout *La = new QHBoxLayout();
    La->addWidget(L[0]);
    La->addWidget(CP);
    QHBoxLayout *Lb = new QHBoxLayout();
    Lb->addWidget(L[1]);
    Lb->addWidget(NP);
    G->addLayout(La, 0, 0);
    G->addLayout(Lb, 1, 0);
    G->addWidget(Change, 2, 0);
    QHBoxLayout *Lc = new QHBoxLayout();
    Lc->addWidget(L[2]);
    Lc->addWidget(UpdateInterval);
    Lc->addWidget(L[3]);
    QHBoxLayout *Ld = new QHBoxLayout();
    Ld->addWidget(L[4]);
    Ld->addWidget(BlockInterval);
    Ld->addWidget(L[5]);
    G->addLayout(Lc, 0, 1);
    G->addLayout(Ld, 1, 1);
    G->addWidget(ChangeInterval, 2, 1);
    G->addItem(new QSpacerItem(1, 1), 3, 0, 3, 1);
    G->addItem(new QSpacerItem(1, 1), 2, 1, 3, 1);
    // Block widget
    BlockL = new QLineEdit(this);
    BlockL->setEchoMode(QLineEdit::Password);
    OKB = new QPushButton(QIcon(":/main/Lock.png"), "", this);
    OKB->setShortcut(Qt::Key_Return);
    connect(OKB, SIGNAL(clicked()), this, SLOT(Unlock()));
    QWidget *Block = new QWidget(this);
    QGridLayout *Gr = new QGridLayout(Block);
    Gr->addItem(new QSpacerItem(1, 1), 0, 0, 4, 1);
    Gr->addItem(new QSpacerItem(1, 1), 0, 1);
    Gr->addWidget(BlockL, 1, 1);
    Gr->addWidget(OKB, 2, 1);
    Gr->addItem(new QSpacerItem(1, 1), 3, 1);
    Gr->addItem(new QSpacerItem(1, 1), 0, 2, 4, 1);
    // Set layout
    Stacked->addWidget(Label);
    Stacked->addWidget(Calendar);
    Stacked->addWidget(Mail);
    Stacked->addWidget(Notes);
    Stacked->addWidget(AdressBook);
    Stacked->addWidget(RSS);
    Stacked->addWidget(PasswordManager);
    Stacked->addWidget(W);
    Stacked->addWidget(Block);
    layout->addWidget(Stacked, 0, 1);
    layout->setMargin(5);
}
QOrganizer::~QOrganizer() {
    delete hash;
    delete hashed;
}
void QOrganizer::setUser(QString useri, QString *hashedi, QString *hashi) {
    user = useri;
    hashed = hashedi;
    hash = hashi;
    setTree();
    UpdateInterval->setValue(UInterval);
    BlockInterval->setValue(BInterval);
    UTimer->setInterval(UInterval*60*1000);
    BTimer->setInterval(BInterval*60*1000);
}
void QOrganizer::setTree() {
    TreeWidget->setFixedWidth(150);
    TreeWidget->header()->hide();
    QTreeWidgetItem *CalendarTI = new QTreeWidgetItem(TreeWidget);
    CalendarTI->setText(0, "Calendar");
    CalendarTI->setIcon(0, QIcon(":/main/Calendar.png"));
    QStringList categories = Calendar->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(CalendarTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem *MailTI = new QTreeWidgetItem(TreeWidget);
    MailTI->setText(0, "Mail");
    MailTI->setIcon(0, QIcon(":/main/Mail.png"));
    categories = Mail->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(MailTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem *NotesTI = new QTreeWidgetItem(TreeWidget);
    NotesTI->setText(0, "Notes");
    NotesTI->setIcon(0, QIcon(":/main/Notes.png"));
    NotesTI->setExpanded(true);

    QTreeWidgetItem *ADTI = new QTreeWidgetItem(TreeWidget);
    ADTI->setText(0, "Adress Book");
    ADTI->setIcon(0, QIcon(":/main/AdressBook.png"));
    categories = AdressBook->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(ADTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem *RSSTI = new QTreeWidgetItem(TreeWidget);
    RSSTI->setText(0, "Feeds Reader");
    RSSTI->setIcon(0, QIcon(":/main/RSS.png"));
    categories = RSS->getChannels();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem *Itm = new QTreeWidgetItem(RSSTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem *PassTI = new QTreeWidgetItem(TreeWidget);
    PassTI->setText(0, "Password Manager");
    PassTI->setIcon(0, QIcon(":/main/PassMGR.png"));

    QTreeWidgetItem *OptionsTI = new QTreeWidgetItem(TreeWidget);
    OptionsTI->setText(0, "Options");
    OptionsTI->setIcon(0, QIcon(":/main/Options.png"));

    QTreeWidgetItem *SaveTI = new QTreeWidgetItem(TreeWidget);
    SaveTI->setText(0, "Save");
    SaveTI->setIcon(0, QIcon(":/main/Save.png"));
    SaveTI->setFlags(SaveTI->flags()^Qt::ItemIsSelectable);
    TreeWidget->expandAll();
}
void QOrganizer::launchFunction(QTreeWidgetItem *Input) {
    if (Input->parent() == NULL) {
        if (Input->text(0) == "Calendar") {
            Calendar->setCategory("");
            Stacked->setCurrentIndex(1);
        } else if (Input->text(0) == "Mail") {
            Mail->setMail("");
            Stacked->setCurrentIndex(2);
        } else if (Input->text(0) == "Notes") {
            Stacked->setCurrentIndex(3);
        } else if (Input->text(0) == "Adress Book") {
            AdressBook->setCategory("");
            Stacked->setCurrentIndex(4);
        } else if (Input->text(0) == "Feeds Reader") {
            RSS->setChannel("");
            Stacked->setCurrentIndex(5);
        } else if (Input->text(0) == "Password Manager") {
            Stacked->setCurrentIndex(6);
        } else if (Input->text(0) == "Options") {
            Stacked->setCurrentIndex(7);
        } else if (Input->text(0) == "Save") {
            qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org");
            QMessageBox::information(this, "Saved", "Saved.");
            Stacked->setCurrentIndex(0);
        }
    } else {
        if (Input->parent()->text(0) == "Calendar") {
            Calendar->setCategory(Input->text(0));
            Stacked->setCurrentIndex(1);
        } else if (Input->parent()->text(0) == "Mail") {
            Mail->setMail(Input->text(0));
            Stacked->setCurrentIndex(2);
        } else if (Input->parent()->text(0) == "Adress Book") {
            AdressBook->setCategory(Input.text(0));
            Stacked->setCurrentIndex(4);
        } else if (Input->parent()->text(0) == "Feeds Reader") {
            RSS->setChannel(Input->text(0));
            Stacked->setCurrentIndex(5);
        }
    }
}
void QOrganizer::updateCalendar() {
    QString currentCategory;
    bool selected = false;
    QTreeWidgetItem *Itm = TreeWidget->currentItem();
    if (Itm->parent() != NULL) {
        Itm = Itm->parent();
        currentCategory = TreeWidget->currentItem()->text(0);
    }
    for (int i = Itm->childCount(); i > 0; i--) {
        Itm->removeChild(TreeWidget->itemAt(0, 0)->child(i-1));
    }
    QStringList categories = Calendar->getCategories();
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem *Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
            if (categories[i] == currentCategory) {
                Itm->child(i)->setSelected(true);
                selected = true;
            }
        }
        if (!selected) {
            Itm->setSelected(true);
        } else {
            Itm->setSelected(false);
        }
    }
}
void QOrganizer::Notification(QString M) {
    Tray->showMessage("Event notification", M, QSystemTrayIcon::Information, 3000);
}
void QOrganizer::updateMail() {
    QTreeWidgetItem *Itm = TreeWidget->currentItem();
    if (Itm->parent() != NULL) {
        Itm = Itm->parent();
    }
    QString currentCategory = Mail->getCurrent();
    bool selected = false;
    QStringList categories = Mail->getCategories();
    for (int i = Itm->childCount(); i > 0; i--) {
        Itm->removeChild(Itm->child(i-1));
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem *Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
            if (categories[i] == currentCategory) {
                Itm->child(i)->setSelected(true);
                selected = true;
            }
        }
        if (!selected) {
            Itm->setSelected(true);
        } else {
            Itm->setSelected(false);
        }
    }
}
void QOrganizer::updateAdressBook() {
    QTreeWidgetItem *Itm = TreeWidget->currentItem();
    if (Itm->parent() != NULL) {
        Itm = Itm->parent();
    }
    QString currentCategory = AdressBook->getCurrent();
    bool selected = false;
    QStringList categories = AdressBook->getCategories();
    for (int i = Itm->childCount(); i > 0; i--) {
        Itm->removeChild(Itm->child(i-1));
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem *Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
            if (categories[i] == currentCategory) {
                Itm->child(i)->setSelected(true);
                selected = true;
            }
        }
        if (!selected) {
            Itm->setSelected(true);
        } else {
            Itm->setSelected(false);
        }
    }
}
void QOrganizer::updateRSS() {
    QTreeWidgetItem *Itm = TreeWidget->currentItem();
    if (Itm->parent() != NULL) {
        Itm = Itm->parent();
    }
    QString currentCategory = RSS->getCurrent();
    bool selected = false;
    QStringList categories = RSS->getChannels();
    for (int i = Itm->childCount(); i > 0; i--) {
        Itm->removeChild(Itm->child(i-1));
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem *Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
            if (categories[i] == currentCategory) {
                Itm->child(i)->setSelected(true);
                selected = true;
            }
        }
        if (!selected) {
            Itm->setSelected(true);
        } else {
            Itm->setSelected(false);
        }
    }
}
void QOrganizer::doubleClick(QString Text) {
    QTreeWidgetItem *Itm = TreeWidget->selectedItems().first();
    for (int i = 0; Itm->childCount(); i++) {
        if (Itm->child(i)->text(0) == Text) {
            if (!Itm->isExpanded()) {
                Itm->setExpanded(true);
            }
            Itm->setSelected(false);
            Itm->child(i)->setSelected(true);
            break;
        }
    }
}
void QOrganizer::TrayClick(QSystemTrayIcon::ActivationReason I) {
    if (I == QSystemTrayIcon::Context) {
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org");
        closing = true;
        this->close();
    } else if (I == QSystemTrayIcon::DoubleClick) {
        if (this->isHidden()) {
            UTimer->stop();
            BTimer->stop();
            this->show();
        } else {
            UTimer->start();
            BTimer->start();
            Calendar->setCategory("");
            Mail->setMail("");
            RSS->setChannel("");
            if (Stacked->currentIndex() != 8) {
                Stacked->setCurrentIndex(0);
            }
            if (TreeWidget->selectedItems().size() != 0) {
                TreeWidget->selectedItems().first()->setSelected(false);
            }
            this->hide();
        }
    } else if (I == QSystemTrayIcon::MiddleClick&&this->isHidden()) {
        Tray->setIcon(QIcon(":/main/QOrganizerDownload.png"));
        Updates[0]=Calendar->getUpdate();
        Mail->getUpdate();
        RSS->getUpdate();
    }
}
void QOrganizer::MailNews(QString I) {
    Updates[1]=I;
    if (!Updates[2].isEmpty()) {
        Tray->setIcon(QIcon(":/main/QOrganizer.png"));
        Tray->showMessage("Update.", Updates[0]+"\n"+Updates[1]+"\n"+Updates[2], QSystemTrayIcon::Information, 3000);
        Updates[0]="";
        Updates[1]="";
        Updates[2]="";
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org");
    }
}
void QOrganizer::RSSNews(QString I) {
    Updates[2]=I;
    if (!Updates[1].isEmpty()) {
        Tray->setIcon(QIcon(":/main/QOrganizer.png"));
        Tray->showMessage("Update.", Updates[0]+"\n"+Updates[1]+"\n"+Updates[2], QSystemTrayIcon::Information, 3000);
        Updates[0]="";
        Updates[1]="";
        Updates[2]="";
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org");
    }
}
void QOrganizer::closeEvent(QCloseEvent *E) {
    if (!closing) {
        if (!shown) {
            QString M;
            if (UInterval == 1) {
                M="minute";
            } else {
                M="minutes";
            }
            QMessageBox::information(this, "Closing", "QOrganizer will be hidden to the tray and updating every "+QString::number(UInterval)+" "+M+
                                     ".\nClick tray icon with middle mouse button to start immediate update.\nClick tray icon with right mouse button to exit.");
            shown = true;
        }
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove("/")+".org");
        UTimer->start();
        BTimer->start();
        Calendar->setCategory("");
        Mail->setMail("");
        RSS->setChannel("");
        Stacked->setCurrentIndex(0);
        if (TreeWidget->selectedItems().size() != 0) {
            TreeWidget->selectedItems().first()->setSelected(false);
        }
        this->hide();
        E->ignore();
    } else {
        E->accept();
    }
}
void QOrganizer::updateTime() {
    Tray->setIcon(QIcon(":/main/QOrganizerDownload.png"));
    Updates[0]=Calendar->getUpdate();
    Mail->getUpdate();
    RSS->getUpdate();
}
void QOrganizer::Block() {
    if (Stacked->currentIndex() != 8) {
        TreeWidget->setDisabled(true);
        Stacked->setCurrentIndex(8);
    }
}

void QOrganizer::Validator(QString input) {
    if (input.length() < 8&&!input.isEmpty()) {
        NP->setStyleSheet("QLineEdit{background: #FF8888;}");
    } else {
        bool whitespaces = false;
        for (int i = 0; i < input.length(); i++) {
            if (input[i].isSpace()) {
                whitespaces = true;
                break;
            }
        }
        if (whitespaces) {
            NP->setStyleSheet("QLineEdit{background: #FF8888;}");
        } else {
            NP->setStyleSheet("QLineEdit{background: white;}");
        }
    }
}
void QOrganizer::NewPassword() {
    if (NP->styleSheet() == "QLineEdit{background: white;}"&&!(NP->text().isEmpty())) {
        QString CPhash = QString(QCryptographicHash::hash(salting(CP->text()).toUtf8(), QCryptographicHash::Sha3_512));
        QString CPhashed = QString(calculateXOR(CP->text().toUtf8(), CPhash.toUtf8()));
        if (hash == CPhash&&hashed == CPhashed) {
            delete hash;
            delete hashed;
            hash = new QString(QCryptographicHash::hash(salting(NP->text()).toUtf8(), QCryptographicHash::Sha3_512));
            hashed = new QString(calculateXOR(NP->text().toUtf8(), hash->toUtf8()));
            QMessageBox::information(this, "Password", "Password changed.");
        } else {
            QMessageBox::critical(this, "Error", "Error during changing password");
        }
        CP->clear();
        NP->clear();
    }
}
void QOrganizer::SetInterval() {
    if (UpdateInterval->value() > 0&&BlockInterval->value() > 0) {
        UInterval = UpdateInterval->value();
        UTimer->setInterval(UInterval*60*1000);
        BInterval = BlockInterval->value();
        BTimer->setInterval(BInterval*60*1000);
        QMessageBox::information(this, "Times", "Update interval and time to lock changed.");
    }
}
void QOrganizer::Unlock() {
    QString CPhash = QString(QCryptographicHash::hash(salting(BlockL->text()).toUtf8(), QCryptographicHash::Sha3_512));
    QString CPhashed = QString(calculateXOR(BlockL->text().toUtf8(), CPhash.toUtf8()));
    if (hash == CPhash&&hashed == CPhashed) {
        TreeWidget->setEnabled(true);
        BlockL->setStyleSheet("QLineEdit{background: white;}");
        Stacked->setCurrentIndex(0);
    } else {
        BlockL->setStyleSheet("QLineEdit{background: #FF8888;}");
    }
    BlockL->clear();
}
