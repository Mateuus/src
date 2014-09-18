#pragma once

#include "GameCommon.h"

class obj_ServerPlayer;

class obj_ServerDamageArea : public GameObject
{
	DECLARE_CLASS(obj_ServerDamageArea, GameObject)

private:
	float		nextScan_;
	float		m_Radius;
	float		m_Damage;
	int			m_TeamID; // 0 - blue, 1 - red, 2 - disabled
	void		ScanForPlayers();

public:
	obj_ServerDamageArea();
	~obj_ServerDamageArea();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	virtual	void		ReadSerializedData(pugi::xml_node& node);
};
