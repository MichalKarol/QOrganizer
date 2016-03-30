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

#include "qorgmodel.h"

// Two classes used for IO
class MainIO : public QObject {
    Q_OBJECT
  public:
    MainIO(QString path, QPair<QByteArray, QByteArray> pair, uint version, qorgModel* parent);
    ~MainIO();

  private:
    QString path;
    QString data[7];
    QPair<QByteArray, QByteArray> pair;
    uint version = 105;

    // Controlling object usage
    int threadNumber = 0;
    QMutex* threadNumberMutex = NULL;

    bool running = false;
    QMutex* runningMutex = NULL;

  public slots:
    void input();
    void output();

  private slots:
    void inputFinished();
    void outputFinished();

  signals:
    void finished();
    void error(QString error);

};
class SimpleIO : public QObject {
    Q_OBJECT
  public:
    SimpleIO(QString& data, uint version, uint nr, qorgModel* parent);
    ~SimpleIO();

  private:
    QString& data;
    uint version = 0;
    uint nr = 0;

  public slots:
    void input();
    void output();

  signals:
    void finished();

};

MainIO::MainIO(QString path, QPair<QByteArray, QByteArray> pair, uint version, qorgModel* parent) : QObject(parent) {
    this->path = path;
    this->pair = pair;
    this->version = version;
    runningMutex = new QMutex();
    threadNumberMutex = new QMutex();

    // Killing itself when ended
    connect(this, &MainIO::finished, this, &MainIO::deleteLater);
    connect(this, &MainIO::error, this, &MainIO::deleteLater);
}
MainIO::~MainIO() {
    this->thread()->quit();
}

