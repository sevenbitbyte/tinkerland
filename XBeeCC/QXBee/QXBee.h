#ifndef QXBEE_H
#define QXBEE_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include "qextserialport/qextserialport.h"
#include "QXBeeCommon.h"

class QXBee : public QThread{
    Q_OBJECT
    public:
        //QXBee(QString port, QObject* parent);
        QXBee(QString port, BaudRateType baud=BAUD9600, QObject* parent=NULL);
        //QXBee(QString port, BaudRateType baud=BAUD9600, ParityType parity=PAR_NONE, QObject* parent=NULL);
        ~QXBee();

        enum RecvState{Idle=0, PacketStart, PacketLengthH, PacketLengthL,
                       PacketType, PacketData, PacketChecksum, MessageStart};
        enum CommMode{ErrorMode, TransparentMode, ATMode, APIMode};
        enum ModemStatus{HardReset=0x0, WatchdogRest, Disassociated,
                         SyncLost, CoordRealign, CoordStart};

        CommMode getCommMode(){return currentMode;}

        QString getPortName();

        BaudRateType setBaudRate(BaudRateType baud=BAUD115200);

        QByteArray readRegister(QString regName);

        bool setRegister(QString regName, QByteArray value, bool allowCMDRegs=false);
        /*
        bool changeBaud(BaudRateType baud);
        bool sendMsg(QString msg, QXBeeAddr addr,  bool waitForACK=true);

        QString getVersionString();
        QString getFrimwareVersion();
        QString getHardwareVersion();
        int getSignalStrength();
        int getCCAFails();
        int getACKFails();

        void clearCCAFails();
        void clearACKFails();*/

    public slots:
        void connectToXBee();

    protected:
        void run();
        bool enterATMode();
        bool exitATMode();
        bool enterAPIMode();
        bool exitAPIMode();
        bool sendPacket(QXBeePacket pack);
        bool waitMessage(int timeout=-1, int innerTimeout=200);
        void readXBeeInfo();
        void readXBeeSettings();
        //QByteArray readMessage(int timeoutSec, int timeoutMs=500, int innerDelay=500);
        //CommMode setCommMode(CommMode mode);

        //QByteArray sendCommand(QString cmd, QByteArray param, );

    protected slots:

        //void readAPIPacket();
        //void parsePacket(QXBeePacket pack);

    private slots:
        void readMsg();

    private:
        QXBeeInfo info;
        QXBeeSettings settings;
        QTimer* timer;
        QXBeeAddr addr;
        QMutex readMutex;
        QextSerialPort *serPort;
        CommMode currentMode;
        RecvState recvState;
        char commandChar;
        quint16 gaurdTimeS;
        quint16 gaurdTimeMs;
        QByteArray currentMessage;
        QByteArray currentString;
        QXBeePacket currentPacket;
        quint8 currentFrameId;
        //ModemStatus currentModemStatus;

    signals:
        void packetReady(QXBeePacket pack);
        void readMsg(QString msg);
        void statusChanged(ModemStatus status);
        void XBeeReady();
        void XBeeError(QString errStr);
};

#endif
