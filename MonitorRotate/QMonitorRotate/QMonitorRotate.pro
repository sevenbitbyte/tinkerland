#-------------------------------------------------
#
# Project created by QtCreator 2013-02-15T03:24:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QMonitorRotate
TEMPLATE = app
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
CONFIG += qt \
    thread \
    warn_on
SOURCES += main.cpp \
    mainwindow.cpp \
    qextserialport/qextserialport.cpp \
    qextserialport/qextserialbase.cpp \
    serialdevice.cpp
HEADERS += mainwindow.h \
    qextserialport/qextserialport.h \
    qextserialport/qextserialbase.h \
    serialdevice.h
FORMS += mainwindow.ui
unix:DEFINES = _TTY_POSIX_
win32:DEFINES = _TTY_WIN_ \
    QWT_DLL \
    QT_DLL
unix:HEADERS += qextserialport/posix_qextserialport.h
unix:SOURCES += qextserialport/posix_qextserialport.cpp
unix:DEFINES += _TTY_POSIX_
win32:HEADERS += qextserialport/win_qextserialport.h
win32:SOURCES += qextserialport/win_qextserialport.cpp
win32:DEFINES += _TTY_WIN_
