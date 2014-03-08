#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "GameLevel.h"
#include "EnvmapProbes.h"
#include "EnvmapGen.h"
#include "obj_EnvmapProbe.h"
#include "Editors/ObjectManipulator3d.h"

IMPLEMENT_CLASS(obj_EnvmapProbe, "obj_EnvmapProbe", "Object");
AUTOREGISTER_CLASS(obj_EnvmapProbe);

static r3dTexture *EnvProbeIcon = NULL;

namespace
{
	const int DEFAULT_ENVMAP_EDGE = 128;
}

obj_EnvmapProbe::obj_EnvmapProbe ()
: mRadius ( 150 )
, mTexture( NULL )
, mSelected ( false )
, mInfiniteRadius( 0 )
, mID( mIDGen++ )
, mEdgeSize( DEFAULT_ENVMAP_EDGE )
{
	g_EnvmapProbes.Register( this );

	ObjFlags	|=	OBJFLAG_SkipOcclusionCheck
						|
					OBJFLAG_DisableShadows
						| 
					OBJFLAG_ForceSleep;

}

//------------------------------------------------------------------------

obj_EnvmapProbe::~obj_EnvmapProbe ()
{
	g_EnvmapProbes.Unregister( this );
	if( mTexture )
	{
		r3dRenderer->DeleteTexture( mTexture, 1 );
	}
}

//------------------------------------------------------------------------
/*virtual*/
BOOL
obj_EnvmapProbe::OnCreate()
{
	if (!EnvProbeIcon) EnvProbeIcon = r3dRenderer->LoadTexture("Data\\Images\\EnvProbe.dds");

	DrawOrder	= OBJ_DRAWORDER_LAST;
	ObjFlags	|=	OBJFLAG_SkipOcclusionCheck
						|
					OBJFLAG_DisableShadows
						|	
					OBJFLAG_ForceSleep;

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();

	return GameObject::OnCreate();
}

//------------------------------------------------------------------------
/*virtual*/
BOOL
obj_EnvmapProbe::OnDestroy()
{
	return GameObject::OnDestroy();
}

//------------------------------------------------------------------------

struct EnvmapProbeRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		R3DPROFILE_FUNCTION("EnvmapProbeRenderable");
		EnvmapProbeRenderable *This = static_cast<EnvmapProbeRenderable*>( RThis );

		This->Parent->DoDraw();
	}

	obj_EnvmapProbe*	Parent;	
};


/*virtual*/
void
obj_EnvmapProbe::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) /*OVERRIDE*/
{
#ifdef FINAL_BUILD
	return;
#else
	// don't draw debug info if we're not in editor mode
	extern bool g_bEditMode;
	if ( !g_bEditMode )
		return;

	if( r_hide_icons->GetInt() )
		return ;

	EnvmapProbeRenderable rend;

	rend.Init();
	rend.Parent = this;
	rend.SortValue = 14 * RENDERABLE_USER_SORT_VALUE;

	render_arrays[ rsDrawDebugData ].PushBack( rend );
#endif
}

//------------------------------------------------------------------------
void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
void obj_EnvmapProbe::DoDraw()
{
#ifndef FINAL_BUILD
	r3dRenderer->SetTex(NULL);
	r3dRenderer->SetMaterial(NULL);

	r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );

	if((GetPosition()-gCam).Length() < 100)
	{
		r3dDrawIcon3D(GetPosition(), EnvProbeIcon, r3dColor(255,255,255), 24);
	}

	r3dRenderer->SetTex(NULL);
	r3dRenderer->SetMaterial(NULL);

	if ( g_Manipulator3d.IsSelected( this ) )
	{
		float radius = mInfiniteRadius ? 150.f : mRadius;

		float helperSphereRadius = R3D_MIN( 22.f, radius * 0.22f );

		r3dColor greyedColr = r3dColor( 0x7f, 0x7f, 0x7f, 0x99 );

		r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_NZ );

		r3dDrawUniformSphere ( GetPosition(), radius, gCam, greyedColr );
		r3dDrawSphereSolid ( GetPosition(), helperSphereRadius, gCam, greyedColr );

		r3dRenderer->Flush();
		r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );

		r3dColor useColr;

		// draw sphere
		useColr = mInfiniteRadius ? r3dColor( 0xd0, 0xd0, 0x70, 0xff ) : r3dColor( 0xa0, 0xa0, 0xd0, 0xff );
		r3dDrawUniformSphere ( GetPosition(), radius, gCam, useColr );

		r3dDrawSphereSolid ( GetPosition(), helperSphereRadius, gCam, useColr );

		r3dRenderer->Flush();
	}
#endif
}

//------------------------------------------------------------------------

BOOL obj_EnvmapProbe::Update()
{
	return TRUE;
}

//------------------------------------------------------------------------
void obj_EnvmapProbe::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node envmapNode = node.child("envmapProbe");
	float fMaxRadius;
	int id ( WRONG_ID );
	int isInfinite;
	fMaxRadius = envmapNode.attribute("radius").as_float();
	isInfinite = envmapNode.attribute("infRadius").as_int();
	id = envmapNode.attribute("id").as_int();

	r3d_assert( id != WRONG_ID );

	pugi::xml_attribute edge_att = envmapNode.attribute("edge") ;

	if( !edge_att.empty() )
	{
		mEdgeSize = edge_att.as_int() ;
	}
	else
	{
		mEdgeSize = DEFAULT_ENVMAP_EDGE ;
	}

	mEdgeSize = R3D_MAX( R3D_MIN( mEdgeSize, 2048 ), 4 ) ;

	SetRadius( fMaxRadius );
	SetID( id );

	mInfiniteRadius = isInfinite;

	mTexture = r3dRenderer->LoadTexture( GetTextureName( false ).c_str() );

	if( !mTexture->IsLoaded() )
	{
		r3dRenderer->DeleteTexture( mTexture, true );
		mTexture = NULL;
	}

}
//------------------------------------------------------------------------
void obj_EnvmapProbe::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node envmapNode = node.append_child();
	envmapNode.set_name("envmapProbe");
	envmapNode.append_attribute("radius") = mRadius;
	envmapNode.append_attribute("infRadius") = mInfiniteRadius;
	envmapNode.append_attribute("id") = mID;
	envmapNode.append_attribute("edge") = mEdgeSize;
}


