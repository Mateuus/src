#pragma once

#pragma pack(push)
#pragma pack(1)

// MAKE SURE to increase this after chanding following structs
#define GBWEAPINFO_VERSION 0x00000001

struct GBWeaponInfo
{
	float m_AmmoMass;
	float m_AmmoSpeed;
	float m_AmmoDamage;
	float m_AmmoDecay;
	float m_AmmoArea;
	float m_AmmoDelay;
	float m_AmmoTimeout;

	uint32_t m_numClips;
	uint32_t m_clipSize;
	float m_reloadTime;
	float m_reloadActiveTick;
	float m_fireDelay;
	float m_spread; 
	float m_recoil;
};

struct GBGearInfo
{
	float m_Weight;
	float m_damagePerc;
	float m_damageMax;
	float m_bulkiness;
	float m_inaccuracy;
	float m_stealth;
	int   m_ProtectionLevel;
};

#pragma pack(pop)
