#pragma once

#include "r3dTypeTuples.h"
#include "..\..\..\GameEngine\gameobjects\GameObj.h"

#ifdef FINAL_BUILD
#define R3D_ALLOW_LIGHT_PROBES 0
#else
#define R3D_ALLOW_LIGHT_PROBES 1
#endif

#if R3D_ALLOW_LIGHT_PROBES

struct Probe
{
	enum
	{
		FLAG_SKY_DIRTY = 1,
		FLAG_BOUNCE_DIRTY = 2
	};

	enum
	{
		NUM_DIRS = 4
	};

	typedef r3dTL::TFixedArray< r3dPoint3D, NUM_DIRS > FixedDirArr;
	typedef r3dTL::TFixedArray< D3DXVECTOR4, NUM_DIRS > FixedFloat4Arr;
	typedef r3dTL::TFixedArray< UINT16, NUM_DIRS > FixedUINT16Arr;
	typedef r3dTL::TFixedArray< UINT32, NUM_DIRS > FixedUINT32Arr;
	typedef r3dTL::TFixedArray< float, NUM_DIRS > FixedFloatArr;

	// for for any given direction in radians
	static const float DIR_FOV;
	static FixedDirArr ViewDirs;
	static FixedDirArr UpVecs;

	FixedFloat4Arr SH_BounceR;
	FixedFloat4Arr SH_BounceG;
	FixedFloat4Arr SH_BounceB;

	FixedFloatArr SkyVisibility;

	r3dPoint3D Position;

	FixedUINT16Arr BasisColors;
	FixedUINT32Arr BasisColors32;

	UINT8 Flags;

	Probe();
};

#pragma pack(push)
#pragma pack(1)
union ProbeIdx
{
	struct
	{
		UINT32 TileX : 8;
		UINT32 TileZ : 8;
		UINT32 InTileIdx : 16;
	};

	UINT32 CombinedIdx;
};
#pragma pack(pop)

class ProbeProxy: public GameObject
{
	DECLARE_CLASS(ProbeProxy, GameObject)
public:
	ProbeProxy();

	virtual BOOL		OnPositionChanged();
	virtual	void 		SetPosition(const r3dPoint3D& pos);

	void SetIdx( ProbeIdx idx );

	ProbeIdx Idx;
};


class ProbeMaster
{
	friend class UndoLightProbeCreateDelete;

public:
	typedef r3dTL::TArray< UINT8 > Bytes;
	typedef r3dTL::TArray< Probe > Probes;

	typedef r3dTL::TArray< ProbeIdx > ProbeIdxArray;

	struct ProbeTile
	{
		r3dBox3D			BBox;
		Probes				TheProbes;
		ProbeIdxArray		ProximityProbeMap;

		ProbeTile();
	};

	typedef r3dTL::T2DArray< ProbeTile > ProbeMap;
	typedef r3dTL::TArray< const ProbeTile* > ConstProbeTileArray;
	typedef r3dTL::TFixedArray< D3DXVECTOR4, Probe::NUM_DIRS > BasisArray;

	enum ProbeVisualizationMode
	{
		VISUALIZE_SKY_VISIBILITY,
		VISUALIZE_SKY_SH,
		VISUALIZE_SUN_SH,
		VISUALIZE_SKY_SH_MUL_VISIBILITY,
		VISUALIZE_BOUNCE_DIR0,
		VISUALIZE_BOUNCE_DIR1,
		VISUALIZE_BOUNCE_DIR2,
		VISUALIZE_BOUNCE_DIR3,
		VISUALIZE_BOUNCE_LAST = VISUALIZE_BOUNCE_DIR3,
		VISUALIZE_PROBE_COLORS,
		VISUALIZE_COUNT
	};

	struct Settings
	{
		int ProbeTextureWidth;
		int ProbeTextureHeight;
		int ProbeTextureDepth;

		float ProbeTextureSpanX;
		float ProbeTextureSpanY;
		float ProbeTextureSpanZ;

		D3DFORMAT ProbeTextureFmt;

		float ProbePopulationStepX;
		float ProbePopulationStepY;
		float ProbePopulationStepZ;

		float ProbeElevation;

		int MaxVerticalProbes;

		int NominalProbeTileCountX;
		int NominalProbeTileCountZ;

		Settings();
	};

	struct Info
	{
		float ProbeMapWorldActualXSize;
		float ProbeMapWorldActualYSize;
		float ProbeMapWorldActualZSize;

		float ProbeMapWorldNominalXSize;
		float ProbeMapWorldNominalYSize;
		float ProbeMapWorldNominalZSize;

