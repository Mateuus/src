#include "r3dPCH.h"
#include "r3d.h"

#include "r3dDebug.h"
#include "../../../Eternity/sf/Console/config.h"
#include "m_Endround.h"
#include "GameCode\UserProfile.h"
#include "multiplayer\ClientGameLogic.h"
#include "ObjectsCode\ai\AI_Player.H"
#include "ObjectsCode\WEAPONS\WeaponArmory.h"

#include "multiplayer/MasterServerLogic.h"
#include "FrontEndNew.h"

extern int g_CCBlackWhite;
extern float g_fCCBlackWhitePwr;
extern int RUS_CLIENT;

HUDEndRound* HUDEndRound::This_ = NULL;

HUDEndRound::HUDEndRound()
{
	isActive_ = false;
	m_movieInitComplete = false;
	asyncThread_ = NULL;
	This_ = this;
}

HUDEndRound::~HUDEndRound()
{
	This_ = NULL;
}

void HUDEndRound::onExit(bool forceAsyncThreadDeath)
{
	gClientLogic().gameReadyToExit = true;
	g_CCBlackWhite = false;
	g_fCCBlackWhitePwr = 0.0f;
	r_gameplay_blur_strength->SetFloat(0.0f);

	if(asyncThread_ && forceAsyncThreadDeath) // terminate thread, do not auto join to next game, player clicked on Exit Game
	{
		TerminateThread(asyncThread_, 0);
		CloseHandle(asyncThread_);
		asyncThread_ = NULL;

		// clean up if we are exiting right in the middle of us trying to join to next game
		if(gMasterServerLogic.IsConnected())
			gMasterServerLogic.Disconnect();
	}

}

void HUDEndRound::eventEndOfRoundContinueClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	bool timerVisible = args[0].GetBool();
	if(timerVisible && gClientLogic().curRoundLevelUpMax <= 10)
	{
		// show pop up
		Scaleform::GFx::Value args[2];
		if(gClientLogic().curRoundRewardIdx == 4 || gClientLogic().curRoundRewardIdx == 5)
		{
			args[0].SetStringW(gLangMngr.getString("$HUD_EndRound_ExitMsg1"));
			if(gClientLogic().curRoundRewardIdx == 4)
				args[1].SetString(gWeaponArmory.getIconByItemId(301003, true));
			else
				args[1].SetString(gWeaponArmory.getIconByItemId(301001, true));
		}
		else
		{
			int nextLevel = gClientLogic().curRoundLevelUpMax+1;
			// find bonus desc
			int b = -1;
			for(int i=0; i<MAX_NUM_RANKS; ++i)
			{
				if(gWeaponArmory.m_LevelUpBonus[i].nextLevel == nextLevel)
				{
					b = i;
					break;
				}
			}
			r3d_assert(b!=-1);
			int numItems = 0;
			if(b!=-1)
			{
				for(int i=0; i<8; ++i)
				{
					if(gWeaponArmory.m_LevelUpBonus[b].items[i]==0)
						break;
					else
						numItems++;
				}
			}
			for(int i=0; i<numItems; ++i)
			{
				if(gWeaponArmory.getWeaponConfig(gWeaponArmory.m_LevelUpBonus[b].items[i])!=NULL) // show only guns
				{
					wchar_t tmpStr[512];
					swprintf(tmpStr, 512, gLangMngr.getString("$HUD_EndRound_ExitMsg2"), gWeaponArmory.getNameWByItemId(gWeaponArmory.m_LevelUpBonus[b].items[i], true));
					args[0].SetStringW(tmpStr);
					args[1].SetString(gWeaponArmory.getIconByItemId(gWeaponArmory.m_LevelUpBonus[b].items[i], true));
					break;
				}
			}
		}
		gfxMovie.Invoke("_root.api.showExitPopUp", args, 2);		
	}
	else
	{
		gfxMovie.SetVariable("_root.TimerEnd._visible", false);
		onExit(true);
	}
}

