#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    qWarning("Current thread = 0x%x 0x%x - MainWindow()", QObject::thread(), QApplication::instance()->thread());
//QXBee* xbee0=new QXBee("/dev/ttyUSB0", BAUD9600, this);
    ui->setupUi(this);
    connect(ui->deviceEdit, SIGNAL(textChanged(QString)), SLOT(testSerialPort(QString)));
    connect(ui->addButton, SIGNAL(clicked()), SLOT(addXBee()));
}

MainWindow::~MainWindow()
{
    while(!xbeeList.empty()){
        xbeeList.last()->quit();
        xbeeList.last()->wait(100);
        delete xbeeList.last();
        xbeeList.pop_back();
    }
    delete ui;
}

void MainWindow::testSerialPort(QString port){
    QFileInfo serial(port);
    if(!serial.isDir() && serial.exists() && !portUsed(port)){
        ui->addButton->setEnabled(true);
    }
    else{
        ui->addButton->setEnabled(false);
    }
}

void MainWindow::addXBee(){
    QFileInfo serial(ui->deviceEdit->text());
    if(!serial.isDir() && serial.exists() && !portUsed(ui->deviceEdit->text())){
        ui->statusBar->showMessage("Adding XBee", 1500);
        BaudRateType baudRate=BAUD9600;
        if(ui->baudBox->currentText() == QString("1200 bps")){
            qWarning("badRate =1");
            baudRate=BAUD1200;
        }
        else if(ui->baudBox->currentText() == QString("2400 bps")){
            baudRate=BAUD2400;
            qWarning("badRate =2");
        }
        else if(ui->baudBox->currentText() == QString("4800 bps")){
            baudRate=BAUD4800;
            qWarning("badRate =3");
        }
        else if(ui->baudBox->currentText() == QString("9600 bps")){
            baudRate=BAUD9600;
            qWarning("badRate =4");
        }
        else if(ui->baudBox->currentText() == QString("19200 bps")){
            baudRate=BAUD19200;
            qWarning("badRate =5");
        }
        else if(ui->baudBox->currentText() == QString("38400 bps")){
            baudRate=BAUD38400;
            qWarning("badRate =6");
        }
        else if(ui->baudBox->currentText() == QString("57600 bps")){
            baudRate=BAUD57600;
            qWarning("badRate =7");
        }
        else if(ui->baudBox->currentText() == QString("115200 bps")){
            baudRate=BAUD115200;
            qWarning("badRate =8");
        }

        QXBee* xbee=new QXBee(ui->deviceEdit->text(), baudRate, this);
        xbeeList.push_back(xbee);
        connect(xbee, SIGNAL(XBeeReady()), SLOT(updateManageTab()));
        connect(xbee, SIGNAL(XBeeError(QString)), SLOT(reportError(QString)));
        qWarning("Current thread = 0x%x - addXBee", QThread::currentThread());
        xbee->start();
    }
    else if(portUsed(ui->deviceEdit->text())){
        QMessageBox::critical(this, "Serial Error", "That serial port is already in use!");
    }
}

void MainWindow::updateManageTab(){
    QXBee* xbee=(QXBee*)sender();
    qWarning("Update called from %s", xbee->getPortName().toAscii().data());
    ui->deviceEdit->clear();


    QTableWidgetItem *portItem = new QTableWidgetItem(xbee->getPortName());
    QTableWidgetItem *statusItem = new QTableWidgetItem(QString("Ready"));

    int row=ui->xbeeTable->rowCount();
    //int col=ui->xbeeTable->columnCount();
    ui->xbeeTable->setRowCount(row+1);
    //ui->xbeeTable->setColumnCount(col+1);
    ui->xbeeTable->setItem(row, 0, portItem);
    ui->xbeeTable->setItem(row, 1, statusItem);
}

bool MainWindow::portUsed(QString port){
    for(int i=0; i<xbeeList.size(); i++){
        if(xbeeList.at(i)->getPortName() == port){
            return true;
        }
    }
    return false;
}

void MainWindow::reportError(QString str){
    QObject* s=sender();
    QString errorMsg="Error: ";
    if(xbeeList.contains((QXBee*)s)){ //Reporting an XBee Error
        errorMsg.append(((QXBee*)s)->getPortName());
        errorMsg.append(" - ");
        errorMsg.append(str);
        int i=xbeeList.indexOf((QXBee*)s);
        if(xbeeList[i]==s){
            qWarning("Removing XBee[%i] - %s", i, ((QXBee*)s)->getPortName().toAscii().data());
            xbeeList[i]->quit();
            delete xbeeList[i];
            xbeeList.removeAt(i);
        }
    }
    ui->statusBar->showMessage(errorMsg, 5000);
    qWarning("%s", errorMsg.toAscii().data());
}
