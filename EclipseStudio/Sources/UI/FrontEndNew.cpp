#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "r3dDebug.h"

#include "FrontEndNew.h"
#include "GameCode\UserProfile.h"
#include "GameCode\UserFriends.h"
#include "GameCode\UserRewards.h"
#include "GameCode\UserSkills.h"
#include "GameCode\UserClans.h"

#include "CkHttpRequest.h"
#include "CkHttpResponse.h"
#include "backend/HttpDownload.h"
#include "backend/WOBackendAPI.h"

#include "../rendering/Deffered/CommonPostFX.h"
#include "../rendering/Deffered/PostFXChief.h"

#include "multiplayer/MasterServerLogic.h"
#include "multiplayer/LoginSessionPoller.h"

#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/Ammo.h"
#include "../ObjectsCode/weapons/Gear.h"
#include "../ObjectsCode/ai/AI_Player.h"
#include "../ObjectsCode/ai/AI_PlayerAnim.h"
#include "../ObjectsCode/Gameplay/UIWeaponModel.h"
#include "GameLevel.h"
#include "Scaleform/Src/Render/D3D9/D3D9_Texture.h"
#include "../../Eternity/Source/r3dEternityWebBrowser.h"

#include "m_LoadingScreen.h"

#include "HUDDisplay.h"
extern HUDDisplay*	hudMain;

#include "HWInfo.h"

#include "shellapi.h"
#include "SteamHelper.h"
#include "../Editors/CameraSpotsManager.h"
#include "HUD_ShootingGallery.h"

// for IcmpSendEcho
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WINXP
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")

extern	char		_p2p_masterHost[MAX_PATH];
extern	int		_p2p_masterPort;


static const float gTimeToShowAchievementsRibbons = 2.0f;

static float playerRot = 0;
static bool g_sShowExpireMessage = true;

FrontendUI* FrontendUI::This_ = NULL;
bool		FrontendUI::frontendFirstTimeInit = false;
bool		FrontendUI::frontendRequestShowBuyPack = false;
NetPacketsGameBrowser::GBPKT_C2M_QuickGameReq_s FrontendUI::quickGameInfo_;
FrontendUI::FilterGames	FrontendUI::m_FilterGames;

extern r3dScreenBuffer*	Scaleform_RenderToTextureRT;
extern int _render_Terrain;
extern bool IsNeedExit();
extern void ClearFullScreen_Menu();

extern int RUS_CLIENT;
extern int MASSIVE_CLIENT;

const static char* monthNames[12] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December" 
};

float getRatio(float num1, float num2)
{
	if(num1 == 0)
		return 0.0f;
	if(num2 == 0)
		return num1;
	
	return num1/num2;
}

DWORD WINAPI as_AchievementUnlockThread(void* in_ptr)
{
	gUserProfile.RecordFrontEndAchievements();

	return 0;
}

FrontendUI::FrontendUI(const char * movieName, EGameResult gameResult)
: UIMenu(movieName)
, r3dIResource(r3dIntegrityGuardian())
{
	r3d_assert(This_ == NULL);
	This_ = this;

	RTScaleformTexture = NULL;
	RTWelcomeBackScaleformTexture = NULL;
	needReInitScaleformTexture = false;

//	needScaleformReset = gameResult!=GRESULT_Unknown;

	showBuyPackMovie = false;
	needUpdateProfileOnActivate = false;
	needUpdateProfile = false;
	showDisconnectErrorMessage = false;
	showLoadoutWasFixedMsg = false;
	disconnectErrorMsg = NULL;
	if(gameResult == GRESULT_Disconnect)
	{
		showDisconnectErrorMessage = true;
		if(gClientLogic().m_highPingTimer > 60)
			disconnectErrorMsg = gLangMngr.getString("DisconnectMsgPing");		
		else if(gClientLogic().cheatAttemptReceived_ && gClientLogic().cheatAttemptCheatId_ == PKT_S2C_CheatWarning_s::CHEAT_AFK)
			disconnectErrorMsg = gLangMngr.getString("DisconnectMsgAFK");
		else if(gClientLogic().cheatAttemptReceived_)
			disconnectErrorMsg = gLangMngr.getString("DisconnectMsgCheat");
		else
			disconnectErrorMsg = gLangMngr.getString("DisconnectMsg");
	}
	else if(gameResult == GRESULT_Failed_To_Join_Game)
	{
		showDisconnectErrorMessage = true;
		disconnectErrorMsg = gLangMngr.getString("CannotJoinGame");
	}
	showQuickJoin = (gameResult == GRESULT_Finished) || (gameResult==GRESULT_ShownWelcomePackage);
	exitRequested_ = false;
	needUpdateSettings_ = false;
	needExitByGameJoin_ = false;
	m_ArmoryModify_RequireExit = false;
	
	m_Player = 0;
	m_WeaponModel = 0;
	prevServerTimeUpdate = r3dGetTime();
	minToAddToServerTime = 0;

	m_RequestedMysteryBox = NULL;
	m_MysteryBoxItemIdRequested = 0;
	mStore_buyItemID =0;
	mStore_buyItemExp =0;
	mStore_buyItemPrice =0;
	mStore_buyItemPriceGD = 0;
	memset(mStore_ChangeGamertag, 0, sizeof(mStore_ChangeGamertag));

	mArmory_UnlockItemID = 0;
	mArmory_UnlockSlotID = 0;
	mArmory_UnlockUpgradeID = 0;
	mArmory_UnlockIsGP = 0;
	mArmory_UpgradeSuccess = -1;
	
	asyncThread_ = NULL;
	asyncErr_[0] = 0;
	
	lastServerReqTime_ = -1;

	m_pBackgroundTex = NULL;
	//m_pBackgroundPremiumTex = NULL;
	m_hasPremiumAccount = false;
	m_premiumBackgroundFadeIn = 0;

	m_waitingForKeyRemap = -1;
	
	m_friendPopupActive = false;
	m_numFriendsUpdates = 0;

	m_clanPopupActive = false;
	
	m_lbCurPos[0] = -1;
	m_lbCurPos[1] = -1;
	m_lbCurPos[2] = -1;
	m_lbCurPos[3] = -1;
	m_lbTableID   = 0;

	m_NumAchievementsRibbonsInQueue = 0;
	for( int achievementIndex =0; achievementIndex < MAX_FRONTEND_ACHIEVEMENT_QUEUE; achievementIndex++ )
	{
		m_AchievementRibbonQueue[ achievementIndex] = 0;
	}
	m_LastAchievementShownTime = 0;

	m_StartShootingGallery = false;

	extern bool g_bDisableP2PSendToHost;
	g_bDisableP2PSendToHost = true;
}

FrontendUI::~FrontendUI()
{
	r3d_assert(asyncThread_ == NULL);

	This_ = NULL;

	SAFE_RELEASE(RTScaleformTexture);
	SAFE_RELEASE(RTWelcomeBackScaleformTexture);
	
	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
	//if(m_pBackgroundPremiumTex)
	//	r3dRenderer->DeleteTexture(m_pBackgroundPremiumTex);

	if(m_Player)
		GameWorld().DeleteObject(m_Player);
	if(m_WeaponModel)
		GameWorld().DeleteObject(m_WeaponModel);

	extern void DestroyGame();
	DestroyGame();

	extern bool g_bDisableP2PSendToHost;
	g_bDisableP2PSendToHost = false;

	_render_Terrain = 1;
	WorldLightSystem.Destroy();
}

void FrontendUI::SetNeedUpdateSettings()
{
	needUpdateSettings_ = true;
}

void FrontendUI::AddSettingsChangeFlag( DWORD flag )
{
	settingsChangeFlags_ |= flag;
}

static int compRes( const void* r1, const void* r2 )
{
	const r3dDisplayResolution* rr1 = (const r3dDisplayResolution*)r1 ;
	const r3dDisplayResolution* rr2 = (const r3dDisplayResolution*)r2 ;

	return rr1->Width - rr2->Width; // sort resolutions by width
}

void FrontendUI::SyncGraphicsUI()
{
	const DisplayResolutions& reses = r3dRenderer->GetDisplayResolutions();

	DisplayResolutions supportedReses ;

	for( uint32_t i = 0, e = reses.Count(); i < e; i ++ )
	{
		const r3dDisplayResolution& r = reses[ i ];
		float aspect = (float)r.Width / r.Height ;
		supportedReses.PushBack( r );
	}

	if(supportedReses.Count() == 0)
		r3dError("Couldn't find any supported video resolutions. Bad video driver?!\n");

	qsort( &supportedReses[ 0 ], supportedReses.Count(), sizeof supportedReses[ 0 ], compRes );

	gfxMovie.Invoke("_root.api.clearScreenResolutions", "");
	for( uint32_t i = 0, e = supportedReses.Count() ; i < e; i ++ )
	{
		char resString[ 128 ] = { 0 };
		const r3dDisplayResolution& r = supportedReses[ i ] ;
		_snprintf( resString, sizeof resString - 1, "%dx%d", r.Width, r.Height );
		gfxMovie.Invoke("_root.api.addScreenResolution", resString);
	}

	int width	= r_width->GetInt();
	int height	= r_height->GetInt();

	int desktopWidth, desktopHeight ;
	r3dGetDesktopDimmensions( &desktopWidth, &desktopHeight );

	if( !r_ini_read->GetBool() )
	{
		if( desktopWidth < width || desktopHeight < height )
		{
			width = desktopWidth ;
			height = desktopHeight ;
		}
	}

	bool finalResSet = false ;
	int selectedRes = 0;
	for( uint32_t i = 0, e = supportedReses.Count() ; i < e; i ++ )
	{
		const r3dDisplayResolution& r = supportedReses[ i ] ;
		if( width == r.Width && height == r.Height )
		{
			selectedRes = i;
			finalResSet = true;
			break;
		}
	}

	if( !finalResSet )
	{
		int bestSum = 0 ;

		for( uint32_t i = 0, e = supportedReses.Count() ; i < e; i ++ )
		{
			const r3dDisplayResolution& r = supportedReses[ i ] ;

			if( width >= r.Width && 
				height >= r.Height )
			{
				if( r.Width + r.Height > bestSum )
				{
					selectedRes = i;
					bestSum = r.Width + r.Height ;
					finalResSet = true ;
				}
			}
		}
	}

	if( !finalResSet )
	{
		int bestSum = 0x7fffffff ;

		// required mode too small, find smallest mode..
		for( uint32_t i = 0, e = supportedReses.Count() ; i < e; i ++ )
		{
			const r3dDisplayResolution& r = supportedReses[ i ] ;

			if( r.Width + r.Height < bestSum )
			{
				finalResSet = true ;

				selectedRes = i;
				bestSum = r.Width + r.Height ;
			}
		}
	}

	Scaleform::GFx::Value var[30];
	var[0].SetNumber(selectedRes);
	var[1].SetNumber( r_overall_quality->GetInt());
	var[2].SetNumber( r_brightness->GetFloat());
	var[3].SetNumber( r_contrast->GetFloat());
	var[4].SetNumber( s_sound_volume->GetFloat());
	var[5].SetNumber( s_music_volume->GetFloat());
	var[6].SetNumber( s_comm_volume->GetFloat());
	var[7].SetNumber( g_tps_camera_mode->GetInt());
	var[8].SetNumber( g_enable_voice_commands->GetBool());
	var[9].SetNumber( r_antialiasing_quality->GetInt());
	var[10].SetNumber( r_ssao_quality->GetInt());
	var[11].SetNumber( r_terrain_quality->GetInt());
	var[12].SetNumber( r_decoration_quality->GetInt() ); 
	var[13].SetNumber( r_water_quality->GetInt());
	var[14].SetNumber( r_shadows_quality->GetInt());
	var[15].SetNumber( r_lighting_quality->GetInt());
	var[16].SetNumber( r_particles_quality->GetInt());
	var[17].SetNumber( r_mesh_quality->GetInt());
	var[18].SetNumber( r_anisotropy_quality->GetInt());
	var[19].SetNumber( r_postprocess_quality->GetInt());
	var[20].SetNumber( r_texture_quality->GetInt());
	var[21].SetNumber( g_vertical_look->GetBool());
	var[22].SetNumber( 0 ); // not used
	var[23].SetNumber( g_mouse_wheel->GetBool());
	var[24].SetNumber( g_mouse_sensitivity->GetFloat());
	var[25].SetNumber( g_mouse_acceleration->GetBool());
	var[26].SetNumber( g_toggle_aim->GetBool());
	var[27].SetNumber( g_toggle_crouch->GetBool());
	var[28].SetNumber( r_fullscreen->GetInt());
	var[29].SetNumber( r_vsync_enabled->GetInt()+1);

	gfxMovie.Invoke("_root.api.setOptions", var, 30);
 
	gfxMovie.Invoke("_root.api.reloadOptions", "");
}

int FrontendUI::StoreDetectBuyIdx()
{
	int buyIdx = 0;
	if(mStore_buyItemPrice>0)
	{
		if(mStore_buyItemExp == 1)
			buyIdx = 1;
		else if(mStore_buyItemExp == 7)
			buyIdx = 2;
		else if(mStore_buyItemExp == 30)
			buyIdx = 3;
		else if(mStore_buyItemExp == 2000)
			buyIdx = 4;
	}
	else if(mStore_buyItemPriceGD > 0)
	{
		if(mStore_buyItemExp == 1)
			buyIdx = 5;
		else if(mStore_buyItemExp == 7)
			buyIdx = 6;
		else if(mStore_buyItemExp == 30)
			buyIdx = 7;
		else if(mStore_buyItemExp == 2000)
			buyIdx = 8;
	}

	return buyIdx;
}

bool FrontendUI::UpdateInventoryWithBoughtItem()
{
	int numItem = gUserProfile.ProfileData.NumItems;

	// check if we bought consumable
	int quantityToAdd = 1;
	int totalQuantity = 1;
	bool isConsumable = false;
	{
		const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(mStore_buyItemID);
		if(wc)
		{
			if(wc->category == storecat_UsableItem && wc->m_numClips>1)
			{
				quantityToAdd = wc->m_clipSize;
				isConsumable = true;
			}
		}
	}

	// see if we already have this item in inventory
	bool found = false;
	uint32_t itemExp = 0; // for UI, in minutes
	for( int i=0; i<numItem; ++i)
	{
		if(gUserProfile.ProfileData.Inventory[i].itemID == mStore_buyItemID)
		{
			itemExp = (int)ceilf(float(gUserProfile.ProfileData.Inventory[i].expiration)/1440.0f);
			gUserProfile.ProfileData.Inventory[i].expiration += mStore_buyItemExp*1440; // convert from days into minutes
			if(isConsumable)
			{
				gUserProfile.ProfileData.Inventory[i].quantity += quantityToAdd;
				totalQuantity = gUserProfile.ProfileData.Inventory[i].quantity;
			}
			itemExp += mStore_buyItemExp;
			found = true;
			break;
		}
	}
	if(!found)
	{
		wiInventoryItem& itm = gUserProfile.ProfileData.Inventory[gUserProfile.ProfileData.NumItems++];
		itm.expiration = mStore_buyItemExp*1440; // convert from days into minutes
		itm.itemID     = mStore_buyItemID;
		itm.quantity   = quantityToAdd;
		
		totalQuantity = quantityToAdd;
		itemExp = mStore_buyItemExp;
	}

	Scaleform::GFx::Value var[4];
	var[0].SetNumber(mStore_buyItemID);
	var[1].SetNumber(itemExp*1440);
	var[2].SetNumber(totalQuantity);
	var[3].SetBoolean(isConsumable);
	gfxMovie.Invoke("_root.api.addInventoryItem", var, 4);

	updateDefaultAttachments(!found, mStore_buyItemID);

	return found;
}

void FrontendUI::DelayServerRequest()
{
	// allow only one server request per second
	if(r3dGetTime() < lastServerReqTime_ + 1.0f) {
		::Sleep(1000);
	}
	lastServerReqTime_ = r3dGetTime();
}

bool FrontendUI::ConnectToMasterServer()
{
	masterConnectTime_ = r3dGetTime();
	if(gMasterServerLogic.badClientVersion_)
		return false;
	if(gMasterServerLogic.IsConnected())
		return true;
	
	gMasterServerLogic.Disconnect();
	if(!gMasterServerLogic.StartConnect(_p2p_masterHost, _p2p_masterPort))
	{
		This_->SetAsyncError(0, gLangMngr.getString("NoConnectionToMasterServer"));
		return false;
	}

	const float endTime = r3dGetTime() + 30.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);
		//if(gMasterServerLogic.IsConnected())
		//	return true;
        
		if(gMasterServerLogic.versionChecked_ && gMasterServerLogic.badClientVersion_)
			return false;
			
		// if we received server id, connection is ok.
		if(gMasterServerLogic.masterServerId_)
		{
			r3d_assert(gMasterServerLogic.versionChecked_);
			return true;
		}
	
		// early timeout by enet connect fail
		if(!gMasterServerLogic.net_->IsStillConnecting())
			break;
	}
	
	This_->SetAsyncError(8, gLangMngr.getString("TimeoutToMasterServer"));
	return false;
}

void FrontendUI::ParseGameJoinAnswer()
{
	r3d_assert(gMasterServerLogic.gameJoinAnswered_);
	
	// logic for friends game joining with password
	if(browseJoinSlot_ == -2 &&
	   ((gMasterServerLogic.gameJoinAnswer_.result == GBPKT_M2C_JoinGameAns_s::rWrongPassword && !m_friendJoinGameGotPwd) || 
	   (gMasterServerLogic.gameJoinAnswer_.result == GBPKT_M2C_JoinGameAns_s::rLevelTooLow && !m_friendJoinGameOKHighLevelGame)) )
	{
		// return, password will be asked in OnJoinFriendGameSuccess
		return;
	}

	switch(gMasterServerLogic.gameJoinAnswer_.result)
	{
	case GBPKT_M2C_JoinGameAns_s::rOk:
		needExitByGameJoin_ = true;
		return;
	case GBPKT_M2C_JoinGameAns_s::rNoGames:
		SetAsyncError(0, gLangMngr.getString("JoinGameNoGames"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rWrongCreateGameKey:
		SetAsyncError(0, gLangMngr.getString("ProblemCreatingGame"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rHaveCreatedGame:
		SetAsyncError(0, gLangMngr.getString("AlreadyCreatedGame"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rGameFull:
		SetAsyncError(0, gLangMngr.getString("GameIsFull"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rGameFinished:
		SetAsyncError(0, gLangMngr.getString("GameIsAlmostFinished"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rGameNotFound:
		SetAsyncError(0, gLangMngr.getString("GameNotFound"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rWrongPassword:
		SetAsyncError(0, gLangMngr.getString("WrongPassword"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rLevelTooLow:
		SetAsyncError(0, gLangMngr.getString("GameTooLow"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rLevelTooHigh:
		SetAsyncError(0, gLangMngr.getString("GameTooHigh"));
		return;
	case GBPKT_M2C_JoinGameAns_s::rJoinDelayActive:
		SetAsyncError(0, gLangMngr.getString("JoinDelayActive"));
		return;
	}

	wchar_t buf[128];
	swprintf(buf, 128, gLangMngr.getString("UnableToJoinGameCode"), gMasterServerLogic.gameJoinAnswer_.result);
	SetAsyncError(0, buf);
}


// we currently don't use this as the server handles it and to enforce it you force people to change their loadouts for no reason. 
bool FrontendUI::CheckCybersportLimits()
{
	for(int slot = 0; slot < gUserProfile.ProfileData.NumSlots; slot++)
	{
		const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(gUserProfile.ProfileData.ArmorySlots[slot].SecondaryWeaponID);
		if(wc && wc->category == storecat_SUPPORT)
		{
			wchar_t tempstr[256];
			swprintf(tempstr, 256, gLangMngr.getString("Sabotage_NoRPG"), slot + 1);

			Scaleform::GFx::Value args[2];
			args[0].SetStringW(tempstr);
			args[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
			return false;
		}
	}

	return true;
}

unsigned int WINAPI FrontendUI::as_BrowseGamesThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	This_->DelayServerRequest();
	if(!This_->ConnectToMasterServer())
		return 0;

	gMasterServerLogic.RequestGameList();

	const float endTime = r3dGetTime() + 120.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);
		if(gMasterServerLogic.gameListReceived_)
		{
			This_->ProcessSupervisorPings();
			return 1;
		}
		
		if(!gMasterServerLogic.IsConnected())
			break;
	}
	
	This_->SetAsyncError(0, gLangMngr.getString("FailedReceiveGameList"));
	return 0;
}

int FrontendUI::GetSupervisorPing(DWORD ip)
{
	HANDLE hIcmpFile = IcmpCreateFile();
	if(hIcmpFile == INVALID_HANDLE_VALUE) {
		r3dOutToLog("IcmpCreatefile returned error: %d\n", GetLastError());
		return -1;
	}    

	char  sendData[32] = "Data Buffer";
	DWORD replySize   = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
	void* replyBuf    = (void*)_alloca(replySize);
    
	// send 3 pings with 700ms (will take max 2sec)
	DWORD sendResult;
	for(int i=0; i<3; i++) {
		sendResult = IcmpSendEcho(hIcmpFile, ip, sendData, sizeof(sendData), NULL, replyBuf, replySize, 700);
		if(sendResult > 0)
			break;
	}

	IcmpCloseHandle(hIcmpFile);

	if(sendResult == 0) {
		//r3dOutToLog("IcmpSendEcho returned error: %d\n", GetLastError());
		return -2;
	}

        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuf;
        if(pEchoReply->Status == IP_SUCCESS)
        {
		return pEchoReply->RoundTripTime / 2;
        }
        
        r3dOutToLog("IcmpSendEcho returned status %d\n", pEchoReply->Status);
        return -3;
}

void FrontendUI::ProcessSupervisorPings()
{
	memset(&superPings_, 0, sizeof(superPings_));
	
	for(size_t i = 0; i < gMasterServerLogic.supers_.size(); ++i)
	{
		const GBPKT_M2C_SupervisorData_s& super = gMasterServerLogic.supers_[i];
		if(super.ID >= R3D_ARRAYSIZE(superPings_))
			r3dError("Too Many servers, please contact support@thewarinc.com");
			
		int ping = GetSupervisorPing(super.ip);
		superPings_[super.ID] = ping ? ping : 1;
	}
}

int FrontendUI::GetGamePing(DWORD gameId)
{
	// high word of gameId is supervisor Id
	int superId = (gameId >> 16);
	r3d_assert(superId < R3D_ARRAYSIZE(superPings_));
	return superPings_[superId];
}

void FrontendUI::processNewGameList()
{
	int numGames = (int)gMasterServerLogic.games_.size();
	//r3dOutToLog("Got %d games\n", numGames);

	char buf[512];
	int gameCounter = 0;
	for(int i=0; i<numGames; i++) 
	{
		const GBPKT_M2C_GameData_s& gd = gMasterServerLogic.games_[i];
		const GBGameInfo& ginfo = gd.info;

		// process filters
		{
			if(m_FilterGames.conquest || m_FilterGames.deathmatch || m_FilterGames.sabotage) // if all false, then do not filter
			{
				bool gameMode_match = false;
				if(m_FilterGames.conquest && gd.info.mapType == GBGameInfo::MAPT_Conquest)
					gameMode_match = true;
				if(m_FilterGames.deathmatch && gd.info.mapType == GBGameInfo::MAPT_Deathmatch)
					gameMode_match = true;
				if(m_FilterGames.sabotage && gd.info.mapType == GBGameInfo::MAPT_Bomb)
					gameMode_match = true;

				if(!gameMode_match)
					continue;
			}

			if(!RUS_CLIENT && !MASSIVE_CLIENT)
			{
				if(m_FilterGames.regionSelected == 0 && (gd.info.region != GBNET_REGION_US_East && gd.info.region != GBNET_REGION_US_West))
					continue;
				if(m_FilterGames.regionSelected == 1 && gd.info.region != GBNET_REGION_Europe)
					continue;
			}

			if(ginfo.permGameIdx > 0) 
			{
				// special hack for DEV server
				if(gd.info.minLevel == 0 && gd.info.maxLevel == 60)
				{
					// do nothing
				}
				else
				{
					if(m_FilterGames.levelBracket == 0 && !(gd.info.minLevel >= 0 && gd.info.maxLevel < 10))
						continue;
					if(m_FilterGames.levelBracket == 1 && !(gd.info.minLevel >= 10 && gd.info.maxLevel < 20))
						continue;
					if(m_FilterGames.levelBracket == 2 && !(gd.info.minLevel >= 20))
						continue;
				}
			}
			else
			{
				// for user created games, logic is different, as they might have 1-60 levels
				if(m_FilterGames.levelBracket == 0) 
				{
					// do nothing. allow all games
				}
				if(m_FilterGames.levelBracket == 1 && (gd.info.maxLevel <= 10)) // do not show low level games
					continue;
				if(m_FilterGames.levelBracket == 2 && (gd.info.maxLevel <= 20))
					continue;
			}

		}
		// finished filters

		int ping = GetGamePing(gd.gameId);
		if(ping > 0)
			ping = R3D_CLAMP(ping + random(10)-5, 1, 10000);

		const char* region = "";
		if(ginfo.region == GBNET_REGION_US_West)
			region = "[US]";
		else if(ginfo.region == GBNET_REGION_US_East)
			region = "[US]";
		else if(ginfo.region == GBNET_REGION_Europe)
			region = "[EU]";

		char gamename[128];
		if(ginfo.permGameIdx > 0 && (!RUS_CLIENT && !MASSIVE_CLIENT))
			sprintf(gamename, "%s LVL [%d-%d] Game %d", region, ginfo.minLevel, ginfo.maxLevel, gameCounter++);
		else if(ginfo.permGameIdx > 0 && (RUS_CLIENT || MASSIVE_CLIENT))
			sprintf(gamename, "LVL [%d-%d] Game %d", ginfo.minLevel, ginfo.maxLevel, gameCounter++);
		else
			sprintf(gamename, "%s", ginfo.name);
			
		Scaleform::GFx::Value var[13];
		var[0].SetNumber(i);
		var[1].SetString(gamename);
		var[2].SetNumber(ping);
		var[3].SetNumber(gd.curPlayers);
		var[4].SetNumber(ginfo.maxPlayers);
		switch(ginfo.mapId)
		{
		case GBGameInfo::MAPID_WO_Crossroads16:
			var[5].SetStringW(gLangMngr.getString("MapCrossroadsName"));
			var[6].SetString("$Levels/WO_Crossroads16/map_icon_cross16.dds");
			break;
		case GBGameInfo::MAPID_WO_Grozny:
			var[5].SetStringW(gLangMngr.getString("MapEasternFallName"));
			var[6].SetString("$Levels/WO_Grozny/map_icon_grozny.dds");
			break;
		case GBGameInfo::MAPID_WO_Grozny_CQ:
			var[5].SetStringW(gLangMngr.getString("MapEasternFallName"));
			var[6].SetString("$Levels/WO_Grozny_CQ/map_icon_grozny.dds");
			break;
		case GBGameInfo::MAPID_WO_Torn:
			var[5].SetStringW(gLangMngr.getString("MapTornName"));
			var[6].SetString("$Levels/WO_Torn_cq/map_icon_torn.dds");
			break;
		case GBGameInfo::MAPID_WO_Torn_CT:
			var[5].SetStringW(gLangMngr.getString("MapDustName"));
			var[6].SetString("$Levels/WO_Torn_ct/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Inferno:
			var[5].SetStringW(gLangMngr.getString("MapInferno"));
			var[6].SetString("$Levels/wo_inferno/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Jungle02:
			var[5].SetStringW(gLangMngr.getString("MapJungleRuins"));
			var[6].SetString("$Levels/wo_jungleruins/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Citadel_DM:
			var[5].SetStringW(gLangMngr.getString("MapCitadel"));
			var[6].SetString("$Levels/wo_citadel_dm/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Shipyard:
			var[5].SetStringW(gLangMngr.getString("MapShippingYardName"));
			var[6].SetString("$Levels/wo_shippingyard/map_icon_shippingyard.dds");
			break;
		case GBGameInfo::MAPID_WO_Wasteland:
			var[5].SetStringW(gLangMngr.getString("MapWasteland"));
			var[6].SetString("$Levels/wo_wasteland/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_EasternBunkerTDM:
			var[5].SetStringW(gLangMngr.getString("MapInferno")); // use same name
			var[6].SetString("$Levels/wo_eastern_bunker_tdm/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Crossroads2:
			var[5].SetStringW(gLangMngr.getString("MapCrossroads2Name"));
			var[6].SetString("$Levels/WO_Crossroads2/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_TornTown:
			var[5].SetString("torn town");
			var[6].SetString("$Levels/wo_TornTown_tdm/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Nightfall_CQ:
			var[5].SetStringW(gLangMngr.getString("MapNightfallName"));
			var[6].SetString("$Levels/wo_nightfall_cq/map_icon_cross16.dds");
			break;
		case GBGameInfo::MAPID_WO_NightfallPAX:
			var[5].SetStringW(gLangMngr.getString("MapNightfallPAX"));
			var[6].SetString("$Levels/wo_nightfall_PAX/map_icon_cross16.dds");
			break;
		case GBGameInfo::MAPID_BurningSea:
			var[5].SetStringW(gLangMngr.getString("MapBurningSeaName"));
			var[6].SetString("$Levels/wo_burning_sea/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_Valley:
			var[5].SetString("Valley");
			var[6].SetString("$Levels/wo_valley/map_icon.dds");
			break;
		case GBGameInfo::MAPID_WO_TestConquest:
			var[5].SetString("TEST CONQUEST");
			var[6].SetString("");
			break;
		case GBGameInfo::MAPID_WO_TestDeathmatch:
			var[5].SetString("TEST DEATHMATCH");
			var[6].SetString("");
			break;
		case GBGameInfo::MAPID_WO_TestSabotage:
			var[5].SetString("TEST SABOTAGE");
			var[6].SetString("");
			break;
		default:
			var[5].SetString("unknown");
			var[6].SetString("");
			break;
		}
		switch(ginfo.mapType)
		{
		case GBGameInfo::MAPT_Conquest:
			var[7].SetString("Conquest");
			break;
		case GBGameInfo::MAPT_Deathmatch:
			var[7].SetString("Deathmatch");
			break;
		case GBGameInfo::MAPT_Bomb:
			var[7].SetString("Sabotage");
			break;
		default:
			var[7].SetString("unknown");
			break;
		}
		sprintf(buf, "Auto Balance: %s\nFriendly Fire: %s\nMax Players: %d", ginfo.autoBalance ? "Yes" : "No", ginfo.friendlyFire ? "Yes" : "No", (int)ginfo.maxPlayers);
		var[8].SetString(buf);
		var[9].SetBoolean(true);//ginfo.practiceGame==false); // ranked icon

		// reward cap, hard coded on server too
		int rewardCap = 100;
		switch(ginfo.mapId)
		{
		case GBGameInfo::MAPID_WO_Nightfall_CQ:
		case GBGameInfo::MAPID_WO_NightfallPAX:
			rewardCap = 100;
			break;
		case GBGameInfo::MAPID_WO_Crossroads16:
		case GBGameInfo::MAPID_BurningSea:
		case GBGameInfo::MAPID_WO_Crossroads2:
			rewardCap = 100;
			break;
		case GBGameInfo::MAPID_WO_Grozny:
		case GBGameInfo::MAPID_WO_Jungle02:
		case GBGameInfo::MAPID_WO_Citadel_DM:
			rewardCap = 100;
			break;
		case GBGameInfo::MAPID_WO_Shipyard:
		case GBGameInfo::MAPID_WO_Wasteland:
			rewardCap = 25;
			break;
		}
		var[10].SetNumber(rewardCap);

		var[11].SetString("");

		var[12].SetNumber(ginfo.maxLevel);

		gfxMovie.Invoke("_root.api.addGame", var, 13);

		//for(int j=0; j<gd.curPlayers; ++j)
		//{
		//	Scaleform::GFx::Value pvar[6];
		//	pvar[0].SetString(tempName);
		//	pvar[1].SetString(gd.users[j].teamId==1?"red":"blue");
		//	pvar[2].SetString(gd.users[j].gbUserName);
		//	pvar[3].SetNumber(0);
		//	pvar[4].SetNumber(0);
		//	pvar[5].SetNumber(0);
		//	pMovie->Invoke("_root.api.browseGames.addPlayer", pvar, 6);
		//}
	}
}

void FrontendUI::OnGameListReceived()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	processNewGameList();	
	gfxMovie.Invoke("_root.api.refreshGameList", "");
}

void FrontendUI::OnGameListReceived_ServerBrowse()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	processNewGameList();	
	gfxMovie.Invoke("_root.api.refreshServerList", "");
}

unsigned int WINAPI FrontendUI::as_CreateGameThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	// connect first to retreive server id
	This_->DelayServerRequest();
	if(!This_->ConnectToMasterServer())
		return 0;

	// get current game creation key
	DWORD createGameKey = 0;
	r3d_assert(gMasterServerLogic.masterServerId_);
	{
		bool basicGameOrSabotage = This_->createGameInfo_.isBasicGame || This_->createGameInfo_.mapType == GBGameInfo::MAPT_Bomb;
		int apiCode = gUserProfile.ApiGetCreateGameKey(gMasterServerLogic.masterServerId_, &createGameKey, basicGameOrSabotage);
		if(apiCode != 0)
		{
			This_->SetAsyncError(apiCode, gLangMngr.getString("FailedCreatingGame"));
			return 0;
		}
		if(createGameKey == 0)
		{
			if(basicGameOrSabotage)
				This_->SetAsyncError(0, gLangMngr.getString("NeedPremiumOrLevel20ToCreateGame"));
			else
				This_->SetAsyncError(0, gLangMngr.getString("NeedPremiumToCreateGame"));
			return 0;
		} 
	}

	// connect again, in case we was disconnected because API request was too long
	if(!This_->ConnectToMasterServer())
		return 0;

	// set correct game key and send request
	This_->createGameInfo_.createGameKey = createGameKey;
	This_->createGameInfo_.CustomerID = gUserProfile.CustomerID;
	gMasterServerLogic.SendCreateGame(This_->createGameInfo_);

	const float endTime = r3dGetTime() + 60.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);
		if(gMasterServerLogic.gameJoinAnswered_)
		{
			This_->ParseGameJoinAnswer();
			return 1;
		}
		if(!gMasterServerLogic.IsConnected())
			break;
	}

	This_->SetAsyncError(0, gLangMngr.getString("FailedCreatingGame"));
	return 0;
}

unsigned int WINAPI FrontendUI::as_JoinGameThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	// check if we can join this game
	if(This_->browseJoinSlot_ >= 0)
	{
		r3d_assert(gMasterServerLogic.gameListReceived_);
		WORD status = gMasterServerLogic.games_[This_->browseJoinSlot_].status;
		if(status & 1)
		{
			This_->SetAsyncError(0, gLangMngr.getString("GameFinishedCannotJoin"));
			return 0;
		}
		else if(status & 2)
		{
			This_->SetAsyncError(0, gLangMngr.getString("GameHasMaximumPlayers"));
			return 0;
		}
		else if(status & 4)
		{
			This_->SetAsyncError(0, gLangMngr.getString("GameIsAlmostFinished"));
			return 0;
		}
	}

	This_->DelayServerRequest();
	if(!This_->ConnectToMasterServer())
		return 0;
		
	if(This_->browseJoinSlot_ == -1)
	{
		gMasterServerLogic.SendJoinQuickGame(This_->quickGameInfo_);
	}
	else if(This_->browseJoinSlot_ == -2)
	{
		gMasterServerLogic.SendFriendJoinGame(This_->m_friendJoinPacket);
	}
	else
	{
		gMasterServerLogic.SendJoinGame(This_->browseJoinSlot_, This_->m_GameJoin_Password);
		memset(This_->m_GameJoin_Password, 0, sizeof(This_->m_GameJoin_Password)); // reset password
	}

	const float endTime = r3dGetTime() + 60.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);

		if(!gMasterServerLogic.IsConnected())
			break;

		if(gMasterServerLogic.gameJoinAnswered_)
		{
			This_->ParseGameJoinAnswer();
			return 1;
		}
	}
		
	This_->SetAsyncError(0, gLangMngr.getString("TimeoutJoiningGame"));
	return 0;
}

unsigned int WINAPI FrontendUI::as_ChangeGamertagThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mStore_buyItemID!=0);
	r3d_assert(This->mStore_buyItemExp!=0);

	int buyIdx = This->StoreDetectBuyIdx();
	if(buyIdx == 0)
	{
		This->SetAsyncError(-1, L"buy item fail, cannot locate buy index");
		return 0;
	}

	int apiCode = gUserProfile.ApiChangeGamertag(This->mStore_buyItemID, buyIdx, This->mStore_ChangeGamertag);
	if(apiCode != 0)
	{
		if(apiCode == 9)
			This->SetAsyncError(0, gLangMngr.getString("ThisGameTagAlreadyExists"));
		else if(apiCode == 4) 
			This->SetAsyncError(0, gLangMngr.getString("GamerTagNotValid"));
		else if(apiCode == 7) 
			This->SetAsyncError(0, gLangMngr.getString("NotEnougMoneyToBuyItem"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("FailedToBuyItem"));
		return 0;
	}

	return 1;
}

unsigned int WINAPI FrontendUI::as_BuyItemThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mStore_buyItemID!=0);
	r3d_assert(This->mStore_buyItemExp!=0);
		
	int buyIdx = This->StoreDetectBuyIdx();
	if(buyIdx == 0)
	{
		This->SetAsyncError(-1, L"buy item fail, cannot locate buy index");
		return 0;
	}

	// just in case, check that we're not buying loot boxes
	const ItemConfig* ic = gWeaponArmory.getItemConfig(This->mStore_buyItemID);
	r3d_assert(! (ic && (ic->category == storecat_MysteryBox || ic->category == storecat_LootBox)));
	
	int apiCode = gUserProfile.ApiBuyItem(This->mStore_buyItemID, buyIdx);
	if(apiCode != 0)
	{
		if(apiCode == 7) 
			This->SetAsyncError(0, gLangMngr.getString("NotEnougMoneyToBuyItem"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("FailedToBuyItem"));
		return 0;
	}
	
	return 1;
}

unsigned int WINAPI FrontendUI::as_BuyAttachmentThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mStore_buyWpnID!=0);
	r3d_assert(This->mStore_buyItemID!=0);
	r3d_assert(This->mStore_buyItemExp!=0);

	int buyIdx = This->StoreDetectBuyIdx();
	if(buyIdx == 0)
	{
		This->SetAsyncError(-1, L"buy item fail, cannot locate buy index");
		return 0;
	}

	const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(This->mStore_buyItemID);
	r3d_assert(attm);

	int apiCode = gUserProfile.ApiWeaponAttachBuy(This->mStore_buyWpnID, This->mStore_buyItemID, attm->m_type, buyIdx);
	if(apiCode != 0)
	{
		if(apiCode == 7) 
			This->SetAsyncError(0, gLangMngr.getString("NotEnougMoneyToBuyItem"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("FailedToBuyItem"));
		return 0;
	}

	return 1;
}

void FrontendUI::OnBuyItemSuccess()
{
	r3d_assert(mStore_buyItemID!=0);
	r3d_assert(mStore_buyItemExp!=0);
	
	bool skipInventory = false;
	bool showPackageMessage = false;
	bool showGamertagMsg = false;
	bool showStatResetMsg = false;

	if(mStore_buyItemID == 301004) // premium account
	{
		gfxMovie.Invoke("_root.api.unlockCreateGame", 0);
		m_hasPremiumAccount = true;
		m_premiumBackgroundFadeIn = 0.01f;
		needUpdateProfile = true;
	}
	else if(mStore_buyItemID >= 301045 && mStore_buyItemID <= 301049) // gamertag and stats reset
	{
		skipInventory = true;
		if(mStore_buyItemID == 301045)
			showGamertagMsg = true;
		else
			showStatResetMsg = true;
		gUserProfile.GetProfile(); // update profile data

		// update UI
		setBaseStats();
		setAdditionalStats();
		UpdateSkillUI();
	}
	else if(mStore_buyItemID >= 500000 && mStore_buyItemID < 600000) // packages
	{
		skipInventory = true;
		showPackageMessage = true;
		// reload inventory to figure out what server has added into our inventory
		gUserProfile.GetProfile(); // update profile data
		Scaleform::GFx::Value var[4];
		for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
		{
			var[0].SetNumber(gUserProfile.ProfileData.Inventory[i].itemID);
			var[1].SetNumber(gUserProfile.ProfileData.Inventory[i].expiration); //UI expects expiration to be in minutes
			var[2].SetNumber(1);
			var[3].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addInventoryItem", var, 4); // UI will check if item already added and if yes will just update expiration
		}
		needUpdateProfile = true;
	}
	else if(mStore_buyItemID == 301106 || mStore_buyItemID == 301107)
	{
		// Account_Weapon4Xdays or Account_GP2GD
		needUpdateProfile = true;
		skipInventory = true;
	}

	bool isNewItem = true;
	if(!skipInventory)
	{
		isNewItem = !UpdateInventoryWithBoughtItem();
	}

	// in case if we were in welcome back screen
	gfxMovie.Invoke("_root.api.confirmRenew", "");

	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);

	gfxMovie.SetVariable("_root.Main.EquipmentChar.forceItemIDFocus", (int)mStore_buyItemID);
	gfxMovie.SetVariable("_root.Main.EquipmentWeapon.forceItemIDFocus", (int)mStore_buyItemID);
	gfxMovie.Invoke("_root.api.reloadCurrentStoreList", "");

	if(showPackageMessage)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("PackageBoughtSuccess"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}
	else if(showGamertagMsg)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("GamerTagChangeSuccess"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}
	else if(showStatResetMsg)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("StatResetSuccess"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}
}

void FrontendUI::OnBuyAttachmentSuccess()
{
	r3d_assert(mStore_buyWpnID!=0);
	r3d_assert(mStore_buyItemID!=0);
	r3d_assert(mStore_buyItemExp!=0);

	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);

	bool found = false;
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; ++i)
	{
		if(gUserProfile.ProfileData.FPSAttachments[i].WeaponID == mStore_buyWpnID && gUserProfile.ProfileData.FPSAttachments[i].AttachmentID == mStore_buyItemID)
		{
			found = true;
			gUserProfile.ProfileData.FPSAttachments[i].expiration += mStore_buyItemExp*1440;
			break;
		}
	}
	if(!found)
	{
		const WeaponAttachmentConfig* attm1 = gWeaponArmory.getAttachmentConfig(mStore_buyItemID);

		// check if we have anything in that slot, if yes, then DO NOT auto equip new attachment
		bool autoEquipNewAttachment = true;
		for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; ++i)
		{
			wiUserProfile::temp_fps_attach& att = gUserProfile.ProfileData.FPSAttachments[i];
			if(att.WeaponID == mStore_buyWpnID && att.isEquipped)
			{
				const WeaponAttachmentConfig* attm2 = gWeaponArmory.getAttachmentConfig(att.AttachmentID);
				if(attm1->m_type == attm2->m_type)
				{
					//att.isEquipped = false;
					autoEquipNewAttachment = false;
					break;
				}
			}
		}

		int c = gUserProfile.ProfileData.NumFPSAttachments++;
		gUserProfile.ProfileData.FPSAttachments[c].AttachmentID = mStore_buyItemID;
		gUserProfile.ProfileData.FPSAttachments[c].expiration = mStore_buyItemExp*1440;
		gUserProfile.ProfileData.FPSAttachments[c].WeaponID = mStore_buyWpnID;
		gUserProfile.ProfileData.FPSAttachments[c].isEquipped = autoEquipNewAttachment;

		if(autoEquipNewAttachment)
		{
			const WeaponAttachmentConfig* wpnAttachments[WPN_ATTM_MAX] = {0};
			m_WeaponModel->m_CurrentWeapon->getCurrentAttachments(&wpnAttachments[0]);
			wpnAttachments[attm1->m_type] = attm1;
			m_WeaponModel->m_CurrentWeapon->setWeaponAttachments(wpnAttachments);

			Scaleform::GFx::Value var[3];
			var[0].SetNumber(mStore_buyWpnID);
			var[1].SetNumber(attm1->m_type);
			var[2].SetNumber(mStore_buyItemID);
			gfxMovie.Invoke("_root.api.setAttachmentSpec", var, 3);
		}
	}

	gfxMovie.Invoke("_root.api.resetAvailableFPSUpgrades", "");
	addAvailableFPSUpgrades(mStore_buyWpnID);

	gfxMovie.Invoke("_root.api.refreshFPSAttachmentsDlgList", "");

	gfxMovie.Invoke("_root.api.updateFPSUpgradeWindow", NULL, 0);
}

unsigned int WINAPI FrontendUI::as_BuyLootBoxThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mStore_buyItemID!=0);
	r3d_assert(This->mStore_buyItemExp!=0);
		
	const ItemConfig* ic = gWeaponArmory.getItemConfig(This->mStore_buyItemID);
	r3d_assert((ic && (ic->category == storecat_MysteryBox || ic->category == storecat_LootBox)));
	
	int buyIdx = This->StoreDetectBuyIdx();
	int apiCode = gUserProfile.ApiLootBoxBuy(This->mStore_buyItemID, buyIdx);
	if(apiCode != 0)
	{
		if(ic && ic->category == storecat_LootBox)
		{
			if(apiCode == 7) 
				This->SetAsyncError(0, gLangMngr.getString("NotEnougMoneyToUnlockCrate"));
			else
				This->SetAsyncError(apiCode, gLangMngr.getString("FailedToUnlockCrate"));
		}
		else
		{
			if(apiCode == 7) 
				This->SetAsyncError(0, gLangMngr.getString("NotEnougMoneyToBuyItem"));
			else
				This->SetAsyncError(apiCode, gLangMngr.getString("FailedToBuyItem"));
		}
		return 0;
	}
	
	return 1;
}

void FrontendUI::OnBuyLootBoxSuccess()
{
	r3d_assert(mStore_buyItemID!=0);

	uint32_t mysteryBoxID = mStore_buyItemID;
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	// if it was loot box, update quantity and inventory
	const ItemConfig* ic = gWeaponArmory.getItemConfig(mStore_buyItemID);
	if(ic && (ic->category == storecat_LootBox))
	{
		for(uint32_t i =0; i<gUserProfile.ProfileData.NumItems; ++i)
		{
			if(gUserProfile.ProfileData.Inventory[i].itemID == mStore_buyItemID)
			{
				gUserProfile.ProfileData.Inventory[i].quantity--;
				
				Scaleform::GFx::Value var[4];
				var[0].SetNumber(gUserProfile.ProfileData.Inventory[i].itemID);
				var[1].SetNumber(gUserProfile.ProfileData.Inventory[i].expiration);
				var[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
				var[3].SetBoolean(false);
				gfxMovie.Invoke("_root.api.addInventoryItem", var, 4);

				break;
			}
		}
		gfxMovie.Invoke("_root.api.refreshArmoryInventory", (int)mStore_buyItemID);
	}

	if(gUserProfile.lastMysteryWin_.ItemID > 0) 
	{
		// won item: change parameters, so update inventory code will add correct item
		mStore_buyItemID  = gUserProfile.lastMysteryWin_.ItemID;
		mStore_buyItemExp = gUserProfile.lastMysteryWin_.ExpDays;
		UpdateInventoryWithBoughtItem();

		gfxMovie.Invoke("_root.api.reloadCurrentStoreList", "");
	}

	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);

	//show mystery win with item/gd from gUserProfile.lastMysteryWin_
	Scaleform::GFx::Value var[4];
	var[0].SetNumber(mysteryBoxID);
	var[1].SetNumber(gUserProfile.lastMysteryWin_.ItemID);
	var[2].SetNumber(gUserProfile.lastMysteryWin_.ExpDays*1440);
	var[3].SetNumber(gUserProfile.lastMysteryWin_.GD);
	gfxMovie.Invoke("_root.api.showMysteryBoxWin", var, 4);
	
	return;
}

unsigned int WINAPI FrontendUI::as_SellLootBoxThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mStore_buyItemID!=0);
		
	int apiCode = gUserProfile.ApiLootBoxSell(This->mStore_buyItemID);
	if(apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("ApiError"));
		return 0;
	}
	
	return 1;
}

void FrontendUI::OnSellLootBoxSuccess()
{
	r3d_assert(mStore_buyItemID!=0);
	
	// decrease that item quantity in inventory and update GD
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; i++) 
	{
		if(gUserProfile.ProfileData.Inventory[i].itemID == mStore_buyItemID) 
		{
			gUserProfile.ProfileData.Inventory[i].quantity--;

			Scaleform::GFx::Value var[4];
			var[0].SetNumber(gUserProfile.ProfileData.Inventory[i].itemID);
			var[1].SetNumber(gUserProfile.ProfileData.Inventory[i].expiration);
			var[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
			var[3].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addInventoryItem", var, 4);

			break;
		}
	}
	gfxMovie.Invoke("_root.api.refreshArmoryInventory", (int)mStore_buyItemID);

	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);
	
	return;
}

void FrontendUI::OnBuyPremiumAccountSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	OnBuyItemSuccess();

	gfxMovie.Invoke("_root.api.unlockCreateGame", 1);
	m_hasPremiumAccount = true;
	m_premiumBackgroundFadeIn = 0.01f;
}


unsigned int WINAPI FrontendUI::as_UpdateLoadout(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	int apiCode = gUserProfile.ApiModifyLoadoutSlot(This->m_CurrentModifySlot);
	if(apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToModifyLoadoutSlot"));
		return 0;
	}
	
	return 1;
}

void FrontendUI::OnUpdateLoadoutSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	UpdateLoadoutUI();
	wiWeaponAttachments emptyAttm;
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[m_CurrentModifySlot], emptyAttm);

	if(m_ArmoryModify_RequireExit)
	{
		m_ArmoryModify_RequireExit = false;
		gfxMovie.Invoke("_root.api.armoryFinishedSavingLoadoutSlotOnExit", "");

		// retarded scaleform doesn't want to send event for some reason, hard coding it here
		m_needPlayerRenderingRequest = 3;
	}

	return;
}

