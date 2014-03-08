#include "r3dPCH.h"
#include "r3d.h"

#include "Particle.h"

#include "r3dBinMat.h"
#include "../../../GameEngine/TrueNature/Sun.h"

static	int 	r3dMaterialIDGenerator = 100;
extern 	int 	CurrentTexID[16];

	// flag for skipping material loading
	int	_r3d_MatLib_SkipAllMaterials = 0;

const static char* DEPOT_TAG  = "data/objectsdepot";
const static int   DEPOT_TLEN = 17;

int VS_FOREST_ID										= -1 ;
int VS_FOREST_NONINSTANCED_ID							= -1 ;
int VS_FOREST_NOANIM_ID									= -1 ;
int VS_FOREST_NOANIM_NONINSTANCED_ID					= -1 ;
int VS_SKIN_ID											= -1 ;
int VS_SKIN_DEPTH_ID									= -1 ;
int VS_DEPTH_ID											= -1 ;
int VS_SMSKIN_ORTHO_ID									= -1 ;
int VS_SMSKIN_PROJ_ID									= -1 ;
int VS_DEPTH_PREPASS_ID									= -1 ;
int VS_FILLGBUFFER_ID									= -1 ;
int VS_FILLGBUFFER_DISP_ID								= -1 ;
int VS_FILLGBUFFER_APEX_ID								= -1 ;
int VS_FILLGBUFFER_INSTANCED_ID							= -1 ;
int VS_SMDEPTHPASS_ORTHO_ID								= -1 ;
int VS_SMDEPTHPASS_PROJ_ID								= -1 ;
int VS_SMDEPTHPASS_PARABOLOID_PROJ_ID					= -1 ;
int VS_SMDEPTHPASS_ORTHO_INSTANCED_ID					= -1 ;
int VS_SMDEPTHPASS_PROJ_INSTANCED_ID					= -1 ;
int VS_SMDEPTHPATH_FOREST_ORTHO_ID						= -1 ;
int VS_SMDEPTHPATH_FOREST_ORTHO_NONINSTANCED_ID			= -1 ;
int VS_SMDEPTHPATH_FOREST_PROJ_ID						= -1 ;
int VS_SMDEPTHPATH_FOREST_PROJ_NONINSTANCED_ID			= -1 ;
int VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_ID				= -1 ;
int VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_NONINSTANCED_ID	= -1 ;
int VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_ID				= -1 ;
int VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_NONINSTANCED_ID	= -1 ;
int VS_SMDEPTHPATH_APEX_ORTHO_ID						= -1 ;
int VS_SMDEPTHPATH_APEX_PROJ_ID							= -1 ;

int VS_TRANSPARENT_ID[2][MAX_LIGHTS_FOR_TRANSPARENT] = 
{
	{ -1, -1, -1 }, 
	{ -1, -1, -1 } 
};

int PS_DEPTH_ID							= -1 ;
int PS_SMDEPTHPATH_ID					= -1 ;
int PS_SMDEPTHPATH_HW_ID				= -1 ;
int PS_SMDEPTHPATH_NORMAL_OFFSET_ID		= -1 ;
int PS_TRANSPARENT_ID					= -1 ;
int PS_TRANSPARENT_AURA_ID				= -1 ;
int PS_TRANSPARENT_CAMOUFLAGE_ID		= -1 ;
int PS_TRANSPARENT_CAMOUFLAGE_FP_ID		= -1 ;

static r3dMaterial* gPrevMaterial = NULL ;
static r3dMaterial* gPrevShadowMaterial = NULL ;
extern r3dScreenBuffer*	DepthBuffer;
extern r3dSun *Sun;

int gNumForwardLightsForTransparent = 0;

FillbufferPixelShadersArr gFillbufferPixelShaders;

void SetFillGBufferPixelShader( FillbufferShaderKey k )
{
	int id = gFillbufferPixelShaders[ k.key ] ;

	r3d_assert( id >= 0 ) ;

	r3dRenderer->SetPixelShader( id );
}

int gMatFrameScores[MAX_MAT_FRAME_SCORES];

// pointer to envmap probes function
r3dTexture* (*r3dMat_EnvmapProbes_GetClosestTexture)(const r3dPoint3D& pos) = NULL;
// pointer to gCamouflageDataManager::GetCurrentData
const r3dMaterial::CamoData& (*r3dMat_gCamouflageDataManager_GetCurrentData)() = NULL;
             
void ( *g_SetupFilmToneConstants )( int reg ) ;

//////////////////////////////////////////////////////////////////////////

void FillMacrosForShaderKey(FillbufferShaderKey k, ShaderMacros &outMacro)
{
	outMacro.Resize(5);

	outMacro[0].Name		= "HIGH_QUALITY";
	outMacro[0].Definition	= k.flags.low_q ? "0" : "1";

	outMacro[1].Name		= "ALPHA_CUT";
	outMacro[1].Definition	= k.flags.alpha_test ? "1" : "0";

	outMacro[2].Name		= "FILL_ROADS";
	outMacro[2].Definition	= k.flags.roads ? "1" : "0";

	outMacro[3].Name		= "CAMOUFLAGE";
	outMacro[3].Definition	= k.flags.camouflage ? "1" : "0";

	outMacro[4].Name		= "SIMPLE";
	outMacro[4].Definition	= k.flags.simple ? "1" : "0";
}

//////////////////////////////////////////////////////////////////////////

r3dString ConstructShaderNameForKey(FillbufferShaderKey k)
{
	r3dString name = "PS_FILLGBUFFER";
	if (k.flags.low_q) name += "_LQ";
	if (k.flags.disp ) name += "_DISP";
	if (k.flags.roads) name += "_FILL_ROADS";
	if (k.flags.alpha_test) name += "_ATEST";
	if (k.flags.camouflage) name += "_CAMOUFLAGE";
	if (k.flags.simple) name += "_SIMPLE";

	return name;
}

//////////////////////////////////////////////////////////////////////////

r3dMaterial::r3dMaterial()
{
	Init();
}

r3dMaterial::~r3dMaterial()
{
	Reset();
}

//------------------------------------------------------------------------

void r3dMaterial::Init()
{
	r3dscpy(Name, "_undefined_");
	DepotName[0] = 0;
	r3dscpy(OriginalDir, "Data\\Images");

	TypeName[ 0 ] = 0;

	RefCount = 0 ;

	Flags          = 0;
	WasEdited      = 0;
	AlphaFlagSet = 0 ;
	EmissiveColor		= r3dColor(0,0,0);
	DiffuseColor		= r3dColor(255, 255, 255);
	SpecularColor		= r3dColor(255,255,255);

	SelfIllumMultiplier = 0.f ;

	GlossTexture		= NULL;
	Texture		= NULL;
	BumpTexture		= NULL;
	imgEnvPower		= NULL;
	IBLTexture		= NULL;

	DensityTexture		= NULL;
	DistortionTexture = 0;
	SpecularPowTexture = 0;

	DetailNormalTexture = NULL;

	DetailScale		= 10.0f;
	DetailAmmount	= 0.3f;

	SpecularPower	= 0;
	ReflectionPower	= 0;
	SpecularPower1	= 0.5;

	AlphaRef		= 0;

	ID             = r3dMaterialIDGenerator++;

	m_DoDispl = false;
	m_DisplDepthVal = 0.1f;

	CamouflageMask = 0;

	lowQSelfIllum = 0;
	lowQMetallness = 0;

	TransparencyMultiplier = 1 ;

}


