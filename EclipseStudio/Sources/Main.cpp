#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"
#include "shellapi.h"
#include "resource.h"

#include "Particle.h"

#pragma warning (disable: 4244)
#pragma warning (disable: 4305)
#pragma warning (disable: 4101)

#include "cvar.h"
#include "fmod/soundsys.h"

#include "APIScaleformGFX.h"
#include "GameCommon.h"
#include "GameLevel.h"

#include "ObjectsCode/world/EnvmapProbes.h"
#include "ObjectsCode/world/DecalChief.h"
#include "ObjectsCode/world/MaterialTypes.h"
#include "ObjectsCode/world/WaterPlane.h"

#include "ObjectsCode/Nature/wind.h"

#include "../SF/CmdProcessor/CmdProcessor.h"
#include "../SF/Console/Config.h"
#include "../SF/Console/EngineConsole.h"
#include "../SF/Version.h"

#include "Rendering/Deffered/CommonPostFX.h"

#include "Menus\m_AppSelect.h"
#include "Menus\m_Main.h"

#include "UI\m_LoadingScreen.h"
#include "UI\m_Login.h"
#include "UI\FrontendShared.h"
#include "UI\m_EndRound.h"

#include "UI/HUDCameraEffects.h"

#include "Editors/ObjectManipulator3d.h"
#include "Editors/LevelEditor_Collections.h"

#include "RENDERING\Deffered\VisibilityGrid.h"
#include "rendering\Probes\ProbeMaster.h"

#include "ObjectsCode/weapons/WeaponArmory.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"

#include "DamageLib.h"

#include "JobChief.h"
#include "r3dBackgroundTaskDispatcher.h"
#include "Rendering/Deffered/RenderDeferredPointLightsOptimized.h"

#include "LangMngr.h"

#include "HWInfo.h"
#include "SteamHelper.h"

#include "ObjectsCode/Nature/GrassLib.h"

#include "ObjectsCode/WEAPONS/FlashbangVisualController.h"
#include "../../Eternity/Source/r3dEternityWebBrowser.h"

#include "r3dDeviceQueue.h"

#include "GameCode\UserRewards.h"

extern bool g_bEditMode;
extern bool g_bStartedAsParticleEditor;

#include "Gameplay_Params.h"
	const CGamePlayParams* GPP = new CGamePlayParams();

CD3DFont* 	Font_Label;
CD3DFont* 	Font_Editor;

extern void 	PlayEditor();

extern char	LevelEditName[64];
extern char initialCameraSpotName[64];
extern HANDLE	r3d_CurrentProcess;
extern void r3dFreeGOBMeshes();
extern void AI_Player_FreeStuff();
extern void UserProfile_SetRankPointsRUS();
#if APEX_ENABLED
void DestroyApexUserRenderer();
#endif

void r3dInitShaders();

void SaveSettingsCallback( int oldI, float oldF )
{
	void writeGameOptionsFile() ;
	writeGameOptionsFile() ;
}

void CursorModeCallback( int oldI, float oldF )
{
	if( !oldI )
	{
		r3dMouse::Show( true ) ;
	}
}

void InitRender(int bUseSet = 0)
{
	r_out_of_vmem_encountered->SetChangeCallback( &SaveSettingsCallback ) ;

	if (!bUseSet)
	{
		r_width->SetInt( 1024 );
		r_height->SetInt( 768 );
		r_bpp->SetInt( 32 );
		r_fullscreen->SetBool( false ); 
	}

	int Flags = 0;

	if ( !r_fullscreen->GetBool() ) 
		Flags |= R3DSetMode_Windowed;

	MoveWindow(win::hWnd, 0, 0, r_width->GetInt(), r_height->GetInt(), 0);

	r3dRenderer = new r3dRenderLayer;

	r3dRenderer->Init(win::hWnd, NULL);

	if( ( r_local_vmem_size->GetInt() <= 256 * 1024 * 1024 && !r_ini_read->GetInt() )
			||
		r_out_of_vmem_encountered->GetInt()
		) 
	{
		r3dOutToLog( "Setting low memory requirement options because we have only %d memory\n", r_local_vmem_size->GetInt() / 1024 / 1024 ) ;

		r_out_of_vmem_encountered->SetInt( 0 ) ;

		r_width->SetInt( 800 ) ;
		r_height->SetInt( 600 ) ;

		r_fullscreen->SetInt( 0 ) ;

		r_texture_quality->SetInt( 1 ) ;

		void applyGraphicOptionsSoft( uint32_t ) ;
		applyGraphicOptionsSoft( FrontEndShared::SC_TEXTURE_QUALITY ) ;

		void writeGameOptionsFile();
		writeGameOptionsFile();
	}

	r3dOutToLog("Setting mode:  %dx%dx%d Flags=%d\n", r_width->GetInt(), r_height->GetInt(), r_bpp->GetInt(), Flags);

	r3dRenderer->InitStereo() ;

	if( !r3dRenderer->SetMode( r_width->GetInt(), r_height->GetInt(), r_bpp->GetInt(), Flags, 0 /*R3D_PATH_DX9*/) )
	{
		bool failed = true ;
		if( ! ( Flags & R3DSetMode_Windowed ) )
		{
			r3dOutToLog("SetMode failed, trying to set windowed flag and trying again\n");
			Flags |= R3DSetMode_Windowed;
			if( r3dRenderer->SetMode( r_width->GetInt(), r_height->GetInt(), r_bpp->GetInt(), Flags, 0 /*R3D_PATH_DX9*/) )
			{
				failed = false ;
			}
		}

		if( failed )
		{
			r3dError("Failed to init D3D Device!\n");
			r3dRenderer->Close();
			exit( 0 );
		}
	}

	ShowWindow(win::hWnd, TRUE);

	r3dInitShaders();

	r3dInitMaterials();

	r3d_assert(g_pJobChief == 0);
	g_pJobChief = new JobChief();
	g_pJobChief->Init();

	g_pBackgroundTaskDispatcher = new r3dBackgroundTaskDispatcher();
	g_pBackgroundTaskDispatcher->Init() ;

	g_EnvmapProbes.Init();
	r3d_assert(g_pDecalChief == 0);
	g_pDecalChief = new DecalChief();
	g_pDecalChief->Init();
	// should follow g_DecalChief
	r3d_assert(g_pMaterialTypes == 0);
	g_pMaterialTypes = new MaterialTypes();
	g_pMaterialTypes->Load();

#if R3D_ALLOW_LIGHT_PROBES
	g_pProbeMaster = new ProbeMaster ;
	g_pProbeMaster->Init() ;
#endif

	r3dRenderer->StartRender();
	r3dRenderer->EndRender( true );

	r3dUtilInit();
	InitOcclusionQuerySystem();

	gFlashbangVisualController.Init();

#ifndef FINAL_BUILD
	InitObjCategories();
#endif

	if ( d_mouse_window_lock->GetBool() )
	{
		Mouse->MoveTo((int)r3dRenderer->ScreenW2, (int)r3dRenderer->ScreenH2);
	}
	Mouse->SetCapture();

	{
		r3dIntegrityGuardian ig ;

		Font_Label = new CD3DFont( ig, "Tahoma", 12, D3DFONT_BOLD | D3DFONT_FILTERED | D3DFONT_SKIPGLYPH );
		Font_Label->CreateSystemFont();
	}

	{
		r3dIntegrityGuardian ig ;

		Font_Editor = new CD3DFont(ig, "Verdana", 10, D3DFONT_BOLD | D3DFONT_FILTERED | D3DFONT_SKIPGLYPH);
		Font_Editor->CreateSystemFont();
	}

	r3dScaleformGfxCreate();
#if ENABLE_WEB_BROWSER
	g_pBrowserManager = new EternityWebBrowser();
#endif
}