unsigned int WINAPI FrontendUI::as_GNAGetBalanceThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	r3d_assert(RUS_CLIENT);
	int apiCode = gUserProfile.ApiGNAGetBalance();
	if(apiCode != 0)
	{
		//no need to display error here
		//This->SetAsyncError(apiCode, gLangMngr.getString("FailedToModifyLoadoutSlot"));
		return 0;
	}

	return 1;
}

void FrontendUI::OnGNAGetBalanceSuccess()
{
	r3d_assert(RUS_CLIENT);
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	return;
}

unsigned int WINAPI FrontendUI::as_SteamBuyGPThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	int apiCode = gUserProfile.ApiSteamStartBuyGP(This->m_steamGpItemId);
	if(apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("FailedToBuyItem"));
		return 0;
	}
	
	// wait for authorization result - for 10min and pray that callback successfully passed to client
	const float endTime = r3dGetTime() + 600.0f;
	while(true)
	{
		::Sleep(10);
		if(gUserProfile.steamAuthResp.gotResp)
			break;
			
		// if we didn't got steam callback
		if(r3dGetTime() > endTime) {
			This->SetAsyncError(0, gLangMngr.getString("FailedToBuyItem"));
			return 0;
		}
	}

	// if authorization was denied, just silently quit
	r3d_assert(gUserProfile.steamAuthResp.gotResp);
	if(gUserProfile.steamAuthResp.bAuthorized != 1)
		return 1;
			
	// auth ok, finish transaction.
	apiCode = gUserProfile.ApiSteamFinishBuyGP(gUserProfile.steamAuthResp.ulOrderID);
	if(apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("FailedToBuyItem"));
		return 0;
	}
	
	return 1;
}

void FrontendUI::OnSteamBuyGPSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	return;
}

void FrontendUI::OnSteamBuyPackSuccess()
{
	showBuyPackMovie = false;
	needUpdateProfile = true;

	gfxBuyPackMovie.Invoke("_global.hideInfoMsg", "");
	return;
}


void writeGameOptionsFile();

bool NeedUploadReport(const CHWInfo& hw)
{
	// always send report if hardware is not supported
	if(hw.gfxErrors)
		return true;

	// query for game registry node
	HKEY hKey;
	int hr;
	hr = RegOpenKeyEx(HKEY_CURRENT_USER, 
		"Software\\Arktos Entertainment Group\\War Inc Battlezone", 
		0, 
		KEY_ALL_ACCESS, 
		&hKey);
	if(hr != ERROR_SUCCESS)
		return true;

	__int64 repTime = 0;
	DWORD size = sizeof(repTime);
	hr = RegQueryValueEx(hKey, "UpdaterTime2", NULL, NULL, (BYTE*)&repTime, &size);
	RegCloseKey(hKey);

	if(hr == ERROR_SUCCESS)
	{
		__int64 time = _time64(NULL);
		// report every 2 days
		if(time < repTime + (60 * 60 * 24 * 2))
			return false;
	}

	return true;
}

static volatile LONG gProfileIsAquired = 0;
static volatile LONG gProfileOK = 0;
static volatile LONG gProfileLoadStage = 0;

// static calendar info, donwloaded only once
//volatile bool FrontendUI::gHasCalendar = false;
//FrontendUI::EventCalendar FrontendUI::m_CalendarEvents[32];
//int FrontendUI::m_numCalendarEvents = 0;

extern CHWInfo g_HardwareInfo;

static void SetLoadStage(const char* stage)
{
	const static char* sname = NULL;
	static float stime = 0;
#ifndef FINAL_BUILD	
	if(sname) 
	{
		r3dOutToLog("SetLoadStage: %4.2f sec in %s\n", r3dGetTime() - stime, sname);
	}
#endif

	sname = stage;
	stime = r3dGetTime();
	gProfileLoadStage++;
}

#define LOAD_DATA_IN_BACKGROUND 1
static void LoadFrontendGameData(FrontendUI* UI)
{
	//
	// load shooting gallery
	//
	SetLoadStage("Shooting Range");
	{
		extern void DoLoadGame(const char* LevelFolder, int MaxPlayers, bool unloadPrev );
		DoLoadGame(r3dGameLevel::GetHomeDir(), 4, true );
	}

	//
	// create player and FPS weapon
	//
	SetLoadStage("Player Model");
	{
		obj_AI_Player* plr = (obj_AI_Player *)srv_CreateGameObject("obj_AI_Player", "Player", r3dPoint3D(0,0,0));
		plr->TeamID = TEAM_BLUE;
		plr->PlayerState = PLAYER_IDLE;
		plr->bDead = 0;
		plr->LoadoutSlot = 0;
		plr->CurLoadout = gUserProfile.ProfileData.ArmorySlots[0];
		plr->m_disablePhysSkeleton = true;
		plr->m_fPlayerRotationTarget = plr->m_fPlayerRotation = playerRot;

		// we need it to be created as a networklocal character for physics.
		plr->NetworkLocal = true;
		plr->OnCreate();
		plr->NetworkLocal = false;
		// switch player to UI idle mode
		plr->uberAnim_->IsInUI = true;
		plr->uberAnim_->AnimPlayerState = -1;

		// create FPS weapon
		UIWeaponModel* wpn = (UIWeaponModel*)srv_CreateGameObject("UIWeaponModel", "weapon", r3dPoint3D(0,0,0));

		UI->SetLoadedThings(plr, wpn);
	}
}

static bool ActualGetProfileData(FrontendUI* UI)
{
	gProfileLoadStage = 0;

	#ifdef LOAD_DATA_IN_BACKGROUND
	// need to load game data first, because of DestroyGame() in destructor
	LoadFrontendGameData(UI);
	#endif

	SetLoadStage("ApiGetShopData");
	if(gUserProfile.ApiGetShopData() != 0)
		return false;
		
	// get game rewards from server.
	SetLoadStage("ApiGameRewards");
	if(g_GameRewards == NULL)
		g_GameRewards = new CGameRewards();
	if(!g_GameRewards->loaded_) {
		if(g_GameRewards->ApiGetDataGameRewards() != 0) {
			return false;
		}
	}
		
	// update items info only once and do not check for errors
	static bool gotCurItemsData = false;
	SetLoadStage("ApiGetItemsInfo");
	if(!gotCurItemsData) {
		gotCurItemsData = true;
		gUserProfile.ApiGetItemsInfo();
	}

	SetLoadStage("GetProfile");
	if(gUserProfile.GetProfile() != 0)
		return false;

	// get clan info, if any
	SetLoadStage("GetClans");
	if(gUserProfile.ProfileData.ClanID!=0)
	{
		gUserProfile.clans->ApiClanGetInfo(gUserProfile.ProfileData.ClanID, &gUserProfile.clans->clanInfo_, &gUserProfile.clans->clanMembers_);
	}

	SetLoadStage("IsFPSEnabled checks");
	if(gUserProfile.ProfileData.IsFPSEnabled)
	{
		g_camera_mode->SetInt(2);
		// check for missing default attachments
#ifndef FINAL_BUILD
		float t1 = r3dGetTime();
#endif
		wiUserProfile& pd = gUserProfile.ProfileData;
		for(uint32_t i=0; i<pd.NumItems; ++i)
		{
			if(const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(pd.Inventory[i].itemID)) // go through all weapons
			{
				for(int j=0; j<WPN_ATTM_MAX; ++j) 
				{
					if(wpn->FPSDefaultID[j]>0)// check for missing default attachments
					{
						bool found = false;
						for(uint32_t k=0; k<pd.NumFPSAttachments; ++k) 
						{
							if(pd.FPSAttachments[k].WeaponID == wpn->m_itemID && gUserProfile.ProfileData.FPSAttachments[k].AttachmentID == wpn->FPSDefaultID[j])
							{
								found = true;
								break;
							}
						}
						if(!found) // if not found, let's manually add it
						{
							int apiCode = gUserProfile.ApiWeaponAttachFixDefaults(wpn->m_itemID);
							if(apiCode != 0)
							{
								r3dOutToLog("Failed to fix default attachments, code=%d\n", apiCode);
							}
							else
							{
								// manually add missing default attachments
								for(int h=0; h<WPN_ATTM_MAX; ++h) 
								{
									bool foundAttm = false;
									for(uint32_t k=0; k<pd.NumFPSAttachments; ++k)
									{
										if(pd.FPSAttachments[k].WeaponID == wpn->m_itemID && pd.FPSAttachments[k].AttachmentID == wpn->FPSDefaultID[h])
										{
											foundAttm = true;
											break;
										}
									}
									if(!foundAttm)
									{
										pd.FPSAttachments[pd.NumFPSAttachments++] = wiUserProfile::temp_fps_attach(wpn->m_itemID, wpn->FPSDefaultID[h], 2000*1440, 0);
									}
								}
							}
							break; // break out of main loop, as API function will add all default attachments
						}
					}
				}
			}
		}

		// check for expired attachments and replace them with default attachments
		for(uint32_t i=0; i<pd.NumItems; ++i)
		{
			if(const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(pd.Inventory[i].itemID)) // go through all weapons
			{
				int		curAttmsInstalled[WPN_ATTM_MAX];	
				memset(curAttmsInstalled, 0, sizeof(curAttmsInstalled));
				for(uint32_t k=0; k<pd.NumFPSAttachments; ++k) // collect what attachments are currently equipped
				{
					if(pd.FPSAttachments[k].WeaponID == wpn->m_itemID && pd.FPSAttachments[k].isEquipped)
					{
						const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(pd.FPSAttachments[k].AttachmentID);
						if(attm)
							curAttmsInstalled[attm->m_type] = pd.FPSAttachments[k].AttachmentID;
					}
				}
				for(int j=0; j<WPN_ATTM_MAX; ++j) // check for missing attachments that have something in default slot
				{
					if(curAttmsInstalled[j] == 0 && wpn->FPSDefaultID[j]>0)
					{
						int apiCode = gUserProfile.ApiWeaponAttachEquip(wpn->m_itemID, wpn->FPSDefaultID[j], j);
						if(apiCode != 0)
						{
							r3dOutToLog("Failed to equip default attachment after prev.one expired\n");
						}
						else
						{
							for(uint32_t k=0; k<pd.NumFPSAttachments; ++k)
							{
								if(pd.FPSAttachments[k].WeaponID == wpn->m_itemID && pd.FPSAttachments[k].AttachmentID == wpn->FPSDefaultID[j])
								{
									pd.FPSAttachments[k].isEquipped = true;
									break;
								}
							}
						}
					}
				}
			}
		}
#ifndef FINAL_BUILD
		r3dOutToLog("Fixing attachments took: %.1fsec\n", r3dGetTime() - t1);
#endif
	}
	else
	{
		if(g_camera_mode->GetInt()==2) // reset
			g_camera_mode->SetInt(g_tps_camera_mode->GetInt());
	}

	SetLoadStage("ApiSteamGetShop");
	if(gSteam.inited_)
		gUserProfile.ApiSteamGetShop();

	// get daily rewards only once, as we are showing them only when you login into game
	SetLoadStage("ApiRetBonusGetInfo");
	if(FrontendUI::frontendFirstTimeInit)
	{
		gUserProfile.ApiRetBonusGetInfo();
	}

	//
	// retreive friends status
	//
	SetLoadStage("ApiFriendGetStats");
	gUserProfile.friends->friendsPrev_.clear();
	if(!gUserProfile.friends->gotNewData)
		gLoginSessionPoller.ForceTick();
	const float waitEnd = r3dGetTime() + 20.0f;
	while(r3dGetTime() < waitEnd)
	{
		if(gUserProfile.friends->gotNewData) 
		{
			// fetch your friends statistics
			gUserProfile.ApiFriendGetStats(0);
			break;
		}
	}

	// get event calendar
	/*SetLoadStage("Calendar");
	if(!UI->gHasCalendar)
	{
		try
		{
			const char* calendarUrl = "https://api1.thewarinc.com/api_getEventCalendar.xml";
			if(RUS_CLIENT)
				calendarUrl = "https://warinc-api.gamenet.ru/php/api_getEventCalendar.xml";

			CkByteData xmlData;
			HttpDownload http;
			if(!http.Get(calendarUrl, xmlData)) {
				throw "failed to get calendar data";
			}

			pugi::xml_document xmlFile;
			pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace((void*)xmlData.getBytes(), xmlData.getSize());
			if(!parseResult) {
				throw "failed to parse";
			}

			pugi::xml_node xmlNode = xmlFile.child("Calendar").first_child();
			while(!xmlNode.empty())
			{
				if(strcmp(xmlNode.name(), "Event") == 0) 
				{
					if(UI->m_numCalendarEvents < 32)
					{
						UI->m_CalendarEvents[UI->m_numCalendarEvents].month = xmlNode.attribute("month").as_int()-1; // convert from normal month to a tm_mon variable which is [0..11]
						UI->m_CalendarEvents[UI->m_numCalendarEvents].day = xmlNode.attribute("day").as_int();
						UI->m_CalendarEvents[UI->m_numCalendarEvents].text = strdup(xmlNode.attribute("text").value());
						UI->m_numCalendarEvents++;
					}
				} 
				xmlNode = xmlNode.next_sibling();
			}
		}
		catch (const char* err)
		{
			// catch r3dError
			r3dOutToLog("get event calendar error: %s\n", err);
		}

		UI->gHasCalendar = true;	// mark that we have it, even with error
	}*/

	// send HW report if necessary
	SetLoadStage("HWReport");
	if(FrontendUI::frontendFirstTimeInit)
	{
		if(NeedUploadReport(g_HardwareInfo))
		{
			CWOBackendReq req(&gUserProfile, "api_ReportHWInfo_Customer.aspx");
			char buf[1024];
			sprintf(buf, "%I64d", g_HardwareInfo.uniqueId);
			req.AddParam("r00", buf);
			req.AddParam("r10", g_HardwareInfo.CPUString);
			req.AddParam("r11", g_HardwareInfo.CPUBrandString);
			sprintf(buf, "%d", g_HardwareInfo.CPUFreq);
			req.AddParam("r12", buf);
			sprintf(buf, "%d", g_HardwareInfo.TotalMemory);
			req.AddParam("r13", buf);

			sprintf(buf, "%d", g_HardwareInfo.DisplayW);
			req.AddParam("r20", buf);
			sprintf(buf, "%d", g_HardwareInfo.DisplayH);
			req.AddParam("r21", buf);
			sprintf(buf, "%d", g_HardwareInfo.gfxErrors);
			req.AddParam("r22", buf);
			sprintf(buf, "%d", g_HardwareInfo.gfxVendorId);
			req.AddParam("r23", buf);
			sprintf(buf, "%d", g_HardwareInfo.gfxDeviceId);
			req.AddParam("r24", buf);
			req.AddParam("r25", g_HardwareInfo.gfxDescription);

			req.AddParam("r30", g_HardwareInfo.OSVersion);

			if(!req.Issue())
			{
				r3dOutToLog("Failed to upload HW Info\n");
			}
			else
			{
				// mark that we reported it
				HKEY hKey;
				int hr;
				hr = RegCreateKeyEx(HKEY_CURRENT_USER, 
					"Software\\Arktos Entertainment Group\\War Inc Battlezone", 
					0, 
					NULL,
					REG_OPTION_NON_VOLATILE, 
					KEY_ALL_ACCESS,
					NULL,
					&hKey,
					NULL);
				if(hr == ERROR_SUCCESS)
				{
					__int64 repTime = _time64(NULL);
					DWORD size = sizeof(repTime);

					hr = RegSetValueEx(hKey, "UpdaterTime2", NULL, REG_QWORD, (BYTE*)&repTime, size);
					RegCloseKey(hKey);
				}
			}
		}
	}

	SetLoadStage(NULL);
	return true;
}

static unsigned int WINAPI GetProfileDataThread( void * FrontEnd )
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	r3dRandInitInTread rand_in_thread;

	try 
	{
		gProfileOK = 0;
		if(ActualGetProfileData((FrontendUI*)FrontEnd))
			gProfileOK = 1;
	}
	catch(const char* err)
	{
		// catch r3dError
		r3dOutToLog("GetProfileData error: %s\n", err);
	}
		
	InterlockedExchange( &gProfileIsAquired, 1 );

	return 0;
}

void FrontendUI::eventArmorySelectLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int loadoutID = (int)args[0].GetNumber();
	m_CurrentModifySlot = loadoutID;
	wiWeaponAttachments emptyAttm;
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[m_CurrentModifySlot], emptyAttm);
	UpdateSkillUI();
}

void updateIfHasInventory(int& slot, int itemID)
{
	if(itemID == 0)
		slot = itemID;
	else if(gUserProfile.isValidInventoryItem(itemID))
		slot = itemID;
}

void FrontendUI::eventArmorySaveLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 13);
	//slotIdx, slot0, slot1, slot2, slot3, slot4, slot5, slot6, slot7, slot8, slot9, slot10

	int loadoutID = (int)args[0].GetNumber();
	m_CurrentModifySlot = loadoutID;
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].PrimaryWeaponID, (int)args[1].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].SecondaryWeaponID, (int)args[2].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].SidearmWeaponID, (int)args[3].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].Item1, (int)args[4].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].Item2, (int)args[5].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].Item3, (int)args[6].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].Item4, (int)args[7].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].BodyMeshID, (int)args[8].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].BodyHeadID, (int)args[9].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].BodyArmorID, (int)args[10].GetNumber());
	updateIfHasInventory(gUserProfile.ProfileData.ArmorySlots[loadoutID].BodyHeadGearID, (int)args[11].GetNumber());
	m_ArmoryModify_RequireExit = args[12].GetBool();

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("SavingLoadoutSlot"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	StartAsyncOperation(&FrontendUI::as_UpdateLoadout, &FrontendUI::OnUpdateLoadoutSuccess);
}

void FrontendUI::eventArmoryModifyOnEquip(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2);

	int itemID = (int)args[0].GetNumber();
	int slotID = (int)args[1].GetNumber();

	if(slotID == 0)
		m_Player->CurLoadout.PrimaryWeaponID = itemID;
	else if(slotID == 1)
		m_Player->CurLoadout.SecondaryWeaponID = itemID;
	else if(slotID == 2)
		m_Player->CurLoadout.SidearmWeaponID = itemID;
	else if(slotID == 3)
		m_Player->CurLoadout.Item1 = itemID;
	else if(slotID == 4)
		m_Player->CurLoadout.Item2 = itemID;
	else if(slotID == 5)
		m_Player->CurLoadout.Item3 = itemID;
	else if(slotID == 6)
		m_Player->CurLoadout.Item4 = itemID;
	else if(slotID == 7)
		m_Player->CurLoadout.BodyMeshID = itemID;
	else if(slotID == 9)
		m_Player->CurLoadout.BodyArmorID = itemID;
	else if(slotID == 8)
		m_Player->CurLoadout.BodyHeadID = itemID;
	else if(slotID == 10)
		m_Player->CurLoadout.BodyHeadGearID = itemID;
	else
		r3dError("Unknown slot in equip: %d\n", slotID);

	wiWeaponAttachments emptyAttm;
	int currentSlot = m_Player->m_SelectedWeapon;
	
	// need to remember previous animation stack and replace it after UpdateLoadoutSlot, as it'll switch to first weapon automatically.
	std::vector<r3dAnimation::r3dAnimInfo> AnimTracks = m_Player->uberAnim_->anim.AnimTracks;
	m_Player->UpdateLoadoutSlot(m_Player->CurLoadout, emptyAttm);
	// restore animation and switch to selected slot
	m_Player->uberAnim_->anim.AnimTracks = AnimTracks;
	m_Player->ChangeWeaponByIndex(currentSlot);
}

void FrontendUI::eventArmoryCurrentWeaponSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int slotID = (int)args[0].GetNumber();
	m_Player->ChangeWeaponByIndex(slotID);
}

void FrontendUI::eventArmoryRequestCharacterRender(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int request = (int)args[0].GetNumber();
	m_needPlayerRenderingRequest = request;
}

void FrontendUI::eventArmoryRequestUnlockLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2);

	int slot = (int)args[0].GetNumber();
	int specIdx = (int)args[1].GetNumber();

	r3d_assert(slot == gUserProfile.ProfileData.NumSlots);
	if(gUserProfile.ShopUnlockLoadoutCost == 0)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString("Sorry, loadout slot price is not set yet");
		args[1].SetBoolean(true);
		pMovie->Invoke("_root.api.showInfoMsg", args, 2);		
		return;
	}

	if(gUserProfile.ProfileData.Stats.GamePoints < gUserProfile.ShopUnlockLoadoutCost)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString("NotEnoughMoneyToUnlockSlot"));
		args[1].SetBoolean(true);
		pMovie->Invoke("_root.api.showInfoMsg", args, 2);		
		return;
	}

	int apiCode = gUserProfile.ApiUnlockLoadoutSlot2(specIdx);
	if(apiCode == 0)
	{
		UpdateLoadoutUI();
		updateInventoryAndSkillItems();
		updateAllAttachments();
		pMovie->Invoke("_root.api.hideInfoMsg", "");
		pMovie->Invoke("_root.api.confirmLoadoutSlotUnlock", "");
		pMovie->Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
	}
	else
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString("NotEnoughMoneyToUnlockSlot"));
		args[1].SetBoolean(true);
		pMovie->Invoke("_root.api.showInfoMsg", args, 2);		
	}
}

