#pragma once

// DO NOT CHANGE ID OF REWARD!!!
enum EPlayerRewardID
{
	RWD_Kill		= 1,	//Standard
	RWD_Headshot		= 2,	//Headshot
	RWD_AssistedKill	= 3,	//Assist ( hit right before next guy killed target )
	RWD_ExplosionKill	= 4,	//Explosion
	RWD_FriendlyKill	= 5,	//Kill team mate
	RWD_AvengeKill		= 6,	//Avenge kill ( kill enemy who just killed your team mate )
	RWD_RevengeKill		= 7,	//Revenge Kill - kill enemy who killed you
	RWD_Killstreak2		= 14,	//Killstreak 2
	RWD_Killstreak3		= 15,	//Killstreak 3
	RWD_Killstreak4		= 16,	//Killstreak 4
	RWD_Killstreak5		= 17,	//Killstreak 5
	RWD_Killstreak6		= 18,	//Killstreak 6
		
	// CONQUEST ONLY REWARDS
	RWD_CaptureNeutral	= 100,	//Capture Neutral Flag
	RWD_CaptureEnemy	= 101,	//Capture Enemy Flag
	RWD_KillCloseToYourFlag	= 102,	//Kill An Enemy within 25 meters of Your Flag
	RWD_KillCloseToTheirFlag= 103,	//Kill An Enemy within 35 meters of Their Flag 
	RWD_KillAtSpawn		= 104,	//Special 'NO REWARD' for killing at spawn point
		
	// SABOTAGE ONLY REWARDS
	RWD_Bomb_PlantBomb	= 120,	//Plant the Bomb
	RWD_Bomb_DiffuseBomb	= 121,	//Diffuse the Bomb
	RWD_Bomb_WinRoundKills	= 122,	//Win Round (Kills)
	RWD_Bomb_WinRound	= 123,	//Win Round (Objective Complete)
	RWD_Bomb_Pickup		= 124,	//Pick Up The Bomb

	// END OF GAME ROUND
	RWD_Win			= 140,	//Win Match
	RWD_Loss		= 141,	//Lose Match
	RWD_BestPlayerInTeam	= 142,	//Best Player in a team
	RWD_BestPlayerOverall	= 143,	//Best player overall 
		
	//USE OF ITEMS / SPECIAL REWARDS
	RWD_BlackopResupply	= 200,	//Used blackops resupply
	RWD_UseUAV		= 201,	//Use Camera Drone
	RWD_UAVAssist		= 202,	//Player Killed while Tagged by Drone
	RWD_DestroyMine		= 203,	//Destroy enemy mine
		
	// Achievements
	RWD_FirstCaptureNeutral	= 300,	//First neutral flag capture in your team: 
	RWD_FirstCaptureEnemy	= 301,	//First enemy flag capture in your team:
	RWD_Kill5NotDying	= 302,	//Kill 5 without dying:
	RWD_Kill10NotDying	= 303,	//Kill 10 without dying: 
	RWD_Kill25NotDying	= 304,	//Kill 25 without dying: 
	RWD_Kill50NotDying	= 305,	//Kill 50 without dying: 
	RWD_KillGrenadeAfterDeath= 306,	//Kill with grenade after dying: 
	RWD_KillEnemyWithHisGun	= 307,//Kill enemy with his own gun (that you picked up): 
	
	RWD_MAX_REWARD_ID	= 512,
};

class CGameRewards
{
  public:
	struct rwd_s
	{
	  bool		IsSet;
	  std::string	Name;
	
	  int		GD_CQ;
	  int		HP_CQ;
	  int		GD_DM;
	  int		HP_DM;
	  int		GD_SB;
	  int		HP_SB;
	  
	  rwd_s()
	  {
		IsSet = false;
	  }
	};
	
	bool		loaded_;
	rwd_s		rewards_[RWD_MAX_REWARD_ID];

	void		InitDefaultRewards();
	void		  SetReward(int id, const char* name, int v1, int v2, int v3, int v4, int v5, int v6);
	void		ExportDefaultRewards();
	
  public:
	CGameRewards();
	~CGameRewards();
	
	int		ApiGetDataGameRewards();

	const rwd_s&	GetRewardData(int rewardID)
	{
		r3d_assert(rewardID >= 0 && rewardID < RWD_MAX_REWARD_ID);
		return rewards_[rewardID];
	}
};

extern CGameRewards*	g_GameRewards;
