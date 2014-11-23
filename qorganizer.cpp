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


class QOrganizer::VersionUpdater: public QThread {
    Q_OBJECT
public:
    explicit VersionUpdater(QOrganizer* parent) :QThread(parent) {
        work = false;
        running = true;
    }
    void end() {
        while (work) {
            this->msleep(100);
        }
        running = false;
    }
private:
    bool work;
    bool running;
protected:
    void run() {
        while (running) {
            work = true;
            QSslSocket S;
            S.connectToHostEncrypted("raw.githubusercontent.com", 443);
            if (S.waitForConnected()) {
                S.write(QString("GET /MichalKarol/QOrganizer/master/latestVersion.md HTTP/1.1\r\nHost: raw.githubusercontent.com\r\nUser-Agent: QOrganizer\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n").toUtf8());
                if (S.waitForReadyRead()) {
                    QString Reply = S.readAll();
                    Reply.remove(0, Reply.indexOf("\r\n\r\n")+4);
                    Reply = Reply.mid(0, Reply.indexOf("\n"));
                    uint V = Reply.mid(0, 1).toInt();
                    uint SV = Reply.mid(2, 2).toInt();
                    if (V != 1 || SV != 3) {
                        emit VersionUpdate(Reply);
                    }
                }
            }
            S.close();
            work = false;
            for (int i = 0; i < 24*60*60 && running; i++) {
                this->sleep(1);
            }
        }
    }
signals:
    void VersionUpdate(QString);
};
class Closer: public QThread {
    Q_OBJECT
public:
    explicit Closer(QOrganizer* parent, qorgMail* MW, qorgRSS* RW) :QThread(parent) {
        this->MW = MW;
        this->RW = RW;
        M = false;
        R = false;
    }

protected:
    qorgMail* MW;
    qorgRSS* RW;
    bool M;
    bool R;
    void run() {
        while (true) {
            if (!M) {
                if (MW->threadNumber() == 0) {
                    M = true;
                }
            }
            if (!R) {
                if (RW->threadNumber() == 0) {
                    R = true;
                }
            }
            if (M & R) {
                break;
            }
            this->msleep(100);
        }
    }
};

