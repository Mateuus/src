#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "obj_ServerMedKit.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerMedKit, "obj_ServerMedKit", "Object");
AUTOREGISTER_CLASS(obj_ServerMedKit);

obj_ServerMedKit::obj_ServerMedKit()
{
	nextScan_   = 0;
	spawnTime = 0;
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_GameplayItem;
	requestKill = false;
}

obj_ServerMedKit::~obj_ServerMedKit()
{
}

BOOL obj_ServerMedKit::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	// check if same owner has another medkit spawned, and if yes, kill it
	for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_GameplayItem) && (obj->Class->Name == "obj_ServerMedKit"))
		{
			obj_ServerMedKit* medkit = (obj_ServerMedKit*)obj;
			if(medkit->ownerID == ownerID && medkit!=this)
			{
				medkit->requestKill = true;
			}
		}
	}
	
	return 1;
}

BOOL obj_ServerMedKit::OnDestroy()
{
	return parent::OnDestroy();
}

void obj_ServerMedKit::ScanForPlayers()
{
	const float radius = 7.0f;
	obj_ServerPlayer* plrOwner = NULL;
	{
		GameObject* owner = GameWorld().GetObject(ownerID);
		if(owner && owner->isObjType(OBJTYPE_Human))
			plrOwner = (obj_ServerPlayer*)owner;
	}
	
	if(!plrOwner)
		return;

	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if(plr == NULL)
			continue;

		if(plr->isDead || plr->TeamID != teamID)
			continue;

		// check radius
		float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
		if(distSq > (radius * radius))
			continue;

		if(plr->m_Health < plr->getMaxHealth())
		{
			extern CGamePlayParams		GPP_Data;
			//!!! this code is duplicated on client !!!
			float healthNutBoost = 1.0f;
			switch(plr->GetLoadoutData().getSkillLevel(CUserSkills::RECON_HealthNut))
			{
			case 1: healthNutBoost = 1.1f; break;
			case 2: healthNutBoost = 1.2f; break;
			case 3: healthNutBoost = 1.3f; break;
			}
			float healingSpeed = 1.0f;
			switch(plrOwner->GetLoadoutData().getSkillLevel(CUserSkills::MEDIC_HealingSpeed))
			{
			case 1: healingSpeed = 1.1f; break;
			case 2: healingSpeed = 1.2f; break;
			case 3: healingSpeed = 1.3f; break;
			}

			plr->m_Health += GPP_Data.c_fRegenSpeed * 0.1f * 2 * healthNutBoost * healingSpeed; // 0.1f - rate of update. design: The medkit emits heals at 2x the players base regeneration rate
			plr->m_Health = R3D_MIN(plr->m_Health, plr->getMaxHealth());
		}
	}
}

BOOL obj_ServerMedKit::Update()
{
	float curTime = r3dGetTime();

	bool ownerAlive = true;
	// check if owner is still alive, if dead - kill medkit
	{
		GameObject* owner = GameWorld().GetObject(ownerID);
		if(owner && owner->isObjType(OBJTYPE_Human))
		{
			obj_ServerPlayer* plr = (obj_ServerPlayer*)owner;
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

	if(curTime > nextScan_)
	{
		nextScan_ = curTime + 0.1f; // 10 times a sec
		ScanForPlayers();
	}
	return parent::Update();
}

void obj_ServerMedKit::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = itemID;
	n.pos = GetPosition();
	n.var1 = (float)teamID;
}
