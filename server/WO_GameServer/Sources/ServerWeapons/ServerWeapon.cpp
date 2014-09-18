#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "ServerWeapon.h"
#include "../ObjectsCode/obj_ServerPlayer.h"
#include "ServerGameLogic.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

Weapon::Weapon(const WeaponConfig* conf, GameObject* owner, bool first_person, bool allow_async_loading) 
	: m_pConfig(conf), 
	m_Owner(owner)
{
	r3d_assert(m_Owner);
	memset(m_AttachmentIDs, 0, sizeof(m_AttachmentIDs));
	memset(m_WeaponAttachmentStats, 0, sizeof(m_WeaponAttachmentStats));

	Reset();
}

Weapon::~Weapon()
{
}

void Weapon::setWeaponAttachmentsByIDs(const uint32_t* ids)
{
	r3d_assert(ids);
	const WeaponAttachmentConfig* configs[WPN_ATTM_MAX] = {0};
	for(int i=0; i<WPN_ATTM_MAX; ++i)
	{
		if(ids[i]>0)
			configs[i] = gWeaponArmory.getAttachmentConfig(ids[i]);
	}
	setWeaponAttachments(configs);
}

void Weapon::setWeaponAttachments(const WeaponAttachmentConfig** wpnAttmConfigs)
{
	for(int i=0; i<WPN_ATTM_MAX; ++i)
	{
		if(wpnAttmConfigs[i])
			m_AttachmentIDs[i] = wpnAttmConfigs[i]->m_itemID;
		else
			m_AttachmentIDs[i] = 0;
	}

	memset(m_WeaponAttachmentStats, 0, sizeof(m_WeaponAttachmentStats));

	int oldNumBulletsInClip = getClipSize();
	int oldMaxBullets = (m_NumClips) * oldNumBulletsInClip;

	// recalculate stats
	for(int i=0; i<WPN_ATTM_MAX; ++i)
	{
		if(wpnAttmConfigs[i]!=NULL)
		{
			const WeaponAttachmentConfig* attm = wpnAttmConfigs[i];
			{
				m_WeaponAttachmentStats[WPN_ATTM_STAT_DAMAGE] += attm->m_Damage;
				m_WeaponAttachmentStats[WPN_ATTM_STAT_RANGE] += attm->m_Range;
				m_WeaponAttachmentStats[WPN_ATTM_STAT_FIRERATE] += attm->m_Firerate;
				m_WeaponAttachmentStats[WPN_ATTM_STAT_RECOIL] += attm->m_Recoil;
				m_WeaponAttachmentStats[WPN_ATTM_STAT_SPREAD] += attm->m_Spread;
				m_WeaponAttachmentStats[WPN_ATTM_STAT_CLIPSIZE] += attm->m_Clipsize;
			}
		}
	}
	// convert stats to percents (from -100..100 range to -1..1)
	m_WeaponAttachmentStats[WPN_ATTM_STAT_DAMAGE] *= 0.01f; 
	m_WeaponAttachmentStats[WPN_ATTM_STAT_RANGE] *= 0.01f; 
	m_WeaponAttachmentStats[WPN_ATTM_STAT_FIRERATE] *= 0.01f; 
	m_WeaponAttachmentStats[WPN_ATTM_STAT_RECOIL] *= 0.01f; 
	m_WeaponAttachmentStats[WPN_ATTM_STAT_SPREAD] *= 0.01f;

	int newNumBulletsInClip = getClipSize();
	int newMaxBullets = (m_NumClips) * newNumBulletsInClip;

	m_NumBulletsLeft = m_NumBulletsLeft + R3D_MAX(newMaxBullets - oldMaxBullets, 0); // add difference. UI will be auto updated
}

int Weapon::getClipSize() const
{
	uint32_t clipsizeBoost = 0;
	clipsizeBoost += (int)m_WeaponAttachmentStats[WPN_ATTM_STAT_CLIPSIZE];

	return m_pConfig->m_clipSize+clipsizeBoost;
}

void Weapon::Reset()
{
	m_NumClips   = m_pConfig->m_numClips;

	// ABILITY: Double Up
	obj_ServerPlayer* pl = (obj_ServerPlayer*)m_Owner;
	if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(pl->GetLoadoutData().hasItem(AbilityConfig::AB_DoubleUp) && m_pConfig->category!=storecat_SUPPORT && m_pConfig->category!=storecat_GRENADES)
			m_NumClips = m_NumClips * 2;
	}

	Resupply();
}

void Weapon::Resupply()
{
	// total bullets left to shoot
	m_NumBulletsLeft = m_NumClips * getClipSize();
}

