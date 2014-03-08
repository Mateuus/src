
#include "r3dPCH.h"
#include "r3d.h"

#include "r3dDeviceQueue.h"

r3dCubeMap::r3dCubeMap(int size, const r3dIntegrityGuardian& ig /*= r3dIntegrityGuardian()*/ )
: r3dIResource( ig )
{
  size_ = size;
  
  m_pRenderToEnvMap = NULL;
  m_pCubeMap        = NULL;

  CreateQueuedResource( this );

  return;
}

r3dCubeMap::~r3dCubeMap()
{
	ReleaseQueuedResource( this );
}

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);


void r3dCubeMap::D3DCreateResource()
{
  D3DFORMAT fmt = r3dRenderer->d3dpp.BackBufferFormat;

  // Create RenderToEnvMap object
  D3DXCreateRenderToEnvMap(r3dRenderer->pd3ddev, size_, 0, fmt, TRUE, D3DFMT_D16, &m_pRenderToEnvMap);

  // create cubemap
  if(FAILED( D3DXCreateCubeTexture(r3dRenderer->pd3ddev, size_, 1, D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &m_pCubeMap))) {
    if(FAILED( D3DXCreateCubeTexture(r3dRenderer->pd3ddev, size_, 1, 0, fmt, D3DPOOL_DEFAULT, &m_pCubeMap ))) {
      m_pCubeMap = NULL;
      return;
    }
  }

  SetD3DResourcePrivateData(m_pCubeMap, "r3dCubeMap");

  r3dRenderer->Stats.AddRenderTargetMem ( +r3dGetPixelSize( size_* size_ * 6, fmt ) );

  return;
}

void r3dCubeMap::D3DReleaseResource()
{
  R3D_ENSURE_MAIN_THREAD();

  SAFE_RELEASE(m_pCubeMap);
  SAFE_RELEASE(m_pRenderToEnvMap);

  D3DFORMAT fmt = r3dRenderer->d3dpp.BackBufferFormat;
  r3dRenderer->Stats.AddRenderTargetMem ( -(int)r3dGetPixelSize( size_* size_ * 6, fmt ) );
}


void r3dCubeMap::GetCubeMapViewMatrix(D3DXMATRIX *m, D3DCUBEMAP_FACES face)
{
	D3DXVECTOR3 vEyePt   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLookDir;
	D3DXVECTOR3 vUpDir;

  switch(face)
  {
    case D3DCUBEMAP_FACE_POSITIVE_X:
      vLookDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_X:
      vLookDir = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_POSITIVE_Y:
      vLookDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_Y:
      vLookDir = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
      break;
    case D3DCUBEMAP_FACE_POSITIVE_Z:
      vLookDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_Z:
      vLookDir = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
      vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
      break;
  }

  // Set the view transform for this cubemap surface
  D3DXMatrixLookAtLH(m, &vEyePt, &vLookDir, &vUpDir );
  return;
}


