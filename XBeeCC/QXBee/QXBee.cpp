#include "QXBee.h"
//#include <string.h>


QXBee::QXBee(QString port, BaudRateType baud, QObject* parent){
    qWarning("QXBee - %s", port.toAscii().data());
    serPort=new QextSerialPort(port);
    serPort->setBaudRate(baud);
    serPort->setFlowControl(FLOW_OFF);
    serPort->setParity(PAR_NONE);
    serPort->setDataBits(DATA_8);
    serPort->setStopBits(STOP_1);
    serPort->open(QIODevice::ReadWrite);

    currentMode=TransparentMode;
    commandChar=DEFAULT_CMD_CHAR;
    gaurdTimeS=1;
    gaurdTimeMs=10000;
    recvState=Idle;
    currentFrameId=0;

    qWarning("Current thread = 0x%x - constructor", QThread::currentThread());

    connect(serPort, SIGNAL(readyRead()), SLOT(readMsg()) , (Qt::ConnectionType)1);
}

QXBee::~QXBee(){
    qWarning("QXBee - Destroyed");
    if(serPort->isOpen()){
        serPort->close();
    }
    delete serPort;
    delete timer;
}

void QXBee::run(){
    //connectToXBee();
    qWarning("Current thread = 0x%x - run(%i)", QThread::currentThread(), isRunning());
    timer=new QTimer();
    qWarning("Current thread = 0x%x - timer", timer->thread());
    qWarning("Current thread = 0x%x - serPort", serPort->thread());
    //timer->singleShot(100, this, SLOT(connectToXBee()));
    connect(timer, SIGNAL(timeout()), this, SLOT(connectToXBee()), (Qt::ConnectionType)1);
    timer->setSingleShot(true);
    timer->start(100);
    exec();
}

void QXBee::connectToXBee(){
    qWarning("Current thread = 0x%x - connectToXBee", QThread::currentThread());
    qDebug("QXBee open: %d", serPort->isOpen());
    if(!serPort->isOpen()){
        currentMode=ErrorMode;
        emit XBeeError("Could not open serial port");
    }
    else if(enterAPIMode()){
        emit XBeeReady();
        msleep(1000);
        //readXBeeInfo();
        //readXBeeSettings();
    }
}

QString QXBee::getPortName(){
    return serPort->portName();
}

BaudRateType QXBee::setBaudRate(BaudRateType baud){
    //
}

bool QXBee::enterATMode(){
    qWarning("QXBee - enterATMode");
    if(currentMode==ATMode){
        return true;
    }
    else if(currentMode==APIMode){
        if(!exitAPIMode()){
            //emit XBeeError("Could not enter AT mode");
            return false;
        }
    }
    else if(currentMode==TransparentMode){
        QString cmdChars(commandChar);
        cmdChars.append(commandChar);
        cmdChars.append(commandChar);
        serPort->write(cmdChars.toAscii(), cmdChars.length());
        serPort->flush();
        currentMode=ATMode; //Needed to get data readying to work
        if(waitMessage(gaurdTimeS*1000+gaurdTimeMs)){
            char successStr[]={'O', 'K', RETURN_CHAR};
            if(strncmp(successStr,currentMessage.data(),3)==0){
                return true;
            }
            else{
                qWarning("Failed compare on length %i",currentMessage.size());
            }
        }
        currentMode=TransparentMode;
        emit XBeeError("Could not enter AT mode");
        return false;
    }
    return false;
}

bool QXBee::enterAPIMode(){
    if(currentMode!=ATMode){
        if(!enterATMode()){
            qWarning("QXBee - enterATMode Failed!");
            return false;
        }
    }
    qWarning("QXBee - enterATMode Success");
    QByteArray param=QByteArray::fromHex("0x1");
    if(setRegister("AP", param, true)){
        qWarning("QXBee - enterAPIMode Success");
        currentMode=APIMode;
        return true;
    }
    qWarning("QXBee - enterAPIMode Failed");
    return false;
}

bool QXBee::exitAPIMode(){}

bool QXBee::exitATMode(){}

