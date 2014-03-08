#include "r3dPCH.h"
#include "r3d.h"

#include "../SF/script.h"
#include "r3dBackgroundTaskDispatcher.h"

#include "r3dDeviceQueue.h"

typedef r3dTL::TArray< TextureReloadListener > TextureReloadListeners ;

static TextureReloadListeners gTextureReloadListeners ;

static 	int 	r3dLastTextureID     = 100;

int	r3dTexture_ScaleCoef = 1;
int	r3dTexture_MinSize   = 1;		// Minimum possible texture dimension after scaling
int	r3dTexture_ScaleBoxInterpolate = 0;	// Use D3D_FILTER_BOX interpolation
int	r3dTexture_UseEmpty  = 0; // for server

//C
//
// r3dTexture
//
//

r3dTexture::r3dTexture()
{
	Missing = 0;

	m_Loaded = 0 ;

	m_TexArray = NULL;

	m_pDelayTextureArray = 0;
	m_iNumTextures = 0;
	m_LastAccess = 0;
	m_AccessCounter = 0;

	Width 		= 0;
	Height    	= 0;
	Depth			= 0;
	TexFormat 	= D3DFMT_UNKNOWN;
	Flags         = 0;

	Instances     = 0;

	pNext         = NULL;
	pPrev         = NULL;

	NumMipMaps	= 0;

	bPersistent	= 0;
	bCubemap		= false;

	ID	        = r3dLastTextureID++;
}

void r3dTexture::Unload()
{
	if( ( m_Loaded || Flags&fCreated ) && !( Flags & fRenderTarget ) )
	{
		int size = -(int) GetTextureSizeInVideoMemory() ;

		UpdateTextureStats( size ) ;
	}

	DestroyInternal() ;

	InterlockedExchange( &m_Loaded, 0 ) ;
}

r3dTexture::~r3dTexture()
{
	Unload();
}

void *r3dTexture::Lock(int LockForWrite, const RECT *LocRect)
{
	D3DLOCKED_RECT lr;

	if( !m_TexArray[0].Valid() )
		return NULL;

	DWORD flags( 0 );

	if( !LockForWrite )
	{
		flags |= D3DLOCK_READONLY;
	}

	GetD3DTunnel().LockRect(0, &lr, LocRect, flags) ;

	Flags |= r3dTexture::fLocked;
	if(LockForWrite)
		Flags |= r3dTexture::fLockedForWrite;
	Pitch  = lr.Pitch;
	return lr.pBits;

}

void r3dTexture::Unlock()
{
	if(Flags & r3dTexture::fLocked) 
	{
		GetD3DTunnel().UnlockRect(0);
		Flags &= ~r3dTexture::fLocked;
	}

}

D3DPOOL r3dDefaultTexturePool = D3DPOOL_MANAGED;

void CalcDownScaleToMatchMaxDim( int& ioRatio, int Dim, int MaxDim )
{
	if( Dim / ioRatio > MaxDim )
	{
		ioRatio = R3D_MAX( MaxDim / Dim, 1 );

		if( Dim / ioRatio > MaxDim )
		{
			ioRatio ++;
		}

		r3d_assert( Dim / ioRatio >= MaxDim );
	}
}

namespace
{
	struct DownTexParams
	{
		r3dD3DTextureTunnel* texTunnel ;
		int mipsDown ;
	};

	void DownCube( r3dD3DTextureTunnel* texTunnel, int mipsDown )
	{
		R3D_ENSURE_MAIN_THREAD();

		IDirect3DCubeTexture9* cubeTex = texTunnel->AsTexCube();

		D3DSURFACE_DESC desc;

		D3D_V( cubeTex->GetLevelDesc( mipsDown, &desc ) );

		int origLevelCount = cubeTex->GetLevelCount();

		IDirect3DCubeTexture9* res;

		D3D_V( r3dRenderer->pd3ddev->CreateCubeTexture( desc.Width, origLevelCount - mipsDown, 0, desc.Format, desc.Pool, &res, NULL ) );

		for( int i = 0, e = res->GetLevelCount(); i < e; i ++ )
		{
			D3DLOCKED_RECT from, to;

			for( int j = 0; j < 6; j ++ )
			{
				D3D_V( cubeTex->LockRect( D3DCUBEMAP_FACES( j ), i + mipsDown, &from, NULL, D3DLOCK_READONLY ) );
				D3D_V( res->LockRect( D3DCUBEMAP_FACES( j ), i, &to, NULL, 0 ) );

				D3DSURFACE_DESC desc;

				D3D_V( res->GetLevelDesc( i, &desc ) );

				r3d_assert( from.Pitch == to.Pitch );
				memcpy( to.pBits, from.pBits, r3dGetPixelSize( to.Pitch * desc.Height, desc.Format ) );

				D3D_V( cubeTex->UnlockRect( D3DCUBEMAP_FACES( j ), i + mipsDown ) );
				D3D_V( res->UnlockRect( D3DCUBEMAP_FACES( j ), i ) );
			}
		}

		texTunnel->ReleaseAndReset();

		texTunnel->Set( res );
	}

