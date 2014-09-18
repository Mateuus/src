#pragma once
#include "GameCode/UserProfile.h"
#include "GameCode/UserRewards.h"

// track usage per each loadout
struct CLoadoutUsageTrack
{
	int		LoadoutID;
	  
	float		StartPlayTime;
	float		TimePlayed;
	int		HonorPoints;

	CLoadoutUsageTrack()
	{
		LoadoutID     = 0; // will be filled in end of game update
		StartPlayTime = -1;
		TimePlayed    = 0;
		HonorPoints   = 0;
	}
	  
	void AddPlayTime()
	{
		r3d_assert(StartPlayTime >= 0);
		TimePlayed   += (r3dGetTime() - StartPlayTime);
		StartPlayTime = -1;
	}
};

class CServerUserProfile : public CUserProfile
{
  public:
	CServerUserProfile();

	int ApiGiveItem(int itemId, int expiration);
	int ApiGiveItemInMin(int itemId, int expiration_minutes);
	int ApiRemoveItem(int itemId);
	int ApiUpdateAchievements(int numAchs, wiAchievement* achs);

	virtual bool MarkAchievementComplete( int whichAchievement );

};