bool QXBee::sendPacket(QXBeePacket pack){
    if(currentMode==APIMode && serPort->isOpen()){
        pack.length=pack.data.length()+2;
        quint32 checksum=pack.type+pack.frameId;
        for(int i=0; i<pack.data.length(); i++){
            checksum+=pack.data.data()[i];
        }
        pack.checksum=checksum;
        pack.checksum=0xFF - pack.checksum;

        char lenH=INT16_TO_8H(pack.length);
        char lenL=INT16_TO_8L(pack.length);

        //qDebug("Length:0x%x Type:0x%x Data:%s ChkSum:0x%x", pack.length,
        //         pack.type, pack.data.data(), pack.checksum);

        quint8 packStr[]={0x7e,0x00,0x04,0x08,0x52,0x44,0x4C,0x15, 0x00};
        //quint8 *packStr=new quint8[pack.length+5];
        /*packStr[0]=PACKET_STX;
        packStr[1]=lenH;
        packStr[2]=lenL;
        packStr[3]=pack.type;
        packStr[4]=pack.frameId;
        packStr[pack.length+3]=pack.checksum;
        packStr[pack.length+4]=0x0;
        memcpy((char*)packStr+5, pack.data.data(), pack.data.length());*/

        for(int i=0; i < (pack.length+5); i++){
            qDebug("Write[%i] 0x%x", i, packStr[i]);
        }
        int writeCount=serPort->write((char*)packStr, pack.length+4);
        serPort->flush();
        /*char stx=PACKET_STX;
        int writeCount=serPort->write(&stx, 1);
        writeCount+=serPort->write(&lenH, 1);
        writeCount+=serPort->write(&lenL, 1);
        writeCount+=serPort->write((char*)&pack.type, 1);
        for(int i=0; i<pack.data.length(); i++){
            writeCount+=serPort->write((char*)pack.data.at(i), i);
        }
        writeCount+=serPort->write((char*)pack.checksum, 1);*/
        if(writeCount==pack.length+4){
            qDebug("Wrote %i bytes from packet 0x%x", writeCount, pack.frameId);
            return true;
        }
        int totalBytes=pack.length+4;
        qDebug("Error: sendPacket - Write failed, sent %i bytes out of %i", writeCount, totalBytes);
    }
    return false;
}

bool QXBee::waitMessage(int timeout, int innerTimeout){
    if(readMutex.tryLock()){
        qWarning("QXBee waitMessage(%i) locked readMutex", timeout);
        bool retVal=readMutex.tryLock(timeout);
        readMutex.unlock();
        if(recvState!=Idle){
            retVal=waitMessage(innerTimeout, innerTimeout);
        }
        return retVal;
    }
    qWarning("Did not lock Mutex!");
    return false;
}

