#include "r3dPCH.h"

#pragma warning( disable : 4244 )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4800 )
#pragma warning( disable : 4996 )

#include "r3d.h"
#include "r3dBuffer.h"
#include "r3dBudgeter.h"

#include "GameCommon.h"
#include "rendering/Deffered/DeferredHelpers.h"
#include "rendering/Deffered/CommonPostFX.h"

#include "../SF/Console/Config.h"
#include "../SF/RenderBuffer.h"

#include "ObjectsCode/world/MaterialTypes.h"

#include "r3dBitMaskArray.h"

#include "Terrain.h"
#include "GameLevel.h"

#include "DebugHelpers.h"

#include "sun.h"

#include "..\..\bin\Data\Shaders\DX9_P1\system\LibSM\shadow_config.h" // shader config file

#include "JobChief.h"
#include "HeightNormalVert.h"

#include "r3dDeviceQueue.h"
#if VEHICLES_ENABLED
#include "PhysX/PhysXAPI/vehicle/PxVehicleUtils.h"
#endif

extern int __r3dDisplayMipLevels;
#ifndef FINAL_BUILD
void SetTextureAsMipmapTest(r3dTexture* tex, int stageID);
void SetTextureDensityChecker(int stageID);
#endif
void AdvanceLoadingProgress( float );

extern r3dSun		*Sun;

#define CellGridSize 64
#define CellGridSizePS3 64
int Mod = 1;

#define N_OVERTILES 2

r3dITerrain	*Terrain = NULL;
r3dTerrain	*Terrain1 = NULL;

int __TerraModulateFlag = D3DTOP_MODULATE2X;
int g_bTerrainUseLightHack = 0;

int __terra_UseHackedWorld = 0;
D3DXMATRIX *__terra_HackedWorldMatrix;

int g_TerraVS_ID = -1 ;
int g_TerraLQ_VS_ID = -1 ;

int g_Terra_SM_VS_ID = -1 ;
int g_Terra_SM_LQ_VS_ID = -1 ;

static bool gNeedShaders = true;

extern bool g_bEditMode;

void PrintSplatLocationPS3( char (& buf ) [ 1024 ], const char * szSourceFile );


#define FNAME_TERRAIN_INI			"%s/terrain.ini"			// need move to *.h file
#define FNAME_TERRAIN_HEIGHTMAP		"%s/terrain.heightmap"		// need move to *.h file

#define FNAME_TERRAIN_DATA_PS3		"%s/terrain_data_ps3.bin"	// need move to *.h file

#define TERRAIN_SIGNATURE			'RRET'
#define TERRAIN_VERSION				5
#define TERRAIN_VERSION_PS3			13

void ReadTerrainHeader(r3dFile* file, uint32_t& dwSignature, uint32_t& dwVersion)
{
	fread( &dwSignature, sizeof( dwSignature ), 1, file );
	if ( dwSignature == TERRAIN_SIGNATURE )
	{
		fread( &dwVersion, sizeof( dwVersion ), 1, file );		
	}
	else
	{
		fseek( file, 0, SEEK_SET );
		r3dArtBug( "!r3dTerrain::InitData : Old version terrain data file" );
	}
}

#define FNAME_UV_CUBE				"Data/TerrainData/uv_cube.dds"
#define REMAP_OFFSET(x,z,tcx) ( (int)((x)/N_OVERTILES)*N_OVERTILES*N_OVERTILES + (x)%N_OVERTILES + ((z)%N_OVERTILES)*N_OVERTILES + tcx * ( (int)((z)/N_OVERTILES)*N_OVERTILES ) )

int RemapOffsetPC( int x, int z, int tcx )
{
	return REMAP_OFFSET(x, z, tcx);
}

int RemapOffsetPS3( int x, int z, int tcx )
{
	return x + z * tcx;
}


#define LAYER_VCONST_OFFSET			21

#define PX_V(arg) if( !(arg) ) { r3dError( "PhysX Call " #arg " failed!" ); }

//--------------------------------------------------------------------------------------------------------
template <class T> 
inline T LerpFast ( T from, T to, float weight )
{
	return from + weight * ( to - from );
}

//////////////////////////////////////////////////////////////////////////

ErosionPattern::ErosionPattern()
: w(0)
, h(0)
, data(0)
{

}

//------------------------------------------------------------------------
/*explicit */

ErosionPattern::ErosionPattern(const char *ddsFileName)
{
	r3dFile *f = r3d_open(ddsFileName);
	if (!f)
		return;

	int size = f->size - 128;
	if (size <= 0)
		return;

	fseek(f, 12, SEEK_SET);
	fread(&h, sizeof(h), 1, f);
	fread(&w, sizeof(w), 1, f);

	r3d_assert(w * h <= size);
	if (w * h > size)
		return;
	size = w * h;

	data = new byte[size];
	fseek(f, 128, SEEK_SET);
	fread(data, size, 1, f);
	fclose(f);
}

//------------------------------------------------------------------------

ErosionPattern::~ErosionPattern()
{
	delete [] data;
}

//------------------------------------------------------------------------

float ErosionPattern::Sample(float u, float v)
{
	u = r3dTL::Clamp(u, 0.0f, 1.0f);
	v = r3dTL::Clamp(v, 0.0f, 1.0f);

	int x = static_cast<int>(u * w);
	int y = static_cast<int>(v * h);

	int idx = y * w + x;
	return data[idx] / 255.0f;
}

//------------------------------------------------------------------------

float ErosionPattern::SampleBilinear(float u, float v)
{
	float uPixelSize = 1.0f / w;
	float vPixelSize = 1.0f / h;

	float s0 = Sample(u, v);
	float s1 = Sample(u + uPixelSize, v);
	float s2 = Sample(u, v + vPixelSize);
	float s3 = Sample(u + uPixelSize, v + vPixelSize);

	float dummy = 0;
	float subpixU = modf(u * w, &dummy);
	float subpixV = modf(v * h, &dummy);

	float h0 = LerpFast(s0, s1, subpixU);
	float h1 = LerpFast(s2, s3, subpixU);
	float h = LerpFast(h0, h1, subpixV);

	return h;
}

ErosionPattern *gErosionPattern = 0;
void InitErosionPattern(const char *filePath)
{
	delete gErosionPattern;
	gErosionPattern = new ErosionPattern(filePath);
}

void FreeErosionPattern()
{
	delete gErosionPattern;
	gErosionPattern = 0;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	typedef r3dTL::TArray< unsigned char > Bytes;
	typedef r3dTL::TArray< float > Floats;
}

#pragma pack(push)
#pragma pack(1)
struct PackedTerrainVert_t
{
	uint16_t Height;
	uint16_t Normal;
	uint16_t Color;
	uint16_t Delta;
	uint16_t MorphNormal;
};

struct PackedPS3Vert_t
{
	uint16_t Normal;
	uint16_t Height;
	uint16_t Color;
};


#pragma pack(pop)

inline static void ChangeEndianess( uint16_t& val )
{
	val = (val >> 8) + (val << 8);
}

template< typename T >
inline static void ChangeEndianess( T& val )
{
	char* byte = (char*)&val;
	const size_t sz = sizeof val;

	for( size_t i = 0, e = sz / 2; i < e; i ++ )
	{
		std::swap( byte[ i ], byte[ sz - i - 1 ] );
	}	
}

inline static void ChangeEndianess( PackedTerrainVert_t& v )
{
	ChangeEndianess( v.Height		);
	ChangeEndianess( v.Normal		);
	ChangeEndianess( v.Color		);
	ChangeEndianess( v.Delta		);
	ChangeEndianess( v.MorphNormal	);
}

inline static void ToPackedVert( PackedTerrainVert_t& o, const HeightNormalVert& i, float minH, float maxH )
{
	// height
	float h = ( i.Height() - minH ) / ( maxH - minH )* 2.f - 1;
	o.Height = ( int16_t)( h * 32767.f );

	// color
	uint32_t R = ( (i.Color() & 0xff0000) >> 16 );
	uint32_t G = ( (i.Color() & 0x00ff00) >> 8 );
	uint32_t B = ( (i.Color() & 0x0000ff) >> 0 );

	o.Color = ( ( (int16_t)( R * 0x1f / 255 ) << 11 ) | ( (int16_t)( G * 0x3f / 255 ) << 5 ) | (int16_t)( B * 0x1f / 255 ) );

	// normal
	o.Normal = PackNorm(i.Normal());

	float half_range = 0.5f * ( maxH - minH );
	o.Delta = (int16_t)( i.Delta() / half_range * 32767.f );

	// normal morph
	o.MorphNormal = i.MorphNormal();

	ChangeEndianess( o );
}

inline static void ToPackedPS3Vert( PackedPS3Vert_t& o, const HeightNormalVert& i, float minH, float maxH )
{
	PackedTerrainVert_t v;
	ToPackedVert( v, i, minH, maxH );

	o.Normal		= v.Normal;
	o.Height		= v.Height;
	ChangeEndianess( v.Color );
	v.Color -= 32768;
	ChangeEndianess( v.Color );
	o.Color			= v.Color;
}

inline static uint32_t GetTotalVertexCount( int tilesX, int tilesZ, int cellGridDim )
{
	return tilesX * tilesZ * ( cellGridDim + 1 ) * ( cellGridDim + 1 );
}

inline static void FromPackedVert( HeightNormalVert& o, const PackedTerrainVert_t& ie, float minH, float maxH )
{
	PackedTerrainVert_t i = ie;
	ChangeEndianess( i );

	// height

	o.SetHeight(static_cast<int16_t>(i.Height));

	// color
	o.SetColor(0xff000000 | 
		( i.Color >> 11 ) * 255 / 0x1f << 16 | 
		( i.Color >> 5 & 0x3f ) * 255 / 0x3f << 8 | 
		( i.Color & 0x1f ) * 255 / 0x1f << 0);

	// normal
	o.SetNormal(i.Normal);

	// height morph
	float half_range = 0.5f * ( maxH - minH );
	o.SetDelta((int16_t)i.Delta * half_range / 32767.f);

	// normal morph
	o.SetMorphNormal(i.MorphNormal);
}

struct MorphWeightVert_t
{
//private:
	float		fWeight;

public:
	float Weight() const
	{
		return fWeight;
	}

	void SetWeight(float w)
	{
		fWeight = w;
	}
};

struct TerrainPosVert_t
{
	r3dPoint3D vPos;
};

enum
{
	NORTH_CONNECTION	= 1,
	EAST_CONNECTION		= 2,
	SOUTH_CONNECTION	= 4,
	WEST_CONNECTION		= 8,
};

namespace
{

	uint32_t GetHeightFieldDataSize( const PxHeightFieldDesc& desc )
	{
		return desc.nbColumns * desc.nbRows * desc.samples.stride;
	}

	int dLodSteps_PC[ LODDESC_COUNT ] = { 1, 2, 4, CellGridSize / 2 };
	int dLodSteps_PC_LOW[ LODDESC_COUNT ] = { 1, 4, 8, CellGridSize / 2 };
	int dLodSteps_PS3[ LODDESC_COUNT ] = { 1, 2, 4, CellGridSizePS3 };

	static int CountConnectionIndices( int step, int prevStep, int sideLodConnections )
	{
		int vertDim = CellGridSize / step;

		int total = ( vertDim - 2 ) * ( vertDim - 2 ) * 2;

		r3d_assert( ! ( step % prevStep ) );

		int ratio = step / prevStep;

		int connectCount = ratio * 2 + ( ratio + 1 ) * ( vertDim - 2 );
		int looseCount = ( vertDim - 2 ) * 2 + 2;

		if( sideLodConnections & NORTH_CONNECTION )	total += connectCount;
		else total += looseCount; 

		if( sideLodConnections & EAST_CONNECTION )	total += connectCount;
		else total += looseCount;

		if( sideLodConnections & SOUTH_CONNECTION )	total += connectCount;
		else total += looseCount;

		if( sideLodConnections & WEST_CONNECTION )	total += connectCount;
		else total += looseCount;

		return total * 3;

	}

	void ConstructLodDescs( TerrainLodDesc (&dLods)[ LODDESC_COUNT ], int (&dSteps)[ LODDESC_COUNT ], int cellGridSize )
	{
		int nIndexOffset = 0;

		for( int i = 1; i < TERRA_CONNECTION_TYPE_COUNT; i ++ )
		{
			dLods[ 0 ].nTriCount[ i ] = 0;
			dLods[ 0 ].nIndexOffset[ i ] = 0;
		}

		for ( int k = 0; k < LODDESC_COUNT; k++ )
		{
			dLods[ k ].nStep = dSteps[ k ];
			int nCount = CellGridSize / dSteps[ k ];
			dLods[ k ].nTriCount[ 0 ]		= nCount * nCount * 2;
			dLods[ k ].nIndexOffset[ 0 ]	= nIndexOffset;

			nIndexOffset += dLods[ k ].nTriCount[ 0 ] * 3 ;

			if( k )
			{
				for( int i = 1; i < TERRA_CONNECTION_TYPE_COUNT; i ++ )
				{
					dLods[ k ].nTriCount[ i ]		= CountConnectionIndices( dLods[ k ].nStep, dLods[ k - 1 ].nStep, i ) / 3;
					dLods[ k ].nIndexOffset[ i ]	= nIndexOffset;

					nIndexOffset += dLods[ k ].nTriCount[ i ] * 3 ;
				}
			}
		}
	}

}

//------------------------------------------------------------------------

UpdateVertexDataSettings::UpdateVertexDataSettings()
{
	TileCountX			= 0;
	TileCountZ			= 0;

	UpdateTileXStart	= 0;
	UpdateTileXEnd		= 0;

	UpdateTileZStart	= 0;
	UpdateTileZEnd		= 0;

	CellGridDim			= 0;

	Tiles				= NULL;
	dLods				= NULL;

	RemapOffset			= NULL;

	LockedBuffer		= NULL;
	LockOffset			= 0;
}

//--------------------------------------------------------------------------------------------------------

namespace
{
	enum
	{
		MAX_SHADER_LAYERS = 4
	};

	union TerrainShaderKey
	{
		uint16_t Value;

		struct SFlags
		{
			uint16_t num_layers	: 2;
			uint16_t simple 	: 1;
			uint16_t split0 	: 1;
			uint16_t split1 	: 1;
			uint16_t split2		: 1;
			uint16_t split3		: 1;
			uint16_t multi		: 1;
			uint16_t aux_enabled: 1;
			uint16_t lq			: 1;

		} Flags;
	};

	// NOTE : less are actually used, cause split variations
	// decrease when num_layers decreases + no splits at all in LQ version
	typedef r3dTL::TFixedArray< int, 1024 > TerrainShadersArr;

	TerrainShadersArr TerrainShaders;

	typedef r3dTL::TFixedArray< int, MAX_SHADER_LAYERS > TerrainMinimapShaderArr;

	TerrainMinimapShaderArr TerrainMinimapShaders; 

	const char* NumLayersToString( int NumLayers )
	{
		switch( NumLayers )
		{
		case 0:
			return "1";
		case 1:
			return "2";
		case 2:
			return "3";
		case 3:
			return "4";
		default:
			r3dError( "Catastrophic error..." );
			return NULL;
			break;
		}
	}

	void FillShaderMacros( TerrainShaderKey Key, ShaderMacros& oMacros )
	{
		oMacros.Resize( 9 );

		oMacros[ 0 ].Name		= "NUM_LAYERS";
		oMacros[ 0 ].Definition	= NumLayersToString( Key.Flags.num_layers );

		oMacros[ 1 ].Name		= "RENDER_SIMPLE_TERRAIN";
		oMacros[ 1 ].Definition	= Key.Flags.simple ? "1" : "0";

		oMacros[ 2 ].Name		= "SPLIT_0";
		oMacros[ 2 ].Definition	= Key.Flags.split0 ? "1" : "0";

		oMacros[ 3 ].Name		= "SPLIT_1";
		oMacros[ 3 ].Definition	= Key.Flags.split1 ? "1" : "0";

		oMacros[ 4 ].Name		= "SPLIT_2";
		oMacros[ 4 ].Definition	= Key.Flags.split2 ? "1" : "0";

		oMacros[ 5 ].Name		= "SPLIT_3";
		oMacros[ 5 ].Definition	= Key.Flags.split3 ? "1" : "0";

		oMacros[ 6 ].Name		= "MULTI";
		oMacros[ 6 ].Definition	= Key.Flags.multi ? "1" : "0";

		oMacros[ 7 ].Name		= "AUX_ENABLED";
		oMacros[ 7 ].Definition	= Key.Flags.aux_enabled ? "1" : "0";

		oMacros[ 8 ].Name		= "LQ";
		oMacros[ 8 ].Definition	= Key.Flags.lq ? "1" : "0";

	}

	uint32_t GetIndexCountPerTile( uint32_t nCellGridSize, uint32_t Step )
	{
		return nCellGridSize * nCellGridSize * 3 * 2 / Step / Step;
	}

	R3D_FORCEINLINE void SetMP2VertexShader()
	{
		r3dRenderer->SetVertexShader( r_terrain_quality->GetInt() == 1 ? g_TerraLQ_VS_ID : g_TerraVS_ID ) ;		
	}
}

enum
{
	X_ORIENT,
	Y_ORIENT,
	Z_ORIENT,
	MULT_ORIENT,
	MIXED
};


r3dTerrain::r3dTerrain()
: NumMats( 0 )
, m_HeightmapSize( 1.f )
, m_NormalmapScale( 1.f )
, m_eTilingMode( eTilingMode_None )
, m_fShaderLODFadeStart ( 2000.0f )
, m_fShaderLODFadeEnd ( 3000.0f )
, m_HeightNormalVB( NULL )
, m_MorphMaskVB( NULL )
, m_VB( NULL )
, m_IB( NULL )
, m_HeightNormalVDecl( NULL )
, m_HeightNormalVDeclLQ( NULL )
, m_nTileCountX( 0 )
, m_nTileCountZ( 0 )
, m_nVertPerTile( 0 )
#ifndef FINAL_BUILD
, m_pUndoItem( NULL )
#endif
, m_pColorData( NULL )
, NormalMapData( NULL )
, bNeedUpdatePhysics ( false )
, bInHeightEditMode ( false )
, CellSize( 1.0f )
, m_HFScale( 1.f )
, m_InvHFScale( 1.f )
, Width ( 1024 )
, Height ( 1024 )
, NormalPShaderIDX( -1 )
, SplitPShaderIDX( -1 )
, SelectedTileX( -1 )
, SelectedTileZ( -1 )
, HighlightedTileX( -1 )
, HighlightedTileZ( -1 )
, m_dOvertileOffset( 0 )
, LastQLInit( 0 )
{
	physicsTerrain = 0;
	physicsHeightField = 0;
	bLoaded   = 0;
	Terrain1   = this;
	SplitDistance	= 1000.0f;

	m_szName[ 0 ] = '\0';

	SetCellSize( CellSize );


	int nIndexOffset = 0;
	int dSteps[ LODDESC_COUNT ] = { 1, 2, 4, CellGridSize / 2 };

	ConstructLodDescs( m_dLods, dLodSteps_PC, CellGridSize );

	for( int i = 0, e = TERRAIN_MAT_COUNT; i < e; i ++ )
	{
		MatSplatTex[ i ] = NULL;
		EditorMatSplatTex[ i ] = NULL;
	}

	__TerraLOD1	= 1;
	__TerraLOD2	= 2;
	__TerraLOD3	= 24;

	BaseTerraLOD1 = 1;
	BaseTerraLOD2 = 2;
	BaseTerraLOD3 = 24;

	MaterialDataWidth = 0 ;
	MaterialDataHeight = 0 ;

	memset( MatTypeIdxes, 0, sizeof MatTypeIdxes ) ;

#ifndef WO_SERVER
	CHeightChanged::Register();
	CLayerMaskPaint::Register();
	CLayerColorPaint::Register();
#endif
}

r3dTerrain::~r3dTerrain()
{
	if(physicsTerrain)
	{
		physicsTerrain->release();
		physicsHeightField->release();
		physicsTerrain		= NULL;
		physicsHeightField	= NULL;
	}
	if( Terrain == Terrain1 )
	{
		Terrain = NULL ;
	}
	Terrain1   = NULL ;

	Unload();
}


