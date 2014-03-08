#pragma once

#include "r3dProtect.h"
#include "GameCommon.h"
#include "GameCode/UserProfile.h"
#include "multiplayer/P2PMessages.h"
#include "multiplayer/NetCellMover.h"
#include "ObjectsCode/AI/AI_PlayerAnim.H"
#include "../../../../src/GameEngine/CamouflageDataManager.h"
#include "APIScaleformGfx.h"
#include "AI_PlayerSenses.h"

class Weapon;
class Gear;
class CUberEquip;
class CUberAnim;
class obj_Vehicle;

class obj_AI_Player : public GameObject
{
public:
	DECLARE_CLASS(obj_AI_Player, GameObject)

	obj_AI_Player();

	enum AuraType
	{
		AT_NONE,
		AT_BERSERK,
		AT_HEALING,
		AT_MORHINE,
		AT_SPAWNPROTECTION,
		AT_COUNT
	} m_AuraType ;

	float	m_BerserkerTime;
	Scaleform::GFx::Value m_CommRoseIcon;
	Scaleform::GFx::Value m_MarkedTargetIcon;
	Scaleform::GFx::Value m_PingTeamIcon;
	Scaleform::GFx::Value m_WoundedTeamIcon;
	Scaleform::GFx::Value m_SpawnProtectIcon;
	Scaleform::GFx::Value m_MedicTeamIcon;
	Scaleform::GFx::Value m_TagIcon;
	char				m_MinimapTagIconName[64]; // temp, to be optimized soon
	bool				m_onMinimap; // temp
	float	m_commRoseTimer;
	float	m_SpawnProtectedUntil;
	float   m_pingIconTimer;
	float	m_bombDropCooldown;
	float	m_targetIconTime;
	float	m_localTargetIconTime; // we need this to know if you can retag the player. 
#define MAX_UAV_TARGETS 4
	gobjid_t m_localPlayer_lastUAVTarget[MAX_UAV_TARGETS];

	int		m_Loaded ;

	float   CurrentSpread;

	bool	isPremiumAccount;
	r3dSec_string<64, 0xF7>	m_EncryptedUserName; // utf8 string so use it carefully. Encrypted to make life more difficult for haxorz :)
	// encrypted user name
	void		GetUserName(char* out) const {
		m_EncryptedUserName.get(out);
	}
	void  GetUserNameAndClanTag(char* out) const {
		if(ClanID != 0)
		{
			char tmpUsername[64];
			m_EncryptedUserName.get(tmpUsername);
			const char* tagColor = "#000000";
			switch(ClanTagColor)
			{
			case 1: tagColor = "#aa0000"; break;
			case 2: tagColor = "#a6780c"; break;
			case 3: tagColor = "#10a49e"; break;
			case 4: tagColor = "#20a414"; break;
			case 5: tagColor = "#104ea4"; break;
			case 6: tagColor = "#9610a4"; break;
			case 7: tagColor = "#444444"; break;
			case 8: tagColor = "#a4a4a4"; break;
			default:
				break;
			}
			sprintf(out, "<font color='%s'>[%s]</font>%s", tagColor, ClanTag, tmpUsername);
			return;
		}
		else
			return GetUserName(out);
	}
	int		ClanID;
	int		ClanTagColor;
	char		ClanTag[5*2];

	int		LoadoutSlot;
	wiLoadoutSlot	CurLoadout;
	wiWeaponAttachments CurWeaponAttachments;
	float		m_GearWeight;

    // boosts
	bool		m_haveBoostStamina;

	r3dSec_type<int, 0x21C3DFCA> TeamID;			// Virtual team id used for markings, 0 - blue, 1-red

	// minimap
	bool	m_visibleOnMinimap;

	bool	m_enableRendering;

	// siege
	float	m_siegeArmingTimer;

	bool shownWeaponPickup;
#if ENABLE_ZOMBIES
	PlayerLifeProps lifeProperties;
#endif // ENABLE_ZOMBIES

	void setHasSabotageBomb(bool set);
	bool hasSabotageBomb() const { return m_hasBomb; }

private:
	bool	m_ItemSimulateLeftClick;
	bool	m_hasBomb;
	float	m_fVisibleTimer; // timer for how long player is visible on minimap
	/**	Memorize bounding box, when switch to ragdoll. We will restore it after switching back. */
	r3dBoundBox m_OrigBBox;

public:
	void	setMinimapVisibleTimer(float time, bool force);

	// color correction texture, only for local player
	static r3dTexture* s_damageHitColorCorrectionTex;
	static r3dTexture* s_laserPointerTex;
	static r3dTexture* s_flashlightFlashTex;
	static r3dTexture* s_laserFlashTex;
	static r3dTexture* s_lightHaloTex;
	static float s_damageHitColorCorrectionVal;

