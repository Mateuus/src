#pragma once

#include "GameCommon.h"

// class to drop permanent weapon drops on server
// client will ignore this object, it is only loaded on server!
class obj_PermWeaponDrop : public GameObject
{
	DECLARE_CLASS(obj_PermWeaponDrop, GameObject)
private:
	uint32_t m_weaponID; // itemID from armory
	uint32_t attm0;
	uint32_t attm1;
	uint32_t attm2;
	uint32_t attm3;
	uint32_t attm4;

public:
	obj_PermWeaponDrop();
	virtual ~obj_PermWeaponDrop();

	virtual	BOOL		OnCreate() ;
	virtual	BOOL		OnDestroy() OVERRIDE ;

	virtual	BOOL		Load(const char *name);

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

	void				DrawWeaponMesh(eRenderStageID DrawState);

	virtual BOOL		Update();

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif
};