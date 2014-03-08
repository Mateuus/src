#ifndef	__R3D_MESH_H
#define	__R3D_MESH_H

#define R3D_PIVOT_CENTER	1
#define R3D_PIVOT_BOTTOM	2

#include "MeshGlobalBuffer.h"

#define R3D_ALLOW_ASYNC_MESH_LOADING 1

class r3dVertexBuffer;

#pragma pack(push)
#pragma pack(1)

// mesh, skinned
struct r3dSkinnedMeshVertex
{
	const static D3DVERTEXELEMENT9 VBDecl[];
private:
	static LPDIRECT3DVERTEXDECLARATION9	pDecl;
public:
	short4	 	pos;
	short2 		texcoord;   // The texture coordinates
	ubyte4		normal;
	ubyte4		tangent;
	short4		weights;	// 4 blend weights
	ubyte4		indices;	// 4 blend indices

	static LPDIRECT3DVERTEXDECLARATION9 getDecl();
};

struct r3dInstancedDataVertex
{
private:
	static LPDIRECT3DVERTEXDECLARATION9	pDecl;
public:
	static D3DVERTEXELEMENT9 VBDecl[];
	D3DXMATRIX world;

	static LPDIRECT3DVERTEXDECLARATION9 getDecl();
};
struct r3dInstancedUnionMeshVertex
{
private:
	static LPDIRECT3DVERTEXDECLARATION9	pDecl;
public:
	static D3DVERTEXELEMENT9 VBDecl[];
	r3dPoint3D 	Pos;
	r3dPoint3D	Normal;
	r3dColor24 	color;
	float 		tu, tv;   // The texture coordinates
	r3dVector	vU;
	r3dVector	vV;

	D3DXMATRIX world;

	static LPDIRECT3DVERTEXDECLARATION9 getDecl();
};

struct PrecalculatedMeshVSConsts
{
	float4 worldViewProj[ 4 ] ;
	float4 scaledWorld[ 3 ] ;
	float4 normalWorld[ 3 ] ;
	float4 scaledWorldView[ 3 ] ;
	float4 normalWorldView[ 3 ] ;
	float4 texcUnpackScale ;
};

#pragma pack(pop)

struct MeshDeferredRenderable : Renderable
{
	static void Draw( Renderable* RThis, const r3dCamera& Cam );

	int						BatchIdx;
	DWORD					Color;
	r3dMesh*				Mesh;
};

struct MeshShadowRenderable : Renderable
{
	static void DrawSingleBatch( Renderable* RThis, const r3dCamera& Cam );
	static void Draw( Renderable* RThis, const r3dCamera& Cam );

	void (*SubDrawFunc)( Renderable* This, const r3dCamera& Cam );

	r3dMesh*				Mesh;
	int						BatchIdx;
};

class MeshTextureDensityVisualizer;

//---------------------------------------------------------

class r3dMesh
{
private:
	struct InstancedData
	{
		D3DXMATRIX world;
	};
	// not memory efficient, but PC will handle :) If not, will refactor later (do not port that to PS3!!)
	static const int MaxInstancesToDraw = 64;
	InstancedData InstanceArray[MaxInstancesToDraw];
	int	numInstancesInArray;
	static r3dVertexBuffer *	pInstancesVB;
	static int					numInstancesInVB;
	volatile LONG				m_Loaded ;
	volatile LONG				m_Drawable ;

public:

	enum EObjectFlags
	{
		obfStatic           = 0x0001,	// Can't be moved/rotated
		obfNoRotate         = 0x0002,	// Can't be rotated, but can be moved
		obfNotClipable      = 0x0008,	// Object is not clipable
		obfDynaLightApplied   = 0x0100,	// dynamic lights applied.
		obfSaveColors         = 0x0200,
		obfMissingSource      = 0x0400,	// no .SCO file when loading mesh ( only .SCB was there )
		obfPlayerMesh         = 0x0800, // add to player buffer when calculating statistics
	};

	enum EVertexFlags
	{
		vfPrecise			= 0x0001,
		vfBending			= 0x0002,
		vfUnsharedBuffer	= 0x0004
	};


	char		Name[R3D_MAX_OBJECT_NAME];
	r3dString	FileName;	

	r3dBox3D	localBBox;
	r3dPoint3D	CentralPoint;
	r3dPoint3D  vPivot;

	int			RefCount ;

	int			NumVertices;
	int			NumIndices;

	r3dPoint3D	unpackScale;
	r3dPoint2D	texcUnpackScale;

