#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerVehicleSpawn.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

#include "ObjectsCode/obj_ServerVehicle.h"

IMPLEMENT_CLASS(obj_ServerVehicleSpawn, "obj_VehicleSpawn", "Object");
AUTOREGISTER_CLASS(obj_ServerVehicleSpawn);

#define TIME_TO_RESPAWN_VEHICLE 15.0f

obj_ServerVehicleSpawn::obj_ServerVehicleSpawn()
{
	spawnedVehicle = NULL;
	m_TimeUntilRespawn = -1;
}

obj_ServerVehicleSpawn::~obj_ServerVehicleSpawn()
{
}

BOOL obj_ServerVehicleSpawn::Load(const char *fname)
{
	if(!parent::Load(fname)) 
		return FALSE;

	return TRUE;
}

BOOL obj_ServerVehicleSpawn::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;
	return 1;
}

BOOL obj_ServerVehicleSpawn::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerVehicleSpawn::Update()
{
	if(m_TimeUntilRespawn > 0)
	{
		m_TimeUntilRespawn-= r3dGetFrameTime();
		if(m_TimeUntilRespawn <= 0) // drop weapon
		{
			RespawnCar();
		}
	}
	
	return parent::Update();
}

void obj_ServerVehicleSpawn::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("VehicleSpawn");
	_snprintf_s( vehicle_Model, 64, "%s", myNode.attribute("VehicleModel").value());
	
	m_TimeUntilRespawn = 0.1f; // wait til the map is loaded to spawn a vehicle (so we can get a valid network ID)
}

void obj_ServerVehicleSpawn::RespawnCar()
{

	r3d_assert( spawnedVehicle == NULL);

#if VEHICLES_ENABLED
	if( vehicle_Model[0] != '\0' ) {
		spawnedVehicle = static_cast<obj_ServerVehicle*> ( srv_CreateGameObject("obj_ServerVehicle", vehicle_Model, GetPosition()));
		if (spawnedVehicle ) 
		{
			spawnedVehicle->setSpawner( this );
			PKT_S2C_CreateNetObject_s packet;
			spawnedVehicle->fillInSpawnData(packet);
			gServerLogic.p2pBroadcastToActive(NULL, &packet, sizeof(packet), true);
			spawnedVehicle->NetworkID = gServerLogic.net_lastFreeId++;

		} 
		
	}
#endif
}

void obj_ServerVehicleSpawn::markVehicleDestroyed()
{
	m_TimeUntilRespawn = TIME_TO_RESPAWN_VEHICLE;
}
