QT       += core network
include(..\common\common.pri)
include(..\loadSave\saveLoadProcessor.pri)

SOURCES += $$PWD/tmeasyapi.cpp \
    $$PWD/tmpeer.cpp \
    $$PWD/tmtokenmanager.cpp

HEADERS  += $$PWD/tmeasyapi.h \
    $$PWD/tmpeer.h \
    $$PWD/tmtokenmanager.h

INCLUDEPATH += $$PWD\

