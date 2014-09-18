#pragma once

struct WeaponConfig;

//
// stripped weapon armory for master server
//
class MSWeaponArmory
{
public:
	MSWeaponArmory();
	~MSWeaponArmory();
	
	// loads weapon library
	bool loadItemsDB(char* xmlData, int xmlSize);
	bool  loadWeapon(pugi::xml_node& xmlWeapon);
	bool  loadGear(pugi::xml_node& xmlGear);
	bool  loadItem(pugi::xml_node& xmlItem);

	static const int MAX_NUMBER_WEAPONS = 512;
	WeaponConfig* m_WeaponArray[MAX_NUMBER_WEAPONS]; // todo: rewrite into hash later
	uint32_t	m_NumWeaponsLoaded;

	static const int MAX_NUMBER_GEAR = 1024;
	GearConfig* m_GearArray[MAX_NUMBER_GEAR]; // todo: rewrite into hash later
	uint32_t	m_NumGearLoaded;

	static const int MAX_NUMBER_ITEM = 2048;
	ItemConfig* m_ItemArray[MAX_NUMBER_ITEM]; // todo: rewrite into hash later
	uint32_t	m_NumItemLoaded;
};
extern MSWeaponArmory* gMSWeaponArmory;