	// for UAV view
	static r3dTexture* s_UAVFriendlyBlinkTex;

	uint32_t		m_needToDrawRiotShieldPlacement; // itemID of what shield to draw
	r3dMesh*		m_RiotShieldMeshPlacement; // mesh to render when you are placing shield, not sure how to implement that better rather than having pointer to mesh directly
	r3dMesh*		m_RiotPersonalShieldMeshPlacement;
	bool			m_spawnedRiotShield;

	uint32_t		m_needToDrawAutoTurretPlacement; // itemID of turret
	r3dMesh*		m_AutoTurretMeshPlacement; 
	bool			m_spawnedAutoTurret;

	uint32_t		m_needToDrawRespawnBeaconPlacement; // itemID of turret
	r3dMesh*		m_RespawnBeaconMeshPlacement; 

	// shared for all placeable items
	PrecalculatedMeshVSConsts	m_ConsumableItemPlacementMeshVSConsts;
	bool			m_canPlaceConsumableItem;


	class r3dPhysSkeleton* m_PhysSkeleton;
	bool m_disablePhysSkeleton; // for character in frontend and respawn

	// anim
	CUberEquip*	uberEquip_;
	CUberAnim*	uberAnim_;
	r3dSkeleton*	GetSkeleton();	// access to r3dSkeleton inside uberAnim
	r3dAnimation&	GetAnimation();	// access to r3dAnimation inside uberAnim
	int		PlayerState;	// player state as Playerstate_e
	int		PlayerMoveDir;	// movement direction as CUberData::EAnimDir

	void UpdateLoadoutSlot(const struct wiLoadoutSlot& slot, const struct wiWeaponAttachments& attms, bool canBeWithoutWeapon = false); // updates meshes from loadout slot
	void   UpdateLoadoutWeaponAttachments(const struct wiWeaponAttachments& attms);

	void updateWeaponUI();

	float	grenadeReadyTime;

	void		SyncAnimation(bool force = false);
	void		UpdateAnimations(const D3DXMATRIX& CharDrawMatrix);

	r3dPoint3D	GetHeadPosition();

	Weapon*		m_Weapons[NUM_WEAPONS_ON_PLAYER]; // 0,1,2 - rifles. 3-item, 4-item, 5-item, 6-item
	int		m_SelectedWeapon;
	int		m_PrevSelectedWeapon;
	int		m_QuickThrowGrenade;
	bool		hasScopeMode() const;
	bool		hasScopeMode_hidePlayerModel() const;
	bool		m_isPressedFireTrigger;
	bool		m_isAiming;
	bool		m_isChangedAiming1;  // not resetting event about aiming mode change. used in Weapon::Update
	bool		m_isInScope;
	bool		m_isFinishedAiming; // this one will make sure that player is aiming and that he finished playing aim animation, so no insta aim by clicking fire and aim at the same time

	bool		ChangeWeaponByIndex(int index);
	void		UpdateCharWeaponMeshes();

	static void AddBloodOnGround(const r3dPoint3D& posOfShot); // will cast a ray down from posOfShot
	float		m_BloodTimer;
	
	// hit marker
	float		m_HitMarkerFadeout;
	r3dTexture*	m_HitMarkerTex; // only for local player


	// UAV
	bool		uavViewActive_;
	int		uavRequested_;
	gobjid_t	uavId_;
	void		ProcessSpawnUAV();
	bool		 GetUAVSpawnPos(r3dPoint3D* pos);
	void		ToggleUAVView(bool force_disable=false);

	bool		laserViewActive_;
	void		ToggleLaserView(bool force_disable=false);

	// local stats for scoreboard
	struct stats_s
	{
		int		Score;
		int		GD;
		int		Kills;
		int		Deaths;

		// for kill tag (passed from server - total player stats)
		int		TotalScore;
		int		Wins;
		int		Loss;

		stats_s() 
		{
			Score = 0;
			GD = 0;
			Kills  = 0;
			Deaths = 0;
			TotalScore = 0;
			Wins = 0;
			Loss = 0;
		}
	};
	stats_s		GameStats;
	int			numLootBoxesPickedup;
	int			numBombsPlanted;
	int			numBombsDiffused;
	int			numSaboRoundsWon;
	
	// queue of local player added rewards
	std::vector<PKT_S2C_AddScore_s> rewards_;
	float		lastRewardShowTime_;
	void		ProcessRewardQueue();

	float		bodyAdjust_x;		// rotation
	float		bodyAdjust_y[2];	// bend [cur, target]

	int		boneId_Bip01_Spine1;
	int		boneId_Bip01_Spine2;
	int		boneId_Bip01_Neck;
	int		boneId_Bip01_LUpperArm;
	int		boneId_Bip01_RUpperArm;
	int		boneId_Bip01_Head;

