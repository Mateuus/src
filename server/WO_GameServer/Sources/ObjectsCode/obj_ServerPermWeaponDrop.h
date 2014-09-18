#pragma once

#include "GameCommon.h"
#include "../EclipseStudio/Sources/GameCode/UserProfile.h"

class obj_ServerPermWeaponDrop : public GameObject
{
	DECLARE_CLASS(obj_ServerPermWeaponDrop, GameObject)
public:
	uint32_t m_weaponID;
	wiWeaponAttachment attms;
	float  m_TimeUntilRespawn;

public:
	obj_ServerPermWeaponDrop();
	~obj_ServerPermWeaponDrop();

	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				ReturnToPickupArea(); // player picked up weapon, spawn new one
};
