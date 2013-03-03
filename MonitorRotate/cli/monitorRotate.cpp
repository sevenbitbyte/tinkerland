#include <cc++/serial.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace ost;

enum Directions{DirectionNormal=0, DirectionLeft=1, DirectionInverted=2, DirectionRight=3};
enum PacketTypes{PktOrientation, PktAccValues, PktAccStatus, PktMagValues, PktMagStatus, PktDeviceStatus};

int main(int argc, char** argv){
    string port="/dev/ttyACM0";
    int baudRate=115200;
    if(argc==2){
        port=argv[1];
    }

    ttystream stream;
    stream.setFlowControl(Serial::flowNone);
    stream.setStopBits(1);
    stream.setParity(Serial::parityNone);
    stream.setCharBits(8);
    stream.setSpeed(baudRate);
    
    cout<<"Opening: "<<port.c_str()<<endl;
    stream.open(port.c_str());
    
    /*if(!stream.isOpen()){
        cout<<"Failed to open port: " << port << endl;
        return -1;
    }*/
    
    if(stream.isPending(Serial::pendingError, 1000)){
        cout<<"Pending error detected!"<<endl;
    }
    
    cout<<"Listening..."<<endl;
    
    while(1){
        
        if(stream.isPending(Serial::pendingInput, 100)){
            uint8_t code = 0x0;
            
            stream >> code;
            
            //cout<<code;
            
            /*string temp;
            stream >> temp;
            
            cout << "Data:" << temp << endl;*/
            
            
            switch(code){
                case DirectionNormal:
                    cout << "Normal" << endl;
                    system("xrandr -o 0");
                    break;
                case DirectionLeft:
                    cout << "Left" << endl;
                    system("xrandr -o 1");
                    break;
                case DirectionInverted:
                    cout << "Inverted" << endl;
                    system("xrandr -o 2");
                    break;
                case DirectionRight:
                    cout << "Right" << endl;
                    system("xrandr -o 3");
                    break;
            }
        }
        usleep(5000);
        //cout<<"Loop"<<endl;
    }
}