	void DoDownCube( void* params )
	{
		DownTexParams* pms = ( DownTexParams* ) params ;

		DownCube( pms->texTunnel, pms->mipsDown );
	}


	void DownTex2D( r3dD3DTextureTunnel* tex2DTunnel, int mipsDown )
	{
		R3D_ENSURE_MAIN_THREAD();

		D3DSURFACE_DESC desc;

		IDirect3DTexture9* tex2D = tex2DTunnel->AsTex2D() ;

		D3D_V( tex2D->GetLevelDesc( mipsDown, &desc ) );

		int origLevelCount = tex2D->GetLevelCount();

		IDirect3DTexture9* res;

		int mips = origLevelCount - mipsDown ;

		HRESULT hres = r3dRenderer->pd3ddev->CreateTexture( desc.Width, desc.Height, mips, 0, desc.Format, desc.Pool, &res, NULL );

		r3dRenderer->CheckOutOfMemory( hres ) ;

		if( hres != D3D_OK )
		{
			if( hres == E_OUTOFMEMORY )
			{
				r3dOutToLog( "DownTex2D: out of memory!\n" );
			}

			if( hres == D3DERR_OUTOFVIDEOMEMORY )
			{
				r3dOutToLog( "DownTex2D: out VIDEO memory!\n" );
			}

			r3dError( "DownTex2D: CreateTexture call failed: Width = %d, Height = %d, mips = %d, format = %d, pool = %d\n", desc.Width, desc.Height, mips, desc.Format, desc.Pool );
		}


		for( int i = 0, e = res->GetLevelCount(); i < e; i ++ )
		{
			D3DLOCKED_RECT from, to;

			D3D_V( tex2D->LockRect( i + mipsDown, &from, NULL, D3DLOCK_READONLY ) );
			D3D_V( res->LockRect( i, &to, NULL, 0 ) );

			D3DSURFACE_DESC desc;

			D3D_V( res->GetLevelDesc( i, &desc ) );

			r3d_assert( from.Pitch == to.Pitch );
			memcpy( to.pBits, from.pBits, to.Pitch * r3dGetPitchHeight( desc.Height, desc.Format ) );

			D3D_V( tex2D->UnlockRect( i + mipsDown ) );
			D3D_V( res->UnlockRect( i ) );
		}
		
		tex2DTunnel->ReleaseAndReset();

		tex2DTunnel->Set( res );
	}

	void DoDownTex2D( void* params )
	{
		DownTexParams* pms = ( DownTexParams* ) params ;

		DownTex2D( pms->texTunnel, pms->mipsDown );
	}

	void DownTex3D( r3dD3DTextureTunnel* TexTunnel, int mipsDown )
	{
		R3D_ENSURE_MAIN_THREAD();

		D3DVOLUME_DESC desc;

		IDirect3DVolumeTexture9* volTex = TexTunnel->AsTexVolume();

		D3D_V( volTex->GetLevelDesc( mipsDown, &desc ) );

		int origLevelCount = volTex->GetLevelCount();

		IDirect3DVolumeTexture9* res;

		D3D_V( r3dRenderer->pd3ddev->CreateVolumeTexture( desc.Width, desc.Height, desc.Depth, origLevelCount - mipsDown, 0, desc.Format, desc.Pool, &res, NULL ) );

		for( int i = 0, e = res->GetLevelCount(); i < e; i ++ )
		{
			D3DLOCKED_BOX from, to;

			D3D_V( volTex->LockBox( i + mipsDown, &from, NULL, D3DLOCK_READONLY ) );
			D3D_V( res->LockBox( i, &to, NULL, 0 ) );

			D3DVOLUME_DESC desc;

			D3D_V( res->GetLevelDesc( i, &desc ) );

			r3d_assert( from.RowPitch == to.RowPitch && 
						from.SlicePitch == to.SlicePitch );

			memcpy( to.pBits, from.pBits, r3dGetPixelSize( to.SlicePitch * desc.Depth, desc.Format ) );

			D3D_V( volTex->UnlockBox( i + mipsDown ) );
			D3D_V( res->UnlockBox(  i ) );
		}

		TexTunnel->ReleaseAndReset() ;
		TexTunnel->Set( volTex ) ;
	}

	void DoDownTex3D( void* params )
	{
		DownTexParams* pms = ( DownTexParams* ) params ;

		DownTex3D( pms->texTunnel, pms->mipsDown );
	}


	DWORD ilog2( DWORD val )
	{
		if( val <= 1 )
			return 0;

		int res = 0;

		while( val /= 2 )
		{
			res ++;
		}

		return res;
				
	}
}

