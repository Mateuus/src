#ifndef __WEAPON_ARMORY_H__
#define __WEAPON_ARMORY_H__

class Weapon;
class Ammo;
class Gear;
class GameObject;

#include "GameCode\UserProfile.h"
#include "WeaponConfig.h"

class WeaponArmory
{
public:
	WeaponArmory();
	~WeaponArmory();

	struct LevelUpBonus
	{
		int nextLevel; // level for each we giving reward
		char* reward1;
		char* reward2;
		char* reward3;
		int gp;
		int gd;
		int sp;
		uint32_t items[8]; // up to 8 items are giving as reward for level up. should be more than enough. don't want to make dynamic array. if items[i]==0 then no item
		LevelUpBonus():nextLevel(-1), reward1(0), reward2(0), reward3(0), gp(0), gd(0) { memset(items, 0, sizeof(items));}
		~LevelUpBonus() { if(reward1) free(reward1); if(reward2) free(reward2); if(reward3) free(reward3);}
		void setReward(int level, int _gp, int _gd, int _sp, const char* _reward1, const char* _reward2="", const char* _reward3="",
						uint32_t it0=0, uint32_t it1=0, uint32_t it2=0, uint32_t it3=0, uint32_t it4=0, uint32_t it5=0, uint32_t it6=0, uint32_t it7=0, uint32_t it8=0);
	};
	LevelUpBonus m_LevelUpBonus[MAX_NUM_RANKS];
	void InitRewards();


	// loads weapon library
	bool Init();
	
	// releases all weapons.
	void Destroy();

	void UnloadMeshes();

	// returns pointer to a weapon, or null if not found
	// you are responsible for deleting pointer after you are done
	Weapon* createWeapon(uint32_t itemID, GameObject* owner, bool first_person, bool allow_async_loading );
	Gear* createGear(uint32_t itemID);
	
	const WeaponConfig* getWeaponConfig(uint32_t itemID);
	const WeaponAttachmentConfig* getAttachmentConfig(uint32_t itemID);
	const GearConfig* getGearConfig(uint32_t itemID);
	const ItemConfig* getItemConfig(uint32_t itemID);
	const PackageConfig* getPackageConfig(uint32_t itemID);
	const ScopeConfig* getScopeConfig(const char* name);
	const AchievementConfig* getAchievementConfig(const char* name);

	int getNumWeapons() const { return m_NumWeaponsLoaded; }
	int getNumAttachments() const { return m_NumWeaponAttmLoaded; }
	int getNumGears() const { return m_NumGearLoaded; }
	int getNumItems() const { return m_NumItemLoaded; }
	int getNumPackages() const { return m_NumPackageLoaded; }
	int getNumAchievements() const { return m_NumAchievementLoaded; }

	const WeaponConfig* getWeaponConfigByIndex(uint32_t index);
	const WeaponAttachmentConfig* getAttachmentConfigByIndex(uint32_t index);
	const GearConfig* getGearConfigByIndex(uint32_t index);
	const ItemConfig* getItemConfigByIndex(uint32_t index);
	const PackageConfig* getPackageConfigByIndex(uint32_t index);
	const AchievementConfig* getAchievementByIndex(uint32_t index);
	const AchievementConfig* getAchievementByID(uint32_t ID);

	STORE_CATEGORIES getCategoryByItemId(uint32_t itemId);
	const char*		 getIconByItemId(uint32_t itemId, bool emptyStringOnNULL=false);
	const char*		 getNameByItemId(uint32_t itemId, bool emptyStringOnNULL=false);
	const wchar_t*	 getNameWByItemId(uint32_t itemId, bool emptyStringOnNULL=false);
	const wchar_t*	 getDescByItemId(uint32_t itemId, bool emptyStringOnNULL=false);
	int				 getLevelReqByItemId(uint32_t itemId);

	void dumpStats() ;

private:
	bool loadAmmo(pugi::xml_node& xmlAmmo);
	bool loadWeapon(pugi::xml_node& xmlWeapon);
	bool loadWeaponAttachment(pugi::xml_node& xmlWeaponAttachment);
	bool loadGear(pugi::xml_node& xmlGear);
	bool loadItem(pugi::xml_node& xmlItem);
	bool loadPackage(pugi::xml_node& xmlItem);
	bool loadScope(pugi::xml_node& xmlScope);
	bool loadAchievement(pugi::xml_node& xmlAchievement);
	
	Ammo* getAmmo(const char* ammoName);

	static const int MAX_NUMBER_AMMO = 32;
	Ammo* m_AmmoArray[MAX_NUMBER_AMMO]; // todo: rewrite into hash later
	uint32_t	m_NumAmmoLoaded;

	static const int MAX_NUMBER_WEAPONS = 512;
	WeaponConfig* m_WeaponArray[MAX_NUMBER_WEAPONS]; // todo: rewrite into hash later
	uint32_t	m_NumWeaponsLoaded;

	static const int MAX_NUMBER_WEAPON_ATTACHMENTS = 512;
	WeaponAttachmentConfig* m_WeaponAttmArray[MAX_NUMBER_WEAPON_ATTACHMENTS]; // todo: rewrite into hash later
	uint32_t	m_NumWeaponAttmLoaded;

	static const int MAX_NUMBER_GEAR = 1024;
	GearConfig* m_GearArray[MAX_NUMBER_GEAR]; // todo: rewrite into hash later
	uint32_t	m_NumGearLoaded;

	static const int MAX_NUMBER_ITEM = 2048;
	ItemConfig* m_ItemArray[MAX_NUMBER_ITEM]; // todo: rewrite into hash later
	uint32_t	m_NumItemLoaded;

	static const int MAX_NUMBER_PACKAGE = 128;
	PackageConfig* m_PackageArray[MAX_NUMBER_PACKAGE]; // todo: rewrite into hash later
	uint32_t	m_NumPackageLoaded;

	static const int MAX_NUMBER_SCOPE = 128;
	ScopeConfig* m_ScopeArray[MAX_NUMBER_SCOPE]; // todo: rewrite into hash later
	uint32_t	m_NumScopeLoaded;

	static const int MAX_NUMBER_ACHIEVEMENT = 128;
	AchievementConfig* m_AchievementArray[MAX_NUMBER_ACHIEVEMENT]; // todo: rewrite into hash later
	uint32_t	m_NumAchievementLoaded;
};
extern WeaponArmory gWeaponArmory;

#endif //__WEAPON_ARMORY_H__