void r3dMaterial::Reset()
{
	if(Texture)
		r3dRenderer->DeleteTexture(Texture);

	if(BumpTexture)
		r3dRenderer->DeleteTexture(BumpTexture);

	if(GlossTexture)
		r3dRenderer->DeleteTexture(GlossTexture);

	if(imgEnvPower)
		r3dRenderer->DeleteTexture(imgEnvPower);

	if(IBLTexture)
		r3dRenderer->DeleteTexture(IBLTexture);

	if(DetailNormalTexture)
		r3dRenderer->DeleteTexture(DetailNormalTexture);

	if(DensityTexture)
		r3dRenderer->DeleteTexture(DensityTexture);

	if(CamouflageMask)
		r3dRenderer->DeleteTexture(CamouflageMask);

	if(DistortionTexture)
		r3dRenderer->DeleteTexture(DistortionTexture);

	if(SpecularPowTexture)
		r3dRenderer->DeleteTexture(SpecularPowTexture);
}


void r3dMaterial::Set()
{
}

#ifndef FINAL_BUILD
void SetTextureAsMipmapTest(r3dTexture* tex, int stageID)
{
	extern r3dTexture *__r3dMipLevelTextures[5];
	if(tex->GetNumMipmaps() < 9)
		r3dRenderer->SetTex(__r3dShadeTexture[2], stageID); // white
	else if(tex->GetNumMipmaps() > 13)
		r3dError("Texture (%s) has more than 13 mip levels!!! WTF!!! It's more than 4096x4096\n", tex->getFileLoc().FileName);
	else
		r3dRenderer->SetTex(__r3dMipLevelTextures[13-tex->GetNumMipmaps()], stageID);
}

//////////////////////////////////////////////////////////////////////////

void SetTextureDensityChecker(int stageID)
{
	r3dRenderer->SetTex(__r3dShadeTexture[9], stageID);
}
#endif

static int VSShaderMap	[ R3D_MATVS_COUNT ]
						[ 2 ]	// displacement on / off
						;

static int VSShadowShaderMap	[ R3D_MATVS_COUNT ]
								[ 2 ] // perspective or ortho
								;


enum
{
	DISPLACE_OFF,
	DISPLACE_ON
};

static void FillVSShaderMaps()
{
	//------------------------------------------------------------------------
	VSShaderMap	[ R3D_MATVS_DEFAULT							][ DISPLACE_OFF		] = VS_FILLGBUFFER_ID;
	VSShaderMap	[ R3D_MATVS_SKIN							][ DISPLACE_OFF		] = VS_SKIN_ID ;
	VSShaderMap	[ R3D_MATVS_FOREST							][ DISPLACE_OFF		] = VS_FOREST_NOANIM_ID ;
	VSShaderMap	[ R3D_MATVS_FOREST_NONINSTANCED				][ DISPLACE_OFF		] = VS_FOREST_NOANIM_NONINSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_ANIMATEDFOREST					][ DISPLACE_OFF		] = VS_FOREST_ID ;
	VSShaderMap	[ R3D_MATVS_ANIMATEDFOREST_NONINSTANCED		][ DISPLACE_OFF		] = VS_FOREST_NONINSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_INSTANCING						][ DISPLACE_OFF		] = VS_FILLGBUFFER_INSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_APEX_DESTRUCTIBLE				][ DISPLACE_OFF		] = VS_FILLGBUFFER_APEX_ID;

	VSShaderMap	[ R3D_MATVS_DEFAULT							][ DISPLACE_ON		] = VS_FILLGBUFFER_DISP_ID ;
	VSShaderMap	[ R3D_MATVS_SKIN							][ DISPLACE_ON		] = VS_SKIN_ID ;
	VSShaderMap	[ R3D_MATVS_FOREST							][ DISPLACE_ON		] = VS_FOREST_NOANIM_ID ;
	VSShaderMap	[ R3D_MATVS_FOREST_NONINSTANCED				][ DISPLACE_ON		] = VS_FOREST_NOANIM_NONINSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_ANIMATEDFOREST					][ DISPLACE_ON		] = VS_FOREST_ID ;
	VSShaderMap	[ R3D_MATVS_ANIMATEDFOREST_NONINSTANCED		][ DISPLACE_ON		] = VS_FOREST_NONINSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_INSTANCING						][ DISPLACE_ON		] = VS_FILLGBUFFER_INSTANCED_ID ;
	VSShaderMap	[ R3D_MATVS_APEX_DESTRUCTIBLE				][ DISPLACE_ON		] = VS_FILLGBUFFER_APEX_ID;

	//------------------------------------------------------------------------

	VSShadowShaderMap	[ R3D_MATVS_DEFAULT							][ SPT_ORTHO ] = VS_SMDEPTHPASS_ORTHO_ID;
	VSShadowShaderMap	[ R3D_MATVS_SKIN							][ SPT_ORTHO ] = VS_SMSKIN_ORTHO_ID ;
	VSShadowShaderMap	[ R3D_MATVS_FOREST							][ SPT_ORTHO ] = VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_ID ;
	VSShadowShaderMap	[ R3D_MATVS_FOREST_NONINSTANCED				][ SPT_ORTHO ] = VS_SMDEPTHPATH_FOREST_ORTHO_NOANIM_NONINSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_ANIMATEDFOREST					][ SPT_ORTHO ] = VS_SMDEPTHPATH_FOREST_ORTHO_ID ;
	VSShadowShaderMap	[ R3D_MATVS_ANIMATEDFOREST_NONINSTANCED		][ SPT_ORTHO ] = VS_SMDEPTHPATH_FOREST_ORTHO_NONINSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_INSTANCING						][ SPT_ORTHO ] = VS_SMDEPTHPASS_ORTHO_INSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_APEX_DESTRUCTIBLE				][ SPT_ORTHO ] = VS_SMDEPTHPATH_APEX_ORTHO_ID;
						
	VSShadowShaderMap	[ R3D_MATVS_DEFAULT							][ SPT_PROJ	] = VS_SMDEPTHPASS_PROJ_ID;
	VSShadowShaderMap	[ R3D_MATVS_SKIN							][ SPT_PROJ	] = VS_SMSKIN_PROJ_ID ;
	VSShadowShaderMap	[ R3D_MATVS_FOREST							][ SPT_PROJ	] = VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_ID ;
	VSShadowShaderMap	[ R3D_MATVS_FOREST_NONINSTANCED				][ SPT_PROJ	] = VS_SMDEPTHPATH_FOREST_PROJ_NOANIM_NONINSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_ANIMATEDFOREST					][ SPT_PROJ	] = VS_SMDEPTHPATH_FOREST_PROJ_ID ;
	VSShadowShaderMap	[ R3D_MATVS_ANIMATEDFOREST_NONINSTANCED		][ SPT_PROJ	] = VS_SMDEPTHPATH_FOREST_PROJ_NONINSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_INSTANCING						][ SPT_PROJ	] = VS_SMDEPTHPASS_PROJ_INSTANCED_ID ;
	VSShadowShaderMap	[ R3D_MATVS_APEX_DESTRUCTIBLE				][ SPT_PROJ ] = VS_SMDEPTHPATH_APEX_PROJ_ID;
}

void r3dMaterial::SetVertexShader( r3dMatVSType VSType, int DoDisplace )
{
	r3dRenderer->SetVertexShader( VSShaderMap	[ VSType		]
												[ DoDisplace	] );
}

void r3dMaterial::SetShadowVertexShader ( r3dMatVSType VSType )
{
	r3dRenderer->SetVertexShader( VSShadowShaderMap	[ VSType						]
													[ r3dRenderer->ShadowPassType	] );

}


