#pragma once

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"

class Ammo
{
	friend class WeaponArmory;
public:
	Ammo(const char* ammoName);
	~Ammo();
	
	void	unloadModel() {} // not used, called from WeaponArmory
private:
	// config
	char m_Name[32];
	char*	m_ModelPath;
	char*	m_ParticleTracer; // may be null
	char*	m_ShellExtractParticle; // may be null
	char* m_DecalSource;
	char* m_BulletClass;
};
