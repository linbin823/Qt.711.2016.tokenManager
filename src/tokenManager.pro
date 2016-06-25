#-------------------------------------------------
#
# Project created by QtCreator 2016-06-21T14:20:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tokenManager
TEMPLATE = app


SOURCES += main.cpp\
    sender.cpp \
    receiver.cpp \
    tokenmanager.cpp

HEADERS  += \
    sender.h \
    receiver.h \
    tokenmanager.h

FORMS    +=

DISTFILES += \
    ..\doc\Plan \
    ../HISTORY.txt \
    ../README.txt
