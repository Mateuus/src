#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerLootBox.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerLootBox, "obj_ServerLootBox", "Object");
AUTOREGISTER_CLASS(obj_ServerLootBox);

const float c_fLootBoxDropLifetime = 60.0f;

obj_ServerLootBox::obj_ServerLootBox()
{
	nextScan_   = 0;
	spawnTime = 0;
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_LootBoxDrop;
}

obj_ServerLootBox::~obj_ServerLootBox()
{
}

BOOL obj_ServerLootBox::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;
	return 1;
}

BOOL obj_ServerLootBox::OnDestroy()
{
	return parent::OnDestroy();
}

bool obj_ServerLootBox::ScanForPlayers()
{
	const float radius = 1.5f;
	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if(plr == NULL)
			continue;

		if(plr->isDead)
			continue;

		// check radius
		float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
		if(distSq > (radius * radius))
			continue;
			
		gServerLogic.GiveItemToPlayer(plr, itemID);

		PKT_S2C_LootBoxPickedUp_s resp;
		resp.itemID = itemID;
		gServerLogic.p2pSendToPeer(plr->peerId_, plr, &resp, sizeof(resp));

		PKT_S2C_DestroyDroppedLootBox_s n;
		n.spawnID = toP2pNetId(NetworkID);
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n));

		return true;
	}

	return false;
}

BOOL obj_ServerLootBox::Update()
{
	float curTime = r3dGetTime();

	if((spawnTime + c_fLootBoxDropLifetime) < curTime || gServerLogic.gameStartCountdown>0) // remove all on round start
	{
		// destroy
		PKT_S2C_DestroyDroppedLootBox_s n;
		n.spawnID = toP2pNetId(NetworkID);
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n));

		return FALSE;
	}

	if(curTime > nextScan_)
	{
		nextScan_ = curTime + 0.1f; // 10 times a sec
		if(ScanForPlayers())
			return FALSE;
	}

	return parent::Update();
}

