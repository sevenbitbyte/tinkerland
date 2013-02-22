#include <limits>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	device = NULL;

	connect(ui->portCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(serialPortPathChangeSlot(QString)));
	connect(ui->portCombo, SIGNAL(editTextChanged(QString)), this, SLOT(serialPortPathChangeSlot(QString)));
	connect(ui->portOpenButton, SIGNAL(clicked()), this, SLOT(openPortSlot()));
	connect(ui->accelAutoRotateCheck, SIGNAL(toggled(bool)), this, SLOT(enableAutoRotateSlot(bool)));
	connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(visibleTabChangedSlot(int)));

	populatePortPathCombo();
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::populatePortPathCombo(){
	QDir devDir("/dev");

	QMap<QString, QString> serialDevices;
	QFileInfoList files = devDir.entryInfoList(QDir::System|QDir::Writable);

	for(int i=0; i<files.count(); i++){
		QFileInfo info = files[i];

		if(info.fileName().startsWith("tty")){
			serialDevices.insert(info.fileName(), info.absoluteFilePath());
		}
	}

	int index = -1;
	if(ui->portCombo->count()){
		index = ui->portCombo->currentIndex();
		ui->portCombo->clear();
	}

	ui->portCombo->insertItems(0, serialDevices.values());

	if(index != -1){
		ui->portCombo->setCurrentIndex(index);
	}
}

BaudRateType MainWindow::stringToBaud(QString str) const{
	BaudRateType baudRate = BAUD115200;

	if(str.contains("1200")){
        baudRate=BAUD1200;
    }
    else if(str.contains("2400")){
        baudRate=BAUD2400;
    }
    else if(str.contains("4800")){
        baudRate=BAUD4800;
    }
    else if(str.contains("9600")){
        baudRate=BAUD9600;
    }
    else if(str.contains("19200")){
        baudRate=BAUD19200;
    }
    else if(str.contains("38400")){
        baudRate=BAUD38400;
    }
    else if(str.contains("57600")){
        baudRate=BAUD57600;
    }
    else if(str.contains("115200")){
        baudRate=BAUD115200;
    }

	return baudRate;
}

void MainWindow::serialPortPathChangeSlot(QString portPath){
	QFile file(portPath);

	bool enable = file.exists();

	ui->portBaudCombo->setEnabled(enable);
	ui->portOpenButton->setEnabled(enable);
}


void MainWindow::openPortSlot(){
	if(device != NULL){
		delete device;
		device = NULL;
	}

	if(!ui->portOpenButton->isChecked()){
		ui->portCombo->setEnabled(true);
		ui->portBaudCombo->setEnabled(true);

		ui->tabWidget->setEnabled(false);

		ui->systemStatusEdit->clear();
		ui->accelStatusEdit->clear();
		return;
	}

	errorState.originFileInfo = QFileInfo(ui->portCombo->currentText());
	errorState.originCreateTime = errorState.originFileInfo.created();
	errorState.type = ErrorState::Err_None;

	BaudRateType baud = stringToBaud(ui->portBaudCombo->currentText());
	device = new SerialDevice(ui->portCombo->currentText(), baud, this);


	connect(device, SIGNAL(systemStatusSignal(SystemStatusPacket)), this, SLOT(systemStatusSlot(SystemStatusPacket)));
	connect(device, SIGNAL(brokenPipe()), this, SLOT(deviceBrokenPipeSlot()));
	connect(device, SIGNAL(accelerometerValuesSignal(AccelValuePacket)), this, SLOT(accelValueSlot(AccelValuePacket)));
	connect(device, SIGNAL(accelSamplesPerSecondSignal(float)), this, SLOT(accelSamplesPerSecSlot(float)));

	ui->tabWidget->setEnabled(true);
	ui->portCombo->setEnabled(false);
	ui->portBaudCombo->setEnabled(false);
	ui->systemStatusEdit->setText("Connecting");
}

