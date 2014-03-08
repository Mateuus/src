#include "r3dPCH.h"
#include "r3d.h"

#include "../GameEngine/TrueNature/Terrain.h"
#include "../sf/RenderBuffer.h"

#include "GameCommon.h"

#include "GrassLib.h"
#include "GrassGen.h"
#include "GrassMap.h"
#include "GrassEditorPlanes.h"

extern r3dITerrain* Terrain;
extern GrassPlanesManager* g_GrassPlanesManager;
using namespace r3dTL;

GrassMap* g_pGrassMap = 0;

namespace
{
	const char* GrazCellDataFileName = "grasscells.bin";

	char GrazCellData_SIG101[] = "GRAZCELZ101";
	char GrazCellData_SIG101_2[] = "\3\3GRAZCELZ101";
	char GrazCellData_SIG102[] = "\3\3GRAZCELZ102";

	char GrazCellTexesBlob_SIG[] = "\3\3GRAZTEXZ100";
	

	int FULL_VERTEX_SHADER_ID			= -1;
	int MASKED_VERTEX_SHADER_ID			= -1;
	int PIXEL_SHADER_ID					= -1;
	int PIXEL_SHADER_NOCLIP_ID			= -1;
	int P0_PIXEL_SHADER_ID				= -1;
	int P1_PIXEL_SHADER_ID				= -1;

	const D3DFORMAT MASK_TEX_FMT		= D3DFMT_L8;
	const UINT		MASK_TEX_FMT_SIZE	= 1;
	const D3DFORMAT HEIGHT_TEX_FMT		= D3DFMT_L8;
	const UINT		HEIGHT_TEX_FMT_SIZE	= 1;

	clock_t PrevClock = clock();
	float Time = 0;

	typedef TArray< unsigned char > Bytes;

	void ResizeTexture(	uint32_t oldTotalXLen, uint32_t oldTotalZLen, float oldExcessX, float oldExcessZ, const Bytes& oldCompoundTex,
						uint32_t newTotalXLen, uint32_t newTotalZLen, float newExcessX, float newExcessZ, Bytes& newCompoundTex );

	void SaveTexture( FILE* fout, r3dTexture* tex, bool isMask );
	void FillTexture( r3dTexture* tex, const Bytes& data, bool isMask );
}

//------------------------------------------------------------------------

GrassCellEntry::GrassCellEntry()
: MaskTexture( NULL )
, TypeIdx( -1 )
{

}

//------------------------------------------------------------------------

GrassCell::GrassCell()
: HeightTexture( NULL )
, Position( 0.f, 0.f, 0.f )
, YMax( 0.f )
{

}

//------------------------------------------------------------------------

GrassMap::GrassMap()
: mXLength( 0.f )
, mZLength( 0.f )
, mInitialized( false )
, mLoadVersion( 0 )
, mDebugGrassType( -1 ), mCellSize( 0 )
{

}

//------------------------------------------------------------------------

GrassMap::~GrassMap()
{

}

//------------------------------------------------------------------------

void
GrassMap::Init(float width, float height)
{
	r3d_assert( !mInitialized ) ;

	mXLength		= width;
	mZLength		= height;

	mCellSize		= g_pGrassLib->GetSettings().CellScale;

	mCells.Resize(	int( mXLength / mCellSize ) + 1, 
					int( mZLength / mCellSize ) + 1 );
#if 0
	for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
	{
		if( obj->Class->Name == "obj_Grass" )
		{
			obj_Grass* grass = static_cast< obj_Grass* >( obj );

			if( grass->IsIsolated() )
				continue;

			const obj_Grass::Settings& sts = grass->GetSettings();

			GrassCell& cell = mCells[ sts.CellZ ][ sts.CellX ];

			for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
			{
				if( cell.Entries[ i ].Grass->GetSettings().TypeName == sts.TypeName )
				{
					r3dOutToLog( "Conflicting grass map entries: %s at [%d,%d]", sts.TypeName.c_str(), sts.CellX, sts.CellZ );
					continue;
				}
			}

			r3d_assert( sts.CellX >= 0 && sts.CellX < (int)mCells.Width() );
			r3d_assert( sts.CellZ >= 0 && sts.CellZ < (int)mCells.Height() );

			cell.HeightTexture = sts.HeightTex;
			cell.YMin = sts.YMin;
			cell.YMax = sts.YMax;

			GrassCellEntry entry;

			entry.Grass			= grass;
			entry.MaskTexture	= sts.MaskTex;

			cell.Entries.PushBack( entry );
		}
	}
#endif

	mInitialized = true;
}

//------------------------------------------------------------------------

void
GrassMap::Close()
{
	mXLength = 0;
	mZLength = 0;

	mCellSize = 0;

	for( uint32_t j = 0, e = mCells.Height(); j < e; j ++ )
	{
		for( uint32_t i = 0, e = mCells.Width(); i < e; i ++ )
		{
			// non 0 height texture means it's allocated
			GrassCell& cell = mCells[ j ][ i ];
			if( cell.HeightTexture )
			{
				r3dRenderer->DeleteTexture( cell.HeightTexture );
				cell.HeightTexture = NULL;

				for( uint32_t i = 0, e = cell.Entries.Count() ; i < e; i ++ )
				{
					GrassCellEntry& en = cell.Entries[ i ];
					if( en.MaskTexture )
					{
						r3dRenderer->DeleteTexture( en.MaskTexture );
					}
				}
			}
		}
	}

	mCells.Resize( 0, 0 );

	mInitialized = false;
}

//------------------------------------------------------------------------

bool
GrassMap::Save( const r3dString& levelHomeDir )
{
	if( g_pGrassGen->IsDirty() )
	{
		g_pGrassGen->Save() ;
	}

	OptimizeMasks();

	r3dString grazPaz = levelHomeDir + AR_GRAZ_PAZ;

	// try creating just in case
	mkdir( grazPaz.c_str() );

	grazPaz += "\\";

	if( !SaveCellData( grazPaz + GrazCellDataFileName ) )
		return false;

#if 0
	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			const GrassCell& cell = mCells[ z ][ x ];

			if( cell.HeightTexture )
			{
				UString heightGrazPaz = grazPaz + GetGrassMapHeightTexName( x, z );

				D3D_V( D3DXSaveTextureToFile( heightGrazPaz.c_str(), D3DXIFF_DDS, cell.HeightTexture->Tex, NULL ) );

				for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
				{
					const GrassCellEntry& gce = cell.Entries[ i ];

					if( gce.MaskTexture )
					{
						UString maskGrazPaz = grazPaz + GetGrassMapMaskTexName( g_pGrassLib->GetEntryNameByIdx( gce.TypeIdx ), x, z );
						D3D_V( D3DXSaveTextureToFile( maskGrazPaz.c_str(), D3DXIFF_DDS, gce.MaskTexture->Tex, NULL ) );
					}
				}
			}
		}
	}
#endif

	return true;
}

//------------------------------------------------------------------------

