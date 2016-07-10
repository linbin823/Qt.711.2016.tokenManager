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
    tokenmanager.cpp \
    example.cpp \
    tmeasyapi.cpp \
    sierrmsg.cpp

HEADERS  += \
    tokenmanager.h \
    example.h \
    tmeasyapi.h \
    sierrmsg.h \
    siloadsave.h

FORMS    += \
    example.ui

DISTFILES += \
    ..\doc\Plan \
    ../HISTORY.txt \
    ../README.txt
