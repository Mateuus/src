#pragma once

#include "GameCommon.h"

class obj_ServerMedKit : public GameObject
{
	DECLARE_CLASS(obj_ServerMedKit, GameObject)
private:
	float		nextScan_;
	void	ScanForPlayers();
public:
	float	spawnTime;
	uint32_t itemID;
	uint32_t teamID;
	bool	requestKill;

public:
	obj_ServerMedKit();
	~obj_ServerMedKit();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				fillInSpawnData(PKT_S2C_CreateNetObject_s& n);
};
