#include "r3dpch.h"
#include "r3d.h"

#include "UI/UIimEdit.h"

#include "GameCommon.h"
#include "GameObjects\gameobj.h"
#include "GameObjects\objmanag.h"

#include "EnvmapProbes.h"
#include "GameLevel.h"
#include "WaterPlane.h"
#include "../GameEngine/TrueNature/Terrain.h"

extern r3dITerrain* Terrain;

//////////////////////////////////////////////////////////////////////////

int GetWaterDownScale()
{
	return r_texture_quality->GetInt() == 1 ? 2 : 1 ;
}

void waterReloadTexture( r3dTexture*& tex )
{
	if( !tex )
		return;

	char FileName[ 1024 ];
	r3dscpy( FileName, tex->getFileLoc().FileName );
	FileName[ 1023 ] = 0;

	tex->Unload();
	tex->Load( FileName, D3DFMT_FROM_FILE, GetWaterDownScale() );
}

LPDIRECT3DVERTEXDECLARATION9 R3D_WATER_PLANE_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_WATER_PLANE_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_WATER_PLANE_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		( r3dDeviceTunnel::CreateVertexDeclaration(VBDecl, &pDecl) );
	}
	return pDecl;
}

//////////////////////////////////////////////////////////////////////////

static r3dVector WaterPlaneComputeNormal ( r3dTL::T2DArray<r3dVector> const &dPoints, r3dTL::T2DArray<uint8_t> const &dGrid, uint32_t iI, uint32_t iJ )
{
	uint32_t iW = dGrid.Width ();
	uint32_t iH = dGrid.Height ();

	r3dVector vNormal(0,0,0);
	int iCnt = 0;

	if(iI<iW&&iJ<iH&&dGrid[iI][iJ])
	{
		r3dVector const & v0 = dPoints[iI][iJ];
		r3dVector const & v1 = dPoints[iI][iJ+1];
		r3dVector const & v2 = dPoints[iI+1][iJ+1];
		r3dVector const & v3 = dPoints[iI+1][iJ];

		r3dVector vNormal0 = -(v0-v1).Cross(v2-v1).NormalizeTo();
		r3dVector vNormal1 = -(v2-v3).Cross(v0-v3).NormalizeTo();
		vNormal+=vNormal0;
		vNormal+=vNormal1;
		iCnt+=2;		
	}

	if(iI>0&&iJ>0&&iI-1<iW&&iJ-1<iH&&dGrid[iI-1][iJ-1])
	{
		r3dVector const & v0 = dPoints[iI-1][iJ-1];
		r3dVector const & v1 = dPoints[iI-1][iJ];
		r3dVector const & v2 = dPoints[iI][iJ];
		r3dVector const & v3 = dPoints[iI][iJ-1];

		r3dVector vNormal0 = -(v0-v1).Cross(v2-v1).NormalizeTo();
		r3dVector vNormal1 = -(v2-v3).Cross(v0-v3).NormalizeTo();
		vNormal+=vNormal0;
		vNormal+=vNormal1;
		iCnt+=2;		
	}

	if(iI>0&&iI-1<iW&&iJ<iH&&dGrid[iI-1][iJ])
	{
		r3dVector const & v0 = dPoints[iI-1][iJ];
		r3dVector const & v2 = dPoints[iI][iJ+1];
		r3dVector const & v3 = dPoints[iI][iJ];

		r3dVector vNormal1 = -(v2-v3).Cross(v0-v3).NormalizeTo();
		vNormal+=vNormal1;
		iCnt+=1;		
	}

	if(iJ>0&&iI<iW&&iJ-1<iH&&dGrid[iI][iJ-1])
	{
		r3dVector const & v0 = dPoints[iI][iJ-1];
		r3dVector const & v1 = dPoints[iI][iJ];
		r3dVector const & v2 = dPoints[iI+1][iJ];
		
		r3dVector vNormal0 = -(v0-v1).Cross(v2-v1).NormalizeTo();
		vNormal+=vNormal0;
		iCnt+=1;		
	}

	r3d_assert(iCnt>0);

	vNormal.Normalize ();

	return vNormal;
}

//////////////////////////////////////////////////////////////////////////

const char WATER_PLANES_DIR [] = "water_planes";
const WORD WATER_VERSION_ID = 3;
const WORD WATER_VERSION_BIN_ID = 5;

extern r3dScreenBuffer*	DepthBuffer;
extern float terra_GetH(const r3dPoint3D &vPos);

//////////////////////////////////////////////////////////////////////////