#include "D3Dcommon.h"

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName)
{
#if R3D_SET_DEBUG_D3D_NAMES

	DWORD sz = strlen(FName);
	res->SetPrivateData(WKPDID_D3DDebugObjectName, FName, sz, 0);

	void* p = 0;

	res->QueryInterface(IID_IDirect3DTexture9, &p);
	if(p)
	{
		LPDIRECT3DTEXTURE9 t = (LPDIRECT3DTEXTURE9)p;
		int mipsCount = t->GetLevelCount();

		for (int i = 0; i < mipsCount; ++i)
		{
			LPDIRECT3DSURFACE9 surf; 
			t->GetSurfaceLevel(i, &surf);
			surf->SetPrivateData(WKPDID_D3DDebugObjectName, FName, sz, 0);
			surf->Release();
		}

		t->Release();
		return;
	}

	p = 0;
	res->QueryInterface(IID_IDirect3DCubeTexture9, &p);
	if(p)
	{
		LPDIRECT3DCUBETEXTURE9 t = (LPDIRECT3DCUBETEXTURE9)p;
		int mipsCount = t->GetLevelCount();

		for (int i = 0; i < mipsCount; ++i)
		{
			for (int j = D3DCUBEMAP_FACE_POSITIVE_X; j <= D3DCUBEMAP_FACE_NEGATIVE_Z; ++j)
			{
				LPDIRECT3DSURFACE9 surf; 
				t->GetCubeMapSurface((D3DCUBEMAP_FACES)j, i, &surf);
				surf->SetPrivateData(WKPDID_D3DDebugObjectName, FName, sz, 0);
				surf->Release();
			}
		}

		t->Release();

		return;
	}

#endif
}

struct PostLoadStuffParams
{
	r3dD3DTextureTunnel* tunnel ;
	int * oNumMips ;
	const char* FileName ;
	void* StuffTofree ;
};

void DoPostLoadStuff( void* params )
{
	PostLoadStuffParams* pms = (PostLoadStuffParams*) params ;

	*pms->oNumMips = (*pms->tunnel)->GetLevelCount();
	SetD3DResourcePrivateData( (*pms->tunnel).Get(), pms->FileName );

	free( pms->StuffTofree );
}

