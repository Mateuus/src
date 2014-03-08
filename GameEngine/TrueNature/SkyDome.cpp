#include "r3dPCH.h"
#include "r3d.h"

#include "skydome.h"
#include "sun.h"
#include "cubemap.h"

CloudPlane* g_pCloudPlane=0;



#include "XPSObject.h"

#include "skydome.h"
#include "sun.h"

#include "r3dDeviceQueue.h"

extern r3dSun		*Sun;


// A structure for our custom vertex type


LPDIRECT3DVERTEXDECLARATION9 R3D_SKY_VERTEX::pDecl = 0;
D3DVERTEXELEMENT9 R3D_SKY_VERTEX::VBDecl[] = 
{
	{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

LPDIRECT3DVERTEXDECLARATION9 R3D_SKY_VERTEX::getDecl()
{
	if(pDecl == 0)
	{
		r3dDeviceTunnel::CreateVertexDeclaration( VBDecl, &pDecl ) ;
		r3d_assert(pDecl);
	}
	return pDecl;
}

r3dXPSObject<R3D_SKY_VERTEX>*	SkyDomeXPS;
r3dXPSObject<R3D_SKY_VERTEX>*	SkyDomeDown;

CubemapTransform::Matrices cubeMtx;

bool g_bSkyDomeNeedFullUpdate = false;
bool updateAfterRestore = false;

StaticSkySettings::StaticSkySettings()
: bEnabled( 0 )
, bPlanarMapping( 0 )
, tex( 0 )
, mesh( 0 )
, texScaleX( 0.0001f )
, texScaleY( 0.0001f )
, texOffsetX( 0.5f )
, texOffsetY( 0.5f )
{

}

r3dSkyDome::r3dSkyDome( const r3dIntegrityGuardian& ig )
: r3dIResource( ig )
, cloudTex(0), cloudCoverTex(0)
, m_fFogHeight		( 200 )
, m_fFogOffset		( 0 )
, m_volumeFogColor	( 255, 255, 255 )
, m_fCloadAnim		( 0 )
, m_fWindFactor		( 1 )
, StatidSkyPS_ID	( -1 )
, StatidSkyVS_ID	( -1 )
, StatidSkyTexgVS_ID( -1 )
{
	bLoaded		= 0;
	cubemap		= 0;
}

r3dSkyDome ::~r3dSkyDome()
{
	Unload();
}

void r3dSkyDome::SetParams(float fFogHeight, float fFogOffset, r3dColor volumeFogColor, float fWindFactor)
{
	m_fFogHeight = fFogHeight;
	m_fFogOffset = fFogOffset;
	m_volumeFogColor = volumeFogColor;
	m_fWindFactor = fWindFactor;
}

void r3dSkyDome::SetStaticSkyParams( const StaticSkySettings& sts )
{
	SSTSettings = sts ;
}

void r3dSkyDome::InitCloudPlane()
{
	if( g_pCloudPlane->IsActive() )
	{		
		g_pCloudPlane->Init() ;
		RefreshTexture() ;
	}
}

BOOL r3dSkyDome::Load(const char* DirName)
{
	bLoaded  = 1;

	SkyDomeXPS = new r3dXPSObject<R3D_SKY_VERTEX>;
	SkyDomeDown = new r3dXPSObject<R3D_SKY_VERTEX>;

	SkyDomeXPS->InitDome(25160.0, 3000.0, 64, 64);
	SkyDomeDown->InitDome(25160.0, -3000.0, 64, 64);

	cubemap = r3dScreenBuffer::CreateClass( "Skydome cubemap", 128, 128, D3DFMT_R5G6B5, r3dScreenBuffer::Z_NO_Z, 1, 0 );
	cubemap->SetDebugD3DComment("r3dSkyDome");

	tempRt0 = r3dScreenBuffer::CreateClass( "Temp Cubemap RT0", 128, 128, D3DFMT_R5G6B5 );
	tempRt0->SetDebugD3DComment("r3dSkyDome helper RT0");
	tempRt1 = r3dScreenBuffer::CreateClass( "Temp Cubemap RT1", 64, 64, D3DFMT_R5G6B5 );
	tempRt1->SetDebugD3DComment("r3dSkyDome helper RT1");

	updateAfterRestore = true;	//redraw cubemap

	CubemapTransform::getMatrices(D3DXVECTOR3(0,0,0), cubeMtx);

	SAFE_DELETE( g_pCloudPlane ) ;

	g_pCloudPlane = new CloudPlane() ;

	g_pCloudPlane->Load( DirName ) ;

	InitCloudPlane() ;

	hoffman.create();

	StatidSkyPS_ID			= r3dRenderer->GetPixelShaderIdx( "PS_SKYSTATIC" );
	StatidSky_Norm_PS_ID	= r3dRenderer->GetPixelShaderIdx( "PS_SKYSTATIC_W_NORMALS" );

	StatidSkyVS_ID = r3dRenderer->GetVertexShaderIdx( "VS_SKYSTATIC" );
	StatidSkyTexgVS_ID = r3dRenderer->GetVertexShaderIdx( "VS_SKYSTATIC_TEXG" );

	r3d_assert( StatidSkyPS_ID >= 0 &&
				StatidSky_Norm_PS_ID >= 0 &&
				StatidSkyVS_ID >= 0 &&
				StatidSkyTexgVS_ID >= 0		);

	return TRUE;
}

BOOL r3dSkyDome::Unload()
{
	if(!bLoaded)
		return 0;

	SAFE_DELETE(cubemap);
	SAFE_DELETE(tempRt0);
	SAFE_DELETE(tempRt1);

	r3dRenderer->DeleteTexture(cloudTex);
	r3dRenderer->DeleteTexture(cloudCoverTex);

	cloudTex = NULL ;
	cloudCoverTex = NULL ;

	SAFE_DELETE(SkyDomeXPS);
	SAFE_DELETE(SkyDomeDown);

	SAFE_DELETE(g_pCloudPlane);

	bLoaded = 0;
	return TRUE;
}

void r3dSkyDome::RefreshTexture()
{
	if( g_pCloudPlane && g_pCloudPlane->IsActive() )
	{
		if(cloudTex) r3dRenderer->DeleteTexture(cloudTex);
		if(cloudCoverTex) r3dRenderer->DeleteTexture(cloudCoverTex);

		if(g_pCloudPlane->sceneParam.textureFileName[0] != 0) cloudTex = r3dRenderer->LoadTexture(g_pCloudPlane->sceneParam.textureFileName);
		if(g_pCloudPlane->sceneParam.textureCoverFileName[0] != 0) cloudCoverTex = r3dRenderer->LoadTexture(g_pCloudPlane->sceneParam.textureCoverFileName);
	}
}

static R3D_FORCEINLINE void SetTransformMatrix( const D3DXMATRIX& mtx )
{
	D3DXMATRIX ShaderMat = mtx ;

	D3DXMatrixTranspose( &ShaderMat, &ShaderMat );
	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );
}

void
r3dSkyDome::DrawDome( const r3dCamera& Cam, const D3DXMATRIX& viewProj, float mieScale, bool normals, float amplify, bool hemisphere )
{
	if( !SSTSettings.bEnabled && !g_pCloudPlane )
	{
		InitCloudPlane() ;
	}

	r3dVector V = -Sun->SunLight.Direction;
	V.Normalize();

	if( SSTSettings.bEnabled )
	{
		if( normals )
			r3dRenderer->SetPixelShader( StatidSky_Norm_PS_ID ) ;
		else
			r3dRenderer->SetPixelShader( StatidSkyPS_ID ) ;

		r3dRenderer->SetTex( SSTSettings.tex ) ;
	}
	else
	{
		r3dRenderer->SetCullMode( D3DCULL_NONE );
	}

	D3DXMATRIX ShaderMat;

	bool customMesh = false ;

	if( SSTSettings.bEnabled )
	{
		if( SSTSettings.mesh )
		{
			r3dRenderer->SetVertexShader( StatidSkyVS_ID ) ;
			customMesh = true ;
		}
		else
		{
			if( SSTSettings.bPlanarMapping )
			{
				float vConst[ 4 ] = {  SSTSettings.texScaleX, SSTSettings.texScaleY, SSTSettings.texOffsetX, SSTSettings.texOffsetY };
				D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 8, vConst, 1 ) );

				r3dRenderer->SetVertexShader( StatidSkyTexgVS_ID ) ;
			}
			else
			{
				r3dRenderer->SetVertexShader( StatidSkyVS_ID ) ;
			}
		}
	}

	r3dPoint2D tCloudsDim (256.0f, 256.0f);
	hoffman.renderSkyLight(Cam, V, r3dPoint3D(m_fFogHeight, m_fFogOffset, m_fCloadAnim), m_volumeFogColor, tCloudsDim, normals, mieScale, amplify, !SSTSettings.bEnabled );

	if( customMesh )
	{
		D3DXMATRIX scale ;

		// this mesh is  short4 packed so we extend it to 30000.f using these numbers
		D3DXMatrixScaling( &scale, SKY_DOME_RADIUS, SKY_DOME_RADIUS, SKY_DOME_RADIUS ) ;

		r3dRenderer->SetCullMode( D3DCULL_CCW ) ;
		SetTransformMatrix( scale * viewProj ) ;
		SSTSettings.mesh->DrawMeshSimple( 0 ) ;

		if( !hemisphere )
		{
			D3DXMatrixScaling( &scale, SKY_DOME_RADIUS, -SKY_DOME_RADIUS, SKY_DOME_RADIUS ) ;

			r3dRenderer->SetCullMode( D3DCULL_CW ) ;

			SetTransformMatrix( scale * viewProj ) ;
			SSTSettings.mesh->DrawMeshSimple( 0 ) ;

			r3dRenderer->RestoreCullMode();
		}
	}
	else
	{
		SetTransformMatrix( viewProj ) ;

		r3dRenderer->SetCullMode( D3DCULL_CCW ) ;
		SkyDomeXPS->Draw();

		if( !hemisphere )
		{
			r3dRenderer->SetCullMode( D3DCULL_CW ) ;
			SkyDomeDown->Draw();
		}
	}

	r3dRenderer->RestoreCullMode() ;
}