void CloseRender()
{
	delete Font_Label;
	delete Font_Editor; 

#if R3D_ALLOW_LIGHT_PROBES
	if( g_pProbeMaster )
		g_pProbeMaster->Close() ;

	SAFE_DELETE( g_pProbeMaster ) ;
#endif

#if ENABLE_WEB_BROWSER
	SAFE_DELETE(g_pBrowserManager);
#endif

#ifndef FINAL_BUILD
	CloseCategories();
#endif

	CloseOcclusionQuerySystem();
	r3dUtilClose();

	SAFE_DELETE(g_pMaterialTypes);
	g_pDecalChief->Close();
	SAFE_DELETE(g_pDecalChief);
	g_EnvmapProbes.Close();

	gFlashbangVisualController.Destroy();

	g_pJobChief->Close();
	SAFE_DELETE(g_pJobChief);

	g_pBackgroundTaskDispatcher->Close();
	SAFE_DELETE(g_pBackgroundTaskDispatcher);

#if APEX_ENABLED
	DestroyApexUserRenderer();
#endif

	r3dScaleformGfxDestroy();

	r3dCloseMaterials();

	r3dRenderer->Close(); 

	SAFE_DELETE(r3dRenderer); 
}


#ifdef FINAL_BUILD
const char * g_szApplicationName = "War Brasil";
#else
const char * g_szApplicationName = "Eclipse Studio";
#endif 

int32_t	g_nProjectVersionMajor = 1;
int32_t	g_nProjectVersionMinor = 0;

	int	RUS_CLIENT = 0;
	int	MASSIVE_CLIENT = 0;

extern	char	Login_PassedUser[256];
extern	char	Login_PassedPwd[256];
extern	char	Login_PassedAuth[256];
extern	char	Login_GNA_userid[256];
extern	char	Login_GNA_appkey[256];
extern	char	Login_GNA_token[256];

static	char*	gSurveyOutLink = NULL;

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
		case '\"':
			in_QM = TRUE;
			in_TEXT = TRUE;
			if(in_SPACE) {
				argv[argc] = _argv+j;
				argc++;
			}
			in_SPACE = FALSE;
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			if(in_TEXT) {
				_argv[j] = '\0';
				j++;
			}
			in_TEXT = FALSE;
			in_SPACE = TRUE;
			break;
		default:
			in_TEXT = TRUE;
			if(in_SPACE) {
				argv[argc] = _argv+j;
				argc++;
			}
			_argv[j] = a;
			j++;
			in_SPACE = FALSE;
			break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

CHWInfo g_HardwareInfo;

// This function called by engine before main app window created, before any IO initialized. 
void game::PreInit()
{
	//@FOR STEAM- SetCurrentDirectory("Z:\\tsg\\WarOnline\\bin\\");

	u_srand(GetTickCount());

	g_HardwareInfo.Grab();

	win::hWinIcon  = NULL;
	win::szWinName = GetBuildVersionString();

#ifdef FINAL_BUILD
	win::hWinIcon = ::LoadIcon(win::hInstance, MAKEINTRESOURCE(IDI_WARINC));
	if(strstr(__r3dCmdLine, "-WOUpdatedOk") == NULL && strstr(__r3dCmdLine, "-gna") == NULL)
	{
		MessageBox(NULL, "Please run game launcher.", g_szApplicationName, MB_OK);
		ExitProcess(0);
	}
#endif	

#ifdef _DEBUG
	r3dOutToLog("cmd: %s\n", __r3dCmdLine);
#endif	
	
	// parse command line
	int argc = 0;
	char** argv = CommandLineToArgvA(__r3dCmdLine, &argc);
	for(int i=0; i<argc; i++) 
	{
		if(strcmp(argv[i], "-login") == 0 && (i + 1) < argc)
		{
			r3dscpy(Login_PassedUser, argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "-pwd") == 0 && (i + 1) < argc)
		{
			r3dscpy(Login_PassedPwd, argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "-WOLogin") == 0 && (i + 1) < argc)
		{
			r3dscpy(Login_PassedAuth, argv[++i]);
			continue;
		}
		
		if(strcmp(argv[i], "-steam") == 0)
		{
			gSteam.IS_ENABLED = true;
			continue;
		}

		if(strcmp(argv[i], "-survey") == 0 && (i + 1) < argc)
		{
			gSurveyOutLink = argv[++i];
			continue;
		}

		if(strcmp(argv[i], "-massive") == 0)
		{
			MASSIVE_CLIENT = 1;
			continue;
		}

		if(strcmp(argv[i], "-gna") == 0)
		{
			if(!((i + 3) < argc))
				r3dError("too few arguments for gamenet");

			r3dscpy(Login_GNA_userid, argv[++i]);
			r3dscpy(Login_GNA_appkey, argv[++i]);
			r3dscpy(Login_GNA_token, argv[++i]);

			RUS_CLIENT = 1;
			r3dOutToLog("gamenet client enabled\n");
			
#ifdef FINAL_BUILD
			g_szApplicationName = "FireStorm";
			win::szWinName = GetBuildVersionString();
			win::hWinIcon = ::LoadIcon(win::hInstance, MAKEINTRESOURCE(IDI_FIRESTORM));
#endif			
			
			UserProfile_SetRankPointsRUS();
			continue;
		}

#ifndef FINAL_BUILD
#if !DISABLE_PROFILER
		if (strcmp(argv[i], "-profile") == 0)
		{
			if (i + 1 >= argc)
				r3dError("Incorrect syntax for '-profile' option. Use '-profile level_name'");
			strcpy_s(LevelEditName, _countof(LevelEditName), argv[i + 1]);
			//	Set scheduled profile time to -1 to adjust it automatically after level will be loaded
			gScheduledProfileTime = -1.0f;
		}

		if (strcmp(argv[i], "-gprofile") == 0)
		{
			if (i + 1 >= argc)
				r3dError("Incorrect syntax for '-gprofile' option. Use '-gprofile level_name'");

			strcpy_s(LevelEditName, _countof(LevelEditName), argv[i + 1]);

			gProfileD3DFromCommandLine = true;
			gScheduledProfileTime = -1.0f;
		}
#endif

		if (strcmp(argv[i], "-camera") == 0)
		{
			if (i + 1 >= argc)
				r3dError("Incorrect syntax for '-camera' option. Use '-camera camera_spot_name'");

			strcpy_s(initialCameraSpotName, _countof(initialCameraSpotName), argv[i + 1]);
		}
#endif
	}
}


