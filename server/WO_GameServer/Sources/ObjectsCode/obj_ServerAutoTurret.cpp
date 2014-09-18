#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "obj_ServerAutoTurret.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerAutoTurret, "obj_ServerAutoTurret", "Object");
AUTOREGISTER_CLASS(obj_ServerAutoTurret);

obj_ServerAutoTurret::obj_ServerAutoTurret()
{
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_GameplayItem;
	requestKill = false;
	isConsumableVersion = false;
	Health = 1;
	peerId_    = -1;
}

obj_ServerAutoTurret::~obj_ServerAutoTurret()
{
}

BOOL obj_ServerAutoTurret::OnCreate()
{
	parent::OnCreate();
	r3d_assert(peerId_ >= 0);
	ObjFlags |= OBJFLAG_SkipCastRay;

	if(!isConsumableVersion)
	{
		for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_GameplayItem) && (obj->Class->Name == "obj_ServerAutoTurret"))
			{
				obj_ServerAutoTurret* turret = (obj_ServerAutoTurret*)obj;
				if(turret->ownerID == ownerID && turret!=this && !turret->isConsumableVersion)
				{
					turret->requestKill = true;
				}
			}
		}
	}

	return 1;
}

BOOL obj_ServerAutoTurret::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerAutoTurret::Update()
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

void obj_ServerAutoTurret::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = itemID;
	n.pos = GetPosition();
	n.var1 = GetRotationVector().x;
}

void obj_ServerAutoTurret::DoDamage(float dmg)
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

BOOL obj_ServerAutoTurret::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	//@TODO somehow check that originator of that packet have playerIdx that match peer
	switch(EventID)
	{
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitNothing);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitStatic);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitStaticPierced);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitDynamic);
	}

	return FALSE;
}
#undef DEFINE_GAMEOBJ_PACKET_HANDLER


void obj_ServerAutoTurret::OnNetPacket(const PKT_C2C_PlayerHitStatic_s& n)
{
	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), false);
}

void obj_ServerAutoTurret::OnNetPacket(const PKT_C2C_PlayerHitStaticPierced_s& n)
{
	// just relay packet. not a real hit, just idintication that we pierced some static geometry, will be followed up by real HIT packet
	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), false);
}


void obj_ServerAutoTurret::OnNetPacket(const PKT_C2C_PlayerHitNothing_s& n)
{
	gServerLogic.RelayPacket(peerId_, &n, sizeof(n), false);
}

void obj_ServerAutoTurret::OnNetPacket(const PKT_C2C_PlayerHitDynamic_s& n)
{
	// make sure we're shooting at another player
	GameObject* targetObj = GameWorld().GetNetworkObject(n.targetId);
	if(!targetObj)
	{
		gServerLogic.LogInfo(peerId_, "HitBody0", "not valid targetId");
		return;
	}
	if(obj_ServerPlayer* targetPlr = IsServerPlayer(targetObj))
	{
		//@TODO: check and validate hitting
		//r3dOutToLog("hit from %s to %s\n", fromObj->Name.c_str(), targetObj->Name.c_str()); CLOG_INDENT;

		gServerLogic.RelayPacket(peerId_, &n, sizeof(n), true);

		float damage = Damage; // flat damage for now
		if(gServerLogic.ApplyDamageToPlayer(this, (obj_ServerPlayer*)targetObj, this->GetPosition()+r3dPoint3D(0,1,0), damage, n.hit_body_bone, n.hit_body_part, false, storecat_UsableItem))
		{
			//HACK: track Kill here, because we can't pass weapon ItemID to ApplyDamageToPlayer yet
			//int isKill = ((obj_ServerPlayer*)targetObj)->isDead ? 1 : 0;
			//gServerLogic.TrackWeaponUsage(m_WeaponArray[m_SelectedWeapon]->getConfig()->m_itemID, 0, 1, isKill);
		}
	}
	else 
	{
		if(gServerLogic.CanDamageThisObject(targetObj))
		{
			//r3dOutToLog("hit from %s to %s\n", Name.c_str(), targetObj->Name.c_str()); CLOG_INDENT;

			//@TODO: check and validate hitting

			gServerLogic.RelayPacket(peerId_, &n, sizeof(n), true);

			// apply damage
			float damage = Damage; // flat damage for now
			gServerLogic.ApplyDamage(this, targetObj, this->GetPosition()+r3dPoint3D(0,1,0), damage, false, storecat_UsableItem);
		}
		else
		{
			gServerLogic.LogInfo(peerId_, "HitBody1", "hit object that is not damageable!");
		}
	}
}