bool
GrassMap::Load( const r3dString& levelHomeDir )
{
	if( !LoadCellData( levelHomeDir + AR_GRAZ_PAZ "\\" + GrazCellDataFileName ) )
	{
		if( Terrain )
		{
			const r3dTerrainDesc& desc = Terrain->GetDesc() ;
			Init( desc.XSize, desc.ZSize );
		}
		return false;
	}

	//	Check for incorrect grass data size
	if( Terrain )
	{
		const r3dTerrainDesc& desc = Terrain->GetDesc() ;

		if (	int( desc.XSize / mCellSize ) >  int( mXLength / mCellSize ) || 
				int( desc.ZSize / mCellSize ) >  int( mZLength / mCellSize ) )
		{
			Close();
			Init( desc.XSize, desc.ZSize );
		}
	}

	if( mLoadVersion == 101 )
	{
		bool hasActiveCells = false;

		for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
		{
			for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
			{
				GrassCell& cell = mCells[ z ][ x ];

				const r3dString& grazHeightTexPaz = levelHomeDir + AR_GRAZ_PAZ "\\" + GetGrassMapHeightTexName( x, z );

				if( !r3d_access( grazHeightTexPaz.c_str(), 0 ) )
				{
					cell.HeightTexture = r3dRenderer->LoadTexture( grazHeightTexPaz.c_str() );

					hasActiveCells = true;

					if( cell.HeightTexture->GetWidth() != CELL_HEIGHT_TEX_DIM )
					{
						r3dRenderer->DeleteTexture( cell.HeightTexture );
						cell.HeightTexture = NULL;
						CreateCell( x, z );
					}

					for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
					{
						GrassCellEntry& gce = cell.Entries[ i ];

						const r3dString& grazMaskTexPaz = levelHomeDir + AR_GRAZ_PAZ "\\" + GetGrassMapMaskTexName( g_pGrassLib->GetEntryNameByIdx( gce.TypeIdx ), x, z );

						if( !r3d_access( grazMaskTexPaz.c_str(), 0 ) )
						{
							gce.MaskTexture = r3dRenderer->LoadTexture( grazMaskTexPaz.c_str() );

							if( gce.MaskTexture->GetWidth() != CELL_MASK_TEX_DIM )
							{
								r3dRenderer->DeleteTexture( gce.MaskTexture );
								gce.MaskTexture = r3dRenderer->AllocateTexture();
								
								r3dD3DTextureTunnel texTunenl ;

								r3dFile *ff = r3d_open(grazMaskTexPaz.c_str(),"rb");

								if ( ff )
								{
									if( int ff_size = ff->size )
									{
										void* SrcData = malloc ( ff_size+1 );
										if(SrcData == NULL)
											r3dError("Out of memory!");
										fread(SrcData, 1, ff_size, ff);
										fclose(ff) ;

										r3dDeviceTunnel::D3DXCreateTextureFromFileInMemoryEx( SrcData, ff_size, CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, 1, 0, MASK_TEX_FMT, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_NONE, 0, NULL, NULL, &texTunenl, "Grass Cell Mask");

										free( SrcData );
									}								
								}
								else
								{
									r3dError( "Couldn't find grass texture %s!", grazMaskTexPaz.c_str() );
								}

								gce.MaskTexture->Setup( CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, 1, MASK_TEX_FMT, 1, &texTunenl, false );
								gce.MaskTexture->RegisterCreated();
							}
						}
					}
				}
			}
		}

		if( hasActiveCells && Terrain )
		{
			Terrain->SetOrthoDiffuseTextureDirty() ;
		}
	}

	return true;
}

//------------------------------------------------------------------------

