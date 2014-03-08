//=========================================================================
//	Module: NavMeshActor.cpp
//	Copyright (C) 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"

#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "NavMeshActor.h"

//////////////////////////////////////////////////////////////////////////

extern float gNavAgentHeight;
extern float gNavAgentRadius;
extern float gNavAgentMaxClimb;

const int MAX_NUM_NAV_AGENTS = 100;
const float agentAcceleration = 100.0f;
const float agentMaxSpeed = 10.5f;

extern r3dCamera gCam;
r3dNavMeshActorsManager gNavMeshActorsManager;

//////////////////////////////////////////////////////////////////////////

r3dNavAgentParams::r3dNavAgentParams()
{
	ZeroMemory(this, sizeof(*this));
	height = gNavAgentHeight;
	radius = gNavAgentRadius;
	maxAcceleration = agentAcceleration;
	maxSpeed = agentMaxSpeed;
	collisionQueryRange = radius * 12.0f;
	pathOptimizationRange = radius * 30.0f;
	updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	obstacleAvoidanceType = 3;
	separationWeight = 2.0f;
}

//-------------------------------------------------------------------------
//	r3dNavMeshActorsManager
//-------------------------------------------------------------------------

r3dNavMeshActorsManager::r3dNavMeshActorsManager()
: crowd(dtAllocCrowd())
, inited(false)
{
	::ZeroMemory(&crowdDebugInfo, sizeof(crowdDebugInfo));
}

//////////////////////////////////////////////////////////////////////////

r3dNavMeshActorsManager::~r3dNavMeshActorsManager()
{
	if (crowd)
		dtFreeCrowd(crowd);
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavMeshActorsManager::Init(r3dNavigationMesh &navMesh)
{
	if (!crowd)
	{
		inited = false;
		return inited;
	}

	inited = crowd->init(MAX_NUM_NAV_AGENTS, gNavAgentRadius, navMesh.navMesh);

	// Setup local avoidance params to different qualities.
	dtObstacleAvoidanceParams params;
	// Use mostly default settings, copy from dtCrowd.
	memcpy(&params, crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

	// Low (11)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 1;
	crowd->setObstacleAvoidanceParams(0, &params);

	// Medium (22)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 2;
	crowd->setObstacleAvoidanceParams(1, &params);

	// Good (45)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 3;
	crowd->setObstacleAvoidanceParams(2, &params);

	// High (66)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;

	crowd->setObstacleAvoidanceParams(3, &params);

	return inited;
}

//////////////////////////////////////////////////////////////////////////

int r3dNavMeshActorsManager::CreateAgent(const r3dPoint3D &pos, const r3dNavAgentParams &agentParams)
{
	R3DPROFILE_FUNCTION("Nav::CreateAgent");
	int idx = INVALID_NAVIGATION_AGENT_ID;
	if (!inited)
		return idx;

	idx = crowd->addAgent(&pos.x, &agentParams);
	return idx;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::RemoveAgent(int agentId)
{
	R3DPROFILE_FUNCTION("Nav::RemoveAgent");
	if (!inited || agentId == INVALID_NAVIGATION_AGENT_ID || agentId >= crowd->getAgentCount())
		return;
	
	crowd->removeAgent(agentId);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::RemoveAllAgents()
{
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		RemoveAgent(i);
	}
}

//////////////////////////////////////////////////////////////////////////

r3dPoint3D r3dNavMeshActorsManager::GetPosition(int agentId) const
{
	r3dPoint3D pos(0, 0, 0);
	if (!inited || agentId == INVALID_NAVIGATION_AGENT_ID || agentId >= crowd->getAgentCount())
		return pos;

	const dtCrowdAgent *a = crowd->getAgent(agentId);
	if (!a)
		return pos;

	pos.Assign(a->npos[0], a->npos[1], a->npos[2]);
	return pos;
}

//////////////////////////////////////////////////////////////////////////

r3dPoint3D r3dNavMeshActorsManager::GetVelocity(int agentId) const
{
	r3dPoint3D vel(0, 0, 0);
	if (!inited || agentId == INVALID_NAVIGATION_AGENT_ID || agentId >= crowd->getAgentCount())
		return vel;

	const dtCrowdAgent *a = crowd->getAgent(agentId);
	if (!a)
		return vel;

	vel.Assign(a->vel[0], a->vel[1], a->vel[2]);
	return vel;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::NavigateCommonFunc(int agentId, const r3dPoint3D &pt, Fn fn)
{
	if (!inited || agentId == INVALID_NAVIGATION_AGENT_ID || agentId >= crowd->getAgentCount())
		return;

	const dtNavMeshQuery *nmq = crowd->getNavMeshQuery();
	if (!nmq)
		return;

	const dtQueryFilter* filter = crowd->getFilter();
	const float *queryExts = crowd->getQueryExtents();

	float targetPos[3] = {0};
	dtPolyRef targetPoly;

	dtStatus st = nmq->findNearestPoly(&pt.x, queryExts, filter, &targetPoly, targetPos);
	st;

	bool rv = (*crowd.*fn)(agentId, targetPoly, targetPos);
	rv;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::NavigateTo(int agentId, const r3dPoint3D &pt)
{
	R3DPROFILE_FUNCTION("Nav::NavigateTo");
	NavigateCommonFunc(agentId, pt, &dtCrowd::requestMoveTarget);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::AdjustMoveTarget(int agentId, const r3dPoint3D &pt)
{
	R3DPROFILE_FUNCTION("Nav::AdjustMoveTarget");
	NavigateCommonFunc(agentId, pt, &dtCrowd::adjustMoveTarget);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::NavigateAllTo(const r3dPoint3D &pt)
{
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		NavigateTo(i, pt);
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::Update()
{
	R3DPROFILE_FUNCTION("Nav::Update");
	if (!inited)
		return;

	crowd->update(r3dGetFrameTime(), &crowdDebugInfo);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavMeshActorsManager::DebugDrawAgents()
{
	if (!inited)
		return;

	int num = crowd->getAgentCount();
	for (int i = 0; i < num; ++i)
	{
		const dtCrowdAgent *a = crowd->getAgent(i);
		if (!a || !a->active)
			continue;

		r3dDrawBox3D(a->npos[0], a->npos[1], a->npos[2], a->params.radius, a->params.height, a->params.radius, r3dColor::white);
	}
}

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_RECAST_NAVIGATION