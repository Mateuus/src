#ifndef __PWAR_OBJECT_BUILDING_H
#define __PWAR_OBJECT_BUILDING_H

#include "UI\UIimEdit.h"

class obj_ParticleSystem ;

class obj_Building : public MeshGameObject
{
	DECLARE_CLASS(obj_Building, MeshGameObject)

public:

	int				m_bAnimated;
	int				m_bGlobalAnimFolder;
	char			m_sAnimName[256];
	r3dSkeleton*	m_BindSkeleton; // //@TODO: make it shared for every same object name
	int				m_IsSkeletonShared ;
	r3dAnimPool		m_AnimPool;
	r3dAnimation	m_Animation;

	obj_ParticleSystem*	m_DestructionParticles ;
	int							m_DestructionSoundID ;

	bool			NeedDrawAnimated(const r3dCamera& Cam);
	void			DrawAnimated(const r3dCamera& Cam, bool shadow_pass);
	void			ChangeAnim();

	void			DestroyBuilding();
	void			FixBuilding();
	
public:
	obj_Building();
	virtual	~obj_Building();

	static void	LoadSkeleton( const char* baseMeshFName, r3dSkeleton** oSkeleton, int * oIsSkeletonShared ) ;

	virtual	BOOL		OnCreate();
	virtual	BOOL		Load(const char* fname);
	virtual	BOOL		Update();

	virtual GameObject *Clone ();

	virtual BOOL		GetObjStat ( char * sStr, int iLen );

	virtual	void 		SetPosition(const r3dPoint3D& pos);

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected ) OVERRIDE;
	virtual void		DrawSelected( const r3dCamera& Cam, eRenderStageID DrawState );
#endif

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendShadowRenderables( RenderArray& rarr, const r3dCamera& Cam ) OVERRIDE;

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	virtual void		UpdateDestructionData();

protected:
	void PostCloneParamsCopy(obj_Building *pNew);
};

#endif	// __PWAR_OBJECT_BUILDING_H