bool
GrassMap::HasGrassCells() const
{
	for( uint32_t j = 0, e = mCells.Height(); j < e; j ++ )
	{
		for( uint32_t i = 0, e = mCells.Width(); i < e; i ++ )
		{
			const GrassCell& cell = mCells[ j ][ i ];
			if( cell.HeightTexture )
			{
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------

void
GrassMap::OptimizeMasks()
{
	float uniformThreshold = g_pGrassLib->GetSettings().UniformMaskThreshold * 255.f;
	float blankTrheshold = g_pGrassLib->GetSettings().BlankMaskThreshold * 255.f;
	float blankMaxTrheshold = g_pGrassLib->GetSettings().MaxBlankMaskThreshold * 255.f;

	uint32_t optimizedUniform( 0 ), optimizedBlank( 0 ), optimizedHeightmap( 0 ), totalCount( 0 );

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			GrassCell& cell = mCells[ z ][ x ];

			for( uint32_t i = 0, e = cell.Entries.Count(); i < e; )
			{
				GrassCellEntry& gce = cell.Entries[ i ];

				bool do_delete = false;

				if( gce.MaskTexture )
				{
					totalCount ++;

					UINT64 pixelSum( 0 );
					uint32_t maxPixel( 0 );

					IDirect3DTexture9* tex = gce.MaskTexture->AsTex2D();

					D3DLOCKED_RECT lrect;
					D3D_V( tex->LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) );

					unsigned char* p = (unsigned char*)lrect.pBits;

					for( uint32_t z = 0, e = CELL_MASK_TEX_DIM; z < e; z ++ )
					{
						for( uint32_t x = 0, e = CELL_MASK_TEX_DIM; x < e; x ++ )
						{
							uint32_t val = *p ++;

							maxPixel = R3D_MAX( maxPixel, val );
							pixelSum += val;
						}

						p += lrect.Pitch - CELL_MASK_TEX_DIM;
					}

					tex->UnlockRect( 0 );

					pixelSum *= 10;
					pixelSum /= CELL_MASK_TEX_DIM * CELL_MASK_TEX_DIM;
					
					float avgPixel = pixelSum / 10.f;

					bool do_optimize( false );

					if( avgPixel > uniformThreshold )
					{
						optimizedUniform ++;
						do_optimize = true;
					}

					if( maxPixel < blankMaxTrheshold && avgPixel < blankTrheshold )
					{
						optimizedBlank ++;
						do_optimize = true;
						do_delete = true;
					}

					if( do_optimize )
					{
						r3dRenderer->DeleteTexture( gce.MaskTexture );
						gce.MaskTexture = NULL;
					}
				}

				if( do_delete )
				{
					cell.Entries.Erase( i );
					e --;

					// deleted all entries, hence height texture is unnecessery
					if( !e )
					{
						r3dRenderer->DeleteTexture( cell.HeightTexture );
						cell.HeightTexture = NULL;
						optimizedHeightmap ++;
					}
				}
				else
				{
					i ++;
				}
			}
		}
	}

	r3dOutToLog( "Grass saving: optimized %d out %d masked tiles ( %d blanks and %d uniforms )\n", 
					optimizedBlank + optimizedUniform,
					totalCount,
					optimizedBlank,
					optimizedUniform );

	r3dOutToLog( "Grass saving: optimized %d heightmaps\n", optimizedHeightmap );

}

//------------------------------------------------------------------------

void
GrassMap::Paint( float x, float z, float radius, float dir, const r3dString& grassType )
{
	r3d_assert( mInitialized );

	if (mCells.Width() == 0 || mCells.Height() == 0)
		return;

	if( Terrain && !Terrain->GetDesc().OrthoDiffuseTex )
	{
		Terrain->SetOrthoDiffuseTextureDirty() ;
	}

	uint32_t grazTypeIdx = g_pGrassLib->GetEntryIdxByName( grassType );
	r3d_assert( grazTypeIdx != uint32_t(-1) );

	float centreDX, centreDZ;
	float unusedDX, unusedDZ;

	int centreX, centreZ;

	int upperLeftX, upperLeftZ;
	int lowerRightX, lowerRightZ;

	DeriveCellCoords( x, z, centreX, centreZ, centreDX, centreDZ );

	DeriveCellCoords( x - radius, z - radius, upperLeftX, upperLeftZ, unusedDX, unusedDZ );
	DeriveCellCoords( x + radius, z + radius, lowerRightX, lowerRightZ, unusedDX, unusedDZ );

	float relRad = radius / mCellSize;

	upperLeftX	= R3D_MIN( R3D_MAX( upperLeftX, 0 ), (int)mCells.Width() - 1 );
	lowerRightX	= R3D_MIN( R3D_MAX( lowerRightX, 0 ), (int)mCells.Width() - 1 );

	upperLeftZ	= R3D_MIN( R3D_MAX( upperLeftZ, 0 ), (int)mCells.Height() - 1 );
	lowerRightZ	= R3D_MIN( R3D_MAX( lowerRightZ, 0 ), (int)mCells.Height() - 1 );

	for( int iz = upperLeftZ, e = lowerRightZ; iz <= e; iz ++ )
	{
		for( int ix = upperLeftX, e = lowerRightX; ix <= e; ix ++ )
		{
			float dx = centreX + centreDX - ix;
			float dz = centreZ + centreDZ - iz;

			GrassCell& cell = mCells[ iz ][ ix ];

			if( !cell.HeightTexture )
			{
				if( dir > 0.f )
					CreateCell( ix, iz );
				else
					continue ;
			}

			GrassCellEntry* pgce ( NULL );

			for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
			{
				GrassCellEntry& gce = cell.Entries[ i ];

				if( gce.TypeIdx == grazTypeIdx )
				{
					pgce = &gce;
				}
			}

			if( !pgce )
			{
				CreateCellEntry( cell, ix, iz, grazTypeIdx );
				pgce = &cell.Entries[ cell.Entries.Count() - 1 ];
			}

			if( dir < 0.f && !pgce->MaskTexture )
			{
				pgce->MaskTexture = r3dRenderer->AllocateTexture();
				pgce->MaskTexture->Create( CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, MASK_TEX_FMT, 1 );

				Bytes bytes( CELL_MASK_TEX_DIM * CELL_MASK_TEX_DIM * MASK_TEX_FMT_SIZE, 0xff );
				FillTexture( pgce->MaskTexture, bytes, true );
			}

			if( pgce->MaskTexture )
			{
				DoPaint( dx, dz, relRad, dir, pgce->MaskTexture );
			}
		}
	}
}

//------------------------------------------------------------------------

void
GrassMap::UpdateHeight( float x, float z, float radius )
{
	r3d_assert( mInitialized );

	float unusedDX, unusedDZ;

	int upperLeftX, upperLeftZ;
	int lowerRightX, lowerRightZ;

	DeriveCellCoords( x - radius, z - radius, upperLeftX, upperLeftZ, unusedDX, unusedDZ );
	DeriveCellCoords( x + radius, z + radius, lowerRightX, lowerRightZ, unusedDX, unusedDZ );

	float relRad = radius / mCellSize;

	for( int	iz	= R3D_MAX( upperLeftZ, 0 ), 
				e	= R3D_MIN( lowerRightZ, (int)mCells.Height() - 1 ); 
				iz <= e; iz ++ )
	{
		for( int	ix	= R3D_MAX( upperLeftX, 0 ), 
					e	= R3D_MIN( lowerRightX, (int)mCells.Width() - 1 ); 
					ix <= e; ix ++ )
		{
			GrassCell& cell = mCells[ iz ][ ix ];

			if( cell.HeightTexture )
			{
				UpdateCellHeight( ix, iz, cell );
			}
		}
	}
}

//------------------------------------------------------------------------

void
GrassMap::UpdateHeight()
{
	for( int z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( int x = 0, e = mCells.Width(); x < e; x ++ )
		{
			GrassCell& cell = mCells[ z ][ x ];

			if( cell.HeightTexture )
			{
				UpdateCellHeight( x, z, cell );
			}
		}
	}
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------

namespace
{
	struct SetRestoreClampAddressMode
	{
		explicit SetRestoreClampAddressMode( DWORD samp )
		{
			Sampler = samp;

			D3D_V( r3dRenderer->pd3ddev->GetSamplerState( Sampler, D3DSAMP_ADDRESSU, &PrevAddressU ) );
			D3D_V( r3dRenderer->pd3ddev->GetSamplerState( Sampler, D3DSAMP_ADDRESSV, &PrevAddressV ) );

			D3D_V( r3dRenderer->pd3ddev->SetSamplerState( Sampler, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ) );
			D3D_V( r3dRenderer->pd3ddev->SetSamplerState( Sampler, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ) );
		}

		~SetRestoreClampAddressMode()
		{
			D3D_V( r3dRenderer->pd3ddev->SetSamplerState( Sampler, D3DSAMP_ADDRESSU, PrevAddressU ) );
			D3D_V( r3dRenderer->pd3ddev->SetSamplerState( Sampler, D3DSAMP_ADDRESSV, PrevAddressV ) );
		}

		DWORD PrevAddressU;
		DWORD PrevAddressV;

		DWORD Sampler;

	};
}


//------------------------------------------------------------------------

R3D_FORCEINLINE static void PushDebugBBox( const r3dBoundBox& bbox )
{
	void PushDebugBox(	r3dPoint3D p0, r3dPoint3D p1, r3dPoint3D p2, r3dPoint3D p3,
						r3dPoint3D p4, r3dPoint3D p5, r3dPoint3D p6, r3dPoint3D p7,
						r3dColor color ) ;

	PushDebugBox(	bbox.Org,
					bbox.Org + r3dPoint3D( bbox.Size.x, 0.f, 0.f ),
					bbox.Org + r3dPoint3D( 0.f, bbox.Size.y, 0.f ), 
					bbox.Org + r3dPoint3D( bbox.Size.x, bbox.Size.y, 0.f ), 
					bbox.Org + r3dPoint3D( 0.f, 0.f, bbox.Size.z ), 
					bbox.Org + r3dPoint3D( bbox.Size.x, 0.f, bbox.Size.z ),
					bbox.Org + r3dPoint3D( 0.f, bbox.Size.y, bbox.Size.z ), 
					bbox.Org + r3dPoint3D( bbox.Size.x, bbox.Size.y, bbox.Size.z ),
					r3dColor( 0, 255, 0 )
					) ;
}

void
GrassMap::Prepare()
{
	if( Terrain )
		Terrain->UpdateOrthoDiffuseTexture();
}

void
GrassMap::Draw( const r3dCamera& cam, Path path, bool useDepthEqual )
{
	R3DPROFILE_FUNCTION("GrassMap::Draw");

	// ensure we did initialize
	r3d_assert( FULL_VERTEX_SHADER_ID >= 0 );

	SetRestoreClampAddressMode setRestoreClamp0( 0 ); (void)setRestoreClamp0;
	SetRestoreClampAddressMode setRestoreClamp1( 1 ); (void)setRestoreClamp1;

	// compute bounds in terms of cell xz

	float grazVizRad = GetVisRad();

	float maxFadeDistanceCoef = 0.f ;

	for( int i = 0, e = g_pGrassLib->GetEntryCount() ; i < e ; i ++ )
	{
		const GrassLibEntry& gle = g_pGrassLib->GetEntry( i ) ;
		maxFadeDistanceCoef = R3D_MAX( maxFadeDistanceCoef, gle.FadeDistance ) ;
	}

	maxFadeDistanceCoef = R3D_MAX( R3D_MIN( maxFadeDistanceCoef, R3D_MAX_GRASS_PATCH_DISTANCE_COEF ), 0.f )	 ;
	grazVizRad *= maxFadeDistanceCoef ;

	float xmin = cam.x - grazVizRad;
	float xmax = cam.x + grazVizRad;

	float zmin = cam.z - grazVizRad;
	float zmax = cam.z + grazVizRad;

	int ZCellMin = int( zmin / mCellSize );
	int ZCellMax = int( zmax / mCellSize );

	int	z	= R3D_MAX( ZCellMin, 0 ),
		ze	= R3D_MIN( ZCellMax, int( mCells.Height() ) - 1 );

	float fz = z * mCellSize - cam.z;

	float grazVizRadSqr = grazVizRad * grazVizRad;

	int width = mCells.Width();

	float maxMeshScale			= g_pGrassLib->GetSettings().MaxMeshScale;
	r3dPoint3D maxMeshScaleVec ( maxMeshScale, maxMeshScale, maxMeshScale );

	r3dPoint3D cellScaleBase (	mCellSize + maxMeshScale * 2, 
								mCellSize + maxMeshScale * 2,
								mCellSize + maxMeshScale * 2 );

	//------------------------------------------------------------------------
	// Setup common render states

	switch( path )
	{
	case COMBINED_PATH:
		if( useDepthEqual )
		{
			r3dRenderer->SetPixelShader( PIXEL_SHADER_NOCLIP_ID );
		}
		else
		{
			r3dRenderer->SetPixelShader( PIXEL_SHADER_ID );
		}
			
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_ZC | R3D_BLEND_ZW | R3D_BLEND_PUSH );
		break ;

	case DEPTH_PATH:
		r3dRenderer->SetPixelShader( P0_PIXEL_SHADER_ID );
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_ZC | R3D_BLEND_ZW | R3D_BLEND_PUSH );
		break ;

	case COLOR_PATH:
		r3dRenderer->SetPixelShader( P1_PIXEL_SHADER_ID );
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_ZC | R3D_BLEND_PUSH );
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE ) );
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE1, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE ) );
		break ;
	}

	if( useDepthEqual )
	{
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_ZFUNC, D3DCMP_EQUAL ) ) ;
	}

	r3dTexture *tex = __r3dShadeTexture[2];
	if ( Terrain )
	{
		tex = Terrain->GetDesc().OrthoDiffuseTex ;
	}
	r3dRenderer->SetTex( tex, 1 );
	r3dSetFiltering( R3D_BILINEAR, 1 );

	//------------------------------------------------------------------------

	float halfCellSize = mCellSize * 0.5f;

	for( ; z <= ze; z ++, fz += mCellSize )
	{
		float fz_for_x0 = fz;
		float fz_for_x1 = fz + mCellSize;

		// oh no...
		if( ((unsigned&) fz_for_x0  ^ (unsigned&) fz_for_x1 ) & 0x80000000 )
		{
			fz_for_x0 = 0.f;
		}

		float fz_for_x = R3D_MIN( fabsf( fz_for_x0 ), fabsf( fz_for_x1 ) );

		float fx = sqrtf( R3D_MAX( grazVizRadSqr - fz_for_x * fz_for_x, 0.f ) );

		float fx_min = -fx + cam.x ;
		float fx_max = +fx + cam.x;

		int XCellMin = R3D_MAX( int( fx_min / mCellSize ), 0 );
		int XCellMax = R3D_MIN( int( fx_max / mCellSize ), width - 1 );

		for( int x = XCellMin, xe = XCellMax; x <= xe; x ++ )
		{			
			const GrassCell& cell = mCells[ z ][ x ];

			if( cell.HeightTexture )
			{
				r3dBoundBox bbox;

				bbox.Org	= cell.Position - maxMeshScaleVec;
				bbox.Size	= cellScaleBase;

				bbox.Size.y	= maxMeshScale * 3 + cell.YMax - cell.Position.y;

				if( !r3dRenderer->IsBoxInsideFrustum( bbox ) )
					continue;

				DrawCell( cell, cam, x, z );

#ifndef FINAL_BUILD
				if( r_grass_show_boxes->GetInt() )
				{

					for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
					{
						if( cell.Entries[ i ].TypeIdx == mDebugGrassType )
						{
							PushDebugBBox( bbox ) ;
							break ;
						}
					}				
				}
#endif
			}
		}
	}

	if( path == COLOR_PATH )
	{
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE1, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );
	}

	if( useDepthEqual )
	{
		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL ) ) ;
	}

	r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
}