//--------------------------------------------------------------------------------------------------------
int r3dTerrain::GetCellGridSize() const
{
	return CellGridSize;
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::UpdateAllVertexData()
{
	UpdateVertexDataSettings sts;

	PrepareUpdateVertexDataSettings( sts, 0, m_nTileCountX - 1, 0, m_nTileCountZ - 1 );

	bool res = UpdateVertexData( sts );

	m_HeightNormalVB->Unlock();

	return res;
}

//------------------------------------------------------------------------

void
r3dTerrain::PrepareUpdateVertexDataSettings( UpdateVertexDataSettings& oSts, int xstart, int xend, int zstart, int zend )
{
	oSts.TileCountX		= m_nTileCountX;
	oSts.TileCountZ		= m_nTileCountZ;

	oSts.UpdateTileXStart	= xstart;
	oSts.UpdateTileXEnd		= xend;

	oSts.UpdateTileZStart	= zstart;
	oSts.UpdateTileZEnd		= zend;

	oSts.CellGridDim			= CellGridSize;

	oSts.Tiles				= &m_dTiles;
	oSts.dLods				= &m_dLods;

	oSts.RemapOffset			= RemapOffsetPC;

	int nVLockOffset	= REMAP_OFFSET(xstart,zstart,m_nTileCountX)*m_nVertPerTile;
	int nVLockCount		= (REMAP_OFFSET(xend, zend,m_nTileCountX) + 1)*m_nVertPerTile - nVLockOffset;

	oSts.LockedBuffer	= m_HeightNormalVB->Lock( nVLockOffset, nVLockCount ); // fixme
	oSts.LockOffset		= nVLockOffset;

}

//------------------------------------------------------------------------

bool
r3dTerrain::UpdateVertexData( const RECT& rc )
{
	R3D_ENSURE_MAIN_THREAD();

	UpdateVertexDataSettings sts;

	PrepareUpdateVertexDataSettings( sts, rc.left, rc.right, rc.bottom, rc.top );

	bool res = UpdateVertexData( sts );

	m_HeightNormalVB->Unlock();

	return res;
}

//--------------------------------------------------------------------------------------------------------

static r3dTL::TArray< r3dPoint3D > NormalBuf;

template < typename V >
bool r3dTerrain::DoUpdateVertexData( const UpdateVertexDataSettings& sts )
{
#ifdef WO_SERVER
	return true;
#endif
	r3dOutToLog("TERRAIN: UpdateVertexData\n");
	r3d_assert( sts.UpdateTileXStart < sts.TileCountX && sts.UpdateTileXEnd < sts.TileCountX );
	r3d_assert( sts.UpdateTileZStart < sts.TileCountZ && sts.UpdateTileZEnd < sts.TileCountZ );

	int iwidth = (int)Width;

	int VertPerTile = ( sts.CellGridDim + 1 ) * ( sts.CellGridDim + 1 );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	>	calc tile bounds (height)
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	for ( int nCellZ = sts.UpdateTileZStart; nCellZ <= sts.UpdateTileZEnd; nCellZ++ )
	{
		int nMinZ = nCellZ * sts.CellGridDim  - 1;
		int nMaxZ = ( nCellZ + 1 ) * sts.CellGridDim  + 1;

		nMinZ = r3dTL::Max( 0, nMinZ );
		nMaxZ = r3dTL::Min( (int)Height - 1, nMaxZ );

		for ( int nCellX = sts.UpdateTileXStart; nCellX <= sts.UpdateTileXEnd; nCellX++ )
		{
			int nTile = nCellZ * sts.TileCountX + nCellX;
			TerrainTile_c &tile = (*sts.Tiles)[ nTile ];

			tile.m_HeightMin = FLT_MAX;
			tile.m_HeightMax = -FLT_MAX;

			int nMinX = nCellX * sts.CellGridDim - 1;
			int nMaxX = ( nCellX + 1 ) * sts.CellGridDim + 1;

			nMinX = r3dTL::Max( 0, nMinX );
			nMaxX = r3dTL::Min( (int)Width - 1, nMaxX );

			for ( int z = nMinZ; z <= nMaxZ; z++ )
			{
				for ( int x = nMinX; x <= nMaxX; x++ )
				{
					float fHeight = HeightFieldData[ x + z * iwidth ];

					m_MaxHeight = R3D_MAX(m_MaxHeight, fHeight);
					m_MinHeight = R3D_MIN(m_MinHeight, fHeight);

					if ( tile.m_HeightMin > fHeight )
						tile.m_HeightMin = fHeight;

					if ( tile.m_HeightMax < fHeight )
						tile.m_HeightMax = fHeight;
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	>	Fill buffer height & normals (need use region)
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	V * pV;
	V * pVLock;

	int nTotalVert = sts.TileCountX * sts.TileCountZ * VertPerTile;

	pVLock				= (V *)sts.LockedBuffer;
	int nVLockOffset	= sts.LockOffset;
	int nVLockCount		= (sts.RemapOffset(sts.UpdateTileXEnd, sts.UpdateTileZEnd, sts.TileCountX ) + 1)*VertPerTile - nVLockOffset;

	pV = pVLock;

	// tile pass

	for ( int	z = sts.UpdateTileZStart * sts.CellGridDim,
		nMaxZ = ( sts.UpdateTileZEnd + 1 ) * sts.CellGridDim + 1;
		z < nMaxZ; z++ )
	{
		for (	int	x = sts.UpdateTileXStart * sts.CellGridDim, 
			nMaxX = ( sts.UpdateTileXEnd + 1 ) * sts.CellGridDim + 1; 
			x < nMaxX; x++ )
		{

			float xx = R3D_MIN( (float)x, Width - 1 );
			float zz = R3D_MIN( (float)z, Height - 1 );
			float sx = GetEditHeight( xx<Width-1 ? xx+1 : xx, zz) - GetEditHeight(xx>0 ? xx-1 : xx, zz);
			if (xx == 0 || xx == Width-1)
				sx *= 2;

			float sy = GetEditHeight( xx, zz<Height-1 ? zz+1 : zz) - GetEditHeight(xx, zz>0 ?  zz-1 : zz);
			if (zz == 0 || zz == Height -1)
				sy *= 2;

			r3dPoint3D vN(-sx, /*2**/CellSize, -sy);
			vN.Normalize();

			NormalMapData[ (uint32_t)(zz * Width + xx) ] = vN;
		}
	}


	// do separable normal 'blur'
	for( int i = 0; i < 4; i ++ )
	{
		NormalBuf.Resize( (uint32_t)Width * (uint32_t)Height );

		r3dPoint3D* source;
		r3dPoint3D* target;

		int dirx;
		int dirz;

		if( i & 1 )
		{
			source = &NormalBuf[ 0 ];
			target = NormalMapData;

			dirx = 1;
			dirz = 0;
		}
		else
		{
			source = NormalMapData;
			target = &NormalBuf[ 0 ];

			dirx = 0;
			dirz = 1;
		}

		int iwidth = (int)Width;
		int iheight = (int)Height;

		for ( int	z = sts.UpdateTileZStart * sts.CellGridDim,
			nMaxZ = ( sts.UpdateTileZEnd + 1 ) * sts.CellGridDim;
			z < nMaxZ; z++ )
		{
			for (	int	x = sts.UpdateTileXStart * sts.CellGridDim, 
				nMaxX = ( sts.UpdateTileXEnd + 1 ) * sts.CellGridDim; 
				x < nMaxX; x++ )
			{

				int xc = x, zc = z;

				int x0	= R3D_MAX( xc - dirx, 0 ), 
					x1	= R3D_MIN( xc + dirx, iwidth - 1 ),
					z0	= R3D_MAX( zc - dirz, 0 ), 
					z1	= R3D_MIN( zc + dirz, iheight - 1 );


				r3dPoint3D	vN =	source[ zc * iwidth + xc ];
				vN +=	source[ z0 * iwidth + x0 ];
				vN +=	source[ z1 * iwidth + x1 ];

				vN.Normalize();

				target[ zc * iwidth + xc ] = vN;
			}
		}
	}

	for ( int nCellZ = sts.UpdateTileZStart; nCellZ <= sts.UpdateTileZEnd; nCellZ++ )
	{
		for ( int nCellX = sts.UpdateTileXStart; nCellX <= sts.UpdateTileXEnd; nCellX++ )
		{
			int nMinX = nCellX * sts.CellGridDim;
			int nMinZ = nCellZ * sts.CellGridDim;

			int nMaxX = nMinX + ( sts.CellGridDim + 1 );
			int nMaxZ = nMinZ + ( sts.CellGridDim + 1 );

			int nOffset = sts.RemapOffset(nCellX, nCellZ, sts.TileCountX)*VertPerTile - nVLockOffset;
			r3d_assert ( nOffset >= 0 );
			pV = pVLock + nOffset;

			for ( int z = nMinZ; z < nMaxZ; z++ )
			{
				for ( int x = nMinX; x < nMaxX; x++, pV++ )
				{
					float xx = R3D_MIN( (float)x, Width - 1 );
					float zz = R3D_MIN( (float)z, Height - 1 );

					float h = GetEditHeight( xx, zz );

					(*pV).SetHeight(h);
					(*pV).SetDelta(0.f);
					(*pV).SetMorphNormal(PackNorm(0xff80ff80));

					(*pV).SetNormal(PackNorm(NormalMapData[ (uint32_t)(zz * Width + xx) ].GetPacked()));

					(*pV).SetColor(GetEditColor( xx, zz ));
				}		
			}
		}
	}

	float delta = 0.05f / m_nTileCountZ;

	for ( int nCellZ = sts.UpdateTileZStart; nCellZ <= sts.UpdateTileZEnd; nCellZ++ )
	{
		for ( int nCellX = sts.UpdateTileXStart; nCellX <= sts.UpdateTileXEnd; nCellX++ )
		{
			int nMinX = nCellX * sts.CellGridDim;
			int nMinZ = nCellZ * sts.CellGridDim;

			int nMaxX = nMinX + ( sts.CellGridDim + 1 );
			int nMaxZ = nMinZ + ( sts.CellGridDim + 1 );


			int nOffset = sts.RemapOffset(nCellX, nCellZ, sts.TileCountX)*VertPerTile - nVLockOffset;
			r3d_assert ( nOffset >= 0 );
			pV = pVLock + nOffset;

			for ( int z = nMinZ; z < nMaxZ; z++ )
			{
				for ( int x = nMinX; x < nMaxX; x++, pV++ )
				{
					float xx = R3D_MIN( (float)x, Width - 1 );
					float zz = R3D_MIN( (float)z, Height - 1 );

					for ( int i = 1; i < LODDESC_COUNT; i++ )
					{
						int nValX = (int)xx % (*sts.dLods)[ i ].nStep;
						int nValZ = (int)zz % (*sts.dLods)[ i ].nStep;
						if ( ! nValX && ! nValZ )
							continue;

						int x1 = floor( xx / (*sts.dLods)[ i ].nStep ) * (*sts.dLods)[ i ].nStep;
						int x2 = x1 + (*sts.dLods)[ i ].nStep;

						int z1 = floor( zz / (*sts.dLods)[ i ].nStep ) * (*sts.dLods)[ i ].nStep;
						int z2 = z1 + (*sts.dLods)[ i ].nStep;

						x2 = R3D_MIN( x2, (int)Width - 1 );
						z2 = R3D_MIN( z2, (int)Height - 1 );


						float h11 = GetEditHeight( x1, z1 );
						float h21 = GetEditHeight( x2, z1 );
						float h12 = GetEditHeight( x1, z2 );
						float h22 = GetEditHeight( x2, z2 );

						float hCurrent;

						if ( nValX + nValZ < (*sts.dLods)[ i ].nStep )
						{
							float fx = ( xx - x1 ) / (*sts.dLods)[ i ].nStep;
							float fz = ( zz - z1 ) / (*sts.dLods)[ i ].nStep;
							hCurrent = ( h21 - h11 ) * fx + ( h12 - h11 ) * fz + h11;
						}
						else
						{
							float fx = ( x2 - xx ) / (*sts.dLods)[ i ].nStep;
							float fz = ( z2 - zz ) / (*sts.dLods)[ i ].nStep;
							hCurrent = ( h12 - h22 ) * fx + ( h21 - h22 ) * fz + h22;
						}

						float h = GetEditHeight( xx, zz );

						(*pV).SetDelta(hCurrent - h);

						// normals

						r3dPoint3D v11 = NormalMapData[ z1 * (uint32_t)Width + x1 ]; 
						r3dPoint3D v21 = NormalMapData[ z1 * (uint32_t)Width + x2 ]; 
						r3dPoint3D v12 = NormalMapData[ z2 * (uint32_t)Width + x1 ]; 
						r3dPoint3D v22 = NormalMapData[ z2 * (uint32_t)Width + x2 ]; 

						r3dPoint3D vCurrent;

						float fDLength = sqrtf( 2 );

						if ( nValX + nValZ < (*sts.dLods)[ i ].nStep )
						{
							float fx = ( xx - x1 ) / (*sts.dLods)[ i ].nStep;
							float fz = ( zz - z1 ) / (*sts.dLods)[ i ].nStep;
							float fd = ( fDLength - sqrtf( fz * fz * 2.f ) )  / fDLength;

							r3dPoint3D v1 = LerpFast( v11, v12, fz ); // vert
							r3dPoint3D v2 = LerpFast( v12, v21, fd ); // 

							v1.Normalize();
							v2.Normalize();

							float f = ( xx - x1 ) / ( (*sts.dLods)[ i ].nStep - ( zz - z1 ) );
							vCurrent = LerpFast( v1, v2, f ); 
							vCurrent.Normalize();
						}
						else
						{
							float fx = ( x2 - xx ) / (*sts.dLods)[ i ].nStep;
							float fz = ( z2 - zz ) / (*sts.dLods)[ i ].nStep;

							float fd = ( fDLength - sqrtf( fz * fz * 2.f ) )  / fDLength;

							r3dPoint3D v1 = LerpFast( v22, v21, fz );
							r3dPoint3D v2 = LerpFast( v21, v12, fd );
							v1.Normalize();
							v2.Normalize();


							float f = ( x2 - xx ) / ( (*sts.dLods)[ i ].nStep - ( z2 - zz ) );

							vCurrent = LerpFast( v1, v2, f ); 
							vCurrent.Normalize();
						}

						(*pV).SetMorphNormal(PackNorm(vCurrent.GetPacked()));

						break;
					}
				}		
			}
		}

		AdvanceLoadingProgress( delta );
	}

	return true;
}


bool
r3dTerrain::UpdateVertexData( const UpdateVertexDataSettings& sts )
{
	if( r_terrain_quality->GetInt() == 1 )	
	{
		return DoUpdateVertexData<HeightNormalVertLQ>( sts );
	}
	else
	{
		return DoUpdateVertexData<HeightNormalVert>( sts );
	}
}


float r3dTerrain::GetCellSize()
{
	return CellSize;
}

void r3dTerrain::SetCellSize(float NewCellSize)
{
	CellSize        = NewCellSize;

	TotalWidth		= Width * CellSize;
	TotalHeight		= Height * CellSize;

	WorldSize       = TotalWidth;

	UpdateDesc() ;
}

namespace
{
	int ipow2( int e )
	{
		int val = 1;
		for( ;e--;)
		{
			val *= 2;
		}
		return val;
	}
}


int r3dTerrain::Load(const char* DirName)
{
	char 	TempStr[128];
	char 	TempStr1[128];
	r3dFile	*f;

	__HeightmapBlend = 0.25f;
	BaseTerraLOD1 = 1;
	BaseTerraLOD2 = 2;
	BaseTerraLOD3 = 24;

	r3dscpy( m_szName, DirName );

	sprintf( TempStr, FNAME_TERRAIN_INI, DirName );

	if ( r3dFileExists( TempStr ) )
	{
		r3dOutToLog("TERRAIN: Loading from script '%s'\n", TempStr);
		return LoadFromScript( TempStr ) ? 1 : 0;
	}

	char THN[128], TLN[128], TMB[128];
	NumMats = 0;
	sprintf(TempStr,  "%s\\Terrain\\%s", DirName, "terrain.def");
	if((f = r3d_open(TempStr,"rt")) != NULL)
	{
		fgets(TempStr1, sizeof(TempStr1), f);
		sscanf(TempStr1,"%s %f %f", THN, &CellSize, &m_NormalmapScale );
		fgets(TempStr1, sizeof(TempStr1), f);
		sscanf(TempStr1,"%s", TLN);

		//
		// Load base material
		//
		fgets(TempStr1, sizeof(TempStr1), f);
		sscanf(TempStr1,"%s %f", TMB, &m_tBaseLayer.fScale);
		sprintf(TempStr,  "%s\\Terrain\\%s", DirName, TMB);


		m_tBaseLayer.pMapDiffuse = r3dRenderer->LoadTexture(TempStr);


		//
		// Get Number of material "splats"
		//
		fgets(TempStr1, sizeof(TempStr1), f);
		sscanf(TempStr1,"%d", &NumMats);

		// 
		// Initialize different shit
		//
		sprintf(TempStr,  "%s\\%s", DirName, TLN);
		sprintf(TempStr1,  "%s\\GrassMap.bmp", DirName);


		//imgNormalMap = r3dRenderer->LoadTexture(TempStr);

		for (int i=0;i<NumMats;i++)
		{
			char T1[64];

			//
			// Load splatting material
			//
			fgets(TempStr1, sizeof(TempStr1), f);
			sscanf(TempStr1,"%s", T1);

			sprintf(TempStr,  "%s\\Terrain\\%s", DirName, T1);

			MatSplatTex[i] = r3dRenderer->LoadTexture(TempStr, D3DFMT_A8R8G8B8 );
			EditorMatSplatTex[i] = MatSplatTex[i];

			for (int k=0;k<4;k++)
			{
				fgets(TempStr1, sizeof(TempStr1), f);
				sscanf(TempStr1,"%s %f", T1, &m_dMatLayers[i][k].fScale );

				sprintf(TempStr,  "%s\\Terrain\\%s", DirName, T1);

				m_dMatLayers[i][k].pMapDiffuse = r3dRenderer->LoadTexture(TempStr);
			}

		}

		fclose(f);
	}
	else
	{
		// init default material
		NumMats = 1;

		for( int i = 0; i < TERRAIN_LAYERS_PER_MAT; i ++ )
		{
			Layer_t& layer = m_dMatLayers[ 0 ][ i ];

			layer.fGloss		= 0.f;
			layer.fScale		= 1.f;
			layer.fSplit		= 1.f;
			layer.pMapDiffuse	= r3dRenderer->LoadTexture( "MissinTexture.dds" );
			layer.pMapNormal	= r3dRenderer->LoadTexture( "MissinTexture.dds" );
		}
	}


	//
	// HeightMap
	//
	sprintf(TempStr, "%s\\Terrain\\%s", DirName,THN);



	// Init other important values
	SetCellSize(CellSize);

	bLoaded = 1;

	m_dVisibleTiles.Reserve ( (int)Width / CellGridSize * (int)Height / CellGridSize ) ;

	UpdateDesc() ;

	return TRUE;
}

//------------------------------------------------------------------------

void
r3dTerrain::UpdateDesc()
{
	r3dTerrainDesc desc ;

	desc.LayerCount = 1 ;

	for( int i = 0, e = MatSplatTex.COUNT ; i < e ; i ++ )
	{
		if( MatSplatTex[ i ] )
		{
			desc.LayerCount += 4 ;
		}
	}

	desc.XSize = GetTotalWidth() ;
	desc.ZSize = GetTotalWidth() ;

	desc.MinHeight = m_MinHeight ;
	desc.MaxHeight = m_MaxHeight ;

	desc.CellCountX = (int)Width ;
	desc.CellCountZ = (int)Height ;

	if( r3dTexture* splat = MatSplatTex[ 0 ] )
	{
		desc.SplatResolutionU = splat->GetWidth() ;
		desc.SplatResolutionV = splat->GetHeight() ;
	}
	else
	{
		desc.SplatResolutionU = desc.CellCountX ;
		desc.SplatResolutionV = desc.CellCountZ ;
	}

	desc.CellCountPerTile = CellGridSize ;

	desc.CellSize = CellSize ;

	SetDesc( desc ) ;
}

//------------------------------------------------------------------------

void
r3dTerrain::SelectTile( const r3dPoint3D& pos )
{
	SelectedTileX = pos.x / CellGridSize / CellSize;
	SelectedTileZ = pos.z / CellGridSize / CellSize;
}

//------------------------------------------------------------------------

void
r3dTerrain::HighlightTile( const r3dPoint3D& pos )
{
	HighlightedTileX = pos.x / CellGridSize / CellSize;
	HighlightedTileZ = pos.z / CellGridSize / CellSize;
}

//------------------------------------------------------------------------

r3dTerrain::Layers*
r3dTerrain::GetSelectedLayers()
{
	if( SelectedTileX < 0				|| 
		SelectedTileZ < 0				|| 
		SelectedTileX >= m_nTileCountX	|| 
		SelectedTileZ >= m_nTileCountZ
		)
	{
		return NULL;
	}

	int nLayer = REMAP_OFFSET(SelectedTileX,SelectedTileZ,m_nTileCountX);

	return &TileLayers[ nLayer ];
}

//------------------------------------------------------------------------

void
r3dTerrain::ClearLayerAtSelectedTile( int layer )
{
	if( SelectedTileX < 0				|| 
		SelectedTileZ < 0				|| 
		SelectedTileX >= m_nTileCountX	|| 
		SelectedTileZ >= m_nTileCountZ	||
		layer < 1 || layer >= 33
		)
	{
		return;
	}

	int mat		= ( layer - 1 ) / TERRAIN_LAYERS_PER_MAT;
	int channel	= 3 - ( layer - 1 ) % TERRAIN_LAYERS_PER_MAT;

	if( !MatSplatTex[ mat ] )
		return;
	if(!MatSplatTex[mat]->IsValid())
		return;

	IDirect3DTexture9* tex = MatSplatTex[ mat ]->AsTex2D();

	D3DLOCKED_RECT lrect;
	RECT rect;

	int TEX_WIDTH = MatSplatTex[ mat ]->GetWidth() ;
	int TEX_HEIGHT = MatSplatTex[ mat ]->GetHeight() ;

	int TEX_TILE_X = TEX_WIDTH / m_nTileCountX ;
	int TEX_TILE_Z = TEX_HEIGHT / m_nTileCountZ ;

	int xs = SelectedTileX * TEX_TILE_X;
	int zs = (int) TEX_HEIGHT - ( SelectedTileZ + 1 ) * TEX_TILE_Z;
	int xe = ( SelectedTileX + 1 ) * TEX_TILE_X;
	int ze = (int) TEX_HEIGHT - SelectedTileZ * TEX_TILE_Z;

	xs = R3D_MAX( xs - 1, 0 );
	zs = R3D_MAX( zs - 1, 0 );

	xe = R3D_MIN( xe + 1, TEX_WIDTH );
	ze = R3D_MIN( ze + 1, TEX_HEIGHT );

	rect.left	= xs;
	rect.right	= xe;
	rect.top	= zs;
	rect.bottom	= ze;

	int mask = ~( 0xff << channel * 8 );

	D3D_V( tex->LockRect( 0, &lrect, &rect, 0 ) );

	for( uint32_t j = 0, e = ze - zs; j < e; j ++ )
	{
		for( uint32_t i = 0, e = (xe - xs)*4; i < e; i += 4 )
		{
			*(DWORD*)((char*) lrect.pBits + i + j * lrect.Pitch ) &= mask;
		}
	}

	D3D_V( tex->UnlockRect( 0 ) );
}

//------------------------------------------------------------------------

struct TerraAppendShadowOpt
{
	r3dTerrain::VisibleTiles* visibleTiles ;
	float *oMinX ; 
	float *oMaxX ; 
	float *oMinY ; 
	float *oMaxY ;

	volatile LONG InMinMaxAccess ;
};

#define MULTITHREADED_TERRAIN_SHADOW_OPT 0

void TerraAppendShadowOptFunc( void* Data, size_t ItemStart, size_t ItemCount )
{
	TerraAppendShadowOpt* data = (TerraAppendShadowOpt*) Data;

	const D3DXMATRIX& view = r3dRenderer->ViewMatrix ;

	D3DXVECTOR4 tpos[ 8 ] ;

	for( size_t i = ItemStart, e = ItemStart + ItemCount ; i < e ; i ++ )
	{
		const r3dTerrain::VisibleTile& tile = (*data->visibleTiles)[ i ];

		float z = tile.tTileBox.Org.z ;

		D3DXVECTOR3 pos( tile.tTileBox.Org.x, tile.tTileBox.Org.y, z );
		D3DXVec3Transform( &tpos[ 0 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x + tile.tTileBox.Size.x, tile.tTileBox.Org.y, z );
		D3DXVec3Transform( &tpos[ 1 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x + tile.tTileBox.Size.x, tile.tTileBox.Org.y + tile.tTileBox.Size.y, z );
		D3DXVec3Transform( &tpos[ 2 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x, tile.tTileBox.Org.y + tile.tTileBox.Size.y, z );
		D3DXVec3Transform( &tpos[ 3 ], &pos, &view );

		z = tile.tTileBox.Org.z + tile.tTileBox.Size.z ;

		pos = D3DXVECTOR3( tile.tTileBox.Org.x, tile.tTileBox.Org.y, z );
		D3DXVec3Transform( &tpos[ 4 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x + tile.tTileBox.Size.x, tile.tTileBox.Org.y, z );
		D3DXVec3Transform( &tpos[ 5 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x + tile.tTileBox.Size.x, tile.tTileBox.Org.y + tile.tTileBox.Size.y, z );
		D3DXVec3Transform( &tpos[ 6 ], &pos, &view );

		pos = D3DXVECTOR3( tile.tTileBox.Org.x, tile.tTileBox.Org.y + tile.tTileBox.Size.y, z );
		D3DXVec3Transform( &tpos[ 7 ], &pos, &view );


#if MULTITHREADED_TERRAIN_SHADOW_OPT
		for( ; InterlockedExchange( &data->InMinMaxAccess, 1 ) == 1 ; ) ;
#endif

		for( size_t i = 0, e = R3D_ARRAYSIZE( tpos ) ; i < e; i ++ )
		{
			*data->oMinX = R3D_MIN( tpos[ i ].x, *data->oMinX );
			*data->oMaxX = R3D_MAX( tpos[ i ].x, *data->oMaxX );
			*data->oMinY = R3D_MIN( tpos[ i ].y, *data->oMinY );
			*data->oMaxY = R3D_MAX( tpos[ i ].y, *data->oMaxY );
		}

		data->InMinMaxAccess = 0 ;
	}
}

void r3dTerrain::AppendShadowMapOptimizations( float* oMinX, float* oMaxX, float *oMinY, float* oMaxY )
{
	R3DPROFILE_FUNCTION( "r3dTerrain::AppendShadowMapOptimizations" );

	TerraAppendShadowOpt opts;

	opts.visibleTiles	= &m_dVisibleTiles ;
	opts.oMinX			= oMinX ;
	opts.oMaxX			= oMaxX ;
	opts.oMinY			= oMinY ;
	opts.oMaxY			= oMaxY ;
	opts.InMinMaxAccess	= 0 ;

#if MULTITHREADED_TERRAIN_SHADOW_OPT
	// the procedure itself is very fast so multithreading makes things worse on 2 cores
	g_pJobChief->Exec( TerraAppendShadowOptFunc, &opts, m_dVisibleTiles.Count() );
#else
	TerraAppendShadowOptFunc( &opts, 0, m_dVisibleTiles.Count() );
#endif
	
}
//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::LoadFromScript( const char * fileName )
{
	Script_c script;

	if ( ! script.OpenFile( fileName ) )
		return false;

	char buffer[ MAX_PATH ];

	while ( ! script.EndOfFile() )
	{
		script.GetToken( buffer );
		if ( ! *buffer )
			break;

		if ( ! strcmp( buffer, "size:" ) )
		{
			Height = Width = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "cell_size:" ) )
		{
			SetCellSize( script.GetFloat() );
		}
		else if ( ! strcmp( buffer, "height:" ) )
		{
			m_HeightmapSize = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "normal_scl:" ) )
		{
			m_NormalmapScale = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "hm_blend:" ) )
		{
			__HeightmapBlend = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "split_dist:" ) )
		{
			SplitDistance = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "lod:" ) )
		{
			BaseTerraLOD1 = script.GetFloat();
			BaseTerraLOD2 = script.GetFloat();
			BaseTerraLOD3 = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "tiling_mode:" ) )
		{
			script.GetToken( buffer );
			m_eTilingMode = StringToTilingMode( buffer );
		}	
		else if ( ! strcmp( buffer, "base_layer" ) )
		{
			LoadLayerFromScript( script, m_tBaseLayer );
		}
		else if ( ! strcmp( buffer, "material" ) )
		{
			LoadMaterialsFromScript( script );
		}
		else if (	! strcmp( buffer, "SimpleTerrainRenderFadeStart:" ) ||
			! strcmp( buffer, "shader_lod_fade_start:" )
			)
		{
			m_fShaderLODFadeStart = script.GetFloat();
		}
		else if (	! strcmp( buffer, "SimpleTerrainRenderFadeEnd:" ) ||
			! strcmp( buffer, "shader_lod_fade_end:" )
			)
		{
			m_fShaderLODFadeEnd = script.GetFloat();
		}
		else if( ! strcmp( buffer, "camera_near_plane:" ))
		{
			r_near_plane->SetFloat( script.GetFloat() );
		}
		else if( ! strcmp( buffer, "camera_far_plane:" ) )
		{
			r_far_plane->SetFloat( script.GetFloat() );
		}
	}
	r3dOutToLog("TERRAIN: Finished reading script file\n");
	script.CloseFile();

	m_dVisibleTiles.Reserve ( (int)Width / CellGridSize * (int)Height / CellGridSize );

	InitData();
	return true;
}

//------------------------------------------------------------------------

static int GetTerraTexDownScale()
{
	int DownScale = 1;

	switch( r_texture_quality->GetInt() )
	{
	case 1:
		return 2 ;
		break ;
	default:
		return 1 ;
		break ;
	}

	return DownScale;
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::LoadLayerFromScript( Script_c &script, Layer_t &layer )
{
#ifdef WO_SERVER
	return false;
#endif
	char buffer[ MAX_PATH ];
	char szName[ MAX_PATH ];

	bool hasTextures = false;

	int DownScale = GetTerraTexDownScale();

	layer.MatTypeName[ 0 ] = 0;

	script.SkipToken( "{" );
	while ( ! script.EndOfFile() )
	{
		script.GetToken( buffer );
		if ( *buffer == '}' )
			break;

		if ( ! strcmp( buffer, "map_diffuse:" ) )
		{
			hasTextures = true;

			script.GetString( szName, sizeof( szName ) );
			layer.pMapDiffuse = r3dRenderer->LoadTexture( szName, D3DFMT_UNKNOWN, false, DownScale );				
		}
		else if( !strcmp(buffer, "split_enable:" ))
		{
			layer.bDoSplit = script.GetInt() ? 1 : 0;
		}
		else if ( ! strcmp( buffer, "scale:" ) )
		{
			layer.fScale = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "split:" ) )
		{
			layer.fSplit = script.GetFloat();
		}
		else if ( ! strcmp( buffer, "map_normal:" ) )
		{
			hasTextures = true;

			script.GetString( szName, sizeof( szName ) );
			layer.pMapNormal = r3dRenderer->LoadTexture( szName, D3DFMT_UNKNOWN, false, DownScale );
		}
		else if ( ! strcmp( buffer, "gloss:" ) )
		{
			layer.fGloss = script.GetFloat();
		}
		else if( ! strcmp( buffer, "mat_type:"  ) )
		{
			char buff[ 512 ];

			buff[ sizeof buff - 1 ] = 0;

			script.GetLine( buff, sizeof buff - 1 );

			sscanf( buff, "%31s", layer.MatTypeName );
		}
	}

	return hasTextures;
}

//--------------------------------------------------------------------------------------------------------
void r3dTerrain::LoadMaterialsFromScript( Script_c &script )
{
#ifdef WO_SERVER
	return;
#endif
	char buffer[ MAX_PATH ];
	char szName[ MAX_PATH ] = {0};

	script.SkipToken( "{" );
	if ( NumMats >= TERRAIN_MAT_COUNT )
		return;

	int nLayer = 0;

	bool layersHasTextures = false;

	while ( ! script.EndOfFile() )
	{
		script.GetToken( buffer );
		if ( *buffer == '}' )
			break;

		if ( ! strcmp( buffer, "map_splat_mask:" ) )
		{
			char ralativePath[ MAX_PATH ] = {0};
			script.GetString( ralativePath, sizeof( ralativePath ) );
			char levelDir[MAX_PATH];
			sprintf(levelDir, "%s", r3dGameLevel::GetHomeDir());	
			_strlwr(levelDir);
			_strlwr(ralativePath);

			int rl = strlen(ralativePath);
			int dl = strlen(levelDir);

			std::replace(levelDir, levelDir + dl, '\\', '/');
			std::replace(ralativePath, ralativePath + rl, '\\', '/');

			char* fr = strstr(ralativePath, levelDir);
			if(!fr)
			{
				sprintf(szName, "%s/%s", levelDir, ralativePath);	
			}
			else
			{
				sprintf(szName, "%s", ralativePath);	
			}


		}
		else if ( ! strcmp( buffer, "layer" ) )
		{
			if ( nLayer < TERRAIN_LAYERS_PER_MAT )
			{
				layersHasTextures |= LoadLayerFromScript( script, m_dMatLayers[ NumMats ][ nLayer++ ] );
			}
		}
	}

	if ( szName[0] )
	{
		if( layersHasTextures )
		{
			extern bool g_bEditMode;
			if ( g_bEditMode )
			{
				MatSplatTex[ NumMats ] = r3dRenderer->LoadTexture( szName );

				if( !(MatSplatTex[ NumMats ]->IsLoaded() ) )
				{
					r3dRenderer->DeleteTexture( MatSplatTex[ NumMats ] );
					r3d_assert( !EditorMatSplatTex[ NumMats ] );

					int oldNumMats = NumMats;
					EditorSplatOn( NumMats );
					NumMats = oldNumMats;
				}
				else
					EditorMatSplatTex[ NumMats ] = MatSplatTex[ NumMats ];
			}
			else
			{
				char tbuf[1024];
				PrintSplatLocationPS3(tbuf, szName);
				MatSplatTex[ NumMats ] = r3dRenderer->LoadTexture( tbuf );
				EditorMatSplatTex[ NumMats ] = r3dRenderer->LoadTexture( szName );
			}
		}
		else
		{
			if( !r3d_access( szName, 0 ) )
			{
				r3dOutToLog( "r3dTerrain::LoadMaterialsFromScript: deleting %s because it is unused.\n", szName );
				remove( szName );
			}

			EditorMatSplatTex[ NumMats ] = MatSplatTex[ NumMats ] = NULL ;
		}
	}

	r3d_assert( nLayer == TERRAIN_LAYERS_PER_MAT );
	NumMats++;
}

//------------------------------------------------------------------------

namespace
{
	inline float saturate( float a )
	{
		return R3D_MAX( R3D_MIN( a, 1.f ), 0.f );
	}

	// NOTE : sync with harshize in PS3 terrain pixel shader
	inline float harshize( float a )
	{
		return saturate( 8 * a - 3.5 );
	}
}

void
r3dTerrain::CalculateTileFaceOrients( uint8_t* target, int tileX, int tileY, int cellGridSize, int step, TerraFaceOrientStats& oStats )
{
	int sx = tileX * cellGridSize;
	int sy = tileY * cellGridSize;

	int ex = ( tileX + 1 ) * cellGridSize ;
	int ey = ( tileY + 1 ) * cellGridSize ;

	int iwidth = (int)Width;
	int iheight = (int)Height;

	TerraFaceOrientStats stats = { 0 };

	int m = 0;

	for( int j = sy, e = ey; j < e; j += step )
	{
		for( int i = sx, e = ex; i < e; i += step )
		{

			if( ! m )
			{
				*target = 0;
			}

			int ip = R3D_MIN( i + step, iwidth - 1 );
			int jp = R3D_MIN( j + step, iheight - 1 );

			r3dPoint3D n0 = NormalMapData[ i + j * iwidth ];
			r3dPoint3D n1 = NormalMapData[ i + jp * iwidth ];
			r3dPoint3D n2 = NormalMapData[ ip + j * iwidth ];
			r3dPoint3D n3 = NormalMapData[ ip + jp * iwidth ];

			n0.Normalize();
			n1.Normalize();
			n2.Normalize();
			n3.Normalize();

			r3dPoint3D tn0 = n0 + n1 + n2;
			r3dPoint3D tn1 = n1 + n2 + n3;

			tn0.Normalize();
			tn1.Normalize();

			for( int t = 0; t < 2; t ++ )
			{
				r3dPoint3D sN[ 4 ];

				if( t == 0 )
				{
					sN[ 0 ] = tn0;
					sN[ 1 ] = n0;
					sN[ 2 ] = n1;
					sN[ 3 ] = n2;
				}
				else
				{
					sN[ 0 ] = tn1;
					sN[ 1 ] = n1;
					sN[ 2 ] = n2;
					sN[ 3 ] = n3;
				}

				int bestForX = 0;
				int bestForY = 0;

				float bestXDeviation = 999.f;
				float bestYDeviation = 999.f;

				for( int i = 0; i < 4; i ++ )
				{
					float xdev = fabs( fabs( sN[ i ].x ) - 0.5f );
					float ydev = fabs( fabs( sN[ i ].y ) - 0.5f );

					if( xdev < bestXDeviation )
					{
						bestForX = i;
						bestXDeviation = xdev;
					}

					if( ydev < bestYDeviation )
					{
						bestForY = i;
						bestYDeviation = ydev;
					}
				}

				float xfact = harshize( fabs( sN[ bestForX ].x ) );
				float yfact = harshize( fabs( sN[ bestForY ].y ) );

				uint32_t bits;

				if( yfact >= 1.f )
				{
					bits = Y_ORIENT;
					stats.YOrient ++;
				}
				else
				{
					bool multi = false;

					if( yfact <= 0.f )
					{
						if( xfact >= 1.f )
						{
							bits = X_ORIENT;
							stats.XOrient ++;
						}
						else
						{
							if( xfact <= 0.f )
							{
								bits = Z_ORIENT;
								stats.ZOrient ++;
							}
							else
							{
								multi = true;
							}
						}
					}
					else
					{
						multi = true;
					}

					if( multi )
					{
						bits = MULT_ORIENT;
						stats.MultiOrient ++;
					}
				}

				*target |=  bits << ( m + t * 2 );
			}

			m += 4;

			if( m == 8 )
			{
				target ++;
				m = 0;
			}
		}
	}

	int maxCount = cellGridSize * cellGridSize * 2 / step / step;

	if( stats.XOrient == maxCount )
	{
		stats.FullXOrient ++;
	}

	if( stats.YOrient == maxCount )
	{
		stats.FullYOrient ++;
	}

	if( stats.ZOrient == maxCount )
	{
		stats.FullZOrient ++;
	}

	if( stats.MultiOrient == maxCount )
	{
		stats.FullMultiOrient ++;
	}

	oStats = stats;
}


enum ConnectorType_t
{
	eConnectorType_Bottom,
	eConnectorType_Left,
	eConnectorType_Right,
	eConnectorType_Top,

	eConnectorType_Count
};

//--------------------------------------------------------------------------------------------------------
int GetVIndex( int nInd, ConnectorType_t eType, int iOffset )
{
	int iMult = iOffset < 0 ? 0 : 1;
	if ( eType == eConnectorType_Bottom )
	{
		return nInd + iMult * iOffset * ( CellGridSize + 1 ) * ( CellGridSize + 1 );
	}
	else if ( eType == eConnectorType_Top )
	{
		return nInd + ( CellGridSize + 1 ) * CellGridSize + iMult * ( N_OVERTILES * ( N_OVERTILES - 1 ) + iOffset ) * ( CellGridSize + 1 ) * ( CellGridSize + 1 );
	}
	else if ( eType == eConnectorType_Left )
	{
		return nInd * ( CellGridSize + 1 ) + iMult * N_OVERTILES * iOffset * ( CellGridSize + 1 ) * ( CellGridSize + 1 );
	}
	else if ( eType == eConnectorType_Right )
	{
		return nInd * ( CellGridSize + 1 ) + CellGridSize + iMult * ( N_OVERTILES * iOffset + N_OVERTILES - 1 ) * ( CellGridSize + 1 ) * ( CellGridSize + 1 );
	}

	assert( false );
	return INVALID_INDEX;
}

//------------------------------------------------------------------------

void
r3dTerrain::PreparePhysXHeightFieldDesc	( PxHeightFieldDesc& hfDesc )
{
	int w = Width, 
		h = Height;

	hfDesc.format				= PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns			= w;
	hfDesc.nbRows				= h;
	hfDesc.convexEdgeThreshold	= 0;
	hfDesc.thickness			= -1000.0f;

	// allocate storage for samples
	hfDesc.samples.stride		= sizeof(PxU32);
	hfDesc.samples.data			= malloc(hfDesc.samples.stride*w*h);
	if(hfDesc.samples.data==NULL)
		r3dError("Out of memory!");

	r3d_assert( hfDesc.samples.data );
}

//------------------------------------------------------------------------

void
r3dTerrain::FinishPhysXHeightFieldDesc	( PxHeightFieldDesc& hfDesc )
{
	free((void*)hfDesc.samples.data);
	hfDesc.samples.data = NULL;
}

//------------------------------------------------------------------------

void
r3dTerrain::UpdateHFShape()
{
	r3d_assert(_CrtCheckMemory());
	PxHeightFieldGeometry hfGeom = GetHFShape();

	r3d_assert(_CrtCheckMemory());
	PxU32	w = (PxU32)Width,
		h = (PxU32)Height;

	// do not create a new shape, just update current shape with new parameters
	hfGeom.heightScale = m_InvHFScale;
	hfGeom.rowScale = WorldSize / PxReal(h);
	hfGeom.columnScale = WorldSize / PxReal(w);

	r3d_assert( physicsTerrain );
	r3d_assert( physicsTerrain->getNbShapes () == 1 );
	
	r3d_assert(_CrtCheckMemory());
	PxShape* shapes[1] = {0};
	physicsTerrain->getShapes(&shapes[0], 1);
	
	r3d_assert(_CrtCheckMemory());

	r3d_assert( shapes[0]->getGeometryType() == PxGeometryType::eHEIGHTFIELD );
	//shapes[0]->setGeometry(hfGeom); // memory corruption. not sure why.
	
	r3d_assert(_CrtCheckMemory());
}

//------------------------------------------------------------------------

void
r3dTerrain::UpdatePhysHeightField ()
{
	r3d_assert(_CrtCheckMemory());
	r3dOutToLog("TERRAIN: UpdatePhysHeightField\n");

	if(physicsTerrain)
	{
		physicsTerrain->release();
		physicsTerrain = 0;
		physicsHeightField->release();
		physicsHeightField = 0;
	}

	const Floats& source = HeightFieldData;
	r3d_assert( source.Count() == (int)Width * (int)Height );

	r3d_assert(_CrtCheckMemory());
	PreparePhysXHeightFieldDesc( physicsHeightFieldDesc );
	r3d_assert(_CrtCheckMemory());

	char* currentByte = (char*)physicsHeightFieldDesc.samples.data;

	m_MaxHeight = -FLT_MAX;
	m_MinHeight = FLT_MAX;

	for( uint32_t i = 0, e = source.Count(); i < e; i ++ )
	{
		m_MaxHeight = R3D_MAX( source[ i ], m_MaxHeight );
		m_MinHeight = R3D_MIN( source[ i ], m_MinHeight );
	}

	r3d_assert(_CrtCheckMemory());
	SetupHFScale();

	r3d_assert(_CrtCheckMemory());
	float norm = m_HFScale;

	PxU32	w = (PxU32)Width,
		h = (PxU32)Height;

	for (PxU32 column = 0; column < w; column++)
	{
		for (PxU32 row = 0; row < h; row++)
		{
			PxI16 height = (PxI32)( R3D_MIN( source[ row * w + column ] * norm + 0.5f, 32767.f ) );

			PxHeightFieldSample* currentSample = (PxHeightFieldSample*)currentByte;
			currentSample->height = height;
			currentSample->materialIndex0 = 1;
			currentSample->materialIndex1 = 1;
			currentSample->clearTessFlag();
			currentByte += physicsHeightFieldDesc.samples.stride;
		}
	}

	physicsHeightField = g_pPhysicsWorld->PhysXSDK->createHeightField(physicsHeightFieldDesc);

	FinishPhysXHeightFieldDesc( physicsHeightFieldDesc ); 

	PxHeightFieldGeometry shapeGeom(physicsHeightField, PxMeshGeometryFlags(), m_InvHFScale, WorldSize / PxReal(h), WorldSize / PxReal(w));
	PxTransform pose(PxVec3(0,0,0), PxQuat(0,0,0,1));
	physicsTerrain = g_pPhysicsWorld->PhysXSDK->createRigidStatic(pose);

	PxShape* aHeightFieldShape = physicsTerrain->createShape(shapeGeom, *g_pPhysicsWorld->defaultMaterial);

	PxFilterData filterData(PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
	aHeightFieldShape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
#if VEHICLES_ENABLED
	PxSetupDrivableShapeQueryFilterData(&qfilterData);
#endif
	aHeightFieldShape->setQueryFilterData(qfilterData);

	g_pPhysicsWorld->AddActor(*physicsTerrain);

	UpdateHFShape();

	r3d_assert(_CrtCheckMemory());

/*	
	r3d_assert( physicsHeightField );
	const Floats& source = HeightFieldData;

	r3d_assert( source.Count() == (int)Width * (int)Height );

	r3d_assert(_CrtCheckMemory());
	PreparePhysXHeightFieldDesc( physicsHeightFieldDesc );
	r3d_assert(_CrtCheckMemory());

	char* currentByte = (char*)physicsHeightFieldDesc.samples.data;

	m_fMaxHeight = -FLT_MAX;
	m_fMinHeight = FLT_MAX;

	for( uint32_t i = 0, e = source.Count(); i < e; i ++ )
	{
		m_fMaxHeight = R3D_MAX( source[ i ], m_fMaxHeight );
		m_fMinHeight = R3D_MIN( source[ i ], m_fMinHeight );
	}

	r3d_assert(_CrtCheckMemory());
	SetupHFScale();

	r3d_assert(_CrtCheckMemory());
	float norm = m_HFScale;

	PxU32	w = (PxU32)Width,
		h = (PxU32)Height;

	for (PxU32 column = 0; column < w; column++)
	{
		for (PxU32 row = 0; row < h; row++)
		{
			PxI16 height = (PxI32)( R3D_MIN( source[ row * w + column ] * norm + 0.5f, 32767.f ) );

			PxHeightFieldSample* currentSample = (PxHeightFieldSample*)currentByte;
			currentSample->height = height;
			currentSample->materialIndex0 = 1;
			currentSample->materialIndex1 = 1;
			currentSample->clearTessFlag();
			currentByte += physicsHeightFieldDesc.samples.stride;
		}
	}

	r3d_assert(_CrtCheckMemory());

	if(physicsHeightField)
	{
		physicsHeightField->release();
		physicsHeightField = NULL;
	}
	r3d_assert(_CrtCheckMemory());
	physicsHeightField = g_pPhysicsWorld->PhysXSDK->createHeightField(physicsHeightFieldDesc);
r3d_assert(_CrtCheckMemory());
	FinishPhysXHeightFieldDesc( physicsHeightFieldDesc );
r3d_assert(_CrtCheckMemory());
	UpdateHFShape();
	r3d_assert(_CrtCheckMemory());*/
}

//------------------------------------------------------------------------

void
r3dTerrain::ExtractHeightFieldData()
{
	r3dOutToLog("TERRAIN: ExtractHeightFieldData\n");
	r3d_assert( physicsHeightField );

	PxU32	w	= (PxU32) Width, 
			h	= (PxU32) Height;

	HeightFieldData.Resize( w * h );

	// otherwise too slow...
	PxU32 dataSize = GetHeightFieldDataSize( physicsHeightFieldDesc );

	Bytes data( dataSize );

	PX_V( physicsHeightField->saveCells( &data[0], dataSize ) );

	char* srcP		= (char*)&data[ 0 ];

	float scale = m_InvHFScale;

	for( int x = 0; x < w; x++  )	
	{
		for( int z = 0; z < h; z++ )
		{
			HeightFieldData[ x + z * w ] = ((PxHeightFieldSample*)srcP)->height * scale;
			srcP += physicsHeightFieldDesc.samples.stride;
		}
	}
}


//------------------------------------------------------------------------

void
r3dTerrain::PrepareForSettingsUpdateInGame()
{
	r3d_assert( !g_bEditMode ) ; // should happen in game only

	uint32_t	w	= (uint32_t) Width, 
				h	= (uint32_t) Height;

	if( !HeightFieldData.Count() )
	{
		HeightFieldData.Resize( w * h ) ;		
	}

	if( !NormalMapData )
	{
		NormalMapData = new r3dPoint3D[ w * h ] ;
	}

	if( !m_pColorData )
	{
		m_pColorData =  new uint32_t[ w * h ];

		r3dFile *f1 = OpenTerrainBinFile();
		r3d_assert( f1 );

		uint32_t sig, ver = 0 ;

		ReadTerrainHeader( f1, sig, ver ) ;

		ReadColorData( f1, ver, w * h, true ) ;

		fclose (f1);
	}

	ExtractHeightFieldData();
}

//------------------------------------------------------------------------

void
r3dTerrain::ReleaseSettingsUpdateData()
{
	r3d_assert( !g_bEditMode ) ; // should happen in game only

	Floats().Swap( HeightFieldData ) ;
	SAFE_DELETE_ARRAY( NormalMapData ) ;
	SAFE_DELETE_ARRAY( m_pColorData ) ;
}

//------------------------------------------------------------------------

void
r3dTerrain::HeightEditingEnter( )
{
	r3d_assert( g_bEditMode );

	if( !bInHeightEditMode )
	{
		ExtractHeightFieldData();
	}

	bInHeightEditMode = 1;
}

//------------------------------------------------------------------------

void
r3dTerrain::HeightEditingLeave( )
{
	if( bInHeightEditMode )
	{
		bInHeightEditMode = 0;
		UpdatePhysHeightField();
	}
}

//------------------------------------------------------------------------

void
r3dTerrain::UpdatePhysHeightField( PxU32* source )
{
	r3d_assert( !bInHeightEditMode );
	r3d_assert( physicsHeightField );

	PreparePhysXHeightFieldDesc( physicsHeightFieldDesc );

	memcpy( (void*)physicsHeightFieldDesc.samples.data, source, GetHeightFieldDataSize( physicsHeightFieldDesc ) );

	if(physicsHeightField)
	{
		physicsHeightField->release();
		physicsHeightField = NULL;
	}
	physicsHeightField = g_pPhysicsWorld->PhysXSDK->createHeightField(physicsHeightFieldDesc);

	FinishPhysXHeightFieldDesc( physicsHeightFieldDesc );

	UpdateHFShape();
}

//------------------------------------------------------------------------

void SavePhysXHeightFieldDesc(const PxHeightFieldDesc& desc, FILE* file)
{
	fwrite(&desc, sizeof(desc), 1, file);
	fwrite(desc.samples.data, desc.samples.stride * desc.nbColumns * desc.nbRows, 1, file);
}

void LoadPhysXHeightFieldDesc(PxHeightFieldDesc& desc, r3dFile* file)
{
	fread(&desc, sizeof(desc), 1, file);
	desc.samples.data = malloc(desc.samples.stride * desc.nbColumns * desc.nbRows);
	if(desc.samples.data==NULL)
		r3dError("Out of memory!");
	fread((void*)desc.samples.data, desc.samples.stride * desc.nbColumns * desc.nbRows, 1, file);	
}

void r3dTerrain::CreateDefaultHeightField()
{
	PreparePhysXHeightFieldDesc( physicsHeightFieldDesc );

	char* currentByte = (char*)physicsHeightFieldDesc.samples.data;

	for( uint32_t i = 0, e = physicsHeightFieldDesc.nbColumns * physicsHeightFieldDesc.nbRows; i < e; i ++, currentByte += physicsHeightFieldDesc.samples.stride )
	{
		PxHeightFieldSample* currentSample = (PxHeightFieldSample*)currentByte;
		currentSample->height = 0;
		currentSample->materialIndex0 = 1;
		currentSample->materialIndex1 = 1;
		currentSample->clearTessFlag();
	}

	physicsHeightField = g_pPhysicsWorld->PhysXSDK->createHeightField(physicsHeightFieldDesc);
	FinishPhysXHeightFieldDesc( physicsHeightFieldDesc );
}

//------------------------------------------------------------------------

void r3dTerrain::CreateDefaultPhysicsData()
{
	r3d_assert(_CrtCheckMemory());
	r3dOutToLog("TERRAIN: Attempting to create physics data\n");
	if(physicsTerrain)
	{
		physicsTerrain->release();
		physicsTerrain = 0;
		physicsHeightField->release();
		physicsHeightField = 0;
	}

	r3d_assert(_CrtCheckMemory());
	CreateDefaultHeightField();

	r3d_assert(_CrtCheckMemory());
	// dummy shape so that physx doesn't complain much
	PxHeightFieldGeometry shapeGeom(physicsHeightField, PxMeshGeometryFlags(), 1.0f, 1.0f, 1.0f);
	PxTransform pose(PxVec3(0,0,0), PxQuat(0,0,0,1));
	physicsTerrain = g_pPhysicsWorld->PhysXSDK->createRigidStatic(pose);
	
	r3d_assert(_CrtCheckMemory());
	PxShape* aHeightFieldShape = physicsTerrain->createShape(shapeGeom, *g_pPhysicsWorld->defaultMaterial);
	r3d_assert(_CrtCheckMemory());
	PxFilterData filterData(PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
	aHeightFieldShape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
#if VEHICLES_ENABLED
	PxSetupDrivableShapeQueryFilterData(&qfilterData);
#endif
	aHeightFieldShape->setQueryFilterData(qfilterData);
	r3d_assert(_CrtCheckMemory());
}

//------------------------------------------------------------------------

void r3dTerrain::CreatePhysicsData ( const Shorts& source )
{
	r3dOutToLog("TERRAIN: Attempting to create physics data\n");
	if(physicsTerrain)
	{
		physicsTerrain->release();
		physicsTerrain = 0;
		physicsHeightField->release();
		physicsHeightField = 0;
	}

	PreparePhysXHeightFieldDesc( physicsHeightFieldDesc );

	char* currentByte = (char*)physicsHeightFieldDesc.samples.data;

	PxU32	w = (PxU32)Width,
		h = (PxU32)Height;

	r3d_assert( source.Count() == w * h );

	for( uint32_t i = 0, e = source.Count(); i < e; i ++ )
	{
		PxHeightFieldSample* currentSample = (PxHeightFieldSample*)currentByte;
		currentSample->height = source[ i ];
		currentSample->materialIndex0 = 0;
		currentSample->materialIndex1 = 0;
		currentSample->clearTessFlag();
		currentByte += physicsHeightFieldDesc.samples.stride;
	}

	physicsHeightField = g_pPhysicsWorld->PhysXSDK->createHeightField(physicsHeightFieldDesc);

	FinishPhysXHeightFieldDesc( physicsHeightFieldDesc ); 

	PxHeightFieldGeometry shapeGeom(physicsHeightField, PxMeshGeometryFlags(), m_InvHFScale, WorldSize / PxReal(h), WorldSize / PxReal(w));
	PxTransform pose(PxVec3(0,0,0), PxQuat(0,0,0,1));
	physicsTerrain = g_pPhysicsWorld->PhysXSDK->createRigidStatic(pose);

	PxShape* aHeightFieldShape = physicsTerrain->createShape(shapeGeom, *g_pPhysicsWorld->defaultMaterial);

	PxFilterData filterData(PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
	aHeightFieldShape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<PHYSCOLL_STATIC_GEOMETRY, 0, 0, 0);
#if VEHICLES_ENABLED
	PxSetupDrivableShapeQueryFilterData(&qfilterData);
#endif
	aHeightFieldShape->setQueryFilterData(qfilterData);

	g_pPhysicsWorld->AddActor(*physicsTerrain);

	UpdateHFShape();
	//------------------------------------------------------------------------
}

//------------------------------------------------------------------------

static void ReloadTexture( r3dTexture* tex, int DownScale )
{
	if( tex )
	{
		char fname[ 1024 ];
		r3dscpy( fname, tex->getFileLoc().FileName );

		tex->Unload();
		tex->Load( fname, D3DFMT_UNKNOWN, DownScale );
	}
}

void
r3dTerrain::ReloadTextures()
{
	int DownScale = GetTerraTexDownScale();

	ReloadTexture( m_tBaseLayer.pMapDiffuse, DownScale );
	ReloadTexture( m_tBaseLayer.pMapNormal, DownScale );

	for ( int i = 0; i < NumMats ; i ++ )
	{
		for( int k = 0; k < TERRAIN_LAYERS_PER_MAT; k ++ )
		{
			ReloadTexture( m_dMatLayers[ i ][ k ].pMapDiffuse, DownScale );
			ReloadTexture( m_dMatLayers[ i ][ k ].pMapNormal, DownScale );
		}
	}
}

//------------------------------------------------------------------------

void
r3dTerrain::RecalcLodData()
{
	float LODProximityCoef = 1.f;

	if( r_terrain_quality->GetInt() == 1 )
	{
		LODProximityCoef = 0.633f;
	}

	__TerraLOD1 =  BaseTerraLOD1 * LODProximityCoef ;
	__TerraLOD2	=  BaseTerraLOD2 * LODProximityCoef ;
	__TerraLOD3 =  BaseTerraLOD3 * LODProximityCoef ;

	__TerraLOD1 = R3D_MAX( __TerraLOD1, 1.f );
	__TerraLOD2 = R3D_MAX( __TerraLOD2, __TerraLOD1 + 1.f );
	__TerraLOD3 = R3D_MAX( __TerraLOD3, __TerraLOD2 + 1.f );

	if( 1 == r_terrain_quality->GetInt() )
	{
		ConstructLodDescs( m_dLods, dLodSteps_PC_LOW, CellGridSize );
	}
	else
	{
		ConstructLodDescs( m_dLods, dLodSteps_PC, CellGridSize );
	}

	ChangeLodParam();
}

//------------------------------------------------------------------------

static r3dTL::TArray< uint16_t > gTempIndicesBuffers[ 4 ];
static r3dTL::TArray< uint8_t > gOrientMap;

void
r3dTerrain::FillTileSplitIndexes( uint16_t* Indices, int Step, int TileX, int TileY, uint32_t BaseIdxOffset )
{
	uint32_t countPerTile		= CellGridSize * CellGridSize * 2 / 4 / Step / Step;;
	uint32_t idxCountPerTile	= GetIndexCountPerTile( CellGridSize, Step );

	for( uint32_t	i = 0, e = sizeof gTempIndicesBuffers / sizeof gTempIndicesBuffers[ 0 ]; 
		i < e; 
		i ++ )
	{
		gTempIndicesBuffers[ i ].Resize( idxCountPerTile );
	}

	gOrientMap.Resize( countPerTile );

	TerraFaceOrientStats stats = { 0 };

	CalculateTileFaceOrients( &gOrientMap[ 0 ], TileX, TileY, CellGridSize, Step, stats );

	uint8_t* orient = &gOrientMap[ 0 ];

	uint32_t vertIdx = 0;
	uint32_t lineVertIdx = 0;

	uint16_t *targets [ 4 ] = {	&gTempIndicesBuffers[ 0 ][ 0 ], 
								&gTempIndicesBuffers[ 1 ][ 0 ],
								&gTempIndicesBuffers[ 2 ][ 0 ],
								&gTempIndicesBuffers[ 3 ][ 0 ] };

	for( uint32_t f = 0, fe = CellGridSize * CellGridSize / Step / Step; f < fe; orient ++ )
	{
		for( uint32_t i = 0; i < 8 && f < fe; i += 4, vertIdx += Step, lineVertIdx +=Step, f ++ )
		{
			if( lineVertIdx == CellGridSize )
			{
				lineVertIdx = 0;
				vertIdx += ( Step - 1 ) * ( CellGridSize + 1 )  + 1 ;
			}

			uint8_t source = *orient >> i & 0x03;

			uint16_t* indTarget = targets[ source ];

			*indTarget++ = vertIdx;
			*indTarget++ = vertIdx + ( CellGridSize + 1 ) * Step ;
			*indTarget++ = vertIdx + Step;

			targets[ source ] += 3;

			source = *orient >> ( i + 2 ) & 0x03;

			indTarget = targets[ source ];

			*indTarget++ = vertIdx + ( CellGridSize + 1 ) * Step ;
			*indTarget++ = vertIdx + ( CellGridSize + 1 ) * Step + Step ;
			*indTarget++ = vertIdx + Step;

			targets[ source ] += 3;
		}
	}

	uint32_t xcount = targets[ 0 ] - &gTempIndicesBuffers[ 0 ][ 0 ];
	uint32_t ycount = targets[ 1 ] - &gTempIndicesBuffers[ 1 ][ 0 ];
	uint32_t zcount = targets[ 2 ] - &gTempIndicesBuffers[ 2 ][ 0 ];
	uint32_t mcount = targets[ 3 ] - &gTempIndicesBuffers[ 3 ][ 0 ];

	r3d_assert( xcount <= gTempIndicesBuffers[ 0 ].Count () &&
				ycount <= gTempIndicesBuffers[ 1 ].Count () &&
				zcount <= gTempIndicesBuffers[ 2 ].Count () &&
				mcount <= gTempIndicesBuffers[ 3 ].Count () );

	r3d_assert( stats.XOrient		== xcount / 3 &&
				stats.YOrient		== ycount / 3 && 
				stats.ZOrient		== zcount / 3 &&
				stats.MultiOrient	== mcount / 3 );

	r3d_assert( xcount + ycount + zcount + mcount == idxCountPerTile );

	if( Indices )
	{
		memcpy( Indices + 0,						&gTempIndicesBuffers[ 0 ][ 0 ], xcount * 2 );
		memcpy( Indices + xcount,					&gTempIndicesBuffers[ 1 ][ 0 ], ycount * 2 );
		memcpy( Indices + xcount + ycount,			&gTempIndicesBuffers[ 2 ][ 0 ], zcount * 2 );
		memcpy( Indices + xcount + ycount + zcount,	&gTempIndicesBuffers[ 3 ][ 0 ], mcount * 2 );
	}

	int nTile = REMAP_OFFSET( TileX, TileY, m_nTileCountX );

	TileOrientData& targetOrient = TileOrients[ nTile ];

	targetOrient.XOffset	= BaseIdxOffset + ( TileX + TileY * m_nTileCountX ) * idxCountPerTile;
	targetOrient.YOffset	= targetOrient.XOffset + xcount;
	targetOrient.ZOffset	= targetOrient.YOffset + ycount;
	targetOrient.MOffset	= targetOrient.ZOffset + zcount;
	targetOrient.XCount		= xcount / 3;
	targetOrient.YCount		= ycount / 3;
	targetOrient.ZCount		= zcount / 3;
	targetOrient.MCount		= mcount / 3;

	if( !!xcount + !!ycount + !!zcount + !!mcount > 1 )
	{		
		targetOrient.Orient = MULT_ORIENT;
	}
	else
	{
		if( xcount )
		{
			targetOrient.Orient = X_ORIENT;
		}
		else
		if( ycount )
		{
			targetOrient.Orient = Y_ORIENT;
		}
		else
		if( zcount )
		{
			targetOrient.Orient = Z_ORIENT;
		}
	}
}

//------------------------------------------------------------------------

static int ConstructConnectionIndices( uint16_t* target, int step, int prevStep, int sideLodConnections )
{
	uint16_t* start = target;

	enum
	{
		CELL_VERT_SIZE = CellGridSize + 1
	};

	r3d_assert( !( step % prevStep ) );

	const int TO_NEXT_LINE			= CELL_VERT_SIZE * step ;

	bool NorthAndWest		= sideLodConnections & NORTH_CONNECTION && sideLodConnections & WEST_CONNECTION ;
	bool NotNorthAndNotWest	= !(sideLodConnections & NORTH_CONNECTION) && !(sideLodConnections & WEST_CONNECTION) ;

	bool SouthAndEast		= sideLodConnections & SOUTH_CONNECTION && sideLodConnections & EAST_CONNECTION ;
	bool NotSouthAndNotEast	= !(sideLodConnections & SOUTH_CONNECTION) && !(sideLodConnections & EAST_CONNECTION) ;

	// inside
	{
		int vidx = TO_NEXT_LINE + step;

		for( uint32_t i = step, e = CellGridSize - step; i < e; i += step )
		{
			for( uint32_t j = step, e = CellGridSize - step; j < e; j += step, vidx += step )
			{
				*target ++ = vidx ;
				*target ++ = vidx + TO_NEXT_LINE ;
				*target ++ = vidx + step ;

				*target ++ = vidx + TO_NEXT_LINE ;
				*target ++ = vidx + TO_NEXT_LINE + step ;
				*target ++ = vidx + step;
			}

			vidx += 2 * step + CELL_VERT_SIZE * ( step - 1 ) + 1;
		}
	}


	// construct north connection
	if( sideLodConnections & NORTH_CONNECTION )
	{
		int vidx = 0;

		// exterior 0

		if( !NorthAndWest )
		{
			for( int i = 0, e = step; i < e; i += prevStep )
			{
				*target ++ = vidx + i ;
				*target ++ = vidx + TO_NEXT_LINE ;
				*target ++ = vidx + prevStep + i ;
			}
		}

		vidx += step;

		// interior
		for( int i = step, e = CellGridSize - step; i < e; i += step, vidx += step )
		{
			for( int i = 0; i < step ; i += prevStep )
			{
				*target ++ = vidx + i ;
				*target ++ = vidx + TO_NEXT_LINE ;
				*target ++ = vidx + prevStep + i ;
			}

			*target ++ = vidx + step ;
			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx + TO_NEXT_LINE + step ;
		}

		// exterior 1

		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + i ;
			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx + prevStep + i ;
		}
	}
	else
	{
		int vidx = 0;

		// exterior 0
		if( !NotNorthAndNotWest )
		{
			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx + TO_NEXT_LINE + step ;
			*target ++ = vidx + step ;
		}

		vidx += step;

		// interior
		for( int i = step; i < CellGridSize - step; i += step, vidx += step )
		{
			*target ++ = vidx ;
			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx + step ;

			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx + TO_NEXT_LINE + step ;
			*target ++ = vidx + step ;
		}

		// exterior 1
		*target ++ = vidx ;
		*target ++ = vidx + TO_NEXT_LINE ;
		*target ++ = vidx + step ;
	}

	// construct east connection
	if( sideLodConnections & EAST_CONNECTION )
	{
		int vidx = CELL_VERT_SIZE - 1;

		// exterior 0

		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + i * CELL_VERT_SIZE ;
			*target ++ = vidx + TO_NEXT_LINE - step ;
			*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE ;
		}

		vidx += TO_NEXT_LINE;

		// interior
		for( int i = step, e = CellGridSize - step; i < e; i += step, vidx += TO_NEXT_LINE )
		{
			*target ++ = vidx - step;
			*target ++ = vidx + TO_NEXT_LINE - step ;
			*target ++ = vidx ;

			for( int i = 0; i < step; i += prevStep )
			{
				*target ++ = vidx + i * CELL_VERT_SIZE ;
				*target ++ = vidx + TO_NEXT_LINE - step ;
				*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE  ;
			}
		}

		// exterior 1

		if( !SouthAndEast )
		{
			for( int i = 0, e = step; i < e; i += prevStep )
			{
				*target ++ = vidx + i * CELL_VERT_SIZE ;
				*target ++ = vidx - step + TO_NEXT_LINE ;
				*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE ;
			}
		}
	}
	else
	{
		int vidx = CELL_VERT_SIZE - 1;

		*target ++ = vidx ;
		*target ++ = vidx + TO_NEXT_LINE - step ;
		*target ++ = vidx + TO_NEXT_LINE ;

		vidx += TO_NEXT_LINE;

		// interior
		for( int i = step; i < CellGridSize - step; i += step, vidx += TO_NEXT_LINE )
		{
			*target ++ = vidx - step ;
			*target ++ = vidx + TO_NEXT_LINE - step ;
			*target ++ = vidx ;

			*target ++ = vidx + TO_NEXT_LINE - step ;
			*target ++ = vidx + TO_NEXT_LINE ;
			*target ++ = vidx ;
		}

		*target ++ = vidx - step;
		*target ++ = vidx + TO_NEXT_LINE - step;
		*target ++ = vidx ;		
	}


	// construct south connection
	if( sideLodConnections & SOUTH_CONNECTION )
	{
		int vidx = CellGridSize * CELL_VERT_SIZE ;

		// exterior 0
		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + i ;
			*target ++ = vidx + prevStep + i ;
			*target ++ = vidx - TO_NEXT_LINE + step ;
		}

		vidx += step;

		// interior
		for( int i = step, e = CellGridSize - step; i < e; i += step, vidx += step )
		{
			*target ++ = vidx ;
			*target ++ = vidx - TO_NEXT_LINE + step ;
			*target ++ = vidx - TO_NEXT_LINE ;

			for( int i = 0 ; i < step; i += prevStep )
			{
				*target ++ = vidx + i ;
				*target ++ = vidx + i + prevStep ;
				*target ++ = vidx - TO_NEXT_LINE + step ;
			}
		}

		// exterior 1

		if( !SouthAndEast ) 
		{
			for( int i = 0, e = step; i < e; i += prevStep )
			{
				*target ++ = vidx + prevStep + i ;
				*target ++ = vidx - TO_NEXT_LINE + step ;
				*target ++ = vidx + i ;
			}
		}
	}
	else
	{
		int vidx = CellGridSize * CELL_VERT_SIZE ;

		// exterior 0
		*target ++ = vidx ;
		*target ++ = vidx + step ;
		*target ++ = vidx - TO_NEXT_LINE + step ;

		vidx += step;

		// interior
		for( int i = step; i < CellGridSize - step; i += step, vidx += step )
		{
			*target ++ = vidx ;
			*target ++ = vidx + step ;
			*target ++ = vidx - TO_NEXT_LINE + step ;

			*target ++ = vidx ;
			*target ++ = vidx - TO_NEXT_LINE + step ;
			*target ++ = vidx - TO_NEXT_LINE ;
		}

		// exterior 1

		if( !NotSouthAndNotEast )
		{
			*target ++ = vidx - TO_NEXT_LINE ;
			*target ++ = vidx ;
			*target ++ = vidx - TO_NEXT_LINE + step ;
		}
	}

	// construct west connection
	if( sideLodConnections & WEST_CONNECTION )
	{
		int vidx = 0;

		// exterior 0

		if( !NorthAndWest )
		{
			for( int i = 0, e = step; i < e; i += prevStep )
			{
				*target ++ = vidx + i * CELL_VERT_SIZE ;
				*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE ;
				*target ++ = vidx + step ;
			}
		}

		vidx += TO_NEXT_LINE;

		// interior
		for( int i = step, e = CellGridSize - step; i < e; i += step, vidx += TO_NEXT_LINE )
		{
			for( int i = 0; i < step ; i += prevStep )
			{
				*target ++ = vidx + i * CELL_VERT_SIZE ;
				*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE  ;
				*target ++ = vidx + step ;
			}

			*target ++ = vidx + step;
			*target ++ = vidx + step * CELL_VERT_SIZE;
			*target ++ = vidx + TO_NEXT_LINE + step ;
		}

		// exterior 1
		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + i * CELL_VERT_SIZE ;
			*target ++ = vidx + ( i + prevStep ) * CELL_VERT_SIZE ;
			*target ++ = vidx + step ;
		}

	}
	else
	{
		int vidx = 0;

		*target ++ = vidx + step ;
		*target ++ = vidx + TO_NEXT_LINE ;
		*target ++ = vidx + TO_NEXT_LINE + step ;

		vidx += TO_NEXT_LINE;

		// interior
		for( int i = step; i < CellGridSize - step; i += step, vidx += TO_NEXT_LINE )
		{
			*target ++ = vidx + step ;
			*target ++ = vidx ;
			*target ++ = vidx + TO_NEXT_LINE ;

			*target ++ = vidx + TO_NEXT_LINE + step ;
			*target ++ = vidx + step ;
			*target ++ = vidx + TO_NEXT_LINE ;
		}

		*target ++ = vidx + step;
		*target ++ = vidx ;
		*target ++ = vidx + TO_NEXT_LINE ;
	}

	if( NorthAndWest )
	{
		int vidx = 0;

		// exterior 0

		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + i + prevStep ;
			*target ++ = vidx + i ;
			*target ++ = vidx + CELL_VERT_SIZE * ( i + prevStep ) ;
		}

		for( int i = 0, e = step - prevStep; i < e; i += prevStep )
		{
			*target ++ = vidx + i + prevStep ;
			*target ++ = vidx + CELL_VERT_SIZE * ( i + prevStep );
			*target ++ = vidx + CELL_VERT_SIZE * ( i + 2 * prevStep );
		}

		*target ++ = vidx + step ;
		*target ++ = vidx + TO_NEXT_LINE ;
		*target ++ = vidx + TO_NEXT_LINE + step ;
	}

	if( SouthAndEast )
	{
		int vidx = ( CellGridSize - step + 1 ) * CELL_VERT_SIZE - step - 1 ;

		*target ++ = vidx + step ;
		*target ++ = vidx ;
		*target ++ = vidx + TO_NEXT_LINE ;

		for( int i = 0, e = step; i < e; i += prevStep )
		{
			*target ++ = vidx + CELL_VERT_SIZE * i + step ;
			*target ++ = vidx + TO_NEXT_LINE + i ;
			*target ++ = vidx + TO_NEXT_LINE + i + prevStep ;
		}

		for( int i = 0, e = step - prevStep; i < e; i += prevStep )
		{
			*target ++ = vidx + CELL_VERT_SIZE * ( i + prevStep ) + step ;
			*target ++ = vidx + CELL_VERT_SIZE * i + step ;
			*target ++ = vidx + TO_NEXT_LINE + i + prevStep ;
		}
	}

	if( NotNorthAndNotWest )
	{
		int vidx = 0 ;

		*target ++ = vidx ;
		*target ++ = vidx + TO_NEXT_LINE ;
		*target ++ = vidx + step ;
	}

	if( NotSouthAndNotEast )
	{
		int vidx = ( CellGridSize - step + 1 ) * CELL_VERT_SIZE - step - 1 ;

		*target ++ = vidx + step + TO_NEXT_LINE;
		*target ++ = vidx + step ;
		*target ++ = vidx + TO_NEXT_LINE ;
	}

	return target - start;
}

//------------------------------------------------------------------------

bool CreateWorkPath(char* dest);

void r3dTerrain::CreateTempFolders( char* dest ) 
{
	for( char* p = m_szName ; *p ; p ++ )
	{
		if( *p == '\\' )
		{
			CreateWorkPath(dest);

			int lastChar = strlen( dest ) ;
			int toAppend = p - m_szName ;
			memcpy( dest + lastChar, m_szName, toAppend ) ;

			*( dest + lastChar + toAppend ) = 0 ;
			mkdir( dest ) ;
		}
	}

	CreateWorkPath(dest);
	strcat( dest, m_szName );
	mkdir( dest );
}

const char* 
r3dTerrain::IndexDataCacheName()
{
	char dest[1024];

	CreateTempFolders( dest ) ;

	return Va( "%s/terrain.tileorient", dest);
}

void
r3dTerrain::UpdateTileSplitIndexes( int TileX, int TileZ )
{
	r3d_assert( g_bEditMode ) ;

	int idxCountPerTile = GetIndexCountPerTile( CellGridSize , m_dLods[ 0 ].nStep );	


	int offsetFromTileSplitStart = idxCountPerTile * ( TileX + TileZ * m_nTileCountX ) ;

	FillTileSplitIndexes( &EditSplitIndices[ offsetFromTileSplitStart ], m_dLods[ 0 ].nStep, TileX, TileZ, TileOrients[ 0 ].XOffset );

	if( r_terrain_quality->GetInt() > 1 )
	{
		uint16_t* lock = (uint16_t*)m_IB->Lock( TileOrients[ 0 ].XOffset + offsetFromTileSplitStart, idxCountPerTile );

		memcpy( lock, &EditSplitIndices[ offsetFromTileSplitStart ], idxCountPerTile * 2 ) ;

		m_IB->Unlock();
	}
}

void
r3dTerrain::RecreateVertexBuffer()
{
	LastQLInit = r_terrain_quality->GetInt() ;

	SAFE_DELETE( m_HeightNormalVB ) ;

	int nTotalVert = GetTotalVertexCount( m_nTileCountX, m_nTileCountZ, CellGridSize );

	m_HeightNormalVB = new r3dVertexBuffer( nTotalVert, 
		
		r_terrain_quality->GetInt() == 1 ? sizeof( HeightNormalVertLQ ) : sizeof( HeightNormalVert )
		
		, 0, false, g_bEditMode ? true : false, TerrainBufferMem );

	r3d_assert( m_HeightNormalVB );
}

void
r3dTerrain::RecreateIndexData()
{
	r3dOutToLog("TERRAIN: RecreateIndexData\n");
	RecalcLodData();	

	int nOverTilesSize = N_OVERTILES * N_OVERTILES;

	int bLQTerrain = r_terrain_quality->GetInt() == 1 ;

	SAFE_DELETE( m_MorphMaskVB );

	// create VB morph mask
	m_MorphMaskVB = new r3dVertexBuffer( m_nVertPerTile * LODDESC_COUNT * nOverTilesSize, sizeof( MorphWeightVert_t ), 0, false, false, TerrainBufferMem );
	r3d_assert( m_MorphMaskVB );

	MorphWeightVert_t * pVMorph;

	pVMorph = ( MorphWeightVert_t * )m_MorphMaskVB->Lock( 0, m_nVertPerTile * LODDESC_COUNT * nOverTilesSize );

	MorphWeightVert_t * pVM = pVMorph;
	for ( int i = 0; i < LODDESC_COUNT; i++ )
	{
		MorphWeightVert_t * pSrc = pVMorph;

		for ( int z = 0; z <= CellGridSize; z++ )
		{
			for ( int x = 0; x <= CellGridSize; x++, pVMorph++ )
			{
				pVMorph->fWeight = 0;

				int nValX = x % m_dLods[ i ].nStep;
				int nValZ = z % m_dLods[ i ].nStep;
				if (	( nValX || nValZ ) && 
					x && x < CellGridSize &&
					z && z < CellGridSize )
					pVMorph->fWeight = 1.f;
			}
		}

		for ( int i = 0; i < nOverTilesSize - 1; i++ )
		{
			memcpy( pVMorph, pSrc, m_nVertPerTile * sizeof( MorphWeightVert_t * ) );
			pVMorph += m_nVertPerTile;
		}
	}

	m_MorphMaskVB->Unlock();

	//
	int nIndexCount = 0;

	for( int k = 0; k < LODDESC_COUNT; k++ )
	{
		for( int c = 0; c < TERRA_CONNECTION_TYPE_COUNT; c ++ )
		{
			nIndexCount += m_dLods[ k ].nTriCount[ c ] * 3;
		}
	}

	nIndexCount += N_OVERTILES * N_OVERTILES * m_dLods[ LODDESC_COUNT - 1 ].nTriCount[ 0 ] * 3 ;

	SAFE_DELETE( m_IB );

	r3d_assert( m_dLods[ 0 ].nTriCount[ 0 ] * 3 == GetIndexCountPerTile( CellGridSize, m_dLods[ 0 ].nStep ) );

	if( !bLQTerrain )
	{
		nIndexCount += m_dLods[ 0 ].nTriCount[ 0 ] * 3 * m_nTileCountX * m_nTileCountZ ;
	}

	m_IB = new r3dIndexBuffer( nIndexCount, false, 2, TerrainBufferMem );


	uint16_t * pStart;
	uint16_t * pInd;

	pStart = pInd = ( uint16_t * )m_IB->Lock();

	for( int k = 0; k < LODDESC_COUNT; k++ )
	{
		int nStep = m_dLods[ k ].nStep;

		int nVertPerLine = ( CellGridSize + 1 ) * nStep;
		int nCountInLine = CellGridSize / nStep;

		uint16_t* pLodIndStart = pInd;

		r3d_assert( pInd - pStart == m_dLods[ k ].nIndexOffset[ 0 ] );

		for( int i = 0; i < nCountInLine; i++ )
		{
			for( int j = 0; j < nCountInLine; j++ )
			{
				*pInd++ = i * nVertPerLine + (j * nStep );
				*pInd++ = i * nVertPerLine + (j * nStep ) + nVertPerLine;
				*pInd++ = i * nVertPerLine + (j * nStep ) + nStep;

				*pInd++ = i * nVertPerLine + (j * nStep ) + nVertPerLine;
				*pInd++ = i * nVertPerLine + (j * nStep ) + nVertPerLine + nStep;
				*pInd++ = i * nVertPerLine + (j * nStep ) + nStep;
			}
		}

		r3d_assert( pInd - pLodIndStart == m_dLods[ k ].nTriCount[ 0 ] * 3 );

		if( k )
		{
			for( int i = 1; i < TERRA_CONNECTION_TYPE_COUNT; i ++ )
			{
				r3d_assert( pInd - pStart == m_dLods[ k ].nIndexOffset[ i ] );

				int count = ConstructConnectionIndices(	pStart + m_dLods[ k ].nIndexOffset[ i ], m_dLods[ k ].nStep, m_dLods[ k - 1 ].nStep, i );

				r3d_assert( count == m_dLods[ k ].nTriCount[ i ] * 3 );

				pInd += count;
			}
		}
	}

	m_dOvertileOffset = pInd - pStart;
	{
		int nStep = m_dLods[ LODDESC_COUNT - 1 ].nStep;

		int nVertPerLine = ( CellGridSize + 1 ) * nStep;
		int nCountInLine = CellGridSize / nStep;

		for ( int m = 0; m < N_OVERTILES * N_OVERTILES; m++ )
		{
			for( int i = 0; i < nCountInLine; i++ )
			{
				for( int j = 0; j < nCountInLine; j++ )
				{
					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep );
					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep ) + nVertPerLine;
					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep ) + nStep;

					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep ) + nVertPerLine;
					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep ) + nVertPerLine + nStep;
					*pInd++ = m * m_nVertPerTile + i * nVertPerLine + (j * nStep ) + nStep;
				}
			}
		}
	}

	int startOffset = pInd - pStart;
	int idxCountPerTile = GetIndexCountPerTile( CellGridSize , m_dLods[ 0 ].nStep );

	int splitIndicesCount = idxCountPerTile * m_nTileCountX * m_nTileCountZ ;

	r3d_assert( startOffset + 
					splitIndicesCount * !bLQTerrain
						==  m_IB->GetItemCount() );

	uint16_t* splitIndicesBufferStart = pStart + startOffset ;

	r3dOutToLog("splitIndicesCount=%d\n", splitIndicesCount);

	uint32_t ParanoidCookie = 0xCEE1AA4 ;

	struct
	{
		void operator() ( int count, uint32_t cookie )
		{
			target->Resize( count + 2 ) ;		
			*(uint32_t*)&( *target )[ count ] = cookie ;

			allocated = 1 ;
		}

		int allocated ;
		UShorts* target ;
	} allocateTempIndices = { 0, &EditSplitIndices } ;

	if( g_bEditMode )
	{
		allocateTempIndices( splitIndicesCount, ParanoidCookie ) ;
	}

	uint16_t* firstAttemptTarget ; 
	
	if( g_bEditMode )
	{
		firstAttemptTarget = &EditSplitIndices[ 0 ] ;
	}
	else
	{
		firstAttemptTarget = bLQTerrain ? 0 : splitIndicesBufferStart ;
	}

	if( !ReadIndexDataCache( firstAttemptTarget, splitIndicesCount ) )
	{
		if( !g_bEditMode )	
		{
			allocateTempIndices( splitIndicesCount, ParanoidCookie ) ;
		}

		r3dOutToLog( "Recreating terrain tile orient data..." );

		TileOrients.Resize( m_nTileCountX * m_nTileCountZ );

		float delta = 0.05f / m_nTileCountZ ;

		uint16_t * pSplitIndicesStart = &EditSplitIndices[ 0 ] ;

		for( uint32_t j = 0, e = m_nTileCountZ; j < e; j ++ )
		{
			for( uint32_t i = 0, e = m_nTileCountX; i < e; i ++ )
			{
				uint16_t * offset = pSplitIndicesStart + idxCountPerTile * ( i + j * m_nTileCountX );
				FillTileSplitIndexes( offset, m_dLods[ 0 ].nStep, i, j, startOffset );
			}

			AdvanceLoadingProgress( delta );
		}

		r3dOutToLog( "done\n" );

		WriteIndexDataCache( pSplitIndicesStart, splitIndicesCount );
	}

	r3d_assert( !allocateTempIndices.allocated || *(uint32_t*)&EditSplitIndices[ splitIndicesCount ] == ParanoidCookie ) ;

	if( !bLQTerrain && allocateTempIndices.allocated )
	{
		// copy from 'edit indices' array
		memcpy( splitIndicesBufferStart, &EditSplitIndices[ 0 ], splitIndicesCount * 2 ) ;
	}

	if( !g_bEditMode )
	{
		// this frees memory
		UShorts().Swap( EditSplitIndices ) ;
	}
	else
	{
		// cut off debug cookie
		EditSplitIndices.Resize( splitIndicesCount ) ;
	}

	m_IB->Unlock();
}   

