#ifndef __WEAPON_CONFIG_H__
#define __WEAPON_CONFIG_H__

#include "../../GameCode/UserProfile.h"
#include "r3dProtect.h"

enum WeaponAnimTypeEnum
{
	WPN_ANIM_ASSAULT,
	WPN_ANIM_PISTOL,
	WPN_ANIM_GRENADE,
	WPN_ANIM_RPG,
	WPN_ANIM_SMG,
	WPN_ANIM_MINE,
	WPN_ANIM_MELEE,

	WPN_ANIM_COUNT
};

enum WeaponFiremodeEnum
{
	WPN_FRM_SINGLE=1<<0,
	WPN_FRM_TRIPLE=1<<1,
	WPN_FRM_AUTO=1<<2,
};

#include "../../../ServerNetPackets/NetPacketsWeaponInfo.h"

extern const char* WeaponAttachmentBoneNames[WPN_ATTM_MAX];

struct WeaponAttachmentConfig
{
private:
	mutable r3dMesh* m_Model;
	mutable r3dMesh* m_Model_AIM; // second model for when you are aiming. enabled only for scopes right now
public:
	// config
	uint32_t m_itemID;
	WeaponAttachmentTypeEnum m_type;
	int m_specID;

	char* m_MuzzleParticle;
	char* m_Description;
	wchar_t* m_DescriptionW;

	char* m_StoreIcon;
	char* m_StoreName;
	wchar_t* m_StoreNameW;

	char* m_ModelPath;

	char* m_ScopeAnimPath;

	r3dSec_type<int, 0xFA5F4CBC> m_LevelRequired;

	// mods
	float	m_Damage;
	float	m_Range;
	int		m_Firerate;
	float	m_Recoil;
	float	m_Spread;
	int		m_Clipsize;
	const struct ScopeConfig* m_scopeConfig;
	const struct ScopeConfig* m_scopeConfigTPS; // spec.config for when you are switching to TPS mode
	float	m_scopeZoom; // 0..1; 0 - no zoom. 1 - maximum zoom

	// new weapon sounds IDs
	int		m_sndFireID_single;
	int		m_sndFireID_auto;
	int		m_sndFireID_single_player; // for local player
	int		m_sndFireID_auto_player; // for local player

private:	
	// make copy constructor and assignment operator inaccessible
	WeaponAttachmentConfig(const WeaponAttachmentConfig& rhs);
	WeaponAttachmentConfig& operator=(const WeaponAttachmentConfig& rhs);

public:
	WeaponAttachmentConfig(uint32_t id)
	{
		m_itemID = id;
		m_type = WPN_ATTM_INVALID;
		m_specID = 0;
		m_Model = NULL;
		m_Model_AIM = NULL;
		m_MuzzleParticle = NULL;
		m_StoreIcon = NULL;
		m_ModelPath = NULL;
		m_StoreName = NULL;
		m_Description = NULL;
		m_StoreNameW = NULL;
		m_DescriptionW = NULL;

		m_ScopeAnimPath = NULL;

		m_Damage = 0.0f;
		m_Range = 0.0f;
		m_Firerate = 0;
		m_Recoil = 0.0f;
		m_Spread = 0.0f;
		m_Clipsize = 0;
		m_scopeConfig = NULL;
		m_scopeConfigTPS = NULL;
		m_scopeZoom= 1.0f; 

		m_sndFireID_single = -1;
		m_sndFireID_auto   = -1;
		m_sndFireID_single_player = -1;
		m_sndFireID_auto_player = -1;
	}
	~WeaponAttachmentConfig() 
	{
		resetMesh();
		free(m_MuzzleParticle);
		free(m_StoreIcon);
		free(m_ModelPath);
		free(m_StoreName);
		free(m_Description);
		free(m_StoreNameW);
		free(m_DescriptionW);
		free(m_ScopeAnimPath);
	}
	bool loadBaseFromXml(pugi::xml_node& xmlAttachment);

	r3dMesh* getMesh( bool allow_async_loading, bool aim_model ) const;

	int getMeshRefs() const ;
	int getAimMeshRefs() const ;