void r3dSkyDome::Update(const r3dCamera &Cam)
{
	struct PerfEvent
	{
		PerfEvent()
		{
			D3DPERF_BeginEvent( 0, L"r3dSkyDome::Update" ) ;
		}

		~PerfEvent()
		{
			D3DPERF_EndEvent() ;
		}

	} perfEvent; (void)perfEvent ;

	r3dVector V = -Sun->SunLight.Direction;
	V.Normalize();

	static r3dVector OldV = -V;

	if( OldV != V || g_bSkyDomeNeedFullUpdate || updateAfterRestore )
	{
		DrawCubemap( Cam );
		OldV = V;
		updateAfterRestore = false;
	}	

	if( g_pCloudPlane )
	{
		SSceneParamter& cloudsParams = g_pCloudPlane->sceneParam;
		if( cloudsParams.isActive != 0 )
		{
			V = -V;

			// need specific far
			D3DXMATRIX proj ;

			r3dCamera betterCam = Cam ;

			const float DESIRED_FAR = 30000.f ;

			betterCam.FarClip = R3D_MAX( betterCam.FarClip, DESIRED_FAR ) ;

			r3dRenderer->BuildPerspectiveMtx( betterCam, &proj );

			D3DMATRIX viewProj = r3dRenderer->ViewMatrix * proj ;

			if(cloudsParams.useEngineColors != 0)
			{
				cloudsParams.m_vLightColor = D3DXVECTOR3(Sun->SunLight.R/255.0f, Sun->SunLight.G/255.0f, Sun->SunLight.B/255.0f);
				cloudsParams.m_vAmbientLight = D3DXVECTOR3(r3dRenderer->AmbientColor.R/255.0f, r3dRenderer->AmbientColor.G/255.0f, r3dRenderer->AmbientColor.B/255.0f);
			}

			r3dRenderer->SetRenderingMode ( R3D_BLEND_NOALPHA | R3D_BLEND_ZC  );
			g_pCloudPlane->Render(*Cam.d3dx(), *V.d3dx(), r3dGetFrameTime()*0.1f, viewProj, DESIRED_FAR, cloudTex, cloudCoverTex);
		}
	}

	r3dRenderer->SetCamera(Cam);
	r3dRenderer->RestoreCullMode();

}

