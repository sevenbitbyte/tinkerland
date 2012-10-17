#include "QJoystick.h"
#include <QString>
#include <QtCore>

#include <iostream>
#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../Common/timedRecieve.h"

#define X_AXIS  0
#define Y_AXIS  1
#define DEFAULT_BUTTON_STATE FALSE
#define DEFAULT_AXIS_VALUE 0

using namespace std;

QJoystick::QJoystick(QString device):deviceFile(device){
    //Open device
    cout << device.toAscii().data() << endl;
    if((fd=open(device.toAscii().data(), O_RDONLY))==-1){
        perror("Joystick");
        good=false;
        return;
    }
    joyName=new char[256];
    if(ioctl(fd, JSIOCGNAME(256), joyName)==-1){
        good=false;
        return;
    }
    ioctl(fd, JSIOCGAXES, &numAxes);
    ioctl(fd, JSIOCGBUTTONS, &numButtons);
    ioctl(fd, JSIOCGVERSION, &driverVersion);
    cout << "Found device: "<<joyName<<endl;
    cout << "Axes: " << (int)numAxes << endl;
    cout << "Buttons: " << (int)numButtons << endl;
    cout << "Version: " << driverVersion << endl;
    good=true;
    start();
}

QJoystick::~QJoystick(){
    good=false;
    close(fd);
}

void QJoystick::run(){
    msleep(100);    //Allow time for signals to be connected
    //Emit default axis values
    for(int i=0; i<numAxes; i++){
        emit axisValue((quint8)i, DEFAULT_AXIS_VALUE);
    }

    //Emit default button states
    for(int i=0; i<numButtons; i++){
        emit buttonPressed((quint8)i, DEFAULT_BUTTON_STATE);
    }

    struct js_event eventData;
    int readSize=0;
    while(isGood()){
        if(timedRecieve(1, 0, fd)>0){
            //Read in upto 64 js_event's
            readSize=read(fd, &eventData, sizeof(struct js_event));
            if(readSize<sizeof(struct js_event)){
                perror("Read");
                continue;
            }
            switch(eventData.type){
                case JS_EVENT_INIT:
                    cout << "Init event" <<endl;
                    break;
                case JS_EVENT_AXIS:
                    emit axisValue(eventData.number, eventData.value);
                    break;
                case JS_EVENT_BUTTON:
                    //cout<<"Button:"<<(int)eventData.number;
                    //cout<<"="<<eventData.value<<endl;
                    emit buttonPressed(eventData.number,(bool)eventData.value);
                    break;
                default:
                    cout<<"Unknown type("<<(int)eventData.type<<"):";
                    cout<<(int)eventData.number;
                    cout<<"="<<eventData.value<<endl;
                    break;
            }
        }
        else{
            //cout << "Timedout" << endl;
        }
    }
    cout << "Stopping!" <<endl;
}

bool QJoystick::isGood(){
    return good;
}

quint32 QJoystick::getDriverVersion(){
    return driverVersion;
}

quint8 QJoystick::getNumButtons(){
    return numButtons;
}

quint8 QJoystick::getNumAxes(){
    return numAxes;
}

QString QJoystick::getName(){
    QString ret=joyName;
    return ret;
}

QString QJoystick::getDeviceLocation(){
    return deviceFile;
}

