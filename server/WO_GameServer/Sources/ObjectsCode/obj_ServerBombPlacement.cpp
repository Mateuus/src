#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerBombPlacement.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ObjectsCode/WEAPONS/WeaponConfig.h"

#include "ServerGameLogic.h"

IMPLEMENT_CLASS(obj_ServerBombPlacement, "obj_BombPlacement", "Object");
AUTOREGISTER_CLASS(obj_ServerBombPlacement);

BombPlacementMngr gBombPlacementMgr;

obj_ServerBombPlacement::obj_ServerBombPlacement()
{
	m_DestructionTimer = 35.0f;
	m_ArmingTimer = 3.0f;
	m_DisarmingTimer = 10.0f;
	m_ActivationRadius = 2.0f;

	m_CurrentTimer = 0;
}

obj_ServerBombPlacement::~obj_ServerBombPlacement()
{
}

BOOL obj_ServerBombPlacement::Load(const char *fname)
{
	if(!parent::Load(fname)) 
		return FALSE;

	return TRUE;
}

BOOL obj_ServerBombPlacement::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	Status = SS_EMPTY;
	NetworkLocal = true;
	NetworkID    = gServerLogic.net_lastFreeId++;
	r3dOutToLog("BombPlacement %d at %f %f %f\n", NetworkID, GetPosition().x, GetPosition().y, GetPosition().z);

	gBombPlacementMgr.RegisterBombPlacement(this);

	return 1;
}

BOOL obj_ServerBombPlacement::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerBombPlacement::Update()
{
	float TimePassed = r3dGetFrameTime();

	if(Status == SS_ARMED)
	{
		m_CurrentTimer += TimePassed;
		if(m_CurrentTimer > m_DestructionTimer)
		{
			PKT_S2C_Bomb_Exploded_s n;
			gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n), true); // don't say from which bomb placement packet received. part of gameplay that counter terrorists don't know what object was armed

			// allow 5 seconds to wait before closing the round
			Status = SS_DESTROYED; 
			m_CurrentTimer = 0;

			// kill everyone in 30 meter radius
			for(int i=0; i<ServerGameLogic::MAX_NUM_PLAYERS; ++i)
			{
				obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
				if(plr && !plr->isDead)
				{
					if((plr->GetPosition() - m_bombChargePos).Length() < 30.0f)
						gServerLogic.DoKillPlayer(plr, plr, storecat_INVALID, false);
				}
			}
		}
	}
	else if(Status == SS_DESTROYED)
	{
		m_CurrentTimer += TimePassed;
		if(m_CurrentTimer > 3.0f)
		{
			// let server know that terrorists won the round
			gServerLogic.Bomb_ReportWinRound(0, true);
			Status = SS_EMPTY; 
			m_CurrentTimer = 0;
		}
	}

	return TRUE;
}

void obj_ServerBombPlacement::HostSendUpdate()
{
}

void obj_ServerBombPlacement::handleActivateRequest(obj_ServerPlayer* pl, const r3dPoint3D& pos)
{
	if(pl->TeamID == 0 && Status == SS_EMPTY && pl->m_carryingBomb) // team 0 - terrorists
	{
		Status = SS_ARMED;
		m_CurrentTimer = 0;

		m_bombChargePos = pos;

		PKT_S2C_Bomb_BombPlaced_s n;
		n.pos = pos;
		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n), true); // don't say from which bomb placement packet received. part of gameplay that counter terrorists don't know what object was armed

		pl->m_carryingBomb = false;
		pl->incrementAchievement( ACHIEVEMENT_PLANT_100_BOMBS, 1 );

		gServerLogic.AddPlayerReward(pl, RWD_Bomb_PlantBomb);
	}
	if(pl->TeamID == 1 && Status == SS_ARMED) // team 1 - counter terrorists
	{
		pl->incrementAchievement( ACHIEVEMENT_DEFUSE_100_BOMBS, 1 );
		gServerLogic.AddPlayerReward(pl, RWD_Bomb_DiffuseBomb);

		// if counter terrorist disarm bomb - it means that they won!
		gServerLogic.Bomb_ReportWinRound(1, true);
		Status = SS_EMPTY;
	}
}

BOOL obj_ServerBombPlacement::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	return TRUE;
}

void obj_ServerBombPlacement::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("BombPlaceObj");
	m_DestructionTimer = myNode.attribute("DestructionTimer").as_float();
	m_ArmingTimer = myNode.attribute("ArmingTimer").as_float();
	m_DisarmingTimer = myNode.attribute("DisarmingTimer").as_float();
	m_ActivationRadius = myNode.attribute("ActivationRadius").as_float();
}