void r3dSkyDome::Draw( const r3dCamera &Cam, bool normals, float amplify, bool hemisphere )
{
	R3DPROFILE_FUNCTION("r3dSkyDome::Draw");

	struct PerfEvent
	{
		PerfEvent()
		{
			D3DPERF_BeginEvent( 0, L"r3dSkyDome::Draw" ) ;
		}

		~PerfEvent()
		{
			D3DPERF_EndEvent() ;
		}
		
	} perfEvent; (void)perfEvent ;

	if(!bLoaded)
		return;

	r3dCamera 	Cam1;
	r3dPoint3D  Pos;

	memcpy ( &Cam1, Cam, sizeof ( r3dCamera));

	Pos.Assign(0, 0, 0); //-300
	Cam1.SetPosition(Pos);
	Cam1.SetPlanes     ( 1.0f, SKY_FAR_PLANE );

	r3dRenderer->SetCamera(Cam1);
	r3dRenderer->SetMaterial(NULL);

	// update
	m_fCloadAnim += 0.002f * m_fWindFactor * r3dGetFrameTime();
	if(m_fCloadAnim > 2.0f) 
		m_fCloadAnim = m_fCloadAnim - 2.0f;

	r3dRenderer->SetRenderingMode ( R3D_BLEND_NOALPHA | R3D_BLEND_ZC );

	D3DXMATRIX viewProj = r3dRenderer->ViewProjMatrix ;

	DrawDome( Cam, viewProj, 1.f, normals, amplify, hemisphere );

	//r3dRenderer->pd3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	r3dRenderer->SetCamera(Cam);
	r3dRenderer->RestoreCullMode();
}

