#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerWeaponDrop.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerWeaponDrop, "obj_ServerWeaponDrop", "Object");
AUTOREGISTER_CLASS(obj_ServerWeaponDrop);

const float c_fWeaponDropLifetime = 30.0f;

obj_ServerWeaponDrop::obj_ServerWeaponDrop()
{
	spawnTime = 0;
	itemID = 0;
	numBullets = 0;
	prevOwner = invalidGameObjectID;
	isPermanentDrop = false;
	ObjTypeFlags |= OBJTYPE_WeaponDrop;
}

obj_ServerWeaponDrop::~obj_ServerWeaponDrop()
{
}

BOOL obj_ServerWeaponDrop::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;
	return 1;
}

BOOL obj_ServerWeaponDrop::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerWeaponDrop::Update()
{
	float curTime = r3dGetTime();
	if(((spawnTime + c_fWeaponDropLifetime) < curTime || gServerLogic.gameStartCountdown>0) && !isPermanentDrop) // remove all dropped guns on round start
	{
		// destroy
		PKT_S2C_DestroyDroppedWeapon_s n;
		n.spawnID = toP2pNetId(NetworkID);
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n));

		return FALSE;
	}

	return parent::Update();
}