static void InitSounds()
{
	snd_InitSoundSystem();
	snd_LoadSoundEffects("Data\\Sounds", "Sounds.fev");
}

void ReloadMesh(const char* fname);


#ifndef FINAL_BUILD
//--------------------------------------------------------------------------------------------------------
void CallbackFileChange( const char * szFileName )
{
	char buffer[ MAX_PATH ];
	GetCurrentDirectory( sizeof( buffer ), buffer );
	uint32_t dwSize = strlen( buffer );
	if ( 0 != strncmp( buffer, szFileName, dwSize ) )
		return;

	const char * szName = szFileName + dwSize + 1;

	FixedString s( szName );
	FixedString sExt = s.GetExt();
	strlwr(sExt.str());
	if ( sExt == ".dds" || sExt == ".tga" || sExt == ".bmp" )
	{
		r3dRenderer->ReloadTextureData( szName );
		return;
	}	

	if ( sExt == ".hls")
	{
		r3dRenderer->ReloadShaderByFileName(szName);
		return;
	}

	if ( sExt == ".sco")
	{
		ReloadMesh( szName );
		return;
	}

	if ( sExt == ".anm")
	{
		extern r3dAnimPool* g_CharactersAnimationsPool;
		g_CharactersAnimationsPool->Reload(szName);
		return;
	}

	if( g_pMaterialTypes->CheckNeedReload( szName ) )
	{
		if( !g_pMaterialTypes->Load() )
		{
			MessageBox( NULL, "Error reloading material types! Please, check your XML syntax!", "Error", MB_ICONEXCLAMATION );
		}
	}
}
#endif

#define INI_FILE "gameSettings.ini"
#define INPUT_MAP_FILE "inputMap.xml"

void applyGraphicOptionsSoft( uint32_t settingsFlags )
{
	extern float __WorldRenderBias;

	switch( r_anisotropy_quality->GetInt() )
	{
	case 1:
		r_anisotropy->SetInt( 1 );
		//__WorldRenderBias = -1.45f;
		break;
	case 2:
		//__WorldRenderBias = -0.45f;
		r_anisotropy->SetInt( 2 );
		break;
	case 3:
		r_anisotropy->SetInt( 4 );
		//__WorldRenderBias = 0.f;
		break;
	case 4:
		r_anisotropy->SetInt( 8 );
		//__WorldRenderBias = 0.f;
		break;
	}

	switch( r_ssao_quality->GetInt() )
	{
	case 1:
		r_ssao->SetInt( 0 );
		break;
	case 2:
		r_ssao->SetInt( 1 );
		r_ssao_method->SetInt( 1 );
		r_half_scale_ssao->SetInt( 1 );
		r_ssao_blur_w_normals->SetInt( 0 );
		break;
	case 3:
		r_ssao->SetInt( 1 );
		r_ssao_method->SetInt( 2 );
		r_half_scale_ssao->SetInt( 1 );
		r_ssao_blur_w_normals->SetInt( 0 );
		break;
	case 4:
		r_ssao->SetInt( 1 );
		r_ssao_method->SetInt( 1 );
		r_half_scale_ssao->SetInt( 0 );
		r_ssao_blur_w_normals->SetInt( 0 );
		break;
	case 5:
		r_ssao->SetInt( 1 );
		r_ssao_method->SetInt( 2 );
		r_half_scale_ssao->SetInt( 0 );
		r_ssao_blur_w_normals->SetInt( 1 );
		break;
	};

	switch( r_antialiasing_quality->GetInt() )
	{
	case 1:
		r_fxaa->SetInt( 0 );
		break;
	case 2:
		r_fxaa->SetInt( 1 );
		break;
	case 3:
		r_fxaa->SetInt( 1 );
		break;
	case 4:
		r_fxaa->SetInt( 1 );
		break;
	}

	switch( r_postprocess_quality->GetInt() )
	{
	case 1:
		r_dof			->SetInt( 0 );
		r_film_grain	->SetInt( 0 );
		r_bloom			->SetInt( 0 );
		r_glow			->SetInt( 0 );
		r_sun_rays		->SetInt( 0 );
		break;
	case 2:
		r_dof			->SetInt( 1 );
		r_film_grain	->SetInt( 0 );
		r_bloom			->SetInt( 1 );
		r_glow			->SetInt( 1 );
		r_sun_rays		->SetInt( 0 );
		break;
	case 3:
		r_dof			->SetInt( 1 );
		r_film_grain	->SetInt( 1 );
		r_bloom			->SetInt( 1 );
		r_glow			->SetInt( 1 );
		r_sun_rays		->SetInt( 1 );
		break;
	}

	if( settingsFlags & FrontEndShared::SC_SHADOWS_QUALITY )
	{

		const int MAX_DIR_TEX_SIZE = r_max_texture_dim->GetInt() ? R3D_MIN( r_max_texture_dim->GetInt(), 2048 ) : 2048 ;

		switch( r_shadows_quality->GetInt() )
		{
		case 1:
			r_transp_shadows->SetInt( 0 ) ;
			r_terra_shadows->SetInt( 0 );
			r_shadow_blur->SetInt( 0 );
			r_dir_sm_size->SetInt( MAX_DIR_TEX_SIZE );
			r_shared_sm_size->SetInt( 1024 );
			r_shared_sm_cube_size->SetInt( 512 );
			r_active_shadow_slices->SetInt( NumShadowSlices - 2 );
			r3d_assert( r_active_shadow_slices->GetInt() ) ;
			r_shadows->SetInt( 1 );
			ShadowSplitDistancesOpaque = &ShadowSplitDistancesOpaqueLow[0];
			break;

		case 2:
			r_transp_shadows->SetInt( 0 ) ;
			r_terra_shadows->SetInt( 1 );
			r_shadow_blur->SetInt( 0 );
			r_dir_sm_size->SetInt( MAX_DIR_TEX_SIZE );
			r_shared_sm_size->SetInt( 1024 );
			r_shared_sm_cube_size->SetInt( 512 );
			r_active_shadow_slices->SetInt( NumShadowSlices -1);
			r_shadows->SetInt( 1 );
			ShadowSplitDistancesOpaque = &ShadowSplitDistancesOpaqueMed[0];
			break;

		case 3:
			r_terra_shadows->SetInt( 1 );
			r_shadow_blur->SetInt( 0 );
			r_dir_sm_size->SetInt( MAX_DIR_TEX_SIZE );
			r_shared_sm_size->SetInt( 1024 );
			r_shared_sm_cube_size->SetInt( 512 );
			r_active_shadow_slices->SetInt( NumShadowSlices );
			r_shadows->SetInt( 1 );
			ShadowSplitDistancesOpaque = &ShadowSplitDistancesOpaqueHigh[0];
			break;

		case 4:
			r_terra_shadows->SetInt( 1 );
			r_shadow_blur->SetInt( 1 );
			r_dir_sm_size->SetInt( MAX_DIR_TEX_SIZE );
			r_shared_sm_size->SetInt( 1024 );
			r_shared_sm_cube_size->SetInt( 1024 );
			r_active_shadow_slices->SetInt( NumShadowSlices );
			r_shadows->SetInt( 1 );
			ShadowSplitDistancesOpaque = &ShadowSplitDistancesOpaqueHigh[0];
			break;		
		}
	}

	if( settingsFlags & FrontEndShared::SC_PARTICLES_QUALITY )
	{
		switch( r_particles_quality->GetInt() )
		{
		case 1:
			r_distort->SetInt( 0 );
			r_half_res_particles->SetInt( 1 ) ;
			r_particle_shadows->SetInt( 0 ) ;
			break ;

		case 2:
			r_distort->SetInt( 1 );
			r_half_res_particles->SetInt( 1 ) ;
			r_particle_shadows->SetInt( 1 ) ;
			break ;

		case 3:
			r_distort->SetInt( 1 );
			r_half_res_particles->SetInt( 1 ) ;
			r_particle_shadows->SetInt( 1 ) ;
			break ;

		case 4:
			r_distort->SetInt( 1 );
			r_half_res_particles->SetInt( 0 ) ;
			r_particle_shadows->SetInt( 1 ) ;
			break ;
		};
	}

	if( settingsFlags & FrontEndShared::SC_DECORATIONS_QUALITY )
	{
		switch( r_decoration_quality->GetInt() )
		{
		case 1:
			r_grass_view_coef->SetFloat( 0.125f );
			r_grass_draw->SetBool( 0 );
			r_grass_skip_step->SetInt( 1 ) ;
			break ;

		case 2:
			// view coef now stays the same, but density of grass
			// gets less 
			r_grass_view_coef->SetFloat( 1.0f );
			r_grass_draw->SetBool( 1 );
			r_grass_skip_step->SetInt( 1 ) ;
			break ;

		case 3:
			r_grass_view_coef->SetFloat( 1.0f );
			r_grass_draw->SetBool( 1 );
			r_grass_skip_step->SetInt( 0 ) ;
			break ;
		};
	}

	if( !r_half_scale_ssao->GetInt() && r_ssao->GetInt() && r_grass_draw->GetInt() )
	{
		r_split_grass_render->SetInt( 1 ) ;
	}
	else
	{
		r_split_grass_render->SetInt( 0 ) ;
	}

	void SyncLightingAndSSAO();
	SyncLightingAndSSAO();
}

