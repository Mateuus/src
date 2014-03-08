#include "r3dPCH.h"
#include "r3d.h"

#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"

#include "obj_AutoTurret.h"

#include "ObjectsCode/world/DecalChief.h"
#include "ObjectsCode/world/MaterialTypes.h"

#include "Ammo.h"

#include "Particle.h"
#include "ObjectsCode\Effects\obj_ParticleSystem.h"


IMPLEMENT_CLASS(obj_AutoTurret, "obj_AutoTurret", "Object");
AUTOREGISTER_CLASS(obj_AutoTurret);

obj_AutoTurret::obj_AutoTurret() :
 m_MuzzleParticle(NULL)
,m_sndNewFire(0)
{
	m_PrivateModel = NULL;
	m_Skeleton = NULL;
	m_ItemID = -1;
	m_RotX = 0;
	ObjTypeFlags = OBJTYPE_GameplayItem;
	_nextScan = 0;
}

obj_AutoTurret::~obj_AutoTurret()
{

}

BOOL obj_AutoTurret::OnCreate()
{
	r3d_assert(m_ItemID > 0);

	m_WpnConfig = gWeaponArmory.getWeaponConfig(m_ItemID);
	if(!m_WpnConfig)
		return FALSE;

	m_Spread = m_WpnConfig->m_spread;
	m_AngleMax = m_WpnConfig->m_reloadTime;
	m_AngleSpeed = m_WpnConfig->m_reloadActiveTick;
	m_Range = m_WpnConfig->m_AmmoDecay;
	m_FireDelay = 60.0f/m_WpnConfig->m_recoil;

	m_LastFireTime = 0.0f;

	m_CurAngle = 0;
	m_CurAngleTarget = m_AngleMax;
	m_TargetID = invalidGameObjectID;

	if(m_ItemID == WeaponConfig::ITEMID_AutoTurret)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\item_turret_01_m60.sco", true, false, true, true );
	}
	else
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\item_turret_01_m60.sco", true, false, true, true );
	}

	if(m_PrivateModel==NULL)
		return FALSE;

	m_Skeleton = new r3dSkeleton();
	m_Skeleton->LoadBinary("Data\\ObjectsDepot\\Weapons\\item_turret_01_m60.skl");
	m_TurretBoneID = m_Skeleton->GetBoneID("Bone_Turret");
	D3DXMATRIX ident; D3DXMatrixIdentity(&ident);
	m_Skeleton->SetBoneWorldTM(m_Skeleton->GetBoneID("Bone_Root"), &ident);

	ReadPhysicsConfig();

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// raycast and see where the ground is and place dropped box there
	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
	if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
	{
		SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
		SetRotationVector(r3dPoint3D(m_RotX, 0, 0));
	}

	UpdateTransform();

	const char* muzzleParticle = m_WpnConfig->m_MuzzleParticle;
	if(muzzleParticle[0]!='\0')
	{
		m_MuzzleParticle = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", muzzleParticle, r3dPoint3D(0,0,0));
		if(m_MuzzleParticle->Torch != 0)
		{
			m_MuzzleParticle->bKeepAlive = true; // do not allow it to be destroyed
			m_MuzzleParticle->bRender = false;

			// create light
			m_MuzzleLight.Assign(0,0,0);
			m_MuzzleLight.SetType(R3D_OMNI_LIGHT);
			m_MuzzleLight.SetRadius(0.0f, 4.0f);
			m_MuzzleLight.SetColor(255, 201, 14);
			m_MuzzleLight.bCastShadows = false;
			m_MuzzleLight.TurnOff();
			WorldLightSystem.Add(&m_MuzzleLight); 
		}
		else
			m_MuzzleParticle = 0; // it failed to load, object manager will delete it
	}

	return parent::OnCreate();
}

BOOL obj_AutoTurret::OnDestroy()
{
	m_PrivateModel = NULL;
	SAFE_DELETE(m_Skeleton);

	if(m_MuzzleParticle)
		m_MuzzleParticle->bKeepAlive = false;
	if(m_MuzzleLight.pLightSystem)
		WorldLightSystem.Remove(&m_MuzzleLight); 

	if(m_sndNewFire)
	{
		SoundSys.KeyOff(m_sndNewFire, "trigger");
		m_sndNewFire = 0;
	}

	extern bool gDestroyingWorld;
	if(!gDestroyingWorld)
		srv_CreateGameObject("obj_ParticleSystem", "Explosion_Grenade_01", GetPosition());

	return parent::OnDestroy();
}

