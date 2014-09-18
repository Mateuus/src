#include "r3dPCH.h"
#include "r3d.h"

#include "ObjectsCode/weapons/WeaponConfig.h"
#include "MasterServerWeaponArmory.h"

MSWeaponArmory* gMSWeaponArmory = NULL;

MSWeaponArmory::MSWeaponArmory()
{
	m_NumWeaponsLoaded = 0;
	m_NumGearLoaded    = 0;
	m_NumItemLoaded    = 0;
}

MSWeaponArmory::~MSWeaponArmory()
{
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i) {
		SAFE_DELETE(m_WeaponArray[i]);
	}
	m_NumWeaponsLoaded = 0;

	for(uint32_t i=0; i<m_NumGearLoaded; ++i) {
		SAFE_DELETE(m_GearArray[i]);
	}
	m_NumGearLoaded = 0;

	for(uint32_t i=0; i<m_NumItemLoaded; ++i) {
		SAFE_DELETE(m_ItemArray[i]);
	}
	m_NumItemLoaded = 0;
}

bool MSWeaponArmory::loadItemsDB(char* xmlData, int xmlSize)
{
	pugi::xml_document xmlFile;
	pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(xmlData, xmlSize);
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

	return true;
}

bool MSWeaponArmory::loadWeapon(pugi::xml_node& xmlWeapon)
{
	r3d_assert(!xmlWeapon.empty());

	uint32_t itemID = xmlWeapon.attribute("itemID").as_uint();
	// check if we have that weapon in our database
	for(uint32_t i=0; i<m_NumWeaponsLoaded; ++i) // todo: change to hash table
	{
		if(m_WeaponArray[i]->m_itemID == itemID)
		{
			r3dOutToLog("Trying to load a weapon with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumWeaponsLoaded > MAX_NUMBER_WEAPONS-2)
	{
		r3dOutToLog("Trying to load more than maximum number of weapons. Maximum is '%d'\n", MAX_NUMBER_WEAPONS);
		return false;
	}

	WeaponConfig* weapon = new WeaponConfig(itemID);
	if(!weapon->loadBaseFromXml(xmlWeapon))
	{
		delete weapon;
		return false;
	}

	m_WeaponArray[m_NumWeaponsLoaded] = weapon;
	m_NumWeaponsLoaded++;
	return true;
}

bool MSWeaponArmory::loadGear(pugi::xml_node& xmlGear)
{
	r3d_assert(!xmlGear.empty());

	uint32_t itemID = xmlGear.attribute("itemID").as_uint();
	for(uint32_t i=0; i<m_NumGearLoaded; ++i) // todo: change to hash table
	{
		if(m_GearArray[i]->m_itemID == itemID)
		{
			r3dOutToLog("Trying to load a gear with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumGearLoaded > MAX_NUMBER_GEAR-2)
	{
		r3dOutToLog("Trying to load more than maximum number of gear. Maximum is '%d'\n", MAX_NUMBER_GEAR);
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

bool MSWeaponArmory::loadItem(pugi::xml_node& xmlItem)
{
	r3d_assert(!xmlItem.empty());

	uint32_t itemID = xmlItem.attribute("itemID").as_uint();
	for(uint32_t i=0; i<m_NumItemLoaded; ++i) // todo: change to hash table
	{
		if(m_ItemArray[i]->m_itemID == itemID)
		{
			r3dOutToLog("Trying to load an item with id '%d' that is already loaded!", itemID);
			return false;
		}
	}
	if(m_NumItemLoaded > MAX_NUMBER_ITEM-1)
	{
		r3dOutToLog("Trying to load more than maximum number of items. Maximum is '%d'\n", MAX_NUMBER_ITEM);
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
