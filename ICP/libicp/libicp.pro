#-------------------------------------------------
#
# Project created by QtCreator 2011-08-14T01:07:08
#
#-------------------------------------------------

QT       -= gui

TARGET = icp
TEMPLATE = lib
CONFIG += staticlib

SOURCES += libicp.cpp \
    ../lib/point.cpp \
    ../lib/kdnode.cpp \
    ../lib/icpmatcher.cpp

HEADERS += libicp.h \
    ../lib/point.h \
    ../lib/kdnode.h \
    ../lib/icpmatcher.h \
    ../lib/debug.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    ../lib/Makefile