	static const int ConstNumMatChunks = 256;
	r3dTriBatch	MatChunks[256];
	char**		MatChunksNames; // used when saving to bin file format only
	int			NumMatChunks;

	int			Flags;

	int			VertexFlags ;

	MeshGlobalBuffer::Entry buffers;
	r3dVertexBuffer*		UnsharedVertexBuffer ;
	r3dIndexBuffer*			UnsharedIndexBuffer ;

	r3dPoint3D*	VertexPositions;
	r3dVector*	VertexNormals;
	r3dPoint2D*	VertexUVs;
	r3dPoint3D*	VertexTangents;
	char*		VertexTangentWs;
	r3dColor*	VertexColors;

	uint32_t*	Indices;

	int			SizeInVMem ;

	int			HasAlphaTextures;
	float		ExtrudeAmmount;
public:
	r3dMesh( int UsePreciseVertices );
	virtual ~r3dMesh();

	int			GetNumVertexes	() const				{ return NumVertices; }
	int			GetNumMaterials	() const				{ return NumMatChunks; }
	int			GetNumPolygons	() const;
	int			GetMaterialsSize() const;

	const r3dPoint3D& getCentralPoint() const { return CentralPoint; }
	const r3dPoint3D& getPivot() const { return vPivot; }

	r3dMaterial*	GetFaceMaterial( int faceIdx ) const;

	virtual r3dPoint3D* GetVertexPositions() const ;
	virtual uint32_t*	GetIndices() const ;

	virtual const r3dPoint3D &	GetVertexPos ( int nIndex ) const { return VertexPositions[ nIndex ]; }

	void		InitVertsList(int NumVerts);
	void		InitIndexList(int numIndexes);

	// flags
	BOOL		IsFlagSet(int flag)		{ return BOOL(Flags & flag);	}
	void		SetFlag(int flag, BOOL bSet)	{ bSet ? (Flags |= flag) : (Flags &= ~flag);	}
	BOOL		IsDrawable() const			{ return m_Drawable ; }
	BOOL		IsLoaded() const			{ return m_Loaded ; }
	void		SetDrawable();
	void		SetLoaded() ;

	void		Unload();


	// I/O functions
	bool		Load(const char* fname, bool use_default_material = false, bool force_sync = true );
	void		FindAlphaTextures();

	static void Init() ;
	static void Close() ;
private:

	static void DoLoadMesh( struct r3dTaskParams* params );
	bool		DoLoad( bool use_default_material ) ;

	int 		LoadAscii(r3dFile *f, bool use_default_material );

	bool 		LoadBin(r3dFile *f, bool use_default_material );
	bool 		SaveBin(const char* fname);
	bool 		SaveBinPS3(const char* fname);

	void		ResetXForm();
public:

	static bool	CanLoad( const char* fname ) ;

	void		NormalizeTexcoords() ;

	static void	FillMeshBuffers( r3dTaskParams* params ) ;
	void		FillBuffersAsync() ;
	void		FillBuffers() ;

	// fill buffers code for 2 types of vertices
	template <typename VERTEX>
	void		FillBuffersUnique();
	template <typename VERTEX> 
	void		FillSingleVertexBase(VERTEX* pV, int index);
	void		FillSingleVertex(void *pV_, int index);

	// weight
	struct r3dWeight
	{
		BYTE	BoneID[4];
		float	Weight[4];
	};
	r3dWeight*	pWeights;
	int		vbCurSize_;

private:
	friend void DoFillBuffersMeshMainThread( void* Ptr ) ;
	void		DoFillBuffersMainThread() ;
	void		DoFillBuffers() ;

	void		AllocateWeights();
	void		TryLoadWeights(const char* baseFileName);

	void		LoadWeights(const char* fname);
	void		LoadWeights_BinaryV1(r3dFile* f, bool from_scb);
	bool		SaveWeights_BinaryV1(FILE* f);
	void		DeleteWeights();
public:

	//Drawing
	void		GetWorldMatrix( const r3dVector& vPos, const r3dVector& vScl, const r3dVector& vRot, D3DXMATRIX &mRes );
	void		SetVSConsts(const r3dVector& pos, const r3dVector& scale, const D3DXMATRIX& rotation);
	void		SetVSConsts(const D3DXMATRIX& world);