/*static*/
void r3dMaterial::SetupTransparentStates( int UseSkins, int psID )
{
	r3d_assert(gNumForwardLightsForTransparent < MAX_LIGHTS_FOR_TRANSPARENT);
	if (gNumForwardLightsForTransparent < MAX_LIGHTS_FOR_TRANSPARENT)
		r3dRenderer->SetVertexShader(VS_TRANSPARENT_ID[ UseSkins ][ gNumForwardLightsForTransparent ] );
	else
		r3dRenderer->SetVertexShader();

	if( g_SetupFilmToneConstants  )
		g_SetupFilmToneConstants ( 24 ) ;

	r3dTransparentSetDistort( 1 ) ;

	r3dRenderer->SetPixelShader(psID);
}

void
r3dMaterial::SetupCompoundDiffuse( D3DXVECTOR4 * outVec, int forTransaprency )
{
	* outVec = D3DXVECTOR4(	DiffuseColor.R * RenderedObjectColor.R / ( 255.0f * 255.0f ),
							DiffuseColor.G * RenderedObjectColor.G / ( 255.0f * 255.0f ),
							DiffuseColor.B * RenderedObjectColor.B / ( 255.0f * 255.0f ),
							forTransaprency ? TransparencyMultiplier : lowQSelfIllum ) ;
}

void r3dMaterial::Start( r3dMatVSType VSType, UINT SetupFlags )
{
	if( gPrevMaterial == this )
	{
		D3DXVECTOR4 diffuse ;
		SetupCompoundDiffuse( &diffuse, Flags & R3D_MAT_TRANSPARENT ) ;
		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_MAT_DIFFUSE, (float*)&diffuse, 1 ) ) ;
		return;
	}
	else
	{
		if( r3dMaterial* mat = gPrevMaterial )
		{
			gPrevMaterial = NULL;
			mat->End();
		}
	}

	if( !AlphaFlagSet )
	{
		if( !Texture || Texture->IsLoaded() )
		{
			SetAlphaFlag() ;
			AlphaFlagSet = 1 ;
		}
	}

	if( !( SetupFlags & R3D_MATF_NO_SHADERS )  )
	{
		bool lowQLighting = r_lighting_quality->GetInt() == 1;
		bool highQLighting = r_lighting_quality->GetInt() == 3;

		bool doDispl = m_DoDispl > 0 && highQLighting;

		if( SetupFlags & R3D_MATF_ROAD )
		{
			// not implemented... implement.
			r3d_assert( !m_DoDispl );
		}
		
		if (Flags & R3D_MAT_TRANSPARENT)
		{
			int psid = PS_TRANSPARENT_ID ;

			if( Flags & R3D_MAT_TRANSPARENT_CAMOUFLAGE )
			{
				psid = PS_TRANSPARENT_CAMOUFLAGE_ID ;
			}

			if( Flags & R3D_MAT_TRANSPARENT_CAMO_FP )
			{
				psid = PS_TRANSPARENT_CAMOUFLAGE_FP_ID ;
			}

			SetupTransparentStates( VSType == R3D_MATVS_SKIN, psid );
		}
		else
		{
			SetVertexShader( VSType, doDispl );

			int NeedAlphaCut = SetupFlags & R3D_MATF_FORCE_ALPHATEST | Flags & R3D_MAT_HASALPHA ;
			FillbufferShaderKey key;
			key.flags.alpha_test = NeedAlphaCut > 0 ? 1 : 0;
			key.flags.disp = doDispl ? 1 : 0;

			SetLQ( &key ) ;

			key.flags.camouflage = (Flags & R3D_MAT_CAMOUFLAGE) ? 1 : 0;
			key.flags.roads = (SetupFlags & R3D_MATF_ROAD) ? 1 : 0;

			if(key.flags.camouflage)
			{
				if (r3dMat_gCamouflageDataManager_GetCurrentData)
				{
					const CamoData & cm = r3dMat_gCamouflageDataManager_GetCurrentData();
					if(cm.curTex==0) // don't run camo if curTex is null, otherwise camo will be just pitch black
						key.flags.camouflage = 0;
				}
				else
					key.flags.camouflage = 0;
			}

			if( r_in_minimap_render->GetInt() )
			{
				key.flags.simple = 1 ;
			}

			if( !( SetupFlags & R3D_MATF_NO_PIXEL_SHADER ) )
			{
				SetFillGBufferPixelShader( key ) ;
			}
		}
	}

	StartCommonProperties();

	bool lowQ = r_overall_quality->GetInt() == 1;

	extern int __r3dDisplayMipLevels;

#ifndef FINAL_BUILD
	int AllowMeshTextures = !r_disable_mesh_textures->GetInt();
#else
	int AllowMeshTextures = 1;
#endif

	if (Texture && AllowMeshTextures)
	{
#ifndef FINAL_BUILD
		if (__r3dDisplayMipLevels)
			SetTextureAsMipmapTest(Texture, 0);
		else if (d_show_checker_texture->GetBool())
			SetTextureDensityChecker(0);
		else
#endif
			r3dRenderer->SetTex(Texture);
	}
	else
		r3dRenderer->SetTex( __r3dShadeTexture[2]);

	if ((Flags & R3D_MAT_CAMOUFLAGE) && !(Flags & ( R3D_MAT_TRANSPARENT_CAMOUFLAGE | R3D_MAT_TRANSPARENT_CAMO_FP ) ) && r3dMat_gCamouflageDataManager_GetCurrentData)
	{
		const CamoData & cm = r3dMat_gCamouflageDataManager_GetCurrentData();
		if(cm.curTex) // don't run camo if curTex is null, otherwise camo will be just pitch black
		{
			r3dRenderer->SetTex(cm.prevTex ? cm.prevTex : Texture, 8);
			r3dRenderer->SetTex(cm.curTex, 9);
			r3dRenderer->SetTex(CamouflageMask, 10);

			float currT = r3dGetTime();
			float camoLerpF = (currT - cm.transitionStartTime) / CAMOUFLAGE_LERP_DURATION;
			camoLerpF = r3dTL::Clamp(camoLerpF, 0.0f, 1.0f);

			D3DXVECTOR4 v(camoLerpF, 0, 0, 0);
			r3dRenderer->pd3ddev->SetPixelShaderConstantF(MC_CAMOINTERPOLATOR, &v.x, 1);
		}
		else
		{
			r3dRenderer->SetTex(__r3dShadeTexture[2], 8);
			r3dRenderer->SetTex(__r3dShadeTexture[2], 9);
			r3dRenderer->SetTex(CamouflageMask, 10);
		}
	}

	if (BumpTexture && AllowMeshTextures) r3dRenderer->SetTex(BumpTexture,1);
	else
		r3dRenderer->SetTex( __r3dShadeTexture[1],1);

	if (GlossTexture && AllowMeshTextures)
	{
		if (!lowQ )
			r3dRenderer->SetTex(GlossTexture,2);
	}
	else
		r3dRenderer->SetTex(__r3dShadeTexture[7], 2);

	if(imgEnvPower && AllowMeshTextures)
		r3dRenderer->SetTex(imgEnvPower,3);
	else
		r3dRenderer->SetTex(__r3dShadeTexture[2], 3);

	if(IBLTexture && AllowMeshTextures)
	{
		if (!lowQ)
			r3dRenderer->SetTex(IBLTexture,4);
	}
	else
	{		
		r3dRenderer->SetTex(__r3dShadeTexture[2], 4);
	}

	if (DetailNormalTexture && AllowMeshTextures)
	{
		if (!lowQ)
			r3dRenderer->SetTex(DetailNormalTexture,5);
	}
	else
		r3dRenderer->SetTex( __r3dShadeTexture[8],5);

	if (DensityTexture && AllowMeshTextures) r3dRenderer->SetTex(DensityTexture,6);
	else
		r3dRenderer->SetTex( __r3dShadeTexture[2],6);

	// transparent doesn't need this texture yet it overrides skydome cubemap which is needed
	// by transparent
	if( !( Flags & R3D_MAT_TRANSPARENT ) )
	{
		if (SpecularPowTexture && AllowMeshTextures )
			r3dRenderer->SetTex(SpecularPowTexture,7);
		else
			r3dRenderer->SetTex( __r3dShadeTexture[2],7);
	}

	if( !( SetupFlags & R3D_MATF_NO_PIXEL_SHADER ) )
	{
		D3DXVECTOR4 vConsts[ 4 ] =
		{
			D3DXVECTOR4(SpecularPower, ReflectionPower, SpecularPower1, 1.0f),
			D3DXVECTOR4( 0.f, 0.f, 0.f, lowQSelfIllum),
			D3DXVECTOR4( SpecularColor.R/255.0f, SpecularColor.G/255.0f, SpecularColor.B/255.0f, lowQMetallness),
			D3DXVECTOR4( SelfIllumMultiplier, 0.0f, DetailScale, DetailAmmount )
		} ;

		SetupCompoundDiffuse( &vConsts[ 1 ], Flags & R3D_MAT_TRANSPARENT ) ;

		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF(  MC_MATERIAL_PARAMS, (float *)vConsts,  R3D_ARRAYSIZE( vConsts ) ) );

		TL_STATIC_ASSERT( MC_MAT_DIFFUSE - MC_MATERIAL_PARAMS >= 0 ) ;
		TL_STATIC_ASSERT( MC_MAT_SPECULAR - MC_MATERIAL_PARAMS >= 0 ) ;
		TL_STATIC_ASSERT( MC_MAT_GLOW - MC_MATERIAL_PARAMS >= 0 ) ;

		TL_STATIC_ASSERT( MC_MAT_DIFFUSE - MC_MATERIAL_PARAMS < R3D_ARRAYSIZE( vConsts ) ) ;
		TL_STATIC_ASSERT( MC_MAT_SPECULAR - MC_MATERIAL_PARAMS < R3D_ARRAYSIZE( vConsts ) ) ;
		TL_STATIC_ASSERT( MC_MAT_GLOW - MC_MATERIAL_PARAMS < R3D_ARRAYSIZE( vConsts ) ) ;

		if ( m_DoDispl )
		{
			D3DXVECTOR4 v = D3DXVECTOR4( m_DisplDepthVal, 0, 0, 0 );
			D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_DISPLACE, (float*)&v, 1 ) );
		}
	}

	StartTransparent();
}



