#include "engine.h"

/* Clear singletons */
ObjectStorage* ObjectStorage::_instance = NULL;

/** TrackedObject **/
TrackedObject::TrackedObject(QString name, Object_Types objType){
	_name = name;
	_objType = objType;
	_uuid = QUuid::createUuid();
}

QUuid TrackedObject::getUuid(){
	return _uuid;
}

QString TrackedObject::getName(){
	return _name;
}

Object_Types TrackedObject::getObjectType(){
	return _objType;
}
/** TrackedObject **/


/** GameConfig **/
GameConfig::GameConfig(QString name, QUuid creator){
	_name = name;
	_adminUsers.push_back(creator);
}

void GameConfig::setValue(QString key, QString value){
	_settings[key] = value;
}

QString GameConfig::getValue(QString key){
	return _settings[key];
}

bool GameConfig::containsKey(QString key){
	return _settings.contains(key);
}

QList<QUuid> GameConfig::getAdminUsers(){
	return _adminUsers;
}

QList<QUuid> GameConfig::getPlayers(){
	return _players.keys();
}

/** GameConfig **/


/** ObjectStorage **/
ObjectStorage::ObjectStorage(){

}

ObjectStorage* ObjectStorage::getInstance(){
	if(_instance == NULL){
		_instance = new ObjectStorage();
	}

	return _instance;
}

/**
  *	Adds a tracked object to the object storage engine
  *
  *	@param	obj	TrackedObject to store
  *	@return	Returns true if the object has been added to storage. Returns false
  *			if an error was encountered
  */
bool ObjectStorage::addObject(TrackedObject* obj){
	if(obj != NULL){
		Object_Types type = obj->getObjectType();

		QMap<Object_Types, TrackedObjectMap*>::iterator iter = _tables.find(type);

		TrackedObjectMap* objMap = NULL;

		if(iter == _tables.end()){
			objMap = new TrackedObjectMap();
			_tables.insert(type, objMap);
		}
		else{
			objMap = iter.value();
		}

		QUuid uid = obj->getUuid();
		TrackedObjectMap::iterator valueIter = objMap->find(uid);

		if(valueIter != objMap->end()){
			objMap[uid] = obj;
		}
		else{
			objMap->insert(uid, obj);
		}

		return true;
	}

	return false;
}

/**
  *	Lookup a tracked object by UUID and object type
  *
  *	@param	uid		Unique ID of desired object
  *	@param	objType	Type code of the desired object
  *
  */
TrackedObject* ObjectStorage::getObject(QUuid uid, quint16 objType){
	TrackedObject* obj = NULL;

	TrackedObjectMap* objMap = NULL;

	QMap<Object_Types, TrackedObjectMap*>::iterator iter = _tables.find(objType);
	if(iter != _tables.end()){
		//Found a map
		objMap = iter.value();

		obj = objMap[uid];
	}

	return obj;
}
/** ObjectStorage **/
