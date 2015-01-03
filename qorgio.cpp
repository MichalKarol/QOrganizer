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

bool qorgIO::ReadFile(QString* hashed, QString* hash, QOrganizer* main, QString path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
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
        AES_KEY* aesKey = new AES_KEY;
        AES_set_decrypt_key((unsigned char*)Passwd.toUtf8().data(), 256, aesKey);
        int sizeofinput = DataBA.size();
        unsigned char* Output;
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
        if (Decrypted.contains("QOrganizer 1.02")) {
            Decrypted = From103(From102(Decrypted));
        }
        if (Decrypted.contains("QOrganizer 1.03")) {
            Decrypted = From103(Decrypted);
        }
        QStringList L = Decrypted.split("\n\n");
        main->Options->input(L[1]);
        main->Calendar->input(L[2]);
        main->Mail->input(L[3]);
        main->Notes->input(L[4]);
        main->AddressBook->input(L[5]);
        main->RSS->input(L[6]);
        main->PasswordManager->input(L[7]);
        Decrypted.clear();
        return true;
    } else {
        if(main->isHidden()) {
            QMessageBox::critical(main, "Error", "Cannot open file due to"+file.errorString());
        } else {
            main->Notification("Error","Cannot open file due to: "+file.errorString());
        }
        return false;
    }
}
bool qorgIO::SaveFile(QString* hashed, QString* hash, QOrganizer* main, QString path) {
    QString Out="QOrganizer 1.04";
    QString data="\n\n";
    data.append(main->Options->output());
    data.append(main->Calendar->output());
    data.append(main->Mail->output());
    data.append(main->Notes->output());
    data.append(main->AddressBook->output());
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
    AES_KEY* aesKey = new AES_KEY;
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
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << Out;
        file.close();
        Out.clear();
        return true;
    } else {
        if(main->isHidden()) {
            QMessageBox::critical(main, "Error", "Cannot save file due to"+file.errorString());
        } else {
           main->Notification("Error","Cannot save file due to: "+file.errorString());
        }
        return false;
   }
}
QString qorgIO::From102(QString I) {
    QString O;
    QStringList L = I.split("\n\n");
    O.append(L[0]+"\n\n");
    O.append(L[1]+"\n\n");
    O.append(L[2]+"\n\n");
    if (!L[3].isEmpty()) {
        QStringList A = L[3].split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            switch (B.size()-1) {
            case 5: {
            }break;
            case 6: {
            }break;
            case 8: {
                B.removeAt(1);  // Remove Name of sender
                B.append("");   // Email_Message_ID
                B.append("");   // Email_ReplyTo
                B.append("");   // Email_RecipientsTo
                B.append("");   // Email_RecipientsCC
            }break;
            case 14: {
            }break;
            }
            A[i].clear();
            for (int j = 0; j < B.size()-1; j++) {
                A[i].append(B[j]);
                if (j+1 != B.size()-1) {
                    A[i].append(" ");
                } else {
                    A[i].append(" \n");
                }
            }
            B = A[i].split(" ");
        }
        L[3].clear();
        for (int i = 0; i < A.size(); i++) {
            L[3].append(A[i]);
        }
    }
    O.append(L[3]+"\n");
    O.append(L[4]+"\n\n");
    O.append(L[5]+"\n\n");
    O.append(L[6]+"\n\n");
    O.append(L[7]+"\n\n");
    O.append(L[8]);
    return O;
}
QString qorgIO::From103(QString I) {
    QString O;
    QStringList L = I.split("\n\n");
    O.append(L[0]+"\n\n");
    O.append(L[1]+"\n\n"); // Options
    O.append(L[2]+"\n\n"); // Calendar
    O.append(L[3]+"\n\n"); // Mail
    O.append(L[4]+"\n\n"); // Notes
    if (!L[5].isEmpty()) {
        QStringList A = L[5].split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            if (B.size()-1 == 10) {
                B.append("");   // Photo
                B.append("");   // ExtraInfomation
            }
            A[i].clear();
            for (int j = 0; j < B.size()-1; j++) {
                A[i].append(B[j]);
                if (j+1 != B.size()-1) {
                    A[i].append(" ");
                } else {
                    A[i].append(" \n");
                }
            }
            B = A[i].split(" ");
        }
        L[5].clear();
        for (int i = 0; i < A.size(); i++) {
            L[5].append(A[i]);
        }
    }
    O.append(L[5]+"\n"); // Address Book
    O.append(L[6]+"\n\n"); //RSS
    O.append(L[7]+"\n\n"); //Pass manager
    O.append(L[8]); //AES_CHECK
    return O;
}