void r3dTexture::LoadTextureInternal(int index, void* FileInMemoryData, uint32_t FileInMemorySize, D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem, const char* DEBUG_NAME )
{
	D3DPOOL targetPool = SystemMem ? D3DPOOL_SYSTEMMEM : r3dDefaultTexturePool ;

	int TgW = 0;
	int TgH = 0;
	int TgD = 0;

	R3D_DEIVCE_QUEUE_OBJ( D3DXIMAGE_INFO, pInfo) ;
	ZeroMemory(&pInfo, sizeof (pInfo));

	D3DXGetImageInfoFromFileInMemory( FileInMemoryData, FileInMemorySize, &pInfo );

	int Mip = 1;

	int DownScaledDueToMaxDim = 1;

	UINT ScaledWidth	= pInfo.Width;
	UINT ScaledHeight	= pInfo.Height;
	UINT ScaledDepth	= pInfo.Depth;

	if( int maxDim = r_max_texture_dim->GetInt() )
	{
		CalcDownScaleToMatchMaxDim( DownScaledDueToMaxDim, ScaledWidth	, maxDim );
		CalcDownScaleToMatchMaxDim( DownScaledDueToMaxDim, ScaledHeight	, maxDim );
		CalcDownScaleToMatchMaxDim( DownScaledDueToMaxDim, ScaledDepth	, maxDim );

		ScaledWidth		= R3D_MAX( int( ScaledWidth		/ DownScaledDueToMaxDim ), 1 );
		ScaledHeight	= R3D_MAX( int( ScaledHeight	/ DownScaledDueToMaxDim ), 1 );
		ScaledDepth		= R3D_MAX( int( ScaledDepth		/ DownScaledDueToMaxDim ), 1 );
	}

	int NextDownScale = R3D_MAX( DownScale / DownScaledDueToMaxDim, 1 );

	ScaledWidth		= R3D_MAX( int( ScaledWidth		/ NextDownScale ), 1 );
	ScaledHeight	= R3D_MAX( int( ScaledHeight	/ NextDownScale ), 1 );
	ScaledDepth		= R3D_MAX( int( ScaledDepth		/ NextDownScale ), 1 );

	UINT totalMipDown = ilog2( R3D_MAX( 
								R3D_MAX( pInfo.Width / ScaledWidth, 
										pInfo.Height / ScaledHeight ), 
											pInfo.Depth / ScaledDepth ) );

	// too small or may be incompatible with DXT after downscale
	if( 
		( (int)pInfo.Width <= DownScaleMinDim || (int)pInfo.Height <= DownScaleMinDim )
			||

		( pInfo.Width < 16 || pInfo.Height < 16 )
		)
	{
		totalMipDown = 0 ;

		ScaledWidth		= pInfo.Width;
		ScaledHeight	= pInfo.Height;
		ScaledDepth		= pInfo.Depth;
	}

	TgW = ScaledWidth;
	TgH = ScaledHeight;
	TgD = ScaledDepth;

	bCubemap = pInfo.ResourceType == D3DRTYPE_CUBETEXTURE;

	int ScaleFilter = totalMipDown ? D3DX_FILTER_POINT : D3DX_FILTER_NONE ;

	r3d_assert( !(TgD > 1 && bCubemap) );

	const bool ALLOW_ASYNC = !!g_async_d3dqueue->GetInt() ;

	if( bCubemap )
	{			
		if( totalMipDown && totalMipDown < pInfo.MipLevels )
		{

			r3dDeviceTunnel::D3DXCreateCubeTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, pInfo.Width, pInfo.MipLevels - totalMipDown, 0, TargetTexFormat, targetPool,
																		D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0x00000000, &pInfo, NULL, &m_TexArray[ index ], ALLOW_ASYNC );

			R3D_DEIVCE_QUEUE_OBJ( DownTexParams, parms ) ;

			parms.texTunnel = &m_TexArray[ index ] ;
			parms.mipsDown = totalMipDown ;

			AddCustomDeviceQueueItem( DoDownCube, &parms );
		}
		else
		{
			r3dDeviceTunnel::D3DXCreateCubeTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, ScaledWidth, pInfo.MipLevels, 0, TargetTexFormat, targetPool,
																		ScaleFilter, ScaleFilter, 0x00000000, &pInfo, NULL, &m_TexArray[ index ], ALLOW_ASYNC ) ;
		}
	}
	else
	{
		if( TgD <= 1 )
		{
			if( totalMipDown && totalMipDown < pInfo.MipLevels )
			{
				// load original texture

				r3dDeviceTunnel::D3DXCreateTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, pInfo.Width, pInfo.Height, pInfo.MipLevels, 0, TargetTexFormat, targetPool,
																		D3DX_FILTER_NONE, D3DX_FILTER_NONE ,0x00000000, &pInfo, NULL, &m_TexArray[ index ], DEBUG_NAME, ALLOW_ASYNC );
				
				// create downscaled version by copying mip levels into a new texture
				R3D_DEIVCE_QUEUE_OBJ( DownTexParams, params ) ;

				params.texTunnel = &m_TexArray[ index ] ;
				params.mipsDown = totalMipDown ;

				AddCustomDeviceQueueItem( DoDownTex2D, &params );
			}
			else
			{
				r3dDeviceTunnel::D3DXCreateTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, ScaledWidth, ScaledHeight, pInfo.MipLevels, 0, TargetTexFormat, targetPool,
																		ScaleFilter, ScaleFilter, 0x00000000, &pInfo, NULL, &m_TexArray[ index ], DEBUG_NAME, ALLOW_ASYNC );
			}
		}
		else
		{
			if( totalMipDown && totalMipDown < pInfo.MipLevels )
			{
				r3dDeviceTunnel::D3DXCreateVolumeTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, ScaledWidth, ScaledHeight, ScaledDepth, pInfo.MipLevels - totalMipDown, 0, TargetTexFormat, targetPool,
																			D3DX_FILTER_POINT, D3DX_FILTER_POINT, 0x00000000, &pInfo, NULL, &m_TexArray[ index ], ALLOW_ASYNC );

				R3D_DEIVCE_QUEUE_OBJ( DownTexParams, params ) ;

				params.texTunnel = &m_TexArray[ index ] ;
				params.mipsDown = totalMipDown ;

				AddCustomDeviceQueueItem( DoDownTex3D, &params );

			}
			else
			{
				r3dDeviceTunnel::D3DXCreateVolumeTextureFromFileInMemoryEx(	FileInMemoryData, FileInMemorySize, ScaledWidth, ScaledHeight, ScaledDepth, pInfo.MipLevels, 0, TargetTexFormat, targetPool,
																			ScaleFilter, ScaleFilter, 0x00000000, &pInfo, NULL, &m_TexArray[ index ], ALLOW_ASYNC );
			}
		}
	}


	if( !ALLOW_ASYNC )
	{
		free( FileInMemoryData );
		FileInMemoryData = 0 ;
	}

	if( !g_async_d3dqueue->GetInt() )
	{
		if(!m_TexArray[ index ].Valid()) // failed to load?
		{
			r3dArtBug("Failed to load texture '%s'\n", Location.FileName);
		}
	}

	Width       = TgW;
	Height      = TgH;
	Depth		= TgD;

	TexFormat   = pInfo.Format;

	R3D_DEIVCE_QUEUE_OBJ( PostLoadStuffParams, parms ) ;

	parms.FileName = Location.FileName ;
	parms.oNumMips = &NumMipMaps ;
	parms.tunnel = &m_TexArray[index] ;
	parms.StuffTofree = FileInMemoryData ;

	if( g_async_d3dqueue->GetInt() )
		AddCustomDeviceQueueItem( DoPostLoadStuff, &parms );
	else
		ProcessCustomDeviceQueueItem( DoPostLoadStuff, &parms );

}

void r3dTexture::LoadTextureInternal( int index, const char* FName, D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem )
{
	r3d_assert(index >=0 && index < m_iNumTextures);
	if(r3dTexture_UseEmpty) 
	{
		Width       = 16;
		Height      = 16;
		Depth		= 1;
		TexFormat   = D3DFMT_A8R8G8B8;

		sprintf(Location.FileName, "%s", FName);
		strlwr(&Location.FileName[0]);

		m_TexArray[index].Set( 0 );
		return;
	}

	r3dFile *ff = r3d_open(FName,"rb");

	if (ff)
	{		
		Missing = 0 ;

		int ff_size = ff->size;
		if(ff_size == 0) // bad texture???
			r3dError("Bad texture '%s' - has zero size!!!\n", FName);

		void* SrcData = malloc ( ff_size+1 );
		if(SrcData == NULL)
			r3dError("Out of memory!");
		fread(SrcData, 1, ff_size, ff);
		fclose(ff) ;

		LoadTextureInternal( index, SrcData, ff_size, TargetTexFormat, DownScale, DownScaleMinDim, SystemMem, Location.FileName ) ;
	}
	else
	{
		LoadTextureInternal(index, "Data\\Shaders\\Texture\\MissingTexture.dds", TargetTexFormat, DownScale, DownScaleMinDim, SystemMem );
		Missing = 1;
		return;
	}
}

