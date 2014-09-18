#pragma once

#include "GameCommon.h"
#include "Backend/ServerUserProfile.h"
#include "multiplayer/P2PMessages.h"
#include "multiplayer/NetCellMover.h"

#define MAX_UAV_TARGETS 8

class Gear;
class Weapon;

class obj_ServerPlayer : public GameObject
{
  public:
	DECLARE_CLASS(obj_ServerPlayer, GameObject)

	// info about player
	DWORD		peerId_;
	float		startPlayTime_;
	float		m_PlayerRotation;
	
	char		userName[64];
	int		NextSpawnNode;
	int		TeamID;
	float		m_LastTimeTeamSwitch;
	void		SwitchTeam(int newTeam) {
	  r3d_assert(newTeam < TEAM_MAX_ID);
	  TeamID = newTeam;
	  m_LastTimeTeamSwitch = r3dGetTime();
	}

	// mark this player as in damage area
	float	m_InDamageAreaTime;

 //
 // equipment and loadouts
 //
	enum ESlot
	{
	  SLOT_Body = 0,
	  SLOT_Armor,
	  SLOT_Head1,
	  SLOT_Head2,
	  SLOT_Head3,
	  SLOT_Max,
	};
	Gear*		gears_[SLOT_Max];

	Weapon*		m_WeaponArray[NUM_WEAPONS_ON_PLAYER];
	int		m_SelectedWeapon;
	void		ResetSelectedWeapon() {
	  m_SelectedWeapon = 0;
	  r3d_assert(m_WeaponArray[m_SelectedWeapon]);
	}
	int		GetSelectedWeapon() const {
	  r3d_assert(m_SelectedWeapon >= 0);
	  r3d_assert(m_WeaponArray[m_SelectedWeapon]);
	  return m_SelectedWeapon;
	}
	int getSelectedWeaponItemID() const;

	float		m_DamageReduction_LastTimeHit;
	int			m_DamageReduction_BulletCounter;

	uint32_t	m_RequestedWeaponPickup; // itemID of the weapon
	wiWeaponAttachment m_RequestedWeaponPickupAttms; // attms of the picked up weapon
	int			m_RequestedWeaponPickupNumBullets;
	gobjid_t	m_RequestedWeaponPickUpPrevOwner;
	BYTE		m_RequestWeaponPickupSecretCode;
	bool		m_DisableWeaponDrop;

	float		CalcWeaponDamage(const r3dPoint3D& shootPos);
	void		DoResupply();
	bool		FireWeapon(int wid, bool wasAiming, int executeFire, DWORD targetId, const char* pktName); // should be called only for FIRE event
	float		lastWeapDataRep;	// time when last PKT_C2S_PlayerWeapDataRep_s was received

	int		m_SelectedLoadout;
	int		SetLoadoutSlot(int lslot);
	void		 SetWeaponSlot(int wslot, int* weapId);
	void		 SetGearSlot(int gslot, int* gearId);
	int		GetLoadoutSlot() const {
	  r3d_assert(m_SelectedLoadout >= 0);
	  return m_SelectedLoadout;
	}
	
	const wiLoadoutSlot& GetLoadoutData() const {
	  return profile_.ProfileData.ArmorySlots[m_SelectedLoadout];
	}

	void getAttachmentData(wiWeaponAttachments& attms) const;

	int			FireHitCount; // counts how many FIRE and HIT events received. They should be equal. If not - cheating detected
	float		m_Health;
	float		getMaxHealth();
	int		isDead;
	float		deathTime;
	float		respawnInvulnerableTime; // time after spawn when you are invulnerable
	float		berserkerTime;
	float		LastEnemyKillTime;
	int			Killstreaks;
	gobjid_t	LastKilledBy;
	float		lastRespawnTimeAtBase;
	r3dPoint3D	lastRespawnPosAtBase;
	int			m_NumberAssistedKills;

	int			m_RocketKills; // per rocket rocket kills.
	int			m_AirstrikeKills;
	int			m_CampingKills;
	int			m_SniperKillsWithoutMissing; 
	bool		m_LastSniperShotZoomed;
	int			m_KillsPerPlayer[ MAX_POSSIBLE_PLAYERS ];
	int			inventoryIndexForKills;
	
	struct UAVinfo
	{
		gobjid_t	UAVTargetedBy;
		float		UAVTargetedByTime;
		UAVinfo(): UAVTargetedBy(), UAVTargetedByTime(0.0f) {}
	};
	UAVinfo		listOfUAVHits[MAX_UAV_TARGETS];
	int			numOfUAVHits;

	void		DoDeath();
	void		DoRespawn(const r3dPoint3D& pos, float dir, float spawnProtection);
	
	float		lastTimeHit;
	int		lastHitBodyPart;
	float		ApplyDamage(float damage, GameObject* fromObj, int bodyPart, STORE_CATEGORIES damageSource);
	float		 ReduceDamageByGear(int gslot, float damage);
	
