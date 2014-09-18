#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerSiegeObjective.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

IMPLEMENT_CLASS(obj_ServerSiegeObjective, "obj_SiegeObjective", "Object");
AUTOREGISTER_CLASS(obj_ServerSiegeObjective);

SiegeObjectiveMngr gSiegeObjMgr;

obj_ServerSiegeObjective::obj_ServerSiegeObjective()
{
	m_DestructionTimer = 30.0f;
	m_ActivationTimer = 5.0f;

	m_CurrentTimer = 0;

	m_BlueSpawnHashID = -1;
	m_RedSpawnHashID = -1;

	m_RedCP = NULL;
	m_BlueCP = NULL;
}

obj_ServerSiegeObjective::~obj_ServerSiegeObjective()
{
}

BOOL obj_ServerSiegeObjective::Load(const char *fname)
{
  if(!parent::Load(fname)) 
    return FALSE;

  return TRUE;
}

BOOL obj_ServerSiegeObjective::OnCreate()
{
  parent::OnCreate();
  ObjFlags |= OBJFLAG_SkipCastRay;

  Status = SS_ACTIVE;
  NetworkLocal = true;
  NetworkID    = gSiegeObjMgr.RegisterSiegeObjective(this) + NETID_SIEGEOBJ_START;
  r3dOutToLog("SiegeObj %d at %f %f %f\n", NetworkID, GetPosition().x, GetPosition().y, GetPosition().z);

  if(m_BlueSpawnHashID == -1)
	  r3dError("Blue spawn isn't set for siege objective\n");
  if(m_RedSpawnHashID == -1)
	  r3dError("Red spawn isn't set for siege objective\n");

  {
	  GameObject* obj = GameWorld().GetObjectByHash(m_BlueSpawnHashID);
	  if(obj == NULL)
		  r3dError("Blue spawn hash ID doesn't map to any object\n");
	  if(obj->Class->Name != "obj_ControlPoint")
		  r3dError("Blue spawn isn't obj_ControlPoint\n");
	  m_BlueCP = (obj_ServerControlPoint*)obj;
  }
  {
	  GameObject* obj = GameWorld().GetObjectByHash(m_RedSpawnHashID);
	  if(obj == NULL)
		  r3dError("Red spawn hash ID doesn't map to any object\n");
	  if(obj->Class->Name != "obj_ControlPoint")
		  r3dError("Red spawn isn't obj_ControlPoint\n");
	  m_RedCP = (obj_ServerControlPoint*)obj;
  }


  return 1;
}

obj_ServerControlPoint* obj_ServerSiegeObjective::getCP(int teamID)
{
	if(teamID == 0) // blue
	{
		return m_BlueCP;
	}
	else
		return m_RedCP;
}

BOOL obj_ServerSiegeObjective::OnDestroy()
{
  return parent::OnDestroy();
}

BOOL obj_ServerSiegeObjective::Update()
{
  float TimePassed = r3dGetFrameTime();

  if(Status == SS_ARMED)
  {
	  m_CurrentTimer += TimePassed;
	  if(m_CurrentTimer > m_DestructionTimer)
	  {
		  Status = SS_DESTROYED;

		  // enable next objective, if any
		  obj_ServerSiegeObjective* next = gSiegeObjMgr.GetNextObjective(this);
		  if(next)
		  {
			  next->Status = SS_ACTIVE;
			  next->HostSendUpdate();
		  }
	  }
	  HostSendUpdate();
  }

  return TRUE;
}

void obj_ServerSiegeObjective::HostSendUpdate()
{
   PKT_S2C_SiegeUpdate_s n;
   n.status = (BYTE)Status;
   n.destruction_timer = m_CurrentTimer;
   
   gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerSiegeObjective::handleActivateRequest(obj_ServerPlayer* pl)
{
	if(pl->TeamID == 0 && Status == SS_ACTIVE) // blue attacking
	{
		Status = SS_ARMED;
		m_CurrentTimer = 0;
	}
	if(pl->TeamID == 1 && Status == SS_ARMED) // red defending
	{
		Status = SS_ACTIVE;
		m_CurrentTimer = 0;
	}
}

BOOL obj_ServerSiegeObjective::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
  return TRUE;
}

void obj_ServerSiegeObjective::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("SiegeObj");
	m_DestructionTimer = myNode.attribute("DestructionTimer").as_float();
	m_ActivationTimer = myNode.attribute("ActivationTimer").as_float();
	m_BlueSpawnHashID = myNode.attribute("BlueHashID").as_uint();
	m_RedSpawnHashID = myNode.attribute("RedHashID").as_uint();
}
