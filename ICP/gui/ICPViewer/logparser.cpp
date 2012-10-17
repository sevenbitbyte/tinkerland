#include <QtCore>
#include "logparser.h"
#include "point.h"

LogParser::LogParser()
{
	lineCount=0;
}

LogParser::LogParser(QString path){
	lineCount=0;
	openLog(path);
}

LogParser::~LogParser(){
    if(logFile.isOpen()){
	logFile.close();
    }
}


bool LogParser::openLog(QString path){
	if(logFile.isOpen()){
		logFile.close();
	}

	logFile.setFileName(path);

	if(logFile.open(QIODevice::ReadOnly)){
		lineCount=0;
		return true;
	}

	return false;
}

EntryHeader* LogParser::getCurrentHeader(){
	return &currentHeader;
}

bool LogParser::readLogEntry(){
	// Read in the next non-comment line
	bool read = readHeader();
	if(!read)
		return false;

	// Filter by interface
	if(currentHeader.interface == "laser") {
		if(!parseLidar()){
			return readLogEntry();
		}
	} else if(currentHeader.interface == "position2d") {
		if(!parsePosition()){
			return readLogEntry();
		}
	}
	else{
		return readLogEntry();
	}

	return true;
}

bool LogParser::readHeader(){
	if(logFile.atEnd()){
		emit eof();
	}

	while(!logFile.atEnd()){
		// Read data
		QString lineBuf = logFile.readLine();

		lineCount++;
		
		// Check if this is a comment line
		if(lineBuf[0] == QChar('#'))
			continue;

		// Separate tokens
		currentTokens = lineBuf.split(' ', QString::SkipEmptyParts);

		bool ok;

		// Read 
		if(currentTokens.size() > MIN_HDR_TOKENS){
			// Message time	
			float time = currentTokens.at(0).toFloat(&ok);
			RETURN_IFEQ(ok, false, false);
			float msec = (time - (int)time) * 1000;
			currentHeader.time = QDateTime(QDate(0,0,0));
			currentHeader.time = currentHeader.time.addSecs( (int)time );
			currentHeader.time = currentHeader.time.addMSecs( (int)msec );
			
			// Address of host
			currentHeader.host = currentTokens.at(1).toUInt(&ok);
			RETURN_IFEQ(ok, false, false);

			// Robot
			currentHeader.robot = currentTokens.at(2).toUInt(&ok);
			RETURN_IFEQ(ok, false, false);

			// Interface type
			currentHeader.interface = currentTokens.at(3);

			// Interface index
			currentHeader.index = currentTokens.at(4).toUInt(&ok);
			RETURN_IFEQ(ok, false, false);

			// Message type
			currentHeader.type = currentTokens.at(5).toUInt(&ok);
			RETURN_IFEQ(ok, false, false);

			// Message sub-type
			currentHeader.subtype = currentTokens.at(6).toUInt(&ok);
			RETURN_IFEQ(ok, false, false);

			emit pos(logFile.pos());
			return true;
		}
	}

	emit pos(logFile.pos());
	return false;
}

int LogParser::byteSize(){
	if(logFile.isOpen()){
		return logFile.size();
	}

	return 0;
}

bool LogParser::parsePosition()
{
	PositionEntry position;
	bool ok;

	// Only want message Type = 1, Subtype = 1
	if(currentHeader.type != 1 || currentHeader.subtype != 1)
		return false;

	// Enough data to be Position2d?
	if(currentTokens.size() < 13)
		return false;

	// Parse in position data, fail on parse error
	position.pose.x = currentTokens.at(7).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.pose.y = currentTokens.at(8).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.pose.a = currentTokens.at(9).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.velocity.x = currentTokens.at(10).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.velocity.y = currentTokens.at(11).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.velocity.a = currentTokens.at(12).toDouble(&ok);
	RETURN_IFEQ(ok, false, false);

	position.stall = currentTokens.at(13).toInt(&ok);
	RETURN_IFEQ(ok, false, false);

	// Emit signal
	PositionEntry* position_emit = new PositionEntry;
	*position_emit = position;
	position_emit->hdr = currentHeader;
	emit positionDataAvail(position_emit);

	return true;
}

bool LogParser::parseLidar()
{
	LidarEntry lidar;
	Position p;
	uint32_t numPoints = 0;
	double angularRes = 0;
	bool ok;

	// Message Type 1 only
	if(currentHeader.type != 1)
		return false;

	// Subtype 1: PLAYER_LASER_DATA_SCAN
	if(currentHeader.subtype == 1) {

		if(currentTokens.size() < 13)
			return false;

		angularRes = currentTokens.at(10).toDouble(&ok);
		RETURN_IFEQ(ok, false, false);

		numPoints = currentTokens.at(12).toUInt(&ok);
		RETURN_IFEQ(ok, false, false);

		if(numPoints == 0 || angularRes == 0)
			return false;

		// Read in each point
		for(uint32_t i = 0; i < numPoints; i++) {
			double angle = (angularRes * i) - M_PI_2;
			double range = currentTokens.at(2*i + 13).toDouble(&ok);

			if(range < 8.1){
				Point* rangePoint = new Point;
				rangePoint->fromPolar(angle, range);
				if(ok) {
					lidar.points.append(rangePoint);
				}
			}
		}

		// No position given
		lidar.pose.x = lidar.pose.y = lidar.pose.a = 0;

	// Subtype 2: PLAYER_LASER_DATA_SCAN
	} else if(currentHeader.subtype == 2) {

		if(currentTokens.size() < 15)
			return false;

		// Parse position
		p.x = currentTokens.at(8).toDouble(&ok);
		RETURN_IFEQ(ok, false, false);
		p.y = currentTokens.at(9).toDouble(&ok);
		RETURN_IFEQ(ok, false, false);		
		p.a = currentTokens.at(10).toDouble(&ok);
		RETURN_IFEQ(ok, false, false);

		// Copy valid position over
		lidar.pose = p;

		angularRes = currentTokens.at(13).toDouble(&ok);
		RETURN_IFEQ(ok, false, false);

		numPoints = currentTokens.at(15).toUInt(&ok);
		RETURN_IFEQ(ok, false, false);

		if(numPoints == 0 || angularRes == 0)
			return false;

		// Read in each point
		for(uint32_t i = 0; i < numPoints; i++) {
			double angle = angularRes * i;
			double range = currentTokens.at(2*i + 13).toDouble(&ok);
			if(ok) {
				Point* rangePoint = new Point;
				rangePoint->fromPolar(angle, range);
				lidar.points.append(rangePoint);
			}
		}
	}

	// Emit signal
	LidarEntry* lidar_emit = new LidarEntry;
	*lidar_emit = lidar;
	lidar_emit->hdr = currentHeader;
	emit lidarDataAvail(lidar_emit);

	return true;
}