void r3dTexture::UpdateTextureStats( int size )
{
	if( Flags & fPlayerTexture )
		r3dRenderer->Stats.AddPlayerTexMem ( size );
	else
		r3dRenderer->Stats.AddTexMem ( size );
}

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);

void r3dTexture::SetDebugD3DComment(const char* text)
{
	for (int i = 0; i < m_iNumTextures; ++i)
	{
		r3dDeviceTunnel::SetD3DResourcePrivateData(	&m_TexArray[i], text );
	}
}

void
r3dTexture::MarkPlayerTexture()
{
	Flags |= fPlayerTexture ;
}

struct TextureLoadTaskParams : r3dTaskParams
{
	D3DFORMAT	TargetTexFormat ;
	int			DownScale ;
	int			DownScaleMinDim ;
	int			SystemMem ;
	r3dTexture* Loadee ;
};

r3dTaskParramsArray< TextureLoadTaskParams > g_TextureLoadTaskParams ;

/*static*/
void
r3dTexture::LoadTexture( struct r3dTaskParams* taskParams )
{
	TextureLoadTaskParams* params = static_cast<TextureLoadTaskParams*>( taskParams ) ;
	params->Loadee->DoLoad( params->TargetTexFormat, params->DownScale, params->DownScaleMinDim, params->SystemMem ) ;
}

int r3dTexture::Load( const char* fname, D3DFORMAT targetTexFormat, int downScale /*= 1*/, int downScaleMinDim /*= 1*/, int systemMem /*= 0*/ )
{
	sprintf(Location.FileName, "%s", fname);
	strlwr(&Location.FileName[0]);

	if( g_async_loading->GetInt() && R3D_IS_MAIN_THREAD() && g_pBackgroundTaskDispatcher)
	{
		r3dBackgroundTaskDispatcher::TaskDescriptor td ;

		TextureLoadTaskParams* params = g_TextureLoadTaskParams.Alloc() ;

		params->TargetTexFormat = targetTexFormat;
		params->DownScale		= downScale;
		params->DownScaleMinDim = downScaleMinDim;
		params->SystemMem		= systemMem ;

		params->Loadee			= this ;

		td.Params = params ;
		td.Fn = LoadTexture ;
		td.CompletionFlag	= 0 ;

		g_pBackgroundTaskDispatcher->AddTask( td ) ;
	}
	else
	{
		DoLoad( targetTexFormat, downScale, downScaleMinDim, systemMem ) ;
	}

	return 1 ;	
}

void r3dTexture::DestroyInternal()
{
	if( m_TexArray )
	{
		for(int i=0; i<m_iNumTextures; ++i)
		{
			if( m_TexArray[i].Valid() )
			{
				m_TexArray[i].ReleaseAndReset();
			}
		}

		delete [] m_TexArray;
		m_TexArray = NULL ;
	}
	SAFE_DELETE_ARRAY(m_pDelayTextureArray);
}

int r3dTexture::DoLoad( D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem )
{
	D3DXIMAGE_INFO pInfo;

	int Ret = 0;

	ZeroMemory(&pInfo, sizeof (pInfo));

	const char* FName = Location.FileName ;

	// check if we are loading animated texture *.ddv
	if(strstr(FName, ".ddv"))
	{
		Script_c script;
		if ( ! script.OpenFile( FName, true ) )
		{
			r3dArtBug("Failed to open '%s'\n", FName);
			return 0;
		}

		int numElements = 0;
		// we don't how many textures in ddv, so let's reserve for 50 maximum
		char tempFilenames[50][512];
		int  tempDelays[50];

		char tempStr[1024];
		while(!script.EndOfFile())
		{
			memset(tempStr, 0, sizeof(tempStr));
			script.GetLine(tempStr, 1024);
			if(strlen(tempStr) > 5)
			{
				sscanf(tempStr, "%s %d", tempFilenames[numElements], &tempDelays[numElements]);
				numElements++;
				if(numElements>=50)
				{
					r3dArtBug("Too many textures in DDV ('%s'), limit is 50!\n", FName);
					break;
				}
			}
		}
		script.CloseFile();

		const char* tSlash = strrchr(FName, '/');
		r3dscpy_s(tempStr, int(tSlash-FName)+1, FName);

		m_iNumTextures = numElements;
		m_TexArray = new r3dD3DTextureTunnel [ numElements ] ;
		m_pDelayTextureArray = new float[numElements];
		for(int i=0; i<numElements; ++i)
		{
			char fullpath[512];
			sprintf(fullpath, "%s%s", tempStr, tempFilenames[i]);
			LoadTextureInternal( i, fullpath, TargetTexFormat, DownScale, DownScaleMinDim, SystemMem );
			m_pDelayTextureArray[i] = (float)tempDelays[i]/1000.0f; // ms->seconds
		}		
	}
	else
	{
		m_iNumTextures = 1;
		m_TexArray = new r3dD3DTextureTunnel[ 1 ] ;

		LoadTextureInternal( 0, FName, TargetTexFormat, DownScale, DownScaleMinDim, SystemMem );
	}

	int size = GetTextureSizeInVideoMemory() ;

	UpdateTextureStats( size ) ;

	InterlockedExchange( &m_Loaded, 1 ) ;

	return 1;
}

