#pragma once

#include "GameCommon.h"
#include "../EclipseStudio/Sources/GameCode/UserProfile.h"

class obj_ServerWeaponDrop : public GameObject
{
	DECLARE_CLASS(obj_ServerWeaponDrop, GameObject)
public:
	float	spawnTime;
	uint32_t itemID;
	uint32_t numBullets;
	gobjid_t prevOwner;
	bool	isPermanentDrop;
	wiWeaponAttachment attms;

public:
	obj_ServerWeaponDrop();
	~obj_ServerWeaponDrop();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
};
