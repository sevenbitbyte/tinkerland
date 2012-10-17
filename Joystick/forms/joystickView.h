#ifndef JOYSTICK_VIEW_H
#define JOYSTICK_VIEW_H

#include "ui_joystick.h"
#include "../QJoystick.h"

class JoystickView : public QWidget
{
    Q_OBJECT
    public:
        JoystickView(QString joystickFile, QWidget* parent=NULL);
        ~JoystickView();

    private:
        void initForm();

    private slots:
        void updateAxis(quint8 axis, qint32 value);
        void updateButton(quint8 button, bool value);

    private:
        Ui::JoystickView joyView;
        QJoystick* joystick;

};

#endif
