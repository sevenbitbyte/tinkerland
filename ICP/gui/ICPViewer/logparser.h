#ifndef LOGPARSER_H
#define LOGPARSER_H

#include "point.h"
#include <inttypes.h>
#include <QtCore>

struct EntryHeader{
	QDateTime time;
	uint32_t host;
	uint32_t robot;
	QString interface;
	uint32_t index;
	uint32_t type;
	uint32_t subtype;
};

struct LidarEntry{
	EntryHeader hdr;
	Position pose;
	QVector<Point*> points;
};

struct PositionEntry{
	EntryHeader hdr;
	Position pose;
	Position velocity;
	int stall;
};


#define MIN_HDR_TOKENS (7)

#define RETURN_IFEQ(a,b,c) if(a==b){return c;}

class LogParser : public QObject {
	Q_OBJECT

	public:
		LogParser();
		LogParser(QString path);
		~LogParser();

		bool openLog(QString path);

		/**
		  *	Reads a log header, then reads the rest of the entry and emits
		  *	appropriate signals.
		  *
		  *	@return	Returns false when any read error occurs
		  */
		bool readLogEntry();

		EntryHeader* getCurrentHeader();

	//protected:
		bool readHeader();

		int byteSize();

	signals:
		//! Emitted when a position entry is parsed
		void positionDataAvail(PositionEntry*);

		//! Emitted when lidar data is parsed
		void lidarDataAvail(LidarEntry*);

		void pos(int);

		void eof();


	private:
		QFile logFile;
		uint32_t lineCount;
		EntryHeader currentHeader;
		QStringList currentTokens;
		bool parsePosition();
		bool parseLidar();
};

#endif // LOGPARSER_H
