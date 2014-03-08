#include "r3dPCH.h"
#include "r3d.h"

#include "r3dBackgroundTaskDispatcher.h"

#include "r3dNetwork.h"

#include "Particle.h"

#pragma warning (disable: 4244)
#pragma warning (disable: 4305)
#pragma warning (disable: 4101)

#include "cvar.h"
#include "fmod/soundsys.h"

#include "APIScaleformGFX.h"
#include "GameCommon.h"
#include "GameLevel.h"

#include "../SF/CmdProcessor/CmdProcessor.h"
#include "../SF/Console/Config.h"
#include "../SF/Console/EngineConsole.h"

#include "r3dNetwork.h"
#include "multiplayer/ClientGameLogic.h"
#include "multiplayer/MasterServerLogic.h"
#include "multiplayer/LoginSessionPoller.h"

#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/Gameplay/obj_SupplyCrate.h"
#include "ObjectsCode/Gameplay/obj_SiegeObjective.h"
#include "ObjectsCode/Gameplay/obj_BombPlacement.h"

#include "UI\m_EndRound.h"
#include "UI\FrontEndNew.h"
#include "UI\m_Login.h"
#include "UI\m_LoginGNA.h"
#include "UI\m_LoadingScreen.h"
#include "UI\WelcomePackage.h"

	char		_p2p_masterHost[MAX_PATH] = ""; // master server ip
	int		_p2p_masterPort = GBNET_CLIENT_PORT;
static	char		_p2p_gameHost[MAX_PATH] = "";	// game server ip
static	int		_p2p_gamePort           = 0;	// game server port
static	__int64		_p2p_gameSessionId	= 0;

// temporary externals from game.cpp
extern void GameStateGameLoop();
extern void InitGame();
extern void DestroyGame();
extern void GameFrameStart();

extern bool IsNeedExit();
extern void InputUpdate();

extern EGameResult PlayNetworkGame();

void tempDoMsgLoop()
{
	r3dProcessWindowMessages();

	gClientLogic().Tick();
	gMasterServerLogic.Tick();

	return;
}

static void SetNewLogFile()
{
	extern void r3dChangeLogFile(const char* fname);
	char buf[512];
	sprintf(buf, "logs\\r3dlog_client_%d.txt", GetTickCount());
	r3dChangeLogFile(buf);
}

static bool ConnectToGameServer()
{
	r3d_assert(_p2p_gamePort);
	r3d_assert(_p2p_gameHost[0]);
	r3d_assert(_p2p_gameSessionId);

	gClientLogic().Reset();
	return gClientLogic().Connect(_p2p_gameHost, _p2p_gamePort);
}

static void MasterServerQuckJoin()
{
	r3d_assert(_p2p_masterHost[0]);

	gMasterServerLogic.StartConnect(_p2p_masterHost, _p2p_masterPort);
	if(!DoConnectScreen(&gMasterServerLogic, &MasterServerLogic::IsConnected, gLangMngr.getString("WaitConnectingToServer"), 30.f ) )
		r3dError("can't connect to master server\n");

	gMasterServerLogic.RequestGameList();
	gMasterServerLogic.WaitForGameList();

	if(gMasterServerLogic.games_.size() > 0) 
	{
		gMasterServerLogic.SendJoinGame(0);
	} 
#ifndef FINAL_BUILD
	else
	{
		NetPacketsGameBrowser::GBPKT_C2M_CreateGame_s n;
		n.mapId           = GBGameInfo::MAPID_Editor_Particles;
		n.mapType         = GBGameInfo::MAPT_Conquest;
		n.maxPlayers   = 16;
		n.timeLimitVar    = 3;
		n.numRoundsVar	 = 0;
		n.friendlyFire    = 0;
		n.autoBalance     = 1;
		n.isBasicGame	= 0;
		gMasterServerLogic.SendCreateGame(n);
	}
#endif
	gMasterServerLogic.WaitForGameJoin();

	if(gMasterServerLogic.gameJoinAnswer_.result != GBPKT_M2C_JoinGameAns_s::rOk)
		r3dError("failed to join game: res %d\n", gMasterServerLogic.gameJoinAnswer_.result);

	gMasterServerLogic.GetJoinedGameServer(_p2p_gameHost, &_p2p_gamePort, &_p2p_gameSessionId);

	gMasterServerLogic.Disconnect();
}

