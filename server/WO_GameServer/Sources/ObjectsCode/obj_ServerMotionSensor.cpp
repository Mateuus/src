#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "multiplayer/P2PMessages.h"

#include "obj_ServerMotionSensor.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerMotionSensor, "obj_ServerMotionSensor", "Object");
AUTOREGISTER_CLASS(obj_ServerMotionSensor);

obj_ServerMotionSensor::obj_ServerMotionSensor()
{
	itemID = 0;
	ObjTypeFlags |= OBJTYPE_GameplayItem;
	requestKill = false;
	isConsumableVersion = false;
	timeCreated = 0;
}

obj_ServerMotionSensor::~obj_ServerMotionSensor()
{
}

BOOL obj_ServerMotionSensor::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;
	timeCreated = r3dGetTime();

	if(!isConsumableVersion)
	{
		int numSensorsAllowed = 0; // means only 1 sensor at a time
		GameObject* owner = GameWorld().GetObject(ownerID);
		if(owner && owner->isObjType(OBJTYPE_Human))
		{
			obj_ServerPlayer* plr = (obj_ServerPlayer*)owner;
			switch(plr->GetLoadoutData().getSkillLevel(CUserSkills::RECON_MotionSensor))
			{
			case 3:
			case 4:
				numSensorsAllowed = 1; break;
			case 5:
				numSensorsAllowed = 2; break;
			default: break;
			}
		}

		// we need to kill only oldest sensors, so sort them by time
		std::vector<obj_ServerMotionSensor*> sensors;
		for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_GameplayItem) && (obj->Class->Name == "obj_ServerMotionSensor"))
			{
				obj_ServerMotionSensor* sensor = (obj_ServerMotionSensor*)obj;
				if(sensor->ownerID == ownerID && sensor!=this && !sensor->isConsumableVersion)
				{
					sensors.push_back(sensor);
				}
			}
		}
		struct SensorTimeSort {
			static int sort(const void* P1, const void* P2)	{
				const obj_ServerMotionSensor* g1 = *(obj_ServerMotionSensor**)P1;
				const obj_ServerMotionSensor* g2 = *(obj_ServerMotionSensor**)P2;
				return (int)g1->timeCreated < (int)g2->timeCreated;
			}
		};
		if(sensors.size() > 1) 
			qsort(&sensors[0], sensors.size(), sizeof(obj_ServerMotionSensor*), SensorTimeSort::sort);

		for(std::vector<obj_ServerMotionSensor*>::iterator it = sensors.begin(); it!=sensors.end(); ++it)
		{
			obj_ServerMotionSensor* sensor = *it;
			if(numSensorsAllowed == 0)
				sensor->requestKill = true;
			else
				numSensorsAllowed--;
		}
	}

	return 1;
}

BOOL obj_ServerMotionSensor::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerMotionSensor::Update()
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

void obj_ServerMotionSensor::fillInSpawnData(PKT_S2C_CreateNetObject_s& n)
{
	n.spawnID = toP2pNetId(NetworkID);
	n.itemID = itemID;
	n.pos = GetPosition();
}