void FrontendUI::eventArmoryOnOpenUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(false);
}

void FrontendUI::eventArmoryUpgradeUnlockSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(false);
}

void FrontendUI::eventArmoryUpgradeSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(false);
}

void FrontendUI::addAvailableFPSUpgrades(int wpnID)
{
	const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(wpnID);
	r3d_assert(wpn);

	for(int i=0; i<gWeaponArmory.getNumAttachments(); ++i)
	{
		const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfigByIndex(i);
		r3d_assert(attm);

		if((wpn->FPSSpecID[attm->m_type] > 0 && wpn->FPSSpecID[attm->m_type] == attm->m_specID) || wpn->FPSDefaultID[attm->m_type] == attm->m_itemID)
		{
			float expiration = 0.0f;
			for(uint32_t j=0; j<gUserProfile.ProfileData.NumFPSAttachments; ++j)
			{
				const wiUserProfile::temp_fps_attach& fpsAttm = gUserProfile.ProfileData.FPSAttachments[j];
				if(fpsAttm.WeaponID == wpnID && fpsAttm.AttachmentID == attm->m_itemID)
				{
					expiration = (float)fpsAttm.expiration;
					break;
				}
			}
			bool isDefaultAttm = false;
            if(wpn->FPSDefaultID[attm->m_type] == attm->m_itemID) // default attm - mark as permanent
			{
				expiration = 2000.0f*1440.0f;
				isDefaultAttm = true;
			}

			bool hasPrice = false;
			for(uint32_t j=0; j<g_NumStoreItems; ++j)
			{
				if(g_StoreItems[j].itemID == attm->m_itemID)
				{
					hasPrice = g_StoreItems[j].hasAnyPrice();
					break;
				}
			}
			if(expiration > 0 || hasPrice)
			{
				Scaleform::GFx::Value var[4];
				var[0].SetNumber(attm->m_type);
				var[1].SetNumber(attm->m_itemID);
				var[2].SetNumber(expiration); // in minutes
				var[3].SetBoolean(isDefaultAttm);
				gfxMovie.Invoke("_root.api.addAvailableFPSUpgrade", var, 4);
			}
		}
	}
}

void FrontendUI::eventArmoryOnOpenFPSUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int wpnID = (int)args[0].GetNumber();
	addAvailableFPSUpgrades(wpnID);

	m_Player->m_enableRendering = false;
	m_WeaponModel->enableRendering = true;
	m_WeaponModel->setWeaponModel(wpnID, m_Player);
	const WeaponAttachmentConfig* wpnAttachments[WPN_ATTM_MAX] = {0};
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; ++i)
	{
		if(gUserProfile.ProfileData.FPSAttachments[i].WeaponID == wpnID && gUserProfile.ProfileData.FPSAttachments[i].isEquipped)
		{
			const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(gUserProfile.ProfileData.FPSAttachments[i].AttachmentID);
			r3d_assert(attm);
			wpnAttachments[attm->m_type] = attm;
		}
	}
	m_WeaponModel->m_CurrentWeapon->setWeaponAttachments(wpnAttachments);

	gfxMovie.Invoke("_root.api.showFPSUpgradeWindow", NULL, 0);
}

void FrontendUI::eventArmoryOnCloseFPSUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2);

	int wpnID = (int)args[0].GetNumber();
	bool needSave = args[1].GetBool();

	m_WeaponModel->enableRendering = false;
	m_Player->m_enableRendering = true;

	gfxMovie.Invoke("_root.api.onCloseFPSUpgradeWindow", NULL, 0);
}

void FrontendUI::eventArmoryOnEquipAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 3);

	int wpnID = (int)args[0].GetNumber();
	int attmID = (int)args[1].GetNumber();
    int attmSlot = (int)args[2].GetNumber();

	const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(wpnID);
	r3d_assert(wpn);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	// if trying to remove attm from slot that has default attm, then set it to default attm
	if(attmID == 0)
	{
		if(wpn->FPSDefaultID[attmSlot] >0 )
		{
			attmID = wpn->FPSDefaultID[attmSlot];
		}
	}

	mArmory_EquipWpnID = wpnID;
	mArmory_EquipAttmID = attmID;
	mArmory_EquipAttmSlot = attmSlot;
	StartAsyncOperation(&FrontendUI::as_EquipAttachmentThread, &FrontendUI::OnEquipAttachmentSuccess);
}

unsigned int WINAPI FrontendUI::as_EquipAttachmentThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	r3d_assert(This->mArmory_EquipWpnID!=0);

	int apiCode = gUserProfile.ApiWeaponAttachEquip(This->mArmory_EquipWpnID, This->mArmory_EquipAttmID, This->mArmory_EquipAttmSlot);
	if(apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToEquipAttachment"));
		return 0;
	}

	return 1;
}

void FrontendUI::OnEquipAttachmentSuccess()
{
	r3d_assert(mArmory_EquipWpnID!=0);

	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	// remove old slot
	const WeaponAttachmentConfig* wpnAttachments[WPN_ATTM_MAX] = {0}; 
	m_WeaponModel->m_CurrentWeapon->getCurrentAttachments(&wpnAttachments[0]);
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; ++i)
	{
		if(gUserProfile.ProfileData.FPSAttachments[i].WeaponID == mArmory_EquipWpnID && gUserProfile.ProfileData.FPSAttachments[i].isEquipped)
		{
			const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(gUserProfile.ProfileData.FPSAttachments[i].AttachmentID);
			if(attm->m_type == mArmory_EquipAttmSlot)
			{
				wpnAttachments[attm->m_type] = NULL;
				gUserProfile.ProfileData.FPSAttachments[i].isEquipped = false;
				break;
			}
		}
	}

	// mark it as equipped now
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; ++i)
	{
		if(gUserProfile.ProfileData.FPSAttachments[i].WeaponID == mArmory_EquipWpnID && gUserProfile.ProfileData.FPSAttachments[i].AttachmentID == mArmory_EquipAttmID)
		{
			const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(mArmory_EquipAttmID);
			wpnAttachments[attm->m_type] = attm;
			gUserProfile.ProfileData.FPSAttachments[i].isEquipped = true;
			break;
		}
	}
	m_WeaponModel->m_CurrentWeapon->setWeaponAttachments(wpnAttachments);

	Scaleform::GFx::Value var[3];
	var[0].SetNumber(mArmory_EquipWpnID);
	var[1].SetNumber(mArmory_EquipAttmSlot);
	var[2].SetNumber(mArmory_EquipAttmID);
	gfxMovie.Invoke("_root.api.setAttachmentSpec", var, 3);

	gfxMovie.Invoke("_root.api.updateFPSUpgradeWindow", NULL, 0);
}

void FrontendUI::eventStoreBuyAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 5);

	mStore_buyWpnID = (uint32_t)args[0].GetNumber();
	mStore_buyItemID = (uint32_t)args[1].GetNumber();
	mStore_buyItemExp = (uint32_t)args[2].GetNumber();
	mStore_buyItemPrice = (uint32_t)args[3].GetNumber();
	mStore_buyItemPriceGD = (uint32_t)args[4].GetNumber();

	if(mStore_buyItemPrice == 0 && mStore_buyItemPriceGD == 0)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_BuyAttachmentThread, &FrontendUI::OnBuyAttachmentSuccess);

	CheckOnBuyAchievements();
}

void FrontendUI::ProcessClanAPIResult(int apiCode)
{
	if(apiCode!=0)
	{
		/*
		20 - not enough slots in clan	
		21 - already joined clan
		22 - no gamertag
		23 - no permission
		24 - already have pending application to specified clan
		25 - have maximum pending applications
		26 - buy GC before donating
		27 - clan name already exist
		28 - clan tag already exist
		*/
		const wchar_t* errorMsg = NULL;
		wchar_t tmpErrorStr[64] = {0};
		if(apiCode>=20 && apiCode<=28)
		{
			char tmpStr[32] = {0};
			sprintf(tmpStr, "ClanError_Code%d", apiCode);
			errorMsg = gLangMngr.getString(tmpStr);
		}
		else
		{
			swprintf(tmpErrorStr, 64, L"Clan error, code: %d", apiCode);
			errorMsg = tmpErrorStr;
		}
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(errorMsg);
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}
}

void FrontendUI::eventClansShowClans(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	r3d_assert(gUserProfile.clans);

	// todo: should by async call!
	int apiCode = gUserProfile.clans->ApiClanGetLeaderboard();
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);
		return;
	}
	
	for(std::list<CUserClans::ClanInfo_s>::iterator it=gUserProfile.clans->leaderboard_.begin(); it!=gUserProfile.clans->leaderboard_.end(); ++it)
	{
		const CUserClans::ClanInfo_s& clanInfo = *it;
		//function addClanToLeaderboard(clanID:Number, emblemID:Number, emblemColorID:Number, Name:String, Tag:String, Level:Number, Creator:String, Members:Number, Desc:String, ClanXP:Number); 
		Scaleform::GFx::Value var[10];
		var[0].SetNumber(clanInfo.ClanID);
		var[1].SetNumber(clanInfo.ClanEmblemID);
		var[2].SetNumber(clanInfo.ClanEmblemColor);
		var[3].SetString(clanInfo.ClanName);
		var[4].SetString(clanInfo.ClanTag);
		var[5].SetNumber(clanInfo.ClanLevel+1);
		var[6].SetString(clanInfo.OwnerGamertag);
		var[7].SetNumber(clanInfo.NumClanMembers);
		var[8].SetString(clanInfo.ClanLore);
		var[9].SetNumber(clanInfo.ClanXP);
		gfxMovie.Invoke("_root.api.addClanToLeaderboard", var, 10);
	}

	gfxMovie.Invoke("_root.api.showClansLeaderboard", NULL, 0);
}

void FrontendUI::eventClansShowClanMembers(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);
	r3d_assert(gUserProfile.clans);

	int clanID = (int)args[0].GetNumber();

	CUserClans::ClanInfo_s clanInfo;
	CUserClans::TClanMemberList members;
	// todo: async it!
	int apiCode = gUserProfile.clans->ApiClanGetInfo(clanID, &clanInfo, &members);	
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);	
		return;
	}
	for(CUserClans::TClanMemberList::iterator it=members.begin(); it!=members.end(); ++it)
	{
		const CUserClans::ClanMember_s& member = *it;
		gfxMovie.Invoke("_root.api.addClanMembersList", member.gamertag);
	}
	gfxMovie.Invoke("_root.api.showClanMemberList", NULL, 0);
}

void FrontendUI::updateMyClanInfoUI()
{
	r3d_assert(gUserProfile.clans);
	
	if(gUserProfile.clans->clanInfo_.ClanID!=0)
	{
		if(RUS_CLIENT) // disable donate GC for RUS client
		{
			gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.ClanReserve._visible", false);
			gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.DonateGCBtn._visible", false);
		}

		const CUserClans::ClanInfo_s& clanInfo = gUserProfile.clans->clanInfo_;
		Scaleform::GFx::Value var[11];
		var[0].SetNumber(clanInfo.ClanID);
		var[1].SetNumber(R3D_CLAMP(gUserProfile.ProfileData.ClanRank, 0, 2));
		var[2].SetString(clanInfo.ClanName);
		var[3].SetString(clanInfo.ClanTag);
		var[4].SetNumber(clanInfo.ClanTagColor);
		var[5].SetNumber(clanInfo.ClanEmblemID);
		var[6].SetNumber(clanInfo.ClanEmblemColor);
		var[7].SetNumber(clanInfo.ClanXP);
		var[8].SetNumber(clanInfo.ClanLevel+1);
		var[9].SetNumber(clanInfo.ClanGP);
		var[10].SetNumber(clanInfo.MaxClanMembers);
		gfxMovie.Invoke("_root.api.setMyClanInfo", var, 11);

		for(CUserClans::TClanMemberList::iterator it=gUserProfile.clans->clanMembers_.begin(); it!=gUserProfile.clans->clanMembers_.end(); ++it)
		{
			const CUserClans::ClanMember_s& member = *it;
			var[0].SetNumber(member.CustomerID);
			var[1].SetString(member.gamertag);
			var[2].SetNumber(R3D_CLAMP(member.ClanRank, 0, 2));
			var[3].SetNumber(member.stats.Wins);
			var[4].SetNumber(member.stats.Losses);
			char wlratio[32];
			sprintf(wlratio, "%.2f", getRatio((float)member.stats.Wins, (float)member.stats.Losses));
			char kdratio[32];
			sprintf(kdratio, "%.2f", getRatio((float)member.stats.Kills, (float)member.stats.Deaths));
			var[5].SetString(wlratio);
			var[6].SetNumber(member.stats.Kills);
			var[7].SetNumber(member.stats.Deaths);
			var[8].SetString(kdratio);
			var[9].SetNumber(member.ContributedXP);
			var[10].SetNumber(member.ContributedGP);
			gfxMovie.Invoke("_root.api.addMyClanMembers", var, 11);
		}
	}
	else
	{
		// reset
		Scaleform::GFx::Value var[11];
		var[0].SetNumber(0);
		var[1].SetNumber(999);
		var[2].SetString("");
		var[3].SetString("");
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		var[6].SetNumber(0);
		var[7].SetNumber(0);
		var[8].SetNumber(0);
		var[9].SetNumber(0);
		var[10].SetNumber(0);
		gfxMovie.Invoke("_root.api.setMyClanInfo", var, 11);
	}
}

void FrontendUI::eventClansCreateClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 6); // name:String, tag:String, EmblemID:Number, EmblemColor:Number, TagColor:Number, Desc:String
	r3d_assert(gUserProfile.clans);

	const char* clanName = args[0].GetString();
	const char* clanTag = args[1].GetString();
	int emblemID = (int)args[2].GetNumber();
	int emblemColor = (int)args[3].GetNumber();
	int tagColor = (int)args[4].GetNumber();
	const char* clanDesc = args[5].GetString();

	// test clan tag for invalid symbols
	{
		if(strpbrk(clanTag, "{}[]<>!\"'/?,.@#$%^&*()-+=_|`~ ")!=NULL)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("ClanError_BadTag"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
	}
	// test if clan name has at least 5 characters that are not spaces
	{
		int numChars = 0;
		// cannot use isalpha due to non english letters
		for(size_t i=0; i<strlen(clanName); ++i)
		{
			if(clanName[i]!=' ')
				++numChars;
		}
		if(numChars < 5)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("$FR_ClanCreateNeedName"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
	}
	int needGPToCreateClan = 0;
	int apiCode = gUserProfile.clans->ApiClanCheckIfCreateNeedMoney(&needGPToCreateClan);
	if(apiCode != 0)
	{
		ProcessClanAPIResult(apiCode);	
		return;
	}
	if(needGPToCreateClan && gUserProfile.ProfileData.Stats.GamePoints < gUserProfile.ShopClanCreate)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanCreateNotEnoughGP"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	CUserClans::CreateParams_s createParams;
	r3dscpy(createParams.ClanName, clanName);
	createParams.ClanNameColor = 0; // not used
	r3dscpy(createParams.ClanTag, clanTag);
	createParams.ClanTagColor = tagColor;
	createParams.ClanEmblemID = emblemID;
	createParams.ClanEmblemColor = emblemColor;
	apiCode = gUserProfile.clans->ApiClanCreate(createParams);
	if(apiCode != 0)
	{
		ProcessClanAPIResult(apiCode);	
		return;
	}
	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);

	apiCode = gUserProfile.clans->ApiClanSetLore(clanDesc);
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);
		//return;
	}

	updateMyClanInfoUI();

	// go to MyClan after creating your clan
	gfxMovie.Invoke("_root.Main.showScreen", "CommClansMyClan");
}

void FrontendUI::eventClansJoinClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2); // clanID:Number, applicationText:String
	r3d_assert(gUserProfile.clans);

	if(gUserProfile.ProfileData.ClanID != 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanJoinAlreadyInClan"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	int clanID = (int)args[0].GetNumber();
	const char* application = args[1].GetString();

	int apiCode = gUserProfile.clans->ApiClanApplyToJoin(clanID, application);
	if(apiCode != 0)
	{
		ProcessClanAPIResult(apiCode);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("$FR_ClanJoinApplicationSent"));
	var[1].SetBoolean(true);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	return;
}

void FrontendUI::eventClansLeaveClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	r3d_assert(gUserProfile.clans);
	r3d_assert(gUserProfile.ProfileData.ClanID!=0);

	if(gUserProfile.ProfileData.ClanRank == 0 && gUserProfile.clans->clanMembers_.size() > 1)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanLeaderCannotLeaveClan"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	int apiCode = gUserProfile.clans->ApiClanLeave();
	if(apiCode != 0)
	{
		ProcessClanAPIResult(apiCode);	
		return;
	}

	updateMyClanInfoUI();
	// go to JoinClan after leaving your clan
	gfxMovie.Invoke("_root.Main.showScreen", "CommClans");
}

void FrontendUI::eventClansDonateGCToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //numGC:Number
	r3d_assert(gUserProfile.clans);

	int numGC = (int)args[0].GetNumber();
	if(numGC > 0)
	{
		if(gUserProfile.ProfileData.Stats.GamePoints < numGC)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("ClanError_NotEnoughGPToDonate"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
		int apiCode = gUserProfile.clans->ApiClanDonateGPToClan(numGC);	
		if(apiCode != 0)
		{
			ProcessClanAPIResult(apiCode);
			return;
		}
		gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
		updateMyClanInfoUI();
		char tmpStr[32];
		sprintf(tmpStr, "%d GC", gUserProfile.clans->clanInfo_.ClanGP);
		gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.ClanReserve.Reserve.text", tmpStr);
		gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);
	}
}

void FrontendUI::eventClansDonateGCToMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2); //customerID:Number, numGC:Number
	r3d_assert(gUserProfile.clans);

	int customerID = (int)args[0].GetNumber();
	int numGC = (int)args[1].GetNumber();

	if(numGC > 0)
	{
		if(gUserProfile.clans->clanInfo_.ClanGP < numGC)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("ClanError_NotEnoughGPInClan"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}

		int apiCode = gUserProfile.clans->ApiClanDonateGPToMember(customerID, numGC);
		if(apiCode!=0)
		{
			ProcessClanAPIResult(apiCode);
			return;
		}

		if(gUserProfile.CustomerID == customerID)
		{
			gUserProfile.ProfileData.Stats.GamePoints += numGC;
			gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
		}

		updateMyClanInfoUI();
		char tmpStr[32];
		sprintf(tmpStr, "%d GC", gUserProfile.clans->clanInfo_.ClanGP);
		gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.ClanReserve.Reserve.text", tmpStr);
	}
}

void FrontendUI::eventClansPromoteMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //customerID:Number
	r3d_assert(gUserProfile.clans);

	int memberID = (int)args[0].GetNumber();
	if(memberID == gUserProfile.CustomerID)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanCannotPromoteYourself"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	CUserClans::ClanMember_s* clanMember = gUserProfile.clans->GetMember(memberID);
	if(clanMember)
	{
		if(clanMember->ClanRank>0)
		{
			int clanRank = R3D_CLAMP(clanMember->ClanRank, 0, 2);
			int apiCode = gUserProfile.clans->ApiClanSetRank(memberID, clanRank-1);
			if(apiCode!=0)
			{
				ProcessClanAPIResult(apiCode);	
				return;
			}
			updateMyClanInfoUI();
			gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);
		}
	}
	else
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString("No such clan member");
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}
}

void FrontendUI::eventClansDemoteMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //customerID:Number
	r3d_assert(gUserProfile.clans);

	int memberID = (int)args[0].GetNumber();
	if(memberID == gUserProfile.CustomerID)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanCannotDemoteYourself"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	CUserClans::ClanMember_s* clanMember = gUserProfile.clans->GetMember(memberID);
	if(clanMember) 
	{
		if(clanMember->ClanRank<2) // possible ranks for now: 0, 1, 2
		{
			int apiCode = gUserProfile.clans->ApiClanSetRank(memberID, clanMember->ClanRank+1);
			if(apiCode!=0)
			{
				ProcessClanAPIResult(apiCode);	
				return;
			}
			updateMyClanInfoUI();
			gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);
		}
		else
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("ClanError_CannotDemoteMore"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
	}
	else
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString("No such clan member");
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}
}

void FrontendUI::eventClansKickMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //customerID:Number
	r3d_assert(gUserProfile.clans);

	int memberID = (int)args[0].GetNumber();
	if(memberID == gUserProfile.CustomerID)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("$FR_ClanCannotKickYourself"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	int apiCode = gUserProfile.clans->ApiClanKick(memberID);
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);
		return;
	}
	updateMyClanInfoUI();
	gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);

}

void FrontendUI::eventClansShowApplications(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	r3d_assert(gUserProfile.clans);

	// we should already have clan application list from SessionLoginPoller - use current one
	r3dCSHolder cs1(gUserProfile.clans->csClans_);

	for(std::list<CUserClans::ClanApplication_s>::iterator it = gUserProfile.clans->clanApplications_.begin(); it!=gUserProfile.clans->clanApplications_.end(); ++it)
	{
		const CUserClans::ClanApplication_s& appl = *it;
		Scaleform::GFx::Value var[11];
		var[0].SetNumber(appl.ClanApplID);
		var[1].SetString(appl.Gamertag.c_str());
		var[2].SetNumber(appl.stats.getRankLevel());
		var[3].SetNumber(appl.stats.Wins);
		var[4].SetNumber(appl.stats.Losses);
		char wlratio[32];
		sprintf(wlratio, "%.2f", getRatio((float)appl.stats.Wins, (float)appl.stats.Losses));
		char kdratio[32];
		sprintf(kdratio, "%.2f", getRatio((float)appl.stats.Kills, (float)appl.stats.Deaths));
		var[5].SetString(wlratio);
		var[6].SetNumber(appl.stats.Kills);
		var[7].SetNumber(appl.stats.Deaths);
		var[8].SetString(kdratio);
		char timePlayed[32] = {0};
		int hours = appl.stats.TimePlayed/3600;
		int minutes = (appl.stats.TimePlayed-hours*3600)/60;
		int sec = appl.stats.TimePlayed%60;
		sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
		var[9].SetString(timePlayed);
		var[10].SetString(appl.Note.c_str());
		gfxMovie.Invoke("_root.api.addClanApplicationInfo", var, 11);
	}
	gfxMovie.Invoke("_root.Main.CommClanAppsAnim.updateApplications", NULL, 0);
}

void FrontendUI::eventClansApplicationAnswer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2); //clanApplID:Number, accept:Boolean
	r3d_assert(gUserProfile.clans);

	int clanApplID = (int)args[0].GetNumber();
	bool accept = args[1].GetBool();

	int apiCode = gUserProfile.clans->ApiClanApplyAnswer(clanApplID, accept);
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);
		return;
	}
	// remove that application from our list
	r3dCSHolder cs1(gUserProfile.clans->csClans_);
	for(std::list<CUserClans::ClanApplication_s>::iterator it = gUserProfile.clans->clanApplications_.begin(); it!=gUserProfile.clans->clanApplications_.end(); ++it)
	{
		if(it->ClanApplID == clanApplID)
		{
			gUserProfile.clans->clanApplications_.erase(it);
			break;
		}
	}

}

void FrontendUI::eventClansApplicationInvitePlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //playerName:String
	r3d_assert(gUserProfile.clans);

	const char* name = args[0].GetString();
	int apiCode = gUserProfile.clans->ApiClanSendInvite(name);
	if(apiCode!=0)
	{
		ProcessClanAPIResult(apiCode);
		return;
	}
	
	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("ClanPlayerInviteSentSuccessfully"));
	var[1].SetBoolean(true);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
	return;
}

void FrontendUI::eventClanPopUpClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	r3d_assert(gUserProfile.clans);

	m_clanPopupActive = false; 
}

void FrontendUI::eventClansInviteAnswer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2); //clanID:Number, accept:Boolean
	r3d_assert(gUserProfile.clans);

	int clanInviteID = (int)args[0].GetNumber();
	bool accept = args[1].GetBool();

	int apiCode = gUserProfile.clans->ApiClanAnswerInvite(clanInviteID, accept);
	if(apiCode != 0)
	{
		ProcessClanAPIResult(apiCode);
	}
	if(accept)
	{
		updateMyClanInfoUI();
		// if we are currently in clan section, go straight to MyClans in that case
		Scaleform::GFx::Value var;
		gfxMovie.GetMovie()->GetVariable(&var, "_root.Main.activeScreenName");
		const char* currentScreen = var.GetString();
		if(strcmp(currentScreen, "CommClans")==0 || strcmp(currentScreen, "CommClansCreate")==0)
		{
			gfxMovie.Invoke("_root.Main.showScreen", "CommClansMyClan");
		}
	}
	m_clanPopupActive = false; 
}

void FrontendUI::eventClansBuyClanSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1); //slotIdx:Number
	r3d_assert(gUserProfile.clans);

	int slotIdx = (int)args[0].GetNumber();
	int apiCode = gUserProfile.clans->ApiClanBuyAddMembers(slotIdx);
	if(apiCode != 0)
	{
		if(apiCode==7)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("NotEnougMoneyToBuyItem"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
		else
			ProcessClanAPIResult(apiCode);
		return;
	}
	updateMyClanInfoUI();
	gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.AvailableSlots.Num.text", gUserProfile.clans->clanInfo_.MaxClanMembers);
	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
}

void FrontendUI::UpdateClanPopups()
{
	if(m_clanPopupActive || asyncThread_ != NULL || m_friendPopupActive)
		return;

	if(m_clanPopupQuery.size() == 0)
		return;

	ClanPopup_s pop = m_clanPopupQuery.front();
	m_clanPopupQuery.pop_front();

	Scaleform::GFx::Value var[5];
	switch(pop.type)
	{
	default:
		r3dError("crap");
		break;
	case CLPopup_LeftClan: // left clan
		return; // no pop up for that event yet
		break;
	case CLPopup_JoinedClan: // joined clan
		{
			//showYouJoinedClan(ClanName:String, ClanLevel:Number, emblemID:Number, emblemIDColor:Number);
			var[0].SetString(pop.clanName);
			var[1].SetNumber(pop.clanLevel);
			var[2].SetNumber(pop.clanEmblemID);
			var[3].SetNumber(pop.clanEmblemColor);
			gfxMovie.Invoke("_root.api.showYouJoinedClan", var, 4);
		}
		break;
	case CLPopup_NewInvites: // 
		{
			//showYouHaveClanInvite(clanID:Number, ClanName:String, ClanLevel:Number, emblemID:Number, emblemIDColor:Number);
			var[0].SetNumber(pop.ID);
			var[1].SetString(pop.clanName);
			var[2].SetNumber(pop.clanLevel);
			var[3].SetNumber(pop.clanEmblemID);
			var[4].SetNumber(pop.clanEmblemColor);
			gfxMovie.Invoke("_root.api.showYouHaveClanInvite", var, 5);
		}
		break;
	case CLPopup_NewApplications: // 
		//showYouHaveClanApps();
		gfxMovie.Invoke("_root.api.showYouHaveClanApps", NULL, 0);
		break;
	}

	m_clanPopupActive = true;

	return;
}

void FrontendUI::updateClanData()
{
	// if we're in middle of something, don't update clans
	if(asyncThread_)
		return;

	CUserClans& clans= *gUserProfile.clans;

	UpdateClanPopups();

	// do nothing if no new data
	if(!clans.gotNewData)
		return;

	r3dCSHolder cs1(gUserProfile.clans->csClans_);
	// check for new members
	if(gUserProfile.clans->clanInfo_.NumClanMembers != gUserProfile.clans->clanCurData_.NumClanMembers)
	{
		int apiCode = gUserProfile.clans->ApiClanGetInfo(gUserProfile.clans->clanInfo_.ClanID, &gUserProfile.clans->clanInfo_, &gUserProfile.clans->clanMembers_);
		if(apiCode==0)
		{
			updateMyClanInfoUI();
			gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);
		}
	}
	if(gUserProfile.clans->clanInfo_.MaxClanMembers != gUserProfile.clans->clanCurData_.MaxClanMembers)
	{
		gUserProfile.clans->clanInfo_.MaxClanMembers = gUserProfile.clans->clanCurData_.MaxClanMembers;
		gfxMovie.SetVariable("_root.Main.CommClanMyClanAnim.MyClan.Descr.AvailableSlots.Num.text", gUserProfile.clans->clanInfo_.MaxClanMembers);
	}
	// check for clan membership
	if(gUserProfile.ProfileData.ClanID != 0 && gUserProfile.clans->clanCurData_.ClanID == 0) // got kicked from clan
	{
		gUserProfile.clans->clanInfo_ = CUserClans::ClanInfo_s();
		gUserProfile.clans->clanMembers_.clear();
		updateMyClanInfoUI();
		// if we are currently in clan section, go straight to JoinClan in that case
		Scaleform::GFx::Value var;
		gfxMovie.GetMovie()->GetVariable(&var, "_root.Main.activeScreenName");
		const char* currentScreen = var.GetString();
		if(strcmp(currentScreen, "CommClansApps")==0 || strcmp(currentScreen, "CommClansMyClan")==0)
		{
			gfxMovie.Invoke("_root.Main.showScreen", "CommClans");
		}

		m_clanPopupQuery.push_back(ClanPopup_s(CLPopup_LeftClan, 0, 0, 0, 0, ""));
	}
	if(gUserProfile.ProfileData.ClanID == 0 && gUserProfile.clans->clanCurData_.ClanID != 0) // got into clan
	{
		int apiCode = gUserProfile.clans->ApiClanGetInfo(gUserProfile.clans->clanCurData_.ClanID, &gUserProfile.clans->clanInfo_, &gUserProfile.clans->clanMembers_);
		if(apiCode == 0)
		{
			gUserProfile.ProfileData.ClanID = gUserProfile.clans->clanCurData_.ClanID;
			updateMyClanInfoUI();
			// if we are currently in clan section, go straight to MyClans in that case
			Scaleform::GFx::Value var;
			gfxMovie.GetMovie()->GetVariable(&var, "_root.Main.activeScreenName");
			const char* currentScreen = var.GetString();
			if(strcmp(currentScreen, "CommClans")==0 || strcmp(currentScreen, "CommClansCreate")==0)
			{
				gfxMovie.Invoke("_root.Main.showScreen", "CommClansMyClan");
			}
			m_clanPopupQuery.push_back(ClanPopup_s(CLPopup_JoinedClan, gUserProfile.ProfileData.ClanID, gUserProfile.clans->clanInfo_.ClanEmblemID, gUserProfile.clans->clanInfo_.ClanEmblemColor, gUserProfile.clans->clanInfo_.ClanLevel, gUserProfile.clans->clanInfo_.ClanName));
		}
		else
		{
			ProcessClanAPIResult(apiCode);
			return;
		}
	}
	if(gUserProfile.ProfileData.ClanID!=0 && gUserProfile.ProfileData.ClanRank != gUserProfile.clans->clanCurData_.ClanRank)
	{
		gUserProfile.ProfileData.ClanRank = gUserProfile.clans->clanCurData_.ClanRank;
		for(CUserClans::TClanMemberList::iterator it=gUserProfile.clans->clanMembers_.begin(); it!=gUserProfile.clans->clanMembers_.end(); ++it)
		{
			CUserClans::ClanMember_s& member = *it;
			if(member.CustomerID == gUserProfile.CustomerID)
			{
				member.ClanRank = gUserProfile.ProfileData.ClanRank;
				break;
			}
		}
		updateMyClanInfoUI();
		gfxMovie.Invoke("_root.Main.CommClanMyClanAnim.updateMembers", NULL, 0);
	}
	// check for new invites and put them into queue
	while(!gUserProfile.clans->clanInvites_.empty())
	{
		const CUserClans::ClanInvite_s& invite = *gUserProfile.clans->clanInvites_.begin();
		// check that we don't have that invite in the queue already (if user is not responding to invite, then next poller update will fetch same invite again)
		bool found = false;
		for(std::deque<ClanPopup_s>::iterator it = m_clanPopupQuery.begin(); it!=m_clanPopupQuery.end(); ++it)
		{
			if(it->type==CLPopup_NewInvites && (invite.ClanInviteID == it->ID))
			{
				found = true;
				break;
			}
		}
		if(!found)
			m_clanPopupQuery.push_back(ClanPopup_s(CLPopup_NewInvites, invite.ClanInviteID, invite.ClanEmblemID, invite.ClanEmblemColor, invite.ClanLevel, invite.ClanName.c_str()));
		
		gUserProfile.clans->clanInvites_.erase(gUserProfile.clans->clanInvites_.begin());
	}
	static size_t prevNumApplications = 0;
	// check for new applications
	if(!gUserProfile.clans->clanApplications_.empty())
	{
		if(prevNumApplications < gUserProfile.clans->clanApplications_.size())
		{
			m_clanPopupQuery.push_back(ClanPopup_s(CLPopup_NewApplications, 0, 0, 0, 0, ""));
		}
		prevNumApplications = gUserProfile.clans->clanApplications_.size();
	}
	else
		prevNumApplications = 0;

	// mark that we synced status
	clans.gotNewData   = false;
	UpdateClanPopups();
}


void FrontendUI::eventOptionsControlsRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int remapIndex = (int)args[0].GetNumber();
	r3d_assert(m_waitingForKeyRemap == -1);

	m_waitingForKeyRemap = remapIndex;
}

void FrontendUI::eventWelcomeBackClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

#if ENABLE_WEB_BROWSER
	d_show_browser->SetBool(false);
	g_pBrowserManager->SetSize(4, 4);
#endif
}

void FrontendUI::eventSwitchToShootingRange(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	SwitchShootingRange(true);
}

void FrontendUI::eventClaimDailyReward(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	if(gUserProfile.curRetentionDays_ == 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("WB_NoDailyBonusToClaim"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_ClaimDailyRewardThread, &FrontendUI::OnClaimDailyRewardSuccess);
}

unsigned int WINAPI FrontendUI::as_ClaimDailyRewardThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;

	int apiCode = gUserProfile.ApiRetBonusClaim(); 
	if(apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToClaimDailyReward"));
		return 0;
	}

	return 1;
}

void FrontendUI::OnClaimDailyRewardSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	gfxMovie.Invoke("_root.api.claimDailyRewardSuccess", "");

	gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);
	gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);

	r3d_assert(gUserProfile.curRetentionDays_>0);
	int gotBonus = gUserProfile.retentionBonusByDays_[gUserProfile.curRetentionDays_-1];
	{
		Scaleform::GFx::Value var[2];
		wchar_t tmpStr[256];
		if(!RUS_CLIENT)
			swprintf(tmpStr, 256, gLangMngr.getString("WB_ClaimedDailyReward"), gotBonus, L"GC");
		else
			swprintf(tmpStr, 256, gLangMngr.getString("WB_ClaimedDailyReward"), gotBonus, L"WP");
		var[0].SetStringW(tmpStr);
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

void FrontendUI::eventBrowseGamesRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
//	r3d_assert(args);
//	r3d_assert(argCount == 3);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("FetchingGamesListFromServer"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	gfxMovie.Invoke("_root.api.clearGames", "");

	StartAsyncOperation(&FrontendUI::as_BrowseGamesThread, &FrontendUI::OnGameListReceived);
}

void FrontendUI::eventBrowseGamesUpdateFilter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 5);

	m_FilterGames.conquest = args[0].GetBool();
	m_FilterGames.deathmatch= args[1].GetBool();
	m_FilterGames.sabotage = args[2].GetBool();

	m_FilterGames.regionSelected = (int)args[3].GetNumber();
	m_FilterGames.levelBracket = (int)args[4].GetNumber();

	r_server_region->SetInt(m_FilterGames.regionSelected);
	gfxMovie.Invoke("_root.api.setDefaultRegion", r_server_region->GetInt());
	writeGameOptionsFile();
}