void QXBee::readMsg(){
    //qWarning("Current thread = 0x%x - readMsg", QThread::currentThread());
    bool requested=true;
    if(readMutex.tryLock()){
        qDebug("readMessage() - Unexpected data");
        requested=false;
    }
    int numBytes=serPort->bytesAvailable();
    int readBytes=0;
    qDebug("ReadMessage called, %i bytes available", numBytes);
    if(numBytes<0){return;}
    if(recvState==Idle){
        currentMessage.clear();
        currentString.clear();
        currentPacket.type=0;
        currentPacket.length=0;
        currentPacket.checksum=0;
    }
    if(currentMode==ATMode){
        char buf[numBytes];
        readBytes=serPort->read(buf, numBytes);
        currentMessage.append(QByteArray::fromRawData(buf,readBytes));
        if(currentMessage.size() > 0){
            if(currentMessage.at(currentMessage.size()-1) == RETURN_CHAR){
                recvState=Idle;
                readMutex.unlock();
            }
            else{
                recvState=MessageStart;
            }
        }
        else{
            qDebug("readMessage() - Did not read anything!");
        }
    }
    else if(currentMode==APIMode){
        qDebug("Reading packet %i bytes", numBytes);
//Idle, PacketStart, PacketLength, PacketType, PacketData, PacketChecksum, MessageStart
        char buf=0xff;
        if(recvState==Idle && numBytes>0){
            if(serPort->read(&buf, 1) == 1 && buf==PACKET_STX){
                recvState=PacketLengthH;
                numBytes--;
            }
        }
        if(recvState==PacketLengthH && numBytes>0){
            if(serPort->read(&buf, 1) == 1){
                currentPacket.length=(((quint16) buf)<<8) & 0xff00;
                recvState=PacketLengthH;
                numBytes--;
            }
        }
        if(recvState==PacketLengthL && numBytes>0){
            if(serPort->read(&buf, 1) == 1){
                currentPacket.length &= 0xff00;
                currentPacket.length |= (quint16)buf;
                recvState=PacketType;
                numBytes--;
            }
        }
        if(recvState==PacketType && numBytes>0){
            buf=0xff;
            if(serPort->read(&buf, 1) == 1 && buf!=0xff){
                currentPacket.type=buf;
                recvState=PacketData;
                numBytes--;
            }
        }
        if(recvState==PacketData && numBytes>0){
            char bufPtr[numBytes];
            readBytes=serPort->read(bufPtr, numBytes);
            if(readBytes>0){
                currentPacket.data.append(QByteArray::fromRawData(bufPtr,readBytes));
            }
            if(currentPacket.length==currentPacket.data.size()){
                recvState=PacketChecksum;
            }
        }
        if(recvState==PacketChecksum && numBytes>0){
            if(serPort->read(&buf, 1) == 1){
                currentPacket.checksum=buf;
                recvState=Idle;
                readMutex.unlock();
                emit packetReady(currentPacket);
                //return;
            }
        }else{
        qDebug("readMsg - Partial packet read recvState=%i", recvState);
    }
    }
}


bool QXBee::setRegister(QString regName, QByteArray value, bool allowCMDRegs){
    if(!allowCMDRegs){
        //Check regName against allowed list
    }
    if(currentMode==ATMode){
        QString cmd("AT");
        cmd.append(regName);
        cmd.append(value.toHex());
        cmd.append(RETURN_CHAR);
        qWarning("Writing [%s]", cmd.toAscii().data());
        serPort->write(cmd.toAscii().data(), cmd.length());
        serPort->flush();
        if(waitMessage(gaurdTimeS*1000+gaurdTimeMs)){
            char successStr[]={'O', 'K', RETURN_CHAR};
            if(strncmp(successStr,currentMessage.data(),3)==0){
                currentMode=ATMode;
                return true;
            }
            else{
                qWarning("Failed compare on length %i",currentMessage.size());
            }
        }
        qWarning("setRegister(%s%s)", regName.toAscii().data(), QString(value.toHex()).toAscii().data());
        return false;
    }
    else if(currentMode==APIMode){
        QXBeePacket pack;
        pack.data=value;
        pack.type=ID_AT_CMD;
        return sendPacket(pack);
    }
    else if(currentMode==TransparentMode){
        if(enterATMode()){
            if(setRegister(regName, value, allowCMDRegs)){
                exitATMode();
                return true;
            }
            exitATMode();
            return false;
        }
        return false;
    }
    return false;
}

QByteArray QXBee::readRegister(QString regName){
    if(currentMode==APIMode){
        //regName.append(RETURN_CHAR);
        QXBeePacket pack;
        pack.type=ID_AT_CMD;
        pack.data=regName.toAscii();
        pack.frameId=0x52;//currentFrameId;
        currentFrameId++;
        char packStr[]={0x7e,0x00,0x04,0x08,0x52,0x44,0x4C,0x15, 0x00};
        serPort->write(packStr, 8);
        serPort->flush();
        //if(sendPacket(pack)){
            if(waitMessage(10000)){
                //Packet is ready
                qWarning("Current Pack.length = %i", currentPacket.length);
                return currentPacket.data;
            }
          //  qWarning("Current Pack.length = %i %i", serPort->bytesAvailable(), serPort->lastError());
        //}
    }
    return currentPacket.data;
}

void QXBee::readXBeeInfo(){
    QByteArray buf=readRegister("DL");
    qWarning("Long Version - %s", buf.data());
}

void QXBee::readXBeeSettings(){

}