//------------------------------------------------------------------------

void
GrassMap::ConformWithNewCellSize()
{
	uint32_t totalXLen = CELL_MASK_TEX_DIM * mCells.Width();
	uint32_t totalZLen = CELL_MASK_TEX_DIM * mCells.Height();

	if( totalXLen * totalZLen > ( 256 + 1 ) * ( 256 + 1 ) * 32 * 32 )
	{
		r3dArtBug( "GrassMap::ConformWithNewCellSize: current map is too detailed, can't resize - resetting to new size\n" );
		ClearCells();
		return;
	}

	typedef TArray< Bytes > CompoundTextures;
		
	CompoundTextures compoundTexes ( g_pGrassLib->GetEntryCount() );

	for( uint32_t i = 0, e = compoundTexes.Count(); i < e; i ++ )
	{
		compoundTexes[ i ].Resize( totalXLen * totalZLen );
	}

	for( uint32_t typeIdx = 0, e = g_pGrassLib->GetEntryCount(); typeIdx < e; typeIdx ++ )
	{
		unsigned char* compoundTex = &compoundTexes[ typeIdx ][ 0 ];

		for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
		{
			for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
			{
				const GrassCell& cell = mCells[ z ][ x ];

				for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
				{
					const GrassCellEntry& gce = cell.Entries[ i ];
					if( gce.TypeIdx == typeIdx )
					{
						char* dest = (char*)compoundTex + z * CELL_MASK_TEX_DIM * totalXLen + x * CELL_MASK_TEX_DIM;

						if( gce.MaskTexture )
						{
							IDirect3DTexture9* tex = gce.MaskTexture->AsTex2D();

							D3DLOCKED_RECT lrect;
							D3D_V( tex->LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) );

							char* data = (char*)lrect.pBits;

							for( uint32_t tz = 0, tze = CELL_MASK_TEX_DIM; tz < tze; tz ++ )
							{
								for( uint32_t tx = 0, txe = CELL_MASK_TEX_DIM; tx < txe; tx ++ )
								{
									*dest++ = *data++;
								}

								dest += totalXLen - CELL_MASK_TEX_DIM;
								data += lrect.Pitch - CELL_MASK_TEX_DIM;
							}

							D3D_V( tex->UnlockRect( 0 ) );
						}
						else
						{

							for( uint32_t tz = 0, tze = CELL_MASK_TEX_DIM; tz < tze; tz ++ )
							{
								for( uint32_t tx = 0, txe = CELL_MASK_TEX_DIM; tx < txe; tx ++ )
								{
									*dest++ = (char)255;
								}

								dest += totalXLen - CELL_MASK_TEX_DIM;
							}							
						}
					}
				}
			}
		}
	}

	float oldCellSize	= mCellSize;
	float oldExcessX	= mCells.Width() - mXLength / mCellSize;
	float oldExcessZ	= mCells.Height() - mZLength / mCellSize;

	ClearCells();

	uint32_t newTotalXLen = CELL_MASK_TEX_DIM * mCells.Width();
	uint32_t newTotalZLen = CELL_MASK_TEX_DIM * mCells.Height();

	float newExcessX	= mCells.Width() - mXLength / mCellSize;
	float newExcessZ	= mCells.Height() - mZLength / mCellSize;

	for( uint32_t i = 0, e = compoundTexes.Count() ; i < e ; i ++ )
	{
		Bytes newCompoundTex ( newTotalXLen * newTotalZLen );

		ResizeTexture(	totalXLen, totalZLen, oldExcessX, oldExcessZ, compoundTexes[ i ],
						newTotalXLen, newTotalZLen, newExcessX, newExcessZ, newCompoundTex );

		CreateCells( newCompoundTex, i );
	}
}

//------------------------------------------------------------------------

void
GrassMap::ClearCells()
{
	float xl = mXLength;
	float zl = mZLength;
	Close();
	Init(xl, zl);
}

//------------------------------------------------------------------------

void
GrassMap::ClearGrassType( const r3dString& grassType )
{
	uint32_t grazTypeIdx = g_pGrassLib->GetEntryIdxByName( grassType );

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			GrassCell& cell = mCells[ z ][ x ];

			for( uint32_t i = 0, e = cell.Entries.Count(); i < e; )
			{
				GrassCellEntry& gce = cell.Entries[ i ];
				if( gce.TypeIdx == grazTypeIdx )
				{
					e -- ;
					cell.Entries.Erase( i ) ;
				}
				else
					i ++ ;
			}
		}
	}

}

//------------------------------------------------------------------------

r3dString
GrassMap::GetUsedTypes() const
{
	typedef std::set< int > UsedIndexes ;

	UsedIndexes usedIndexes ;

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			const GrassCell& cell = mCells[ z ][ x ];

			for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
			{
				const GrassCellEntry& gce = cell.Entries[ i ];
				usedIndexes.insert( gce.TypeIdx ) ;
			}
		}
	}

	r3dString result ;

	for( UsedIndexes::const_iterator i = usedIndexes.begin(), e = usedIndexes.end() ; i != e ; ++ i )
	{
		result += g_pGrassLib->GetEntryNameByIdx( *i ) ;
		result += "\n" ;
	}

	return result ;
}

//------------------------------------------------------------------------

void GrassMap::SetDebugType( const r3dString& typeName )
{
	mDebugGrassType = g_pGrassLib->GetEntryIdxByName( typeName ) ;
}

//------------------------------------------------------------------------
/*static*/

float
GrassMap::GetVisRad()
{
	return r_grass_view_coef->GetFloat() * r_grass_view_dist->GetFloat();
}

//------------------------------------------------------------------------

void
GrassMap::DeriveCellCoords( float X, float Z, int& oCellX, int& oCellZ, float& oCellDX, float& oCellDZ )
{
	float relX = X / mCellSize;
	float relZ = Z / mCellSize;

	oCellX = int( relX );
	oCellZ = int( relZ );

	oCellDX = relX - oCellX;
	oCellDZ = relZ - oCellZ;
}

//------------------------------------------------------------------------

void
GrassMap::ToWorldCoords( int CellX, int CellZ, float CellDX, float CellDZ, float& oX, float& oZ )
{
	oX = ( CellX + CellDX ) * mCellSize;
	oZ = ( CellZ + CellDZ ) * mCellSize;
}

//------------------------------------------------------------------------

void
GrassMap::CreateCell( int X, int Z )
{
	r3d_assert( mInitialized );

	GrassCell& cell = mCells[ Z ][ X ];

	CreateCellHeightTexture( cell );

	ToWorldCoords( X, Z, 0, 0, cell.Position.x, cell.Position.z );
	UpdateCellHeight( X, Z, cell );

}

//------------------------------------------------------------------------

void
GrassMap::CreateCells( const Bytes& CompoundTex, uint32_t TypeIdx )
{
	uint32_t totalDim = mCells.Width() * CELL_MASK_TEX_DIM;

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			uint32_t x0 = x * CELL_MASK_TEX_DIM;
			uint32_t x1 = x0 + CELL_MASK_TEX_DIM;

			uint32_t z0 = z * CELL_MASK_TEX_DIM;
			uint32_t z1 = z0 + CELL_MASK_TEX_DIM;

			bool found = false;

			// see if we have any non zero mask points
			{
				for( uint32_t z = z0, e = z1; z < e && !found ; z ++ )
				{
					for( uint32_t x = x0, e = x1; x < e && !found ; x ++ )
					{
						if( CompoundTex[ x + z * totalDim ] )
						{
							found = true;
						}
					}
				}
			}

			if( found )
			{
				CreateCell( x, z );

				GrassCell& cell = mCells[ z ][ x ];

				CreateCellEntry( cell, x, z, TypeIdx );

				GrassCellEntry& gce = cell.Entries[ cell.Entries.Count() - 1 ];

				IDirect3DTexture9* tex = gce.MaskTexture->AsTex2D();

				D3DLOCKED_RECT lrect;
				D3D_V( tex->LockRect( 0, &lrect, NULL, 0 ) );

				unsigned char* p = (unsigned char*)lrect.pBits;

				for( uint32_t z = 0, zz = z0, e = CELL_MASK_TEX_DIM; z < e; z ++, zz++ )
				{
					for( uint32_t x = 0, xx = x0, e = CELL_MASK_TEX_DIM; x < e; x ++, xx++ )
					{
						*p ++ = CompoundTex[ xx + zz * totalDim ];
					}

					p += lrect.Pitch - CELL_MASK_TEX_DIM;
				}

				tex->UnlockRect( 0 );
			}
		}
	}
}