static int processMenu(UIMenu* menu)
{
	r3dSetAsyncLoading( 0 ) ;
	menu->Load();
	int res = 0;
	if(menu->Initialize())
	{
		while(res == 0) {
			res = menu->Update();

			FileTrackDoWork();
		}
	}
	menu->Unload();
	SAFE_DELETE(menu);
	return res;
}

static int showLoginMenu(const wchar_t* showMessage)
{
	const char* swf = "data\\menu\\LoginMenu.swf";

	extern int RUS_CLIENT;
	if(RUS_CLIENT && g_test_rus_client->GetBool()==false)
	{
		CLoginGNAMenu* menu = new CLoginGNAMenu(swf);
		menu->m_ShowMessageIfAny = showMessage;
		return processMenu(menu);
	}
	CLoginMenu* menu = new CLoginMenu(swf);
	menu->m_ShowMessageIfAny = showMessage;
	return processMenu(menu);
}

static int showFrontEndMenu(EGameResult gameResult)
{
	UIMenu* frontend = NULL;
	frontend = new FrontendUI("data\\menu\\Frontend_New.swf", gameResult);
//	else
//		frontend = new CFrontendMenu("data\\menu\\Frontend.swf", gameResult);
	return processMenu(frontend);
}

static int showWelcomePackageMenu()
{
	const char* swf = "data\\menu\\WelcomePackage2.swf";
	WelcomePackageMenu* menu = new WelcomePackageMenu(swf);
	return processMenu(menu);
}

