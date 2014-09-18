#pragma once

#include "GameCommon.h"

class obj_ServerRiotShield: public GameObject
{
	DECLARE_CLASS(obj_ServerRiotShield, GameObject)
private:
public:
	uint32_t itemID;
	bool	requestKill;
	bool	isConsumableVersion;
	float	Health;

public:
	obj_ServerRiotShield();
	~obj_ServerRiotShield();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				fillInSpawnData(PKT_S2C_CreateNetObject_s& n);

	void				DoDamage(float dmg);
};