//------------------------------------------------------------------------

void
GrassMap::CreateCellEntry( GrassCell& cell, int X, int Z, uint32_t TypeIdx )
{
	GrassCellEntry entry;

	float wx, wz;
	ToWorldCoords( X, Z , 0, 0, wx, wz );

	entry.TypeIdx		= TypeIdx;
	entry.MaskTexture	= r3dRenderer->AllocateTexture();

	entry.MaskTexture->Create( CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, MASK_TEX_FMT, 1 );

	D3DLOCKED_RECT lockedRect;

	IDirect3DTexture9* tex = entry.MaskTexture->AsTex2D();

	D3D_V( tex->LockRect( 0, &lockedRect, NULL, 0 ) );

	unsigned char* data = (unsigned char* )lockedRect.pBits;

	memset( data, 0, CELL_MASK_TEX_DIM * lockedRect.Pitch );

	D3D_V( tex->UnlockRect( 0 ) );

	cell.Entries.PushBack( entry );
}

//------------------------------------------------------------------------

void
GrassMap::DoPaint( float CentreDX, float CentreDZ, float Radius, float Dir, r3dTexture* Tex )
{
	int sx = R3D_MIN( R3D_MAX( int(( CentreDX - Radius ) * CELL_MASK_TEX_DIM ), 0 ), CELL_MASK_TEX_DIM - 1 );
	int sz = R3D_MIN( R3D_MAX( int(( CentreDZ - Radius ) * CELL_MASK_TEX_DIM ), 0 ), CELL_MASK_TEX_DIM - 1 );
	int ex = R3D_MIN( R3D_MAX( int(( CentreDX + Radius ) * CELL_MASK_TEX_DIM ), 0 ), CELL_MASK_TEX_DIM - 1 );
	int ez = R3D_MIN( R3D_MAX( int(( CentreDZ + Radius ) * CELL_MASK_TEX_DIM ), 0 ), CELL_MASK_TEX_DIM - 1 );

	IDirect3DTexture9* tex = static_cast<IDirect3DTexture9*> ( Tex->GetD3DTexture() );

	D3DLOCKED_RECT lockedRect;
	D3D_V( tex->LockRect( 0, &lockedRect, NULL, 0 ) );

	unsigned char* weights = (unsigned char*)lockedRect.pBits;

	float Scale = 255.f;

	bool goDown = Dir > 0 ? false : true;

	float A, B;

	if( goDown )
	{
		A = 0.0f;
		B = 1.0f;
	}
	else
	{
		A = 1.0f;
		B = -1.0f;
	}

	for( int j = sz; j <= ez; j ++ )
	{
		for( int i = sx; i <= ex; i ++ )
		{
			unsigned char* dest = weights + j * lockedRect.Pitch + i;

			float dx = CentreDX - float( i ) / CELL_MASK_TEX_DIM;
			float dz = CentreDZ - float( j ) / CELL_MASK_TEX_DIM;

			float dist = sqrtf( dx*dx + dz*dz );

			float f = dist / Radius;

			int toReplace = int( Scale * R3D_MAX( A + B * f * f * f * f, 0.f ) );

			if( goDown )
				*dest = R3D_MAX( R3D_MIN( (int)*dest, toReplace ), 0 );
			else
				*dest = R3D_MIN( R3D_MAX( (int)*dest, toReplace ), 255 );
		}
	}

	D3D_V( tex->UnlockRect( 0 ) );
}

//------------------------------------------------------------------------

void
GrassMap::UpdateCellHeight( int X, int Z, GrassCell& cell )
{
	float ymin( FLT_MAX ), ymax( -FLT_MAX );
	for( int z = 0, e = CELL_HEIGHT_TEX_DIM; z < e; z ++ )	
	{		
		for( int x = 0, e = CELL_HEIGHT_TEX_DIM; x < e; x ++ )
		{
			r3dPoint3D wpoint( 0, 0, 0 );

			ToWorldCoords( X, Z, float( x ) / ( CELL_HEIGHT_TEX_DIM - 1 ), float( z ) / ( CELL_HEIGHT_TEX_DIM - 1 ), wpoint.x, wpoint.z );

			float height = GetGrassHeight ( wpoint );

			ymin = R3D_MIN( ymin, height );
			ymax = R3D_MAX( ymax, height );
		}
	}

	float ySize = ymax - ymin;

	D3DLOCKED_RECT lockedRect;

	IDirect3DTexture9* tex = cell.HeightTexture->AsTex2D();

	D3D_V( tex->LockRect( 0, &lockedRect, NULL, 0 ) );

	unsigned char* data = (unsigned char* )lockedRect.pBits;

	for( int z = 0, e = CELL_HEIGHT_TEX_DIM; z < e; z ++ )	
	{		
		for( int x = 0, e = CELL_HEIGHT_TEX_DIM; x < e; x ++ )
		{
			r3dPoint3D wpoint( 0, 0, 0 );

			ToWorldCoords( X, Z, float( x ) / ( CELL_HEIGHT_TEX_DIM - 1 ), float( z ) / ( CELL_HEIGHT_TEX_DIM - 1 ), wpoint.x, wpoint.z );

			float height = GetGrassHeight ( wpoint );

			float normHeight = ( height - ymin ) / ySize;

			*data++ = R3D_MIN( R3D_MAX( int ( normHeight * 255.f ), 0 ), 255 );
		}

		data += lockedRect.Pitch - sizeof( data[ 0 ] ) * CELL_HEIGHT_TEX_DIM;
	}

	D3D_V( tex->UnlockRect( 0 ) );

	// update cell height data
	{
		cell.Position.y		= ymin;
		cell.YMax			= ymax;
	}
}

//------------------------------------------------------------------------