void* MainMenuSoundEvent = 0;
extern int RUS_CLIENT;
void ExecuteNetworkGame()
{
	// make sure that our login session poller will be terminated on function exit
	class CLoginSessionHolder
	{
	  public:
		CLoginSessionHolder() {};
		~CLoginSessionHolder() { 
			gLoginSessionPoller.Stop(); 
		}
	};
	CLoginSessionHolder loginholder;


	{
		r3dPoint3D soundPos(0,0,0), soundDir(0,0,1), soundUp(0,1,0);
		SoundSys.Update(soundPos, soundDir, soundUp);
	}

	int mainmenuTheme = -1;
	//if(!RUS_CLIENT)
	//	mainmenuTheme = SoundSys.GetEventIDByPath("Sounds/MainMenu GUI/mainmenu_theme");
	//else
		mainmenuTheme = SoundSys.GetEventIDByPath("Sounds/MainMenu GUI/warinc_menu01");
	MainMenuSoundEvent = snd_PlaySound(mainmenuTheme, r3dPoint3D(0,0,0));

	r3dscpy(_p2p_masterHost, g_serverip->GetString());

	bool quickGameJoin = false;
	/*  
	if(IDYES == MessageBox(NULL, "frontend?", "", MB_YESNO))
	quickGameJoin = false;
	*/    

	g_trees->SetBool(true);
	d_mouse_window_lock->SetBool(true);

#if 0 // buy packs is disabled for now!
	extern int RUS_CLIENT;
	int numRans = g_num_game_executed2->GetInt();
	if(/*((numRans%10==0||numRans == 1) && !RUS_CLIENT) ||*/ (RUS_CLIENT && numRans%3==0))
		FrontendUI::frontendRequestShowBuyPack = true;
#endif

	const wchar_t* showLoginErrorMsg = 0;

repeat_the_login:
	gLoginSessionPoller.Stop();

	if(!quickGameJoin)
	{
		int login_ret = showLoginMenu(showLoginErrorMsg);
		showLoginErrorMsg = 0;
		if(login_ret == CLoginMenu::RET_Exit)
			return;

		gLoginSessionPoller.Start(gUserProfile.CustomerID, gUserProfile.SessionID);
	}
	FrontendUI::frontendFirstTimeInit = true; // each time after login menu, show welcome screen

	EGameResult gameResult = GRESULT_Unknown;
repeat_the_menu:
	if(!quickGameJoin)
	{
		int res;
		if(gUserProfile.AccountStatus==100) // just registered, show welcome package
		{
			res = showWelcomePackageMenu();
			if(res == WelcomePackageMenu::RET_Exit || res == 0)
				return;
			gameResult = GRESULT_ShownWelcomePackage;
		}

		if(gameResult != GRESULT_Finished_ReadyNextRound)
			res = showFrontEndMenu(gameResult);
		else
			res = FrontEndShared::RET_JoinGame;

		gMasterServerLogic.Disconnect();

		if(res == FrontEndShared::RET_Exit || res == 0)
			return;
		if( res == FrontEndShared::RET_Diconnected)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_Disconnected");
			goto repeat_the_login;
		}
		else if( res == FrontEndShared::RET_Banned)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_AccountFrozen");
			goto repeat_the_login;
		}
		else if( res == FrontEndShared::RET_DoubleLogin)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_DoubleLogin");
			goto repeat_the_login;
		}
		r3d_assert(res == FrontEndShared::RET_JoinGame);
		gMasterServerLogic.GetJoinedGameServer(_p2p_gameHost, &_p2p_gamePort, &_p2p_gameSessionId);
	}
	else
	{
		// this code path is used only for testing when quickGameJoin is true.
		SetNewLogFile();

		StartLoadingScreen();
		MasterServerQuckJoin();
		StopLoadingScreen();

		// game server now require customer ID
		// so this is valid line because we skipped frontend/login in this code path
		gUserProfile.CustomerID = 1282052887;

		// fill temporary profile, for spawn menu rendering
		gUserProfile.ProfileData.ArmorySlots[0].LoadoutID = 1;
		gUserProfile.ProfileData.ArmorySlots[0].PrimaryWeaponID = 101001;
	}

	r3dEnsureDeviceAvailable();

	StartLoadingScreen();

	if(!ConnectToGameServer())
	{
		gClientLogic().Disconnect();
		StopLoadingScreen();
		showLoginErrorMsg = gLangMngr.getString("LoginMenu_CannotConnectServer");
		goto repeat_the_login;
	}

	r3dEnsureDeviceAvailable();

	if(gClientLogic().ValidateServerVersion(_p2p_gameSessionId) == 0)
	{
		gClientLogic().Disconnect();

		StopLoadingScreen();
		if(gClientLogic().serverVersionStatus_ == 0) // timeout on validating version
		{
			gameResult = GRESULT_Disconnect;
			goto repeat_the_menu;
		}
		else
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_ClientUpdateRequired");
			goto repeat_the_login;
		}
	}
	gLoginSessionPoller.ForceTick(); // force to update that we joined the game

	r3dEnsureDeviceAvailable();

	if(gClientLogic().WaitForLevelName() == 0)
	{
		gClientLogic().Disconnect();

		StopLoadingScreen();
		gameResult = GRESULT_Disconnect;
		goto repeat_the_menu;
	}

	switch(gClientLogic().m_gameInfo.mapId) 
	{
	default: 
		r3dError("invalid map id\n");
	case GBGameInfo::MAPID_Editor_Particles: 
		r3dGameLevel::SetHomeDir("Editor_Particles"); 
		break;
	case GBGameInfo::MAPID_WO_Crossroads16: 
		r3dGameLevel::SetHomeDir("WO_Crossroads16"); 
		break;
	case GBGameInfo::MAPID_WO_Crossroads2: 
		r3dGameLevel::SetHomeDir("WO_Crossroads2"); 
		break;
	case GBGameInfo::MAPID_WO_Nightfall_CQ: 
		r3dGameLevel::SetHomeDir("wo_nightfall_cq"); 
		break;
	case GBGameInfo::MAPID_WO_NightfallPAX: 
		r3dGameLevel::SetHomeDir("wo_nightfall_PAX"); 
		break;
	case GBGameInfo::MAPID_WO_Grozny:
		r3dGameLevel::SetHomeDir("WO_Grozny"); 
		break;
	case GBGameInfo::MAPID_WO_Grozny_CQ:
		r3dGameLevel::SetHomeDir("WO_Grozny_CQ"); 
		break;
	case GBGameInfo::MAPID_WO_Torn: 
		r3dGameLevel::SetHomeDir("wo_torn_cq"); 
		break;
	case GBGameInfo::MAPID_WO_Torn_CT: 
		r3dGameLevel::SetHomeDir("WO_Torn_ct"); 
		break;
	case GBGameInfo::MAPID_WO_Shipyard: 
		r3dGameLevel::SetHomeDir("wo_shippingyard"); 
		break;
	case GBGameInfo::MAPID_WO_Wasteland: 
		r3dGameLevel::SetHomeDir("wo_wasteland"); 
		break;
	case GBGameInfo::MAPID_WO_EasternBunkerTDM: 
		r3dGameLevel::SetHomeDir("wo_eastern_bunker_tdm"); 
		break;
	case GBGameInfo::MAPID_WO_TornTown: 
		r3dGameLevel::SetHomeDir("wo_TornTown_tdm"); 
		break;
	case GBGameInfo::MAPID_WO_Valley: 
		r3dGameLevel::SetHomeDir("WorkInProgress/wo_valley"); 
		break;
	case GBGameInfo::MAPID_WO_Inferno:
		r3dGameLevel::SetHomeDir("wo_inferno"); 
		break;
	case GBGameInfo::MAPID_WO_Jungle02:
		r3dGameLevel::SetHomeDir("wo_jungleruins");
		break;
	case GBGameInfo::MAPID_WO_Citadel_DM:
		r3dGameLevel::SetHomeDir("wo_citadel_dm");
		break;
	case GBGameInfo::MAPID_WO_TestConquest: 
		r3dGameLevel::SetHomeDir("WorkInProgress/TestConquest");
		break;
	case GBGameInfo::MAPID_WO_TestDeathmatch: 
		r3dGameLevel::SetHomeDir("WorkInProgress/TestDeathmatch");
		break;
	case GBGameInfo::MAPID_WO_TestSabotage: 
		r3dGameLevel::SetHomeDir("WorkInProgress/TestSabotage");
		break;
	case GBGameInfo::MAPID_BurningSea: 
		r3dGameLevel::SetHomeDir("WO_Burning_Sea");
		break;
	}

	// start the game
	gameResult = PlayNetworkGame();
	
	if
	(
		gameResult == GRESULT_Exit ||
		gameResult == GRESULT_Disconnect ||
		gameResult == GRESULT_Finished
	)
	{
		r3dRenderer->ChangeForceAspect(16.0f / 9);
	}

	StopLoadingScreen();

	gLoginSessionPoller.ForceTick(); // force to update that we left the game
	
	if(gameResult == GRESULT_DoubleLogin) {
		showLoginErrorMsg = gLangMngr.getString("LoginMenu_DoubleLogin");
		goto repeat_the_login;
	}

	if(gameResult != GRESULT_Exit)
		goto repeat_the_menu;

}

