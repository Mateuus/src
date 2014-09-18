#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerVehicle.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

#include "ObjectsCode/obj_ServerVehicleSpawn.h"

//
//
// 	class for Control Crates Objects
//
//

IMPLEMENT_CLASS(obj_ServerVehicle, "obj_ServerVehicle", "Object");
AUTOREGISTER_CLASS(obj_ServerVehicle);

obj_ServerVehicle::obj_ServerVehicle() : 
	netMover(this, 0.2f, (float)PKT_C2C_MoveSetCell_s::VEHICLE_CELL_RADIUS)
{
	ResetHealth();
	state_     = Vehicle_Active;
	peerId_    = -1;
	spawner	   = NULL;
#if VEHICLES_ENABLED
	ObjTypeFlags |= OBJTYPE_Vehicle;
#endif
}

obj_ServerVehicle::~obj_ServerVehicle()
{
}

BOOL obj_ServerVehicle::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerVehicle::OnCreate()
{
	parent::OnCreate();

	r3d_assert(peerId_ >= 0);
	
	netMover.SrvSetCell(GetPosition());
	return 1;
}

BOOL obj_ServerVehicle::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerVehicle::Update()
{
	return TRUE;
}

void obj_ServerVehicle::DoDestroy(DWORD killerNetworkID)
{
	if(state_ == obj_ServerVehicle::Vehicle_Killed)
		return;
		
	state_ = obj_ServerVehicle::Vehicle_Killed;
		
// 	PKT_S2C_VehicleSetState_s n;
// 	n.state    = (BYTE)state_;
// 	n.killerId = gp2pnetid_t(killerNetworkID);
// 	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));

	if( spawner ) {
		spawner->markVehicleDestroyed();
		spawner = NULL;
	}
}

void obj_ServerVehicle::DoDamage(float damage, DWORD killerNetworkID)
{
	if(state_ == obj_ServerVehicle::Vehicle_Killed)
		return;

	Health -= damage;

	r3dOutToLog("Vehicle Damage: %f\n", damage, Health);
	if(Health < 0)
	{
		DoDestroy(killerNetworkID);
		return;
	}
	
	if(Health < 130 && state_ != Vehicle_Damaged)
	{
		state_ = Vehicle_Damaged;
		
// 		PKT_S2C_VehicleSetState_s n;
// 		n.state    = (BYTE)state_;
// 		n.killerId = gp2pnetid_t(killerNetworkID);
// 		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
	}
	
	return;
}

void obj_ServerVehicle::OnNetPacket(const PKT_C2C_MoveSetCell_s& n)
{
	if(state_ == Vehicle_Killed)
		return;

	netMover.SetCell(n);

	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), true);
}

void obj_ServerVehicle::OnNetPacket(const PKT_C2C_MoveRel_s& n)
{
	if(state_ == Vehicle_Killed)
		return;

	const CNetCellMover::moveData_s& md = netMover.DecodeMove(n);
	SetPosition(md.pos);
	SetRotationVector(r3dPoint3D(md.turnAngle, 0, 0));
	
	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), false);
}


#undef DEFINE_GAMEOBJ_PACKET_HANDLER
#define DEFINE_GAMEOBJ_PACKET_HANDLER(xxx) \
	case xxx: { \
		const xxx##_s&n = *(xxx##_s*)packetData; \
		if(packetSize != sizeof(n)) { \
			r3dOutToLog("!!!!errror!!!! %s packetSize %d != %d\n", #xxx, packetSize, sizeof(n)); \
			return TRUE; \
		} \
		OnNetPacket(n); \
		return TRUE; \
	}

BOOL obj_ServerVehicle::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveSetCell);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveRel);
	}
  
	return FALSE;
}

void obj_ServerVehicle::setSpawner( obj_ServerVehicleSpawn* newSpawner )
{
	spawner = newSpawner;
}

#undef DEFINE_GAMEOBJ_PACKET_HANDLER