void
GrassMap::DrawCell( const GrassCell& cell, const r3dCamera& Cam, int X, int Z )
{
	D3DPERF_BeginEvent ( 0x0, L"DrawGrass" );

	r3dPoint3D halfChunkScale = GetGrassChunkScale() * 0.5f;
	float halfCellScale = mCellSize * 0.5f;

	D3DXMATRIX scaleMtx;
	D3DXMATRIX animScaleMtx;

	D3DXMatrixScaling( &scaleMtx, halfChunkScale.x, halfChunkScale.y, halfChunkScale.z );

	float twoPi = 2 * R3D_PI;

	float animScaleX = twoPi / halfChunkScale.x;
	float animScaleZ = twoPi / halfChunkScale.z;

	float visRad = GetVisRad() ;

	D3DXMatrixScaling( &animScaleMtx, animScaleX, twoPi / halfChunkScale.y, animScaleZ );

	float heightScale = (cell.YMax - cell.Position.y) / halfChunkScale.y;

	D3DXVECTOR4 vDistConstant = D3DXVECTOR4( Cam.x * animScaleX, Cam.z * animScaleZ, 0.f, 1.f ) ;

	float invTerraWidth		=  1.f / mXLength ;
	float invTerraHeight	=  1.f / mZLength ;

	if( Terrain )
	{
		const r3dTerrainDesc& desc = Terrain->GetDesc() ;

		invTerraWidth = 1.f / desc.XSize ;
		invTerraHeight = 1.f / desc.ZSize ;
	}

	r3dRenderer->SetCullMode( D3DCULL_NONE );

	for( uint32_t i = 0, e = cell.Entries.Count(); i < e; i ++ )
	{
		const GrassCellEntry& gce = cell.Entries[ i ];

		const GrassLibEntry& gle = g_pGrassLib->GetEntry( gce.TypeIdx );

		r3dPoint3D pos = cell.Position + r3dPoint3D( halfCellScale, 0.f, halfCellScale );

		if( ( pos - gCam ).Length() > gle.FadeDistance * visRad + mCellSize )
			continue ;

		if( !r_grass_show_debug->GetBool() )
		{
			float visRadScaledSqr = visRad * animScaleX * gle.FadeDistance ;
			visRadScaledSqr *= visRadScaledSqr;

			vDistConstant.z = -5.f / visRadScaledSqr ;
			vDistConstant.w = 5.f ;
		}
		else
		{
			vDistConstant.z = 0.f ;
			vDistConstant.w = 1.f ;
		}

		for( uint32_t i = 0, e = gle.Chunks.Count(); i < e; i ++ )
		{
			const GrassChunk& chunk =	gle.Chunks[ i ];

			int varIdx =  ( X ^ Z ) % chunk.NumVariations;

			chunk.VertexBuffs[ varIdx ]->Set( 0 );
			chunk.IndexBuffer->Set();

			D3DXMATRIX world;

			D3DXMatrixTranslation( &world, pos.x, pos.y, pos.z );

			D3DXMatrixMultiply( &world, &scaleMtx, &world );

			D3DXMATRIX ShaderMat =  world * r3dRenderer->ViewProjMatrix ;
			D3DXMatrixTranspose( &ShaderMat, &ShaderMat );

			D3DXVECTOR4 vConsts[ 10 ];
			// float4x4 	mWVP 				: register( c0 );
			memcpy( vConsts, &ShaderMat, sizeof ShaderMat );
			
			D3DXMatrixMultiply( &world, &world, &animScaleMtx );

			// float4 		vDescaleX			: register( c4 );
			vConsts[4] = D3DXVECTOR4( world.m[0][0], world.m[1][0], world.m[2][0], world.m[3][0] );

			// float4 		vDescaleZ			: register( c5 );
			vConsts[5] = D3DXVECTOR4( world.m[0][2], world.m[1][2], world.m[2][2], world.m[3][2] );

			// float4 		vHeightScale_Time	: register( c6 );
			vConsts[6] = D3DXVECTOR4( heightScale, r_grass_anim_amp->GetFloat(), Time, 0 );

			// float4		vTexcoordTransform	: register( c7 );
			vConsts[7] = D3DXVECTOR4( 0.5f * halfChunkScale.x / halfCellScale, 0.5f, 0.f, 0.f );

			// float4		vDistFade			: register( c8 );
			vConsts[8] = vDistConstant;

			// float4		vWorldTexcTransform	: register( c9 );
			vConsts[9] = D3DXVECTOR4(	halfChunkScale.x, halfChunkScale.z, pos.x, pos.z );

			vConsts[9].x *= invTerraWidth;
			vConsts[9].y *= invTerraHeight;
			vConsts[9].z *= invTerraWidth;
			vConsts[9].w *= invTerraHeight;

			vConsts[9].y = -vConsts[9].y;
			vConsts[9].w = 1.0f - vConsts[9].w;

			D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float*)vConsts, sizeof vConsts / sizeof vConsts[ 0 ] ) );

#if 0
			float len = 2.f * sqrtf( Cam.vPointTo.x * Cam.vPointTo.x + Cam.vPointTo.z * Cam.vPointTo.z );

			float vPSConsts[8] = {	Cam.vPointTo.x / len + 0.5f, 0.5f, Cam.vPointTo.z / len + 0.5f, 0,
									chunk.AlphaRef, 0, 0, 0 };
#else
			// float4 vCamNormal : register( c0 );
			float vPSConsts[8] = {	0.5f, 1.0f, 0.5f, 0,
			// float2 vAlphaRef_TintStrength	: register( c1 );
									chunk.AlphaRef, chunk.TintStrength, 0, 0 };

#endif

			D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( 0, vPSConsts, 2 ) );

			r3dSetFiltering( R3D_ANISOTROPIC, 0 );
			r3dRenderer->SetTex( chunk.Texture );

			r3dSetFiltering( R3D_BILINEAR, D3DVERTEXTEXTURESAMPLER0 );
			r3dRenderer->SetTex( cell.HeightTexture, D3DVERTEXTEXTURESAMPLER0 );

			r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

			d3dc._SetDecl( GrassChunk::VDecl );

			if( gce.MaskTexture )
			{
				r3dSetFiltering( R3D_BILINEAR, D3DVERTEXTEXTURESAMPLER1 );
				r3dRenderer->SetTex( gce.MaskTexture, D3DVERTEXTEXTURESAMPLER1 );

				r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
				r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

				r3dRenderer->SetVertexShader( MASKED_VERTEX_SHADER_ID );
			}
			else
			{
				r3dRenderer->SetVertexShader( FULL_VERTEX_SHADER_ID );
			}

			UINT triCount = chunk.IndexBuffer->GetItemCount() / 3;

			r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, chunk.VertexBuffs[ varIdx ]->GetItemCount(), 0, triCount  );
		}
	}

	r3dRenderer->RestoreCullMode();

	D3DPERF_EndEvent ();
	
}

//------------------------------------------------------------------------

bool
GrassMap::SaveCellData( const r3dString& FilePath )
{
	r3d_assert( g_pGrassLib->GetEntryCount() ) ;

	FILE* fout = fopen( FilePath.c_str(), "wb" );

	if( !fout )
	{
		r3dOutToLog( "GrassMap::SaveCellData: Couldn't open grass cells file %s for writing!", FilePath.c_str() );
		return false;
	}

	struct AutoClose
	{
		~AutoClose()
		{
			fclose( file );
		}

		FILE *file;
	} autoClose = { fout }; (void)autoClose;

	fwrite( GrazCellData_SIG102, sizeof GrazCellData_SIG102, 1, fout );

	// write type map
	uint32_t typeCount = g_pGrassLib->GetEntryCount();
	fwrite_be( typeCount, fout );

	for( uint32_t i = 0, e = typeCount; i < e; i ++ )
	{
		const char* str = g_pGrassLib->GetEntryNameByIdx( i ).c_str();
		fwrite( str, strlen( str ) + 1, 1, fout );
	}

	// write texture dimmensions
	fwrite_be( CELL_HEIGHT_TEX_DIM, fout );
	fwrite_be( CELL_MASK_TEX_DIM, fout );

	uint32_t val = mCells.Width();
	fwrite_be( val, fout );

	val = mCells.Height();
	fwrite_be( val, fout );

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			const GrassCell& cell = mCells[ z ][ x ];

			char haveHeightTexture = !!cell.HeightTexture;
			fwrite_be( haveHeightTexture, fout );

			if( haveHeightTexture )
			{
				SaveTexture( fout, cell.HeightTexture, false );

				fwrite_be( cell.Position.y, fout );
				fwrite_be( cell.YMax, fout );

				r3d_assert( cell.Entries.Count() < 256 );
				unsigned char count = (unsigned char)cell.Entries.Count();

				fwrite_be( count, fout );

				for( uint32_t i = 0, e = count; i < e; i ++ )
				{
					unsigned char idx = (unsigned char)cell.Entries[ i ].TypeIdx;
					fwrite_be( idx, fout );

					if( r3dTexture* tex = cell.Entries[i].MaskTexture )
					{
						char val = 1;
						fwrite_be( val, fout );
						SaveTexture( fout, tex, true );
					}
					else
					{
						char val = 0;
						fwrite_be( val, fout );
					}
				}
			}
			else
			{
				r3d_assert( !cell.Entries.Count() );
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------

bool ReadString( r3dString& oString, r3dFile* fin );

//------------------------------------------------------------------------

bool
GrassMap::LoadCellData( const r3dString& FilePath )
{
	r3dFile * fin = r3d_open( FilePath.c_str(), "rb" );

	if( !fin )
	{
		//r3dArtBug( "GrassMap::LoadCellData: Couldn't open grass cells file %s for reading!", FilePath.c_str() );
		return false;
	}

	struct AutoClose
	{
		~AutoClose()
		{
			fclose( file );
		}

		r3dFile *file;
	} autoClose = { fin }; (void)autoClose;

	char compSig[ sizeof GrazCellData_SIG101_2 ];

	if( fread( compSig, sizeof compSig, 1, fin ) != 1 )
		return false;

	if( !strcmp( GrazCellData_SIG102, compSig ) )
	{
		return LoadCellData_102( fin );
	}
	else
	{
		if( strcmp( GrazCellData_SIG101, compSig )!=0 )
		{
			if( strcmp( GrazCellData_SIG101_2, compSig )!=0 )
			{
				r3dArtBug( "GrassMap::LoadCellData: file %s has unsuppotrted version( current is %s but got %s )!\n", FilePath.c_str(), GrazCellData_SIG101, compSig );
				return false;
			}
		}
		else
		{
			fseek( fin, (int)sizeof GrazCellData_SIG101 - (int) sizeof GrazCellData_SIG101_2, SEEK_CUR );
		}

		return LoadCellData_101( fin );
	}

	return false;
}

//------------------------------------------------------------------------

bool
GrassMap::LoadCellData_101( r3dFile* fin )
{
	R3D_ENSURE_MAIN_THREAD();

	uint32_t count;

	if( fread_be( count, fin ) != 1 )
		return false;

	TArray< r3dString > cellTypes;

	for( uint32_t i = 0, e = count; i < e; i ++ )
	{
		r3dString type;

		if( !ReadString( type, fin ) )
			return false;

		cellTypes.PushBack( type );
	}

	uint32_t width, height;

	if( fread_be( width, fin ) != 1 )
		return false;

	if( fread_be( height, fin ) != 1 )
		return false;

	if (width < 1) width = 1;
	if (height < 1) height = 1;

	float fWidth = (width - 1) * g_pGrassLib->GetSettings().CellScale;
	float fHeight = (height - 1) * g_pGrassLib->GetSettings().CellScale;
	Init(fWidth, fHeight);

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			GrassCell& cell = mCells[ z ][ x ];

			cell.Position.x = x * mCellSize;
			cell.Position.z = z * mCellSize;

			if( fread_be( cell.Position.y, fin ) != 1 )
				return false;

			if( fread_be( cell.YMax, fin ) != 1 )
				return false;

			unsigned char count;

			if( fread_be( count, fin ) != 1 )
				return false;

			for( uint32_t i = 0, e = count; i < e; i ++ )
			{
				unsigned char typeIdx;
				if( fread_be( typeIdx, fin ) != 1 )
					return false;

				r3d_assert ( typeIdx < cellTypes.Count() );

				uint32_t realIdx;
				realIdx = g_pGrassLib->GetEntryIdxByName( cellTypes[ typeIdx ] );

				if( realIdx == uint32_t(-1) )
				{
					// this grass type is no longer here... conitnue
					continue;
				}

				GrassCellEntry gce;
				gce.TypeIdx = realIdx;

				cell.Entries.PushBack( gce );
			}
		}
	}

	mLoadVersion = 101;

	return true;	

}