void FrontendUI::eventBrowseRegionRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("FetchingGamesListFromServer"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	gfxMovie.Invoke("_root.api.clearGames", "");

	StartAsyncOperation(&FrontendUI::as_BrowseGamesThread, &FrontendUI::OnGameListReceived_ServerBrowse);
}

void FrontendUI::eventBrowseGamesJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2);

	// scaleform workaround if there is no games
	if(!args[0].IsNumber())
		return;
	int slot = (int)args[0].GetNumber();
	bool spectator = args[1].GetBool();

	// join game button can be pressed without actual games, filter that.
	if(!gMasterServerLogic.gameListReceived_)
		return;
	if(slot >= (int)gMasterServerLogic.games_.size())
		return;

	// reset quickgame
	quickGameInfo_.gameMap = 0xFF;
	quickGameInfo_.gameMode = 0xFF;

	browseJoinSlot_ = slot;

	gClientLogic().requestToJoinAsSpectator = false;

	if(browseJoinSlot_ != -1)
	{
		r3d_assert(gMasterServerLogic.gameListReceived_);
		const GBPKT_M2C_GameData_s& gd = gMasterServerLogic.games_[This_->browseJoinSlot_];

		// check if game has password
		if(gd.status & 8)
		{
			gfxMovie.Invoke("_root.api.showEnterPassword", "");
			return;
		}

		/*if(gMasterServerLogic.games_[This_->browseJoinSlot_].info.mapId == GBGameInfo::MAPID_BurningSea)
		{
			// check for premium or top 100
			if(!(m_hasPremiumAccount || gUserProfile.ProfileData.Stats.HonorPoints > 93000))
			{
				Scaleform::GFx::Value var[2];
				var[0].SetStringW(gLangMngr.getString("GameAvailableToPremiumOrTop100"));
				var[1].SetBoolean(true);
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
				return;
			}
		}*/

		int plrLevel = gUserProfile.ProfileData.Stats.getRankLevel();

		// don't allow high level player to join a newbie game
		if(plrLevel > gd.info.maxLevel)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("CannotJoinGameLevelTooHigh"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			return;
		}
		// if newbie is joining high level game, show him a warning
		if(plrLevel < gd.info.minLevel)
		{
			gfxMovie.Invoke("_root.api.showHighLevelGameWarning", "");
			return;
		}
		
		if(spectator && gd.info.mapType == GBGameInfo::MAPT_Bomb)
			gClientLogic().requestToJoinAsSpectator = true;
	}


	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	StartAsyncOperation(&FrontendUI::as_JoinGameThread);
}

void FrontendUI::eventBrowseGamesEnterPassword(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	const char* pwd = args[0].GetString();
	
	if(browseJoinSlot_ == -2) // friends game joining mode
		r3dscpy(m_friendJoinPacket.pwd, pwd);
	else
		r3dscpy(m_GameJoin_Password, pwd);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	StartAsyncOperation(&FrontendUI::as_JoinGameThread);
}

void FrontendUI::eventConfirmHighLevelGameJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	if(browseJoinSlot_ == -2) // friends game joining mode
		m_friendJoinPacket.playerOKToJoinHighLevelGame = true;

	StartAsyncOperation(&FrontendUI::as_JoinGameThread);
}

void FrontendUI::eventCreateGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 13);

	char gameName[32];
	r3dscpy(gameName, args[0].GetString());
	const char* gamePass = args[1].GetString();
	int gameType = (int)args[2].GetNumber();
	int mapID = (int)args[3].GetNumber();
	int maxPlayers = (int)args[4].GetNumber();
	int serverRegion = (int)args[5].GetNumber();
	int timeLimit = (int)args[6].GetNumber(); 
	int numRounds = (int)args[7].GetNumber(); // sabo game only
	bool friendlyFire = args[8].GetBool();
	bool autoBalance = args[9].GetBool();
	//bool practiceGame = args[10].GetBool();
	bool basicGame = args[11].GetBool();
	int levelBracket = (int)args[12].GetNumber();

	bool has_anything = false;
	// skip white spaces in the beginning
	size_t start_text=0;
	size_t argLen = strlen(gameName);
	for(size_t i=0; i<argLen; ++i)
	{
		start_text = i;
		if(!isspace((unsigned char)gameName[i]))
			break;
	}

	// check that name isn't empty
	bool correctGameName = (argLen-start_text)>3;
	if(!correctGameName)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("GameNameTooShort"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	NetPacketsGameBrowser::GBPKT_C2M_CreateGame_s& info = createGameInfo_;
	info.region = GBNET_REGION_US_West;
	if(serverRegion == 0)
		info.region = GBNET_REGION_US_West;
	else if(serverRegion == 1)
		info.region = GBNET_REGION_Europe;
	if(RUS_CLIENT) // always override region for russian client
		info.region = GBNET_REGION_Russia;
	if(MASSIVE_CLIENT)
		info.region = GBNET_REGION_US_West;

	r_server_region->SetInt(serverRegion);
	gfxMovie.Invoke("_root.api.setDefaultRegion", r_server_region->GetInt());
	writeGameOptionsFile();

	r3dscpy(info.name, &gameName[start_text]);
	if(strlen(gamePass) > 1)
		r3dscpy(info.pwd, gamePass);

	switch(gameType)
	{
	case 1:
		info.mapType	  = GBGameInfo::MAPT_Conquest;
		break;
	case 2:
		info.mapType	  = GBGameInfo::MAPT_Deathmatch;
		break;
	case 3:
		info.mapType	  = GBGameInfo::MAPT_Siege;
		break;
	case 4:
		info.mapType	 = GBGameInfo::MAPT_Bomb;
		break;
	default:
		r3dError("Unknown map type!!!!\n");
		break;
	}

	info.mapId        = mapID;
	info.maxPlayers   = maxPlayers;
	info.timeLimitVar    = timeLimit;
	info.numRoundsVar = numRounds;
	info.friendlyFire    = friendlyFire;
	info.autoBalance     = autoBalance;
	info.minPlayerLevel  = 1;
	info.maxPlayerLevel  = 60;
	if(levelBracket == 1)
	{
		info.minPlayerLevel  = 1;
		info.maxPlayerLevel  = 60;
	}
	else if(levelBracket == 2)
	{
		info.minPlayerLevel  = 1;
		info.maxPlayerLevel  = 10;
	}
	else if(levelBracket == 3)
	{
		info.minPlayerLevel  = 11;
		info.maxPlayerLevel  = 20;
	}
	else if(levelBracket == 4)
	{
		info.minPlayerLevel  = 21;
		info.maxPlayerLevel  = 40;
	}
	else if(levelBracket == 5)
	{
		info.minPlayerLevel  = 41;
		info.maxPlayerLevel  = 60;
	}

	int curLevel = gUserProfile.ProfileData.Stats.getRankLevel();
	// if you wouldn't be able to play the game.. 
	if ( curLevel < info.minPlayerLevel || curLevel > info.maxPlayerLevel )
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("LevelDoesntMeetBracket"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		return;
	}

	info.isBasicGame	 = basicGame;

#ifndef FINAL_BUILD
	r3dOutToLog("Request to create game: name '%s', password '%s'\n", info.name, info.pwd);
#endif

	// reset quickgame
	quickGameInfo_.gameMap = 0xFF;
	quickGameInfo_.gameMode = 0xFF;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("CreatingGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	StartAsyncOperation(&FrontendUI::as_CreateGameThread);
}

void FrontendUI::eventJoinQuickGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 3);

	int gameMode  = (int)args[0].GetNumber();
	int gameMap = (int)args[1].GetNumber();
	int region = (int)args[2].GetNumber();

	// convert gameMode, gameMap already in proper format
	switch(gameMode)
	{
	case 1:
		gameMode	  = GBGameInfo::MAPT_Conquest;
		break;
	case 2:
		gameMode	  = GBGameInfo::MAPT_Deathmatch;
		break;
	case 4:
		gameMode	  = GBGameInfo::MAPT_Bomb;
		break;
	case 9:
		gameMode	 = 0xFF;
		break;
	default:
		r3dError("Unknown map type!!!!\n");
		break;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	
	browseJoinSlot_ = -1; // -1 slot is quick game join!
	gClientLogic().requestToJoinAsSpectator = false;

	// prepare network packet
	quickGameInfo_.CustomerID = gUserProfile.CustomerID;
	quickGameInfo_.playerLevel= gUserProfile.ProfileData.Stats.getRankLevel();
	quickGameInfo_.gameMap    = (BYTE)gameMap;
	quickGameInfo_.gameMode   = (BYTE)gameMode;
	quickGameInfo_.region     = (BYTE)GBNET_REGION_US_West;
	if(region==0)
		quickGameInfo_.region     = (BYTE)GBNET_REGION_US_West;
	else if(region ==1)
		quickGameInfo_.region     = (BYTE)GBNET_REGION_Europe;
	else if(region == 2)
		quickGameInfo_.region     = (BYTE)GBNET_REGION_Unknown;
	if(RUS_CLIENT) // always override region for russian client
		quickGameInfo_.region     = (BYTE)GBNET_REGION_Russia;
	if(MASSIVE_CLIENT)
		quickGameInfo_.region	  = (BYTE)GBNET_REGION_US_West;
	r_server_region->SetInt(region);
	gfxMovie.Invoke("_root.api.setDefaultRegion", r_server_region->GetInt());
	writeGameOptionsFile();

	StartAsyncOperation(&FrontendUI::as_JoinGameThread);
}

void FrontendUI::eventStorePurchaseGP(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gSteam.inited_)
	{
		r3d_assert(false && "eventStorePurchaseGP not supposed to be called when steam is active!");
		return;
	}

	if(RUS_CLIENT)
	{
		char url[256];
		sprintf(url, "http://www.gamenet.ru/money/");
		ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
	}
	else if(MASSIVE_CLIENT)
	{
		char url[256];
		sprintf(url, "http://massivegaming.ph/");
		ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
	}
	else
	{
		// open browser with web store open
		char authKey[128];
		gUserProfile.GenerateSessionKey(authKey);
		char url[256];
		sprintf(url, "https://account.thewarinc.com/Store.php?WoLogin=%s", authKey);
		ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
	}

	// minimize our window
	ShowWindow(win::hWnd, SW_MINIMIZE);
	needUpdateProfileOnActivate = true;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
}

void FrontendUI::eventStorePurchaseGP_Steam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	// steam purchase
	r3d_assert(gSteam.inited_ && "eventStorePurchaseGP_Steam not supposed to be called when steam is not active!");
	if(gSteam.RecheckUser() == false)
	{
		// steam was closed or user logged off
		Scaleform::GFx::Value args[2];
		args[0].SetString("You are logged out from Steam, please login again");
		args[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
		return;
	}

	m_steamGpItemId = (int)args[0].GetNumber();

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		
	StartAsyncOperation(&FrontendUI::as_SteamBuyGPThread, &FrontendUI::OnSteamBuyGPSuccess);
	return;
}

void FrontendUI::eventStoreEntered(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(RUS_CLIENT)
	{
		// update gamenet balance
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontendUI::as_GNAGetBalanceThread, &FrontendUI::OnGNAGetBalanceSuccess);
	}
	
	return;
}

void FrontendUI::eventInventorySellLootBox(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int itemID = (int)args[0].GetNumber();

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	mStore_buyItemID = itemID;
	StartAsyncOperation(&FrontendUI::as_SellLootBoxThread, &FrontendUI::OnSellLootBoxSuccess);
}

void FrontendUI::eventStoreRequestPackageDetails(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int itemID = (int)args[0].GetNumber();

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	m_RequestedMysteryBox = NULL;
	m_MysteryBoxItemIdRequested = itemID;
	StartAsyncOperation(&FrontendUI::as_GetMysteryBoxDetailsThread, &FrontendUI::OnGetMysteryBoxSuccess);
}

unsigned int WINAPI FrontendUI::as_GetMysteryBoxDetailsThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;

	// get info about the mystery box
	This->m_RequestedMysteryBox = NULL;
	int apiCode = gUserProfile.ApiMysteryBoxGetContent(This->m_MysteryBoxItemIdRequested, (const CClientUserProfile::MysteryBox_s**)&This->m_RequestedMysteryBox);
	if(apiCode == 0 && This->m_RequestedMysteryBox)
	{
		return 1;
	}
	
	This->SetAsyncError(apiCode, gLangMngr.getString("FailedToGetItemInfo"));
	return 0;
}

void FrontendUI::OnGetMysteryBoxSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	r3d_assert(m_RequestedMysteryBox);
	r3d_assert(m_MysteryBoxItemIdRequested);

	Scaleform::GFx::Value prm[4];
	CClientUserProfile::MysteryBox_s* box = (CClientUserProfile::MysteryBox_s*)m_RequestedMysteryBox;
	prm[0].SetNumber(m_MysteryBoxItemIdRequested);
	char tmpStr[32] = "0";
	bool isPackage = gWeaponArmory.getPackageConfig(m_MysteryBoxItemIdRequested)!=NULL;
	if(isPackage) // only packages can have one empty itemID that shows WP got
	{
		// look for GD
		for(uint32_t j=0; j<box->content.size(); ++j)
		{
			if(box->content[j].ItemID == 0)
			{
				if(box->content[j].GDMin == box->content[j].GDMax)
					sprintf(tmpStr, "%d", box->content[j].GDMin);
				else
					sprintf(tmpStr, "%d-%d", box->content[j].GDMin, box->content[j].GDMax);
				break;
			}
		}
	}
	prm[1].SetNumber(0); // gp
	prm[2].SetString(tmpStr); // gd
	prm[3].SetNumber(0); // sp
	gfxMovie.Invoke("_root.api.addPackageInfo", prm, 4);

	static int gdCounter = 0; // should be static, as we want to keep increasing itemID for fake WP items, otherwise if we open one box, and then open second one, it will overwrite info from first box (will reuse itemID)
	for(uint32_t j=0; j<box->content.size(); ++j)
	{
		if(box->content[j].ItemID == 0 && !isPackage)
		{
			Scaleform::GFx::Value var[3];
			//public function updateItem(itemID:Number, name:String, desc:String)
			var[0].SetNumber(1000+gdCounter);
			char tmpGD[32] = "";
			wchar_t tmpDesc[256] = L"";
			if(box->content[j].GDMin == box->content[j].GDMax)
			{
				sprintf(tmpGD, "%d WP", box->content[j].GDMin);
				swprintf(tmpDesc, 256, gLangMngr.getString("UI_ChanceOfWinningWP2"), box->content[j].GDMin);
			}
			else
			{
				sprintf(tmpGD, "%d-%d WP", box->content[j].GDMin, box->content[j].GDMax);
				swprintf(tmpDesc, 256, gLangMngr.getString("UI_ChanceOfWinningWP1"), box->content[j].GDMin, box->content[j].GDMax);
			}
			var[1].SetString(tmpGD);
			var[2].SetStringW(tmpDesc);
			gfxMovie.Invoke("_root.api.updateItem", var, 3);

			prm[1].SetNumber(1000+gdCounter);
			prm[2].SetNumber(2000*1440); 
			prm[3].SetNumber(2000*1440); 

			++gdCounter;
		}
		else
		{
			prm[1].SetNumber(box->content[j].ItemID);
			prm[2].SetNumber(box->content[j].ExpDaysMin*1440); 
			prm[3].SetNumber(box->content[j].ExpDaysMax*1440); 
		}
		gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);
	}

	gfxMovie.Invoke("_root.api.showStorePackageInfo", "");

	m_RequestedMysteryBox = 0;
	m_MysteryBoxItemIdRequested = 0;
}


void FrontendUI::eventStoreBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 4);

	mStore_buyItemID = (uint32_t)args[0].GetNumber();
	mStore_buyItemExp = (uint32_t)args[1].GetNumber();
	mStore_buyItemPrice = (uint32_t)args[2].GetNumber();
	mStore_buyItemPriceGD = (uint32_t)args[3].GetNumber();
	if(mStore_buyItemID == 301045) // change gamer tag
	{
		gfxMovie.Invoke("_root.api.showChangeGamertag", "");
		return;
	}

	if(mStore_buyItemPrice == 0 && mStore_buyItemPriceGD == 0)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	if ( mStore_buyItemID == WeaponConfig::ITEMID_Cypher2 ) // UAV item. 
	{
		MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_PURCHASE_UAV );
	}

	// launch different tasks if we buying lootboxes or normal item
	const ItemConfig* ic = gWeaponArmory.getItemConfig(mStore_buyItemID);

	if(ic && (ic->category == storecat_MysteryBox || ic->category == storecat_LootBox))
	{
		StartAsyncOperation(&FrontendUI::as_BuyLootBoxThread, &FrontendUI::OnBuyLootBoxSuccess);
	}
	else
	{
		StartAsyncOperation(&FrontendUI::as_BuyItemThread, &FrontendUI::OnBuyItemSuccess);
	}

	CheckOnBuyAchievements();
	
}

void FrontendUI::eventStoreChangeGamertag(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	//r3dscpy(mStore_ChangeGamertag, wideToUtf8(args[0].GetStringW()));
	r3dscpy(mStore_ChangeGamertag, args[0].GetString()); // scaleform returns regular string... seems like it us UTF8
	wchar_t tempCheckString[32];
	r3dscpy(tempCheckString, utf8ToWide(mStore_ChangeGamertag));

	// check validity of gamertag
	size_t argLen = wcslen(tempCheckString);
	if(argLen < 3)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("NewGamerTagMinLenRequirement"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	for(size_t i=0; i<argLen; ++i)
	{
		if(iswspace(tempCheckString[i]))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("NewGamerTagCannotHaveSpaces"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}
	}
	for(size_t i=0; i<argLen; ++i)
	{
		if(!iswalnum(tempCheckString[i]))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("NewGamerTagShouldHaveLettersAndNumbers"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}
	}

	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontendUI::as_ChangeGamertagThread, &FrontendUI::OnBuyItemSuccess);
	}
}

void FrontendUI::eventWelcomeBackBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 4);

	mStore_buyItemID = (uint32_t)args[0].GetNumber();
	mStore_buyItemExp = (uint32_t)args[1].GetNumber();
	mStore_buyItemPrice = (uint32_t)args[2].GetNumber();
	mStore_buyItemPriceGD = (uint32_t)args[3].GetNumber();
	if(mStore_buyItemPrice > 0 || mStore_buyItemPriceGD > 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontendUI::as_BuyItemThread, &FrontendUI::OnBuyItemSuccess);
		CheckOnBuyAchievements();

	}
}

void FrontendUI::eventRenewItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 4);

	mStore_buyItemID = (uint32_t)args[0].GetNumber();
	mStore_buyItemExp = (uint32_t)args[1].GetNumber();
	mStore_buyItemPrice = (uint32_t)args[2].GetNumber();
	mStore_buyItemPriceGD = (uint32_t)args[3].GetNumber();
	if(mStore_buyItemPrice > 0 || mStore_buyItemPriceGD > 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontendUI::as_BuyItemThread, &FrontendUI::OnBuyItemSuccess);
		
		CheckOnBuyAchievements();

	}
}

void FrontendUI::eventUnlockPremiumAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	
	// if premium account buying is disabled
	if(mStorePremiumPrice == 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("PremiumNotAvailable"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	// check if user have enough money
	if(gUserProfile.ProfileData.Stats.GamePoints < (int)mStorePremiumPrice)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("NotEnoughGP"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	mStore_buyItemID = 301004;
	mStore_buyItemExp = 30;
	mStore_buyItemPrice = mStorePremiumPrice;
	mStore_buyItemPriceGD = 0;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_BuyItemThread, &FrontendUI::OnBuyPremiumAccountSuccess);
}

void FrontendUI::eventExit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	exitRequested_ = true;
}

void FrontendUI::eventOptionsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	// get options
	g_tps_camera_mode->SetInt(0);
	g_enable_voice_commands			->SetBool(true);

	int old_fullscreen = r_fullscreen->GetInt();
	r_fullscreen->SetBool(true);

	int old_vsync = r_vsync_enabled->GetInt();
	r_vsync_enabled			->SetInt(0);

	if(old_fullscreen!=r_fullscreen->GetInt() || old_vsync!=r_vsync_enabled->GetInt())
	{
		// show message telling player that to change windows\fullscreen he have to restart game
		// todo: make fullscreen/window mode switch on the fly?
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("RestartGameForChangesToTakeEffect"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}

	switch( r3dGetDeviceStrength() )
	{
	case S_WEAK:
		r_overall_quality->SetInt(1);
		break;
	case S_MEDIUM:
		r_overall_quality->SetInt(2);
		break;
	case S_STRONG:
		r_overall_quality->SetInt(3);
		break;
	case S_ULTRA:
		r_overall_quality->SetInt(4);
		break;
	default:
		r_overall_quality->SetInt(1);
		break;
	}

	DWORD settingsChangedFlags = 0;
	GraphicSettings settings;

	switch( r_overall_quality->GetInt() )
	{
	case 1:
		FillDefaultSettings( settings, S_WEAK );
		settingsChangedFlags = SetDefaultSettings( S_WEAK );
		break;
	case 2:
		FillDefaultSettings( settings, S_MEDIUM );
		settingsChangedFlags = SetDefaultSettings( S_MEDIUM );
		break;
	case 3:
		FillDefaultSettings( settings, S_STRONG );
		settingsChangedFlags = SetDefaultSettings( S_STRONG );
		break;
	case 4:
		FillDefaultSettings( settings, S_ULTRA );
		settingsChangedFlags = SetDefaultSettings( S_ULTRA );
		break;
	case 5:
		{
			settings.mesh_quality			= (int)args[17].GetNumber();
			settings.texture_quality		= (int)args[20].GetNumber();
			settings.terrain_quality		= (int)args[11].GetNumber();
			settings.water_quality			= (int)args[13].GetNumber();
			settings.shadows_quality		= (int)args[14].GetNumber();
			settings.lighting_quality		= (int)args[15].GetNumber();
			settings.particles_quality		= (int)args[16].GetNumber();
			settings.decoration_quality		= (int)args[12].GetNumber();
			settings.anisotropy_quality		= (int)args[18].GetNumber();
			settings.postprocess_quality	= (int)args[19].GetNumber();
			settings.ssao_quality			= (int)args[10].GetNumber();
		}
		break;
	default:
		r3d_assert( false );
	}

	// AA is separate and can be changed at any overall quality level
	settings.antialiasing_quality	= 0;

	settingsChangedFlags |= GraphSettingsToVars( settings );

	AddSettingsChangeFlag( settingsChangedFlags );

	// clamp brightness and contrast, otherwise if user set it to 0 the screen will be white
	r_brightness			->SetFloat(0.5f);
	r_contrast				->SetFloat(0.5f);

	s_sound_volume			->SetFloat(1.0f);
	s_music_volume			->SetFloat(1.0f);
	s_comm_volume			->SetFloat(1.0f);

	SetNeedUpdateSettings();

	// write to ini file
	writeGameOptionsFile();
	SyncGraphicsUI();
}

void FrontendUI::eventOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 23);

	// get options
	g_tps_camera_mode->SetInt((int)args[7].GetNumber());
	g_enable_voice_commands			->SetBool( !!(int)args[8].GetNumber() );

	const char* res = args[0].GetString();
	int width = 1280, height = 720;
	sscanf(res, "%dx%d", &width, &height );

	r_width->SetInt( width );
	r_height->SetInt( height );

	int old_fullscreen = r_fullscreen->GetInt();
	r_fullscreen->SetInt( (int)args[21].GetNumber() );

	int old_vsync = r_vsync_enabled->GetInt();
	r_vsync_enabled			->SetInt((int)args[22].GetNumber()-1);

	if(old_fullscreen!=r_fullscreen->GetInt() || old_vsync!=r_vsync_enabled->GetInt())
	{
		// show message telling player that to change windows\fullscreen he have to restart game
		// todo: make fullscreen/window mode switch on the fly?
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("RestartGameForChangesToTakeEffect"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
	}

	r_overall_quality		->SetInt( (int)args[1].GetNumber());

	DWORD settingsChangedFlags = 0;
	GraphicSettings settings;

	switch( r_overall_quality->GetInt() )
	{
	case 1:
		FillDefaultSettings( settings, S_WEAK );
		settingsChangedFlags = SetDefaultSettings( S_WEAK );
		break;
	case 2:
		FillDefaultSettings( settings, S_MEDIUM );
		settingsChangedFlags = SetDefaultSettings( S_MEDIUM );
		break;
	case 3:
		FillDefaultSettings( settings, S_STRONG );
		settingsChangedFlags = SetDefaultSettings( S_STRONG );
		break;
	case 4:
		FillDefaultSettings( settings, S_ULTRA );
		settingsChangedFlags = SetDefaultSettings( S_ULTRA );
		break;
	case 5:
		{
			settings.mesh_quality			= (int)args[17].GetNumber();
			settings.texture_quality		= (int)args[20].GetNumber();
			settings.terrain_quality		= (int)args[11].GetNumber();
			settings.water_quality			= (int)args[13].GetNumber();
			settings.shadows_quality		= (int)args[14].GetNumber();
			settings.lighting_quality		= (int)args[15].GetNumber();
			settings.particles_quality		= (int)args[16].GetNumber();
			settings.decoration_quality		= (int)args[12].GetNumber();
			settings.anisotropy_quality		= (int)args[18].GetNumber();
			settings.postprocess_quality	= (int)args[19].GetNumber();
			settings.ssao_quality			= (int)args[10].GetNumber();
		}
		break;
	default:
		r3d_assert( false );
	}

	// AA is separate and can be changed at any overall quality level
	settings.antialiasing_quality	= (int)args[9].GetNumber();

	settingsChangedFlags |= GraphSettingsToVars( settings );

	AddSettingsChangeFlag( settingsChangedFlags );

	// clamp brightness and contrast, otherwise if user set it to 0 the screen will be white
	r_brightness			->SetFloat( R3D_CLAMP((float)args[2].GetNumber(), 0.25f, 0.75f) );
	r_contrast				->SetFloat( R3D_CLAMP((float)args[3].GetNumber(), 0.25f, 0.75f) );

	s_sound_volume			->SetFloat( R3D_CLAMP((float)args[4].GetNumber(), 0.0f, 1.0f) );
	s_music_volume			->SetFloat( R3D_CLAMP((float)args[5].GetNumber(), 0.0f, 1.0f) );
	s_comm_volume			->SetFloat( R3D_CLAMP((float)args[6].GetNumber(), 0.0f, 1.0f) );


	SetNeedUpdateSettings();
	SyncGraphicsUI();

	// write to ini file
	writeGameOptionsFile();

	// if we changed resolution, we need to reset scaleform, otherwise visual artifacts will show up
//	needScaleformReset = true;
}

void FrontendUI::eventOptionsControlsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	InputMappingMngr->resetKeyMappingsToDefault();
	for(int i=0; i<r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxMovie.Invoke("_root.api.setKeyboardMapping", args, 2);
	}
	void writeInputMap();
	writeInputMap();

	// update those to match defaults in Vars.h
	g_vertical_look			->SetBool(false);
	g_mouse_wheel			->SetBool(true);
	g_mouse_sensitivity		->SetFloat(1.0f);
	g_mouse_acceleration	->SetBool(false);
	g_toggle_aim			->SetBool(false);
	g_toggle_crouch			->SetBool(false);

	// write to ini file
	writeGameOptionsFile();
	SyncGraphicsUI();
}

void FrontendUI::eventOptionsControlsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 7);

	g_vertical_look			->SetBool( !!(int)args[0].GetNumber() );
	g_mouse_wheel			->SetBool( !!(int)args[2].GetNumber() );
	g_mouse_sensitivity		->SetFloat( (float)args[3].GetNumber() );
	g_mouse_acceleration	->SetBool( !!(int)args[4].GetNumber() );
	g_toggle_aim			->SetBool( !!(int)args[5].GetNumber() );
	g_toggle_crouch			->SetBool( !!(int)args[6].GetNumber() );

	// write to ini file
	writeGameOptionsFile();

	SyncGraphicsUI();
}

void FrontendUI::eventOptionsLanguageSelection(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	const char* newLang = args[0].GetString();

	if(strcmp(newLang, g_user_language->GetString())==0)
		return; // same language

	g_user_language->SetString(newLang);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("NewLanguageSetAfterRestart"));
	var[1].SetBoolean(true);
	pMovie->Invoke("_root.api.showInfoMsg", var, 2);		

	// write to ini file
	writeGameOptionsFile();
}

void FrontendUI::eventLearnSkill(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 3);

	int curLoadout = (int)args[0].GetNumber();
	int skillID = (int)args[1].GetNumber();
	int nextLevel = (int)args[2].GetNumber();

	int apiCode = gUserProfile.ApiSkillLearn(curLoadout, skillID, nextLevel);
	if(apiCode == 0)
	{
		pMovie->Invoke("_root.api.hideInfoMsg", "");
		Scaleform::GFx::Value var[3];
		//setLoadoutSkillInfo(loadoutID:Number, skillID:Number, curLevel:Number);
		var[0].SetNumber(curLoadout);
		var[1].SetNumber(skillID);
		var[2].SetNumber(nextLevel);
		gfxMovie.Invoke("_root.api.setLoadoutSkillInfo", var, 3);

		UpdateSkillUI();
	}
	else 
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("FailedToLearnSkill"));
		var[1].SetBoolean(true);
		pMovie->Invoke("_root.api.showInfoMsg", var, 2);		
	}
}

void FrontendUI::eventResetSkillTree(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int curLoadout = (int)args[0].GetNumber();

	int apiCode = gUserProfile.ApiSkillReset(curLoadout);
	if(apiCode == 0)
	{
		// update loadouts skills
		for(int i=0; i<CUserSkills::SKILL_ID_END; ++i)
		{
			if(gUserProfile.ShopSkillCosts2[i][0] != 0) // if first level has price - then skill exists
			{
				Scaleform::GFx::Value var[3];
				//function setLoadoutSkillInfo(loadoutID:Number, skillID:Number, curLevel:Number);
				var[0].SetNumber(curLoadout);
				var[1].SetNumber(i);
				int sk = (i%100);
				var[2].SetNumber(gUserProfile.ProfileData.ArmorySlots[curLoadout].Skills[sk]);
				gfxMovie.Invoke("_root.api.setLoadoutSkillInfo", var, 3);
			}
		}
		UpdateSkillUI();
	}
	else 
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("FailedToLearnSkill"));
		var[1].SetBoolean(true);
		pMovie->Invoke("_root.api.showInfoMsg", var, 2);		
	}
}

void FrontendUI::eventBuyPackContinue(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);
	showBuyPackMovie = false;
}

void FrontendUI::eventBuyPackSteam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);
	
	r3d_assert(gSteam.inited_ && "eventBuyPackSteam not supposed to be called when steam is not active!");
	if(gSteam.RecheckUser() == false)
	{
		// steam was closed or user logged off
		Scaleform::GFx::Value args[2];
		args[0].SetString("You are logged out from Steam, please login again");
		args[1].SetBoolean(true);
		gfxBuyPackMovie.Invoke("_global.showInfoMsg", args, 2);		
		return;
	}

	int slotID = (int)args[0].GetNumber();
	
	// hard coded
	m_steamGpItemId = 1003; // collectors edition

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxBuyPackMovie.Invoke("_global.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_SteamBuyGPThread, &FrontendUI::OnSteamBuyPackSuccess);
	return;
}

void FrontendUI::eventBuyPackNonSteam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 2);

	showBuyPackMovie = false;
	int slotID = (int)args[0].GetNumber();
	bool creditCard = (int)args[1].GetNumber()==0; // 0-credit card, 1-paypal
	
	// steam purchase
	if(gSteam.inited_)
	{
		r3d_assert(false && "eventBuyPackNonSteam not supposed to be called when steam is active!");
		return;
	}

	if(RUS_CLIENT) // disabled for RUS_CLIENT in Main_Network.cpp, but just in case, lets live this code in here
	{
		int showStorePackage = 0;
		if(slotID == 1)
			showStorePackage = 500019;
		else if(slotID == 2)
			showStorePackage = 500020;
		else
			showStorePackage = 500021;

		gfxMovie.SetVariable("_root.api.showStoreItemID", (int)showStorePackage);
		gfxMovie.Invoke("_root.api.showStore", "");

		return;
	}

	// generate session key
	char authKey[1024];
	gUserProfile.GenerateSessionKey(authKey);
	
	char url[256];
	if(creditCard) {
		//sprintf(url, "Store_IngameCC.php?ItemID=PACK_RETAIL%d", slotID);
		sprintf(url, "Store_IngameCC.php?ItemID=PACK_COLLECTOR_EDITION");
	} else {
		//sprintf(url, "Store_IngamePP.php?ItemID=PACK_RETAIL%d", slotID);
		sprintf(url, "Store_IngamePP.php?ItemID=PACK_COLLECTOR_EDITION");
	}

	char full_url[1024];
	sprintf(full_url, "https://account.thewarinc.com/%s&WoLogin=%s", url, authKey);
	ShellExecute(NULL, "open", full_url, "", NULL, SW_SHOW);

	// minimize our window
	ShowWindow(win::hWnd, SW_MINIMIZE);
	needUpdateProfileOnActivate = true;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
}

//
// friends events & processing funcs
//

bool FrontendUI::WaitForFriendsUpdate()
{
	gUserProfile.friends->ClearNewDataFlag();
	gLoginSessionPoller.ForceTick();

	// wait for 60 sec for new data
	const float waitEnd = r3dGetTime() + 60.0f;
	while(r3dGetTime() < waitEnd)
	{
		::Sleep(10);
		
		if(gUserProfile.friends->gotNewData)
			return true;
	}
	
	return false;
}

void FrontendUI::SyncFriendsDataToUI()
{
	Scaleform::GFx::Value var[12];
	
	// clear list
	gfxMovie.Invoke("_root.api.clearFriendsList", var, 0);
	
	// repopulate friends
	EnterCriticalSection(&gUserProfile.friends->csFriends_);
	std::vector<CUserFriends::Friend_s> localFriendCopy = gUserProfile.friends->friendsCur_;
	LeaveCriticalSection(&gUserProfile.friends->csFriends_);

	struct CFriendNameSort {
		static int sort(const void* P1, const void* P2)	{
			const CUserFriends::Friend_s& g1 = *(const CUserFriends::Friend_s*)P1;
			const CUserFriends::Friend_s& g2 = *(const CUserFriends::Friend_s*)P2;
			return stricmp(g1.gamerTag, g2.gamerTag);
		}
	};
	struct CFriendOnlineSort {
		static int sort(const void* P1, const void* P2)	{
			const CUserFriends::Friend_s& g1 = *(const CUserFriends::Friend_s*)P1;
			const CUserFriends::Friend_s& g2 = *(const CUserFriends::Friend_s*)P2;
			return (int)g1.isOnline < (int)g2.isOnline;
		}
	};
	if(localFriendCopy.size() > 2) // sort by online status firstly, then by name
	{
		qsort(&localFriendCopy[0], localFriendCopy.size(), sizeof(CUserFriends::Friend_s), CFriendOnlineSort::sort);
		int lastOnline = 0;
		for(size_t i=0; i<localFriendCopy.size(); ++i)
		{
			if(!localFriendCopy[i].isOnline)
				break;
			lastOnline = i+1;
		}
		if(lastOnline > 0)
		{
			if(lastOnline > 1)
				qsort(&localFriendCopy[0], lastOnline, sizeof(CUserFriends::Friend_s), CFriendNameSort::sort);
			qsort(&localFriendCopy[lastOnline], localFriendCopy.size()-lastOnline, sizeof(CUserFriends::Friend_s), CFriendNameSort::sort);
		}
		else
			qsort(&localFriendCopy[0], localFriendCopy.size(), sizeof(CUserFriends::Friend_s), CFriendNameSort::sort);
	}

	for(std::vector<CUserFriends::Friend_s>::iterator it = localFriendCopy.begin(); 
		it != localFriendCopy.end(); 
		++it)
	{
		const CUserFriends::Friend_s& fr = *it;
		
		wiStats stats = gUserProfile.friends->GetFriendStats(fr.friendID);

		char wlratio[32];
		sprintf(wlratio, "%.2f", getRatio((float)stats.Wins, (float)stats.Losses));
		char kdratio[32];
		sprintf(kdratio, "%.2f", getRatio((float)stats.Kills, (float)stats.Deaths));
		char timePlayed[32] = {0};
		int hours = stats.TimePlayed/3600;
		int minutes = (stats.TimePlayed-hours*3600)/60;
		int sec = stats.TimePlayed%60;
		sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
		
		const char* status = "offline";
		if(fr.isOnline && fr.gameSessionId)
			status = "ingame";
		else if(fr.isOnline)
			status = "online";
			
		var[ 0].SetNumber(fr.friendID);
		var[ 1].SetString(fr.gamerTag);
		var[ 2].SetString(status);
		var[ 3].SetNumber(stats.HonorPoints);
		var[ 4].SetNumber(stats.Wins);
		var[ 5].SetNumber(stats.Losses);
		var[ 6].SetString(wlratio);
		var[ 7].SetNumber(stats.Kills);
		var[ 8].SetNumber(stats.Deaths);
		var[ 9].SetString(kdratio);
		var[10].SetString(timePlayed);
                gfxMovie.Invoke("_root.api.addFriendToList", var, 11);
	}

	gfxMovie.Invoke("_root.api.refreshFriendsList", var, 0);
}

