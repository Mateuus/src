#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "ObjectsCode\Effects/obj_ParticleSystem.H"

#include "Editors/LevelEditor.h"

#include "building.h"

#include "DamageLib.h"

IMPLEMENT_CLASS(obj_Building, "obj_Building", "Object");
AUTOREGISTER_CLASS(obj_Building);
obj_Building::obj_Building() 
: m_DestructionParticles( NULL )
, m_DestructionSoundID( -1 )
, m_IsSkeletonShared( 0 )
{
	ObjTypeFlags |= OBJTYPE_Building;

	m_sAnimName[0] = 0;
	m_bAnimated = 0;
	m_bGlobalAnimFolder = 0;
	m_BindSkeleton = NULL;
}

obj_Building::~obj_Building()
{
	if( !m_IsSkeletonShared )
		SAFE_DELETE(m_BindSkeleton);
}


int __RepositionObjectsOnTerrain = 0;
extern float GetTerrainFollowPosition( const r3dBoundBox& BBox);


void obj_Building::SetPosition(const r3dPoint3D& pos)
{
	parent::SetPosition( pos );
	r3dPoint3D v = pos;

	if (!MeshLOD[0]) 
		return;

#ifndef FINAL_BUILD
	if (__RepositionObjectsOnTerrain)
	{
		float h = GetTerrainFollowPosition(GetBBoxWorld());
		float fRes = h;
		parent::SetPosition( r3dPoint3D( GetPosition().X, fRes,GetPosition().Z) );	
	}
#endif
}

#ifndef FINAL_BUILD
float obj_Building::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )	
	{
		starty += imgui_Static(scrx, starty, "Building properties");

		std::string sDir = FileName.c_str();
		int iPos1 = sDir.find_last_of('\\');
		int iPos2 = sDir.find_last_of('/');

		if ( ( iPos1 < iPos2 && iPos2 != std::string::npos ) || ( iPos1 == std::string::npos ) )
			iPos1 = iPos2;

		if ( iPos1 != std::string::npos )
			sDir.erase(iPos1 + 1,sDir.length() - iPos1 - 1 );

		std::string sDirFind = sDir + "*.sco";

		if(m_Animation.pSkeleton && MeshLOD[0]->IsSkeletal() )
		{
			int useAnim = m_bAnimated;
			starty += imgui_Checkbox(scrx, starty, "Animated", &m_bAnimated, 1);
			if(useAnim != m_bAnimated)
			{
				PropagateChange( &obj_Building::ChangeAnim, selected ) ;
			}

			if(m_bAnimated)
			{
				int check = m_bGlobalAnimFolder ;
				starty += imgui_Checkbox(scrx, starty, "Global Anim Folder", &check, 1);

				PropagateChange( 1, &obj_Building::m_bAnimated, selected ) ;
				PropagateChange( check, &obj_Building::m_bGlobalAnimFolder, this, selected ) ;
				
				static char sAnimSelected[256] = {0};
				static float fAnimListOffset = 0;

				std::string sDirFind ;

				if( m_bGlobalAnimFolder )
				{
					// try global animation folder
					sDirFind = GLOBAL_ANIM_FOLDER "\\*.anm" ;				
				}
				else
				{
					sDirFind = sDir + "Animations\\*.anm";
				}

				r3dscpy(sAnimSelected, m_sAnimName);
				if ( imgui_FileList (scrx, starty, 360, 200, sDirFind.c_str (), sAnimSelected, &fAnimListOffset, true ) )
				{
					r3dscpy(m_sAnimName, sAnimSelected);
					PropagateChange( &obj_Building::ChangeAnim, selected ) ;
				}
				starty += 200;
			}
		}

		if( m_HitPoints > 0 )
		{
			if ( imgui_Button ( scrx, starty, 360, 20, "Destroy" ) )
			{
				PropagateChange( &obj_Building::DestroyBuilding, selected ) ;
			}
		}
		else
		{
			if ( imgui_Button ( scrx, starty, 360, 20, "Ressurect" ) )
			{
				PropagateChange( &obj_Building::FixBuilding, selected ) ;
			}
		}

		starty += 22.f ;

		if( selected.Count() <= 1 )
		{
			if ( imgui_Button ( scrx, starty, 360, 20, m_pDamageLibEntry ? "To Destruction Params" : "Create Destruction Params" ) )
			{
				LevelEditor.ToDamageLib( GetDestructionKey().c_str() );
			}
		}

		starty += 22.0f;
	}
	return starty - scry ;
}
#endif


