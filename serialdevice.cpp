#include "serialdevice.h"


SystemStatusPacket::SystemStatusPacket(DevicePacket& packet){
	Q_ASSERT(packet.type == PktDeviceStatus);

	/*
	 *	NOTE:	System status packet data layout
	 *
	 *	AccelStatus		Orientation			SampleRate	SBStatus				index	SBFps
	 *	[I2CStatusType]	[OrientationType]	[uint16]	[ShiftBriteStatusType]	[uint8]	[uint16]
	 *	0				1					2,3			4						5		6,7
	 */

	const quint8* data = (const quint8*) packet.data.constData();

	this->i2cStatus = (I2CStatusType) data[0];
	this->orientation = (OrientationType) data[1];
	this->accelSampleRate = (data[2] << 8) | (data[3]);

	this->sbStatus = (ShiftBriteStatusType) data[4];
	this->sbIndex = data[5];
	this->sbFPS = (data[6] << 8) | (data[7]);

	Q_ASSERT(isValid() == true);
}

bool SystemStatusPacket::isValid() const {
	return (this->i2cStatus <= I2C_Error) && (this->sbStatus <= SB_Stop) && (this->orientation <= DirectionRight);
}



AccelValuePacket::AccelValuePacket(DevicePacket& packet){
	timestamp = QDateTime::currentDateTimeUtc();

	const short* data = (const short*) packet.data.constData();

	this->z = data[0];
	this->y = data[1];
	this->x = data[2];
}


AccelValuePacket::AccelValuePacket(){
	timestamp = QDateTime::currentDateTimeUtc();

	this->z = 0;
	this->y = 0;
	this->x = 0;
}

SerialDevice::SerialDevice(QString portPath, BaudRateType baud, QObject *parent) :
	QObject(parent)
{
	serialPort = new QextSerialPort(portPath);
	serialPort->setBaudRate(baud);
	serialPort->setFlowControl(FLOW_OFF);
    serialPort->setParity(PAR_NONE);
    serialPort->setDataBits(DATA_8);
    serialPort->setStopBits(STOP_1);
	serialPort->open(QIODevice::ReadWrite);

	rxState = Rx_Idle;

	connect(serialPort, SIGNAL(readyRead()), this, SLOT(readSerialDataSlot()));
	connect(this, SIGNAL(packetReadySignal(DevicePacket)), this, SLOT(processDevicePacketSlot(DevicePacket)));
}


SerialDevice::~SerialDevice(){
	if(serialPort != NULL){
		if(serialPort->isOpen()){
			serialPort->close();
		}

		delete serialPort;
	}
}



void SerialDevice::readSerialDataSlot(){
	int bytesReady = serialPort->bytesAvailable();
	int bytesRead  = 0;

	if(bytesReady < 0){
		qDebug() << "ERROR: Read message called with " << bytesReady << " bytes available";

		serialPort->close();
		emit brokenPipe();
	}


	char buf[bytesReady];
	bytesRead = serialPort->read(buf, bytesReady);

	if(rxState == Rx_Idle){
		packetBuffer.clear();
	}
	packetBuffer.append(buf, bytesRead);

	//Packet reading state machine
	while(packetBuffer.length() > 0){
		if(rxState == Rx_Idle || rxState == Rx_PacketStart){
			//Check for STX
			if(packetBuffer.at(0) == (char) COMM_PACKET_STX){
				rxState = Rx_PacketType;
				packetBuffer.remove(0, 1);
			}
			else{
				rxState = Rx_PacketStart;
				packetBuffer.remove(0, 1);
			}
		}
		else if(rxState == Rx_PacketType){
			currentPacket.type = (PacketTypes) packetBuffer.at(0);
			packetBuffer.remove(0, 1);
			rxState = Rx_PacketLength;
		}
		else if(rxState == Rx_PacketLength){
			currentPacket.length = packetBuffer.at(0);
			currentPacket.data.clear();
			packetBuffer.remove(0, 1);
			rxState = Rx_PacketData;
		}
		else if(rxState == Rx_PacketData){
			int bytesExpected = currentPacket.length - currentPacket.data.length();

			if(bytesExpected >= packetBuffer.size()){
				currentPacket.data.append(packetBuffer);
				packetBuffer.clear();
			}
			else{
				const char* data = packetBuffer.constData();
				currentPacket.data.append(data, bytesExpected);
				packetBuffer.remove(0, bytesExpected);
			}

			if(currentPacket.length == currentPacket.data.length()){
				rxState = Rx_Idle;
				emit packetReadySignal(currentPacket);
			}
		}
	}
}


void SerialDevice::processDevicePacketSlot(DevicePacket packet){
	if(packet.type == PktDeviceStatus){
		SystemStatusPacket systemStatus(packet);

		emit systemStatusSignal(systemStatus);
	}
	if(packet.type == PktAccValues){
		AccelValuePacket accelValues(packet);

		emit accelerometerValuesSignal(accelValues);

		float deltaMs = accelValues.timestamp.toMSecsSinceEpoch() - lastPacket.timestamp.toMSecsSinceEpoch();

		emit accelSamplesPerSecondSignal(1000.0f / deltaMs);

		lastPacket = accelValues;
	}
}


void SerialDevice::sendCommandSlot(DevicePacket packet){
	if(packet.type == PktHostCmd){
		QByteArray data;

		data.append((char) COMM_PACKET_STX);
		data.append((char) packet.type);
		data.append((char) packet.length);
		data.append(packet.data);

		serialPort->write(data);
	}
}


void SerialDevice::enableAccelDataStreamSlot(bool enable){
	DevicePacket packet;

	packet.type = PktHostCmd;
	packet.length = sizeof(quint16);

	quint16 flags = PktDeviceStatus;

	if(enable){
		flags |= PktAccValues;
	}

	packet.data.append((char*) &flags, sizeof(quint16));

	sendCommandSlot(packet);
}
