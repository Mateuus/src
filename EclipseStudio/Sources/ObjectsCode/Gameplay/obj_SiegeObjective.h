#pragma once

#include "GameCommon.h"
#include "APIScaleformGfx.h"

class obj_SiegeObjective : public GameObject
{
	DECLARE_CLASS(obj_SiegeObjective, GameObject)

	float m_DestructionTimer; // time until it will explode
	float m_ActivationTimer; // time for arming/disarming device
	uint32_t m_BlueSpawnHashID;
	uint32_t m_RedSpawnHashID;

	Scaleform::GFx::Value m_CPIcon;
    char ControlName[32];
	const char* UITagName;
	const char* UITagNameLong;
	int		needToAddToMinimap;

	float	showCaptureProgressTimer;
	bool	CaptureProgressVisible;
	float	m_CurrentTimer;

public:
	enum SIEGE_STATUS
	{
		SS_ACTIVE = 0,
		SS_ARMED,
		SS_DISABLED,
		SS_DESTROYED,
	};
	SIEGE_STATUS Status; 

	obj_SiegeObjective();
	virtual ~obj_SiegeObjective();

	virtual	BOOL		OnCreate();
	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam );

	virtual BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
	virtual BOOL		Update();
	void SetStatusOnMinimap();
	void UpdateBombStatusHUD();

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif
};

class SiegeObjectiveMngr
{
public:
	enum { MAX_SIEGE_OBJECTIVES = 16 };
	obj_SiegeObjective* siegeObjs[MAX_SIEGE_OBJECTIVES];
	int		numSiegeObj;

	int		RegisterSiegeObjective(obj_SiegeObjective* obj) {
		r3d_assert(numSiegeObj < MAX_SIEGE_OBJECTIVES);
		if(numSiegeObj > 1) // lock other
			obj->Status = obj_SiegeObjective::SS_DISABLED;
		siegeObjs[numSiegeObj++] = obj;
		return numSiegeObj - 1;
	}

public:
	SiegeObjectiveMngr() { Reset(); };
	~SiegeObjectiveMngr() {};

	void Reset() {
		numSiegeObj = 0;
	}

	int		NumObjectives() const {
		return numSiegeObj;
	}

	obj_SiegeObjective* GetActiveObjective() {
		for(int i =0; i<numSiegeObj; ++i)
			if(siegeObjs[i]->Status == obj_SiegeObjective::SS_ACTIVE || siegeObjs[i]->Status == obj_SiegeObjective::SS_ARMED)
				return siegeObjs[i];

		return NULL;
	}

	int getObjectiveIndex(obj_SiegeObjective* obj)
	{
		for(int i=0; i<numSiegeObj; ++i)
			if(siegeObjs[i] == obj)
				return i;

		return -1;
	}
};

extern	SiegeObjectiveMngr gSiegeObjMgr;