void FrontendUI::UpdateFriendData()
{
	// if we're in middle of something, don't update friends
	if(asyncThread_)
		return;
	
	CUserFriends& friends = *gUserProfile.friends;

	UpdateFriendPopups();
	
	// do nothing if no new data
	if(!friends.gotNewData)
		return;
		
	// query friend add requests
	while(true)
	{
		CUserFriends::AddReq_s req = friends.GetPendingFriendReq();
		if(req.friendID == 0)
			break;

		FriendPopup_s pop(FRPopup_AddReq, req.friendID, req.gamerTag, req.HonorPoints, req.clanName);
		m_friendPopupQuery.push_back(pop);
	}
	
	// sync friends
	EnterCriticalSection(&friends.csFriends_);
	bool isChanged = false;
	
	// check if friend was added/deleted
	if(friends.friendsCur_.size() != friends.friendsPrev_.size())
		isChanged = true;
		
	// see if friend was removed
	for(std::vector<CUserFriends::Friend_s>::iterator it = friends.friendsPrev_.begin(); 
		it != friends.friendsPrev_.end(); 
		++it)
	{
		const CUserFriends::Friend_s& fr  = *it;
		const CUserFriends::Friend_s  now = friends.GetFriendData(fr.friendID);
		const wiStats stats = friends.GetFriendStats(fr.friendID);
		
		if(now.friendID == 0)
		{
			FriendPopup_s pop(FRPopup_FriendRemoved, fr.friendID, fr.gamerTag, stats.HonorPoints, fr.clanName);
			m_friendPopupQuery.push_back(pop);
			
			// and allow friend requests from his again
			friends.ClearSavedFriendReq(fr.friendID);
		}
	}
	
	// loop for current friends, see if something was changed
	for(std::vector<CUserFriends::Friend_s>::iterator it = friends.friendsCur_.begin(); 
		it != friends.friendsCur_.end(); 
		++it)
	{
		const CUserFriends::Friend_s& fr = *it;
		const CUserFriends::Friend_s* prev = friends.GetPrevFriendData(fr.friendID);
		const wiStats stats = friends.GetFriendStats(fr.friendID);
		
		// new friend added
		if(!prev && m_numFriendsUpdates > 0)
		{
			FriendPopup_s pop(FRPopup_FriendAdded, fr.friendID, fr.gamerTag, stats.HonorPoints, fr.clanName);
			m_friendPopupQuery.push_back(pop);

			// in case of friend adding because of them adding you
			// you'll get friend without statistics, so grab it
			if(stats.HonorPoints == 0)
				m_friendsAddedIds.push_back(fr.friendID);
		}
		
		// went online (skip FIRST update)
		if(fr.isOnline && (prev && prev->isOnline != fr.isOnline)) 
		{
			FriendPopup_s pop(FRPopup_Online, fr.friendID, fr.gamerTag, stats.HonorPoints, fr.clanName);
			m_friendPopupQuery.push_back(pop);

			SetUIFriendStatus(fr.friendID, "online");
		}

		// went offline
		if(!fr.isOnline && (prev && prev->isOnline != fr.isOnline)) 
		{
			FriendPopup_s pop(FRPopup_Offline, fr.friendID, fr.gamerTag, stats.HonorPoints, fr.clanName);
			m_friendPopupQuery.push_back(pop);

			SetUIFriendStatus(fr.friendID, "offline");
		}

		// joined/left game
		if(fr.isOnline && (prev && prev->gameSessionId != fr.gameSessionId))
		{
			if(fr.gameSessionId > 0)
			{
				FriendPopup_s pop(FRPopup_GameJoined, fr.friendID, fr.gamerTag, stats.HonorPoints, fr.clanName);
				m_friendPopupQuery.push_back(pop);

				SetUIFriendStatus(fr.friendID, "ingame");
			}
			else
			{
				SetUIFriendStatus(fr.friendID, "online");
			}
		}
	}
	
	// mark that we synched status
	friends.friendsPrev_ = friends.friendsCur_;
	friends.gotNewData   = false;
	
	LeaveCriticalSection(&friends.csFriends_);

	if(isChanged || m_numFriendsUpdates == 0) {
		SyncFriendsDataToUI();
	}
	
	UpdateFriendPopups();
	
	m_numFriendsUpdates++;
	
	// update added friends statistics
	r3d_assert(asyncThread_ == NULL);
	if(m_friendsAddedIds.size()) 
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontendUI::as_GetFriendStatsThread, &FrontendUI::OnGetFriendStatsSuccess);
	}
}

void FrontendUI::SetUIFriendStatus(DWORD friendId, const char* status)
{
	Scaleform::GFx::Value var[2];
	var[0].SetNumber(friendId);
	var[1].SetString(status);
	gfxMovie.Invoke("_root.api.updateFriendStatus", var, 2);
}

void FrontendUI::UpdateFriendPopups()
{
	if(m_friendPopupActive || asyncThread_ != NULL || m_clanPopupActive)
		return;
		
	if(m_friendPopupQuery.size() == 0)
		return;
		
	FriendPopup_s pop = m_friendPopupQuery.front();
	m_friendPopupQuery.pop_front();
		
	Scaleform::GFx::Value var[4];
	var[0].SetNumber(pop.friendId);
	var[1].SetString(pop.gamerTag);
	var[2].SetNumber(pop.HonorPoints);
	var[3].SetString(pop.clanName);
	switch(pop.type)
	{
		default:
			r3dError("crap");
			break;
		case FRPopup_Online: // online
			gfxMovie.Invoke("_root.api.showFriendOnline", var, 4);
			break;
		case FRPopup_Offline: // offline
			gfxMovie.Invoke("_root.api.showFriendOffline", var, 4);
			break;
		case FRPopup_GameJoined: // joined game
			gfxMovie.Invoke("_root.api.showFriendJoinedGame", var, 4);
			break;
		case FRPopup_AddReq: // add request
			gfxMovie.Invoke("_root.api.showFriendRequest", var, 4);
			break;
		case FRPopup_FriendAdded: // friend request accepted - friend added
			gfxMovie.Invoke("_root.api.showFriendRequestAccepted", var, 4);
			break;
		case FRPopup_FriendRemoved: // friend removed
			gfxMovie.Invoke("_root.api.showFriendRemoved", var, 4);
			break;
	}

	m_friendPopupActive = true;
	
	return;
}

unsigned int WINAPI FrontendUI::as_AddFriendThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	This->DelayServerRequest();
	
	int apiCode = gUserProfile.ApiFriendAddReq(This->m_friendAddName, &This->m_friendAddStatus);
	if(apiCode != 0)
	{
		if(apiCode == 6)
			This->SetAsyncError(0, gLangMngr.getString("FriendNotFound"));
		else
			This->SetAsyncError(0, gLangMngr.getString("FriendApiError"));
		return 0;
	}

	return 1;
}

void FrontendUI::OnAddFriendSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");

	// show new message about add status
	Scaleform::GFx::Value var[2];
	var[1].SetBoolean(true);
	switch(m_friendAddStatus)
	{
		default:
			r3d_assert(0);
			break;
		case 0:
			var[0].SetStringW(gLangMngr.getString("FriendAddOk"));
			break;
		case 1:
			var[0].SetStringW(gLangMngr.getString("FriendAddWait"));
			break;
		case 2:
			var[0].SetStringW(gLangMngr.getString("FriendAddAlready"));
			break;
		case 3:
			var[0].SetStringW(gLangMngr.getString("FriendAddDeny"));
			break;
		case 4:
			var[0].SetStringW(gLangMngr.getString("FriendAddBlock"));
			break;
	}
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
	return;
}

void FrontendUI::eventAddFriend(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dscpy(m_friendAddName, args[0].GetString()); // friend name

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_AddFriendThread, &FrontendUI::OnAddFriendSuccess);
}

unsigned int WINAPI FrontendUI::as_RemoveFriendThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;

	This->DelayServerRequest();
	
	int friendStatus = 0;
	int apiCode = gUserProfile.ApiFriendRemove(This->m_friendRemoveId);
	if(apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("FriendApiError"));
		return 0;
	}
	
	// force update, so UpdateFriendData() will sync it to UI
	This->WaitForFriendsUpdate();

	return 1;
}

void FrontendUI::OnRemoveFriendSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	return;
}

void FrontendUI::eventRemoveFriend(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_friendRemoveId = (DWORD)args[0].GetNumber();
	r3d_assert(m_friendRemoveId);

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_RemoveFriendThread, &FrontendUI::OnRemoveFriendSuccess);
}

void FrontendUI::OnJoinFriendGameSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	
	if(!m_friendJoinGameGotPwd && gMasterServerLogic.gameJoinAnswer_.result == GBPKT_M2C_JoinGameAns_s::rWrongPassword)
	{
		m_friendJoinGameGotPwd = true;
		gfxMovie.Invoke("_root.api.showEnterPassword", "");
		return;
	}
	if(!m_friendJoinGameOKHighLevelGame && gMasterServerLogic.gameJoinAnswer_.result == GBPKT_M2C_JoinGameAns_s::rLevelTooLow)
	{
		m_friendJoinGameOKHighLevelGame = true;
		gfxMovie.Invoke("_root.api.showHighLevelGameWarning", "");
		return;
	}

	return;
}

void FrontendUI::eventFriendJoinGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	DWORD friendId = (DWORD)args[0].GetNumber();
	
	CUserFriends::Friend_s fr = gUserProfile.friends->GetFriendData(friendId);
	r3d_assert(fr.friendID);
	if(fr.friendID == 0)
		return;

	if(fr.gameSessionId == 0 || !fr.isOnline)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetStringW(gLangMngr.getString("FriendNotInGame"));
		var[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	
	// fill join packet
	m_friendJoinPacket.CustomerID  = gUserProfile.CustomerID;
	m_friendJoinPacket.FriendID    = friendId;
	m_friendJoinPacket.sessionId   = fr.gameSessionId;
	m_friendJoinPacket.playerLevel = gUserProfile.ProfileData.Stats.getRankLevel();
	m_friendJoinPacket.pwd[0]      = 0;
	m_friendJoinPacket.playerOKToJoinHighLevelGame = false;
	// set friend join mode
	browseJoinSlot_ = -2;
	m_friendJoinGameGotPwd = false;
	m_friendJoinGameOKHighLevelGame = false;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_JoinGameThread, &FrontendUI::OnJoinFriendGameSuccess);
}

unsigned int WINAPI FrontendUI::as_AddFriendAckThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	int friendStatus = 0;
	int apiCode = gUserProfile.ApiFriendAddAns(This->m_friendAckId, This->m_friendAckAllow);
	if(apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("FriendApiError"));
		gUserProfile.friends->ClearSavedFriendReq(This->m_friendAckId);
		return 0;
	}
	
	// force update, so UpdateFriendData() will sync it to UI
	This->WaitForFriendsUpdate();
	
	// fetch new friend stats
	if(This->m_friendAckAllow)
	{
		gUserProfile.ApiFriendGetStats(This->m_friendAckId);
	}
	
	return 1;
}

void FrontendUI::OnAddFriendAckSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	return;
}

void FrontendUI::eventFriendRequestAccept(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_friendAckId    = (DWORD)args[0].GetNumber();
	m_friendAckAllow = true;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_AddFriendAckThread, &FrontendUI::OnAddFriendAckSuccess);
}

void FrontendUI::eventFriendRequestDecline(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_friendAckId    = (DWORD)args[0].GetNumber();
	m_friendAckAllow = false;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_AddFriendAckThread, &FrontendUI::OnAddFriendAckSuccess);
}

unsigned int WINAPI FrontendUI::as_GetFriendStatsThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	r3d_assert(This->m_friendsAddedIds.size() > 0);
	
	for(size_t i = 0, size = This->m_friendsAddedIds.size(); i < size; ++i)
	{
		gUserProfile.ApiFriendGetStats(This->m_friendsAddedIds[i]);
	}
	
	This->m_friendsAddedIds.clear();
	
	return 1;
}

void FrontendUI::OnGetFriendStatsSuccess()
{
	SyncFriendsDataToUI();
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	return;
}

void FrontendUI::eventFriendPopupClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_friendPopupActive = false;
	
	// switch to next popup
	UpdateFriendPopups();
}


//
// leaderboard
//
unsigned int WINAPI FrontendUI::as_GetLeaderboardThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontendUI* This = (FrontendUI*)in_data;
	
	This->DelayServerRequest();

	int friendStatus = 0;
	int apiCode = gUserProfile.ApiGetLeaderboard(This->m_lbTableID, This->m_lbCurPos[This->m_lbTableID], &This->m_lbCurPos[This->m_lbTableID]);
	if(apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("FriendApiError"));
		return 0;
	}
	
	return 1;
}

void FrontendUI::OnGetLeaderboardSuccess()
{
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	
	if(gUserProfile.m_lbData[m_lbTableID].size() == 0)
	{
		// we encountered end of leaderboard, do nothing
		return;
	}

	Scaleform::GFx::Value var[20];
	
	// refresh leaderboard list
	gfxMovie.Invoke("_root.api.clearLeaderboardList", "");
	for(size_t i = 0, size = gUserProfile.m_lbData[m_lbTableID].size(); i < size; ++i)
	{
		const CClientUserProfile::LBEntry_s& lb = gUserProfile.m_lbData[m_lbTableID][i];

		char kdratio[32];
		sprintf(kdratio, "%.2f", getRatio((float)lb.stats.Kills, (float)lb.stats.Deaths));
		char timePlayed[32] = {0};
		int hours = lb.stats.TimePlayed/3600;
		int minutes = (lb.stats.TimePlayed-hours*3600)/60;
		int sec = lb.stats.TimePlayed%60;
		sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
		
		var[0].SetNumber(m_lbCurPos[m_lbTableID] + i + 1);
		var[1].SetBoolean(lb.havePremium);
		var[2].SetString(lb.gamertag);
		var[3].SetNumber(lb.stats.HonorPoints);
		var[4].SetNumber(lb.stats.Wins);
		var[5].SetNumber(lb.stats.Losses);
		var[6].SetNumber(lb.stats.Kills);
		var[7].SetNumber(lb.stats.Deaths);
		var[8].SetString(kdratio);
		var[9].SetString(timePlayed);
		gfxMovie.Invoke("_root.api.addPlayerToLeaderboard", var, 10);
	}

	gfxMovie.Invoke("_root.api.refreshLeaderboardList", "");
	
	return;
}

void FrontendUI::eventOnEnterLeaderboard(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_lbTableID = (int)args[0].GetNumber();
	r3d_assert(m_lbTableID >= 0 && m_lbTableID <= 3);

	if(m_lbTableID[m_lbCurPos] != -1)
	{
		// update leaderboard data with current tab data
		OnGetLeaderboardSuccess();
		return;
	}
		
	// first time update
	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_GetLeaderboardThread, &FrontendUI::OnGetLeaderboardSuccess);
}

void FrontendUI::eventOnLeaderboardButton(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	int btnIdx = (int)args[0].GetNumber();
	
	const int PAGE_SIZE = 100;
	switch(btnIdx)
	{
		default: 
			r3d_assert(0);

		case -1: // prev button
			if(m_lbCurPos[m_lbTableID] == 0)
				return;
			m_lbCurPos[m_lbTableID] = R3D_MAX(m_lbCurPos[m_lbTableID] - PAGE_SIZE, 0);
			break;
				
		case 0: // top 25
			if(m_lbCurPos[m_lbTableID] == 0)
				return;
			m_lbCurPos[m_lbTableID] = 0;
			break;
		
		case 1: // next
			m_lbCurPos[m_lbTableID] += PAGE_SIZE;
			break;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontendUI::as_GetLeaderboardThread, &FrontendUI::OnGetLeaderboardSuccess);
}

static bool isStoreFilteredItem(uint32_t itemId)
{
	// unlock loadout slot items
	if(itemId >= 301050 && itemId <= 301054)
		return true;

	// new unlock loadout slot items
	if(itemId == 301142)
		return true;
		
	if(itemId == 301143 || (itemId >= 301144 && itemId <= 301149)) // clan items
		return true;
		
	// unlock item upgrade slot item
	if(itemId >= 301138 && itemId <= 301141)
		return true;
		
	// generic ability item
	if(itemId == 301055)
		return true;

	// unused abilities (AB_ReservedN)
	if(itemId >= 301073 && itemId <= 301080)
		return true;
		
	return false;
}

static void addAllItemsToStore()
{
	// reset store and add all items from DB
	g_NumStoreItems = 0;

	#define SET_STOREITEM \
		memset(&st, 0, sizeof(st)); \
		st.itemID = item->m_itemID;\
		st.pricePerm = 60000;\
		st.gd_pricePerm = 60000;

	for(int i=0; i<gWeaponArmory.getNumItems(); ++i)
	{
		const ItemConfig* item = gWeaponArmory.getItemConfigByIndex(i);
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}

	for(int i=0; i<gWeaponArmory.getNumWeapons(); ++i)
	{
		const WeaponConfig* item = gWeaponArmory.getWeaponConfigByIndex(i);
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}

	for(int i=0; i<gWeaponArmory.getNumGears(); ++i)
	{
		const GearConfig* item = gWeaponArmory.getGearConfigByIndex(i);
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}
	
	#undef SET_STOREITEM
}

// weirdo function to sort boost items based on it boost type and value
static int BoostItemSort(const void* storeIdx1, const void* storeIdx2)
{
	const ItemConfig* icA = gWeaponArmory.getItemConfig(g_StoreItems[*(int*)storeIdx1].itemID);
	const ItemConfig* icB = gWeaponArmory.getItemConfig(g_StoreItems[*(int*)storeIdx2].itemID);
	r3d_assert(icA && icB);
	
	// detect icon names
	const char* pA_1 = strrchr(icA->m_StoreIcon, '/');
	const char* pB_1 = strrchr(icB->m_StoreIcon, '/');
	r3d_assert(pA_1 && pB_1);
	// sort only boost items, but make 'Items' first.
	if(pA_1[1] != 'B' || pB_1[1] != 'B') {
		return -strcmp(pA_1, pB_1);
	}
	
	// boost have filename in form of Boost_Clipsize_MG_40.dds
	// split it to tokens
	
	// get pointers to tokens
	const char* pA_t1 = strchr(icA->m_StoreIcon, '_');
	const char* pA_t2 = strchr(pA_t1 + 1, '_');
	const char* pA_t3 = strchr(pA_t2 + 1, '_');
	const char* pB_t1 = strchr(icB->m_StoreIcon, '_');
	const char* pB_t2 = strchr(pB_t1 + 1, '_');
	const char* pB_t3 = strchr(pB_t2 + 1, '_');
	if(!pA_t1 || !pA_t2 || !pA_t3)
		return strcmp(pA_1, pB_1);
	if(!pB_t1 || !pB_t2 || !pB_t3)
		return strcmp(pA_1, pB_1);
	
	// make tokens
	char tA[3][64];
	char tB[3][64];
	strcpy(tA[0], pA_t1+1); *strchr(tA[0], '_') = 0;
	strcpy(tA[1], pA_t2+1); *strchr(tA[1], '_') = 0;
	strcpy(tA[2], pA_t3+1); *strchr(tA[2], '.') = 0;
	strcpy(tB[0], pB_t1+1); *strchr(tB[0], '_') = 0;
	strcpy(tB[1], pB_t2+1); *strchr(tB[1], '_') = 0;
	strcpy(tB[2], pB_t3+1); *strchr(tB[2], '.') = 0;
	
	// sort by weapon type
	int cmp1 = strcmp(tA[1], tB[1]);
	if(cmp1) return cmp1;

	// sort by boost type
	int cmp2 = strcmp(tA[0], tB[0]);
	if(cmp2) return cmp2;

	// sort by value
	int cmp3 = atoi(tA[2]) - atoi(tB[2]);
	return cmp3;
}

void FrontendUI::addStore()
{
#if 0
	// add all items to store for test purpose
	addAllItemsToStore();
#endif	

	// add items from store
	std::vector<int> boosts;
	Scaleform::GFx::Value var[10];
	for(uint32_t i=0; i<g_NumStoreItems; ++i)
	{
		if(isStoreFilteredItem(g_StoreItems[i].itemID))
			continue;

		{
			// push boost items for future sorting
			const ItemConfig* ic = gWeaponArmory.getItemConfig(g_StoreItems[i].itemID);
			if(ic && ic->category == storecat_Boost) {
				boosts.push_back(i);
				continue;
			}
			if(ic && ic->category == storecat_Airstrike) // skip it
				continue;
		}
	
		var[0].SetNumber(g_StoreItems[i].itemID);
		var[1].SetNumber(gWeaponArmory.getLevelReqByItemId(g_StoreItems[i].itemID));
		var[2].SetNumber(g_StoreItems[i].price1day);
		var[3].SetNumber(g_StoreItems[i].price7day);
		var[4].SetNumber(g_StoreItems[i].price30day);
		var[5].SetNumber(g_StoreItems[i].pricePerm);
		var[6].SetNumber(g_StoreItems[i].gd_price1day);
		var[7].SetNumber(g_StoreItems[i].gd_price7day);
		var[8].SetNumber(g_StoreItems[i].gd_price30day);
		var[9].SetNumber(g_StoreItems[i].gd_pricePerm);
		
		gfxMovie.Invoke("_root.api.addStoreItem", var, 10);
	}
	
	if(boosts.size() > 0)
		qsort(&boosts[0], boosts.size(), sizeof(int), BoostItemSort);

	// add sorted boosts
	for(size_t j=0; j<boosts.size(); ++j)
	{
		uint32_t i = boosts[j];

		var[0].SetNumber(g_StoreItems[i].itemID);
		var[1].SetNumber(gWeaponArmory.getLevelReqByItemId(g_StoreItems[i].itemID));
		var[2].SetNumber(g_StoreItems[i].price1day);
		var[3].SetNumber(g_StoreItems[i].price7day);
		var[4].SetNumber(g_StoreItems[i].price30day);
		var[5].SetNumber(g_StoreItems[i].pricePerm);
		var[6].SetNumber(g_StoreItems[i].gd_price1day);
		var[7].SetNumber(g_StoreItems[i].gd_price7day);
		var[8].SetNumber(g_StoreItems[i].gd_price30day);
		var[9].SetNumber(g_StoreItems[i].gd_pricePerm);
		
		gfxMovie.Invoke("_root.api.addStoreItem", var, 10);
	}
	
	return;
}

void FrontendUI::initItems()
{
	// add categories
	{
		Scaleform::GFx::Value var[8];
		var[3].SetNumber(0);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		var[6].SetNumber(0);
		var[7].SetBoolean(true); // visible in store

		var[0].SetNumber(storecat_Package);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Package]));
		var[2].SetNumber(4); 
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_Account);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Account]));
		var[2].SetNumber(2); 
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_MysteryBox);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_MysteryBox]));
		var[2].SetNumber(3);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[7].SetBoolean(false); // visible in store
		var[0].SetNumber(storecat_Abilities);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Abilities]));
		var[2].SetNumber(2);
		var[3].SetNumber(4);
		var[4].SetNumber(5);
		var[5].SetNumber(6);
		var[6].SetNumber(7);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);
		
		var[0].SetNumber(storecat_LootBox);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_LootBox]));
		var[2].SetNumber(0);
		var[3].SetNumber(0);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		var[6].SetNumber(0);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_Items);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Items]));
		var[2].SetNumber(2); 
		var[3].SetNumber(4);
		var[4].SetNumber(5);
		var[5].SetNumber(6);
		var[6].SetNumber(7);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_UsableItem);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_UsableItem]));
		var[2].SetNumber(2); 
		var[3].SetNumber(4);
		var[4].SetNumber(5);
		var[5].SetNumber(6);
		var[6].SetNumber(7);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[3].SetNumber(0);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		var[6].SetNumber(0);
		
		var[7].SetBoolean(true); // visible in store
		var[0].SetNumber(storecat_Boost);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Boost]));
		var[2].SetNumber(2); 
		gfxMovie.Invoke("_root.api.addCategory", var, 8);
		var[7].SetBoolean(false); // visible in store

		var[0].SetNumber(storecat_Heroes);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Heroes]));
		var[2].SetNumber(3); // big slot type in armory
		var[3].SetNumber(8);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_Characters);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Characters]));
		var[2].SetNumber(3); // big slot type in armory
		var[3].SetNumber(8);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_Heads);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Heads]));
		var[3].SetNumber(9);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_Gear);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_Gear]));
		var[3].SetNumber(10);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_HeadGear);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_HeadGear]));
		var[3].SetNumber(11);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_ASR);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_ASR]));
		var[2].SetNumber(1);
		var[3].SetNumber(1);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_SNP);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_SNP]));
		var[2].SetNumber(1);
		var[3].SetNumber(1);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_SMG);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_SMG]));
		var[2].SetNumber(1);
		var[3].SetNumber(2);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_SHTG);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_SHTG]));
		var[2].SetNumber(1);
		var[3].SetNumber(2);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);
		var[4].SetNumber(0);

		var[0].SetNumber(storecat_SUPPORT);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_SUPPORT]));
		var[2].SetNumber(1);
		var[3].SetNumber(2);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_MG);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_MG]));
		var[2].SetNumber(1);
		var[3].SetNumber(1);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_HG);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_HG]));
		var[2].SetNumber(2);
		var[3].SetNumber(3);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_MELEE);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_MELEE]));
		var[2].SetNumber(2);
		var[3].SetNumber(3);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_GRENADES);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_GRENADES]));
		var[2].SetNumber(2);
		var[3].SetNumber(4);
		var[4].SetNumber(5);
		var[5].SetNumber(6);
		var[6].SetNumber(7);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[0].SetNumber(storecat_FPSAttachment);
		var[1].SetStringW(gLangMngr.getString(STORE_CATEGORIES_NAMES[storecat_FPSAttachment]));
		var[2].SetNumber(0);
		var[3].SetNumber(0);
		var[4].SetNumber(0);
		var[5].SetNumber(0);
		var[6].SetNumber(0);
		gfxMovie.Invoke("_root.api.addCategory", var, 8);

		var[7].SetBoolean(false); // visible in store
	}

	// add items
	{
		Scaleform::GFx::Value var[8];
		// add weapons
		for(int i=0; i<gWeaponArmory.getNumWeapons(); ++i)
		{
			const WeaponConfig* weapon = gWeaponArmory.getWeaponConfigByIndex(i);
			r3d_assert(weapon);

			var[0].SetNumber(weapon->m_itemID);
			var[1].SetNumber(weapon->category);
			var[2].SetStringW(weapon->m_StoreNameW);
			var[3].SetStringW(weapon->m_DescriptionW);
			var[4].SetString(weapon->m_StoreIcon);
			var[5].SetBoolean(false); // disabled, attachment system used instead //weapon->m_IsUpgradeable?true:false);
			var[6].SetBoolean((weapon->IsFPS&gUserProfile.ProfileData.IsFPSEnabled)?true:false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);

			int accuracy = 0;
			int recoil   = 0;
			getWeaponParamForUI(weapon, NULL, &accuracy, NULL, &recoil);

			int d1=0, d2=0, fr1=0, fr2=0, r1=0, r2=0, c1=0, c2=0;
			getWeaponStatMinMaxForUI(weapon, &d1, &d2, &fr1, &fr2, &c1, &c2, &r1, &r2);

			if(weapon->category!=storecat_UsableItem)
			{
				// order of item stats shouldn't be changed, otherwise weapon upgrades UI will be brocken
				var[1].SetStringW(gLangMngr.getString("StatDamage"));
				var[2].SetNumber(int(weapon->m_AmmoDamage));
				var[3].SetNumber(d1);
				var[4].SetNumber(d2);
				var[5].SetBoolean(true);
				var[6].SetString("damage");
				var[7].SetBoolean(false);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);

				var[1].SetStringW(gLangMngr.getString("StatAccuracy"));
				var[2].SetNumber(accuracy);
				var[3].SetNumber(0);
				var[4].SetNumber(100);
				var[5].SetBoolean(true);
				var[6].SetString("spread");
				var[7].SetBoolean(true);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);

				var[1].SetStringW(gLangMngr.getString("StatRecoil"));
				var[2].SetNumber(recoil);
				var[3].SetNumber(0);
				var[4].SetNumber(100);
				var[5].SetBoolean(true);
				var[6].SetString("recoil");
				var[7].SetBoolean(true);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);

				var[1].SetStringW(gLangMngr.getString("StatFireRate"));
				var[2].SetNumber(int(60.0f / weapon->m_fireDelay));
				var[3].SetNumber(fr1);
				var[4].SetNumber(fr2);
				var[5].SetBoolean(true);
				var[6].SetString("firerate");
				var[7].SetBoolean(false);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);

				var[1].SetStringW(gLangMngr.getString("StatRange"));
				var[2].SetNumber((int)weapon->m_AmmoDecay);
				var[3].SetNumber(r1);
				var[4].SetNumber(r2);
				var[5].SetBoolean(false);
				var[6].SetString("");
				var[7].SetBoolean(false);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);

				var[1].SetStringW(gLangMngr.getString("StatCapacity"));
				var[2].SetNumber(weapon->m_clipSize);
				var[3].SetNumber(c1);
				var[4].SetNumber(c2);
				var[5].SetBoolean(false);
				var[6].SetString("clipsize");
				var[7].SetBoolean(false);
				gfxMovie.Invoke("_root.api.addItemStat", var, 8);
			}

			for(uint32_t j=0; j<gUserProfile.ProfileData.NumFPSAttachments; ++j)
			{
				if(weapon->m_itemID == gUserProfile.ProfileData.FPSAttachments[j].WeaponID && gUserProfile.ProfileData.FPSAttachments[j].isEquipped)
				{
					const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(gUserProfile.ProfileData.FPSAttachments[j].AttachmentID);
					if(attm)
					{
						var[0].SetNumber(weapon->m_itemID);
						var[1].SetNumber(attm->m_type);
						var[2].SetNumber(attm->m_itemID);
						gfxMovie.Invoke("_root.api.setAttachmentSpec", var, 3);
					}
				}
			}
		}
		// add gear
		for(int i=0; i<gWeaponArmory.getNumGears(); ++i)
		{
			const GearConfig* gear = gWeaponArmory.getGearConfigByIndex(i);
			r3d_assert(gear);

			var[0].SetNumber(gear->m_itemID);
			var[1].SetNumber(gear->category);
			var[2].SetStringW(gear->m_StoreNameW);
			var[3].SetStringW(gear->m_DescriptionW);
			var[4].SetString(gear->m_StoreIcon);
			var[5].SetBoolean(false);
			var[6].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);

			var[1].SetStringW(gLangMngr.getString("StatProtection"));
			var[2].SetNumber(ceilf(gear->m_damagePerc*100.0f)); 
			var[3].SetNumber(0);
			var[4].SetNumber(100);
			var[5].SetBoolean(true);
			var[6].SetString("");
			var[7].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItemStat", var, 8);

			var[1].SetStringW(gLangMngr.getString("StatWeight"));
			float mobility = 1.0f/(1.0f + gear->m_Weight/100.0f); // each 100kg halfs your moving speed
			
			var[2].SetNumber(ceilf(mobility*100.0f));
			var[3].SetNumber(0); 
			var[4].SetNumber(100); 
			var[5].SetBoolean(true);
			var[6].SetString("");
			var[7].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItemStat", var, 8);
		}
		for(int i=0; i<gWeaponArmory.getNumItems(); ++i)
		{
			const ItemConfig* item= gWeaponArmory.getItemConfigByIndex(i);
			r3d_assert(item);

			// add items
			var[0].SetNumber(item->m_itemID);
			var[1].SetNumber(item->category);
			var[2].SetStringW(item->m_StoreNameW);
			var[3].SetStringW(item->m_DescriptionW);
			var[4].SetString(item->m_StoreIcon);
			var[5].SetBoolean(false);
			var[6].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);
		}
		for(int i=0; i<gWeaponArmory.getNumAttachments(); ++i)
		{
			const WeaponAttachmentConfig* attm= gWeaponArmory.getAttachmentConfigByIndex(i);
			r3d_assert(attm);

			// add items
			var[0].SetNumber(attm->m_itemID);
			var[1].SetNumber(storecat_FPSAttachment);
			var[2].SetStringW(attm->m_StoreNameW);
			var[3].SetStringW(attm->m_DescriptionW);
			var[4].SetString(attm->m_StoreIcon);
			var[5].SetBoolean(false);
			var[6].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);

			var[0].SetNumber(attm->m_itemID);
			var[1].SetNumber(attm->m_type);
			var[2].SetNumber(attm->m_Damage);
			var[3].SetNumber(attm->m_Range);
			var[4].SetNumber(attm->m_Firerate);
			var[5].SetNumber(attm->m_Recoil);
			var[6].SetNumber(attm->m_Spread);
			var[7].SetNumber(attm->m_Clipsize);
			gfxMovie.Invoke("_root.api.addAttachmentData", var, 8);
		}
		for(int i=0; i<gWeaponArmory.getNumPackages(); ++i)
		{
			const PackageConfig* pkg= gWeaponArmory.getPackageConfigByIndex(i);
			r3d_assert(pkg);

			// add packages
			var[0].SetNumber(pkg->m_itemID);
			var[1].SetNumber(pkg->category);
			var[2].SetStringW(pkg->m_StoreNameW);
			var[3].SetStringW(pkg->m_DescriptionW);
			var[4].SetString(pkg->m_StoreIcon);
			var[5].SetBoolean(false);
			var[6].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);

			Scaleform::GFx::Value prm[4];
			prm[0].SetNumber(pkg->m_itemID);
			prm[1].SetNumber(0);
			prm[2].SetNumber(pkg->m_addGD); // gd
			prm[3].SetNumber(pkg->m_addSP);
			gfxMovie.Invoke("_root.api.addPackageInfo", prm, 4);

			prm[3].SetNumber(0); // exp2 for mystery crates

			prm[1].SetNumber(pkg->m_itemID1);
			prm[2].SetNumber(pkg->m_itemID1Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);

			prm[1].SetNumber(pkg->m_itemID2);
			prm[2].SetNumber(pkg->m_itemID2Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);

			prm[1].SetNumber(pkg->m_itemID3);
			prm[2].SetNumber(pkg->m_itemID3Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);

			prm[1].SetNumber(pkg->m_itemID4);
			prm[2].SetNumber(pkg->m_itemID4Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);

			prm[1].SetNumber(pkg->m_itemID5);
			prm[2].SetNumber(pkg->m_itemID5Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);

			prm[1].SetNumber(pkg->m_itemID6);
			prm[2].SetNumber(pkg->m_itemID6Exp*1440);
			gfxMovie.Invoke("_root.api.addItemToPackage", prm, 4);
		}
		for(int i=0; i<20; ++i) // add temp WP items for showing mystery box content
		{
			// add items
			var[0].SetNumber(1000+i); // use range 1000..., it is not used anywhere and should be safe
			var[1].SetNumber(storecat_INVALID);
			var[2].SetStringW(L"WP item");
			var[3].SetStringW(L"");
			var[4].SetString("$Data/Weapons/StoreIcons/WPIcon.dds");
			var[5].SetBoolean(false);
			var[6].SetBoolean(false);
			gfxMovie.Invoke("_root.api.addItem", var, 7);
		}
	}

	// add inventory
	updateInventoryAndSkillItems();
	addStore();

	// add promo items
	{
		// temp, replace with proper promo items
		Scaleform::GFx::Value var[2];
		if(!RUS_CLIENT)
		{
			for(uint32_t i=0; i<g_NumStoreItems; i+=10)
			{
				var[0].SetNumber(g_StoreItems[i].itemID);
				var[1].SetString(gWeaponArmory.getIconByItemId(g_StoreItems[i].itemID));
				gfxMovie.Invoke("_root.api.addPromoItem", var, 2);
			}
		}
		else
		{
			var[0].SetNumber(500021);
			var[1].SetString(gWeaponArmory.getIconByItemId(500021));
			gfxMovie.Invoke("_root.api.addPromoItem", var, 2);

			var[0].SetNumber(500020);
			var[1].SetString(gWeaponArmory.getIconByItemId(500020));
			gfxMovie.Invoke("_root.api.addPromoItem", var, 2);

			var[0].SetNumber(500019);
			var[1].SetString(gWeaponArmory.getIconByItemId(500019));
			gfxMovie.Invoke("_root.api.addPromoItem", var, 2);
		}

	}
}

