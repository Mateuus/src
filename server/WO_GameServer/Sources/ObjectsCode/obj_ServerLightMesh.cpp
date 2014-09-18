#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerLightMesh.h"
#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

IMPLEMENT_CLASS(obj_ServerLightMesh, "obj_LightMesh", "Object");
AUTOREGISTER_CLASS(obj_ServerLightMesh);

obj_ServerLightMesh::obj_ServerLightMesh()
{
	bLightOn = true;
}

obj_ServerLightMesh::~obj_ServerLightMesh()
{
}

BOOL obj_ServerLightMesh::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerLightMesh::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	NetworkID = gServerLogic.net_lastFreeId++;
	NetworkLocal = true;

	return 1;
}

BOOL obj_ServerLightMesh::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerLightMesh::Update()
{
	parent::Update();

	return TRUE;
}

BOOL obj_ServerLightMesh::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	// no packets to process
	return FALSE;
}

void obj_ServerLightMesh::SyncLightStatus(DWORD peerID)
{
	if(bLightOn)
		return;

	// only send this if light is off
	PKT_S2C_LightMeshStatus_s n;
	if(peerID == 0)
		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
	else
		gServerLogic.p2pSendToPeer(peerID, this, &n, sizeof(n));
}