		float ProbeMapWorldXStart;
		float ProbeMapWorldYStart;
		float ProbeMapWorldZStart;

		int TotalProbeProximityCellsCountX;
		int TotalProbeProximityCellsCountY;
		int TotalProbeProximityCellsCountZ;

		int TileProbeProximityCellsCountX;
		int TileProbeProximityCellsCountZ;

		int ProximityCellsInTileX;
		int ProximityCellsInTileZ;

		float CellSizeX;
		float CellSizeY;
		float CellSizeZ;

		int ActualProbeTileCountX;
		int ActualProbeTileCountZ;

		int ProximityMapSize;

		Info();
	};

	struct SkyDirectColors
	{
		r3dTL::TFixedArray< D3DXVECTOR4, Probe::NUM_DIRS > DirColor;
	};

	typedef r3dTL::TFixedArray< r3dTexture*, Probe::NUM_DIRS > DirectionTextures;
	typedef r3dTL::TArray< ProbeProxy* > ProbeProxies;

public:
	ProbeMaster();
	~ProbeMaster();

public:
	void Init();
	void InitEditor();
	void Close();

	// for given level
	bool IsCreated() const ;

	void Save( const char* levelDir );
	int Load( const char* levelDir );

public:
	void Test();

	void Create( float startX, float startY, float startZ, float sizeX, float sizeY, float sizeZ );

	void UpdateProximity();

	void UpdateSkyVisibility( int DirtyOnly );
	void UpdateBounce( int DirtyOnly );

	void ShowProbes( ProbeVisualizationMode mode );
	void ShowProbeVolumesScheme();

	r3dScreenBuffer* GetBounceRT() const;

	void UpdateProbeVolumes();

	void UpdateSkyAndSun();

	void RelightProbes();

	r3dTexture* GetProbeTexture( int direction ) const;

	const Settings& GetSettings() const;
	const Info&		GetInfo() const;
	void			SetSettings( const Settings& settings );

	void PopulateProbes( int tileX, int tileZ );

	int GetProbeCount( int tileX, int tileZ );
	int GetProbeCount() const;

	void SwitchVolumeFormat();

	r3dPoint3D GetProbeTexturePosition() const;

	void UpdateCamera( const r3dPoint3D& cam );

	int3 GetProximityCell() const;

	const SkyDirectColors& GetSkyDirectColors() const;

	void SelectProbes( float x0, float y0, float x1, float y1 );
	void DeleteProxies();
	void DeleteProxyFor( ProbeIdx idx );

	Probe* GetProbe( ProbeIdx idx );
	ProbeIdx MoveProbe( ProbeIdx idx, const r3dPoint3D& newPos );
	Probe* GetClosestProbe( const r3dPoint3D& pos );

	ProbeIdx AddProbe( const r3dPoint3D& pos );
	ProbeIdx AddProbe( const Probe& probe );

	void DeleteProbes( const r3dTL::TArray< GameObject* > & objects );
	void DeleteProbes( const ProbeIdxArray& probeIdxArray );
	void DeleteProbe( ProbeIdx idx, bool deleteProxy );

	const r3dBoundBox* GetTileBBox( int tileX, int tileZ );

private:
	void RelightProbe( Probe* probe );

	void StartUpdatingBounce();
	void UpdateProbeBounce( Probe* probe );
	void StopUpdatingBounce();

	void StartUpdatingSkyVisibility();
	void UpdateProbeSkyVisibility( Probe* probe );
	void StopUpdatingSkyVisibility();

	void StartProbesVisualization( ProbeVisualizationMode mode );
	void VisualizeProbe( const Probe* probe );
	void StopProbesVisualization();

	void StartProbeDirections();
	void DrawProbeDirection( const Probe* probe, int dirIdx );
	void DrawProbeDirections( const Probe* probe );
	void DrawProbeProximity( const Probe* probe );
	void DrawProximityBox();
	void StopProbeDirections();

	void FillNormalToSHTex();

	void CreateTempRTAndSysMemTex();
	void DestroyTempRTAndSysMemTex();

	void ClearProbeMap();
	void ClearProbeMapTile( int tileX, int tileZ );
	void UpdateBBox( int x, int z, const r3dPoint3D& pos );
	int AddProbe( int x, int z, const Probe& probe );

	void ResetProbeMapBBoxes();
	void ResetProbeMapBBox( int tileX, int tileZ );

	void RecreateVolumeTexes();