void r3dMaterial::End()
{
	if( gPrevMaterial == this )
		return;

	EndCommonProperties();
	EndTransparent();
}

void r3dMaterial::StartShadows( r3dMatVSType VSType )
{
	SetShadowVertexShader( VSType );
	r3dRenderer->SetPixelShader( PS_SMDEPTHPATH_ID );

	StartCommonProperties();

	if (Texture)
		r3dRenderer->SetTex(Texture);
	else
		r3dRenderer->SetTex( __r3dShadeTexture[2]);

}

void r3dMaterial::EndShadows()
{

}

//////////////////////////////////////////////////////////////////////////

void SetParticleDepth();

void r3dMaterial::StartTransparent()
{
	extern r3dCamera gCam;
	if (Flags & R3D_MAT_TRANSPARENT)
	{
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_ZC | R3D_BLEND_PUSH);
		SetParticleDepth();
		r3dRenderer->SetTex(r3dMat_EnvmapProbes_GetClosestTexture(gCam), 8);

		r3dRenderer->SetTex( DistortionTexture ? DistortionTexture : __r3dShadeTexture[10], 2);

		D3DXVECTOR4 vecs[3];
		vecs[0] = D3DXVECTOR4(gCam.x, gCam.y, gCam.z, 1);
		// float3 vCamera					: register( c17 );
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(17, &vecs[0].x, 1);
		if (Sun)
		{
			r3dVector v(-Sun->SunDir);
			v.Normalize();
			vecs[0] = D3DXVECTOR4(v.x, v.y, v.z, 0.f );
			vecs[1] = D3DXVECTOR4(Sun->SunLight.R / 255, Sun->SunLight.G / 255, Sun->SunLight.B / 255, 1);
		}
		else
		{
			vecs[0] = vecs[1] = D3DXVECTOR4(0, 0, 0, 0.f);
		}
		vecs[2] = D3DXVECTOR4(r3dRenderer->AmbientColor.R / 255.0f, r3dRenderer->AmbientColor.G / 255.0f, r3dRenderer->AmbientColor.B / 255.0f, 1);
		if (!(Flags & (R3D_MAT_TRANSPARENT_CAMOUFLAGE | R3D_MAT_TRANSPARENT_CAMO_FP ) ) )
		{
			float extrudeAmount[4] = {0};
			D3D_V(r3dRenderer->pd3ddev->SetVertexShaderConstantF(23, &extrudeAmount[0], 1));
		}
		
		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF(MC_SUNDIR, &vecs[0].x, R3D_ARRAYSIZE(vecs)) );

		TL_STATIC_ASSERT( MC_SUNCOLOR - MC_SUNDIR >= 0 ) ;
		TL_STATIC_ASSERT( MC_AMBIENTCOLOR - MC_SUNDIR >= 0 ) ;

		TL_STATIC_ASSERT( MC_SUNCOLOR - MC_SUNDIR < R3D_ARRAYSIZE(vecs) ) ;
		TL_STATIC_ASSERT( MC_AMBIENTCOLOR - MC_SUNDIR < R3D_ARRAYSIZE(vecs) ) ;

		vecs[0] = D3DXVECTOR4(SpecularPower1, SpecularPower, ReflectionPower, 0.0f);

		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF(MC_MATERIAL_PARAMS, &vecs[0].x, 1) );
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dMaterial::EndTransparent()
{
	if (Flags & R3D_MAT_TRANSPARENT)
	{
		r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		r3dRenderer->SetTex(0, 2);
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dMaterial::StartCommonProperties()
{
	if(Flags & R3D_MAT_DOUBLESIDED)
	{
		r3dRenderer->SetCullMode( D3DCULL_NONE );
	}
}

void r3dMaterial::EndCommonProperties()
{
	if(Flags & R3D_MAT_DOUBLESIDED)
	{
		r3dRenderer->RestoreCullMode();
	}
}

/*static*/
void
r3dMaterial::ResetMaterialFilter()
{
	gPrevMaterial = NULL;
}

static void _GetTexName(const char* param, char* TexName)
{
	const char* p1;

	*TexName = 0;
	for(p1 = param; *p1 == ' ' || *p1 == '\t'; p1++) ;
	r3dscpy(TexName, p1);

	char* p2;
	for(p2 = TexName + strlen(TexName) - 1; *p2 == ' '; *p2-- = 0) ;

	if(stricmp(TexName, "NONE") == 0)
		*TexName = 0;

	return;
}

enum TexType
{
	TT_DIFFUSE,
	TT_METALNESS,
	TT_GLOW,
	TT_NORMAL,
	TT_NORMALDETAIL,
	TT_DENSITYMAP,
	TT_ROUGHNESS,
	TT_CAMOUFLAGEMASK,
	TT_DISTORTION,
	TT_SPECPOW,
	TT_COUNT
};

int MinDimmensionTable[ TT_COUNT ][ 3 ] = 
{
	/*TT_DIFFUSE		*/	{  512, 512, 512 } ,
	/*TT_METALNESS		*/	{  512, 512, 512 } ,
	/*TT_GLOW			*/	{  512, 512, 512 } ,
	/*TT_NORMAL			*/	{  512, 512, 512 } ,
	/*TT_NORMALDETAIL	*/	{  512, 512, 512 } ,
	/*TT_DENSITYMAP		*/	{  512, 512, 512 } ,
	/*TT_ROUGHNESS		*/	{  512, 512, 512 } ,
	/*TT_CAMOUFLAGEMASK	*/	{  512, 512, 512 } ,
	/*TT_DISTORTION		*/	{  512, 512, 512 } ,
	/*TT_SPECPOW		*/	{  512, 512, 512 }
} ;

int DownScaleTable[ TT_COUNT ][ 3 ] =
{
	/*TT_DIFFUSE		*/	{ 2, 1, 1 } ,
	/*TT_METALNESS		*/	{ 2, 2, 1 } ,
	/*TT_GLOW			*/	{ 2, 2, 1 } ,
	/*TT_NORMAL			*/	{ 2, 2, 1 } ,
	/*TT_NORMALDETAIL	*/	{ 2, 2, 1 } ,
	/*TT_DENSITYMAP		*/	{ 2, 2, 1 } ,
	/*TT_ROUGHNESS		*/	{ 2, 2, 1 } ,
	/*TT_CAMOUFLAGEMASK	*/	{ 2, 2, 1 } ,
	/*TT_DISTORTION		*/	{ 2, 2, 1 } ,
	/*TT_SPECPOW		*/	{ 2, 2, 1 } 
} ;

static inline int GetMinScaleDim( TexType ttype )
{
	r3d_assert( ttype >= 0 && ttype < TT_COUNT ) ;

	int ql = R3D_MAX( R3D_MIN( r_texture_quality->GetInt(), 3 ), 1 ) ;

	return MinDimmensionTable[ ttype ][ ql - 1 ] ;
}

static inline int GetDownScale( TexType ttype )
{
	r3d_assert( ttype >= 0 && ttype < TT_COUNT ) ;

	int ql = R3D_MAX( R3D_MIN( r_texture_quality->GetInt(), 3 ), 1 ) ;

	return DownScaleTable[ ttype ][ ql - 1 ] ;
}

r3dTexture* r3dMaterial::MatLoadTexture(const char* fname, const char* slot, int DownScale, int MinDownScaleDim, bool player )
{
	if(*fname == 0)
		return NULL;

	char TempPath[MAX_PATH];
	sprintf(TempPath, "%s\\%s", OriginalDir, fname);

	if(r3d_access(TempPath, 0) != 0) {
		// try .dds
		r3dscpy(TempPath + strlen(TempPath) - 3, "dds");
	}

	r3dTexture* tex = r3dRenderer->LoadTexture( TempPath, D3DFMT_UNKNOWN, false, DownScale, MinDownScaleDim, 0, player ? PlayerTexMem : TexMem );

	if(tex == NULL)
		r3dArtBug( "Material: %s texture %s cannot be loaded\n", slot, TempPath );

	return tex;
}

void SetFlag(int& flags, int flag) { flags |= flag; }
void ResetFlag(int& flags, int flag) { flags &= ~flag; }
void SetFlag(int& flags, int flag, bool invert)
{
	invert ? ResetFlag(flags, flag) : SetFlag(flags, flag);
}

int r3dMaterial::LoadAscii(r3dFile *f, const char* szTexPath)
{
	bool player = Flags & R3D_MAT_PLAYER ? true : false ;

	char 	buf[256];
	char 	*param;

	if(!f->IsValid()) 
		return 0;

	while(!feof(f)) {
		*buf = 0;
		if(fgets(buf, sizeof(buf), f) == NULL)
			return 0;
		if(strstr(buf,"[MaterialBegin]") != NULL)
			break;
	}

	TypeName[ 0 ] = 0;

	if(feof(f))
		return FALSE;

	char	TextureName[128]			= "";
	char	NormalMapName[256]			= "";
	char	EnvMapName[128]				= "";
	char	GlossMapName[128]			= "";
	char	IBLMapName[128]				= "";
	char	DetailNMapName[128]			= "";
	char	DensityMapName[128]			= "";
	char	CamouflageMaskName[128]		= "";
	char	DistortionMapName[128]		= "";
	char	SpecPowMapName[128]			= "";

	TextureName[0]			= 0;
	NormalMapName[0]		= 0;
	EnvMapName[0]			= 0;
	GlossMapName[0]			= 0;
	IBLMapName[0]			= 0;
	DetailNMapName[0]		= 0;
	DensityMapName[0]		= 0;
	CamouflageMaskName[0]	= 0;
	DistortionMapName[0]	= 0;
	SpecPowMapName[0]		= 0;

	sprintf(OriginalDir,  "%s", szTexPath);

	while(!feof(f))
	{
		*buf = 0;
		if(fgets(buf, sizeof(buf), f) == NULL)
			return FALSE;
		buf[strlen(buf)-1] = 0;

		if(strstr(buf,"[MaterialEnd]"))
			break;

		if((param = strchr(buf, '=')) == NULL)
			continue;
		param++;

		if(!strnicmp(buf,"Name",4)) {
			sscanf(param,"%s", Name);
			//r3dOutToLog("Material: Starting %s\n", Name);
			continue;
		}

		if(!strnicmp(buf,"Flags",5)) {
			char FlagsS[128];
			sscanf(param,"%s",FlagsS);
			//r3dOutToLog("Material: Material flags are %s\n", FlagsS);

			continue;
		}

		if(!strnicmp(buf,"SpecularPower",13)) {
			sscanf(param,"%f", &SpecularPower);
			continue;
		}

		if(!strnicmp(buf,"DetailScale",11)) {
			sscanf(param,"%f", &DetailScale);
			continue;
		}

		if(!strnicmp(buf,"DetailAmmount",13)) {
			sscanf(param,"%f", &DetailAmmount);
			continue;
		}

		if(!strnicmp(buf,"Specular1Power",14)) {
			sscanf(param,"%f", &SpecularPower1);
			continue;
		}

		if(!strnicmp(buf,"displace",8)) 
		{
			int n;
			sscanf(param,"%d", &n);
			m_DoDispl = n != 0;
			continue;
		}

		if(!strnicmp(buf,"displ_val",9)) 
		{
			sscanf(param,"%f", &m_DisplDepthVal);
			continue;
		}

		if(!strnicmp(buf,"ReflectionPower",15)) {
			sscanf(param, "%f", &ReflectionPower);
			continue;
		}

		if (!strnicmp(buf, "AlphaTransparent", 16))
		{
			int transparencyFlag = 0;
			sscanf(param, "%d", &transparencyFlag);
			SetFlag(Flags, R3D_MAT_TRANSPARENT, transparencyFlag == 0);
			continue;
		}

		if (!strnicmp(buf, "Camouflage", 10))
		{
			int camoFlag = 0;
			sscanf(param, "%d", &camoFlag);
			SetFlag(Flags, R3D_MAT_CAMOUFLAGE, camoFlag == 0);
			continue;
		}

		if(!strnicmp(buf, "DoubleSided", sizeof "DoubleSided" - 1))
		{
			int DoubleSided;
			sscanf(param, "%d", &DoubleSided);
			SetFlag(Flags, R3D_MAT_DOUBLESIDED, DoubleSided == 0);
			continue;
		}

		if(!strnicmp(buf,"Color24",7)) {
			int	iR, iG, iB;
			sscanf(param,"%d %d %d", &iR, &iG, &iB);
			DiffuseColor.R = iR;
			DiffuseColor.G = iG;
			DiffuseColor.B = iB;
			continue;
		}

		if( !strnicmp(buf, "Type", 4 ) )
		{
			char Buf[ 256 ];
			sscanf( param, "%s", Buf );
			r3dscpy( TypeName, Buf );
			continue;
		}

		if( !strnicmp(buf, "lowQSelfIllum", 13 ) )
		{
			sscanf(param, "%f", &lowQSelfIllum);
			continue;
		}

		if( !strnicmp(buf, "lowQMetallness", 14 ) )
		{
			sscanf(param, "%f", &lowQMetallness);
			continue;
		}

		if( !strnicmp( buf, "TransparentShadows", sizeof "TransparentShadows" - 1 ) || 
			!strnicmp( buf, "ForceTransparent", sizeof "ForceTransparent" - 1 )
			)
		{
			int ForceTransparent;
			sscanf( param, "%d", &ForceTransparent );
			SetFlag(Flags, R3D_MAT_FORCEHASALPHA, ForceTransparent == 0);
			SetAlphaFlag();
			continue;
		}

		if(!strnicmp(buf,"Texture",        7)) { _GetTexName(param, TextureName);	continue; }

		if(!strnicmp(buf,"NormalMap",      9)) { _GetTexName(param, NormalMapName);  continue; }

		if(!strnicmp(buf,"EnvMap",         6)) { _GetTexName(param, EnvMapName);   	continue; }

		if(!strnicmp(buf,"SpecularMap",    8)) { _GetTexName(param, GlossMapName);  continue; }

		if(!strnicmp(buf,"GlowMap",        7)) { _GetTexName(param, IBLMapName);  continue; }

		if(!strnicmp(buf,"DetailNMap",    10)) { _GetTexName(param, DetailNMapName);  continue; }

		if(!strnicmp(buf,"DensityMap",    10)) { _GetTexName(param, DensityMapName);  continue; }

		if(!strnicmp(buf,"CamoMask",       8)) { _GetTexName(param, CamouflageMaskName);  continue; }

		if(!strnicmp(buf,"ImagesDir",      9)) { _GetTexName(param, OriginalDir);  continue; }

		if(!strnicmp(buf,"DistortionMap", 13)) { _GetTexName(param, DistortionMapName);  continue; }

		if(!strnicmp(buf,"SpecPowMap",    10)) { _GetTexName(param, SpecPowMapName);  continue; }

		if(!strnicmp(buf,"SelfIllumMultiplier", 19)) { sscanf(param, "%f", &SelfIllumMultiplier);  continue; }		

	} // while not end of material description

	// load textures
	Texture             = MatLoadTexture(TextureName,           "Diffuse"           , GetDownScale( TT_DIFFUSE          ), GetMinScaleDim( TT_DIFFUSE       ), player );
	GlossTexture        = MatLoadTexture(GlossMapName,          "Metalness"         , GetDownScale( TT_METALNESS        ), GetMinScaleDim( TT_METALNESS         ), player );
	IBLTexture          = MatLoadTexture(IBLMapName,            "GlowMap"           , GetDownScale( TT_GLOW             ), GetMinScaleDim( TT_GLOW                  ), player );
	BumpTexture         = MatLoadTexture(NormalMapName,         "Normalmap"         , GetDownScale( TT_NORMAL           ), GetMinScaleDim( TT_NORMAL                ), player );
	DetailNormalTexture = MatLoadTexture(DetailNMapName,        "DetailN"           , GetDownScale( TT_NORMALDETAIL     ), GetMinScaleDim( TT_NORMALDETAIL          ), player );
	DensityTexture      = MatLoadTexture(DensityMapName,        "DensityMap"        , GetDownScale( TT_DENSITYMAP       ), GetMinScaleDim( TT_DENSITYMAP            ), player );
	imgEnvPower         = MatLoadTexture(EnvMapName,            "Roughness"         , GetDownScale( TT_ROUGHNESS        ), GetMinScaleDim( TT_ROUGHNESS             ), player );
	CamouflageMask      = MatLoadTexture(CamouflageMaskName,    "CamouflageMask"    , GetDownScale( TT_CAMOUFLAGEMASK   ), GetMinScaleDim( TT_CAMOUFLAGEMASK        ), player );
	DistortionTexture   = MatLoadTexture(DistortionMapName,     "DistortionMap"     , GetDownScale( TT_DISTORTION       ), GetMinScaleDim( TT_DISTORTION            ), player );
	SpecularPowTexture  = MatLoadTexture(SpecPowMapName,        "DistortionMap"     , GetDownScale( TT_SPECPOW          ), GetMinScaleDim( TT_SPECPOW               ), player );	

	// NOTE : because textures may be delay loaded, this function is called once more in r3dMaterial::Start
	SetAlphaFlag();

	//r3dOutToLog("Material: Material %s [%s] loaded, ID: %d\n", Name, DepotName, ID);

	return TRUE;
}

static const char* mlGetFileNameFromTexture(const r3dTexture* tex)
{
	const char* f = tex->getFileLoc().FileName;
	const char* p = f;
	if((p=strrchr(f, '\\')) != NULL) return p + 1;
	if((p=strrchr(f, '/')) != NULL) return p + 1;
	return f;
}

static void mlSaveTextureName(FILE* f, const char* texId, r3dTexture* tex)
{
	if(tex && (tex->IsLoaded()))
		fprintf(f, "%s= %s\n", texId, mlGetFileNameFromTexture(tex));
}

int r3dMaterial::SaveAscii(FILE* f)
{
	r3d_assert(f);

	fprintf(f, "[MaterialBegin]\n");
	fprintf(f, "Name= %s\n", Name);
	fprintf(f, "Flags= _unused_\n");
	fprintf(f, "SpecularPower= %.2f\n", SpecularPower);
	fprintf(f, "Specular1Power= %.2f\n", SpecularPower1);
	fprintf(f, "ReflectionPower= %.2f\n", ReflectionPower);
	fprintf(f, "DoubleSided= %d\n", (Flags & R3D_MAT_DOUBLESIDED) ? 1 : 0);
	fprintf(f, "Color24= %d %d %d\n", DiffuseColor.R, DiffuseColor.G, DiffuseColor.B);
	fprintf(f, "DetailScale= %.2f\n", DetailScale);
	fprintf(f, "DetailAmmount= %.2f\n", DetailAmmount);
	fprintf(f, "displace= %d\n", m_DoDispl ? 1 : 0 );
	fprintf(f, "displ_val= %.3f\n", m_DisplDepthVal );
	fprintf(f, "ForceTransparent= %d\n", (Flags & R3D_MAT_FORCEHASALPHA) ? 1 : 0 );
	fprintf(f, "AlphaTransparent= %d\n", (Flags & R3D_MAT_TRANSPARENT) ? 1 : 0 );
	fprintf(f, "Camouflage= %d\n", (Flags & R3D_MAT_CAMOUFLAGE) ? 1 : 0 );
	fprintf(f, "lowQSelfIllum= %.2f\n", lowQSelfIllum);
	fprintf(f, "lowQMetallness= %.2f\n", lowQMetallness);
	fprintf(f, "SelfIllumMultiplier=%.4f\n", SelfIllumMultiplier);

	if( TypeName[0] )
		fprintf( f, "Type= %s\n", TypeName );

	if(!DepotName[0]) 
		fprintf(f, "ImagesDir= %s\n", OriginalDir);

	mlSaveTextureName(f, "Texture",			Texture);
	mlSaveTextureName(f, "NormalMap",		BumpTexture);
	mlSaveTextureName(f, "EnvMap",			imgEnvPower);
	mlSaveTextureName(f, "SpecularMap",		GlossTexture);
	mlSaveTextureName(f, "GlowMap",			IBLTexture);
	mlSaveTextureName(f, "DetailNMap",		DetailNormalTexture);
	mlSaveTextureName(f, "DensityMap",		DensityTexture);
	mlSaveTextureName(f, "CamoMask",		CamouflageMask);
	mlSaveTextureName(f, "DistortionMap",	DistortionTexture);
	mlSaveTextureName(f, "SpecPowMap",		SpecularPowTexture);

	fprintf(f, "[MaterialEnd]\n");
	fprintf(f, "\n");

	return TRUE;
}

//------------------------------------------------------------------------

static void mlCopyTexture(const r3dTexture* tex, const char* outDir)
{
	if(!tex)
		return;

	const char* in = tex->getFileLoc().FileName;
	char out[MAX_PATH];
	sprintf(out, "%s\\%s", outDir, mlGetFileNameFromTexture(tex));
	r3dOutToLog("copying %s->%s\n", in, out);

	if(::CopyFile(in, out, FALSE) == 0) {
		r3dOutToLog("!!! CopyFile failed, err:%d\n", GetLastError());
	}
}

int r3dMaterial::CopyTextures(const char* outDir)
{
	mlCopyTexture(Texture,      outDir);
	mlCopyTexture(GlossTexture, outDir);
	mlCopyTexture(BumpTexture,  outDir);
	mlCopyTexture(imgEnvPower,  outDir);
	mlCopyTexture(IBLTexture,   outDir);
	mlCopyTexture(DetailNormalTexture, outDir);
	mlCopyTexture(DensityTexture, outDir);
	mlCopyTexture(CamouflageMask, outDir);
	mlCopyTexture(DistortionTexture, outDir);
	mlCopyTexture(SpecularPowTexture, outDir);

	return TRUE;
}

static void mlReloadTexture( r3dTexture*& tex, TexType tt )
{
	if( !tex )
		return;

	char FileName[ 1024 ];
	r3dscpy( FileName, tex->getFileLoc().FileName );
	FileName[ 1023 ] = 0;

	tex->Unload();
	tex->Load( FileName, D3DFMT_FROM_FILE, GetDownScale( tt ) );
}

void r3dMaterial::ReloadTextures()
{
	mlReloadTexture( Texture				, TT_DIFFUSE		);
	mlReloadTexture( GlossTexture			, TT_GLOW			);
	mlReloadTexture( BumpTexture			, TT_NORMAL			);
	mlReloadTexture( imgEnvPower			, TT_ROUGHNESS		);
	mlReloadTexture( IBLTexture				, TT_GLOW			);
	mlReloadTexture( DetailNormalTexture	, TT_NORMALDETAIL	);
	mlReloadTexture( DensityTexture			, TT_DENSITYMAP		);
	mlReloadTexture( CamouflageMask			, TT_CAMOUFLAGEMASK );
	mlReloadTexture( DistortionTexture		, TT_DISTORTION		);
	mlReloadTexture( SpecularPowTexture		, TT_SPECPOW		);

	SetAlphaFlag();
}

void r3dMaterial::SetAlphaFlag()
{
	Flags &= ~R3D_MAT_HASALPHA ;

	if( Flags & R3D_MAT_FORCEHASALPHA )
	{
		Flags |= R3D_MAT_HASALPHA ;
	}
	else
	{
		if( Texture && Texture->IsValid())
		{
			D3DFORMAT format = Texture->GetD3DFormat() ;

			if( format != D3DFMT_DXT1 && format != D3DFMT_R8G8B8 )
			{
				Flags |= R3D_MAT_HASALPHA ;
			}
		}
	}
}

/*static*/
r3dColor r3dMaterial::RenderedObjectColor = r3dColor::white ;


int	     NumManagedMaterialsInLibrary = 0;
r3dMaterial* ManagedMatlibPointers[4096];

int& r3dMaterialLibrary::GetCurrent(r3dMaterial**& materialsArray)
{
	materialsArray = ManagedMatlibPointers;
	return NumManagedMaterialsInLibrary;
}

void ResetMatFrameScores()
{
	memset( &gMatFrameScores[0], 0, sizeof( gMatFrameScores[0] ) * MAX_MAT_FRAME_SCORES ) ;
}

void r3dMaterialLibrary::Reset()
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	ResetMatFrameScores();

	r3d_assert(NumManagedMaterialsInLibrary == 0);
	NumManagedMaterialsInLibrary = 0;

	memset (&ManagedMatlibPointers[0], 0, sizeof(ManagedMatlibPointers) );

	// Add default material everytime
 	ManagedMatlibPointers[0] = new r3dMaterial;
 	r3dscpy(ManagedMatlibPointers[0]->Name, "-DEFAULT-");
 	NumManagedMaterialsInLibrary = 1;
}

int r3dMaterialLibrary::UnloadManaged()
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	for(int i=0; i<NumManagedMaterialsInLibrary; i++)
	{
		SAFE_DELETE(ManagedMatlibPointers[i]);
	}
	NumManagedMaterialsInLibrary = 0;

	return 1;
}