#ifndef FINAL_BUILD
void
obj_Building::DrawSelected( const r3dCamera& Cam, eRenderStageID DrawState )
{
	if( m_bAnimated )
	{
		// do nothing for now..
	}
	else
	{
		MeshGameObject::DrawSelected( Cam, D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.223f) ) ;
	}
}
#endif

BOOL obj_Building::OnCreate()
{
	parent::OnCreate();
	
	ChangeAnim();

	return 1;
}

/*static*/
void
obj_Building::LoadSkeleton( const char* baseMeshFName, r3dSkeleton** oSkeleton, int * oIsSkeletonShared )
{
	const char* fname = baseMeshFName ;

	char skelname[MAX_PATH];
	r3dscpy(skelname, fname);
	r3dscpy(&skelname[strlen(skelname)-3], "skl");

	bool loadSkel = false ;

	r3dSkeleton* bindSkeleton = 0 ;

	if( oIsSkeletonShared )
		*oIsSkeletonShared = 0 ;

	if(r3d_access(skelname, 0) == 0)
	{
		bindSkeleton = new r3dSkeleton();
		bindSkeleton->LoadBinary(skelname);
	}
#ifndef FINAL_BUILD // required only in editor, to place armor in editor and for armor to automatically load default skeleton. not needed in the game
	else
	{
		bindSkeleton		= GetDefaultSkeleton( fname );

		if( oIsSkeletonShared )
			*oIsSkeletonShared	= 1 ;
	}
#endif

	*oSkeleton = bindSkeleton ;
}

BOOL obj_Building::Load(const char* fname)
{
	if(!parent::Load(fname))
		return FALSE;
			
	// try to load default skeleton
	if( MeshLOD[0]->IsSkeletal() )
	{
		LoadSkeleton( fname, &m_BindSkeleton, &m_IsSkeletonShared ) ;

		if( m_BindSkeleton )
		{
			m_Animation.Init(m_BindSkeleton, &m_AnimPool);
			m_Animation.Update(0.0f, r3dPoint3D(0, 0, 0), mTransform);
		}
	}


	if( m_pDamageLibEntry )
	{
		if( m_pDamageLibEntry->HasSound )
		{
			m_DestructionSoundID = SoundSys.GetEventIDByPath( m_pDamageLibEntry->SoundName.c_str() );
		}
	}
	
	return TRUE;
}

BOOL obj_Building::Update()
{
	if( m_bAnimated || MeshLOD[ 0 ]->IsSkeletal() && m_BindSkeleton )
		m_Animation.Update(r3dGetAveragedFrameTime(), r3dPoint3D(0, 0, 0), mTransform);

	if( m_DestructionParticles )
	{
		if( !m_DestructionParticles->isActive() )
		{
			m_DestructionParticles = 0 ;
		}
	}

	return parent::Update();
}

