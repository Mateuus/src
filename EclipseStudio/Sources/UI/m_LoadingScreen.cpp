#include "r3dPCH.h"
#include "r3d.h"

#include "m_LoadingScreen.h"
#include "GameCode\UserProfile.h"

#include "Multiplayer\MasterServerLogic.h"

#include "LangMngr.h"

#include "r3dDeviceQueue.h"

LoadingScreen::LoadingScreen( const char * movieName ) 
: UIMenu(movieName) 
{
	m_pBackgroundTex = 0;
	m_RenderingDisabled = false;
}

//------------------------------------------------------------------------

LoadingScreen::~LoadingScreen()
{
	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
	m_pBackgroundTex = 0;
}

//------------------------------------------------------------------------

bool LoadingScreen::Initialize()
{
//	char EulaText[70000] = "";

	return true;
}

void ClearFullScreen_Menu();

int LoadingScreen::Update()
{
	R3D_ENSURE_MAIN_THREAD();

	r3dMouse::Show();
	r3dStartFrame();

	if( r3dRenderer->DeviceAvailable )
	{
		r3dRenderer->StartRender(1);
		r3dRenderer->StartFrame();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		ClearFullScreen_Menu();

		// for now just draw a static picture in background, later on will be a video
		r3d_assert(m_pBackgroundTex);

		float x, y, w, h;
		r3dRenderer->GetBackBufferViewport(&x, &y, &w, &h);
		D3DVIEWPORT9 oldVp, newVp;

		r3dRenderer->DoGetViewport(&oldVp);
		newVp = oldVp;
		newVp.X = 0;
		newVp.Y = 0;
		newVp.Width = r3dRenderer->d3dpp.BackBufferWidth;
		newVp.Height = r3dRenderer->d3dpp.BackBufferHeight;
		r3dRenderer->SetViewport( (float)newVp.X, (float)newVp.Y, (float)newVp.Width, (float)newVp.Height );
		DWORD oldScissor = 0;
		r3dRenderer->pd3ddev->GetRenderState(D3DRS_SCISSORTESTENABLE, &oldScissor);
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		r3dDrawBox2D(x, y, w, h, r3dColor24::white, m_pBackgroundTex);
		r3dRenderer->SetViewport( (float)oldVp.X, (float)oldVp.Y, (float)oldVp.Width, (float)oldVp.Height );
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, oldScissor);

		if(!m_RenderingDisabled)
		{
			gfxMovie.UpdateAndDraw();
		}

		r3dRenderer->Flush();  
		r3dRenderer->EndFrame();
	}

	r3dRenderer->EndRender( true );
	r3dEndFrame();

	return 0;
}

//------------------------------------------------------------------------
void LoadingScreen::SetLoadingTexture(const char* ImagePath)
{
	R3D_ENSURE_MAIN_THREAD();

	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
	m_pBackgroundTex = r3dRenderer->LoadTexture(ImagePath);
	r3d_assert(m_pBackgroundTex);
}

void LoadingScreen::SetData( const char* ImagePath, const wchar_t* Name, const wchar_t* Message, int mapType, const wchar_t* tip_of_the_day )
{
	R3D_ENSURE_MAIN_THREAD();

	if(!m_RenderingDisabled)
		SetLoadingTexture(ImagePath);

	if(g_num_matches_played->GetInt() == 1 && g_num_game_executed2->GetInt()==1) // first time launch of the game, show keyboard schematic
	{
		gfxMovie.SetVariable("_root.Main._visible", false);
		gfxMovie.SetVariable("_root.Main2._visible", true);
	}
	else
	{
		gfxMovie.SetVariable("_root.Main._visible", true);
		gfxMovie.SetVariable("_root.Main2._visible", false);
	}


	gfxMovie.SetVariable("_global.MapName", Name);
	gfxMovie.SetVariable("_global.MapDesc", Message);
	{
		const wchar_t* gameMode=L""; const char* modeType="";
		switch(mapType)
		{
		case GBGameInfo::MAPT_Bomb:
			gameMode = gLangMngr.getString("$GameMode_Sabotage");
			modeType = "Siege";
			break;
		case GBGameInfo::MAPT_Conquest:
			gameMode = gLangMngr.getString("$GameMode_Conquest");
			modeType = "Conquest";
			break;
		case GBGameInfo::MAPT_Deathmatch:
			gameMode = gLangMngr.getString("$GameMode_Deathmatch");
			modeType = "DM";
			break;
		default:
			break;
		}
		gfxMovie.SetVariable("_global.MapModeName", gameMode);
		gfxMovie.SetVariable("_global.MapModeType", modeType); //Conquest, DM, Siege
	}

	gfxMovie.SetVariable("_global.TipCaption", gLangMngr.getString("TipOfTheDay"));
	gfxMovie.SetVariable("_global.TipText", tip_of_the_day?tip_of_the_day:L"");
}

//------------------------------------------------------------------------

void
LoadingScreen::SetProgress( float progress )
{
	R3D_ENSURE_MAIN_THREAD();
	gfxMovie.SetVariable("_global.LoadedPercent", progress*100);
}

//------------------------------------------------------------------------

static volatile float	gProgress;
static LoadingScreen*	gLoadingScreen;

