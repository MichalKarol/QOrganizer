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

#include <qorgtools.h>
// Input filters
QString inputString(QString input) {
    if (input[0] == QChar(0x10FFFF)) {
        return QString(QByteArray::fromBase64(input.remove(0, 1).toUtf8()));
    }

    return input;
}
uint inputUInt(QString input) {
    return static_cast<uint>(input.toInt());
}
uchar inputUchar(QString input) {
    return static_cast<uchar>(input.toInt());
}
bool inputBool(QString input) {
    return (input == "1");
}
QDateTime inputDateTime(QString input) {
    return QDateTime::fromString(input, Qt::ISODate);
}
QDate inputDate(QString input) {
    return QDate::fromString(input, Qt::ISODate);
}

// Output filters
QString outputFilter(QString input) {
    if (input.contains(' ')
            || input.contains('\n')) {
        return QChar(0x10FFFF) + QString(input.toUtf8().toBase64());
    }

    return input;
}
QString outputFilter(uint input) {
    return QString::number(input);
}
QString outputFilter(uchar input) {
    return outputFilter(static_cast<uint>(input));
}
QString outputFilter(bool input) {
    return (input ? "1" : "0" );
}
QString outputFilter(QDateTime input) {
    return input.toString(Qt::ISODate);
}
QString outputFilter(QDate input) {
    return input.toString(Qt::ISODate);
}

// Encryption
QByteArray encryptAesEcb(QByteArray& toEncrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Encryption error."));
    QByteArray output;

    AES_KEY* aesKey = new AES_KEY;

    if (AES_set_encrypt_key(reinterpret_cast<uchar*>(password.data()), 256, aesKey)) {
        delete aesKey;
        aesKey = NULL;
        throw errorMessage;
    }

    uchar* ciphertext = new uchar[16];
    memset(ciphertext, '\0', 16);
    AES_ecb_encrypt(reinterpret_cast<uchar*>(toEncrypt.data()), ciphertext, aesKey, AES_ENCRYPT);
    delete aesKey;
    aesKey = NULL;

    output = QByteArray(reinterpret_cast<const char*>(ciphertext), 16);
    memset(ciphertext, '\0', 16);

    delete[] ciphertext;
    ciphertext = NULL;
    toEncrypt.clear();
    password.clear();

    return output;
}
QByteArray encryptAesCbc(QByteArray& iv, QByteArray& toEncrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Encryption error."));
    QByteArray output;

    AES_KEY* aesKey = new AES_KEY;

    if (AES_set_encrypt_key(reinterpret_cast<uchar*>(password.data()), 256, aesKey)) {
        delete aesKey;
        aesKey = NULL;
        throw errorMessage;
    }

    size_t dataSize = ((toEncrypt.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uchar* ciphertext = new uchar[dataSize];
    memset(ciphertext, '\0', dataSize);
    AES_cbc_encrypt(reinterpret_cast<uchar*>(toEncrypt.data()), ciphertext, dataSize, aesKey,
                    reinterpret_cast<uchar*>(iv.data()), AES_ENCRYPT);
    delete aesKey;
    aesKey = NULL;

    output = QByteArray(reinterpret_cast<const char*>(ciphertext), dataSize);
    memset(ciphertext, '\0', dataSize);

    delete[] ciphertext;
    ciphertext = NULL;
    iv.clear();
    toEncrypt.clear();
    password.clear();
    dataSize = 0;

    return output;
}
QByteArray encryptAesGcm(QByteArray& iv, QByteArray& aad, QByteArray& toEncrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Encryption error."));
    QByteArray output;

    EVP_CIPHER_CTX* ctx;
    int len;
    int ciphertext_len;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        throw errorMessage;
    }

    /* Initialise the encryption operation. */
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw errorMessage;
    }

    /* Set IV length if default 12 bytes (96 bits) is not appropriate */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw errorMessage;
    }

    /* Initialise key and IV */
    if (!EVP_EncryptInit_ex(ctx, NULL, NULL, reinterpret_cast<uchar*>(password.data()), reinterpret_cast<uchar*>(iv.data()))) {
        EVP_CIPHER_CTX_free(ctx);
        throw errorMessage;
    }

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if (!aad.isEmpty()) {
        if (!EVP_EncryptUpdate(ctx, NULL, &len, reinterpret_cast<uchar*>(aad.data()), aad.length())) {
            EVP_CIPHER_CTX_free(ctx);
            throw errorMessage;
        }
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    size_t dataSize = ((toEncrypt.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uchar* ciphertext = new uchar[dataSize];
    memset(ciphertext, '\0', dataSize);

    if (!EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<uchar*>(toEncrypt.data()), toEncrypt.length())) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] ciphertext;
        ciphertext = NULL;
        throw errorMessage;
    }

    ciphertext_len = len;

    /* Finalise the encryption. Normally ciphertext bytes may be written at
     * this stage, but this does not occur in GCM mode
     */
    if (!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] ciphertext;
        ciphertext = NULL;
        throw errorMessage;
    }

    ciphertext_len += len;

    uchar* tag = new uchar[16];
    memset(tag, '\0', 16);

    /* Get the tag */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] ciphertext;
        ciphertext = NULL;
        delete[] tag;
        tag = NULL;
        throw errorMessage;
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    output = QByteArray(reinterpret_cast<const char*>(tag), 16);
    output.append(QByteArray(reinterpret_cast<const char*>(ciphertext), ciphertext_len));

    memset(ciphertext, '\0', dataSize);
    memset(tag, '\0', 16);

    delete[] ciphertext;
    ciphertext = NULL;
    delete[] tag;
    tag = NULL;
    iv.clear();
    toEncrypt.clear();
    password.clear();

    return output;
}

