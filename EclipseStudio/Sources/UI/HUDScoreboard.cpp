#include "r3dPCH.h"
#include "r3d.h"
#include "r3dProtect.h"

#include "HUDScoreboard.h"

#include "ObjectsCode/AI/AI_Player.h"
#include "../multiplayer/ClientGameLogic.h"

HUDScoreboard::HUDScoreboard()
{
	isActive_ = false;
	isInit = false;
}

HUDScoreboard::~HUDScoreboard()
{
}

bool HUDScoreboard::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\Scoreboard.swf", true)) 
		return false;

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

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
		extern int RUS_CLIENT;
		if(!RUS_CLIENT)
		{
			// add virtual ranks past level 60. not more than 99 total
			for(int i=0; i<39; ++i)
			{
				var[0].SetStringW(gLangMngr.getString("PlayerRank59"));
				var[1].SetNumber(g_RanksPoints[MAX_NUM_RANKS-1] + 14141000*(i+1));
				gfxMovie.Invoke("_root.api.m_RanksData.addRank", var, 2);
			}
		}
	}

	isActive_ = false;
	isInit = true;
	return true;
}

bool HUDScoreboard::Unload()
{
	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDScoreboard::Update()
{
	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
}

void HUDScoreboard::Draw()
{
	gfxMovie.UpdateAndDraw();
}

void HUDScoreboard::Deactivate()
{
	isActive_ = false;
}

struct ScoreboardPlayerStat
{
	char name[256];
	int score;
	int kills;
	int deaths;
	int totalXP;
	bool isPremium;
	bool isTop;
	bool isDead;
	bool isLocalPlayer;
	bool hasBomb;
	void set(int s, int k, int d, int total, bool prem, bool top, bool dead, bool localP, bool hasB) 
	{
		score =s; kills = k; deaths = d; 
		totalXP = total; isPremium = prem; isTop = top; isDead = dead; isLocalPlayer = localP; hasBomb = hasB;
	}
};

static int sortByScore(const void *P1, const void *P2)
{
	const ScoreboardPlayerStat* g1 = (const ScoreboardPlayerStat*)P1;
	const ScoreboardPlayerStat* g2 = (const ScoreboardPlayerStat*)P2;

	return g2->score - g1->score;
}

void HUDScoreboard::Activate()
{
	r3d_assert(!isActive_);
	isActive_ = true;

	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.removePlayers"), "");

	ScoreboardPlayerStat allyPlayers[ClientGameLogic::MAX_NUM_PLAYERS];
	ScoreboardPlayerStat enemyPlayers[ClientGameLogic::MAX_NUM_PLAYERS];
	int numAlly=0, numEnemy=0;

	const ClientGameLogic& CGL = gClientLogic();
	for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
	{
		obj_AI_Player* plr = CGL.GetPlayer(i);
		if(!plr)
			continue;
			
		bool ally = false;
		if(CGL.localPlayer_)
		{
			if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
				ally = plr->TeamID == 1;
			else
				ally = plr->TeamID==CGL.localPlayer_->TeamID;
		}
		else
			ally = plr->TeamID == 1;

		if(ally)
		{
			plr->GetUserNameAndClanTag(allyPlayers[numAlly].name);
			allyPlayers[numAlly].set(plr->GameStats.Score, plr->GameStats.Kills, plr->GameStats.Deaths, plr->GameStats.TotalScore, plr->isPremiumAccount, plr->GameStats.TotalScore > 1400000, plr->bDead==1, CGL.localPlayer_ == plr, plr->hasSabotageBomb());
			numAlly++;
		}
		else
		{
			plr->GetUserNameAndClanTag(enemyPlayers[numEnemy].name);
			enemyPlayers[numEnemy].set(plr->GameStats.Score, plr->GameStats.Kills, plr->GameStats.Deaths, plr->GameStats.TotalScore, plr->isPremiumAccount, plr->GameStats.TotalScore > 1400000, plr->bDead==1, CGL.localPlayer_ == plr, plr->hasSabotageBomb());
			numEnemy++;
		}
	}

	// sort by score
	qsort((void *)allyPlayers, numAlly, sizeof(allyPlayers[0]), sortByScore);
	qsort((void *)enemyPlayers, numEnemy, sizeof(enemyPlayers[0]), sortByScore);
	
	Scaleform::GFx::Value vars[11];
	for(int i=0; i<numAlly; i++)
	{
		vars[0].SetBoolean(true);
		vars[1].SetString(allyPlayers[i].name);
		vars[2].SetNumber(allyPlayers[i].score);
		vars[3].SetNumber(allyPlayers[i].kills);
		vars[4].SetNumber(allyPlayers[i].deaths);
		vars[5].SetNumber(allyPlayers[i].totalXP);
		vars[6].SetBoolean(allyPlayers[i].isPremium);
		vars[7].SetBoolean(allyPlayers[i].isTop);
		vars[8].SetBoolean(allyPlayers[i].isDead);
		vars[9].SetBoolean(allyPlayers[i].isLocalPlayer);
		vars[10].SetBoolean(allyPlayers[i].hasBomb);
		gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.addPlayer"), vars, 11);
	}
	for(int i=0; i<numEnemy; i++)
	{
		vars[0].SetBoolean(false);
		vars[1].SetString(enemyPlayers[i].name);
		vars[2].SetNumber(enemyPlayers[i].score);
		vars[3].SetNumber(enemyPlayers[i].kills);
		vars[4].SetNumber(enemyPlayers[i].deaths);
		vars[5].SetNumber(enemyPlayers[i].totalXP);
		vars[6].SetBoolean(enemyPlayers[i].isPremium);
		vars[7].SetBoolean(enemyPlayers[i].isTop);
		vars[8].SetBoolean(enemyPlayers[i].isDead);
		vars[9].SetBoolean(enemyPlayers[i].isLocalPlayer);
		vars[10].SetBoolean(enemyPlayers[i].hasBomb);
		gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.addPlayer"), vars, 11);
	}

	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.populateList"), "");
}


