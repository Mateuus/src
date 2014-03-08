#include "r3dpch.h"
#include "r3d.h"

#include "GameCommon.h"
#include "Ammo.h"
#include "ObjectsCode\Effects\obj_ParticleSystem.h"
#include "Rocket.h"
#include "..\world\DecalChief.h"
#include "..\world\MaterialTypes.h"
#include "ExplosionVisualController.h"

#include "..\AI\AI_Player.H"
#include "WeaponConfig.h"
#include "Weapon.h"

#include "multiplayer/P2PMessages.h"
#include "..\..\multiplayer\ClientGameLogic.h"

IMPLEMENT_CLASS(obj_Rocket, "obj_Rocket", "Object");
AUTOREGISTER_CLASS(obj_Rocket);
IMPLEMENT_CLASS(obj_RocketGrenade, "obj_RocketGrenade", "Object");
AUTOREGISTER_CLASS(obj_RocketGrenade);

obj_Rocket::obj_Rocket()
{
	m_Ammo = NULL;
	m_Weapon = 0;
	m_ParticleTracer = NULL;
	m_DisableDistanceTraveled = true;
	//m_hasCollision = false;
}

r3dMesh* obj_Rocket::GetObjectMesh()
{
	r3d_assert(m_Ammo);
	return getModel();
}

/*virtual*/
r3dMesh*
obj_Rocket::GetObjectLodMesh() /*OVERRIDE*/
{
	r3d_assert(m_Ammo);
	return getModel();
}

BOOL obj_Rocket::OnCreate()
{
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(!owner)
		return FALSE;
		
	m_isSerializable = false;

	ReadPhysicsConfig();
	PhysicsConfig.group = PHYSCOLL_PROJECTILES;

	// perform our own movement to sync over network properly
	PhysicsConfig.isKinematic = true; 
	PhysicsConfig.isDynamic = true;

	r3d_assert(m_Ammo);
	r3d_assert(m_Weapon);
	parent::OnCreate();

	m_CreationTime = r3dGetTime();
	m_CreationPos = GetPosition();

	if(m_Ammo->m_ParticleTracer)
		m_ParticleTracer = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", m_Ammo->m_ParticleTracer, GetPosition() );

	r3dBoundBox bboxLocal ;

	bboxLocal.Org.Assign( -0.5f, -0.25f, -0.25f );
	bboxLocal.Size.Assign(1.0f, 0.1f, 0.1f);

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();

	m_AppliedVelocity = m_FireDirection*m_Weapon->m_AmmoSpeed;// * 2;

	m_DistanceTraveled = 0;
	m_isFlying = true;

	return TRUE;
}

BOOL obj_Rocket::OnDestroy()
{
	if(m_ParticleTracer)
		m_ParticleTracer->bKillDelayed = 1;

	return parent::OnDestroy();
}

void obj_Rocket::onHit(const r3dPoint3D& pos, const r3dPoint3D& norm, const r3dPoint3D& fly_dir)
{
	setActiveFlag(0);

	// if owner dissappeared, do nothing.
	const GameObject* owner = GameWorld().GetObject(ownerID);
	if(!owner)
		return;

	// add decal
	DecalParams params;
	params.Dir		= norm;
	params.Pos		= pos;
	params.TypeID	= GetDecalID( r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource) );
	if( params.TypeID != INVALID_DECAL_ID )
		g_pDecalChief->Add( params );

	SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash(m_Weapon->m_PrimaryAmmo->m_DecalSource), GetPosition(), r3dPoint3D(0,1,0));

	//	Start radial blur effect
	gExplosionVisualController.AddExplosion(GetPosition(), m_Weapon->m_AmmoArea);

	// do damage only if owner is local player, otherwise we will broadcast damage multiple times
	r3d_assert(owner);
	r3dVector center = GetPosition()-fly_dir*0.25f;
	if(owner->NetworkLocal)
	{
		// send that you hit nothing no matter what you hit.  This avoids the anti-cheat system being tripped.
		PKT_C2C_PlayerHitNothing_s n;
		p2pSendToHost(owner, &n, sizeof(n), true);

		obj_AI_Player* plr = (obj_AI_Player*)owner;
		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		{
			if(plr->m_Weapons[i] && plr->m_Weapons[i]->getItemID() == m_Weapon->m_itemID)
			{
				gClientLogic().ApplyExplosionDamage(center, m_Weapon->m_AmmoArea, i);
				break;
			}
		}
	}

