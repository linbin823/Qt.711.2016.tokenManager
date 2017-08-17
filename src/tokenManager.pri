QT       += core network
include($$PWD/../lib/loadSaveProcessor/src/loadsaveProcessor.pri)

SOURCES +=\
    $$PWD/tmpeer.cpp \
    $$PWD/tmtokenmanager.cpp \
    $$PWD/tmeasyapi.cpp

HEADERS  +=\
    $$PWD/tmpeer.h \
    $$PWD/tmtokenmanager.h \
    $$PWD/tmeasyapi.h

INCLUDEPATH += $$PWD\

DISTFILES += \
    $$PWD/../../HISTORY.txt \
    $$PWD/../../README.txt
