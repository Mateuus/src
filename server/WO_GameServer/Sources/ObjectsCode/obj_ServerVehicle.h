#pragma once

#include "GameCommon.h"
#include "multiplayer/NetCellMover.h"

class obj_ServerPlayer;
class obj_ServerVehicleSpawn;

class obj_ServerVehicle : public GameObject
{
	DECLARE_CLASS(obj_ServerVehicle, GameObject)

public:
	// this is a sanity test case in most situation. 
	enum EVehicleState {
		Vehicle_Active,
		Vehicle_Damaged,
		Vehicle_Killed,
	};
	EVehicleState	state_;
	
	DWORD		peerId_;	// peer id associated with player who created this Vehicle
	float		Health;

	CNetCellMover	netMover;
	float		turnAngle;

	obj_ServerVehicleSpawn * spawner;
	
	virtual	BOOL OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
	void		 OnNetPacket(const PKT_C2C_MoveSetCell_s& n);
	void		 OnNetPacket(const PKT_C2C_MoveRel_s& n);
	
	void		fillInSpawnData(PKT_S2C_CreateNetObject_s& n) 
	{
		n.spawnID  = toP2pNetId(NetworkID);
		n.pos      = GetPosition();
		n.var1     = GetRotationVector().x;
		n.var2 = netMover.SrvGetCell().x;
		n.var3 = netMover.SrvGetCell().y;
		n.var4 = netMover.SrvGetCell().z;
		n.var5    = 2;
	}

	void		DoDestroy(DWORD killerNetworkID);
	void		DoDamage(float damage, DWORD killerNetworkID);

	void		ResetHealth() { Health = 450; }

public:
	obj_ServerVehicle();
	~obj_ServerVehicle();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
	void				setSpawner( obj_ServerVehicleSpawn* newSpawner);;
};
