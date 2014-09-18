#pragma once

#include "GameCommon.h"

class obj_ServerMine : public GameObject
{
	DECLARE_CLASS(obj_ServerMine, GameObject)

private:
	void checkForMaximumMines();
public:
	obj_ServerMine();
	~obj_ServerMine();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				onExplode(bool silent); // silent true if you need to just destroy mine without explosion and dealing damage

	void				fillInSpawnData(PKT_S2C_SpawnMine_s& n);

	virtual BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	void				switchOwner(GameObject* newOwner);

	const struct WeaponConfig*	m_wc;
	float				m_CreationTime;


	bool				bDestroyed;
	float				autoDeleteTimer;
	bool				destroyOnGameStart;
};
