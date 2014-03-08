#pragma once

#define AR_GRAZ_PAZ "\\grass"

//////////////////////////////////////////////////////////////////////////

struct GrassCellEntry
{
	GrassCellEntry();

	int					TypeIdx;
	r3dTexture*			MaskTexture;
};

typedef r3dTL::TArray< GrassCellEntry > GrassCellEntries;

struct GrassCell
{
	GrassCell();

	GrassCellEntries	Entries;
	r3dTexture*			HeightTexture;

	r3dPoint3D			Position;

	float				YMax;
};

class GrassMap
{
	// types
public:

	enum Path
	{
		DEPTH_PATH,
		COLOR_PATH,
		COMBINED_PATH
	};

	typedef r3dTL::T2DArray< GrassCell >	GrassCells;
	typedef r3dTL::TArray< unsigned char >	Bytes;

	static const int CELL_HEIGHT_TEX_DIM	= 64;
	static const int CELL_MASK_TEX_DIM		= 32;


	// construction/ destruction
public:
	GrassMap();
	~GrassMap();

	// manipulation/ accesss
public:
	void Init(float width, float height);
	void Close();

	bool Save( const r3dString& levelHomeDir );
	bool Load( const r3dString& levelHomeDir );

	bool HasGrassCells() const;

	// remove masks were all values are approximately 1
	void OptimizeMasks();

	void Paint( float x, float z, float radius, float dir, const r3dString& grassType );
	void UpdateHeight( float x, float z, float radius );
	void UpdateHeight();

	void Prepare() ;
	void Draw( const r3dCamera& cam, Path path, bool useDepthEqual );

	void ConformWithNewCellSize();

	void ClearCells();
	void ClearGrassType( const r3dString& grassType ) ;

	r3dString GetUsedTypes() const ;

	void SetDebugType( const r3dString& typeName ) ;

	static float GetVisRad();
	void GetGridWorldSize(float &x, float &z) const { x = mXLength; z = mZLength; }
	float GetCellSize();

	// helpers
private:
	void DeriveCellCoords( float X, float Z, int& oCellX, int& oCellZ, float& oCellDX, float& oCellDZ );
	void ToWorldCoords( int CellX, int CellZ, float CellDX, float CellDZ, float& oX, float& oZ );
	void CreateCell( int X, int Z );
	void CreateCells( const Bytes& CompoundTex, uint32_t TypeIdx );
	void CreateCellEntry( GrassCell& cell, int X, int Z, uint32_t TypeIdx  );

	void DoPaint( float CentreDX, float CentreDZ, float Radius, float Dir, r3dTexture* Tex );
	void UpdateCellHeight( int X, int Z, GrassCell& cell );
	void DrawCell( const GrassCell& cell, const r3dCamera& Cam, int X, int Z );

	bool SaveCellData( const r3dString& HomeDir );
	bool LoadCellData( const r3dString& HomeDir );

	bool LoadCellData_101( r3dFile* fin );
	bool LoadCellData_102( r3dFile* fin );

	void CreateCellHeightTexture( GrassCell& cell );

	// data
private:
	GrassCells		mCells;

	float			mXLength;
	float			mZLength;

	float			mCellSize;

	bool			mInitialized;

	int				mLoadVersion;

	int				mDebugGrassType ;

};
extern GrassMap* g_pGrassMap;

r3dString GetGrassMapTexPath( const r3dString& LevelPath );
r3dString GetGrassMapHeightTexName( int CellX, int CellZ );
r3dString GetGrassMapMaskTexName( const r3dString& Type, int CellX, int CellZ );

void InitGrass();
void LoadGrassLib();
void UnloadGrassLib();
void AnimateGrass();
void PrepareGrass() ;
void DrawGrass( GrassMap::Path path, bool UseDepthEqual );
void CloseGrass();
float GetGrassHeight(const r3dPoint3D &pt);

