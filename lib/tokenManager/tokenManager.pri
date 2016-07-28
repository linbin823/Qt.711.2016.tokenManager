QT       += core network
include(..\..\..\Qt.711.2016.common\lib\common\common.pri)
include(..\..\..\Qt.711.2016.saveLoadProcessor\lib\loadSave\saveLoadProcessor.pri)

SOURCES += $$PWD/tmeasyapi.cpp \
    $$PWD/tmpeer.cpp \
    $$PWD/tmtokenmanager.cpp

HEADERS  += $$PWD/tmeasyapi.h \
    $$PWD/tmpeer.h \
    $$PWD/tmtokenmanager.h

INCLUDEPATH += $$PWD\

DISTFILES += \
    ../../HISTORY.txt \
    ../../README.txt

DISTFILES += \
    $$PWD\../../HISTORY.txt \
    $$PWD\../../README.txt
