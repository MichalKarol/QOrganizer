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

bool qorgIO::ReadFile(QByteArray hash, QByteArray hashed, QOrganizer* main, QString path) {
    QFile fileQFile(path);
    if (fileQFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&fileQFile);
        QString text = stream.readAll();
        fileQFile.close();

        QString Header = text.mid(0, 15);
        QString IV;
        QString Data;
        if (Header == "QOrganizer 1.02"
                || Header == "QOrganizer 1.03") {
             IV = text.mid(15, text.indexOf("\n") - 15);
            Data = text.mid(text.indexOf("\n") + 1, text.length()-text.indexOf("\n") - 1);
        } else {
            IV = text.mid(16, text.indexOf("\n", 17) - 16);
            Data = text.mid(text.indexOf("\n" , 17) + 1, text.length()-text.indexOf("\n", 17) - 1);
        }

        QByteArray IVBA = QByteArray::fromBase64(IV.toUtf8());
        IV.clear();

        QByteArray DataBA = QByteArray::fromBase64(Data.toUtf8());
        Data.clear();

        QByteArray Password = calculateXOR(hashed, hash);

        if (Header == "QOrganizer 1.02"
                || Header == "QOrganizer 1.03") {
            if (Password.length() <  32) {
                Password.append(QString(32 - Password.length(), '\0'));
            }
        } else {
            QByteArray P1 = Password;
            IVBA = decryptUsingAES(QByteArray(), IVBA, Password).mid(0, 16);
            Password = calculateXOR(P1, IVBA);
        }

        QByteArray Output = decryptUsingAES(IVBA, DataBA, Password);

        QString Decrypted = Header;
        Decrypted.append(Output);
        Output.clear();

        if (!Decrypted.contains("QOrganizer") || !Decrypted.contains(QString(AES_BLOCK_SIZE, '.'))) {
            Decrypted.clear();
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
        if (main->isHidden()) {
            QMessageBox::critical(main, "Error", "Cannot open file due to"+fileQFile.errorString());
        } else {
            main->Notification("Error", "Cannot open file due to: "+fileQFile.errorString());
        }
        return false;
    }
}
bool qorgIO::SaveFile(QByteArray hash, QByteArray hashed, QOrganizer* main, QString path) {
    QString Out="QOrganizer 1.04\n";
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

    QByteArray Password = calculateXOR(hashed, hash);

    uchar IV[AES_BLOCK_SIZE];
    RAND_bytes(IV, AES_BLOCK_SIZE);

    QByteArray IVBA(reinterpret_cast<const char*>(IV), AES_BLOCK_SIZE);

    memset(&IV, '\0', AES_BLOCK_SIZE);

    QByteArray IVPassword = calculateXOR(Password, IVBA);

    QByteArray IVCopy = IVBA;
    IVBA = encryptUsingAES(QByteArray(), IVBA, Password);
    Out.append(IVBA.toBase64());

    QByteArray Data = QByteArray(data.toUtf8());

    QByteArray Encrypted = encryptUsingAES(IVCopy, Data, IVPassword);

    Out.append("\n");
    Out.append(Encrypted.toBase64());
    Encrypted.clear();

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << Out;
        file.close();
        Out.clear();
        return true;
    } else {
        if (main->isHidden()) {
            QMessageBox::critical(main, "Error", "Cannot save file due to"+file.errorString());
        } else {
           main->Notification("Error", "Cannot save file due to: "+file.errorString());
        }
        return false;
   }
}
QString qorgIO::From102(QString I) {
    QString O;
    QStringList L = I.split("\n\n");
    O.append(L[0]+"\n\n");
    O.append(L[1]+"\n\n");  // Options
    O.append(L[2]+"\n\n");  // Calendar
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
    O.append(L[4]+"\n\n");  // Mail
    O.append(L[5]+"\n\n");  // Notes
    O.append(L[6]+"\n\n");  // AddressBook
    O.append(L[7]+"\n\n");  // Pass manager
    O.append(L[8]);  // AES Check
    return O;
}
QString qorgIO::From103(QString I) {
    QString O;
    QStringList L = I.split("\n\n");
    O.append(L[0]+"\n\n");
    O.append(L[1]+"\n\n");  // Options
    if (!L[2].isEmpty()) {
        QStringList A = L[2].split("\n");
        for (int i = 0; i < A.size(); i++) {
            QStringList B = A[i].split(" ");
            if (B.size()-1 == 7) {
                QString tmp = B[3];
                B.removeAt(3);
                B[6] = B[5];
                B[5] = tmp;  // Set order
                B.append("");

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
        L[2].clear();
        for (int i = 0; i < A.size(); i++) {
            L[2].append(A[i]);
        }
    }
    O.append(L[2]+"\n");  // Calendar
    O.append(L[3]+"\n\n");  // Mail
    O.append(L[4]+"\n\n");  // Notes
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
    O.append(L[5]+"\n");  // Address Book
    O.append(L[6]+"\n\n");  // RSS
    O.append(L[7]+"\n\n");  // Pass manager
    O.append(L[8]);  // AES Check
    return O;
}
