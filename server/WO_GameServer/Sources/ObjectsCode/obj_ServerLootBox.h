#pragma once

#include "GameCommon.h"

class obj_ServerLootBox : public GameObject
{
	DECLARE_CLASS(obj_ServerLootBox, GameObject)
private:
	float		nextScan_;
	bool	ScanForPlayers();
public:
	float	spawnTime;
	uint32_t itemID;

public:
	obj_ServerLootBox();
	~obj_ServerLootBox();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
};