// Decryption
QByteArray decryptAesEcb(QByteArray& toDecrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Decryption error."));
    QByteArray output;

    AES_KEY* aesKey = new AES_KEY;

    if (AES_set_decrypt_key(reinterpret_cast<uchar*>(password.data()), 256, aesKey)) {
        delete aesKey;
        aesKey = NULL;
        throw errorMessage;
    }

    uchar* plaintext = new uchar[16];
    memset(plaintext, '\0', 16);
    AES_ecb_encrypt(reinterpret_cast<uchar*>(toDecrypt.data()), plaintext, aesKey, AES_DECRYPT);
    delete aesKey;
    aesKey = NULL;

    output = QByteArray(reinterpret_cast<const char*>(plaintext), 16);
    memset(plaintext, '\0', 16);

    delete[] plaintext;
    plaintext = NULL;
    toDecrypt.clear();
    password.clear();

    return output;
}
QByteArray decryptAesCbc(QByteArray& iv, QByteArray& toDecrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Decryption error."));
    QByteArray output;

    AES_KEY* aesKey = new AES_KEY;

    if (AES_set_decrypt_key(reinterpret_cast<uchar*>(password.data()), 256, aesKey)) {
        delete aesKey;
        aesKey = NULL;
        throw errorMessage;
    }

    size_t dataSize = ((toDecrypt.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uchar* plaintext = new uchar[dataSize];
    memset(plaintext, '\0', dataSize);
    AES_cbc_encrypt(reinterpret_cast<uchar*>(toDecrypt.data()), plaintext, dataSize, aesKey,
                    reinterpret_cast<uchar*>(iv.data()), AES_DECRYPT);
    delete aesKey;
    aesKey = NULL;

    output = QByteArray(reinterpret_cast<const char*>(plaintext), dataSize);
    memset(plaintext, '\0', dataSize);

    delete[] plaintext;
    plaintext = NULL;
    iv.clear();
    toDecrypt.clear();
    password.clear();
    dataSize = 0;

    return output;
}
QByteArray decryptAesGcm(QByteArray& iv, QByteArray& aad, QByteArray& toDecrypt, QByteArray& password) {
    QString errorMessage = QString(QObject::tr("Decryption error."));
    QByteArray output;

    QByteArray tag = toDecrypt.mid(0, 16);
    QByteArray ciphertext = toDecrypt.mid(16, toDecrypt.length() - 16);

    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    int ret;

    /* Create and initialise the context */
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        tag.clear();
        ciphertext.clear();
        throw errorMessage;
    }

    /* Initialise the decryption operation. */
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        tag.clear();
        ciphertext.clear();
        throw errorMessage;
    }

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL)) {
        tag.clear();
        ciphertext.clear();
        throw errorMessage;
    }

    /* Initialise key and IV */
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, reinterpret_cast<uchar*>(password.data()), reinterpret_cast<uchar*>(iv.data()))) {
        tag.clear();
        ciphertext.clear();
        throw errorMessage;
    }

    /* Provide any AAD data. This can be called zero or more times as
     * required
     */
    if (!EVP_DecryptUpdate(ctx, NULL, &len, reinterpret_cast<uchar*>(aad.data()), aad.length())) {
        tag.clear();
        ciphertext.clear();
        throw errorMessage;
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    size_t dataSize = ((toDecrypt.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uchar* plaintext = new uchar[dataSize];
    memset(plaintext, '\0', dataSize);

    if (!EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<uchar*>(ciphertext.data()), ciphertext.length())) {
        tag.clear();
        ciphertext.clear();
        memset(plaintext, '\0', dataSize);
        delete[] plaintext;
        plaintext = NULL;
        throw errorMessage;
    }

    plaintext_len = len;

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, reinterpret_cast<uchar*>(tag.data()))) {
        tag.clear();
        ciphertext.clear();
        memset(plaintext, '\0', dataSize);
        delete[] plaintext;
        plaintext = NULL;
        throw errorMessage;
    }

    /* Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if (ret <= 0) {
        tag.clear();
        ciphertext.clear();
        memset(plaintext, '\0', dataSize);
        delete[] plaintext;
        plaintext = NULL;
        throw errorMessage;
    }

    output = QByteArray(reinterpret_cast<const char*>(plaintext), plaintext_len);
    memset(plaintext, '\0', dataSize);
    delete[] plaintext;
    plaintext = NULL;

    tag.clear();
    ciphertext.clear();
    iv.clear();
    toDecrypt.clear();
    password.clear();

    return output;
}
QByteArray calculateXOR(QByteArray& data, QByteArray& key) {
    QByteArray xored;

    for (int i = 0, j = 0; i < data.length(); i++, j++) {
        if (j == key.length()) {
            j = 0;
        }

        xored.append(data.at(i)^key.at(j));
    }

    return xored;
}

// Other
QString bit7ToBit8(QString input) {
    while (input.indexOf("&") != -1) {
        if (input.indexOf("-") < input.indexOf("&")) {
            input = input.mid(0, input.indexOf("-")) + "##45" + input.mid(input.indexOf("-") + 1, input.length() - 1);
        } else {
            QString Cl = input.mid(0, input.indexOf("&"));
            QString Fo = input.mid(input.indexOf("-") + 1, input.length() - input.indexOf("-") - 1);
            QByteArray Array = QByteArray::fromBase64(input.mid(input.indexOf("&") + 1, input.indexOf("-") - input.indexOf("&") - 1).toUtf8());
            QString Out;

            while (!Array.isEmpty()) {
                uint utf16 = ((unsigned char)Array[0] << 8) + (unsigned char)Array[1];
                Array.remove(0, 2);
                uint unicode = 0;

                if (utf16 >= 0xD800 && utf16 <= 0xDFFF) {
                    unicode = ((utf16 - 0xD800) << 10) + (((unsigned char)Array[0] << 8) + (unsigned char)Array[1] - 0xDC00) + 0x10000;
                    Array.remove(0, 2);
                } else {
                    unicode = utf16;
                }

                Out.append(QChar(unicode));
            }

            input = Cl + Out + Fo;
        }
    }

    while (input.indexOf("##45") != -1) {
        input = input.mid(0, input.indexOf("##45")) + "-" + input.mid(input.indexOf("##45") + 4, input.length() - input.indexOf("##45") - 4);
    }

    return input;
}
QString bit8ToBit7(QString input) {
    QString Output;
    QByteArray String = input.toUtf8();

    for (int i = 0; i < String.length(); i++) {
        if ((uchar)String[i] > 0x7F) {
            QByteArray Str;

            while (true) {
                uint unicode = 0;
                uchar A = String[i];
                uchar B = String[i + 1];

                if (A < 0xE0) {
                    unicode = ((A - 192) << 6) + (B - 128);
                    i += 1;
                } else if (A < 0xF0 && A >= 0xE0) {
                    uchar C = String[i + 2];
                    unicode = ((A - 224) << 12) + ((B - 128) << 6) + (C - 128);
                    i += 2;
                } else if (A > 0xF0) {
                    uchar C = String[i + 2];
                    uchar D = String[i + 3];
                    unicode = ((A - 240) << 18) + ((B - 128) << 12) + ((C - 128) << 6) + (D - 128);
                    i += 3;
                }

                if (unicode < 0x10000) {
                    uchar C1 = (unicode >> 8);
                    uchar C2 = unicode % 256;
                    Str.append(C1);
                    Str.append(C2);
                } else {
                    unicode -= 0x10000;
                    uint W1 = (unicode >> 10) + 0xD800;
                    uint W2 = (unicode % 1024) + 0xDC00;
                    uchar C1 = (W1 >> 8);
                    uchar C2 = W1 % 256;
                    uchar C3 = (W2 >> 8);
                    uchar C4 = W2 % 256;
                    Str.append(C1);
                    Str.append(C2);
                    Str.append(C3);
                    Str.append(C4);
                }

                if (i + 1 < String.length()) {
                    if ((uchar)String[i + 1] > 0x7F) {
                        i += 1;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            Output.append(QString("&" + Str.toBase64() + "-").remove("="));
        } else {
            Output.append(static_cast<char>(String[i]));
        }
    }

    return Output;
}
QString quotedPrintableEncode(QByteArray input) {
    QString HEX = "0123456789ABCDEF";
    QString Output;

    for (int i = 0; i < input.length(); i++) {
        if (input[i].operator  >= (static_cast<char>(32))
                && input[i].operator  <= (static_cast<char>(126))) {
            Output += input[i];
        } else {
            Output.append('=');
            Output.append(HEX.at(((input[i] >>  4) & 0x0F)));
            Output.append(HEX.at(input[i] & 0x0F));
        }
    }

    return Output;
}
QByteArray quotedPrintableDecode(QByteArray input) {
    input.replace("=\r\n", "");
    QString HEX = "0123456789ABCDEF";
    QByteArray BA;

    for (int i = 0; i < input.length(); i++) {
        if (input[i] == '=') {
            int A = HEX.indexOf(input.at(i + 1), 0, Qt::CaseInsensitive);
            int B = HEX.indexOf(input.at(i + 2), 0, Qt::CaseInsensitive);
            BA.append(static_cast<char>(A * 16 + B));
            i += 2;
        } else {
            BA.append(input[i]);
        }
    }

    return BA;
}
void colorItem(QTreeWidgetItem* Itm, char P) {
    QColor color = QGuiApplication::palette().color(QPalette::Window);
    int r = color.red();
    int g = color.green();
    int b = color.blue();

    switch (P) {
    case 5: {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor((r + 0xFF) / 2, g / 2, b / 2, 162));
        }
    }
    break;

    case 4: {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor((r + 0xFF) / 2, (g + 0xB7) / 2, b / 2, 162));
        }
    }
    break;

    case 2: {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor((r + 0x70) / 2, g / 2, (b + 0x70) / 2, 162));
        }
    }
    break;

    case 1: {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor(r / 2, g / 2, (b + 0xFF) / 2, 162));
        }
    }
    break;

    case 0: {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor(r / 2, (g + 0xFF) / 2, b / 2, 162));
        }
    }
    break;
    }
}
QString nameFilter(QString input) {
    input.replace("?= =?", "?==?");

    while (input.contains("=?")) {
        int St = input.indexOf("=?");
        int Se = input.indexOf("?", St + 2);
        int Th = input.indexOf("?", Se + 1);
        int La = input.indexOf("?=", Th + 1);
        QString Charset = input.mid(St + 2, Se - St - 2);
        QString Type = input.mid(Se + 1, Th - Se - 1);
        QString Data = input.mid(Th + 1, La - Th - 1);
        Data.replace("_", " ");

        if (Type.toUpper() == "B") {
            if (Charset.toUpper() != "UTF-8") {
                QList <QByteArray> availableCodecs = QTextCodec::availableCodecs();
                QTextCodec* C = NULL;

                for (int i = 0; i < availableCodecs.size(); i++) {
                    QString codecName = QString(availableCodecs[i]);

                    if (codecName.contains(Charset, Qt::CaseInsensitive)) {
                        C = QTextCodec::codecForName(availableCodecs[i]);
                        break;
                    }
                }

                if (C != NULL) {
                    Data = C->toUnicode(QByteArray::fromBase64(Data.toUtf8()));
                }

            } else {
                Data = QString(QByteArray::fromBase64(Data.toUtf8()));
            }
        } else if (Type.toUpper() == "Q") {
            if (Charset.toUpper() != "UTF-8") {
                QList <QByteArray> availableCodecs = QTextCodec::availableCodecs();
                QTextCodec* C = NULL;

                for (int i = 0; i < availableCodecs.size(); i++) {
                    QString codecName = QString(availableCodecs[i]);

                    if (codecName.contains(Charset, Qt::CaseInsensitive)) {
                        C = QTextCodec::codecForName(availableCodecs[i]);
                        break;
                    }
                }

                if (C != NULL) {
                    Data = C->toUnicode(quotedPrintableDecode(Data.toUtf8()));
                }
            } else {
                Data = quotedPrintableDecode(Data.toUtf8());
            }
        }

        input = input.mid(0, St) + Data + input.mid(La + 2, input.length() - La - 2);
    }

    return input;
}
QString htmlCleaner(QString input) {
    QByteArray Output;
    input.replace("<", "&lt;");
    input.replace(">", "&gt;");
    input.replace("&amp;", "&");
    QTextDocument D;
    D.setHtml(input);
    input = D.toPlainText();

    for (int i = 0; i < input.length() - 4; i++) {
        if (input[i] == '&' && input[i + 1] == '#' && input[i + 4] == ';') {
            QChar C(input.mid(i + 2, 2).toInt());
            Output.append(C);
            i += 4;
        } else {
            Output.append(input.at(i));
        }
    }

    Output.append(input.mid(input.length() - 4, 4).toUtf8());
    return QString(Output);
}
QList <QString> split(QString input) {
    QList <QString> IL;
    int L = 0;
    bool Q = false;
    QString Data;

    for (int i = 0; i < input.length(); i++) {
        if (input[i] == ' '
                && L == 0
                && !Q) {
            IL.append(Data);
            Data.clear();
        } else {
            if (input[i] == '(') {
                L++;
            } else if (input[i] == ')') {
                L--;
            }

            if (input[i] == '"'
                    && !Q) {
                Q = !Q;
            } else if (input[i] == '"'
                       && Q) {
                Q = !Q;
            }

            Data.append(input[i]);
        }
    }

    IL.append(Data);
    return IL;
}
QPair <QByteArray, QByteArray> createNewPair(QByteArray& data) {
    QPair <QByteArray, QByteArray> output;

    uchar* hash = new uchar[1024];
    memset(hash, '\0', 1024);

    if (!RAND_bytes(hash, 1024)) {
        data.clear();
        throw QString(QObject::tr("Cannot randomize seed."));
    }


    output.first = QByteArray(reinterpret_cast<const char*>(hash), 1024);

    output.first = QCryptographicHash::hash(
                       QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), output.first).toByteArray(), QCryptographicHash::Sha3_512)
                       + output.first
                       + QCryptographicHash::hash(output.first, QCryptographicHash::Sha3_512)
                       , QCryptographicHash::Sha3_256);

    qDebug() << "First:" << output.first.toBase64();


    QByteArray salt = QCryptographicHash::hash(
                          QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), data).toByteArray(), QCryptographicHash::Sha3_512)
                          + data
                          + QCryptographicHash::hash(data, QCryptographicHash::Sha3_512)
                          , QCryptographicHash::Sha3_256);

    qDebug() << "Salt:" << salt.toBase64() << salt.size();

    uchar* key = new uchar[32];
    memset(key, '\0', 32);

    if (!PKCS5_PBKDF2_HMAC(data.data(), data.length(), reinterpret_cast<const unsigned char*>(salt.data()), salt.size(), 256000, EVP_sha256(), 32, key)) {
        data.clear();
        throw QString(QObject::tr("Cannot hash password"));
    }

    output.second = QByteArray(reinterpret_cast<const char*>(key), 32);

    output.second = QCryptographicHash::hash(
                        QCryptographicHash::hash(QUuid::createUuidV5(QUuid(), output.second).toByteArray(), QCryptographicHash::Sha3_512)
                        +  output.second
                        + QCryptographicHash::hash( output.second, QCryptographicHash::Sha3_512)
                        , QCryptographicHash::Sha3_256);
    qDebug() << "Second: " << output.second.toBase64();

    output.second = calculateXOR(output.second, output.first);

    qDebug() << "Key: " << output.second.toBase64() << "\n\n";

    data.clear();
    return output;
}

ItemPushButton::ItemPushButton(QIcon icon, QWidget* parent, uint IID) : QPushButton(icon, "", parent)    {
    itemID = IID;
    this->setStyleSheet("background: transparent; border: none;");
    connect(this, &ItemPushButton::clicked, this, &ItemPushButton::Emits);
}
void ItemPushButton::Emits() {
    emit clicked(itemID);
}

ValidatingLineEdit::ValidatingLineEdit(QString regexp, QString reason, QWidget* parent, bool fading) : QLineEdit(parent) {
    this->fading = fading;
    this->height = 30;

    timer.setSingleShot(true);
    timer.setInterval(60 * 1000);

    this->reason = reason;

    icon = new QLabel(this);
    icon->setStyleSheet("background: transparent; border: none;");

    validator = new QRegExpValidator(QRegExp(regexp), this);
    this->setValidator(validator);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(2);
    mainLayout->addWidget(icon, 0, Qt::AlignRight);

    connect(&timer, &QTimer::timeout, this, &ValidatingLineEdit::deactivate);
    connect(this, &ValidatingLineEdit::textEdited, this, &ValidatingLineEdit::validateText);
}
void ValidatingLineEdit::resizeEvent(QResizeEvent* event) {

    if (height != event->size().height()) {
        pixmap = QPixmap(":/main/Warning.png").scaled(event->size().height() - 4 , event->size().height() - 4, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int left, top, bottom;
        this->getTextMargins (&left, &top, NULL, &bottom);
        this->setTextMargins(left, top, event->size().height() - 2, bottom);
        height = event->size().height();
    }

    QLineEdit::resizeEvent(event);
}
void ValidatingLineEdit::activate(QString reason, bool fading) {
    if (fading) {
        timer.start();
    }

    icon->setPixmap(pixmap);
    icon->setToolTip(reason);
}
void ValidatingLineEdit::validateText() {
    if (this->hasAcceptableInput()) {
        deactivate();
    } else {
        activate(reason, fading);
    }
}
void ValidatingLineEdit::deactivate() {
    timer.stop();
    icon->clear();
    icon->setToolTip("");
}

CertAccept::CertAccept(QSslCertificate cert) {
    setWindowTitle("SSL certificate dialog.");
    setWindowIcon(QIcon(":/main/QOrganizer.png"));
    QLabel* La = new QLabel("Do you trust this certificate?", this);
    QTextBrowser* Text = new QTextBrowser(this);
    Text->setText(cert.toText());
    QPushButton* No = new QPushButton("Blacklist", this);
    No->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(No, &QPushButton::clicked, this, &CertAccept::reject);
    QPushButton* Yes = new QPushButton("Accept", this);
    Yes->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    connect(Yes, &QPushButton::clicked, this, &CertAccept::accept);
    QGridLayout* L = new QGridLayout(this);
    L->addWidget(La, 0, 0, 1, 2);
    L->addWidget(Text, 1, 0, 1, 2);
    L->addWidget(No, 2, 0);
    L->addWidget(Yes, 2, 1);
}
