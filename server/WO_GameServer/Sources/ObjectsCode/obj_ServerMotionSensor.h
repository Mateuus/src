#pragma once

#include "GameCommon.h"

class obj_ServerMotionSensor : public GameObject
{
	DECLARE_CLASS(obj_ServerMotionSensor, GameObject)
private:
public:
	uint32_t itemID;
	bool	requestKill;
	bool	isConsumableVersion;
	float	timeCreated;

public:
	obj_ServerMotionSensor();
	~obj_ServerMotionSensor();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				fillInSpawnData(PKT_S2C_CreateNetObject_s& n);
};
