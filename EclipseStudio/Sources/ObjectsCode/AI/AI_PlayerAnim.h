#pragma once

#include "r3dProtect.h"

enum Playerstate_e
{
	PLAYER_INVALID = -1,

	PLAYER_IDLE = 0,
	PLAYER_IDLEAIM,

	PLAYER_MOVE_CROUCH,
	PLAYER_MOVE_CROUCH_AIM,
	PLAYER_MOVE_WALK_AIM,
	PLAYER_MOVE_RUN,
	PLAYER_MOVE_SPRINT,

	PLAYER_DIE,

	PLAYER_NUM_STATES,
};

// class holding shared animation data between all players
class CUberData
{
  public:
	enum
	{
		INVALID_BONE_ID = -1 
	};

	// animation pool
	r3dAnimPool	animPool_;

	// base binding skeleton for animations
	const r3dSkeleton* bindSkeleton_;

	// possible animation variations per each weapon
	enum
	{
		AIDX_UIIdle,
		AIDX_IdleLower,
		AIDX_StandLower,
		// from here all animations is upper body
		AIDX_IdleUpper,
		AIDX_StandUpper,
		AIDX_CrouchBlend,
		AIDX_CrouchAim,
		AIDX_WalkBlend,
		AIDX_WalkAim,
		AIDX_RunBlend,
		AIDX_SprintBlend,
		AIDX_ReloadWalk,
		AIDX_ReloadIdle,
		AIDX_ReloadCrouch,
		AIDX_ShootWalk,
		AIDX_ShootAim,
		AIDX_ShootCrouch,
		AIDX_COUNT,
	};
	std::string	blendStartBones_[AIDX_COUNT];
	void		LoadUpperBlendStartBones();

	// lower walking animation directions
	enum EAnimDir
	{
		ANIMDIR_Stand,
		ANIMDIR_Str,
		ANIMDIR_StrLeft,
		ANIMDIR_StrRight,
		ANIMDIR_Left,
		ANIMDIR_Right,
		ANIMDIR_Back,
		ANIMDIR_BackLeft,
		ANIMDIR_BackRight,
		ANIMDIR_COUNT,
	};
static	int		GetMoveDirFromAcceleration(const r3dPoint3D& accel);

	struct animIndices_s
	{
	  // lower portions of animations
	  int		crouch[ANIMDIR_COUNT];
	  int		walk[ANIMDIR_COUNT];
	  int		run[ANIMDIR_COUNT];
	  int		sprint[ANIMDIR_COUNT];
	  
	  // misc ones
	  int		turnins[5];
	  int		grenades_tps[20];
	  int		grenades_fps[20];
	  int		bombs_tps[20];
	  int		bombs_fps[20];
	  int		jumps[20];
	  int		deaths[20];
	  int		UI_IdleNoWeapon;
	};
	animIndices_s	aid_;
	int		GetGrenadeAnimId(bool IsFPS, int PlayerState, int GrenadeState);
	int		GetJumpAnimId(int PlayerState, int JumpState);
	
	// default weapon animation indices
	int		wpn1[AIDX_COUNT];

  private:
	friend class obj_AI_Player;
	friend class obj_Zombie;
	friend void AI_Player_LoadStuff();
	friend void AI_Player_FreeStuff();
	CUberData();
	~CUberData();

	void		LoadSkeleton();

	void		LoadAnimations();
	void		 LoadLowerAnimations();
	void		 LoadWeaponAnim(int (&wid)[AIDX_COUNT], int (&wid_fps)[AIDX_COUNT], const char* names[AIDX_COUNT]);
	void		 LoadGrenadeAnim();
	void		 LoadJumpAnim();
	void		 LoadDeathAnim();

	friend class Weapon; // for being able to load weapon specific AIM animations in FPS mode
	int		AddAnimation(const char* name, const char* anim_fname = NULL);
	void		AddAnimationWithFPS(const char* name, int& aid, int& fps_aid);
	
	int		TryToAddAnimation(const char* name, const char* anim_fname = NULL);

	void		LoadWeaponTable();
};

// class for holding and drawing eqipment
class Gear;
struct WeaponConfig;
class Weapon;

enum ESlot
{
	SLOT_Body = 0,
	SLOT_Armor,
	SLOT_Head1,
	SLOT_Head2,
	SLOT_Head3,
	SLOT_Weapon,
	SLOT_WeaponBackRight,
	SLOT_WeaponBackLeft,
	SLOT_WeaponBackLeftRPG, // backleft and backleftRPG are on the same side, so they are mutually exclusive
	SLOT_WeaponSide,
	SLOT_Max,
};

