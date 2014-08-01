#include "qorgio.h"
#include  <openssl/aes.h>
#include  <openssl/rand.h>
using namespace std;
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
    QString Passwd = QString(calculateXOR(hashed->toUtf8(), hash->toUtf8()));
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
    if ((!Decrypted.contains("<ORG>"))||(!Decrypted.contains("</ORG>"))||(!Decrypted.contains("QOrganizer"))) {
        QMessageBox::critical(main, "Error", "Invalid file of password!");
        return false;
    }
    Decrypted = InputSS(Decrypted, "ORG");
    main->UInterval = InputI(Decrypted, "UINTERVAL");
    main->BInterval = InputI(Decrypted, "BINTERVAL");
    main->Calendar->input(InputSS(Decrypted, "CALENDAR"));
    main->Mail->input(InputSS(Decrypted, "MAILV"));
    main->Notes->input(InputSS(Decrypted, "NOTES"));
    main->AdressBook->input(InputSS(Decrypted, "PERSONV"));
    main->RSS->input(InputSS(Decrypted, "CHANNELV"));
    main->PasswordManager->input(InputSS(Decrypted, "PROGRAMV"));
    Decrypted.clear();
    return true;
}
void qorgIO::SaveFile(QString *hashed, QString *hash, QOrganizer *main, QString path) {
    QString OUT="QOrganizer 1.00";
    QString data;
    data.append(main->Calendar->output());
    data.append(main->Mail->output());
    data.append(main->Notes->output());
    data.append(main->AdressBook->output());
    data.append(main->RSS->output());
    data.append(main->PasswordManager->output());
    data.append(OutputTools(static_cast<int>(main->UInterval), "UINTERVAL"));
    data.append(OutputTools(static_cast<int>(main->BInterval), "BINTERVAL"));
    data = OutputToolsS(data, "ORG");
    if (data.length()%AES_BLOCK_SIZE == 0) {
        data+=".";
    }
    QString Passwd = QString(calculateXOR(hashed->toUtf8(), hash->toUtf8()));
    if (Passwd.length() <  32) {
        Passwd.append(QString(32-Passwd.length(), '\0'));
    }
    AES_KEY *aesKey = new AES_KEY;
    AES_set_encrypt_key((unsigned char*)Passwd.toUtf8().data(), 256, aesKey);
    const size_t encslength = ((data.length() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char IV[AES_BLOCK_SIZE+1]={0};
    RAND_bytes(IV, AES_BLOCK_SIZE);
    OUT.append(QByteArray((const char*)IV, AES_BLOCK_SIZE+1).toBase64());
    unsigned char* aOUT;
    aOUT = new unsigned char[encslength];
    memset(aOUT, 0, sizeof(encslength));
    AES_cbc_encrypt((unsigned char*)data.toUtf8().data(), aOUT, data.length(), aesKey, IV, AES_ENCRYPT);
    OUT+="\n";
    OUT.append(QByteArray((const char*)aOUT, encslength).toBase64());
    Passwd.clear();
    memset(aOUT, 0, sizeof(encslength));
    delete[] aOUT;
    delete aesKey;
    QFile file(path);
    QTextStream stream(&file);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    stream << OUT;
    file.close();
    OUT.clear();
}
