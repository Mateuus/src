#ifndef __DROPPED_WEAPON_H__
#define __DROPPED_WEAPON_H__

#include "GameCommon.h"
#include "..\..\GameCode\UserProfile.h"

class DroppedWeapon : public GameObject
{
	DECLARE_CLASS(DroppedWeapon, GameObject)
public:
	DroppedWeapon();
	virtual ~DroppedWeapon();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*GetObjectMesh();
	virtual r3dMesh*GetObjectLodMesh() OVERRIDE;

	uint32_t			m_WeaponItemID;
	uint32_t			m_NumBullets;
	wiWeaponAttachment	m_Attms;
	bool		m_IsPicked;

	bool		m_isGood;

private:
	r3dMesh* m_PrivateModel;
};

#endif __DROPPED_WEAPON_H__