QOrganizer::QOrganizer() {
    setWindowIcon(QIcon(":/main/QOrganizer.png"));
    setWindowTitle("QOrganizer");
    this->setMinimumWidth(1024);
    Options = new qorgOptions(this);
    connect(Options, SIGNAL(Update()), this, SLOT(updateTime()));
    connect(Options, SIGNAL(Block()), this, SLOT(Block()));
    connect(Options, SIGNAL(CNPassword(QString*, QString*, QString*, QString*)), this, SLOT(NewPassword(QString*, QString*, QString*, QString*)));
    AdressBook = new qorgAB(this);
    connect(AdressBook, SIGNAL(updateTree()), this, SLOT(updateAdressBook()));
    Calendar = new qorgCalendar(this, AdressBook);
    connect(Calendar, SIGNAL(updateTree()), this, SLOT(updateCalendar()));
    connect(Calendar, SIGNAL(Notification(QString)), this, SLOT(Notification(QString)));
    Mail = new qorgMail(this, AdressBook, Options);
    connect(Mail, SIGNAL(updateTree()), this, SLOT(updateMail()));
    connect(Mail, SIGNAL(doubleClick(QString)), this, SLOT(doubleClick(QString)));
    connect(Mail, SIGNAL(sendUpdate(QString)), this, SLOT(MailNews(QString)));
    Notes = new qorgNotes(this);
    RSS = new qorgRSS(this, Options);
    connect(RSS, SIGNAL(updateTree()), this, SLOT(updateRSS()));
    connect(RSS, SIGNAL(doubleClick(QString)), this, SLOT(doubleClick(QString)));
    connect(RSS, SIGNAL(sendUpdate(QString)), this, SLOT(RSSNews(QString)));
    PasswordManager = new qorgPasswd(this);
    QGridLayout* layout = new QGridLayout(this);
    TreeWidget = new QTreeWidget();
    connect(TreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(launchFunction(QTreeWidgetItem*)));
    layout->addWidget(TreeWidget, 0, 0, Qt::AlignLeft);
    Stacked = new QStackedWidget();
    // Home widget
    QTextBrowser* Label = new QTextBrowser(this);
    Label->setText("QOrganizer 1.03\nCreated by: Mkarol (mkarol@linux.pl)\n16.11.2014\nYou could help in developing this software by donating:"
                   "\nBitcoins: 17wTU13S31LMdVuVmxxXyBwnj7kJwm74wK");
    VU =new VersionUpdater(this);
    connect(VU, SIGNAL(VersionUpdate(QString)), this, SLOT(VersionUpdate(QString)));
    VU->start();
    // Timers and tray
    Tray = new QSystemTrayIcon(QIcon(":/main/QOrganizer.png"), this);
    Tray->show();
    closing = false;
    shown = false;
    // Block widget
    BlockL = new QLineEdit(this);
    BlockL->setEchoMode(QLineEdit::Password);
    OKB = new QPushButton(QIcon(":/main/Lock.png"), "", this);
    OKB->setShortcut(Qt::Key_Return);
    connect(OKB, SIGNAL(clicked()), this, SLOT(Unlock()));
    QWidget* Block = new QWidget(this);
    QGridLayout* Gr = new QGridLayout(Block);
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
    Stacked->addWidget(Options);
    Stacked->addWidget(Block);
    layout->addWidget(Stacked, 0, 1);
    layout->setMargin(5);
}
QOrganizer::~QOrganizer() {
    for (int i = TreeWidget->topLevelItemCount(); i > 0; i--) {
        for (int j = TreeWidget->topLevelItem(i-1)->childCount(); j > 0; j--) {
            delete TreeWidget->topLevelItem(i-1)->child(j-1);
        }
        delete TreeWidget->topLevelItem(i-1);
    }
    delete hash;
    delete hashed;
}
void QOrganizer::setUser(QString useri, QString* hashedi, QString* hashi) {
    user = useri;
    hashed = hashedi;
    hash = hashi;
    connect(Tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayClick(QSystemTrayIcon::ActivationReason)));
    setTree();
}
void QOrganizer::setTree() {
    TreeWidget->setFixedWidth(150);
    TreeWidget->header()->hide();
    QTreeWidgetItem* CalendarTI = new QTreeWidgetItem(TreeWidget);
    CalendarTI->setText(0, "Calendar");
    CalendarTI->setIcon(0, QIcon(":/main/Calendar.png"));
    QStringList categories = Calendar->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(CalendarTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem* MailTI = new QTreeWidgetItem(TreeWidget);
    MailTI->setText(0, "Mail");
    MailTI->setIcon(0, QIcon(":/main/Mail.png"));
    categories = Mail->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(MailTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem* NotesTI = new QTreeWidgetItem(TreeWidget);
    NotesTI->setText(0, "Notes");
    NotesTI->setIcon(0, QIcon(":/main/Notes.png"));
    NotesTI->setExpanded(true);

    QTreeWidgetItem* ADTI = new QTreeWidgetItem(TreeWidget);
    ADTI->setText(0, "Adress Book");
    ADTI->setIcon(0, QIcon(":/main/AdressBook.png"));
    categories = AdressBook->getCategories();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(ADTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem* RSSTI = new QTreeWidgetItem(TreeWidget);
    RSSTI->setText(0, "Feeds Reader");
    RSSTI->setIcon(0, QIcon(":/main/RSS.png"));
    categories = RSS->getChannels();
    for (int i = 0; i < categories.size(); i++) {
        QTreeWidgetItem* Itm = new QTreeWidgetItem(RSSTI);
        Itm->setText(0, categories[i]);
        Itm->setToolTip(0, categories[i]);
    }

    QTreeWidgetItem* PassTI = new QTreeWidgetItem(TreeWidget);
    PassTI->setText(0, "Password Manager");
    PassTI->setIcon(0, QIcon(":/main/PassMGR.png"));

    QTreeWidgetItem* OptionsTI = new QTreeWidgetItem(TreeWidget);
    QTreeWidgetItem* OTI1 = new QTreeWidgetItem(OptionsTI);
    OTI1->setText(0, "SSL Manager");
    OptionsTI->setText(0, "Options");
    OptionsTI->setIcon(0, QIcon(":/main/Options.png"));

    QTreeWidgetItem* SaveTI = new QTreeWidgetItem(TreeWidget);
    SaveTI->setText(0, "Save");
    SaveTI->setIcon(0, QIcon(":/main/Save.png"));
    SaveTI->setFlags(SaveTI->flags()^Qt::ItemIsSelectable);
    TreeWidget->expandAll();
}

void QOrganizer::launchFunction(QTreeWidgetItem* Input) {
    if (Input->parent() == NULL) {
        if (Input->text(0) == "Calendar") {
            Calendar->setCategory("");
            Stacked->setCurrentIndex(1);
        } else if (Input->text(0) == "Mail") {
            Mail->setMail(-1);
            Stacked->setCurrentIndex(2);
        } else if (Input->text(0) == "Notes") {
            Stacked->setCurrentIndex(3);
        } else if (Input->text(0) == "Adress Book") {
            AdressBook->setCategory("");
            Stacked->setCurrentIndex(4);
        } else if (Input->text(0) == "Feeds Reader") {
            RSS->setChannel(-1);
            Stacked->setCurrentIndex(5);
        } else if (Input->text(0) == "Password Manager") {
            Stacked->setCurrentIndex(6);
        } else if (Input->text(0) == "Options") {
            Options->setWidget(0);
            Stacked->setCurrentIndex(7);
        } else if (Input->text(0) == "Save") {
            qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove(QDir::separator())+".org");
            QMessageBox::information(this, "Saved", "Saved.");
            Stacked->setCurrentIndex(0);
        }
    } else {
        if (Input->parent()->text(0) == "Calendar") {
            Calendar->setCategory(Input->text(0));
            Stacked->setCurrentIndex(1);
        } else if (Input->parent()->text(0) == "Mail") {
            for (int i = 0; i < Input->parent()->childCount(); i++) {
                if (Input->parent()->child(i) == Input) {
                    Mail->setMail(i);
                    break;
                }
            }
            Stacked->setCurrentIndex(2);
        } else if (Input->parent()->text(0) == "Adress Book") {
            AdressBook->setCategory(Input->text(0));
            Stacked->setCurrentIndex(4);
        } else if (Input->parent()->text(0) == "Feeds Reader") {
            for (int i = 0; i < Input->parent()->childCount(); i++) {
                if (Input->parent()->child(i) == Input) {
                    RSS->setChannel(i);
                    break;
                }
            }
            Stacked->setCurrentIndex(5);
        } else if (Input->parent()->text(0) == "Options") {
            Options->setWidget(1);
            Stacked->setCurrentIndex(7);
        }
    }
}
void QOrganizer::doubleClick(QString Text) {
    QTreeWidgetItem* Itm = TreeWidget->selectedItems().first();
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

void QOrganizer::updateCalendar() {
    QString currentCategory = TreeWidget->currentItem()->text(0);
    bool selected = false;
    QTreeWidgetItem* Itm = TreeWidget->topLevelItem(0);
    for (int i = Itm->childCount(); i > 0; i--) {
        Itm->removeChild(TreeWidget->itemAt(0, 0)->child(i-1));
    }
    QStringList categories = Calendar->getCategories();
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem* Itmc = new QTreeWidgetItem(Itm);
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
    QTreeWidgetItem* Itm = TreeWidget->topLevelItem(1);
    int currentMail = Mail->getCurrent();
    QStringList categories = Mail->getCategories();
    for (int i = Itm->childCount(); i > 0; i--) {
        delete Itm->child(i-1);
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem* Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
        }
        if (currentMail != -1) {
            Itm->setSelected(false);
            Itm->child(currentMail)->setSelected(true);
        } else {
            Itm->setSelected(true);
        }
    }
}
void QOrganizer::MailNews(QString I) {
    Updates[1]=I;
    TreeWidget->topLevelItem(1)->setDisabled(false);
    TreeWidget->topLevelItem(1)->setToolTip(0, "");
    if (!Updates[2].isEmpty()) {
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove(QDir::separator())+".org");
        Tray->setIcon(QIcon(":/main/QOrganizer.png"));
        Tray->showMessage("Update.", Updates[0]+"\n"+Updates[1]+"\n"+Updates[2], QSystemTrayIcon::Information, 3000);
        Updates[0].clear();
        Updates[1].clear();
        Updates[2].clear();
        Tray->setToolTip("");
    } else {
        Tray->setToolTip("RSS is updating.");
    }
}

void QOrganizer::updateAdressBook() {
    QTreeWidgetItem* Itm = TreeWidget->topLevelItem(3);
    QString currentCategory = AdressBook->getCurrent();
    bool selected = false;
    QStringList categories = AdressBook->getCategories();
    for (int i = Itm->childCount(); i > 0; i--) {
        delete Itm->child(i-1);
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem* Itmc = new QTreeWidgetItem(Itm);
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
    QTreeWidgetItem* Itm = TreeWidget->topLevelItem(4);
    int currentChannel = RSS->getCurrent();
    QStringList categories = RSS->getChannels();
    for (int i = Itm->childCount(); i > 0; i--) {
        delete Itm->child(i-1);
    }
    if (categories.size() > 0) {
        Itm->setExpanded(true);
        for (int i = 0; i < categories.size(); i++) {
            QTreeWidgetItem* Itmc = new QTreeWidgetItem(Itm);
            Itmc->setText(0, categories[i]);
            Itmc->setToolTip(0, categories[i]);
        }
        if (currentChannel != -1) {
            Itm->child(currentChannel)->setSelected(true);
            Itm->setSelected(false);
        } else {
            Itm->setSelected(true);
        }
    }
}
void QOrganizer::RSSNews(QString I) {
    Updates[2]=I;
    TreeWidget->topLevelItem(4)->setDisabled(false);
    TreeWidget->topLevelItem(4)->setToolTip(0, "");
    if (!Updates[1].isEmpty()) {
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove(QDir::separator())+".org");
        Tray->setIcon(QIcon(":/main/QOrganizer.png"));
        Tray->showMessage("Update.", Updates[0]+"\n"+Updates[1]+"\n"+Updates[2], QSystemTrayIcon::Information, 3000);
        Updates[0].clear();
        Updates[1].clear();
        Updates[2].clear();
        Tray->setToolTip("");
    } else {
        Tray->setToolTip("Mail is updating.");
    }
}

void QOrganizer::TrayClick(QSystemTrayIcon::ActivationReason I) {
    if (I == QSystemTrayIcon::Context) {
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove(QDir::separator())+".org");
        closing = true;
        this->close();
    } else if (I == QSystemTrayIcon::DoubleClick) {
        BlockL->clear();
        if (this->isHidden()) {
            Options->stop(0);
            Options->stop(1);
            this->show();
        } else {
            Options->start(0);
            Options->start(1);
            Calendar->setCategory("");
            Mail->setMail(-1);
            RSS->setChannel(-1);
            if (Stacked->currentIndex() != 8) {
                Stacked->setCurrentIndex(0);
            }
            if (TreeWidget->selectedItems().size() != 0) {
                TreeWidget->selectedItems().first()->setSelected(false);
            }
            this->hide();
        }
    } else if (I == QSystemTrayIcon::MiddleClick && this->isHidden() && Tray->icon().pixmap(20, 20).toImage() != QIcon(":/main/QOrganizerDownload.png").pixmap(20, 20).toImage()) {
        Tray->setIcon(QIcon(":/main/QOrganizerDownload.png"));
        Updates[0] = Calendar->getUpdate();
        Mail->getUpdate();
        RSS->getUpdate();
        Tray->setToolTip("Mail and RSS is updating.");
        Options->stop(0);
        Options->start(0);
        TreeWidget->topLevelItem(1)->setDisabled(true);
        TreeWidget->topLevelItem(1)->setToolTip(0, "Mail is updating right now.");
        TreeWidget->topLevelItem(4)->setDisabled(true);
        TreeWidget->topLevelItem(4)->setToolTip(0, "RSS reader is updating right now.");
    }
}
void QOrganizer::closeEvent(QCloseEvent* E) {
    if (!closing) {
        if (!shown) {
            QString M;
            if (Options->UInterval == 1) {
                M="minute";
            } else {
                M="minutes";
            }
            QMessageBox::information(this, "Closing", "QOrganizer will be hidden to the tray and updating every "+QString::number(Options->UInterval)+" "+M+
                                     ".\nClick tray icon with middle mouse button to start immediate update.\nClick tray icon with right mouse button to exit.");
            shown = true;
        }
        Options->start(0);
        Options->start(1);
        Calendar->setCategory("");
        Mail->setMail(-1);
        RSS->setChannel(-1);
        Stacked->setCurrentIndex(0);
        if (TreeWidget->selectedItems().size() != 0) {
            TreeWidget->selectedItems().first()->setSelected(false);
        }
        this->hide();
        E->ignore();
    } else {
        QEventLoop L1;
        VU->end();
        connect(VU, SIGNAL(finished()), &L1, SLOT(quit()));
        L1.exec();
        Closer* C = new Closer(this, Mail, RSS);
        QEventLoop L2;
        connect(C, SIGNAL(finished()), &L2, SLOT(quit()));
        C->start();
        L2.exec();
        qorgIO::SaveFile(hashed, hash, this, QDir::homePath()+"/.qorganizer/"+QString::fromUtf8(QCryptographicHash::hash(user.toUtf8(), QCryptographicHash::Sha3_512).toBase64()).remove(QDir::separator())+".org");
        E->accept();
    }
}
void QOrganizer::VersionUpdate(QString I) {
    if (this->isHidden()) {
        QMessageBox::information(this, "Version", "New version available: "+I);
    } else {
        Tray->showMessage("Version", "New version available: "+I);
    }
}