	r3dSec_type<float, 0x1515CD3A>	m_Health;

	int		boneId_Bip01_R_Hand;
	int		boneId_Bip01_L_Hand;

	// for adrenaline and epinephrine shots
	float		m_SpeedBoost;
	float		m_SpeedBoostTime;

	float		m_MorhineShotTime;

	float		m_BandagesEffectTime;

	// var where we will keep velocity, regardless of physx
	// this is needed, because animation and network moving player code
	// depends on GetVelocity()
	// and velocity doesn't do a thing in ControllerPhysObj
	r3dPoint3D	plr_local_Velocity;	
	virtual	void 			SetVelocity(const r3dPoint3D& vel) { plr_local_Velocity = vel; }
	virtual	const r3dPoint3D&	GetVelocity() const { return plr_local_Velocity; }
	float	plr_local_moving_speed;

	// info about player
	float		Height;
	float		getPlayerHeightForCamera() const;

	float		getMaxHealth(); // return max possible health (affected by skills)

	// spring energy vars
	r3dSec_type<float, 0x3786FA41> m_Energy;
	r3dSec_type<float, 0x56DDB60C> m_EnergyPenaltyTime;
	float		getMaxStamina();

	// sniper & breath vars
	//r3dSec_type<float, 0xA88B28F5> m_BreathPenaltyTime;
	r3dVector SniperViewMod; // to simulate rifle movement when trying to snipe
	r3dSec_type<float, 0x061F2DC8> m_Breath;
	float		getMaxBreath();
	bool		m_isHoldingBreath;

	float		getMinimapDetectionRadius();
	float		getMinimapDetectionFireRadius();
	float		lastDamageRcvd;
	float		lastTimeHit;
	float		lastTimeHit_OldHealth;
	int		lastTimeHitBone;
	r3dPoint3D	lastTimeHitForce;
	void		ApplyDamage(const r3dPoint3D& dmgPos, float damage, GameObject* fromObj, int bodyBone, int dmgType);
	float		BloodEffect; // timer
	int		bDead;
	float		TimeOfDeath; // for animations after dying
	STORE_CATEGORIES DeathDamageSource;
	r3dPoint3D	PosOfDeath;
	float		TimeOfLastRespawn;
	int		Dead_KillerID; // killer ID of a person who killed you
	bool		DisableKillerView; // if user clicks on a mouse, show respawn right away
	bool		bCrouch;

	// jump stuff
	void		DetectIfOnGround();
	bool		bOnGround; // true if player is touching any geometry
	void		StartJump();
	bool		IsJumpActive();
	float		JumpVelocity;
	float		JumpStartTime;	// time when we actually need to apply JumpVelocity
	float		StartFallingHeight;
	float		StartFallingTime;

	r3dVector RecoilViewMod; // adds recoil to ViewAngle
	float		m_LastKillTime;
	r3dVector RecoilViewModTarget;
	r3dVector RecoilViewModTarget2; // used for when you are being shot

	r3dVector ViewAngle;		// mouse controlled view angle, in degrees. 
	int		m_KillStreak; // how many kills within certain time
	r3dSec_type<float,		0x5Fc4c6Aa>	RecoilCooldown;
	r3dPoint3D RealAcceleration; // actual player speed, after weight mod, jump, gravity, etc
	r3dPoint3D InputAcceleration; // that is the desired accel based on what buttons player pressed
	float m_RemainingSecondWindTime;  // for the second wind Ability

	r3dVector m_vVision;		// current vector of vision view

	D3DXMATRIX 	DrawRotMatrix;	// character rotation matrix
	D3DXMATRIX 	DrawFullMatrix;

	D3DXMATRIX	MoveMatrix;
	r3dVector	GetvForw() const { return r3dVector(MoveMatrix._31, MoveMatrix._32, MoveMatrix._33); }
	r3dVector	GetvRight() const { return r3dVector(MoveMatrix._11, MoveMatrix._12, MoveMatrix._13); }
	r3dVector	GetvRightDraw() const { return r3dVector(DrawRotMatrix._11, DrawRotMatrix._12, DrawRotMatrix._13); }

	float		m_fPlayerRotation;
	float		m_fPlayerRotationTarget;
	void		UpdateRotation();
	void		 UpdateUpperBodyAngLegs();
	void		 UpdateRotationToTarget(float fY, float fTurnSpeed);

	CNetCellMover	netMover;

	// set player position and reset cell base, so it'll be updated on next movement
	void TeleportPlayer(const r3dPoint3D& pos, const char* reason)
	{
		//r3dOutToLog("... %p: TeleportPlayer %s\n", this, reason);
		SetPosition(pos);
		netMover.Teleport(pos);
	}

