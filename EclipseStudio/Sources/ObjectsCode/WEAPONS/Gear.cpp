#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "Gear.h"

r3dMesh* GearConfig::getMesh() const
{
	r3d_assert( m_Model ) ;
	return m_Model ;
}

r3dMesh* GearConfig::getFirstPersonMesh() const
{
	if(m_FirstPersonModel == 0)
	{
#ifndef FINAL_BUILD
		extern bool g_bEditMode;
		if(g_bEditMode && !r3dFileExists(m_ModelPath_1st))
			return NULL;
#endif
		m_FirstPersonModel = r3dGOBAddMesh(m_ModelPath_1st, true, false, true, true );
		if(m_FirstPersonModel==NULL)
		{
			r3dError("ART: failed to load mesh '%s'\n", m_ModelPath_1st);
		}
		r3d_assert(m_FirstPersonModel);
	}

	return m_FirstPersonModel;
}

int GearConfig::getMeshRefs() const
{
	return m_Model ? m_Model->RefCount : 0 ;
}

//------------------------------------------------------------------------

int GearConfig::getConfigMeshRefs() const
{
	return m_ModelRefCount ;
}

void GearConfig::aquireMesh() const
{
	if( !m_Model )
	{
		m_Model = r3dGOBAddMesh(m_ModelPath, true, false, true, true );

		if(m_Model==0)
		{
			r3dError("ART: failed to load mesh '%s'\n", m_ModelPath);
		}
		r3d_assert(m_Model);

		r3d_assert( !m_ModelRefCount ) ;

		m_ModelRefCount = 1 ;
	}
	else
	{
		r3d_assert( !r_allow_ingame_unloading->GetInt() || m_ModelRefCount ) ;
		m_ModelRefCount ++ ;
	}
}

void GearConfig::releaseMesh() const
{
	if( r_allow_ingame_unloading->GetInt() )
	{
		if( m_ModelRefCount == 1 )
		{
			r3d_assert( m_Model ) ;

			r3dGOBReleaseMesh( m_Model ) ;

			m_Model = 0 ;
		}
	}

	m_ModelRefCount -- ;
	r3d_assert( m_ModelRefCount >= 0 ) ;
}

int g_GearBalance ;


Gear::Gear(const GearConfig* conf) : m_pConfig(conf)
{
	Reset();

	g_GearBalance ++ ;

	m_pConfig->aquireMesh() ;
}

Gear::~Gear()
{
	g_GearBalance -- ;

	m_pConfig->releaseMesh() ;
}

void Gear::Reset()
{
}