void	r3dMaterialLibrary::Destroy()
{
	UnloadManaged();
}


int r3dMaterialLibrary::LoadLibrary(const char* szFileName, const char* szTexPath)
{
	r3dFile		*f;
	long		FileStart;
	char		buf[256];
	int		NumMats;

	r3dMaterial** MatlibPointers;
	int NumMaterialsInLibrary = GetCurrent(MatlibPointers);

	r3d_assert(NumMaterialsInLibrary > 0 && "r3dMaterialLibrary::Reset wasn't called");

	// create default library.
	if((f=r3d_open(szFileName, "rt")) == NULL) 
	{
		r3dArtBug("MatLib: can't open %s\n", szFileName);
		return 0;
	}

	// detect number of materials
	NumMats   = 0;
	FileStart = ftell(f);
	while(!feof(f))
	{
		if(fgets(buf, sizeof(buf), f) == NULL)
			break;
		if(strstr(buf, "[MaterialBegin]") != NULL)
			NumMats++;
	}
	fseek(f, FileStart, SEEK_SET);
	r3dOutToLog("MatLib: Loading %s, %d materials\n", szFileName, NumMats);

	for (int i=0;i<NumMats;i++)
	{
		MatlibPointers[NumMaterialsInLibrary] = new r3dMaterial;
		if (MatlibPointers[NumMaterialsInLibrary]->LoadAscii(f, szTexPath))
			NumMaterialsInLibrary ++;
		else
			SAFE_DELETE(MatlibPointers[NumMaterialsInLibrary]);
	}

	fclose(f);

	return 1;
}


