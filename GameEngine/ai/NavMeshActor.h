//=========================================================================
//	Module: NavMeshActor.h
//	Copyright (C) 2012.
//=========================================================================

#pragma once
#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "RecastAndDetour/DetourCrowd.h"
#include "NavMesh.h"

//////////////////////////////////////////////////////////////////////////

const int INVALID_NAVIGATION_AGENT_ID = -1;

//////////////////////////////////////////////////////////////////////////

struct r3dNavAgentParams: public dtCrowdAgentParams
{
	r3dNavAgentParams();
};

//////////////////////////////////////////////////////////////////////////

class r3dNavMeshActorsManager
{
	dtCrowd *crowd;
	dtCrowdAgentDebugInfo crowdDebugInfo;
	bool inited;

	typedef bool (dtCrowd::*Fn)(const int idx, dtPolyRef ref, const float* pos);
	void NavigateCommonFunc(int agentId, const r3dPoint3D &pt, Fn fn);

public:
	r3dNavMeshActorsManager();
	~r3dNavMeshActorsManager();
	bool Init(r3dNavigationMesh &navMesh);
	int CreateAgent(const r3dPoint3D &pos, const r3dNavAgentParams &agentParams = r3dNavAgentParams());
	void RemoveAgent(int agentId);
	void RemoveAllAgents();

	r3dPoint3D GetPosition(int agentId) const;
	r3dPoint3D GetVelocity(int agentId) const;

	void NavigateTo(int agentId, const r3dPoint3D &pt);
	void AdjustMoveTarget(int agentId, const r3dPoint3D &pt);
	void NavigateAllTo(const r3dPoint3D &pt);
	void Update();

	void DebugDrawAgents();
};

//////////////////////////////////////////////////////////////////////////

extern r3dNavMeshActorsManager gNavMeshActorsManager;
#endif // ENABLE_RECAST_NAVIGATION