/*	if(gClientLogic().localPlayer_)
	{
		gClientLogic().localPlayer_->explosionCenter = center;
		gClientLogic().localPlayer_->explosionRadius = m_Weapon->m_AmmoArea;
		gClientLogic().localPlayer_->explosionTime = 30.0f;
	}*/
}

void obj_Rocket::OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace)
{
	return;
	/*if(tobj)
	{
		// stupid physx for whatever reason is reporting collision inside of geometry. let's do a short raycast to fix that
		// changing skin_width doesn't help.
		NxRay ray; 
		NxRaycastHit hit;
		ray.orig = NxVec3(oldstate.Position.x, oldstate.Position.y, oldstate.Position.z);
		ray.dir = NxVec3(trace.NewPosition.x-oldstate.Position.x, trace.NewPosition.y-oldstate.Position.y, trace.NewPosition.z - oldstate.Position.z);
		NxShape* shape = 0;
		if(ray.dir.magnitudeSquared() > 0.01f) // in case if colliding right on first frame, ray dir length will be null and physx will assert
		{
			float rayLen = ray.dir.magnitude();
			ray.dir.normalize();
			shape = g_pPhysicsWorld->PhysXScene->raycastClosestShape(ray, NX_ALL_SHAPES, hit, COLLIDABLE_STATIC_MASK, rayLen+0.1f,NX_RAYCAST_IMPACT|NX_RAYCAST_NORMAL, 0, 0);
		}
		m_hasCollision = true;
		if(shape)
		{
			m_CollisionPoint.Assign(hit.worldImpact.x, hit.worldImpact.y, hit.worldImpact.z);
			m_CollisionNormal.Assign(hit.worldNormal.x, hit.worldNormal.y, hit.worldNormal.z);
		}
		else
		{
			m_CollisionPoint = trace.NewPosition;
			m_CollisionNormal = trace.Normal;
		}
	}*/
}

void MatrixGetYawPitchRoll ( const D3DXMATRIX & , float &, float &, float & );

class rocketFilterCallback : public PxSceneQueryFilterCallback
{
	const GameObject* owner;
public:
	rocketFilterCallback(const GameObject* _owner) : owner(_owner) {};

	virtual PxSceneQueryHitType::Enum preFilter(const PxFilterData& filterData, PxShape* shape, PxSceneQueryFilterFlags& filterFlags)
	{
		if(shape)
		{
			PxRigidActor& actor = shape->getActor();
			PhysicsCallbackObject* target = static_cast<PhysicsCallbackObject*>(actor.userData);
			if(target)
			{
				GameObject *gameObj = target->isGameObject();
				if(gameObj)
				{
					if(gameObj == owner)
						return PxSceneQueryHitType::eNONE;
				}
			}
		}
		return PxSceneQueryHitType::eBLOCK;
	}

	virtual PxSceneQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxSceneQueryHit& hit)
	{
		// shouldn't be called!
		return PxSceneQueryHitType::eBLOCK;
	}
};

