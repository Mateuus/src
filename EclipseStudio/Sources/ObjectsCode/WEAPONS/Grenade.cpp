#include "r3dpch.h"
#include "r3d.h"

#include "Grenade.h"
#include "ObjectsCode\Effects\obj_ParticleSystem.h"
#include "..\world\DecalChief.h"
#include "..\world\MaterialTypes.h"
#include "ExplosionVisualController.h"
#include "FlashbangVisualController.h"
#include "Gameplay_Params.h"

#include "multiplayer/P2PMessages.h"

#include "..\..\multiplayer\ClientGameLogic.h"
#include "..\ai\AI_Player.H"
#include "WeaponConfig.h"
#include "Weapon.h"

#include "..\..\ui\HUDDisplay.h"
extern HUDDisplay*	hudMain;

IMPLEMENT_CLASS(obj_Grenade, "obj_Grenade", "Object");
AUTOREGISTER_CLASS(obj_Grenade);

obj_Grenade::obj_Grenade()
{
	m_Ammo = NULL;
	m_Weapon = 0;
	m_ParticleTracer = NULL;
	bHadCollided = false;
	lastCollisionNormal.Assign(0,1,0);
}

r3dMesh* obj_Grenade::GetObjectMesh()
{
	r3d_assert(m_Ammo);
	return getModel();
}

/*virtual*/
r3dMesh*
obj_Grenade::GetObjectLodMesh()
{
	r3d_assert(m_Ammo);
	return getModel();
}

BOOL obj_Grenade::OnCreate()
{
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(!owner)
		return FALSE;

	m_isSerializable = false;

	ReadPhysicsConfig();
	PhysicsConfig.group = PHYSCOLL_PROJECTILES;
	PhysicsConfig.isFastMoving = true;

	r3d_assert(m_Ammo);
	r3d_assert(m_Weapon);

	PhysicsConfig.type = PHYSICS_TYPE_BOX;
	r3dVector pos = GetPosition();
	r3dVector size(0.5f,0.5f,0.5f);
	r3dMesh* mesh = GetObjectMesh();
	D3DXMATRIX mat = GetRotationMatrix();
	PhysicsObject = BasePhysicsObject::CreateDynamicObject(PhysicsConfig, this, &pos, &size, mesh, &mat);

	parent::OnCreate();

	m_CreationTime = r3dGetTime() - m_AddedDelay;
	m_CreationPos = GetPosition();

	if(m_Ammo->m_ParticleTracer)
		m_ParticleTracer = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", m_Ammo->m_ParticleTracer, GetPosition() );

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;
	UpdateTransform();

	m_FireDirection.y += 0.1f; // to make grenade fly where you point
	m_AppliedVelocity = m_FireDirection*m_Weapon->m_AmmoSpeed;
	SetVelocity(m_AppliedVelocity);

	if(hudMain)
		hudMain->addGrenadeIcon(m_GrenadeIcon);

	return TRUE;
}

BOOL obj_Grenade::OnDestroy()
{
	if(m_ParticleTracer)
		m_ParticleTracer->bKillDelayed = 1;

	if(hudMain && !m_GrenadeIcon.IsUndefined())
		hudMain->deleteScreenIcon(m_GrenadeIcon);

	return parent::OnDestroy();
}

