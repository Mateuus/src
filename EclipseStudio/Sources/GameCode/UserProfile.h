#pragma once

static const uint32_t MAX_NUM_RANKS = 60;
extern int g_RanksPoints[MAX_NUM_RANKS];

/// STATS TRACKING ///
struct wiStatsTracking
{
// NOTE: you MUST increase P2PNET_VERSION if you change this structure
	int RewardID;

	int GD; // game dollars
	int GP; // game points
	int HP; // XP
	wiStatsTracking& operator+=(const wiStatsTracking& rhs) 
	{ 
		HP+=rhs.HP; 
		GP+=rhs.GP; 
		GD+=rhs.GD;
		return *this;
	}
	wiStatsTracking() { memset(this, 0, sizeof(*this)); }
	wiStatsTracking(int rewardId, int gd, int gp, int hp)
		: RewardID(rewardId), HP(hp), GP(gp), GD(gd) {}
	int getTotalHP() const { return HP;}
};

// Items categories
// if you make any changes, please update STORE_CATEGORIES_NAMES in UserProfile.cpp
enum STORE_CATEGORIES
{
	storecat_INVALID	= 0,
	storecat_Account	= 1,
	storecat_Boost		= 2,
	storecat_MysteryBox	= 3,
	storecat_Items		= 4,
	storecat_Abilities	= 5,
	storecat_Airstrike	= 6,	// special type for artillery strikes, not visible in shop
	storecat_LootBox	= 7,	// same as mystery box, but can not see content and can sell it
	storecat_Package	= 9,	// special type for multiple items package

	storecat_Characters	= 10,
	storecat_Gear		= 11,
	storecat_Heads		= 12,
	storecat_HeadGear	= 13,
	storecat_Camo		= 14,
	storecat_Voice		= 15,
	storecat_Heroes		= 16,
	
	storecat_FPSAttachment  = 19,

	storecat_ASR		= 20,	// Assault Rifles
	storecat_SNP		= 21,	// Sniper rifles
	storecat_SHTG		= 22,	// Shotguns
	storecat_MG		= 23,	// Machine guns
	storecat_SUPPORT	= 24,	// grenade launchers, other heavy items
	storecat_HG		= 25,	// handguns
	storecat_SMG		= 26,	// submachineguns
	storecat_GRENADES	= 27,   // special items like grenades, that goes into item slots
	storecat_UsableItem     = 28,	// usable items, but not a weapon
	storecat_MELEE			=29, // melee items (knifes, etc)
	
	storecat_NUM_ITEMS, // should be the last one!!
};
extern const char* STORE_CATEGORIES_NAMES[storecat_NUM_ITEMS];
bool isWeaponCategory(STORE_CATEGORIES cat);
bool isGearCategory(STORE_CATEGORIES cat);
bool isItemCategory(STORE_CATEGORIES cat);

struct wiStoreItem
{
	uint32_t itemID;
	uint32_t price1day;
	uint32_t price7day;
	uint32_t price30day;
	uint32_t pricePerm;
	
	// gold dollars price
	uint32_t gd_price1day;
	uint32_t gd_price7day;
	uint32_t gd_price30day;
	uint32_t gd_pricePerm;

	bool hasAnyPrice()
	{
		return price1day>0 || price7day>0 || price30day > 0 || pricePerm > 0 || gd_price1day > 0 || gd_price7day > 0 || gd_price30day > 0 || gd_pricePerm > 0;
	}
};

static const uint32_t MAX_NUM_STORE_ITEMS = 10000; // fixed at 10000 for now
extern wiStoreItem g_StoreItems[MAX_NUM_STORE_ITEMS]; 
extern uint32_t g_NumStoreItems;

struct	wiStats
{
// NOTE: you MUST increase P2PNET_VERSION if you change this structure

	int	GamePoints;
	int GameDollars;
	int	HonorPoints;		// Your ranking points
	int	SkillPoints;

	int	Kills;
	int	Deaths;

	int	Headshots;
	int	AssistKills;

	int	ShotsFired;
	int	ShotsHits;

	int	Wins;
	int	Losses;
	int	CaptureNeutralPoints;  // how many control points you've taken
	int	CaptureEnemyPoints;  // how many control points you've taken

	int	TimePlayed;	// time played in seconds
	
