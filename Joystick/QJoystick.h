#ifndef QJOYSTICK_CLASS_H
#define QJOYSTICK_CLASS_H
#include <linux/joystick.h>
#include <QThread>
#include <QtCore>
#include <QString>
#include <QWidget>


class QJoystick : public QThread{
    Q_OBJECT
    public:
        QJoystick(QString device);
        ~QJoystick();
        bool isGood();
        void run();

    public slots:
        quint32 getDriverVersion();
        quint8  getNumButtons();
        quint8  getNumAxes();
        QString getName();
        QString getDeviceLocation();

    signals:
        void axisValue(quint8 axis, qint32 value);
        void buttonPressed(quint8 button, bool state);

    private:
        int fd;
        bool good;
        quint8 numAxes;
        quint8 numButtons;
        quint32 driverVersion;
        char *joyName;
        QString deviceFile;
};

#endif 
