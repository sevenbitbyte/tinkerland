#include "joystickView.h"
#include "../QJoystick.h"

#define X_AXIS 0
#define Y_AXIS 1
#define WHEEL_AXIS 2

JoystickView::JoystickView(QString joystickFile, QWidget* parent)
:QWidget(parent)
{
    joyView.setupUi(this);
    joystick=new QJoystick(joystickFile);
    initForm();
    connect(joystick, SIGNAL(axisValue(quint8, qint32)),
            this, SLOT(updateAxis(quint8, qint32)));
    connect(joystick, SIGNAL(buttonPressed(quint8, bool)),
            this, SLOT(updateButton(quint8, bool)));
}

JoystickView::~JoystickView(){

}

void JoystickView::initForm(){
    QString driverStr, buttonStr, axisStr, nameStr, locationStr;
    driverStr.setNum(joystick->getDriverVersion());
    buttonStr.setNum(joystick->getNumButtons());
    axisStr.setNum(joystick->getNumAxes());
    nameStr=joystick->getName();
    locationStr=joystick->getDeviceLocation();

    for(int i=0; i<joystick->getNumButtons(); i++){
        joyView.buttonTable->insertRow(i);
    }

    joyView.driverVersionEdit->setText(driverStr);
    joyView.buttonCountEdit->setText(buttonStr);
    joyView.axisCountEdit->setText(axisStr);
    joyView.nameEdit->setText(nameStr);
    joyView.locationEdit->setText(locationStr);
}

void JoystickView::updateAxis(quint8 axis, qint32 value){
    QString newValue;
    newValue.setNum(value);

    switch(axis){
        case X_AXIS:
            joyView.xAxisEdit->setText(newValue);
            break;
        case Y_AXIS:
            joyView.yAxisEdit->setText(newValue);
            break;
        case WHEEL_AXIS:
            joyView.wheelAxisEdit->setText(newValue);
            break;
        default:
            break;
    }
}

void JoystickView::updateButton(quint8 button, bool value){
    QTableWidgetItem *item;
    if(value){
        item=new QTableWidgetItem(QString("True"));
    }
    else{
        item=new QTableWidgetItem(QString("False"));
    }
    joyView.buttonTable->setItem(button, 0, item);
}