IDirect3DBaseTexture9* r3dTexture::GetD3DTexture()
{
	if( m_Loaded )
	{
		if(m_iNumTextures == 1)
			return m_TexArray[0].Get();
		else if(m_iNumTextures > 1)
		{
			r3d_assert(m_pDelayTextureArray);
			r3d_assert(m_AccessCounter >= 0 && m_AccessCounter < m_iNumTextures);
			if((r3dGetTime() - m_LastAccess) > m_pDelayTextureArray[m_AccessCounter])
			{
				m_LastAccess = r3dGetTime();
				m_AccessCounter = (m_AccessCounter+1)%m_iNumTextures;
			}
			r3d_assert(m_AccessCounter >= 0 && m_AccessCounter < m_iNumTextures);
			return m_TexArray[m_AccessCounter].Get();
		}

	}

	return NULL;
}

void r3dTexture::SetNewD3DTexture(IDirect3DBaseTexture9* newTex) 
{
	if(m_TexArray[0].Valid()) 
		m_TexArray[0].ReleaseAndReset(); 

	m_TexArray[0].Set( newTex );
}

void r3dTexture::Setup( int XSize, int YSize, int ZSize, D3DFORMAT TexFmt, int aNumMipMaps, r3dD3DTextureTunnel* texture, bool isRenderTarget )
{
	bCubemap = false;

	TexFormat = TexFmt;

	NumMipMaps = aNumMipMaps;
	Width      = XSize;
	Height     = YSize;
	Depth      = ZSize;

	r3d_assert(Instances == 0);
	Instances = 1;

	r3d_assert(m_iNumTextures==0);
	m_TexArray = new r3dD3DTextureTunnel [ 1 ] ;
	m_iNumTextures = 1;
	m_TexArray[ 0 ] = *texture ;

	if( isRenderTarget )
	{
		Flags |= fRenderTarget ;
	}
	else
	{
		int size = GetTextureSizeInVideoMemory() ;

		UpdateTextureStats( size ) ;
	}

	InterlockedExchange( &m_Loaded, 1 ) ;
}

void r3dTexture::SetupCubemap( int EdgeLength, D3DFORMAT TexFmt, int aNumMipMaps, r3dD3DTextureTunnel* texture, bool isRenderTarget )
{
	TexFormat = TexFmt;

	bCubemap    = true;
	NumMipMaps  = aNumMipMaps;
	Width       = EdgeLength;
	Height      = EdgeLength;
	Depth       = 1;

	r3d_assert(Instances == 0);
	Instances = 1;

	r3d_assert(m_iNumTextures==0);
	m_TexArray = new r3dD3DTextureTunnel [ 1 ] ;
	m_iNumTextures = 1;
	m_TexArray[ 0 ] = *texture ;

	if( isRenderTarget )
	{
		Flags |= fRenderTarget ;
	}
	else
	{
		int size = GetTextureSizeInVideoMemory() ;

		UpdateTextureStats( size ) ;
	}

	InterlockedExchange( &m_Loaded, 1 ) ;
}

float GetD3DTexFormatSize(D3DFORMAT Fmt)
{
	switch(Fmt)
	{
	default:
		r3dOutToLog("GetD3DTexFormatSize, unknown format: %d\n", Fmt);
		return 2;
		// depth texture
	case D3DFMT_D24X8:
	case D3DFMT_D24S8:
		return 4.0f ;
	case D3DFMT_DXT1: 
		return 0.5f; // dxt1 compression ratio 1:8
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5: 
		return 1.0f; // dxt2,3,4,5 compression ratio 1:4
	case D3DFMT_R8G8B8: 
		return 3.0f;
	case D3DFMT_A8B8G8R8:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_G16R16:
	case D3DFMT_R32F:
		return 4.0f;
	case D3DFMT_G32R32F:
		return 8.0f ;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A4R4G4B4:
		return 2.0f;
	case D3DFMT_L16:
		return 2.0f;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_L8:
	case D3DFMT_A8R3G3B2: 
		return 1.0f;
	case D3DFMT_A32B32G32R32F:
		return 16.0f;
	}
}

int r3dTexture::GetTextureSizeInVideoMemory()
{
	int mips = NumMipMaps ;

	if( !mips )
	{
		if( IDirect3DBaseTexture9* tex = m_TexArray[ 0 ].AsBaseTex() )
		{
			mips = tex->GetLevelCount() ;
		}
	}

	int texSize = r3dGetTextureSizeInVideoMemory(Width, Height, Depth, mips, TexFormat);
	return texSize * m_iNumTextures;
}