const int cacheVersion = -54;

bool
r3dTerrain::WriteIndexDataCache(uint16_t* startData, uint32_t elementsCount)
{
	//SharedDevice shared; (void) shared;

	void* ptr = startData;
	uint32_t dataLen = m_IB->GetDataLength();
	FILE* tof = fopen(IndexDataCacheName(), "wb");	
	if(!tof) 
	{
		//m_pIB->Unlock();
		return false;
	}

	fwrite(&cacheVersion, sizeof(cacheVersion), 1, tof);
	fwrite(&heightFieldDataCRC32, sizeof(heightFieldDataCRC32), 1, tof);
	
	int terrain_quality = r_terrain_quality->GetInt();
	fwrite(&terrain_quality, sizeof(terrain_quality), 1, tof);
	//	fwrite(&m_dOvertileOffset, sizeof(m_dOvertileOffset), 1, tof);	

	int count = TileOrients.Count();
	fwrite(&count, sizeof(count), 1, tof);
	fwrite(&TileOrients[0], sizeof(TileOrients[0]), TileOrients.Count(), tof);

	fwrite(&elementsCount, sizeof(elementsCount), 1, tof);
	fwrite(ptr, sizeof(uint16_t), elementsCount, tof);
	fclose(tof);
	//	m_pIB->Unlock();

	return true;
}

