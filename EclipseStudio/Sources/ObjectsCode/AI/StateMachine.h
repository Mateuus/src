//=========================================================================
//	Module: StateMachine.h
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#pragma once

//////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------
//	State class
//-------------------------------------------------------------------------

template <typename EntityType>
class r3dState
{
public:
	virtual void Enter(EntityType *o) = 0;
	virtual void Execute(EntityType *o) = 0;
	virtual void Exit(EntityType *o) = 0;
};

//-------------------------------------------------------------------------
//	General purpose state machine class
//-------------------------------------------------------------------------

template <typename EntityType>
class r3dFiniteStateMachine
{
public:
	typedef r3dState<EntityType> EntityState;

	explicit r3dFiniteStateMachine(EntityType *o)
	: owner(o)
	, currentState(0)
	, prevState(0)
	, globalState(0)
	{}

	void SetCurrentState(EntityState *s)			{ currentState = s; }
	void SetPreviousState(EntityState *s)			{ prevState = s; }
	void SetGlobalState(EntityState *s)				{ globalState = s; }
	EntityState * GetCurrentState(EntityState *s)	{ return currentState; }
	EntityState * GetPreviousState(EntityState *s)	{ return prevState; }

	/**	Update FSM. */
	void Update()
	{
		if (currentState)
			currentState->Execute(owner);
		if (globalState)
			globalState->Execute(owner);
	}

	/**	Change FSM to new state. */
	void ChangeState(EntityState *newState)
	{
		r3d_assert(newState);
		prevState = currentState;
		if (currentState)
			currentState->Exit(owner);
		currentState = newState;
		currentState->Enter(owner);
	}

	void RevertToPreviousState()
	{
		ChangeState(prevState);
	}

	bool IsInState(const EntityState &st) const
	{
		return &st == currentState;
	}

private:
	EntityType *owner;
	EntityState *currentState;
	EntityState *prevState;
	EntityState *globalState;
};