	// called when unloading level
	void resetMesh() { m_Model = 0; m_Model_AIM = 0; }
};

struct WeaponConfig
{
private:
	mutable r3dMesh* m_Model;
	mutable r3dMesh* m_Model_FPS;
	mutable int	m_ModelRefCount ;

public:

	mutable r3dSkeleton* m_Model_FPS_Skeleton;
	mutable r3dAnimPool*  m_AnimPool_FPS;
	
	// PTUMIK: if adding new skill based items, make sure to add them to DB FN_VALIDATE_LOADOUT proc and also to CUserProfile::isValidInventoryItem()
	enum EUsableItemIDs
	{
		ITEMID_LLDR    = 101253,
		ITEMID_Cypher2 = 101254,
		ITEMID_MedKit  = 101256, // medkit, skill based. non consumable
		ITEMID_Bandages  = 101261, // consumable
		ITEMID_Bandages2 = 101262, // consumable
		ITEMID_MotionSensor = 101257, // skill based, non consumable
		ITEMID_MotionSensorConsumable = 101264, // consumable version
		ITEMID_RiotShield = 101260, // skill based
		ITEMID_RiotShieldConsumable = 101266, // consumable version
		ITEMID_AdrenalineShot = 101279, // skill based
		ITEMID_EpinephrineShot = 101280, // consumable version
		ITEMID_MorphineShot		= 101281, // skill based
		ITEMID_MethylmorphineShot=101282, // consumable version
		ITEMID_AutoTurret	=101259, // skill based
		ITEMID_AutoTurretCons=101263, // consumable version
		ITEMID_RespawnBeacon=101258, // skill based
		ITEMID_RespawnBeaconCons=101265, // consumable version
	};

	// config
	uint32_t m_itemID;
	STORE_CATEGORIES category;
	char* m_MuzzleParticle;
	char* m_Description;
	wchar_t* m_DescriptionW;

	char* FNAME;

	char* m_StoreIcon;
	char* m_StoreName;
	r3dSec_type<int, 0xCF5488FD> m_LevelRequired;
	wchar_t* m_StoreNameW;
	int m_IsUpgradeable;

	char* m_ModelPath;
	char* m_ModelPath_1st;
	char* m_AnimPrefix; // used for FPS mode, for grips


	class Ammo*	m_PrimaryAmmo;

	float m_AmmoMass;
	float m_AmmoSpeed;
	float m_AmmoDamage;
	float m_AmmoDecay;
	float m_AmmoArea;
	float m_AmmoDelay;
	float m_AmmoTimeout;
	bool  m_AmmoImmediate;

	r3dSec_type<uint32_t, 0xFA43D7CC> m_numClips;
	r3dSec_type<uint32_t, 0xAF8CFA32> m_clipSize;
	float m_reloadActiveTick; // when active reload becomes available. Duration is not longer than 10% of reload time or 0.4sec maximum; for grenades used as a time when grenade should be launched from a hand
	r3dSec_type<float, 0xAD56D3C1> m_reloadTime;
	r3dSec_type<float, 0xF2454A6C> m_spread;  // spread set as diameter at 50meter range
	r3dSec_type<float, 0x1327D2F9> m_fireDelay;
	r3dSec_type<float, 0xA246DAFC> m_recoil;

	WeaponAnimTypeEnum m_AnimType;
	uint32_t m_fireModeAvailable; // flags
	const struct ScopeConfig* m_scopeConfig;
	float						m_scopeZoom; // 0..1; 0 - no zoom. 1 - maximum zoom

	mutable r3dPoint3D	muzzlerOffset;
	mutable bool		muzzleOffsetDetected;

	r3dPoint3D adjMuzzlerOffset; // used privately, do not use it
	mutable r3dPoint3D shellOffset; // where shells are coming out

	// sound sample ID's
	int		m_sndFireID;
	int		m_sndFireDistantID;
	int		m_sndReloadID;

	// new weapon sounds IDs
	int		m_sndFireID_single;
	int		m_sndFireID_auto;
	int		m_sndFireID_single_player; // for local player
	int		m_sndFireID_auto_player; // for local player
	