void QOrganizer::updateTime() {
    if (Updates[0].isEmpty()) {
        Tray->setIcon(QIcon(":/main/QOrganizerDownload.png"));
        Updates[0]=Calendar->getUpdate();
        Mail->getUpdate();
        RSS->getUpdate();
        Tray->setToolTip("Mail and RSS is updating.");
        TreeWidget->topLevelItem(1)->setDisabled(true);
        TreeWidget->topLevelItem(1)->setToolTip(0, "Mail is updating right now.");
        TreeWidget->topLevelItem(4)->setDisabled(true);
        TreeWidget->topLevelItem(4)->setToolTip(0, "RSS reader is updating right now.");
    }
}
void QOrganizer::Block() {
    TreeWidget->setDisabled(true);
    TreeWidget->collapseAll();
    Stacked->setCurrentIndex(8);
    Options->stop(1);
}
void QOrganizer::NewPassword(QString* CA, QString* CB, QString* NA, QString* NB) {
    if ((*hash) == CA && (*hashed) == CB) {
        delete hash;
        delete hashed;
        hash = NA;
        hashed = NB;
        QMessageBox::information(this, "Password", "Password changed.");
    } else {
        QMessageBox::critical(this, "Error", "Error during changing password");
    }
}

void QOrganizer::Unlock() {
    QString CPhash = QString(QCryptographicHash::hash(salting(BlockL->text()).toUtf8(), QCryptographicHash::Sha3_512));
    QString CPhashed = QString(calculateXOR(BlockL->text().toUtf8(), CPhash.toUtf8()).toBase64());
    if (hash == CPhash&&hashed == CPhashed) {
        TreeWidget->setEnabled(true);
        BlockL->setStyleSheet("QLineEdit{background: white;}");
        Stacked->setCurrentIndex(0);
    } else {
        BlockL->setStyleSheet("QLineEdit{background: #FF8888;}");
    }
    BlockL->clear();
}

#include <qorganizer.moc>
