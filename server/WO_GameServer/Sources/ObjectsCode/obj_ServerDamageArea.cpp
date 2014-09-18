#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerDamageArea.h"
#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

const float c_fAreaDamageInterval = 1.0f; // hard coded to 1 second, as in editor we set damage per second

IMPLEMENT_CLASS(obj_ServerDamageArea, "obj_DamageArea", "Object");
AUTOREGISTER_CLASS(obj_ServerDamageArea);


obj_ServerDamageArea::obj_ServerDamageArea()
{
	nextScan_   = 0;
	m_Radius = 0;
	m_Damage = 0;
	m_TeamID = 2;
}

obj_ServerDamageArea::~obj_ServerDamageArea()
{
}

BOOL obj_ServerDamageArea::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerDamageArea::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	return 1;
}

BOOL obj_ServerDamageArea::OnDestroy()
{
	return parent::OnDestroy();
}

void obj_ServerDamageArea::ScanForPlayers()
{
	const float curTime = r3dGetTime();

	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if(plr == NULL)
			continue;

		// check radius
		float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
		if(distSq > (m_Radius * m_Radius))
			continue;

		if(m_TeamID == 2 || plr->TeamID == m_TeamID) {
			r3dOutToLog("DamageArea %u: %s got %f damage\n", GetHashID(), plr->userName, m_Damage);
			gServerLogic.ApplyDamageToPlayer(plr, plr, plr->GetPosition(), m_Damage, -1, 0, false, storecat_INVALID);
		}
		
		if(plr->TeamID == m_TeamID)
		{
			plr->m_InDamageAreaTime = r3dGetTime() + c_fAreaDamageInterval;
		}
	}

	return;
}

BOOL obj_ServerDamageArea::Update()
{
	const float curTime = r3dGetTime();
	if(curTime > nextScan_)
	{
		nextScan_ = curTime + c_fAreaDamageInterval;
		ScanForPlayers();
	}

	return TRUE;
}

void obj_ServerDamageArea::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node damageAreaNode = node.child("damageArea");
	m_Radius = damageAreaNode.attribute("radius").as_float();
	m_Damage = damageAreaNode.attribute("damage").as_float();
	if(!damageAreaNode.attribute("teamID").empty())
		m_TeamID = damageAreaNode.attribute("teamID").as_int();
}
