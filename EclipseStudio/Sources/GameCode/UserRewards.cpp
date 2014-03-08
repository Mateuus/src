#include "r3dPCH.h"
#include "r3d.h"
#include "shellapi.h"

#include "UserRewards.h"
#include "backend/WOBackendAPI.h"

	CGameRewards*	g_GameRewards = NULL;

CGameRewards::CGameRewards()
{
	loaded_ = false;
	InitDefaultRewards();
}

CGameRewards::~CGameRewards()
{
}

void CGameRewards::SetReward(int id, const char* name, int v1, int v2, int v3, int v4, int v5, int v6)
{
	r3d_assert(id >= 0 && id < RWD_MAX_REWARD_ID);
	rwd_s& rwd = rewards_[id];
	rwd.Name  = name;
	rwd.GD_CQ = v1;
	rwd.HP_CQ = v2;
	rwd.GD_DM = v3;
	rwd.HP_DM = v4;
	rwd.GD_SB = v5;
	rwd.HP_SB = v6;
	rwd.IsSet = true;
}

void CGameRewards::InitDefaultRewards()
{
	// HERE is COPY-PASTED CODE FROM ExportDefaultRewards()
	// USED FOR SERVER

	SetReward(1, "Kill", 2, 10, 5, 10, 10, 10);
	SetReward(2, "Headshot", 10, 20, 10, 20, 10, 10);
	SetReward(3, "AssistedKill", 1, 10, 1, 5, 10, 10);
	SetReward(4, "ExplosionKill", 100, 50, 10, 10, 10, 10);
	SetReward(5, "FriendlyKill", 0, -20, 0, 0, 0, -20);
	SetReward(6, "AvengeKill", 10, 100, 10, 10, 10, 10);
	SetReward(7, "RevengeKill", 20, 250, 20, 15, 10, 10);
	SetReward(14, "Killstreak2", 100, 50, 10, 10, 0, 0);
	SetReward(15, "Killstreak3", 150, 100, 20, 20, 0, 0);
	SetReward(16, "Killstreak4", 200, 125, 50, 35, 0, 0);
	SetReward(17, "Killstreak5", 500, 150, 100, 50, 0, 0);
	SetReward(18, "Killstreak6", 1000, 200, 250, 75, 0, 0);
	SetReward(100, "CaptureNeutral", 100, 250, 0, 0, 0, 0);
	SetReward(101, "CaptureEnemy", 200, 500, 0, 0, 0, 0);
	SetReward(102, "KillCloseToYourFlag", 50, 50, 0, 0, 0, 0);
	SetReward(103, "KillCloseToTheirFlag", 50, 20, 0, 0, 0, 0);
	SetReward(104, "KillAtSpawn", 0, 0, 0, 0, 0, 0);
	SetReward(120, "Bomb_PlantBomb", 0, 0, 0, 0, 10, 200);
	SetReward(121, "Bomb_DiffuseBomb", 0, 0, 0, 0, 20, 200);
	SetReward(122, "Bomb_WinRoundKills", 0, 0, 0, 0, 10, 10);
	SetReward(123, "Bomb_WinRound", 0, 0, 0, 0, 25, 50);
	SetReward(124, "Bomb_Pickup", 0, 0, 0, 0, 5, 0);
	SetReward(140, "Win", 500, 500, 500, 500, 500, 500);
	SetReward(141, "Loss", 200, 250, 200, 250, 200, 250);
	SetReward(142, "BestPlayerInTeam", 150, 250, 150, 250, 150, 250);
	SetReward(143, "BestPlayerOverall", 250, 500, 250, 500, 250, 500);
	SetReward(200, "BlackopResupply", 10, 10, 10, 10, 10, 10);
	SetReward(201, "UseUAV", 0, 50, 0, 50, 0, 50);
	SetReward(202, "UAVAssist", 50, 50, 50, 50, 50, 50);
	SetReward(203, "DestroyMine", 0, 20, 0, 20, 0, 20);
	SetReward(300, "FirstCaptureNeutral", 150, 100, 0, 0, 0, 0);
	SetReward(301, "FirstCaptureEnemy", 250, 200, 0, 0, 0, 0);
	SetReward(302, "Kill5NotDying", 25, 50, 25, 50, 25, 50);
	SetReward(303, "Kill10NotDying", 65, 100, 65, 100, 65, 100);
	SetReward(304, "Kill25NotDying", 150, 200, 150, 200, 150, 200);
	SetReward(305, "Kill50NotDying", 150, 200, 150, 200, 150, 200);
	SetReward(306, "KillGrenadeAfterDeath", 10, 100, 10, 100, 10, 100);
	SetReward(307, "KillEnemyWithHisGun", 50, 50, 50, 50, 50, 50);
}

void CGameRewards::ExportDefaultRewards()
{
	const char* fname = "@rewards.txt";
	FILE* f = fopen_for_write(fname, "wt");
	for(int i=0; i<RWD_MAX_REWARD_ID; i++)
	{
		const rwd_s& rwd = GetRewardData(i);
		if(!rwd.IsSet) continue;
		fprintf(f, "\tSetReward(%d, \"%s\", %d, %d, %d, %d, %d, %d);\n",
			i, 
			rwd.Name.c_str(), 
			rwd.GD_CQ, rwd.HP_CQ,
			rwd.GD_DM, rwd.HP_DM,
			rwd.GD_SB, rwd.HP_SB);
	}
	fclose(f);

	ShellExecute(NULL, "open", "@rewards.txt", "", "", SW_SHOW);
	MessageBox(NULL, "ExportDefaultRewards success", "ExportDefaultRewards", MB_OK);
	TerminateProcess(r3d_CurrentProcess, 0);
}


int CGameRewards::ApiGetDataGameRewards()
{
	CWOBackendReq req("api_GetDataGameRewards.aspx");
	// HACK_FIX: without this line we'll get HTTP 411 on live IIS by some reasons
	// description:
	//  The Web server (running the Web site) thinks that the HTTP data stream sent by the client (e.g. your Web browser or our CheckUpDown robot) should include a 'Content-Length' specification
	req.AddParam("411", "1");

	if(!req.Issue())
	{
		r3dOutToLog("ApiGetDataGameRewards FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	pugi::xml_node xmlRwd = xmlFile.child("rewards");
	
	// parse all rewards
	xmlRwd = xmlRwd.first_child();
	while(!xmlRwd.empty())
	{
		int RewardID = xmlRwd.attribute("ID").as_int();
		if(RewardID < 0 || RewardID >= RWD_MAX_REWARD_ID) {
		#ifndef FINAL_BUILD
			r3dError("received invalid reward %d", RewardID);
		#endif
			xmlRwd = xmlRwd.next_sibling();
			continue;
		}

		rwd_s& rwd = rewards_[RewardID];
		rwd.Name  = xmlRwd.attribute("Name").value();
		rwd.GD_CQ = xmlRwd.attribute("GD_CQ").as_int();
		rwd.HP_CQ = xmlRwd.attribute("HP_CQ").as_int();
		rwd.GD_DM = xmlRwd.attribute("GD_DM").as_int();
		rwd.HP_DM = xmlRwd.attribute("HP_DM").as_int();
		rwd.GD_SB = xmlRwd.attribute("GD_SB").as_int();
		rwd.HP_SB = xmlRwd.attribute("HP_SB").as_int();
		rwd.IsSet = true;

		xmlRwd = xmlRwd.next_sibling();
	}
	
	loaded_ = true;
	
	//ExportDefaultRewards();
		
	return 0;
}