	void		AppendShadowRenderables( RenderArray& oArr );
	void		AppendRenderablesDeferred( RenderArray& oArr, const r3dColor& color );
	void		AppendTransparentRenderables( RenderArray& oArr, const r3dColor& color, float dist, int forceAll );
	void		DrawMeshStart( const r3dColor* objectColor );
	void		DrawMeshEnd();
	void		DrawMeshDeferredBatch( int batchIdx, UINT MatFlags );
	void		DrawMeshDeferred( const r3dColor& objectColor, UINT MatFlags );		// Fill G-Buffer
	void		DrawMeshWithoutMaterials();
	void		DrawMeshInstanced( const D3DXMATRIX* world, int ShadowMap, bool force_render=false );
	void		DrawMeshSimple( int bTextured );
	void		DrawMeshShadowsBatch( int batchIdx );
	void		DrawMeshShadows();

	void		DrawMeshStartInstances ();
	void		DrawMeshEndInstances ();
	void		DrawMeshSimpleInstances ( int instanceCount );

	void		SetupTexcUnpackScaleVSConst() const ;

	void		RecalcBoundBox();
	void		RecalcBasisVectors();
	void		RecalcTangentWs( r3dPoint3D* VertexBinormals );	

	void		OptimizeVCache();

	template <typename T>
	static void	RemapComponent( T* comp, const std::vector<int>& compMap ) ;

	// collisions
	virtual	BOOL		ContainsRay(const r3dPoint3D& vStart, const r3dPoint3D& vNormalizedDirection, float RayLen, float *ClipDist, r3dMaterial **material, const r3dVector& pos, const D3DXMATRIX& rotation, int * OutMinFace = NULL );
	virtual	BOOL		ContainsQuickRay(const r3dPoint3D& vStart, const r3dPoint3D& vNormalizedDirection, float RayLen, float *ClipDist, r3dMaterial **material, const r3dVector& pos, const D3DXMATRIX& rotation);

	int			IsSkeletal() const ;

	static void	FlushLoadingBuffers() ;

private:
	static void MeshLoaded ( const char* sMesh );
	static void MeshUnloaded ( const char* sMesh );

#ifndef FINAL_BUILD
	MeshTextureDensityVisualizer *densityVisualizer;
#endif

public:

	typedef void ( CallbackMeshLoadUnload_t )( const char * szFileName );

	static CallbackMeshLoadUnload_t * g_pMeshLoadCallbacker;
	static CallbackMeshLoadUnload_t * g_pMeshUnloadCallbacker;

};

void r3dMeshSetVSConsts( const D3DXMATRIX& world, const r3dPoint3D* UnpackScale, const r3dPoint2D* TexcUnpackScale );
void r3dPrepareMeshVSConsts(PrecalculatedMeshVSConsts &out, const D3DXMATRIX &world, const r3dPoint3D* ScaleBox, const r3dPoint2D* TexcUnpackScale );
/**	Set matrices into shader constants. */
void r3dMeshSetVSTexcUnpackScale( r3dPoint2D unpackScale ) ;
void r3dApplyPreparedMeshVSConsts(const PrecalculatedMeshVSConsts &vsc);
void r3dApplyPreparedMeshVSConsts_DepthPrepass(const PrecalculatedMeshVSConsts &vsc);

R3D_FORCEINLINE int GetRevIDist( float dist )
{
	return 0xffff - R3D_MIN( (int)dist, 0xffff );
}

R3D_FORCEINLINE int r3dMesh::IsSkeletal() const
{
	return pWeights ? 1 : 0 ;
}

R3D_FORCEINLINE void r3dMesh::SetupTexcUnpackScaleVSConst() const
{
	r3dMeshSetVSTexcUnpackScale( this->texcUnpackScale ) ;
}

void FillPosition( R3D_MESH_VERTEX& vertex, const r3dPoint3D& pos, const r3dPoint3D& unpackScale );
void FillPosition( R3D_BENDING_MESH_VERTEX& vertex, const r3dPoint3D& pos, const r3dPoint3D& unpackScale );
void FillTexcoord( R3D_MESH_VERTEX& vertex, const r3dPoint2D& uv, const r3dPoint2D& texcUnpackScale );
void FillTexcoord( R3D_BENDING_MESH_VERTEX& vertex, const r3dPoint2D& uv, const r3dPoint2D& texcUnpackScale );
r3dPoint2D UnpackTexcoord(short u, short v, const r3dPoint2D &texcUnpackScale);

void ToSkinFileName( char (&skinFile)[256], const char* baseFile );

#endif //__R3D_MESH_H
