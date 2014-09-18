#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "obj_ServerRiotShield.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerRiotShield, "obj_ServerRiotShield", "Object");
AUTOREGISTER_CLASS(obj_ServerRiotShield);

obj_ServerRiotShield::obj_ServerRiotShield()
{
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_GameplayItem;
	requestKill = false;
	isConsumableVersion = false;
	Health = 1;
}

obj_ServerRiotShield::~obj_ServerRiotShield()
{
}

BOOL obj_ServerRiotShield::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	if(!isConsumableVersion)
	{
		for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_GameplayItem) && (obj->Class->Name == "obj_ServerRiotShield"))
			{
				obj_ServerRiotShield* shield = (obj_ServerRiotShield*)obj;
				if(shield->ownerID == ownerID && shield!=this && !shield->isConsumableVersion)
				{
					shield->requestKill = true;
				}
			}
		}
	}

	return 1;
}

BOOL obj_ServerRiotShield::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerRiotShield::Update()
{
	float curTime = r3dGetTime();

	bool ownerAlive = true;
	// check if owner is still alive, if dead - kill it
	{
		GameObject* owner = GameWorld().GetObject(ownerID);
		if(owner && owner->isObjType(OBJTYPE_Human))
		{
			obj_ServerPlayer* plr = (obj_ServerPlayer*)owner;
			if(!isConsumableVersion) // consumable version stays alive even if owner gets killed
				ownerAlive = !plr->isDead;
		}
		else
			ownerAlive = false;
	}

	if(gServerLogic.gameStartCountdown>0 || !ownerAlive || requestKill) // remove all on round start
	{
		// destroy
		PKT_S2C_DestroyNetObject_s n;
		n.spawnID = toP2pNetId(NetworkID);
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n));

		return FALSE;
	}

	return parent::Update();
}

void obj_ServerRiotShield::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = itemID;
	n.pos = GetPosition();
	n.var1 = GetRotationVector().x;
}

void obj_ServerRiotShield::DoDamage(float dmg)
{
	if(Health > 0)
	{
		Health -= dmg;
		if(Health <= 0.0f)
		{
			requestKill = true;
		}
	}
}