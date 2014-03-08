#include "r3dpch.h"
#include "r3d.h"

#include "Mine.h"

#include "r3dDeviceQueue.h"

#include "ObjectsCode\Effects\obj_ParticleSystem.h"
#include "..\world\DecalChief.h"
#include "..\world\MaterialTypes.h"
#include "ExplosionVisualController.h"

#include "multiplayer/P2PMessages.h"

#include "..\..\multiplayer\ClientGameLogic.h"
#include "..\ai\AI_Player.H"

#include "..\AI\AI_Player.H"
#include "WeaponConfig.h"
#include "Weapon.h"

#include "..\..\ui\HUDDisplay.h"
extern HUDDisplay*	hudMain;

IMPLEMENT_CLASS(obj_Mine, "obj_Mine", "Object");
AUTOREGISTER_CLASS(obj_Mine);


#define CLAYMORE_ITEM_ID 101139

obj_Mine::obj_Mine()
{
	m_Ammo = NULL;
	m_Weapon = 0;
	m_ParticleTracer = NULL;
	ObjTypeFlags = OBJTYPE_Mine;
	wasTriggered = false;
}

r3dMesh* obj_Mine::GetObjectMesh()
{
	r3d_assert(m_Ammo);
	return getModel();
}

/*virtual*/
r3dMesh*
obj_Mine::GetObjectLodMesh() /*OVERRIDE*/
{
	r3d_assert(m_Ammo);
	return getModel();
}

BOOL obj_Mine::OnCreate()
{
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(!owner)
		return FALSE;

	m_isSerializable = false;

	ReadPhysicsConfig();
	PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;

	PhysicsConfig.isTrigger = true;
	PhysicsConfig.needBoxCollision = true;

	r3d_assert(m_Ammo);
	r3d_assert(m_Weapon);

	m_CreationTime = r3dGetTime();
	m_CreationPos = GetPosition();

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// create physics object only after setting final position, otherwise PhysX will issue a warning, that static object was moved
	parent::OnCreate();

	UpdateTransform();

	return TRUE;
}

BOOL obj_Mine::OnDestroy()
{
	if(!m_MineIcon.IsUndefined() && hudMain)
		hudMain->deleteScreenIcon(m_MineIcon);	

	return parent::OnDestroy();
}

void obj_Mine::onExplode()
{
	// if owner disappeared, do nothing.
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(owner==NULL)
		return;
	r3d_assert(owner->ObjTypeFlags&OBJTYPE_Human);

	// add decal
	DecalParams params;
	params.Dir		= r3dPoint3D(0,1,0);
	params.Pos		= GetPosition();
	params.TypeID	= GetDecalID( r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource) );
	if( params.TypeID != INVALID_DECAL_ID )
		g_pDecalChief->Add( params );

	D3DXMATRIX rotMatrix = GetRotationMatrix();
	r3dVector forwVector(rotMatrix._31, rotMatrix._32, rotMatrix._33);
	SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource), GetPosition(), forwVector); // for explosion to go in front of bomb

	//	Start radial blur effect
	gExplosionVisualController.AddExplosion(GetPosition(), m_Weapon->m_AmmoArea);

	// do damage only if owner is local player, otherwise we will broadcast damage multiple times
	r3d_assert(owner);
	if(owner->NetworkLocal)
	{
		obj_AI_Player* plr = (obj_AI_Player*)owner;
		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		{
			if(plr->m_Weapons[i] && plr->m_Weapons[i]->getItemID() == m_Weapon->m_itemID)
			{
				if( m_Weapon->m_itemID == CLAYMORE_ITEM_ID ) 
				{
					gClientLogic().ApplyExplosionDamage(GetPosition(), m_Weapon->m_AmmoArea, i, forwVector, 90.0f );
				} 
				else 
				{
					gClientLogic().ApplyExplosionDamage(GetPosition(), m_Weapon->m_AmmoArea, i);
				}
				break;
			}
		}
	}

	setActiveFlag(0);
}

