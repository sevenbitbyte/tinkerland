######################################################################
# Automatically generated by qmake (2.01a) Thu Jul 5 16:41:15 2007
######################################################################

TEMPLATE = app
TARGET = QJoystickView
DEPENDPATH += .
INCLUDEPATH += .

# Input
FORMS += forms/joystick.ui

HEADERS += QJoystick.h ../Common/timedRecieve.h forms/joystickView.h
SOURCES += QJoystick.cpp main.cpp forms/joystickView.cpp

RESOURCES += joystick.qrc

unix {
  UI_DIR = forms
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
