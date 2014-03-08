#include "r3dpch.h"
#include "r3d.h"

#include "GameCommon.h"
#include "Airstrike.h"

//////////////////////////////////////////////////////////////////////
////// THIS FILE IS SHARED BETWEEN CLIENT AND SERVER /////////////////
//////////////////////////////////////////////////////////////////////
#ifdef WO_SERVER
AirstrikeInFlight g_AirstrikesInFlight[MAX_AIRSTRIKES_IN_FLIGHT];
#endif

static const float AS_RADIUS_MAX = 15.0f;
static const float AS_ACCURACY_MAX = 40.0f;
static const float AS_DAMAGE_MAX = 500.0f;
static const float AS_DELAY_MAX = 12.0f;

static const int g_NumAirstrikesInArray = 4; // !!!!if you change this number, please update it in P2PMessages.h in PKT_S2C_SetGamePlayParams_s!!!!
AirstrikeDataset  AirstrikesArray[g_NumAirstrikesInArray] = {
	{ 301083, 1, "Airstrike_MortarStrike",		60, 0, 0,	5.6f, 5.6f/AS_RADIUS_MAX,   250.0f, 250.0f/AS_DAMAGE_MAX,		1.0f, 3.0f, 1, 11.0f, 1.0f-11.0f/AS_ACCURACY_MAX,  50.0f, 5, 1.0f-5.0f/AS_DELAY_MAX, "Airstrike_MortarStrikeDesc", "$Data/Weapons/StoreIcons/AS_MortarStrike.dds", "Data/ObjectsDepot/Weapons/Rocket_AT4.sco", "WeaponFX_GranadeTrail_01", "Explosion_RPG_02", NULL, NULL, NULL, "Sounds/Effects/Explosions/Bomb01"},
	{ 301084, 1, "Airstrike_MortarBarrage",		60, 0, 0,	5.6f, 5.6f/AS_RADIUS_MAX,   250.0f, 250.0f/AS_DAMAGE_MAX,		1.0f, 3.0f, 5, 20.0f, 1.0f-20.0f/AS_ACCURACY_MAX,  50.0f, 5, 1.0f-5.0f/AS_DELAY_MAX, "Airstrike_MortarBarrageDesc", "$Data/Weapons/StoreIcons/AS_MortatBarrage.dds", "Data/ObjectsDepot/Weapons/Rocket_AT4.sco", "WeaponFX_GranadeTrail_01", "Explosion_RPG_02", NULL, NULL, NULL, "Sounds/Effects/Explosions/Bomb01"},
	{ 301085, 1, "Airstrike_ArtilleryStrike",	60, 0, 0,	7.0f, 7.0f/AS_RADIUS_MAX,   350.0f, 350.0f/AS_DAMAGE_MAX,		1.0f, 3.0f, 5, 30.0f, 1.0f-30.0f/AS_ACCURACY_MAX,  80.0f, 10, 1.0f-10.0f/AS_DELAY_MAX, "Airstrike_ArtilerryStrikeDesc", "$Data/Weapons/StoreIcons/AS_ArtilerryStrike.dds", "Data/ObjectsDepot/Weapons/Rocket_RPO.sco", "WeaponFX_GranadeTrail_01", "Explosion_Bomb_01", NULL, NULL, NULL, "Sounds/Effects/Explosions/Bomb01"},
	{ 301086, 1, "Airstrike_LaserGuidedBomb",	60, 0, 0,	11.0f, 11.0f/AS_RADIUS_MAX, 500.0f, 500.0f/AS_DAMAGE_MAX,		1.0f, 3.0f, 1, 9.0f,  1.0f- 9.0f/AS_ACCURACY_MAX,  80.0f, 10, 1.0f-10.0f/AS_DELAY_MAX, "Airstrike_LaserGuigedBombDesc", "$Data/Weapons/StoreIcons/AS_LaserGuigedBomb.dds", "Data/ObjectsDepot/Weapons/Rocket_MPISRAW.sco", "WeaponFX_GranadeTrail_01", "explosion_artillerybarrage_01", NULL, NULL, NULL, "Sounds/Effects/Explosions/Bomb01"},
};

int getNumAirstrikes()
{
	return g_NumAirstrikesInArray;
}

AirstrikeDataset* getAirstrike(int i)
{
	r3d_assert(i>=0 && i<g_NumAirstrikesInArray);
	return &AirstrikesArray[i];
}

AirstrikeDataset* getAirstrikeByID(int itemID)
{
	for(int i=0; i<g_NumAirstrikesInArray; ++i)
		if(AirstrikesArray[i].itemID == itemID)
			return &AirstrikesArray[i];

	return NULL;
}

float AIRSTRIKE_Team_Cooldown[2] = {0.0f, 0.0f};
