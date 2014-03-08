//=========================================================================
//	Module: obj_Zombie.h
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#pragma once

#if ENABLE_ZOMBIES

#include "gameobjects/obj_Mesh.h"
#include "../ai/AI_PlayerAnim.h"
#include "ZombieStates.h"
#include "ai/NavMeshActor.h"

//////////////////////////////////////////////////////////////////////////

const int ZOMBIE_BODY_PARTS_COUNT = 3;

//////////////////////////////////////////////////////////////////////////

class obj_AI_Player;
class r3dPhysSkeleton;
class obj_ZombieSpawn;

class obj_Zombie: public GameObject
{
	DECLARE_CLASS(obj_Zombie, GameObject)

public:
	/**	Configuration structure. All parameters except speed in range [0..100]. */
	struct Config
	{
		float detectionRadius;
		float speed;

		Config()
		: detectionRadius(1.0f)
		, speed(3.0f)
		{}
	};

	obj_Zombie();
	~obj_Zombie();

	virtual	BOOL OnCreate();
	virtual BOOL Update();
	virtual BOOL OnDestroy();
	const Config & GetAIConfig() const { return cfg; }
	virtual void OnPreRender();
	void Die(const r3dPoint3D &hitRay);
	void SetSpawn(obj_ZombieSpawn *s) { spawn = s; }

	virtual void AppendShadowRenderables(RenderArray &rarr, const r3dCamera& cam);
	virtual void AppendRenderables(RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& cam);

#ifndef FINAL_BUILD
	void DrawDebugInfo() const;
#endif

private:
	friend class ZombieLookForTargetState;
	friend class ZombieMoveToTargetState;
	friend class ZombieAttackTargetState;
	friend class ZombieDieState;
	friend class ZombieGlobalState;

	/**	Navigation agent idx. */
	int navAgentIdx;

	/**	Chased player game object id. */
	gobjid_t chasedTarget;

	/**	Last seen chased target pos. Used for i.e. "soft-lock" targeting. */
	r3dPoint3D lastTargetPos;

	/**	Zombie AI config. */
	Config cfg;

	/**	Zombie animation system. */
	CUberAnim *uberAnim;

	/**	Zombie AI state machine. */
	r3dFiniteStateMachine<obj_Zombie> *fsm;

	/**	Zombie ragdoll. */
	r3dPhysSkeleton *physSkeleton;

	/**	Time since zombie death. */
	float deadTime;

	/**	Spawn that produce this zombie. */
	obj_ZombieSpawn *spawn;

	/**	Death hit velocity. */
	r3dPoint3D deathHitVel;

	/**	AI navigation agent parameters. */
	r3dNavAgentParams aiParams;

	/**	Array of meshes with zombie parts. */
	r3dMesh * zombieParts[ZOMBIE_BODY_PARTS_COUNT];

	/**	Currently played sound. */
	void *playingSndHandle;

	void UpdateAnimations();
};

#endif // ENABLE_ZOMBIES
