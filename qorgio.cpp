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

#include <qorgio.h>

bool qorgIO::ReadFile(QString *hashed, QString *hash, QOrganizer *main, QString path) {
    QFile file(path);
    QTextStream stream(&file);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = stream.readAll();
    file.close();
    QString Header = text.mid(0, 15);
    QString IV = text.mid(15, text.indexOf("\n")-15);
    QString Data = text.mid(text.indexOf("\n")+1, text.length()-text.indexOf("\n")-1);
    QByteArray IVBA = QByteArray::fromBase64(IV.toUtf8());
    IV.clear();
    QByteArray DataBA = QByteArray::fromBase64(Data.toUtf8());
    Data.clear();
    QString Passwd = QString(calculateXOR(QByteArray::fromBase64(hashed->toUtf8()), hash->toUtf8()));
    if (Passwd.length() <  32) {
        Passwd.append(QString(32-Passwd.length(), '\0'));
    }
    AES_KEY *aesKey = new AES_KEY;
    AES_set_decrypt_key((unsigned char*)Passwd.toUtf8().data(), 256, aesKey);
    int sizeofinput = DataBA.size();
    unsigned char *Output;
    Output = new unsigned char[sizeofinput];
    memset(Output, '\0', sizeof(sizeofinput));
    AES_cbc_encrypt((unsigned char*)DataBA.data(), Output, sizeofinput, aesKey, (unsigned char*)IVBA.data(), AES_DECRYPT);
    Passwd.clear();
    IVBA.clear();
    DataBA.clear();
    QString Decrypted = Header;
    Decrypted.append((const char*)Output);
    memset(Output, 0, sizeof(sizeofinput));
    delete[] Output;
    delete aesKey;
    if (!Decrypted.contains("QOrganizer") || !Decrypted.contains(QString(AES_BLOCK_SIZE, '.'))) {
        QMessageBox::critical(main, "Error", "Invalid file of password!");
        return false;
    }
    QStringList L = Decrypted.split("\n\n");
    main->Options->input(L[1]);
    main->Calendar->input(L[2]);
    main->Mail->input(L[3]);
    main->Notes->input(L[4]);
    main->AdressBook->input(L[5]);
    main->RSS->input(L[6]);
    main->PasswordManager->input(L[7]);
    Decrypted.clear();
    return true;
}
void qorgIO::SaveFile(QString *hashed, QString *hash, QOrganizer *main, QString path) {
    QString Out="QOrganizer 1.02";
    QString data="\n\n";
    data.append(main->Options->output());
    data.append(main->Calendar->output());
    data.append(main->Mail->output());
    data.append(main->Notes->output());
    data.append(main->AdressBook->output());
    data.append(main->RSS->output());
    data.append(main->PasswordManager->output());
    data.append(QString(AES_BLOCK_SIZE, '.'));
    if (data.length() % AES_BLOCK_SIZE != 0) {
        data.append(QByteArray(AES_BLOCK_SIZE - (data.length() % AES_BLOCK_SIZE), ' '));
    }
    QString Passwd = QString(calculateXOR(QByteArray::fromBase64(hashed->toUtf8()), hash->toUtf8()));
    if (Passwd.length() <  32) {
        Passwd.append(QString(32-Passwd.length(), '\0'));
    }
    AES_KEY *aesKey = new AES_KEY;
    AES_set_encrypt_key((unsigned char*)Passwd.toUtf8().data(), 256, aesKey);
    const size_t encslength = data.length();
    unsigned char IV[AES_BLOCK_SIZE+1]={0};
    RAND_bytes(IV, AES_BLOCK_SIZE);
    Out.append(QByteArray((const char*)IV, AES_BLOCK_SIZE+1).toBase64());
    unsigned char* aOUT;
    aOUT = new unsigned char[encslength];
    memset(aOUT, 0, sizeof(encslength));
    AES_cbc_encrypt((unsigned char*)data.toUtf8().data(), aOUT, data.length(), aesKey, IV, AES_ENCRYPT);
    Out+="\n";
    Out.append(QByteArray((const char*)aOUT, encslength).toBase64());
    Passwd.clear();
    memset(aOUT, 0, sizeof(encslength));
    delete[] aOUT;
    delete aesKey;
    QFile file(path);
    QTextStream stream(&file);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    stream << Out;
    file.close();
    Out.clear();
}
