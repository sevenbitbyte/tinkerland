# -------------------------------------------------
# Project created by QtCreator 2009-04-25T22:59:53
# -------------------------------------------------
TARGET = XBeeCC
TEMPLATE = app
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
CONFIG += qt \
    thread \
    warn_on
SOURCES += main.cpp \
    mainwindow.cpp \
    QXBee/QXBee.cpp \
    qextserialport/qextserialport.cpp \
    qextserialport/qextserialbase.cpp
HEADERS += mainwindow.h \
    QXBee/QXBeeConfig.h \
    QXBee/QXBee.h \
    qextserialport/qextserialport.h \
    qextserialport/qextserialbase.h \
    QXBee/QXBeeCommon.h
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
