#pragma once
// define gameplay constants here

// MAKE SURE to increase this value after adding/removing/moving variables
#define GAMEPLAYPARAM_VERSION	0x000005

#pragma pack(push)
#pragma pack(1)
struct CGamePlayParams
{
	//////////////////////////////////////////////////////////////
	/////////// PLAYER SHOOTING CONSTANTS ////////////////////////
	//////////////////////////////////////////////////////////////
	float c_fSpreadRecoilMod_Running;
	float c_fSpreadRecoilMod_Aim;
	float c_fSpreadRecoilMod_Crouch;
	float c_fSpreadRecoilMod_Moving;
	float c_fSpreadTriggerHappyNum; // after ten bullets your spread will increase by
	float c_fSpreadTriggerHappyMod; // 1.0f - means doubles (as we adding, not multiplying)
	float c_fSpreadModShotgun_Aim;
	float c_fSpreadModShotgun_Crouch;
	float c_fGlobalSpreadMod;

	float c_fZeroSpreadDelayBetweenShots;

	float c_fRecoilCooldownSpeed;

	float c_MaxVerticalRecoil;

	float c_fSprintMaxEnergy;

	float c_fMinimapEnemyShootVisibleTimer;
	float c_fMinimapEnemyShootVisibleTimerSniper;
    float c_fMinimapEnemyShootVisibleRadius;
	float c_fMinimapEnemyVisibileRadius;

	float c_fMinimapNinjaKillerTimer;

	float NOT_USED__REUSE_ME;

	float c_fKillStreakTimeout; // kill streaks count if killed within this time (update this on server too in RewardKillPlayer())

	float c_fSniperRifleHoldBreathTime;
	float c_fSniperRifleAfterHoldBounceTime;

	float c_fFlashBangDuration;
	float c_fFlashBangRadius;

	float c_fHitMarkerFadeoutSpeed;

	float c_fRegenSpeed;

	// speeds
    float AI_BASE_MOD_SIDE;
    float AI_BASE_MOD_FORWARD;
    float AI_BASE_MOD_BACKWARD;
	float AI_WALK_SPEED;
	float AI_RUN_SPEED;
	float AI_SPRINT_SPEED;
	float UAV_FLY_SPEED_V;
	float UAV_FLY_SPEED_H;


//  set default values here
	CGamePlayParams()
	{
    //////////////////////////////////////////////////////////////
   	/////////// PLAYER SHOOTING CONSTANTS ////////////////////////
    //////////////////////////////////////////////////////////////
    c_fSpreadRecoilMod_Running = 1.2f;
	c_fSpreadRecoilMod_Aim = 0.4f;
	c_fSpreadRecoilMod_Crouch = 0.85f;
	c_fSpreadRecoilMod_Moving = 1.2f;
	c_fSpreadModShotgun_Aim = 1.00f;
	c_fSpreadModShotgun_Crouch = 0.9f;
	c_fGlobalSpreadMod = 1.0f;

	c_fZeroSpreadDelayBetweenShots = 0.5f;

	c_fRecoilCooldownSpeed = 4.5f;

	c_MaxVerticalRecoil = 8.0f;

	c_fSprintMaxEnergy = 9.0f;

	c_fMinimapEnemyShootVisibleTimer = 1.0f;
	c_fMinimapEnemyShootVisibleTimerSniper = 10.0f;
    c_fMinimapEnemyShootVisibleRadius = 30.0f;
	c_fMinimapEnemyVisibileRadius = 3.0f;

	c_fMinimapNinjaKillerTimer = 30.0f;

	NOT_USED__REUSE_ME = 1.0f;

	c_fKillStreakTimeout = 15.0f; // kill streaks count if killed within this time (update this on server too in RewardKillPlayer())

	c_fSniperRifleHoldBreathTime = 3.0f;
	c_fSniperRifleAfterHoldBounceTime = 3.0f;

	c_fFlashBangDuration = 5.0f;
	c_fFlashBangRadius = 15.0f;

	c_fHitMarkerFadeoutSpeed = 3.0f;

	c_fRegenSpeed = 5.0f;  // Regen 5 points per second as standard

	// speeds 
    AI_BASE_MOD_SIDE = 0.75f;
    AI_BASE_MOD_FORWARD = 1.0f;
    AI_BASE_MOD_BACKWARD = 0.75f;
	AI_WALK_SPEED   = 2.0f * 1.0f;
	AI_RUN_SPEED    = 4.5f * 1.0f;
	AI_SPRINT_SPEED = 6.2f * 1.0f;
	UAV_FLY_SPEED_V = 10.0f;
	UAV_FLY_SPEED_H = 5.0f;
	}
	
	DWORD GetCrc32() const {
		return r3dCRC32((const BYTE*)this, sizeof(*this));
	}
};
#pragma pack(pop)

extern	const CGamePlayParams* GPP;
