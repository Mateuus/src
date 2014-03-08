#include "r3dPCH.h"
#include "r3d.h"

#include "ObjectsCode/world/EnvmapProbes.h"
#include "../../RENDERING/Deffered/HUDFilters.h"

extern	r3dScreenBuffer*	ScreenBuffer;
extern	r3dCamera			gCam;
extern	float 				__r3dGlobalAspect;
extern	bool				g_bEditMode;
extern	int					g_RenderRadialBlur;

void GameRender();
void UpdateZPrepassSettings();
void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);

void GenerateEnvmap( r3dTexture* tex, const r3dString& texName, const r3dPoint3D& pos )
{
#ifdef FINAL_BUILD
	return;
#else
	R3D_ENSURE_MAIN_THREAD();

	r3d_assert( tex->isCubemap() );

	IDirect3DCubeTexture9* cube;

	// tell' em all not to draw themselves
	struct ClearRestoreRenderSettings
	{
		ClearRestoreRenderSettings()
		{
			prevEditorInitted			= g_bEditMode;
			prevColorCorrection			= gHUDFilterSettings[HUDFilter_Default].enableColorCorrection;
			prevRadialBlur				= g_RenderRadialBlur;
			prevHideIcons				= r_hide_icons->GetInt();
			prevOcclusionQueryEnabled	= r_use_oq->GetInt();
			prevZPrepass				= r_z_allow_prepass->GetInt();
			prevDeviceClears			= r_device_clear->GetInt();

			savedCam					= gCam;

			g_bEditMode					= false;
			g_RenderRadialBlur			= false;
			gHUDFilterSettings[HUDFilter_Default].enableColorCorrection = 0;
			r_hide_icons->SetInt(1);
			r_use_oq->SetInt(0);
			r_z_allow_prepass->SetInt(0);
			r_device_clear->SetInt(0);

			UpdateZPrepassSettings();
			g_EnvmapProbes.SetForceGlobal( true );
		}

		~ClearRestoreRenderSettings()
		{
			g_bEditMode				= prevEditorInitted;
			g_RenderRadialBlur		= prevRadialBlur;
			gHUDFilterSettings[HUDFilter_Default].enableColorCorrection	= prevColorCorrection;
			r_hide_icons->SetInt(prevHideIcons);
			r_use_oq->SetInt(prevOcclusionQueryEnabled);
			r_z_allow_prepass->SetInt(prevZPrepass);
			r_device_clear->SetInt( prevDeviceClears );

			gCam					= savedCam;

			UpdateZPrepassSettings();
			g_EnvmapProbes.SetForceGlobal( false );
		}

		bool prevEditorInitted;
		int prevColorCorrection;
		int prevRadialBlur;
		int prevOcclusionQueryEnabled;
		int prevHideIcons;
		int prevZPrepass;
		int prevDeviceClears;

		r3dCamera savedCam;

	} clearRestoreRenderSettings; (void)clearRestoreRenderSettings;

	D3DPERF_BeginEvent(D3DCOLOR_XRGB(255,255,255), L"CUBE MAP RENDER");

	r3d_assert( tex->GetWidth() == tex->GetHeight() );

	D3D_V( r3dRenderer->pd3ddev->CreateCubeTexture(	tex->GetWidth(),	1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &cube, NULL ) );
	SetD3DResourcePrivateData(tex->GetD3DTexture(), "GenerateEnvmap: Color");

	IDirect3DSurface9* cubeSizedRT( NULL );
	

	r3dPoint3D dirs[] = {	r3dPoint3D( +1, +0, +0 ), 
							r3dPoint3D( -1, +0, +0 ),
							r3dPoint3D( +0, +1, +0 ),
							r3dPoint3D( +0, -1, +0 ),
							r3dPoint3D( +0, +0, +1 ),
							r3dPoint3D( +0, +0, -1 ) };

	r3dPoint3D ups[] = {	r3dPoint3D( +0, +1, +0 ), 
							r3dPoint3D( +0, +1, +0 ),
							r3dPoint3D( +0, +0, -1 ),
							r3dPoint3D( +0, +0, +1 ),
							r3dPoint3D( +0, +1, +0 ),
							r3dPoint3D( +0, +1, +0 ) };

	gCam			= pos;
	gCam.FOV		= 90;
	gCam.Aspect		= 1.0f;

	for( int i = 0, e = sizeof dirs / sizeof dirs[ 0 ]; i < e; i ++ )
	{
		gCam.vPointTo	= dirs[ i ];
		gCam.vUP		= ups[ i ];

		r3dRenderer->SetCamera( gCam );

		GameRender();

		D3D_V( cube->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &cubeSizedRT ) );

		g_pPostFXChief->AddFX( gPFX_ConvertToLDR ) ;

		g_pPostFXChief->Execute( false, true ) ;

		IDirect3DSurface9 *srcSurf( NULL ), *destSurf( NULL );
		D3D_V( g_pPostFXChief->GetBuffer( PostFXChief::RTT_PINGPONG_NEXT )->Tex->AsTex2D()->GetSurfaceLevel( 0, &srcSurf ) );

#if 0
		D3D_V( D3DXSaveTextureToFile( (std::string("cubeface") + char('0' + i ) + ".dds").c_str(), D3DXIFF_DDS, ScreenBuffer->Tex->AsTex2D(), NULL ) );
#endif

		D3D_V( r3dRenderer->pd3ddev->StretchRect( srcSurf, NULL, cubeSizedRT, NULL, D3DTEXF_LINEAR ) );

		srcSurf->Release();
		cubeSizedRT->Release();
	}

	LPD3DXBUFFER derBuffer( NULL );

	D3D_V( D3DXSaveTextureToFileInMemory( &derBuffer, D3DXIFF_DDS, cube, NULL ) );

	cube->Release();

	IDirect3DCubeTexture9* convertedTex( NULL );

	// generate mips
	D3D_V( D3DXCreateCubeTextureFromFileInMemoryEx(	r3dRenderer->pd3ddev, 
													derBuffer->GetBufferPointer(), derBuffer->GetBufferSize(),
													tex->GetWidth(), 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, 
													D3DX_FILTER_LINEAR, D3DX_FILTER_BOX, 0, NULL, NULL, &convertedTex ) );

	int mips = 0 ;

	int w = tex->GetWidth() ;

	for( ; w ; w /= 2 )
	{
		mips ++ ;	
	}

	for( int i = 1, e = mips ; i < e ; i ++ )
	{
		D3DLOCKED_RECT lrects[ 6 ] ;

		D3DSURFACE_DESC desc ;
		convertedTex->GetLevelDesc( i, &desc ) ;

		int dim = desc.Width ;
		
		for( int f = 0 ; f < 6 ; f ++ )
		{
			D3D_V( convertedTex->LockRect( D3DCUBEMAP_FACES( f ), i, &lrects[ f ], NULL, 0 ) ) ;
		}

		struct Avarage
		{
			R3D_FORCEINLINE int operator() ( int v0, int v1 )
			{
				union RGBA
				{
					unsigned char rgba[ 4 ] ;
					unsigned val ;
				} a, b ;

				a.val = (unsigned)v0 ;
				b.val = (unsigned)v1 ;

				for( int i = 0 ; i < 4 ; i ++ )
				{
					a.rgba[ i ] = ( b.rgba[ i ] + a.rgba[ i ] ) / 2 ;
				}

				return a.val ;
			}

			R3D_FORCEINLINE int operator() ( int v0, int v1, int v2 )
			{
				union RGBA
				{
					unsigned char rgba[ 4 ] ;
					unsigned val ;
				} a, b, c ;

				a.val = (unsigned)v0 ;
				b.val = (unsigned)v1 ;
				c.val = (unsigned)v2 ;

				for( int i = 0 ; i < 4 ; i ++ )
				{
					a.rgba[ i ] = ( a.rgba[ i ] + b.rgba[ i ] + c.rgba[ i ] ) / 3 ;
				}

				return a.val ;
			}

		};

		struct
		{
			void operator () ( void* vp0, void* vp1, int step0, int step1, int count )
			{
				int *p0 = (int*)vp0 ;
				int *p1 = (int*)vp1 ;

				Avarage avg ;
				for( int i = 0, e = count ; i < e; i ++ )
				{
					int a = avg( *p0, *p1 ) ;
					*p0 = a ;
					*p1 = a ;

					p0 += step0 ;
					p1 += step1 ;
				}
			}

			void operator () ( void* vp0, void* vp1, void* vp2 )
			{
				int *p0 = (int*)vp0 ;
				int *p1 = (int*)vp1 ;
				int *p2 = (int*)vp2 ;

				Avarage avg ;
		
				int a = avg( *p0, *p1, *p2 ) ;

				*p0 = a ;
				*p1 = a ;
				*p2 = a ;
			}
		} fixup ; fixup ;

#if 0
		struct
		{
			void operator () ( void* vp0, void* vp1, int step0, int step1, int count, int col )
			{
				int *p0 = (int*)vp0 ;
				int *p1 = (int*)vp1 ;

				for( int i = 0, e = count ; i < e; i ++ )
				{
					*p0 = col * i / ( count - 1 ) ;
					*p1 = col * i / ( count - 1 ) ;

					p0 += step0 ;
					p1 += step1 ;
				}
			}

			void operator () ( void* vp0, void* vp1, void* vp2, int col )
			{
				int *p0 = (int*)vp0 ; *p0 = col ;
				int *p1 = (int*)vp1 ; *p1 = col ;
				int *p2 = (int*)vp2 ; *p2 = col ;
			}
		} show ; show ;
#endif
		
		if( dim > 1 )
		{
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + 2 * dim - 1, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim, dim, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + 2 * dim - 1, dim, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + dim, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + 2 * dim - 1, dim, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + 2 * dim - 1, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + dim, dim, dim, dim - 2 ) ;

			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim - 2, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + 2 * dim - 1, -1, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + 1, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + dim, 1, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + 1, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + (dim - 1) * dim + 1, 1, 1, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim - 2, (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + 1, -1, 1, dim - 2 ) ;

			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim * ( dim - 1 ) + 1, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + 2 * dim - 1, 1, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + dim * ( dim - 1 ) + dim - 2, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + dim, -1, dim, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + dim * ( dim - 1 ) + 1, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + 1, 1, 1, dim - 2 ) ;
			fixup( (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim * ( dim - 1 ) + dim - 2, (int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + (dim - 1) * dim + 1, -1, 1, dim - 2 ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim * ( dim - 1 ), 
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + dim - 1,
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + dim - 1 + dim * ( dim - 1 ) ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + dim - 1 + dim * ( dim - 1 ), 
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits,
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + dim * ( dim - 1 ) ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + dim * ( dim - 1 ), 
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + dim * ( dim - 1 ),
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim - 1 + dim * ( dim - 1 ) ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim - 1 + dim * ( dim - 1 ), 
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Y ].pBits + dim * ( dim - 1 ) + dim - 1,				
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim * ( dim - 1 ) ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits, 
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + dim * ( dim - 1 ) + dim - 1,
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits + dim - 1 ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits + dim - 1, 
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + dim * ( dim - 1 ),
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Z ].pBits ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_X ].pBits, 
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits,
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits + dim - 1 ) ;

			fixup(	(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_X ].pBits + dim - 1, 
					(int*)lrects[ D3DCUBEMAP_FACE_POSITIVE_Y ].pBits + dim - 1,
					(int*)lrects[ D3DCUBEMAP_FACE_NEGATIVE_Z ].pBits ) ;
		}
		else
		{
			// Lowest mip. Have to make same color for edgeless rendering on "non seamless cubemap" cards
			// find max to make everything "skylike"

			struct
			{
				void operator() ( D3DLOCKED_RECT* lrect )
				{
					union RGBA
					{
						unsigned char rgba[ 4 ] ;
						unsigned val ;
					} a ;

					a.val = *(int*)lrect->pBits ;

					int intensity = a.rgba[ 0 ] + a.rgba[ 1 ] + a.rgba[ 2 ] + a.rgba[ 2 ] ;

					if( intensity > maxIntens )
					{
						maxIntens = intensity ;
						maxVal = a.val ;
					}

				}

				int maxVal ;
				int maxIntens ;

			} maxColor = { 0, 0  } ;

			for( int f = 0 ; f < 6 ; f ++ )
			{
				maxColor( lrects + f ) ;
			}

			for( int f = 0 ; f < 6 ; f ++ )
			{
				*(int*)lrects[ f ].pBits = maxColor.maxVal ;
			}
		}

		for( int f = 0 ; f < 6 ; f ++ )
		{
			D3D_V( convertedTex->UnlockRect( D3DCUBEMAP_FACES( f ), i ) ) ;
		}
	}

	derBuffer->Release();

	D3D_V( D3DXSaveTextureToFileInMemory( &derBuffer, D3DXIFF_DDS, convertedTex, NULL ) );

	convertedTex->Release();

	D3D_V( D3DXCreateCubeTextureFromFileInMemoryEx(	r3dRenderer->pd3ddev, 
													derBuffer->GetBufferPointer(), derBuffer->GetBufferSize(),
													tex->GetWidth(), 0, 0, D3DFMT_R5G6B5, D3DPOOL_MANAGED, 
													D3DTEXF_LINEAR, D3DTEXF_LINEAR, 0, NULL, NULL, &convertedTex ) );	

	D3D_V( D3DXSaveTextureToFile( texName.c_str(), D3DXIFF_DDS, convertedTex, NULL ) );
	tex->SetNewD3DTexture(convertedTex);

	derBuffer->Release();

	D3DPERF_EndEvent();
#endif
}
