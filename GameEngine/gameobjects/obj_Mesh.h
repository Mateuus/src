#ifndef __PWAR_OBJ_MESH_H
#define __PWAR_OBJ_MESH_H

#include "GameObj.h"

struct DamageLibEntry ;

void r3dGOBReloadMesh(const char* fname);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class MeshGameObject : public GameObject
{
	DECLARE_CLASS(MeshGameObject, GameObject)
public:

	enum
	{
		NUM_LODS = 4 
	};

	r3dMesh* 	MeshLOD[ NUM_LODS ];			// 0 - should be always, 1-3 - optional
	r3dMesh* 	DestructionMeshLOD[ NUM_LODS ];	// 0 - should be always, 1-3 - optional

	r3dMesh* 	PlayerOnly_CollisionMesh; // if available, only in editor!

	float	 		MeshLODDistSq[ NUM_LODS - 1 ];
	r3dColor24		m_ObjectColor;
	int				m_HitPoints;
	DamageLibEntry*	m_pDamageLibEntry ;

public:
	MeshGameObject();
	virtual				~MeshGameObject();

	virtual BOOL		Update();				// ObjMan: tick update
	void				Draw(const r3dCamera& Cam, eRenderStageID DrawState);

	int					ChoseMeshLOD( float distSq, r3dMesh* (&TargetLOD)[ NUM_LODS ] );
	int					ChoseMeshLOD( float distSq );
	int					ChooseMainCameraLOD();

	void				ReloadMeshData	( r3dMesh * pFreeMesh );

	void				ReloadDestructionMesh( const char* nameOfChangedMesh );
	void				RemoveDestructionMesh();

	virtual void		UpdateDestructionData();

	std::string			GetDestructionKey() const ;

	virtual r3dMesh*GetObjectMesh() { return MeshLOD[ 0 ] ; };
	
	virtual r3dMesh*GetObjectLodMesh() ;

	virtual	BOOL		Load(const char* name);
	virtual BOOL		UpdateLoading() OVERRIDE;

#ifndef FINAL_BUILD
	void				UpdateShadows( const int& Shadows );
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected ) OVERRIDE;
#endif

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendShadowRenderables( RenderArray& rarr, const r3dCamera& Cam ) OVERRIDE;

	virtual	BOOL		OnCreate();
	virtual void		OnPreRender();

	virtual GameObject *Clone ();

	virtual BOOL		GetObjStat ( char * sStr, int iLen );

	virtual void		UpdateTransform();

	virtual void		SetColor(r3dColor24 &clr);
#ifndef FINAL_BUILD
	virtual void		DrawSelected( const r3dCamera& Cam, const D3DXVECTOR4& color );
#endif

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	/**	From physics callback object. */
	virtual r3dMaterial * GetMaterial(uint32_t faceID);

protected:
	r3dMesh* ( & GetPreferredMeshLODSet() ) [ NUM_LODS ] ;
	BOOL DoLoad(	r3dMesh* (TargetLODs)[ NUM_LODS ],
					const char* szFileName, bool LoadCollision );

};

R3D_FORCEINLINE r3dMesh* ( & MeshGameObject::GetPreferredMeshLODSet() ) [ NUM_LODS ]
{
	if( m_HitPoints <= 0 && DestructionMeshLOD[ 0 ] )
		return DestructionMeshLOD ;
	else
		return MeshLOD ;
}

extern r3dMesh * gob_MeshFactoryCache[2048];

typedef r3dTL::TArray< r3dPoint3D > Positions ;

extern Positions gDEBUG_DrawPositions ;

r3dMesh *	r3dGOBAddMesh(const char* fname, bool addToLibrary=true, bool use_default_material=false, bool allow_async = false, bool player_mesh = false );
void		r3dGOBReleaseMesh( r3dMesh* mesh ) ;

void		r3dProcessReleasedMeshes() ;
void		r3dFlushReleasedMeshes() ;

void	r3dUpdateMeshMaterials();

#endif	// __PWAR_OBJ_MESH_H
