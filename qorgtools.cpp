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

#include <qorgtools.h>

QByteArray calculateXOR(QByteArray A, QByteArray B) {
    QByteArray xored;
    for (int i = 0, j = 0; i < A.length(); i++, j++) {
        if (j == B.length()) {
            j = 0;
        }
        xored.append(A.at(i)^B.at(j));
    }
    return xored;
}
QString Bit7ToBit8(QString I) {
    while (I.indexOf("&") != -1) {
        if (I.indexOf("-") < I.indexOf("&")) {
            I = I.mid(0, I.indexOf("-"))+"##45"+I.mid(I.indexOf("-")+1, I.length()-1);
        } else {
            QString Cl = I.mid(0, I.indexOf("&"));
            QString Fo = I.mid(I.indexOf("-")+1, I.length()-I.indexOf("-")-1);
            QByteArray Array = QByteArray::fromBase64(I.mid(I.indexOf("&")+1, I.indexOf("-")-I.indexOf("&")-1).toUtf8());
            QString Out;
            while (!Array.isEmpty()) {
                uint utf16=(unsigned char)Array[0]*256+(unsigned char)Array[1];
                Array.remove(0, 2);
                uint unicode = 0;
                if (utf16 >= 0xD800&&utf16 <= 0xDFFF) {
                    unicode=(utf16-0xD800)*1024+((unsigned char)Array[0]*256+(unsigned char)Array[1]-0xDC00)+0x10000;
                    Array.remove(0, 2);
                } else {
                    unicode = utf16;
                }
                if (unicode < 0x80) {
                    char A = unicode%128;
                    Out+=A;
                } else if (unicode > 0x7F&&unicode < 0x800) {
                    char A = 128+unicode%64;
                    char B = 192+((unicode << 21) >> 27);
                    QByteArray H;
                    H.append(B);
                    H.append(A);
                    Out+=H;
                } else if (unicode > 0x7FF&&unicode < 0x100000) {
                    char A = 128+unicode%64;
                    char B = 128+((unicode << 20) >> 26);
                    char C = 224+((unicode << 16) >> 28);
                    QByteArray H;
                    H.append(C);
                    H.append(B);
                    H.append(A);
                    Out+=H;
                } else {
                    char A = 128+unicode%64;
                    char B = 128+((unicode << 20) >> 26);
                    char C = 128+((unicode << 14) >> 26);
                    char D = 240+((unicode << 11) >> 29);
                    QByteArray H;
                    H.append(D);
                    H.append(C);
                    H.append(B);
                    H.append(A);
                    Out+=H;
                }
            }
            I = Cl+Out+Fo;
        }
    }
    while (I.indexOf("##45") != -1) {
        I = I.mid(0, I.indexOf("##45"))+"-"+I.mid(I.indexOf("##45")+4, I.length()-I.indexOf("##45")-4);
    }
    return I;
}
QString QPEncode(QByteArray I) {
    QString HEX="0123456789ABCDEF";
    QString Output;
    for (int i = 0; i < I.length(); i++) {
        if (I[i].operator  >= (static_cast<char>(32))&&I[i].operator  <= (static_cast<char>(126))) {
            Output+=I[i];
        } else {
            Output.append('=');
            Output.append(HEX.at(((I[i] >>  4) & 0x0F)));
            Output.append(HEX.at(I[i] & 0x0F));
        }
    }
    return Output;
}
QByteArray QPDecode(QByteArray I) {
    I.replace("=\r\n", "");
    QString HEX="0123456789ABCDEF";
    QByteArray BA;
    for (int i = 0; i < I.length(); i++) {
        if (I[i] == '=') {
            int A = HEX.indexOf(I.at(i+1));
            int B = HEX.indexOf(I.at(i+2));
            BA.append(static_cast<char>(A*16+B));
            i+=2;
        } else {
            BA.append(I[i]);
        }
    }
    return BA;
}
QString OutputTools(QString I, QString TAG) {
    return "<"+TAG+">"+I.toUtf8().toBase64()+"</"+TAG+">\n";
}
QString OutputToolsS(QString I, QString TAG) {
    return "<"+TAG+">"+I+"</"+TAG+">\n";
}
QString OutputTools(int I, QString TAG) {
    return "<"+TAG+">"+QString::number(I)+"</"+TAG+">\n";
}
QString OutputTools(bool I, QString TAG) {
    if (I) {
        return "<"+TAG+">1</"+TAG+">\n";
    }
    return "<"+TAG+">0</"+TAG+">\n";
}
QString InputS(QString IN, QString TAG) {
    return QString(QByteArray::fromBase64(
                       IN.mid(IN.indexOf("<"+TAG+">")+TAG.length()+2,
                              IN.indexOf("</"+TAG+">")-IN.indexOf("<"+TAG+">")-TAG.length()-2).toUtf8()));
}
QString InputSS(QString IN, QString TAG) {
    return IN.mid(IN.indexOf("<"+TAG+">")+TAG.length()+2,
                  IN.indexOf("</"+TAG+">")-IN.indexOf("<"+TAG+">")-TAG.length()-2);
}
int InputI(QString IN, QString TAG) {
    return IN.mid(IN.indexOf("<"+TAG+">")+TAG.length()+2,
                  IN.indexOf("</"+TAG+">")-IN.indexOf("<"+TAG+">")
                  -TAG.length()-2).toInt();
}
bool InputB(QString IN, QString TAG) {
    return (IN.mid(IN.indexOf("<"+TAG+">")+TAG.length()+2,
                   IN.indexOf("</"+TAG+">")-IN.indexOf("<"+TAG+">")
                   -TAG.length()-2) == "1");
}
void colorItem(QTreeWidgetItem *Itm, char P) {
    switch (P) {
    case 5:
    {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor("#FF8888"));
        }
    }break;
    case 4:
    {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor("#FFDA88"));
        }
    }break;
    case 2:
    {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor("#B888B8"));
        }
    }break;
    case 1:
    {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor("#8888FF"));
        }
    }break;
    case 0:
    {
        for (int i = 0; i < Itm->columnCount(); i++) {
            Itm->setBackgroundColor(i, QColor("#88FF88"));
        }
    }break;
    }
}
QString salting(QString A) {
    QString output;
    int salt = 0;
    for (int i = 0; i < A.length(); i++) {
        salt+=static_cast<int>(A[i].toLatin1());
    }
    switch (salt%5) {
    case 0:
    {
        output="OC2mwEEOYlPoj7lwyJpr";
    }break;
    case 1:
    {
        output="PNzXx09G93Ll3cgmfGXu";
    }break;
    case 2:
    {
        output="QBJfDzsanikedIfvpdp0";
    }break;
    case 3:
    {
        output="4yUSTcpLDFIm8SVT2vx9";
    }break;
    case 4:
    {
        output="oU9CBiL1dKHpaAWRFDAC";
    }break;
    }
    output.append(A);
    switch (salt%4) {
    case 0:
    {
        output+="3WLtmogKQqdYChOGArYT";
    }break;
    case 1:
    {
        output+="3WLtmogKQqdYChOGArYT";
    }break;
    case 2:
    {
        output+="OCvxh3gJviSdQbQ81frA";
    }break;
    case 3:
    {
        output+="zU4AlSJIIks3MhyzUJv5";
    }break;
    }
    return output;
}