r3dMaterial* r3dMaterialLibrary::FindMaterial(const char* MatName)
{
//	r3d_assert(NumMaterialsInLibrary > 0 && "r3dMaterialLibrary::Reset wasn't called");
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	if(_r3d_MatLib_SkipAllMaterials)
		return ManagedMatlibPointers[0];

	r3dMaterial* m = HasMaterial(MatName);
	
	return m ? m : ManagedMatlibPointers[0];
}

r3dMaterial* r3dMaterialLibrary::HasMaterial(const char* MatName)
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	int NumMaterialsInLibrary;
	r3dMaterial** MatlibPointers;
	NumMaterialsInLibrary = GetCurrent(MatlibPointers);
	for (int i=0;i<NumMaterialsInLibrary;i++)
		if(!stricmp(MatName, MatlibPointers[i]->Name))
			return MatlibPointers[i];

	return 0;
}

/*static*/
r3dMaterial*
r3dMaterialLibrary::GetDefaultMaterial()
{
	r3d_assert( NumManagedMaterialsInLibrary > 0 ) ;
	// 0 is supposed to always be default material.
	return ManagedMatlibPointers[ 0 ] ;
}

int r3dMaterialLibrary::SaveLibrary(const char* szFileName, const char* szTexPath, int bCompact)
{
	r3dOutToLog("r3dMaterialLibrary::SaveLibrary to %s, texDir:%s\n", szFileName, szTexPath);
	CLOG_INDENT;

	FILE* f = fopen(szFileName, "wt");
	if(!f) {
		r3dError("can't open %s for writing\n", szFileName);
		return 0;
	}

	int NumMaterialsInLibrary;
	r3dMaterial** MatlibPointers;

	NumMaterialsInLibrary = GetCurrent(MatlibPointers);
	for(int i=0;i<NumMaterialsInLibrary;i++) {
		MatlibPointers[i]->SaveAscii(f);
	}

	fclose(f);

	// create texture directory
	CreateDirectory(szTexPath, NULL);

	// copy textures
	r3dOutToLog("Copying textures\n"); CLOG_INDENT;

	NumMaterialsInLibrary = GetCurrent(MatlibPointers);
	for(int i=0;i<NumMaterialsInLibrary;i++) 
		MatlibPointers[i]->CopyTextures(szTexPath);

	return 1;
}