bool
r3dTerrain::ReadIndexDataCache(uint16_t* startData, uint32_t elementsCount)
{
	if(r3d_access(IndexDataCacheName(), 0) != 0)
		return false;

	void* ptr = startData;

	FILE* tof = fopen(IndexDataCacheName(), "rb");
	int tag;
	fread(&tag, sizeof(tag), 1, tof);
	if(tag != cacheVersion)
	{
		fclose(tof);
		return false;
	}

	unsigned long storedCRC32;
	fread(&storedCRC32, sizeof(storedCRC32), 1, tof);

	if(storedCRC32 != heightFieldDataCRC32)
	{
		fclose(tof);
		return false;
	}

	int terrain_quality;
	fread(&terrain_quality, sizeof(terrain_quality), 1, tof);

	// cache is quality independent now ( where indices are split, we always have step 1 )
#if 1
	// lq 1 built caches are sheisse but sometimes soon this will be fixed.
	if(terrain_quality == 1 && r_terrain_quality->GetInt() != 1)
	{
		fclose(tof);
		return false;
	}
#endif

	int count;
	fread(&count, sizeof(count), 1, tof);
	TileOrients.Resize(count);
	fread(&TileOrients[0], sizeof(TileOrients[0]), TileOrients.Count(), tof);

	uint32_t dataLen;
	fread(&dataLen, sizeof(dataLen), 1, tof);
	if(dataLen != elementsCount)
	{

		fclose(tof);
		return false;
	}

	if( ptr )
	{
		fread(ptr, sizeof(uint16_t), elementsCount, tof);
	}

	fclose(tof);

	return true;
}

//------------------------------------------------------------------------

void
r3dTerrain::SetupHFScale()
{
	float heightScale = R3D_MAX( fabs( m_MaxHeight ), fabs( m_MinHeight ) );

	if(heightScale == 0.0f)
		heightScale = 1.0f;

	m_HFScale = 32767.0f / heightScale;
	m_InvHFScale = 1.f / m_HFScale;
}

//------------------------------------------------------------------------

PxHeightFieldGeometry r3dTerrain::GetHFShape() const
{
	r3d_assert( physicsTerrain );
	r3d_assert( physicsTerrain->getNbShapes () == 1 );
	PxShape* shapes[1] = {0};
	physicsTerrain->getShapes(&shapes[0], 1);
	r3d_assert( shapes[0]->getGeometryType() == PxGeometryType::eHEIGHTFIELD );
	PxHeightFieldGeometry geom;
	bool res = shapes[0]->getHeightFieldGeometry(geom);
	r3d_assert(res);
	return geom;
}

//------------------------------------------------------------------------


void
r3dTerrain::EditorSplatOn( int matIdx )
{
	r3dTexture *& SplatTex = EditorMatSplatTex[ matIdx ];

	int SWidth, SHeight ;

	// default values...
	SWidth = MaterialDataWidth ;
	SHeight = MaterialDataHeight ;

	for( uint32_t i = 0, e = SplatTextures::COUNT ; i < e ; i ++ )
	{
		if( r3dTexture* tex = EditorMatSplatTex[ i ] )
		{
			SWidth = tex->GetWidth();
			SHeight = tex->GetHeight();

			break ;
		}
	}

	if( !SWidth || !SHeight )
	{
		SWidth = Width ;
		SHeight = Height ;
	}

	if( !SplatTex )
	{
		SplatTex = r3dRenderer->AllocateTexture();
		SplatTex->Create( SWidth, SHeight, D3DFMT_A8R8G8B8, 1);
	}

	MatSplatTex[ matIdx ] = EditorMatSplatTex[ matIdx ];

	NumMats = R3D_MAX( matIdx+1, NumMats );
}

//------------------------------------------------------------------------

void r3dTerrain::EditorSplatOff( int matIdx )
{
	MatSplatTex[ matIdx ] = NULL;
}

//------------------------------------------------------------------------

bool
r3dTerrain::IsSplatOn( int matIdx )
{
	return MatSplatTex[ matIdx ] ? true : false;
}

//--------------------------------------------------------------------------------------------------------
void r3dTerrain::ChangeLodParam()
{
	m_dLods[ 0 ].fDist = 0;
	m_dLods[ 1 ].fDist = __TerraLOD1 * CellGridSize * CellSize;
	m_dLods[ 2 ].fDist = __TerraLOD2 * CellGridSize * CellSize;
	m_dLods[ 3 ].fDist = __TerraLOD3 * CellGridSize * CellSize;
}

//--------------------------------------------------------------------------------------------------------

namespace
{
	void check_fread( void* p, size_t size, r3dFile* fin )
	{
		size_t num_read = fread( p, size, 1, fin );
		r3d_assert( num_read == 1 );
	}
}

void r3dTerrain::ReadColorData(r3dFile* file, int dwVersion, int mapItemCount, bool seekFromEndOfFile )
{

#ifndef WO_SERVER
	// read color data
	SAFE_DELETE_ARRAY( m_pColorData );
	m_pColorData	= new uint32_t[ mapItemCount ];
	uint32_t memSize = mapItemCount * sizeof(uint32_t);
	r3d_assert(m_pColorData);

	if( seekFromEndOfFile )
	{
		fseek( file, -(int)memSize, SEEK_END ) ;
	}

	if( m_pColorData )
	{
		if ( dwVersion > 0 )
		{
			fread( m_pColorData, memSize, 1, file );
		}
		else
		{
			memset( m_pColorData, 0xff, memSize );
		}
	}
	else
	{
		if ( dwVersion > 0 )
		{
			fseek( file, memSize, SEEK_CUR );
		}
	}
#endif
}

void r3dTerrain::LoadFormatVersion_0(r3dFile* file)
{
	size_t elemCount = Width * Height;
	HeightFieldData.Resize( elemCount );
	check_fread( &HeightFieldData[0], HeightFieldData.Count() * sizeof( HeightFieldData[0] ), file );
	UpdatePhysHeightField();
}

void r3dTerrain::LoadFormatVersion_1(r3dFile* file)
{
	LoadFormatVersion_0(file);
}

HeightNormalVert* r3dTerrain::LoadFormatVersion_2_3_Begin(r3dFile* file)
{
	check_fread( &m_MinHeight, sizeof m_MinHeight, file );
	check_fread( &m_MaxHeight, sizeof m_MaxHeight, file );

	SetupHFScale();

	PxU32 size = GetHeightFieldDataSize( physicsHeightFieldDesc );

	Bytes data( size );
	check_fread( &data[0], size, file );
	UpdatePhysHeightField( (PxU32*)&data[0] );

	uint32_t tilesCount;
	check_fread( &tilesCount, sizeof tilesCount, file );
	m_dTiles.Resize( tilesCount );
	TileLODs.Resize( tilesCount );

	check_fread( &m_dTiles[0], sizeof m_dTiles[0] * tilesCount, file );

#ifndef WO_SERVER
	HeightNormalVert* locked;
	{
		R3D_ENSURE_MAIN_THREAD();
		locked = (HeightNormalVert*)m_HeightNormalVB->Lock( 0, m_HeightNormalVB->GetItemCount() );
	}
	return locked;
#endif
	return 0;
}

void r3dTerrain::LoadFormatVersion_2_3_End()
{
#ifndef WO_SERVER
	{
		R3D_ENSURE_MAIN_THREAD();
		m_HeightNormalVB->Unlock();
	}
#endif
}

void r3dTerrain::LoadFormatVersion_2_Main(r3dFile* file, HeightNormalVert* locked)
{
#ifndef WO_SERVER
	check_fread( locked, m_HeightNormalVB->GetDataLength(), file );

	int normalDataSize = sizeof NormalMapData[0] * (int)Width * (int)Height;
	fseek( file, normalDataSize, SEEK_CUR );
#endif
}

void r3dTerrain::LoadFormatVersion_3_Main(r3dFile* file, HeightNormalVert* locked)
{
#ifndef WO_SERVER
	int count = m_HeightNormalVB->GetItemCount();
	PackedTerrainVert_t* packedVerts = new PackedTerrainVert_t[count];

	check_fread( packedVerts, count * sizeof(PackedTerrainVert_t) , file );

	for( uint32_t i = 0, e = count; i < e; i ++ )
	{
		FromPackedVert( locked[ i ], packedVerts[ i ], m_MinHeight, m_MaxHeight );
	}

	delete [] packedVerts;
#endif
}


void r3dTerrain::LoadFormatVersion_2(r3dFile* file)
{
	HeightNormalVert* locked = LoadFormatVersion_2_3_Begin(file);
	LoadFormatVersion_2_Main(file, locked);
	LoadFormatVersion_2_3_End();
}

void r3dTerrain::LoadFormatVersion_3(r3dFile* file)
{
	HeightNormalVert* locked = LoadFormatVersion_2_3_Begin(file);
	LoadFormatVersion_3_Main(file, locked);
	LoadFormatVersion_2_3_End();	
}

void r3dTerrain::LoadFormatVersion_4(r3dFile* file)
{
	size_t elemCount = (size_t)Width * Height;
	Shorts hfData( elemCount );

	check_fread( &m_MinHeight, sizeof m_MinHeight, file );
	check_fread( &m_MaxHeight, sizeof m_MinHeight, file );

	SetupHFScale();

	check_fread( &hfData[0], hfData.Count() * sizeof( hfData[0] ), file );
	heightFieldDataCRC32 = r3dCRC32((const BYTE *)&hfData[0], hfData.Count() * sizeof( hfData[0] ));
	CreatePhysicsData( hfData );
}

void r3dTerrain::LoadFormatVersion_5(r3dFile* file)
{
	LoadFormatVersion_4(file);

	int tileLayersCount;
	fread( &tileLayersCount, sizeof(tileLayersCount), 1, file );
	TileLayers.Resize(tileLayersCount);
	fread( &TileLayers[0], sizeof(Layers), tileLayersCount, file );
}

void r3dTerrain::LoadFormatVersion_X(r3dFile* file, uint32_t dwVersion)
{
	switch( dwVersion )
	{
	case 0:
	case 1:
	case 3:
		CreateDefaultPhysicsData();
		break;
	}

	// read heightmap
	switch( dwVersion )
	{
	case 0:	LoadFormatVersion_0(file); break;
	case 1:	LoadFormatVersion_1(file); break;
	case 2: LoadFormatVersion_2(file); break;
	case 3: LoadFormatVersion_3(file); break;
	case 4: LoadFormatVersion_4(file); break;
	case 5: LoadFormatVersion_5(file); break;
	default:
		r3dError( "r3dTerrain::InitData: unknown .heightmap version!" );
	}
}

void r3dTerrain::UpdateVertexDataFormat_X(int dwVersion)
{
	// read heightmap
	switch( dwVersion )
	{
	case 0:	
	case 1:
	case 2:
	case 4: UpdateVertexDataFormat_0_1_2_4(); break;
	case 3: UpdateVertexDataFormat_3(); break;
	case 5: UpdateVertexDataFormat_5(); break;
	}
}

void r3dTerrain::UpdateVertexDataFormat_3()
{
	UpdateTileArrayFromSplatTextures();
}

void r3dTerrain::UpdateVertexDataFormat_5()
{
	int nCount = Width / CellGridSize * Height / CellGridSize;
	m_dTiles.Resize( nCount );
	TileLODs.Resize( nCount );

	ExtractHeightFieldData();
	UpdateAllVertexData();

	if(g_bEditMode)
	{
		UpdateTileArrayFromSplatTextures();
	}
}

void r3dTerrain::UpdateVertexDataFormat_0_1_2_4()
{
	int nCount = Width / CellGridSize * Height / CellGridSize;
	m_dTiles.Resize( nCount );
	TileLODs.Resize( nCount );

	ExtractHeightFieldData();
	UpdateAllVertexData();
	UpdateTileArrayFromSplatTextures();
}

void r3dTerrain::RecalcParams()
{
	m_nTileCountX = ( Width ) / CellGridSize;
	m_nTileCountZ = ( Height ) / CellGridSize;
	m_nVertPerTile = ( CellGridSize + 1 ) * ( CellGridSize + 1 );
}

r3dFile*
r3dTerrain::OpenTerrainBinFile()
{
	return r3d_open( Va( FNAME_TERRAIN_HEIGHTMAP, m_szName ), "rb") ;
}

void r3dTerrain::InitData()
{
	r3dOutToLog("TERRAIN: InitData\n");
	// 	RecalcLodData();

	SetCellSize( CellSize );

	bNeedUpdatePhysics = false;
	bInHeightEditMode = false;

	m_MinHeight = 0.f;
	m_MaxHeight = 1000.f;

	MakeSceneBox();

	RecalcParams();

	// create VB HeightNormalVert
	uint32_t mapItemCount = Width * Height;

	HeightFieldData.Clear();
	LayerData.Clear();

	MaterialDataWidth = 0 ;
	MaterialDataHeight = 0 ;


#ifndef WO_SERVER
	RecreateVertexBuffer();

	NormalMapData	= new r3dPoint3D[ mapItemCount ];
	r3d_assert(NormalMapData);
	memset( NormalMapData, 0, sizeof NormalMapData[ 0 ] * mapItemCount );
#endif

	r3dFile *f1 = OpenTerrainBinFile();
	r3d_assert( f1 );

	uint32_t dwSignature;
	uint32_t dwVersion = 0;

	ReadTerrainHeader(f1, dwSignature, dwVersion);
	LoadFormatVersion_X(f1, dwVersion);

	// NOTE : Color data should be always stored at the end of file!
	ReadColorData(f1, dwVersion, mapItemCount, false );

	fclose (f1);

#ifndef WO_SERVER

	DefaultInitVertexPos();
	CreateVertexDeclaration();
	UpdateVertexDataFormat_X(dwVersion);
	RecreateIndexData();
	UpdateMaterials( 0, 0, m_nTileCountX, m_nTileCountZ );

#endif

	// free it (just in case...)
	if( !g_bEditMode )
	{
		Floats().Swap( HeightFieldData );
		delete [] NormalMapData;
		delete [] m_pColorData;

		r3dTL::TArray< r3dPoint3D >().Swap(	NormalBuf );

		NormalMapData	= NULL;
		m_pColorData	= NULL;
	}

#ifndef WO_SERVER

	if( !g_bEditMode )
	{
		for( int i = 0, e = SplatTextures::COUNT; i < e; i ++ )
		{
			if( EditorMatSplatTex[ i ] )
			{
				r3dRenderer->DeleteTexture( EditorMatSplatTex[ i ] );
				EditorMatSplatTex[ i ] = NULL;
			}
		}
	}

#endif

	UpdateDesc() ;

	bLoaded = 1;
}

void r3dTerrain::MakeSceneBox()
{
	// make scene boxes bigger for a huge level
	extern float gSceneBox_LevelBase;
	extern unsigned int gSceneBox_MinObjCount;
	if(WorldSize > 10000.0f)
	{
		gSceneBox_LevelBase = 30.0f;
		gSceneBox_MinObjCount = 5;
	}
	if(WorldSize > 20000.0f)
	{
		gSceneBox_LevelBase = 60.0f;
		gSceneBox_MinObjCount = 10;
	}
}

void r3dTerrain::CreateVertexDeclaration()
{
	D3DVERTEXELEMENT9 dclVert[] =
	{
		{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,		0 },
		{ 1, 0,		D3DDECLTYPE_SHORT2N,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,		0 },
		{ 1, 4,		D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_COLOR,			0 },
		{ 1, 8,		D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_COLOR,			1 },
		{ 2, 0,		D3DDECLTYPE_FLOAT1,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,		1 },
		D3DDECL_END()
	};

	r3dDeviceTunnel::CreateVertexDeclaration( dclVert, &m_HeightNormalVDecl );

	D3DVERTEXELEMENT9 dclVertLQ[] =
	{
		{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,		0 },
		{ 1, 0,		D3DDECLTYPE_UBYTE4,		D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,		0 },
		{ 1, 4,		D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_COLOR,			0 },
		D3DDECL_END()
	};

	r3dDeviceTunnel::CreateVertexDeclaration( dclVertLQ, &m_HeightNormalVDeclLQ );
}

void r3dTerrain::DefaultInitVertexPos()
{
	r3dOutToLog("TERRAIN: DefaultInitVertexPos\n");
	m_VB = new r3dVertexBuffer( N_OVERTILES * N_OVERTILES * m_nVertPerTile, sizeof( TerrainPosVert_t ), 0, false, false, TerrainBufferMem );
	r3d_assert( m_VB );

	TerrainPosVert_t * pV = (TerrainPosVert_t *)m_VB->Lock() ;

	int iTileOffsetX = 0;
	int iTileOffsetZ = 0;

	for ( int m = 0; m < N_OVERTILES * N_OVERTILES; m++ )
	{
		for( int i = 0; i <= CellGridSize; i++ )
		{
			for( int j = 0; j <= CellGridSize; j++, pV++ )
			{
				pV->vPos.Assign( iTileOffsetX * CellGridSize + j, 0, iTileOffsetZ * CellGridSize + i );
			}
		}

		iTileOffsetX++;
		if ( iTileOffsetX >= N_OVERTILES )
		{
			iTileOffsetX = 0;
			iTileOffsetZ++;
		}
	}

	m_VB->Unlock();
}

//--------------------------------------------------------------------------------------------------------
inline void FixUInt32( uint32_t & val )
{
	val = ( ( val >> 24 ) & 0x000000ff ) | ( ( val >> 8 ) & 0x0000ff00 ) | ( ( val << 8 ) & 0x00ff0000 ) | ( ( val << 24 ) & 0xff000000 );
};

//--------------------------------------------------------------------------------------------------------
inline void FixInt32( int32_t & val )
{
	val = ( ( val >> 24 ) & 0x000000ff ) | ( ( val >> 8 ) & 0x0000ff00 ) | ( ( val << 8 ) & 0x00ff0000 ) | ( ( val << 24 ) & 0xff000000 );
};

//--------------------------------------------------------------------------------------------------------
inline void FixUInt16( uint16_t & val )
{
	val = ( ( val >> 8 ) & 0x00ff ) | ( ( val << 8 ) & 0xff00 ) ;
};

//--------------------------------------------------------------------------------------------------------
inline void FixInt16( int16_t & val )
{
	val = ( ( val >> 8 ) & 0x00ff ) | ( ( val << 8 ) & 0xff00 ) ;
};


union FloatIntConverter_t
{
	uint32_t	m_int32;
	float		m_float;
};

//--------------------------------------------------------------------------------------------------------
inline void FixFloat( float & val )
{
	FloatIntConverter_t c;
	c.m_float = val;
	FixUInt32( c.m_int32 );
	val = c.m_float;
};

void PrintSplatLocation( char (& buf ) [ 1024 ], int idx )
{
	if( idx )
	{
		_snprintf( buf, 1024, "Terrain\\Mat-Splat%d.dds", idx );
	}
	else
	{
		_snprintf( buf, 1024, "Terrain\\Mat-Splat.dds" );
	}
}

void PrintSplatLocationPS3( char (& buf ) [ 1024 ], const char * szSourceFile )
{
	char sDir [512];
	char sName [512];
	char sExt [512];

	_splitpath ( szSourceFile, NULL, sDir, sName, sExt );
	sprintf ( buf, "%s%s%s%s", sDir, sName, "_ps3", sExt );
}


void r3dTerrain::SaveData( const char * fileName, bool writeCache )
{
	FILE * hFile;

	mkdir( ( r3dString( fileName ) + "\\Terrain\\" ).c_str() ) ;

	r3d_assert(_CrtCheckMemory());

	hFile = fopen_for_write( Va( FNAME_TERRAIN_HEIGHTMAP, fileName ), "wb");
	r3d_assert( hFile );
	if ( ! hFile )
		return;

	uint32_t dwSignature = TERRAIN_SIGNATURE;
	uint32_t dwVersion = TERRAIN_VERSION;

	fwrite( &dwSignature, sizeof( dwSignature ), 1, hFile );
	fwrite( &dwVersion, sizeof( dwVersion ), 1, hFile );

	r3d_assert( physicsHeightField );

	r3d_assert(_CrtCheckMemory());

	uint32_t hmSize = GetHeightFieldDataSize( physicsHeightFieldDesc );

	uint32_t paranoidHMSize = hmSize + 16;

	r3d_assert(_CrtCheckMemory());
	// give extra and check if we 'guessed' the size right
	Bytes sampleData( paranoidHMSize );

	r3d_assert(_CrtCheckMemory());

	uint32_t numWritten = physicsHeightField->saveCells( &sampleData[0], paranoidHMSize );

	r3d_assert( numWritten == hmSize );

	fwrite( &m_MinHeight, sizeof m_MinHeight, 1, hFile );
	fwrite( &m_MaxHeight, sizeof m_MaxHeight, 1, hFile );

	int sampleCount = hmSize / physicsHeightFieldDesc.samples.stride;
	char* byteSamplePtr = (char*)&sampleData[0];

	r3dTL::TArray< PxI16 > hfShrinkedSamples( sampleCount );

	for( int i = 0, e = sampleCount; i < e; i ++, byteSamplePtr += physicsHeightFieldDesc.samples.stride )
	{
		PxHeightFieldSample* s = (PxHeightFieldSample*)byteSamplePtr;
		hfShrinkedSamples[ i ] = s->height;
	}

	fwrite( &hfShrinkedSamples[0], hfShrinkedSamples.Count() * sizeof hfShrinkedSamples[0], 1, hFile );
	heightFieldDataCRC32 = r3dCRC32((const BYTE *)&hfShrinkedSamples[0], hfShrinkedSamples.Count() * sizeof( hfShrinkedSamples[0] ));

	if( !TileLayers.Count() )
	{
		UpdateTileArrayFromSplatTextures();
	}

	int tileLayersCount = TileLayers.Count();
	fwrite( &tileLayersCount, sizeof(tileLayersCount), 1, hFile );
	fwrite( &TileLayers[0], sizeof(Layers), tileLayersCount, hFile );

	// NOTE : Color data should be always stored at the end of file!
	fwrite( m_pColorData, sizeof(uint32_t), Width*Height, hFile );

	fclose(hFile);	

	hFile = fopen_for_write( Va( FNAME_TERRAIN_INI, fileName ), "wt");
	assert( hFile );
	if ( ! hFile )
		return;

	fprintf( hFile, "size: \t\t%f\n", Width );
	fprintf( hFile, "cell_size: \t%f\n", CellSize );
	fprintf( hFile, "height: \t%f\n", m_HeightmapSize );

	fprintf( hFile, "normal_scl: \t%f\n", m_NormalmapScale );
	fprintf( hFile, "hm_blend: \t%f\n", __HeightmapBlend );
	fprintf( hFile, "split_dist: \t%f\n", SplitDistance );
	fprintf( hFile, "lod: \t\t%f, %f, %f\n", BaseTerraLOD1, BaseTerraLOD2, BaseTerraLOD3 );

	fprintf( hFile, "tiling_mode: \t%s\n", TilingModeToString( m_eTilingMode ) );
	fprintf( hFile, "shader_lod_fade_start: \t%.3f\n", m_fShaderLODFadeStart );
	fprintf( hFile, "shader_lod_fade_end: \t%.3f\n", m_fShaderLODFadeEnd );
	fprintf( hFile, "camera_near_plane: \t%.2f\n", r_near_plane->GetFloat() );
	fprintf( hFile, "camera_far_plane: \t%.2f\n", r_far_plane->GetFloat() );

	fprintf( hFile, "\nbase_layer\n" );
	SaveLayerToScript( hFile, m_tBaseLayer, "" );

	for ( int i = 0; i < NumMats; i++ )
	{
		fprintf( hFile, "\nmaterial\n{\n" );

		bool savedSplat = false;

		bool layersHaveTextuers = false;

		for( int j = 0; j < TERRAIN_LAYERS_PER_MAT; j ++ )
		{
			if( m_dMatLayers[ i ][ j ].pMapDiffuse || 
				m_dMatLayers[ i ][ j ].pMapNormal )
			{
				layersHaveTextuers = true;

				break;
			}
		}

		if( MatSplatTex[ i ] && layersHaveTextuers )
		{
			savedSplat = true;

			r3dTexture* splatTex = MatSplatTex[ i ];

			if( splatTex->getFileLoc().FileName[0] == '$' )
			{
				char tempStr[ 1024 ];
				PrintSplatLocation( tempStr, i );
				char fullPath[ 1024 ];
				r3dscpy( fullPath, fileName );
				strcat( fullPath, "\\" );
				strcat( fullPath, tempStr );
				splatTex->OverwriteFileLocation( fullPath );
			}

			char tempStr[ 1024 ];
			PrintSplatLocation( tempStr, i );

			int levelNameLen = strlen( fileName );

			size_t levelPathLen = strlen( fileName ) + 1;

			fprintf( hFile, "\tmap_splat_mask: \t\"%s\"\n", tempStr );

			splatTex->Save( ( r3dString( fileName ) + "\\" + tempStr ).c_str() );

			r3dTexture * pTempTex = r3dRenderer->LoadTexture( splatTex->getFileLoc().FileName, D3DFMT_DXT5, true );
			if ( pTempTex )
			{
				char sFullPath [ 1024 ];

				PrintSplatLocationPS3( sFullPath, splatTex->getFileLoc().FileName );

				pTempTex->Save(sFullPath, true);

				fprintf( hFile, "\tmap_splat_mask_ps3: \t\"%s\"\n", sFullPath + levelPathLen );
			}

			r3dRenderer->DeleteTexture(pTempTex);
			pTempTex = NULL;
		}

		if( !savedSplat 
				&& 
			// are we per chance autosaving? Do not delete in case that's so
			!strcmp( r3dGameLevel::GetHomeDir(), r3dGameLevel::GetSaveDir() ) )
		{
			char tempStr[ 1024 ];
			char relativeStr[ 1024 ];
			PrintSplatLocation( relativeStr, i );
			sprintf(tempStr,"%s/%s", r3dGameLevel::GetHomeDir(), relativeStr );

			if( !r3d_access( tempStr, 0 ) )
			{
				r3dOutToLog( "r3dTerrain::SaveData: removing unused %s.\n", tempStr );
				remove( tempStr );
			}

			char tempStrPS3[ 1024 ];

			PrintSplatLocationPS3( tempStrPS3, tempStr );

			if( !r3d_access( tempStrPS3, 0 ) )
			{
				r3dOutToLog( "r3dTerrain::SaveData: removing unused %s.\n", tempStrPS3 );
				remove( tempStrPS3 );
			}

			r3dRenderer->DeleteTexture( EditorMatSplatTex[ i ] );
			MatSplatTex[ i ] = EditorMatSplatTex[ i ] = NULL;
		}

		for ( int j = 0; j < TERRAIN_LAYERS_PER_MAT; j++ )
		{
			fprintf( hFile, "\n\tlayer\n" );
			SaveLayerToScript( hFile, m_dMatLayers[ i ][ j ], "\t" );
		}

		fprintf( hFile, "}" );
	}

	fclose( hFile );

	int idxCountPerTile = GetIndexCountPerTile( CellGridSize , m_dLods[ 0 ].nStep );
	int iCount = idxCountPerTile * m_nTileCountX * m_nTileCountZ;

	if( writeCache )
	{
		R3D_ENSURE_MAIN_THREAD();

		r3d_assert( g_bEditMode ) ;
		r3d_assert( EditSplitIndices.Count() == iCount ) ;

		WriteIndexDataCache( &EditSplitIndices[0], iCount );
	}
}

