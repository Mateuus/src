//=========================================================================
//	Module: obj_ZombieSpawn.h
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#pragma once

#if ENABLE_ZOMBIES

#include "GameCommon.h"
#include "gameobjects/GameObj.h"
#include "obj_Zombie.h"

//////////////////////////////////////////////////////////////////////////

extern int gSndZombieBreatheID;
extern int gSndZombieGrowlID;
extern int gSndZombieAttack1ID;
extern int gSndZombieAttack2ID;

//////////////////////////////////////////////////////////////////////////

class obj_ZombieSpawn: public GameObject
{
	DECLARE_CLASS(obj_ZombieSpawn, GameObject)

	float timeStamp;
	float timeSinceLastSpawn;
	r3dTL::TArray<obj_Zombie*> zombies;

	/**	Zombie spawn radius. */
	float spawnRadius;

	/**	Max zombie count. */
	uint32_t maxZombieCount;

	/**	Minmax zombie parameters. */
	float minDetectionRadius;
	float maxDetectionRadius;

	float minSpeed;
	float maxSpeed;

	/**	Zombie spawn rate (zombies per sec) */
	float zombieSpawnRate;

	/**	Body parts paths. */
	typedef r3dTL::TArray<std::string> Names;
	Names zombiePartNames[ZOMBIE_BODY_PARTS_COUNT];

#ifndef FINAL_BUILD
	float SelectNewZombiePartElement(float y, int &arrIdx);
#endif

public:
	obj_ZombieSpawn();
	~obj_ZombieSpawn();

	virtual void AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam);
#ifndef FINAL_BUILD
	virtual float DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected);
#endif
	virtual BOOL OnCreate();
	virtual BOOL Update();
	virtual BOOL OnDestroy();
	virtual	void ReadSerializedData(pugi::xml_node& node);
	virtual void WriteSerializedData(pugi::xml_node& node);

	void DeleteZombie(obj_Zombie *z);

	float GetRandDetectionRadius() const;
	float GetRandSpeed() const;

	void GetRandomBodyPartNames(std::string (&partNames)[ZOMBIE_BODY_PARTS_COUNT]) const;
};

#endif // ENABLE_ZOMBIES