void applyGraphicsOptions( uint32_t settingsFlags )
{

	applyGraphicOptionsSoft( settingsFlags );

	struct PushPopBackGroundLoading
	{
		PushPopBackGroundLoading()
		{
			prevVal = g_async_loading->GetInt() ;
			r3dSetAsyncLoading( 0 ) ;
		}

		~PushPopBackGroundLoading()
		{
			r3dSetAsyncLoading( prevVal ) ;
		}

		int prevVal ;
	} pushPopBackGroundLoading ; (void)pushPopBackGroundLoading ;

	if( settingsFlags & FrontEndShared::SC_WATER_QUALITY )
	{
		for( GameObject *obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
		{
			if( obj->Class->Name == "obj_Lake" )
			{
				// recreates it with new dimmensions
				static_cast<WaterBase*>( static_cast<obj_Lake*>(obj) )->CreateRefractionBuffer();
			}
			else
			if( obj->Class->Name == "obj_WaterPlane" )
			{
				// recreates it with new dimmensions
				static_cast<WaterBase*>( static_cast<obj_WaterPlane*>(obj) )->CreateRefractionBuffer();
			}
		}
	}

	if( settingsFlags & FrontEndShared::SC_PARTICLES_QUALITY )
	{
		if( g_pDecalChief )
		{
			g_pDecalChief->UpdateTexturesForQualitySettings();
		}
	}

	if( settingsFlags & FrontEndShared::SC_DECORATIONS_QUALITY )
	{
		if( g_pGrassLib )
		{
			g_pGrassLib->Unload() ;
		}

		if( Terrain && g_pGrassLib && ( r_decoration_quality->GetInt() > 1 || g_bEditMode ) )
		{
			g_pGrassLib->Load() ;
		}
	}

	if( settingsFlags & FrontEndShared::SC_TEXTURE_QUALITY )
	{
		void r3dParticleSystemReloadCachedDataTextures();
		r3dParticleSystemReloadCachedDataTextures();
		r3dMaterialLibrary::ReloadMaterialTextures();
		r3dGameLevel::Environment.ReloadTextures();

		if( g_pGrassLib )
			g_pGrassLib->ReloadTextures();

		if( g_pDecalChief )
			g_pDecalChief->ReloadTextures();

		if( Terrain )
		{
			Terrain->ReloadTextures();
		}

		for( GameObject *obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
		{
			if( obj->Class->Name == "obj_Lake" )
			{
				// recreates it with new dimmensions
				static_cast<WaterBase*>( static_cast<obj_Lake*>(obj) )->ReloadTextures();
			}
			else
			if( obj->Class->Name == "obj_WaterPlane" )
			{
				// recreates it with new dimmensions
				static_cast<WaterBase*>( static_cast<obj_WaterPlane*>(obj) )->ReloadTextures();
			}
		}
	}

	if( settingsFlags & FrontEndShared::SC_TERRAIN_QUALITY )
	{
		if( Terrain1 )
		{
			// only do this if ql changes to 1 or from 1
			if( r_terrain_quality->GetInt() == 1 && Terrain1->LastQLInit != 1 
					||
				r_terrain_quality->GetInt() != 1 && Terrain1->LastQLInit <= 1 
				)
			{
				if( !g_bEditMode )
				{
					Terrain1->PrepareForSettingsUpdateInGame();
				}

				Terrain1->RecreateVertexBuffer();
				Terrain1->UpdateAllVertexData();
				Terrain1->RecreateIndexData();

				if( !g_bEditMode )
				{
					Terrain1->ReleaseSettingsUpdateData();
				}
			}
		}

#ifndef FINAL_BUILD
		if( Terrain2 )
		{
			Terrain2->UpdateQualitySettings() ;
		}
#endif
	}

	if( settingsFlags & FrontEndShared::SC_SHADOWS_QUALITY )
	{
		ResetShadowCache();
		UpdateHWSchadowScheme();
		CurRenderPipeline->DestroyShadowResources();
		CurRenderPipeline->CreateShadowResources();
	}

	if (settingsFlags & FrontEndShared::SC_LIGHTING_QUALITY)
	{
		CurRenderPipeline->DestroyAuxResource();
		CurRenderPipeline->CreateAuxResource();

		void SyncLightingAndSSAO();
		SyncLightingAndSSAO();
	}

	UpdateMLAA();
}

static void DoExecIni( const char* Path )
{
	ExecVarIni( Path );
	// always override game settings ini with "local" ini
	ExecVarIni( "local.ini" );

}

bool CreateFullIniPath( char* dest, bool old )
{
	bool res = old ? CreateWorkPath(dest) : CreateConfigPath(dest);
	if(res)
		strcat( dest, INI_FILE );
	return res;
}

bool CreateFullMappingPath( char* dest, bool old )
{
	bool res = old ? CreateWorkPath(dest) : CreateConfigPath(dest);
	if(res)
		strcat( dest, INPUT_MAP_FILE );
	return res;
}

void OnFoundIniFile( const char* FullIniPath )
{
	r3dOutToLog( "readGameOptionsFile: found INI at %s\n", FullIniPath );

	DoExecIni( FullIniPath );

	// check if user has changed monitor and his new monitor doesn't support his resolution (by checking his desktop res)
	int deskW, deskH ;
	r3dGetDesktopDimmensions( &deskW, &deskH );
	if(r_width->GetInt() > deskW || r_height->GetInt() > deskH)
	{
		r3dOutToLog("Desktop resolution is smaller than in settings. Resetting to desktop resolution\n");
		r_width->SetInt( deskW );
		r_height->SetInt( deskH );
	}



}

void readGameOptionsFile()
{
	r_ini_read->SetBool( true );

#ifdef FINAL_BUILD
	// before reading ini to allow geeks to override it
	r_limit_fps->SetInt( 60 );
#endif

	// try local first
	if( !r3d_access( "./" INI_FILE, 4 ) )
	{
		r3dOutToLog( "readGameOptionsFile: found %s in local folder\n", INI_FILE );
		DoExecIni( "./" INI_FILE );
		g_locl_settings->SetBool( true );
	}
	else
	{
		g_locl_settings->SetBool( false );

		char FullIniPath[ MAX_PATH * 2 ];

		bool createdPath = CreateFullIniPath( FullIniPath, false );

		if( createdPath && r3d_access( FullIniPath, 4 ) == 0 )
		{			
			OnFoundIniFile( FullIniPath );
		}
		else
		{
			// true using old folder (appdata)
			createdPath = CreateFullIniPath( FullIniPath, true ) ;

			if( createdPath && r3d_access( FullIniPath, 4 ) == 0 )
			{
				OnFoundIniFile( FullIniPath );
			}
			else
			{
				r_ini_read->SetBool( false );

				if( !createdPath )
				{
					r3dOutToLog( "readGameOptionsFile: Error: couldn't get local app path! Using defaults!\n" );
				}
				else
				{
					r3dOutToLog( "readGameOptionsFile: couldn't open both %s and %s! Using defaults.\n", INI_FILE, FullIniPath );
				}

				int deskW, deskH ;
				r3dGetDesktopDimmensions( &deskW, &deskH );

				r_width->SetInt( deskW );
				r_height->SetInt( deskH );

				r3dOutToLog( "Selected resolution from desktop dimensions: %dx%d\n", r_width->GetInt(), r_height->GetInt() );

				SetDefaultSettings( r3dGetDeviceStrength() );
			}
		}
	}
	r_fullscreen_load->SetInt(r_fullscreen->GetInt());
/*#ifdef FINAL_BUILD
	r_force_aspect->SetFloat( 16.f / 9.f );
#endif*/

	switch( r_overall_quality->GetInt() )
	{
	case 1:
		SetDefaultSettings( S_WEAK );
		break;

	case 2:
		SetDefaultSettings( S_MEDIUM );
		break;

	case 3:
		SetDefaultSettings( S_STRONG );
		break;

	case 4:
		SetDefaultSettings( S_ULTRA );
		break;
	}

	applyGraphicOptionsSoft( FrontEndShared::SC_ALL );

	if(r_server_region->GetInt()==-1) // locate our region
	{
		int our_region = 0; // 0-us, 1-eu
		LCID userLCID = GetUserDefaultLCID();
		switch(userLCID)
		{ // from http://support.microsoft.com/kb/193080
		case 1025: //		Arabic (Saudi Arabia)                
		case 2049: //		Arabic (Iraq)                        
		case 3073: //		Arabic (Egypt)                       
		case 4097: //		Arabic (Libya)                       
		case 5121: //		Arabic (Algeria)                     
		case 6145: //		Arabic (Morocco)                     
		case 7169: //		Arabic (Tunisia)                     
		case 8193: //		Arabic (Oman)                        
		case 9217: //		Arabic (Yemen)                       
		case 10241: //		Arabic (Syria)                      
		case 11265: //		Arabic (Jordan)                     
		case 12289: //		Arabic (Lebanon)                    
		case 13313: //		Arabic (Kuwait)                     
		case 14337: //		Arabic (U.A.E.)                     
		case 15361: //		Arabic (Bahrain)                    
		case 16385: //		Arabic (Qatar)                      
		case 1026: //		Bulgarian                            
		case 1029: //		Czech                                
		case 1030: //		Danish                               
		case 1031: //		German (Standard)                    
		case 2055: //		German (Swiss)                       
		case 3079: //		German (Austrian)                    
		case 4103: //		German (Luxembourg)                  
		case 5127: //		German (Liechtenstein)               
		case 1032: //		Greek                                
		case 2057: //		English (United Kingdom)             
		case 6153: //		English (Ireland)                    
		case 1035: //		Finnish                              
		case 1036: //		French (Standard)                    
		case 2060: //		French (Belgian)                     
		case 4108: //		French (Swiss)                       
		case 5132: //		French (Luxembourg)                  
		case 1037: //		Hebrew                               
		case 1038: //		Hungarian                            
		case 1039: //		Icelandic                            
		case 1040: //		Italian (Standard)                   
		case 2064: //		Italian (Swiss)                      
		case 1043: //		Dutch (Standard)                     
		case 2067: //		Dutch (Belgian)                      
		case 1044: //		Norwegian (Bokmal)                   
		case 2068: //		Norwegian (Nynorsk)                  
		case 1045: //		Polish                               
		case 2070: //		Portuguese (Portugal)                
		case 1048: //		Romanian                             
		case 1049: //		Russian                              
		case 1050: //		Croatian                             
		case 2074: //		Serbian (Latin)                      
		case 3098: //		Serbian (Cyrillic)                   
		case 1051: //		Slovak                               
		case 1052: //		Albanian                             
		case 1053: //		Swedish                              
		case 2077: //		Swedish (Finland)                    
		case 1055: //		Turkish                              
		case 1058: //		Ukrainian                            
		case 1059: //		Belarusian                           
		case 1060: //		Slovenian                            
		case 1061: //		Estonian                             
		case 1062: //		Latvian                              
		case 1063: //		Lithuanian                           
		case 1078: //		Afrikaans                            
		case 1080: //		Faeroese  
			our_region = 1;
		default:
			our_region = 0;
		}
		r_server_region->SetInt(our_region);
	}

	if(g_user_language->GetString()[0]=='\0') // locate our language
	{
		LCID userLCID = GetUserDefaultLCID();
		switch(userLCID)
		{ // from http://support.microsoft.com/kb/193080
		case 1025: //		Arabic (Saudi Arabia)                
		case 2049: //		Arabic (Iraq)                        
		case 3073: //		Arabic (Egypt)                       
		case 4097: //		Arabic (Libya)                       
		case 5121: //		Arabic (Algeria)                     
		case 6145: //		Arabic (Morocco)                     
		case 7169: //		Arabic (Tunisia)                     
		case 8193: //		Arabic (Oman)                        
		case 9217: //		Arabic (Yemen)                       
		case 10241: //		Arabic (Syria)                      
		case 11265: //		Arabic (Jordan)                     
		case 12289: //		Arabic (Lebanon)                    
		case 13313: //		Arabic (Kuwait)                     
		case 14337: //		Arabic (U.A.E.)                     
		case 15361: //		Arabic (Bahrain)                    
		case 16385: //		Arabic (Qatar) 
		case 1026: //		Bulgarian                            
		case 1029: //		Czech                                
		case 1030: //		Danish                               
			g_user_language->SetString("english");
			break;
		case 1031: //		German (Standard)                    
		case 2055: //		German (Swiss)                       
		case 3079: //		German (Austrian)                    
		case 4103: //		German (Luxembourg)                  
		case 5127: //		German (Liechtenstein)               
			g_user_language->SetString("german");
			break;
		case 1032: //		Greek                                
		case 2057: //		English (United Kingdom)             
		case 6153: //		English (Ireland)                    
		case 1035: //		Finnish  
			g_user_language->SetString("english");
			break;
		case 1036: //		French (Standard)                    
		case 2060: //		French (Belgian)                     
		case 4108: //		French (Swiss)                       
		case 5132: //		French (Luxembourg)                  
			g_user_language->SetString("french");
			break;
		case 1037: //		Hebrew                               
		case 1038: //		Hungarian                            
		case 1039: //		Icelandic    
			g_user_language->SetString("english");
			break;
		case 1040: //		Italian (Standard)                   
		case 2064: //		Italian (Swiss)                      
			g_user_language->SetString("italian");
			break;
		case 1043: //		Dutch (Standard)                     
		case 2067: //		Dutch (Belgian)                      
		case 1044: //		Norwegian (Bokmal)                   
		case 2068: //		Norwegian (Nynorsk)                  
		case 1045: //		Polish                               
			g_user_language->SetString("english");
			break;
		case 2070: //		Portuguese (Portugal)                
			g_user_language->SetString("spanish");
			break;
		case 1048: //		Romanian                             
			g_user_language->SetString("english");
			break;
		case 1049: //		Russian                              
			g_user_language->SetString("russian");
			break;
		case 1050: //		Croatian                             
		case 2074: //		Serbian (Latin)                      
		case 3098: //		Serbian (Cyrillic)                   
		case 1051: //		Slovak                               
		case 1052: //		Albanian                             
		case 1053: //		Swedish                              
		case 2077: //		Swedish (Finland)                    
		case 1055: //		Turkish                              
			g_user_language->SetString("english");
			break;
		case 1058: //		Ukrainian                            
		case 1059: //		Belarusian                           
			g_user_language->SetString("russian");
			break;
		case 1060: //		Slovenian                            
		case 1061: //		Estonian                             
		case 1062: //		Latvian                              
		case 1063: //		Lithuanian                           
		case 1078: //		Afrikaans                            
		case 1080: //		Faeroese  
			g_user_language->SetString("english");
			break;
		default:
			g_user_language->SetString("english");
			break;
		}
	}
}

void writeGameOptionsFile()
{
	char fullPath[ MAX_PATH * 2 ];

	bool saveToLocal = true ;

	if( !g_locl_settings->GetBool() )
	{
		if( CreateFullIniPath( fullPath, false ) )
		{
			r3dOutToLog( "writeGameOptionsFile: Saving settings to %s\n", fullPath );
			g_pCmdProc->SaveVars( fullPath );

			saveToLocal = false ;
		}
		else
		{
			r3dOutToLog( "writeGameOptionsFile: couldn't create path to %s\n", fullPath );
		}
	}

	if( saveToLocal )
	{
		r3dOutToLog( "writeGameOptionsFile: saving settings to local folder.\n" );
		g_pCmdProc->SaveVars( INI_FILE );
	}
}

void OnFoundInputMap( const char* FullIniPath )
{
	r3dOutToLog( "readInputMap: found file at %s\n", FullIniPath );
	InputMappingMngr->loadMapping(FullIniPath);
}

void readInputMap()
{
	char FullIniPath[ MAX_PATH * 2 ];
	bool createdPath = CreateFullMappingPath( FullIniPath, false );
	if( createdPath && r3d_access( FullIniPath, 4 ) == 0 )
	{
		OnFoundInputMap( FullIniPath );
	}
	else
	{
		createdPath = CreateFullMappingPath( FullIniPath, true );

		if( createdPath && r3d_access( FullIniPath, 4 ) == 0 )
		{
			OnFoundInputMap( FullIniPath );
		}
		else
		{
			if( !createdPath )
			{
				r3dOutToLog( "readInputMap: Error: couldn't get local app path! Using defaults!\n" );
			}
			else
			{
				r3dOutToLog( "readInputMap: couldn't open both %s and %s! Using defaults.\n", INPUT_MAP_FILE, FullIniPath );
			}
		}
	}
}

void writeInputMap()
{
	char fullPath[ MAX_PATH * 2 ];

	if( CreateFullMappingPath( fullPath, false ) )
	{
		r3dOutToLog( "writeInputMap: Saving settings to %s\n", fullPath );
		InputMappingMngr->saveMapping(fullPath);
	}
	else
	{
		r3dOutToLog( "writeInputMap: couldn't create path to %s\n", fullPath );
	}
}

// Called right after main application window is created and OS critical systems initialized
// Probably it's good place to start networking, etc
void game::Init()
{
	// create named event to signalize that game is started
	// handle will be automatically closed on program termination
	static HANDLE g_gameEvt = CreateEvent(NULL, FALSE, FALSE, "Global\\WarInc_Game_001");

	r3dOutToLog("ComputerID: 0x%I64x\n", g_HardwareInfo.uniqueId);
	r3dOutToLog("Game Version: %s\n", GetBuildVersionString());

	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof(stat);
	GlobalMemoryStatusEx(&stat);
	r3dOutToLog("Available memory: %d MB\n", (DWORD)(stat.ullTotalPhys / 1024 / 1024));

	r3dFileManager_OpenArchive("wo");

	RegisterAllVars();
#ifndef FINAL_BUILD
	RegisterHUDCommands();
#endif

	readGameOptionsFile();
	g_num_game_executed2->SetInt(g_num_game_executed2->GetInt()+1);
	writeGameOptionsFile(); // to make sure that it always exists

	if(g_test_rus_client->GetBool())
		RUS_CLIENT = 1;

	// set language
	if(RUS_CLIENT || g_force_rus_lang->GetInt())
		gLangMngr.Init(LANG_RU);
	else if(strcmp(g_user_language->GetString(), "french")==0)
		gLangMngr.Init(LANG_FR);
	else if(strcmp(g_user_language->GetString(), "german")==0)
		gLangMngr.Init(LANG_DE);
	else if(strcmp(g_user_language->GetString(), "italian")==0)
		gLangMngr.Init(LANG_IT);
	else if(strcmp(g_user_language->GetString(), "spanish")==0)
		gLangMngr.Init(LANG_SP);
	else if(strcmp(g_user_language->GetString(), "english")==0)
		gLangMngr.Init(LANG_EN);
	else if(strcmp(g_user_language->GetString(), "russian")==0)
		gLangMngr.Init(LANG_RU);
	else // default to english, should not happen
	{
		r3d_assert(false);
		gLangMngr.Init(LANG_EN);
	}

	InputMappingMngr = new r3dInputMappingMngr;

	readInputMap();
	writeInputMap(); // to make sure that it always exists

	InitSounds();

#ifndef FINAL_BUILD
	char buffer[ MAX_PATH ];
	GetCurrentDirectory( sizeof( buffer ), buffer );
	FileTrackChanges( buffer, CallbackFileChange );
#endif	

	GameWorld_Create();
	ClientGameLogic::CreateInstance();
}


//
// Called after MainLoop returns 
//
void game::Shutdown()
{
	FileTrackShutdown();

	snd_CloseSoundSystem();

	gLangMngr.Destroy();

	SAFE_DELETE(InputMappingMngr);
	UnregisterAllVars();

	ClientGameLogic::DeleteInstance();
	GameWorld_Destroy();
}


void 	ExecuteModelViewer()
{
	int m_ret = 0;
}


void 	ExecuteDirector()
{
	int m_ret = 0;
}

void 	ExecuteFXCreator()
{
	int m_ret = 0;
}



#ifndef FINAL_BUILD
void UpdateDB(const char* api_addr, const char* out_xml);
#endif

void ExecuteNetworkGame();
void ExecuteLevelEditor();
void ExecuteCharacterEditor();
void ExecuteParticleEditor();
void ExecutePhysicsEditor();
void ExecuteBackendTest();

extern int		_r3d_bTerminateOnZ;

void game::MainLoop()
{
	// init steam we need to initialize this before the renderer for the overlay.
	if(!RUS_CLIENT)
	{
		gSteam.InitSteam();
		if(gSteam.steamID) {
			gUserProfile.RegisterSteamCallbacks();
		}
	}
	

	InitRender(1);

	CurRenderPipeline = new r3dDefferedRenderer;
	CurRenderPipeline->Init();

	SetFocus(win::hWnd);

	r3dMenuInit();

	r3dParticleSystemInit();

	InitDesktopSystem();

	InitPostFX();
	InitPointLightsRendererV2();

	g_pWind = new r3dWind ;

	g_pEngineConsole = new EngineConsole;
	g_pDefaultConsole = g_pEngineConsole;
	g_pEngineConsole->Init();
	g_pEngineConsole->SetCommandProcessor( g_pCmdProc );

	g_DamageLib = new DamageLib ;
	g_DamageLib->Load();

#ifndef FINAL_BUILD
	g_Manipulator3d.Init();

	// show non-level specific art bugs
	r3dShowArtBugs() ;
#endif

	g_cursor_mode->SetChangeCallback( &CursorModeCallback ) ;

	_r3d_bTerminateOnZ = r_terminateOnZ->GetInt();

	// set dynamic matlib by default.
	// make it statis only in GAME mode
	r3dMaterialLibrary::IsDynamic = true;

	imgui_SetFixMouseCoords( false );


#if 0 // PAX_BUILD
	int m_ret = Menu_AppSelect::bStartGameSVN; 
#else
#ifdef FINAL_BUILD
	int m_ret = Menu_AppSelect::bStartGamePublic; // start game
#else
	int m_ret = 0;
	if ( *d_map_force_load->GetString() || LevelEditName[0]!='\0' )
		m_ret = Menu_AppSelect::bStartLevelEditor;
	else
		CALL_MENU(Menu_AppSelect);
#endif // __FINAL
#endif

#ifndef FINAL_BUILD
	// all choises is editors by default
	g_bEditMode = true;
	g_bStartedAsParticleEditor = false;
#endif

	void InitGrass();
	InitGrass();

	g_pHUDCameraEffects = new HUDCameraEffects ;

	gWeaponArmory.Init();

	// for editors, do not lock mouse. when we start game, in ExecuteNetworkGame we will set that var to true
	d_mouse_window_lock->SetBool(false);

	switch (m_ret)
	{
#ifndef FINAL_BUILD
	case	Menu_AppSelect::bUpdateDB:
		g_bEditMode = false;
		UpdateDB("127.0.0.1", "Data/Weapons/itemsDB.xml");
		MessageBox(0, "Successfully updated English DB!", "Result", MB_OK);

		UpdateDB("127.0.0.1", "Data/Weapons/itemsDB_RU.xml");
		MessageBox(0, "Successfully updated Russian DB!", "Result", MB_OK);
		break;
#endif

	case Menu_AppSelect::bStartGamePublic:
		// override server settings if special key isn't set
		if(strstr(__r3dCmdLine, "-ffgrtvzdf") == NULL)
		{
			if(RUS_CLIENT) {
				g_serverip->SetString("127.0.0.1");
			} else if(MASSIVE_CLIENT) {
				g_serverip->SetString("127.0.0.1");
			} else {
				g_serverip->SetString("127.0.0.1");
			}
		}

		// override API settings
		if(RUS_CLIENT) {
			g_api_ip->SetString("127.0.0.1");
		} else if(MASSIVE_CLIENT) {
			g_api_ip->SetString("127.0.0.1");
		} else {
			g_api_ip->SetString("127.0.0.1");
		}
	case	Menu_AppSelect::bStartGameSVN:
		g_bEditMode = false;
		ExecuteNetworkGame();
		break;
#ifndef FINAL_BUILD
	case	Menu_AppSelect::bStartLevelEditor:
		g_pVisibilityGrid = new VisibiltyGrid ;
		ExecuteLevelEditor();
		break;

	case	Menu_AppSelect::bStartParticleEditor:
		g_bStartedAsParticleEditor = true;
		ExecuteParticleEditor();
		break;

	case Menu_AppSelect::bStartPhysicsEditor:
		ExecutePhysicsEditor();
		break;

	case Menu_AppSelect::bStartCharacterEditor:
		ExecuteCharacterEditor();
		break;
#endif
	};

#ifndef FINAL_BUILD
	if( g_bEditMode )
	{
		SAFE_DELETE( g_pVisibilityGrid );
	}
#endif
	
	// shutdown steam
	if(gSteam.inited_) {
		gUserProfile.DeregisterSteamCallbacks();
		gSteam.Shutdown();
	}

	SAFE_DELETE( g_pWind ) ;

	r3dMaterialLibrary::UnloadManaged();
	r3dMaterialLibrary::Reset();	
	MeshGlobalBuffer::unloadManaged();
	gWeaponArmory.Destroy();

#ifndef FINAL_BUILD
	g_Manipulator3d.Close();
#endif

	r3dMenuClose();

	AI_Player_FreeStuff();

	SAFE_DELETE( g_pHUDCameraEffects ) ;
	
	SAFE_DELETE( g_GameRewards );

	void CloseGrass();
	CloseGrass();

	DestroyPointLightsRendererV2();

	ClosePostFX();

	SAFE_DELETE( g_DamageLib );
	
	r3dFreeGOBMeshes();

	g_pEngineConsole->Release();
	SAFE_DELETE( g_pEngineConsole );

	g_pDefaultConsole = NULL;

	ReleaseDesktopSystem();

	CloseCollections();

	r3dParticleSystemClose();
	CurRenderPipeline->Close();
	SAFE_DELETE(CurRenderPipeline);
	
	r3dMaterialLibrary::Destroy();

	CloseRender();

	if(gSurveyOutLink)
		ShellExecute(NULL, "open", gSurveyOutLink, "", NULL, SW_SHOW);

	return;
}

#ifndef FINAL_BUILD
extern const char* g_ServerKey;
extern int		gDomainPort;
extern bool		gDomainUseSSL;

#include "CkByteData.h"
void UpdateDB(const char* api_addr, const char* out_xml)
{
	CkHttp http;
	int success = http.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if (success != 1) 
		r3dError("Internal error!!!");

	// get items DB
	{
		CkHttpRequest req;
		req.UsePost();
		req.put_Path("/php/api_getItemsDB.php");
		req.AddParam("serverkey",g_ServerKey);

		CkHttpResponse *resp = 0;
		resp = http.SynchronousRequest(api_addr, gDomainPort, gDomainUseSSL, req);
		if(!resp)
			r3dError("timeout getting items db");
			
		// we can't use getBosyStr() because it'll fuckup characters inside UTF-8 xml
		CkByteData bodyData;
		resp->get_Body(bodyData);
		
		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(
			(void*)bodyData.getBytes(), 
			bodyData.getSize(), 
			pugi::parse_default, 
			pugi::encoding_utf8);
		if(!parseResult)
			r3dError("Failed to parse server weapon XML, error: %s", parseResult.description());

		xmlFile.save_file(
			out_xml, 
			PUGIXML_TEXT("\t"), 
			pugi::format_default, 
			pugi::encoding_utf8);
	}

	return;
}
#endif

void PrepareEditor(const char* levelName)
{
	r3dGameLevel::SetHomeDir(levelName);

	char Str[256];
	sprintf(Str, "%s\\Constants.var", r3dGameLevel::GetHomeDir());
	cvars_Read(Str);	
}

void 	ExecuteLevelEditor()
{
#ifndef FINAL_BUILD
	int m_ret = 0;

	//d_map_force_load->SetString("WorkInProgress\\Editor_Particles"); //@
	if (LevelEditName[0]=='\0')
	{
		if ( *d_map_force_load->GetString() )
		{
			r3dscpy( LevelEditName, d_map_force_load->GetString() ); 
		}
		else
		{
			CALL_MENU(Menu_Main);
			if(m_ret == 0) // exit
				return;
		}
	}

	PrepareEditor(LevelEditName);
	PlayEditor();
#endif
}

extern int CurHUDID;

void ExecuteCharacterEditor()
{
	PrepareEditor("WorkInProgress/Editor_Character");
	CurHUDID = 5;
	PlayEditor();
}

void 	ExecuteParticleEditor()
{
	PrepareEditor("WorkInProgress/Editor_Particles");
	CurHUDID = 3;
	PlayEditor();
}

void ExecutePhysicsEditor()
{
	PrepareEditor("WorkInProgress/Editor_Physics");
	CurHUDID = 4;
	PlayEditor();
}