//------------------------------------------------------------------------

void
r3dTerrain::SaveDataPS3	( const char * fileName )
{
	FILE * hFile;

	hFile = fopen_for_write( Va( FNAME_TERRAIN_HEIGHTMAP, fileName ), "wb");
	r3d_assert( hFile );
	if ( ! hFile )
		return;

	ExtractHeightFieldData();

	uint32_t dwSignature = TERRAIN_SIGNATURE;
	uint32_t dwVersion = TERRAIN_VERSION_PS3;

	fwrite( &dwSignature, sizeof( dwSignature ), 1, hFile );
	fwrite( &dwVersion, sizeof( dwVersion ), 1, hFile );

	float val = m_MinHeight;
	ChangeEndianess( val );
	fwrite( &val, sizeof val, 1, hFile );

	val = m_MaxHeight;
	ChangeEndianess( val );
	fwrite( &val, sizeof val, 1, hFile );

	r3d_assert( physicsHeightField );

	uint32_t hmSize = GetHeightFieldDataSize( physicsHeightFieldDesc );

	uint32_t paranoidHMSize = hmSize + 16;

	// give extra and check if we 'guessed' the size right
	Bytes sampleData( paranoidHMSize );

	uint32_t numWritten = physicsHeightField->saveCells( &sampleData[0], paranoidHMSize );
	r3d_assert( numWritten == hmSize );

	r3dTL::TArray< PxI16 > shortSamples;
	shortSamples.Resize( physicsHeightFieldDesc.nbColumns * physicsHeightFieldDesc.nbRows );

	uint32_t sampleCount = shortSamples.Count();
	ChangeEndianess( sampleCount );
	fwrite( &sampleCount, sizeof sampleCount, 1, hFile );

	PxHeightFieldSample* hfs = (PxHeightFieldSample*)&sampleData[ 0 ];
	for( uint32_t i = 0, e = shortSamples.Count(); i < e; i ++, hfs ++ )
	{
		shortSamples[ i ] = hfs->height;
		ChangeEndianess( shortSamples[ i ] );
	}

	fwrite( &shortSamples[0], sizeof( PxI16 ) * shortSamples.Count(), 1, hFile );

	TerrainTiles tiles;

	UpdateVertexDataSettings sts;

	sts.CellGridDim = CellGridSizePS3;
	sts.TileCountX	= ( Width ) / CellGridSizePS3;
	sts.TileCountZ	= ( Height ) / CellGridSizePS3;

	sts.UpdateTileXStart	= 0;
	sts.UpdateTileXEnd		= sts.TileCountX - 1;

	sts.UpdateTileZStart	= 0;
	sts.UpdateTileZEnd		= sts.TileCountZ - 1;

	sts.Tiles				= &tiles;
	sts.Tiles->Resize( sts.TileCountX * sts.TileCountZ );

	TerrainLodDesc desc[ LODDESC_COUNT ];
	ConstructLodDescs( desc, dLodSteps_PS3, CellGridSizePS3 );

	sts.dLods				= &desc;

	sts.RemapOffset			= RemapOffsetPS3;

	uint32_t VERT_COUNT = GetTotalVertexCount( sts.TileCountX, sts.TileCountZ, sts.CellGridDim );

	sts.LockedBuffer		= new HeightNormalVert[ VERT_COUNT ];
	sts.LockOffset			= 0;

	UpdateVertexData( sts );

	uint8_t LOD_STEPS = LODDESC_COUNT ;
	fwrite( &LOD_STEPS, 1, 1, hFile );

	uint16_t gridSize = CellGridSizePS3 ;
	ChangeEndianess( gridSize );
	fwrite( &gridSize, sizeof gridSize, 1, hFile );

	for( int i = 0, e = LODDESC_COUNT; i < e; i ++ )
	{
		int val = dLodSteps_PS3[ i ];
		ChangeEndianess( val );
		fwrite( &val, sizeof val, 1, hFile );
	}

	uint32_t tilesCount = tiles.Count();
	ChangeEndianess( tilesCount );
	fwrite( &tilesCount, sizeof tilesCount, 1, hFile );

	for( uint32_t i = 0, e = tiles.Count(); i < e; i ++ )
	{
		TerrainTile_c& tile = tiles[ i ];

		ChangeEndianess( tile.m_HeightMin );
		ChangeEndianess( tile.m_HeightMax );

		fwrite( &tile, sizeof tile, 1, hFile );
	}

	HeightNormalVert* fullVert = (HeightNormalVert*)sts.LockedBuffer;
	PackedPS3Vert_t* packedPS3Vert = new PackedPS3Vert_t[ VERT_COUNT ];
	for( uint32_t i = 0, e = VERT_COUNT; i < e; i ++ )
	{
		ToPackedPS3Vert(  packedPS3Vert[ i ], fullVert[ i ], m_MinHeight, m_MaxHeight );
	}

	fwrite( packedPS3Vert, sizeof packedPS3Vert[ 0 ] * VERT_COUNT, 1, hFile );

	delete [] fullVert;
	delete [] packedPS3Vert;

	TileLayerArr tileLayers;

	UpdateTileArrayFromSplatTextures( tileLayers, sts.TileCountX, sts.TileCountZ, CellGridSizePS3, RemapOffsetPS3 );

	{
		uint32_t count = tileLayers.Count();
		ChangeEndianess( count );
		fwrite( &count, sizeof count, 1, hFile );
	}

	for( uint32_t i = 0, e = tileLayers.Count(); i < e; i ++ )
	{
		// up to MAX_SHADER_LAYERS on PS3
		fwrite( &tileLayers[ i ][ 0 ], MAX_SHADER_LAYERS * sizeof tileLayers[ i ][ 0 ], 1, hFile );		
	}

	r3d_assert( g_bEditMode ); // if this fails you should prolly make terrain vbuffer perma readable

	uint32_t countPerTile = CellGridSizePS3 * CellGridSizePS3 * 2 / 4;

	r3dTL::TArray< uint8_t > OrientData( countPerTile * sts.TileCountX * sts.TileCountZ );
	r3dTL::TArray< uint8_t > TileOrientData( sts.TileCountX * sts.TileCountZ );

	TerraFaceOrientStats stats = { 0 };

	{
		uint32_t count = OrientData.Count();
		ChangeEndianess( count );
		fwrite( &count, sizeof count, 1, hFile );
	}

	for( uint32_t j = 0, t = 0, e = sts.TileCountZ; j < e; j ++ )
	{
		for( uint32_t i = 0, e = sts.TileCountX; i < e; i ++, t += countPerTile )
		{
			TerraFaceOrientStats tstats;

			CalculateTileFaceOrients( &OrientData[ t ], i, j, CellGridSizePS3, 1, tstats );

			uint8_t& tileOrientData = TileOrientData[ i + j * sts.TileCountX ];

			r3d_assert( tstats.FullXOrient + tstats.FullYOrient + tstats.FullZOrient + tstats.FullMultiOrient <= 1 );

			tileOrientData = MIXED;

			if( tstats.FullXOrient )		tileOrientData = X_ORIENT;
			if( tstats.FullYOrient )		tileOrientData = Y_ORIENT;
			if( tstats.FullZOrient )		tileOrientData = Z_ORIENT;
			if( tstats.FullMultiOrient )	tileOrientData = MULT_ORIENT;

			stats.XOrient			+= tstats.XOrient;
			stats.YOrient			+= tstats.YOrient;
			stats.ZOrient			+= tstats.ZOrient;
			stats.MultiOrient		+= tstats.MultiOrient;

			stats.FullXOrient		+= tstats.FullXOrient;
			stats.FullYOrient		+= tstats.FullYOrient;
			stats.FullZOrient		+= tstats.FullZOrient;
			stats.FullMultiOrient	+= tstats.FullMultiOrient;
		}
	}

	fwrite( &OrientData[ 0 ], OrientData.Count(), 1, hFile );

	uint32_t count = TileOrientData.Count();

	ChangeEndianess( count );

	fwrite( &count, sizeof count, 1, hFile );

	fwrite( &TileOrientData[ 0 ], TileOrientData.Count(), 1, hFile );

	fclose( hFile );

}

//--------------------------------------------------------------------------------------------------------
void r3dTerrain::SaveLayerToScript( FILE * hFile, const Layer_t &layer, char * tab )
{
	fprintf( hFile, "%s{\n", tab );

	if( layer.pMapDiffuse )
	{
		fprintf( hFile, "%s\tmap_diffuse: \t\"%s\"\n", tab, layer.pMapDiffuse->getFileLoc().FileName );
	}

	if( layer.pMapNormal )
	{
		fprintf( hFile, "%s\tmap_normal: \t\"%s\"\n", tab, layer.pMapNormal->getFileLoc().FileName );
	}

	fprintf( hFile, "%s\tsplit_enable: \t\t%d\n", 	tab, layer.bDoSplit		);
	fprintf( hFile, "%s\tscale: \t\t%f\n", 			tab, layer.fScale		);
	fprintf( hFile, "%s\tsplit: \t\t%f\n", 			tab, layer.fSplit		);
	fprintf( hFile, "%s\tgloss: \t\t%f\n",			tab, layer.fGloss		);
	fprintf( hFile, "%s\tmat_type: \t%s\n",			tab, layer.MatTypeName	);

	fprintf( hFile, "%s}\n", tab );
}

//--------------------------------------------------------------------------------------------------------
r3dTerrain::TilingMode_e r3dTerrain::StringToTilingMode( const char * str )
{
	if ( ! strcmp( str, "infinite" ) )
		return eTilingMode_Infinite;
	if ( ! strcmp( str, "once" ) )
		return eTilingMode_Once;
	if ( ! strcmp( str, "none" ) )
		return eTilingMode_None;

	assert( false && "unknown tiling mode" );
	return eTilingMode_None;
}

//--------------------------------------------------------------------------------------------------------
const char * r3dTerrain::TilingModeToString( r3dTerrain::TilingMode_e eMode )
{
	if ( eMode == eTilingMode_Infinite )
		return "infinite";
	if ( eMode == eTilingMode_Once )
		return "once";

	return "none";
}


void r3dTerrain::Unload()
{
	if(!bLoaded)
		return;

#ifndef WO_SERVER
	R3D_ENSURE_MAIN_THREAD();
#endif

	SAFE_DELETE_ARRAY( m_pColorData );
	SAFE_DELETE_ARRAY( NormalMapData );

	r3dRenderer->DeleteTexture(m_tBaseLayer.pMapDiffuse);
	m_tBaseLayer.pMapDiffuse = NULL;

	r3dRenderer->DeleteTexture( m_tBaseLayer.pMapNormal );
	m_tBaseLayer.pMapNormal = NULL;


	for (int i=0;i<NumMats;i++)
	{
		r3dRenderer->DeleteTexture(m_dMatLayers[i][0].pMapDiffuse);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][1].pMapDiffuse);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][2].pMapDiffuse);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][3].pMapDiffuse);

		if( EditorMatSplatTex[ i ] )
		{
			r3dRenderer->DeleteTexture( EditorMatSplatTex[ i ] );
		}

		EditorMatSplatTex[i] = NULL;
		MatSplatTex[i] = NULL;

		r3dRenderer->DeleteTexture(m_dMatLayers[i][0].pMapNormal);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][1].pMapNormal);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][2].pMapNormal);
		r3dRenderer->DeleteTexture(m_dMatLayers[i][3].pMapNormal);
	}

	SAFE_DELETE( m_VB );
	SAFE_DELETE( m_IB );
	SAFE_DELETE( m_HeightNormalVB );
	SAFE_DELETE( m_MorphMaskVB );

	bLoaded = 0;
	return;
}


int _render_Terrain = 1;

#include "hoffmanscatter.h"
extern    HoffmanScatter hoffman;



//--------------------------------------------------------------------------------------------------------
float r3dTerrain::GetLodIndex( int TileX, int TileZ )
{
	float fWidthCell = CellGridSize * CellSize;

	float fDX = fabs(( gCam.x / fWidthCell ) - (float)( TileX + 0.5 ) );
	float fDZ = fabs(( gCam.z / fWidthCell ) - (float)( TileZ + 0.5 ) );

	float fDist = sqrtf( fDX * fDX + fDZ * fDZ );

	if ( fDist > __TerraLOD3 ) 
		return 3.f;

	if ( fDist > __TerraLOD2 ) 
		return ( fDist - __TerraLOD2 ) / ( __TerraLOD3 - __TerraLOD2 ) + 2.f;

	if ( fDist > __TerraLOD1 ) 
		return ( fDist - __TerraLOD1 ) / ( __TerraLOD2 - __TerraLOD1 ) + 1.f;

	return fDist / __TerraLOD1;
}

//------------------------------------------------------------------------

R3D_FORCEINLINE void FillXVSConstants( float (&c)[ 4 ][ 4 ] )
{
	// float4 vTexcTransform0 : register( c31 );
	c[0][0] = 0.f; c[0][1] = 0.f; c[0][2] = 1.f; c[0][3] = 0.f;
	// float4 vTexcTransform1 : register( c32 );
	c[1][0] = 0.f; c[1][1] = 1.f; c[1][2] = 0.f; c[1][3] = 0.f;
	// float4 vTexcTransSel   : register( c33 );
	c[2][0] = 0.f; c[2][1] = 1.f; c[2][2] = 1.f; c[2][3] = 0.f;
	// float2 vSignSel        : register( c34 );
	c[3][0] = 1.f; c[3][1] = 0.f; c[3][2] = 0.f; c[3][3] = 0.f;
}

R3D_FORCEINLINE void FillYVSConstants( float (&c)[ 4 ][ 4 ] )
{
	// float4 vTexcTransform0 : register( c31 );
	c[0][0] = 1.f; c[0][1] = 0.f; c[0][2] = 0.f; c[0][3] = 0.f;
	// float4 vTexcTransform1 : register( c32 );
	c[1][0] = 0.f; c[1][1] = 0.f; c[1][2] = 1.f; c[1][3] = 0.f;
	// float4 vTexcTransSel   : register( c33 );
	c[2][0] = 1.f; c[2][1] = 0.f; c[2][2] = 0.f; c[2][3] = 1.f;
	// float2 vSignSel        : register( c34 );
	c[3][0] = 0.f; c[3][1] = 0.f; c[3][2] = 0.f; c[3][3] = 0.f;
}

R3D_FORCEINLINE void FillZVSConstants( float (&c)[ 4 ][ 4 ] )
{
	// float4 vTexcTransform0 : register( c31 );
	c[0][0] = 1.f; c[0][1] = 0.f; c[0][2] = 0.f; c[0][3] = 0.f;
	// float4 vTexcTransform1 : register( c32 );
	c[1][0] = 0.f; c[1][1] = 1.f; c[1][2] = 0.f; c[1][3] = 0.f;
	// float4 vTexcTransSel   : register( c33 );
	c[2][0] = 1.f; c[2][1] = 0.f; c[2][2] = 0.f; c[2][3] = 1.f;
	// float2 vSignSel        : register( c34 );
	c[3][0] = 0.f; c[3][1] = 1.f; c[3][2] = 0.f; c[3][3] = 0.f;
}

R3D_FORCEINLINE void FillMultiVSConstants( float (&c)[ 4 ][ 4 ] )
{
	// float4 vTexcTransform0 : register( c31 );
	c[0][0] = 0.f; c[0][1] = 0.f; c[0][2] = 0.f; c[0][3] = 0.f;
	// float4 vTexcTransform1 : register( c32 );
	c[1][0] = 0.f; c[1][1] = 0.f; c[1][2] = 0.f; c[1][3] = 0.f;
	// float4 vTexcTransSel   : register( c33 );
	c[2][0] = 0.f; c[2][1] = 0.f; c[2][2] = 0.f; c[2][3] = 0.f;
	// float2 vSignSel        : register( c34 );
	c[3][0] = 0.f; c[3][1] = 0.f; c[3][2] = 0.f; c[3][3] = 0.f;
}

//------------------------------------------------------------------------

inline
void
r3dTerrain::DrawTile( const VisibleTile &tile, const r3dCamera& Cam, bool SecondPass, TileRenderMode TileRMode )
{
	int TileIdx				= tile.nTile;
	int nLod				= (int)tile.fLod;
	const r3dBoundBox& bbox = tile.tTileBox;
	bool bRenderBigTile		= tile.bRenderBigTile;
	int NearTile			= tile.bNearTile;

	float fWidthCell = CellGridSize * CellSize;

#if 0
	fFrom[ 0 ] = m_dLods[ nLod ].fDist + CellGridSize * 0.5f * CellSize ;
	fSize[ 0 ] = m_dLods[ r3dTL::Min( LODDESC_COUNT - 1, nLod + 1 ) ].fDist - m_dLods[ nLod ].fDist - CellGridSize * CellSize * 0.5f;
#endif

	D3DXMATRIX m;

	D3DXMatrixScaling( &m, tile.fMirrorNormalX, 1, tile.fMirrorNormalZ );
	if ( tile.fMirrorNormalX < 0 )
		m._41 = CellGridSize;
	if ( tile.fMirrorNormalZ < 0 )
		m._43 = CellGridSize;

	m._11 *= CellSize; m._21 *= CellSize; m._31 *= CellSize; m._41 *= CellSize; m._41 += bbox.Org.x;
	m._13 *= CellSize; m._23 *= CellSize; m._33 *= CellSize; m._43 *= CellSize; m._43 += bbox.Org.z;

	float mul ;
	float add ;

	if( r_terrain_quality->GetInt() == 1 )
	{
		mul = m_MaxHeight - m_MinHeight;
		add = m_MinHeight;

		mul *= ( 1.f / 65535.f );
	}
	else
	{
		mul = 0.5 * m_MaxHeight - 0.5 * m_MinHeight;
		add = 0.5 * m_MaxHeight + 0.5 * m_MinHeight;		
	}

	D3DXVECTOR4 vConsts[] = 
	{
		// float4x3 mMirror  		: register( c25 );
		D3DXVECTOR4( m._11, m._21, m._31, m._41 ),
		D3DXVECTOR4( m._12, m._22, m._32, m._42 ),
		D3DXVECTOR4( m._13, m._23, m._33, m._43 ),
		// float4   vCamT 			: register( c28 );
		D3DXVECTOR4( bbox.Org.x, CellSize, bbox.Org.z, 0 ),
		// float4   vMorphF_HMinMax	: register( c29 );
		D3DXVECTOR4( fmodf( tile.fLod, 1.f ), mul, add, 0.f ),
		// float4   vPixOffset_NormalMapFadeK	: register( c30 );
		D3DXVECTOR4 (	tile.fMirrorNormalX * 0.5f / Width , 
		tile.fMirrorNormalZ * -0.5f / Height, 
		1.0f / ( m_fShaderLODFadeEnd - m_fShaderLODFadeStart ), 
		- m_fShaderLODFadeStart / ( m_fShaderLODFadeEnd - m_fShaderLODFadeStart ) )
	};

	D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF(  25, (float *)&vConsts[0], sizeof vConsts / sizeof vConsts[ 0 ] ) );

	d3dc._SetDecl( r_terrain_quality->GetInt() == 1 ? m_HeightNormalVDeclLQ : m_HeightNormalVDecl );

	m_HeightNormalVB->Set( 1, TileIdx * m_nVertPerTile );


	m_MorphMaskVB->Set( 2, r3dTL::Min( LODDESC_COUNT - 1, nLod + 1 ) * m_nVertPerTile * (N_OVERTILES * N_OVERTILES) );


	m_VB->Set( 0 );
	m_IB->Set();

	r3d_assert( !bRenderBigTile || nLod == LODDESC_COUNT - 1 );

	UINT VertCount		= (bRenderBigTile ? (N_OVERTILES * N_OVERTILES) : 1) * m_nVertPerTile;
	UINT IndexOffset	= (bRenderBigTile ? m_dOvertileOffset : m_dLods[ nLod  ].nIndexOffset[ tile.ConnectionBits ] );
	UINT TriangleCount	= (bRenderBigTile?(N_OVERTILES * N_OVERTILES):1) * m_dLods[ nLod ].nTriCount[ tile.ConnectionBits ] ;

	switch( TileRMode )
	{
	case TRM_SIMPLE:
		r3dRenderer->Stats.AddNumTerrainDraws( 1 );
		r3dRenderer->Stats.AddNumTerrainTris ( +TriangleCount );

		r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, VertCount, IndexOffset, TriangleCount );

		break;

	case TRM_HEAVYNESS:
		{
			r3dRenderer->Stats.AddNumTerrainDraws( 1 );
			r3dRenderer->Stats.AddNumTerrainTris ( +TriangleCount );

			const Layers & layers = TileLayers [ TileIdx ];

			int count = 0;

			while( count < Layers::COUNT && layers[ count ] != WRONG_LAYER_IDX )
			{
				count ++;
			}			

			float vConsts[ 3 ][ 4 ] = {};

			if( count > 4 )
			{
				vConsts[ 0 ][ 0 ] = 1.0f;
				vConsts[ 0 ][ 1 ] = 0.0f;
				vConsts[ 0 ][ 2 ] = 0.0f;
				vConsts[ 0 ][ 3 ] = 0.33f;
			}
			else
			{
				float heavyness = ( count - 1 ) / 3.f;

				float colr0[4] = { 0.f, 1.f, 0.f, 0.33f };
				float colr1[4] = { 1.f, 1.f, 0.f, 0.33f };

				for( int i = 0; i < 4; i ++ )
				{
					vConsts[ 0 ][ i ] = LerpFast( colr0[ i ], colr1[ i ], R3D_MIN( heavyness, 1.f ) );
				}
			}

			int tileX = bbox.Org.x / CellSize / CellGridSize;
			int tileZ = bbox.Org.z / CellSize / CellGridSize;

			if( SelectedTileX == tileX && SelectedTileZ == tileZ )
			{
				vConsts[ 0 ][ 3 ] *= 0.125f;
			}
			else
				if( HighlightedTileX == tileX && HighlightedTileZ == tileZ )
				{
					vConsts[ 0 ][ 3 ] *= 1.33f;
				}

				vConsts[ 1 ][ 0 ] = bbox.Org.x / GetTotalWidth();
				vConsts[ 1 ][ 1 ] = 1.f - bbox.Org.z / GetTotalHeight();
				vConsts[ 1 ][ 2 ] = ( bbox.Org.x  + bbox.Size.x )/ GetTotalWidth();
				vConsts[ 1 ][ 3 ] = 1.f - ( bbox.Org.z + bbox.Size.z )/ GetTotalHeight();

				vConsts[ 2 ][ 0 ] = 64.f * ( m_nTileCountX + m_nTileCountZ );

				D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( 0, vConsts[0], sizeof vConsts / sizeof vConsts[ 0 ] ) );

				r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, VertCount, IndexOffset, TriangleCount );
		}
		break;

	case TRM_SPLIT:
		{
			const Layers & layers = TileLayers [ TileIdx ];

			TerrainShaderKey key;

			key.Value = 0;

			float fWorldSizeInv = 1.f / WorldSize;

			if( r_lighting_quality->GetInt() == 1 )
			{
				key.Flags.aux_enabled = 0 ;
			}
			else
			{
				key.Flags.aux_enabled = 1 ;
			}

			enum 
			{
				// float4   vGloss                : register( c1 );
				C_GLOSS,
				// float4   vMatScale             : register( c2 );
				C_MAT_SCALE,
				// float4   vMatScaleSplit        : register( c3 );
				C_MAT_SCALE_SPLIT,
				// float4   vUseMaskAsAlpha       : register( c4 );
				C_USE_MASK_AS_ALPHA,
				// float4   vSelectMaskChannel[4] : register( c5 );
				C_MASK_CHANNEL_0,
				C_MASK_CHANNEL_1,
				C_MASK_CHANNEL_2,
				C_MASK_CHANNEL_3,

				C_COUNT
			};

			float psConsts[ C_COUNT ][4];

			memset( psConsts, 0, sizeof psConsts );

			int n = 0;
			int count = 0;

			float UseMaskAsAlpha = 0.f;

			if( SecondPass )
			{
				for( n = 0; n < MAX_SHADER_LAYERS && layers[ n ] != WRONG_LAYER_IDX && n < Layers::COUNT ; )
				{
					if( layers[ n ] != WRONG_LAYER_IDX )
					{
						n ++;		
					}
				}

				UseMaskAsAlpha = 1.f;
			}

			psConsts[ C_USE_MASK_AS_ALPHA ][ 0 ] = UseMaskAsAlpha;
			psConsts[ C_USE_MASK_AS_ALPHA ][ 1 ] = UseMaskAsAlpha;
			psConsts[ C_USE_MASK_AS_ALPHA ][ 2 ] = UseMaskAsAlpha;
			psConsts[ C_USE_MASK_AS_ALPHA ][ 3 ] = UseMaskAsAlpha;

			while( n < Layers::COUNT && layers[ n ] != WRONG_LAYER_IDX )
			{
				count = 0;

				while( n < Layers::COUNT && layers[ n ] != WRONG_LAYER_IDX && count < MAX_SHADER_LAYERS  )
				{
					Layer_t * layer;
					int idx = layers[ n ];
					int mat, lay;

					bool active = true;

					if( !idx )
						layer = &m_tBaseLayer;
					else
					{
						mat = ( idx - 1 ) / TERRAIN_LAYERS_PER_MAT;
						active = MatSplatTex[ mat ] ? 1 : 0;

						lay = ( idx - 1 ) % TERRAIN_LAYERS_PER_MAT;
						layer = &m_dMatLayers[ mat ][ lay ];
					}

					if( active )
					{
						psConsts[ C_GLOSS ][ count ]			= layer->fGloss;
						psConsts[ C_MAT_SCALE ][ count ]		= layer->fScale * fWorldSizeInv;
						psConsts[ C_MAT_SCALE_SPLIT ][ count ]	= layer->fScale * fWorldSizeInv / layer->fSplit;

#ifndef FINAL_BUILD
						if (__r3dDisplayMipLevels)
							SetTextureAsMipmapTest( layer->pMapDiffuse, count );
						else if (d_show_checker_texture->GetBool())
							SetTextureDensityChecker(count);
						else
#endif
							r3dRenderer->SetTex( layer->pMapDiffuse, count );
						r3dRenderer->SetTex( layer->pMapNormal, count + 4 );

						if( idx )
						{
							int cidx = C_MASK_CHANNEL_0 + count;
							psConsts[ cidx ][ 0 ] =	lay == 1;
							psConsts[ cidx ][ 1 ] =	lay == 2;
							psConsts[ cidx ][ 2 ] =	lay == 3;
							psConsts[ cidx ][ 3 ] =	lay == 0;
							r3dRenderer->SetTex( MatSplatTex[ mat ], count + 8 );
						}
						else
						{
							int cidx = C_MASK_CHANNEL_0;
							psConsts[ cidx ][ 0 ] =	0.f;
							psConsts[ cidx ][ 1 ] =	0.f;
							psConsts[ cidx ][ 2 ] =	0.f;
							psConsts[ cidx ][ 3 ] =	0.f;
							r3dRenderer->SetTex( NULL, 8 );
						}

						switch( count )
						{
						case 0:
							key.Flags.split0 = layer->bDoSplit;
							break;
						case 1:
							key.Flags.split1 = layer->bDoSplit;
							break;
						case 2:
							key.Flags.split2 = layer->bDoSplit;
							break;
						case 3:
							key.Flags.split3 = layer->bDoSplit;
							break;
						}					

						count ++;
					}

					n++;
				}		

				if ( count )
				{
					const TileOrientData& oriData = TileOrients[ TileIdx ];

					int MultiShaderID = -1 ;
					int SingleShaderID = -1 ;
					int LQShaderID = -1 ;

					if( g_bTerrainUseLightHack )
					{
						MultiShaderID = TerrainMinimapShaders[ count - 1 ];
						SingleShaderID = MultiShaderID;
						LQShaderID = MultiShaderID ;
					}
					else
					{
						key.Flags.num_layers	= count - 1;
						key.Flags.simple		= !NearTile ;
						key.Flags.multi			= 1;

						MultiShaderID	= TerrainShaders[ key.Value ];

						key.Flags.multi			= 0;

						SingleShaderID	= TerrainShaders[ key.Value ];

						key.Flags.split0 = 0 ;
						key.Flags.split1 = 0 ;
						key.Flags.split2 = 0 ;
						key.Flags.split3 = 0 ;

						key.Flags.lq = 1 ;

						LQShaderID	= TerrainShaders[ key.Value ];
					}

					int needLQ = r_terrain_quality->GetInt() == 1 ;

					r3d_assert( MultiShaderID >= 0 || needLQ && LQShaderID >= 0 );

					D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( 1, psConsts[0], sizeof psConsts / sizeof psConsts[ 0 ] ) );

					float vsConsts[ 4 ][ 4 ];

					bool AllowSpilt = true;
#define ALLOW_COMPARE_TERRAIN_PERFOMANCE 0

#if ALLOW_COMPARE_TERRAIN_PERFOMANCE
					AllowSpilt = !r_debug_helper->GetInt();
#endif

					if( !nLod && AllowSpilt && !needLQ )
					{
						bool singleShaderSet = false;

						if( oriData.XCount )
						{
							FillXVSConstants( vsConsts );
							D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ] ) );
							r3dRenderer->SetPixelShader( SingleShaderID );
							singleShaderSet = true;
							r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, m_nVertPerTile, oriData.XOffset, oriData.XCount );

							r3dRenderer->Stats.AddNumTerrainDraws( 1 );
						}

						if( oriData.YCount )
						{
							FillYVSConstants( vsConsts );
							D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ]  ) );

							if( !singleShaderSet )
							{
								r3dRenderer->SetPixelShader( SingleShaderID );
								singleShaderSet = true;
							}

							r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, m_nVertPerTile, oriData.YOffset, oriData.YCount );

							r3dRenderer->Stats.AddNumTerrainDraws( 1 );
						}

						if( oriData.ZCount )
						{
							FillZVSConstants( vsConsts );

							D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ]  ) );

							if( !singleShaderSet )
							{
								r3dRenderer->SetPixelShader( SingleShaderID );
								singleShaderSet = true;
							}

							r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, m_nVertPerTile, oriData.ZOffset, oriData.ZCount );

							r3dRenderer->Stats.AddNumTerrainDraws( 1 );
						}

						if( oriData.MCount )
						{
							FillMultiVSConstants( vsConsts );

							D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ]  ) );

							r3dRenderer->SetPixelShader( MultiShaderID );

							r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, m_nVertPerTile, oriData.MOffset, oriData.MCount );

							r3dRenderer->Stats.AddNumTerrainDraws( 1 );
						}
					}
					else
					{
						if( needLQ )
						{
							FillYVSConstants( vsConsts );
						}
						else
						{
							switch( oriData.Orient )
							{
							case X_ORIENT:
								FillXVSConstants( vsConsts );
								break;

							case Y_ORIENT:
								FillYVSConstants( vsConsts );
								break;

							case Z_ORIENT:
								FillZVSConstants( vsConsts );
								break;

							case MULT_ORIENT:
							case MIXED:
								FillMultiVSConstants( vsConsts );
								break;
							}
						}

						D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ]  ) );

						if( needLQ )
						{
							r3d_assert( LQShaderID > 0 );
							r3dRenderer->SetPixelShader( LQShaderID );
						}
						else
						{
							if( oriData.Orient == MULT_ORIENT || oriData.Orient == MIXED )
							{
								r3dRenderer->SetPixelShader( MultiShaderID );
							}
							else
							{
								r3dRenderer->SetPixelShader( SingleShaderID );
							}
						}