void HUDEndRound::StartAsyncOperation(fn_thread threadFn)
{
	r3d_assert(asyncThread_ == NULL);
	asyncThread_ = (HANDLE)_beginthreadex(NULL, 0, threadFn, this, 0, NULL);
	if(asyncThread_ == NULL)
		r3dError("Failed to begin thread");
}

void HUDEndRound::SetAsyncError(int apiCode, const wchar_t* msg)
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

void HUDEndRound::ProcessAsyncOperation()
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
}

void HUDEndRound::onNextRound()
{
	// we must have valid game info from previous game
	r3d_assert(gClientLogic().m_gameInfo.IsValid());
	
	m_nextRoundReq.CustomerID = gUserProfile.CustomerID;
	m_nextRoundReq.playerLevel= gClientLogic().m_gameInfo.minLevel + 1; //gUserProfile.ProfileData.Stats.getRankLevel();
	m_nextRoundReq.sessionId  = gClientLogic().m_sessionId;
	m_nextRoundReq.region     = gClientLogic().m_gameInfo.region;
	m_nextRoundReq.mapId      = gClientLogic().m_gameInfo.mapId;
	m_nextRoundReq.mapType    = gClientLogic().m_gameInfo.mapType;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(gLangMngr.getString("JoiningGame"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		

	StartAsyncOperation(&HUDEndRound::as_NextRoundThread);
}

unsigned int WINAPI HUDEndRound::as_NextRoundThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;

	if(!This_->ConnectToMasterServer())
		return 0;

	gMasterServerLogic.SendNextRoundReq(This_->m_nextRoundReq);

	const float endTime = r3dGetTime() + 60.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);

		if(!gMasterServerLogic.IsConnected())
			break;

		if(gMasterServerLogic.gameJoinAnswered_)
		{
			switch(gMasterServerLogic.gameJoinAnswer_.result)
			{
			case GBPKT_M2C_JoinGameAns_s::rOk:
				This_->onExit(false);
				gClientLogic().gameReadyForNextRound = true;
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rNoGames:
				This_->SetAsyncError(0, gLangMngr.getString("JoinGameNoGames"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rWrongCreateGameKey:
				This_->SetAsyncError(0, gLangMngr.getString("ProblemCreatingGame"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rHaveCreatedGame:
				This_->SetAsyncError(0, gLangMngr.getString("AlreadyCreatedGame"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rGameFull:
				This_->SetAsyncError(0, gLangMngr.getString("GameIsFull"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rGameFinished:
				This_->SetAsyncError(0, gLangMngr.getString("GameIsAlmostFinished"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rGameNotFound:
				This_->SetAsyncError(0, gLangMngr.getString("GameNotFound"));
				return 1;
			case GBPKT_M2C_JoinGameAns_s::rWrongPassword:
				This_->SetAsyncError(0, gLangMngr.getString("WrongPassword"));
				return 1;
			}

			wchar_t buf[128];
			swprintf(buf, 128, gLangMngr.getString("UnableToJoinGameCode"), gMasterServerLogic.gameJoinAnswer_.result);
			This_->SetAsyncError(0, buf);
			return 1;
		}
	}

	This_->SetAsyncError(0, gLangMngr.getString("TimeoutJoiningGame"));
	return 0;
}

extern	char		_p2p_masterHost[MAX_PATH];
extern	int		_p2p_masterPort;
bool HUDEndRound::ConnectToMasterServer()
{
	if(gMasterServerLogic.badClientVersion_)
		return false;
	if(gMasterServerLogic.IsConnected())
		return true;

	gMasterServerLogic.Disconnect();
	if(!gMasterServerLogic.StartConnect(_p2p_masterHost, _p2p_masterPort))
	{
		r3dOutToLog("EndRound: NoConnectionToMasterServer\n");
//		This_->SetAsyncError(0, gLangMngr.getString("NoConnectionToMasterServer"));
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

	r3dOutToLog("EndRound: TimeoutToMasterServer\n");
	//This_->SetAsyncError(8, gLangMngr.getString("TimeoutToMasterServer"));
	return false;
}

void HUDEndRound::levelUpCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	// init curlevel at first call
	if(curLevel == 0)
		curLevel = gClientLogic().curRoundLevelUpMin + 1;

	// check if we need to show level up pop up
	if(curLevel <= gClientLogic().curRoundLevelUpMax)
	{
		// find bonus desc
		int b = -1;
		for(int i=0; i<MAX_NUM_RANKS; ++i)
		{
			if(gWeaponArmory.m_LevelUpBonus[i].nextLevel == curLevel)
			{
				b = i;
				break;
			}
		}

		int numItems = 0;
		if(b!=-1)
		{
			for(int i=0; i<8; ++i)
			{
				if(gWeaponArmory.m_LevelUpBonus[b].items[i]==0)
					break;
				else
					numItems++;
			}
		}
		for(int i=0; i<numItems; ++i)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gWeaponArmory.getNameByItemId(gWeaponArmory.m_LevelUpBonus[b].items[i], true));
			var[1].SetStringW(gWeaponArmory.getDescByItemId(gWeaponArmory.m_LevelUpBonus[b].items[i], true));
			var[2].SetString(gWeaponArmory.getIconByItemId(gWeaponArmory.m_LevelUpBonus[b].items[i], true));
			gfxMovie.Invoke("_root.api.addRewardItem", var, 3);
		}
		curLevel++;
	}
}

bool HUDEndRound::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\EndOfRound.swf", true)) 
		return false;

	isActive_ = false;
	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDEndRound>(this, &HUDEndRound::FUNC)
	gfxMovie.RegisterEventHandler("eventEndOfRoundContinueClick", MAKE_CALLBACK(eventEndOfRoundContinueClick));
	gfxMovie.RegisterEventHandler("levelUpCallback", MAKE_CALLBACK(levelUpCallback));

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

	if(!gfxHUDScreen.Load("Data\\Menu\\EndOfRound_HUDScreen.swf", false)) 
		return false;
	gfxHUDScreen.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	return true;
}

bool HUDEndRound::Unload()
{
	r_gameplay_blur_strength->SetFloat(0.0f);
	gfxMovie.Unload();
	gfxHUDScreen.Unload();
	m_movieInitComplete = false;
	return true;
}

struct playerData
{
	char username[256];
	int score;
	int kills;
	int deaths;
	bool localPlayer;
};

int playerData_Comparator( void const * p0, void const* p1 )
{
	playerData* r0 = (playerData*)p0;
	playerData* r1 = (playerData*)p1;

	return (int)r1->score - (int)r0->score;
}
void HUDEndRound::Activate()
{
	r3d_assert(isActive_ == false);
	r3dMouse::Show();

	curLevel = 0;
	handledOnNextRound = false;

	timeActivated = r3dGetTime();


	if(gClientLogic().localPlayer_ == NULL)
	{
		gClientLogic().gameReadyToExit = true;
		return;
	}

	if(gClientLogic().localPlayer_->TeamID!=0 && gClientLogic().localPlayer_->TeamID!=1) // if no team, then just exit
	{
		gClientLogic().gameReadyToExit = true;
		return;
	}
	
	if(gClientLogic().gameFinished_ == false)
	{
		gClientLogic().gameReadyToExit = true;
		return;
	}

	isActive_ = true;

	// stop any footstep sounds
	for(int i=0; i<ClientGameLogic::MAX_NUM_PLAYERS; ++i)
	{
		if(gClientLogic().GetPlayer(i))
			gClientLogic().GetPlayer(i)->stopFootstepsSound();
	}
}

void HUDEndRound::Update()
{
	ProcessAsyncOperation();

	const float transitionTime = 1.0f;
	float timeLeft = r3dGetTime()-timeActivated;
	g_CCBlackWhite = 1;
	if(timeLeft < transitionTime)
		g_fCCBlackWhitePwr = timeLeft/transitionTime;
	else
		g_fCCBlackWhitePwr = 1.0f;
	r_gameplay_blur_strength->SetFloat(g_fCCBlackWhitePwr);

}

void HUDEndRound::Draw()
{
	const float EndRoundTime = 20.0f; // time while we are sitting on end round until next round
	const float ScoreScreenTime = 7.0f; // we show score for that time

	if(g_fCCBlackWhitePwr > 0.3f)
	{
		if(!m_movieInitComplete)
			initMovieData();
		float timeLeft = r3dGetTime()-timeActivated;
		if(timeLeft < ScoreScreenTime) // 7 seconds show score of all users
			gfxHUDScreen.UpdateAndDraw();
		else
		{
			if(timeLeft > EndRoundTime && !handledOnNextRound && gClientLogic().m_gameInfo.permGameIdx>0) // auto roll only for permanent games
			{
				handledOnNextRound = true;
				onNextRound();
			}

			// disable timer in custom games
			if(gClientLogic().m_gameInfo.permGameIdx == 0)
			{
				gfxMovie.SetVariable("_root.TimerEnd._visible", false);
			}

			static int prevTimer = -1;
			int newTimer = (int)ceilf(R3D_MAX(EndRoundTime-timeLeft, 0.0f));
			if(prevTimer != newTimer)
			{
				prevTimer = newTimer;
				gfxMovie.Invoke("_root.api.setTimer", newTimer);
			}
			gfxMovie.UpdateAndDraw();
		}
	}
}

float getRatio(float num1, float num2);
void HUDEndRound::initMovieData()
{
	r3d_assert(gClientLogic().localPlayer_);

	if(gClientLogic().gameFinished_) // update stats only if game was finished. if player left the game before that - he will get nothing
	{
		if(gClientLogic().finishData_.reason == PKT_S2C_GameFinish_s::REASON_Draw)
		{
			gfxHUDScreen.SetVariable("_root.Title.text", "$HUD_EndRound_TeamDraw");
			gfxHUDScreen.SetVariable("_root.TitleShad.text", "$HUD_EndRound_TeamDraw");
		}
		else if(gClientLogic().finishData_.winTeam == gClientLogic().localPlayer_->TeamID) 
		{
			gfxHUDScreen.SetVariable("_root.Title.text", "$HUD_EndRound_TeamWon");
			gfxHUDScreen.SetVariable("_root.TitleShad.text", "$HUD_EndRound_TeamWon");
		} 
		else 
		{
			gfxHUDScreen.SetVariable("_root.Title.text", "$HUD_EndRound_TeamLost");
			gfxHUDScreen.SetVariable("_root.TitleShad.text", "$HUD_EndRound_TeamLost");
		}

		gfxHUDScreen.SetVariable("_global.TeamWin", gClientLogic().finishData_.winTeam?"Red":"Blue");
		gfxHUDScreen.SetVariable("_global.UserColor", gClientLogic().localPlayer_->TeamID?"Red":"Blue");
	}
	else
	{
		r3d_assert(false); // shouldn't happen
	}

	playerData players[2][64];
	int numPlayers[2] = {0};

	ObjectManager& GW = GameWorld();
	for(GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
	{
		if(!(obj->isObjType(OBJTYPE_Human)))
			continue;
		const obj_AI_Player* plr = (obj_AI_Player*)obj;
		plr->GetUserNameAndClanTag(players[plr->TeamID][numPlayers[plr->TeamID]].username);
		players[plr->TeamID][numPlayers[plr->TeamID]].score = plr->GameStats.Score;
		players[plr->TeamID][numPlayers[plr->TeamID]].kills = plr->GameStats.Kills;
		players[plr->TeamID][numPlayers[plr->TeamID]].deaths = plr->GameStats.Deaths;
		players[plr->TeamID][numPlayers[plr->TeamID]].localPlayer = (plr==gClientLogic().localPlayer_)?true:false;
		++numPlayers[plr->TeamID];
		r3d_assert(numPlayers[plr->TeamID] <= 64);
	}
	qsort( &players[0][0], numPlayers[0], sizeof(playerData), playerData_Comparator );
	qsort( &players[1][0], numPlayers[1], sizeof(playerData), playerData_Comparator );
	
	// blue team
	gfxHUDScreen.SetVariable("_global.Players", numPlayers[0]);
	for(int i=0; i<numPlayers[0]; ++i)
	{
		char tmpStr[64];
		sprintf(tmpStr, "_global.BoardPlayer%dGametag", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[0][i].username);
		sprintf(tmpStr, "_global.BoardPlayer%dScore", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[0][i].score);
		sprintf(tmpStr, "_global.BoardPlayer%dK", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[0][i].kills);
		sprintf(tmpStr, "_global.BoardPlayer%dD", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[0][i].deaths);

		if(players[0][i].localPlayer)
			gfxHUDScreen.SetVariable("_global.UserPosition", i+1);
	}

	// red team
	gfxHUDScreen.SetVariable("_global.RedPlayers", numPlayers[1]);
	for(int i=0; i<numPlayers[1]; ++i)
	{
		char tmpStr[64];
		sprintf(tmpStr, "_global.RedBoardPlayer%dGametag", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[1][i].username);
		sprintf(tmpStr, "_global.RedBoardPlayer%dScore", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[1][i].score);
		sprintf(tmpStr, "_global.RedBoardPlayer%dK", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[1][i].kills);
		sprintf(tmpStr, "_global.RedBoardPlayer%dD", i+1);
		gfxHUDScreen.SetVariable(tmpStr, players[1][i].deaths);

		if(players[1][i].localPlayer)
			gfxHUDScreen.SetVariable("_global.UserPosition", i+1);
	}

	if(RUS_CLIENT) { // russian client is not using GP
		gClientLogic().curRoundStat_.GamePoints = 0;
		gClientLogic().curRoundScore_.GP = 0;
	}

	const wiStats& rstat = gClientLogic().curRoundStat_;
	const wiStatsTracking& rscore = gClientLogic().curRoundScore_;

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
		gfxMovie.Invoke("_root.api.setMapType", "conquest");
	else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch)
		gfxMovie.Invoke("_root.api.setMapType", "deathmatch");
	else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		gfxMovie.Invoke("_root.api.setMapType", "sabotage");


	gfxMovie.Invoke("_root.api.setXP", gUserProfile.ProfileData.Stats.HonorPoints); // current XP, before this match
	gfxMovie.Invoke("_root.api.setStatsTotalScore", R3D_MAX(rstat.HonorPoints, 0));
	gfxMovie.Invoke("_root.api.setStatsWPEarned", rstat.GameDollars);

	gfxMovie.Invoke("_root.api.setStatsBaseScore", rscore.HP);
	gfxMovie.Invoke("_root.api.setStatsBonusScore", 0); // 0 for now, should be end of round scores
	gfxMovie.Invoke("_root.api.setStatsKills", rstat.Kills);
	gfxMovie.Invoke("_root.api.setStatsDeaths", rstat.Deaths);
	gfxMovie.Invoke("_root.api.setStatsHeadshots", rstat.Headshots);
	gfxMovie.Invoke("_root.api.setStatsShotsFired", rstat.ShotsFired);
	char tmpStr[32];
	sprintf(tmpStr, "%.2f", getRatio((float)rstat.ShotsHits, (float)rstat.ShotsFired));
	gfxMovie.Invoke("_root.api.setStatsAccuracy", tmpStr); //Accuracy

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
	{
		gfxMovie.Invoke("_root.api.setStatsCapturedFlags", rstat.CaptureNeutralPoints + rstat.CaptureEnemyPoints);
	}
	else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		gfxMovie.Invoke("_root.api.setStatsPlantedBombs", gClientLogic().localPlayer_->numBombsPlanted);
		gfxMovie.Invoke("_root.api.setStatsDiffusedBombs", gClientLogic().localPlayer_->numBombsDiffused);
		gfxMovie.Invoke("_root.api.setStatsSaboWonRounds", gClientLogic().localPlayer_->numSaboRoundsWon);
	}

	gfxMovie.Invoke("_root.api.setSupplyCratesPickedUp", gClientLogic().localPlayer_->numLootBoxesPickedup);

	// FOR FRANK:
	// example for achievements, you should also list all achievements earned during the current match
	// gfxMovie.Invoke("_root.api.addAchievementItem", "$Data/Menu/medals/SharpShooter.dds");

	bool hasAnyMedals = false;
	if(rstat.Headshots > 0)
	{
		hasAnyMedals = true;
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/SharpShooter.dds");
		else
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/SharpShooter_RU.dds");
	}
	if(rstat.Kills >= 10)
	{
		hasAnyMedals = true;
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Eliminator.dds");
		else
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Eliminator_RU.dds");
	}
	else if(rstat.Kills >= 5)
	{
		hasAnyMedals = true;
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Badass.dds");
		else
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Badass_RU.dds");
	}
	if(rstat.CaptureNeutralPoints > 0)
	{
		hasAnyMedals = true;
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/First.dds");
	}
	if(rstat.CaptureEnemyPoints > 0)
	{
		hasAnyMedals = true;
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Agressor.dds");
		else
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/Agressor_RU.dds");
	}
	if(!hasAnyMedals)
	{
		if(!RUS_CLIENT)
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/FinishedRound.dds");
		else
			gfxMovie.Invoke("_root.api.addMedalItem", "$Data/Menu/medals/FinishedRound_RU.dds");
	}

	// one time reward for finishing round
	{
		Scaleform::GFx::Value var[3];
		bool addedReward = true;
		switch(gClientLogic().curRoundRewardIdx) // 0-100WP, 1-LootBox1, 2-LootBox2, 3-LootBox3, 4-2xWP for 20min, 5-2xXP for 20min
		{
		case 0:
			var[0].SetString("100WP");
			var[1].SetString("");
			var[2].SetString("$Data/Weapons/StoreIcons/WPIcon.dds");
			break;
		case 1:
			var[0].SetString(gWeaponArmory.getNameByItemId(301118, true));
			var[1].SetString("");
			var[2].SetString(gWeaponArmory.getIconByItemId(301118, true));
			break;
		case 2:
			var[0].SetString(gWeaponArmory.getNameByItemId(301119, true));
			var[1].SetString("");
			var[2].SetString(gWeaponArmory.getIconByItemId(301119, true));
			break;
		case 3:
			var[0].SetString(gWeaponArmory.getNameByItemId(301120, true));
			var[1].SetString("");
			var[2].SetString(gWeaponArmory.getIconByItemId(301120, true));
			break;
		case 4: // wp
			var[0].SetString(gWeaponArmory.getNameByItemId(301003, true));
			var[1].SetStringW(gLangMngr.getString("$HUD_EndRound_20MinRental"));
			var[2].SetString(gWeaponArmory.getIconByItemId(301003, true));
			break;
		case 5: // xp
			var[0].SetString(gWeaponArmory.getNameByItemId(301001, true));
			var[1].SetStringW(gLangMngr.getString("$HUD_EndRound_20MinRental"));
			var[2].SetString(gWeaponArmory.getIconByItemId(301001, true));
			break;
		default:
			addedReward = false;
			break;
		}
		if(addedReward)
			gfxMovie.Invoke("_root.api.addRewardItem", var, 3);
	}


	// increase gUserProfile with passed stats
	gUserProfile.ProfileData.Stats += rstat;

	//if(gClientLogic().m_gameInfo.practiceGame)
	//	gfxMovie.Invoke("_root.api.showPracticeGameWarning", "");

	m_movieInitComplete = true;
}

void HUDEndRound::ShowAchievementRibbon( int whichAchievement )
{

	const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByID( whichAchievement);
	r3d_assert(achievementInfo);
	gfxMovie.Invoke("_root.api.addAchievementItem", achievementInfo->hud_icon );

}
