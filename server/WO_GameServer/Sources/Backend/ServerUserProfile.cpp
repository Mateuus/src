#include "r3dPCH.h"
#include "r3d.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"
#include "CkByteData.h"

#include "backend/ServerUserProfile.h"
#include "../../EclipseStudio/Sources/backend/WOBackendAPI.h"

#include "ObjectsCode/WEAPONS/WeaponConfig.h"
#include "ObjectsCode/WEAPONS/WeaponArmory.h"

#include "../AsyncFuncs.h"
#include "../ServerGameLogic.h"

const static char* gServerWOApiKey = "CfFkqQWjfgksYG56893GDhjfjZ20";

CServerUserProfile::CServerUserProfile()
{
}

int CServerUserProfile::ApiGiveItem(int itemId, int expiration)
{
	CWOBackendReq req(this, "api_SrvGiveItem.aspx");
	req.AddParam("ItemID",  itemId);
	req.AddParam("BuyDays", expiration);
	req.AddParam("skey1",   gServerWOApiKey);
	if(!req.Issue())
	{
		r3dOutToLog("ApiGiveItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CServerUserProfile::ApiGiveItemInMin(int itemId, int expiration_minutes)
{
	CWOBackendReq req(this, "api_SrvGiveItemInMinutes.aspx");
	req.AddParam("ItemID",  itemId);
	req.AddParam("BuyMinutes", expiration_minutes);
	req.AddParam("skey1",   gServerWOApiKey);
	if(!req.Issue())
	{
		r3dOutToLog("ApiGiveItemInMin FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CServerUserProfile::ApiRemoveItem(int itemId)
{
	CWOBackendReq req(this, "api_SrvRemoveItem.aspx");
	req.AddParam("ItemID",  itemId);
	req.AddParam("skey1",   gServerWOApiKey);
	if(!req.Issue())
	{
		r3dOutToLog("ApiRemoveItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CServerUserProfile::ApiUpdateAchievements(int numAchs, wiAchievement* achs)
{
	
	if(numAchs>0)
	{
		CWOBackendReq req(this, "api_SrvUpdateAchievements.aspx");
		req.AddParam("skey1",  gServerWOApiKey);
		req.AddParam("NumAch", numAchs);
		char achIDstr[32];
		char achValstr[32];
		char achUnlstr[32];
		for(int i=0; i<numAchs; ++i)
		{
			sprintf(achIDstr, "AchID%d", i);
			sprintf(achValstr, "AchVal%d", i);
			sprintf(achUnlstr, "AchUnl%d", i);

			req.AddParam(achIDstr, achs[i].ID);
			req.AddParam(achValstr, achs[i].value);
			req.AddParam(achUnlstr, achs[i].unlocked);

		}
		if(!req.Issue())
		{
			r3dOutToLog("ApiUpdateAchievements FAILED, code: %d\n", req.resultCode_);
			return req.resultCode_;
		}
	}

	return 0;
}



bool CServerUserProfile::MarkAchievementComplete( int whichAchievement )
{

	bool returnValue = CUserProfile::MarkAchievementComplete( whichAchievement );

	if( returnValue == true ) {

		const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
		wiAchievement* achievementData = ProfileData.getAchievementDataByID( achievementInfo->id );

		ServerGameLogic::AchievementThreadData* achivementThreadData = new ServerGameLogic::AchievementThreadData;
		achivementThreadData->targetProfile = this;
		achivementThreadData->achievement = achievementData;
		achivementThreadData->achievementCount = 1;
		CreateThread(NULL, 0, UpdateAchievementsThread, achivementThreadData, 0, NULL);
		achievementData->dirty = false;
	}

	return returnValue;
}