void MainWindow::deviceBrokenPipeSlot(){
	errorState.type = ErrorState::Err_BrokenPipe;
	errorState.startTime = QDateTime::currentDateTimeUtc();

	if(device != NULL){
		qDebug() << "ERROR: Broken pipe, disconnecting" ;

		delete device;
		device = NULL;
	}

	if(ui->portOpenButton->isChecked()){
		//Connect still pressed, we should attempt to reconnect to device

		//Does file still exist
		QFileInfo deviceFile(ui->portCombo->currentText());

		if(deviceFile.exists()){
			if(deviceFile.created().toMSecsSinceEpoch() == errorState.originCreateTime.toMSecsSinceEpoch()){

				qDebug() << "INFO: sleep 2000";
				QTimer::singleShot(2000, this, SLOT(deviceBrokenPipeReconnectSlot()));
				return;
			}
		}
	}

	qDebug() << "INFO: sleep 200";
	QTimer::singleShot(200, this, SLOT(deviceBrokenPipeReconnectSlot()));
}


void MainWindow::deviceBrokenPipeReconnectSlot(){
	populatePortPathCombo();

	QString filename = errorState.originFileInfo.fileName();

	int index = ui->portCombo->findText(filename, Qt::MatchExactly);

	if(index == -1){
		filename.remove(filename.length()-1, 1);
		filename.append("*");

		index = ui->portCombo->findText(filename, Qt::MatchRegExp);
	}

	if(index != -1){
		if(device != NULL){
			delete device;
			device = NULL;
			qDebug() << "INFO: disconnecting" ;
		}

		qDebug() << "INFO: Connecting";

		errorState.originFileInfo = QFileInfo(ui->portCombo->currentText());
		errorState.originCreateTime = errorState.originFileInfo.created();
		errorState.type = ErrorState::Err_None;

		BaudRateType baud = stringToBaud(ui->portBaudCombo->currentText());
		device = new SerialDevice(ui->portCombo->currentText(), baud, this);


		connect(device, SIGNAL(systemStatusSignal(SystemStatusPacket)), this, SLOT(systemStatusSlot(SystemStatusPacket)));
		connect(device, SIGNAL(brokenPipe()), this, SLOT(deviceBrokenPipeSlot()));
		connect(device, SIGNAL(accelerometerValuesSignal(AccelValuePacket)), this, SLOT(accelValueSlot(AccelValuePacket)));
		connect(device, SIGNAL(accelSamplesPerSecondSignal(float)), this, SLOT(accelSamplesPerSecSlot(float)));

	}
	else{
		qDebug() << "INFO: Sleeping 500";
		QTimer::singleShot(500, this, SLOT(deviceBrokenPipeReconnectSlot()));
	}
}


void MainWindow::systemStatusSlot(SystemStatusPacket status){
	ui->systemStatusEdit->setText("Connected");

	switch(status.i2cStatus){
		case I2C_Detect:
			ui->accelStatusEdit->setText("Detecting");
			break;
		case I2C_Init:
			ui->accelStatusEdit->setText("Initializing");
			break;
		case I2C_Error:
			ui->accelStatusEdit->setText("Error");
			break;
		case I2C_Run:
			ui->accelStatusEdit->setText("Running");
			break;
		default:
			ui->accelStatusEdit->setText("Unknown");
			break;
	}


	if(ui->accelAutoRotateCheck->isChecked()){
		ui->accelOrientationCombo->setCurrentIndex(status.orientation);
	}
}


void MainWindow::enableAutoRotateSlot(bool enable){
	if(device != NULL){
		ui->accelOrientationCombo->setEnabled(!enable);
	}
}


void MainWindow::visibleTabChangedSlot(int index){
	if(device != NULL){
		device->enableAccelDataStreamSlot(index == 1);
	}
}


void MainWindow::accelValueSlot(AccelValuePacket values){
	float scaleValue = ((float)values.x) / std::numeric_limits<short>::max();

	ui->xAxisBar->setValue( (int)(scaleValue * 100.0f) );

	scaleValue = ((float)values.y) / std::numeric_limits<short>::max();
	ui->yAxisBar->setValue( (int)(scaleValue * 100.0f) );

	scaleValue = ((float)values.z) / std::numeric_limits<short>::max();
	ui->zAxisBar->setValue( (int)(scaleValue * 100.0f) );
}

void MainWindow::accelSamplesPerSecSlot(float samples){
	ui->accelSampleRateLcd->display((int)samples);
}
