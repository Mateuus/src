#include "r3dPCH.h"
#include "r3d.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"
#include "CkByteData.h"

#include "UserProfile.h"
#include "UserFriends.h"
#include "UserClans.h"
#include "UserSkills.h"
#include "backend/WOBackendAPI.h"

#include "ObjectsCode/WEAPONS/WeaponConfig.h"
#include "ObjectsCode/WEAPONS/WeaponArmory.h"

#ifndef WO_SERVER
#include "SteamHelper.h"
#endif

int g_RanksPoints[MAX_NUM_RANKS] = 
{
	1000,
	2000,
	3000,
	4000,
	5000,
	6000,
	7000,
	8000,
	20000,
	25000,
	30000,
	35000,
	40000,
	45000,
	50000,
	55000,
	60000,
	65000,
	85000,
	92500,
	100000,
	107500,
	115000,
	122500,
	130000,
	137500,
	145000,
	152500,
	200000,
	225000,
	250000,
	275000,
	300000,
	327500,
	355000,
	382500,
	410000,
	437500,
	600000,
	696000,
	808000,
	938000,
	1089000,
	1264000,
	1467000,
	1702000,
	1975000,
	2291000,
	2658000,
	3084000,
	3578000,
	4151000,
	4816000,
	5587000,
	6481000,
	7518000,
	8721000,
	10117000,
	12141000,
};

static int g_RUS_RanksPoints[MAX_NUM_RANKS] = 
{
	1000,
	2000,
	3000,
	4000,
	5200,
	6600,
	8200,
	10000,
	13000,
	16900,
	21970,
	28561,
	37129,
	48268,
	62749,
	81573,
	106045,
	137858,
	179216,
	232981,
	302875,
	393738,
	511859,
	665417,
	865042,
	1124554,
	1461920,
	1900496,
	2470645,
	2717710,
	2989481,
	3288429,
	3617272,
	3978999,
	4376899,
	4814589,
	5296048,
	5825652,
	6408218,
	7049039,
	7753943,
	8529338,
	9382271,
	10320499,
	11352548,
	12487803,
	13736584,
	15110242,
	16621266,
	18782031,
	21223695,
	23982775,
	27340363,
	31168014,
	35531536,
	40861267,
	46990457,
	54039025,
	62144879,
};

void UserProfile_SetRankPointsRUS()
{
	r3d_assert(sizeof(g_RUS_RanksPoints) == sizeof(g_RanksPoints));
	memcpy(g_RanksPoints, g_RUS_RanksPoints, sizeof(g_RanksPoints));
}

/*
void UserProfile_GenerateSQLRanks()
{
	FILE* f = fopen_for_write("ranks.sql", "wt");
	fprintf(f, "delete from DataRankPoints\n");
	for(int i=0; i<MAX_NUM_RANKS-1; i++) {
		fprintf(f, "insert into DataRankPoints values (%d, %d)\n", i+1, g_RanksPoints[i]);
	}
	fprintf(f, "insert into DataRankPoints values (%d, %d)\n", MAX_NUM_RANKS, 99999999);
	fclose(f);
}
*/

