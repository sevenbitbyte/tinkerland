#ifndef QXBEECOMMON_H
#define QXBEECOMMON_H

#define PACKET_STX 0x7E
//API Packet IDs
#define ID_ModemStatus  0x8A
#define ID_AT_CMD       0x08
#define ID_AT_QUEUE_PARAM   0x09
#define ID_AT_CMD_RESPONSE  0x88
#define ID_TX64_REQ     0x00
#define ID_TX16_REQ     0x01
#define ID_TX_STATUS    0x89
#define ID_RX64_PACK    0x80
#define ID_RX16_PACK    0x81

#define BROADCAST_ADDR16    0xFFFF
#define BROADCAST_ADDR64    0x000000000000FFFF
#define DEFAULT_CMD_CHAR    '+'

#define RETURN_CHAR 0x0D

#define INT16_TO_8H(s) (s>>8)
#define INT16_TO_8L(s) (s&0x00FF)

struct QXBeeAddr{
    quint16 addr16;
    quint32 addrL;
    quint32 addrH;
    quint16 panId;
    bool isCoordinator;
    quint8 channel;
};

struct QXBeePacket{
    quint16 length;
    quint8  checksum;
    quint8  type;
    char frameId;
    QByteArray data;
};


struct QXBeeInfo{
    QString longVersion;
    QString version;
    QString nodeIdentifier;
    quint16 CCAFailures;
    quint16 ACKFailures;
};

struct QXBeeSettings{
    QXBeeAddr destNode;
    quint8 scanDuration;
    quint16 scanList;
    quint8 powerLevel;
    quint8 nodeDescTime;
    quint8 parity;
    quint8 macMode;
    bool AESEnabled;
    quint16 gaurdTime;
    quint8 rssi;
    quint16 commandTimeout;
    char commandChar;
    quint8 CCAThreshold;
    quint16 baudRate;
    quint8 associationID;
    quint8 A1;
    quint8 A2;
};

#endif // QXBEECOMMON_H
