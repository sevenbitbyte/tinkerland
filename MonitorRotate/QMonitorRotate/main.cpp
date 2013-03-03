#include <QApplication>
#include "mainwindow.h"

#include <signal.h>

void sighupMainHandler(int signal){
	//qDebug() << "INFO: Received signal " << signal;
}

int main(int argc, char *argv[])
{

	signal(SIGHUP, SIG_IGN);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();


	return a.exec();
}
