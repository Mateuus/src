#ifndef	__R3D_MAT_H
#define	__R3D_MAT_H

#include "r3dColor.h"
#include "r3dRender.h"

class r3dFile;

enum r3dMatFlags_e
{
	R3D_MAT_ADDOP					= (1<<8),
	R3D_MAT_SUBOP					= (1<<9),

	R3D_MAT_DOUBLESIDED				= (1<<21),
	R3D_MAT_FORCEHASALPHA			= (1<<22),
	R3D_MAT_HASALPHA				= (1<<23),
	R3D_MAT_TRANSPARENT				= (1<<24),
	R3D_MAT_CAMOUFLAGE				= (1<<25),
	R3D_MAT_TRANSPARENT_CAMOUFLAGE	= (1<<26),
	R3D_MAT_TRANSPARENT_CAMO_FP		= (1<<27),
	R3D_MAT_SKIP_DRAW				= (1<<28), // Do not draw batch with this material
	R3D_MAT_PLAYER					= (1<<29)
};

enum r3dMatVSType
{
	R3D_MATVS_DEFAULT,
	R3D_MATVS_SKIN,
	R3D_MATVS_FOREST,
	R3D_MATVS_ANIMATEDFOREST,
	R3D_MATVS_FOREST_NONINSTANCED,
	R3D_MATVS_ANIMATEDFOREST_NONINSTANCED,
	R3D_MATVS_INSTANCING,
	R3D_MATVS_APEX_DESTRUCTIBLE,

	R3D_MATVS_COUNT
};

enum r3dMatPathFlags
{
	R3D_MATF_ROAD				= 0x01,
	R3D_MATF_FORCE_ALPHATEST	= 0x02,
	R3D_MATF_NO_SHADERS			= 0x08,
	R3D_MATF_NO_PIXEL_SHADER	= 0x10
};

//////////////////////////////////////////////////////////////////////////

union FillbufferShaderKey
{
	uint8_t key;
	struct Flags
	{
		uint8_t low_q		: 1;
		uint8_t disp		: 1;
		uint8_t alpha_test	: 1;
		uint8_t roads		: 1;
		uint8_t camouflage	: 1;
		uint8_t simple		: 1;
		uint8_t unused_		: 1;
	};

	Flags flags;
	FillbufferShaderKey(): key(0) {}
};

R3D_FORCEINLINE void SetLQ( FillbufferShaderKey* key )
{
	if( r_lighting_quality->GetInt() == 1 )
	{
		key->flags.low_q = 1;
	}
}

/**	Pixel shader id storage. Modify size of this array when new pins are introduced. */
typedef r3dTL::TFixedArray<int, 128> FillbufferPixelShadersArr;
extern FillbufferPixelShadersArr gFillbufferPixelShaders;
void FillMacrosForShaderKey(FillbufferShaderKey k, ShaderMacros &outMacro);
r3dString ConstructShaderNameForKey(FillbufferShaderKey k);
extern int PS_TRANSPARENT_ID;

#define CAMOUFLAGE_LERP_DURATION 2.0f

//////////////////////////////////////////////////////////////////////////

class r3dMaterial
{
public:
	int	      	ID;
	int         Flags;
	int			WasEdited;	// if it was touched by material editor
	int			AlphaFlagSet ;

	int			RefCount ;

	char        Name[ R3D_MAX_MATERIAL_NAME ];
	char        TypeName[ 32 ]; // from g_pMaterialType
	char        DepotName[ MAX_PATH ];
	char        OriginalDir[ 256 ];

	r3dColor	EmissiveColor;
	r3dColor	DiffuseColor;
	r3dColor	SpecularColor;

	r3dTexture* Texture;
	r3dTexture* BumpTexture;
	r3dTexture*	GlossTexture;
	r3dTexture*	imgEnvPower;
	r3dTexture*	IBLTexture;

	r3dTexture*	DensityTexture;
	r3dTexture* CamouflageMask;
	r3dTexture* DistortionTexture;

	r3dTexture*	DetailNormalTexture;
	r3dTexture* SpecularPowTexture;

	float		DetailScale;
	float		DetailAmmount;

	int			m_DoDispl;
	float		m_DisplDepthVal;

