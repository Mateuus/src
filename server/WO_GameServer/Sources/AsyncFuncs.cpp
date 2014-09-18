#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"

#include "AsyncFuncs.h"
#include "ServerGameLogic.h"
#include "ObjectsCode/obj_ServerPlayer.h"
#include "ObjectsCode/WEAPONS/WeaponArmory.h"
#include "ObjectsCode/WEAPONS/WeaponConfig.h"

#include "../../EclipseStudio/Sources/backend/WOBackendAPI.h"

extern	__int64 cfg_sessionId;

DWORD WINAPI GetProfileDataThread(void* in_ptr)
{
	ServerGameLogic::peerInfo_s& peer = *(ServerGameLogic::peerInfo_s*)in_ptr;
	r3dOutToLog("@@ PROFILE [%d]: started\n", peer.CustomerID);
  
	CServerUserProfile prof;
	prof.CustomerID = peer.CustomerID;
	prof.SessionID  = peer.SessionID;
	if(prof.GetProfile(true) != 0)
	{
		r3dOutToLog("@@ PROFILE [%d]: failed to GetProfile()\n", prof.CustomerID);
		peer.haveProfile  = 2;
		return 0;
	}
  
	// set profile
	r3d_assert(peer.CustomerID == prof.CustomerID);
	peer.temp_profile = prof;
	peer.haveProfile  = 1;
	peer.getProfileH  = NULL;
	r3dOutToLog("@@ PROFILE [%d]: finished: screen: %s\n", prof.CustomerID, prof.ScreenName);
	return 0;
}

DWORD WINAPI AddWeaponStatsThread(void* in_ptr)
{
	CWOBackendReq req("api_SrvAddWeaponStats.aspx");
	req.AddParam("skey1", "CfFkqQWjfgksYG56893GDhjfjZ20");
	
	req.AddParam("MapType", gServerLogic.ginfo_.mapType);
	for(size_t i=0, size=gServerLogic.weaponStats_.size(); i<size; ++i)
	{
		const ServerGameLogic::WeaponStats_s& ws = gServerLogic.weaponStats_[i];
		
		char key[256];
		sprintf(key, "w%d", i);
		char wstr[1024];
		sprintf(wstr, "%d %d %d %d", ws.ItemID, ws.ShotsFired, ws.ShotsHits, ws.Kills);
		
		req.AddParam(key, wstr);
	}
	
	if(!req.Issue())
	{
		r3dOutToLog("!!!! AddWeaponStatsThread failed, code: %d\n", req.resultCode_);
		return 0;
	}

	return 0;
}

DWORD WINAPI UpdateProfileThread(void* in_ptr)
{
	ServerGameLogic::ProfileUpd_s& upd = *(ServerGameLogic::ProfileUpd_s*)in_ptr;
	r3dOutToLog("@@ UpdateProfileThread[%d]: started, customer: %d\n", upd.idx, upd.CustomerID);
  
	// give 50 msec for each thread to start
	::Sleep(upd.idx * 50);

	CWOBackendReq req("api_SrvAddUserRoundResult4.aspx");
	req.AddSessionInfo(upd.CustomerID, 0);
	req.AddParam("skey1", "CfFkqQWjfgksYG56893GDhjfjZ20");
	
	char gsid[128];
	sprintf(gsid, "%I64d", cfg_sessionId);
	req.AddParam("GameSessionID", gsid);
	
	const wiStats& stat = upd.stats;
	req.AddParam("MapID",       upd.MapID);
	req.AddParam("MapType",     upd.MapType);
	req.AddParam("TeamID",      upd.TeamID);
	req.AddParam("GamePoints",  stat.GamePoints);
	req.AddParam("GameDollars",  stat.GameDollars);
	req.AddParam("HonorPoints", stat.HonorPoints);
	req.AddParam("SkillPoints", stat.SkillPoints);
	req.AddParam("Kills",       stat.Kills);
	req.AddParam("Deaths",      stat.Deaths);
	req.AddParam("ShotsFired",  stat.ShotsFired);
	req.AddParam("ShotsHits",   stat.ShotsHits);
	req.AddParam("Headshots",   stat.Headshots);
	req.AddParam("AssistKills", stat.AssistKills);
	req.AddParam("Wins",        stat.Wins);
	req.AddParam("Losses",      stat.Losses);
	req.AddParam("CaptureNeutralPoints", stat.CaptureNeutralPoints);
	req.AddParam("CaptureEnemyPoints",   stat.CaptureEnemyPoints);
	req.AddParam("TimePlayed",  stat.TimePlayed);
	req.AddParam("LevelUpMin", upd.plrLevelUpMin);
	req.AddParam("LevelUpMax", upd.plrLevelUpMax);
	
	// add per loadout stats
	int loadoutIdx = 0;
	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; i++) 
	{
		const CLoadoutUsageTrack& lu = upd.loadoutUsage_[i];
		if(lu.LoadoutID == 0)
			continue;
		if(lu.HonorPoints == 0 && (int)lu.TimePlayed == 0)
			continue;
			
		char key1[64];
		char key2[64];
		char key3[64];
		sprintf(key1, "lid%d", loadoutIdx);
		sprintf(key2, "ltp%d", loadoutIdx);
		sprintf(key3, "lxp%d", loadoutIdx);
		req.AddParam(key1, lu.LoadoutID);
		req.AddParam(key2, (int)lu.TimePlayed);
		req.AddParam(key3, lu.HonorPoints);
		
		loadoutIdx++;
	}
	
	if(!req.Issue())
	{
		upd.status = 2;
		//@TODO: CRASH OR SEND EMAIL???
		r3dOutToLog("!!!! UpdateProfile failed, code: %d\n", req.resultCode_);
		return 0;
	}

	r3dOutToLog("@@ UpdateProfileThread[%d] Part 1: finished\n", upd.idx);

	wiAchievement achievementsToRecord[NUM_ACHIEVEMENTS];
	int numAchievementsToUpdate = 0;
	for( int achievementIndex = 0; achievementIndex < NUM_ACHIEVEMENTS; achievementIndex++ )
	{
		const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( achievementIndex );
		wiAchievement* achievementData = upd.targetProfile->ProfileData.getAchievementDataByID( achievementInfo->id );
		if ( achievementData != NULL && achievementData->dirty == true ) {
			achievementsToRecord[ numAchievementsToUpdate ] = *achievementData;
			numAchievementsToUpdate++;
			achievementData->dirty = false;
		}
	}
	if ( numAchievementsToUpdate > 0 ) {
		upd.targetProfile->ApiUpdateAchievements( numAchievementsToUpdate, achievementsToRecord);
	}

	r3dOutToLog("@@ UpdateProfileThread[%d]: finished\n", upd.idx);
	upd.status = 1;

	return 0;
}

