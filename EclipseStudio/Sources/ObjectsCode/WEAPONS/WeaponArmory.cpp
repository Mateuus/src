#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "WeaponArmory.h"

#ifndef WO_SERVER
  #include "Ammo.h"
  #include "Weapon.h"
  #include "Gear.h"
#else
  #include "ServerWeapons/ServerAmmo.h"
  #include "ServerWeapons/ServerWeapon.h"
  #include "ServerWeapons/ServerGear.h"
#endif

extern int RUS_CLIENT;

WeaponArmory gWeaponArmory;


WeaponArmory::WeaponArmory()
{
	memset(m_AmmoArray, 0, sizeof(Ammo*)*MAX_NUMBER_AMMO);
	m_NumAmmoLoaded = 0;
	memset(m_WeaponArray, 0, sizeof(WeaponConfig*)*MAX_NUMBER_WEAPONS);
	m_NumWeaponsLoaded = 0;
	memset(m_WeaponAttmArray, 0, sizeof(WeaponAttachmentConfig*)*MAX_NUMBER_WEAPON_ATTACHMENTS);
	m_NumWeaponAttmLoaded = 0;
	memset(m_GearArray, 0, sizeof(GearConfig*)*MAX_NUMBER_GEAR);
	m_NumGearLoaded = 0;
	memset(m_ItemArray, 0, sizeof(ItemConfig*)*MAX_NUMBER_ITEM);
	m_NumItemLoaded = 0;
	memset(m_PackageArray, 0, sizeof(PackageConfig*)*MAX_NUMBER_PACKAGE);
	m_NumPackageLoaded = 0;
	memset(m_ScopeArray, 0, sizeof(ScopeConfig*)*MAX_NUMBER_SCOPE);
	m_NumScopeLoaded = 0;
	memset(m_AchievementArray, 0, sizeof(AchievementConfig*)*MAX_NUMBER_ACHIEVEMENT);
	m_NumAchievementLoaded = 0;
}

WeaponArmory::~WeaponArmory()
{
	r3d_assert(m_WeaponArray[0]==NULL);
	r3d_assert(m_NumWeaponsLoaded==0);
	r3d_assert(m_WeaponAttmArray[0]==NULL);
	r3d_assert(m_NumWeaponAttmLoaded==0);
	r3d_assert(m_GearArray[0]==NULL);
	r3d_assert(m_NumGearLoaded==0);
	r3d_assert(m_AmmoArray[0]==NULL);
	r3d_assert(m_NumAmmoLoaded==0);
	r3d_assert(m_ItemArray[0]==NULL);
	r3d_assert(m_NumItemLoaded==0);
	r3d_assert(m_PackageArray[0]==NULL);
	r3d_assert(m_NumPackageLoaded==0);
	r3d_assert(m_ScopeArray[0]==NULL);
	r3d_assert(m_NumScopeLoaded==0);
	r3d_assert(m_AchievementArray[0]==NULL);
	r3d_assert(m_NumAchievementLoaded==0);
}

bool WeaponArmory::Init()
{
	InitRewards();

	// material library will kill materials in previously loaded weapon meshes, so reload it
	Destroy();

	r3d_assert(m_WeaponArray[0] == NULL);
	r3d_assert(m_NumWeaponsLoaded == 0);
	r3d_assert(m_WeaponAttmArray[0]==NULL);
	r3d_assert(m_NumWeaponAttmLoaded==0);
	r3d_assert(m_AmmoArray[0]==NULL);
	r3d_assert(m_NumAmmoLoaded==0);
	r3d_assert(m_GearArray[0]==NULL);
	r3d_assert(m_NumGearLoaded==0);
	r3d_assert(m_ItemArray[0]==NULL);
	r3d_assert(m_NumItemLoaded==0);
	r3d_assert(m_PackageArray[0]==NULL);
	r3d_assert(m_NumPackageLoaded==0);
	r3d_assert(m_ScopeArray[0]==NULL);
	r3d_assert(m_NumScopeLoaded==0);
	r3d_assert(m_AchievementArray[0]==NULL);
	r3d_assert(m_NumAchievementLoaded==0);

	// load game stuff
	{
		const char* GameDBFile = "Data/Weapons/gameDB.xml";

		r3dFile* f = r3d_open(GameDBFile, "rb");
		if ( !f )
		{
			r3dError("Failed to open %s\n", GameDBFile);
			return false;
		}

		char* fileBuffer = new char[f->size + 1];
		r3d_assert(fileBuffer);
		fread(fileBuffer, f->size, 1, f);
		fileBuffer[f->size] = 0;

		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(fileBuffer, f->size);
		fclose(f);
		if(!parseResult)
			r3dError("Failed to parse XML, error: %s", parseResult.description());

		pugi::xml_node xmlAchievement = xmlFile.child("AchievementDB").child("Achievement");
		while(!xmlAchievement.empty())
		{
			loadAchievement(xmlAchievement);
			xmlAchievement = xmlAchievement.next_sibling();
			if(strcmp(xmlAchievement.name(), "Achievement")!=0)
				break;
		}


		if( NUM_ACHIEVEMENTS != m_NumAchievementLoaded ) {
			r3dError(" Achievement count mismatch.");
		}

		//we don't want to start the server if we have 
		r3d_assert( NUM_ACHIEVEMENTS == m_NumAchievementLoaded );

		// delete only after we are done parsing xml!
		delete [] fileBuffer;
	}

	// load ammo firstly
	{
		r3dFile* f = r3d_open("Data/Weapons/AmmoDB.xml", "rb");
		if ( !f )
		{
			r3dOutToLog("Failed to open Data/Weapons/AmmoDB.xml\n");
			return false;
		}

		char* fileBuffer = new char[f->size + 1];
		r3d_assert(fileBuffer);
		fread(fileBuffer, f->size, 1, f);
		fileBuffer[f->size] = 0;

		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(fileBuffer, f->size);
		fclose(f);
		if(!parseResult)
			r3dError("Failed to parse XML, error: %s", parseResult.description());
		pugi::xml_node xmlArmory = xmlFile.child("AmmoArmory");
		pugi::xml_node xmlAmmo = xmlArmory.child("Ammo");
		while(!xmlAmmo.empty())
		{
			loadAmmo(xmlAmmo);
			xmlAmmo = xmlAmmo.next_sibling();
			if(strcmp(xmlAmmo.name(), "Ammo")!=0)
				break;
		}

		// load scopes
		pugi::xml_node xmlScope = xmlArmory.child("Scope");
		while(!xmlScope.empty())
		{
			loadScope(xmlScope);
			xmlScope = xmlScope.next_sibling();
			if(strcmp(xmlScope.name(), "Scope")!=0)
				break;
		}

		// delete only after we are done parsing xml!
		delete [] fileBuffer;
	}

	const char* ItemsDBFile = "Data/Weapons/itemsDB.xml";
	if(RUS_CLIENT)
		ItemsDBFile = "Data/Weapons/itemsDB_RU.xml";
	
	r3dFile* f = r3d_open(ItemsDBFile, "rb");
	if ( !f )
	{
		r3dError("Failed to open %s\n", ItemsDBFile);
		return false;
	}

	char* fileBuffer = new char[f->size + 1];
	r3d_assert(fileBuffer);
	fread(fileBuffer, f->size, 1, f);
	fileBuffer[f->size] = 0;
	pugi::xml_document xmlFile;
	pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(fileBuffer, f->size);
	fclose(f);
	if(!parseResult)
		r3dError("Failed to parse XML, error: %s", parseResult.description());
	pugi::xml_node xmlDB = xmlFile.child("DB");
	{
		pugi::xml_node xmlArmory = xmlDB.child("WeaponsArmory");
		pugi::xml_node xmlWeapon = xmlArmory.child("Weapon");
		while(!xmlWeapon.empty())
		{
			loadWeapon(xmlWeapon);
			xmlWeapon = xmlWeapon.next_sibling();
		}
	}
	{
		pugi::xml_node xmlArmory = xmlDB.child("GearArmory");
		pugi::xml_node xmlGear = xmlArmory.child("Gear");
		while(!xmlGear.empty())
		{
			loadGear(xmlGear);
			xmlGear = xmlGear.next_sibling();
		}
	}
	{
		pugi::xml_node xmlItems = xmlDB.child("ItemsDB");
		pugi::xml_node xmlItem = xmlItems.child("Item");
		while(!xmlItem.empty())
		{
			loadItem(xmlItem);
			xmlItem = xmlItem.next_sibling();
		}
	}
	{
		pugi::xml_node xmlAttachments = xmlDB.child("AttachmentArmory");
		pugi::xml_node xmlAttm = xmlAttachments.child("Attachment");
		while(!xmlAttm.empty())
		{
			loadWeaponAttachment(xmlAttm);
			xmlAttm = xmlAttm.next_sibling();
		}
	}
	{
		pugi::xml_node xmlItems = xmlDB.child("PackagesDB");
		pugi::xml_node xmlItem = xmlItems.child("Package");
		while(!xmlItem.empty())
		{
			loadPackage(xmlItem);
			xmlItem = xmlItem.next_sibling();
		}
	}

	// delete only after we are done parsing xml!
	delete [] fileBuffer;

	return true;
}