	float		SpecularPower;
	float		SpecularPower1;
	float		ReflectionPower;

	/**	Low quality parameters replacements for textures. */
	float		lowQSelfIllum;
	float		lowQMetallness;

	float		AlphaRef;

	float		TransparencyMultiplier ;
	float		SelfIllumMultiplier ;

	/**	Camouflage data. */
	struct CamoData
	{
		r3dTexture *prevTex;
		r3dTexture *curTex;
		float transitionStartTime;
		CamoData(): prevTex(0), curTex(0), transitionStartTime(0) {}
	};

	static r3dColor RenderedObjectColor ;

private:
	void		Init();
	void	  	Reset();
	r3dTexture*	MatLoadTexture(const char* fname, const char* slot, int DownScale, int MinDownScaleDim, bool player );

	void		StartCommonProperties();
	void		EndCommonProperties();

	void		StartTransparent();
	void		EndTransparent();


public:
	r3dMaterial();
	~r3dMaterial();

	static void	SetRenderedObjectColor( const r3dColor& color ) ;
	void		SetupCompoundDiffuse( D3DXVECTOR4 * outVec, int forTransparency ) ;

	int		LoadAscii(r3dFile *f, const char* szTexturePath);
	int		SaveAscii(FILE *f);

	int		CopyTextures(const char* outDir);

	void		ReloadTextures();

	void		Set();
	void		RenderPrimitive(int Start1, int End1, int Start2, int End2);

	static void	SetupTransparentStates( int UseSkins, int psID = PS_TRANSPARENT_ID );

	void		Start( r3dMatVSType VSType, UINT SetupFlags );
	void		End();

	void		StartShadows( r3dMatVSType VSType );
	void		EndShadows();

	void		SetAlphaFlag();

	static void ResetMaterialFilter();
	static void SetVertexShader( r3dMatVSType VSType, int DoDisplace );
	static void SetShadowVertexShader ( r3dMatVSType VSType );

};

enum
{
	MAX_MAT_FRAME_SCORES	= 4096,
	MAT_FRAME_SCORE_MASK	= 0xFFF
};

extern int gMatFrameScores[MAX_MAT_FRAME_SCORES];

void ResetMatFrameScores();

class r3dMaterialLibrary
{
public:
	static	void	Reset();
	static	int		UnloadManaged();
	static	void	Destroy();

	static	int	LoadLibrary(const char* szFileName, const char* szTexPath);
	static	int	SaveLibrary(const char* szFileName, const char* szTexPath, int bCompact=0);

	static	int& GetCurrent(r3dMaterial**& materialsArray);

	static  void ReloadMaterialTextures();

	static	r3dMaterial*	FindMaterial(const char* MatName);
	static	r3dMaterial*	HasMaterial(const char* MatName);
	static	r3dMaterial*	GetDefaultMaterial() ;

	// will try load material by depot mesh name
	static	bool		IsDynamic;
	static	r3dMaterial*	RequestMaterialByMesh(const char* MatName, const char* MeshName, bool playerMaterial );
	static	r3dMaterial*	RequestMaterialByName(const char* MatName, const char* depot, bool playerMaterial );
	static	void		UpdateDepotMaterials();

	static  void UnloadManagedMaterial( r3dMaterial* mat ) ;
};

void r3dInitMaterials();
void r3dCloseMaterials();

//------------------------------------------------------------------------
// Material shaders

