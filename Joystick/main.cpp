#include <iostream>
#include <qapplication.h>
#include "forms/joystickView.h"

using namespace std;

int main( int argc, char ** argv )
{
    if(argc!=2){
        cout<<"Usage: "<<argv[0]<<" [device file]"<<endl;
        exit(1);
    }
    QString file(argv[1]);
    QApplication a( argc, argv );
    JoystickView joyView(file);
    joyView.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
