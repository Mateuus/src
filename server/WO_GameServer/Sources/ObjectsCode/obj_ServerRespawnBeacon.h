#pragma once

#include "GameCommon.h"

class obj_ServerRespawnBeacon : public GameObject
{
	DECLARE_CLASS(obj_ServerRespawnBeacon, GameObject)
private:
public:
	uint32_t itemID;
	bool	requestKill;
	bool	isConsumableVersion;

public:
	obj_ServerRespawnBeacon();
	~obj_ServerRespawnBeacon();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				fillInSpawnData(PKT_S2C_CreateNetObject_s& n);
};
