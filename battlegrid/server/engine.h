#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "userdata.h"

#include<QtCore>

/*
{
	"object-type": "movedata",
	"uuid": "<SOME_CRAZY_ASCII_NOISE_HERE>"
	"movedata":
	{

		"player-uuid": ""
		"game-uuid": ""		//This
		"data":
		{

		}
	}

}
*/

/*class GameEngine{
	public:
		virtual ~IGameEngine();
		virtual bool addUser(UserData* user);
		virtual bool addOwner(UserData* owner);
		virtual QList<UserData*> gameOwners();

		virtual QList<UserData*> getPlayers();
};*/

enum Object_Types { User_Object=0x01, GameConfig_Object, GameState_Object, GameMove_Object };

class TrackedObject{
	public:
		TrackedObject(QString name, Object_Types objType);

		QUuid getUuid();
		QString getName();
		Object_Types getObjectType();

	private:
		QUuid _uuid;
		QString _name;
		quint16 _objType;
};

enum Player_State {Player_Invited=0x0, Player_Blocked, Player_Joined, Player_Quit};

struct PlayerGameData{
	Player_State state;
	QByteArray* data;
};

class GameState{
	public:
		virtual ~IGameEngine() = 0;
		virtual bool applyMove(IGameMove* move);

		/**
		  *	Returns a list of uuid's of the users who are currently taking a turn
		  */
		virtual QList<QUuid> getTurnList() = 0;
};


class GameConfig : public TrackedObject{
	public:
		GameConfig(QString name, QUuid creator);

		void setValue(QString key, QString value);
		QString getValue(QString key);
		bool containsKey(QString key);

		void addAdminUser(QUuid uid);
		bool removeAdminUser(QUuid uid);

		void addPlayer(QUuid uid);
		Player_State getPlayerState(QUuid uid);

		QList<QUuid> getAdminUsers();
		QList<QUuid> getPlayers();

	protected:
		PlayerGameData* getPlayerData(QUuid user);

		QMap<QString, QString> _settings;
		//QList<GameMove*> _moves;
		QList<QUuid> _moves;

	private:
		QUuid _gameEngine;
		QList<QUuid> _adminUsers;
		QMap<QUuid, PlayerGameData*> _players;
};


class GameMove : public TrackedObject{
	public:
		GameMove(QUuid user, QUuid game, QByteArray* data);
		QByteArray* getMoveData();

	protected:
		QByteArray* _data;
		QUuid _game;
		QUuid _user;
};

typedef QMap<QUuid, GameConfig*> GameConfigMap;

typedef QMap<QUuid, TrackedObject*> TrackedObjectMap;

class ObjectStorage{
	public:
		ObjectStorage* getInstance();

		bool addObject(TrackedObject* obj);
		TrackedObject* getObject(QUuid uid, quint16 objType);

	private:
		ObjectStorage();
		QMap<Object_Types, TrackedObjectMap*> _tables;

		static ObjectStorage* _instance;
};

class IGameEngine{
	public:
		virtual bool applyMove(GameConfig* config, GameMove* move) = 0;

		virtual GameState* getCurrentState() = 0;
};

#endif // GAMEENGINE_H