/*static */ void r3dMaterialLibrary::ReloadMaterialTextures()
{
	int NumMaterialsInLibrary;
	r3dMaterial** MatlibPointers;

	NumMaterialsInLibrary = GetCurrent(MatlibPointers);
	for(int i=0;i<NumMaterialsInLibrary;i++) 
		MatlibPointers[i]->ReloadTextures();
}

//
// handling depot per-file dynamic materials
//
bool	r3dMaterialLibrary::IsDynamic = false;

void MakeMaterialFileName( char* MatFile, const char* MatName, const char* DepotName )
{
	sprintf( MatFile, "%s\\%s\\Materials\\%s.mat", DEPOT_TAG, DepotName, MatName );
}

r3dMaterial* r3dMaterialLibrary::RequestMaterialByMesh(const char* MatName, const char* szMeshNameIn, bool playerMaterial )
{
//	r3d_assert(NumMaterialsInLibrary > 0 && "r3dMaterialLibrary::Reset wasn't called");
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	if(_r3d_MatLib_SkipAllMaterials)
		return ManagedMatlibPointers[0];

	char MeshName[ MAX_PATH ];
	FixFileName( szMeshNameIn, MeshName );

	// see if we already have it
	r3dMaterial* m = HasMaterial(MatName);
	if(m)
	{
		m->RefCount ++ ;
		return m;
	}

	// determine depot
	if(strnicmp(MeshName, DEPOT_TAG, DEPOT_TLEN) != 0) {
		r3dArtBug("MatLib: can't determine depot from file %s\n", MeshName);
		return ManagedMatlibPointers[0];
	}
	char depot[MAX_PATH];
	char* p;
	r3dscpy(depot, MeshName + DEPOT_TLEN + 1);
	if((p = strrchr(depot, '/')) != 0) *p = 0;
	if((p = strrchr(depot, '\\')) != 0) *p = 0;

	return RequestMaterialByName(MatName, depot, playerMaterial);

}

