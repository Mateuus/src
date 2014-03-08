#ifndef __ROCKET_H__
#define __ROCKET_H__

#include "GameCommon.h"
#include "Ammo.h"

class obj_Rocket : public AmmoShared
{
	DECLARE_CLASS(obj_Rocket, AmmoShared)
	friend Ammo;
	r3dPoint3D	m_CollisionPoint;
	r3dPoint3D  m_CollisionNormal;
	//bool		m_hasCollision;
	float		m_DistanceTraveled;
	bool		m_isFlying;
	bool		m_DisableDistanceTraveled;

	void		onHit(const r3dPoint3D& pos, const r3dPoint3D& norm, const r3dPoint3D& fly_dir);
public:
	obj_Rocket();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();
	virtual	void		OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace);
	virtual	BOOL		Update();

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;
};

// same as rocket, but doesn't have distance traveled requirement
class obj_RocketGrenade : public obj_Rocket
{
	DECLARE_CLASS(obj_RocketGrenade, obj_Rocket)
public:
	obj_RocketGrenade() { m_DisableDistanceTraveled = true; }
};

#endif	// __ROCKET_H__