class CUberEquip
{
public:
	obj_AI_Player* player;
	bool	isFirstPerson;
	// equipped things
	class slot_s
	{
	public:
		// only one of those should be non null
		Weapon*		wpn;	// NOT owning pointer, obj_Player own it.
		Gear*		gear;	// owning pointer to gear

		slot_s() { 
			gear = NULL;
			wpn = NULL;
		}
	};
	slot_s		slots_[SLOT_Max];

	enum DrawType
	{
		DT_DEFERRED,
		DT_SHADOWS,
		DT_AURA,
		DT_DEPTH
	};
	
private:
	void		DrawSlot(ESlot slotId, const D3DXMATRIX& world, DrawType dt, bool skin, bool draw_firstperson, const r3dSkeleton* wpnSkeleton);
	void		DrawSlotMesh(r3dMesh* mesh, const D3DXMATRIX& world, DrawType dt, bool skin);

public:
	CUberEquip(obj_AI_Player* plr);
	~CUberEquip();

	void		SetSlot(ESlot slotId, Gear* gear);
	void		SetSlot(ESlot slotId, Weapon* wpn);

	D3DXMATRIX	getWeaponBone(const r3dSkeleton* skel, const D3DXMATRIX& offset);
	r3dPoint3D	getBonePos(int BoneID, const r3dSkeleton* skel, const D3DXMATRIX& offset);
	r3dMesh*	getSlotMesh(ESlot slotId);


	void		Draw(const r3dSkeleton* skel, const D3DXMATRIX& CharMat, bool draw_weapon, DrawType dt, bool first_person);

	void		ResetSlots();

	int			IsLoaded() ;
};


class CUberAnim
{
  public:
	CUberData*	data_;
	obj_AI_Player* player;
	r3dAnimation	anim;
	
	bool		IsFPSMode();

	enum
	{
		INVALID_TRACK_ID = -1
	};

	int		reloadAnimTrackID;
	int		recoilAnimTrackID;
	int		turnInPlaceTrackID;
	int		grenadePinPullTrackID;
	int		grenadeThrowTrackID;
	int		shootAnimTrackID;

	int		jumpTrackID;
	int		jumpState;
	int		jumpPlayerState; // state when player started to jump (keep to avoid starting jump idle then switch to run)
	bool		jumpWeInAir;     // flag that we finally in air (started to jump upward)
	float		jumpAirTime;
	int		jumpMoveTrackID; // track id of underlying lower body anim

	int		bombPlantingTrackID;

	int		AnimPlayerState;
	int		AnimMoveDir;
	
	// right now we have same speed for all-direction movement, so only one speed per state
	float		AnimSpeedStates[PLAYER_NUM_STATES];
	float		AnimSpeedRunFwd;
	void		FillAnimStatesSpeed();
	
	// grenade throw frames for each anim
	float		GetGrenadeLaunchFrame();
	
	// current weapon
	const Weapon* CurrentWeapon;
	
	// special flag for UI idles...
	int		IsInUI;

	void		SwitchToState(int PlayerState, int MoveDir);
	
  public:
	CUberAnim(obj_AI_Player* in_player, CUberData* in_data);
	~CUberAnim();

	int		GetBoneID(const char* Name) const;

	void		SyncAnimation(int PlayerState, int MoveDir, bool force, const Weapon* weap);
	void		StartGrenadePinPullAnimation();
	void		StartGrenadeThrowAnimation();
	void		StopGrenadeAnimations();
	bool		IsGrenadeLaunched();
	int		GetGrenadeAnimState();
	void		StartBombPlantingAnimation(int bombState); // bombState: 0-start, 1-loop, 2-end, 3-stop everything
	void		StartTurnInPlaceAnim();
	void		StopTurnInPlaceAnim();
	void		UpdateTurnInPlaceAnim();

	bool		scaleReloadAnimTime;
	void		StartReloadAnim();
	void		StopReloadAnim();

	void		StartShootAnim();
	void		StopShootAnim();
	void		updateShootAnim(bool disable_loop);

	void		StartRecoilAnim();
	
	float		jumpAnimSpeed;
	float		jumpStartTimeByState[2];
	float		jumpStartTime;
	void		StartJump();
	void		UpdateJump(bool bOnGround);
	
	void		StartDeathAnim();

	bool		IsGrenadePinPullActive();
	bool		IsGrenadePinPullFinished();
};