void obj_Building::ChangeAnim()
{
	if(!m_Animation.pSkeleton)
		return;
		
	m_Animation.StopAll();
	if(!m_bAnimated)
		return;
		
	if(m_sAnimName[0] == 0)
		return;

	char animname[MAX_PATH];	


	if( m_bGlobalAnimFolder )
	{
		// try global folder
		sprintf( animname, GLOBAL_ANIM_FOLDER "\\%s", m_sAnimName ) ;
	}
	else
	{
		FixFileName(FileName.c_str(), animname);
		char* p = strrchr(animname, '/');
		r3d_assert(p);
		sprintf(p, "\\Animations\\%s", m_sAnimName);
	}

	if(r3d_access(animname, 0) != 0)
	{
		m_sAnimName[0] = 0;
		m_bAnimated = 0;
		return;
	}
		
	int aid = m_AnimPool.Add(m_sAnimName, animname);
	if(aid == -1)
	{
		m_sAnimName[0] = 0;
		m_bAnimated = 0;
		return;
	}
	
	m_Animation.StartAnimation(aid, ANIMFLAG_Looped, 0.0f, 0.0f, 0.0f);
}

void
obj_Building::DestroyBuilding()
{
	this->m_HitPoints = 0 ;

	if( m_pDamageLibEntry )
	{
		if( m_pDamageLibEntry->HasParicles && !m_DestructionParticles )
		{
			m_DestructionParticles = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", m_pDamageLibEntry->ParticleName.c_str(), GetPosition() );
		}
		else
		{
			if( m_DestructionParticles && m_DestructionParticles->isActive() )
				m_DestructionParticles->Restart();
		}

		if( m_pDamageLibEntry->HasSound && m_DestructionSoundID != -1 )
		{
			SoundSys.Play( m_DestructionSoundID, GetPosition() );
		}
	}
}

void
obj_Building::FixBuilding()
{
	m_HitPoints = 0XA107 ;

	if( m_pDamageLibEntry )
	{
		m_HitPoints = m_pDamageLibEntry->HitPoints ;
	}

	if( m_DestructionParticles )
	{
		GameWorld().DeleteObject( m_DestructionParticles );
		m_DestructionParticles = 0 ;
	}
}

GameObject *obj_Building::Clone ()
{
	obj_Building * pNew = (obj_Building*)srv_CreateGameObject("obj_Building", FileName.c_str(), GetPosition () );
	
	PostCloneParamsCopy(pNew);

	return pNew;
}

//////////////////////////////////////////////////////////////////////////

