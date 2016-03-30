#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T12:59:56
#
#-------------------------------------------------

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QOrganizer
TEMPLATE = app

SOURCES += main.cpp\
    qorgtools.cpp \
    qorgio.cpp \
    qorganizer.cpp \
    qorgmodel.cpp \
    qorgcalendarmodel.cpp \
    qorgnotesmodel.cpp \
    qorgaddressbookmodel.cpp \
    qorgpasswordmanagermodel.cpp \
    qorgoptionsmodel.cpp \
    qorgfeedsreadermodel.cpp \
    #qorgcalendar.cpp \ // Changes
    #qorgmailmodel.cpp \
    #qorgmail.cpp \ // Changes
    #qorgnotes.cpp \ // Changes
    #qorgab.cpp \
    #qorgrss.cpp \
    #qorgpasswd.cpp \
    #qorgoptions.cpp \
    #qorgfeedreader.cpp \
    #qorgpasswordmanager.cpp \



HEADERS  += \
    qorgtools.h \
    qorgio.h \
    qorganizer.h \
    qorgmodel.h \
    qorgcalendarmodel.h \
    qorgnotesmodel.h \
    qorgaddressbookmodel.h \
    qorgpasswordmanagermodel.h \
    qorgoptionsmodel.h \
    qorgfeedsreadermodel.h \
    #qorgcalendar.h \ // Changes
    #qorgmailmodel.h \
    #qorgmail.h \ // Changes
    #qorgnotes.h \ // Changes
    #qorgab.h \
    #qorgrss.h \
    #qorgpasswd.h \
    #qorgoptions.h \
    #qorgpasswordmanager.h \
    #qorgfeedreader.h \
    #qorgaddressbook.h \

LIBS += -lcrypto

RESOURCES += \
    resources.qrc

DISTFILES += \
    LICENCE.md \
    latestVersion.md \
    README.md \
    changelog.md

CONFIG += c++11