	// some old things
	float		Height;
	
	// battle zone
	float		bzOutTime;

	// resupply crates usage
	float		m_LastTimeResupplied;
	bool		m_CanUseBlackopsCrate;
	bool		m_RequestedResupply;
	
	// precalculated boost vars
	float		boostXPBonus_;
	float		boostWPBonus_;

	// stats
	CServerUserProfile profile_;
	void		SetProfile(const CServerUserProfile& in_profile);
	
	CLoadoutUsageTrack loadoutUsage_[wiUserProfile::MAX_LOADOUT_SLOTS];

	int			numKillWithoutDying;
	BYTE		LevelUpMin; // track how many levels player got after end of round
	BYTE		LevelUpMax; 
	wiStats		RoundStats_;
	wiStatsTracking DetailedReward_;
	wiStatsTracking	AddReward(const wiStatsTracking& rwd);

	// was shot from history
	struct ShotFromHistory
	{
		int networkID;
		float timestamp;
		ShotFromHistory() : networkID(-1), timestamp(0) {} 
	};
	ShotFromHistory m_ShotFromHistory[2]; // track two last players that shot this player
	
	// capturing points tracking
	DWORD		capturingPointId_;	// ID of currently capturing point, 0 if none
	float		capturingPointStart_;
	float		capturingPointCooldown_;
	int		capturingPointStreak_;

	// big surprise ability
	float		m_BigSurpriseTimer;

	// uav
	gobjid_t	uavId_;
	bool		uavRequested_;
	void		UseItem_UAV(const r3dPoint3D& pos, float rotX);
	void		UseItem_MedKit(const r3dPoint3D& pos);
	void		UseItem_MotionSensor(const r3dPoint3D& pos, uint32_t itemID);
	void		UseItem_RespawnBeacon(const r3dPoint3D& pos, float rotX, uint32_t itemID);
	void		UseItem_RiotShield(const r3dPoint3D& pos, float rotX, uint32_t itemID);
	void		UseItem_AutoTurret(const r3dPoint3D& pos, float rotX, uint32_t itemID);

	bool		isTargetDummy_;

	bool		m_HasDamagedSelf;

	// bomb mode only
	bool	m_carryingBomb;

	float		lastChatTime_;
	int		numChatMessages_;
	
	CNetCellMover	netMover;
	float		moveAccumDist;
	float		moveAccumTime;
	float		movePrevTime;
	
	float		lastPlayerAction_;	// used for AFK checks

	float		m_MorphineTime;
	float		m_MorphineEffect;

	// packet sequences, used to skip late packets after logic reset
	// for example teleports, game state reset, etc
	DWORD		myPacketSequence;	// expected packets sequence ID
	DWORD		clientPacketSequence;	// received packets sequence ID
	const char*	packetBarrierReason;
	void		SetLatePacketsBarrier(const char* reason);

private: // disable access to SetPosition directly, use TeleportPlayer
	void		SetPosition(const r3dPoint3D& pos)
	{
		__super::SetPosition(pos);
	}
public:
	void		TeleportPlayer(const r3dPoint3D& pos);
	
	void		OnNetPacket(const PKT_C2C_PacketBarrier_s& n);
	void		OnNetPacket(const PKT_C2C_MoveSetCell_s& n);
	void		OnNetPacket(const PKT_C2C_MoveRel_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerJump_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerSwitchWeapon_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerUseItem_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerFired_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitNothing_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitStatic_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitStaticPierced_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerHitDynamic_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerReadyGrenade_s& n);
	void		OnNetPacket(const PKT_C2S_PlayerCreateMine_s& n);
	void		OnNetPacket(const PKT_C2C_PlayerSwitchMine_s& n);
	
	void		OnNetPacket(const PKT_C2S_PlayerWeapDataRep_s& n);
	void		OnNetPacket(const PKT_C2S_TEST_PlayerSetWeapon_s& n);
	
	void		RelayPacket(const DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered = true);

  public:
	obj_ServerPlayer();
	
virtual	BOOL		Load(const char *name);
virtual	BOOL		OnCreate();			// ObjMan: called before objman take control of this object
virtual	BOOL		OnDestroy();			// ObjMan: just before destructor called

virtual	void		RecalcBoundBox();

virtual	BOOL		Update();			// ObjMan: tick update

virtual	BOOL	 	OnCollide(GameObject *obj, CollisionInfo &trace);

virtual	void		Draw(r3dCamera& Cam, eRenderStageID DrawState);

virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
void				markAchievementComplete( int whichAchievement, bool ignorePractice = false );
void				checkAchievementByValue( int whichAchievement, int value );
void				incrementAchievement( int whichAchievement, int value, bool ignorePractice = false );
void 				RecordAchievements();
};

