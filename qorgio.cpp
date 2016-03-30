//    Copyright (C) 2014 Michał Karol <michal.p.karol@gmail.com>

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

#include <qorgio.h>

bool qorgIO::readFile(QPair <QByteArray, QByteArray> passwordPair, qorgModel* mainModel) {
    /*QFile file(mainModel->getPath());
    if (file.open(QIODevice::ReadOnly)) {
        QString header = file.read(16).remove(15,1); // Removing \n from header
        QByteArray IV;
        int dataLength = 0;
        QByteArray data;
        if (header == "QOrganizer 1.02"
                || header == "QOrganizer 1.03"
                || header == "QOrganizer 1.04") {
            QByteArray text  = file.readAll();
            // Read all at once
            if (header == "QOrganizer 1.02"
                    || header == "QOrganizer 1.03") {
                IV = QByteArray::fromBase64(text.mid(0, text.indexOf("\n")));
                data = QByteArray::fromBase64(text.mid(text.indexOf("\n") + 1, text.length()-text.indexOf("\n") - 1));
            } else if (header == "QOrganizer 1.04") {
                IV = QByteArray::fromBase64(text.mid(0, text.indexOf("\n")));
                data = QByteArray::fromBase64(text.mid(text.indexOf("\n") + 1, text.length()-text.indexOf("\n") - 1));
            }
        } else if (header == "QOrganizer 1.05") {
            // Changes were made to avoid encoding everything with BASE64
            IV = file.read(32);
            dataLength = file.read(32).toInt();
            if (dataLength < 0) {
                return false;
            }
            data = file.read(dataLength);
        }
        file.close();

        QByteArray password = calculateXOR(hashed, hash);
        if (header == "QOrganizer 1.02"
                || header == "QOrganizer 1.03") {
            if (password.length() <  32) {
                password.append(QString(32 - password.length(), '\0'));
            }
        } else {
            QByteArray p1 = password;
            IV = decryptUsingAES(QByteArray(), IV, password).mid(0, 16);
            password = calculateXOR(p1, IV);
        }

        QByteArray output = decryptUsingAES(IV, data, password);

        QString decryptedString = header;
        decryptedString.append(output);
        output.clear();

    //        if (!(decryptedString.contains("QOrganizer")
    //              && decryptedString.contains(QString(AES_BLOCK_SIZE, '.')))) {
    //            decryptedString.clear();
    //            QMessageBox::critical(main, "Error", "Invalid file of password!");
    //            return false;
    //        }

        uint version = header.mid(11, 1).toUInt()*100 + header.mid(13, 2).toUInt();
        mainModel->input(decryptedString, version);
        decryptedString.clear();

        return true;
    } else {
    //        if (main->isHidden()) {
    //            QMessageBox::critical(main, "Error", "Cannot open file due to: "+file.errorString());
    //        } else {
    //            main->Notification("Error", "Cannot open file due to: "+file.errorString());
    //        }
        return false;
    }*/
}
bool qorgIO::saveFile(QPair<QByteArray, QByteArray> passwordPair, qorgModel* mainModel) {
    /*QByteArray output="QOrganizer 1.05\n";

    QString dataString = mainModel->output();
    dataString.append(QString(AES_BLOCK_SIZE, '.'));
    if (dataString.length() % AES_BLOCK_SIZE != 0) {
        dataString.append(QByteArray(AES_BLOCK_SIZE - (dataString.length() % AES_BLOCK_SIZE), ' '));
    }

    QByteArray password = calculateXOR(hashed, hash);

    uchar *IV = new uchar[AES_BLOCK_SIZE];
    RAND_bytes(IV, AES_BLOCK_SIZE);

    QByteArray IVByteArray(reinterpret_cast<const char*>(IV), AES_BLOCK_SIZE);

    memset(IV, '\0', AES_BLOCK_SIZE);

    QByteArray IVPassword = calculateXOR(password, IVByteArray);

    QByteArray IVCopy = IVByteArray;
    IVByteArray = encryptUsingAES(QByteArray(), IVByteArray, password);
    output.append(IVByteArray);

    QByteArray data = QByteArray(dataString.toUtf8());
    dataString.clear();

    QByteArray encrypted = encryptUsingAES(IVCopy, data, IVPassword);
    QString encryptedSizeNumber = QString("%1").arg(encrypted.size(), 32, 10, QChar('0'));

    output.append(encryptedSizeNumber);
    output.append(encrypted);
    encrypted.clear();

    QFile file(mainModel->getPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(output.data(), output.size());
        file.close();
        output.clear();
        return true;
    } else {
        output.clear();
    //        if (main->isHidden()) {
    //            QMessageBox::critical(main, "Error", "Cannot save file due to"+file.errorString());
    //        } else {
    //            main->Notification("Error", "Cannot save file due to: "+file.errorString());
    //        }
        return false;
    }*/
}