int wiStats::getRankLevel() const
{
	for(int i=0; i<MAX_NUM_RANKS; ++i)
	{
		if(HonorPoints < g_RanksPoints[i])
			return i+1;
	}
	return MAX_NUM_RANKS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WO_SERVER
CClientUserProfile	gUserProfile;
#endif

const char* g_ServerKey = "CfFkqQWjfgksYG56893GDhjfjZ20";

bool isWeaponCategory(STORE_CATEGORIES cat)
{
	return (cat>=storecat_ASR && cat <=storecat_UsableItem);
}

bool isGearCategory(STORE_CATEGORIES cat)
{
	return (cat>=storecat_Characters && cat<=storecat_Heroes);
}

bool isItemCategory(STORE_CATEGORIES cat)
{
	return (cat>=storecat_Account && cat<=storecat_Items);
}

wiInventoryItem::wiInventoryItem()
{
}

wiInventoryItem::~wiInventoryItem()
{
}

wiAchievement* wiUserProfile::getAchievementDataByID( int achievementID )
{
	if( Achievements[achievementID].ID == achievementID ){
		return &(Achievements[achievementID]);
	} 
	else 
	{
		return NULL;
	}
	
}

wiAchievement* wiUserProfile::addAchievement( int achievementID )
{
	r3d_assert( getAchievementDataByID( achievementID) == NULL );
	r3d_assert( achievementID < MAX_POTENTIAL_ACHIEVEMENTS );

	Achievements[achievementID].ID = achievementID;
	Achievements[achievementID].unlocked = 0;
	Achievements[achievementID].value = 0;

	return &(Achievements[achievementID]);
}


CUserProfile::CUserProfile()
{
	memset((void *)&ProfileData, 0, sizeof(ProfileData));

	CustomerID = 0;
	SessionID = 0;
	AccountStatus = 0;
	r3dscpy(ScreenName, "unknown");
	ScreenNameW[0] = 0;

	ProfileData.NumSlots = 0;
	
	NumNewItems = 0;
}

CUserProfile::~CUserProfile()
{
}

int CUserProfile::getInventoryItemByID(uint32_t id) const
{
	if(id == 0)
		return -1;

	// todo: make it faster!
	for(uint32_t i=0; i<ProfileData.NumItems; ++i)
	{
		if(ProfileData.Inventory[i].itemID == id)
			return i;
	}
	return -1;
}

static void parseLoadoutSlot(const char* slotData, wiLoadoutSlot& w)
{
	r3d_assert(slotData);
	// should match arguments of ModifyLoadoutSlot!!
	int nargs = sscanf(slotData, "%d %d %d %d %d %d %d %d %d %d %d %d %d", 
		&w.BodyArmorID, 
		&w.BodyHeadGearID, 
		&w.BodyHeadID, 
		&w.BodyMeshID, 
		&w.BodySkinID, 
		&w.BodyVoiceID, 
		&w.Item1, 
		&w.Item2, 
		&w.Item3, 
		&w.Item4, 
		&w.PrimaryWeaponID, 
		&w.SecondaryWeaponID, 
		&w.SidearmWeaponID);
	if(nargs != 13)
	{
		r3dOutToLog("Incorrect number of args in loadout slot %d\n", nargs);
		memset(&w, 0, sizeof(wiLoadoutSlot));
	}
	
	return;
}

int CUserProfile::GetProfile(bool fromServer)
{
	CWOBackendReq req(this, "api_GetProfile4.aspx");
	if(fromServer)
		req.AddParam("jg", "1");
		
	if(!req.Issue())
	{
		r3dOutToLog("GetProfile FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	pugi::xml_node xmlAccount = xmlFile.child("account");
	uint32_t custID = xmlAccount.attribute("CustomerID").as_uint();
	if(custID == 0) // bad request
	{
		r3dOutToLog("Bad request in GetProfile()\n");
		return 9;
	}
	
	r3dscpy(ScreenName, xmlAccount.attribute("gamertag").value());
	wcscpy(ScreenNameW, utf8ToWide(ScreenName));
	
	AccountStatus = xmlAccount.attribute("AccountStatus").as_int();
	ProfileData.Stats.GamePoints  = xmlAccount.attribute("gamepoints").as_int();
	ProfileData.Stats.GameDollars  = xmlAccount.attribute("GameDollars").as_int();
	ProfileData.Stats.HonorPoints = xmlAccount.attribute("HonorPoints").as_int();
	ProfileData.Stats.SkillPoints = xmlAccount.attribute("SkillPoints").as_int();
	ProfileData.Stats.Kills       = xmlAccount.attribute("Kills").as_int();
	ProfileData.Stats.Deaths      = xmlAccount.attribute("Deaths").as_int();
	ProfileData.Stats.ShotsFired  = xmlAccount.attribute("ShotsFired").as_int();
	ProfileData.Stats.ShotsHits   = xmlAccount.attribute("ShotsHits").as_int();
	ProfileData.Stats.Headshots   = xmlAccount.attribute("Headshots").as_int();
	ProfileData.Stats.AssistKills = xmlAccount.attribute("AssistKills").as_int();
	ProfileData.Stats.Wins        = xmlAccount.attribute("Wins").as_int();
	ProfileData.Stats.Losses      = xmlAccount.attribute("Losses").as_int();
	ProfileData.Stats.CaptureNeutralPoints = xmlAccount.attribute("CaptureNeutralPoints").as_int();
	ProfileData.Stats.CaptureEnemyPoints   = xmlAccount.attribute("CaptureEnemyPoints").as_int();
	ProfileData.Stats.TimePlayed  = xmlAccount.attribute("TimePlayed").as_int();
	ProfileData.isDevAccount	  = xmlAccount.attribute("IsDev").as_int();
	// faction scores
	ProfileData.FactionScores[0]  = xmlAccount.attribute("F1S").as_int();
	ProfileData.FactionScores[1]  = xmlAccount.attribute("F2S").as_int();
	ProfileData.FactionScores[2]  = xmlAccount.attribute("F3S").as_int();
	ProfileData.FactionScores[3]  = xmlAccount.attribute("F4S").as_int();
	ProfileData.FactionScores[4]  = xmlAccount.attribute("F5S").as_int();
	// clan
	ProfileData.ClanID            = xmlAccount.attribute("ClanID").as_int();
	ProfileData.ClanRank          = xmlAccount.attribute("ClanRank").as_int();
	r3dscpy(ProfileData.ClanTag,    xmlAccount.attribute("ClanTag").value());
	ProfileData.ClanTagColor      = xmlAccount.attribute("ClanTagColor").as_int();
	// test fps flag
	ProfileData.IsFPSEnabled      = xmlAccount.attribute("IsFPSEnabled").as_int();

	const char* curTime = xmlAccount.attribute("time").value();
	memset(&ServerTime, 0, sizeof(ServerTime));
	sscanf(curTime, "%d %d %d %d %d", 
		&ServerTime.tm_year, &ServerTime.tm_mon, &ServerTime.tm_mday,
		&ServerTime.tm_hour, &ServerTime.tm_min);
	ServerTime.tm_year -= 1900;
	ServerTime.tm_mon -= 1;
	ServerTime.tm_isdst = 1; // day light saving time

	// fill things
	ParseLoadouts(xmlAccount.child("loadouts"));
	ParseAchievements(xmlAccount.child("achievements"));
	ParseFPSAttachments(xmlAccount.child("fpsattach"));
	ParseInventory(xmlAccount.child("inventory"));
	ParseNewItemsInStore(xmlAccount.child("nis"));
	ParseStatistics(xmlAccount.child("sday"), DailyStats);
	ParseStatistics(xmlAccount.child("sweek"), WeeklyStats);
	
	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
	{
		wiLoadoutSlot& w = ProfileData.ArmorySlots[i];
		if(w.LoadoutID == 0)
			continue;

		if(!isValidInventoryItem(w.BodyArmorID))
			w.BodyArmorID = 0;
		if(!isValidInventoryItem(w.BodyHeadGearID))
			w.BodyHeadGearID = 0;
		if(!isValidInventoryItem(w.BodyHeadID))
			w.BodyHeadID = 0;
		if(!isValidInventoryItem(w.BodyMeshID))
			w.BodyMeshID = 0;
		if(!isValidInventoryItem(w.BodySkinID))
			w.BodySkinID = 0;
		if(!isValidInventoryItem(w.BodyVoiceID))
			w.BodyVoiceID = 0;
		if(!isValidInventoryItem(w.Item1))
			w.Item1 = 0;
		if(!isValidInventoryItem(w.Item2))
			w.Item2 = 0;
		if(!isValidInventoryItem(w.Item3))
			w.Item3 = 0;
		if(!isValidInventoryItem(w.Item4))
			w.Item4 = 0;
		if(!isValidInventoryItem(w.PrimaryWeaponID))
			w.PrimaryWeaponID = 0;
		if(!isValidInventoryItem(w.SecondaryWeaponID))
			w.SecondaryWeaponID = 0;
		if(!isValidInventoryItem(w.SidearmWeaponID))
			w.SidearmWeaponID = 0;
	}
	
	return 0;
}

bool CUserProfile::isValidInventoryItem(uint32_t id) const
{
	bool skillItem = false;
	if( id == WeaponConfig::ITEMID_MedKit ||
		id == WeaponConfig::ITEMID_MotionSensor ||
		id == WeaponConfig::ITEMID_RiotShield ||
		id == WeaponConfig::ITEMID_AdrenalineShot ||
		id == WeaponConfig::ITEMID_MorphineShot ||
		id == WeaponConfig::ITEMID_AutoTurret ||
		id == WeaponConfig::ITEMID_RespawnBeacon 
		)
		skillItem = true;

	if(getInventoryItemByID(id)!=-1 || skillItem)
		return true;

	return false;
}

void CUserProfile::ParseLoadouts(pugi::xml_node& xmlItem)
{
	ProfileData.NumSlots = 0;
	
	// parse all slots
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		wiLoadoutSlot& w = ProfileData.ArmorySlots[ProfileData.NumSlots++];
		if(ProfileData.NumSlots > wiUserProfile::MAX_LOADOUT_SLOTS)
			r3dError("more that 6 profiles!");

		w.LoadoutID   = xmlItem.attribute("id").as_uint();
		w.Class       = xmlItem.attribute("cl").as_int();
		w.HonorPoints = xmlItem.attribute("xp").as_int();
		w.TimePlayed  = xmlItem.attribute("tm").as_int();

		// we hope that our "designers" will limit SP spend on each tier by 255...
		w.SpendSP[0]  = (BYTE)xmlItem.attribute("sp1").as_int();
		w.SpendSP[1]  = (BYTE)xmlItem.attribute("sp2").as_int();
		w.SpendSP[2]  = (BYTE)xmlItem.attribute("sp3").as_int();
		const char* skills = xmlItem.attribute("sv").value();
		int skLen = R3D_MIN((int)strlen(skills), CUserSkills::NUM_SKILLS_PER_TIER * CUserSkills::NUM_TIERS);
		for(int i=0, len = skLen; i<len;i++)
		{
			if(skills[i] >= '0' && skills[i] <= '9')
				w.Skills[i] = BYTE(skills[i] - '0');
		}
		
		const char* slotData = xmlItem.attribute("lo").value();
		parseLoadoutSlot(slotData, w);

		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseAchievements(pugi::xml_node& xmlItem)
{

	for(int AchievementIndex=0; AchievementIndex< wiUserProfile::MAX_POTENTIAL_ACHIEVEMENTS; AchievementIndex++)
	{
		// makes it so this appears invalid (The ID Doesn't match it's index)
		ProfileData.Achievements[AchievementIndex].ID = wiUserProfile::MAX_POTENTIAL_ACHIEVEMENTS;
	}

	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		uint32_t AchID  = xmlItem.attribute("id").as_uint();
		uint32_t AchValue = xmlItem.attribute("v").as_uint();
		uint32_t AchUnlocked = xmlItem.attribute("u").as_uint();
		if(AchID == 0) { // in pugixml, for empty line it will return empty xml_node
			xmlItem = xmlItem.next_sibling();
			continue;
		}
		

		if( AchID < 512) {

			wiAchievement& itm = ProfileData.Achievements[AchID];

			itm.ID     = AchID;
			itm.value  = AchValue;
			itm.unlocked = AchUnlocked;
			itm.dirty = false;
		}
		
		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseInventory(pugi::xml_node& xmlItem)
{
	ProfileData.NumItems = 0;
	ProfileData.hasExpiringItems = false;

	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		uint32_t itemId  = xmlItem.attribute("id").as_uint();
		uint32_t minLeft = xmlItem.attribute("ml").as_uint();
		uint32_t quantity= xmlItem.attribute("qt").as_uint();
		if(quantity == 0) // Quantity attribute can be empty to save space
			quantity = 1;
		if(itemId == 0) { // in pugixml, for empty line it will return empty xml_node
			xmlItem = xmlItem.next_sibling();
			continue;
		}
		
		wiInventoryItem& itm = ProfileData.Inventory[ProfileData.NumItems++];
		r3d_assert(ProfileData.NumItems < 512);

		itm.itemID     = itemId;
		itm.expiration = minLeft;
		itm.quantity   = quantity;

		if(minLeft <= 24*60) // 24 hours
			ProfileData.hasExpiringItems = true;

		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseFPSAttachments(pugi::xml_node& xmlItem)
{
	ProfileData.NumFPSAttachments = 0;

	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		wiUserProfile::temp_fps_attach& att = ProfileData.FPSAttachments[ProfileData.NumFPSAttachments++];
		r3d_assert(ProfileData.NumFPSAttachments < 2048);

		att.WeaponID     = xmlItem.attribute("wi").as_uint();
		att.AttachmentID = xmlItem.attribute("ai").as_uint();
		att.expiration   = xmlItem.attribute("ml").as_uint();
		att.isEquipped   = xmlItem.attribute("eq").as_uint();

		xmlItem = xmlItem.next_sibling();
	}
	
	return;
}

void CUserProfile::ParseNewItemsInStore(pugi::xml_node& xmlItem)
{
	NumNewItems = 0;

	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		uint32_t itemId  = xmlItem.attribute("id").as_uint();
		if(itemId != 0) // in pugixml, for empty line it will return empty xml_node
		{
			NewItemsInStore[NumNewItems++] = itemId;
			r3d_assert(NumNewItems < R3D_ARRAYSIZE(NewItemsInStore));
		}
		
		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseStatistics(pugi::xml_node& xmlStat, PlayedStats_s& stat)
{
	// statistics
	stat.GamesPlayed = xmlStat.attribute("dg").as_int();
	stat.Kills = xmlStat.attribute("ki").as_int();
	stat.Headshots = xmlStat.attribute("hs").as_int();
	stat.CaptureFlags = xmlStat.attribute("cf").as_int();
	stat.MatchesCQ = xmlStat.attribute("mcq").as_int();
	stat.MatchesDM = xmlStat.attribute("mdm").as_int();
	stat.MatchesSB = xmlStat.attribute("msb").as_int();
}

wiStoreItem g_StoreItems[MAX_NUM_STORE_ITEMS] = {0}; 
uint32_t g_NumStoreItems = 0;

const char* STORE_CATEGORIES_NAMES[storecat_NUM_ITEMS] = 
{
	"$CatInvalid", // 0
	"$CatAccount", //1
	"$CatBoosts", //2
	"$CatMysteryBox", //3
	"$CatItems", //4
	"$CatAbilities", //5
	"$CatInvalid", //6
	"$CatLootBox", //7
	"$CatInvalid", //8
	"$CatDeals", //9
	"$CatChar", //10
	"$CatGear", //11
	"$CatHeads", //12
	"$CatHeadGear", //13
	"$CatInvalid", //14
	"$CatInvalid", //15
	"$CatHeroes", //16
	"$CatInvalid", //17
	"$CatInvalid", //18
	"$CatFPSAttachment", //19
	"$CatASR", //20
	"$CatSniper", //21
	"$CatSHTG", //22
	"$CatMG", //23
	"$CatSUP", //24
	"$CatHG", //25
	"$CatSMG", //26
	"$CatGrenade", //27
	"$CatUsableItem", //28
	"$CatMelee", // 29
};

static void addItemToStore(uint32_t itemID, 
						   uint32_t price1d, uint32_t price7d, uint32_t price30d, uint32_t pricePerm,
						   uint32_t gd_price1d, uint32_t gd_price7d, uint32_t gd_price30d, uint32_t gd_pricePerm)
{
	g_StoreItems[g_NumStoreItems].itemID       = itemID;
	g_StoreItems[g_NumStoreItems].price1day    = price1d;
	g_StoreItems[g_NumStoreItems].price7day    = price7d;
	g_StoreItems[g_NumStoreItems].price30day   = price30d;
	g_StoreItems[g_NumStoreItems].pricePerm    = pricePerm;
	g_StoreItems[g_NumStoreItems].gd_price1day = gd_price1d;
	g_StoreItems[g_NumStoreItems].gd_price7day = gd_price7d;
	g_StoreItems[g_NumStoreItems].gd_price30day= gd_price30d;
	g_StoreItems[g_NumStoreItems].gd_pricePerm = gd_pricePerm;

	if(g_StoreItems[g_NumStoreItems].itemID > 0) // bug in pugixml, for empty line it will return empty xml_node
		g_NumStoreItems++;
	r3d_assert(g_NumStoreItems < MAX_NUM_STORE_ITEMS);
}

int CUserProfile::ApiGetShopData()
{
	CWOBackendReq req(this, "api_GetShop5.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("GetShopData FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	g_NumStoreItems = 0;

	const char* d = req.bodyStr_;
	const char* p = d;
	if(p[0] != 'S' || p[1] != 'H' || p[2] != 'O' || p[3] != '3') {
		r3dOutToLog("GetShopData: bad answer #1\n");
		return 9;
	}
	p += 4;

	// new skills. served as skillID/byte array of 5 per price
	while(true)
	{
		WORD skillID = *(WORD*)p; p += 2;
		if(skillID == 0xFFFF)
			break;
		r3d_assert(skillID < CUserSkills::CLASS_MAX * CUserSkills::SKILL_CLASS_MULT);

		ShopSkillCosts2[skillID][0] = *(BYTE*)p;  p += 1;
		ShopSkillCosts2[skillID][1] = *(BYTE*)p;  p += 1;
		ShopSkillCosts2[skillID][2] = *(BYTE*)p;  p += 1;
		ShopSkillCosts2[skillID][3] = *(BYTE*)p;  p += 1;
		ShopSkillCosts2[skillID][4] = *(BYTE*)p;  p += 1;
	}

	if(p[0] != 'S' || p[1] != 'H' || p[2] != 'O' || p[3] != '3') {
		r3dOutToLog("GetShopData: bad answer #2\n");
		r3d_assert(false);
		return 9;
	}
	p += 4;
	
	// shop items
	while(1) 
	{
		if((p - d) >= req.bodyLen_) {
			r3dOutToLog("GetShopData: bad answer #2\n");
			return 9;
		}

		// end tag
		if(p[0] == 'S' && p[1] == 'H' && p[2] == 'O' && p[3] == '3')
			break;

		// item
		DWORD itemId    = *(DWORD*)p; p += 4;
		BYTE  priceBits = *(BYTE*)p;  p += 1;
		BYTE  itemCat   = *(BYTE*)p;  p += 1;

		DWORD  price1d=0, price7d=0, price30d=0, pricePerm=0;
		DWORD  gd_price1d=0, gd_price7d=0, gd_price30d=0, gd_pricePerm=0;

		r3d_assert(priceBits);
		if(priceBits & 0x1) {
			price1d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x2) {
			price7d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x4) {
			price30d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x8) {
			pricePerm = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x10) {
			gd_price1d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x20) {
			gd_price7d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x40) {
			gd_price30d = *(DWORD*)p;  p += 4;
		}
		if(priceBits & 0x80) {
			gd_pricePerm = *(DWORD*)p;  p += 4;
		}

		addItemToStore(itemId, 
			price1d, price7d, price30d, pricePerm,
			gd_price1d, gd_price7d, gd_price30d, gd_pricePerm);
	}

	DeriveGamePricesFromItems();

	return 0;
}

void CUserProfile::DeriveGamePricesFromItems()
{
	// now ability prices and unlock slot prices defined by items
	for(uint32_t i = 0; i<g_NumStoreItems; i++) 
	{
		const wiStoreItem& itm = g_StoreItems[i];
		
		switch(itm.itemID) 
		{
		case 301143: ShopClanCreate = itm.pricePerm; break;
		case 301142: ShopUnlockLoadoutCost = itm.pricePerm; break; 
		case 301150: ShopResetLoadoutCost = itm.pricePerm; break;
		}
		
		// clan add members items
		if(itm.itemID >= 301144 && itm.itemID <= 301149)
		{
			ShopClanAddMembers_GP[itm.itemID - 301144]  = itm.pricePerm;
			ShopClanAddMembers_Num[itm.itemID - 301144] = itm.price1day;
		}
	}
}

bool CUserProfile::CheckAchievementByValue( int whichAchievement, int value )
{
	const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
	if( achievementInfo->value == 0 ) {
		// this should not happen.  Complain. 
		r3dOutToLog("We're checking the achievement index %d which is achievement id %d, and it doesn't have a value", whichAchievement, achievementInfo->id);
		return false;
	}

	if( achievementInfo->value <= value ) {
		return MarkAchievementComplete( whichAchievement );
	}
	else 
	{
		return false;
	}
	

}

bool CUserProfile::MarkAchievementComplete( int whichAchievement )
{

	const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
	if( achievementInfo->enabled == false )
	{
		// no disabled achievements. 
		return false;
	}
	
	wiAchievement* achievementData = ProfileData.getAchievementDataByID( achievementInfo->id );
	
	if( achievementData != NULL && achievementData->unlocked != 0 )
	{
		// already accomplished
		return false; 
	}
	
	if( achievementData == NULL ) {
		achievementData = ProfileData.addAchievement( achievementInfo->id );
	}

	achievementData->unlocked = 1;
	achievementData->dirty = true;
	
	return true;

}

bool CUserProfile::IncrementAchievement( int whichAchievement, int value )
{
	const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
	if( achievementInfo->value == 0 ) {
		// this should not happen.  Complain. 
		r3dOutToLog("We're incrementing an achievement index %d which is achievement id %d, and it doesn't have a value", whichAchievement, achievementInfo->id);
		return false;
	}

	if( achievementInfo->enabled == false )
	{
		// no disabled achievements. 
		return false;
	}
	
	wiAchievement* achievementData = ProfileData.getAchievementDataByID( achievementInfo->id );

	if( achievementData != NULL && achievementData->unlocked != 0 )
	{
		// already accomplished we shouldn't infrement this. 
		return false; 
	}

	if( achievementData == NULL ) {
		achievementData = ProfileData.addAchievement( achievementInfo->id );
	}

	achievementData->value += value;
	achievementData->dirty = true;

	if( achievementInfo->value <= (int) achievementData->value ) {
		return MarkAchievementComplete( whichAchievement );
	} 
	else 
	{
		return false; 
	}

}

#ifndef WO_SERVER

class GameObject;
#include "ObjectsCode/Weapons/WeaponArmory.h"

extern int RUS_CLIENT;

CClientUserProfile::CClientUserProfile()
{
	ShopUnlockLoadoutCost = 0;
	ShopResetLoadoutCost = 0;
	ShopClanCreate = 0;
	memset(&ShopClanAddMembers_GP, 0, sizeof(ShopClanAddMembers_GP));
	memset(&ShopClanAddMembers_Num, 0, sizeof(ShopClanAddMembers_Num));
	memset(&ShopSkillCosts2, 0, sizeof(ShopSkillCosts2));

	steamCallbacks = NULL;
	friends = new CUserFriends();
	clans = new CUserClans();

	curRetentionDays_ = 0;
	minutesToNextRetDay_ = 0;
}

CClientUserProfile::~CClientUserProfile()
{
	SAFE_DELETE(friends);
	SAFE_DELETE(clans);
}

void CClientUserProfile::GenerateSessionKey(char* outKey)
{
	char sessionInfo[128];
	sprintf(sessionInfo, "%d:%d", CustomerID, SessionID);
	for(size_t i=0; i<strlen(sessionInfo); ++i)
		sessionInfo[i] = sessionInfo[i]^0x64;
	
	CkString str;
	str = sessionInfo;
	str.base64Encode("utf-8");
	
	strcpy(outKey, str.getUtf8());
	return;
}

// special code that'll replace name/description/icon for specified item
template <class T>
static void replaceItemNameParams(T* itm, pugi::xml_node& xmlNode)
{
	const char* name = xmlNode.attribute("name").value();
	const char* desc = xmlNode.attribute("desc").value();
	const char* fname = xmlNode.attribute("fname").value();
	
	// replace description
	if(strcmp(desc, itm->m_Description) != 0)
	{
		free(itm->m_Description);
		free(itm->m_DescriptionW);

		itm->m_Description = strdup(desc);
		itm->m_DescriptionW = wcsdup(utf8ToWide(itm->m_Description));
	}
	
	// replace name
	if(strcmp(name, itm->m_StoreName) != 0)
	{
		free(itm->m_StoreName);
		free(itm->m_StoreNameW);

		itm->m_StoreName = strdup(name);
		itm->m_StoreNameW = wcsdup(utf8ToWide(itm->m_StoreName));
	}
	
	// replace store icon (FNAME)
	char storeIcon[256];
	sprintf(storeIcon, "$Data/Weapons/StoreIcons/%s.dds", fname);
	if(strcmp(storeIcon, itm->m_StoreIcon) != 0)
	{
		free(itm->m_StoreIcon);
		itm->m_StoreIcon = strdup(storeIcon);
	}
}

int CClientUserProfile::ApiGetItemsInfo()
{
	CWOBackendReq req(this, "api_GetItemsInfo.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("GetItemsInfo FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlItems = xmlFile.child("items");

	// read gears (in <gears><g>...)
	pugi::xml_node xmlNode = xmlItems.child("gears").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		GearConfig* gc = const_cast<GearConfig*>(gWeaponArmory.getGearConfig(itemId));
		if(gc)
		{
			gc->m_LevelRequired = xmlNode.attribute("lv").as_int();
			gc->m_Weight        = xmlNode.attribute("wg").as_float();
			gc->m_damagePerc    = xmlNode.attribute("dp").as_float() / 100.0f;
			gc->m_damageMax     = xmlNode.attribute("dm").as_float();
		}

		xmlNode = xmlNode.next_sibling();
	}

	// read weapons (in <weapons><w>...)
	xmlNode = xmlItems.child("weapons").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		WeaponConfig* wc = const_cast<WeaponConfig*>(gWeaponArmory.getWeaponConfig(itemId));
		if(wc)
		{
			wc->m_LevelRequired = xmlNode.attribute("lv").as_int();
			wc->m_AmmoDamage    = xmlNode.attribute("d1").as_float();
			wc->m_AmmoDecay     = xmlNode.attribute("d2").as_float();
			wc->m_numClips      = xmlNode.attribute("c1").as_uint();
			wc->m_clipSize      = xmlNode.attribute("c2").as_uint();
			wc->m_fireDelay     = 60.0f / (xmlNode.attribute("rf").as_float());
			wc->m_spread        = xmlNode.attribute("sp").as_float();
			wc->m_recoil        = xmlNode.attribute("rc").as_float();
		}

		xmlNode = xmlNode.next_sibling();
	}

	// read packages(in <packages><p>...)
	xmlNode = xmlItems.child("packages").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		PackageConfig* pc = const_cast<PackageConfig*>(gWeaponArmory.getPackageConfig(itemId));
		if(pc)
		{
			replaceItemNameParams<PackageConfig>(pc, xmlNode);

			pc->m_addGD = xmlNode.attribute("gd").as_int();
			pc->m_addSP = xmlNode.attribute("sp").as_int();
			pc->m_itemID1    = xmlNode.attribute("i1i").as_int();
			pc->m_itemID1Exp = xmlNode.attribute("i1e").as_int();
			pc->m_itemID2    = xmlNode.attribute("i2i").as_int();
			pc->m_itemID2Exp = xmlNode.attribute("i2e").as_int();
			pc->m_itemID3    = xmlNode.attribute("i3i").as_int();
			pc->m_itemID3Exp = xmlNode.attribute("i3e").as_int();
			pc->m_itemID4    = xmlNode.attribute("i4i").as_int();
			pc->m_itemID4Exp = xmlNode.attribute("i4e").as_int();
			pc->m_itemID5    = xmlNode.attribute("i5i").as_int();
			pc->m_itemID5Exp = xmlNode.attribute("i5e").as_int();
			pc->m_itemID6    = xmlNode.attribute("i6i").as_int();
			pc->m_itemID6Exp = xmlNode.attribute("i6e").as_int();
		}

		xmlNode = xmlNode.next_sibling();
	}

	// read mystery crates/loot boxes names
	xmlNode = xmlItems.child("generics").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		ItemConfig* itm = const_cast<ItemConfig*>(gWeaponArmory.getItemConfig(itemId));
		if(itm)
		{
			replaceItemNameParams<ItemConfig>(itm, xmlNode);
		}

		xmlNode = xmlNode.next_sibling();
	}

	return 0;
}

int CClientUserProfile::ApiUnlockLoadoutSlot2(int Class)
{
	if(ProfileData.NumSlots >= wiUserProfile::MAX_LOADOUT_SLOTS)
	{
#ifndef FINAL_BUILD
		r3dError("too many slots unlocked already");
#endif
		return 6;
	}

	CWOBackendReq req(this, "api_LoadoutUnlock.aspx");
	req.AddParam("Class", Class);
	if(!req.Issue())
	{
		r3dOutToLog("UnlockLoadoutSlot failed: %d", req.resultCode_);
		return false;
	}

	// reread profile
	GetProfile();
	
	return 0;
}

int CClientUserProfile::ApiResetLoadoutSlot(int SlotID, int Class)
{
	r3d_assert(ShopResetLoadoutCost);
	r3d_assert(SlotID >= 0 && SlotID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiLoadoutSlot& w = ProfileData.ArmorySlots[SlotID];

	CWOBackendReq req(this, "api_LoadoutReset.aspx");
	req.AddParam("LoadoutID",  w.LoadoutID);
	req.AddParam("Class", Class);
	if(!req.Issue())
	{
		r3dOutToLog("ApiResetLoadoutSlot failed: %d", req.resultCode_);
		return false;
	}

	// reread profile
	GetProfile();
	
	return 0;
}

bool CClientUserProfile::WelcomePackageProcess(int specID)
{
	CWOBackendReq req(this, "api_WelcomePackage4.aspx");
	req.AddParam("specID", specID);
	if(!req.Issue())
	{
		r3dOutToLog("WelcomePackage failed: %d", req.resultCode_);
		return false;
	}

	AccountStatus = 101;
	return true;
}

int CClientUserProfile::ApiSkillLearn(int SlotID, int SkillID, int SkillLevel)
{
	r3d_assert(SlotID >= 0 && SlotID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiLoadoutSlot& w = ProfileData.ArmorySlots[SlotID];

	CWOBackendReq req(this, "api_SkillLearn.aspx");
	req.AddParam("LoadoutID",  w.LoadoutID);
	req.AddParam("SkillID",    SkillID);
	req.AddParam("SkillLevel", SkillLevel);
	if(!req.Issue())
	{
		r3dOutToLog("ApiSkillLearn FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int spendSp1, spendSp2, spendSp3;
	int nargs = sscanf(req.bodyStr_, "%d %d %d", &spendSp1, &spendSp2, &spendSp3);
	if(nargs != 3)
	{
		r3dError("wrong answer for ApiSkillLearn");
		return 9;
	}

	w.SpendSP[0] = (BYTE)spendSp1;
	w.SpendSP[1] = (BYTE)spendSp2;
	w.SpendSP[2] = (BYTE)spendSp3;
			
	int idx = SkillID % 100; // CUserSkills::SKILL_CLASS_MULT;
	r3d_assert(idx >= 0 && idx < R3D_ARRAYSIZE(w.Skills));
	w.Skills[idx] = SkillLevel;
	return 0;
}

int CClientUserProfile::ApiSkillReset(int SlotID)
{
	r3d_assert(SlotID >= 0 && SlotID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiLoadoutSlot& w = ProfileData.ArmorySlots[SlotID];

	CWOBackendReq req(this, "api_SkillReset.aspx");
	req.AddParam("LoadoutID",  w.LoadoutID);
	if(!req.Issue())
	{
		r3dOutToLog("ApiSkillReset FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	w.SpendSP[0] = 0;
	w.SpendSP[1] = 0;
	w.SpendSP[2] = 0;
	memset(w.Skills, 0, sizeof(w.Skills));

	return 0;
}

int CClientUserProfile::ApiModifyLoadoutSlot(int SlotID)
{
	r3d_assert(SlotID >= 0 && SlotID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiLoadoutSlot& w = ProfileData.ArmorySlots[SlotID];

	CWOBackendReq req(this, "api_LoadoutModify.aspx");
	req.AddParam("LoadoutID", w.LoadoutID);
	req.AddParam("i1",     w.BodyArmorID);
	req.AddParam("i2",     w.BodyHeadGearID);
	req.AddParam("i3",     w.BodyHeadID);
	req.AddParam("i4",     w.BodyMeshID);
	req.AddParam("i5",     w.BodySkinID);
	req.AddParam("i6",     w.BodyVoiceID);
	req.AddParam("i7",     w.Item1);
	req.AddParam("i8",     w.Item2);
	req.AddParam("i9",     w.Item3);
	req.AddParam("i10",    w.Item4);
	req.AddParam("i11",    w.PrimaryWeaponID);
	req.AddParam("i12",    w.SecondaryWeaponID);
	req.AddParam("i13",    w.SidearmWeaponID);
	if(!req.Issue())
	{
		r3dOutToLog("ModifyLoadoutSlot FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	parseLoadoutSlot(req.bodyStr_, w);
	return 0;
}

int CClientUserProfile::ApiBuyItem(int itemId, int buyIdx)
{
	r3d_assert(buyIdx > 0);
	if(RUS_CLIENT && buyIdx >= 1 && buyIdx <= 4) // move buy idx to gamenet range [9-12]
		buyIdx += 8;

	CWOBackendReq req(this, "api_BuyItem3.aspx");
	req.AddParam("ItemID", itemId);
	req.AddParam("BuyIdx", buyIdx);
	if(!req.Issue())
	{
		r3dOutToLog("BuyItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	if(nargs != 1)
	{
		r3dError("wrong answer for ApiBuyItem");
		return 9;
	}

	// update balance
	if(buyIdx >= 5 && buyIdx <= 8)
		ProfileData.Stats.GameDollars = balance;
	else
		ProfileData.Stats.GamePoints  = balance;

	return 0;
}

int CClientUserProfile::ApiChangeGamertag(int itemId, int buyIdx, const char* gametag)
{
	r3d_assert(buyIdx > 0);
	if(RUS_CLIENT && buyIdx >= 1 && buyIdx <= 4) // move buy idx to gamenet range [9-12]
		buyIdx += 8;

	CWOBackendReq req(this, "api_ChangeGamerTag2.aspx");
	req.AddParam("ItemID",  itemId);
	req.AddParam("BuyIdx",  buyIdx);
	req.AddParam("gametag", gametag);
	if(!req.Issue())
	{
		r3dOutToLog("ChangeGamertag FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	if(nargs != 1)
	{
		r3dError("wrong answer for ApiChangeGamertag");
		return 9;
	}

	// update balance
	if(buyIdx >= 5 && buyIdx <= 8)
		ProfileData.Stats.GameDollars = balance;
	else
		ProfileData.Stats.GamePoints  = balance;

	return 0;
}

int CClientUserProfile::ApiGetCreateGameKey(int serverId, DWORD* out_createGameKey, int isBasicGame)
{
	*out_createGameKey = 0;
	
	CWOBackendReq req(this, "api_GetCreateGameKey3.aspx");
	req.AddParam("ServerID", serverId);
	req.AddParam("BasicGame", isBasicGame);
	if(!req.Issue())
	{
		r3dOutToLog("GetCreateGameKey FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_; 
	}

	int CreateGameKey = 0;
	int nargs = sscanf(req.bodyStr_, "%d" , &CreateGameKey);
	if(nargs != 1)
	{
		r3dError("wrong answer for GetCreateGameKey");
		return 9;
	}
	
	*out_createGameKey = CreateGameKey;
	return 0;
}

int CClientUserProfile::ApiGNAGetBalance()
{
	if(!RUS_CLIENT)
		r3dError("ApiGNAUpdateBalance");
		
	CWOBackendReq req(this, "api_GNAGetBalance.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("GNAUpdateBalance FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d" , &balance);
	if(nargs != 1)
	{
		r3dError("wrong answer for GNAUpdateBalance");
		return 9;
	}
	
	ProfileData.Stats.GamePoints = balance;
	return 0;
}


//
//
// Steam part of the code
//
//
#include "steam_api_dyn.h"

class CSteamClientCallbacks
{
  public:
	STEAM_CALLBACK( CSteamClientCallbacks, OnMicroTxnAuth, MicroTxnAuthorizationResponse_t, m_MicroTxnAuth);

	CSteamClientCallbacks() : 
		m_MicroTxnAuth(this, &CSteamClientCallbacks::OnMicroTxnAuth)
	{
	}
};

void CSteamClientCallbacks::OnMicroTxnAuth(MicroTxnAuthorizationResponse_t *pCallback)
{
	gUserProfile.steamAuthResp.gotResp     = true;
	gUserProfile.steamAuthResp.bAuthorized = pCallback->m_bAuthorized;
	gUserProfile.steamAuthResp.ulOrderID   = pCallback->m_ulOrderID;
}

void CClientUserProfile::RegisterSteamCallbacks()
{
	r3d_assert(steamCallbacks == NULL);
	steamCallbacks = new CSteamClientCallbacks();
}

void CClientUserProfile::DeregisterSteamCallbacks()
{
	SAFE_DELETE(steamCallbacks);
}

int CClientUserProfile::ApiSteamGetShop()
{
	steamGPShopData.Clear();

	CWOBackendReq req(this, "api_SteamBuyGP.aspx");
	req.AddParam("func", "shop");

	if(!req.Issue())
	{
		r3dOutToLog("ApiSteamGetShop FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlItem = xmlFile.child("SteamGPShop").first_child();
	while(!xmlItem.empty())
	{
		SteamGPShop_s d;
		d.gpItemID = xmlItem.attribute("ID").as_uint();
		d.GP       = xmlItem.attribute("GP").as_uint();
		d.BonusGP  = xmlItem.attribute("BonusGP").as_uint();
		d.PriceUSD = xmlItem.attribute("Price").as_uint();
		steamGPShopData.PushBack(d);

		xmlItem = xmlItem.next_sibling();
	}
	
	return 0;
}

int CClientUserProfile::ApiSteamStartBuyGP(int gpItemId)
{
	r3d_assert(gSteam.steamID);
	steamAuthResp.gotResp = false;
	
	char	strSteamId[1024];
	sprintf(strSteamId, "%I64u", gSteam.steamID);

	CWOBackendReq req(this, "api_SteamBuyGP.aspx");
	req.AddParam("func", "auth");
	req.AddParam("steamId",  strSteamId);
	req.AddParam("gpItemId", gpItemId);
	req.AddParam("country",  gSteam.country);

	if(!req.Issue())
	{
		r3dOutToLog("ApiSteamStartBuyGP FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CClientUserProfile::ApiSteamFinishBuyGP(__int64 orderId)
{
	char	strOrderId[1024];
	sprintf(strOrderId, "%I64d", orderId);

	CWOBackendReq req(this, "api_SteamBuyGP.aspx");
	req.AddParam("func", "fin");
	req.AddParam("orderId", strOrderId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiSteamFinishBuyGP FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	// update balance
	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	r3d_assert(nargs == 1);
	
	ProfileData.Stats.GamePoints = balance;
	return 0;
}

//
// friends APIs
//
int CClientUserProfile::ApiFriendAddReq(const char* gamertag, int* outFriendStatus)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "addReq");
	req.AddParam("name", gamertag);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendAddReq FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int friendStatus;
	int nargs = sscanf(req.bodyStr_, "%d", &friendStatus);
	r3d_assert(nargs == 1);
	*outFriendStatus = friendStatus;
	
	return 0;
}

int CClientUserProfile::ApiFriendAddAns(DWORD friendId, bool allow)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "addAns");
	req.AddParam("FriendID", friendId);
	req.AddParam("Allow", allow ? "1" : "0");

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendAddAns FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	return 0;
}

int CClientUserProfile::ApiFriendRemove(DWORD friendId)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "remove");
	req.AddParam("FriendID", friendId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendRemove FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	return 0;
}

int CClientUserProfile::ApiFriendGetStats(DWORD friendId)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "stats");
	req.AddParam("FriendID", friendId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendGetStats FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	friends->SetCurrentStats(xmlFile);
	
	return 0;
}

int CClientUserProfile::ApiGetLeaderboard(int TableID, int StartPos, int* out_CurPos)
{
	r3d_assert(TableID >= 0 && TableID <= 3);
	
	CWOBackendReq req(this, "api_LeaderboardGet.aspx");
	req.AddParam("t", TableID);
	req.AddParam("pos", StartPos);

	if(!req.Issue())
	{
		r3dOutToLog("ApiGetLeaderboard FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlLeaderboard = xmlFile.child("leaderboard");
	*out_CurPos = xmlLeaderboard.attribute("pos").as_int();
	int LbSize = xmlLeaderboard.attribute("size").as_int(); // for future use
	m_lbData[TableID].reserve(100);
	m_lbData[TableID].clear();

	pugi::xml_node xmlItem = xmlLeaderboard.first_child();
	while(!xmlItem.empty())
	{
		LBEntry_s lb;
		r3dscpy(lb.gamertag, xmlItem.attribute("GT").value());
		lb.stats.HonorPoints = xmlItem.attribute("XP").as_uint();
		lb.stats.Kills       = xmlItem.attribute("k").as_uint();
		lb.stats.Deaths      = xmlItem.attribute("d").as_uint();
		lb.stats.Wins        = xmlItem.attribute("w").as_uint();
		lb.stats.Losses      = xmlItem.attribute("l").as_uint();
		lb.stats.ShotsFired  = xmlItem.attribute("f").as_uint();
		lb.stats.ShotsHits   = xmlItem.attribute("h").as_uint();
		lb.stats.TimePlayed  = xmlItem.attribute("t").as_uint();
		lb.havePremium       = xmlItem.attribute("p").as_bool();
		m_lbData[TableID].push_back(lb);

		xmlItem = xmlItem.next_sibling();
	}
	
	return 0;
}

int CClientUserProfile::ApiMysteryBoxGetContent(int itemId, const MysteryBox_s** out_box)
{
	*out_box = NULL;

	// see if we already have that box
	for(size_t i=0, size=mysteryBoxes_.size(); i<size; i++) {
		if(mysteryBoxes_[i].ItemID == itemId) {
			*out_box = &mysteryBoxes_[i];
			return 0;
		}
	}

	CWOBackendReq req(this, "api_MysteryBox.aspx");
	req.AddParam("func", "info");
	req.AddParam("LootID", itemId);
	if(!req.Issue())
	{
		r3dOutToLog("ApiMysteryBoxGetContent FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	// save data for new mystery box
	MysteryBox_s box;
	box.ItemID = itemId;
	
	pugi::xml_node xmlItem = xmlFile.child("box").first_child();
	while(!xmlItem.empty())
	{
		MysteryLoot_s loot;
		loot.ItemID = xmlItem.attribute("ID").as_uint();
		if(loot.ItemID == 0) {
			loot.GDMin = xmlItem.attribute("v1").as_uint();
			loot.GDMax = xmlItem.attribute("v2").as_uint();
		} else {
			loot.ExpDaysMin = xmlItem.attribute("v1").as_uint();
			loot.ExpDaysMax = xmlItem.attribute("v2").as_uint();
		}

		box.content.push_back(loot);
		xmlItem = xmlItem.next_sibling();
	}
	
	mysteryBoxes_.push_back(box);
	*out_box = &mysteryBoxes_.back();
	return 0;
}

int CClientUserProfile::ApiLootBoxBuy(int itemId, int buyIdx)
{
	r3d_assert(buyIdx > 0);
	if(RUS_CLIENT && buyIdx >= 1 && buyIdx <= 4) // move buy idx to gamenet range [9-12]
		buyIdx += 8;

	CWOBackendReq req(this, "api_MysteryBox.aspx");
	req.AddParam("func", "roll");
	req.AddParam("ItemID", itemId);
	req.AddParam("BuyIdx", buyIdx);
	if(!req.Issue())
	{
		r3dOutToLog("ApiLootBoxBuy FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d %d %d %d", 
		&lastMysteryWin_.ItemID, 
		&lastMysteryWin_.ExpDays, 
		&lastMysteryWin_.GD,
		&balance);
	if(nargs != 4)
	{
		r3dError("wrong answer for ApiMysteryBoxBuy");
		return 9;
	}
	
	// update balance
	if(buyIdx >= 5 && buyIdx <= 8)
		ProfileData.Stats.GameDollars = balance;
	else
		ProfileData.Stats.GamePoints  = balance;
		
	ProfileData.Stats.GameDollars += lastMysteryWin_.GD;

	return 0;
}

int CClientUserProfile::ApiLootBoxSell(int itemId)
{
	// make sure that we have price for selling loot box (permanent GD price in store)
	uint32 price = 0;
	for(uint32_t i=0; i<g_NumStoreItems; i++) {
		if(g_StoreItems[i].itemID == itemId) {
			price = g_StoreItems[i].gd_pricePerm;
			break;
		}
	}
	if(price == 0) {
#ifndef FINAL_BUILD
		r3dError("there is no sell price for lootbox\n");
#else
		return 6;
#endif
	}

	CWOBackendReq req(this, "api_MysteryBox.aspx");
	req.AddParam("func", "sell");
	req.AddParam("ItemID", itemId);
	if(!req.Issue())
	{
		r3dOutToLog("ApiLootBoxSell FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	ProfileData.Stats.GameDollars += price;

	lastMysteryWin_.GD     = price;
	lastMysteryWin_.ItemID = 0;
	return 0;
}

int CClientUserProfile::ApiWeaponAttachBuy(int WeaponID, int AttachID, int slot, int buyIdx)
{
	r3d_assert(buyIdx > 0);
	if(RUS_CLIENT && buyIdx >= 1 && buyIdx <= 4) // move buy idx to gamenet range [9-12]
		buyIdx += 8;

	CWOBackendReq req(this, "api_WeaponAttach.aspx");
	req.AddParam("func",     "buy");
	req.AddParam("WeaponID", WeaponID);
	req.AddParam("AttachID", AttachID);
	req.AddParam("Slot",     slot);
	req.AddParam("BuyIdx",   buyIdx);
	if(!req.Issue())
	{
		r3dOutToLog("ApiWeaponAttachBuy FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	//TODO: we added *AND EQUIPPED* new attachment to the weapon

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	if(nargs != 1)
	{
		r3dError("wrong answer for ApiWeaponAttachBuy");
		return 9;
	}

	if(buyIdx >= 5 && buyIdx <= 8)
		ProfileData.Stats.GameDollars = balance;
	else
		ProfileData.Stats.GamePoints  = balance;
		
	return 0;
}

int CClientUserProfile::ApiWeaponAttachEquip(int WeaponID, int AttachID, int slot)
{
	CWOBackendReq req(this, "api_WeaponAttach.aspx");
	req.AddParam("func",     "equip");
	req.AddParam("WeaponID", WeaponID);
	req.AddParam("AttachID", AttachID);
	req.AddParam("Slot",     slot);
	if(!req.Issue())
	{
		r3dOutToLog("ApiWeaponAttachEquip FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	//TODO: weapon attachment equipped
	
	return 0;
}

int CClientUserProfile::ApiWeaponAttachFixDefaults(int WeaponID)
{
	CWOBackendReq req(this, "api_WeaponAttach.aspx");
	req.AddParam("func",     "fix");
	req.AddParam("WeaponID", WeaponID);
	if(!req.Issue())
	{
		r3dOutToLog("ApiWeaponAttachFixDefaults FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	return 0;
}

int CClientUserProfile::ApiRetBonusGetInfo()
{
	CWOBackendReq req(this, "api_RetBonus.aspx");
	req.AddParam("func", "info");
	if(!req.Issue())
	{
		r3dOutToLog("ApiRetBonusGetInfo FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);

	pugi::xml_node xmlBonus = xmlFile.child("retbonus");
	curRetentionDays_ = xmlBonus.attribute("d").as_int();
	minutesToNextRetDay_ = xmlBonus.attribute("m").as_int();

	retentionBonusByDays_.clear();
	pugi::xml_node xmlDays = xmlBonus.child("days").first_child();
	while(!xmlDays.empty())
	{
		int bonus = xmlDays.attribute("b").as_int();
		retentionBonusByDays_.push_back(bonus);

		xmlDays = xmlDays.next_sibling();
	}
	if(curRetentionDays_ > (int)retentionBonusByDays_.size())
		curRetentionDays_ = (int)retentionBonusByDays_.size(); // equal size, as index is curRetentionDays_-1
	
	return 0;
}

int CClientUserProfile::ApiRetBonusClaim()
{
	CWOBackendReq req(this, "api_RetBonus.aspx");
	req.AddParam("func", "give");
	if(!req.Issue())
	{
		r3dOutToLog("ApiRetBonusClaim FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	if(nargs != 1)
	{
		r3dError("wrong answer for ApiRetBonusClaim");
		return 9;
	}

	// update balance
	if(RUS_CLIENT)
		ProfileData.Stats.GameDollars = balance;
	else
		ProfileData.Stats.GamePoints  = balance;
	
	return 0;
}

int CClientUserProfile::ApiUpdateAchievements(int numAchs, wiAchievement* achs)
{
	if(numAchs>0)
	{
		CWOBackendReq req(this, "api_ClientUpdateAchievements.aspx");
		req.AddParam("NumAch",  numAchs);
		char achIDstr[32];
		char achValstr[32];
		char achUnlstr[32];
		for(int i=0; i<numAchs; ++i)
		{
			sprintf(achIDstr, "AchID%d", i);
			sprintf(achValstr, "AchVal%d", i);
			sprintf(achUnlstr, "AchUnl%d", i);
			req.AddParam(achIDstr, achs[i].ID);
			req.AddParam(achValstr, achs[i].value);
			req.AddParam(achUnlstr, achs[i].unlocked);
		}
		if(!req.Issue())
		{
			r3dOutToLog("ApiUpdateAchievements FAILED, code: %d\n", req.resultCode_);
			return req.resultCode_;
		}
	}

	return 0;
}

void CClientUserProfile::RecordFrontEndAchievements()
{
	wiAchievement achievementsToRecord[NUM_ACHIEVEMENTS];
	int numAchievements = 0;
	for( int achievementIndex = FIRST_FRONT_END_ACHIEVEMENT; achievementIndex < NUM_ACHIEVEMENTS; achievementIndex++ )
	{
		const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( achievementIndex );
		wiAchievement* achievementData = ProfileData.getAchievementDataByID( achievementInfo->id );
		if ( achievementData != NULL && achievementData->dirty ) {
			achievementsToRecord[ numAchievements ] = *achievementData;
			numAchievements++;
			achievementData->dirty = false;
		}
	}
	if ( numAchievements > 0 ) {
		ApiUpdateAchievements( numAchievements, achievementsToRecord);
	}
}

#endif // ! WO_SERVER