void r3dCubeMap::GetCubeMapViewDir(D3DXVECTOR3 *vLookDir, D3DCUBEMAP_FACES face)
{
  switch(face)
  {
    case D3DCUBEMAP_FACE_POSITIVE_X:
      *vLookDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_X:
      *vLookDir = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_POSITIVE_Y:
      *vLookDir = D3DXVECTOR3( 0.01f, 0.99f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_Y:
      *vLookDir = D3DXVECTOR3( 0.01f,-0.99f, 0.0f);
      break;
    case D3DCUBEMAP_FACE_POSITIVE_Z:
      *vLookDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
      break;
    case D3DCUBEMAP_FACE_NEGATIVE_Z:
      *vLookDir = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
      break;
  }

  return;
}



int r3dCubeMap::Render(const r3dPoint3D& pos, r3dCamera &Cam1, fn_DrawWorld draw)
{
  if(!m_pCubeMap || !m_pRenderToEnvMap)
    return 0;

  D3DXMATRIX mWorld1, 
			mView1 = r3dRenderer->ViewMatrix , 
			mProj1 = r3dRenderer->ProjMatrix ;

  D3DXMatrixIdentity( &mWorld1 ) ;

  HRESULT hr;
  hr = m_pRenderToEnvMap->BeginCube(m_pCubeMap);
  if(FAILED(hr))
    return 0;

  for(int face = 0; face < 6; face++) {
    m_pRenderToEnvMap->Face((D3DCUBEMAP_FACES)face, D3DX_FILTER_LINEAR);
    r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, r3dRenderer->Fog.Color.GetPacked(), 1.0f, 0 );

    D3DXMATRIX mWorld, mView, mProj;

/*
    D3DXMatrixIdentity(&mView);
    D3DXVECTOR3 vLookDir;
    GetCubeMapViewMatrix(&mView, (D3DCUBEMAP_FACES)face, vLookDir);
    mView._41 = pos.x; 
    mView._42 = pos.y; 
    mView._43 = pos.z;

    D3DXMatrixIdentity(&mWorld);

    D3DXMatrixIdentity(&mProj);
    D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.5f, 1.0f, r3dRenderer->NearClip, r3dRenderer->FarClip);

    r3dRenderer->pd3ddev->SetTransform(D3DTS_WORLD,      &mWorld);
    r3dRenderer->pd3ddev->SetTransform(D3DTS_PROJECTION, &mProj);
    r3dRenderer->pd3ddev->SetTransform(D3DTS_VIEW,       &mView);


  // update frustum planes for renderer
    D3DXMATRIX mat;
    D3DXMatrixMultiply( &mat, &mView, &mProj );
    D3DXMatrixInverse( &mat, NULL, &mat );
    r3dRenderer->vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  0.0f); // xyz
    r3dRenderer->vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  0.0f); // Xyz
    r3dRenderer->vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  0.0f); // xYz
    r3dRenderer->vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  0.0f); // XYz
    r3dRenderer->vecFrustum[4] = D3DXVECTOR3(-1.0f, -1.0f,  1.0f); // xyZ
    r3dRenderer->vecFrustum[5] = D3DXVECTOR3( 1.0f, -1.0f,  1.0f); // XyZ
    r3dRenderer->vecFrustum[6] = D3DXVECTOR3(-1.0f,  1.0f,  1.0f); // xYZ
    r3dRenderer->vecFrustum[7] = D3DXVECTOR3( 1.0f,  1.0f,  1.0f); // XYZ

    for(int i = 0; i < 8; i++ )
        D3DXVec3TransformCoord( &r3dRenderer->vecFrustum[i], &r3dRenderer->vecFrustum[i], &mat );

    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[0], &r3dRenderer->vecFrustum[0], &r3dRenderer->vecFrustum[1], &r3dRenderer->vecFrustum[2] ); // Near
    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[1], &r3dRenderer->vecFrustum[6], &r3dRenderer->vecFrustum[7], &r3dRenderer->vecFrustum[5] ); // Far
    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[2], &r3dRenderer->vecFrustum[2], &r3dRenderer->vecFrustum[6], &r3dRenderer->vecFrustum[4] ); // Left
    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[3], &r3dRenderer->vecFrustum[7], &r3dRenderer->vecFrustum[3], &r3dRenderer->vecFrustum[5] ); // Right
    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[4], &r3dRenderer->vecFrustum[2], &r3dRenderer->vecFrustum[3], &r3dRenderer->vecFrustum[6] ); // Top
    D3DXPlaneFromPoints( &r3dRenderer->planeFrustum[5], &r3dRenderer->vecFrustum[1], &r3dRenderer->vecFrustum[0], &r3dRenderer->vecFrustum[4] ); // Bottom
*/


    D3DXVECTOR3 vDir;
    GetCubeMapViewDir(&vDir, (D3DCUBEMAP_FACES)face);
//    GetCubeMapRefractionDir(&vDir, (D3DCUBEMAP_FACES)face);

    float oldfov = Cam1.FOV;
    Cam1.FOV = 90;
    Cam1.SetPosition(pos);
    Cam1.PointTo(r3dPoint3D(pos.x + vDir.x, pos.y + vDir.y, pos.z + vDir.z));
    r3dRenderer->SetCamera(Cam1);

    draw();
    Cam1.FOV = oldfov;
  }

  m_pRenderToEnvMap->End(D3DX_FILTER_LINEAR);

#ifndef FINAL_BUILD
  r3dRenderer->pd3ddev->SetTransform(D3DTS_WORLD,      &mWorld1);
  r3dRenderer->pd3ddev->SetTransform(D3DTS_PROJECTION, &mProj1);
  r3dRenderer->pd3ddev->SetTransform(D3DTS_VIEW,       &mView1);
#endif

  return 1;
}

#if 0
int r3dCubeMap::Load(const char* FName)
{
  if(isDynamic) SAFE_RELEASE(m_pRenderToEnvMap);
  SAFE_RELEASE(m_pCubeMap);

  isDynamic = 0;

/*
  FAILED( D3DXCreateTextureFromFile(r3dRenderer->pd3ddev, FName, m_pCubeMap))
  {
      m_pCubeMap = NULL;
      return 0;
  }
*/
  return 0;
}
#endif