	/**	Turn on or off physics simulation for character physics (basically remove/add actors from scene). */
	void TogglePhysicsSimulation(bool on);
	
	
	float		afterRespawnTimer;
	float		lastWeapDataRep;	// time when weapon data report was sent
	float		lastTimeWeaponSwitch;	// to prevent switching weapon and keep firing

	r3dPoint3D		viewTargetPos;
	r3dPoint3D		viewTargetNorm;


	void		UpdateTargetting();
	void		PlayerFired(int hitId, const r3dPoint3D& pos);
	r3dPoint3D	GetMuzzlerPosition();

	void		DoDeath(int networdIDKiller, bool forced_by_server, STORE_CATEGORIES damageSource); // not passing the actual pointer, to make sure that we will not later access dead pointer, in case if killer left the game
	void		DoRespawn(int newTeamId, int slotNum, const wiLoadoutSlot& lslot, const wiWeaponAttachments& attms, const r3dPoint3D& pos, float dir, float spawnProtection);
	void		Undie();
	void		DoRagdoll(bool toRagdoll, int boneId = 0, const r3dPoint3D* impulse = NULL);

	void		DrawLabel();
	void		DrawReticle();
	float		m_ReticleTargetScale;
	float		m_ReticleCurrentScale;
public:

	virtual	BOOL		Load(const char *name);
	virtual	BOOL		OnCreate();			// ObjMan: called before objman take control of this object
	virtual	BOOL		OnDestroy();			// ObjMan: just before destructor called

	//virtual	void 		SetPosition(const r3dPoint3D& pos);

	virtual	BOOL		Update();			// ObjMan: tick update

	bool isWeaponGrenadeOutofAmmo( int weaponIndex );

	void				UpdateLocalPlayerMovement();
	void				UpdateLoaded() ;

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
	void			 OnNetPacket(const PKT_S2C_MoveTeleport_s& n);
	void			 OnNetPacket(const PKT_C2C_MoveSetCell_s& n);
	void			 OnNetPacket(const PKT_C2C_MoveRel_s& n);
	void			 OnNetPacket(const PKT_C2C_PlayerJump_s& n);
	void			 OnNetPacket(const PKT_S2C_AddScore_s& n);

	virtual void		UpdateTransform();

	void		stopFootstepsSound();
	void		SetTransparentCamouflage(float alpha);

	void		CheckFireWeapon();
	bool		IsPlantingMine();
	float		GetGrenadeLaunchDelay();

	void		OnGameEnded();
	BYTE		getBodyNameFromBoneName(const char* boneName);

#if VEHICLES_ENABLED

	enum VehicleViews{
		VehicleView_None,
		VehicleView_Driver,
		VehicleView_Passenger,
	};

	VehicleViews vehicleViewActive_;
	

	obj_Vehicle* canEnterVehicle();
	void enterVehicle( obj_Vehicle* target_Vehicle );
	bool isDriving();
	void exitVehicle();
	bool isInVehicle();
	bool TestExitPosition( r3dVector &exitPosition, bool ignoreGround = false );

#endif 


private:
	void		executeWeaponFireLocalPlayer(const r3dPoint3D& from, float recoil, float spread, float recoilCoolDownMod, float holdingDelay, bool executeWeaponFireCode, bool checkAgainsMuzzler, int damagePercFromPiercing);

	/**	Footsteps sound. */
	float		footsteps_timeSinceLastMissingHitResult;
	void		*footStepsSnd;
	struct footStepEntry_s {
	  int		id;
	  void*		snd;
	  footStepEntry_s() : id(-1), snd(0) {}
	  footStepEntry_s(int i, void* s):id(i), snd(s) {}
	};
	r3dTL::TStaticArray<footStepEntry_s, 128> footStepsArr;
	int		currentFootStepsSoundID;
	void		UpdateFootsteps();
public:
	CamoTimeLine camoTimeLine;
};

struct TCamoSettings
{
	TCamoSettings();

	float texScale ;
	float distortScale ;
	float animSpeed ;

	float reflectionAmmount ;

	r3dColor color0 ;
	r3dColor color1 ;

} extern gCamoSettings ;

void InitializePhysSkeletonCache( int count, float progressStart, float progressEnd ) ;
void DestroyPhysSkeletonCache() ;

// for use when processing a bullet hit by obj_bullet or obj_AI_player::executeWeaponFireLocalPlayer
bool ProcessBulletHit( int &damageFromPiercable, GameObject* owner, const r3dPoint3D &hitPoint, const r3dPoint3D &hitNormal, GameObject* shootTarget, const r3dMaterial* shootMaterial, const char* hitActorName, const WeaponConfig *weaponInfo);