float getDegreeWithTarget(const r3dPoint3D& dirToTarget, const r3dPoint3D& turretDir, const r3dPoint3D& turretLeft)
{
	float dot = dirToTarget.Dot(turretDir);
	float dot2 = dirToTarget.Dot(turretLeft);

	float deg = R3D_RAD2DEG(acosf(dot));
	
	if(dot2<0) 
		deg = -deg;
	deg = R3D_CLAMP(deg, -180.0f, 180.0f);
	return deg;
}
float getMinimumAngleDistance(float from, float to);
float dist_Point_to_Line( const r3dPoint3D& p, const r3dPoint3D& l1, const r3dPoint3D& l2);
bool ProcessBulletHit( int &damageFromPiercable, GameObject* owner, const r3dPoint3D &hitPoint, const r3dPoint3D &hitNormal, GameObject* shootTarget, const r3dMaterial* shootMaterial, const char* hitActorName, const WeaponConfig *weaponInfo);
BOOL obj_AutoTurret::Update()
{
	D3DXMATRIX rot;

	obj_AI_Player* turretOwner = (obj_AI_Player*)GameWorld().GetObject(ownerID);
	if(!turretOwner)
		return parent::Update();
		
	float fY = getMinimumAngleDistance(m_CurAngle, m_CurAngleTarget);
	if(R3D_ABS(fY)>0.1f)
	{
		float fTimePassed = r3dGetFrameTime();
		float step = m_AngleSpeed*fTimePassed;
		m_CurAngle += fY < 0.0f ? -step : +step;
		// check if we finished rotation, overrotated
		float fY2 = getMinimumAngleDistance(m_CurAngle, m_CurAngleTarget);
		if((fY > 0 && fY2 <= 0) || (fY < 0 && fY2 >= 0) 
			|| 
			// don't overreact - can get to any angle with this sort of step anyway (happens on low fps)
			step >= 180.f )
		{
			m_CurAngle = m_CurAngleTarget;
		}
	}
	else
		m_CurAngle = m_CurAngleTarget;

	// scan mode
	if(m_TargetID == invalidGameObjectID)
	{
		if(m_CurAngle > 0 && m_CurAngle >= m_CurAngleTarget)
		{
			m_CurAngleTarget = -m_AngleMax;
		}
		else if(m_CurAngle < 0 && m_CurAngle <= m_CurAngleTarget)
		{
			m_CurAngleTarget = m_AngleMax;
		}
	}

	D3DXMatrixRotationY(&rot, R3D_DEG2RAD(m_CurAngle));
	m_Skeleton->SetBoneWorldTM(m_TurretBoneID, &rot);

	D3DXMATRIX turretRotMatrix = GetRotationMatrix();
	r3dPoint3D turretDir(turretRotMatrix._31, turretRotMatrix._32, turretRotMatrix._33);
	turretDir = -turretDir; // model is rotated by 180deg
	r3dPoint3D turretLeft(-turretRotMatrix._11, -turretRotMatrix._12, -turretRotMatrix._13);

	{
		D3DXMATRIX muzzleRotMat;
		D3DXMatrixMultiply(&muzzleRotMat, &turretRotMatrix, &rot);
		r3dPoint3D turretDir2(muzzleRotMat._31, muzzleRotMat._32, muzzleRotMat._33);
		muzzlerDir = -turretDir2; // model is rotated by 180deg
		muzzlerPoint = GetPosition() + r3dPoint3D(0,0.7f,0) + muzzlerDir*0.6f;
	}

	DWORD collisionGroupToCheck = 1<<PHYSCOLL_NETWORKPLAYER;
	if(!turretOwner->NetworkLocal)
		collisionGroupToCheck |= 1<<PHYSCOLL_LOCALPLAYER; // otherwise remote turrret will not "see" local player

	// check if target is still in range
	if(m_TargetID != invalidGameObjectID)
	{
		bool lostTarget = false;
		obj_AI_Player* plr = (obj_AI_Player*)GameWorld().GetObject(m_TargetID);
		if(plr)
		{
			float dist = (GetPosition() - plr->GetPosition()).Length();
			if(dist > m_Range)
				lostTarget = true;
			else if(plr->bDead)
				lostTarget = true;
			else
			{
				m_CurAngleTarget = getDegreeWithTarget(((plr->GetPosition()-GetPosition()).Normalize()), turretDir, turretLeft);
				if(m_CurAngleTarget <-m_AngleMax || m_CurAngleTarget > m_AngleMax)
					lostTarget = true;

				// check if visible
				PxRaycastHit hit;
				PxSceneQueryFilterData filter2(PxFilterData(COLLIDABLE_STATIC_MASK|collisionGroupToCheck, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
				r3dPoint3D dirToTarget = (plr->GetPosition() - GetPosition()).NormalizeTo();
				r3dPoint3D startScanFrom = GetPosition() + r3dPoint3D(0,0.5f,0) + dirToTarget.Normalize()*0.5f;
				g_pPhysicsWorld->raycastSingle(PxVec3(startScanFrom.x, startScanFrom.y, startScanFrom.z), PxVec3(dirToTarget.x, dirToTarget.y, dirToTarget.z), dist, 
					PxSceneQueryFlag::eIMPACT, hit, filter2);
				PhysicsCallbackObject* clbObj = NULL;
				if(hit.shape && (clbObj = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
				{
					GameObject* gameObj = clbObj->isGameObject();
					if(gameObj != plr)
						lostTarget = true;
				}
				else
					lostTarget = true;

			}
		}
		else
			lostTarget = true;
		
		if(lostTarget)
		{
			m_TargetID = invalidGameObjectID;
			m_CurAngleTarget = m_AngleMax;
		}
		else
		{
			m_CurAngleTarget = getDegreeWithTarget(((plr->GetPosition()-GetPosition()).Normalize()), turretDir, turretLeft);	
		}

	}

	// vision check
	if(m_TargetID == invalidGameObjectID && r3dGetTime() > _nextScan)
	{
		_nextScan = r3dGetTime() + 0.1f; // 10fps

		PxSphereGeometry visionGeom(m_Range); // physX doesn't support cones, so use sphere for now
		PxTransform pose(PxVec3(GetPosition().x, GetPosition().y, GetPosition().z), PxQuat(0,0,0,1));
		PxSceneQueryFilterData filter(PxFilterData(collisionGroupToCheck, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
		PxShape* results[2048] = {0}; // 2048 - because each player has about 22 meshes that will get hit (ragdoll)
		int numRes = g_pPhysicsWorld->PhysXScene->overlapMultiple(visionGeom, pose, results, 2048, filter);
		if(numRes)
		{
			obj_AI_Player* closestTarget = NULL;
			float			closestDist = 999999.0f;
			// find target
			for(int i=0; i<numRes; ++i)
			{
				PhysicsCallbackObject* clbObj = NULL;
				if( results[i] && (clbObj = static_cast<PhysicsCallbackObject*>(results[i]->getActor().userData)))
				{
					GameObject* gameObj = clbObj->isGameObject();
					if(gameObj && gameObj->isObjType(OBJTYPE_Human))
					{
						obj_AI_Player* target = (obj_AI_Player*)gameObj;
						float dist = (GetPosition() - target->GetPosition()).Length();
						if(target->bDead)
							continue;
						if(target->TeamID == turretOwner->TeamID)
							continue;
						if(dist > m_Range)
							continue;
						if(dist > closestDist)
							continue;
						// check if within visible cone
						{
							float deg = getDegreeWithTarget(((target->GetPosition()-GetPosition()).Normalize()), turretDir, turretLeft);	
							if(deg < -m_AngleMax || deg > m_AngleMax)
								continue;
						}

						// check if visible
						PxRaycastHit hit;
						PxSceneQueryFilterData filter2(PxFilterData(COLLIDABLE_STATIC_MASK|collisionGroupToCheck, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
						r3dPoint3D dirToTarget = (target->GetPosition() - GetPosition()).NormalizeTo();
						r3dPoint3D startScanFrom = GetPosition() + r3dPoint3D(0,0.5f,0) + dirToTarget.Normalize()*0.5f;
						g_pPhysicsWorld->raycastSingle(PxVec3(startScanFrom.x, startScanFrom.y, startScanFrom.z), PxVec3(dirToTarget.x, dirToTarget.y, dirToTarget.z), dist, 
							PxSceneQueryFlag::eIMPACT, hit, filter2);
						if(hit.shape == results[i])
						{
							closestDist = dist;
							closestTarget = target;
						}
					}
				}
			}
			if(closestTarget) // found target!
			{
				{
//					char tmpStr[256];closestTarget->GetUserName(tmpStr);
//					r3dOutToLog("Found player: %s\n", tmpStr);
				}

				m_TargetID = closestTarget->GetSafeID();
				m_CurAngleTarget = getDegreeWithTarget(((closestTarget->GetPosition()-GetPosition()).Normalize()), turretDir, turretLeft);
			}

		}
	}

	// check if we can fire at target
	if(m_TargetID!=invalidGameObjectID && turretOwner->NetworkLocal)
	{
		bool stoppedFiring = R3D_ABS(m_CurAngleTarget-m_CurAngle)>5.0f || (r3dGetTime()-m_LastFireTime)<m_FireDelay;
		// update fire sound
		if(m_sndNewFire)
		{
			SoundSys.SetParamValue(m_sndNewFire, "(distance)", (gCam-muzzlerPoint).Length());
			if(!snd_SetSoundPos(m_sndNewFire, GetPosition()))
				m_sndNewFire = 0;
			else
			{
				if(stoppedFiring)
				{
					SoundSys.KeyOff(m_sndNewFire, "trigger");
					m_sndNewFire = 0;
				}
			}
		}

		if(R3D_ABS(m_CurAngleTarget-m_CurAngle)<5.0f && (r3dGetTime()-m_LastFireTime)>=m_FireDelay)
		{
			m_LastFireTime = r3dGetTime();
			obj_AI_Player* plr = (obj_AI_Player*)GameWorld().GetObject(m_TargetID);
			r3d_assert(plr);

			// shoot!
			r3dPoint3D dir = (plr->GetPosition()-GetPosition()).Normalize();

			// convert spread from diameter at 50 meter range to radius at 1meter range
			float spread = (m_Spread*0.5f)/50.0f; // 0.5 - because spread initially set as diameter, and we need radius. 50-because spread is set at 50 meter range. formula is: tanA=opp/adj = so that is angle that we need to feed into raycast

			// limit spread to prevent ppl shooting behind, etc.
			spread = R3D_CLAMP(spread, -0.5f, 0.5f); 
			D3DXMATRIX rotMat;
			D3DXMatrixRotationYawPitchRoll(&rotMat, u_GetRandom(-spread, spread), u_GetRandom(-spread, spread), u_GetRandom(-spread, spread));
			r3dPoint3D dirWithSpread;
			D3DXVec3TransformNormal((D3DXVECTOR3*)&dirWithSpread, (D3DXVECTOR3*)&dir, &rotMat);
			{
				r3dPoint3D shootFrom = muzzlerPoint;
				PxRaycastHit hit;
				PhysicsCallbackObject* target = NULL;
				PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|collisionGroupToCheck, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
				g_pPhysicsWorld->raycastSingle(PxVec3(shootFrom.x, shootFrom.y, shootFrom.z), PxVec3(dirWithSpread.x, dirWithSpread.y, dirWithSpread.z), m_Range, 
					PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL, hit, filter);

				GameObject* hitTarget = NULL;
				r3dMaterial* hitMat = NULL;
				r3dPoint3D hitPos(shootFrom + dirWithSpread*500), hitNorm(-dirWithSpread);
				const char* hitTargetName = NULL;
				if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
				{
					hitTargetName = hit.shape->getActor().getName();

					hitTarget = target->isGameObject();
					if(hitTarget)
					{
						if( hitTarget->isObjType( OBJTYPE_Mesh ) )
							hitMat = static_cast< MeshGameObject* > ( target )->MeshLOD[ 0 ]->GetFaceMaterial( hit.faceIndex );
					}
					else if(target->hasMesh())
					{
						hitMat = target->hasMesh()->GetFaceMaterial( hit.faceIndex );
					}
					if (!hitMat)
						hitMat = target->GetMaterial(hit.faceIndex);


					hitPos.x = hit.impact.x;
					hitPos.y = hit.impact.y;
					hitPos.z = hit.impact.z;

					hitNorm.x = hit.normal.x;
					hitNorm.y = hit.normal.y;
					hitNorm.z = hit.normal.z;
				}
				int piercableNotUsed=0;
				ProcessBulletHit( piercableNotUsed, this, hitPos, hitNorm, hitTarget, hitMat, hitTargetName, m_WpnConfig);

				const obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
				float distToLocalPlayer = 0.0f;
				bool turretVisible = r3dRenderer->IsSphereInsideFrustum(GetPosition()+r3dPoint3D(0,0.5f,0), 1.0f)>0;
				if(localPlayer)
					distToLocalPlayer = (localPlayer->GetPosition() - GetPosition()).Length();

				if(m_MuzzleParticle && distToLocalPlayer < 100.0f && turretVisible)
				{
					m_MuzzleParticle->bRender = true;
					m_MuzzleParticle->SetPosition(muzzlerPoint);
					m_MuzzleParticle->Torch->Direction.Assign(muzzlerDir.x, muzzlerDir.y, muzzlerDir.z);
					m_MuzzleParticle->Restart();

					m_MuzzleLight.TurnOn();
					m_MuzzleLight.Assign(muzzlerPoint);
					m_MuzzleLight.Intensity = 2.0f;
				}

				if(m_sndNewFire == 0)
					m_sndNewFire = snd_PlaySound(m_WpnConfig->m_sndFireID_auto, GetPosition());
			}
		}
	}
	else // no target
	{
		if(m_sndNewFire)
		{
			SoundSys.KeyOff(m_sndNewFire, "trigger");
			m_sndNewFire = 0;
		}
	}

	if(m_MuzzleParticle)
	{
		m_MuzzleParticle->SetPosition(muzzlerPoint);
		if(m_MuzzleLight.IsOn())
		{
			m_MuzzleLight.Assign(muzzlerPoint);
			float light_lifetime = 0.15f;
			m_MuzzleLight.Intensity = R3D_CLAMP(1.0f-((r3dGetTime()-m_LastFireTime)/light_lifetime), 0.0f, 1.0f)*2.0f;
			if(m_MuzzleLight.Intensity == 0.0f)
				m_MuzzleLight.TurnOff();
		}
	}

	return parent::Update();
}

BOOL obj_AutoTurret::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	r3d_assert(!(ObjFlags & OBJFLAG_JustCreated)); // make sure that object was actually created before processing net commands
	const ClientGameLogic& CGL = gClientLogic();

	/// processing only fire events, if that will change, update this code
	{
		const obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
		float distToLocalPlayer = 0.0f;
		bool turretVisible = r3dRenderer->IsSphereInsideFrustum(GetPosition()+r3dPoint3D(0,0.5f,0), 1.0f)>0;
		if(localPlayer)
			distToLocalPlayer = (localPlayer->GetPosition() - GetPosition()).Length();

		if(m_MuzzleParticle && distToLocalPlayer < 100.0f && turretVisible)
		{
			m_MuzzleParticle->bRender = true;
			m_MuzzleParticle->SetPosition(muzzlerPoint);
			m_MuzzleParticle->Torch->Direction.Assign(muzzlerDir.x, muzzlerDir.y, muzzlerDir.z);
			m_MuzzleParticle->Restart();

			m_MuzzleLight.TurnOn();
			m_MuzzleLight.Assign(muzzlerPoint);
			m_MuzzleLight.Intensity = 2.0f;
		}

		if(m_sndNewFire == 0)
			m_sndNewFire = snd_PlaySound(m_WpnConfig->m_sndFireID_auto, GetPosition());
	}

	switch(EventID)
	{
	default: return FALSE;
	case PKT_C2C_PlayerHitNothing:
		{
			break; 
		}
	case PKT_C2C_PlayerHitStatic:
	case PKT_C2C_PlayerHitStaticPierced:
		{
			const PKT_C2C_PlayerHitStatic_s& n = *(PKT_C2C_PlayerHitStatic_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			// create decal
			if(n.decalIdx != 0 )
			{
				DecalParams params;
				params.Dir	= n.hit_norm;
				params.Pos	= n.hit_pos;
				params.TypeID	= (uint32_t)(n.decalIdx) - 1;
				if(params.TypeID >= 0 && params.TypeID < (int)g_pDecalChief->GetTypeCount())
				{
					g_pDecalChief->Add( params );
				}
				else
				{
					// some cheat or invalid data
				}
			}

			// impact particle
			if(n.particleIdx != 0)
			{
				uint32_t particleIdx = (uint32_t)(n.particleIdx) - 1;
				if(particleIdx < ImpactParticleEntry::NumRegistered)
				{
					SpawnImpactParticle(particleIdx, n.hit_pos, n.hit_norm);
				}
				else
				{
					// some cheat or invalid data
				}
			}

			{
				if(GameObject* from = GameWorld().GetNetworkObject(n.FromID))
				{
					r3dPoint3D waterSplashPos;
					extern bool TraceWater(const r3dPoint3D& start, const r3dPoint3D& finish, r3dPoint3D& waterSplashPos);
					if( TraceWater( from->GetPosition(), n.hit_pos, waterSplashPos))
					{	
						extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
						waterSplashPos.y -= 0.1f;
						WaterSplash(waterSplashPos, 0.1f, 30.0f, 0.04f, 2);
					}
				}
			}

			// add impulse
			GameObject* target = GameWorld().GetObjectByHash(n.hash_obj);
			if(target)
				if(target->PhysicsObject)
					target->PhysicsObject->AddImpulseAtPos(-n.hit_norm*m_WpnConfig->m_AmmoSpeed*m_WpnConfig->m_AmmoMass/150.0f, n.hit_pos);

			// play near miss sound
			{
				if(CGL.localPlayer_)
				{
					float dist = dist_Point_to_Line(CGL.localPlayer_->GetPosition(), GetPosition(), n.hit_pos);
					if(dist < 5.0f && rand()%5==0)
					{
						static int bulletHitMissSoundID = -1;
						if(bulletHitMissSoundID==-1)
							bulletHitMissSoundID = SoundSys.GetEventIDByPath("Sounds/BulletHits/Miss");
						snd_PlaySound(bulletHitMissSoundID, CGL.localPlayer_->GetPosition());
					}
				}
			}

			break;
		}

	case PKT_C2C_PlayerHitDynamic:
		{
			const PKT_C2C_PlayerHitDynamic_s& n = *(PKT_C2C_PlayerHitDynamic_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			r3dPoint3D dir = (GetPosition()-n.hit_pos).NormalizeTo();

			GameObject* target = GameWorld().GetNetworkObject(n.targetId);
			if(!target) 
			{
				r3d_assert(target);
				break;
			}

			// if we hit local player, display hit effect
			if(target->isObjType(OBJTYPE_Human))
			{
				obj_AI_Player* turretOwner = (obj_AI_Player*)GameWorld().GetObject(ownerID);
				if(turretOwner)
				{
					if(!(CGL.m_gameInfo.friendlyFire == 0 && (turretOwner->TeamID == ((obj_AI_Player*)target)->TeamID)))
					{
						turretOwner->AddBloodOnGround(n.hit_pos);
						SpawnImpactParticle(r3dHash::MakeHash("player"), r3dHash::MakeHash(m_WpnConfig->m_PrimaryAmmo->getDecalSource()), n.hit_pos, dir);
					}
				}

				if(target->NetworkLocal == false || CGL.localPlayer_==NULL) 
					break;

				// if hit local player
				extern void TPSGameHUD_AddHitEffect(GameObject* from);
				if(turretOwner)
				{
					if(!(CGL.m_gameInfo.friendlyFire == 0 && (turretOwner->TeamID == ((obj_AI_Player*)target)->TeamID)))
						TPSGameHUD_AddHitEffect(this);
				}

			}
			else // hit something else, for now that is UAV only
			{
				SpawnImpactParticle(r3dHash::MakeHash("Metal"),r3dHash::MakeHash(m_WpnConfig->m_PrimaryAmmo->getDecalSource()), n.hit_pos, dir);
			}

			break;
		}

	}

	return TRUE;
}


struct AutoTurretDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		AutoTurretDeferredRenderable* This = static_cast< AutoTurretDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		This->Parent->m_Skeleton->SetShaderConstants();
		if(This->DrawState != rsCreateSM)
			This->Parent->m_PrivateModel->DrawMeshDeferred(r3dColor::white, 0);
		else
			This->Parent->m_PrivateModel->DrawMeshShadows();
	}

	obj_AutoTurret* Parent;
	eRenderStageID DrawState;
};

void obj_AutoTurret::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam )
{
	uint32_t prevCount = rarr.Count();
	m_PrivateModel->AppendShadowRenderables( rarr );
	for( uint32_t i = prevCount, e = rarr.Count(); i < e; i ++ )
	{
		AutoTurretDeferredRenderable& rend = static_cast<AutoTurretDeferredRenderable&>( rarr[ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsCreateSM;
	}
}


void obj_AutoTurret::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		AutoTurretDeferredRenderable& rend = static_cast<AutoTurretDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsFillGBuffer;
	}
}

r3dMesh* obj_AutoTurret::GetObjectMesh() 
{
	return m_PrivateModel;
}

r3dMesh* obj_AutoTurret::GetObjectLodMesh()
{
	return m_PrivateModel;
}