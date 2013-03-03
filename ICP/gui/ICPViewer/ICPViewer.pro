#-------------------------------------------------
#
# Project created by QtCreator 2011-08-13T19:12:36
#
#-------------------------------------------------

QT       += core gui

TARGET = ICPViewer
TEMPLATE = app


SOURCES += src/main.cpp\
		src/mainwindow.cpp \
    logparser.cpp

HEADERS  += src/mainwindow.h \
    logparser.h

INCLUDEPATH += ../../lib

FORMS    += forms/mainwindow.ui

LIBS += ../../libicp/libicp.a

RESOURCES += \
    resources.qrc