	int*		m_animationIds;
	int*		m_animationIds_FPS;
	
	// fps item attachments
	int		IsFPS;
	int		FPSSpecID[WPN_ATTM_MAX];	// m_specID in WeaponAttachmentConfig for each attachment slot
	int		FPSDefaultID[WPN_ATTM_MAX];	// default attachment item ids in each slot

  private:	
	// make copy constructor and assignment operator inaccessible
	WeaponConfig(const WeaponConfig& rhs);
	WeaponConfig& operator=(const WeaponConfig& rhs);

  public:
	WeaponConfig(uint32_t id)
	{
		m_ModelRefCount = 0 ;

		muzzleOffsetDetected = false ;

		m_itemID = id;
		category = storecat_INVALID;
		m_Model = NULL;
		m_Model_FPS = NULL;
		m_Model_FPS_Skeleton = NULL;
		m_AnimPool_FPS = NULL;
		m_MuzzleParticle = NULL;
		FNAME = NULL;
		m_StoreIcon = NULL;
		m_ModelPath = NULL;
		m_ModelPath_1st = NULL;
		m_StoreName = NULL;
		m_Description = NULL;
		m_PrimaryAmmo = NULL;
		m_scopeConfig = NULL;
		m_StoreNameW = NULL;
		m_DescriptionW = NULL;

		m_AnimPrefix = NULL;

		m_LevelRequired = 1;
		m_IsUpgradeable = 0;

		m_AmmoMass			= 0.1f;
		m_AmmoSpeed			= 100.f;
		m_AmmoDamage		= 1.0f;
		m_AmmoDecay			= 0.1f;
		m_AmmoArea			= 0.1f;
		m_AmmoDelay			= 0.f;
		m_AmmoTimeout		= 1.0f;
		m_AmmoImmediate		= true;

		m_numClips			= 3;
		m_clipSize			= 30;
		m_reloadTime		= 1.0f;
		m_reloadActiveTick	= 0.f; // when active reload becomes available. Duration is not longer than 10% of reload time or 0.4sec maximum; for grenades used as a time when grenade should be launched from a hand
		m_fireDelay			= 0.5f;
		m_spread			= 0.01f; 
		m_recoil			= 0.1f;
		m_AnimType			= WPN_ANIM_ASSAULT;
		m_fireModeAvailable	= WPN_FRM_SINGLE; // flags
		m_scopeZoom			= 0; // 0..1; 0 - no zoom. 1 - maximum zoom

		muzzlerOffset		= r3dPoint3D( 0.25f, 0.f, 0.f );
		adjMuzzlerOffset	= muzzlerOffset ; // used privately, do not use it
		shellOffset			= r3dPoint3D( 0, 0, 0 ); // where shells are coming out

		m_sndFireID			= -1 ;
		m_sndFireDistantID	= -1 ;
		m_sndReloadID		= -1 ;

		m_sndFireID_single = -1;
		m_sndFireID_auto   = -1;
		m_sndFireID_single_player = -1;
		m_sndFireID_auto_player = -1;
		
		m_animationIds          = NULL;
		m_animationIds_FPS		= NULL;
		
		IsFPS = 0;
		memset(FPSSpecID, 0, sizeof(FPSSpecID));
		memset(FPSDefaultID, 0, sizeof(FPSDefaultID));
	}
	~WeaponConfig() 
	{
		resetMesh();
		free(m_MuzzleParticle);
		free(m_StoreIcon);
		free(FNAME);
		free(m_ModelPath);
		free(m_ModelPath_1st);
		free(m_StoreName);
		free(m_Description);
		free(m_StoreNameW);
		free(m_DescriptionW);
		free(m_AnimPrefix);
		SAFE_DELETE_ARRAY(m_animationIds);
		SAFE_DELETE_ARRAY(m_animationIds_FPS);
	}
	bool loadBaseFromXml(pugi::xml_node& xmlWeapon);
	
