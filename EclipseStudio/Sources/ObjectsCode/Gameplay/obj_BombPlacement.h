#pragma once

#include "GameCommon.h"

class obj_BombPlacement : public GameObject
{
public:
	enum BOMB_STATUS
	{
		SS_EMPTY = 0,
		SS_ARMED,
		SS_DESTROYED,
	};
	DECLARE_CLASS(obj_BombPlacement, GameObject)
private:
	char ControlName[32];
	const char* UITagName;
	int		needToAddToMinimap;
	int		bombSoundID;
	float	timeUntilPlayBeep;
	float	timeWhenArmed;

	BOMB_STATUS Status; 

public:
	const char* getUITagName() const { return UITagName; }

	float m_DestructionTimer; // time until it will explode
	float m_ArmingTimer; // time for arming/disarming device
	float m_DisarmingTimer;
	float m_ActivationRadius; // radius in which you can place bomb (but to disarm you have to be on top of the actual bomb)

	float	m_CurrentTimer;

	r3dPoint3D	m_satchelChargePos;

	r3dMesh* m_satchelChargeMesh;

	obj_BombPlacement();
	virtual ~obj_BombPlacement();

	void	setStatus(BOMB_STATUS s);
	BOMB_STATUS getStatus() const { return Status; }

	float	getActivationRadius() const { return m_ActivationRadius;}

	virtual	BOOL		OnCreate();
	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

	void				DrawSatchelMesh(eRenderStageID DrawState);

	virtual BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
	virtual BOOL		Update();

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif

};


class BombPlacementMngr
{
public:
	enum { MAX_BOMB_PLACEMENTS = 8 };
	obj_BombPlacement* bombObjs[MAX_BOMB_PLACEMENTS];
	int		numObj;

	void RegisterBombPlacement(obj_BombPlacement* obj) {
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

	obj_BombPlacement* GetBombPlacement(int idx) {
		r3d_assert(idx >= 0 && idx < numObj);
		return bombObjs[idx];
	}
};

extern	BombPlacementMngr gBombPlacementMgr;