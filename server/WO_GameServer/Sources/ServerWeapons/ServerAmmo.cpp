#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "ServerAmmo.h"

Ammo::Ammo(const char* ammoName)
{
	r3d_assert(ammoName);
	r3d_assert(strlen(ammoName)<32);
	r3dscpy(m_Name, ammoName);

	m_ModelPath = NULL;
	m_ParticleTracer = 0;
	m_ShellExtractParticle = 0;
	m_DecalSource = NULL;
	m_BulletClass = 0;
}

Ammo::~Ammo()
{
	free(m_ModelPath);
	free(m_ParticleTracer);
	free(m_ShellExtractParticle);
	free(m_DecalSource);
	free(m_BulletClass);
}
