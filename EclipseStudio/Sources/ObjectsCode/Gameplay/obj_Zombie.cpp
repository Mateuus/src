//=========================================================================
//	Module: obj_Zombie.cpp
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"

#if ENABLE_ZOMBIES

#include "obj_Zombie.h"
#include "ai/NavMeshActor.h"
#include "../AI/r3dPhysSkeleton.h"
#include "obj_ZombieSpawn.h"

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(obj_Zombie, "obj_Zombie", "Object");
AUTOREGISTER_CLASS(obj_Zombie);

extern r3dSec_type<CUberData*, 0x345fd82a> AI_Player_UberData;
r3dPhysSkeleton* AquireCacheSkeleton();
void ReleaseCachedPhysSkeleton(r3dPhysSkeleton* skel);

/////////////////////////////////////////////////////////////////////////

namespace
{
	struct ZombieMeshesDeferredRenderable: public MeshDeferredRenderable
	{
		ZombieMeshesDeferredRenderable()
		: Parent(0)
		{
		}

		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw(Renderable* RThis, const r3dCamera& Cam)
		{
			R3DPROFILE_FUNCTION("ZombieMeshesDeferredRenderable");

			ZombieMeshesDeferredRenderable* This = static_cast<ZombieMeshesDeferredRenderable*>(RThis);

			r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
			This->Parent->OnPreRender();

			MeshDeferredRenderable::Draw(RThis, Cam);
		}

		obj_Zombie *Parent;
	};

//////////////////////////////////////////////////////////////////////////

	struct ZombieMeshesShadowRenderable: public MeshShadowRenderable
	{
		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw( Renderable* RThis, const r3dCamera& Cam )
		{
			R3DPROFILE_FUNCTION("ZombieMeshesShadowRenderable");
			ZombieMeshesShadowRenderable* This = static_cast< ZombieMeshesShadowRenderable* >( RThis );

			r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);

			This->Parent->OnPreRender();

			This->SubDrawFunc( RThis, Cam );
		}

		obj_Zombie *Parent;
	};
} // unnamed namespace

//////////////////////////////////////////////////////////////////////////

obj_Zombie::obj_Zombie()
: navAgentIdx(INVALID_NAVIGATION_AGENT_ID)
, chasedTarget(invalidGameObjectID)
, fsm(0)
, physSkeleton(0)
, deadTime(0)
, spawn(0)
, uberAnim(0)
, lastTargetPos(0, 0, 0)
, playingSndHandle(0)
{
	ObjTypeFlags |= OBJTYPE_Zombie;
	m_bEnablePhysics = false;

	if (!AI_Player_UberData)
	{
		AI_Player_UberData = new CUberData;
	}

	m_isSerializable = false;

	ZeroMemory(zombieParts, _countof(zombieParts) * sizeof(zombieParts[0]));
}

//////////////////////////////////////////////////////////////////////////