void obj_Building::PostCloneParamsCopy(obj_Building *pNew)
{
	if (!pNew) return;
	pNew->SetRotationVector(GetRotationVector());

	r3dscpy(pNew->m_sAnimName, m_sAnimName);
	pNew->m_bAnimated = m_bAnimated;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Building::GetObjStat ( char * sStr, int iLen)
{
	char sAddStr [MAX_PATH];
	sAddStr[0] = 0;
	if (MeshGameObject::GetObjStat (sStr, iLen))
		r3dscpy(sAddStr, sStr);

	return TRUE;
}

void obj_Building::ReadSerializedData(pugi::xml_node& node)
{
	parent::ReadSerializedData(node);
	pugi::xml_node buildingNode = node.child("building");
	
	r3dscpy(m_sAnimName, buildingNode.attribute("AnimName").value());
	m_bAnimated = buildingNode.attribute("Animated").as_int();

	pugi::xml_attribute gloanfol = buildingNode.attribute("GlobalAnimFolder") ;

	if( !gloanfol.empty() )
	{
		m_bGlobalAnimFolder = !!gloanfol.as_int();
	}
}

void obj_Building::WriteSerializedData(pugi::xml_node& node)
{
	parent::WriteSerializedData(node);
	pugi::xml_node buildingNode = node.append_child();
	buildingNode.set_name("building");
	
	if(m_sAnimName[0])
	{
		buildingNode.append_attribute("AnimName") = m_sAnimName;
		buildingNode.append_attribute("Animated") = m_bAnimated;

		// don't spam it..
		if( m_bGlobalAnimFolder )
		{
			buildingNode.append_attribute("GlobalAnimFolder") = m_bGlobalAnimFolder;
		}
	}
}

/*virtual*/
void
obj_Building::UpdateDestructionData()
{
	MeshGameObject::UpdateDestructionData();

	if( !m_pDamageLibEntry )
	{
		if( m_DestructionParticles )
		{
			GameWorld().DeleteObject( m_DestructionParticles );
			m_DestructionParticles = NULL ;
		}

		m_DestructionSoundID = -1 ;
	}
	else
	{
		if( !m_pDamageLibEntry->HasParicles  )
		{
			GameWorld().DeleteObject( m_DestructionParticles );
			m_DestructionParticles = NULL ;
		}

		if( !m_pDamageLibEntry->HasSound )
		{
			m_DestructionSoundID = -1 ;
		}
		else
		{
			m_DestructionSoundID = SoundSys.GetEventIDByPath( m_pDamageLibEntry->SoundName.c_str() );
		}
	}
}

bool obj_Building::NeedDrawAnimated(const r3dCamera& Cam)
{
	float distSq = (Cam - GetPosition()).LengthSq();
	int meshLodIndex = ChoseMeshLOD( distSq, MeshLOD );

	if( MeshLOD[ meshLodIndex ]->IsSkeletal() && m_BindSkeleton )
		return true ;
	
	if(!m_bAnimated)
		return false;
	
	return true;
}

void obj_Building::DrawAnimated(const r3dCamera& Cam, bool shadow_pass)
{
	r3d_assert(m_Animation.pSkeleton);

	// recalc animation if it dirty
	m_Animation.Recalc();
	
	int oldVsId = r3dRenderer->GetCurrentVertexShaderIdx();

	m_Animation.pSkeleton->SetShaderConstants();

	r3dMesh* mesh = MeshLOD[0];
	r3d_assert( mesh->IsSkeletal() );

	D3DXMATRIX world;
	r3dPoint3D pos = GetPosition();
	D3DXMatrixTranslation(&world, pos.x, pos.y, pos.z );
	r3dMeshSetVSConsts( world, NULL, NULL );
	
	if(!shadow_pass)
	{
		mesh->DrawMeshDeferred( m_ObjectColor, 0 );
	}
	else
	{
		mesh->DrawMeshShadows();
	}
	
	r3dRenderer->SetVertexShader(oldVsId);
}

struct BuildingAniDeferredRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BuildingAniDeferredRenderable* This = static_cast<BuildingAniDeferredRenderable*>( RThis );
		This->Parent->DrawAnimated(Cam, false);
	}

	obj_Building* Parent;
};

struct BuildingAniShadowRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BuildingAniShadowRenderable* This = static_cast<BuildingAniShadowRenderable*>( RThis );
		This->Parent->DrawAnimated(Cam, true);
	}

	obj_Building* Parent;
};

struct BuildingAniDebugRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BuildingAniDebugRenderable* This = static_cast<BuildingAniDebugRenderable*>( RThis );
		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		This->Parent->m_Animation.pSkeleton->DrawSkeleton(Cam, This->Parent->GetPosition());
	}

	obj_Building* Parent;
};

#define	RENDERABLE_BUILDING_SORT_VALUE (31*RENDERABLE_USER_SORT_VALUE)

void obj_Building::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam )
{
	// [from MeshGameObject code] always use main camera to determine shadow LOD
	if(!NeedDrawAnimated(gCam))
	{
		parent::AppendShadowRenderables(rarr, Cam);
		return;
	}

	if( !gDisableDynamicObjectShadows )
	{
		BuildingAniShadowRenderable rend;
		rend.Init();
		rend.Parent	= this;
		rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

		rarr.PushBack( rend );
	}
}

void obj_Building::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if(!NeedDrawAnimated(Cam))
	{
		parent::AppendRenderables(render_arrays, Cam);
		return;
	}

	// deferred
	{
		BuildingAniDeferredRenderable rend;
		rend.Init();
		rend.Parent	= this;
		rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

		render_arrays[ rsFillGBuffer ].PushBack( rend );
	}

	/*// skeleton draw
	{
		BuildingAniDebugRenderable rend;
		rend.Init();
		rend.Parent	= this;
		rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}*/
}


