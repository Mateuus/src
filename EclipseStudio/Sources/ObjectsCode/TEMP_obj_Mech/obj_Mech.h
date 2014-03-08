#pragma once

#ifndef FINAL_BUILD

enum EMechSlot
{
	MSLOT_Mech1 = 0,
	MSLOT_Mech2,
	MSLOT_Mech3,
	MSLOT_Mech4,
	MSLOT_Weapon1,
	MSLOT_Weapon2,
	MSLOT_Weapon3,
	MSLOT_Max,
};

class CMechUberEquip
{
public:
	// equipped things
	class slot_s
	{
	public:
		float		fOffset;
		int		iIdx;
		std::string	name;
		
		r3dMesh*	mesh;

		slot_s() {
			fOffset = 0;
			iIdx    = -1;
			mesh    = NULL;
		}
	};
	slot_s		slots_[MSLOT_Max];

	enum DrawType
	{
		DT_DEFERRED,
		DT_SHADOWS,
		DT_AURA,
		DT_DEPTH,
	};
	
private:
	void		DrawSlot(int slotId, const D3DXMATRIX& world, DrawType dt, bool skin);

public:
	CMechUberEquip();
	~CMechUberEquip();

	void		LoadSlot(int slotId, const char* fname);

	D3DXMATRIX	getWeaponBone(int idx, const r3dSkeleton* skel, const D3DXMATRIX& offset);

	void		Draw(const r3dSkeleton* skel, const D3DXMATRIX& CharMat, bool draw_weapon, DrawType dt);
	void		DrawSM(const r3dSkeleton* skel, const D3DXMATRIX& CharMat, bool draw_weapon);
	
	void		Reset();
};

class obj_Mech : public GameObject
{
	DECLARE_CLASS(obj_Mech, GameObject)

public:
	std::string	sDir;
	
	CMechUberEquip	uberEquip_;

	char		sSkelSelected[256];
	float		fSkelListOffset;

	r3dSkeleton*	m_BindSkeleton;
	void		ReloadSkeleton();

	char		sAnimSelected[256];
	float		fAnimListOffset;
	
	r3dAnimation*	m_Animation;
	r3dAnimPool	m_AnimPool;

	float		adjBody;
	float		adjWeap1;
	float		adjWeap2;
	float		adjWeap3;
	
	int		boneId_MechPelvis;
	int		boneId_Weapon1;
	int		boneId_Weapon2;
	int		boneId_Weapon3;
	
	int		m_renderSkel;
	
	const D3DXMATRIX& GetMtx() {
		return mTransform;
	}

	
public:
	obj_Mech();
	virtual	~obj_Mech();

	virtual	BOOL	OnCreate();
	virtual	BOOL	Load(const char* fname);
	virtual	BOOL	Update();

	virtual GameObject*	Clone() { r3dError("do not user"); return NULL; }

	virtual	float	DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected ) OVERRIDE;

	virtual void	AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;
	virtual void	AppendShadowRenderables( RenderArray& rarr, const r3dCamera& Cam ) OVERRIDE;
};

#endif
