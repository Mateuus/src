#pragma once

#include "GameCommon.h"

class obj_AI_Player;

class obj_SupplyCrate : public MeshGameObject
{
	DECLARE_CLASS(obj_SupplyCrate, MeshGameObject)
	const char* m_SupplyMeshPath;
	int		needToAddToMinimap;
	char	SupplyCrateName[32];
	int		SupplyType; // 0 - regular, 1-blackops
public:
	obj_SupplyCrate();
	virtual ~obj_SupplyCrate();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
};


class obj_BlackopsCrate : public obj_SupplyCrate
{
	DECLARE_CLASS(obj_BlackopsCrate, obj_SupplyCrate)
public:
	obj_BlackopsCrate();
	virtual ~obj_BlackopsCrate() {};
};

class SupplyCrateManager
{
private:
	obj_SupplyCrate* m_supplyCrates[64];
	int				 m_numCrates;

public:
	SupplyCrateManager() : m_numCrates(0) {}
	~SupplyCrateManager() {}

	void Reset() { m_numCrates = 0; }

	void RegisterSupplyCrate(obj_SupplyCrate* crate)
	{
		m_supplyCrates[m_numCrates++] = crate;
		r3d_assert(m_numCrates < 64);
	}

	int getNumCrates() const { return m_numCrates; }

	obj_SupplyCrate* getCrateInRadius(const r3dVector& pos, const float radius)
	{
		for(int i =0; i<m_numCrates; ++i)
		{
			float dist =(m_supplyCrates[i]->GetPosition() - pos).LengthSq();
			if(dist < (radius*radius))
				return m_supplyCrates[i];
		}
		return NULL;
	}
} extern g_SupplyCrateMngr;