void StartLoadingScreen()
{
	r3d_assert( !gLoadingScreen );
	gLoadingScreen = new LoadingScreen( "Data\\Menu\\LoadingScreen.swf" );

	gLoadingScreen->Load();
	gLoadingScreen->Initialize();
}
void DisableLoadingRendering()
{
	gLoadingScreen->SetRenderingDisabled( true );
}

//------------------------------------------------------------------------

void StopLoadingScreen()
{
	r3d_assert( gLoadingScreen );
	delete gLoadingScreen;

	gLoadingScreen = NULL;
}

void SetLoadingTexture(const char* ImagePath)
{
	if(gLoadingScreen)
	{
		gLoadingScreen->SetLoadingTexture(ImagePath);
	}
}

//------------------------------------------------------------------------

void SetLoadingProgress( float progress )
{
	progress = R3D_MAX( R3D_MIN( progress, 1.f ), 0.f );
	InterlockedExchange( (volatile long*)&gProgress, (LONG&)progress );
}

//------------------------------------------------------------------------

void AdvanceLoadingProgress( float add )
{
	float newVal = R3D_MAX( R3D_MIN( gProgress + add, 1.f ), 0.f );
	InterlockedExchange( (volatile long*)&gProgress, (LONG&)newVal );
}

float GetLoadingProgress()
{
	return gProgress;
}

//------------------------------------------------------------------------

void SetLoadingPhase( const char* Phase )
{
	Phase;
}

//------------------------------------------------------------------------

int DoLoadingScreen( volatile LONG* Loading, const wchar_t* LevelName, const wchar_t* LevelDescription, const char* LevelFolder, float TimeOut, int gameMode )
{
	r3d_assert( gLoadingScreen );

	char sFullPath[512];
	sprintf( sFullPath, "%s\\%s", LevelFolder, "LoadingScreen.dds" );

	// no dollar sign for access
	if(r3d_access(sFullPath + 1 , 0) != 0)
	{
		int sel = rand()%3;
		sprintf( sFullPath, "%s\\LoadingScreen%d.dds", LevelFolder, sel );
	}

	char tempStr[32];
	sprintf(tempStr, "TipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 12.99f))));
	if(gameMode == GBGameInfo::MAPT_Conquest || gameMode == GBGameInfo::MAPT_Bomb)
	{
		if(u_GetRandom(0.0f, 1.0f) > 0.7f)
		{
			if(gameMode == GBGameInfo::MAPT_Conquest)
				sprintf(tempStr, "ConquestTipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 3.99f))));
			else if(gameMode == GBGameInfo::MAPT_Bomb)
				sprintf(tempStr, "SabotageTipOfTheDay%d", int(floorf(u_GetRandom(0.0f, 4.99f))));
		}
	}
	gLoadingScreen->SetData( sFullPath, LevelName, LevelDescription, gameMode, gLangMngr.getString(tempStr));

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while( *Loading )
	{
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		r3dProcessWindowMessages();

		if( r3dRenderer->DeviceAvailable )
		{
			float timeStart = r3dGetTime();

			float MaxRenderTime = 0.033f;

			for( ; r3dGetTime() - timeStart < 0.033f ; )
			{
				ProcessDeviceQueue( timeStart, MaxRenderTime ) ;
			}
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress( gProgress );
	}

	return 1;
}

//------------------------------------------------------------------------
bool IsNeedExit();
int DoConnectScreen( volatile LONG* Loading, const wchar_t* Message, float TimeOut )
{
	r3d_assert( gLoadingScreen );

	gLoadingScreen->SetData( "Data\\Menu\\ConnectScreen.dds", gLangMngr.getString("Connecting"), Message, -1, NULL );

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while( *Loading )
	{
		r3dProcessWindowMessages();

		if(IsNeedExit())
			return 0;
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress( checkTimeOut ? 1.f - ( endWait - r3dGetTime() ) / TimeOut : gProgress );
		Sleep( 33 );
	}

	return 1;
}

//------------------------------------------------------------------------

template <typename T>
int DoConnectScreen( T* Logic, bool (T::*CheckFunc)(), const wchar_t* Message, float TimeOut )
{
	r3d_assert( gLoadingScreen );

	gLoadingScreen->SetData( "Data\\Menu\\ConnectScreen.dds", gLangMngr.getString("Connecting"), Message, -1, NULL );

	bool checkTimeOut = TimeOut != 0.f;

	const float startWait = r3dGetTime();
	const float endWait = startWait + TimeOut;

	for(;;)
	{
		extern void tempDoMsgLoop();
		tempDoMsgLoop();

		if( (Logic->*CheckFunc)() )
			break;

		if(IsNeedExit())
			return 0;
		if( checkTimeOut && r3dGetTime() > endWait ) 
		{
			return 0;
		}

		// draw loaing screen only after some time
		// so minor waits will be performed without graphics change
		if(r3dGetTime() > startWait + 1.0f)
		{
			gLoadingScreen->Update();
			gLoadingScreen->SetProgress( checkTimeOut ? 1.f - ( endWait - r3dGetTime() ) / TimeOut : gProgress );
		}

		Sleep( 33 );
	}

	return 1;
}

template int DoConnectScreen( ClientGameLogic* Logic, bool (ClientGameLogic::*CheckFunc)(), const wchar_t* Message, float TimeOut );
template int DoConnectScreen( MasterServerLogic* Logic, bool (MasterServerLogic::*CheckFunc)(), const wchar_t* Message, float TimeOut );

//------------------------------------------------------------------------

