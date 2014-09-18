#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "obj_ServerRespawnBeacon.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerRespawnBeacon, "obj_ServerRespawnBeacon", "Object");
AUTOREGISTER_CLASS(obj_ServerRespawnBeacon);

obj_ServerRespawnBeacon::obj_ServerRespawnBeacon()
{
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_GameplayItem;
	requestKill = false;
	isConsumableVersion = false;
}

obj_ServerRespawnBeacon::~obj_ServerRespawnBeacon()
{
}

BOOL obj_ServerRespawnBeacon::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	// only one of each allowed
	for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_GameplayItem) && (obj->Class->Name == "obj_ServerRespawnBeacon"))
		{
			obj_ServerRespawnBeacon* sensor = (obj_ServerRespawnBeacon*)obj;
			if(sensor->ownerID == ownerID && sensor!=this && sensor->isConsumableVersion==isConsumableVersion)
			{
				sensor->requestKill = true;
			}
		}
	}

	return 1;
}

BOOL obj_ServerRespawnBeacon::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerRespawnBeacon::Update()
{
	float curTime = r3dGetTime();

	bool ownerExists = true;
	{
		GameObject* owner = GameWorld().GetObject(ownerID);
		if(!owner)
			ownerExists = false;
	}

	if(gServerLogic.gameStartCountdown>0 || !ownerExists || requestKill) // remove all on round start
	{
		// destroy
		PKT_S2C_DestroyNetObject_s n;
		n.spawnID = toP2pNetId(NetworkID);
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n));

		return FALSE;
	}

	return parent::Update();
}

void obj_ServerRespawnBeacon::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = itemID;
	n.pos = GetPosition();
	n.var1 = GetRotationVector().x;
}
