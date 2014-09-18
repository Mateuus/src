#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerMine.h"
#include "ObjectsCode/obj_ServerPlayer.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"

#include "ServerGameLogic.h"

IMPLEMENT_CLASS(obj_ServerMine, "obj_ServerMine", "Object");
AUTOREGISTER_CLASS(obj_ServerMine);

obj_ServerMine::obj_ServerMine()
{
	bDestroyed = false;
	autoDeleteTimer = 0.0f;
	destroyOnGameStart = false;
	m_wc = 0;
	ObjTypeFlags = OBJTYPE_Mine;
	m_CreationTime = r3dGetTime();
}

obj_ServerMine::~obj_ServerMine()
{
}

BOOL obj_ServerMine::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerMine::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	if(m_wc == NULL)
		return FALSE;

	checkForMaximumMines();

	if(!gServerLogic.m_isGameHasStarted)
		destroyOnGameStart = true;

	return 1;
}

void obj_ServerMine::checkForMaximumMines()
{
	// check how many mines player has and limit them to 4 mines total
	obj_ServerMine* oldestMine = NULL;
	float oldestCreationTime = 999999999999999.0f;
	int numMines = 0;
	for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_Mine))
		{
			obj_ServerMine* mine = (obj_ServerMine*)obj;
			if(mine->ownerID == ownerID && !mine->bDestroyed)
			{
				numMines++;
				if(mine->m_CreationTime < oldestCreationTime)
				{
					oldestMine = mine;
					oldestCreationTime = mine->m_CreationTime;
				}
			}
		}
	}
	if(numMines>=5)
	{
		oldestMine->onExplode(true);
	}
}

BOOL obj_ServerMine::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerMine::Update()
{
	parent::Update();

	if(gServerLogic.m_isGameHasStarted && destroyOnGameStart)
	{
		onExplode(true);
	}

	
	obj_ServerPlayer* pl = IsServerPlayer(GameWorld().GetObject(ownerID));
	if(pl == NULL)
		onExplode(true);
	else if(pl->isDead)
		onExplode(true);

	if(bDestroyed && autoDeleteTimer == 0.0f)
		autoDeleteTimer = 20.0f; // to make sure that we will not receive a packet for already killed object on the server

	if(autoDeleteTimer > 0)
	{
		autoDeleteTimer -= r3dGetFrameTime();
		if(autoDeleteTimer < 0)
			return FALSE;
	}

	return TRUE;
}

void obj_ServerMine::onExplode(bool silent)
{
	if(bDestroyed)
		return;

	bDestroyed = true;

	if(silent)
	{
		PKT_S2C_DestroyMine_s n;
		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
	}
	else
	{
		PKT_S2C_ExplodeMine_s n;
		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
	}
}

void obj_ServerMine::fillInSpawnData(PKT_S2C_SpawnMine_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = m_wc->m_itemID;
	n.pos = GetPosition();
	n.rot = GetRotationVector();
}

BOOL obj_ServerMine::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
	case PKT_C2S_TriggerMine:
		onExplode(false);
		return TRUE;
	}

	return FALSE;
}

void obj_ServerMine::switchOwner(GameObject* newOwner)
{
	ownerID = newOwner->GetSafeID();
	PKT_S2C_NewOwnerMine_s n;
	n.newOwnerID = toP2pNetId(newOwner->NetworkID);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));

	checkForMaximumMines();
}