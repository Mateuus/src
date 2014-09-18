#pragma once

#include "GameCommon.h"

class obj_ServerAutoTurret: public GameObject
{
	DECLARE_CLASS(obj_ServerAutoTurret, GameObject)
private:
	void		OnNetPacket(const PKT_C2C_PlayerHitNothing_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitStatic_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitStaticPierced_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitDynamic_s& n);
	
public:
	DWORD		peerId_;	// peer id associated with player who created this UAV
	uint32_t itemID;
	bool	requestKill;
	bool	isConsumableVersion;
// gameplay params
	float	Health;
	float   Damage;
	float	Range;

public:
	obj_ServerAutoTurret();
	~obj_ServerAutoTurret();

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	void				fillInSpawnData(PKT_S2C_CreateNetObject_s& n);

	void				DoDamage(float dmg);
};