//static 
r3dTL::TArray<obj_WaterPlane*> obj_WaterPlane::m_dWaterPlanes;

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(obj_WaterPlane, "obj_WaterPlane", "Object");
AUTOREGISTER_CLASS(obj_WaterPlane);

obj_WaterPlane::obj_WaterPlane()
	: m_fCellGridSize ( 50.0f )
	, m_fWaterPlaneHeight ( 10.0f )
	, m_fWaterPlaneHeightOnTerrain ( 1.0f )
	, m_bWaterPlaneFollowTerrain ( false )
	, m_iCoastSmoothLevels ( 2 )
	, m_fOldCellGridSize ( 0.0f )
	, m_bResizeInfoExists ( false )
	, m_Plane ( NULL )
{
	m_dOldGrid.Clear ();
	m_dWaterPlanes.PushBack ( this );
	memset(WaterColor, 0, sizeof(WaterColor));
	FoamTexture = 0;
	SpecularTexture = 0;
	ColorTexture = 0;
	MaskTexture = 0;
}

obj_WaterPlane::~obj_WaterPlane()
{
	for ( uint32_t i = 0; i < m_dWaterPlanes.Count(); i++ )
	{
		if ( m_dWaterPlanes[i] == this )
		{
			m_dWaterPlanes.Erase ( i );
			break;
		}
	}
}




BOOL obj_WaterPlane::OnCreate()
{
	parent::OnCreate();
#ifdef WO_SERVER
	return false;
#endif

	char Str[256];

	DrawOrder	= OBJ_DRAWORDER_LAST;

	

	ObjFlags |= OBJFLAG_SkipCastRay 
					| 
				OBJFLAG_SkipOcclusionCheck
					|
				OBJFLAG_DisableShadows
					|	
				OBJFLAG_ForceSleep; // todo: remove skipObstacle and add proper aabb!!!

	m_Plane = new r3dXPSObject<R3D_WATER_PLANE_VERTEX>;
	
#ifdef WO_SERVER
	int bRenderRefraction = 0;
#else
	extern int bRenderRefraction;
	bRenderRefraction = 1;
#endif

	WaterBase::OnCreateBase();
	CreateRefractionBuffer();

	if ( ! WaterColor[0] )
	{
		for(int i=0; i<25; ++i)
		{
			sprintf(Str,"Data\\Water\\waves_%.2d.dds",i);
			WaterColor[i] = r3dRenderer->LoadTexture( Str, D3DFMT_UNKNOWN, false, GetWaterDownScale() );
		}
		//FoamTexture = r3dRenderer->LoadTexture("Data\\Water\\foam01.dds");
		//SpecularTexture = r3dRenderer->LoadTexture("Data\\Water\\water_gloss.dds");
		ColorTexture = r3dRenderer->LoadTexture("Data\\Water\\LakeColor.dds", D3DFMT_UNKNOWN, false, GetWaterDownScale() );
		//MaskTexture = r3dRenderer->LoadTexture("Data\\Water\\LakeMask.dds");
	}

	UpdateWaterPlane();

	return 1;
}

BOOL obj_WaterPlane::OnDestroy()
{
	WaterBase::OnDestroyBase();
	for(int i=0; i<25; ++i)
		r3dRenderer->DeleteTexture(WaterColor[i]);
	//r3dRenderer->DeleteTexture(FoamTexture);
	//r3dRenderer->DeleteTexture(SpecularTexture);
	r3dRenderer->DeleteTexture(ColorTexture);
	//r3dRenderer->DeleteTexture(MaskTexture);

	SAFE_DELETE(m_Plane);

	return parent::OnDestroy();
}

BOOL obj_WaterPlane::Update()
{
	// in case of changing follow-terrain or height

	r3dBoundBox bboxLocal = GetBBoxLocal() ;

	if(	m_bWaterPlaneFollowTerrain ) bboxLocal.Org.y = m_meshHeight;// + m_fWaterPlaneHeightOnTerrain;
	else	bboxLocal.Org.y = m_fWaterPlaneHeight;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();
	return TRUE;
}

//------------------------------------------------------------------------
/*virtual*/

struct WaterRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		R3DPROFILE_FUNCTION("WaterRenderable");
		WaterRenderable *This = static_cast<WaterRenderable*>( RThis );

		This->Parent->DoDraw( Cam );
	}

	obj_WaterPlane* Parent;	
};

struct WaterDepthRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		R3DPROFILE_FUNCTION("WaterDepthRenderable");
		WaterDepthRenderable *This = static_cast<WaterDepthRenderable*>( RThis );

		This->Parent->DoDrawDepth( Cam );
	}

	obj_WaterPlane* Parent;	
};

