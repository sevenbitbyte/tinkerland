#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QObject>
#include <QtCore>
#include "qextserialport/qextserialport.h"


#define COMM_PACKET_STX  (0x81)

enum PacketTypes{PktOrientation=1, PktAccValues=2, PktAccStatus=4, PktMagValues=8, PktMagStatus=16, PktDeviceStatus=32, PktHostCmd=64};

struct DevicePacket{
	PacketTypes type;
	quint8 length;
	QByteArray data;
};

enum I2CStatusType { I2C_Detect=0, I2C_Init=1, I2C_Run=2, I2C_Error=3 };
enum ShiftBriteStatusType { SB_Init=0, SB_Step=1, SB_Run=2, SB_Stop=3 };
enum OrientationType { DirectionNormal=0, DirectionLeft=1, DirectionInverted=2, DirectionRight=3 };


struct SystemStatusPacket{
	SystemStatusPacket(DevicePacket& packet);

	//Accelerometer status
	I2CStatusType i2cStatus;
	OrientationType orientation;
	unsigned int accelSampleRate;

	//ShiftBrite status
	ShiftBriteStatusType sbStatus;
	unsigned int sbIndex;
	unsigned int sbFPS;

	bool isValid() const;
};


struct AccelValuePacket{
	AccelValuePacket();
	AccelValuePacket(DevicePacket& packet);

	int x;
	int y;
	int z;
	QDateTime timestamp;
};


class SerialDevice : public QObject
{
	Q_OBJECT
	public:
		explicit SerialDevice(QString portPath, BaudRateType baud, QObject *parent = 0);
		~SerialDevice();
		
	signals:
		void packetReadySignal(DevicePacket packet);

		void systemStatusSignal(SystemStatusPacket status);
		void accelerometerStatusSignal(I2CStatusType status);
		void orientationSignal(OrientationType orientation);

		void accelerometerValuesSignal(AccelValuePacket values);
		void accelSamplesPerSecondSignal(float samples);

		void brokenPipe();
		
	public slots:
		/**
		 * @brief	Reads in available data from the serial port, emits a packet
		 *			ready signal when a valid packet is read.
		 */
		void readSerialDataSlot();

		/**
		 * @brief	Processes the content of a device packet, emit all appropriate
		 *			device status signals.
		 */
		void processDevicePacketSlot(DevicePacket packet);

		void sendCommandSlot(DevicePacket packet);

		void enableAccelDataStreamSlot(bool enable);

	private:
		enum RecvState { Rx_Idle=0, Rx_PacketStart, Rx_PacketLength, Rx_PacketType, Rx_PacketData};


		RecvState rxState;
		QextSerialPort* serialPort;

		DevicePacket currentPacket;
		QByteArray packetBuffer;
		AccelValuePacket lastPacket;
};

#endif // SERIALDEVICE_H