void r3dTexture::RegisterCreated()
{
	Flags     |= fCreated;
	sprintf(Location.FileName, "$Created");

#if 0
	int size = + GetTextureSizeInVideoMemory() ;

	UpdateTextureStats( size ) ;
#endif
}

void
r3dTexture::RegisterCreatedCubemap()
{
	Flags		|= fCreated;
	sprintf(Location.FileName, "$Created");

#if 0
	int size = + GetTextureSizeInVideoMemory() ;

	UpdateTextureStats( size ) ;
#endif
}

int r3dTexture::Create(int XSize, int YSize, D3DFORMAT TexFmt, int _NumMipMaps, int SystemMem /*= 0*/ )
{
	r3dD3DTextureTunnel texTun ;

	r3dDeviceTunnel::CreateTexture(	XSize, YSize, _NumMipMaps,
									(r3dDefaultTexturePool == D3DPOOL_MANAGED) ? 0 : D3DUSAGE_DYNAMIC,
									TexFmt, SystemMem ? D3DPOOL_SYSTEMMEM : r3dDefaultTexturePool,
									&texTun );

	Setup( XSize, YSize, 1, TexFmt, _NumMipMaps, &texTun, false );

	RegisterCreated();

#if 0
	r3dOutToLog("r3dTexture: Texture [%dx%d] created\n", Width, Height);
#endif

	InterlockedExchange( &m_Loaded, 1 ) ;

	return 1;
}

//------------------------------------------------------------------------

void r3dTexture::Destroy()
{
	r3d_assert( Flags & fCreated ) ;

	if( !( Flags & fRenderTarget ) )
	{
		int size = -GetTextureSizeInVideoMemory() ;
		UpdateTextureStats( size ) ;
	}

	DestroyInternal() ;

	Flags &= ~fCreated ;

	InterlockedExchange( &m_Loaded, 0 ) ;

	r3d_assert(Instances == 1);
	Instances = 0;

	r3d_assert(m_iNumTextures>0);
	m_iNumTextures = 0 ;
}

//-------------------------------------------------------------------------

int r3dTexture::CreateVolume(int Width, int Height, int Depth, D3DFORMAT TargetTexFormat, int NumMipMaps, int SystemMem)
{
	r3dD3DTextureTunnel tex3D;

	r3dDeviceTunnel::CreateVolumeTexture
	(
		Width,
		Height,
		Depth,
		NumMipMaps,
		(r3dDefaultTexturePool == D3DPOOL_MANAGED) ? 0 : D3DUSAGE_DYNAMIC,
		TargetTexFormat,
		SystemMem ? D3DPOOL_SYSTEMMEM : r3dDefaultTexturePool,
		&tex3D
	);

	Setup( Width, Height, Depth, TargetTexFormat, NumMipMaps, &tex3D, false );

	RegisterCreated();

#if 0
	r3dOutToLog("r3dTexture: Texture [%dx%d] created\n", Width, Height);
#endif

	InterlockedExchange( &m_Loaded, 1 ) ;

	return 1;
}

//------------------------------------------------------------------------

int
r3dTexture::CreateCubemap( int EdgeLength, D3DFORMAT TargetTexFormat, int aNumMipMaps )
{
	r3dD3DTextureTunnel texTunnel ;

	r3dDeviceTunnel::CreateCubeTexture(
		EdgeLength,
		aNumMipMaps,
		(r3dDefaultTexturePool == D3DPOOL_MANAGED) ? 0 : D3DUSAGE_DYNAMIC,
		TargetTexFormat,
		r3dDefaultTexturePool,
		&texTunnel );

	SetupCubemap( EdgeLength, TargetTexFormat, aNumMipMaps, &texTunnel, false );
	
	NumMipMaps	= aNumMipMaps;

	RegisterCreatedCubemap();

	InterlockedExchange( &m_Loaded, 1 ) ;

	return 1;
}

//------------------------------------------------------------------------
int r3dTexture::Save(const char* Name, bool bFullMipChain/* = false*/)
{
	D3DXIMAGE_FILEFORMAT	Format = D3DXIFF_TGA;

	char drive[ 16 ], path[ 512 ], name[ 512 ], ext[ 128 ];

	_splitpath( Name, drive, path, name, ext );

	if ( !stricmp(ext,".BMP") ) Format = D3DXIFF_BMP;
	if ( !stricmp(ext,".DDS") ) Format = D3DXIFF_DDS; 

	HRESULT hres = 0 ;

	if ( bFullMipChain )
		hres = D3DXSaveTextureToFile ( Name, Format, AsTex2D(), NULL ) ;
	else
	{
		IDirect3DSurface9 	*L0 = NULL;
		AsTex2D()->GetSurfaceLevel(0, &L0);
		hres = D3DXSaveSurfaceToFile(Name, Format, L0, NULL, NULL) ;
		L0->Release();
	}

	if( hres != S_OK )
	{
		r3dOutToLog( "r3dTexture::Save: Couldn't save to : %s\n", Name ) ;
		return 0 ;
	}

	return 1 ;
}

