#pragma once

#include "GameCommon.h"
#include "multiplayer/NetCellMover.h"

class obj_ServerPlayer;

class obj_ServerUAV : public GameObject
{
	DECLARE_CLASS(obj_ServerUAV, GameObject)

public:
	DWORD		peerId_;	// peer id associated with player who created this UAV
	float		Health;
	enum EUAVState {
		UAV_Active,
		UAV_Damaged,
		UAV_Killed,
	};
	EUAVState	state_;

	CNetCellMover	netMover;
	float		turnAngle;

	virtual	BOOL OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
	void		 OnNetPacket(const PKT_C2C_MoveSetCell_s& n);
	void		 OnNetPacket(const PKT_C2C_MoveRel_s& n);

	void		fillInSpawnData(PKT_S2C_CreateNetObject_s& n);

	void		DoDestroy(DWORD killerNetworkID);
	void		DoDamage(float damage, DWORD killerNetworkID);

	void		ResetHealth() { Health = 450; }

public:
	obj_ServerUAV();
	~obj_ServerUAV();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
};