	bool isSameParameters(const WeaponConfig& rhs) const
	{
		#define DO(XX) if(XX != rhs.XX) return false;
		DO(m_AmmoMass);
		DO(m_AmmoSpeed);
		DO(m_AmmoDamage);
		DO(m_AmmoDecay);
		DO(m_AmmoArea);
		DO(m_AmmoDelay);
		DO(m_AmmoTimeout);

		DO(m_numClips);
		DO(m_clipSize);
		DO(m_reloadTime);
		DO(m_reloadActiveTick);
		DO(m_fireDelay);
		DO(m_spread); 
		DO(m_recoil);
		#undef DO
		return true;
	}
	void copyParametersTo(GBWeaponInfo& wi) const
	{
		#define DO(XX) wi.XX = XX
		DO(m_AmmoMass);
		DO(m_AmmoSpeed);
		DO(m_AmmoDamage);
		DO(m_AmmoDecay);
		DO(m_AmmoArea);
		DO(m_AmmoDelay);
		DO(m_AmmoTimeout);

		DO(m_numClips);
		DO(m_clipSize);
		DO(m_reloadTime);
		DO(m_reloadActiveTick);
		DO(m_fireDelay);
		DO(m_spread); 
		DO(m_recoil);
		#undef DO
	}
	void copyParametersFrom(const GBWeaponInfo& wi)
	{
		#define DO(XX) XX = wi.XX
		DO(m_AmmoMass);
		DO(m_AmmoSpeed);
		DO(m_AmmoDamage);
		DO(m_AmmoDecay);
		DO(m_AmmoArea);
		DO(m_AmmoDelay);
		DO(m_AmmoTimeout);

		DO(m_numClips);
		DO(m_clipSize);
		DO(m_reloadTime);
		DO(m_reloadActiveTick);
		DO(m_fireDelay);
		DO(m_spread); 
		DO(m_recoil);
		#undef DO
	}

	DWORD GetClientParametersHash() const
	{
		// hold copy of variables to hash, work with r3dSecType
#pragma pack(push,1)
		struct hash_s 
		{
			uint32_t m_numClips;
			uint32_t m_clipSize;
			float m_reloadTime;
			float m_fireDelay;
			float m_spread;
			float m_recoil;
		};
#pragma pack(pop)

		hash_s h;
		h.m_numClips   = m_numClips;
		h.m_clipSize   = m_clipSize;
		h.m_reloadTime = m_reloadTime;
		h.m_fireDelay  = m_fireDelay;
		h.m_spread     = m_spread;
		h.m_recoil     = m_recoil;
		return r3dCRC32((BYTE*)&h, sizeof(h));
	}

	r3dMesh* getMesh( bool allow_async_loading, bool first_person ) const;

	int getMeshRefs() const ;
	int getConfigMeshRefs() const ;

	// called when unloading level
	void resetMesh() { 
		m_Model = 0; 
		m_Model_FPS = 0; 
		SAFE_DELETE(m_Model_FPS_Skeleton); 
		SAFE_DELETE(m_AnimPool_FPS); 
	}
	void detectMuzzleOffset(bool first_person) const;

	// because mesh can be delay-loaded
	void updateMuzzleOffset(bool first_person) const ;

	// we will detect mines by ItemID for right now
	enum {
	  GRENADE_ANIM_Normal,
	  GRENADE_ANIM_Claymore,
	  GRENADE_ANIM_VS50,
	  GRENADE_ANIM_V69,
	  GRENADE_ANIM_LASTTYPE,
	};
	int getGrenadeAnimType() const;
	bool isMine() const 
	{
		return category == storecat_GRENADES && getGrenadeAnimType() != GRENADE_ANIM_Normal;
	}

	r3dSkeleton* getSkeleton() const { return m_Model_FPS_Skeleton; }

	void aquireMesh( bool allow_async_loading ) const ;
	void releaseMesh() const ;

	R3D_FORCEINLINE bool hasFPSModel() const
	{
		return m_Model_FPS ? true : false ;
	}

	R3D_FORCEINLINE bool isFPSModelSkeletal() const
	{
		return m_Model_FPS->IsSkeletal() ? true : false ;
	}


};