void obj_Mine::OnTrigger(PxPairFlag::Enum flag, PhysicsCallbackObject* otherObj)
{
	const obj_AI_Player* owner = (const obj_AI_Player*)GameWorld().GetObject(ownerID);
	if(!owner)
		return;

	if(wasTriggered)
		return;

	if(otherObj)
	{
		GameObject* gameObj = otherObj->isGameObject();
		if(gameObj && gameObj->ObjTypeFlags & OBJTYPE_Human)
		{
			obj_AI_Player* plr = (obj_AI_Player*)gameObj;

			if( plr->TeamID!=owner->TeamID && !plr->bDead && ((r3dGetTime()-plr->TimeOfLastRespawn)>10.0f) ) // 3 seconds after respawn, otherwise detonates mine if placed over a dead body
			{

				// we need to confirm line of sight. For claymores.
				const r3dPoint3D offsetMinePosition = GetPosition() + r3dPoint3D( 0.0f, .5f, 0.0f);
				const r3dPoint3D offsetPlayerPosition = plr->GetPosition() + r3dPoint3D( 0.0f, .5f, 0.0f);
				const PxVec3 physxMinePosition = PxVec3( offsetMinePosition.x, offsetMinePosition.y, offsetMinePosition.z );
				const PxVec3 physxPlayerPosition = PxVec3( offsetPlayerPosition.x, offsetPlayerPosition.y, offsetPlayerPosition.z );
				PxVec3 physxDirection = physxPlayerPosition - physxMinePosition;
				float mag = physxDirection.magnitude();
				physxDirection.normalize();

				PxRaycastHit hit;
				PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));

				bool hitResult = g_pPhysicsWorld->raycastSingle(physxMinePosition,  physxDirection, mag, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter);

				if( hitResult == false ) 
				{
					PKT_C2S_TriggerMine_s n;
					p2pSendToHost(this, &n, sizeof(n));
					wasTriggered = true;
				}
				
			}
		}
	}
}

void MatrixGetYawPitchRoll ( const D3DXMATRIX & , float &, float &, float & );
BOOL obj_Mine::Update()
{
	parent::Update();

	const obj_AI_Player* owner = (const obj_AI_Player*)GameWorld().GetObject(ownerID);
	if(owner == NULL) // wait for server to kill this mine
		return TRUE;

	const obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(localPlayer)
	{
		float alpha = 1.0f - R3D_CLAMP((GetPosition() - localPlayer->GetPosition()).Length()/15.0f, 0.0f, 1.0f);
		if(alpha > 0.0f)
		{
			if(m_MineIcon.IsUndefined())
			{
				if(owner->TeamID != localPlayer->TeamID)
				{
					if(localPlayer->CurLoadout.hasItem(AbilityConfig::AB_Sniffer) && (gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb))
					{
						hudMain->addMineEnemyIcon(m_MineIcon);
					}
				}
				else
				{
					hudMain->addMineIcon(m_MineIcon);
				}
			}
			if(!m_MineIcon.IsUndefined())
			{
				hudMain->moveScreenIcon(m_MineIcon, GetPosition(), false);
				hudMain->setScreenIconAlpha(m_MineIcon, alpha);
			}
		}
		else
		{
			if(!m_MineIcon.IsUndefined())
			{
				hudMain->deleteScreenIcon(m_MineIcon);
			}
		}
	}

	return TRUE;
}

BOOL obj_Mine::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
	case PKT_S2C_ExplodeMine:
		{
			onExplode();
		}
		return TRUE;
	case PKT_S2C_DestroyMine:
		{
			setActiveFlag(0);
		}
		return TRUE;
	case PKT_S2C_NewOwnerMine:
		{
			const PKT_S2C_NewOwnerMine_s& n = *(PKT_S2C_NewOwnerMine_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			GameObject* obj = GameWorld().GetNetworkObject(n.newOwnerID);
			if(obj)
				switchOwner(obj);
		}
		return TRUE;
	}
	return FALSE;
}

void obj_Mine::switchOwner(GameObject* obj)
{
	r3d_assert(obj);
	ownerID = obj->GetSafeID();
	if(!m_MineIcon.IsUndefined())
	{
		hudMain->deleteScreenIcon(m_MineIcon);
	}
}