int r3dCubeMap::Save(const char* fname)
{
  D3DXSaveTextureToFile(fname, D3DXIFF_DDS, m_pCubeMap, NULL);
  return 1;
}


//------------------------------------------------------------------------

//------------------------------------------------------------------------

void RenderCubeFaceMipChain( r3dScreenBuffer* cube, int face, r3dScreenBuffer* temp0, r3dScreenBuffer* temp1 ) 
{
	if( cube->ActualNumMipLevels < 2 )
	{
		r3dOutToLog( "RenderMipChain: called for r3dScreenBuffer with only one mip!\n" ) ;
		return ;
	}

	r3d_assert( temp0->Width >= cube->Width ) ;
	r3d_assert( temp0->Height >= cube->Height ) ;

	r3d_assert( temp1->Width >= cube->Width / 2 ) ;
	r3d_assert( temp1->Height >= cube->Height / 2 ) ;

	struct PerfEventsAndStates
	{
		PerfEventsAndStates()
		{	
			D3DPERF_BeginEvent( 0, L"RenderCubeFaceMipChain") ;

			D3D_V( r3dRenderer->pd3ddev->GetRenderState( D3DRS_SCISSORTESTENABLE, & prevScissorEnable ) ) ;

			D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE ) ) ;

			r3dRenderer->SetCullMode( D3DCULL_NONE ) ;
			
		}

		~PerfEventsAndStates()	
		{	
			D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_SCISSORTESTENABLE, prevScissorEnable ) ) ;

			r3dRenderer->RestoreCullMode() ;

			D3DPERF_EndEvent() ;
		}

		DWORD prevScissorEnable ;

	} perfEventsAndStates ; (void)perfEventsAndStates ;

	r3dSetRestoreFSQuadVDecl setRestoreVDecl ; (void) setRestoreVDecl ;

	r3dRenderer->SetVertexShader( "VS_POSTFX_DEFAULT" );
	r3dRenderer->SetPixelShader( "PS_FSCOPY" );

	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP ) ) ;
	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP ) ) ;

	// copy 0 mip
	{
		cube->Activate( 0, face, 0 ) ;
	
		r3dRenderer->SetTex( temp0->Tex ) ;
		r3dSetFiltering( R3D_POINT, 0 ) ;

		const r3dScreenBuffer::Dims& dims = cube->MipDims[ 0 ] ;

		// float4 		g_vTexcTransform	: register ( c36 );
		float vsConst[ 4 ] = { dims.Width / temp0->Width , dims.Height / temp0->Height, 0.5f / temp0->Width, 0.5f / temp0->Height } ;
		D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 36, vsConst, 1 ) ) ;

		r3dDrawFullScreenQuad( false );

		cube->Deactivate() ;
	}

	for( int i = 1, e = cube->ActualNumMipLevels ; i < e ; i ++ )
	{
		const r3dScreenBuffer::Dims& prevDims = cube->MipDims[ i - 1 ] ;
		const r3dScreenBuffer::Dims& dims = cube->MipDims[ i ] ;

		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_MAXMIPLEVEL, 0 ) ) ;
		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_MIPMAPLODBIAS, 0 ) ) ;

		temp1->Activate() ;

		r3dRenderer->SetTex( temp0->Tex ) ;

		r3dSetFiltering( R3D_BILINEAR, 0 ) ;
		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT ) ) ;

		// float4 		g_vTexcTransform	: register ( c36 );
		float vsConst[ 4 ] = { prevDims.Width / temp0->Width , prevDims.Height / temp0->Height, 1.0f / temp0->Width, 1.0f / temp0->Height } ;
		D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 36, vsConst, 1 ) ) ;

		r3dRenderer->SetViewport( 0, 0, dims.Width, dims.Height ) ;

		r3dDrawFullScreenQuad( false );

		temp1->Deactivate() ;

		// copy mip
		{
			cube->Activate( 0, face, i ) ;

			r3dRenderer->SetTex( temp1->Tex ) ;
			r3dSetFiltering( R3D_POINT, 0 ) ;

			// float4 		g_vTexcTransform	: register ( c36 );
			float vsConst[ 4 ] = { dims.Width / temp1->Width , dims.Height / temp1->Height, 0.5f / temp1->Width, 0.5f / temp1->Height } ;
			D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 36, vsConst, 1 ) ) ;

			r3dDrawFullScreenQuad( false );

			cube->Deactivate() ;
		}

		R3D_SWAP( temp0, temp1 ) ;
	}

	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_MIPMAPLODBIAS, 0 ) ) ;
	// to override our mip filter hack
	r3dSetFiltering( R3D_POINT, 0 ) ;

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();

}
