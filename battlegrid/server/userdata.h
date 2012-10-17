#ifndef USERDATA_H
#define USERDATA_H

#include<QtCore>

class UserData
{
	public:
		UserData();

	private:
		QString nickname;
		QUuid uid;

};

#endif // USERDATA_H
