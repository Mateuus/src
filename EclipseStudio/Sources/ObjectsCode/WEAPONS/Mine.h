#ifndef __MINE_H__
#define __MINE_H__

#include "GameCommon.h"
#include "Ammo.h"
#include "APIScaleformGfx.h"

class obj_Mine : public AmmoShared
{
	DECLARE_CLASS(obj_Mine, AmmoShared)
	friend Ammo;
private:
	void		onExplode();
	Scaleform::GFx::Value m_MineIcon;
	bool		wasTriggered;
	void				switchOwner(GameObject* obj);
public:
	obj_Mine();


	virtual	BOOL		OnCreate() OVERRIDE;
	virtual BOOL		OnDestroy() OVERRIDE;
	virtual void		OnTrigger(PxPairFlag::Enum flag, PhysicsCallbackObject* otherObj) OVERRIDE;
	virtual	BOOL		Update() OVERRIDE;

	virtual BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize) OVERRIDE;

	virtual r3dMesh*	GetObjectMesh() OVERRIDE;
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;
};


#endif //__MINE_H__