//------------------------------------------------------------------------

bool
GrassMap::LoadCellData_102( r3dFile* fin )
{
	uint32_t count;

	if( fread_be( count, fin ) != 1 )
		return false;

	TArray< r3dString > cellTypes;

	for( uint32_t i = 0, e = count; i < e; i ++ )
	{
		r3dString type;

		if( !ReadString( type, fin ) )
			return false;

		cellTypes.PushBack( type );
	}

	int maskTexDim, heightTexDim;

	if( fread_be( heightTexDim, fin ) != 1 )
		return false;

	if( fread_be( maskTexDim, fin ) != 1 )
		return false;

	uint32_t width, height;

	if( fread_be( width, fin ) != 1 )
		return false;

	if( fread_be( height, fin ) != 1 )
		return false;

	if (width < 1) width = 1;
	if (height < 1) height = 1;

	float fWidth = (width - 1) * g_pGrassLib->GetSettings().CellScale;
	float fHeight = (height - 1) * g_pGrassLib->GetSettings().CellScale;
	Init(fWidth, fHeight);

	bool hasActiveCells = false;

	for( uint32_t z = 0, e = mCells.Height(); z < e; z ++ )
	{
		for( uint32_t x = 0, e = mCells.Width(); x < e; x ++ )
		{
			GrassCell& cell = mCells[ z ][ x ];

			char haveHeightTex;
			if( fread_be( haveHeightTex, fin ) != 1 )
				return false;

			if( haveHeightTex )
			{
				CreateCellHeightTexture( cell );

				hasActiveCells = true;

				Bytes heightTexData( heightTexDim * heightTexDim * HEIGHT_TEX_FMT_SIZE );

				if( fread( &heightTexData[0], heightTexDim * heightTexDim * HEIGHT_TEX_FMT_SIZE, 1, fin ) != 1 )
					return false;

				if( heightTexDim != CELL_HEIGHT_TEX_DIM )
				{
					UpdateCellHeight( x, z, cell );
				}
				else
				{
					FillTexture( cell.HeightTexture, heightTexData, false );
				}

				cell.Position.x = x * mCellSize;
				cell.Position.z = z * mCellSize;

				if( fread_be( cell.Position.y, fin ) != 1 )
					return false;

				if( fread_be( cell.YMax, fin ) != 1 )
					return false;

				unsigned char count;

				if( fread_be( count, fin ) != 1 )
					return false;

				for( uint32_t i = 0, e = count; i < e; i ++ )
				{
					unsigned char typeIdx;
					if( fread_be( typeIdx, fin ) != 1 )
						return false;

					r3d_assert ( typeIdx < cellTypes.Count() );

					uint32_t realIdx;
					realIdx = g_pGrassLib->GetEntryIdxByName( cellTypes[ typeIdx ] );

					char haveMaskTex;

					if( fread_be( haveMaskTex, fin ) != 1 )
						return false;

					GrassCellEntry gce;

					if( realIdx == uint32_t(-1) )
					{
						if( haveMaskTex )
						{
							fseek( fin, maskTexDim * maskTexDim * MASK_TEX_FMT_SIZE, SEEK_CUR );
						}

						// this grass type is no longer here... conitnue
						continue;
					}
					else
					{
						if( haveMaskTex )
						{
							Bytes maskTexData( maskTexDim * maskTexDim * MASK_TEX_FMT_SIZE );

							if( fread( &maskTexData[0], maskTexData.Count(), 1, fin ) != 1 )
								return false;
							if( maskTexDim != CELL_MASK_TEX_DIM )
							{
								Bytes newData( CELL_MASK_TEX_DIM * CELL_MASK_TEX_DIM * MASK_TEX_FMT_SIZE );
								ResizeTexture( maskTexDim, maskTexDim, 0.f, 0.f, maskTexData, CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, 0.f, 0.f, newData );

								maskTexData.Swap( newData );
							}

							gce.MaskTexture = r3dRenderer->AllocateTexture();
							gce.MaskTexture->Create( CELL_MASK_TEX_DIM, CELL_MASK_TEX_DIM, MASK_TEX_FMT, 1 );
							FillTexture( gce.MaskTexture, maskTexData, true );
						}
					}

					gce.TypeIdx = realIdx;
					cell.Entries.PushBack( gce );
				}
			}
		}
	}

	if( hasActiveCells && Terrain )
	{
		Terrain->SetOrthoDiffuseTextureDirty() ;
	}

	mLoadVersion = 102;

	return true;
}

//------------------------------------------------------------------------

void
GrassMap::CreateCellHeightTexture( GrassCell& cell )
{
	cell.HeightTexture = r3dRenderer->AllocateTexture();
	cell.HeightTexture->Create( CELL_HEIGHT_TEX_DIM, CELL_HEIGHT_TEX_DIM, D3DFMT_L8, 1 );
}

//------------------------------------------------------------------------

float
GrassMap::GetCellSize()
{
	if (mCellSize == 0)
		mCellSize = g_pGrassLib->GetSettings().CellScale;
	return mCellSize;
}

//------------------------------------------------------------------------

GrassMap gGrassMap;

//------------------------------------------------------------------------

r3dString GetGrassMapTexPath( const r3dString& LevelPath )
{
	r3dString res = LevelPath;
	res += AR_GRAZ_PAZ "\\";

	return res;
}

//------------------------------------------------------------------------

r3dString GetGrassMapHeightTexName( int CellX, int CellZ )
{
	char Name[ 128 ];
	sprintf( Name, "height_%d_%d.dds", CellX, CellZ );

	return Name;
}

//------------------------------------------------------------------------

r3dString GetGrassMapMaskTexName( const r3dString& Type, int CellX, int CellZ )
{
	char drive[ 8 ], folder[ 256 ], fname[ 64 ], ext[ 32 ];
	_splitpath( Type.c_str(), drive, folder, fname, ext );

	char Name[ 128 ];
	sprintf( Name, "mask_%s_%d_%d.dds", fname, CellX, CellZ );

	return Name;
}

//------------------------------------------------------------------------

