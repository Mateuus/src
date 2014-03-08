#ifndef __WEAPON_H__
#define __WEAPON_H__

#ifdef WO_SERVER
  #error "client weapon.h included in SERVER"
#endif

#include "r3dProtect.h"

#include "..\..\GameCode\UserProfile.h"
#include "WeaponConfig.h"

class Ammo;
class GameObject;
class obj_AI_Player;

class Weapon
{
	friend class WeaponArmory;
	enum WeaponState
	{
		WPN_EMPTY, // no more ammo
		WPN_READY, // ready to fire
		WPN_RELOADING, // reloading
	};
public:
	Weapon(const WeaponConfig* conf, GameObject* owner, bool first_person, bool allow_async_loading );
	~Weapon();

	void Reset(); // reset number of clips, etc
	void Resupply(); // resupply ammo
	void ResetBullets(int numBullets); // when picking up a weapon, have to overwrite the number of bullets

	bool isReadyToFire(bool triggerPressed, bool scopeMode);

	int  getNumShotsRequired(); // because we have weapons with firerate of 900+ and our fps is around 30 we need to be able to shoot more than one bullet per frame
	void Fire(const r3dPoint3D& hitPos, const D3DXMATRIX& weaponBone, float holdingDelay=0, const r3dPoint3D& grenadeFireFrom = R3D_ZERO_VECTOR );
	void Update(const D3DXMATRIX& weaponBone); // call before fire
	void Reload();

	void OnEquip(); // called when player equips that weapon
	void OnUnequip(); // called when the player is changing weapons.  

	int getNumBulletsLeftInClip() const { return m_numBulletsLeftInClip; }
	int getNumBulletsLeft() const { return m_numBulletsLeft + m_numBulletsLeftInClip; }
	uint32_t getNumBulletsInClip() const;
	int getNumClipsLeft() const { return (int)ceilf(float(m_numBulletsLeft)/(float)getNumBulletsInClip()); }

	bool	isReloading() const { return m_State == WPN_RELOADING; }

	r3dMesh*	getModel( bool allow_async_loading, bool first_person ) const { return m_pConfig->getMesh( allow_async_loading, first_person ); }
	const r3dPoint3D& getMuzzleOffset() const { return m_pConfig->muzzlerOffset; }
	r3dPoint3D getMuzzlePos(const D3DXMATRIX& weaponBone) const;
	const char* getMuzzlePartilce() const { return m_pConfig->m_MuzzleParticle; }

	r3dPoint3D getShellPos(const D3DXMATRIX& weaponBone) const;
	r3dPoint3D getShellDir(const D3DXMATRIX& weaponBone) const;

	WeaponAnimTypeEnum getAnimType() const { return m_pConfig->m_AnimType; }
	bool isGrenadeOrMineAnim() const { return m_pConfig->m_AnimType == WPN_ANIM_GRENADE || m_pConfig->m_AnimType == WPN_ANIM_MINE; }
	
	bool isUsableItem() const { return m_pConfig->category == storecat_UsableItem; }
	int	m_usableItemInventoryIdx;

	uint32_t getItemID() const { return m_pConfig->m_itemID; }

	const char* getDecalSource() const;

	bool		isImmediateShooting() const;

	float	getAmmoSpeed() const;
	float	getAmmoMass() const;
	float	getAmmoDmgDecay() const { return m_pConfig->m_AmmoDecay; }
	
	float	calcDamage(float dist) const; // return 0 if weapon isn't immediate
	
	float getLastTimeFired() const { return m_lastTimeFired; }

	float getSpread() const; // returns diameter of spread at 50meter range
	float getRecoil() const; // recoil is from 0 to 100, convert it to value usable by code
	float getReloadTime() const { return m_ReloadTime; }
	float getReloadProgress() const { return (r3dGetTime()-m_lastReloadingTime); }

