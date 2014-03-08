#ifndef __GRENADE_H__
#define __GRENADE_H__

#include "GameCommon.h"
#include "Ammo.h"
#include "APIScaleformGfx.h"

// data driven class. It is created by Ammo class, for bullets that are not immediate action, for example rockets.
class obj_Grenade : public AmmoShared
{
	DECLARE_CLASS(obj_Grenade, AmmoShared)
	friend Ammo;
private:
	void		onExplode();
	r3dPoint3D	lastCollisionNormal;
	bool		bHadCollided;
	Scaleform::GFx::Value m_GrenadeIcon;
public:
	obj_Grenade();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();
	virtual	void		OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace);
	virtual	BOOL		Update();

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;
};

#endif	// __GRENADE_H__