struct GearConfig
{
private:
	mutable r3dMesh* m_Model;
	mutable r3dMesh* m_FirstPersonModel;
	mutable int m_ModelRefCount ;
public:
	// config
	uint32_t m_itemID;
	STORE_CATEGORIES category;

	char* m_Description;
	char* m_StoreIcon;
	char* m_StoreName;
	wchar_t* m_StoreNameW;
	wchar_t* m_DescriptionW;

	char* m_ModelPath;
	char* m_ModelPath_1st; // 1st person model

	float m_Weight;
	r3dSec_type<int, 0xCF358FA5> m_LevelRequired;
	float m_damagePerc;
	float m_damageMax;
	float m_bulkiness;
	float m_inaccuracy;
	float m_stealth;
	int   m_ProtectionLevel;

  private:	
	// make copy constructor and assignment operator inaccessible
	GearConfig(const GearConfig& rhs);
	GearConfig& operator=(const GearConfig& rhs);
	
  public:
	GearConfig(uint32_t id)
	{
		m_ModelRefCount = 0 ;

		m_itemID = id;
		category = storecat_INVALID;

		m_Model = NULL;
		m_FirstPersonModel = NULL;
		m_ModelPath = NULL;
		m_ModelPath_1st = NULL;
		m_StoreIcon = NULL;
		m_StoreName = NULL;
		m_StoreNameW = NULL;
		m_Description = NULL;
		m_DescriptionW = NULL;
	}
	~GearConfig() 
	{
		resetMesh();
		free(m_ModelPath);
		free(m_ModelPath_1st);
		free(m_StoreIcon);
		free(m_StoreName);
		free(m_StoreNameW);
		free(m_Description);
		free(m_DescriptionW);
	}

	void aquireMesh() const ;
	void releaseMesh() const ;

	bool loadBaseFromXml(pugi::xml_node& xmlGear);
	
	bool isSameParameters(const GearConfig& rhs) const
	{
		#define DO(XX) if(XX != rhs.XX) return false;
		DO(m_Weight);
		DO(m_damagePerc);
		DO(m_damageMax);
		DO(m_bulkiness);
		DO(m_inaccuracy);
		DO(m_stealth);
		DO(m_ProtectionLevel);
		#undef DO
		return true;
	}
	void copyParametersTo(GBGearInfo& wi) const
	{
		#define DO(XX) wi.XX = XX
		DO(m_Weight);
		DO(m_damagePerc);
		DO(m_damageMax);
		DO(m_bulkiness);
		DO(m_inaccuracy);
		DO(m_stealth);
		DO(m_ProtectionLevel);
		#undef DO
	}
	void copyParametersFrom(const GBGearInfo& wi)
	{
		#define DO(XX) XX = wi.XX
		DO(m_Weight);
		DO(m_damagePerc);
		DO(m_damageMax);
		DO(m_bulkiness);
		DO(m_inaccuracy);
		DO(m_stealth);
		DO(m_ProtectionLevel);
		#undef DO
	}

	const char* getModelPath() const { return m_ModelPath ; }
	const char* getFirstPersonModelPath() const { return m_ModelPath_1st; }

	r3dMesh* getMesh() const;
	r3dMesh* getFirstPersonMesh() const;

	int getMeshRefs() const ;
	int getConfigMeshRefs() const ;

	// called when unloading level
	void resetMesh() { m_Model = 0; m_FirstPersonModel = 0; }
};

// general items, without models (premium accs, keys, etc)
struct ItemConfig
{
private:
	mutable r3dMesh* m_Model;
public:
	uint32_t m_itemID;
	STORE_CATEGORIES category;

	char* m_Description;
	char* m_StoreIcon;
	char* m_StoreName;
	wchar_t* m_StoreNameW;
	wchar_t* m_DescriptionW;
	r3dSec_type<int, 0xFA5F4CA5> m_LevelRequired;

	char* m_ModelPath;

	ItemConfig(uint32_t id)
	{
		m_Model = NULL;
		m_ModelPath = NULL;
		m_itemID = id;
		category = storecat_INVALID;
		m_StoreIcon = NULL;
		m_StoreName = NULL;
		m_Description = NULL;
		m_StoreNameW = NULL;
		m_DescriptionW = NULL;
	}
	~ItemConfig() 
	{
		resetMesh();
		free(m_ModelPath);
		free(m_StoreIcon);
		free(m_StoreName);
		free(m_Description);
		free(m_StoreNameW);
		free(m_DescriptionW);
	}