void FrontendUI::addSkillItemToInventory(uint32_t itemID)
{
	Scaleform::GFx::Value var[4];
	var[0].SetNumber(itemID);
	var[1].SetNumber(2000*1440); // show as permanent for now. todo: show skill based string
	var[2].SetNumber(1);
	var[3].SetBoolean(false);
	gfxMovie.Invoke("_root.api.addInventoryItem", var, 4);
}

// check what is the current loadout slot and adds skill items to loadout
void FrontendUI::updateInventoryAndSkillItems()
{
	Scaleform::GFx::Value var[4];
	// clear inventory DB
	gfxMovie.Invoke("_root.api.clearInventory", NULL, 0);

	// add all items
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
	{
		var[0].SetNumber(gUserProfile.ProfileData.Inventory[i].itemID);
		var[1].SetNumber(gUserProfile.ProfileData.Inventory[i].expiration); //UI expects expiration to be in minutes
		var[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
		bool isConsumable = false;
		{
			const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(gUserProfile.ProfileData.Inventory[i].itemID);
			if(wc && wc->category == storecat_UsableItem && wc->m_numClips > 1)
				isConsumable = true;
		}
		var[3].SetBoolean(isConsumable);
		gfxMovie.Invoke("_root.api.addInventoryItem", var, 4);
	}

	// now check for special skills and add those items manually
	const wiLoadoutSlot& curLoadout = gUserProfile.ProfileData.ArmorySlots[m_CurrentModifySlot];
	if(curLoadout.getSkillLevel(CUserSkills::ASSAULT_RiotShield))
		addSkillItemToInventory(WeaponConfig::ITEMID_RiotShield);
	if(curLoadout.getSkillLevel(CUserSkills::ASSAULT_AdrenalineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_AdrenalineShot);
	if(curLoadout.getSkillLevel(CUserSkills::ASSAULT_SpawnBeacon))
		addSkillItemToInventory(WeaponConfig::ITEMID_RespawnBeacon);
	if(curLoadout.getSkillLevel(CUserSkills::ASSAULT_AutoTurret))
		addSkillItemToInventory(WeaponConfig::ITEMID_AutoTurret);
	if(curLoadout.getSkillLevel(CUserSkills::RECON_MotionSensor))
		addSkillItemToInventory(WeaponConfig::ITEMID_MotionSensor);
	if(curLoadout.getSkillLevel(CUserSkills::RECON_RespawnBeacon))
		addSkillItemToInventory(WeaponConfig::ITEMID_RespawnBeacon);
	if(curLoadout.getSkillLevel(CUserSkills::RECON_AdrenalineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_AdrenalineShot);
	if(curLoadout.getSkillLevel(CUserSkills::RECON_AutoTurret))
		addSkillItemToInventory(WeaponConfig::ITEMID_AutoTurret);
	if(curLoadout.getSkillLevel(CUserSkills::RECON_MorphineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_MorphineShot);
	if(curLoadout.getSkillLevel(CUserSkills::SPEC_RiotShield))
		addSkillItemToInventory(WeaponConfig::ITEMID_RiotShield);
	if(curLoadout.getSkillLevel(CUserSkills::SPEC_AutoTurret))
		addSkillItemToInventory(WeaponConfig::ITEMID_AutoTurret);
	if(curLoadout.getSkillLevel(CUserSkills::SPEC_AdrenalineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_AdrenalineShot);
	if(curLoadout.getSkillLevel(CUserSkills::SPEC_SpawnBeacon))
		addSkillItemToInventory(WeaponConfig::ITEMID_RespawnBeacon);
	if(curLoadout.getSkillLevel(CUserSkills::MEDIC_Medkit))
		addSkillItemToInventory(WeaponConfig::ITEMID_MedKit);
	if(curLoadout.getSkillLevel(CUserSkills::MEDIC_MorphineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_MorphineShot);
	if(curLoadout.getSkillLevel(CUserSkills::MEDIC_AdrenalineShot))
		addSkillItemToInventory(WeaponConfig::ITEMID_AdrenalineShot);
	if(curLoadout.getSkillLevel(CUserSkills::MEDIC_RiotShield))
		addSkillItemToInventory(WeaponConfig::ITEMID_RiotShield);
}

void FrontendUI::updateAllAttachments()
{
	Scaleform::GFx::Value var[3];
	for(int i=0; i<gWeaponArmory.getNumWeapons(); ++i)
	{
		const WeaponConfig* weapon = gWeaponArmory.getWeaponConfigByIndex(i);
		r3d_assert(weapon);

		for(uint32_t j=0; j<gUserProfile.ProfileData.NumFPSAttachments; ++j)
		{
			if(weapon->m_itemID == gUserProfile.ProfileData.FPSAttachments[j].WeaponID && gUserProfile.ProfileData.FPSAttachments[j].isEquipped)
			{
				const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(gUserProfile.ProfileData.FPSAttachments[j].AttachmentID);
				if(attm)
				{
					var[0].SetNumber(weapon->m_itemID);
					var[1].SetNumber(attm->m_type);
					var[2].SetNumber(attm->m_itemID);
					gfxMovie.Invoke("_root.api.setAttachmentSpec", var, 3);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void FrontendUI::D3DCreateResource()
{
	needReInitScaleformTexture = true;
}

//////////////////////////////////////////////////////////////////////////

static float aquireProfileStart = 0;
static HANDLE handleGetProfileData = 0;

extern int g_CCBlackWhite;
extern float g_fCCBlackWhitePwr;

bool FrontendUI::Initialize()
{
	bindRTsToScaleForm() ;

	// check for bad values in contrast, brightness
	if(r_contrast->GetFloat() < r_contrast->GetMinVal() || r_contrast->GetFloat() > r_contrast->GetMaxVal())
		r_contrast->SetFloat(0.5f);
	if(r_brightness->GetFloat() < r_brightness->GetMinVal() || r_brightness->GetFloat() > r_brightness->GetMaxVal())
		r_brightness->SetFloat(0.5f);
	
	// reset filters
	r_hud_filter_mode->SetInt(HUDFilter_Default);
	g_fCCBlackWhitePwr = 0;
	g_CCBlackWhite = 0;

	if(g_mouse_sensitivity->GetFloat() < g_mouse_sensitivity->GetMinVal() || g_mouse_sensitivity->GetFloat() > g_mouse_sensitivity->GetMaxVal())
		g_mouse_sensitivity->SetFloat(0.5f);
	if(s_sound_volume->GetFloat() < s_sound_volume->GetMinVal() || s_sound_volume->GetFloat() > s_sound_volume->GetMaxVal())
		s_sound_volume->SetFloat(1.0f);
	if(s_music_volume->GetFloat() < s_music_volume->GetMinVal() || s_music_volume->GetFloat() > s_music_volume->GetMaxVal())
		s_music_volume->SetFloat(1.0f);
	if(s_comm_volume->GetFloat() < s_comm_volume->GetMinVal() || s_comm_volume->GetFloat() > s_comm_volume->GetMaxVal())
		s_comm_volume->SetFloat(1.0f);

	if(RUS_CLIENT)
		m_pBackgroundTex = r3dRenderer->LoadTexture("Data/Menu/Background_rus.dds");
	else
		m_pBackgroundTex = r3dRenderer->LoadTexture("Data/Menu/Background.dds");
	r3d_assert(m_pBackgroundTex);
	//m_pBackgroundPremiumTex = r3dRenderer->LoadTexture("Data/Menu/Background_Premium.dds");
	//r3d_assert(m_pBackgroundPremiumTex);

	gProfileIsAquired = 0;
	aquireProfileStart = r3dGetTime();
	handleGetProfileData = (HANDLE)_beginthreadex(NULL, 0, &GetProfileDataThread, this, 0, 0);
	if(handleGetProfileData == 0)
		r3dError("Failed to begin thread");

#if ENABLE_WEB_BROWSER
	if(frontendFirstTimeInit)
	{
		if(RUS_CLIENT==0 || (RUS_CLIENT && !showBuyPackMovie)) // for rus client, if we are showing buy pack movie, than do not show welcome back screen
		{
			d_show_browser->SetBool(true);
			float scaleX = r3dRenderer->ScreenW/1920.0f;
			float scaleY = r3dRenderer->ScreenH/1080.0f;
			g_pBrowserManager->NotifyWindowPos(int(97.0f*scaleX), int(503*scaleY), 1.0f/scaleX, 1.0f/scaleY); // WB RT pos: 97, 503

			if(RUS_CLIENT)
				g_pBrowserManager->SetURL("https://warinc-api.gamenet.ru/gameui/rotator/");
			else
				g_pBrowserManager->SetURL("http://thewarinc.com/gameui/rotator/");
		}
	}
#endif

	// reacquire the menu.
	gfxMovie.SetKeyboardCapture();

	r_film_tone_a->SetFloat(0.15f);
	r_film_tone_b->SetFloat(0.50f);
	r_film_tone_c->SetFloat(0.10f);
	r_film_tone_d->SetFloat(0.20f);
	r_film_tone_e->SetFloat(0.02f);
	r_film_tone_f->SetFloat(0.30f);
	r_exposure_bias->SetFloat(0.5f);
	r_white_level->SetFloat(11.2f);

	m_needPlayerRenderingRequest = 0;
	m_CurrentModifySlot = 0;

	gClientLogic().Reset(); // reset game finished, otherwise player will not update and will not update its skelet and will not render

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	// show info message and render it one time
	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("RetrievingProfileData"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	if( r3dRenderer->DeviceAvailable )
	{
		// advance movie by 5 frames, so info screen will fade in and show
		Scaleform::GFx::Movie* pMovie = gfxMovie.GetMovie();
		pMovie->Advance((1.0f/pMovie->GetFrameRate()) * 5);

		r3dRenderer->StartFrame();
		ClearFullScreen_Menu();
		r3dRenderer->StartRender(1);

		r3d_assert(m_pBackgroundTex);
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		r3dDrawBoxFS(r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor::white, m_pBackgroundTex);

		gfxMovie.UpdateAndDraw();

		r3dRenderer->Flush();
		CurRenderPipeline->Finalize() ;
		r3dRenderer->EndFrame();
	}
	r3dRenderer->EndRender( true );
	
	// init things to load game level
	r3dGameLevel::SetHomeDir("WO_ShootingRange");
	extern void InitGame_Start();
	InitGame_Start();

	// load buy pack SWF
	if(RUS_CLIENT)
		gfxBuyPackMovie.Load("data\\menu\\AddMenu01_BuyPacks.swf", false);
	else
		gfxBuyPackMovie.Load("data\\menu\\AddMenu03_BuyPacks.swf", false);
	gfxBuyPackMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit);

	#ifndef LOAD_DATA_IN_BACKGROUND
	LoadFrontendGameData(this);
	#endif

	return true;
}

void FrontendUI::bindRTsToScaleForm()
{
	RTScaleformTexture = gfxMovie.BoundRTToImage("merc_rendertarget", Scaleform_RenderToTextureRT->AsTex2D(), (int)Scaleform_RenderToTextureRT->Width, (int)Scaleform_RenderToTextureRT->Height);
#if ENABLE_WEB_BROWSER
	g_pBrowserManager->SetSize(1231, 525);
	RTWelcomeBackScaleformTexture = gfxMovie.BoundRTToImage("wb_rendertarget", g_pBrowserManager->GetWindow()->AsTex2D(), 1231, 525);
#endif
}


bool FrontendUI::Unload()
{
	gfxBuyPackMovie.Unload();
	
	if(hudMain)
	{
		hudMain->Unload();
		SAFE_DELETE(hudMain);
	}

#if ENABLE_WEB_BROWSER
	d_show_browser->SetBool(false);
	g_pBrowserManager->SetSize(4, 4);
#endif


	return UIMenu::Unload();
}

void FrontendUI::setBaseStats()
{
	CUserProfile& prof = gUserProfile;
	const wiStats& stat = prof.ProfileData.Stats;
	{ // set base data
		Scaleform::GFx::Value var[4];
		var[0].SetStringW(prof.ScreenNameW);
		var[1].SetNumber(stat.HonorPoints);
		var[2].SetNumber(stat.GamePoints);
		var[3].SetNumber(stat.GameDollars);
		gfxMovie.Invoke("_root.api.setUsername", var, 4);
	}
	{ // set base stats
		Scaleform::GFx::Value var[7];
		var[0].SetNumber(stat.SkillPoints);
		var[1].SetNumber(stat.Wins);
		var[2].SetNumber(stat.Losses);
		var[3].SetNumber(stat.Kills);
		var[4].SetNumber(stat.Deaths);
		char tempStr[32];
		sprintf(tempStr, "%.2f", getRatio((float)stat.Kills, (float)stat.Deaths));
		var[5].SetString(tempStr);
		char timePlayed[32] = {0};
		int hours = stat.TimePlayed/3600;
		int minutes = (stat.TimePlayed-hours*3600)/60;
		int sec = stat.TimePlayed%60;
		sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
		var[6].SetString(timePlayed);
		gfxMovie.Invoke("_root.api.setStats", var, 7);
	}
}

static void temp_GetRusLvlUpBaseRewards(const WeaponArmory::LevelUpBonus& lub, wchar_t* rwdstr)
{
	wchar_t tempstr1[256] = L"";
	wchar_t tempstr2[256] = L"";
	wchar_t tempstr3[256] = L"";

	// GP-GD-SP rewards
	if(lub.gp > 0) 
		swprintf(tempstr1, 256, L"%s: %d", gLangMngr.getString("$FR_StatGamePoints"), lub.gp);
	if(lub.gd > 0) 
		swprintf(tempstr2, 256, L"%s: %d", gLangMngr.getString("$FR_StatGameDollars"), lub.gd);
	if(lub.sp > 0)
		swprintf(tempstr3, 256, L"%s: %d", gLangMngr.getString("$FR_StatSkillPoints"), lub.sp);

	// combine to single reward string
	rwdstr[0] = 0;
	if(tempstr1[0]) {
		wcscat(rwdstr, tempstr1);
	}
	if(tempstr2[0]) {
		if(rwdstr[0]) wcscat(rwdstr, L"\n");
		wcscat(rwdstr, tempstr2);
	}
	if(tempstr3[0]) {
		if(rwdstr[0]) wcscat(rwdstr, L"\n");
		wcscat(rwdstr, tempstr3);
	}
}

void FrontendUI::setAdditionalStats()
{
	CUserProfile& prof = gUserProfile;
	const wiStats& stat = prof.ProfileData.Stats;
	{ // set additional stats
		Scaleform::GFx::Value var[8];
		var[0].SetNumber(stat.Headshots);
		var[1].SetNumber(stat.AssistKills);
		char wlr[32];
		sprintf(wlr, "%.2f", getRatio((float)stat.Wins, (float)stat.Losses));
		var[2].SetString(wlr);
		var[3].SetNumber(stat.CaptureNeutralPoints);
		var[4].SetNumber(stat.CaptureEnemyPoints);
		var[5].SetNumber(stat.ShotsFired);
		var[6].SetNumber(stat.ShotsHits);
		char acc[32];
		sprintf(acc, "%.2f", getRatio((float)stat.ShotsHits, (float)stat.ShotsFired));
		var[7].SetString(acc);
		gfxMovie.Invoke("_root.api.setAdditionalStats", var, 8);
	}
}

void FrontendUI::initFrontend()
{
	// load in-game HUD for shooting range
	r3d_assert(hudMain == NULL);
	hudMain = new HUDDisplay();
	hudMain->Init(0, true);

	gfxMovie.SetKeyboardCapture(); // reset it to us after hud

	// buy pack API
	{
		gfxBuyPackMovie.SetVariable("_global.isSteamEnabled", gSteam.inited_);
		gfxBuyPackMovie.Invoke("_root.Title.gotoAndStop", gSteam.inited_?2:1);
		gfxBuyPackMovie.Invoke("_root.Btn.Text.gotoAndStop", gSteam.inited_?2:1);

		if(RUS_CLIENT)
			gfxBuyPackMovie.Invoke("_global.switchToRus", "");

		if(frontendRequestShowBuyPack)
		{
			showBuyPackMovie = true;
			frontendRequestShowBuyPack = false;
		}
	}

	// steam API
	if(gSteam.inited_)
	{
		gfxMovie.Invoke("_root.api.enableSteam", "");

		// we should always have at least 3 buy options for steam, otherwise UI will be broken
		Scaleform::GFx::Value var[4];
		for(uint32_t i =0; i<gUserProfile.steamGPShopData.Count(); ++i)
		{
			var[0].SetNumber(gUserProfile.steamGPShopData[i].gpItemID);
			var[1].SetNumber(gUserProfile.steamGPShopData[i].GP);
			var[2].SetNumber(gUserProfile.steamGPShopData[i].BonusGP);
			char tempS[16];
			sprintf(tempS, "%.2f", float(gUserProfile.steamGPShopData[i].PriceUSD)/100.0f);
			var[3].SetString(tempS); // pass as string, otherwise stupid flash will show price as 1.384748343874
			gfxMovie.Invoke("_root.api.addSteamGPOption", var, 4);
		}
	}

	gfxMovie.Invoke("_root.api.setFPSOnlyMode", gUserProfile.ProfileData.IsFPSEnabled);

	// set rank data
	{
		char tempB[64];
		Scaleform::GFx::Value var[2];
		for(int i=0; i<MAX_NUM_RANKS; ++i)
		{
			sprintf(tempB, "PlayerRank%d", i);
			var[0].SetStringW(gLangMngr.getString(tempB));
			if(i==0)
				var[1].SetNumber(0);
			else
				var[1].SetNumber(g_RanksPoints[i-1]);
			gfxMovie.Invoke("_root.api.m_RanksData.addRank", var, 2);
		}
	}
	// set rank reward info
	{
		Scaleform::GFx::Value var[7];
		for(int i=0; i<MAX_NUM_RANKS; ++i)
		{
			if(gWeaponArmory.m_LevelUpBonus[i].nextLevel != -1)
			{
				const WeaponArmory::LevelUpBonus& lub = gWeaponArmory.m_LevelUpBonus[i];
				var[0].SetNumber(lub.nextLevel-1);
				if(!RUS_CLIENT)
				{
					var[1].SetString(lub.reward1);
					var[2].SetString(lub.reward2);
					var[3].SetString(lub.reward3);
					var[4].SetString("");
					var[5].SetString("");
					var[6].SetString("");
				}
				else
				{
					int c = 1; // index in var[]

					wchar_t rwdstr[512];
					temp_GetRusLvlUpBaseRewards(lub, rwdstr);
					if(rwdstr[0])
						var[c++].SetStringW(rwdstr);
							
					// items
					for(int j=0; j<8 && c < 7; j++)
					{
						if(lub.items[j] == 0) 
							continue;
						const char* name = gWeaponArmory.getNameByItemId(lub.items[j]);
						if(!name)
							continue;

						r3d_assert(c <= 6);
						var[c++].SetString(name);
					}
					
					// empty rewards
					while(c <= 6) var[c++].SetString("");
				}
				gfxMovie.Invoke("_root.api.addRankRewardInfo", var, 7);
			}
		}
	}
	// add reward info
	{
		Scaleform::GFx::Value var[7];

		for(int i=0; i<RWD_MAX_REWARD_ID; i++) 
		{
			const CGameRewards::rwd_s& rwd = g_GameRewards->GetRewardData(i);
			if(!rwd.IsSet)
				continue;
				
			char buf1[128];
			char buf2[128];
			char buf3[128];
			sprintf(buf1, "Reward_%s", rwd.Name.c_str());
			sprintf(buf2, "Reward_%sDesc", rwd.Name.c_str());
			sprintf(buf3, "$Data/Menu/Rewards/RWD_%s.png", rwd.Name.c_str());

			// name/description/icon
			var[1].SetStringW(gLangMngr.getString(buf1));
			var[2].SetStringW(gLangMngr.getString(buf2));
			var[3].SetString(buf3);
			var[5].SetNumber(0); // gp
			
			// CONQUEST reward
			if(rwd.GD_CQ || rwd.HP_CQ)
			{
				var[0].SetNumber(0);
				var[4].SetNumber(rwd.GD_CQ); // gd
				var[6].SetNumber(rwd.HP_CQ); // xp
				gfxMovie.Invoke("_root.api.addRewardInfo", var, 7);
			}

			// DEATHMATCH reward
			if(rwd.GD_DM || rwd.HP_DM)
			{
				var[0].SetNumber(1);
				var[4].SetNumber(rwd.GD_DM); // gd
				var[6].SetNumber(rwd.HP_DM); // xp
				gfxMovie.Invoke("_root.api.addRewardInfo", var, 7);
			}

			// SABOTAGE reward
			if(rwd.GD_SB || rwd.HP_SB)
			{
				var[0].SetNumber(2);
				var[4].SetNumber(rwd.GD_SB); // gd
				var[6].SetNumber(rwd.HP_SB); // xp
				gfxMovie.Invoke("_root.api.addRewardInfo", var, 7);
			}
		}
	}

	if(RUS_CLIENT || g_force_rus_lang->GetInt())
	{
		gfxMovie.Invoke("_root.api.setLanguage", "russian");
		gfxMovie.Invoke("_root.api.disableLanguageSelection", "");
	}
	else if(MASSIVE_CLIENT)
	{
		gfxMovie.Invoke("_root.api.setLanguage", "english");
		gfxMovie.Invoke("_root.api.disableLanguageSelection", "");
		gfxMovie.SetVariable("_root.api.m_MassiveClient", 1);
	}
	else
	{
		gfxMovie.Invoke("_root.api.setLanguage", g_user_language->GetString());
	}

	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	r_gameplay_blur_strength->SetFloat(0.0f); // just to make sure

	memset(m_GameJoin_Password, 0, sizeof(m_GameJoin_Password));

/*
	// DISABLED FOR NOW - it's leaking memory now on resp
	
	// start fetching news page
	CkHttpRequest req;
	int success = httpNewsRequest.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if (success != 1) 
	{
		r3dError("Failed to init network!\n");
		return;
	}
	httpNewsRequest.put_UseBgThread(true);
	httpNewsRequest.put_KeepEventLog(true);
	req.UsePost();
	req.put_Path("/GetGameNews.php");
	CkHttpResponse *resp = httpNewsRequest.SynchronousRequest("www.thewarinc.com",80,false,req);
	waitingForNews = true;
*/	
	
	CUserProfile& prof = gUserProfile;
	
#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<FrontendUI>(this, &FrontendUI::FUNC)
	gfxBuyPackMovie.RegisterEventHandler("eventBuyPackContinue", MAKE_CALLBACK(eventBuyPackContinue));
	gfxBuyPackMovie.RegisterEventHandler("eventBuyPackSteam", MAKE_CALLBACK(eventBuyPackSteam));
	gfxBuyPackMovie.RegisterEventHandler("eventBuyPackNonSteam", MAKE_CALLBACK(eventBuyPackNonSteam));

	gfxMovie.RegisterEventHandler("eventArmorySelectLoadoutSlot", MAKE_CALLBACK(eventArmorySelectLoadoutSlot));
	gfxMovie.RegisterEventHandler("eventArmorySaveLoadoutSlot", MAKE_CALLBACK(eventArmorySaveLoadoutSlot));
	gfxMovie.RegisterEventHandler("eventArmoryModifyOnEquip", MAKE_CALLBACK(eventArmoryModifyOnEquip));
	gfxMovie.RegisterEventHandler("eventArmoryCurrentWeaponSlot", MAKE_CALLBACK(eventArmoryCurrentWeaponSlot));
	gfxMovie.RegisterEventHandler("eventArmoryRequestCharacterRender", MAKE_CALLBACK(eventArmoryRequestCharacterRender));
	gfxMovie.RegisterEventHandler("eventArmoryRequestUnlockLoadoutSlot", MAKE_CALLBACK(eventArmoryRequestUnlockLoadoutSlot));
	gfxMovie.RegisterEventHandler("eventBrowseGamesRefresh", MAKE_CALLBACK(eventBrowseGamesRefresh));
	gfxMovie.RegisterEventHandler("eventBrowseGamesUpdateFilter", MAKE_CALLBACK(eventBrowseGamesUpdateFilter));
	gfxMovie.RegisterEventHandler("eventBrowseRegionRefresh", MAKE_CALLBACK(eventBrowseRegionRefresh));
	gfxMovie.RegisterEventHandler("eventBrowseGamesJoin", MAKE_CALLBACK(eventBrowseGamesJoin));
	gfxMovie.RegisterEventHandler("eventBrowseGamesEnterPassword", MAKE_CALLBACK(eventBrowseGamesEnterPassword));
	gfxMovie.RegisterEventHandler("eventCreateGame", MAKE_CALLBACK(eventCreateGame));
	gfxMovie.RegisterEventHandler("eventJoinQuickGame", MAKE_CALLBACK(eventJoinQuickGame));
	gfxMovie.RegisterEventHandler("eventStoreBuyItem", MAKE_CALLBACK(eventStoreBuyItem));
	gfxMovie.RegisterEventHandler("eventUnlockPremiumAccount", MAKE_CALLBACK(eventUnlockPremiumAccount));
	gfxMovie.RegisterEventHandler("eventExit", MAKE_CALLBACK(eventExit));
	gfxMovie.RegisterEventHandler("eventOptionsReset", MAKE_CALLBACK(eventOptionsReset));
	gfxMovie.RegisterEventHandler("eventOptionsApply", MAKE_CALLBACK(eventOptionsApply));
	gfxMovie.RegisterEventHandler("eventOptionsControlsReset", MAKE_CALLBACK(eventOptionsControlsReset));
	gfxMovie.RegisterEventHandler("eventOptionsControlsApply", MAKE_CALLBACK(eventOptionsControlsApply));
	gfxMovie.RegisterEventHandler("eventOptionsLanguageSelection", MAKE_CALLBACK(eventOptionsLanguageSelection));
	gfxMovie.RegisterEventHandler("eventLearnSkill", MAKE_CALLBACK(eventLearnSkill));
	gfxMovie.RegisterEventHandler("eventResetSkillTree", MAKE_CALLBACK(eventResetSkillTree));
	gfxMovie.RegisterEventHandler("eventOptionsControlsRequestKeyRemap", MAKE_CALLBACK(eventOptionsControlsRequestKeyRemap));
	gfxMovie.RegisterEventHandler("eventRenewItem", MAKE_CALLBACK(eventRenewItem));
	gfxMovie.RegisterEventHandler("eventWelcomeBackBuyItem", MAKE_CALLBACK(eventWelcomeBackBuyItem));
	gfxMovie.RegisterEventHandler("eventConfirmHighLevelGameJoin", MAKE_CALLBACK(eventConfirmHighLevelGameJoin));
	gfxMovie.RegisterEventHandler("eventStoreChangeGamertag", MAKE_CALLBACK(eventStoreChangeGamertag));
	gfxMovie.RegisterEventHandler("eventStorePurchaseGP", MAKE_CALLBACK(eventStorePurchaseGP));
	gfxMovie.RegisterEventHandler("eventStorePurchaseGP_Steam", MAKE_CALLBACK(eventStorePurchaseGP_Steam));
	gfxMovie.RegisterEventHandler("eventStoreEntered", MAKE_CALLBACK(eventStoreEntered));
	gfxMovie.RegisterEventHandler("eventStoreRequestPackageDetails", MAKE_CALLBACK(eventStoreRequestPackageDetails));
	gfxMovie.RegisterEventHandler("eventAddFriend", MAKE_CALLBACK(eventAddFriend));
	gfxMovie.RegisterEventHandler("eventRemoveFriend", MAKE_CALLBACK(eventRemoveFriend));
	gfxMovie.RegisterEventHandler("eventFriendJoinGame", MAKE_CALLBACK(eventFriendJoinGame));
	gfxMovie.RegisterEventHandler("eventFriendRequestAccept", MAKE_CALLBACK(eventFriendRequestAccept));
	gfxMovie.RegisterEventHandler("eventFriendRequestDecline", MAKE_CALLBACK(eventFriendRequestDecline));
	gfxMovie.RegisterEventHandler("eventFriendPopupClosed", MAKE_CALLBACK(eventFriendPopupClosed));
	gfxMovie.RegisterEventHandler("eventOnEnterLeaderboard", MAKE_CALLBACK(eventOnEnterLeaderboard));
	gfxMovie.RegisterEventHandler("eventOnLeaderboardButton", MAKE_CALLBACK(eventOnLeaderboardButton));
	gfxMovie.RegisterEventHandler("eventInventorySellLootBox", MAKE_CALLBACK(eventInventorySellLootBox));
	gfxMovie.RegisterEventHandler("eventArmoryOnOpenUpgrade", MAKE_CALLBACK(eventArmoryOnOpenUpgrade));
	gfxMovie.RegisterEventHandler("eventArmoryUpgradeUnlockSlot", MAKE_CALLBACK(eventArmoryUpgradeUnlockSlot));
	gfxMovie.RegisterEventHandler("eventArmoryUpgradeSlot", MAKE_CALLBACK(eventArmoryUpgradeSlot));
	gfxMovie.RegisterEventHandler("eventClaimDailyReward", MAKE_CALLBACK(eventClaimDailyReward));
	gfxMovie.RegisterEventHandler("eventWelcomeBackClosed", MAKE_CALLBACK(eventWelcomeBackClosed));
	gfxMovie.RegisterEventHandler("switchToShootingRange", MAKE_CALLBACK(eventSwitchToShootingRange));
	
	gfxMovie.RegisterEventHandler("eventArmoryOnOpenFPSUpgrade", MAKE_CALLBACK(eventArmoryOnOpenFPSUpgrade));
	gfxMovie.RegisterEventHandler("eventArmoryOnCloseFPSUpgrade", MAKE_CALLBACK(eventArmoryOnCloseFPSUpgrade));
	gfxMovie.RegisterEventHandler("eventArmoryOnEquipAttachment", MAKE_CALLBACK(eventArmoryOnEquipAttachment));
	gfxMovie.RegisterEventHandler("eventStoreBuyAttachment", MAKE_CALLBACK(eventStoreBuyAttachment));

	gfxMovie.RegisterEventHandler("eventClansShowClans", MAKE_CALLBACK(eventClansShowClans));
	gfxMovie.RegisterEventHandler("eventClansShowClanMembers", MAKE_CALLBACK(eventClansShowClanMembers));
	gfxMovie.RegisterEventHandler("eventClansCreateClan", MAKE_CALLBACK(eventClansCreateClan));
	gfxMovie.RegisterEventHandler("eventClansJoinClan", MAKE_CALLBACK(eventClansJoinClan));
	gfxMovie.RegisterEventHandler("eventClansLeaveClan", MAKE_CALLBACK(eventClansLeaveClan));
	gfxMovie.RegisterEventHandler("eventClansDonateGCToClan", MAKE_CALLBACK(eventClansDonateGCToClan));
	gfxMovie.RegisterEventHandler("eventClansDonateGCToMember", MAKE_CALLBACK(eventClansDonateGCToMember));
	gfxMovie.RegisterEventHandler("eventClansPromoteMember", MAKE_CALLBACK(eventClansPromoteMember));
	gfxMovie.RegisterEventHandler("eventClansDemoteMember", MAKE_CALLBACK(eventClansDemoteMember));
	gfxMovie.RegisterEventHandler("eventClansKickMember", MAKE_CALLBACK(eventClansKickMember));
	gfxMovie.RegisterEventHandler("eventClansShowApplications", MAKE_CALLBACK(eventClansShowApplications));
	gfxMovie.RegisterEventHandler("eventClansApplicationAnswer", MAKE_CALLBACK(eventClansApplicationAnswer));
	gfxMovie.RegisterEventHandler("eventClansApplicationInvitePlayer", MAKE_CALLBACK(eventClansApplicationInvitePlayer));
	gfxMovie.RegisterEventHandler("eventClanPopUpClosed", MAKE_CALLBACK(eventClanPopUpClosed));
	gfxMovie.RegisterEventHandler("eventClansInviteAnswer", MAKE_CALLBACK(eventClansInviteAnswer));
	gfxMovie.RegisterEventHandler("eventClansBuyClanSlot", MAKE_CALLBACK(eventClansBuyClanSlot));

	// server region
	{
		gfxMovie.Invoke("_root.api.setDefaultRegion", r_server_region->GetInt());
	}

	// achievements
	{
		Scaleform::GFx::Value var[5];
		for(int i=0; i<gWeaponArmory.getNumAchievements(); ++i)
		{
			const AchievementConfig* ach = gWeaponArmory.getAchievementByIndex(i);
			if(ach->enabled)
			{
				var[0].SetStringW(gLangMngr.getString(ach->name));
				var[1].SetStringW(gLangMngr.getString(ach->desc));
				var[2].SetString(ach->hud_icon);
				wiAchievement* achData = gUserProfile.ProfileData.getAchievementDataByID(ach->id);
				
				if(achData)
				{
					var[3].SetBoolean(achData->unlocked>0);
					var[4].SetNumber(float(achData->value)/float(ach->value));
				}
				else
				{
					var[3].SetBoolean(false);
					var[4].SetNumber(0.0f);
				}
				gfxMovie.Invoke("_root.api.addAchievementInfo", var, 5);
			}
		}
	}

	// premium account
	{
		Scaleform::GFx::Value var[2];
		// find price of premium acc
		mStorePremiumPrice = 0;
		for(uint32_t i=0; i<g_NumStoreItems; i++)
		{
			if(g_StoreItems[i].itemID == 301004)
			{
				mStorePremiumPrice = R3D_MAX(R3D_MAX(R3D_MAX(g_StoreItems[i].price1day, g_StoreItems[i].price7day), g_StoreItems[i].price30day), g_StoreItems[i].pricePerm);
				break;
			}
		}
		m_hasPremiumAccount = gUserProfile.getInventoryItemByID(301004)==-1?false:true;
		if(m_hasPremiumAccount)
			m_premiumBackgroundFadeIn = 0.01f;
		var[0].SetBoolean(m_hasPremiumAccount);
		var[1].SetNumber(mStorePremiumPrice);
		gfxMovie.Invoke("_root.api.setCreateGameUnlocked", var, 2);
	}
	
	setBaseStats();
	setAdditionalStats();
	{	// set stat desc
		Scaleform::GFx::Value var[2];
		var[0].SetString("xp");
		var[1].SetStringW(gLangMngr.getString("StatXPDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("gp");
		var[1].SetStringW(gLangMngr.getString("StatGPDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("gd");
		var[1].SetStringW(gLangMngr.getString("StatGDDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("sp");
		var[1].SetStringW(gLangMngr.getString("StatSPDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("wins");
		var[1].SetStringW(gLangMngr.getString("StatWinsDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("losses");
		var[1].SetStringW(gLangMngr.getString("StatLossesDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("kills");
		var[1].SetStringW(gLangMngr.getString("StatKillDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("deaths");
		var[1].SetStringW(gLangMngr.getString("StatDeathDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("kdratio");
		var[1].SetStringW(gLangMngr.getString("StatKDRatioDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("time");
		var[1].SetStringW(gLangMngr.getString("StatTimeDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("headshots");
		var[1].SetStringW(gLangMngr.getString("StatHeadshotsDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("assistedK");
		var[1].SetStringW(gLangMngr.getString("StatAssistedKDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("winlossR");
		var[1].SetStringW(gLangMngr.getString("StatWLRatioDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("captureNeutral");
		var[1].SetStringW(gLangMngr.getString("StatCaptNeutrDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("captureEnemy");
		var[1].SetStringW(gLangMngr.getString("StatCaptEnemyDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("shotsFired");
		var[1].SetStringW(gLangMngr.getString("StatShotsFiredDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("shotsHit");
		var[1].SetStringW(gLangMngr.getString("StatShotsHitDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);

		var[0].SetString("accuracy");
		var[1].SetStringW(gLangMngr.getString("StatAccuracyDesc"));
		gfxMovie.Invoke("_root.api.setStatDesc", var, 2);
	}

	{
		// test news item
		gfxMovie.Invoke("_root.api.addNews", "");
	}

	// set server date time
	{
		m_serverTime = mktime(&gUserProfile.ServerTime);
		minToAddToServerTime = 0;
		updateServerTime();
	}

	// add calendar
	/*{
		Scaleform::GFx::Value var[2];

		time_t serverTime = m_serverTime;
		for(int i=0; i<10; ++i)		// for now hard coded to 10 days
		{
			struct tm curdate = *localtime(&serverTime);

			// advance time
			serverTime = (time_t)(((unsigned int)serverTime)+86400); // 86400 - one day
			
			wchar_t tmpStr[32] = L"";
			swprintf(tmpStr, 32, L"%s, %d", gLangMngr.getString(monthNames[curdate.tm_mon]), curdate.tm_mday);
			var[0].SetStringW(tmpStr);
			var[1].SetString("");

			if(gHasCalendar)
			{
				for(int i=0; i<m_numCalendarEvents; ++i)
				{
					if(curdate.tm_mon == m_CalendarEvents[i].month && curdate.tm_mday == m_CalendarEvents[i].day)
					{
						var[1].SetString(m_CalendarEvents[i].text);
						break;
					}
				}
			}
			gfxMovie.Invoke("_root.api.addCalendarDay", var, 2);
		}
	}*/

	// loadout data
	{// loadout prices
		Scaleform::GFx::Value var[1];
		wchar_t tempMsg[256];
		swprintf(tempMsg, 256, gLangMngr.getString("$FR_UnlockLoadoutSlotPrice"), gUserProfile.ShopUnlockLoadoutCost);
		var[0].SetStringW(tempMsg);
		gfxMovie.Invoke("_root.api.setLoadoutPrice", var, 1);
	}
	{
		gfxMovie.Invoke("_root.api.setAbilityPrice", 0);
	}

	initItems();

	{
		// find default items to replace bad slots with
		int primarySlotItemID=0, secondarySlotItemID=0;
		for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
		{
			STORE_CATEGORIES cat = gWeaponArmory.getCategoryByItemId(gUserProfile.ProfileData.Inventory[i].itemID);
			if(cat == storecat_ASR || cat == storecat_SNP || cat == storecat_MG)
				primarySlotItemID = gUserProfile.ProfileData.Inventory[i].itemID;
			else if(cat == storecat_SHTG || cat == storecat_SUPPORT || cat == storecat_SMG)
				secondarySlotItemID = gUserProfile.ProfileData.Inventory[i].itemID;

			if(primarySlotItemID!=0 && secondarySlotItemID!=0)
				break;
		}
		// check for bad loadout and fix it
		for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
		{
			if(gUserProfile.ProfileData.ArmorySlots[i].LoadoutID)
			{
				wiLoadoutSlot& slot = gUserProfile.ProfileData.ArmorySlots[i];
				bool require_fix = false;
				
				// check primary weapon
				STORE_CATEGORIES cat = gWeaponArmory.getCategoryByItemId(slot.PrimaryWeaponID);
				if(cat != storecat_ASR && cat != storecat_SNP && cat != storecat_MG)
				{
					require_fix = true;
					slot.PrimaryWeaponID = primarySlotItemID;
				}
				
				// check secondary weapon (note, it can be empty)
				cat = gWeaponArmory.getCategoryByItemId(slot.SecondaryWeaponID);
				if(cat != storecat_SHTG && cat != storecat_SUPPORT && cat != storecat_SMG && cat != storecat_INVALID)
				{
					require_fix = true;
					slot.SecondaryWeaponID = secondarySlotItemID;
				}

				if(slot.Item1 != 0 && (slot.Item1 == slot.Item2 || slot.Item1 == slot.Item3 || slot.Item1 == slot.Item4))
				{
					require_fix = true;
					slot.Item1 = 0;
				}
				if(slot.Item2 != 0 && (slot.Item2 == slot.Item1 || slot.Item2 == slot.Item3 || slot.Item2 == slot.Item4))
				{
					require_fix = true;
					slot.Item2 = 0;
				}
				if(slot.Item3 != 0 && (slot.Item3 == slot.Item1 || slot.Item3 == slot.Item2 || slot.Item3 == slot.Item4))
				{
					require_fix = true;
					slot.Item3 = 0;
				}
				if(slot.Item4 != 0 && (slot.Item4 == slot.Item1 || slot.Item4 == slot.Item2 || slot.Item4 == slot.Item3))
				{
					require_fix = true;
					slot.Item4 = 0;
				}

				if(gUserProfile.ProfileData.IsFPSEnabled)
				{
					const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(slot.PrimaryWeaponID);
					if(wpn && !wpn->IsFPS)
					{
						slot.PrimaryWeaponID = 0;
						require_fix = true;
					}
					wpn = gWeaponArmory.getWeaponConfig(slot.SecondaryWeaponID);
					if(wpn && !wpn->IsFPS)
					{
						slot.SecondaryWeaponID = 0;
						require_fix = true;
					}
					wpn = gWeaponArmory.getWeaponConfig(slot.SidearmWeaponID);
					if(wpn && !wpn->IsFPS)
					{
						slot.SidearmWeaponID = 0;
						require_fix = true;
					}
					// check for grenades
					wpn = gWeaponArmory.getWeaponConfig(slot.Item1);
					if(wpn && !wpn->IsFPS)
					{
						slot.Item1 = 0;
						require_fix = true;
					}
					wpn = gWeaponArmory.getWeaponConfig(slot.Item2);
					if(wpn && !wpn->IsFPS)
					{
						slot.Item2 = 0;
						require_fix = true;
					}
					wpn = gWeaponArmory.getWeaponConfig(slot.Item3);
					if(wpn && !wpn->IsFPS)
					{
						slot.Item3 = 0;
						require_fix = true;
					}
					wpn = gWeaponArmory.getWeaponConfig(slot.Item4);
					if(wpn && !wpn->IsFPS)
					{
						slot.Item4 = 0;
						require_fix = true;
					}
				}

				if(require_fix)
				{
					int apiCode = gUserProfile.ApiModifyLoadoutSlot(i);
					// silently skip errors?

					showLoadoutWasFixedMsg = true;
				}
			}
		}
	}
	UpdateLoadoutUI();

	// update clan UI
	{
		{
			int needMoney = 0;
			int apiCode = gUserProfile.clans->ApiClanCheckIfCreateNeedMoney(&needMoney); 
			r3d_assert(apiCode==0); 
			wchar_t tmpStr[256] = {0};
			if(needMoney)
				swprintf(tmpStr, 256, gLangMngr.getString("$FR_ClanCreateDisclaimer"), 45, gUserProfile.ShopClanCreate); // 45 - minimum level required to create clan without paying
			gfxMovie.SetVariable("_root.api.ClansCreateDisclaimerText", tmpStr);
		}
		{
			Scaleform::GFx::Value var[2];
			for(int i=0; i<4; ++i)
			{
				var[0].SetNumber(gUserProfile.ShopClanAddMembers_GP[i]);
				var[1].SetNumber(gUserProfile.ShopClanAddMembers_Num[i]);
				gfxMovie.Invoke("_root.api.addClanSlotBuyInfo", var, 2);
			}
		}
		{
			Scaleform::GFx::Value var[3];
			char tmpStr1[64], tmpStr2[64], tmpStr3[64];
			int i = 0;
			while(true)
			{
				sprintf(tmpStr3, "Data/Menu/clanEmblem/clan_logo_%02d_back.png", i+1);
				if(r3d_access(tmpStr3, 0)!=0)
					break;
				var[0].SetNumber(i);
				sprintf(tmpStr1, "$Data/Menu/clanEmblem/clan_logo_%02d_back.png", i+1);
				sprintf(tmpStr2, "$Data/Menu/clanEmblem/clan_logo_%02d_color.png", i+1);
				var[1].SetString(tmpStr1);
				var[2].SetString(tmpStr2);
				gfxMovie.Invoke("_root.api.addClanEmblem", var, 3);
				++i;
			}
		}
		updateMyClanInfoUI();

		Scaleform::GFx::Value var[2];
		for(int i=0; i<3; ++i)
		{
			var[0].SetNumber(i);
			char tmpStr[32];
			sprintf(tmpStr, "ClanRank_%d", i);
			var[1].SetStringW(gLangMngr.getString(tmpStr));
			gfxMovie.Invoke("_root.api.addClanRankInfo", var, 2);
		}

		// set clan rank data
		{
			// todo: hard coded for now, probably should be fetched from server
			Scaleform::GFx::Value var[2];
			var[0].SetString(""); // no rank name for clans

			var[1].SetNumber(0); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(907900); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(2723700); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(4539500); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(7263200); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(9986900); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(12710600); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(16342200); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(19973800); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(23605400); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(28144900); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(32684400); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(37223900); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(41763400); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(47210800); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(52658200); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(59921400); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(67184600); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(76263600); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
			var[1].SetNumber(87158400); gfxMovie.Invoke("_root.api.m_ClanRanksData.addRank", var, 2);
		}
	}
	
	// init skills
	{
		Scaleform::GFx::Value var[7];
		// function addSkill(skillID:Number, classID:Number, name:String, desc:String, icon:String, tier:Number, levelinfo:String);
		// add skills
		for(int i=0; i<CUserSkills::SKILL_ID_END; ++i)
		{
			if(gUserProfile.ShopSkillCosts2[i][0] != 0) // if first level has price - then skill exists
			{
				const char* skillName = CUserSkills::SkillNames[i];
				
				int classID = i/100;
				var[0].SetNumber(i);
				var[1].SetNumber(classID);
				char tmpStr[128];
				sprintf(tmpStr, "Skill_%s_Name", skillName);
				var[2].SetStringW(gLangMngr.getString(tmpStr));
				sprintf(tmpStr, "Skill_%s_Desc", skillName);
				var[3].SetStringW(gLangMngr.getString(tmpStr));
				sprintf(tmpStr, "Skill_%s_Icon", skillName);
				var[4].SetStringW(gLangMngr.getString(tmpStr));
				int tier = ((i%100)/10);
				var[5].SetNumber(tier);
				sprintf(tmpStr, "Skill_%s_LevelDesc", skillName);
				var[6].SetStringW(gLangMngr.getString(tmpStr));
				gfxMovie.Invoke("_root.api.addSkill", var, 7);

				for(int j=0; j<5; ++j)
				{
					if(gUserProfile.ShopSkillCosts2[i][j] != 0)
					{
						//function addSkillLevelInfo(skillID:Number, classID:Number, level:Number, cost:Number);
						var[2].SetNumber(j+1);
						int skillPrice = (int)((char)gUserProfile.ShopSkillCosts2[i][j]); // convert to signed, as price can be negative
#ifdef FINAL_BUILD
#else
						if(skillPrice != -100) // -100 - hidden skill, that no one knows about it yet
							skillPrice = R3D_ABS(skillPrice); // in dev build always show prices as positive, as it is a way to "hide" skills from public build
#endif
						var[3].SetNumber(skillPrice);
						gfxMovie.Invoke("_root.api.addSkillLevelInfo", var, 4);
					}
				}

				// update loadouts skills
				for(int j=0; j<wiUserProfile::MAX_LOADOUT_SLOTS; ++j)
				{
					//function setLoadoutSkillInfo(loadoutID:Number, skillID:Number, curLevel:Number);
					var[0].SetNumber(j);
					var[1].SetNumber(i);
					int sk = (i%100);
					var[2].SetNumber(gUserProfile.ProfileData.ArmorySlots[j].Skills[sk]);
					gfxMovie.Invoke("_root.api.setLoadoutSkillInfo", var, 3);
				}
			}
		}

		// update rest of skills
		UpdateSkillUI();
	}

	// add game modes
	{
		Scaleform::GFx::Value var[4];
		var[0].SetNumber(1);
		var[1].SetStringW(gLangMngr.getString("$GameMode_Conquest"));
		var[2].SetString("Conquest");
		var[3].SetStringW(gLangMngr.getString("ConquestDesc"));
		gfxMovie.Invoke("_root.api.addGameMode", var, 4);

		var[0].SetNumber(2);
		var[1].SetStringW(gLangMngr.getString("$GameMode_Deathmatch"));
		var[2].SetString("DM");
		var[3].SetStringW(gLangMngr.getString("DeathmatchDesc"));
		gfxMovie.Invoke("_root.api.addGameMode", var, 4);

// #ifndef FINAL_BUILD
// 		var[0].SetNumber(3);
// 		var[1].SetString("SIEGE");
// 		var[2].SetString("Siege");
// 		var[3].SetString("Siege desc");
// 		gfxMovie.Invoke("_root.api.addGameMode", var, 4);
// #endif

		var[0].SetNumber(4);
		var[1].SetStringW(gLangMngr.getString("$GameMode_Sabotage"));
		var[2].SetString("Sabotage");
		var[3].SetStringW(gLangMngr.getString("SabotageDesc"));
		gfxMovie.Invoke("_root.api.addGameMode", var, 4);

		var[0].SetNumber(9);
		var[1].SetStringW(gLangMngr.getString("GameMode_Any"));
		var[2].SetString("All");
		var[3].SetStringW(gLangMngr.getString("AnyModeDesc"));
		gfxMovie.Invoke("_root.api.addGameMode", var, 4);
	}

	// add game maps
	{
		addGameMap(GBGameInfo::MAPID_WO_Crossroads16, gLangMngr.getString("MapCrossroadsName"), "$Levels/WO_Crossroads16/map_icon_cross16.dds", gLangMngr.getString("MapCrossroadsDesc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Crossroads16, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads16, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads16, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads16, 64);

		addGameMap(GBGameInfo::MAPID_WO_Grozny, gLangMngr.getString("MapEasternFallName"), "$Levels/WO_Grozny/map_icon_grozny.dds", gLangMngr.getString("MapEasternFallDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Grozny, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Grozny, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Grozny, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Grozny, 64);

		addGameMap(GBGameInfo::MAPID_WO_Shipyard, gLangMngr.getString("MapShippingYardName"), "$Levels/wo_shippingyard/map_icon_shippingyard.dds", gLangMngr.getString("MapShippingYardDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Shipyard, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Shipyard, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Shipyard, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Shipyard, 32);

		addGameMap(GBGameInfo::MAPID_BurningSea, gLangMngr.getString("MapBurningSeaName"), "$Levels/WO_Burning_Sea/map_icon.dds", gLangMngr.getString("MapBurningSeaDesc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_BurningSea, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_BurningSea, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_BurningSea, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_BurningSea, 64);

		addGameMap(GBGameInfo::MAPID_WO_Torn, gLangMngr.getString("MapTornName"), "$Levels/WO_Torn_cq/map_icon_torn.dds", gLangMngr.getString("MapTornDesc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Torn, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Torn, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Torn, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Torn, 64);

		addGameMap(GBGameInfo::MAPID_WO_Nightfall_CQ, gLangMngr.getString("MapNightfallName"), "$Levels/wo_nightfall_cq/map_icon_cross16.dds", gLangMngr.getString("MapNightfallDesc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Nightfall_CQ, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Nightfall_CQ, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Nightfall_CQ, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Nightfall_CQ, 32);

		addGameMap(GBGameInfo::MAPID_WO_Wasteland, gLangMngr.getString("MapWasteland"), "$Levels/wo_wasteland/map_icon.dds", gLangMngr.getString("MapWastelandDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Wasteland, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Wasteland, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Wasteland, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Wasteland, 32);

		addGameMap(GBGameInfo::MAPID_WO_Torn_CT, gLangMngr.getString("MapDustName"), "$Levels/WO_Torn_ct/map_icon.dds", gLangMngr.getString("MapDustDesc"), "4");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Torn_CT, 12);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Torn_CT, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Torn_CT, 16);

		addGameMap(GBGameInfo::MAPID_WO_Inferno, gLangMngr.getString("MapInferno"), "$Levels/wo_inferno/map_icon.dds", gLangMngr.getString("MapInfernoDesc"), "4");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Inferno, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Inferno, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Inferno, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Inferno, 32);

		addGameMap(GBGameInfo::MAPID_WO_Jungle02, gLangMngr.getString("MapJungleRuins"), "$Levels/wo_jungleruins/map_icon.dds", gLangMngr.getString("MapJungleRuinsDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Jungle02, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Jungle02, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Jungle02, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Jungle02, 32);

		addGameMap(GBGameInfo::MAPID_WO_Citadel_DM, gLangMngr.getString("MapCitadel"), "$Levels/wo_citadel_dm/map_icon.dds", gLangMngr.getString("MapCitadelDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Citadel_DM, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Citadel_DM, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Citadel_DM, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Citadel_DM, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Citadel_DM, 64);

		addGameMap(GBGameInfo::MAPID_WO_EasternBunkerTDM, gLangMngr.getString("MapInferno"), "$Levels/wo_eastern_bunker_tdm/map_icon.dds", gLangMngr.getString("MapInfernoDesc"), "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_EasternBunkerTDM, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_EasternBunkerTDM, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_EasternBunkerTDM, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_EasternBunkerTDM, 32);

		addGameMap(GBGameInfo::MAPID_WO_Crossroads2, gLangMngr.getString("MapCrossroads2Name"), "$Levels/WO_Crossroads2/map_icon.dds", gLangMngr.getString("MapCrossroads2Desc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Crossroads2, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads2, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads2, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Crossroads2, 64);

#ifndef FINAL_BUILD
		addGameMap(GBGameInfo::MAPID_WO_TornTown, L"torn town", "$Levels/wo_TornTown_tdm/map_icon.dds", L"desc", "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_TornTown, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TornTown, 10);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TornTown, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TornTown, 32);

		addGameMap(GBGameInfo::MAPID_WO_Valley, L"valley", "$Levels/wo_valley/map_icon.dds", L"desc", "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Valley, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Valley, 24);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Valley, 32);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Valley, 64);

		addGameMap(GBGameInfo::MAPID_WO_TestConquest, L"TEST CONQUEST", "", L"will load WorkInProgress/TestConquest", "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_TestConquest, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TestConquest, 16);

		addGameMap(GBGameInfo::MAPID_WO_TestDeathmatch, L"TEST DEATHMATCH", "", L"will load WorkInProgress/TestDeathmatch", "29");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_TestDeathmatch, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TestDeathmatch, 16);

		addGameMap(GBGameInfo::MAPID_WO_TestSabotage, L"TEST SABOTAGE", "", L"will load WorkInProgress/TestSabotage", "4");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_TestSabotage, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_TestSabotage, 16);

		addGameMap(GBGameInfo::MAPID_WO_Grozny_CQ, gLangMngr.getString("MapEasternFallName"), "$Levels/WO_Grozny_CQ/map_icon_grozny.dds", gLangMngr.getString("MapEasternFallDesc"), "19");
		addGameMap_numBasicPlayers(GBGameInfo::MAPID_WO_Grozny_CQ, 16);
		addGameMap_numPremiumPlayers(GBGameInfo::MAPID_WO_Grozny_CQ, 16);
#endif
		addGameMap(255, gLangMngr.getString("MapAnyMapName"), "$Data/Menu/any_map_icon.dds", gLangMngr.getString("MapAnyMapDesc"), "129");
	}

	// add keyboard shortcuts
	for(int i=0; i<r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxMovie.Invoke("_root.api.addKeyboardMapping", args, 2);
	}

	SyncGraphicsUI();

	// daily/weekly rewards
	{
		Scaleform::GFx::Value var[7];
		wchar_t tempBuf[64];
		// daily
		var[0].SetNumber(0);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Kill10Enemies"));
		if(gUserProfile.DailyStats.Kills < 10)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 10-gUserProfile.DailyStats.Kills);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(250);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.DailyStats.Kills)/10.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play5Matches"));
		if(gUserProfile.DailyStats.getNumMatches() < 5)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 5-gUserProfile.DailyStats.getNumMatches());
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(500);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.DailyStats.getNumMatches())/5.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play2Conquest"));
		if(gUserProfile.DailyStats.MatchesCQ < 2)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 2-gUserProfile.DailyStats.MatchesCQ);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(1000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.DailyStats.MatchesCQ)/2.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play2Deathmatch"));
		if(gUserProfile.DailyStats.MatchesDM < 2)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 2-gUserProfile.DailyStats.MatchesDM);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(1200);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.DailyStats.MatchesDM)/2.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play2Sabotage"));
		if(gUserProfile.DailyStats.MatchesSB < 2)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 2-gUserProfile.DailyStats.MatchesSB);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(1200);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.DailyStats.MatchesSB)/2.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		// weekly
		var[0].SetNumber(1);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Kill200Enemies"));
		if(gUserProfile.WeeklyStats.Kills < 200)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 200-gUserProfile.WeeklyStats.Kills);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(5000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.Kills)/200.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Get100Headshots"));
		if(gUserProfile.WeeklyStats.Headshots < 100)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 100-gUserProfile.WeeklyStats.Headshots);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(2500);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.Headshots)/100.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play50Matches"));
		if(gUserProfile.WeeklyStats.getNumMatches() < 50)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 50-gUserProfile.WeeklyStats.getNumMatches());
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(5000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.getNumMatches())/50.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play25Conquest"));
		if(gUserProfile.WeeklyStats.MatchesCQ < 25)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 25-gUserProfile.WeeklyStats.MatchesCQ);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(5000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.MatchesCQ)/25.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play25Deathmatch"));
		if(gUserProfile.WeeklyStats.MatchesDM < 25)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 25-gUserProfile.WeeklyStats.MatchesDM);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(5000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.MatchesDM)/25.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);

		var[1].SetStringW(gLangMngr.getString("$FR_Reward_Play25Sabotage"));
		if(gUserProfile.WeeklyStats.MatchesSB < 25)
			swprintf(tempBuf, 64, gLangMngr.getString("$FR_Reward_XTogo"), 25-gUserProfile.WeeklyStats.MatchesSB);
		else
			swprintf(tempBuf, 64, L"%s", gLangMngr.getString("$FR_Reward_XCompleted"));
		var[2].SetStringW(tempBuf);
		var[3].SetString("$Data/Menu/Rewards/RWD_Kill.png");
		var[4].SetNumber(5000);
		var[5].SetNumber(0);
		var[6].SetNumber(R3D_CLAMP(float(gUserProfile.WeeklyStats.MatchesSB)/25.0f, 0.0f, 1.0f));
		gfxMovie.Invoke("_root.api.addRewardDailyInfo", var, 7);
	}

	// welcome back screen
	{
		for(int i=0; i<gUserProfile.NumNewItems; ++i)
		{
			// check if we already have this item
			if(gUserProfile.getInventoryItemByID(gUserProfile.NewItemsInStore[i])==-1)
			{
				Scaleform::GFx::Value var[1];
				var[0].SetNumber(gUserProfile.NewItemsInStore[i]);
				gfxMovie.Invoke("_root.api.markStoreItemAsNew", var, 1);
			}
		}
		for(uint32_t i=0; i<gUserProfile.retentionBonusByDays_.size(); ++i)
		{
			Scaleform::GFx::Value var[3];
			wchar_t tempStr[16];
			swprintf(tempStr, 16, L"%d %s", (i+1), gLangMngr.getString("$FR_Store_SingularDay"));
			var[0].SetStringW(tempStr);
			wchar_t tempStr2[16];
			if(!RUS_CLIENT)
				swprintf(tempStr2, 16, L"%d GC", gUserProfile.retentionBonusByDays_[i]);
			else
				swprintf(tempStr2, 16, L"%d WP", gUserProfile.retentionBonusByDays_[i]);
			var[1].SetStringW(tempStr2);
			if(!RUS_CLIENT)
				var[2].SetString("$Data/Weapons/StoreIcons/GoldIcon.dds");
			else
				var[2].SetString("$Data/Weapons/StoreIcons/WPIcon.dds");
			
			gfxMovie.Invoke("_root.api.addDailyBonus", var, 3);
		}
		Scaleform::GFx::Value var[3];
		int curBonus = 0;
		if(gUserProfile.curRetentionDays_ > 0)
			curBonus = gUserProfile.retentionBonusByDays_[gUserProfile.curRetentionDays_-1];
		wchar_t tempStr[16];
		if(!RUS_CLIENT)
			swprintf(tempStr, 16, L"%d GC", curBonus);
		else
			swprintf(tempStr, 16, L"%d WP", curBonus);
		var[0].SetStringW(tempStr);
		var[1].SetNumber(gUserProfile.curRetentionDays_);
		wchar_t tempStr2[64];
		int hours = (int)floorf(float(gUserProfile.minutesToNextRetDay_)/60.0f);
		int mins = gUserProfile.minutesToNextRetDay_ - hours*60;
		swprintf(tempStr2, 64, gLangMngr.getString("$FR_WB_UnlockedIn"), hours, mins, random(60));
		var[2].SetStringW(tempStr2);
		gfxMovie.Invoke("_root.api.setCurrentDailyBonus", var, 3);
	}

	if(gUserProfile.ProfileData.hasExpiringItems)
	{
		for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
		{
			if(gUserProfile.ProfileData.Inventory[i].expiration <= 24*60) // 24 hours
			{
				// check if item is available in store
				bool inStore = false;
				for(uint32_t j=0; j<g_NumStoreItems; ++j)
				{
					if(g_StoreItems[j].itemID == gUserProfile.ProfileData.Inventory[i].itemID)
					{
						inStore = g_StoreItems[j].gd_price1day!=0 || g_StoreItems[j].gd_price7day!=0 || g_StoreItems[j].gd_price30day!=0 || g_StoreItems[j].gd_pricePerm!=0 || 
							g_StoreItems[j].price1day!=0 || g_StoreItems[j].price7day!=0 || g_StoreItems[j].price30day!=0 || g_StoreItems[j].pricePerm!=0;
						break;
					}
				}
				
				if(inStore)
					gfxMovie.Invoke("_root.api.markItemAsExpiring", (int)gUserProfile.ProfileData.Inventory[i].itemID);
			}
		}
	}
	// level up bonus
	{
		Scaleform::GFx::Value var[5];
		int curLevel = gUserProfile.ProfileData.Stats.getRankLevel();
		int hasReward = -1;
		for(int i=0;i<MAX_NUM_RANKS; ++i)
		{
			if(gWeaponArmory.m_LevelUpBonus[i].nextLevel==-1)
				break;
			if(curLevel < gWeaponArmory.m_LevelUpBonus[i].nextLevel)
			{
				hasReward = i;
				break;
			}
		}

		var[0].SetBoolean(hasReward!=-1);
		wchar_t tempStr[256];
		swprintf(tempStr, 256, gLangMngr.getString("FR_NextLevelReward"), (hasReward!=-1?gWeaponArmory.m_LevelUpBonus[hasReward].nextLevel:0));
		var[1].SetStringW(hasReward!=-1?tempStr:L"");
		if(!RUS_CLIENT)
		{
			var[2].SetString(hasReward!=-1?gWeaponArmory.m_LevelUpBonus[hasReward].reward1:"");
			var[3].SetString(hasReward!=-1?gWeaponArmory.m_LevelUpBonus[hasReward].reward2:"");
			var[4].SetString(hasReward!=-1?gWeaponArmory.m_LevelUpBonus[hasReward].reward3:"");
		}
		else
		{
			wchar_t rwdstr[512];
			temp_GetRusLvlUpBaseRewards(gWeaponArmory.m_LevelUpBonus[hasReward], rwdstr);
			var[2].SetStringW(rwdstr);
			var[3].SetString("...");
			var[4].SetString("");
		}
		gfxMovie.Invoke("_root.api.setNextLevelReward", var, 5);
	}
	// some localization strings
	{
		gfxMovie.SetVariable("_global.locStr_stLVL",gLangMngr.getString("$FR_Store_LvlRequired1"));
		gfxMovie.SetVariable("_global.locStr_stLVLReq",gLangMngr.getString("$FR_Store_LvlRequired2"));

		gfxMovie.SetVariable("_global.locStr_WB_Rew1_1",gLangMngr.getString("$FR_WB_Reward1_1"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew1_2",gLangMngr.getString("$FR_WB_Reward1_2"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew1_3",gLangMngr.getString("$FR_WB_Reward1_3"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew1_4",gLangMngr.getString("$FR_WB_Reward1_4"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew2_1",gLangMngr.getString("$FR_WB_Reward2_1"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew2_2",gLangMngr.getString("$FR_WB_Reward2_2"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew2_3",gLangMngr.getString("$FR_WB_Reward2_3"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew3_1",gLangMngr.getString("$FR_WB_Reward3_1"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew3_2",gLangMngr.getString("$FR_WB_Reward3_2"));
		gfxMovie.SetVariable("_global.locStr_WB_Rew3_3",gLangMngr.getString("$FR_WB_Reward3_3"));
	}

	gfxMovie.Invoke("_root.api.showFrontend", "");

	if(frontendFirstTimeInit)
	{
		frontendFirstTimeInit= false;
		if((RUS_CLIENT==0 || (RUS_CLIENT && !showBuyPackMovie)) && !showQuickJoin) // for rus client, if we are showing buy pack movie, than do not show welcome back screen
		{
			gfxMovie.Invoke("_root.api.showWelcomeBack", "");
		}

		if(SoundSys.isSoundDisabled()) // notify user that his sound system is broken
		{
			Scaleform::GFx::Value args[2];
			args[0].SetStringW(gLangMngr.getString("SoundSystemDisabled"));
			args[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
		}
	}
	if(showQuickJoin)
	{
		gfxMovie.Invoke("_root.api.showPlayGame", "");
	}

	wiWeaponAttachments emptyAttm;
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[m_CurrentModifySlot], emptyAttm);
	m_needPlayerRenderingRequest = 3;

	m_waitingForKeyRemap = -1;
}

void FrontendUI::addGameMap(int mapID, const wchar_t* name, const char* icon, const wchar_t* desc, const char* options)
{
	Scaleform::GFx::Value var[5];
	var[0].SetNumber(mapID);
	var[1].SetStringW(name);
	var[2].SetString(icon);
	var[3].SetStringW(desc);
	var[4].SetString(options);
	gfxMovie.Invoke("_root.api.addGameMap", var, 5);
}

void FrontendUI::addGameMap_numBasicPlayers(int mapID, int numPl)
{
	Scaleform::GFx::Value var[2];
	var[0].SetNumber(mapID);
	var[1].SetNumber(numPl);
	gfxMovie.Invoke("_root.api.addGameMap_BasicNumPlayers", var, 2);
}

void FrontendUI::addGameMap_numPremiumPlayers(int mapID, int numPl)
{
	Scaleform::GFx::Value var[2];
	var[0].SetNumber(mapID);
	var[1].SetNumber(numPl);
	gfxMovie.Invoke("_root.api.addGameMap_PremiumNumPlayers", var, 2);
}

void FrontendUI::updateServerTime()
{
	prevServerTimeUpdate = r3dGetTime();

	time_t serverTime = (time_t)(((unsigned int)m_serverTime)+60*minToAddToServerTime); 
	minToAddToServerTime++;

	tm* srvTm;
	srvTm = localtime(&serverTime);

	wchar_t dateTime[256];
	swprintf(dateTime, 256, L"%s %d, %04d / %02d:%02d", gLangMngr.getString(monthNames[srvTm->tm_mon]), srvTm->tm_mday, 1900+srvTm->tm_year, srvTm->tm_hour, srvTm->tm_min);
	Scaleform::GFx::Value var[1];
	var[0].SetStringW(dateTime);
	gfxMovie.Invoke("_root.api.setDateTime", var, 1);

	// sergey's cheating
	r3dTimeGradient2 numPlGrad;
	numPlGrad.AddValue(0.0f/24.0f, 310);
	numPlGrad.AddValue(4.0f/24.0f, 370);
	numPlGrad.AddValue(9.0f/24.0f, 840);
	numPlGrad.AddValue(10.0f/24.0f, 870);
	numPlGrad.AddValue(11.0f/24.0f, 880);
	numPlGrad.AddValue(13.0f/24.0f, 775);
	numPlGrad.AddValue(15.0f/24.0f, 500);
	numPlGrad.AddValue(16.0f/24.0f, 556);
	numPlGrad.AddValue(24.0f/24.0f, 310);

	int numPlayers = (int)numPlGrad.GetValue(float(srvTm->tm_hour)/23.0f).x;
	gfxMovie.Invoke("_root.api.setNumServerPlayers", numPlayers);
}

void FrontendUI::StartAsyncOperation(fn_thread threadFn, fn_finish finishFn)
{
	r3d_assert(asyncThread_ == NULL);
  
	asyncFinish_ = finishFn;
	asyncErr_[0] = 0;
	asyncThread_ = (HANDLE)_beginthreadex(NULL, 0, threadFn, this, 0, NULL);
	if(asyncThread_ == NULL)
		r3dError("Failed to begin thread");
}

void FrontendUI::SetAsyncError(int apiCode, const wchar_t* msg)
{
	if(gMasterServerLogic.shuttingDown_)
	{
		swprintf(asyncErr_, sizeof(asyncErr_), L"%s", gLangMngr.getString("MSShutdown1"));
		return;
	}

	if(apiCode == 0) {
		swprintf(asyncErr_, sizeof(asyncErr_), L"%s", msg);
	} else {
		swprintf(asyncErr_, sizeof(asyncErr_), L"%s, code:%d", msg, apiCode);
	}
}

void FrontendUI::ProcessAsyncOperation()
{
	if(asyncThread_ == NULL)
		return;

	DWORD w0 = WaitForSingleObject(asyncThread_, 0);
	if(w0 == WAIT_TIMEOUT) 
		return;

	CloseHandle(asyncThread_);
	asyncThread_ = NULL;
	
	if(gMasterServerLogic.badClientVersion_)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString("ClientMustBeUpdated"));
		args[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
		//@TODO: on infoMsg closing, exit app.
		return;
	}

	if(asyncErr_[0]) 
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(asyncErr_);
		args[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
		return;
	}
	
	if(asyncFinish_)
		(this->*asyncFinish_)();
}

void getWeaponParamForUI(const WeaponConfig* wc, int& damage, int& accuracy, int& firerate);

void FrontendUI::UpdateSkillUI()
{
	const CClientUserProfile& prof = gUserProfile;
	const wiLoadoutSlot& slot = prof.ProfileData.ArmorySlots[m_CurrentModifySlot];

	// update tier info
	Scaleform::GFx::Value var[3];
	for(int j=0; j<wiUserProfile::MAX_LOADOUT_SLOTS; ++j)
	{
		//function setLoadoutTierInfo(loadoutID:Number, tier:Number, info:String);
		// tier 1 always unlocked
		var[0].SetNumber(j);
		var[1].SetNumber(1);
		var[2].SetString(""); // tier 1 unlocked always
		gfxMovie.Invoke("_root.api.setLoadoutTierInfo", var, 3);

		wchar_t tmpStr[256] = {0};

		var[1].SetNumber(2);
		if(slot.SpendSP[0] >= 20) // 20 hardcoded in api_SkillLearn
			var[2].SetString("");
		else
		{
			swprintf(tmpStr, 256, gLangMngr.getString("SkillTier2Locked"), 20-slot.SpendSP[0]);
			var[2].SetStringW(tmpStr);
		}
		gfxMovie.Invoke("_root.api.setLoadoutTierInfo", var, 3);

		var[1].SetNumber(3);
		if(slot.SpendSP[1] >= 20) // 20 hardcoded in api_SkillLearn
			var[2].SetString("");
		else
		{
			swprintf(tmpStr, 256, gLangMngr.getString("SkillTier3Locked"), 20-slot.SpendSP[1]);
			var[2].SetStringW(tmpStr);
		}
		gfxMovie.Invoke("_root.api.setLoadoutTierInfo", var, 3);
	}

	gfxMovie.Invoke("_root.api.setSP", prof.ProfileData.Stats.SkillPoints-(slot.SpendSP[0]+slot.SpendSP[1]+slot.SpendSP[2]));
	gfxMovie.Invoke("_root.api.updateSkillTree", "");

	updateInventoryAndSkillItems();
}

void FrontendUI::UpdateLoadoutUI()
{
	CUserProfile& prof = gUserProfile;
	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
	{
		const static char* classes[] = {
			"$FR_AssaultSpec",
			"$FR_SpecialistSpec",
			"$FR_ReconSpec",
			"$FR_MedicSpec",
		};
		int loadoutLevel = (prof.ProfileData.ArmorySlots[i].HonorPoints / 20000)+1;
		int loadoutClass = prof.ProfileData.ArmorySlots[i].Class;
	
		Scaleform::GFx::Value var[16];
		var[0].SetBoolean(prof.ProfileData.ArmorySlots[i].LoadoutID>0);
		var[1].SetNumber(i);
		var[2].SetNumber(prof.ProfileData.ArmorySlots[i].PrimaryWeaponID);
		var[3].SetNumber(prof.ProfileData.ArmorySlots[i].SecondaryWeaponID);
		var[4].SetNumber(prof.ProfileData.ArmorySlots[i].SidearmWeaponID);
		var[5].SetNumber(prof.ProfileData.ArmorySlots[i].Item1);
		var[6].SetNumber(prof.ProfileData.ArmorySlots[i].Item2);
		var[7].SetNumber(prof.ProfileData.ArmorySlots[i].Item3);
		var[8].SetNumber(prof.ProfileData.ArmorySlots[i].Item4);
		var[9].SetNumber(prof.ProfileData.ArmorySlots[i].BodyMeshID);
		var[10].SetNumber(prof.ProfileData.ArmorySlots[i].BodyHeadID);
		var[11].SetNumber(prof.ProfileData.ArmorySlots[i].BodyArmorID);
		var[12].SetNumber(prof.ProfileData.ArmorySlots[i].BodyHeadGearID);
		var[13].SetNumber(loadoutClass);
		var[14].SetNumber(loadoutLevel);
		char timePlayed[32] = {0};
		int tPlayed = prof.ProfileData.ArmorySlots[i].TimePlayed;
		int hours = tPlayed/3600;
		int minutes = (tPlayed-hours*3600)/60;
		int sec = tPlayed%60;
		sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
		var[15].SetString(timePlayed);

		gfxMovie.Invoke("_root.api.setLoadoutSlotParams", var, 16);
	}
}

void FrontendUI::UpdateSettings()
{
	r3dRenderer->UpdateSettings( ) ;

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	Mouse->SetRange( r3dRenderer->HLibWin );

	void applyGraphicsOptions( uint32_t settingsFlags );

	applyGraphicsOptions( settingsChangeFlags_ );

	gfxMovie.UpdateTextureMatrices("merc_rendertarget", (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
}

extern void InputUpdate();
int FrontendUI::Update()
{
	// we need to reset scaleform before loading frontend, otherwise visual artifacts will start showing up
	// I guess this is a bug with scaleform, or we using it somewhere incorrectly
// 	if(needScaleformReset)
// 	{
// 		r3dScaleformReset();
// 		needScaleformReset = false;
// 	}

	if ( m_StartShootingGallery )
	{
		RunShootingRange();
		m_StartShootingGallery = false;
	}

	if(needReInitScaleformTexture)
	{
		if (RTScaleformTexture && Scaleform_RenderToTextureRT)
			RTScaleformTexture->Initialize(Scaleform_RenderToTextureRT->AsTex2D());
#if ENABLE_WEB_BROWSER
		if(RTWelcomeBackScaleformTexture && g_pBrowserManager->GetWindow())
			RTWelcomeBackScaleformTexture->Initialize(g_pBrowserManager->GetWindow()->AsTex2D());

#endif
		needReInitScaleformTexture = false;
	}

	if(gSteam.inited_)
		SteamAPI_RunCallbacks();

	InputUpdate();

	{
		r3dPoint3D soundPos(0,0,0), soundDir(0,0,1), soundUp(0,1,0);
		SoundSys.Update(soundPos, soundDir, soundUp);
	}


	// we're still retreiving profile
	if(handleGetProfileData != 0 && gProfileIsAquired == 0)
	{
		// run temp drawing loop
		extern void tempDoMsgLoop();
		tempDoMsgLoop();
		
		// replace message with loading stage info
		static int oldStage = -1;
		if(oldStage != gProfileLoadStage)
		{
			oldStage = gProfileLoadStage;

			wchar_t dots[32] = L"";
			for(int i=0; i<gProfileLoadStage; i++) dots[i] = L'.';
			dots[gProfileLoadStage] = 0;
			
			wchar_t info[1024];
			StringCbPrintfW(info, sizeof(info), L"%s\n%s", gLangMngr.getString("RetrievingProfileData"), dots);
			
			updateInfoMsgText(info);
		}

		// NOTE: WARNING: DO NOT TOUCH GameWorld() or anything here - background loading thread in progress!
		r3dStartFrame();
		if( r3dRenderer->DeviceAvailable )
		{
			r3dRenderer->StartFrame();
			ClearFullScreen_Menu();
			r3dRenderer->StartRender(1);

			r3d_assert(m_pBackgroundTex);
			r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
			r3dDrawBoxFS(r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor::white, m_pBackgroundTex);

			gfxMovie.UpdateAndDraw();

			r3dRenderer->Flush();
			CurRenderPipeline->Finalize() ;
			r3dRenderer->EndFrame();
		}

		r3dRenderer->EndRender( true );

		// process d3d device queue, keeping 20fps for rendering
		if( r3dRenderer->DeviceAvailable )
		{
			float endTime = r3dGetTime() + (1.0f / 20);
			while(r3dGetTime() < endTime)
			{
				extern bool ProcessDeviceQueue( float chunkTimeStart, float maxDuration ) ;
				ProcessDeviceQueue(r3dGetTime(), 0.05f);
			}
		}
		
		r3dEndFrame();

		// update browser, so that by the time we get profile our welcome back screen will be ready to show page
#if ENABLE_WEB_BROWSER
		g_pBrowserManager->Update();
#endif

		return 0;
	}

	if(handleGetProfileData != 0)
	{
		// profile is acquired
		r3d_assert(gProfileIsAquired);
		
		if(!gProfileOK)
		{
			r3dOutToLog("Couldn't get profile data! stage: %d\n", gProfileLoadStage);
			return FrontEndShared::RET_Diconnected;
		}

		CloseHandle(handleGetProfileData);
		handleGetProfileData = 0;

		r3dOutToLog( "Acquired base profile data for %f\n", r3dGetTime() - aquireProfileStart );
		if(gUserProfile.AccountStatus >= 200)
		{
			return FrontEndShared::RET_Banned;
		}
		
		r3dResetFrameTime();

		// finish initializing shooting range
		extern void InitGame_Finish();
		InitGame_Finish();

		for(GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj))
		{
			if(!IsObjectVisibleInShooting(obj))
				obj->ObjFlags |= OBJFLAG_SkipDraw;
		}
		_render_Terrain = 0;

		//
		initFrontend();
	}

	// at the moment we must have finished initializing things in background
	r3d_assert(handleGetProfileData == 0);

	if((needUpdateProfileOnActivate && win::bSuspended==0) || needUpdateProfile)
	{
		// after player returns from web store, update GP/GD
		gUserProfile.GetProfile();
		
		needUpdateProfileOnActivate = false;
		needUpdateProfile = false;
		gfxMovie.Invoke("_root.api.hideInfoMsg", "");
		// update amount of money
		gfxMovie.Invoke("_root.api.setGP", gUserProfile.ProfileData.Stats.GamePoints);
		gfxMovie.Invoke("_root.api.setGD", gUserProfile.ProfileData.Stats.GameDollars);
		gfxMovie.Invoke("_root.api.setSP", gUserProfile.ProfileData.Stats.SkillPoints);
		// update inventory
		updateInventoryAndSkillItems();
	}

	{
		if((r3dGetTime() - prevServerTimeUpdate) >= 60.0f)
			updateServerTime();
	}

	if(showDisconnectErrorMessage)
	{
		gUserProfile.ProfileData.hasExpiringItems = false;
		showDisconnectErrorMessage = false;
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(disconnectErrorMsg);
		args[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
	}
	if(showLoadoutWasFixedMsg)
	{
		showLoadoutWasFixedMsg = false;
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString("YourLoadoutHaveBeenResetted"));
		args[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showInfoMsg", args, 2);		
	}

	UpdateFriendData();
	updateClanData();

	if(m_waitingForKeyRemap != -1)
	{
		// query input manager for any input
		if(InputMappingMngr->attemptRemapKey((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetNumber(m_waitingForKeyRemap);
			var[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap));
			gfxMovie.Invoke("_root.api.updateKeyboardMapping", var, 2);
			m_waitingForKeyRemap = -1;

			void writeInputMap();
			writeInputMap();
		}
	}

#if ENABLE_WEB_BROWSER
	g_pBrowserManager->Update();
#endif

	settingsChangeFlags_ = 0;

	r3dMouse::Show();

	extern void tempDoMsgLoop();
	tempDoMsgLoop();

	m_Player->UpdateTransform();
	r3dPoint3D size = m_Player->GetBBoxLocal().Size;

	float distance = GetOptimalDist(size, 30.0f);

 	r3dPoint3D camPos(0, size.y * 0.5f, distance);
	r3dPoint3D playerPosHome(0, 0.0f, 0);
 	r3dPoint3D playerPosArmory(0, 0.0f, 0);
 	r3dPoint3D playerPosArmoryModify(0, 0.0f, 0.0f);
 	r3dPoint3D playerPosArmoryModifyZoom(0, -0.5f, 0.0f);

	float backupFOV = gCam.FOV;
	gCam = camPos;
	gCam.vPointTo = (r3dPoint3D(0, 1, 0) - gCam).NormalizeTo();
	gCam.FOV = 60;

	gCam.SetPlanes(0.01f, 200.0f);
	if(m_needPlayerRenderingRequest==1)
		m_Player->SetPosition(playerPosArmory);
	else if(m_needPlayerRenderingRequest==2)
		m_Player->SetPosition(playerPosArmoryModify);
	else if(m_needPlayerRenderingRequest==3)
		m_Player->SetPosition(playerPosHome);
	else if(m_needPlayerRenderingRequest==4)
	{
		m_Player->SetPosition(playerPosArmoryModifyZoom);
		gCam.FOV *= 0.5f; 
	}

	m_Player->m_fPlayerRotationTarget = m_Player->m_fPlayerRotation = playerRot;

	GameWorld().StartFrame();
	r3dRenderer->SetCamera( gCam );

	GameWorld().Update();

	ProcessAsyncOperation();

	if(showBuyPackMovie)
	{
		gfxBuyPackMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit);
	}
	else
	{
		gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
	}

	ProcessAchievementQueue();

	r3dStartFrame();

	if( r3dRenderer->DeviceAvailable )
	{
		r3dRenderer->StartFrame();

		ClearFullScreen_Menu();

		r3dRenderer->StartRender(1);

		r3d_assert(m_pBackgroundTex);
		//r3d_assert(m_pBackgroundPremiumTex);
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		r3dColor backgroundColor = r3dColor::white;
		//if(m_premiumBackgroundFadeIn < 1.0f)
		//{
		//	backgroundColor.A = BYTE((1.0f-m_premiumBackgroundFadeIn)*255.0f);
			r3dDrawBoxFS(r3dRenderer->ScreenW, r3dRenderer->ScreenH, backgroundColor, m_pBackgroundTex);
		//}
		//if(m_premiumBackgroundFadeIn > 0.0f)
		//{
		//	backgroundColor.A = BYTE(m_premiumBackgroundFadeIn*255.0f);
		//	r3dDrawBoxFS(r3dRenderer->ScreenW, r3dRenderer->ScreenH, backgroundColor, m_pBackgroundPremiumTex);
		//
		//	if(m_premiumBackgroundFadeIn < 1.0f)
		//	{
		//		m_premiumBackgroundFadeIn += r3dGetFrameTime()*1.0f;
		//		m_premiumBackgroundFadeIn = R3D_CLAMP(m_premiumBackgroundFadeIn, 0.0f, 1.0f);
		//	}
		//}

		// DRAW OUR SCALEFORM RT HERE (PLAYER)
		drawPlayer( 1, 1 ) ;
#if 0
		TempTestBuffer->Activate();
		r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		TempTestBuffer->Deactivate();
#endif

		if(!showBuyPackMovie)
			gfxMovie.UpdateAndDraw();
		else
			gfxBuyPackMovie.UpdateAndDraw();

#if 0
		if(m_needPlayerRenderingRequest && !showBuyPackMovie )
		{
			drawPlayer( numStereoPasses, 0 ) ;
		}
#endif

		r3dRenderer->Flush();

		CurRenderPipeline->Finalize() ;

		r3dRenderer->EndFrame();
	}

	r3dRenderer->EndRender( true );

	if( r3dRenderer->DeviceAvailable )
	{
		r3dUpdateScreenShot();
		if(Keyboard->WasPressed(kbsPrtScr))
			r3dToggleScreenShot();
	}

	GameWorld().EndFrame();
	r3dEndFrame();

	gCam.FOV = backupFOV;

	if( needUpdateSettings_ )
	{
		UpdateSettings();
		needUpdateSettings_ = false;
	}

	if(gMasterServerLogic.IsConnected() && asyncThread_ == NULL)
	{
		if(r3dGetTime() > masterConnectTime_ + _p2p_idleTime)
		{
			gMasterServerLogic.Disconnect();
		}
		
		if(gMasterServerLogic.shuttingDown_)
		{
			gMasterServerLogic.Disconnect();
			
			Scaleform::GFx::Value var[2];
			var[0].SetStringW(gLangMngr.getString("MSShutdown1"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
		}
	}

	if(asyncThread_ == NULL)
	{
		if(exitRequested_)
			return FrontEndShared::RET_Exit;

		if(IsNeedExit())
			return FrontEndShared::RET_Exit;

		// check for double login.
		if(!gLoginSessionPoller.IsConnected())
			return FrontEndShared::RET_DoubleLogin;

		if(needExitByGameJoin_)
			return FrontEndShared::RET_JoinGame;
	}
	
	return 0;
}

void FrontendUI::drawPlayer( int numStereoPasses, int toTemp )
{
	struct BeginEndEvent
	{
		BeginEndEvent()
		{
			D3DPERF_BeginEvent( 0, L"FrontendUI::drawPlayer" ) ;
		}
		
		~BeginEndEvent()
		{
			D3DPERF_EndEvent() ;
		}
	} beginEndEvent ;

	int width, height, y_shift;
	GetInterfaceSize(width, height, y_shift, gfxMovie);

	float x_start=0;
	float y_start=0;
	float x_width=0;
	float y_width=0;
	playerRot = 0;

	// get multipliers based on current resolution and default SWF size
	float xMultiplier = (width / 1920.0f);
	float yMultiplier = (height / 1080.0f);

	// get the smallest multiplier
	float multiplier = R3D_MIN(xMultiplier, yMultiplier);

	// get real values based on multiplier
	if(m_needPlayerRenderingRequest==1)
	{
		playerRot = 0;
		x_start = xMultiplier * 0.0f;
		y_start = yMultiplier * 250.0f + y_shift;
		x_width = xMultiplier * 500.0f;
		y_width = yMultiplier * 660.0f;
	}
	/*else if(m_needPlayerRenderingRequest==2)
	{
		playerRot = 0;
		x_start = xMultiplier * 1300.0f;
		y_start = yMultiplier * 200.0f + y_shift;
		x_width = xMultiplier * 500.0f;
		y_width = yMultiplier * 660.0f;
	}*/
	else if(m_needPlayerRenderingRequest==3)
	{
		playerRot = 0; 
		x_start = xMultiplier * 780.0f ;
		y_start = yMultiplier * 220.0f+ y_shift;
		x_width = xMultiplier * 350.0f;
		y_width = yMultiplier * 670.0f;
	}
	/*else if(m_needPlayerRenderingRequest==4)
	{
		x_start = xMultiplier * 44.0f;
		y_start = yMultiplier * 174.0f + y_shift;
		x_width = xMultiplier * 406.0f;
		y_width = yMultiplier * 474.0f;
	}*/

	int old_ssao = r_ssao->GetInt();
	r_ssao->SetInt(0);

	// temp bullshit for sergey
	{
		if(m_WeaponModel->enableRendering)
		{
#ifndef FINAL_BUILD
			if(Keyboard->WasReleased(kbsG))
				d_temp_green_screen->SetBool(!d_temp_green_screen->GetBool());

			bool b = d_temp_green_screen->GetBool();
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.greenBack._visible", b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.BtnSR._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.BtnSave._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot0._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot1._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot2._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot3._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot4._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot5._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot6._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot7._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Slot8._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Lines._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Stat1._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Stat2._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Stat3._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Stat4._visible", !b);
			gfxMovie.SetVariable("_root.Main.PopUpWpnCustomization.Upgrade.Stat6._visible", !b);
#endif
		}
	}

	// disable shadows in weapon attachment screen
	bool old_shadows = r_shadows->GetBool();
	if(m_WeaponModel->enableRendering)
	{
		r_shadows->SetBool(false);
	}

	CurRenderPipeline->PreRender();
	CurRenderPipeline->Render();

	r_ssao->SetInt(old_ssao);
	if(m_WeaponModel->enableRendering)
	{
		r_shadows->SetBool(old_shadows);
	}

	{
		PFX_Fill::Settings efsts ;

		efsts.Value = float4( r_gameui_exposure->GetFloat(), 0.f, 0.f, 0.f ) ;

		gPFX_Fill.PushSettings( efsts ) ;
		g_pPostFXChief->AddFX( gPFX_Fill, PostFXChief::RTT_SCENE_EXPOSURE0, PostFXChief::RTT_AVG_SCENE_LUMA );
		gPFX_Fill.PushSettings( efsts ) ;
		g_pPostFXChief->AddFX( gPFX_Fill, PostFXChief::RTT_SCENE_EXPOSURE1, PostFXChief::RTT_AVG_SCENE_LUMA );

		g_pPostFXChief->AddFX( gPFX_ConvertToLDR );
		g_pPostFXChief->AddSwapBuffers();

		PFX_Fill::Settings fsts;

		fsts.ColorWriteMask = D3DCOLORWRITEENABLE_ALPHA;			

		gPFX_Fill.PushSettings( fsts );

		g_pPostFXChief->AddFX( gPFX_Fill, PostFXChief::RTT_PINGPONG_LAST, PostFXChief::RTT_DIFFUSE_32BIT );

		PFX_StencilToMask::Settings ssts;

		ssts.Value = float4( 0, 0, 0, 1 );

		gPFX_StencilToMask.PushSettings( ssts );

		g_pPostFXChief->AddFX( gPFX_StencilToMask, PostFXChief::RTT_PINGPONG_LAST );

		if( toTemp )
		{
			r3dScreenBuffer* buf = g_pPostFXChief->GetBuffer( PostFXChief::RTT_PINGPONG_LAST ) ;
			r3dScreenBuffer* buf_scaleform = g_pPostFXChief->GetBuffer( PostFXChief::RTT_UI_CHARACTER_32BIT ) ;

			PFX_Copy::Settings sts ;

			sts.TexScaleX = 1.0f;//0.5f ;
			sts.TexScaleY = 1.0f;//0.5f * ( buf_scaleform->Height * buf->Width ) / ( buf->Height * buf_scaleform->Width ) ;
			sts.TexOffsetX = 0.0f;//0.5f - sts.TexScaleX * 0.5f ;
			sts.TexOffsetY = 0.0f;//0.5f - sts.TexScaleY * 0.5f ;

			gPFX_Copy.PushSettings( sts ) ;

			g_pPostFXChief->AddFX( gPFX_Copy, PostFXChief::RTT_UI_CHARACTER_32BIT ) ;
		}

		g_pPostFXChief->Execute( false, true );
	}

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();

	if( !toTemp )
	{
		r3dTexture* Tex = g_pPostFXChief->GetBuffer( PostFXChief::RTT_PINGPONG_LAST )->Tex;

		float texelX = 1.0f / (float)r3dRenderer->ScreenW; 
		float texelY = 1.0f / (float)r3dRenderer->ScreenH;

		float minX, minY, maxX, maxY;
		minX = 0.5f - texelX * (x_width) / 1.7f;
		minY = 0.5f - texelY * (y_width) / 1.7f;
		maxX = 0.5f + texelX * (x_width) / 1.7f;
		maxY = 0.5f + texelY * (y_width) / 1.7f;


		float TC[16] = {minX, minY,
						maxX, minY,
						maxX, maxY,
						minX, maxY,
						minX, minY,
						maxX, minY,
						maxX, maxY,
						minX, maxY};


		r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSU,   D3DTADDRESS_BORDER );
		r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSV,   D3DTADDRESS_BORDER );
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
		r3dRenderer->SetBackBufferViewport();
		r3dTexture* Tex2 = g_pPostFXChief->GetBuffer( PostFXChief::RTT_PINGPONG_LAST )->Tex;
		r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_PUSH );
		r3dDrawBox2D( x_start, y_start, x_width, y_width, r3dColor(255,255,255), Tex2, TC);
		r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
		r3dRenderer->SetRenderingMode( R3D_BLEND_NONE| R3D_BLEND_PUSH );
		r3dDrawBox2DWireframe( x_start, y_start, x_width, y_width, r3dColor(255,255,255));
		r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
	}
}

bool FrontendUI::CheckForPermenentWeapons()
{

	if( mStore_buyItemExp > 1000) // if it's greater than 1000 days, it's a perm. 
	{
		IncrementAchievement( ACHIEVEMENT_PURCHASE_2_PERMENENT_ITEMS, 1);
		IncrementAchievement( ACHIEVEMENT_PURCHASE_5_PERMENENT_ITEMS, 1);
		IncrementAchievement( ACHIEVEMENT_PURCHASE_10_PERMENENT_ITEMS, 1);
		return true; 
	}
	
	return false;

}

void FrontendUI::CheckOnBuyAchievements()
{
	bool newAchievementEarned = false;
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(mStore_buyItemID);

	// don't place any achievements before this function.  We'll use NewAchievementEarned for this code. 
	if ( wc ) 
	{

		switch( wc->category )
		{	
		case storecat_HG:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_PISTOL );
			}
			break; 
		case storecat_SUPPORT:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_SUPPORT );
			}
			break;
		case storecat_ASR:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_ASSAULT );
			}
			break;
		case storecat_SHTG:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_SHOTGUN );
			}
			break;
		case storecat_SNP:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_SNIPER );
			}
			break;
		case storecat_SMG:
			{
				newAchievementEarned = MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_SMG );
			}
			break; 
		default:
			{
				newAchievementEarned = false;
			}
			break;
		}

		if( newAchievementEarned ) 
		{

			bool missingAchievement = false;

			for( int achievementIndex= ACHIEVEMENT_PURCHASE_RENT_PISTOL; achievementIndex <= ACHIEVEMENT_PURCHASE_RENT_SMG; achievementIndex++ )
			{
				const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( achievementIndex );
				wiAchievement* achievementData = gUserProfile.ProfileData.getAchievementDataByID( achievementInfo->id );

				if( achievementData == NULL || achievementData->unlocked == 0 )
				{
					missingAchievement = true;
					break;
				}

			}

			if( missingAchievement == false ) 
			{
				MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_EACH_TYPE );
			}
		}
	}

	// check gear. 
	const GearConfig* gc = gWeaponArmory.getGearConfig( mStore_buyItemID );
	if(gc && (gc->category == storecat_Gear || gc->category == storecat_HeadGear))
	{
		newAchievementEarned |= MarkAchievementComplete( ACHIEVEMENT_PURCHASE_RENT_PURCHASE_RENT_GEAR );
	}

	newAchievementEarned |= CheckForPermenentWeapons();

	if( newAchievementEarned ) 
	{
		Scaleform::GFx::Value var[5];
		// update all front end achievements.
		for( int achievementIndex = FIRST_FRONT_END_ACHIEVEMENT; achievementIndex < NUM_ACHIEVEMENTS; achievementIndex++ )
		{
			const AchievementConfig* ach = gWeaponArmory.getAchievementByIndex(achievementIndex);
			var[0].SetStringW(gLangMngr.getString(ach->name));
			var[1].SetStringW(gLangMngr.getString(ach->desc));
			var[2].SetString(ach->hud_icon);
			wiAchievement* achData = gUserProfile.ProfileData.getAchievementDataByID(ach->id);
			if(achData)
			{
				var[3].SetBoolean(achData->unlocked>0);
				var[4].SetNumber(float(achData->value)/float(ach->value));
			}
			else
			{
				var[3].SetBoolean(false);
				var[4].SetNumber(0.0f);
			}
			gfxMovie.Invoke("_root.api.addAchievementInfo", var, 5);
		}

		// Since we already have a async operation.
		CreateThread(NULL, 0, as_AchievementUnlockThread, NULL, 0, NULL);
	}
}

bool FrontendUI::MarkAchievementComplete( int whichAchievement )
{
	if( gUserProfile.MarkAchievementComplete( whichAchievement ) == true ) 
	{
		RequeustAchievement( whichAchievement );
		return true;
	}
	return false;
	
}

bool FrontendUI::IncrementAchievement( int whichAchievement, int value )
{
	if( gUserProfile.IncrementAchievement( whichAchievement, value ) == true ) 
	{
		RequeustAchievement( whichAchievement );
		return true;
	}
	return false;
}

void FrontendUI::RequeustAchievement( int achievementID )
{
	if( m_NumAchievementsRibbonsInQueue >= MAX_FRONTEND_ACHIEVEMENT_QUEUE )
	{
		r3dOutToLog("Requeusting Achievement when already maxed. Ignoring achievement. ");
		return;
	}

	m_AchievementRibbonQueue[m_NumAchievementsRibbonsInQueue] = achievementID;
	m_NumAchievementsRibbonsInQueue++;
}

void  FrontendUI::ProcessAchievementQueue()
{
	if( m_LastAchievementShownTime + gTimeToShowAchievementsRibbons < r3dGetTime() && m_NumAchievementsRibbonsInQueue > 0 )
	{
		r3d_assert( m_AchievementRibbonQueue[0] != 0 );
		ShowAchievementRibbon( m_AchievementRibbonQueue[0] );	
		for( int achievementIndex = 1; achievementIndex < m_NumAchievementsRibbonsInQueue; achievementIndex++ )
		{
			m_AchievementRibbonQueue[achievementIndex-1] = m_AchievementRibbonQueue[achievementIndex];
		}
		m_NumAchievementsRibbonsInQueue--;

	}

}

void FrontendUI::ShowAchievementRibbon( int whichAchievement )
{

	m_LastAchievementShownTime = r3dGetTime();
	const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString(achievementInfo->name));
	var[1].SetString(achievementInfo->hud_icon);
	gfxMovie.Invoke("_root.api.showAchievementUnlock", var, 2);

}

void FrontendUI::SwitchShootingRange(bool set)
{
	m_StartShootingGallery = set;
}

bool FrontendUI::IsObjectVisibleInShooting(const GameObject* obj)
{
	if(obj->isObjType(OBJTYPE_Human))
		return true;
	if(obj == m_WeaponModel)
		return true;
	if(obj->Class->Name == "obj_LightHelper")
		return true;
		
	return false;
}

void FrontendUI::RunShootingRange()
{
	extern void SetHud ( int iHud );		

	m_Player->NetworkLocal = true;
	m_Player->uberAnim_->IsInUI = false;


	extern BaseHUD* editor_GetHudByIndex(int index);
	ShootingGalleryHUD* targetHud = static_cast<ShootingGalleryHUD*>( editor_GetHudByIndex(6));
	targetHud->TargetPlayer( m_Player );
	SetHud ( 6 );
	
	
	// need the player to be visible. 
	m_Player->m_enableRendering = true;
	m_WeaponModel->enableRendering = false;

	CurrentGameMode = GAMESTATE_PREGAME;
	extern bool gDisableShortcuts;
	gDisableShortcuts = true;
	r3dMouse::Hide();

	wiLoadoutSlot	oldLoadout = m_Player->CurLoadout;
	int oldSelectedSlot = m_Player->m_SelectedWeapon;
	wiWeaponAttachments oldWeaponAttachments = m_Player->CurWeaponAttachments;
	wiLoadoutSlot	targetLoadout;
	wiWeaponAttachments targetAttachments;

	targetLoadout.PrimaryWeaponID = m_WeaponModel->m_CurrentWeapon->getItemID();
	// keep the same body in case there's something special. 
	targetLoadout.BodyArmorID = oldLoadout.BodyArmorID;
	targetLoadout.BodyMeshID = oldLoadout.BodyMeshID;
	targetLoadout.BodyHeadGearID = oldLoadout.BodyHeadGearID;
	targetLoadout.BodySkinID = oldLoadout.BodySkinID;
	targetLoadout.BodyHeadID = oldLoadout.BodyHeadID;
	targetLoadout.BodyVoiceID = oldLoadout.BodyVoiceID;

	// grab all the current attachments.
	m_WeaponModel->m_CurrentWeapon->getCurrentAttachmentIDs(targetAttachments.primary_attachments.attachments);
	for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject( obj ) )
	{
		obj->ObjFlags &= ~OBJFLAG_SkipDraw;
	}

	_render_Terrain = 1;

	m_Player->SetPosition( r3dPoint3D( 212,100,210 ) );
	m_Player->UpdateLoadoutSlot( targetLoadout, targetAttachments, true);
	
	r3dResetFrameTime();
	bool fireWasPressed = true;
	float fireHoldTime = r3dGetTime() + 0.15f;	// to prevent double clicking - in that case weapon will immediately detect fire
	
	while(CurrentGameMode != GAMESTATE_EXIT) 
	{
		r3dStartFrame();

		extern void ClearBackBufferFringes();
		ClearBackBufferFringes();

		R3DPROFILE_START("Game Frame");

		extern void InputUpdate();
		InputUpdate();
		
		// keep ignoring fire button until it's released (mouse in pressed state because of shooting range button click)
		if(fireWasPressed)
		{
			if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE) || r3dGetTime() < fireHoldTime)
				Mouse->ClearPressed();
			else
				fireWasPressed = 0;
		}

		extern void GameFrameStart();

		GameFrameStart();

		extern void GameStateGameLoop();
		GameStateGameLoop();

		R3DPROFILE_END("Game Frame");
		r3dEndFrame();

		extern bool IsNeedExit();

		if ( IsNeedExit() || Keyboard->WasPressed(kbsEsc) || m_StartShootingGallery == false)
		{
			CurrentGameMode = GAMESTATE_EXIT;
		}

	} 

	_render_Terrain = 0;

	for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject( obj ) )
	{
		if(!IsObjectVisibleInShooting(obj))
			obj->ObjFlags |= OBJFLAG_SkipDraw;
	}


	r3dMouse::Show();
	gDisableShortcuts = false;
	// going back to attachment screen, only weapon visible. 
	m_WeaponModel->enableRendering = true;

	m_Player->m_enableRendering = false;
	m_Player->NetworkLocal = false; 
	m_Player->uberAnim_->IsInUI = true;
	m_Player->UpdateLoadoutSlot( oldLoadout, oldWeaponAttachments, true);
	m_Player->ChangeWeaponByIndex(oldSelectedSlot);
}

void FrontendUI::updateDefaultAttachments( bool isNewItem, uint32_t itemID )
{
	// add default attachments
	const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(itemID);
	if(wpn)
	{
		if(isNewItem)
		{
			for(int i=0; i<WPN_ATTM_MAX; ++i)
			{
				if(wpn->FPSDefaultID[i]>0)
				{
					gUserProfile.ProfileData.FPSAttachments[gUserProfile.ProfileData.NumFPSAttachments++] = wiUserProfile::temp_fps_attach(itemID, wpn->FPSDefaultID[i], mStore_buyItemExp*1440, 1);
					const WeaponAttachmentConfig* attm = gWeaponArmory.getAttachmentConfig(wpn->FPSDefaultID[i]);
					Scaleform::GFx::Value var[3];
					var[0].SetNumber(itemID);
					var[1].SetNumber(attm->m_type);
					var[2].SetNumber(attm->m_itemID);
					gfxMovie.Invoke("_root.api.setAttachmentSpec", var, 3);
				}
			}
		}
		else
		{
			for(int i=0; i<WPN_ATTM_MAX; ++i)
			{
				if(wpn->FPSDefaultID[i]>0)
				{
					for(uint32_t j=0; j<gUserProfile.ProfileData.NumFPSAttachments; ++j)
					{
						if(gUserProfile.ProfileData.FPSAttachments[j].WeaponID == itemID && gUserProfile.ProfileData.FPSAttachments[j].AttachmentID == wpn->FPSDefaultID[i])
						{
							gUserProfile.ProfileData.FPSAttachments[j].expiration += mStore_buyItemExp*1440;
						}
					}
				}
			}
		}
	}
}

void FrontendUI::updateInfoMsgText(wchar_t* msg)
{
	gfxMovie.SetVariable("_root.Main.PopUpAnim.PopUp.Text.text", msg);
}