void
obj_WaterPlane::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) /*OVERRIDE*/
{
	if(!m_Plane||!m_Plane->IsValid())
		return;

#define RENDERABLE_WATER_SORT_VALUE (19*RENDERABLE_USER_SORT_VALUE)

	// composite
	{
		WaterRenderable rend;

		rend.Init();

		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_WATER_SORT_VALUE;

		render_arrays[ rsDrawComposite2 ].PushBack( rend );
	}

	// depth
	{
		WaterDepthRenderable rend;

		rend.Init();

		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_WATER_SORT_VALUE;

		render_arrays[ rsDrawDepthEffect ].PushBack( rend );
	}


}

extern float RIPPLES_AREA_SIZE;

bool obj_WaterPlane::WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx)
{
	if(nSplashes==MaxSplashes)	return false;
	if(GetWaterPlaneFollowTerrainFlag())	return false;	//TODO
	const float heightBlend = 0.5f;

	const r3dBoundBox& bboxLocal = GetBBoxLocal() ;

	if(waterSplashPos.X >= bboxLocal.Org.X && waterSplashPos.X <= bboxLocal.Org.X + bboxLocal.Size.X &&
		 waterSplashPos.Z >= bboxLocal.Org.Z && waterSplashPos.Z <= bboxLocal.Org.Z + bboxLocal.Size.Z
		 && m_fWaterPlaneHeight > waterSplashPos.y && m_fWaterPlaneHeight < waterSplashPos.y+height+heightBlend
		 )
	{
		splashes[nSplashes].params.x = (waterSplashPos.x - gCam.x) / RIPPLES_AREA_SIZE + 0.5f;
		splashes[nSplashes].params.y = (waterSplashPos.z - gCam.z) / RIPPLES_AREA_SIZE + 0.5f;
		splashes[nSplashes].params.z = size;
		splashes[nSplashes].texIdx = texIdx;

		if(m_fWaterPlaneHeight > waterSplashPos.y+height)
			amount *= 1 - (m_fWaterPlaneHeight - waterSplashPos.y-height) / heightBlend;

		splashes[nSplashes].params.w = amount;
		nSplashes++;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------

void
obj_WaterPlane::DoDraw( const r3dCamera& Cam )
{
#ifndef WO_SERVER
	R3DPROFILE_FUNCTION("WaterPlane::DoDraw");

	RenderBegin(Cam, m_bWaterPlaneFollowTerrain ? 0.0f : m_fWaterPlaneHeight, m_bWaterPlaneFollowTerrain);

	m_Plane->Draw();

	RenderEnd();

#endif
}

//------------------------------------------------------------------------

void
obj_WaterPlane::DoDrawDepth( const r3dCamera& Cam )
{
	return;
}

void obj_WaterPlane::ReadSerializedData(pugi::xml_node& node)
{
#ifdef WO_SERVER
	return;
#endif
	GameObject::ReadSerializedData(node);
	pugi::xml_node lakeNode = node.child("lake");
	
	pugi::xml_node ch_node = node.child("new_lake");
	if ( ! ch_node.empty() )
	{	
		const char * szName = ch_node.attribute("wave_tex").value();
		if ( * szName )
		{
			char Str[256];
			for(int i=0; i<25; ++i)
			{
				sprintf(Str,"%s%.2d.dds", szName, i);
				WaterColor[i] = r3dRenderer->LoadTexture(Str, D3DFMT_UNKNOWN, false, GetWaterDownScale() );
			}
		}
		//FoamTexture = r3dRenderer->LoadTexture("Data\\Water\\foam01.dds");
		//SpecularTexture = r3dRenderer->LoadTexture("Data\\Water\\water_gloss.dds");
		ColorTexture = r3dRenderer->LoadTexture("Data\\Water\\LakeColor.dds", D3DFMT_UNKNOWN, false, GetWaterDownScale() );
		//MaskTexture = r3dRenderer->LoadTexture("Data\\Water\\LakeMask.dds");


		if(!ch_node.attribute("waterplaneheight").empty())
			m_fWaterPlaneHeight =  ch_node.attribute("waterplaneheight").as_float();
		if(!ch_node.attribute("waterplaneheightonterrain").empty())
			m_fWaterPlaneHeightOnTerrain =  ch_node.attribute("waterplaneheightonterrain").as_float();
		if(!ch_node.attribute("waterplanefollowterrain").empty())
			m_bWaterPlaneFollowTerrain =  ch_node.attribute("waterplanefollowterrain").as_bool();
		if(!ch_node.attribute("cellgridsize").empty())
			m_fCellGridSize =  ch_node.attribute("cellgridsize").as_float();
		if(!ch_node.attribute("coastsmoothlevels").empty())
			m_iCoastSmoothLevels =  ch_node.attribute("coastsmoothlevels").as_int();

		WaterBase::ReadSerializedData(ch_node);

		LoadWaterPlaneGrid (Name.c_str());

		UpdateWaterPlane();
	}
}

void obj_WaterPlane::WriteSerializedData(pugi::xml_node& node)
{
	ResetResizeInfo ();

	GameObject::WriteSerializedData(node);
	pugi::xml_node lakeNode = node.append_child();
	lakeNode.set_name("lake");

	pugi::xml_node ch_node = node.append_child();
	ch_node.set_name( "new_lake" );
	if ( WaterColor[0] )
	{
		char fn[256];
		memcpy(fn, WaterColor[0]->getFileLoc().FileName, sizeof(WaterColor[0]->getFileLoc().FileName));
		fn[ strlen(fn) - strlen("00.dds") ] = 0;
		ch_node.append_attribute("wave_tex") = fn;
	}


	ch_node.append_attribute("waterplaneheight") = m_fWaterPlaneHeight;
	ch_node.append_attribute("waterplaneheightonterrain") = m_fWaterPlaneHeightOnTerrain;
	ch_node.append_attribute("waterplanefollowterrain") = m_bWaterPlaneFollowTerrain;
	ch_node.append_attribute("cellgridsize") = m_fCellGridSize;
	ch_node.append_attribute("coastsmoothlevels") = m_iCoastSmoothLevels;

	WaterBase::WriteSerializedData(ch_node);

	SaveWaterPlaneGrid (Name.c_str());
	SaveWaterPlaneGridBin (Name.c_str());
}

float obj_WaterPlane::GetCellGridSize () const
{
	return m_fCellGridSize;
}

void obj_WaterPlane::SetCellGridSize ( float fSize )
{
	if ( fabsf(m_fCellGridSize - fSize) > FLT_EPSILON )
	{
		if(!m_bResizeInfoExists)
		{
			m_fOldCellGridSize = m_fCellGridSize;
			m_dOldGrid = m_dGrid;
			m_bResizeInfoExists = true;
		}
		m_fCellGridSize = fSize;

		UpdateGridOnChangeCellSize ();

		UpdateWaterPlane ();
	}
}

void obj_WaterPlane::SetWaterPlaneHeightOnTerrain ( float fH )
{
	if ( fabsf(m_fWaterPlaneHeightOnTerrain - fH) > FLT_EPSILON )
	{
		m_fWaterPlaneHeightOnTerrain = fH;
		UpdateWaterPlane ();
	}
}

float obj_WaterPlane::GetWaterPlaneHeightOnTerrain () const
{
	return m_fWaterPlaneHeightOnTerrain;
}


void obj_WaterPlane::SetWaterPlaneHeight ( float fH )
{
	if ( fabsf(m_fWaterPlaneHeight - fH) > FLT_EPSILON )
	{
		m_fWaterPlaneHeight = fH;
		UpdateWaterPlane ();
	}
}

float obj_WaterPlane::GetWaterPlaneHeight () const
{
	return m_fWaterPlaneHeight;
}

void obj_WaterPlane::SetWaterPlaneCoastSmoothLevels ( int iLvls )
{
	if(m_iCoastSmoothLevels!= iLvls)
	{
		m_iCoastSmoothLevels = iLvls;
		UpdateWaterPlane ();
	}
}

int obj_WaterPlane::GetWaterPlaneCoastSmoothLevels () const
{
	return m_iCoastSmoothLevels;
}

void obj_WaterPlane::SetWaterPlaneFollowTerrainFlag(bool bFollow)
{
	if ( m_bWaterPlaneFollowTerrain != bFollow )
	{
		m_bWaterPlaneFollowTerrain = bFollow;
		UpdateWaterPlane ();
	}
}

bool obj_WaterPlane::GetWaterPlaneFollowTerrainFlag() const
{
	return m_bWaterPlaneFollowTerrain;
}



void obj_WaterPlane::SetGridDimensions ( uint32_t iW, uint32_t iH )
{
	if(m_dGrid.Width() == iW && m_dGrid.Height() == iH)
		return;

	m_dGrid.Resize(iW, iH, 0);

	if(m_bResizeInfoExists)
		UpdateGridOnChangeCellSize ();
}
void obj_WaterPlane::GetGridDimensions( uint32_t& iW, uint32_t& iH )
{
	iW = m_dGrid.Width();
	iH = m_dGrid.Height();
}


bool obj_WaterPlane::FillWaterGrid ( uint32_t iW, uint32_t iH, bool bUpdateImmediately )
{
	ResetResizeInfo ();

	bool bChanged = m_dGrid[iW][iH] == 0;
	m_dGrid[iW][iH] = 1;
	if(bUpdateImmediately)
		UpdateWaterPlane ();

	return bChanged;
}

bool obj_WaterPlane::EraseWaterGrid ( uint32_t iW, uint32_t iH, bool bUpdateImmediately )
{
	ResetResizeInfo ();

	bool bChanged = m_dGrid[iW][iH] == 1;
	m_dGrid[iW][iH] = 0;
	if(bUpdateImmediately)
		UpdateWaterPlane ();

	return bChanged;
}

uint8_t obj_WaterPlane::GetWaterGridValue(uint32_t iW, uint32_t iH)
{
	if(iW>=m_dGrid.Width() || iH>= m_dGrid.Height())
		return 0;

	return m_dGrid[iW][iH];
}


void obj_WaterPlane::UpdateWaterPlane ()
{
	if(!m_Plane)
		return;

	if ( m_dGrid.Width() == 0 || m_dGrid.Height() == 0 )
		return;

	m_Plane->Destroy();

	int iCount = 0;
	for ( uint32_t i = 0; i < m_dGrid.Width(); i++ )
		for ( uint32_t j = 0; j < m_dGrid.Height(); j++ )
			if(m_dGrid[i][j])
				iCount++;

	if(iCount == 0)
		return;

	uint32_t iW = m_dGrid.Width();
	uint32_t iH = m_dGrid.Height();

	r3dTL::T2DArray<r3dVector> dGridVertexesPos;
	ComputeWaterVertexesPos(dGridVertexesPos);

	m_Plane->Init(iCount*4,iCount*6, 0);

	R3D_WATER_PLANE_VERTEX * pV = NULL;
	DWORD * pI = NULL;
	int iFirstIndex = 0;
	m_Plane->Lock(iCount*4,iCount*6, &pV, &pI,&iFirstIndex);
	r3d_assert(pV);
	r3d_assert(pI);
	r3d_assert(iFirstIndex==0);

	int iCountAdded = 0;
	r3dVector vMinBound, vMaxBound;

	for ( uint32_t i = 0; i < iW; i++ )
	{
		for ( uint32_t j = 0; j < iH; j++ )
		{
			if(m_dGrid[i][j])
			{
				r3dVector const & v0 = dGridVertexesPos[i][j];
				r3dVector const & v1 = dGridVertexesPos[i][j+1];
				r3dVector const & v2 = dGridVertexesPos[i+1][j+1];
				r3dVector const & v3 = dGridVertexesPos[i+1][j];

				r3dVector vNormal0 = -(v0-v1).Cross(v2-v1).NormalizeTo();
				r3dVector vNormal1 = -(v2-v3).Cross(v0-v3).NormalizeTo();
				r3dVector vNormalSred = (vNormal0+vNormal1).NormalizeTo();

				pV[0].Position = v0;pV[0].Normal = WaterPlaneComputeNormal(dGridVertexesPos,m_dGrid,i,j);
				pV[1].Position = v1;pV[1].Normal = WaterPlaneComputeNormal(dGridVertexesPos,m_dGrid,i,j+1);
				pV[2].Position = v2;pV[2].Normal = WaterPlaneComputeNormal(dGridVertexesPos,m_dGrid,i+1,j+1);
				pV[3].Position = v3;pV[3].Normal = WaterPlaneComputeNormal(dGridVertexesPos,m_dGrid,i+1,j);
				pI[0] = iCountAdded*4 + 0;
				pI[1] = iCountAdded*4 + 1;
				pI[2] = iCountAdded*4 + 2;
				pI[3] = iCountAdded*4 + 0;
				pI[4] = iCountAdded*4 + 2;
				pI[5] = iCountAdded*4 + 3;
				
				pV+=4;
				pI+=6;

				// bound box
				if(!iCountAdded)
				{
					vMinBound = v0;
					vMaxBound = v0;
				}

				vMinBound.x = r3dTL::Min(vMinBound.x, r3dTL::Min(v0.x,r3dTL::Min(v1.x,r3dTL::Min(v2.x,v3.x))));
				vMinBound.y = r3dTL::Min(vMinBound.y, r3dTL::Min(v0.y,r3dTL::Min(v1.y,r3dTL::Min(v2.y,v3.y))));
				vMinBound.z = r3dTL::Min(vMinBound.z, r3dTL::Min(v0.z,r3dTL::Min(v1.z,r3dTL::Min(v2.z,v3.z))));

				vMaxBound.x = r3dTL::Max(vMaxBound.x, r3dTL::Max(v0.x,r3dTL::Max(v1.x,r3dTL::Max(v2.x,v3.x))));
				vMaxBound.y = r3dTL::Max(vMaxBound.y, r3dTL::Max(v0.y,r3dTL::Max(v1.y,r3dTL::Max(v2.y,v3.y))));
				vMaxBound.z = r3dTL::Max(vMaxBound.z, r3dTL::Max(v0.z,r3dTL::Max(v1.z,r3dTL::Max(v2.z,v3.z))));

				iCountAdded++;
				r3d_assert(iCount>=0);
			}
		}
	}

	r3d_assert(iCount==iCountAdded);
	m_Plane->Unlock();

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = vMaxBound - vMinBound;
	m_meshHeight = vMinBound.y;
	bboxLocal.Org = vMinBound;
	if(	m_bWaterPlaneFollowTerrain ) bboxLocal.Org.y = m_meshHeight;// + m_fWaterPlaneHeightOnTerrain;
	else	bboxLocal.Org.y = m_fWaterPlaneHeight;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();


	/*if(HeightTexture)	r3dRenderer->DeleteTexture( HeightTexture );
	HeightTexture = r3dRenderer->AllocateTexture();
	HeightTexture->Create( 512, 512, D3DFMT_L8, 1 );

	D3DLOCKED_RECT lockedRect;
	IDirect3DTexture9* tex = HeightTexture->AsTex2D();
	D3D_V( tex->LockRect( 0, &lockedRect, NULL, 0 ) );

	unsigned char* data = (unsigned char* )lockedRect.pBits;
	for( int z = 0; z < 512; z ++ )	
	{		
		for( int x = 0; x < 512; x ++ )
		{
			r3dPoint3D wpoint = bbox_local.Org + bbox_local.Size * r3dPoint3D(float(x)/512.0f, 0.0f, float(z)/512.0f);
			float h = Terrain->GetHeight ( wpoint );
			if(h>m_fWaterPlaneHeight)	*data++ = 255;
			else	*data++ = 0;
		}
		data += lockedRect.Pitch - sizeof( data[ 0 ] ) * 512;
	}

	D3D_V( tex->UnlockRect( 0 ) );*/
}

void obj_WaterPlane::LoadWaterPlaneGrid(const char*szName)
{
	char dir[MAX_PATH], file[MAX_PATH];
	sprintf(dir, "%s\\%s", r3dGameLevel::GetHomeDir(), WATER_PLANES_DIR);
	sprintf(file, "%s\\%s.dat", dir, szName);
	CreateDirectory(dir, NULL);

	r3dFile * f = r3d_open(file, "rb");
	r3d_assert(f);

	if(!f) {
		r3dArtBug("!!!WARNING!!! obj_WaterPlane %s have no data file %s\n", szName, file);

		setActiveFlag(0);
		return;
	}

	WORD version;
	fread(&version, sizeof(version), 1, f);
	if ( version != WATER_VERSION_ID )
	{
		r3dError("invalid obj_WaterPlane version %d file %s", version, file);
		fclose(f);
		setActiveFlag(0);
		return;
	}

	uint32_t iW = 0;
	uint32_t iH = 0;

	fread(&iW, sizeof(iW), 1, f);
	fread(&iH, sizeof(iH), 1, f);

	m_dGrid.Resize(iW,iH);

	for(uint32_t j=0;j<iH;j++)
	{
		for(uint32_t i=0;i<iW;i++)
		{
			uint8_t iVal = 0;
			fread(&iVal, sizeof(iVal), 1, f);
			m_dGrid[i][j] = iVal;
		}
	}

	fclose(f);
}

void obj_WaterPlane::SaveWaterPlaneGrid(const char*szName)
{
	char dir[MAX_PATH], file[MAX_PATH];
	sprintf(dir, "%s\\%s", r3dGameLevel::GetSaveDir(), WATER_PLANES_DIR);
	sprintf(file, "%s\\%s.dat", dir, szName);
	CreateDirectory(dir, NULL);

	FILE* f = fopen_for_write(file, "wb");
	r3d_assert(f);

	WORD version = WATER_VERSION_ID;
	fwrite(&version, sizeof(version), 1, f);

	uint32_t iW = m_dGrid.Width();
	uint32_t iH = m_dGrid.Height();

	fwrite(&iW, sizeof(iW), 1, f);
	fwrite(&iH, sizeof(iH), 1, f);
	
	for(uint32_t j=0;j<iH;j++)
	{
		for(uint32_t i=0;i<iW;i++)
		{
			uint8_t iVal = m_dGrid[i][j];
			fwrite(&iVal, sizeof(iVal), 1, f);
		}
	}
	fclose(f);
}


void obj_WaterPlane::SaveWaterPlaneGridBin(const char*szName)
{
	char dir[MAX_PATH], file[MAX_PATH];
	sprintf(dir, "%s\\%s", r3dGameLevel::GetSaveDir(), WATER_PLANES_DIR);
	sprintf(file, "%s\\%s_ps3.dat", dir, szName);
	CreateDirectory(dir, NULL);

	FILE* f = fopen_for_write(file, "wb");
	r3d_assert(f);

	WORD version = WATER_VERSION_BIN_ID;
	fwrite(&version, sizeof(version), 1, f);

	int32_t iIndexCount = 0 ;
	int32_t iVertexCount = 0 ;

	R3D_WATER_PLANE_VERTEX * pV = NULL;
	DWORD * pI = NULL;
	int iFirstIndex = 0;

	if( m_Plane->IsValid() )
	{
		iIndexCount = m_Plane->IndexSize;
		iVertexCount = m_Plane->VertexSize;

		m_Plane->Lock(iVertexCount,iIndexCount, &pV, &pI,&iFirstIndex, true);
		r3d_assert(pV);
		r3d_assert(pI);
		r3d_assert(iFirstIndex==0);
	}

	fwrite(&iIndexCount, sizeof(iIndexCount), 1, f);
	fwrite(&iVertexCount, sizeof(iVertexCount), 1, f);
	for(int32_t i=0;i<iIndexCount;i++)
	{
		WORD idx = WORD(*pI);
		fwrite(&idx, sizeof(idx), 1, f);
		pI++;
	}
	for(int32_t i=0;i<iVertexCount;i++)
	{
		R3D_WATER_PLANE_VERTEX vrt = *pV;
		fwrite(&vrt.Position, sizeof(vrt.Position), 1, f);
		fwrite(&vrt.Normal, sizeof(vrt.Normal), 1, f);
		pV++;
	}

	if( m_Plane->IsValid() )
	{
		m_Plane->Unlock ();
	}

	fclose(f);
}

void obj_WaterPlane::ComputeCoastSmoothInfo (r3dTL::T2DArray<uint8_t> & tInfo)
{
	r3d_assert(m_iCoastSmoothLevels<7);

	tInfo.Resize(m_dGrid.Width(),m_dGrid.Height());

	uint32_t iW = tInfo.Width();
	uint32_t iH = tInfo.Height();
	for(uint32_t j=0;j<iH;j++)
		for(uint32_t i=0;i<iW;i++)
			tInfo[i][j] = m_dGrid[i][j]?m_iCoastSmoothLevels+1:0;

	for ( uint32_t k = 1; k < (uint32_t)m_iCoastSmoothLevels+1; k++ )
	{
		for(uint32_t j=0;j<iH;j++)
			for(uint32_t i=0;i<iW;i++)
			{
				if(tInfo[i][j]<k)
					continue;
				
				if((i>0 && tInfo[i-1][j] == k-1)||
					(i<iW-1 && tInfo[i+1][j] == k-1)||
					(j>0 && tInfo[i][j-1] == k-1)||
					(j<iH-1 && tInfo[i][j+1] == k-1))
					tInfo[i][j] = k;
			}
	}
}

void obj_WaterPlane::ComputeWaterVertexesPos (r3dTL::T2DArray<r3dVector> & tInfo)
{
	// FILL ONLY EXISTS VERTEXES INFO
	uint32_t iW = m_dGrid.Width();
	uint32_t iH = m_dGrid.Height();

	r3dTL::T2DArray<uint8_t> dGridCoastInfo;
	ComputeCoastSmoothInfo(dGridCoastInfo);

	tInfo.Clear ();
	tInfo.Resize(iW+1,iH+1);

	for ( uint32_t i = 0; i < iW; i++ )
	{
		for ( uint32_t j = 0; j < iH; j++ )
		{
			if(m_dGrid[i][j])
			{
				r3dVector v0 = r3dVector((i+0)*m_fCellGridSize, 0, (j+0)*m_fCellGridSize);
				r3dVector v1 = r3dVector((i+0)*m_fCellGridSize, 0, (j+1)*m_fCellGridSize);
				r3dVector v2 = r3dVector((i+1)*m_fCellGridSize, 0, (j+1)*m_fCellGridSize);
				r3dVector v3 = r3dVector((i+1)*m_fCellGridSize, 0, (j+0)*m_fCellGridSize);

				if ( m_bWaterPlaneFollowTerrain )
				{
					float f0 = (float)dGridCoastInfo[i][j];
					float f1 = (float)dGridCoastInfo[i][j];
					float f2 = (float)dGridCoastInfo[i][j];
					float f3 = (float)dGridCoastInfo[i][j];
					if(m_iCoastSmoothLevels<=0)
					{
						f0 = f1 = f2 = f3 = 1.0f;
					}
					else
					{
						if(i>0) f0 = r3dTL::Max( (float)dGridCoastInfo[i-1][j], f0);
						if(j>0) f0 = r3dTL::Max( (float)dGridCoastInfo[i][j-1], f0);
						if(i>0&&j>0) f0 = r3dTL::Max( (float)dGridCoastInfo[i-1][j-1], f0);

						if(i>0) f1 = r3dTL::Max( (float)dGridCoastInfo[i-1][j], f1);
						if(j<iH-1) f1 = r3dTL::Max( (float)dGridCoastInfo[i][j+1], f1);
						if(i>0&&j<iH-1) f1 = r3dTL::Max( (float)dGridCoastInfo[i-1][j+1], f1);

						if(i<iW-1) f2 = r3dTL::Max( (float)dGridCoastInfo[i+1][j], f2);
						if(j<iH-1) f2 = r3dTL::Max( (float)dGridCoastInfo[i][j+1], f2);
						if(i<iW-1&&j<iH-1) f2 = r3dTL::Max( (float)dGridCoastInfo[i+1][j+1], f2);

						if(i<iW-1) f3 = r3dTL::Max( (float)dGridCoastInfo[i+1][j], f3);
						if(j>0) f3 = r3dTL::Max( (float)dGridCoastInfo[i][j-1], f3);
						if(j>0&&i<iW-1) f3 = r3dTL::Max( (float)dGridCoastInfo[i+1][j-1], f3);

						f0 = ( (f0 - 1.0f)/(float)m_iCoastSmoothLevels - 0.5f ) * 2.0f;
						f1 = ( (f1 - 1.0f)/(float)m_iCoastSmoothLevels - 0.5f ) * 2.0f;
						f2 = ( (f2 - 1.0f)/(float)m_iCoastSmoothLevels - 0.5f ) * 2.0f;
						f3 = ( (f3 - 1.0f)/(float)m_iCoastSmoothLevels - 0.5f ) * 2.0f;
					}

					v0.y = terra_GetH(v0)+f0*m_fWaterPlaneHeightOnTerrain;
					v1.y = terra_GetH(v1)+f1*m_fWaterPlaneHeightOnTerrain;
					v2.y = terra_GetH(v2)+f2*m_fWaterPlaneHeightOnTerrain;
					v3.y = terra_GetH(v3)+f3*m_fWaterPlaneHeightOnTerrain;
				}

				tInfo[i][j] = v0;
				tInfo[i][j+1] = v1;
				tInfo[i+1][j+1] = v2;
				tInfo[i+1][j] = v3;
			}
		}
	}
}

void obj_WaterPlane::UpdateGridOnChangeCellSize ()
{
	r3d_assert(m_bResizeInfoExists);

	for(uint32_t j = 0; j < m_dGrid.Height (); j++ )
	{
		for(uint32_t i = 0; i < m_dGrid.Width (); i++)
		{
			uint32_t i_old = uint32_t( ( ((float)i+0.5f)*m_fCellGridSize ) / m_fOldCellGridSize );
			uint32_t j_old = uint32_t( ( ((float)j+0.5f)*m_fCellGridSize ) / m_fOldCellGridSize );
			if(/*i_old>=0&&*/i_old<m_dOldGrid.Width () && /*j_old>=0&&*/j_old<m_dOldGrid.Height ())
				m_dGrid[i][j] = m_dOldGrid[i_old][j_old];
			else
				m_dGrid[i][j] = 0;
		}
	}
}

void obj_WaterPlane::ResetResizeInfo ()
{
	if(m_bResizeInfoExists)
	{
		m_dOldGrid.Clear ();
		m_bResizeInfoExists = false;
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_WaterPlane::SetGrid(const GridVecType &v)
{
	m_dGrid = v;
	UpdateWaterPlane();
}