void r3dSkyDome::DrawCubemap( const r3dCamera& Cam )
{
	D3DPERF_BeginEvent( 0, L"r3dSkyDome::DrawCubemap" ) ;

	D3DXMATRIX ShaderMat;

	LPDIRECT3DDEVICE9 d = r3dRenderer->pd3ddev;

	r3dRenderer->SetTex( NULL, 7 );
	//avoid "Direct3D9: (WARN) :Can not render to a render target that is also used as a texture"

	LPDIRECT3DSURFACE9 dss;
	r3dRenderer->GetDSS(&dss);	
	r3dRenderer->SetDSS(0);

	LPDIRECT3DSURFACE9 targets[4];
	for(int i = 0; i < 4; ++i)
	{
		r3dRenderer->GetRT( i, &targets[i] );

		if( i )
		{
			r3dRenderer->SetRT(i, 0);
		}
	}

	r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_NZ ) ;

	for(int i = D3DCUBEMAP_FACE_POSITIVE_X; i <= D3DCUBEMAP_FACE_NEGATIVE_Z; ++i)
	{			
		D3DXMATRIX viewProj = cubeMtx.view[i] * cubeMtx.projection;

		D3DXMATRIX ShaderMat ;

		D3DXMatrixTranspose( &ShaderMat, &viewProj );

		d->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );
		
		tempRt0->Activate() ;
		d->Clear(0, NULL, D3DCLEAR_TARGET, 0xFFFFFFFF, 0.0f, 0);

		DrawDome( Cam, viewProj, 0.f, false, 1.f, false ) ;

		tempRt0->Deactivate() ;

		RenderCubeFaceMipChain( cubemap, i, tempRt0, tempRt1 ) ;
	}

	for(int i = 0; i < 4; ++i)
	{
		if(targets[i])	r3dRenderer->SetRT(i, targets[i]);
		if(targets[i])	targets[i]->Release();
	}	

	if(dss)
	{
		r3dRenderer->SetDSS(dss);
	}

	SAFE_RELEASE(dss);

	D3DPERF_EndEvent();
};

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);

void r3dSkyDome::D3DCreateResource()
{
	updateAfterRestore = true;	//redraw cubemap
}

void r3dSkyDome::D3DReleaseResource()
{
}