	void UpdateProximityMapForTile( int tileX, int tileZ );
	void UpdateProbeProximityMap();
	void UpdateProximityMapSize();

	void FindClosestProbe( int * oIdx, float* oDist, int tileX, int tileZ, float posX, float posY, float posZ );

	void GetProbeTileIndexes( int * oX, int* oZ, float x, float z ) const;

	Probe* GetClosestProbe( int cellx, int celly, int cellz );

	Probe* GetClosestProbe( int tileX, int tileZ, const r3dPoint3D& pos );

	int OutputBakeProgress( const char* operationName, int total, int complete );

	void InitProximityGrid();

	void SelectProbe( ProbeIdx idx, const Probe& p );

	void RecalcTileBBox( int tileX, int tileZ );

	void DrawProbeVolumesFrame();

	void UpdateProximityAndSizeParams();

	void ConvertToTileCoords( float x, float z, int* oTileX, int* oTileZ );

	void FetchAllProbes( Probes* oProbes );

	void AddDirtyProximityTile( ProbeIdx pidx );

	R3D_FORCEINLINE ProbeIdx GetClosestProbeIdx( int cellX, int cellY, int cellZ );
	R3D_FORCEINLINE void SetClosestProbeIdx( int cellX, int cellY, int cellZ, ProbeIdx idx );

private:
	Bytes					m_RTBytesR;
	Bytes					m_RTBytesG;
	Bytes					m_RTBytesB;

	r3dScreenBuffer*		m_TempRT;
	IDirect3DTexture9*		m_SysmemTex;

	r3dScreenBuffer*		m_BounceDiffuseRT;
	r3dScreenBuffer*		m_BounceNormalRT;
	r3dScreenBuffer*		m_BounceAccumSHRRT;
	r3dScreenBuffer*		m_BounceAccumSHGRT;
	r3dScreenBuffer*		m_BounceAccumSHBRT;

	r3dTexture*				m_NormalToSHTex;

	IDirect3DTexture9*		m_BounceSysmemRT_R;
	IDirect3DTexture9*		m_BounceSysmemRT_G;
	IDirect3DTexture9*		m_BounceSysmemRT_B;

	r3dCamera				m_SavedCam;
	int						m_SavedUseOQ;

	ProbeVisualizationMode	m_VisMode;

	ProbeMap				m_ProbeMap;	

	int						m_EditorMode;

	Settings				m_Settings;
	Info					m_Info;

	DirectionTextures		m_DirectionVolumeTextures;

	D3DXVECTOR4				m_SkyDomeSH_R;
	D3DXVECTOR4				m_SkyDomeSH_G;
	D3DXVECTOR4				m_SkyDomeSH_B;

	D3DXVECTOR4				m_SunSH_R;
	D3DXVECTOR4				m_SunSH_G;
	D3DXVECTOR4				m_SunSH_B;

	BasisArray				m_BasisSHArray;

	float					m_SkyVisA;
	float					m_SkyVisB;

	float					m_LastInfoFrame;

	ConstProbeTileArray		m_VisibleProbeTileArray;

	int						m_CamCellX;
	int						m_CamCellY;
	int						m_CamCellZ;

	SkyDirectColors			m_SkyDirColors;

	ProbeProxies			m_ProbeProxies;

	int						m_Created;
	int						m_ProximityMapDirty;

	ProbeIdxArray			m_DirtyProximityTiles;

} extern * g_pProbeMaster;

//------------------------------------------------------------------------

struct ProbeUndoItem
{
	ProbeIdx idx;
	Probe probe;
};

class UndoLightProbeCreateDelete : public IUndoItem
{
public:
	typedef r3dTL::TArray< ProbeUndoItem > ProbeUndoArray;
	static const UndoAction_e ms_eActionID = UA_LIGHTPROBES_CREATE_DELETE;

public:
	UndoLightProbeCreateDelete();

	virtual void				Release			() OVERRIDE;
	virtual UndoAction_e		GetActionID		() OVERRIDE;

	virtual void				Undo			() OVERRIDE;
	virtual void				Redo			() OVERRIDE;

	virtual const FixedString&	GetTitle		() OVERRIDE;

	void				SetIsDelete		( bool isDeleete );
	void				AddItem			( ProbeIdx idx, const Probe& probe );

	static IUndoItem * CreateUndoItem	();
	static void Register();

private:
	void AddProbes( bool select );
	void DeleteProbes();

private:
	ProbeUndoArray	m_UndoArr;
	bool			m_IsDelete;

	static FixedString ms_CreateName;
	static FixedString ms_DelName;
};


#endif