//=========================================================================
//	Module: ZombieStates.h
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#pragma once

#if ENABLE_ZOMBIES

#include "../ai/StateMachine.h"
#include "obj_Zombie.h"

//////////////////////////////////////////////////////////////////////////

class ZombieLookForTargetState: public r3dState<obj_Zombie>
{
public:
	virtual void Enter(obj_Zombie *o);
	virtual void Execute(obj_Zombie *o);
	virtual void Exit(obj_Zombie *o);
};

//////////////////////////////////////////////////////////////////////////

class ZombieMoveToTargetState: public r3dState<obj_Zombie>
{
public:
	virtual void Enter(obj_Zombie *o);
	virtual void Execute(obj_Zombie *o);
	virtual void Exit(obj_Zombie *o);
};

//////////////////////////////////////////////////////////////////////////

class ZombieAttackTargetState: public r3dState<obj_Zombie>
{
public:
	virtual void Enter(obj_Zombie *o);
	virtual void Execute(obj_Zombie *o);
	virtual void Exit(obj_Zombie *o);
};

//////////////////////////////////////////////////////////////////////////

class ZombieDieState: public r3dState<obj_Zombie>
{
public:
	virtual void Enter(obj_Zombie *o);
	virtual void Execute(obj_Zombie *o);
	virtual void Exit(obj_Zombie *o);
};

//////////////////////////////////////////////////////////////////////////

class ZombieGlobalState: public r3dState<obj_Zombie>
{
public:
	virtual void Enter(obj_Zombie *o);
	virtual void Execute(obj_Zombie *o);
	virtual void Exit(obj_Zombie *o);
};

//////////////////////////////////////////////////////////////////////////

extern ZombieLookForTargetState gZombieLookForTargetState;
extern ZombieMoveToTargetState gZombieMoveToTargetState;
extern ZombieAttackTargetState gZombieAttackTargetState;
extern ZombieDieState gZombieDieState;
extern ZombieGlobalState gZombieGlobalState;

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_ZOMBIES