void MainIO::input() {
    QMutexLocker locker(runningMutex);

    if (running) {  // Checking if object is not already used
        return;
    }

    locker.unlock();

    QFile file(path);

    if (file.open(QFile::ReadOnly)) {

        QByteArray header = file.read(15);

        if (version >= 104) {
            header.append(file.read(1)); // Reading additional '\n' from header
        }

        QByteArray iv = QByteArray();
        int dataLength = 0;
        QByteArray data = QByteArray();
        QByteArray output = QByteArray();

        auto clear = [&]() { // Lamda function to clear fields
            header.clear();
            iv.clear();
            dataLength = 0;
            data.clear();
            output.clear();
            path.clear();
            pair.first.clear();
            pair.second.clear();
            version = 0;
        };

        if (version >= 102 && version <= 104) {

            // Reading all data then sorting them out
            QByteArray all = file.readAll();

            iv = QByteArray::fromBase64(all.mid(0, all.indexOf("\n")));
            data = QByteArray::fromBase64(all.mid(iv.size() + 1, all.length() - iv.size() - 1));

        } else if (version == 105) {
            iv = file.read(32);
            dataLength = file.read(32).toInt();

            if (dataLength < 0) {
                emit error(QString(QObject::tr("Data length is incorrect.")));
                clear();
                return;
            }

            data = file.read(dataLength);
        }

        file.close();

        QByteArray password = calculateXOR(pair.first, pair.second);

        if (version >= 104) {
            QByteArray tmpPassword = password;

            try {
                iv = decryptAesEcb(iv, tmpPassword);
            } catch (QString ex) {
                emit error(QString(QObject::tr("Exeption occured: ") + ex));

                clear();
                password.clear();
                tmpPassword.clear();

                return;
            }

            password = calculateXOR(password, iv);
        }


        if (version <= 104) {
            try {
                output = decryptAesCbc(iv, data, password);
            } catch (QString ex) {
                emit error(QString(QObject::tr("Exeption occured: ") + ex));

                clear();
                password.clear();

                return;
            }
        } else {
            QByteArray aad = header + iv; // Sending header with iv data to check if nobody tampered with version or iv.

            try {
                output = decryptAesGcm(iv, aad, data, password);
                output = qUncompress(output);
            } catch (QString ex) {
                emit error(QString(QObject::tr("Exeption occured: ") + ex));

                clear();
                password.clear();
                aad.clear();

                return;
            }
        }

        if (output.endsWith(QByteArray(AES_BLOCK_SIZE, '.'))) {

            QStringList parts = QString(output).split("\n\n");
            output.clear();

            if (parts.size() == 9) { // First part is "\n\n" and the last part contains AES_BLOCK_SIZE long dot string.
                parts.removeFirst(); // Removing empty part
                QThread* thread[7];

                for (uint i = 0; i < 7; i++) {
                    thread[i] = new QThread(this);
                    SimpleIO* simpleInput = new SimpleIO(parts[i], version, i, qobject_cast<qorgModel*>(this->parent()));
                    simpleInput->moveToThread(thread[i]);
                    connect(thread[i], &QThread::started, simpleInput, &SimpleIO::input);
                    connect(simpleInput, &SimpleIO::finished, this, &MainIO::inputFinished);
                    thread[i]->start();
                }
            }


        } else {
            emit error(QString(QObject::tr("Invalid file or password.")));

            clear();
        }

    } else {
        emit error("Cannot open file.");
    }
}
void MainIO::output() {
    QMutexLocker locker(runningMutex);

    if (running) {  // Checking if object is not already used
        return;
    }

    locker.unlock();

    QThread* thread[7];

    for (uint i = 0; i < 7; i++) {
        thread[i] = new QThread(this);
        SimpleIO* simpleOutput = new SimpleIO(data[i], version, i, qobject_cast<qorgModel*>(this->parent()));
        simpleOutput->moveToThread(thread[i]);
        connect(thread[i], &QThread::started, simpleOutput, &SimpleIO::output);
        connect(simpleOutput, &SimpleIO::finished, this, &MainIO::outputFinished);
        thread[i]->start();
    }


}
void MainIO::inputFinished() {
}
void MainIO::outputFinished() {
    QMutexLocker locker(threadNumberMutex);
    threadNumber--;

    if (threadNumber == 0) {
        locker.unlock();

        QByteArray header = QByteArray("QOrganizer 1.05\n");
        QByteArray plaintext = QByteArray();
        QByteArray dotBlock = QByteArray(AES_BLOCK_SIZE, '.');

        for (uint i = 0; i < 7; i++) {
            plaintext.append(data[i]);
        }

        plaintext = qCompress(plaintext, 3); // Third level seems to be very good, later time of compression quickly rise
        // All data ready, now prepare for encryption


        uchar* iv = new uchar[AES_BLOCK_SIZE];

        if (RAND_bytes(iv, AES_BLOCK_SIZE)) {
            emit error(QString(QObject::tr("Error occured during generating randomness.")));
            return;
        }

        QByteArray ivByteArray = QByteArray(reinterpret_cast<const char*>(iv), AES_BLOCK_SIZE);
        memset(iv, '\0', AES_BLOCK_SIZE); // Clearing iv block
        delete[] iv;
        iv = NULL;

        QByteArray password = calculateXOR(pair.first, pair.second);
        QByteArray ivPassword = calculateXOR(password, ivByteArray);






    } else {
        Q_ASSERT_X(threadNumber < 0, "Threads control in MainIO", "Error occured.");
        return;
    }
}

SimpleIO::SimpleIO(QString& data, uint version, uint nr, qorgModel* parent) : QObject(parent), data(data) {
    this->version = version;
    this->nr = nr;

    // Killing itself when ended
    connect(this, &SimpleIO::finished, this, &SimpleIO::deleteLater);
}
SimpleIO::~SimpleIO() {
    this->thread()->quit();
}

void SimpleIO::input() {
    qobject_cast<qorgModel*>(this->parent())->inputData(data, version, nr);
    emit finished();
}
void SimpleIO::output() {
    data = qobject_cast<qorgModel*>(this->parent())->outputData(nr);
    emit finished();
}

// Main class
qorgModel::qorgModel(QObject* parent) : QObject(parent) {
}

