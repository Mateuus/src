#pragma once

#include "GameCommon.h"

class BaseControlPoint : public MeshGameObject
{
	DECLARE_CLASS(BaseControlPoint, MeshGameObject)

  public:
	enum type_e {
	  SPAWN_BLUE,
	  SPAWN_RED,
	  SPAWN_FREE,
	};
	static const float OWN_THRESHOLD;
	
	int		spawnType_;
	bool	m_isTeamBase; 
	float		status_;
	float		capture_radius;
	float		spawnProtectionTime; // each spawn point has individual spawn protection timer
	void		UpdateStatusByType();
	
	int		GetSpawnTeamId() const {
	  if(status_ <= -OWN_THRESHOLD) return 0;	// team_blue
	  if(status_ >=  OWN_THRESHOLD) return 1;	// team_red
	  return 2;				// team_unknown
	}

	static const int MAX_SPAWN_POINTS = 16;
	r3dPoint3D m_SpawnPoints[MAX_SPAWN_POINTS];
	r3dPoint3D m_SpawnDir[MAX_SPAWN_POINTS]; // y value holds angle value
	int			m_NumSpawnPoints;

	void getSpawnPoint(r3dPoint3D& pos, float& dir) const
	{
		r3d_assert(m_NumSpawnPoints > 0 && m_NumSpawnPoints <= MAX_SPAWN_POINTS);
		int i = random(m_NumSpawnPoints-1);
		r3d_assert(i>=0 && i < MAX_SPAWN_POINTS);
		pos = m_SpawnPoints[i];
		dir = m_SpawnDir[i].y;
	}

	void getSpawnPointByIdx(int index, r3dPoint3D& pos, float& dir) const
	{
		r3d_assert(m_NumSpawnPoints > 0 && m_NumSpawnPoints <= MAX_SPAWN_POINTS);
		r3d_assert(index>=0 && index < MAX_SPAWN_POINTS);
		pos = m_SpawnPoints[index];
		dir = m_SpawnDir[index].y;
	}

  public:
	BaseControlPoint();
	virtual ~BaseControlPoint();
	
	// virtual stuffs - server implementation will override that.
	virtual void	HostSendUpdate() { r3d_assert(0); }

	virtual void		WriteSerializedData(pugi::xml_node& node);
	virtual	void		ReadSerializedData(pugi::xml_node& node);

};


class ControlPointsMgr
{
  public:
	// control points stuff
	enum { MAX_CONTROL_POINTS = 50 };
	BaseControlPoint* controlPoints_[MAX_CONTROL_POINTS];
	int		numControlPoints_;
	int		neutralControlPoints[MAX_CONTROL_POINTS];
	int		numNeutralControlPoints;

	int		RegisterControlPoint(BaseControlPoint* cp) {
	  r3d_assert(numControlPoints_ < MAX_CONTROL_POINTS);
	  controlPoints_[numControlPoints_++] = cp;

	  // register neutral points for later use
	  if(!cp->m_isTeamBase)
		  neutralControlPoints[numNeutralControlPoints++] = numControlPoints_-1;

	  return numControlPoints_ - 1;
	}

  public:
	ControlPointsMgr();
	~ControlPointsMgr();

	void		Reset() {
	  numControlPoints_ = 0;
	  numNeutralControlPoints = 0;
	}
	
	void		CheckForNeededPoints();
	
	int		GetControlledPoints(int& out_yours, int& out_their, int teamID); // out_yours/out_their will have num controlled points excluding BASE points. fn returns total number of capturable points

	int		NumCP() const {
	  return numControlPoints_;
	}
	
	BaseControlPoint* GetCP(int idx) {
	  r3d_assert(idx >= 0 && idx < numControlPoints_);
	  return controlPoints_[idx];
	}

	int getCPIndex(BaseControlPoint* cp)
	{
		for(int i =0; i<numControlPoints_; ++i)
			if(controlPoints_[i] == cp)
				return i;

		r3dError("Unregistered CP");
		return 0;
	}

	int GetBaseIndex(int teamID)
	{
		for(int i=0; i<numControlPoints_; ++i)
		{
			if(controlPoints_[i]->m_isTeamBase && controlPoints_[i]->GetSpawnTeamId()==teamID)
				return i;
		}
		r3d_assert(false); // WTF?
		return -1;
	}

	BaseControlPoint* GetBase(int teamID)
	{
		for(int i=0; i<numControlPoints_; ++i)
		{
			if(controlPoints_[i]->m_isTeamBase && controlPoints_[i]->GetSpawnTeamId()==teamID)
				return controlPoints_[i];
		}
		r3d_assert(false); // WTF?
		return 0;
	}
};

extern	ControlPointsMgr gCPMgr;