	const char* getModelPath() { return m_ModelPath ; }

	r3dMesh* getMesh() const;
	int getMeshRefs() const ;

	// called when unloading level
	void resetMesh() { m_Model = 0; }
};

struct PackageConfig : public ItemConfig
{
	uint32_t m_addGD;
	uint32_t m_addSP;

	uint32_t m_itemID1;
	uint32_t m_itemID1Exp;
	uint32_t m_itemID2;
	uint32_t m_itemID2Exp;
	uint32_t m_itemID3;
	uint32_t m_itemID3Exp;
	uint32_t m_itemID4;
	uint32_t m_itemID4Exp;
	uint32_t m_itemID5;
	uint32_t m_itemID5Exp;
	uint32_t m_itemID6;
	uint32_t m_itemID6Exp;

	PackageConfig(uint32_t id) : ItemConfig(id) {}
};

struct ScopeConfig
{
	char name[32];
	bool		hasScopeMode; // instead of just aiming it will show special scope. also will affect camera position and FOV and mouse sensitivity
	bool		lighting;
	bool		hide_player_model;
	r3dTexture* scope_mask; // can be null
	r3dTexture*	scopeBlur_mask; // can be null
	r3dTexture* scope_reticle; // actual scope/reticule whatever else
	r3dTexture* scope_normal;
	r3dTexture* reticule;

	ScopeConfig(const char* n)
	{
		r3dscpy(name, n);
		scope_mask = 0;
		scopeBlur_mask = 0;
		scope_reticle = 0;
		scope_normal = 0;
		reticule = 0;
		lighting = true;
		hide_player_model = true;
		hasScopeMode = false;
	}
	~ScopeConfig()
	{
		if(scope_mask)
			r3dRenderer->DeleteTexture(scope_mask);
		if(scopeBlur_mask)
			r3dRenderer->DeleteTexture(scopeBlur_mask);
		if(scope_reticle)
			r3dRenderer->DeleteTexture(scope_reticle);
		if(scope_normal)
			r3dRenderer->DeleteTexture(scope_normal);
		if(reticule)
			r3dRenderer->DeleteTexture(reticule);
	}
};


struct AbilityConfig
{
	static const int AB_DoubleUp	= 301061;
	static const int AB_Berserker	= 301062;
	static const int AB_GuardianAngel	= 301063;
	static const int AB_Sniffer	= 301064;
	static const int AB_EyeSpy	= 301065;
	static const int AB_SilentStrike= 301066;
	static const int AB_BigSurprise	= 301067;
	static const int AB_BigGuns	= 301068;
	static const int AB_TheFixer	= 301069;
	static const int AB_FlagHog	= 301070;
	static const int AB_BulkUp	= 301071;
	static const int AB_NinjaKiller	= 301072;
	static const int AB_SecondWind = 301073;
};