BOOL obj_Rocket::Update()
{
	parent::Update();
	if(m_CreationTime+/*m_Weapon->m_AmmoTimeout*/30.0f < r3dGetTime()) // use hard coded 30 sec timeout. otherwise designers forget to set this variable and then they weapons are not working
	{
		const GameObject* owner = GameWorld().GetObject(ownerID);
		if(owner && owner->NetworkLocal)
		{
			// send a hit no matter what. 
			PKT_C2C_PlayerHitNothing_s n;
			p2pSendToHost(owner, &n, sizeof(n), true);
		}
		return FALSE;
	}

	r3dVector dir = (GetPosition() - oldstate.Position);
	if(dir.Length()==0)
		dir = m_FireDirection;
	// perform collision check
	if(m_isFlying)
	{
		m_AppliedVelocity += r3dVector(0, -9.81f, 0) * m_Weapon->m_AmmoMass * r3dGetFrameTime();
		r3dPoint3D motion = m_AppliedVelocity * r3dGetFrameTime();
		float motionLen = motion.Length();
		motion.Normalize();
		m_DistanceTraveled += motionLen;

		PxU32 collisionFlag = COLLIDABLE_STATIC_MASK;
		const GameObject* owner = GameWorld().GetObject(ownerID);
		collisionFlag |= (1<<PHYSCOLL_NETWORKPLAYER); // prefilter is taking care of not hitting owner

		PxSphereGeometry sphere(0.2f);
		PxTransform pose(PxVec3(GetPosition().x, GetPosition().y, GetPosition().z), PxQuat(0,0,0,1));

		PxSweepHit hit;
		PxSceneQueryFilterData filter(PxFilterData(collisionFlag, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::ePREFILTER);
		rocketFilterCallback callback(owner);
		if(g_pPhysicsWorld->PhysXScene->sweepSingle(sphere, pose, PxVec3(motion.x, motion.y, motion.z), motionLen, PxSceneQueryFlag::eINITIAL_OVERLAP|PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL, hit, filter, &callback))
		{
			m_CollisionPoint = r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z);
			m_CollisionNormal = r3dPoint3D(hit.normal.x, hit.normal.y, hit.normal.z);
			if(m_DistanceTraveled > 10.0f || m_DisableDistanceTraveled)
			{
				onHit(m_CollisionPoint, m_CollisionNormal, dir);
				return FALSE;
			}
			else
			{
				m_isFlying = false;
				// remove kinematic flag from physX object
				r3d_assert(PhysicsObject->getPhysicsActor()->isRigidDynamic());
				PhysicsObject->getPhysicsActor()->isRigidDynamic()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);
				PhysicsObject->addImpulse(m_AppliedVelocity/5);
			}
			
		}
	}

	// perform movement
	if(m_isFlying)
	{
		SetPosition(GetPosition() + m_AppliedVelocity * r3dGetFrameTime());
	}


//	if(GetVelocity().Length() < 1.0f) // PhysX bug: rocket can get stuck in geometry
//	{
//		onHit(GetPosition(), r3dVector(0,1,0));
//		return FALSE;
//	}

/*	if(m_hasCollision)
	{
		onHit(m_CollisionPoint, m_CollisionNormal);
		return FALSE;
	}*/

	if(m_CreationTime+3.0f>r3dGetTime()) // apply velocity only for 3 seconds
	{
		m_AppliedVelocity += m_AppliedVelocity * r3dGetFrameTime();
		//PhysicsObject->addSmoothVelocity(m_AppliedVelocity);
	}

	if(m_ParticleTracer)
		m_ParticleTracer->SetPosition(GetPosition());

	// calculate proper rotation matrix
	/*dir = (GetPosition() - oldstate.Position);
	if(dir.Length()==0)
		dir = m_FireDirection;
	
	{
		dir.Normalize();
		r3dVector up(0,1,0);
		r3dVector left = dir.Cross(up);
		left.Normalize();
		up = dir.Cross(left);
		up.Normalize();

		D3DXMATRIX m;
		m._11 = left.x;
		m._12 = left.y;
		m._13 = left.z;
		m._14 = 0;
		m._21 = up.x;
		m._22 = up.y;
		m._23 = up.z;
		m._24 = 0;
		m._31 = dir.x;
		m._32 = dir.y;
		m._33 = dir.z;
		m._34 = 0;
		m._41 = 0;
		m._42 = 0;
		m._43 = 0;
		m._44 = 1;

		float fY, fP, fR;
		MatrixGetYawPitchRoll( m, fY, fP, fR );
		SetRotationVector( r3dPoint3D( R3D_RAD2DEG( fY ), R3D_RAD2DEG( fP ), R3D_RAD2DEG( fR )  ) );
	}*/
	return TRUE;
}

