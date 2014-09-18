#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerPermWeaponDrop.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

IMPLEMENT_CLASS(obj_ServerPermWeaponDrop, "obj_PermWeaponDrop", "Object");
AUTOREGISTER_CLASS(obj_ServerPermWeaponDrop);

obj_ServerPermWeaponDrop::obj_ServerPermWeaponDrop()
{
	m_weaponID = 0;
	m_TimeUntilRespawn = -1;
}

obj_ServerPermWeaponDrop::~obj_ServerPermWeaponDrop()
{
}

BOOL obj_ServerPermWeaponDrop::Load(const char *fname)
{
	if(!parent::Load(fname)) 
		return FALSE;

	return TRUE;
}

BOOL obj_ServerPermWeaponDrop::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;
	return 1;
}

BOOL obj_ServerPermWeaponDrop::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ServerPermWeaponDrop::Update()
{
	if(m_TimeUntilRespawn > 0)
	{
		m_TimeUntilRespawn-= r3dGetFrameTime();
		if(m_TimeUntilRespawn <= 0) // drop weapon
		{
			const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_weaponID);
			if(wc)
			{
				gServerLogic.DropWeapon(m_weaponID, GetPosition(), GetRotationVector(), wc->m_numClips*wc->m_clipSize, GetSafeID(), true, attms);
			}
		}
	}
	return parent::Update();
}

void obj_ServerPermWeaponDrop::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("PermWeaponDrop");
	m_weaponID = myNode.attribute("weaponID").as_uint();
	attms.attachments[WPN_ATTM_MUZZLE] = myNode.attribute("attm0").as_uint();
	attms.attachments[WPN_ATTM_UPPER_RAIL] = myNode.attribute("attm1").as_uint();
	attms.attachments[WPN_ATTM_LEFT_RAIL] = myNode.attribute("attm2").as_uint();
	attms.attachments[WPN_ATTM_BOTTOM_RAIL] = myNode.attribute("attm3").as_uint();
	attms.attachments[WPN_ATTM_CLIP] = myNode.attribute("attm4").as_uint();

	m_TimeUntilRespawn = 10.0f; // to make sure that server had enough time to init before we will spawn weapon
}

void obj_ServerPermWeaponDrop::ReturnToPickupArea()
{
	m_TimeUntilRespawn = 60.0f;
}