void WeaponArmory::LevelUpBonus::setReward(int level, int _gp, int _gd, int _sp, const char* _reward1, const char* _reward2, const char* _reward3,
						uint32_t it0, uint32_t it1, uint32_t it2, uint32_t it3, uint32_t it4, uint32_t it5, uint32_t it6, uint32_t it7, uint32_t it8)
{ 
	nextLevel = level;
	gp = _gp; 
	gd = _gd; 
	sp = _sp;
	reward1 = strdup(_reward1); 
	reward2 = strdup(_reward2); 
	reward3 = strdup(_reward3);
	items[0] = it0;
	items[1] = it1;
	items[2] = it2;
	items[3] = it3;
	items[4] = it4;
	items[5] = it5;
	items[6] = it6;
	items[7] = it7;
}

void WeaponArmory::InitRewards()
{
	if(m_LevelUpBonus[0].nextLevel != -1)
		return;

	int c = 0;
	if(!RUS_CLIENT)
	{
		// US rewards
		// for first 10 levels: user should get at least ONE gun for exit screen on end of round to work correctly!
		m_LevelUpBonus[c++].setReward(2, 0, 0, 0, "5 day rental: Blackops crate key", "1 day rental: IMI Tavor", "", 301000, 101173);
		m_LevelUpBonus[c++].setReward(3, 0, 0, 0, "7 day rental: Camera Drone", "2 day rental: Honey Badger, 1 day rental: 2X WP", "", WeaponConfig::ITEMID_Cypher2, 301003, 101106);
		m_LevelUpBonus[c++].setReward(4, 0, 0, 0, "1 day rental: G35 Elite", "", "", 101218);
		m_LevelUpBonus[c++].setReward(5, 0, 0, 0, "2 day rental: Desert Eagle", "", "", 101180);
		m_LevelUpBonus[c++].setReward(6, 0, 0, 0, "3 day rental: Big Surprise ability", "", "", 301067);
		m_LevelUpBonus[c++].setReward(7, 0, 0, 0, "2 day rental: Pecheneg", "", "", 101092);
		m_LevelUpBonus[c++].setReward(8, 0, 0, 0, "1 day rental: Famas F1", "", "", 101037);
		m_LevelUpBonus[c++].setReward(9, 0, 0, 0, "1 day rental: Bizon", "", "", 101109);
		m_LevelUpBonus[c++].setReward(10, 1000, 10000, 0, "1000 GC, 10000 War Points", "1 day rental: 2x WP Boost, 2x XP boost", "2 day rental: Sig 516 Elite", 301003, 301001, 101215);
		m_LevelUpBonus[c++].setReward(11, 0, 0, 0, "3 day rental: QLB 06", "", "", 101202);
		m_LevelUpBonus[c++].setReward(12, 0, 0, 0, "3 day rental: USAS-12", "", "", 101200);
		m_LevelUpBonus[c++].setReward(13, 0, 0, 0, "1 day rental: M 202 Flash", "", "", 101189);
		m_LevelUpBonus[c++].setReward(14, 0, 0, 0, "2 day rental: VSS Vintorez", "", "", 101084);
		m_LevelUpBonus[c++].setReward(15, 0, 10000, 0, "10000 War Points", "3 day rental: M249 Elite, Bizon Elite, SCAR Elite", "1 day rental: All weapons", 101214, 101227, 101219, 301106);
		m_LevelUpBonus[c++].setReward(16, 0, 0, 0, "3 day rental: Mauser HP50", "", "", 101087);
		m_LevelUpBonus[c++].setReward(17, 0, 0, 0, "3 day rental: TR7 SMG", "", "", 101063);
		m_LevelUpBonus[c++].setReward(18, 0, 0, 0, "3 day rental: Honey Badger", "", "", 101106);
		m_LevelUpBonus[c++].setReward(19, 0, 0, 0, "3 day rental: FN P90S", "", "", 101246);
		m_LevelUpBonus[c++].setReward(20, 0, 10000, 0, "10000 War Points", "1 day rental: 2x WP Boost, 2x XP boost", "1 day rental: All weapons", 301003, 301001, 301106);
		m_LevelUpBonus[c++].setReward(25, 0, 0, 0, "5 day rental: All weapons", "", "", 301106);
		m_LevelUpBonus[c++].setReward(30, 0, 50000, 0, "50000 War Points", "1 day rental: 2x WP Boost, 2x XP boost", "", 301003, 301001);
		m_LevelUpBonus[c++].setReward(35, 0, 15000, 0, "15000 War Points", "", "");
		m_LevelUpBonus[c++].setReward(40, 0, 50000, 0, "50000 War Points", "2 day rental: 2x WP Boost, 2x XP boost", "", 301003, 301001);
		m_LevelUpBonus[c++].setReward(45, 0, 15000, 0, "15000 War Points", "7 day rental: All weapons", "", 301106);
		m_LevelUpBonus[c++].setReward(50, 0, 100000, 0, "100000 War Points", "3 day rental: 2x WP Boost", "", 301003);
		m_LevelUpBonus[c++].setReward(55, 0, 0, 0, "2 day rental: 2x WP Boost", "", "", 301003);
		m_LevelUpBonus[c++].setReward(60, 25000, 200000, 0, "25000 GC, 200000 War Points", "30 day rental: All weapons", "", 301106);
		// end of US rewards
	}
	else
	{
		// russian rewards
		m_LevelUpBonus[c++].setReward(2, 0, 1000, 0, "", "", "", 101037, 101137, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(3, 0, 1000, 0, "", "", "", 101004, 101136, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(4, 0, 1000, 0, "", "", "", 20066, 0, 0, 20013, 101005);
		m_LevelUpBonus[c++].setReward(5, 0, 5000, 0, "", "", "", 101060, 101115, 301001, 0, 0);
		m_LevelUpBonus[c++].setReward(6, 0, 1500, 0, "", "", "", 101169, 301003, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(7, 0, 1500, 0, "", "", "", 101139, 101103, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(8, 0, 1500, 0, "", "", "", 101137, 101109, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(9, 0, 1500, 0, "", "", "", 101108, 101267, 101150, 0, 0);
		m_LevelUpBonus[c++].setReward(10, 0, 7500, 1, "", "", "", 101037, 20048, 20040, 101109, 301001);
		m_LevelUpBonus[c++].setReward(11, 0, 1500, 0, "", "", "", 101084, 20040, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(12, 0, 1500, 0, "", "", "", 101130, 20105, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(13, 0, 1500, 0, "", "", "", 20107, 301001, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(14, 0, 1500, 1, "", "", "", 101200, 20091, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(15, 0, 10000, 2, "", "", "", 20088, 101232, 101139, 301001, 0);
		m_LevelUpBonus[c++].setReward(16, 0, 2000, 0, "", "", "", 101095, 20071, 20056, 0, 0);
		m_LevelUpBonus[c++].setReward(17, 0, 2000, 0, "", "", "", 101189, 301001, 301003, 0, 0);
		m_LevelUpBonus[c++].setReward(18, 0, 3000, 0, "", "", "", 301062, 301061, 101224, 0, 0);
		m_LevelUpBonus[c++].setReward(19, 0, 3000, 0, "", "", "", 301067, 301070, 101210, 0, 0);
		m_LevelUpBonus[c++].setReward(20, 0, 15000, 2, "", "", "", 101140, 101085, 101136, 0, 301001);
		m_LevelUpBonus[c++].setReward(21, 0, 3000, 0, "", "", "", 101106, 20067, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(22, 0, 3000, 0, "", "", "", 101202, 20011, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(23, 0, 3000, 0, "", "", "", 101245, 301003, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(24, 0, 3000, 1, "", "", "", 20106, 101064, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(25, 0, 15000, 3, "", "", "", 101077, 101087, 20075, 20073, 301001);
		m_LevelUpBonus[c++].setReward(26, 0, 3000, 0, "", "", "", 20065, 20066, 101032, 0, 0);
		m_LevelUpBonus[c++].setReward(27, 0, 3000, 0, "", "", "", 301001, 101107, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(28, 0, 3000, 0, "", "", "", 101183, 20081, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(29, 0, 3000, 1, "", "", "", 20061, 20049, 101201, 0, 0);
		m_LevelUpBonus[c++].setReward(30, 0, 25000, 4, "", "", "", 101027, 101137, 20075, 20073, 301001);
		m_LevelUpBonus[c++].setReward(31, 0, 5000, 0, "", "", "", 101063, 101008, 20016, 0, 0);
		m_LevelUpBonus[c++].setReward(32, 0, 5000, 0, "", "", "", 20057, 301001, 101232, 0, 0);
		m_LevelUpBonus[c++].setReward(33, 0, 5000, 0, "", "", "", 101098, 20061, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(34, 0, 5000, 1, "", "", "", 20037, 101173, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(35, 0, 20000, 3, "", "", "", 101077, 101109, 20066, 20013, 301001);
		m_LevelUpBonus[c++].setReward(36, 0, 6000, 0, "", "", "", 20040, 101221, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(37, 0, 6000, 0, "", "", "", 101158, 20081, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(38, 0, 6000, 0, "", "", "", 101267, 101055, 20107, 0, 0);
		m_LevelUpBonus[c++].setReward(39, 0, 6000, 1, "", "", "", 101217, 20014, 20022, 0, 0);
		m_LevelUpBonus[c++].setReward(40, 0, 25000, 3, "", "", "", 101215, 101227, 20066, 20057, 301001);
		m_LevelUpBonus[c++].setReward(41, 0, 7000, 0, "", "", "", 101173, 101008, 301063, 0, 0);
		m_LevelUpBonus[c++].setReward(42, 0, 7000, 0, "", "", "", 301067, 301070, 101130, 0, 0);
		m_LevelUpBonus[c++].setReward(43, 0, 7000, 0, "", "", "", 101171, 101084, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(44, 0, 7000, 1, "", "", "", 101227, 101137, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(45, 0, 25000, 2, "", "", "", 101088, 101215, 20016, 20011, 301001);
		m_LevelUpBonus[c++].setReward(46, 0, 7000, 0, "", "", "", 101158, 101191, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(47, 0, 7000, 0, "", "", "", 101088, 101106, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(48, 0, 7000, 0, "", "", "", 101109, 20065, 20053, 0, 0);
		m_LevelUpBonus[c++].setReward(49, 0, 7000, 1, "", "", "", 101093, 20066, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(50, 0, 35000, 2, "", "", "", 101088, 301001, 101227, 101137, 20017);
		m_LevelUpBonus[c++].setReward(51, 0, 7000, 0, "", "", "", 101040, 101008, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(52, 0, 7000, 0, "", "", "", 101108, 20072, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(53, 0, 7000, 0, "", "", "", 101216, 20105, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(54, 0, 7000, 0, "", "", "", 20043, 20011, 101005, 0, 0);
		m_LevelUpBonus[c++].setReward(55, 0, 50000, 2, "", "", "", 101064, 101087, 20003, 20024, 301001);
		m_LevelUpBonus[c++].setReward(56, 0, 9000, 0, "", "", "", 101183, 101196, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(57, 0, 9000, 0, "", "", "", 20029, 101242, 0, 0, 0);
		m_LevelUpBonus[c++].setReward(58, 0, 9000, 0, "", "", "", 20011, 301001, 20066, 101215, 101131);
		m_LevelUpBonus[c++].setReward(59, 0, 9000, 1, "", "", "", 101064, 20061, 20092, 0, 0);
		m_LevelUpBonus[c++].setReward(60, 0, 150000, 0, "", "", "", 101242, 101227, 101088, 101221, 101092);
		// end of russian rewards
	}
	
	r3d_assert(c <= MAX_NUM_RANKS);
	return;
}

bool WeaponArmory::loadAchievement(pugi::xml_node& xmlAchievement)
{
	r3d_assert(!xmlAchievement.empty());

	const char* name = xmlAchievement.attribute("name").value();
	for(uint32_t i=0; i<m_NumAchievementLoaded; ++i) // todo: change to hash table
	{
		if(strcmp(m_AchievementArray[i]->name, name)==0)
		{
			r3dArtBug("Trying to load an achievement '%s' that is already loaded!", name);
			return false;
		}
	}
	if(m_NumAchievementLoaded > MAX_NUMBER_ACHIEVEMENT-1)
	{
		r3dArtBug("Trying to load more than maximum number of achievement. Maximum is '%d'\n", MAX_NUMBER_ACHIEVEMENT);
		return false;
	}
	
	AchievementConfig* ach= new AchievementConfig(xmlAchievement.attribute("id").as_int());
	ach->enabled = ( xmlAchievement.attribute("enabled").as_int() == 1 );
	ach->name = strdup(xmlAchievement.attribute("name").value());
	ach->desc= strdup(xmlAchievement.attribute("desc").value());
	ach->hud_icon = strdup(xmlAchievement.attribute("hudIcon").value());
	ach->value = xmlAchievement.attribute("value").as_int();

	m_AchievementArray[m_NumAchievementLoaded] = ach;
	m_NumAchievementLoaded++;

	return true;
}

bool WeaponArmory::loadAmmo(pugi::xml_node& xmlAmmo)
{
	r3d_assert(!xmlAmmo.empty());

	const char* ammoName = xmlAmmo.attribute("name").value();
	// check if we have that ammo in our database
	for(uint32_t i=0; i<m_NumAmmoLoaded; ++i) // todo: change to hash table
	{
		if(strcmp(m_AmmoArray[i]->m_Name, ammoName)==0)
		{
			r3dArtBug("Trying to load an ammo '%s' that is already loaded!", ammoName);
			return false;
		}
	}
	if(m_NumAmmoLoaded > MAX_NUMBER_AMMO-2)
	{
		r3dArtBug("Trying to load more than maximum number of ammo. Maximum is '%d'\n", MAX_NUMBER_AMMO);
		return false;
	}
	Ammo* ammo = new Ammo(ammoName);

	const char* particleTracer = xmlAmmo.child("ParticleTracer").attribute("file").value();
	if(particleTracer && particleTracer[0]!=0)
		ammo->m_ParticleTracer = strdup(particleTracer);

	const char* particleShellExtract = xmlAmmo.child("ShellExtractParticle").attribute("file").value();
	if(particleShellExtract && particleShellExtract[0]!=0)
		ammo->m_ShellExtractParticle = strdup(particleShellExtract);

	const char* modelName = xmlAmmo.child("Model").attribute("file").value();
	if(modelName && modelName[0]!=0)
		ammo->m_ModelPath = strdup(modelName);

	ammo->m_DecalSource = strdup(xmlAmmo.child("Decal").attribute("source").value());
	ammo->m_BulletClass = strdup(xmlAmmo.child("Bullet").attribute("class").value());

	m_AmmoArray[m_NumAmmoLoaded] = ammo;
	m_NumAmmoLoaded++;

	return true;
}

bool WeaponArmory::loadScope(pugi::xml_node& xmlScope)
{
	r3d_assert(!xmlScope.empty());

	const char* scopeName = xmlScope.attribute("name").value();
	// check if we have that scope in our database
	for(uint32_t i=0; i<m_NumScopeLoaded; ++i) // todo: change to hash table
	{
		if(strcmp(m_ScopeArray[i]->name, scopeName)==0)
		{
			r3dError("Trying to load a scope '%s' that is already loaded!", scopeName);
			return false;
		}
	}
	if(m_NumScopeLoaded > MAX_NUMBER_SCOPE-1)
	{
		r3dError("Trying to load more than maximum number of scope. Maximum is '%d'\n", MAX_NUMBER_SCOPE);
		return false;
	}
	ScopeConfig* scope = new ScopeConfig(scopeName);
#ifndef WO_SERVER
	const char* maskTex = xmlScope.attribute("scopeMask").value();
	if(maskTex && maskTex[0]!=0)
		scope->scope_mask = r3dRenderer->LoadTexture(maskTex);

	const char* blurTex = xmlScope.attribute("scopeBlurMask").value();
	if(blurTex && blurTex[0]!=0)
		scope->scopeBlur_mask = r3dRenderer->LoadTexture(blurTex);
	
	const char* reticuleTex = xmlScope.attribute("scopeReticule").value();
	if(reticuleTex && reticuleTex[0]!=0)
		scope->scope_reticle = r3dRenderer->LoadTexture(reticuleTex);

	const char* normalTex = xmlScope.attribute("scopeNormal").value();
	if(normalTex && normalTex[0]!=0)
		scope->scope_normal = r3dRenderer->LoadTexture(normalTex);

	reticuleTex = xmlScope.attribute("reticule").value();
	if(reticuleTex && reticuleTex[0]!=0)
		scope->reticule = r3dRenderer->LoadTexture(reticuleTex);

	scope->hasScopeMode = xmlScope.attribute("hasScope").as_bool();

	pugi::xml_attribute lighting = xmlScope.attribute("lighting") ;

	if( !lighting.empty() )
	{
		scope->lighting = lighting.as_bool() ;
	}
	else
	{
		scope->lighting = true ;
	}
	
	pugi::xml_attribute hide_player = xmlScope.attribute("hide_player") ;

	if( !hide_player.empty() )
	{
		scope->hide_player_model = hide_player.as_bool() ;
	}
	else
	{
		scope->hide_player_model = true ;
	}

	if(scope->hasScopeMode)
	{
		r3d_assert(scope->scope_mask);
		r3d_assert(scope->scopeBlur_mask);
		r3d_assert(scope->scope_reticle);
	}
#endif
	m_ScopeArray[m_NumScopeLoaded] = scope;
	m_NumScopeLoaded++;

	return true;
}

bool WeaponArmory::loadWeapon(pugi::xml_node& xmlWeapon)
{
	r3d_assert(!xmlWeapon.empty());

	uint32_t itemID = xmlWeapon.attribute("itemID").as_uint();
	// check if we have that weapon in our database
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i) // todo: change to hash table
	{
		if(m_WeaponArray[i]->m_itemID == itemID)
		{
			r3dArtBug("Trying to load a weapon with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumWeaponsLoaded > MAX_NUMBER_WEAPONS-2)
	{
		r3dArtBug("Trying to load more than maximum number of weapons. Maximum is '%d'\n", MAX_NUMBER_WEAPONS);
		return false;
	}

	WeaponConfig* weapon = new WeaponConfig(itemID);
	
	// load base stuff, common for all versions
	if(!weapon->loadBaseFromXml(xmlWeapon))
	{
		delete weapon;
		return false;
	}

	// things specific to game version of WeaponArmory
	const char* bulletName = xmlWeapon.child("PrimaryFire").attribute("bullet").value();
	weapon->m_PrimaryAmmo = getAmmo(bulletName);
	if(weapon->m_PrimaryAmmo == NULL)
	{
		r3dArtBug("Failed to find ammo '%s'. Make sure that you added it into AmmoDB.xml\n", bulletName);
		delete weapon;
		return false;
	}

	weapon->m_scopeConfig = getScopeConfig(xmlWeapon.child("PrimaryFire").attribute("ScopeType").value());
	if(weapon->m_scopeConfig == 0)
	{
		r3dArtBug("Weapon '%s' has no scope config!!\n", weapon->m_StoreName);
		delete weapon;
		return false;
	}

#ifndef WO_SERVER
	weapon->m_sndFireID = SoundSys.GetEventIDByPath(xmlWeapon.child("Sound").attribute("shoot").value());
	char tempStr[512];
	sprintf(tempStr, "%s_distant", xmlWeapon.child("Sound").attribute("shoot").value());
	weapon->m_sndFireDistantID = SoundSys.GetEventIDByPath(tempStr);
	weapon->m_sndReloadID = SoundSys.GetEventIDByPath(xmlWeapon.child("Sound").attribute("reload").value());

	// new weapon sounds
	{
		char tmpStr[512];
		sprintf(tmpStr, "%s_single", xmlWeapon.child("Sound").attribute("shoot").value());
		weapon->m_sndFireID_single = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_auto", xmlWeapon.child("Sound").attribute("shoot").value());
		weapon->m_sndFireID_auto = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_single_Player", xmlWeapon.child("Sound").attribute("shoot").value());
		weapon->m_sndFireID_single_player = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_auto_Player", xmlWeapon.child("Sound").attribute("shoot").value());
		weapon->m_sndFireID_auto_player = SoundSys.GetEventIDByPath(tmpStr);
	}
#endif

	m_WeaponArray[m_NumWeaponsLoaded] = weapon;
	m_NumWeaponsLoaded++;

	return true;
}

bool WeaponArmory::loadWeaponAttachment(pugi::xml_node& xmlWeaponAttachment)
{
	r3d_assert(!xmlWeaponAttachment.empty());

	uint32_t itemID = xmlWeaponAttachment.attribute("itemID").as_uint();
	// check if we have that weapon in our database
	for(uint32_t i=0; i<m_NumWeaponAttmLoaded; ++i) // todo: change to hash table
	{
		if(m_WeaponAttmArray[i]->m_itemID == itemID)
		{
			r3dArtBug("Trying to load a weapon attachment with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumWeaponAttmLoaded > MAX_NUMBER_WEAPON_ATTACHMENTS-2)
	{
		r3dArtBug("Trying to load more than maximum number of weapon attachments. Maximum is '%d'\n", MAX_NUMBER_WEAPON_ATTACHMENTS);
		return false;
	}

	WeaponAttachmentConfig* attm = new WeaponAttachmentConfig(itemID);

	// load base stuff, common for all versions
	if(!attm->loadBaseFromXml(xmlWeaponAttachment))
	{
		delete attm;
		return false;
	}

	const char* scopeType = xmlWeaponAttachment.child("Upgrade").attribute("ScopeType").value();
	attm->m_scopeConfig = getScopeConfig(scopeType);
	{
		size_t scopeTypeLen = strlen(scopeType);
		if(scopeTypeLen>4 && strcmp(&scopeType[scopeTypeLen-4], "_fps")==0)
		{
			char tmpStr[64] = {0};
			memcpy(tmpStr, scopeType, strlen(scopeType)-4);
			attm->m_scopeConfigTPS = getScopeConfig(tmpStr);
		}
		if(attm->m_scopeConfigTPS == NULL)
			attm->m_scopeConfigTPS = attm->m_scopeConfig;
	}
#ifndef WO_SERVER
	// new weapon sounds
	{
		char tmpStr[512];
		sprintf(tmpStr, "%s_single", xmlWeaponAttachment.child("Model").attribute("FireSound").value());
		attm->m_sndFireID_single = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_auto", xmlWeaponAttachment.child("Model").attribute("FireSound").value());
		attm->m_sndFireID_auto = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_single_Player", xmlWeaponAttachment.child("Model").attribute("FireSound").value());
		attm->m_sndFireID_single_player = SoundSys.GetEventIDByPath(tmpStr);
		sprintf(tmpStr, "%s_auto_Player", xmlWeaponAttachment.child("Model").attribute("FireSound").value());
		attm->m_sndFireID_auto_player = SoundSys.GetEventIDByPath(tmpStr);
	}
#endif

	m_WeaponAttmArray[m_NumWeaponAttmLoaded] = attm;
	m_NumWeaponAttmLoaded++;

	return true;
}

bool WeaponArmory::loadGear(pugi::xml_node& xmlGear)
{
	r3d_assert(!xmlGear.empty());

	uint32_t itemID = xmlGear.attribute("itemID").as_uint();
	for(uint32_t i=0; i<m_NumGearLoaded; ++i) // todo: change to hash table
	{
		if(m_GearArray[i]->m_itemID == itemID)
		{
			r3dArtBug("Trying to load a gear with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumGearLoaded > MAX_NUMBER_GEAR-2)
	{
		r3dArtBug("Trying to load more than maximum number of gear. Maximum is '%d'\n", MAX_NUMBER_GEAR);
		return false;
	}

	GearConfig* gear = new GearConfig(itemID);
	// load base stuff, common for all versions
	if(!gear->loadBaseFromXml(xmlGear))
	{
		delete gear;
		return false;
	}

	m_GearArray[m_NumGearLoaded] = gear;
	m_NumGearLoaded++;

	return true;
}

bool WeaponArmory::loadItem(pugi::xml_node& xmlItem)
{
	r3d_assert(!xmlItem.empty());

	uint32_t itemID = xmlItem.attribute("itemID").as_uint();
	for(uint32_t i=0; i<m_NumItemLoaded; ++i) // todo: change to hash table
	{
		if(m_ItemArray[i]->m_itemID == itemID)
		{
			r3dArtBug("Trying to load an item with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumItemLoaded > MAX_NUMBER_ITEM-1)
	{
		r3dArtBug("Trying to load more than maximum number of items. Maximum is '%d'\n", MAX_NUMBER_ITEM);
		return false;
	}

	ItemConfig* item = new ItemConfig(itemID);
	item->category = (STORE_CATEGORIES)xmlItem.attribute("category").as_int();

	const char* desc = xmlItem.child("Store").attribute("desc").value();
	r3d_assert(desc);
	item->m_Description = strdup(desc);
	item->m_StoreIcon = strdup(xmlItem.child("Store").attribute("icon").value());
	item->m_StoreName = strdup(xmlItem.child("Store").attribute("name").value());
	item->m_LevelRequired = xmlItem.child("Store").attribute("LevelRequired").as_int();
	item->m_StoreNameW = wcsdup(utf8ToWide(item->m_StoreName));
	item->m_DescriptionW = wcsdup(utf8ToWide(item->m_Description));

	if(!xmlItem.child("Model").empty())
		item->m_ModelPath = strdup(xmlItem.child("Model").attribute("file").value());

	m_ItemArray[m_NumItemLoaded] = item;
	m_NumItemLoaded++;

	return true;
}

bool WeaponArmory::loadPackage(pugi::xml_node& xmlPackage)
{
	r3d_assert(!xmlPackage.empty());

	uint32_t itemID = xmlPackage.attribute("itemID").as_uint();
	for(uint32_t i=0; i<m_NumPackageLoaded; ++i) // todo: change to hash table
	{
		if(m_PackageArray[i]->m_itemID == itemID)
		{
			r3dArtBug("Trying to load package with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumPackageLoaded> MAX_NUMBER_PACKAGE-1)
	{
		r3dArtBug("Trying to load more than maximum number of packages. Maximum is '%d'\n", MAX_NUMBER_PACKAGE);
		return false;
	}

	PackageConfig* pkg = new PackageConfig(itemID);
	pkg->category = (STORE_CATEGORIES)xmlPackage.attribute("category").as_int();

	const char* desc = xmlPackage.child("Store").attribute("desc").value();
	r3d_assert(desc);
	pkg->m_Description = strdup(desc);
	pkg->m_StoreIcon = strdup(xmlPackage.child("Store").attribute("icon").value());
	pkg->m_StoreName = strdup(xmlPackage.child("Store").attribute("name").value());
	pkg->m_LevelRequired = xmlPackage.child("Store").attribute("LevelRequired").as_int();
	pkg->m_StoreNameW = wcsdup(utf8ToWide(pkg->m_StoreName));
	pkg->m_DescriptionW = wcsdup(utf8ToWide(pkg->m_Description));

	pkg->m_addGD = xmlPackage.child("PackageDesc").attribute("gp").as_uint();
	pkg->m_addSP = xmlPackage.child("PackageDesc").attribute("sp").as_uint();

	pkg->m_itemID1 = xmlPackage.child("PackageDesc").attribute("item1ID").as_uint();
	pkg->m_itemID1Exp = xmlPackage.child("PackageDesc").attribute("item1Exp").as_uint();
	pkg->m_itemID2 = xmlPackage.child("PackageDesc").attribute("item2ID").as_uint();
	pkg->m_itemID2Exp = xmlPackage.child("PackageDesc").attribute("item2Exp").as_uint();
	pkg->m_itemID3 = xmlPackage.child("PackageDesc").attribute("item3ID").as_uint();
	pkg->m_itemID3Exp = xmlPackage.child("PackageDesc").attribute("item3Exp").as_uint();
	pkg->m_itemID4 = xmlPackage.child("PackageDesc").attribute("item4ID").as_uint();
	pkg->m_itemID4Exp = xmlPackage.child("PackageDesc").attribute("item4Exp").as_uint();
	pkg->m_itemID5 = xmlPackage.child("PackageDesc").attribute("item5ID").as_uint();
	pkg->m_itemID5Exp = xmlPackage.child("PackageDesc").attribute("item5Exp").as_uint();
	pkg->m_itemID6 = xmlPackage.child("PackageDesc").attribute("item6ID").as_uint();
	pkg->m_itemID6Exp = xmlPackage.child("PackageDesc").attribute("item6Exp").as_uint();

	m_PackageArray[m_NumPackageLoaded] = pkg;
	m_NumPackageLoaded++;

	return true;
}

void WeaponArmory::Destroy()
{
	for(uint32_t i=0; i<m_NumPackageLoaded; ++i)
	{
		delete m_PackageArray[i];
		m_PackageArray[i] = NULL;
	}
	m_NumPackageLoaded = 0;
	for(uint32_t i=0; i<m_NumItemLoaded; ++i)
	{
		delete m_ItemArray[i];
		m_ItemArray[i] = NULL;
	}
	m_NumItemLoaded = 0;
	for(uint32_t i=0; i<m_NumGearLoaded; ++i)
	{
		delete m_GearArray[i];
		m_GearArray[i] = NULL;
	}
	m_NumGearLoaded = 0;
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i)
	{
		delete m_WeaponArray[i];
		m_WeaponArray[i] = NULL;
	}
	m_NumWeaponsLoaded = 0;
	for(uint32_t i=0; i<m_NumWeaponAttmLoaded; ++i)
	{
		delete m_WeaponAttmArray[i];
		m_WeaponAttmArray[i] = NULL;
	}
	m_NumWeaponAttmLoaded = 0;
	for(uint32_t i=0; i<m_NumAmmoLoaded; ++i)
	{
		delete m_AmmoArray[i];
		m_AmmoArray[i] = NULL;
	}
	m_NumAmmoLoaded = 0;
	for(uint32_t i=0; i<m_NumScopeLoaded; ++i)
	{
		delete m_ScopeArray[i];
		m_ScopeArray[i] = NULL;
	}
	m_NumScopeLoaded = 0;
	for(uint32_t i=0; i<m_NumAchievementLoaded; ++i)
	{
		delete m_AchievementArray[i];
		m_AchievementArray[i] = NULL;
	}
	m_NumAchievementLoaded= 0;
}

void WeaponArmory::UnloadMeshes()
{
	for(uint32_t i=0; i<m_NumItemLoaded; ++i)
		m_ItemArray[i]->resetMesh();
	for(uint32_t i=0; i<m_NumGearLoaded; ++i)
		m_GearArray[i]->resetMesh();
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i)
		m_WeaponArray[i]->resetMesh();
	for(uint32_t i=0; i<m_NumWeaponAttmLoaded; ++i)
		m_WeaponAttmArray[i]->resetMesh();
	for(uint32_t i=0; i<m_NumAmmoLoaded; ++i)
		m_AmmoArray[i]->unloadModel();
}

const WeaponConfig* WeaponArmory::getWeaponConfig(uint32_t itemID)
{
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i)
	{
		if(m_WeaponArray[i]->m_itemID == itemID)
		{
			return m_WeaponArray[i];
		}
	}
	return NULL;
}

const WeaponAttachmentConfig* WeaponArmory::getAttachmentConfig(uint32_t itemID)
{
	for(uint32_t i=0; i<m_NumWeaponAttmLoaded; ++i)
	{
		if(m_WeaponAttmArray[i]->m_itemID == itemID)
		{
			return m_WeaponAttmArray[i];
		}
	}
	return NULL;
}

const GearConfig* WeaponArmory::getGearConfig(uint32_t itemID)
{
	for(uint32_t i=0; i<m_NumGearLoaded; ++i)
	{
		if(m_GearArray[i]->m_itemID == itemID)
		{
			return m_GearArray[i];
		}
	}
	return NULL;
}

const ItemConfig* WeaponArmory::getItemConfig(uint32_t itemID)
{
	for(uint32_t i=0; i<m_NumItemLoaded; ++i)
	{
		if(m_ItemArray[i]->m_itemID == itemID)
		{
			return m_ItemArray[i];
		}
	}
	return NULL;
}

const PackageConfig* WeaponArmory::getPackageConfig(uint32_t itemID)
{
	for(uint32_t i=0; i<m_NumPackageLoaded; ++i)
	{
		if(m_PackageArray[i]->m_itemID == itemID)
		{
			return m_PackageArray[i];
		}
	}
	return NULL;
}

const ScopeConfig* WeaponArmory::getScopeConfig(const char* name)
{
	for(uint32_t i=0; i<m_NumScopeLoaded; ++i)
	{
		if(strcmp(m_ScopeArray[i]->name, name)==0)
		{
			return m_ScopeArray[i];
		}
	}
	return NULL;
}

const AchievementConfig* WeaponArmory::getAchievementConfig(const char* name)
{
	for(uint32_t i=0; i<m_NumAchievementLoaded; ++i)
	{
		if(strcmp(m_AchievementArray[i]->name, name)==0)
		{
			return m_AchievementArray[i];
		}
	}
	return NULL;
}

Gear* WeaponArmory::createGear(uint32_t itemID)
{
	if(itemID == 0)
		return NULL;

	for(uint32_t i=0; i<m_NumGearLoaded; ++i)
	{
		if(m_GearArray[i]->m_itemID == itemID)
		{
			return new Gear(m_GearArray[i]);
		}
	}

	r3dError("Failed to get gear with ID %d\nVersion mismatch!\n", itemID);
	return NULL;
}

Weapon* WeaponArmory::createWeapon(uint32_t itemID, GameObject* owner, bool first_person, bool allow_async_loading)
{
	if(itemID == 0)
		return NULL;

	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i)
	{
		if(m_WeaponArray[i]->m_itemID == itemID)
		{
			return new Weapon(m_WeaponArray[i], owner, first_person, allow_async_loading );
		}
	}

	r3dError("Failed to get weapon with ID %d\nVersion mismatch!\n", itemID);
	return NULL;
}

Ammo* WeaponArmory::getAmmo(const char* ammoName)
{
	for(uint32_t i=0; i<m_NumAmmoLoaded; ++i)
	{
		if(strcmp(m_AmmoArray[i]->m_Name, ammoName) == 0)
			return m_AmmoArray[i];
	}
	return NULL;
}

const WeaponConfig* WeaponArmory::getWeaponConfigByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumWeaponsLoaded);
	return m_WeaponArray[index];
}

const WeaponAttachmentConfig* WeaponArmory::getAttachmentConfigByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumWeaponAttmLoaded);
	return m_WeaponAttmArray[index];
}

const GearConfig* WeaponArmory::getGearConfigByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumGearLoaded);
	return m_GearArray[index];
}

const ItemConfig* WeaponArmory::getItemConfigByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumItemLoaded);
	return m_ItemArray[index];
}

const PackageConfig* WeaponArmory::getPackageConfigByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumPackageLoaded);
	return m_PackageArray[index];
}

const AchievementConfig* WeaponArmory::getAchievementByIndex(uint32_t index)
{
	r3d_assert(/*index >= 0 &&*/ index < m_NumAchievementLoaded);
	return m_AchievementArray[index];
}

const AchievementConfig* WeaponArmory::getAchievementByID(uint32_t ID)
{
	for( uint32_t achievementIndex = 0; achievementIndex < m_NumAchievementLoaded; achievementIndex++ )
	{
		if( m_AchievementArray[achievementIndex]->id == ID )
		{
			return m_AchievementArray[achievementIndex];
		}
	}

	return NULL;
}

STORE_CATEGORIES WeaponArmory::getCategoryByItemId(uint32_t itemId)
{
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->category;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->category;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->category;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->category;
	
	return storecat_INVALID;
}

const char* WeaponArmory::getIconByItemId(uint32_t itemId, bool emptyStringOnNULL)
{
	if(itemId == 0)
		return emptyStringOnNULL?"":NULL;
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->m_StoreIcon;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->m_StoreIcon;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->m_StoreIcon;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->m_StoreIcon;
	
	return emptyStringOnNULL?"":NULL;
}

const char* WeaponArmory::getNameByItemId(uint32_t itemId, bool emptyStringOnNULL)
{
	if(itemId == 0)
		return emptyStringOnNULL?"":NULL;
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->m_StoreName;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->m_StoreName;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->m_StoreName;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->m_StoreName;
	
	return emptyStringOnNULL?"":NULL;
}

const wchar_t* WeaponArmory::getNameWByItemId(uint32_t itemId, bool emptyStringOnNULL)
{
	if(itemId == 0)
		return emptyStringOnNULL?L"":NULL;
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->m_StoreNameW;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->m_StoreNameW;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->m_StoreNameW;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->m_StoreNameW;

	return emptyStringOnNULL?L"":NULL;
}

const wchar_t* WeaponArmory::getDescByItemId(uint32_t itemId, bool emptyStringOnNULL)
{
	if(itemId == 0)
		return emptyStringOnNULL?L"":NULL;
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->m_DescriptionW;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->m_DescriptionW;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->m_DescriptionW;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->m_DescriptionW;
	
	return emptyStringOnNULL?L"":NULL;
}

int WeaponArmory::getLevelReqByItemId(uint32_t itemId)
{
	//TODO: we can detect weapon/gear/item by it's itemID range
	const WeaponConfig* wc = getWeaponConfig(itemId);
	if(wc) return wc->m_LevelRequired;
	const GearConfig* gc = getGearConfig(itemId);
	if(gc) return gc->m_LevelRequired;
	const ItemConfig* ic = getItemConfig(itemId);
	if(ic) return ic->m_LevelRequired;
	const WeaponAttachmentConfig* wac = getAttachmentConfig(itemId);
	if(wac) return wac->m_LevelRequired;
	const PackageConfig* pc = getPackageConfig(itemId);
	if(pc) return pc->m_LevelRequired;

	return 0;
}

void WeaponArmory::dumpStats()
{
#ifndef WO_SERVER
#ifndef FINAL_BUILD
	FILE* fout = fopen( "playermeshes.txt", "wt" ) ;
	
	char buf[ 4096 ] ;

	void ConPrint( const char*, ... ) ;

	for( int i = 0, e = (int)m_NumAmmoLoaded  ; i < e ; i ++ )
	{
		Ammo* ammo = m_AmmoArray[ i ] ;

		if( int refs = ammo->getModelRefCount() )
		{
			sprintf( buf, "Ammo: %s with %d references\n", ammo->getName(), refs ) ;
			fputs( buf, fout ) ;
			ConPrint( "%s", buf ) ;
		}
	}

	for( int i = 0, e = (int)m_NumWeaponsLoaded ; i < e ; i ++ )
	{
		WeaponConfig* cfg = m_WeaponArray[ i ] ;

		int refs = cfg->getMeshRefs() ;
		int cfgRefs = cfg->getConfigMeshRefs() ;
		
		if( refs || cfgRefs )
		{			
			sprintf( buf, "Weapon: %s with %d references, %d mesh references\n", cfg->m_StoreName, cfgRefs, refs ) ;
			fputs( buf, fout ) ;
			ConPrint( "%s", buf ) ;
		}
	}

	for( int i = 0, e = (int)m_NumWeaponAttmLoaded ; i < e ; i ++ )
	{
		WeaponAttachmentConfig* att = m_WeaponAttmArray[ i ] ;

		int refs = att->getMeshRefs() ;
		int aimRefs = att->getAimMeshRefs() ;

		if( refs || aimRefs )
		{
			sprintf( buf, "WeaponAttachment: %s with %d references, %d aim references\n", att->m_StoreName, refs, aimRefs ) ;
			fputs( buf, fout ) ;
			ConPrint( "%s", buf ) ;
		}
	}

	for( int i = 0, e = (int)m_NumGearLoaded ; i < e ; i ++ )
	{
		GearConfig* gear = m_GearArray[ i ] ;

		int refs = gear->getMeshRefs() ;
		int cfgRefs = gear->getConfigMeshRefs() ;

		if( refs || cfgRefs )
		{
			sprintf( buf, "Gear: %s with %d references, %d fps references\n", gear->m_StoreName, cfgRefs, refs ) ;
			fputs( buf, fout ) ;
			ConPrint( "%s", buf ) ;
		}
	}

	for( int i = 0, e = (int)m_NumItemLoaded  ; i < e ; i ++ )
	{
		ItemConfig* itm = m_ItemArray[ i ] ;

		int refs = itm->getMeshRefs() ;

		if( refs )
		{
			sprintf( buf, "Item: %s with %d references\n", itm->m_StoreName, refs ) ;
			fputs( buf, fout ) ;
			ConPrint( "%s", buf ) ;
		}
	}

	extern int g_WeaponBalance ;
	extern int g_GearBalance ;

	ConPrint( "WB: %d, GB: %d\n", g_WeaponBalance, g_GearBalance ) ;

	fclose( fout ) ;
#endif
#endif
}

void DumpArmoryStats()
{
	gWeaponArmory.dumpStats() ;
}