// r3d:: helpers
void _InsertTexture(r3dTexture **FirstTexture, r3dTexture *Tex)
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;
	
	Tex->pPrev        = NULL;
	Tex->pNext        = *FirstTexture;
	if(Tex->pNext)
		Tex->pNext->pPrev = Tex;
	*FirstTexture     = Tex;
}

r3dTexture* _CreateTexture()
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;
	// ID in constructor is simple ++, if called from more than one thread at the same time will fuck up ID for textures
	return new r3dTexture;
}

void _DeleteTexture(r3dTexture* tex)
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	r3d_assert(tex);
	delete tex;
}


r3dTexture *r3dRenderLayer::AllocateTexture()
{
	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	r3dTexture	*Tex = _CreateTexture();

	_InsertTexture(&FirstTexture, Tex);

	return Tex;
}





r3dTexture* r3dRenderLayer::LoadTexture( const char* TexFile, D3DFORMAT TexFormat, bool bCheckFormat, int DownScale /*= 1*/, int DownScaleMinDim /*= 1*/, int SystemMem /*= 0*/, int gameResourcePool /*= 0*/ )
{
	if(!bInited)
		return NULL;

	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	assert(TexFile);

	char szFileName[ MAX_PATH ];
	FixFileName( TexFile, szFileName );

	r3dTexture*	Tex;

	// goes thru all textures and see if we already have with that name
	for(Tex = FirstTexture; Tex; Tex = Tex->pNext)
	{
		if((!bCheckFormat || (Tex->GetD3DFormat() == TexFormat) ) && (Tex->IsLoaded()) && (strcmp(Tex->getFileLoc().FileName, szFileName ) == NULL)) {
			Tex->Instances++;
			return Tex;
		}
	}

	Tex = _CreateTexture();

	if( gameResourcePool == PlayerTexMem )
	{
		Tex->MarkPlayerTexture() ;
	}

	if(!Tex->Load(szFileName, TexFormat, DownScale, DownScaleMinDim, SystemMem )) {
		_DeleteTexture(Tex);
		return NULL;
	}

	//r3dOutToLog("TEXTURE: LoadTexture %s complete\n", szFileName);

	// insert to LList.
	_InsertTexture(&FirstTexture, Tex);

	Tex->Instances++;
	return Tex;
}

//--------------------------------------------------------------------------------------------------------
void r3dRenderLayer::ReloadTextureData( const char * texFile )
{
	if(!bInited)
		return;

	assert( texFile );
	char szFileName[ MAX_PATH ];
	FixFileName( texFile, szFileName );


	for( r3dTexture * Tex = FirstTexture; Tex; Tex = Tex->pNext)
	{
		if( !Tex->IsLoaded() || !r3dIsSamePath(Tex->getFileLoc().FileName, szFileName) )
			continue;

		Tex->Unload();
		Tex->Load( szFileName, Tex->GetD3DFormat() );

		for( int i = 0, e = gTextureReloadListeners.Count() ; i < e ; i ++ )
		{
			gTextureReloadListeners[ i ]( Tex ) ;
		}
	}
}




void r3dRenderLayer::DeleteTexture(r3dTexture *Tex, int bForceDelete)
{
	if(!Tex)
		return;

	r3dCSHolderWithDeviceQueue csholder( g_ResourceCritSection ) ; (void)csholder ;

	//  if(!Tex->IsValid())
	//    return;


	//  if (!Tex->Tex) return;

	Tex->Instances--;
	if (bForceDelete) Tex->Instances = 0;
	if(Tex->Instances > 0)
		return;

	if (!bForceDelete)
	{
		if (Tex->bPersistent) 
		{ 
			Tex->Instances = 1; 
			return;
		}
	}

	if(Tex->pNext)
		Tex->pNext->pPrev = Tex->pPrev;
	if(Tex->pPrev)
		Tex->pPrev->pNext = Tex->pNext;
	if(Tex == FirstTexture)
		FirstTexture      = Tex->pNext;

	_DeleteTexture(Tex);
}

//------------------------------------------------------------------------
/*static*/

void
r3dTexture::Init()
{
	g_TextureLoadTaskParams.Init( 256 ) ;
}

//------------------------------------------------------------------------
/*static*/

void
r3dTexture::Close()
{
}

//------------------------------------------------------------------------

void AddTextureReloadListener( TextureReloadListener listener )
{
	gTextureReloadListeners.PushBack( listener ) ;
}

void RemoveTextureReloadListener( TextureReloadListener listener )
{
	for( int i = 0, e = gTextureReloadListeners.Count() ; i < e ;  )
	{
		if( gTextureReloadListeners[ i ] == listener )
		{
			gTextureReloadListeners.Erase( i ) ;
			e -- ;
		}
		else
		{
			i ++ ;
		}
	}
}

bool HasTextureReloadListener( TextureReloadListener listener )
{
	for( int i = 0, e = gTextureReloadListeners.Count() ; i < e ;  )
	{
		if( gTextureReloadListeners[ i ] == listener )
		{
			return true ;
		}
	}

	return false ;
}