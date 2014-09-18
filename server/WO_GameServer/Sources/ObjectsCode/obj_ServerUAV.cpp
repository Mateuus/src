#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"


#include "obj_ServerUAV.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

//
//
// 	class for Control Crates Objects
//
//

IMPLEMENT_CLASS(obj_ServerUAV, "obj_ServerUAV", "Object");
AUTOREGISTER_CLASS(obj_ServerUAV);

obj_ServerUAV::obj_ServerUAV() : 
	netMover(this, 0.2f, (float)PKT_C2C_MoveSetCell_s::UAV_CELL_RADIUS)
{
	ResetHealth();
	state_     = UAV_Active;
	peerId_    = -1;
}

obj_ServerUAV::~obj_ServerUAV()
{
}

BOOL obj_ServerUAV::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerUAV::OnCreate()
{
	parent::OnCreate();

	r3d_assert(peerId_ >= 0);
	
	netMover.SrvSetCell(GetPosition());
	return 1;
}

BOOL obj_ServerUAV::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerUAV::Update()
{
	return TRUE;
}

void obj_ServerUAV::DoDestroy(DWORD killerNetworkID)
{
	if(state_ == obj_ServerUAV::UAV_Killed)
		return;
		
	state_ = obj_ServerUAV::UAV_Killed;
		
	PKT_S2C_UAVSetState_s n;
	n.state    = (BYTE)state_;
	n.killerId = gp2pnetid_t(killerNetworkID);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));

	//NOTE: UAV object will be always active for whole session.
	//to prevent receiving late move packets from client when uav is killed
}

void obj_ServerUAV::DoDamage(float damage, DWORD killerNetworkID)
{
	if(state_ == obj_ServerUAV::UAV_Killed)
		return;

	Health -= damage;

	r3dOutToLog("UAV Damage: %f\n", damage, Health);
	if(Health < 0)
	{
		DoDestroy(killerNetworkID);
		return;
	}
	
	if(Health < 130 && state_ != UAV_Damaged)
	{
		state_ = UAV_Damaged;
		
		PKT_S2C_UAVSetState_s n;
		n.state    = (BYTE)state_;
		n.killerId = gp2pnetid_t(killerNetworkID);
		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
	}
	
	return;
}

void obj_ServerUAV::OnNetPacket(const PKT_C2C_MoveSetCell_s& n)
{
	if(state_ == UAV_Killed)
		return;

	netMover.SetCell(n);

	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), true);
}

void obj_ServerUAV::OnNetPacket(const PKT_C2C_MoveRel_s& n)
{
	if(state_ == UAV_Killed)
		return;

	if(gServerLogic.GetPeer(peerId_).player)
		gServerLogic.GetPeer(peerId_).player->lastPlayerAction_ = r3dGetTime();

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

BOOL obj_ServerUAV::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveSetCell);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveRel);
	}
  
	return FALSE;
}
#undef DEFINE_GAMEOBJ_PACKET_HANDLER



void obj_ServerUAV::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID  = toP2pNetId(NetworkID);
	n.itemID = WeaponConfig::ITEMID_Cypher2;
	n.pos      = GetPosition();
	n.var1     = GetRotationVector().x;
	r3dPoint3D cellPos = netMover.SrvGetCell();
	n.var2 = cellPos.x;
	n.var3 = cellPos.y;
	n.var4 = cellPos.z;
	n.var5 = (float)state_;
}

