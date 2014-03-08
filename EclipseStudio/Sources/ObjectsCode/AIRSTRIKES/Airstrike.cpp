#include "r3dpch.h"
#include "r3d.h"

#include "GameCommon.h"
#include "ObjectsCode\Effects\obj_ParticleSystem.h"
#include "Airstrike.h"
#include "Particle.h"

#include "multiplayer/P2PMessages.h"
#include "..\..\multiplayer\ClientGameLogic.h"

void Airstrike_Fire(const r3dPoint3D& hitPos, int itemID, float heightOffset, int teamID)
{
#ifndef WO_SERVER
		// find airstrike
		int numAirstrikes = getNumAirstrikes();
		for(int i=0; i<numAirstrikes; ++i)
		{
			if(getAirstrike(i)->itemID == itemID && getAirstrike(i)->isAvailable)
			{
				r3dPoint3D Pos = hitPos + r3dPoint3D(0,getAirstrike(i)->BaseHeight + heightOffset,0); 

				obj_Airstrike* ammoSh = (obj_Airstrike *)srv_CreateGameObject("obj_Airstrike", "airstrike",Pos);
				ammoSh->ASData = getAirstrike(i);
				ammoSh->m_FireDirection = r3dVector(0,-1,0);
				ammoSh->m_HitDestination = hitPos;

				// set cooldowns (add 5 seconds locally to make sure that there will be no desync for cooldowns between client and server)
				AIRSTRIKE_Team_Cooldown[teamID] = 30.0f + 5.0f;
				getAirstrike(i)->CurrentCooldown[teamID] = getAirstrike(i)->Cooldown + 5.0f;

				break;
			}
		}
#endif
}


IMPLEMENT_CLASS(obj_Airstrike, "obj_Airstrike", "Object");
AUTOREGISTER_CLASS(obj_Airstrike);

obj_Airstrike::obj_Airstrike()
{
	m_PrivateModel = NULL;
	m_ParticleTracer = NULL;
	ASData	= NULL;
}

r3dMesh* obj_Airstrike::GetObjectMesh()
{
	return getModel();
}

/*virtual*/
r3dMesh*
obj_Airstrike::GetObjectLodMesh()
{
	return getModel();
}


r3dMesh* obj_Airstrike::getModel()
{
	if(m_PrivateModel == 0)
	{
		m_PrivateModel = r3dGOBAddMesh(ASData->MeshName, true, false, false, true );
		r3dOutToLog("Loaded mesh\n");
		r3d_assert(m_PrivateModel);
	}

	return m_PrivateModel;
}

void obj_Airstrike::unloadModel()
{
	m_PrivateModel = NULL;
}


BOOL obj_Airstrike::OnCreate()
{
	m_isSerializable = false;

	ReadPhysicsConfig();
	PhysicsConfig.group = PHYSCOLL_PROJECTILES;

	// perform our own movement to sync over network properly
	PhysicsConfig.isKinematic = true; 
	PhysicsConfig.isDynamic = true;

	parent::OnCreate();

	m_CreationTime = r3dGetTime();
	m_CreationPos = GetPosition();

	m_DistanceUntilExplosion = (m_CreationPos - m_HitDestination).Length();
	if(ASData->TrailName)
		m_ParticleTracer = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", ASData->TrailName, GetPosition() );

	r3dBoundBox bboxLocal ;

	bboxLocal.Org.Assign( -0.5f, -0.25f, -0.25f );
	bboxLocal.Size.Assign(1.0f, 0.1f, 0.1f);

	SetBBoxLocal( bboxLocal ) ;
	UpdateTransform();

	m_AppliedVelocity = m_FireDirection*ASData->Speed;
	m_DistanceTraveled = 0;
	
	return TRUE;
}

BOOL obj_Airstrike::OnDestroy()
{
	ASData	= NULL;

	if(m_ParticleTracer)
		m_ParticleTracer->bKillDelayed = 1;

	unloadModel();

	return parent::OnDestroy();
}

void Airstrike_Spawn_Explosion(const r3dPoint3D& hitPos, int itemID)
{
	AirstrikeDataset* ASData = getAirstrikeByID(itemID);
	if(ASData == NULL)
		return; // shouldn't happen!

	obj_ParticleSystem* em = NULL;
	em = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", ASData->Explosion1, hitPos);
	r3d_assert(em);
	if(em->Torch)
		em->Torch->Direction = r3dPoint3D(0,1,0);
	em->GlobalScale = 1.0f;
	em->ObjFlags |= OBJFLAG_SkipCastRay;

	if(ASData->ExplosionSound)
	{
		int sndID = SoundSys.GetEventIDByPath(ASData->ExplosionSound);
		snd_PlaySound(sndID, hitPos);
	}
}

void obj_Airstrike::onHit(const r3dPoint3D& pos, const r3dPoint3D& norm, const r3dPoint3D& fly_dir)
{
	setActiveFlag(0);
}

void obj_Airstrike::OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace)
{
	return;
}

void MatrixGetYawPitchRoll ( const D3DXMATRIX & , float &, float &, float & );
BOOL obj_Airstrike::Update()
{
	parent::Update();
	if(m_CreationTime+30.0f < r3dGetTime()) // use hard coded 30 sec timeout. otherwise designers forget to set this variable and then they weapons are not working
		return FALSE;

	// this formula is duplicated on server. Do not change it!
	r3dVector motion = m_AppliedVelocity * r3dGetFrameTime();
	m_DistanceTraveled += motion.Length();

	if(m_DistanceTraveled >= m_DistanceUntilExplosion)
	{
		m_CollisionPoint = m_HitDestination;
		m_CollisionNormal = r3dPoint3D(0,1,0);
		onHit(m_CollisionPoint, m_CollisionNormal, m_FireDirection);
		return FALSE;
	}

	// perform movement
	{
		SetPosition(GetPosition() + motion);
	}

	// acceleration
	{
		m_AppliedVelocity += m_AppliedVelocity * r3dGetFrameTime();
	}

	if(m_ParticleTracer)
		m_ParticleTracer->SetPosition(GetPosition());

	return TRUE;
}

struct AirstrikeSharedDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		AirstrikeSharedDeferredRenderable* This = static_cast< AirstrikeSharedDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	obj_Airstrike* Parent;
};

#ifndef WO_SERVER
void obj_Airstrike::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if(getModel())
	{
		uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
		getModel()->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
		for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
		{
			AirstrikeSharedDeferredRenderable& rend = static_cast<AirstrikeSharedDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
			rend.Init();
			rend.Parent = this;
		}
	}
}
#endif