	float getAmmoDelay() const { return m_pConfig->m_AmmoDelay; }

	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }

	const char* getStoreName() const { return m_pConfig->m_StoreName; }
	const char* getStoreIcon() const { return m_pConfig->m_StoreIcon; }

	void switchFiremode();
	WeaponFiremodeEnum getFiremode() const { return (WeaponFiremodeEnum)m_firemode; }

	const WeaponConfig* getConfig() const { return m_pConfig; }

	int getTriggerPressedCounter() const { return m_triggerPressed; }

	float getRateOfFire() const; // returns delay before next fire available

	bool isLoaded() const ;

	void checkForSkeleton();
	bool isAnimated() const { return m_WeaponAnim_FPS!=NULL; }
	r3dAnimation* getAnimation() const { return m_WeaponAnim_FPS; }
	
	void OnGameEnded();

	void setWeaponAttachmentsByIDs(const uint32_t* ids);
	void setWeaponAttachments(const WeaponAttachmentConfig** wpnAttmConfigs);
	uint32_t getWeaponAttachmentID(WeaponAttachmentTypeEnum attm_type); // if any
	void getCurrentAttachments(const WeaponAttachmentConfig** attms) 
	{ 
		r3d_assert(attms);
		memcpy((void*)attms, m_Attachments, sizeof(m_Attachments)); 
	}
	void getCurrentAttachmentIDs(uint32_t* ids) 
	{
		r3d_assert(ids);
		memset(ids, 0, sizeof(uint32_t)*WPN_ATTM_MAX);
		for(int i=0; i<WPN_ATTM_MAX; ++i)
		{
			if(m_Attachments[i])
				ids[i] = m_Attachments[i]->m_itemID;
		}
	}

	r3dMesh* getWeaponAttachmentMesh(WeaponAttachmentTypeEnum attm_type, bool aim_model);
	bool	 getWeaponAttachmentPos(WeaponAttachmentTypeEnum attm_type, D3DXMATRIX& res);

	float	getWeaponScopeZoom() const;
	bool	hasScopeMode(bool isFPS) const;
	const ScopeConfig* getScopeConfig(bool isFPS) const;

	const int* getWeaponAnimID_FPS() const;

	bool hasLaserPointer(r3dPoint3D& laserPos); 

	bool m_needDelayedAction; // for grenades, to sync with animation
private:
	// data
	const WeaponAttachmentConfig* m_Attachments[WPN_ATTM_MAX];

	obj_AI_Player* m_Owner; // todo: change that to safeID!
	WeaponState	m_State;
	r3dSec_type<float, 0xDFC5CAD5> m_ReloadTime;
	float m_lastReloadingTime;
	float m_fractionTimeLeftFromPreviousShot; // for accurate fire rate
	
	class obj_ParticleSystem* m_MuzzleParticle;
	class obj_ParticleSystem* m_LaserPointerParticle;
	r3dPoint3D				  m_LaserHitPoint;
	class obj_ParticleSystem* m_FlashlightParticle;
	r3dLight				  m_Flashlight;

	r3dSec_type<int, 0xF387EA12> m_ModifiedNumClips; // this clip size is modified if player has ability.
	r3dSec_type<int, 0xFAB535D8> m_numBulletsLeftInClip;
	float m_lastTimeFired;
	r3dSec_type<int, 0x3CF5EA3D> m_numBulletsLeft; // total number of bullets that players has for reloading
	class obj_ParticleSystem* m_ShellExtractParticle;
	r3dLight	m_MuzzleLight;
	int m_firemode;
	int m_triggerPressed; // for how many shots trigger is pressed

	r3dPoint3D m_needDelayedAction_pos;
	float m_needDelayedAction_delay;
	float m_needDelayedAction_startTime;

	r3dSec_type<const WeaponConfig*, 0xFCD2829A> m_pConfig;

	bool	m_isFirstPerson;

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

	void*	m_sndReload;
	void*	m_sndFire;

	// new weapon sounds
	void*	m_sndNewFire;

	// animation for FPS mode
	r3dAnimation* m_WeaponAnim_FPS;
	int			  m_AnimTrack_idle;
	int			  m_AnimTrack_fire;

	// replacement anims for FPS mode, when weapon has scope attached
	int*		m_animationIds_FPS;

	int getWeaponFireSound();
	void reloadMuzzleParticle();
};

#endif //__WEAPON_H__
