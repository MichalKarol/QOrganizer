#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T12:59:56
#
#-------------------------------------------------

QT       += core gui network webkitwidgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QOrganizer
TEMPLATE = app


SOURCES += main.cpp\
    qorgio.cpp \
    qorgcalendar.cpp \
    qorgmail.cpp \
    qorgnotes.cpp \
    qorgab.cpp \
    qorgrss.cpp \
    qorgpasswd.cpp \
    qorglogin.cpp \
    qorganizer.cpp \
    qorgtools.cpp \
    qorgoptions.cpp

HEADERS  += \
    qorgio.h \
    qorgcalendar.h \
    qorgmail.h \
    qorgnotes.h \
    qorgab.h \
    qorgrss.h \
    qorgpasswd.h \
    qorglogin.h \
    qorganizer.h \
    qorgtools.h \
    qorgoptions.h

LIBS += -lcrypto
CONFIG += qt

RESOURCES += \
    resources.qrc

DISTFILES += \
    LICENCE.md \
    latestVersion.md \
    README.md