void InitGrass()
{
	r3d_assert(g_pGrassMap == 0);
	g_pGrassMap = new GrassMap();
	r3d_assert(g_pGrassLib == 0);
	g_pGrassLib = new GrassLib();
	r3d_assert(g_pGrassGen == 0);
	g_pGrassGen = new GrassGen();

	D3DVERTEXELEMENT9 DeclElems[] = 
	{
		{0,  0, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0,  8, D3DDECLTYPE_SHORT2N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	( r3dDeviceTunnel::CreateVertexDeclaration( DeclElems, &GrassChunk::VDecl ) );

	if( r3dRenderer->SupportsVertexTextureFetch )
	{
		FULL_VERTEX_SHADER_ID	= r3dRenderer->GetVertexShaderIdx( "VS_FULL_GRASS" );
		MASKED_VERTEX_SHADER_ID = r3dRenderer->GetVertexShaderIdx( "VS_MASKED_GRASS" );
	}

	PIXEL_SHADER_ID					= r3dRenderer->GetPixelShaderIdx( "PS_GRASS" );	
	PIXEL_SHADER_NOCLIP_ID			= r3dRenderer->GetPixelShaderIdx( "PS_GRASS_NOCLIP" );
	
	P0_PIXEL_SHADER_ID		= r3dRenderer->GetPixelShaderIdx( "PS_GRASS_P0" );
	P1_PIXEL_SHADER_ID		= r3dRenderer->GetPixelShaderIdx( "PS_GRASS_P1" );

	g_pGrassGen->Load();

	if( g_pGrassGen->IsNewerThan( g_pGrassLib->GetStamp() ) )
	{
		r3dOutToLog( "Generating grass.. \n" );

		float timeStart = r3dGetTime();

		g_pGrassGen->GenerateAll();
		g_pGrassLib->Save();

		r3dOutToLog( "Generated grass for %f\n", r3dGetTime() - timeStart );

		g_pGrassLib->Unload() ;
	}
}

//------------------------------------------------------------------------

void LoadGrassLib()
{
	r3dOutToLog( "LoadGrassLib()...\n" );
	if( !g_pGrassLib->Load() )
	{
#ifndef FINAL_BUILD
		r3dOutToLog( "LoadGrassLib(): couldn't load grass lib. Regenerating." ) ;
		g_pGrassGen->GenerateAll() ;
		g_pGrassLib->Save() ;
#endif
	}
}

//------------------------------------------------------------------------

void UnloadGrassLib()
{
	g_pGrassLib->Unload();
}

//------------------------------------------------------------------------

void AnimateGrass()
{
	clock_t newClock = clock();

	clock_t delta = newClock - PrevClock;

	PrevClock = newClock;

	Time += r_grass_anim_speed->GetFloat() * delta / CLOCKS_PER_SEC;

	Time = fmodf( Time, R3D_PI * 2 );
}

//------------------------------------------------------------------------

void PrepareGrass()
{
	if( r_grass_draw->GetBool() && r3dRenderer->SupportsVertexTextureFetch )
	{
		g_pGrassMap->Prepare() ;
	}
}

//------------------------------------------------------------------------

void DrawGrass( GrassMap::Path path, bool UseDepthEqual )
{
	if( r_grass_draw->GetBool() && r3dRenderer->SupportsVertexTextureFetch )
	{
		g_pGrassMap->Draw( gCam, path, UseDepthEqual );
	}
}

//------------------------------------------------------------------------

void CloseGrass()
{
	r3d_assert( PIXEL_SHADER_ID >= 0 );

	FULL_VERTEX_SHADER_ID	= -1;
	MASKED_VERTEX_SHADER_ID	= -1;
	PIXEL_SHADER_ID			= -1;

	SAFE_DELETE(g_pGrassGen);
	SAFE_DELETE(g_pGrassLib);
	SAFE_DELETE(g_pGrassMap);
}

//------------------------------------------------------------------------

float GetGrassHeight(const r3dPoint3D &pt)
{
	float th = -FLT_MAX;
	float ph = -FLT_MAX;
	if (Terrain)
		th = Terrain->GetHeight ( pt );
#ifndef FINAL_BUILD
	if (g_GrassPlanesManager)
		ph = g_GrassPlanesManager->GetHeight(pt.x, pt.z);
#endif
	return std::max(th, ph);
}

//------------------------------------------------------------------------

namespace
{
	float lerp( float a, float b, float t )
	{
		return a + ( b - a ) * t;
	}

	float GetBiliniear( const Bytes& bytes, uint32_t totalXLen, uint32_t totalZLen, float x, float z )
	{

		x = R3D_MIN( R3D_MAX( x, 0.f ), (float) totalXLen - 1.f );
		z = R3D_MIN( R3D_MAX( z, 0.f ), (float) totalZLen - 1.f );

		int x0 = (int)x;
		int z0 = (int)z;

		int x1 = R3D_MIN( x0 + 1, (int)totalXLen - 1 );
		int z1 = R3D_MIN( z0 + 1, (int)totalZLen - 1 );

		float s00 = (float)bytes[ x0 + z0 * totalXLen ];
		float s10 = (float)bytes[ x1 + z0 * totalXLen ];
		float s01 = (float)bytes[ x0 + z1 * totalXLen ];
		float s11 = (float)bytes[ x1 + z1 * totalXLen ];

		float tx = x - (float)x0;
		float tz = z - (float)z0;

		return 		lerp( 
						lerp( s00, s10, tx ),
							lerp( s01, s11, tx ),
								tz
									);
	}

	void ResizeTexture(	uint32_t oldTotalXLen, uint32_t oldTotalZLen, float oldExcessX, float oldExcessZ, const Bytes& oldCompoundTex,
								uint32_t newTotalXLen, uint32_t newTotalZLen, float newExcessX, float newExcessZ, Bytes& newCompoundTex )
	{
		uint32_t oldEffectiveXLen = oldTotalXLen - uint32_t( oldExcessX * GrassMap::CELL_MASK_TEX_DIM );
		uint32_t oldEffectiveZLen = oldTotalZLen - uint32_t( oldExcessZ * GrassMap::CELL_MASK_TEX_DIM );

		uint32_t newEffectiveXLen = newTotalXLen - uint32_t( newExcessX * GrassMap::CELL_MASK_TEX_DIM );
		uint32_t newEffectiveZLen = newTotalZLen - uint32_t( newExcessZ * GrassMap::CELL_MASK_TEX_DIM );

		float kx = (float)oldEffectiveXLen / newEffectiveXLen;
		float kz = (float)oldEffectiveZLen / newEffectiveZLen;

		for( uint32_t x = 0, e = newEffectiveXLen; x < e; x ++ )
		{
			for( uint32_t z = 0, e = newEffectiveZLen; z < e; z ++ )
			{
				float ox0 = x * kx;
				float oz0 = z * kz;

				float ox1 = ( x + 1 ) * kx;
				float oz1 = ( z + 1 ) * kz;

				float sum = 0;

				float count = 0;

				for( float fx = ox0; fx < ox1; fx += 0.5f )
				{
					for( float fz = oz0; fz < oz1; fz += 0.5f )
					{
						sum		+= GetBiliniear( oldCompoundTex, oldTotalXLen, oldTotalZLen, fx, fz );
						count	+= 1.0f;
					}
				}

				sum /= count;
				newCompoundTex[ x + z * newTotalXLen ] = R3D_MIN( R3D_MAX( (int)sum, 0 ), (int)255 );
			}
		}
	}

	void SaveTexture( FILE* fout, r3dTexture* tex, bool isMask )
	{
		UINT fmtSize = isMask ? MASK_TEX_FMT_SIZE : HEIGHT_TEX_FMT_SIZE;

		D3DLOCKED_RECT lrect;

		D3D_V( tex->AsTex2D()->LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) );

		char* p = (char*)lrect.pBits;

		for( UINT i = 0, e = tex->GetHeight(); i < e; i ++ )
		{
			fwrite( p, tex->GetWidth() * fmtSize, 1, fout );
			p += lrect.Pitch;
		}

		D3D_V( tex->AsTex2D()->UnlockRect( 0 ) );
	}

	void FillTexture( r3dTexture* tex, const Bytes& data, bool isMask )
	{
		r3d_assert( data.Count() == tex->GetWidth() * tex->GetHeight() );

		r3dD3DTextureTunnel d3dtex = tex->GetD3DTunnel();

		D3DLOCKED_RECT lrect;
		d3dtex.LockRect( 0, &lrect, NULL, 0 );

		unsigned char* p = (unsigned char*)lrect.pBits;

		int idx = 0;

		int fmtSize = isMask ? MASK_TEX_FMT_SIZE : HEIGHT_TEX_FMT_SIZE;

		for( uint32_t z = 0, e = tex->GetHeight(); z < e; z ++ )
		{
			for( uint32_t x = 0, e = tex->GetWidth() * fmtSize; x < e; x ++ )
			{
				*p++ = data[ idx ++ ];
			}

			p += lrect.Pitch - tex->GetWidth() * fmtSize;
		}

		d3dtex.UnlockRect( 0 );

	}
}

//------------------------------------------------------------------------