#if ALLOW_COMPARE_TERRAIN_PERFOMANCE
						if( r_debug_helper->GetInt() )
						{
							FillMultiVSConstants( vsConsts );
							r3dRenderer->SetPixelShader( MultiShaderID );
							D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 31, vsConsts[0], sizeof vsConsts  / sizeof vsConsts [ 0 ]  ) );
						}
#endif
						r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, 0, 0, VertCount, IndexOffset, TriangleCount );
						r3dRenderer->Stats.AddNumTerrainDraws( 1 );
					}

					r3dRenderer->Stats.AddNumTerrainTris ( TriangleCount );
				}

				if( !SecondPass )
					break;

			}
		}
		break;
	}

}

//--------------------------------------------------------------------------------------------------------

void r3dTerrain::DrawTerrain( const r3dCamera &Cam, bool secondPath, bool shadowRendering, TileRenderMode tileRenderMode, bool invertCulling, bool allowModifyCulling )
{
	if(!_render_Terrain) return;

	R3DPROFILE_FUNCTION("r3dTerrain::DrawTerrain");

	D3DPERF_BeginEvent(D3DCOLOR_XRGB(255,255,255), L"Draw Terrain");

	// cleanup textures( prevent d3d errors of stray cube textures
	for( int i = 0; i < 12 ; i ++ )
	{
		r3dRenderer->SetTex(NULL, i);
	}

	VisibleTiles & dVisibleTiles = m_dVisibleTiles;

	D3DCULL forwCull, backCull;

	forwCull = D3DCULL_CCW;
	backCull = D3DCULL_CW;


	if( invertCulling )
	{
		R3D_SWAP( forwCull, backCull );
	}

	if( allowModifyCulling )
		r3dRenderer->SetCullMode( forwCull );

	for ( uint32_t i = 0; i < dVisibleTiles.Count (); i++ )
	{
		if ( dVisibleTiles[i].bOvertiled )
			continue;

		const VisibleTile &tile = dVisibleTiles[i];

		const int nTile = tile.nTile;

		if( secondPath && TileLayers[ nTile ][ MAX_SHADER_LAYERS ] == WRONG_LAYER_IDX  )
			continue;

		bool needRestoreCull = false;

		if( allowModifyCulling )
		{
			if ( tile.fMirrorNormalX < 0 && tile.fMirrorNormalZ > 0 || tile.fMirrorNormalX > 0 && tile.fMirrorNormalZ < 0 )
			{
				r3dRenderer->SetCullMode( backCull );
				needRestoreCull = true;
			}
		}

		DrawTile( tile, Cam, secondPath, tileRenderMode );

		if( needRestoreCull )
		{
			r3dRenderer->SetCullMode( forwCull );
		}
	}

	// clean up textures
	for( int i = 0; i < 12 ; i ++ )
	{
		r3dRenderer->SetTex(NULL, i);
	}

	d3dc._SetStreamSource( 1, 0, 0, 0 ) ;
	d3dc._SetStreamSource( 2, 0, 0, 0 ) ;

	D3DPERF_EndEvent();
}

//------------------------------------------------------------------------

void
r3dTerrain::DEBUG_DrawTileOrder()
{
	DrawDebugPositions( mDEBUG_TileOrderPoses );
}

//------------------------------------------------------------------------

void
r3dTerrain::InitTileLayerArray( TileLayerArr& oArr, int tileCountX, int tileCountZ )
{
	oArr.Resize( tileCountX * tileCountZ );

	for( uint32_t i = 0, e = oArr.Count(); i < e; i ++ )
	{
		Layers& layers = oArr[ i ];

		layers[ 0 ] = 0;

		for( uint32_t i = 1, e = Layers::COUNT; i < e; i ++ )
		{
			layers[ i ] = WRONG_LAYER_IDX;
		}
	}

}

//------------------------------------------------------------------------

void
r3dTerrain::UpdateTileArrayFromSplatTextures( TileLayerArr& oArr, int tileCountX, int tileCountZ, int cellGridSize, int (*RemapOffset)( int , int , int ) )
{
	InitTileLayerArray( oArr, tileCountX, tileCountZ );

	for( uint32_t i = 0, e = TERRAIN_MAT_COUNT; i < e; i ++ )
	{
		uint32_t baseIdx = i * TERRAIN_LAYERS_PER_MAT + 1;

		if( r3dTexture* splat = EditorMatSplatTex[ i ] )
		{
			if(!splat->IsValid())
				continue;

			int cellTexSizeX = cellGridSize * splat->GetWidth() ;
			r3d_assert( !( cellTexSizeX % ( int ) this->Width ) ) ;
			cellTexSizeX /= ( int ) this->Width ;

			int cellTexSizeZ = cellGridSize * splat->GetHeight() ;
			r3d_assert( !( cellTexSizeZ % ( int ) this->Height ) ) ;
			cellTexSizeZ /= ( int ) this->Height ;

			r3dD3DTextureTunnel tex = splat->GetD3DTunnel();

			D3DLOCKED_RECT lrect;
			tex.LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) ;

			// otherwise 'rewrite me'
			r3d_assert( lrect.Pitch == splat->GetWidth() * 4 );

			DWORD* pixels = (DWORD*) lrect.pBits;

			for( int j = 0, e = tileCountZ; j < e; j ++ )
			{
				for( int i = 0, e = tileCountX; i < e; i ++ )
				{
					int layerPresent[ 4 ] = { 0 };

					{
						// expand so that we're good with bilinear filtering
						int sx = i * cellTexSizeX - 1;
						int sz = ( tileCountZ - j - 1 ) * cellTexSizeZ - 1;

						int ex = sx + cellTexSizeX + 2;
						int ez = sz + cellTexSizeZ + 2;

						sx = R3D_MAX( sx, 0 );
						sz = R3D_MAX( sz, 0 );

						ex = R3D_MIN( ex, cellTexSizeX * tileCountX - 1 );
						ez = R3D_MIN( ez, cellTexSizeZ * tileCountZ - 1 );

						DWORD* p = pixels + sz * cellTexSizeX * tileCountX + sx;

						for( int j = 0, e = ez - sz; j < e; j ++ )
						{
							DWORD* pp = p;
							for( int i = 0, e = ex - sx; i < e; i ++ )
							{
								DWORD val = *pp++;

								layerPresent[ 3 ] |= val & 0xff;
								layerPresent[ 2 ] |= val & 0xff00;
								layerPresent[ 1 ] |= val & 0xff0000;
								layerPresent[ 0 ] |= val & 0xff000000;
							}

							p += cellTexSizeX * tileCountX;
						}
					}

					int tileIdx = RemapOffset( i, j, tileCountX );
					Layers& layers = oArr[ tileIdx ];

					int lastIdx = 0;
					for( int i = 0, e = Layers::COUNT; i < e && layers[ i ] != WRONG_LAYER_IDX; i ++ )
					{
						lastIdx ++;
					}

					for( int i = 0, e = TERRAIN_LAYERS_PER_MAT; i < e && lastIdx < Layers::COUNT; i ++ )
					{
						if( layerPresent[ i ] )
						{
							layers[ lastIdx ++ ] = baseIdx + i ;
						}
					}
				}
			}

			tex.UnlockRect( 0 );
		}
	}
}

//------------------------------------------------------------------------

void
r3dTerrain::InitTileLayerArray()
{
	InitTileLayerArray( TileLayers, m_nTileCountX, m_nTileCountZ  );
}

//------------------------------------------------------------------------

void
r3dTerrain::UpdateTileArrayFromSplatTextures()
{
	UpdateTileArrayFromSplatTextures( TileLayers, m_nTileCountX, m_nTileCountZ, CellGridSize, RemapOffsetPC );
}

//------------------------------------------------------------------------

void
r3dTerrain::InsertLayer( int TileX, int TileZ, int Type )
{
	int offset = REMAP_OFFSET( TileX, TileZ, m_nTileCountX );
	Layers& layers = TileLayers[ offset ];

	int e = 0;
	for( ;	e < MAX_LAYERS_PER_TILE && layers[ e ] != WRONG_LAYER_IDX ; 
		e ++ );

		if( e < MAX_LAYERS_PER_TILE )
		{
			int i = 0;
			for( ;	i < e && layers[ i ] < Type; 
				i ++ );

				if( layers[ i ] == Type )
					return;

			for( int ii = MAX_LAYERS_PER_TILE - 1; ii > i; ii -- )
			{
				layers[ ii ] = layers[ ii - 1 ];
			}

			layers[ i ] = Type;
		}
}

//------------------------------------------------------------------------

void
r3dTerrain::OptimizeLayers()
{
	int splatCount = 0;
	while( MatSplatTex[ splatCount ] )
		splatCount ++;

	BYTE (*lockedSplats[ TERRAIN_MAT_COUNT ] )[ 4 ] = { 0 };
	D3DLOCKED_RECT lrect[ TERRAIN_MAT_COUNT ];

	for( int i = 0; i < splatCount; i ++ )
	{
		if(!MatSplatTex[i]->IsValid())
			return;

		D3D_V( MatSplatTex[ i ]->AsTex2D()->LockRect( 0, &lrect[ i ], NULL, 0 ) );
		lockedSplats[ i ] = ( BYTE (*)[4] )lrect->pBits;
		r3d_assert(
			sizeof lockedSplats[ 0 ] * MatSplatTex[ i ]->GetWidth() == lrect->Pitch &&
			MatSplatTex[ i ]->GetWidth() == MatSplatTex[ 0 ]->GetWidth() &&
			MatSplatTex[ i ]->GetHeight() == MatSplatTex[ 0 ]->GetHeight()
			);
	}

	for( int pidx = 0, e = MatSplatTex[ 0 ]->GetWidth() * MatSplatTex[ 0 ]->GetHeight(); pidx < e; pidx ++ )
	{
		int latestFullIdx = -1;

		for( int i = 0, e = splatCount * TERRAIN_LAYERS_PER_MAT; i < e; i ++ )
		{
			int matIdx = i / TERRAIN_LAYERS_PER_MAT;

			BYTE (&colr)[4] = lockedSplats[ matIdx ][ pidx ];
			if( colr[ 3 - i ] >= 0xf8 )
			{
				colr[ 3 - i ] = 0xff;

				latestFullIdx = i;

				// clear out preceding ones
				for( int i = latestFullIdx - 1; i >= 0; i -- )
				{
					colr[ 3 - i ] = 0;
				}

				break;
			}
		}

		int lastMaterialIdx = latestFullIdx / TERRAIN_LAYERS_PER_MAT - 1;

		for( int i = lastMaterialIdx ; i >= 0; i -- )
		{
			(DWORD&)lockedSplats[ i ][ pidx ] = 0;
		}
	}

	for( int i = 0; i < splatCount; i ++ )
	{
		D3D_V( MatSplatTex[ i ]->AsTex2D()->UnlockRect( 0 ) );
	}
}

//--------------------------------------------------------------------------------------------------------
void r3dTerrain::Update ( bool bShadowMap )
{
	extern r3dCamera gCam;

	if(!bLoaded) return;
	if(!_render_Terrain) return;

	R3DPROFILE_FUNCTION("r3dTerrain::Update");

	D3DXVECTOR3	vCamera( gCam.x, gCam.y, gCam.z );

	float fWidthCell = CellGridSize * CellSize;

	int nMinX ;
	int nMaxX ;

	int nMinZ ;
	int nMaxZ ;

	D3DXVECTOR3 zeroCorner = r3dRenderer->FrustumCorners[ 0 ];

	r3dPoint2D vMin ( zeroCorner.x, zeroCorner.z ) ;
	r3dPoint2D vMax ( zeroCorner.x, zeroCorner.z ) ;

	for( uint32_t i = 1, e = R3D_ARRAYSIZE(r3dRenderer->FrustumCorners); i < e; i ++ )
	{
		D3DXVECTOR3 v = r3dRenderer->FrustumCorners[ i ];

		vMin.x = R3D_MIN( v.x, vMin.x );
		vMin.y = R3D_MIN( v.z, vMin.y );

		vMax.x = R3D_MAX( v.x, vMax.x );
		vMax.y = R3D_MAX( v.z, vMax.y );
	}

	nMinX = vMin.x / fWidthCell - 1;
	nMaxX = vMax.x / fWidthCell + 1;

	nMinZ = vMin.y / fWidthCell - 1;
	nMaxZ = vMax.y / fWidthCell + 1;

	if ( m_eTilingMode == eTilingMode_None ) 
	{
		nMinX = r3dTL::Max( 0, nMinX );
		nMinZ = r3dTL::Max( 0, nMinZ );

		nMaxX = r3dTL::Min( (int)Width / CellGridSize, nMaxX );
		nMaxZ = r3dTL::Min( (int)Height / CellGridSize, nMaxZ );
	}
	else if ( m_eTilingMode == eTilingMode_Once ) 
	{
		nMinX = r3dTL::Max( -(int)Width / CellGridSize, nMinX );
		nMinZ = r3dTL::Max( -(int)Height / CellGridSize, nMinZ );

		nMaxX = r3dTL::Min( 2 * (int)Width / CellGridSize, nMaxX );
		nMaxZ = r3dTL::Min( 2 * (int)Height / CellGridSize, nMaxZ );
	}

	int nCountX = Width / CellGridSize;
	int nCountZ = Height / CellGridSize;

	// this will help us disable connections with invisible tiles
	memset( &TileLODs[ 0 ], LODDESC_COUNT, TileLODs.Count() );

	// not per frame allocations, because of m_dVisibleTiles prealllocated previously
	m_dVisibleTiles.Clear ();

	static r3dTL::TArray <int> g_dMapTiles;

	bool bValidIndexes = ( nMaxX > nMinX ) && ( nMaxZ > nMinZ );

	if ( bValidIndexes )
	{
		m_dVisibleTiles.Reserve ( (nMaxZ - nMinZ)*(nMaxX - nMinX) );
		g_dMapTiles.Resize ( m_nTileCountX * m_nTileCountZ );
		memset( &g_dMapTiles.GetFirst (), 0x00, m_nTileCountX * m_nTileCountZ * sizeof ( g_dMapTiles[0] ) );
	}

	// if we render tiles from near to far ( without sort tiles )

	if(!bShadowMap)
	{
		mDEBUG_TileOrderPoses.Clear();

		{
			static r3dBitMaskArray g_dBitArray;
			if ( bValidIndexes )
				g_dBitArray.Resize ( (nMaxZ - nMinZ)*(nMaxX - nMinX) );
			g_dBitArray.ZeroAll ();
#define TILE_INDEX(in_x,in_z)(in_x)-nMinX + ((in_z)-nMinZ)*(nMaxX - nMinX)
			// render tiles by direction of camera
			int nCameraTileX = ( int )( vCamera.x / fWidthCell );
			int nCameraTileZ = ( int )( vCamera.z / fWidthCell );

			r3dPoint2D vCameraTilePos ( nCameraTileX * fWidthCell + fWidthCell * 0.5f, nCameraTileZ * fWidthCell + fWidthCell * 0.5f );

			r3dPoint2D vCameraDirProj (gCam.vPointTo.x, gCam.vPointTo.z);

			if ( vCameraDirProj.LengthSq() < 0.000001f )
				vCameraDirProj = r3dPoint2D(1,0);

			vCameraDirProj.Normalize();
			r3dPoint2D vCameraDirProjNorm ( vCameraDirProj.y, -vCameraDirProj.x );

			int nStartTileX = vCameraDirProj.x >= 0.0f ? nMinX - 1 : nMaxX + 1 ;
			int nStartTileZ = vCameraDirProj.y >= 0.0f ? nMinZ - 1 : nMaxZ + 1 ;

			int nEndTileX = vCameraDirProj.x >= 0.0f ? nMaxX + 1 : nMinX - 1 ;
			int nEndTileZ = vCameraDirProj.y >= 0.0f ? nMaxZ + 1 : nMinZ - 1 ;

			r3dPoint2D vStartTilePos ( nStartTileX * fWidthCell + fWidthCell * 0.5f, nStartTileZ * fWidthCell + fWidthCell * 0.5f );
			r3dPoint2D vEndTilePos ( nEndTileX * fWidthCell + fWidthCell * 0.5f, nEndTileZ * fWidthCell + fWidthCell * 0.5f );

			r3dPoint2D vStartProjTilePos = vStartTilePos.Project ( vCameraTilePos, vCameraDirProj );
			r3dPoint2D vEndProjTilePos = vEndTilePos.Project ( vCameraTilePos, vCameraDirProj );

			int iStepCount = int((vStartProjTilePos - vEndProjTilePos).Length() / (fWidthCell* 0.5f) ) + 1;

			r3dPoint2D vProjTilePos = vStartProjTilePos;

			for ( int i = 0; i < iStepCount; i++ )
			{
				r3dPoint2D vPt = vProjTilePos;
				// iterate
				int iTileX = int(vPt.x / fWidthCell) - ((vPt.x < 0.0f) ? 1 : 0 );
				int iTileZ = int(vPt.y / fWidthCell) - ((vPt.y < 0.0f) ? 1 : 0 );
				r3dPoint2D vCurTilePos;

				bool bCorrect = true;
				int iStepSirection = 1;

				float fMirrorNormalX;
				float fMirrorNormalZ;

				while ( bCorrect )
				{
					if ((iTileX >= nMinX && iTileX < nMaxX) && (iTileZ >= nMinZ && iTileZ < nMaxZ))
					{
						if ( !g_dBitArray.Test ( TILE_INDEX(iTileX,iTileZ) ) )
						{
							g_dBitArray.Set ( TILE_INDEX(iTileX,iTileZ) );


							int nZ = floorf( ( float ) iTileZ / nCountZ );
							int nX = floorf( ( float ) iTileX / nCountX );
							int nValidZ = abs( iTileZ < 0 ? iTileZ + 1 : iTileZ ) % nCountZ;
							int nValidX = abs( iTileX < 0 ? iTileX + 1 : iTileX ) % nCountX;

							if ( nZ & 0x1 )
							{
								nValidZ = ( nCountZ - 1 )  - nValidZ;
								fMirrorNormalZ = -1;
							}
							else
							{
								fMirrorNormalZ = 1;
							}

							if ( nZ < 0 )
								nValidZ = ( nCountZ - 1 )  - nValidZ;


							if ( nX & 0x1 )
							{
								nValidX = ( nCountX - 1 )  - nValidX;
								fMirrorNormalX = -1;
							}
							else
							{
								fMirrorNormalX = 1;
							}

							if ( nX < 0 )
								nValidX = ( nCountX - 1 )  - nValidX;

							const int nTile = nValidZ * Width / CellGridSize + nValidX;
							const TerrainTile_c &tile = m_dTiles[ nTile ];

							r3dBox3D bbox;

							bbox.Org = r3dPoint3D( iTileX * fWidthCell, tile.m_HeightMin, iTileZ * fWidthCell );
							bbox.Size = r3dPoint3D( fWidthCell, tile.m_HeightMax - tile.m_HeightMin, fWidthCell );


							if (r3dRenderer->IsBoxInsideFrustum(bbox) )
							{
								const float fLod  = GetLodIndex( iTileX, iTileZ );

								TileLODs[ nTile ]			= (int)fLod;

								const int nTileRemapped = REMAP_OFFSET( nValidX, nValidZ, m_nTileCountX );

								VisibleTile visTile;

								visTile.nValidX				= nValidX;
								visTile.nValidZ				= nValidZ;
								visTile.tTileBox			= bbox;
								visTile.nTile				= nTileRemapped;
								visTile.fMirrorNormalX		= fMirrorNormalX;
								visTile.fMirrorNormalZ		= fMirrorNormalZ;
								visTile.fNormalMapFadeOut	= 0.0f;

								visTile.fLod				= fLod;

								visTile.bNearTile			= true;
								visTile.bOvertiled			= false;
								visTile.bRenderBigTile		= false;
								visTile.ConnectionBits		= 0;

								if( r_show_terra_order->GetInt() )
								{
									mDEBUG_TileOrderPoses.PushBack( bbox.Center() );
								}

								m_dVisibleTiles.PushBack ( visTile );
								g_dMapTiles[nTile] = m_dVisibleTiles.Count();
							}
						}
					}

					vPt += iStepSirection * vCameraDirProjNorm * fWidthCell * 0.5f;
					iTileX = int(vPt.x / fWidthCell) - ((vPt.x < 0.0f) ? 1 : 0 );
					iTileZ = int(vPt.y / fWidthCell) - ((vPt.y < 0.0f) ? 1 : 0 );

					bCorrect = true;
					if ( iTileX >= nMaxX && iStepSirection*vCameraDirProjNorm.x >= 0.0f )
						bCorrect = false;
					if ( iTileZ >= nMaxZ && iStepSirection*vCameraDirProjNorm.y >= 0.0f )
						bCorrect = false;

					if ( iTileX < nMinX && iStepSirection*vCameraDirProjNorm.x <= 0.0f )
						bCorrect = false;
					if ( iTileZ < nMinZ && iStepSirection*vCameraDirProjNorm.y <= 0.0f )
						bCorrect = false;

					// change direction
					if ( !bCorrect && iStepSirection > 0 )
					{
						bCorrect = true;
						iStepSirection = -1;
						vPt = vProjTilePos;
						iTileX = int(vPt.x / fWidthCell) - ((vPt.x < 0.0f) ? 1 : 0 );
						iTileZ = int(vPt.y / fWidthCell) - ((vPt.y < 0.0f) ? 1 : 0 );
					}
				}

				vProjTilePos += vCameraDirProj * fWidthCell * 0.5f;
			}

			D3DXVECTOR4 vZVec ( r3dRenderer->ViewMatrix.m[0][2], r3dRenderer->ViewMatrix.m[1][2], r3dRenderer->ViewMatrix.m[2][2], r3dRenderer->ViewMatrix.m[3][2] );

			for ( int i = 0; i < m_dVisibleTiles.Count (); i++ )
			{
				// project all corner and get min depth ( with sync in terrain shader )
				float fDist = FLT_MAX;

				VisibleTile& tile = m_dVisibleTiles[i];

				r3dPoint3D org = tile.tTileBox.Org;
				r3dPoint3D sz = tile.tTileBox.Size;

				using r3dTL::Min;

				D3DXVECTOR4 
					point	( org.x,		org.y,			org.z,			1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x + sz.x,	org.y,			org.z,			1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x,		org.y + sz.y,	org.z,			1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x,		org.y,			org.z + sz.z,	1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x + sz.x,	org.y + sz.y,	org.z,			1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x + sz.x,	org.y,			org.z + sz.z,	1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x,		org.y + sz.y,	org.z + sz.z,	1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );
				point = D3DXVECTOR4	( org.x + sz.x,	org.y + sz.y,	org.z + sz.z,	1.f ); fDist = Min ( fDist, D3DXVec4Dot( &vZVec, &point ) );

				tile.bNearTile = ( fDist < m_fShaderLODFadeEnd );

				tile.fNormalMapFadeOut = R3D_CLAMP( ( fDist - m_fShaderLODFadeStart ) / ( m_fShaderLODFadeEnd - m_fShaderLODFadeStart ), 0.0f, 1.0f );
			}

			// test on correct tile sorting by direction
#if (0)
			int iCnt = 0;

			for ( int x = nMinX; x < nMaxX; x++ )
			{
				for ( int z = nMinZ; z < nMaxZ; z++ )
				{
					if ( !g_dBitArray.Test (TILE_INDEX(x,z)))
						iCnt++;
				}
			}
			r3d_assert ( iCnt == 0 );
#endif
#undef TILE_INDEX

		}
	}
	else
	{
		float fMirrorNormalX;
		float fMirrorNormalZ;

		for ( int iTileZ = nMinZ; iTileZ < nMaxZ; iTileZ++ )
		{
			int nZ = floorf( ( float ) iTileZ / nCountZ );
			int nValidZ = abs( iTileZ < 0 ? iTileZ + 1 : iTileZ ) % nCountZ;

			if ( nZ & 0x1 )
			{
				nValidZ = ( nCountZ - 1 )  - nValidZ;
				fMirrorNormalZ = -1;
			}
			else
			{
				fMirrorNormalZ = 1;
			}

			if ( nZ < 0 )
				nValidZ = ( nCountZ - 1 )  - nValidZ;

			for ( int iTileX = nMinX; iTileX < nMaxX; iTileX++ )
			{			

				int nX = floorf( ( float ) iTileX / nCountX );
				int nValidX = abs( iTileX < 0 ? iTileX + 1 : iTileX ) % nCountX;


				if ( nX & 0x1 )
				{
					nValidX = ( nCountX - 1 )  - nValidX;
					fMirrorNormalX = -1;
				}
				else
				{
					fMirrorNormalX = 1;
				}

				if ( nX < 0 )
					nValidX = ( nCountX - 1 )  - nValidX;


				const int nTile = nValidZ * Width / CellGridSize + nValidX;
				const TerrainTile_c &tile = m_dTiles[ nTile ];

				r3dBox3D bbox;

				bbox.Org = r3dPoint3D( iTileX * fWidthCell, tile.m_HeightMin, iTileZ * fWidthCell );
				bbox.Size = r3dPoint3D( fWidthCell, tile.m_HeightMax - tile.m_HeightMin, fWidthCell );

				if (!r3dRenderer->IsBoxInsideFrustum(bbox))
					continue;

				const int nLod  = GetLodIndex( iTileX, iTileZ );

				const int nTileRemapped = REMAP_OFFSET( nValidX, nValidZ, m_nTileCountX );

				VisibleTile visTile;

				visTile.nValidX				= nValidX ;
				visTile.nValidZ				= nValidZ ;
				visTile.tTileBox			= bbox ;
				visTile.nTile				= nTileRemapped ;
				visTile.fMirrorNormalX		= fMirrorNormalX ;
				visTile.fMirrorNormalZ		= fMirrorNormalZ ;
				visTile.fNormalMapFadeOut	= 0.0f ;

				visTile.fLod				= nLod ;

				visTile.bNearTile			= true ;
				visTile.bOvertiled			= false ;
				visTile.bRenderBigTile		= false ;
				visTile.ConnectionBits		= 0;

				m_dVisibleTiles.PushBack( visTile );

				g_dMapTiles[ nTile ] = m_dVisibleTiles.Count();
			}
		}
	}

	// calculate adjecancy info
	{
		int iwidth = m_nTileCountX;
		int iwidth_1 = iwidth - 1;
		int iheight_1 = m_nTileCountZ - 1;

		for( uint32_t i = 0, e = m_dVisibleTiles.Count(); i < e; i ++ )
		{
			VisibleTile& tile = m_dVisibleTiles[ i ];

			tile.ConnectionBits = 0;

			int nLod = (int)tile.fLod;

			if( tile.nValidX > 0 && TileLODs[ tile.nValidX - 1 + tile.nValidZ * iwidth ] < nLod )
			{
				tile.ConnectionBits |= WEST_CONNECTION;
			}

			if( tile.nValidX < iwidth_1 && TileLODs[ tile.nValidX + 1 + tile.nValidZ * iwidth ] < nLod )
			{
				tile.ConnectionBits |= EAST_CONNECTION;
			}

			if( tile.nValidZ > 0 && TileLODs[ tile.nValidX + ( tile.nValidZ - 1 ) * iwidth ] < nLod )
			{
				tile.ConnectionBits |= NORTH_CONNECTION;
			}

			if( tile.nValidZ < iheight_1 && TileLODs[ tile.nValidX + ( tile.nValidZ + 1 ) * iwidth ] < nLod )
			{
				tile.ConnectionBits |= SOUTH_CONNECTION;
			}
		}
	}


	int iGoodCount = 0;

	float fMaxLod = LODDESC_COUNT - 1;

	if ( N_OVERTILES > 1 && !bShadowMap ) //-V560
	{
		for ( int i = 0; i < m_dVisibleTiles.Count(); i++ )
		{
			VisibleTile & curTile = m_dVisibleTiles[i];

			if ( curTile.fLod < fMaxLod || curTile.ConnectionBits )
				continue;

			const int iTileX = m_dVisibleTiles[i].nValidX;
			const int iTileZ = m_dVisibleTiles[i].nValidZ;
			if ( iTileX % N_OVERTILES != 0 || iTileZ % N_OVERTILES != 0 )
				continue;
			int iOffsetX = 1;
			int iOffsetZ = 0;
			bool bGood = true;
			for ( int k = 1; k < N_OVERTILES*N_OVERTILES; k++ )
			{
				const int iNewTileIndex = g_dMapTiles[iTileX + iOffsetX + m_nTileCountX * (iTileZ + iOffsetZ)];

				if ( iNewTileIndex <= 0 )
				{
					bGood = false;
					break;
				}

				const VisibleTile & newTile = m_dVisibleTiles[iNewTileIndex - 1];

				if ( (int)newTile.fLod != (int)curTile.fLod || newTile.bNearTile != curTile.bNearTile || newTile.ConnectionBits )
				{
					bGood = false;
					break;
				}

				if ( newTile.bNearTile && fabsf(newTile.fNormalMapFadeOut - curTile.fNormalMapFadeOut)> 0.1f )
				{
					bGood = false;
					break;
				}

				iOffsetX++;
				if ( iOffsetX >= N_OVERTILES )
				{
					iOffsetX = 0;
					iOffsetZ++;
				}
			}

			if ( bGood )
			{
				iGoodCount++;
				iOffsetX = 1;
				iOffsetZ = 0;
				for ( int k = 1; k < N_OVERTILES*N_OVERTILES; k++ )
				{
					const int iNewTileIndex = g_dMapTiles[iTileX + iOffsetX + m_nTileCountX * (iTileZ + iOffsetZ)];
					r3d_assert ( iNewTileIndex > 0 );

					VisibleTile & newTile = m_dVisibleTiles[iNewTileIndex - 1];
					newTile.bOvertiled = true;

					iOffsetX++;
					if ( iOffsetX >= N_OVERTILES )
					{
						iOffsetX = 0;
						iOffsetZ++;
					}
				}

				curTile.bRenderBigTile = true;
			}
		}

		//r3dOutToLog ( "GOOD COUNT %i all %i\n", iGoodCount, m_dVisibleTiles.Count () );
	}
}

//------------------------------------------------------------------------

bool
r3dTerrain::PrepareOthographicTerrainRender() /*OVERRIDE*/
{
	return true ;
}

//------------------------------------------------------------------------