DWORD WINAPI AddLogInfoThread(void* in_ptr)
{
	ServerGameLogic::LogInfo_s* data = (ServerGameLogic::LogInfo_s*)in_ptr;
	//r3dOutToLog("AddLogInfoThread: started, customer: %d\n", data->CustomerID);
  
	CWOBackendReq req("api_SrvAddLogInfo.aspx");
	req.AddParam("skey1", "CfFkqQWjfgksYG56893GDhjfjZ20");
	
	char gsid[128];
	sprintf(gsid, "%I64d", cfg_sessionId);
	req.AddParam("GameSessionID", gsid);

	req.AddParam("s_id",    data->CustomerID);	// note - it CAN be null for not-connected users
	req.AddParam("CheatID", data->CheatID);
	req.AddParam("Msg",     data->Msg);
	req.AddParam("Data",    data->Data);
	
	char ipstr[128];
	sprintf(ipstr, "%u", 	data->IP);
	req.AddParam("IP",      ipstr);


	// free memory
	delete data;
	
	// issue
	if(!req.Issue())
	{
		r3dOutToLog("!!!! AddLogInfoThread failed, code: %d\n", req.resultCode_);
		return 0;
	}

	return 0;
}

DWORD WINAPI PlayerBuyItemThread(void* in_ptr)
{
	ServerGameLogic::PlayerBuyItem_s* data = (ServerGameLogic::PlayerBuyItem_s*)in_ptr;
	r3dOutToLog("@@ PlayerBuyItemThread: started, customer: %d, itemID: %d\n", data->CustomerID, data->itemID);
  
	CWOBackendReq req("api_BuyItem3.aspx");
	req.AddSessionInfo(data->CustomerID, data->SessionID);
	
	int buyIdx = data->BuyIdx;
	extern int RUS_CLIENT;
	if(RUS_CLIENT && buyIdx >= 1 && buyIdx <= 4) // move buy idx to gamenet range [9-12]
		buyIdx += 8;

	req.AddParam("ItemID", data->itemID);
	req.AddParam("BuyIdx", buyIdx);
	
	if(!req.Issue())
	{
		r3dOutToLog("!!!! PlayerBuyItemThread failed, code: %d\n", req.resultCode_);
		return 0;
	}

	r3dOutToLog("@@ PlayerBuyItemThread: finished\n");

	delete data; // free memory

	return 0;
}

DWORD WINAPI PlayerGiveItemThread(void* in_ptr)
{
	ServerGameLogic::PlayerGiveItem_s* data = (ServerGameLogic::PlayerGiveItem_s*)in_ptr;
	
	// make temp profile and give item on server
	CServerUserProfile profile;
	profile.SessionID  = data->SessionID;
	profile.CustomerID = data->CustomerID;
	profile.ApiGiveItem(data->itemID, 2000);
  
	delete data; // free memory

	return 0;
}

DWORD WINAPI PlayerGiveItemInMinThread(void* in_ptr)
{
	ServerGameLogic::PlayerGiveItemInMin_s* data = (ServerGameLogic::PlayerGiveItemInMin_s*)in_ptr;
	
	// make temp profile and give item on server
	CServerUserProfile profile;
	profile.SessionID  = data->SessionID;
	profile.CustomerID = data->CustomerID;
	profile.ApiGiveItemInMin(data->itemID, data->minutes);
  
	delete data; // free memory

	return 0;
}

DWORD WINAPI PlayerRemoveItemThread(void* in_ptr)
{
	ServerGameLogic::PlayerRemoveItem_s* data = (ServerGameLogic::PlayerRemoveItem_s*)in_ptr;
	
	// make temp profile and give item on server
	CServerUserProfile profile;
	profile.SessionID  = data->SessionID;
	profile.CustomerID = data->CustomerID;
	profile.ApiRemoveItem(data->itemID);
  
	delete data; // free memory

	return 0;
}

DWORD WINAPI UpdateAchievementsThread(void* in_ptr)
{
	ServerGameLogic::AchievementThreadData* data = ( ServerGameLogic::AchievementThreadData* ) in_ptr;
	data->targetProfile->ApiUpdateAchievements( data->achievementCount, data->achievement );
	delete data;

	return 0;
}