	wiStats()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}

	void operator +=(const wiStats& rhs)
	{
		GamePoints += rhs.GamePoints;
		GameDollars += rhs.GameDollars;
		HonorPoints += rhs.HonorPoints;
		SkillPoints += rhs.SkillPoints;
		Kills += rhs.Kills;
		Deaths += rhs.Deaths;
		Headshots += rhs.Headshots;
		AssistKills += rhs.AssistKills;
		ShotsFired += rhs.ShotsFired;
		ShotsHits += rhs.ShotsHits;
		Wins += rhs.Wins;
		Losses += rhs.Losses;
		CaptureNeutralPoints += rhs.CaptureNeutralPoints;
		CaptureEnemyPoints += rhs.CaptureEnemyPoints;
		TimePlayed += rhs.TimePlayed;
	}

	int getRankLevel() const;
};

enum WeaponAttachmentTypeEnum
{
	WPN_ATTM_INVALID=-1,
	WPN_ATTM_MUZZLE=0,
	WPN_ATTM_UPPER_RAIL=1,
	WPN_ATTM_LEFT_RAIL=2,
	WPN_ATTM_BOTTOM_RAIL=3,
	WPN_ATTM_CLIP=4,
	WPN_ATTM_RECEIVER=5, // not visual
	WPN_ATTM_STOCK=6, // not visual
	WPN_ATTM_BARREL=7, // not visual
	WPN_ATTM_PAINT=8,

	WPN_ATTM_MAX
};

struct wiWeaponAttachment
{
	uint32_t attachments[WPN_ATTM_MAX];
	wiWeaponAttachment() { Reset(); }
	void Reset() {memset(this, 0, sizeof(*this)); }

	bool operator==(const wiWeaponAttachment& rhs)
	{
		bool res = true;
		for(int i=0; i<WPN_ATTM_MAX; ++i)
			res = res && (attachments[i]==rhs.attachments[i]);
		return res;
	}
	bool operator!=(const wiWeaponAttachment& rhs) { return !((*this)==rhs); }
};

struct wiWeaponAttachments
{
	// NOTE: you MUST increase P2PNET_VERSION if you change this structure
	wiWeaponAttachment primary_attachments;
	wiWeaponAttachment secondary_attachments;
	wiWeaponAttachment sidearm_attachments;

	wiWeaponAttachments() { Reset(); }
	void Reset() {memset(this, 0, sizeof(*this)); }

	bool operator==(const wiWeaponAttachments& rhs)
	{
		bool res = (primary_attachments==rhs.primary_attachments)
			&& (secondary_attachments==rhs.secondary_attachments)
			&& (sidearm_attachments==rhs.sidearm_attachments);
		return res;
	}
	bool operator!=(const wiWeaponAttachments& rhs) { return !((*this)==rhs); }
};

struct	wiLoadoutSlot
{
// NOTE: you MUST increase P2PNET_VERSION if you change this structure
	uint32_t LoadoutID;
	int	Class;		// CUserSkills::EClassID
	int	HonorPoints;
	int	TimePlayed;

	int	BodyMeshID;
	int	BodyHeadID;
	int	BodyHeadGearID;
	int	BodyArmorID;
	int	BodySkinID;
	int	BodyVoiceID;

	int	PrimaryWeaponID;
	int	SecondaryWeaponID;
	int	SidearmWeaponID;

	// can be an item that you can use, can be a passibe ability
	int	Item1;
	int	Item2;
	int	Item3;	
	int	Item4;
	
	// skills data. NOTE: we don't need SpendSP here, it should be somewhere in CClientUserProfile
	BYTE	SpendSP[3];	//SPs spend on all tiers
	BYTE	Skills[30];	//CUserSkills::NUM_SKILLS_PER_TIER * CUserSkills::NUM_TIERS];

	wiLoadoutSlot()
	{
		memset(this, 0, sizeof(*this));
	}

	bool isEquipped(int itemID)
	{
		if(itemID == BodyMeshID) return true;
		else if(itemID == BodyHeadID) return true;
		else if(itemID == BodyHeadGearID) return true;
		else if(itemID == BodyArmorID) return true;
		else if(itemID == BodySkinID) return true;
		else if(itemID == BodyVoiceID) return true;
		else if(itemID == PrimaryWeaponID) return true;
		else if(itemID == SecondaryWeaponID) return true;
		else if(itemID == SidearmWeaponID) return true;
		else if(itemID == Item1) return true;
		else if(itemID == Item2) return true;
		else if(itemID == Item3) return true;
		else if(itemID == Item4) return true;
		else
			return false;
	}

	bool hasItem(int id) const
	{
		return (id == Item1 || id == Item2 || id == Item3 || id == Item4);
	}
	