extern int VS_FOREST_ID											;
extern int VS_FOREST_NONINSTANCED_ID							;
extern int VS_FOREST_NOANIM_NONINSTANCED_ID						;
extern int VS_FOREST_NOANIM_ID									;
extern int VS_SKIN_ID											;
extern int VS_SKIN_DEPTH_ID										;
extern int VS_DEPTH_ID											;
extern int VS_SMSKIN_ORTHO_ID									;
extern int VS_SMSKIN_PROJ_ID									;
extern int VS_DEPTH_PREPASS_ID									;
extern int VS_FILLGBUFFER_ID									;
extern int VS_FILLGBUFFER_DISP_ID								;
extern int VS_FILLGBUFFER_APEX_ID								;
extern int VS_FILLGBUFFER_INSTANCED_ID							;
extern int VS_SMDEPTHPASS_ORTHO_ID								;
extern int VS_SMDEPTHPASS_PROJ_ID								;
extern int VS_SMDEPTHPASS_PARABOLOID_PROJ_ID					;
extern int VS_SMDEPTHPASS_ORTHO_INSTANCED_ID					;
extern int VS_SMDEPTHPASS_PROJ_INSTANCED_ID						;
extern int VS_SMDEPTHPATH_FOREST_ORTHO_ID						;
extern int VS_SMDEPTHPATH_FOREST_ORTHO_NONINSTANCED_ID			;
extern int VS_SMDEPTHPATH_FOREST_PROJ_ID						;
extern int VS_SMDEPTHPATH_FOREST_PROJ_NONINSTANCED_ID			;
extern int VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_ID				;
extern int VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_NONINSTANCED_ID	;
extern int VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_ID					;
extern int VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_NONINSTANCED_ID	;
extern int VS_SMDEPTHPATH_APEX_ORTHO_ID							;
extern int VS_SMDEPTHPATH_APEX_PROJ_ID							;

const int MAX_LIGHTS_FOR_TRANSPARENT = 3;
extern int VS_TRANSPARENT_ID[2][MAX_LIGHTS_FOR_TRANSPARENT];
extern int PS_DEPTH_ID;
extern int PS_SMDEPTHPATH_ID;
extern int PS_SMDEPTHPATH_HW_ID;
extern int PS_SMDEPTHPATH_NORMAL_OFFSET_ID;
extern int PS_TRANSPARENT_ID;
extern int PS_TRANSPARENT_AURA_ID;
extern int PS_TRANSPARENT_CAMOUFLAGE_ID;
extern int PS_TRANSPARENT_CAMOUFLAGE_FP_ID;

void SetFillGBufferPixelShader( FillbufferShaderKey k );

enum
{
	// float4 	Material_Params		: register(c0);
	MC_MATERIAL_PARAMS,
	// float4 	Mat_Diffuse			: register(c1);
	MC_MAT_DIFFUSE,
	// float4 	Mat_Specular		: register(c2);
	MC_MAT_SPECULAR,
	// float4	Mat_Glow			: register(c3);
	MC_MAT_GLOW,

	// float4 	CamVec				: register(c4);
	MC_CAMVEC,
	
	// float4	DDepth				: register(c5);
	MC_DDEPTH,

	// float4	CamoInterpolator	: register(c6);
	MC_CAMOINTERPOLATOR,
	// float4	UNUSED				: register(c7);
	MC_UNUSED,
	// float4	MaskColor			: register(c8);
	MC_MASKCOLOR,
	// float	fDisplace			: register(c9);
	MC_DISPLACE,

	// transparent stuff
	// float2	PixelSize			: register(c10);
	MC_PIXELSIZE,
	// float4	SunDir				: register(c11);
	MC_SUNDIR,
	// float4	SunColor			: register(c12);
	MC_SUNCOLOR,
	// float4	AmbientColor		: register(c13);
	MC_AMBIENTCOLOR,

	// float4 Aura					: register(c14);
	MC_AURA,
	// float4 TCamoExtra			: register(c15);
	MC_TCAMO_EXTRA,

	// float4 TCamoColor0			: register(c16);
	MC_TCAMO_COLOR0,

	// float4 TCamoColor1			: register(c17);
	MC_TCAMO_COLOR1,

	// float4 AerialDensity_Distance_Bias	: register (FOG_CONST0); // x - density, y - distance, z - bias
	FOGC_CONST0 = 29,
	// float4 g_fogColor		: register (FOG_CONST1);	//a = density
	FOGC_CONST1 = 30,
	// float4 g_fogParams		: register (FOG_CONST2);	//fadeDist, top, _middle, dist
	FOGC_CONST2 = 31,

	SHADOWC_PIXELDIAMETER = 31

};

R3D_FORCEINLINE
/*static*/
void
r3dMaterial::SetRenderedObjectColor( const r3dColor& color )
{
	RenderedObjectColor = color ;
}

extern void ( *g_SetupFilmToneConstants )( int reg ) ;

#endif //__R3D_MAT_H

