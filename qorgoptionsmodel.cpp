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

#include "qorgoptionsmodel.h"

qorgOptionsModel::qorgOptionsModel(QObject* parent) : QObject(parent) {
}
qorgOptionsModel::~qorgOptionsModel() {
    clear();
}

// IO functions
void qorgOptionsModel::input(QString input, uint version) {
    if (!input.isEmpty()) {
        QStringList lines = input.split("\n");

        for (int i = 0; i < lines.size(); i++) {
            QStringList fields = lines[i].split(" ");
            int control = (version < 105 ? fields.size() - 1 : fields.size());

            if (control > 0) {
                switch (control) {
                    case 1: {
                        if (version < 105) {
                            acceptedSslCertificates.push_back(QSslCertificate::fromData(inputString(fields[0]).toUtf8()).first());
                        } else {
                            QString rawData = inputString(fields[0]);
                            QStringList ciphers = rawData.split(";");

                            if (ciphers.size() == 2) {
                                QList <QString> acceptedCiphersStringList = ciphers[0].split(",", QString::SkipEmptyParts);
                                QList <QString> blacklistedCiphersStringList = ciphers[1].split(",", QString::SkipEmptyParts);

                                for (int j = 0; j < acceptedCiphersStringList.size(); j++) {
                                    acceptedSslCiphers.append(QSslCipher(acceptedCiphersStringList[j]));
                                }

                                for (int j = 0; j < blacklistedCiphersStringList.size(); j++) {
                                    blacklistedSslCiphers.append(QSslCipher(blacklistedCiphersStringList[j]));
                                }

                                sslConfiguration.setCiphers(acceptedSslCiphers);
                                QSslConfiguration::setDefaultConfiguration(sslConfiguration);

                            }
                        }
                    }
                    break;

                    case 2: {
                        updateTime = inputUInt(fields[0]);
                        blockTime = inputUInt(fields[1]);
                    }
                    break;

                    case 3: {
                        if (fields[1] == "A") {
                            acceptedSslCertificates.push_back(QSslCertificate::fromData(inputString(fields[0]).toUtf8()).first());
                        } else {
                            blacklistedSslCertificates.push_back(QSslCertificate::fromData(inputString(fields[0]).toUtf8()).first());
                        }
                    }
                    break;

                    case 4: {
                        virusTotalApiKey = inputString(fields[0]);
                    }
                    break;

                    case 5: {
                        QNetworkProxy proxy;

                        if (inputBool(fields[0])) {
                            proxy.setType(QNetworkProxy::Socks5Proxy);
                        } else {
                            proxy.setType(QNetworkProxy::HttpProxy);
                        }

                        proxy.setHostName(inputString(fields[1]));
                        proxy.setPort(inputUInt(fields[2]));
                        proxy.setUser(inputString(fields[3]));
                        proxy.setPassword(inputString(fields[4]));
                        setProxy(proxy);
                    };

                    break;
                }
            }
        }
    }
}
QString qorgOptionsModel::output() {
    /*QString Out;
    Out.append(outputFilter(updateInterval) + " " + outputFilter(blockInterval) + "\n");
    QString ciphersString;
    for (int i = 0; i < acceptedCiphers->count(); i++) {
        ciphersString.append(acceptedCiphers->item(i)->text() + ",");
    }
    ciphersString.append(";");
    for (int i = 0; i < blacklistedCiphers->count(); i++) {
        ciphersString.append(blacklistedCiphers->item(i)->text() + ",");
    }
    Out.append(outputFilter(ciphersString) + "\n");
    for (uint i = 0; i < sslCertsAccepted.size(); i++) {
        Out.append(outputFilter(QString(sslCertsAccepted[i].toPem()))+" A C\n");
    }
    for (uint i = 0; i < sslCertsBlacklisted.size(); i++) {
        Out.append(outputFilter(QString(sslCertsBlacklisted[i].toPem()))+" B C\n");
    }
    if (!QNetworkProxy::applicationProxy().hostName().isEmpty()) {
        QNetworkProxy tmp = QNetworkProxy::applicationProxy();
        Out.append(outputFilter(tmp.type() == QNetworkProxy::Socks5Proxy)+" "+outputFilter(tmp.hostName())+" "+outputFilter(static_cast<uint>(tmp.port()))
                   +" "+outputFilter(tmp.user())+" "+outputFilter(tmp.password())+"\n");
    }
    if (!apiKey.isEmpty()) {
        Out.append(outputFilter(apiKey)+" A P I\n");
    }
    Out.append("\n\n");
    return output;*/
}

// Functions to change model by controller
void qorgOptionsModel::addAcceptedSslCertificate(QSslCertificate certificate) {
    bool check = (acceptedSslCertificates.end() != std::find(acceptedSslCertificates.begin(), acceptedSslCertificates.end(), certificate));

    if (!check) {
        acceptedSslCertificates.push_back(certificate);
    }
}
void qorgOptionsModel::addBlacklistedSslCertificate(QSslCertificate certificate) {
    bool check = (blacklistedSslCertificates.end() != std::find(blacklistedSslCertificates.begin(), blacklistedSslCertificates.end(), certificate));

    if (!check) {
        blacklistedSslCertificates.push_back(certificate);
    }
}
void qorgOptionsModel::addAcceptedSslCipher(QSslCipher cipher) {
    bool check = (acceptedSslCiphers.end() != std::find(acceptedSslCiphers.begin(), acceptedSslCiphers.end(), cipher));
    bool lock = (blacklistedSslCiphers.end() != std::find(blacklistedSslCiphers.begin(), blacklistedSslCiphers.end(), cipher));

    if (!check && lock) {
        acceptedSslCiphers.push_back(cipher);
        sslConfiguration.setCiphers(acceptedSslCiphers);
        QSslConfiguration::setDefaultConfiguration(sslConfiguration);
    }
}
void qorgOptionsModel::addBlacklistedSslCipher(QSslCipher cipher) {
    bool check = (blacklistedSslCiphers.end() != std::find(blacklistedSslCiphers.begin(), blacklistedSslCiphers.end(), cipher));

    if (!check) {
        blacklistedSslCiphers.push_back(cipher);
    }
}
void qorgOptionsModel::setVirusTotalApiKey(QString apiKey) {
    virusTotalApiKey = apiKey;
}
void qorgOptionsModel::setProxy(QNetworkProxy proxy) {
    currentProxy = proxy;
    QNetworkProxy::setApplicationProxy(currentProxy);
}
void qorgOptionsModel::setTimes(uint update, uint block) {
    updateTime = update;
    blockTime = block;
}
void qorgOptionsModel::clear() {
    acceptedSslCertificates.clear();
    acceptedSslCiphers.clear();
    blacklistedSslCiphers.clear();
    virusTotalApiKey.clear();
    currentProxy = QNetworkProxy();
    updateTime = 30;
    blockTime = 30;
    sslConfiguration = QSslConfiguration::defaultConfiguration();
}

// Functions to send data to view
int qorgOptionsModel::checkCertificate(QSslCertificate certificate) {
    int output = 0;

    bool accepted = (acceptedSslCertificates.end() != std::find(acceptedSslCertificates.begin(), acceptedSslCertificates.end(), certificate));
    bool blacklisted = (blacklistedSslCertificates.end() != std::find(blacklistedSslCertificates.begin(), blacklistedSslCertificates.end(), certificate));

    if (accepted && !blacklisted) {
        output = 1;
    } else if (blacklisted && !accepted) {
        output = -1;
    }

    return output;
}
QString qorgOptionsModel::getVirusTotalApiKey() {
    return virusTotalApiKey;
}