obj_Zombie::~obj_Zombie()
{

}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Zombie::OnCreate()
{
	parent::OnCreate();

	//	Attach navigation agent to zombie
	aiParams.maxSpeed = spawn->GetRandSpeed();
	aiParams.maxAcceleration = 15.0f;
	cfg.speed = aiParams.maxSpeed;
	cfg.detectionRadius = spawn->GetRandDetectionRadius();

	fsm = new r3dFiniteStateMachine<obj_Zombie>(this);

	uberAnim = new CUberAnim(0, AI_Player_UberData);
	fsm->ChangeState(&gZombieLookForTargetState);
	fsm->SetGlobalState(&gZombieGlobalState);

	std::string partNames[ZOMBIE_BODY_PARTS_COUNT];
	spawn->GetRandomBodyPartNames(partNames);

	r3dBoundBox bbox;
	bool fullMesh = true;
	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		if (!partNames[i].empty())
		{
			zombieParts[i] = r3dGOBAddMesh(partNames[i].c_str());
		}
		if (zombieParts[i])
			bbox.ExpandTo(zombieParts[i]->localBBox);
	}
	SetBBoxLocal(bbox);

	UpdateAnimations();

	physSkeleton = AquireCacheSkeleton();
 	physSkeleton->linkParent(uberAnim->anim.pSkeleton, GetTransformMatrix(), this, PHYSCOLL_NETWORKPLAYER) ;
	physSkeleton->SwitchToRagdoll(false);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Zombie::Update()
{
	parent::Update();
	UpdateAnimations();
	fsm->Update();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void obj_Zombie::UpdateAnimations()
{
	const float TimePassed = r3dGetFrameTime();
#if ENABLE_RAGDOLL
	bool ragdoll = physSkeleton && physSkeleton->IsRagdollMode();
	if (!ragdoll)
#endif
	{
		uberAnim->anim.Update(TimePassed, r3dPoint3D(0,0,0), GetTransformMatrix());
		uberAnim->anim.Recalc();
	}

	if(physSkeleton)
		physSkeleton->syncAnimation(uberAnim->anim.pSkeleton, GetTransformMatrix(), uberAnim->anim);

#if ENABLE_RAGDOLL
	if (ragdoll)
	{
		r3dBoundBox bbox = physSkeleton->getWorldBBox();
		SetPosition(bbox.Org + bbox.Size * 0.5f);
		bbox.Org = -bbox.Size * 0.5;
		SetBBoxLocal(bbox);
	}
#endif

}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Zombie::OnDestroy()
{
	parent::OnDestroy();

	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		r3dGOBReleaseMesh(zombieParts[i]);
	}

	gNavMeshActorsManager.RemoveAgent(navAgentIdx);

	SAFE_DELETE(uberAnim);
	SAFE_DELETE(fsm);

	ReleaseCachedPhysSkeleton(physSkeleton);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void obj_Zombie::OnPreRender()
{
	uberAnim->anim.GetCurrentSkeleton()->SetShaderConstants();
}

//////////////////////////////////////////////////////////////////////////

void obj_Zombie::Die(const r3dPoint3D &hitRay)
{
	deathHitVel = hitRay * 20;
	fsm->ChangeState(&gZombieDieState);
}

//////////////////////////////////////////////////////////////////////////

#ifndef FINAL_BUILD
void obj_Zombie::DrawDebugInfo() const
{
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_ZC | R3D_BLEND_PUSH);
	int psId = r3dRenderer->GetCurrentPixelShaderIdx();
	int vsId = r3dRenderer->GetCurrentVertexShaderIdx();
	r3dRenderer->SetPixelShader();
	r3dRenderer->SetVertexShader();
	r3dRenderer->SetTex(0, 0);

	//	Visibility dist
	r3dDrawCircle3D(GetPosition(), cfg.detectionRadius, gCam, 0.1f, r3dColor::white);

	r3dRenderer->Flush();
	r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
	r3dRenderer->SetPixelShader(psId);
	r3dRenderer->SetVertexShader(vsId);
}
#endif

//////////////////////////////////////////////////////////////////////////

void obj_Zombie::AppendRenderables(RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& cam)
{
	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		r3dMesh *mesh = zombieParts[i];

		if (!mesh)
			continue;

		uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
		mesh->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );

		for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
		{
			ZombieMeshesDeferredRenderable& rend = static_cast<ZombieMeshesDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] ) ;

			rend.Init();
			rend.Parent = this;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_Zombie::AppendShadowRenderables(RenderArray &rarr, const r3dCamera& cam)
{
	float distSq = (gCam - GetPosition()).LengthSq();
	float dist = sqrtf( distSq );
	UINT32 idist = (UINT32) R3D_MIN( dist * 64.f, (float)0x3fffffff );

	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		r3dMesh *mesh = zombieParts[i];
		
		if (!mesh)
			continue;

		uint32_t prevCount = rarr.Count();

		mesh->AppendShadowRenderables( rarr );

		for( uint32_t i = prevCount, e = rarr.Count(); i < e; i ++ )
		{
			ZombieMeshesShadowRenderable& rend = static_cast<ZombieMeshesShadowRenderable&>( rarr[ i ] );

			rend.Init() ;
			rend.SortValue |= idist;
			rend.Parent = this ;
		}
	}
}

#endif // ENABLE_ZOMBIES