bool
r3dTerrain::DrawOrthographicTerrain( r3dCamera const &Cam, bool UseZ ) /*OVERRIDE*/
{
	R3D_ENSURE_MAIN_THREAD();

	DrawStartMP( Cam );

	SetMP2VertexShader();
	r3dRenderer->SetPixelShader();

	if( UseZ )
		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZW | R3D_BLEND_ZC );
	else
		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_NZ );

	D3D_V( r3dRenderer->pd3ddev->Clear( 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0 ) );

	// need white alpha or else our d3dxsave/d3dxload bezzle produces enterily black dxt1...
	D3D_V( r3dRenderer->pd3ddev->SetRenderState(	D3DRS_COLORWRITEENABLE, 
													D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | 
													D3DCOLORWRITEENABLE_BLUE ) );

	for( int tz = 0, e = m_nTileCountZ; tz < e; tz++ )
	{
		for( int tx = 0, e = m_nTileCountX; tx < e; tx ++ )
		{
			int tileIdxRemapped = REMAP_OFFSET( tx, tz, m_nTileCountX );

			const int nTile = tz * m_nTileCountX + tx;
			const TerrainTile_c &tile = m_dTiles[ nTile ];

			r3dBox3D bbox;

			bbox.Org = r3dPoint3D( tx * CellSize * CellGridSize, tile.m_HeightMin, tz * CellSize * CellGridSize );
			bbox.Size = r3dPoint3D( CellSize, tile.m_HeightMax - tile.m_HeightMin, CellSize );

			VisibleTile vtile;

			vtile.nValidX			= tx;
			vtile.nValidZ			= tz; 
			vtile.tTileBox			= bbox;
			vtile.nTile				= tileIdxRemapped;
			vtile.fMirrorNormalX	= 1.0f;
			vtile.fMirrorNormalZ	= 1.0f;
			vtile.fNormalMapFadeOut	= 0.0f;
			vtile.fLod				= 0.f;
			vtile.bNearTile			= true;
			vtile.bOvertiled		= false;
			vtile.bRenderBigTile	= false;
			vtile.ConnectionBits	= 0;

			DrawTile( vtile, Cam, false, TRM_SPLIT );
		}
	}

	DrawEndMP();

	D3D_V( r3dRenderer->pd3ddev->SetRenderState(	D3DRS_COLORWRITEENABLE, 
													D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | 
													D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );

	return true;
}

//------------------------------------------------------------------------

void
r3dTerrain::SetupCommonConstants( const r3dCamera& Cam )
{
	D3DXMATRIX ShaderMat;

	D3DXMatrixTranspose( &ShaderMat, &r3dRenderer->ViewProjMatrix );
	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );

	D3DXVECTOR4 vCam ( Cam.x, Cam.y, Cam.z, 0 );
	r3dRenderer->pd3ddev->SetVertexShaderConstantF(  15, (float *)&vCam, 1 );

}

//------------------------------------------------------------------------

void
r3dTerrain::SetupCommonConstantsMP( const r3dCamera& Cam )
{
	SetupCommonConstants( Cam );

	float fWorldSizeInv = 1.f / WorldSize;
	D3DXVECTOR4 uv0( fWorldSizeInv, 0.f, 0.f, 0.f );
	r3dRenderer->pd3ddev->SetVertexShaderConstantF(  24, (float *)&uv0,	1 );

	D3DXVECTOR4 vCam ( Cam.x, Cam.y, Cam.z, 1.f / SplitDistance );
	r3dRenderer->pd3ddev->SetPixelShaderConstantF(  0, (float *)&vCam,	1 );
}

//------------------------------------------------------------------------

void
r3dTerrain::SetupMaterial( int idx )
{
	float fWorldSizeInv = 1.f / WorldSize;

	D3DXVECTOR4 uv0(	m_tBaseLayer.fScale * fWorldSizeInv,
		m_dMatLayers[ idx ][ 0 ].fScale * fWorldSizeInv,
		m_dMatLayers[ idx ][ 1 ].fScale * fWorldSizeInv,
		m_dMatLayers[ idx ][ 2 ].fScale * fWorldSizeInv );

	r3dRenderer->pd3ddev->SetVertexShaderConstantF(  20, (float *)&uv0,	1 );



	D3DXVECTOR4 vGloss( m_tBaseLayer.fGloss, 0, 0, 0 );
	r3dRenderer->pd3ddev->SetPixelShaderConstantF( 14, (float *)&vGloss, 1 );

	vGloss = D3DXVECTOR4( m_dMatLayers[ idx ][ 0 ].fGloss, m_dMatLayers[ idx ][ 1 ].fGloss, m_dMatLayers[ idx ][ 2 ].fGloss, m_dMatLayers[ idx ][ 3 ].fGloss );
	r3dRenderer->pd3ddev->SetPixelShaderConstantF( 15, (float *)&vGloss, 1 );

	D3DXVECTOR4 v3( m_dMatLayers[ idx ][ 2 ].fScale * fWorldSizeInv, -m_dMatLayers[idx][ 2 ].fScale * fWorldSizeInv, m_dMatLayers[idx][ 3 ].fScale * fWorldSizeInv, -m_dMatLayers[ idx ][ 3 ].fScale * fWorldSizeInv ); 
	r3dRenderer->pd3ddev->SetPixelShaderConstantF( 7, (float *)&v3, 1 );

	D3DXVECTOR4 vSplit; 
	vSplit = D3DXVECTOR4(		1.0f/m_dMatLayers[ idx ][0].fSplit, 
		1.0f/m_dMatLayers[ idx ][1].fSplit, 
		1.0f/m_dMatLayers[ idx ][2].fSplit, 
		1.0f/m_dMatLayers[ idx ][3].fSplit );

	r3dRenderer->pd3ddev->SetPixelShaderConstantF(  6, (float *)&vSplit, 1 );


#ifndef FINAL_BUILD
	if (__r3dDisplayMipLevels)
		SetTextureAsMipmapTest( m_tBaseLayer.pMapDiffuse, 0);
	else if (d_show_checker_texture->GetBool())
		SetTextureDensityChecker(0);
	else
#endif
		r3dRenderer->SetTex(m_tBaseLayer.pMapDiffuse);
	r3dRenderer->SetTex(MatSplatTex[ idx ], 1);
	r3dRenderer->SetMipMapBias(-6, 1);

#ifndef FINAL_BUILD
	if (__r3dDisplayMipLevels)
	{
		SetTextureAsMipmapTest(m_tBaseLayer.pMapDiffuse, 0);
		SetTextureAsMipmapTest(m_dMatLayers[ idx ][ 0 ].pMapDiffuse, 3);
		SetTextureAsMipmapTest(m_dMatLayers[ idx ][ 1 ].pMapDiffuse, 4);
		SetTextureAsMipmapTest(m_dMatLayers[ idx ][ 2 ].pMapDiffuse, 5);
		SetTextureAsMipmapTest(m_dMatLayers[ idx ][ 3 ].pMapDiffuse, 6);
	}
	else if (d_show_checker_texture->GetBool())
	{
		SetTextureDensityChecker(0);
		SetTextureDensityChecker(3);
		SetTextureDensityChecker(4);
		SetTextureDensityChecker(5);
		SetTextureDensityChecker(6);
	}
	else
#endif
	{
		r3dRenderer->SetTex(m_tBaseLayer.pMapDiffuse);

		r3dRenderer->SetTex(m_dMatLayers[ idx ][ 0 ].pMapDiffuse, 3);
		r3dRenderer->SetTex(m_dMatLayers[ idx ][ 1 ].pMapDiffuse, 4);
		r3dRenderer->SetTex(m_dMatLayers[ idx ][ 2 ].pMapDiffuse, 5);
		r3dRenderer->SetTex(m_dMatLayers[ idx ][ 3 ].pMapDiffuse, 6);
	}

	r3dRenderer->SetTex( m_tBaseLayer.pMapNormal, 8);
	r3dRenderer->SetTex( m_dMatLayers[ idx ][ 0 ].pMapNormal, 9 );
	r3dRenderer->SetTex( m_dMatLayers[ idx ][ 1 ].pMapNormal, 10 );
	r3dRenderer->SetTex( m_dMatLayers[ idx ][ 2 ].pMapNormal, 11 );
	r3dRenderer->SetTex( m_dMatLayers[ idx ][ 3 ].pMapNormal, 12 );

	r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZC | R3D_BLEND_ZW );
}

//------------------------------------------------------------------------

void
r3dTerrain::DrawStartMP( const r3dCamera &Cam )
{
	if(!_render_Terrain) return;

	SetupCommonConstantsMP( Cam );	

	for ( int i = 0; i < 8; i ++ )
	{
		r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
		r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );

		r3dSetFiltering( R3D_ANISOTROPIC, i );
	}

	for ( int i = 8; i < 12; i ++ )
	{
		r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSU,   D3DTADDRESS_CLAMP );
		r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSV,   D3DTADDRESS_CLAMP );

		r3dRenderer->SetMipMapBias( -6, i );

		r3dSetFiltering( R3D_ANISOTROPIC, i );
	}

	r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZW | R3D_BLEND_ZC );

#ifndef FINAL_BUILD
	r3dSetWireframe( r_terra_wire->GetInt() );
#endif
}

//------------------------------------------------------------------------

void
r3dTerrain::DrawEndMP()
{
	if(!_render_Terrain) return;

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();
	d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetTex(NULL);
	r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZW | R3D_BLEND_ZC );

	for( int i = 8; i < 12; i ++ )
	{
		r3dRenderer->SetMipMapBias( 0, i );
	}

	r3dSetWireframe( 0 );
}

//------------------------------------------------------------------------

void
r3dTerrain::DrawDeferredMultipassInitial()
{
	if(!_render_Terrain) return;

	R3DPROFILE_FUNCTION( "r3dTerrain::DrawDeferredMultipassInitial" );

	if( ! bLoaded ) 
		return;

	const r3dCamera& Cam = gCam ;

	Update ();

	D3DPERF_BeginEvent( 0x0, L"r3dTerrain::DrawDeferredMultipassInitial" );

	DrawStartMP( Cam );

#ifndef FINAL_BUILD
	if (__r3dDisplayMipLevels)
		SetTextureAsMipmapTest( m_tBaseLayer.pMapDiffuse, 1 );
	else if (d_show_checker_texture->GetBool())
		SetTextureDensityChecker(1);
	else
#endif
		r3dRenderer->SetTex( m_tBaseLayer.pMapDiffuse, 1 );
	r3dRenderer->SetTex( m_tBaseLayer.pMapNormal, 2 );

	SetMP2VertexShader();

	DrawTerrain( Cam, false, false, TRM_SPLIT, false, true );

	D3DPERF_EndEvent();

	return;
}

//------------------------------------------------------------------------

void
r3dTerrain::DrawDeferredMultipass()
{
	if(!_render_Terrain) return;

	R3DPROFILE_FUNCTION( "r3dTerrain::DrawDeferredMultipass" );

	if( ! bLoaded ) 
		return;

	const r3dCamera& Cam = gCam ;

	D3DPERF_BeginEvent( 0x0, L"r3dTerrain::DrawDeferredMultipass" );

	r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE ) );
	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE ) );
	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE1, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE ) );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE2, 0 ) );
	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE3, 0 ) );

	SetMP2VertexShader();

	DrawTerrain( Cam, true, false, TRM_SPLIT, false, true );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );
	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE1, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE2, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );
	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE3, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();

	d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetTex(NULL);

	r3dSetWireframe(0);

	DrawEndMP();

	D3DPERF_EndEvent();

	return;

}


//------------------------------------------------------------------------

void
r3dTerrain::DrawMaterialHeavyness()
{
	if(!_render_Terrain) return;

	R3DPROFILE_FUNCTION( "r3dTerrain::DrawMaterialHeavyness" );

	if( ! bLoaded ) 
		return;

	const r3dCamera& Cam = gCam ;

	Update ();

	D3DPERF_BeginEvent( 0x0, L"r3dTerrain::DrawMaterialHeavyness" );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE ) );

	DrawStartMP( Cam );

	r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC | R3D_BLEND_PUSH );

	SetMP2VertexShader();
	r3dRenderer->SetPixelShader( "PS_HEAVYNESS" );

	DrawTerrain( Cam, false, false, TRM_HEAVYNESS, false, true );

	D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA ) );

	r3dRenderer->SetRenderingMode( R3D_BLEND_POP );

	DrawEndMP();

	D3DPERF_EndEvent();

	return;
}

//------------------------------------------------------------------------

void
r3dTerrain::DrawBoxes()
{
	if(!_render_Terrain) return;

	for ( uint32_t i = 0; i < m_dVisibleTiles.Count (); i++ )
	{
		const VisibleTile &tile = m_dVisibleTiles[i];

		const TerrainTile_c &ttile = m_dTiles[ tile.nTile ];

		r3dBox3D bbox;

		float fWidthCell = CellGridSize * CellSize;

		bbox.Org = r3dPoint3D( tile.nValidX * fWidthCell, ttile.m_HeightMin, tile.nValidZ * fWidthCell );
		bbox.Size = r3dPoint3D( fWidthCell, ttile.m_HeightMax - ttile.m_HeightMin, fWidthCell );

		r3dDrawBoundBox( bbox, gCam, r3dColor24::red, 1.f );
	}
}

//------------------------------------------------------------------------

void r3dTerrain::DrawDepth()
{
	if(!_render_Terrain) return;

	if(!bLoaded)
		return;

	R3DPROFILE_FUNCTION("r3dTerrain::DrawDepth");

	struct D3DPERFBraces
	{
		D3DPERFBraces()
		{
			D3DPERF_BeginEvent( 0x0, L"r3dTerrain::DrawDepth" );
		}

		~D3DPERFBraces()
		{
			D3DPERF_EndEvent();
		}

	} D3DPERFBracesVar; (void) D3DPERFBracesVar ;

	r3dRenderer->SetVertexShader( r_terrain_quality->GetInt() == 1 ? g_Terra_SM_LQ_VS_ID : g_Terra_SM_VS_ID ) ;

	D3DXMATRIX 	mWorld;
	D3DXMATRIX ShaderMat;
	D3DXMatrixIdentity(&mWorld);

	ShaderMat =  mWorld * r3dRenderer->ViewProjMatrix ;

	D3DXMatrixTranspose( &ShaderMat, &ShaderMat );

	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );
	SetupCommonConstants( gCam );

	DrawTerrain( gCam, false, true, TRM_SIMPLE, true, false );

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();
	d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() );	
}