void obj_Grenade::onExplode()
{
	// if owner disappeared, do nothing.
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(!owner)
		return;

	bool closeToGround = false;

	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
	if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eDISTANCE, hit, filter))
	{
		if(hit.distance < 5.0f)
			closeToGround = true;
	}

	if(closeToGround)
	{
		// add decal
		DecalParams params;
		params.Dir		= lastCollisionNormal;
		params.Pos		= GetPosition();
		params.TypeID	= GetDecalID( r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource) );
		if( params.TypeID != INVALID_DECAL_ID )
			g_pDecalChief->Add( params );
	}

	//check for underwater splash
	r3dPoint3D waterSplashPos;
	extern bool TraceWater(const r3dPoint3D& start, const r3dPoint3D& finish, r3dPoint3D& waterSplashPos);
	if( TraceWater( GetPosition() - r3dPoint3D(0.0f, m_Weapon->m_AmmoArea, 0.0f), GetPosition() + r3dPoint3D(0.0f, m_Weapon->m_AmmoArea, 0.0f), waterSplashPos))
	{
		SpawnImpactParticle(r3dHash::MakeHash("Water"), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource), waterSplashPos, r3dPoint3D(0,1,0));
		extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
		WaterSplash(GetPosition(), m_Weapon->m_AmmoArea, 10.0f/m_Weapon->m_AmmoArea, 0.04f, 1);
	}

	SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource), GetPosition(), r3dPoint3D(0,1,0));

	//	Start radial blur effect
	gExplosionVisualController.AddExplosion(GetPosition(), m_Weapon->m_AmmoArea);
	// for now hard coded for flash bang grenade
	if(m_Weapon->m_itemID == 101137)
	{
		// only apply flashbank for local player
		if(gClientLogic().localPlayer_ && !gClientLogic().localPlayer_->bDead)
		{
			// check that player can actually see explosion (need better test, for when explosion is behind wall)
			bool explosionVisible = r3dRenderer->IsSphereInsideFrustum(GetPosition(), 2.0f)>0;
			if(explosionVisible)
			{
				float radiusAdd = 0.0f;
				float durationAdd = 0.0f;
				{
					obj_AI_Player* creator_pl = NULL;
					if(owner->isObjType(OBJTYPE_Human))
						creator_pl = (obj_AI_Player*)owner;
					if(creator_pl)
					{
						int FlashbangSkill = creator_pl->CurLoadout.getSkillLevel(CUserSkills::MEDIC_ImprovedFlashbangRadius);
						switch(FlashbangSkill)
						{
						case 1: durationAdd = 0.25f; break;
						case 2: durationAdd = 0.50f; break;
						case 3: durationAdd = 0.75f; break;
						case 4: durationAdd = 1.00f; break;
						case 5: durationAdd = 1.25f; break;
						default:break;
						}
					}
				}
				float dist = (GetPosition()-gClientLogic().localPlayer_->GetPosition()).Length();
				float str = 1.0f - R3D_CLAMP(dist / (GPP->c_fFlashBangRadius+radiusAdd), 0.0f, 1.0f);
				if (str > 0.01f)
				{
					FlashBangEffectParams fbep;
					fbep.duration = (GPP->c_fFlashBangDuration+durationAdd) * str;
					fbep.pos = GetPosition();
					gFlashbangVisualController.StartEffect(fbep);
				}
			}
		}
	}

	// do damage only if owner is local player, otherwise we will broadcast damage multiple times
	r3d_assert(owner);
	if(owner->NetworkLocal)
	{
		PKT_C2C_PlayerHitNothing_s n;
		p2pSendToHost(owner, &n, sizeof(n), true);
        
		obj_AI_Player* plr = (obj_AI_Player*)owner;
		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		{
			if(plr->m_Weapons[i] && plr->m_Weapons[i]->getItemID() == m_Weapon->m_itemID)
			{
				gClientLogic().ApplyExplosionDamage(GetPosition(), m_Weapon->m_AmmoArea, i);
				break;
			}
		}
	}

	setActiveFlag(0);

#if APEX_ENABLED
	//	Apply apex destructions
	g_pApexWorld->ApplyAreaDamage(m_Weapon->m_AmmoDamage * 0.01f, 1, GetPosition(), m_Weapon->m_AmmoArea, true);
#endif
}

void obj_Grenade::OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace)
{
	lastCollisionNormal = trace.Normal;
	bHadCollided = true;
}

void MatrixGetYawPitchRoll ( const D3DXMATRIX & , float &, float &, float & );
BOOL obj_Grenade::Update()
{
	parent::Update();

	if(m_CreationTime+m_Weapon->m_AmmoDelay < r3dGetTime())
	{
		onExplode();
		return FALSE;
	}

	{
		float alpha = 0.0f;
		if(gClientLogic().localPlayer_) // show grenade icon only within 25 meters of player
			alpha = 1.0f - R3D_CLAMP((GetPosition() - gClientLogic().localPlayer_->GetPosition()).Length()/25.0f, 0.0f, 1.0f);
		if(hudMain && !m_GrenadeIcon.IsUndefined())
		{
			hudMain->moveScreenIcon(m_GrenadeIcon, GetPosition(), false);
			hudMain->setScreenIconAlpha(m_GrenadeIcon, alpha);
		}
	}

	if(m_ParticleTracer)
		m_ParticleTracer->SetPosition(GetPosition());

	//// calculate proper rotation matrix
	//r3dVector dir = (GetPosition() - oldstate.Position);
	//if(dir.Length()==0)
	//	dir = m_FireDirection;

	//{
	//	dir.Normalize();
	//	r3dVector up(0,1,0);
	//	r3dVector left = dir.Cross(up);
	//	left.Normalize();
	//	up = dir.Cross(left);
	//	up.Normalize();

	//	D3DXMATRIX m;
	//	m._11 = left.x;
	//	m._12 = left.y;
	//	m._13 = left.z;
	//	m._14 = 0;
	//	m._21 = up.x;
	//	m._22 = up.y;
	//	m._23 = up.z;
	//	m._24 = 0;
	//	m._31 = dir.x;
	//	m._32 = dir.y;
	//	m._33 = dir.z;
	//	m._34 = 0;
	//	m._41 = 0;
	//	m._42 = 0;
	//	m._43 = 0;
	//	m._44 = 1;

	//	float fY, fP, fR;
	//	MatrixGetYawPitchRoll( m, fY, fP, fR );
	//	SetRotationVector( r3dPoint3D( R3D_RAD2DEG( fY ), R3D_RAD2DEG( fP ), R3D_RAD2DEG( fR )  ) );
	//}
	return TRUE;
}