static EGameResult PlayNetworkGame()
{
	gCPMgr.Reset();
	gSiegeObjMgr.Reset();
	g_SupplyCrateMngr.Reset();
	gBombPlacementMgr.Reset();

	r3d_assert(GameWorld().bInited == 0);
	r3d_assert(GameWorld().GetFirstObject() == NULL);
	r3d_assert(gCPMgr.NumCP() == 0);
	r3d_assert(g_SupplyCrateMngr.getNumCrates() == 0);

	r_hud_filter_mode->SetInt(0); // turn off NVG

	if(!gClientLogic().RequestToJoinGame())
	{
		gClientLogic().Disconnect();
		return GRESULT_Failed_To_Join_Game;
	}

	r3dEnsureDeviceAvailable();

	InitGame();

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		r3d_assert(gClientLogic().m_DroppedBomb == NULL);
		gClientLogic().m_DroppedBomb = (obj_DroppedBomb*)srv_CreateGameObject("obj_DroppedBomb", "droppedBomb", r3dPoint3D(0,0,0));
	}

	extern void Do_Collection_Editor_Init();
	Do_Collection_Editor_Init();

	gCPMgr.CheckForNeededPoints();

	extern void TPSGameHUD_OnStartGame();
	TPSGameHUD_OnStartGame();

	GameWorld().Update();

	EGameResult gameResult = GRESULT_Playing;
	if(gClientLogic().RequestToStartGame())
	{
		extern BaseHUD* HudArray[6];
		Hud = HudArray[2];
		extern int CurHUDID;
		CurHUDID = 2;
		Hud->HudSelected ();

		GameWorld().Update();

		// physics warm up
		g_pPhysicsWorld->StartSimulation();
		g_pPhysicsWorld->EndSimulation();

		if(MainMenuSoundEvent)
		{
			SoundSys.Stop(MainMenuSoundEvent);
			SoundSys.Release(MainMenuSoundEvent);
			MainMenuSoundEvent = 0;
		}

		// enable those two lines to be able to profile first frames of game
		//r_show_profiler->SetBool(true);
		//r_profiler_paused->SetBool(false);

		r3dStartFrame();
		while(gameResult == GRESULT_Playing) 
		{
			r3dEndFrame();
			r3dStartFrame();

			ClearBackBufferFringes();

			R3DPROFILE_START("Game Frame");

			InputUpdate();

			GameFrameStart();

			if(!gClientLogic().serverConnected_ && !gClientLogic().gameFinished_)
			{
				gameResult = GRESULT_Disconnect;
				continue;
			}

			GameStateGameLoop();

			R3DPROFILE_END("Game Frame");

			if(gClientLogic().gameReadyToExit)
			{
				gameResult = GRESULT_Finished;
				if(gClientLogic().gameReadyForNextRound)
					gameResult = GRESULT_Finished_ReadyNextRound;
			}

			if(IsNeedExit())
				gameResult = GRESULT_Exit;

			// check for double login.
			#ifdef FINAL_BUILD
			if(!gLoginSessionPoller.IsConnected())
				gameResult = GRESULT_DoubleLogin;
			#endif

			FileTrackDoWork();
		} 
	}
	else // failed to connect
		gameResult = GRESULT_Disconnect;
	
	if(gameResult == GRESULT_Finished || gameResult == GRESULT_Finished_ReadyNextRound) // wait for server to update sql
	{
		if(gClientLogic().gameFinished_) // if game has finished, we need to wait for server before grabbing our profile
		{
			DoConnectScreen( (ClientGameLogic*)&gClientLogic(), &ClientGameLogic::wait_GameClosed, gLangMngr.getString("WaitServerCloseGame"), 60.f );
		}
	}

	gClientLogic().Disconnect();

	extern void Unload_Collections();
	Unload_Collections();

	DestroyGame();

	gClientLogic().m_DroppedBomb = NULL;

	if(gameResult != GRESULT_Exit)
	{
		int mainmenuTheme = -1;
		//if(!RUS_CLIENT)
		//	mainmenuTheme = SoundSys.GetEventIDByPath("Sounds/MainMenu GUI/mainmenu_theme");
		//else
			mainmenuTheme = SoundSys.GetEventIDByPath("Sounds/MainMenu GUI/warinc_menu01");
		MainMenuSoundEvent = snd_PlaySound(mainmenuTheme, r3dPoint3D(0,0,0));
	}

	return gameResult;
}
