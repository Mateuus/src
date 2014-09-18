#pragma once

#include "GameCommon.h"

class obj_ServerBombPlacement : public GameObject
{
	DECLARE_CLASS(obj_ServerBombPlacement, GameObject)

public:
	void		HostSendUpdate();

	float m_DestructionTimer; // time until it will explode
	float m_ArmingTimer; // time for arming/disarming device
	float m_DisarmingTimer;
	float m_ActivationRadius; // radius in which you can place bomb (but to disarm you have to be on top of the actual bomb)

	float m_CurrentTimer; // for destruction

	r3dPoint3D	m_bombChargePos;

	enum BOMB_STATUS
	{
		SS_EMPTY = 0,
		SS_ARMED,
		SS_DESTROYED,
	};
	BOMB_STATUS Status; 

public:
	obj_ServerBombPlacement();
	~obj_ServerBombPlacement();

	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	void			handleActivateRequest(class obj_ServerPlayer* pl, const r3dPoint3D& pos);
};

class BombPlacementMngr
{
public:
	enum { MAX_BOMB_PLACEMENTS = 8 };
	obj_ServerBombPlacement* bombObjs[MAX_BOMB_PLACEMENTS];
	int		numObj;

	void RegisterBombPlacement(obj_ServerBombPlacement* obj) {
		r3d_assert(numObj < MAX_BOMB_PLACEMENTS);
		bombObjs[numObj++] = obj;
	}

public:
	BombPlacementMngr() { Reset(); };
	~BombPlacementMngr() {};

	void Reset() {
		numObj = 0;
	}

	int		NumBombPlacements() const {
		return numObj;
	}

	void CheckData() const {
		if(numObj == 0)
			r3dError("Bomb mode requires bomb placement objects to be present on map!");

		return;
	}

	obj_ServerBombPlacement* GetBombPlacement(int idx) {
		r3d_assert(idx >= 0 && idx < numObj);
		return bombObjs[idx];
	}
};

extern	BombPlacementMngr gBombPlacementMgr;