	int getSkillLevel(int SkillID) const 
	{
		int skillClass = SkillID / 100; //CUserSkills::SKILL_CLASS_MULT;
		if(Class != skillClass)
			return 0;
		int skillIdx = SkillID % 100; //CUserSkills::SKILL_CLASS_MULT;

		r3d_assert(skillIdx >= 0 && skillIdx < R3D_ARRAYSIZE(Skills));
		return Skills[skillIdx];
	}
};

struct wiInventoryItem
{
	uint32_t	itemID;
	uint32_t	expiration; // in minutes
	uint32_t	quantity;
	
  public:
	wiInventoryItem();
	~wiInventoryItem();
};

struct wiAchievement
{
	uint32_t	ID; // achievement ID
	uint32_t	value; // current value
	uint32_t	unlocked; // if it is unlocked achievement
	bool		dirty; // only save the dirty.
};

struct wiUserProfile
{
	int			isDevAccount;

	wiStats		Stats;

	enum { MAX_LOADOUT_SLOTS = 6, };
	wiLoadoutSlot	ArmorySlots[MAX_LOADOUT_SLOTS];
	int		NumSlots;

	enum{ MAX_POTENTIAL_ACHIEVEMENTS = 512 };
	wiAchievement	Achievements[MAX_POTENTIAL_ACHIEVEMENTS ];	// current achievements. this list will contain only in progress and unlocked achievements

	int		FactionScores[5];

	wiInventoryItem	Inventory[512];
	uint32_t	NumItems;
	bool		hasExpiringItems; // set to true if has items in inventory with expiration less than 1 day
	
	struct temp_fps_attach
	{
		temp_fps_attach() {memset(this, 0, sizeof(*this));}
		temp_fps_attach(uint32_t wpnID, uint32_t attmID, uint32_t exp, int equipped) : WeaponID(wpnID), AttachmentID(attmID), expiration(exp), isEquipped(equipped) {}
	  uint32_t	WeaponID;
	  uint32_t	AttachmentID;
	  uint32_t	expiration;	 // in minutes
	  int		isEquipped;
	};
	temp_fps_attach	FPSAttachments[2048];
	uint32_t	NumFPSAttachments;
	
	int		ClanID;
	int		ClanRank;
	char		ClanTag[5*2]; //utf8
	int		ClanTagColor;
	
	int		IsFPSEnabled;

	wiAchievement* getAchievementDataByID( int achievementID );
	wiAchievement* addAchievement( int achievementID );
	wiUserProfile()
	{
		memset(this, 0, sizeof(*this));
	}
};

class CUserProfile
{
  public:
	wiUserProfile	ProfileData;

	DWORD		CustomerID;
	DWORD		SessionID;
	int		AccountStatus;
	char		ScreenName[64];
	wchar_t		ScreenNameW[64];

	// new items in store
	int		NewItemsInStore[256];
	int		NumNewItems;
	
	// struct used to track daily/weekly reward
	struct PlayedStats_s
	{
	  int		GamesPlayed;
	  int		Kills;
	  int		Headshots;
	  int		CaptureFlags;
	  int		MatchesCQ;
	  int		MatchesDM;
	  int		MatchesSB;

	  int		getNumMatches() { return MatchesCQ + MatchesDM + MatchesSB; }
	  PlayedStats_s() 
	  {
		memset(this, 0, sizeof(*this));
	  }
	};
	PlayedStats_s	DailyStats;
	PlayedStats_s	WeeklyStats;

	struct tm	ServerTime;

	int		ShopUnlockLoadoutCost;
	int		ShopResetLoadoutCost;
	BYTE		ShopSkillCosts2[400][5]; //[MAX_SKILL_ID][CUserSkills::CUserSkills::NUM_RANKS]. Price can be negative!!! So before using this variable, convert to signed!
	int		ShopClanCreate;
	int		ShopClanAddMembers_GP[6];	// price for adding clan members
	int		ShopClanAddMembers_Num[6];	// number of adding members
	void		DeriveGamePricesFromItems();

  public:
	CUserProfile();
	virtual ~CUserProfile();

	int 		GetProfile(bool fromServer = false);
	void		 ParseLoadouts(pugi::xml_node& xmlItem);
	void		 ParseInventory(pugi::xml_node& xmlItem);
	void		 ParseFPSAttachments(pugi::xml_node& xmlItem);
	void		 ParseAchievements(pugi::xml_node& xmlItem);
	void		 ParseNewItemsInStore(pugi::xml_node& xmlItem);
	void		 ParseStatistics(pugi::xml_node& xmlStat, PlayedStats_s& stat);
	int		getInventoryItemByID(uint32_t id) const;
	bool	isValidInventoryItem(uint32_t id) const;

	int		ApiGetShopData();