enum {
	// achievements listed here are offset by one from their id.  Hopefully. 
	// !!! PTumik: this is an INDEX into GameDB.xml <AchievementDB> !!!
	ACHIEVEMENT_FIRST_KILL = 0,
	ACHIEVEMENT_5KILLS_IN_A_ROW,
	ACHIEVEMENT_10KILLS_IN_A_ROW,
	ACHIEVEMENT_15KILLS_IN_A_ROW,
	ACHIEVEMENT_20KILLS_IN_A_ROW,
	ACHIEVEMENT_30KILLS_IN_A_GAME,
	ACHIEVEMENT_2KILLS_1ROCKET,
	ACHIEVEMENT_GRENADE_KILL_AFTER_DEATH,
	ACHIEVEMENT_LEVEL2,
	ACHIEVEMENT_LEVEL5,
	ACHIEVEMENT_LEVEL10,
	ACHIEVEMENT_LEVEL20,
	ACHIEVEMENT_LEVEL30,
	ACHIEVEMENT_LEVEL40,
	ACHIEVEMENT_LEVEL50,
	ACHIEVEMENT_LEVEL60,
	ACHIEVEMENT_EARNED10000WP,
	ACHIEVEMENT_KILL_WITH_EVERY_WEAPON,
	ACHIEVEMENT_DOMINATE, // 5 kills with out a kill back
	ACHIEVEMENT_KILLDOMINATOR,
	ACHIEVEMENT_SNIPER_KILL_NO_ZOOM,
	ACHIEVEMENT_KILL_10000_PLAYERS,
	ACHIEVEMENT_300_HEADSHOTS, 
	ACHIEVEMENT_KILL_ENEMY_IN_AIR,
	ACHIEVEMENT_KILL_WHILE_IN_AIR,
	ACHIEVEMENT_KILL_SELF_DAMAGED_ENEMY,
	ACHIEVEMENT_DISARM_REWIRE_EXPLOSIVE_DEVICE,
	ACHIEVEMENT_3ENEMIES_ONE_AIRSTRIKE,
	ACHIEVEMENT_3ENEMIES_WITHOUT_MOVING,
	ACHIEVEMENT_3ENEMIES_IN_A_ROW_WITH_SNIPER,
	ACHIEVEMENT_FIRST_PLAYER_TO_START_CAPTURING,

	// modes
	// Sabotage
	ACHIEVEMENT_PLANT_100_BOMBS,
	ACHIEVEMENT_DEFUSE_100_BOMBS,
	ACHIEVEMENT_WIN_MATCH_IN_SABOTAGE,
	ACHIEVEMENT_DETONATE_50_BOMBS, 

	// DeathMatch
	ACHIEVEMENT_DM_LAST_KILL,
	ACHIEVEMENT_DM_WON_MATCH,
	ACHIEVEMENT_DM_KILL_1000_PLAYERS,
	ACHIEVEMENT_DM_KILL_7_ENEMIES_ASSISTED,

	// Conquest
	ACHIEVEMENT_CAPTURE_5000_FLAGS,
	ACHIEVEMENT_DEFEND_5000_FLAGS,
	ACHIEVEMENT_CONQUEST_WIN_MATCH,

	// Not online. 
	FIRST_FRONT_END_ACHIEVEMENT,
	ACHIEVEMENT_FIRST_GC_PURCHASE = FIRST_FRONT_END_ACHIEVEMENT,
	ACHIEVEMENT_PURCHASE_100000_GC,
	ACHIEVEMENT_PURCHASE_500000_GC,
	ACHIEVEMENT_PURCHASE_2_PERMENENT_ITEMS,
	ACHIEVEMENT_PURCHASE_5_PERMENENT_ITEMS,
	ACHIEVEMENT_PURCHASE_10_PERMENENT_ITEMS,
	ACHIEVEMENT_PURCHASE_RENT_PISTOL,
	ACHIEVEMENT_PURCHASE_RENT_SUPPORT,
	ACHIEVEMENT_PURCHASE_RENT_ASSAULT,
	ACHIEVEMENT_PURCHASE_RENT_SHOTGUN,
	ACHIEVEMENT_PURCHASE_RENT_SNIPER,
	ACHIEVEMENT_PURCHASE_RENT_SMG,
	ACHIEVEMENT_PURCHASE_RENT_EACH_TYPE,
	ACHIEVEMENT_PURCHASE_RENT_PURCHASE_RENT_GEAR,
	ACHIEVEMENT_PURCHASE_RENT_PURCHASE_UAV,
	NUM_ACHIEVEMENTS,
};

struct AchievementConfig
{
	int id;
	bool enabled;
	char* name; // ID in lang file
	char* desc; // ID in lang file
	char* hud_icon;
	int value;
	AchievementConfig(int n)
	{
		r3d_assert(n>=1);
		id = n;	
		name = 0;
		desc = 0;
		hud_icon = 0;
		value =0;
		enabled = false;
	}
	~AchievementConfig()
	{
		free(name);
		free(desc);
		free(hud_icon);
	}
};



#endif __WEAPON_CONFIG_H__