#pragma once

#include "GameCommon.h"

class obj_ServerVehicle;

class obj_ServerVehicleSpawn : public GameObject
{
	DECLARE_CLASS(obj_ServerVehicleSpawn, GameObject)
public:
	float  m_TimeUntilRespawn;

	char				vehicle_Model[64];
	obj_ServerVehicle*		spawnedVehicle;
	void				RespawnCar();
public:
	obj_ServerVehicleSpawn();
	~obj_ServerVehicleSpawn();

	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
	void				markVehicleDestroyed();
	
};