// dist = distance to target
float Weapon::calcDamage(float dist) const
{
	const obj_ServerPlayer* plr = (obj_ServerPlayer*)m_Owner;
	if(plr == NULL) 
	{
		r3dOutToLog("!!! weapon owner is null\n");
		return 0;
	}

	float range = m_pConfig->m_AmmoDecay;
	if(range <= 0)
	{
		gServerLogic.LogInfo(plr->peerId_, "!!! m_AmmoDecay is <= 0", "weapon ID: %d", m_pConfig->m_itemID);
		range = 1;
	}
	range = range * (1.0f+m_WeaponAttachmentStats[WPN_ATTM_STAT_RANGE]);

	if(getCategory() == storecat_ASR)
	{
		if(plr->GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_ASRWeapons)>=4)
			range *= 1.1f;
		else if(plr->GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_ASRWeapons)>=1)
			range *= 1.05f;
	}
	if(getCategory() == storecat_SNP)
	{
		if(plr->GetLoadoutData().getSkillLevel(CUserSkills::RECON_SniperWeapons)>=4)
			range *= 1.1f;
		else if(plr->GetLoadoutData().getSkillLevel(CUserSkills::RECON_SniperWeapons)>=1)
			range *= 1.05f;
	}
	if(getCategory() == storecat_MG)
	{
		if(plr->GetLoadoutData().getSkillLevel(CUserSkills::SPEC_LMGWeapons)>=4)
			range *= 1.1f;
		else if(plr->GetLoadoutData().getSkillLevel(CUserSkills::SPEC_LMGWeapons)>=1)
			range *= 1.05f;
	}
	if(getCategory() == storecat_SMG)
	{
		if(plr->GetLoadoutData().getSkillLevel(CUserSkills::MEDIC_SMGWeapons)>=4)
			range *= 1.1f;
		else if(plr->GetLoadoutData().getSkillLevel(CUserSkills::MEDIC_SMGWeapons)>=1)
			range *= 1.05f;
	}

	float ammoDmg = m_pConfig->m_AmmoDamage;
	if(m_pConfig->category == storecat_MELEE)
		return ammoDmg;

	ammoDmg = ammoDmg * (1.0f + m_WeaponAttachmentStats[WPN_ATTM_STAT_DAMAGE]);

	/*if(m_pConfig->category == storecat_SHTG)
	{
		// combat skills, +damage for shotguns
		if(plr->profile_.ProfileData.Skills[0]==2)
			ammoDmg *= 1.04f; // +2%
		else if(plr->profile_.ProfileData.Skills[0]==3)
			ammoDmg *= 1.08f; // +4%
		else if(plr->profile_.ProfileData.Skills[0]==4)
			ammoDmg *= 1.12f; // +6%
		else if(plr->profile_.ProfileData.Skills[0]==5)
			ammoDmg *= 1.16f; // +8%
	}*/
	/*if(m_pConfig->category == storecat_SNP)
	{
		// sniper skill
		if(plr->profile_.ProfileData.Skills[2]==1)
			ammoDmg *= 1.05f; // +5%
		else if(plr->profile_.ProfileData.Skills[2]==2)
			ammoDmg *= 1.10f; // +10%
		else if(plr->profile_.ProfileData.Skills[2]==3)
			ammoDmg *= 1.15f; // +15%
		else if(plr->profile_.ProfileData.Skills[2]==4)
			ammoDmg *= 1.20f; // +20%
		else if(plr->profile_.ProfileData.Skills[2]==5)
			ammoDmg *= 1.25f; // +25%
	}*/
	/*if(m_pConfig->category == storecat_HG) // hand guns
	{
		// EOD skills, +damage for hand guns
		if(plr->profile_.ProfileData.Skills[3]==2)
			ammoDmg *= 1.04f; // +5%
		else if(plr->profile_.ProfileData.Skills[3]==3)
			ammoDmg *= 1.08f; // +10%
		else if(plr->profile_.ProfileData.Skills[3]==4)
			ammoDmg *= 1.12f; // +15%
		else if(plr->profile_.ProfileData.Skills[3]==5)
			ammoDmg *= 1.16f; // +20%
	}*/
	if(m_pConfig->category == storecat_ASR || m_pConfig->category == storecat_HG || m_pConfig->category == storecat_MG || m_pConfig->category == storecat_SHTG || m_pConfig->category == storecat_SMG || m_pConfig->category == storecat_SNP) 
	{
		if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
		{
			// assault skill
			/*if(plr->profile_.ProfileData.Skills[4]==1)
				ammoDmg *= 1.03f; // +5%
			else if(plr->profile_.ProfileData.Skills[4]==2)
				ammoDmg *= 1.05f; // +10%
			else if(plr->profile_.ProfileData.Skills[4]==3)
				ammoDmg *= 1.07f; // +15%
			else if(plr->profile_.ProfileData.Skills[4]==4)
				ammoDmg *= 1.10f; // +20%
			else if(plr->profile_.ProfileData.Skills[4]==5)
				ammoDmg *= 1.15f; // +25%*/
		}
	}
	/*if(m_pConfig->category == storecat_SMG) // assault rifles
	{
		// special forces skill
		if(plr->profile_.ProfileData.Skills[5]==1)
			ammoDmg *= 1.05f; // +5%
		else if(plr->profile_.ProfileData.Skills[5]==2)
			ammoDmg *= 1.10f; // +10%
		else if(plr->profile_.ProfileData.Skills[5]==3)
			ammoDmg *= 1.15f; // +15%
		else if(plr->profile_.ProfileData.Skills[5]==4)
			ammoDmg *= 1.20f; // +20%
		else if(plr->profile_.ProfileData.Skills[5]==5)
			ammoDmg *= 1.25f; // +25%
	}*/

	// idea: 
	// from 0 to 90% range = damage goes from 100% to 90%
	// from 90% range to 100% range = damage goes from 90 to 50%
	// for over 100% range = damage fixed at 50%
	// Note: we need to make this weapon dependent later. 
	/*if(dist < range*0.9f) // 0 to 90% range
	{
		return R3D_LERP(ammoDmg, ammoDmg*0.9f, powf(dist/(range*0.9f), 2.0f)); // x^2 for a curve
	}
	else if(dist > range) // outside of range
	{
		return ammoDmg*0.5f; 
	}
	else // 90% to 100% range
	{
		return R3D_LERP(ammoDmg*0.9f, ammoDmg*0.5f, powf((dist-range*0.9f)/(range*0.1f), 2.0f)); // x^2 for a curve
	}*/

	// new formula
	// if < 100% range = damage 100%
	// if > range - damage 50%
	if(dist < range)
		return ammoDmg;
	else
		return ammoDmg*0.5f;
}