	virtual bool    MarkAchievementComplete( int whichAchievement );
	bool CheckAchievementByValue( int whichAchievement, int value);
	bool IncrementAchievement( int whichAchievement, int value);

};

#ifndef WO_SERVER	

class CSteamClientCallbacks;
class CUserFriends;
class CUserClans;

class CClientUserProfile : public CUserProfile
{
  public:
	void		GenerateSessionKey(char* outKey);
	
	CUserFriends*	friends;

  public:
	CClientUserProfile();
	~CClientUserProfile();
	
	int		ApiGetItemsInfo();
	int		ApiUnlockLoadoutSlot2(int Class);
	int		ApiResetLoadoutSlot(int SlotID, int Class);
	int		ApiSkillLearn(int SlotID, int SkillID, int SkillLevel);
	int		ApiSkillReset(int SlotID);
	int		ApiModifyLoadoutSlot(int SlotID);
	int		ApiBuyItem(int itemId, int buyIdx);
	int		ApiChangeGamertag(int itemId, int buyIdx, const char* gametag); // returns <0 if not enough money, 0 - server error, 1 - success
	int		ApiGetCreateGameKey(int serverId, DWORD* out_createGameKey, int isBasicGame);
	int		ApiGNAGetBalance();

	bool		WelcomePackageProcess(int specID);
	
	// friends APIs
	int		ApiFriendAddReq(const char* gamertag, int* outFriendStatus);
	int		ApiFriendAddAns(DWORD friendId, bool allow);
	int		ApiFriendRemove(DWORD friendId);
	int		ApiFriendGetStats(DWORD friendId);
	
	// leaderboard
	struct LBEntry_s
	{
	  char		gamertag[64];
	  bool		havePremium;
	  wiStats	stats;
	};
	std::vector<LBEntry_s> m_lbData[4];
	int		ApiGetLeaderboard(int TableID, int StartPos, int* out_CurPos);
	
	// mystery box
	struct MysteryWin_s
	{
	  uint32_t	ItemID;
	  int		ExpDays;
	  int		GD;
	};
	MysteryWin_s	lastMysteryWin_;	// last winning from ApiMysteryBoxBuy/ApiLootBoxUnlock/ApiLootBoxSell

	struct MysteryLoot_s
	{
	  uint32_t	ItemID;	// 0 for GD
	  int		GDMin;
	  int		GDMax;
	  int		ExpDaysMin;
	  int		ExpDaysMax;
	};
	struct MysteryBox_s
	{
	  uint32_t	ItemID;
	  std::vector<MysteryLoot_s> content;
	};
	std::vector<MysteryBox_s> mysteryBoxes_;	// we'll live with copy overhead of struct...
	int		ApiMysteryBoxGetContent(int itemId, const MysteryBox_s** out_box);
	int		ApiLootBoxBuy(int itemId, int buyIdx);
	int		ApiLootBoxSell(int itemId);
	
	//
	// weapon attachment API
	//
	int		ApiWeaponAttachBuy(int WeaponID, int AttachID, int slot, int buyIdx);
	int		ApiWeaponAttachEquip(int WeaponID, int AttachID, int slot);
	int		ApiWeaponAttachFixDefaults(int WeaponID);
	
	// Achievements API
	int		ApiUpdateAchievements(int numAchs, wiAchievement* achs);
	
	//
	// daily retention API
	//
	std::vector<int> retentionBonusByDays_;
	int		curRetentionDays_;
	int		minutesToNextRetDay_;	// minutes left to next retention day switch
	int		ApiRetBonusGetInfo();	// fill curRetentionDays_ and retentionBonusByDays_
	int		ApiRetBonusClaim();
	
	//
	// Clans API is inside this class
	//
	CUserClans*	clans;

	//
	// steam APIs
	//
	struct SteamGPShop_s {
		int	gpItemID;
		int	GP;
		int	BonusGP;
		int	PriceUSD;	// in CENTS
	};
	r3dTL::TArray<SteamGPShop_s> steamGPShopData;
	CSteamClientCallbacks* steamCallbacks;
	void		RegisterSteamCallbacks();
	void		DeregisterSteamCallbacks();
	
	int		ApiSteamGetShop();

	struct SteamAuthResp_s {
 	  bool		gotResp;
	  int		bAuthorized;
	  __int64	ulOrderID;
	}		steamAuthResp;
	int		ApiSteamStartBuyGP(int gpItemId);

	int		ApiSteamFinishBuyGP(__int64 orderId);

	void RecordFrontEndAchievements();

};

// gUserProfile should be defined only in game mode, server must not use this global
extern  CClientUserProfile gUserProfile;	 
#endif	
