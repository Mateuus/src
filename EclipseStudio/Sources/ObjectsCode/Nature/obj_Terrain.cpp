#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "GameLevel.h"
#include "obj_Terrain.h"

//CQuadTerrain	*Terrain;


IMPLEMENT_CLASS(obj_Terrain, "obj_Terrain", "Object");
AUTOREGISTER_CLASS(obj_Terrain);

obj_Terrain		*objTerrain = NULL;

//
//
//
//
//-----------------------------------------------------------------------
BOOL obj_Terrain::Load(const char* fname)
//-----------------------------------------------------------------------
{
	DrawOrder	= OBJ_DRAWORDER_FIRST;

	ObjFlags	|= OBJFLAG_SkipCastRay;
	ObjFlags	|= OBJFLAG_SkipOcclusionCheck;
	ObjFlags	|= OBJFLAG_DisableShadows;

	ObjTypeFlags |= OBJTYPE_Terrain;

	char Str[256];

	sprintf(Str, "%s\\TERRAIN", r3dGameLevel::GetHomeDir());

	Name     = "r3dTerrain"; //Str;
	FileName = "terra1";

	r3dOutToLog ("TERRAIN:  LOADING %s\n", Str);

	char filePath[ 512 ] ;
	sprintf( filePath, "%s\\terrain.heightmap", r3dGameLevel::GetHomeDir() ) ;

	bool have_terrain1 = r3dFileExists( filePath ) ;

	bool loaded_terrain2 = false ;

	sprintf( filePath, "%s\\TERRAIN2\\terrain2.bin", r3dGameLevel::GetHomeDir() ) ;

	if( r3dFileExists( filePath ) ) // if terrain2 exists, always load terrain2
	{
		Terrain2 = new r3dTerrain2 ;
		if( Terrain2->Load() )
		{
			Terrain2->SetPhysUserData( this ) ;

			Terrain = Terrain2 ;

			r_terrain2->SetInt( 1 ) ;

			loaded_terrain2 = true ;
		}
		else
		{
			delete Terrain2 ;
		}
	}

	if( !loaded_terrain2 )
	{
		Terrain1 = new CQuadTerrain;
		Terrain1->Load(r3dGameLevel::GetHomeDir());
		Terrain1->physicsTerrain->userData = this;

		Terrain = Terrain1 ;
	}

	r3dOutToLog ("TERRAIN:  LOADED\n");

	objTerrain = this;
	return TRUE;
}


BOOL obj_Terrain::OnCreate()
{
	parent::OnCreate();

	if(m_SceneBox)
		m_SceneBox->Remove(this);
	m_SceneBox = 0;

	return TRUE;
}



BOOL obj_Terrain::OnDestroy()
{
  SAFE_DELETE(Terrain);

  objTerrain = NULL;

  return parent::OnDestroy();
}


BOOL obj_Terrain::Update()
{
  return TRUE;
}


void obj_Terrain::ReadSerializedData(pugi::xml_node& node)
{
}

void obj_Terrain::WriteSerializedData(pugi::xml_node& node)
{
}


void DrawTerrain()
{
#ifndef WO_SERVER
	extern float __WorldRenderBias;

	if ( Terrain1 && r_terrain->GetBool() 
		&& !( Terrain2 && r_terrain2->GetInt() )
		) 
	{
		R3DPROFILE_FUNCTION("Draw Terrain");
		r3dRenderer->SetMipMapBias(__WorldRenderBias);

		Terrain1->DrawDeferredMultipassInitial();
		Terrain1->DrawDeferredMultipass();
	}

	if( Terrain2 && r_terrain2->GetInt() )
	{
		R3DPROFILE_FUNCTION("Draw Terrain2");
		Terrain2->Render( gCam ) ;
	}
#endif
}

void UpdateTerrain2Atlas()
{
#ifndef WO_SERVER
	if( Terrain2 )
	{
		Terrain2->UpdateAtlas( gCam ) ;
	}
#endif
}
