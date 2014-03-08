#ifndef __R3DTERRA_H
#define __R3DTERRA_H

#include "XPSObject.h"
#include "../../eternity/SF/script.h"
#include "../UndoHistory/UndoHistory.h"

#include "../DebugHelpers.h"

#include "ITerrain.h"

class r3dTerrain;
class r3dVertexBuffer;
class r3dIndexBuffer;

#define CQuadTerrain r3dTerrain

extern int __terra_UseHackedWorld;
extern D3DXMATRIX *__terra_HackedWorldMatrix;


struct r3dVTCache
{
 r3dTexture *Array[8][8];
};

struct TerraFaceOrientStats
{
	uint32_t XOrient;
	uint32_t YOrient;
	uint32_t ZOrient;
	uint32_t MultiOrient;

	uint32_t FullXOrient;
	uint32_t FullYOrient;
	uint32_t FullZOrient;
	uint32_t FullMultiOrient;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHeightChanged : public IUndoItem
{
public:

	struct UndoHeight_t
	{
		int			nIndex;
		float		fPrevHeight;
		float		fCurrHeight;
	};

private:

	static const UndoAction_e	ms_eActionID = UA_TERRAIN_HEIGHT;


	r3dTL::TArray< UndoHeight_t >	m_pData;

	RECT				m_rc;

public:

	void				Release			()							{ PURE_DELETE( this ); };
	UndoAction_e		GetActionID		()							{ return ms_eActionID; };

	void				Undo			();
	void				Redo			();

	void				AddData			( const UndoHeight_t & data )	{ m_pData.PushBack( data ); };
	void				AddRectUpdate	( const RECT &rc )				
	{
		if ( m_rc.bottom > rc.bottom ) 
			m_rc.bottom = rc.bottom;
		if ( m_rc.left > rc.left ) 
			m_rc.left = rc.left;
		if ( m_rc.top < rc.top ) 
			m_rc.top = rc.top;
		if ( m_rc.right < rc.right ) 
			m_rc.right = rc.right;
	}

	CHeightChanged	();

	static IUndoItem * CreateUndoItem	()
	{
		return new CHeightChanged;
	};

	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
#ifndef FINAL_BUILD
		g_pUndoHistory->RegisterUndoAction( action );
#endif
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLayerMaskPaint : public IUndoItem
{

public:

	struct PaintData_t
	{
		PaintData_t();

		uint32_t	*	pData; // old data + new data
		RECT			rc;
		int				MatIdx;
	};

private:

	static const UndoAction_e		ms_eActionID = UA_TERRAIN_MASK_PAINT;
	r3dTL::TArray< PaintData_t >	m_pData;

	RECT				m_rc;

public:

	void				Release			()							
	{ 
		for ( int i = 0; i < (int)m_pData.Count(); i++ )
		{
			SAFE_DELETE_ARRAY( m_pData[ i ].pData );
		}
		PURE_DELETE( this ); 
	};
	UndoAction_e		GetActionID		()							{ return ms_eActionID; };

	void				Undo			();
	void				Redo			();

	void				AddData			( const PaintData_t & data )	{ m_pData.PushBack( data ); };
	void				AddRectUpdate	( const RECT &rc )				
	{
		if ( m_rc.bottom < rc.bottom ) 
			m_rc.bottom = rc.bottom;
		if ( m_rc.left > rc.left ) 
			m_rc.left = rc.left;
		if ( m_rc.top > rc.top ) 
			m_rc.top = rc.top;
		if ( m_rc.right < rc.right ) 
			m_rc.right = rc.right;
	}

	CLayerMaskPaint	();

	static IUndoItem * CreateUndoItem	()
	{
		return new CLayerMaskPaint;
	};

	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
#ifndef FINAL_BUILD
		g_pUndoHistory->RegisterUndoAction( action );
#endif
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLayerColorPaint : public IUndoItem
{
public:
	
	struct UndoColor_t
	{
		int			nIndex;
		uint32_t	dwPrevColor;
		uint32_t	dwCurrColor;
	};


private:

	static const UndoAction_e	ms_eActionID = UA_TERRAIN_COLOR_PAINT;
	r3dTL::TArray< UndoColor_t >	m_pData;

	RECT				m_rc;

public:

	void				Release			()							{ PURE_DELETE( this ); };
	UndoAction_e		GetActionID		()							{ return ms_eActionID; };

	void				Undo			();
	void				Redo			();

	void				AddData			( const UndoColor_t & data )	{ m_pData.PushBack( data ); };
	void				AddRectUpdate	( const RECT &rc )				
	{
		if ( m_rc.bottom > rc.bottom ) 
			m_rc.bottom = rc.bottom;
		if ( m_rc.left > rc.left ) 
			m_rc.left = rc.left;
		if ( m_rc.top < rc.top ) 
			m_rc.top = rc.top;
		if ( m_rc.right < rc.right ) 
			m_rc.right = rc.right;
	}

	CLayerColorPaint	();

	static IUndoItem * CreateUndoItem	()
	{
		return new CLayerColorPaint;
	};

	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
#ifndef FINAL_BUILD
		g_pUndoHistory->RegisterUndoAction( action );
#endif
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TERRAIN_MAT_COUNT 8
#define TERRAIN_LAYERS_PER_MAT 4


#pragma pack(push)
#pragma pack(1)
class TerrainTile_c
{
public: 
	float m_HeightMin;
	float m_HeightMax;

	TerrainTile_c()
		: m_HeightMin( 0 )
		, m_HeightMax( 0 )
	{
	}
};
#pragma pack(pop)

typedef r3dTL::TArray< TerrainTile_c > TerrainTiles;

enum
{
	TERRA_CONNECTION_TYPE_COUNT = 16
};


struct TerrainLodDesc
{
	int nStep;
	int nIndexOffset[ TERRA_CONNECTION_TYPE_COUNT ];
	int nTriCount[ TERRA_CONNECTION_TYPE_COUNT ];

	int nConnectorFaceCount;

	float fDist;
};

#define LODDESC_COUNT			4

struct UpdateVertexDataSettings
{
	UpdateVertexDataSettings();

	uint32_t TileCountX;
	uint32_t TileCountZ;

	uint32_t UpdateTileXStart;
	uint32_t UpdateTileXEnd;

	uint32_t UpdateTileZStart;
	uint32_t UpdateTileZEnd;

	uint32_t CellGridDim;

	TerrainTiles*	Tiles;
	TerrainLodDesc (*dLods)[ LODDESC_COUNT ];
	
	int (*RemapOffset)( int , int , int );

	void*		LockedBuffer;
	uint32_t	LockOffset;
};

struct MaterialType;
struct HeightNormalVert;

struct TileCoord
{
	int X;
	int Z;

	bool operator == ( const TileCoord& c )
	{
		return c.X == X && c.Z == Z ;
	}
};

typedef r3dTL::TArray< TileCoord > TileCoords ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
class r3dTerrain : public r3dITerrain
{
public:
	typedef r3dTL::TArray< float >		Floats;
	typedef r3dTL::TArray< PxI16 >		Shorts;
	typedef r3dTL::TArray< uint8_t >	Bytes;
	typedef r3dTL::TArray< uint16_t >	UShorts;

	PxRigidStatic*		physicsTerrain;
private:
	PxHeightFieldDesc physicsHeightFieldDesc;
	PxHeightField*	physicsHeightField;

	int				bNeedUpdatePhysics;
	int				bInHeightEditMode;

	bool			UpdateVertexData		( const UpdateVertexDataSettings& sts );

public:
	TerrainTiles	m_dTiles;
	Bytes			TileLODs;

	enum
	{
		MAX_LAYERS_PER_TILE = 8,
		WRONG_LAYER_IDX		= 0xff
	};

	typedef r3dTL::TFixedArray< uint8_t, MAX_LAYERS_PER_TILE > Layers;

	typedef r3dTL::TArray< Layers > TileLayerArr;

	int			m_nTileCountX;
	int			m_nTileCountZ;

	int			m_nVertPerTile;

	int 		bLoaded;

	// this is allocated in editor only
	uint32_t *	m_pColorData;
	r3dPoint3D*	NormalMapData;
	Floats		HeightFieldData;
	UShorts		EditSplitIndices;
	
	Bytes		LayerData ;
	int			MatTypeIdxes[ TERRAIN_MAT_COUNT * TERRAIN_LAYERS_PER_MAT + 1 ] ;
	
	uint32_t	MaterialDataWidth ;
	uint32_t	MaterialDataHeight ;

	unsigned long heightFieldDataCRC32;

	TileLayerArr	TileLayers;

	// end of it

	float		Width;
	float		Height;
	float		CellSize;			// size of single cell border
	float		WorldSize;			// size of terrain border

	float		TotalWidth;
	float		TotalHeight;

	float		m_HeightmapSize;	// height - y
	float		m_MinHeight;
	float		m_MaxHeight;
	float		m_HFScale;
	float		m_InvHFScale;

	float		m_NormalmapScale;	// 

	float		SplitDistance;

	int			NumMats;

	TerrainLodDesc		m_dLods[ LODDESC_COUNT ];

	uint32_t			m_dOvertileOffset;

	float				GetLodIndex( int TileX, int TileZ );

	r3dVertexBuffer	*				m_HeightNormalVB;
	r3dVertexBuffer *				m_MorphMaskVB;

	IDirect3DVertexDeclaration9 *	m_HeightNormalVDecl;
	IDirect3DVertexDeclaration9 *	m_HeightNormalVDeclLQ;

	r3dVertexBuffer	*				m_VB;
	r3dIndexBuffer	*				m_IB;

	struct Layer_t
	{
		char			MatTypeName[ 32 ];
		r3dTexture	*	pMapDiffuse;
		r3dTexture	*	pMapNormal;
		float			fScale;
		float			fSplit;
		float			fGloss;
		int				bDoSplit;

		Layer_t() 
			: pMapDiffuse( NULL )
			, pMapNormal( NULL )
			, fScale( 1 )
			, fSplit( 0 )
			, fGloss( 0 )
			, bDoSplit( false )
		{
			MatTypeName[ 0 ] = 0;
		}
	};

	struct VisibleTile
	{
		short nValidX; 
		short nValidZ; 
		r3dBox3D tTileBox;
		short nTile;
		float fMirrorNormalX;
		float fMirrorNormalZ;
		float fNormalMapFadeOut;
		
		float fLod;

		bool bNearTile;
		bool bOvertiled;
		bool bRenderBigTile;
		uint8_t ConnectionBits;
	};

	typedef r3dTL::TArray < VisibleTile >							VisibleTiles;
	typedef r3dTL::TFixedArray< r3dTexture*, TERRAIN_MAT_COUNT >	SplatTextures;

	struct TileOrientData
	{
		uint32_t XOffset;
		uint32_t YOffset;
		uint32_t ZOffset;
		uint32_t MOffset;
		uint16_t XCount;
		uint16_t YCount;
		uint16_t ZCount;
		uint16_t MCount;
		uint8_t Orient;
	};

	typedef r3dTL::TArray< TileOrientData >								OrientData;

	VisibleTiles m_dVisibleTiles;


	Layer_t		m_tBaseLayer;

	Layer_t		m_dMatLayers[ TERRAIN_MAT_COUNT ][ TERRAIN_LAYERS_PER_MAT ];

	OrientData	TileOrients;

	int			SplitPShaderIDX;
	int			NormalPShaderIDX;

	int			SelectedTileX;
	int			SelectedTileZ;

	int			HighlightedTileX;
	int			HighlightedTileZ;

	int			LastQLInit;

	void		SelectTile( const r3dPoint3D& pos );
	void		HighlightTile( const r3dPoint3D& pos );
	Layers*		GetSelectedLayers();
	void		ClearLayerAtSelectedTile( int layer );

	void		AppendShadowMapOptimizations( float* oMinX, float* oMaxX, float *oMinY, float* oMaxY );

	bool		LoadFromScript				( const char * fileName );
	bool		LoadLayerFromScript			( Script_c &script, Layer_t &layer );
	void		LoadMaterialsFromScript		( Script_c &script );

	void		CalculateTileFaceOrients	( uint8_t* target, int tileX, int tileY, int cellGridSize, int step, TerraFaceOrientStats& oStats );

	void		SaveData					( const char * fileName, bool writeCache );
	void		SaveDataPS3					( const char * fileName );
	void		SaveLayerToScript			( FILE * hFile, const Layer_t &layer, char * tab );

	void		RecalcParams				();
	void		InitData					();

	void		PreparePhysXHeightFieldDesc	( PxHeightFieldDesc& hfDesc );
	void		FinishPhysXHeightFieldDesc	( PxHeightFieldDesc& hfDesc );

	void		UpdateHFShape();

	// updates from r3dTerrain::HeightFieldData
	void		UpdatePhysHeightField		();
	void		UpdatePhysHeightField		( PxU32* source );

	// extracts into r3dTerrain::HeightFieldData
	void		ExtractHeightFieldData		();
	void		PrepareForSettingsUpdateInGame();
	void		ReleaseSettingsUpdateData();

	void		HeightEditingEnter			();
	void		HeightEditingLeave			();

	void		CreateDefaultPhysicsData	();
	void		CreateDefaultHeightField	();

	void		CreatePhysicsData	( const Shorts& source );

	// to be able to apply texture settings
	void		ReloadTextures() OVERRIDE ;
	void		RecalcLodData();

	void		FillTileSplitIndexes( uint16_t* Indices, int Step, int TileX, int TileY, uint32_t BaseIdxOffset );
	void		RecreateVertexBuffer();
	void		RecreateIndexData();

	void		SetupHFScale();

	PxHeightFieldGeometry GetHFShape() const;

	char		m_szName[ MAX_PATH ];


	SplatTextures	MatSplatTex;
	SplatTextures	EditorMatSplatTex;

	void		EditorSplatOn( int matIdx );
	void		EditorSplatOff( int matIdx );
	bool		IsSplatOn( int matIdx );

	float		__HeightmapBlend;
	float		__TerraLOD1;
	float		__TerraLOD2;
	float		__TerraLOD3;

	float		BaseTerraLOD1;
	float		BaseTerraLOD2;
	float		BaseTerraLOD3;

	enum TilingMode_e
	{
		eTilingMode_None,
		eTilingMode_Once,
		eTilingMode_Infinite
	};

	enum TileRenderMode
	{
		TRM_SIMPLE,
		TRM_SPLIT,
		TRM_HEAVYNESS
	};

	TilingMode_e	StringToTilingMode( const char * str );
	const char *	TilingModeToString( TilingMode_e eMode );
		
	TilingMode_e	m_eTilingMode;

	float			m_fShaderLODFadeStart;
	float			m_fShaderLODFadeEnd;

	inline
	float			GetEditHeight( uint32_t x, uint32_t z ) const
	{
		return HeightFieldData[ x + z * (int)Width ];
	}

	float			GetHeight	( int x, int z ) OVERRIDE
	{
		return physicsHeightField->getHeight( (PxReal)x, (PxReal)z ) * m_InvHFScale;
	};

	void			AppendTileCoords( const r3dPoint3D& from, const r3dPoint3D& to, TileCoords* outCoords );

	inline
	uint32_t		GetEditColor	( uint32_t x, uint32_t z ) const
	{ 
		return m_pColorData[ z * (uint32_t)Width + x ]; 
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	>	Editor only
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	Positions		mDEBUG_TileOrderPoses;

	bool			ImportHeightmap				( const char * fileName, float yScale );
	bool			ExportHeightmap				( const char * fileName );

	bool			ExportColor					( const char * fileName );
	bool			ImportColor					( const char * fileName );

	float			GetHeightFromNormalizeValue	( float normHeight );


public:

	void			ChangeLodParam				();

#ifndef FINAL_BUILD
	IUndoItem *		m_pUndoItem;
	void			BeginUndoRecord				( const char * title, UndoAction_e eAction );
	void			EndUndoRecord				();
	bool			IsUndoRecord				() const { return m_pUndoItem != NULL; }
#endif

	r3dTerrain();
	~r3dTerrain();
	
	// load terrain
	int				Load(const char* dir);

	void			UpdateDesc() ;

	void			PrepareUpdateVertexDataSettings( UpdateVertexDataSettings& oSts, int xstart, int xend, int zstart, int zend );

	template <typename V>
	bool			DoUpdateVertexData		( const UpdateVertexDataSettings& sts );
	bool			UpdateVertexData		( const RECT& rc );
	bool			UpdateAllVertexData		();
	void			UpdateTileSplitIndexes	( int TileX, int TileZ );
	void			CreateVertexDeclaration	();
	void			DefaultInitVertexPos	();
	void			MakeSceneBox			();
	void			ReadColorData(r3dFile* file, int dwVersion, int mapItemCount, bool seekFromEndOfFile );
	void			LoadFormatVersion_X(r3dFile* file, uint32_t dwVersion);
	void			LoadFormatVersion_0(r3dFile* file);
	void			LoadFormatVersion_1(r3dFile* file);
	void			LoadFormatVersion_2(r3dFile* file);
	void			LoadFormatVersion_3(r3dFile* file);
	void			LoadFormatVersion_4(r3dFile* file);
	void			LoadFormatVersion_5(r3dFile* file);
	void			UpdateVertexDataFormat_X(int dwVersion);
	void			UpdateVertexDataFormat_3();
	void			UpdateVertexDataFormat_5();
	void			UpdateVertexDataFormat_0_1_2_4();
	bool			WriteIndexDataCache(uint16_t* startData, uint32_t elementsCount);
	bool			ReadIndexDataCache(uint16_t* startData, uint32_t elementsCount);

	r3dFile*		OpenTerrainBinFile() ;

	void			CreateTempFolders( char* dest ) ;
	const char*		IndexDataCacheName();

	HeightNormalVert*			LoadFormatVersion_2_3_Begin(r3dFile* file);
	void			LoadFormatVersion_2_3_End();
	void			LoadFormatVersion_2_Main(r3dFile* file, HeightNormalVert* locked);
	void			LoadFormatVersion_3_Main(r3dFile* file, HeightNormalVert* locked);


	int				LoadBinaryFormat(const char* dir);

	int				GetCellGridSize				() const;

	void			Unload();

	void			Update ( bool bShadowMap = false );

	void			DrawTile( const VisibleTile &tile, const r3dCamera& Cam, bool SecondPass, TileRenderMode TileRMode );

	// WARNING: not a per-frame function (allocates/frees render target)
	bool			PrepareOthographicTerrainRender() OVERRIDE ;
	bool			DrawOrthographicTerrain( const r3dCamera& Cam, bool UseZ ) OVERRIDE ;

	void			SetupCommonConstants( const r3dCamera& Cam );
	void			SetupCommonConstantsMP( const r3dCamera& Cam );

	void			SetupMaterial( int idx );

	void			DrawStartMP( const r3dCamera &Cam );
	void			DrawEndMP();

	void			DrawDeferredMultipassInitial();
	void			DrawDeferredMultipass();
	void			DrawMaterialHeavyness();
	void			DrawBoxes();

	void			DrawDepth();
	void			DrawBlack( const r3dCamera &Cam, int bSetShader=1);
	void			DrawTerrain( const r3dCamera &Cam, bool secondPath, bool shadowRendering, TileRenderMode tileRenderMode, bool invertCulling, bool allowModifyCulling );

	void			DEBUG_DrawTileOrder();

	void			InitTileLayerArray();
	void			UpdateTileArrayFromSplatTextures();

	void			InitTileLayerArray( TileLayerArr& oArr, int tileCountX, int tileCountZ );
	void			UpdateTileArrayFromSplatTextures( TileLayerArr& oArr, int tileCountX, int tileCountZ, int cellGridSize, int (*RemapOffset)( int , int , int ) );

	void			InsertLayer( int TileX, int TileZ, int Type );
	void			OptimizeLayers();

	float			GetCellSize();
	void			SetCellSize(float NewCellSize);

	// return height at given point
	float			GetEditHeight( const r3dPoint3D &pnt );
	float			GetHeight( const r3dPoint3D &pnt ) OVERRIDE ;

	void			GetHeightRange( float* oMinHeight, float* oMaxHeight, r3dPoint2D start, r3dPoint2D end ) OVERRIDE ;

	r3dPoint3D		GetNormal(const r3dPoint3D &pnt) OVERRIDE ;

	const MaterialType*	GetMaterialType( const r3dPoint3D& pnt ) OVERRIDE ;

	bool			IsLoaded() OVERRIDE ;

	float			GetTotalWidth() const;
	float			GetTotalHeight() const;

#ifndef FINAL_BUILD
	void			ApplyBrush(const r3dPoint3D &pnt, const float strength, const float radius, const float hardness);
	void			ApplyHeight(const r3dPoint3D &pnt, const float H, const float strength, const float radius, const float hardness);
	void			ApplySmooth(const r3dPoint3D &pnt, const float radius);
	void			ApplyErosion(const r3dPoint3D &pnt, const float strength, const float radius, const float hardness);
	void			ApplyNoise(const r3dPoint3D &pnt, const float strength, int bPositiveOnly, const float radius, const float hardness);
	void			ApplyRamp(const r3dPoint3D& rampStart, const r3dPoint3D& rampEnd, const float rampWidthOuter, const float rampWidthInner);
	void			ApplyMaskBrush(const r3dTerrainPaintBoundControl& boundCtrl, const r3dPoint3D &pnt, int opType, int layerIdx, float val, const float radius, const float hardness );
	void			ApplyColor	(const r3dTerrainPaintBoundControl& boundCtrl, const r3dPoint3D &pnt, const r3dColor &dwColor, const float strength, const float radius, const float hardness);
#endif

	void			UpdateMaterials( uint32_t startTileX, uint32_t startTileZ, uint32_t endTileX, uint32_t endTileZ );
	void			UpdateMaterials();

	static void		LoadShaders();
	static void		SetNeedShaders( bool NeedShaders );

	r3dTexture*		GetDominantTexture( const r3dPoint3D &pos ) OVERRIDE ;

};

extern	r3dITerrain	*Terrain;

extern	r3dTerrain	*Terrain1;

void InitErosionPattern(const char *filePath);
void FreeErosionPattern();

struct ErosionPattern
{
	DWORD w, h;
	byte *data;

	ErosionPattern() ;

	explicit  ErosionPattern(const char *ddsFileName) ;
	
	~ErosionPattern() ;

	float Sample(float u, float v) ;

	float SampleBilinear(float u, float v) ;

};

extern ErosionPattern *gErosionPattern ;

#endif // __R3DTERRA_H