//------------------------------------------------------------------------
/*virtual*/

#ifndef FINAL_BUILD

static float listOffset = 0.f ;

float
obj_EnvmapProbe::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float ystart = scry;

	ystart += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		ystart += 11;
		ystart += imgui_Static ( scrx, ystart, "Envmap Probe Properties" );

		int check = mInfiniteRadius ;
		ystart += imgui_Checkbox( scrx, ystart, "Infinite radius", &check, 1 );
		PropagateChange( check, &obj_EnvmapProbe::mInfiniteRadius, this, selected ) ;

		if( !mInfiniteRadius )
		{
			static float radiusVal = 0.f ;
			radiusVal = mRadius ;
			ystart += imgui_Value_Slider ( scrx, ystart, "Radius", &radiusVal, 15, 10000, "%-02.02f" );
			PropagateChange( radiusVal, &obj_EnvmapProbe::mRadius, this, selected ) ;
		}

		ystart += 11;
		ystart += imgui_Static( scrx, ystart, "Cubemap Edge Size" ) ;

		stringlist_t edgeResolutions ;

		edgeResolutions.push_back( "16" ) ;
		edgeResolutions.push_back( "32" ) ;
		edgeResolutions.push_back( "64" ) ;
		edgeResolutions.push_back( "128" ) ;
		edgeResolutions.push_back( "256" ) ;
		edgeResolutions.push_back( "512" ) ;
		edgeResolutions.push_back( "1024" ) ;

		const float LIST_HEIGHT = 160 ;

		int selection = 0 ;
		int edge = mEdgeSize ;
		for( ; edge ; edge /= 2, selection ++ ) ;

		selection -= 5 ;

		if( selection < 0 ) selection = 0 ;

		imgui_DrawList( scrx, ystart, 360, LIST_HEIGHT, edgeResolutions, &listOffset, &selection ) ;

		selection =	R3D_MIN( R3D_MAX( selection, 0 ), (int)edgeResolutions.size() - 1 ) ;

		edge = 16 << selection ;

		if( edge != mEdgeSize )
		{
			mEdgeSize = edge ;
			r_need_gen_envmap->SetInt( mID + 1 ) ;
		}

		ystart += LIST_HEIGHT ;
	}
	return ystart - scry ;
}
#endif

//------------------------------------------------------------------------

void obj_EnvmapProbe::SetRadius ( float fRadius )
{
	r3d_assert ( fRadius >= 0.0f );
	mRadius = fRadius;
}

//------------------------------------------------------------------------

float obj_EnvmapProbe::GetRadius () const
{
	return mRadius;
}

//------------------------------------------------------------------------

float
obj_EnvmapProbe::GetRadiusSq() const
{
	return mRadius * mRadius;
}

//------------------------------------------------------------------------

r3dTexture*
obj_EnvmapProbe::GetTexture() const
{
	return mTexture;
}

//------------------------------------------------------------------------

void
obj_EnvmapProbe::SetSelected( bool selected )
{
	mSelected = selected;
}

//------------------------------------------------------------------------

void
obj_EnvmapProbe::Generate()
{
	if( mTexture )
	{
		r3dRenderer->DeleteTexture( mTexture, true );
	}

	mTexture = r3dRenderer->AllocateTexture();

	int w = mEdgeSize ;

	int mips = 0 ;

	for( ; w ; w /= 2 )
	{
		mips ++ ;	
	}

	mTexture->CreateCubemap( mEdgeSize, D3DFMT_DXT1, mips );

	GenerateEnvmap( mTexture, GetTextureName( false ), GetPosition() );
}

//------------------------------------------------------------------------

int
obj_EnvmapProbe::IsInfinite() const
{
	return mInfiniteRadius;
}

//------------------------------------------------------------------------

namespace
{
	const char* ENVMAP_FOLDER = "\\Envmaps";
}

r3dString
obj_EnvmapProbe::GetTextureName( bool forSaving ) const
{
	r3d_assert( mID != WRONG_ID );

	char buffer[64];

	sprintf( buffer, "envmap%d.dds", mID );

	return r3dString( forSaving ? r3dGameLevel::GetSaveDir() : r3dGameLevel::GetHomeDir() ) + ENVMAP_FOLDER + "\\" + buffer;
}

//------------------------------------------------------------------------

int
obj_EnvmapProbe::GetID() const
{
	return mID;
}

//------------------------------------------------------------------------

/*static*/
void	
obj_EnvmapProbe::CreateFolder( bool forSaving )
{
	struct _stat st;
	if( _stat( ENVMAP_FOLDER, &st ) )
	{
		mkdir( (r3dString( forSaving ? r3dGameLevel::GetSaveDir() : r3dGameLevel::GetHomeDir() ) + ENVMAP_FOLDER).c_str() );
	}
	else
	{
		r3d_assert( st.st_mode & _S_IFDIR );
	}
}

//------------------------------------------------------------------------

void
obj_EnvmapProbe::SetID( int id )
{
	mID = id;
	mIDGen = R3D_MAX( mIDGen, id + 1 );
}

//------------------------------------------------------------------------
/*static*/ int obj_EnvmapProbe::mIDGen = 0;