//////////////////////////////////////////////////////////////////////////

r3dMaterial* r3dMaterialLibrary::RequestMaterialByName(const char* MatName, const char* depot, bool playerMaterial )
{
	if(_r3d_MatLib_SkipAllMaterials)
		return ManagedMatlibPointers[0];

	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	// make material file name
	char matname[MAX_PATH];

	// see if we already have it
	r3dMaterial* m = HasMaterial(MatName);
	if(m)
	{
		m->RefCount ++ ;
		return m;
	}

	MakeMaterialFileName( matname, MatName, depot ) ;

	if(r3d_access(matname, 0) != 0) {
		r3dArtBug("MatLib: there is no material %s\n", matname);
		return ManagedMatlibPointers[0];
	}

	// texture dir
	char texpath[MAX_PATH];
	sprintf(texpath, "%s/%s/Textures", DEPOT_TAG, depot);

	// load it
	r3dFile* f = r3d_open(matname, "rt");
	r3d_assert(f);

	r3dMaterial* mat = new r3dMaterial;

	if( playerMaterial )
	{
		mat->Flags |= R3D_MAT_PLAYER ;
	}

	r3dscpy(mat->DepotName, depot);
	if(!mat->LoadAscii(f, texpath)) 
	{
		r3dArtBug("MatLib: failed to load material %s\n", matname);
		delete mat;
		fclose(f);

		return ManagedMatlibPointers[0];
	}
	fclose(f);

	r3dMaterial** MatlibPointers;
	int& NumMaterialsInLibrary = GetCurrent(MatlibPointers);


	// add to matlib
	MatlibPointers[NumMaterialsInLibrary] = mat;
	NumMaterialsInLibrary ++;

	mat->RefCount = 1 ;

	return mat;
}

//////////////////////////////////////////////////////////////////////////


void r3dMaterialLibrary::UpdateDepotMaterials()
{
	r3dMaterial** MatlibPointers;
	int NumMaterialsInLibrary = GetCurrent(MatlibPointers);

	r3d_assert(NumMaterialsInLibrary > 0 && "r3dMaterialLibrary::Reset wasn't called");

	for(int i=1; i<NumMaterialsInLibrary; i++)
	{
		r3dMaterial* mat = MatlibPointers[i];
		if(mat->DepotName[0] == 0)
			continue;

		if(!mat->WasEdited)
			continue;

		char matname[MAX_PATH];
		sprintf(matname, "%s\\%s\\Materials\\%s.mat", DEPOT_TAG, mat->DepotName, mat->Name);
		FILE* f = fopen(matname, "wt");
		if(f == NULL) {
			r3dOutToLog("MatLib: can't open %s for writing\n", matname);
			continue;
		}
		mat->SaveAscii(f);
		fclose(f);
	}

	return;
}

//------------------------------------------------------------------------
/*static */

void
r3dMaterialLibrary::UnloadManagedMaterial( r3dMaterial* mat )
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	int foundIdx = -1 ;

	for( int i = 0, e = NumManagedMaterialsInLibrary ; i < e ; i ++ )
	{
		if( ManagedMatlibPointers[ i ] == mat )
		{
			foundIdx = i ;
			break ;
		}
	}

	if( foundIdx < 0 )
	{
		r3dOutToLog( "r3dMaterialLibrary::UnloadMaterial: material %s(%p) is not in the library!\n", mat->Name, mat ) ;
		return ;
	}

	mat->RefCount -- ;

	if( mat->RefCount <= 0 )
	{
		if( mat->RefCount < 0 )
		{
			r3dOutToLog( "r3dMaterialLibrary::UnloadMaterial: negative reference count!\n" ) ;
		}

		SAFE_DELETE( mat ) ;

		for( int i = foundIdx ; i < NumManagedMaterialsInLibrary ; i ++ )
		{
			ManagedMatlibPointers[ i ] = ManagedMatlibPointers[ i + 1 ] ;
		}

		NumManagedMaterialsInLibrary -- ;

		ManagedMatlibPointers[ NumManagedMaterialsInLibrary ] = NULL ;
	}

}

//------------------------------------------------------------------------

void r3dInitMaterials()
{
	FillVSShaderMaps();
}

void r3dCloseMaterials()
{

}