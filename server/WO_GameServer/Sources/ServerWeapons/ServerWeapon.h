#pragma once

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"

class Ammo;
class GameObject;

class Weapon
{
	friend class WeaponArmory;

	enum WeaponAttachmentStatsEnum
	{
		WPN_ATTM_STAT_DAMAGE=0, // %
		WPN_ATTM_STAT_RANGE, // %
		WPN_ATTM_STAT_FIRERATE, // %
		WPN_ATTM_STAT_RECOIL, // %
		WPN_ATTM_STAT_SPREAD, // %
		WPN_ATTM_STAT_CLIPSIZE, // actual number

		WPN_ATTM_STAT_MAX
	};
	float		m_WeaponAttachmentStats[WPN_ATTM_STAT_MAX];
	uint32_t	m_AttachmentIDs[WPN_ATTM_MAX]; 
public:
	const WeaponConfig* m_pConfig;
	GameObject* m_Owner;
	
	int		m_NumClips;
	int		m_NumBulletsLeft;
	
public:
	Weapon(const WeaponConfig* conf, GameObject* owner, bool first_person, bool allow_async_loading);
	~Weapon();
	
	void		Reset();
	void		Resupply();

	int			getClipSize() const;

	float		calcDamage(float dist) const; // return 0 if weapon isn't immediate

	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }
	const WeaponConfig* getConfig() const { return m_pConfig; }

	void setWeaponAttachmentsByIDs(const uint32_t* ids);
	void setWeaponAttachments(const WeaponAttachmentConfig** wpnAttmConfigs);
	void getWeaponAttachmentIDs(uint32_t* attms) const { memcpy(attms, m_AttachmentIDs, sizeof(m_AttachmentIDs)); };
};