void qorgModel::loginFunction(QPair<QByteArray, QByteArray> pair, QString path, uint version, QByteArray newHash) {
    if (!newHash.isEmpty()) {

    }

    qDebug() << path;
    QFile file(path);

    if (file.open(QFile::ReadOnly)) {
        QByteArray password = calculateXOR(pair.first, pair.second);
        file.read(16);

        // Changes were made to avoid encoding everything with BASE64
        QByteArray IV = file.read(32);
        int dataLength = file.read(32).toInt();

        if (dataLength < 0) {
            throw QString(QObject::tr("Bad data provided"));
        }

        qDebug() << dataLength;
        QByteArray data = file.read(dataLength);
        qDebug() << data.size();

        QByteArray tmpPassword = password;

        try {
            IV = decryptAesEcb(IV, tmpPassword);
        } catch (QString ex) {
            // TODO(mkarol) Error handling.
        }

        password = calculateXOR(password, IV);

        QByteArray output;

        try {
            output = decryptAesCbc(IV, data, password);
        } catch (QString ex) {
            qDebug() << ex;
            // TODO(mkarol) Error handling.
        }

        qDebug() << output.size();
        qDebug() << output.mid(0, 100);

        QString sdata = output;

        QStringList parts = sdata.split("\n\n");
        qDebug() << parts.size();

        // NOTE(mkarol) CompressLevel 3 is enough

        for (uint i = 0; i < 10; i++) {
            int sum = 0;
            QTime t = QTime::currentTime();

            for (uint j = 0; j < 9; j++) {
                sum += qCompress(parts[j].toUtf8(), i).size();
            }

            int time = t.msecsTo(QTime::currentTime());
            double percent = ((double)sum / (double)output.size()) * 100;
            qDebug() << "PARTS CL: " << i << "SUM: " << sum << "TIME:" << time << "P: " << percent << "R: " << (100 - percent) / time;
        }

        for (uint i = 0; i < 10; i++) {
            int sum = 0;
            QTime t = QTime::currentTime();
            sum += qCompress(sdata.toUtf8(), i).size();
            int time = t.msecsTo(QTime::currentTime());
            double percent = ((double)sum / (double)output.size()) * 100;
            qDebug() << "FULL CL: " << i << "SUM: " << sum << "TIME:" << time << "P: " << percent << "R: " << (100 - percent) / time;
        }

        int sum = 0;
        QTime t = QTime::currentTime();
        sum += qCompress(sdata.toUtf8()).size();
        int time = t.msecsTo(QTime::currentTime());
        double percent = ((double)sum / (double)output.size()) * 100;
        qDebug() << "DEF FULL " << "SUM: " << sum << "TIME:" << time << "P: " << percent << "R: " << (100 - percent) / time;



        file.close();



    } else {
        //QMessageBox::critical(this, "Error", "Cannot open file to read.");
    }







}
void qorgModel::registerFunction(QPair<QByteArray, QByteArray> pair, QString path) {

}

void qorgModel::input(QString input, uint version) {

}

void qorgModel::inputData(QString parts, uint version, uint nr) {
    switch (nr) {
        case 0: {
            options->input(parts, version);
        };

        break;
        case 1: {
            calendar->input(parts, version);
        };

        break;
        case 2: {
            //mail->input(parts, version);
        };

        break;
        case 3: {
            notes->input(parts, version);
        };

        break;
        case 4: {
            //addressbook->input(parts, version);
        };

        break;
        case 5: {
            //feedsreader->input(parts, version);
        };

        break;
        case 6: {
            //passwordmanager->input(parts, version);
        };

        break;
    }
}
QString qorgModel::outputData(uint nr) {
    QString data;

    switch (nr) {
        case 0: {
            data = options->output();
        };

        break;
        case 1: {
            data = calendar->output();
        };

        break;
        case 2: {
            //data = mail->output();
        };

        break;
        case 3: {
            data = notes->output();
        };

        break;
        case 4: {
            //data = addressbook->output();
        };

        break;
        case 5: {
            //data = feedsreader->output();
        };

        break;
        case 6: {
            //data = passwordmanager->output();
        };

        break;
    }

    return data;
}

QString qorgModel::output() {
    return QString();
}

#include "qorgmodel.moc"
