#include "r3dPCH.h"
#include "r3d.h"
#include "r3dLight.h"
#include "d3dfont.h"

#include "GameObjects/ObjManag.h"

#include "../EclipseStudio/Sources/GameLevel.h"
#include "../EclipseStudio/Sources/GameLevel.cpp"
#include "../EclipseStudio/Sources/GameLevel_IO.cpp"
#include "../EclipseStudio/Sources/ObjectsCode/Gameplay/BattleZone.H"
#include "../../GameEngine/TrueNature/Terrain.h"

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

#include "ServerGameLogic.h"
#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/obj_ServerSiegeObjective.h"
#include "ObjectsCode/obj_ServerBombPlacement.h"
#include "MasterServerLogic.h"
#include "KeepAliveReporter.h"

// just some crap to get server to link
	CD3DFont* 		Font_Label;
	r3dLightSystem WorldLightSystem;
	r3dCamera gCam ;
	float SunVectorXZProj = 0;
	class r3dSun * Sun;
	int CurHUDID = -1;
	float		LakePlaneY = 101.0;

	r3dScreenBuffer* gBuffer_Depth;
	r3dScreenBuffer* ScreenBuffer;
	r3dScreenBuffer* gBuffer_Color;
	r3dScreenBuffer *gScreenSmall;
	r3dScreenBuffer * DepthBuffer;
	float* ShadowSplitDistances;
	ShadowMapOptimizationData gShadowMapOptimizationData [ NumShadowSlices ] ;
	DWORD DriverUpdater(HWND hParentWnd, DWORD VendorId, DWORD v1, DWORD v2, DWORD v3, DWORD v4, DWORD hash) { return hash; }
	void r3dScaleformBeginFrame() {}
	void r3dScaleformEndFrame() {}
	void SetNewSimpleFogParams() {}
	int g_bForceQualitySelectionInEditor = 0;
	void AdvanceLoadingProgress(float) {}
	void SetAdvancedFogParams() {}
	void SetVolumeFogParams() {}
	float ShadowSplitDistancesOpaqueHigh[NumShadowSlices+1] = {0};
	float *ShadowSplitDistancesOpaque = &ShadowSplitDistancesOpaqueHigh[0];
	ShadowMapOptimizationData gShadowMapOptimizationDataOpaque [ NumShadowSlices ];

	r3dCameraAccelerometer gCameraAccelerometer;
// end of temp variables


void sendUsersUpdate(bool forced)
{
  static float nextUpdate = 0.0f;
  if(r3dGetTime() < nextUpdate && !forced) {
    return;
  }
  nextUpdate = r3dGetTime() + 5.0f; // update every 5 sec

  GBUserInfo users[256];
  int num_users = 256;
  gServerLogic.BuildUserList(&num_users, users);
  //r3dOutToLog("user update send: %d\n", num_users);

  gMasterServerLogic.SendGameUpdate(gServerLogic.GetTimeToEnd(), num_users, users);
}

void PlayGameServer()
{
  r3d_assert(gServerLogic.ginfo_.IsValid());
  switch(gServerLogic.ginfo_.mapId) 
  {
	default: r3dError("invalid map id\n");
	case GBGameInfo::MAPID_Editor_Particles: 
		r3dGameLevel::SetHomeDir("Editor_Particles"); 
		break;
	case GBGameInfo::MAPID_WO_Crossroads: 
		r3dGameLevel::SetHomeDir("WO_Crossroads"); 
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
	case GBGameInfo::MAPID_WO_Inferno: 
		r3dGameLevel::SetHomeDir("wo_inferno"); 
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

  r3dResetFrameTime();
  
  GameWorld_Create();

  u_srand(timeGetTime());
  GameWorld().Init(10000);
  
  r3dOutToLog("Loading armory config\n");
  gWeaponArmory.Init();

  g_pPhysicsWorld = new PhysXWorld;
  g_pPhysicsWorld->Init();

  r3dTerrain::SetNeedShaders( false );
  LoadLevel_Objects( 1.f );
  g_BattleZone.LoadBattleZoneGrid( r3dGameLevel::GetHomeDir() );

  GameWorld().Update();

  // check for all needed things for level
  if(gServerLogic.ginfo_.mapType == GBGameInfo::MAPT_Siege)
	  gSiegeObjMgr.CheckForCorrectObjectives();
  else if(gServerLogic.ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	  gBombPlacementMgr.CheckData();
  gCPMgr.CheckForNeededPoints();

  GameWorld().Update();
  r3dGameLevel::SetStartGameTime(r3dGetTime());
  
  // record last net id
  gServerLogic.net_mapLoaded_LastNetID = gServerLogic.net_lastFreeId;

  r3dOutToLog("server main loop started\n");
  gServerLogic.StartGame(true);
  r3dResetFrameTime();
  
  gKeepAliveReporter.SetStarted(true);
  
  while(1) 
  {
    ::Sleep(10);		// limit to 100 FPS
    
    r3dEndFrame();
    r3dStartFrame();
    
    //if(GetAsyncKeyState(VK_F1)&0x8000) r3dError("r3dError test");
    //if(GetAsyncKeyState(VK_F2)&0x8000) r3d_assert(false && "r3d_Assert test");
    gKeepAliveReporter.Tick(gServerLogic.curPlayers_);

    gServerLogic.Tick();
    gMasterServerLogic.Tick();
    
    if(gMasterServerLogic.IsMasterDisconnected()) {
      r3dOutToLog("Master Server disconnected, exiting\n");
      gKeepAliveReporter.SetStarted(false);
      return;
    }

    GameWorld().StartFrame();
    GameWorld().Update();

    // start physics after game world update right now, as gameworld will move some objects around if necessary
    g_pPhysicsWorld->StartSimulation();
    g_pPhysicsWorld->EndSimulation();

    GameWorld().EndFrame();
    
    sendUsersUpdate(false);
    
    if(gServerLogic.gameFinished_)
      break;
  }

  // set that we're finished
  gKeepAliveReporter.SetStarted(false);

  gMasterServerLogic.FinishGame();
  
  gServerLogic.CloseChatLog();
  gServerLogic.UpdatePlayerProfiles();
  gServerLogic.SendGameClose();
  gServerLogic.DumpSimpleStats();
  gServerLogic.DumpPacketStatistics();
  
  GameWorld_Destroy();
  
  return;
}


/*
// unlock achievements
void TPSGameHud_UnlockAchievement(const char* name)
{
	const AchievementConfig* ach=gWeaponArmory.getAchievementConfig(name);
	r3d_assert(ach);
	if(gUserProfile.ProfileData.Achievements[ach->id]==0)
	{
		gUserProfile.ProfileData.Achievements[ach->id]=1;
		gUserProfile.ProfileData.GamePoints += ach->reward;
	}
*/
