#pragma once

#include "GameCommon.h"

class obj_ServerControlPoint;
class obj_ServerSiegeObjective : public GameObject
{
	DECLARE_CLASS(obj_ServerSiegeObjective, GameObject)

public:
	void		HostSendUpdate();

	float m_DestructionTimer; // time until it will explode
	float m_ActivationTimer; // time for arming/disarming device
	uint32_t m_BlueSpawnHashID;
	uint32_t m_RedSpawnHashID;

	float m_CurrentTimer; // for destruction

	obj_ServerControlPoint* m_BlueCP;
	obj_ServerControlPoint* m_RedCP;

	enum SIEGE_STATUS
	{
		SS_ACTIVE = 0,
		SS_ARMED,
		SS_DISABLED,
		SS_DESTROYED,
	};
	SIEGE_STATUS Status; 

	obj_ServerControlPoint* getCP(int teamID);

public:
	obj_ServerSiegeObjective();
	~obj_ServerSiegeObjective();

	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	void			handleActivateRequest(class obj_ServerPlayer* pl);
};

class SiegeObjectiveMngr
{
public:
	enum { MAX_SIEGE_OBJECTIVES = 16 };
	obj_ServerSiegeObjective* siegeObjs[MAX_SIEGE_OBJECTIVES];
	int		numSiegeObj;

	int		RegisterSiegeObjective(obj_ServerSiegeObjective* obj) {
		r3d_assert(numSiegeObj < MAX_SIEGE_OBJECTIVES);
		if(numSiegeObj >= 1) // lock other
			obj->Status = obj_ServerSiegeObjective::SS_DISABLED;
		siegeObjs[numSiegeObj++] = obj;
		return numSiegeObj - 1;
	}

public:
	SiegeObjectiveMngr() { Reset(); };
	~SiegeObjectiveMngr() {};

	void Reset() {
		numSiegeObj = 0;
	}

	void CheckForCorrectObjectives()
	{
		if(numSiegeObj == 0)
			r3dError("No siege objectives!\n");

		for(int i=0; i<numSiegeObj; ++i)
		{
			if(siegeObjs[i]->m_BlueCP == NULL)
				r3dError("Siege obj '%d', no blue spawn\n", i);
			if(siegeObjs[i]->m_RedCP == NULL)
				r3dError("Siege obj '%d', no red spawn\n", i);
		}
	}

	int		NumObjectives() const {
		return numSiegeObj;
	}

	obj_ServerSiegeObjective* GetObjective(int idx) {
		r3d_assert(idx >= 0 && idx < numSiegeObj);
		return siegeObjs[idx];
	}

	obj_ServerSiegeObjective* GetActiveObjective() {
		for(int i =0; i<numSiegeObj; ++i)
			if(siegeObjs[i]->Status == obj_ServerSiegeObjective::SS_ACTIVE || siegeObjs[i]->Status == obj_ServerSiegeObjective::SS_ARMED)
				return siegeObjs[i];

		return NULL;
	}

	obj_ServerSiegeObjective* GetNextObjective(obj_ServerSiegeObjective* obj)
	{
		for(int i =0; i<numSiegeObj-1; ++i)
			if(siegeObjs[i] == obj)
				return siegeObjs[i+1];
		return NULL;
	}
};

extern	SiegeObjectiveMngr gSiegeObjMgr;