//------------------------------------------------------------------------
void r3dTerrain::DrawBlack(const r3dCamera &Cam, int bSetShader)
{
	if(!_render_Terrain) return;

	if(!bLoaded)
		return;

	R3DPROFILE_FUNCTION("r3dTerrain::DrawBlack");

	struct D3DPERFBraces
	{
		D3DPERFBraces()
		{
			D3DPERF_BeginEvent( 0x0, L"r3dTerrain::DrawBlack" );
		}

		~D3DPERFBraces()
		{
			D3DPERF_EndEvent();
		}

	} D3DPERFBracesVar; (void) D3DPERFBracesVar ;

	// NOTE : we were forced to separate update cause of our shadow map optimization insanity

	r3dRenderer->SetVertexShader( r_terrain_quality->GetInt() == 1 ? g_Terra_SM_LQ_VS_ID : g_Terra_SM_VS_ID ) ;
	r3dRenderer->SetPixelShader("PS_TERRA_SM");

	D3DXMATRIX 	mWorld;
	D3DXMATRIX ShaderMat;
	D3DXMatrixIdentity(&mWorld);

	ShaderMat =  mWorld * r3dRenderer->ViewProjMatrix;

	D3DXMatrixTranspose( &ShaderMat, &ShaderMat );

	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );
	SetupCommonConstants( Cam );

	DrawTerrain( Cam, false, true, TRM_SIMPLE, true, true );

	r3dRenderer->RestoreCullMode();

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();
	d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() );	

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	Editor
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//--------------------------------------------------------------------------------------------------------
float r3dTerrain::GetHeightFromNormalizeValue( float normHeight )
{
	assert( normHeight <= 1.f && normHeight >= 0 );

	return m_HeightmapSize * normHeight;
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::ImportHeightmap( const char * fileName, float yScale )
{
	if( !fileName[ 0 ] )
		return false;

	r3dTexture *TempTex = r3dRenderer->LoadTexture( fileName );
	if ( ! TempTex )
		return false;

	m_HeightmapSize = yScale;

	float *ClrData = (float *)TempTex->Lock(0);

	// read actual data

	r3d_assert( TempTex->GetWidth() == Width && TempTex->GetHeight() == Height );

	HeightFieldData.Resize( (int)Height * (int)Width );

	for(int y = 0; y < Height; y++)
	{
		int	inv_y = Height - y - 1;

		for(int x = 0; x < Width; x++)
		{
			float H = *(ClrData + int(y * Width + x)) * m_HeightmapSize;
			HeightFieldData[ int(inv_y*Width+x) ] = H;
		}
	}

	TempTex->Unlock();
	r3dRenderer->DeleteTexture(TempTex);

	UpdatePhysHeightField();
	return UpdateAllVertexData();
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::ExportHeightmap( const char * fileName )
{
	IDirect3DTexture9 * pTex = NULL;
	HRESULT hr = D3DXCreateTexture( r3dRenderer->pd3ddev, Width, Height, 1, 0, D3DFMT_R32F, D3DPOOL_SYSTEMMEM, &pTex );

	if ( FAILED( hr ) )
		return false;

	D3DLOCKED_RECT rc_lock;

	hr = pTex->LockRect( 0, &rc_lock, NULL, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK );
	if ( FAILED( hr ) )
	{
		pTex->Release();
		return false;
	}

	r3d_assert( physicsHeightField );

	ExtractHeightFieldData();

	float norm = 1.f / m_HeightmapSize;

	for( int z = 0, h = (int)Height; z < h; z++)
	{
		float * pData = (float * )( (char * ) rc_lock.pBits + ( (int )Height - z - 1 ) * rc_lock.Pitch );

		for( int x = 0, w = (int)Width; x < w; x++, pData++ )
		{
			*pData = HeightFieldData[ x + z * w ] * norm;
		}
	}

	pTex->UnlockRect( 0 );

	hr = D3DXSaveTextureToFile( fileName, D3DXIFF_DDS, pTex, NULL );
	pTex->Release();

	if ( FAILED( hr ) )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::ExportColor( const char * fileName )
{
	IDirect3DTexture9 * pTex = NULL;
	HRESULT hr = D3DXCreateTexture( r3dRenderer->pd3ddev, Width, Height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTex );

	if ( FAILED( hr ) )
		return false;

	D3DLOCKED_RECT rc_lock;

	hr = pTex->LockRect( 0, &rc_lock, NULL, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK );
	if ( FAILED( hr ) )
	{
		pTex->Release();
		return false;
	}

	r3d_assert( m_pColorData );
	int iHeight = static_cast<int>(Height);
	int iWidth = static_cast<int>(Width);
	byte *dst = reinterpret_cast<byte*>(rc_lock.pBits);
	byte *src = reinterpret_cast<byte*>(m_pColorData);

	uint32_t totalBytes = rc_lock.Pitch * Height;
	r3d_assert(HeightFieldData.Count() * sizeof(m_pColorData[0]) == totalBytes);

	memcpy_s(dst, totalBytes, src, totalBytes);

	pTex->UnlockRect( 0 );

	hr = D3DXSaveTextureToFile( fileName, D3DXIFF_DDS, pTex, NULL );
	pTex->Release();

	if ( FAILED( hr ) )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------------
bool r3dTerrain::ImportColor( const char * fileName )
{
	if( !fileName[ 0 ] )
		return false;

	r3dTexture *TempTex = r3dRenderer->LoadTexture( fileName );
	if ( ! TempTex )
		return false;

	byte *src = reinterpret_cast<byte*>(TempTex->Lock(0));

	// read actual data

	r3d_assert( TempTex->GetWidth() == Width && TempTex->GetHeight() == Height );
	r3d_assert( TempTex->GetLockPitch() == Width * sizeof(m_pColorData[0]) );

	uint32_t totalBytes = TempTex->GetLockPitch() * Height;

	memcpy_s(m_pColorData, totalBytes, src, totalBytes);

	TempTex->Unlock();
	r3dRenderer->DeleteTexture(TempTex);

	return UpdateAllVertexData();
}

//--------------------------------------------------------------------------------------------------------
#ifndef FINAL_BUILD
void r3dTerrain::ApplyBrush(const r3dPoint3D &pnt, const float strength, const float radius, const float hardness)
{

	int nc = int(radius / CellSize);	// number of cells in radius
	int nc1 = int((radius*hardness) / CellSize);	// number of cells in radius*hard
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;

	r3d_assert( physicsHeightField );

	r3d_assert( HeightFieldData.Count() );

	for(int y = y1; y <= y2; y++) {
		for(int x = x1; x <= x2; x++) {
			int dy = y-cy;
			int dx = x-cx;

			float coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
			if(coef > 1.0f) continue;

			if (coef <= hardness ) coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);
			//r3dOutToLog("%d %d\n", x,y);

			int nIndex = y*int(Width) + x;

			float fOld = HeightFieldData[ nIndex ];

			//  if (coef >Strength) coef = Strength;
			coef *= strength;

			// do something with h - it will be modified directly
			float h = fOld + coef ;
			if (h <0) 
				h = 0;

			HeightFieldData[ nIndex ] = h;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = fOld;
			undo.fCurrHeight = h;
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	UpdateVertexData( rc );

	return;
}

//--------------------------------------------------------------------------------------------------------

void r3dTerrain::BeginUndoRecord( const char * title, UndoAction_e eAction )
{
	m_pUndoItem = g_pUndoHistory->CreateUndoItem( eAction );
	assert( m_pUndoItem );
	m_pUndoItem->SetTitle( title );
}

//--------------------------------------------------------------------------------------------------------
void r3dTerrain::EndUndoRecord()
{
	m_pUndoItem = NULL;
}

void r3dTerrain::ApplyHeight(const r3dPoint3D &pnt, const float H,  const float strength, const float radius, const float hardness)
{

	int nc = int(radius / CellSize);	// number of cells in radius
	int nc1 = int((radius*hardness) / CellSize);	// number of cells in radius*hard
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;

	for(int y = y1; y <= y2; y++) {
		for(int x = x1; x <= x2; x++) {
			int dy = y-cy;
			int dx = x-cx;

			float coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
			if( coef > 1.0f ) continue;

			if( coef <= hardness ) coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);
			//r3dOutToLog("%d %d\n", x,y);

			int nIndex = y*int(Width) + x;

			float fOld = HeightFieldData[ nIndex ];

			//  if (coef >Strength) coef = Strength;
			coef *= strength;

			// do something with h - it will be modified directly
			float h1 = H*coef+(1-coef)*fOld;

			if (h1 <0) 
				h1 = 0;

			HeightFieldData[ nIndex ] = h1;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = fOld;
			undo.fCurrHeight = h1;
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	UpdateVertexData( rc );

	return;
}

static float ApplyBoxFilter(float* heightMap, int x, int y, int Width, int Height, int stepOff)
{
	float avg = 0;	// average height of box
	int   pix = 0;	// number of pixels

	// sum all pixels in box
	for(int x1 = x-stepOff; x1 <= x+stepOff; x1++) {
		if(x1 < 0 || x1 >= Width) continue;
		for(int y1 = y-stepOff; y1 <= y+stepOff; y1++) {
			if(y1 < 0 || y1 >= Height) continue;

			avg += heightMap[x1 + y1*Width];
			pix++;
		}
	}

	avg /= pix;

	return avg;
}

void r3dTerrain::ApplyRamp(const r3dPoint3D& rampStart, const r3dPoint3D& rampEnd, const float rampWidthOuter, const float rampWidthInner)
{
	r3dPoint3D rampStart2D = r3dPoint3D(rampStart.x, 0, rampStart.z);
	r3dPoint3D rampEnd2D   = r3dPoint3D(rampEnd.x,   0, rampEnd.z);
	float      dy          = (rampEnd.y - rampStart.y);

	// detect terrain box for update
	float bbx1, bbx2, bby1, bby2;
	bbx1 = R3D_MIN(rampStart.x, rampEnd.x) - rampWidthOuter;
	bbx2 = R3D_MAX(rampStart.x, rampEnd.x) + rampWidthOuter;
	bby1 = R3D_MIN(rampStart.z, rampEnd.z) - rampWidthOuter;
	bby2 = R3D_MAX(rampStart.z, rampEnd.z) + rampWidthOuter;

	// terrain affected cells
	int y1 = r3dTL::Clamp( bby1/CellSize, 0, Height - 1 );
	int y2 = r3dTL::Clamp( bby2/CellSize, 0, Height - 1 );
	int x1 = r3dTL::Clamp( bbx1/CellSize, 0, Width - 1 );
	int x2 = r3dTL::Clamp( bbx2/CellSize, 0, Width - 1 );


	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;

	for(int y = y1; y <= y2; y++) 
	{
		for(int x = x1; x <= x2; x++) 
		{
			r3dPoint3D p(x*CellSize, 0, y*CellSize);
			float k; // dist koef on line [0..1]
			float d = r3dDistancePointLine(p, rampStart2D, rampEnd2D, &k);
			if(k < 0 || k > 1.0f)  continue;
			if(d > rampWidthOuter) continue;


			int nIndex = y*int(Width) + x;

			float h1 = HeightFieldData[ nIndex ];
			float h2 = rampStart.y + (dy * k);

			float h;
			if(d < rampWidthInner) {
				h = h2;
			} else { 
				float lc = ((d - rampWidthInner) / (rampWidthOuter - rampWidthInner));
				h = h2 + (h1 - h2) * lc;
			}

			HeightFieldData[ nIndex ] = h;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = h1;
			undo.fCurrHeight = h;
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );
	UpdateVertexData( rc );

	return;
}


int	_terra_smoothBoxOff = 1;
float	_terra_smoothSpeed   = 1.0f;

void r3dTerrain::ApplySmooth(const r3dPoint3D &pnt, const float radius)
{
	int nc = int(radius / CellSize);	// number of cells in radius
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	static Floats HeightMapData1( Width * Height );

	memcpy( &HeightMapData1[0], &HeightFieldData[0], Width*Height*sizeof(float) );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	float koef = 1.0f;
	if(_terra_smoothSpeed) {
		koef = r3dGetFrameTime() / _terra_smoothSpeed;
		koef = R3D_CLAMP(koef, 0.0f, 1.0f);
	}

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;


	for(int y = y1; y <= y2; y++) 
	{
		for(int x = x1; x <= x2; x++) 
		{
			int dy = y-cy;
			int dx = x-cx;

			float coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
			if(coef > 1.0f) continue;

			int nIndex = y*int(Width) + x;

			float h1 = HeightFieldData[ nIndex ];
			float h2 = ApplyBoxFilter( &HeightMapData1[0], x, y, Width, Height, _terra_smoothBoxOff);

			HeightFieldData[ nIndex ] = h1 + (h2 - h1) * koef;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = h1;
			undo.fCurrHeight = HeightFieldData[ nIndex ];
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	UpdateVertexData( rc );

	return;
}
#endif




//--------------------------------------------------------------------------------------------------------



//0 - B
// 1 - G
// 2 - R
// 3 - A :)
#ifndef FINAL_BUILD
//--------------------------------------------------------------------------------------------------------
void r3dTerrain::ApplyMaskBrush(const r3dTerrainPaintBoundControl& boundCtrl, const r3dPoint3D &pnt, int opType, int layerIdx, float val, const float radius, const float hardness )
{
	float fFactor;

#if 0
	if ( ! IsValidPaintBounds( pnt, 1.f, fFactor ) )
		return;
#endif

	int MatIdx	= layerIdx / TERRAIN_LAYERS_PER_MAT;
	int Channel = TERRAIN_LAYERS_PER_MAT - layerIdx % TERRAIN_LAYERS_PER_MAT - 1;

	r3dTexture* tex = MatSplatTex[ MatIdx ];
	if(tex->GetD3DFormat() != D3DFMT_A8R8G8B8) r3dError("splat tex is not argb8");

	// calc coord in [0..1] inside terrain
	float terrx = pnt.x / float(CellSize * Width);
	float terry = (float(CellSize * Height) - pnt.z) / float(CellSize * Height);
	float terrd = radius / float(CellSize * Width);

	// and convert to pixels
	int nc = int(terrd * (float)tex->GetWidth());
	int cx = int(terrx * (float)tex->GetWidth());	// center cell x
	int cy = int(terry * (float)tex->GetHeight());	// center cell y

	// affected area
	int y1 = cy-nc; if(y1 < 0)             y1 = 0;
	int y2 = cy+nc; if(y2 > tex->GetHeight()-1) y2 = tex->GetHeight()-1;
	int x1 = cx-nc; if(x1 < 0)             x1 = 0;
	int x2 = cx+nc; if(x2 > tex->GetWidth()-1)  x2 = tex->GetWidth()-1;

	if( x1 > tex->GetWidth() - 1		||
		y1 > tex->GetHeight() - 1	|| 
		x2 < 0	||  y2 < 0 )
		return;

	float fSoft = radius * ( 1.f - hardness );

	RECT R;
	R.left = x1;
	R.right = x2 + 1;
	R.top = y1;
	R.bottom = y2 + 1;

	DWORD* colors = (DWORD*)tex->Lock(1, &R);

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_MASK_PAINT );
	CLayerMaskPaint * pUndoItem = ( CLayerMaskPaint * ) m_pUndoItem;

	CLayerMaskPaint::PaintData_t undo;


	int nLine = R.right - R.left;
	int nSize = nLine * ( R.bottom - R.top );
	undo.rc = R;
	undo.MatIdx = MatIdx;
	undo.pData = new uint32_t[ nSize * 2 ];


	for(int y = 0; y < y2 - y1 + 1; y++) 
	{
		uint32_t * pSrc = (uint32_t*)&colors[ y * tex->GetWidth() ];
		uint32_t * pDst = undo.pData + y * nLine;

		memcpy( pDst, pSrc, nLine * sizeof( uint32_t ) );
	}

	if( opType )
	{
		int texHeight = (int)tex->GetHeight();
		int texWidth = (int)tex->GetWidth();

		int cy1 = ( texHeight - y1 ) * (int)Height / texHeight;
		int cy2 = ( texHeight - y2 ) * (int)Height / texHeight;

		int cx1 = x1 * (int)Width / texWidth;
		int cx2 = x2 * (int)Width / texWidth;

		for(	int		y	= R3D_MAX( ( cy2 - 1 ) / CellGridSize, 0 ), 
			ey	= R3D_MIN( ( cy1 + 1 ) / CellGridSize, m_nTileCountZ - 1 );
			y <= ey; y++ )
		{
			for( int	x	= R3D_MAX( ( cx1 - 1 ) / CellGridSize, 0 ),
				ex	= R3D_MIN( ( cx2 + 1 ) / CellGridSize, m_nTileCountX - 1 );
				x <= ex ; x++ )
			{
				InsertLayer( x, y, layerIdx + 1 );
			}
		}
	}

	for(int y = y1; y <= y2; y++) {
		for(int x = x1; x <= x2; x++) 
		{
			float coef = 1.0f;

			if(nc > 0) 
			{
				int dy = y-cy;
				int dx = x-cx;
				coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
				if(coef > 1.0f) 
				{
					continue;
				}
			}

			r3dPoint3D PP = r3dPoint3D((float(x)/tex->GetWidth())*float(CellSize * Width), 0,
				float(CellSize * Height) - (float(y)/tex->GetHeight())*float(CellSize * Width));
			//r3dVector V = GetNormal(PP);
			//V.Normalize();

			PP.y = GetHeight( PP );

			if ( ! terra_IsValidPaintBounds( boundCtrl, PP, fSoft, &fFactor ) )
				continue;

			if(coef <= hardness) 
				coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);

			coef *= fFactor;


			//if (V.Y < MinSlope ) coef = 0;
			//if (V.Y > MaxSlope ) coef = 0;

			int Pitch = tex->GetWidth();
			BYTE* rgba = (BYTE*)&colors[(y-y1)*Pitch+(x-x1)];
			float pix = float(rgba[Channel]);

			if (opType) 
				pix = pix + val*coef; //+(1-coef)*pix;
			else
				pix = pix - val*coef; //+(1-coef)*pix;

			pix = R3D_CLAMP(pix, 0.0f, 255.0f);
			rgba[Channel] = BYTE(pix);
		}
	}


	for(int y = 0; y < y2 - y1 + 1; y++) 
	{
		uint32_t * pSrc = (uint32_t*)&colors[ y * tex->GetWidth() ];
		uint32_t * pDst = undo.pData + y * nLine + nSize;

		memcpy( pDst, pSrc, nLine * sizeof( uint32_t ) );
	}

	pUndoItem->AddData( undo );
	pUndoItem->AddRectUpdate( undo.rc );

	tex->Unlock();

	return;
}




void r3dTerrain::ApplyNoise(const r3dPoint3D &pnt, const float strength, int bPositiveOnly, const float radius, const float hardness)
{
	int nc = int(radius / CellSize);	// number of cells in radius
	int nc1 = int((radius*hardness) / CellSize);	// number of cells in radius*hard
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;

	for(int y = y1; y <= y2; y++) {
		for(int x = x1; x <= x2; x++) {
			int dy = y-cy;
			int dx = x-cx;

			float coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
			if(coef > 1.0f) continue;

			if (coef <= hardness ) coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);

			int nIndex = y*int(Width) + x;
			float fOld = HeightFieldData[ nIndex ];

			//float& h1 = HeightMapData[y*int(Width) + x];


			float Noise = (sinf(float(x)/4000.0f)+cosf(float(y)/4000.0f))*random(strength); //(float(2*random(Power*100.0f)) / 100.0f)*(sinf(pnt.x/1000.0f)+cosf(pnt.z/1000.0f))/2.0f;

			//   Noise *= sinf((pnt.x+pnt.z/2.0f / 100.0f));

			//	  if (!bPositiveOnly) Noise = Noise - Power;
			//	  else
			//		  Noise /= 2.0f;

			float h1 = (fOld+Noise)*coef+(1-coef)*fOld;
			if (h1 <0) 
				h1 = 0;

			HeightFieldData[ nIndex ] = h1;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = fOld;
			undo.fCurrHeight = h1;
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	UpdateVertexData( rc );

	return;
}

#endif

float r3dTerrain::GetEditHeight( const r3dPoint3D &pnt )
{
	register int	X     = (int)(pnt.X / CellSize);
	register int	Y     = (int)(pnt.Z / CellSize);
	register int	xLeft = (int)(pnt.X - (X * CellSize));
	register int	yLeft = (int)(pnt.Z - (Y * CellSize));

	if (X < 0 ) return 0.0f;
	if (Y < 0 ) return 0.0f;

	if(X > Width - 2)  return 0.0f;
	if(Y > Height - 2) return 0.0f;

	// get scalar position of point in grid square
	float tmp_posx = pnt.X / CellSize - (float)X;
	float tmp_posy = pnt.Z / CellSize - (float)Y;

	int w = (int)Width;

	// if in top left half
	if(tmp_posx + tmp_posy < 1) {
		//get height of top left and set the others relative to it
		float tmp_z1 = HeightFieldData[(Y+0)*w+(X+0)];
		float tmp_z2 = HeightFieldData[(Y+0)*w+(X+1)] - tmp_z1;
		float tmp_z3 = HeightFieldData[(Y+1)*w+(X+0)] - tmp_z1;

		// equation to find height
		return tmp_z1 + tmp_z2*tmp_posx + tmp_z3*tmp_posy;
	} else { //if in bottom right half
		//make others relative to bottom left
		float tmp_z1 = HeightFieldData[(Y+1)*w+(X+1)];
		float tmp_z2 = HeightFieldData[(Y+0)*w+(X+1)] - tmp_z1;
		float tmp_z3 = HeightFieldData[(Y+1)*w+(X+0)] - tmp_z1;

		//because the heights are relative to the bottom left, we need to
		//make the scale relative to the bottom left as well with (1-tmp_pos)
		return tmp_z1 + tmp_z2*(1-tmp_posy) + tmp_z3*(1-tmp_posx);
	}

	return 0.0f;
}

float r3dTerrain::GetHeight( const r3dPoint3D &pnt)
{
	r3d_assert(physicsHeightField);

	float rx = pnt.x / CellSize ;
	float rz = pnt.z / CellSize ;

	PxReal x = R3D_MAX( R3D_MIN( rx, Width  - 1.5f ), 0.f );
	PxReal z = R3D_MAX( R3D_MIN( rz, Height - 1.5f ), 0.f );
	return physicsHeightField->getHeight(x, z) * m_InvHFScale;
}

void
r3dTerrain::GetHeightRange( float* oMinHeight, float* oMaxHeight, r3dPoint2D start, r3dPoint2D end )
{
	r3d_assert( g_bEditMode ) ;

	if ( !HeightFieldData.Count() )
	{
		ExtractHeightFieldData() ;
	}

	int	x0	= (int)( start.x / CellSize );
	int	z0	= (int)( start.y / CellSize );
	int	x1	= (int)( end.x / CellSize );
	int	z1	= (int)( end.y / CellSize );

	x0 = R3D_MAX( R3D_MIN( x0, (int)Width - 1 ), 0 ) ;
	x1 = R3D_MAX( R3D_MIN( x1, (int)Width - 1 ), 0 ) ;

	z0 = R3D_MAX( R3D_MIN( z0, (int)Height - 1 ), 0 ) ;
	z1 = R3D_MAX( R3D_MIN( z1, (int)Height - 1 ), 0 ) ;

	int xmi = R3D_MIN( x0, x1 ) ;
	int xma = R3D_MAX( x0, x1 ) ;

	int zmi = R3D_MIN( z0, z1 ) ;
	int zma = R3D_MAX( z0, z1 ) ;

	int iwidth = (int) Width ;

	float minHeight = FLT_MAX ;
	float maxHeight = -FLT_MAX ;

	for( int j = z0 ; j <= z1; j ++ )
	{
		for( int i = x0 ; i <= x1; i ++ )
		{
			float h = HeightFieldData[ i + j * iwidth ] ;
			minHeight = R3D_MIN( h, minHeight ) ;
			maxHeight = R3D_MAX( h, maxHeight ) ;
		}
	}

	*oMinHeight = minHeight ;
	*oMaxHeight = maxHeight ;
}

void
r3dTerrain::AppendTileCoords( const r3dPoint3D& from, const r3dPoint3D& to, TileCoords* outCoords )
{
	r3d_assert( from.x <= to.x );
	r3d_assert( from.z <= to.z );

	int TileFromX = (int) ( from.X / CellSize / CellGridSize );
	int TileFromZ = (int) ( from.Z / CellSize / CellGridSize );

	int TileToX = (int) ceilf( to.X / CellSize / CellGridSize );
	int TileToZ = (int) ceilf( to.Z / CellSize / CellGridSize );

	for( int i = TileFromX, e = TileToX; i < e ; i ++ )
	{
		for( int j = TileFromZ, e = TileToZ; j < e ; j ++ )
		{
			TileCoord c ;

			c.X = R3D_MIN( R3D_MAX( i, 0 ), m_nTileCountX - 1 ) ;
			c.Z = R3D_MIN( R3D_MAX( j, 0 ), m_nTileCountZ - 1 ) ;

			bool add = true ;

			for( uint32_t k = 0, e = outCoords->Count(); k < e; k ++ )
			{
				if( (*outCoords)[ k ] == c )
				{
					add = false ;
					break ;
				}
			}

			if( add )
			{
				outCoords->PushBack( c );
			}
		}
	}

}

r3dVector r3dTerrain::GetNormal(const r3dPoint3D &pnt)
{
	r3dPoint3D v1,v2,v3;
	r3dVector a, b, Normal;

	v1 = pnt + r3dPoint3D(0,0,CellSize);
	v2 = pnt + r3dPoint3D(CellSize,0,-CellSize);
	v3 = pnt + r3dPoint3D(-CellSize,0,-CellSize);

	v1.Y = GetHeight(v1);
	v2.Y = GetHeight(v2);
	v3.Y = GetHeight(v3);

	a      = v2 - v1;
	b      = v3 - v1;
	Normal = a.Cross(b);
	Normal.Normalize();
	if(R3D_ABS(Normal.X) < 0.001) Normal.X = 0.;
	if(R3D_ABS(Normal.Y) < 0.001) Normal.Y = 0.;
	if(R3D_ABS(Normal.Z) < 0.001) Normal.Z = 0.;

	return Normal; 
}

const MaterialType*
r3dTerrain::GetMaterialType( const r3dPoint3D& pnt )
{
	if( !LayerData.Count() )
		return g_pMaterialTypes->GetDefaultMaterial();

	int	X     = (int)(pnt.X / CellSize * MaterialDataWidth / Width );
	int	Z     = (int)(pnt.Z / CellSize * MaterialDataHeight / Height );

	X = R3D_MIN( R3D_MAX( X, 0 ), (int)MaterialDataWidth - 1 );
	Z = R3D_MIN( R3D_MAX( Z, 0 ), (int)MaterialDataHeight - 1 );

	int lidx = LayerData[ X +  Z * MaterialDataWidth ] ;

	if( lidx >= 0 && lidx < R3D_ARRAYSIZE( MatTypeIdxes ) )
	{
		return g_pMaterialTypes->GetByIdx( MatTypeIdxes[ lidx ] );
	}
	else
	{
		return g_pMaterialTypes->GetDefaultMaterial();
	}
}

//------------------------------------------------------------------------

bool
r3dTerrain::IsLoaded() /*OVERRIDE*/
{
	return bLoaded ;
}

//------------------------------------------------------------------------


float r3dTerrain::GetTotalWidth() const
{
	return Width * CellSize;
}

float r3dTerrain::GetTotalHeight() const
{
	return Height * CellSize;
}


//
//
//

#ifndef FINAL_BUILD
void r3dTerrain::ApplyColor(const r3dTerrainPaintBoundControl& boundCtrl, const r3dPoint3D &pnt, const r3dColor &dwColor, const float strength, const float radius, const float hardness)
{
	r3d_assert( g_bEditMode );

	int nc = int(radius / CellSize);	// number of cells in radius
	int nc1 = int((radius*hardness) / CellSize);	// number of cells in radius*hard
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_COLOR_PAINT );
	CLayerColorPaint * pUndoItem = ( CLayerColorPaint * ) m_pUndoItem;

	float fFactor;
	float fSoft = radius * ( 1.f - hardness );

	for(int y = y1; y <= y2; y++) {
		for(int x = x1; x <= x2; x++) {
			int dy = y-cy;
			int dx = x-cx;


			float coef = sqrtf(float(dx*dx+dy*dy)) / float(nc);
			if(coef > 1.0f) 
				continue;


			if (coef <= hardness)
				coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);
			//r3dOutToLog("%d %d\n", x,y);

			r3dPoint3D PP = r3dPoint3D(
				(float(x)/MatSplatTex[ 0 ]->GetWidth())*float(CellSize * Width), 0,
				(float(y)/MatSplatTex[ 0 ]->GetHeight())*float(CellSize * Width));
			PP.y = GetHeight( PP );

			if ( ! terra_IsValidPaintBounds( boundCtrl, PP, fSoft, &fFactor ) )
				continue;

			coef *= fFactor;


			int nIndex = y*int(Width) + x;

			r3dColor clr_old;			
			clr_old.SetPacked( m_pColorData[ nIndex ] );

			//  if (coef >Strength) coef = Strength;
			coef *= strength;

			// do something with h - it will be modified directly

			r3dColor clr;

			clr.R = dwColor.R * coef + clr_old.R * (1-coef);
			clr.G = dwColor.G * coef + clr_old.G * (1-coef);
			clr.B = dwColor.B * coef + clr_old.B * (1-coef);

			clr.R = r3dTL::Clamp( clr.R, 0, 255 );
			clr.G = r3dTL::Clamp( clr.G, 0, 255 );
			clr.B = r3dTL::Clamp( clr.B, 0, 255 );


			//r3dColor clr; = dwColor*coef+clr_old * (1-coef);

			//	if (h1 <0) 
			//		h1 = 0;

			m_pColorData[ nIndex ] = clr.GetPacked();

			CLayerColorPaint::UndoColor_t undo;
			undo.nIndex = nIndex;
			undo.dwPrevColor = clr_old.GetPacked();
			undo.dwCurrColor = clr.GetPacked();
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );
	UpdateVertexData( rc );

	return;
}


void r3dTerrain::ApplyErosion(const r3dPoint3D &pnt, const float strength, const float radius, const float hardness)
{
	int nc = int(radius / CellSize);	// number of cells in radius
	int nc1 = int((radius*hardness) / CellSize);	// number of cells in radius*hard
	int cx = int(pnt.X / CellSize);	// center cell x
	int cy = int(pnt.Z / CellSize);	// center cell y

	cx = r3dTL::Clamp( cx, 0, Width - 1 );
	cy = r3dTL::Clamp( cy, 0, Height - 1 );

	// affected area
	int y1 = cy-nc; if(y1 < 0)        y1 = 0;
	int y2 = cy+nc; if(y2 > Height-1) y2 = Height-1;
	int x1 = cx-nc; if(x1 < 0)        x1 = 0;
	int x2 = cx+nc; if(x2 > Width-1)  x2 = Width-1;

	int centerIdx = cy * static_cast<int>(Width) + cx;
	float centerHeight = HeightFieldData[centerIdx];

	r3d_assert( m_pUndoItem );
	r3d_assert( m_pUndoItem->GetActionID() == UA_TERRAIN_HEIGHT );
	CHeightChanged * pUndoItem = ( CHeightChanged * ) m_pUndoItem;

	float minValue = FLT_MAX;
	for(int y = y1; y <= y2; y++)
	{
		for(int x = x1; x <= x2; x++)
		{
			int nIndex = y*int(Width) + x;
			float f = HeightFieldData[ nIndex ];
			minValue = R3D_MIN(minValue, f);
		}
	}

	for(int y = y1; y <= y2; y++)
	{
		for(int x = x1; x <= x2; x++)
		{
			int dy = y-cy;
			int dx = x-cx;

			float vLen = sqrtf(float(dx * dx + dy * dy));

			float coef = vLen / float(nc);
			if(coef > 1.0f) continue;

			if (coef <= hardness ) coef = 1.0f;
			else
				coef = 1.0f - (coef - hardness) / (1.0f-hardness);

			int nIndex = y*int(Width) + x;
			float fOld = HeightFieldData[ nIndex ];

			int yB = r3dTL::Clamp(y + 1, 0, Height - 1);
			int xB = r3dTL::Clamp(x + 1, 0, Width - 1);

			int nIndexB = yB * static_cast<int>(Width) + x;
			int nIndexR = y * static_cast<int>(Width) + xB;

			float fOldR = HeightFieldData[ nIndexR ];
			float fOldB = HeightFieldData[ nIndexB ];

			float ddyH = fOldB - fOld;
			float ddxH = fOldR - fOld;

			float ddH = R3D_MIN(strength, R3D_MAX(abs(ddyH), abs(ddxH)));

			float u = (x - x1) / static_cast<float>(x2 - x1);
			float v = (y - y1) / static_cast<float>(y2 - y1);
			float c = gErosionPattern->SampleBilinear(u, v);

			float h1 = R3D_MAX(fOld - coef * c * ddH, minValue);
			if (h1 <0) 
				h1 = 0;

			HeightFieldData[ nIndex ] = h1;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = nIndex;
			undo.fPrevHeight = fOld;
			undo.fCurrHeight = h1;
			pUndoItem->AddData( undo );
		}
	}

	RECT rc;
	rc.left = x1 / CellGridSize;
	rc.bottom = y1 / CellGridSize;
	rc.right = x2 / CellGridSize;
	rc.top = y2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	UpdateVertexData( rc );
}

#endif
//------------------------------------------------------------------------

static void EnableWatchdog( void* )
{
	r3dEnableWatchDog( 60.f ) ;
	r3dSetWatchdogItemName( "r3dTerrain::UpdateMaterials" ) ;
}

static void DisableWatchdog( void* )
{
	r3dDisableWatchDog();
}

void
r3dTerrain::UpdateMaterials( uint32_t startTileX, uint32_t startTileZ, uint32_t endTileX, uint32_t endTileZ )
{
	r3dOutToLog("TERRAIN: UpdateMaterials\n");

	float updateStart = r3dGetTime() ;

	struct AddRemoveWatchdog
	{
		AddRemoveWatchdog()
		{
			AddCustomDeviceQueueItem( EnableWatchdog, 0 ) ;
		}

		~AddRemoveWatchdog()
		{
			AddCustomDeviceQueueItem( DisableWatchdog, 0 ) ;
		}
	} addRemoveWatchdog; (void)addRemoveWatchdog ;

	uint8_t baseIdx = g_pMaterialTypes->GetIdx( m_tBaseLayer.MatTypeName );

	Bytes mat_data( CellGridSize * CellGridSize );

	r3dTL::TFixedArray< uint8_t (*)[ 4 ], TERRAIN_MAT_COUNT > LockedTextures;

	for( int i = 0, e = TERRAIN_MAT_COUNT ; i < e; i ++ )	
	{
		if( r3dTexture* tex = EditorMatSplatTex[ i ] )
		{
			LockedTextures[ i ] = (uint8_t(*)[4])tex->Lock( 0 );

			r3d_assert( LockedTextures[ i ] ) ;
		}
		else
		{
			LockedTextures[ i ] = 0 ;
		}
	}

	// TODO :
	int layer_map[4] = { 3, 2, 1, 0 };

	MatTypeIdxes[ 0 ] = baseIdx ;

	for( int j = 0, k = 1, e = TERRAIN_MAT_COUNT ; j < e; j ++ )
	{
		for( int i = 0, e = TERRAIN_LAYERS_PER_MAT ; i < e ; i ++, k ++ )
		{
			MatTypeIdxes[ k ] = g_pMaterialTypes->GetIdx( m_dMatLayers[ j ][ i ].MatTypeName );
		}
	}

	int SPLAT_WIDTH		= 0 ;
	int SPLAT_HEIGHT	= 0 ;

	for( int i = 0, e = SplatTextures::COUNT; i < e; i ++ )
	{
		if( r3dTexture* tex = EditorMatSplatTex[ i ] )
		{
			SPLAT_WIDTH		= EditorMatSplatTex[ i ]->GetWidth() ;
			SPLAT_HEIGHT	= EditorMatSplatTex[ i ]->GetHeight() ;

			break ;
		}
	}

	MaterialDataWidth = SPLAT_WIDTH ;
	MaterialDataHeight = SPLAT_HEIGHT ;

	if( !SPLAT_WIDTH )
	{
		// kill everything
		LayerData.Clear();		
		return ;
	}

	LayerData.Resize( MaterialDataWidth * MaterialDataHeight, 0 );

	for( int i = 0, e = SplatTextures::COUNT; i < e; i ++ )
	{
		if( r3dTexture* tex = EditorMatSplatTex[ i ] )
		{
			r3d_assert( SPLAT_WIDTH == tex->GetWidth() );
			r3d_assert( SPLAT_HEIGHT == tex->GetHeight() );
		}
	}

	int TEX_CELL_SIZE_X = SPLAT_WIDTH / m_nTileCountX ;
	int TEX_CELL_SIZE_Y = SPLAT_HEIGHT / m_nTileCountZ ;

	for( uint32_t tj = startTileZ, te = endTileZ; tj < te ; tj ++ )
	{
		for( uint32_t ti = startTileX, te = endTileX; ti < te ; ti ++ )
		{
			int tileIdx = RemapOffsetPC( ti, tj, m_nTileCountX );

			Layers& layers = TileLayers[ tileIdx ];

			mat_data.Clear();
			mat_data.Resize( TEX_CELL_SIZE_X * TEX_CELL_SIZE_Y, 0 );

			for( uint32_t i = 0, e = MAX_LAYERS_PER_TILE; i < e; i ++ )
			{
				uint8_t	lidx = layers[ i ] ;

				if( !lidx )
					continue ;

				if( lidx == WRONG_LAYER_IDX )
					break ;

				int midx = ( lidx - 1 ) / TERRAIN_LAYERS_PER_MAT ;
				int tidx = ( lidx - 1 ) % TERRAIN_LAYERS_PER_MAT ;

				if( r3dTexture* tex = EditorMatSplatTex[ midx ] )
				{
					RECT LockRect ;

					LockRect.left	= ( ti + 0 ) * TEX_CELL_SIZE_X ;
					LockRect.right	= ( ti + 1 ) * TEX_CELL_SIZE_X ;

					LockRect.top	= ( m_nTileCountZ - tj - 1 ) * TEX_CELL_SIZE_Y ;
					LockRect.bottom	= ( m_nTileCountZ - tj - 0  ) * TEX_CELL_SIZE_Y ;

					r3d_assert( tex->GetD3DFormat() == D3DFMT_A8R8G8B8 );

					uint8_t (*comps)[ 4 ] = LockedTextures[ midx ] + LockRect.top * tex->GetLockPitch() / 4 + LockRect.left ;
						
					for( int j = 0, e = TEX_CELL_SIZE_Y; j < e; j ++ )
					{
						for( int i = 0, e = TEX_CELL_SIZE_X; i < e; i ++, comps ++ )
						{
							uint8_t weight = (*comps)[ layer_map[ tidx ] ];

							if( weight > 160 )
							{
								int elIdx = ( TEX_CELL_SIZE_Y - j - 1 ) * TEX_CELL_SIZE_X + i ;

								mat_data[ elIdx ] = R3D_MAX( lidx, mat_data[ elIdx ] ) ;
							}
						}

						comps += ( tex->GetLockPitch() / 4  - TEX_CELL_SIZE_X );
					}
				}
			}

			int xoffset = ti * TEX_CELL_SIZE_X ;

			for( uint32_t j = 0, e = TEX_CELL_SIZE_Y; j < e; j ++ )
			{
				memcpy( &LayerData[ ( j + tj * TEX_CELL_SIZE_Y ) * m_nTileCountX * TEX_CELL_SIZE_X + xoffset ], &mat_data[ j * TEX_CELL_SIZE_X ], TEX_CELL_SIZE_X );
			}
		}
	}

	for( int i = 0, e = TERRAIN_MAT_COUNT ; i < e; i ++ )	
	{
		if( r3dTexture* tex = EditorMatSplatTex[ i ] )
		{
			tex->Unlock( );
		}
	}

	float updateDuration = r3dGetTime() - updateStart ;

	r3dOutToLog( "Terrain material update took %.2f sec\n", updateDuration ) ;


}

//------------------------------------------------------------------------
r3dTexture * r3dTerrain::GetDominantTexture(const r3dPoint3D &pos)
{
	int	X     = (int)(pos.X / CellSize * MaterialDataWidth / Width );
	int	Z     = (int)(pos.Z / CellSize * MaterialDataHeight / Height );

	X = R3D_MIN( R3D_MAX( X, 0 ), (int)MaterialDataWidth - 1 );
	Z = R3D_MIN( R3D_MAX( Z, 0 ), (int)MaterialDataHeight - 1 );

	int idx = LayerData[ X + Z * (int)MaterialDataWidth ] ;

	if( !idx || idx > TERRAIN_MAT_COUNT * TERRAIN_LAYERS_PER_MAT )
	{
		return m_tBaseLayer.pMapDiffuse ;
	}
	else
	{
		int midx = ( idx - 1 ) / TERRAIN_LAYERS_PER_MAT ;
		int lidx = ( idx - 1 ) % TERRAIN_LAYERS_PER_MAT ;

		r3d_assert( midx < TERRAIN_MAT_COUNT ) ;

		return m_dMatLayers[ midx ][ lidx ].pMapDiffuse ;
	}

}


//------------------------------------------------------------------------

void
r3dTerrain::UpdateMaterials()
{
	UpdateMaterials( 0, 0, m_nTileCountX - 1, m_nTileCountZ - 1 );
}

//------------------------------------------------------------------------
/*static*/

void
r3dTerrain::LoadShaders()
{
	r3dRenderer->AddPixelShaderFromFile( "PS_HEAVYNESS", "Nature\\TerrainV2_Heavyness_ps.hls" );

	{
		ShaderMacros macros( 1 );

		macros[ 0 ].Name		= "VS_LQ" ;
		macros[ 0 ].Definition	= "0" ;

		g_TerraVS_ID  = r3dRenderer->AddVertexShaderFromFile( "VS_TERRADEFERRED_MP2", "Nature\\TerrainV2_Deffered_mp2_vs.hls", 0, macros );	

		macros[ 0 ].Definition	= "1" ;
		g_TerraLQ_VS_ID = r3dRenderer->AddVertexShaderFromFile( "VS_TERRADEFERRED_MP2_LQ", "Nature\\TerrainV2_Deffered_mp2_vs.hls", 0, macros );	
	}

	{
		for( int i = 0, e = TerrainShadersArr::COUNT; i < e; i ++ )
		{
			TerrainShaders[ i ] = -1;
		}

		// fill LQ without excessive variations
		for( int i = 1; i <= MAX_SHADER_LAYERS; i ++ )
		{
			TerrainShaderKey key;

			key.Value = 0 ;

			key.Flags.lq = 1 ;
			key.Flags.num_layers = i - 1 ;

			for( int s = 0; s < 2; s ++ )
			{
				key.Flags.simple		= s;

				for ( int z = 0; z < 2; z ++ )
				{
					key.Flags.aux_enabled = z;

					ShaderMacros macros;

					FillShaderMacros( key, macros );

					char Name[] = "PS_UBER_TERRA_0000";
					_snprintf( Name + sizeof Name - 5, 4, "%04d", key.Value );

					r3d_assert( TerrainShaders[ key.Value ] < 0 );
					TerrainShaders[ key.Value ] = r3dRenderer->AddPixelShaderFromFile( Name, "Nature\\TerrainV2_Deffered_mp2_ps.hls", 0, macros );
				}
			}
		}

		for( int i = 1; i <= MAX_SHADER_LAYERS; i ++ )
		{
			TerrainShaderKey key;

			key.Value = 0;

			key.Flags.num_layers	= i - 1;

			for( int s = 0; s < 2; s ++ )
			{
				key.Flags.simple		= s;

				for( int m = 0; m < 2; m ++ )
				{
					key.Flags.multi		= m;

					for ( int z = 0; z < 2; z ++ )
					{
						key.Flags.aux_enabled = z;

						for( int ii = 0, e = ipow2( i ); ii < e ; ii ++ )
						{
							key.Flags.split0 = ii >> 0 & 1;
							key.Flags.split1 = ii >> 1 & 1;
							key.Flags.split2 = ii >> 2 & 1;
							key.Flags.split3 = ii >> 3 & 1;

							ShaderMacros macros;

							FillShaderMacros( key, macros );

							char Name[] = "PS_UBER_TERRA_0000";
							_snprintf( Name + sizeof Name - 5, 4, "%04d", key.Value );

							r3d_assert( TerrainShaders[ key.Value ] < 0 );
							TerrainShaders[ key.Value ] = r3dRenderer->AddPixelShaderFromFile( Name, "Nature\\TerrainV2_Deffered_mp2_ps.hls", 0, macros );
						}
					}
				}
			}
		}
	}

	for( int i = 1; i <= MAX_SHADER_LAYERS; i ++ )
	{
		ShaderMacros macros;

		TerrainShaderKey key;

		key.Value = 0;
		key.Flags.num_layers	= i - 1;
		key.Flags.simple		= 1;
		key.Flags.multi			= 1;

		FillShaderMacros( key, macros );

		D3DXMACRO LightMacro;

		LightMacro.Name			= "USE_LIGHT" ;
		LightMacro.Definition	= "1" ;

		macros.PushBack( LightMacro );

		char Name[] = "PS_MINI_TERRA_000";
		_snprintf( Name + sizeof Name - 4, 3, "%03d", key.Value );

		TerrainMinimapShaders[ i - 1 ] = r3dRenderer->AddPixelShaderFromFile( Name, "Nature\\TerrainV2_Deffered_mp2_ps.hls", 0, macros );
	}

	r3dRenderer->AddPixelShaderFromFile("PS_TERRA_SM", "Nature\\TerrainV2_SM_ps.hls",1);

	{
		ShaderMacros macros( 1 );

		macros[ 0 ].Name		= "VS_LQ" ;
		macros[ 0 ].Definition	= "0" ;

		g_Terra_SM_VS_ID = r3dRenderer->AddVertexShaderFromFile( "VS_TERRA_SM", "Nature\\TerrainV2_SM_vs.hls", 0, macros ) ;

		macros[ 0 ].Definition	= "1" ;
		g_Terra_SM_LQ_VS_ID = r3dRenderer->AddVertexShaderFromFile( "VS_TERRA_SM_LQ", "Nature\\TerrainV2_SM_vs.hls", 0, macros ) ;
	}

}

//------------------------------------------------------------------------
/*static*/

void r3dTerrain::SetNeedShaders( bool NeedShaders )
{
	gNeedShaders = NeedShaders;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------------
CHeightChanged::CHeightChanged()
{
	m_pData.Reserve( 8 );

	m_sTitle = "Height change";

	m_rc.bottom = m_rc.left = INT_MAX;
	m_rc.top = m_rc.right = -INT_MAX;
}


//--------------------------------------------------------------------------------------------------------
void CHeightChanged::Undo()	
{
	Terrain1->ExtractHeightFieldData();

	for ( int i = m_pData.Count() - 1; i >= 0; i-- ) 
	{
		const UndoHeight_t &data = m_pData[ i ];
		r3d_assert( data.nIndex >= 0 && data.nIndex < Terrain1->Width*Terrain1->Height );

		Terrain1->HeightFieldData[ data.nIndex ] = data.fPrevHeight;
	}

	Terrain1->UpdatePhysHeightField();

	Terrain1->UpdateVertexData( m_rc );
}


//--------------------------------------------------------------------------------------------------------
void CHeightChanged::Redo()	
{
	Terrain1->ExtractHeightFieldData();

	for ( int i = 0; i < m_pData.Count(); i++ ) 
	{
		const UndoHeight_t &data = m_pData[ i ];
		r3d_assert( data.nIndex >= 0 && data.nIndex < Terrain1->Width*Terrain1->Height );
		Terrain1->HeightFieldData[ data.nIndex ] = data.fCurrHeight;
	}

	Terrain1->UpdatePhysHeightField();
	Terrain1->UpdateVertexData( m_rc );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

CLayerMaskPaint::PaintData_t::PaintData_t()
: pData( 0 )
, MatIdx( -1 )
{

}

//--------------------------------------------------------------------------------------------------------
CLayerMaskPaint::CLayerMaskPaint()
{
	m_sTitle = "Paint mask";

	m_rc.top = m_rc.left = INT_MAX;
	m_rc.bottom = m_rc.right = -INT_MAX;
}


//--------------------------------------------------------------------------------------------------------
void CLayerMaskPaint::Undo()	
{
	for ( int i = m_pData.Count() - 1; i >= 0; i-- ) 
	{
		const PaintData_t &data = m_pData[ i ];

		r3d_assert( data.MatIdx >= 0 );

		r3dTexture* tex = Terrain1->MatSplatTex[ data.MatIdx ];
		DWORD* colors = (DWORD*)tex->Lock( 1, &data.rc ); // fixme:

		int nLine = data.rc.right - data.rc.left;

		for(int y = 0; y < data.rc.bottom - data.rc.top; y++) 
		{
			uint32_t * pDst = (uint32_t*)&colors[ y * tex->GetWidth() ];
			uint32_t * pSrc = data.pData + y * nLine;
			memcpy( pDst, pSrc, nLine * sizeof( uint32_t ) );
		}

		tex->Unlock();
	}
}


//--------------------------------------------------------------------------------------------------------
void CLayerMaskPaint::Redo()
{
	for ( int i = 0; i < m_pData.Count(); i++ ) 
	{
		const PaintData_t &data = m_pData[ i ];

		r3d_assert( data.MatIdx >= 0 );

		r3dTexture* tex = Terrain1->MatSplatTex[ data.MatIdx ];
		DWORD* colors = (DWORD*)tex->Lock( 1, &data.rc ); // fixme:

		int nLine = data.rc.right - data.rc.left;
		int nSize = nLine * ( data.rc.bottom - data.rc.top );

		for(int y = 0; y < data.rc.bottom - data.rc.top; y++) 
		{
			uint32_t * pDst = (uint32_t*)&colors[ y * tex->GetWidth() ];
			uint32_t * pSrc = data.pData + y * nLine + nSize;
			memcpy( pDst, pSrc, nLine * sizeof( uint32_t ) );
		}

		tex->Unlock();
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------------------------
CLayerColorPaint::CLayerColorPaint()
{
	m_sTitle = "Color paint";

	m_pData.Reserve( 8 );

	m_rc.bottom = m_rc.left = INT_MAX;
	m_rc.top = m_rc.right = -INT_MAX;
}


//--------------------------------------------------------------------------------------------------------
void CLayerColorPaint::Undo()	
{
	for ( int i = m_pData.Count() - 1; i >= 0; i-- ) 
	{
		const UndoColor_t &data = m_pData[ i ];
		r3d_assert( data.nIndex >= 0 && data.nIndex < Terrain1->Width*Terrain1->Height );
		Terrain1->m_pColorData[ data.nIndex ] = data.dwPrevColor;
	}
	Terrain1->UpdateVertexData( m_rc );
}


//--------------------------------------------------------------------------------------------------------
void CLayerColorPaint::Redo()	
{
	for ( int i = 0; i < m_pData.Count(); i++ ) 
	{
		const UndoColor_t &data = m_pData[ i ];
		r3d_assert( data.nIndex >= 0 && data.nIndex < Terrain1->Width*Terrain1->Height );
		Terrain1->m_pColorData[ data.nIndex ] = data.dwCurrColor;
	}
	Terrain1->UpdateVertexData( m_rc );
}

//------------------------------------------------------------------------

r3dTerrain::Floats TempHMap;