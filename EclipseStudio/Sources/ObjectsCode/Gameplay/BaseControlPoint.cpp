#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "BaseControlPoint.h"

#include "multiplayer/P2PMessages.h"

ControlPointsMgr	gCPMgr;

const float BaseControlPoint::OWN_THRESHOLD = 0.25f;

ControlPointsMgr::ControlPointsMgr()
{
	numControlPoints_ = 0;
}

ControlPointsMgr::~ControlPointsMgr()
{
}

void ControlPointsMgr::CheckForNeededPoints()
{
	// control points must be assigned for both teams
	int cp1 = -1;
	int cp2 = -1;

	for(int i=0; i<numControlPoints_; i++) {
		if(controlPoints_[i]->GetSpawnTeamId() == 0) {
			cp1 = i;
		}
		if(controlPoints_[i]->GetSpawnTeamId() == 1) {
			cp2 = i;
		}
	}

	if(cp1 == -1) {
		r3dError("!!!error!!! there is no control point for blue team\n");
	}

	if(cp2 == -1) {
		r3dError("!!!error!!! there is no control point for red team\n");
	}

	return;
}

int ControlPointsMgr::GetControlledPoints(int& out_yours, int& out_theirs, int teamID)
{
	r3d_assert(teamID==0 || teamID == 1);
	int blue = 0;
	int red  = 0;
	int total = 0;

	for(int i=0; i<numControlPoints_; i++) 
	{
		int cpTeamId = controlPoints_[i]->GetSpawnTeamId();
		if(controlPoints_[i]->m_isTeamBase) // skip bases
			continue;
		if(cpTeamId == teamID) 
			blue++;
		else if(cpTeamId == (1-teamID)) 
			red++;
		total++;
	}

	out_yours  = blue;
	out_theirs = red;
	return total;
}


IMPLEMENT_CLASS(BaseControlPoint, "BaseControlPoint", "Object");
//AUTOREGISTER_CLASS(BaseControlPoint);

BaseControlPoint::BaseControlPoint()
{
	status_    = 0.0f;
	capture_radius = 8.0f;
	spawnProtectionTime = 5.0f;
	spawnType_ = SPAWN_FREE;
	m_NumSpawnPoints = 0;
	m_isTeamBase = false;
}

BaseControlPoint::~BaseControlPoint()
{
}

void BaseControlPoint::UpdateStatusByType()
{
	// set status
	switch(spawnType_) {
	default: r3d_assert(0);
	case SPAWN_BLUE: status_ = -1.0f; break;
	case SPAWN_RED:  status_ =  1.0f; break;
	case SPAWN_FREE: status_ =  0.0f; break;
	}

	return;
}

void BaseControlPoint::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node cpNode = node.child("baseControlPoint");
	spawnType_ = cpNode.attribute("spawnType").as_int();
	if(spawnType_ == SPAWN_FREE)
		m_isTeamBase = false;
	else
		m_isTeamBase = true;

	if(!cpNode.attribute("radius").empty())
		capture_radius = cpNode.attribute("radius").as_float();
	if(!cpNode.attribute("spawnProtectionTime").empty())
		spawnProtectionTime = cpNode.attribute("spawnProtectionTime").as_float();

	m_NumSpawnPoints = cpNode.attribute("numSpawnPoints").as_int();
	r3d_assert(m_NumSpawnPoints >=0 && m_NumSpawnPoints <= MAX_SPAWN_POINTS);
	if(m_NumSpawnPoints == 0) // add default
	{
		m_NumSpawnPoints = 1;
		m_SpawnPoints[0] = GetPosition()+r3dPoint3D(2,0,2);
		m_SpawnDir[0] = r3dPoint3D(0,0,1);
	}
	else
	{
		char tempStr[32];
		for(int i=0; i<m_NumSpawnPoints; ++i)
		{
			sprintf(tempStr, "point%d", i);
			pugi::xml_node spawnNode = cpNode.child(tempStr);
			r3d_assert(!spawnNode.empty());
			m_SpawnPoints[i].x = spawnNode.attribute("x").as_float();
			m_SpawnPoints[i].y = spawnNode.attribute("y").as_float();
			m_SpawnPoints[i].z = spawnNode.attribute("z").as_float();

			sprintf(tempStr, "dir%d", i);
			pugi::xml_node dirNode = cpNode.child(tempStr);
			m_SpawnDir[i].Assign(0,0,1);
			if(!dirNode.empty())
			{
				m_SpawnDir[i].x = dirNode.attribute("x").as_float();
				m_SpawnDir[i].y = dirNode.attribute("y").as_float();
				m_SpawnDir[i].z = dirNode.attribute("z").as_float();
			}
		}
	}
	UpdateStatusByType();
}

void BaseControlPoint::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node cpNode = node.append_child();
	cpNode.set_name("baseControlPoint");
	cpNode.append_attribute("spawnType") = spawnType_;
	cpNode.append_attribute("radius") = capture_radius;
	cpNode.append_attribute("spawnProtectionTime") = spawnProtectionTime;
	cpNode.append_attribute("numSpawnPoints") = m_NumSpawnPoints;
	for(int i=0; i<m_NumSpawnPoints; ++i)
	{
		pugi::xml_node spawnNode = cpNode.append_child();
		char tempStr[32];
		sprintf(tempStr, "point%d", i);
		spawnNode.set_name(tempStr);
		spawnNode.append_attribute("x") = m_SpawnPoints[i].x;
		spawnNode.append_attribute("y") = m_SpawnPoints[i].y;
		spawnNode.append_attribute("z") = m_SpawnPoints[i].z;

		pugi::xml_node dirNode = cpNode.append_child();
		sprintf(tempStr, "dir%d", i);
		dirNode.set_name(tempStr);
		dirNode.append_attribute("x") = m_SpawnDir[i].x;
		dirNode.append_attribute("y") = m_SpawnDir[i].y;
		dirNode.append_attribute("z") = m_SpawnDir[i].z;
	}
}

