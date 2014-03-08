#ifndef HUDDisplay_h
#define HUDDisplay_h

#include "r3d.h"
#include "APIScaleformGfx.h"
#include "../GameCode/UserProfile.h"
#include "../ObjectsCode/weapons/Weapon.h"

#define MAX_HUD_ACHIEVEMENT_QUEUE 8

class obj_AI_Player;
class HUDDisplay
{
private:
	friend void fsChatMessage(void* data, r3dScaleformMovie *pMovie, const char *arg);

	Scaleform::GFx::Value m_SF_HUD;
	Scaleform::GFx::Value m_SF_HUD_Ping;
	Scaleform::GFx::Value m_SF_HUD_PingText;
	Scaleform::GFx::Value m_SF_HUD_Reticles;
	Scaleform::GFx::Value m_SF_HUD_ItemSlots[4];
	Scaleform::GFx::Value m_SF_HUD_YouHaveBomb; // showing if you have bomb or not in sabotage
	bool				m_YouHaveBombSet;

	void setReticleScale_ASR(Scaleform::GFx::Value& reticle, float scale);	
protected:
	int	tickets_[4];	// saved tickets [0-1]:org [2-3]:cur
	int	cpoints_[2];	// saved controlled points
	
	//int	captVisible;
	//int	captValue;

	bool Inited;
	bool ChatWindowVisible;
	bool ReticleVisible;
	bool ShowWaitingForPlayersMsg;

	bool TimerFlashVisible;
	float showTimeMessageDelay;

	bool inDeadMode;
	bool bombTimerVisible;

	float m_PrevBloodLevel;
	float m_PrevEnergyValue;
	float m_PrevBreathValue;

	int m_AchievementQueue[MAX_HUD_ACHIEVEMENT_QUEUE];
	int m_NumAchievementsInQueue;
	int m_CurrentAchievement;
	float lastAchievementTime;

	void	ProcessAchievementQueue();

	struct hitEffectTracker
	{
		float hitTimeEnd;
		DWORD fromID; // safeID of gameObject
		Scaleform::GFx::Value hitEffect;

		hitEffectTracker()
		{
			hitTimeEnd =0;
			fromID = 0;
		}
		~hitEffectTracker()
		{
			hitEffect.SetUndefined();
		}
		hitEffectTracker(const hitEffectTracker& rhs)
		{
			hitTimeEnd = rhs.hitTimeEnd;
			fromID = rhs.fromID;
			hitEffect = rhs.hitEffect;
		}

	};
	std::list<hitEffectTracker> hitEffectList;
	float	getHitEffectAngle(GameObject* from);
	
public:
	r3dScaleformMovie gfxHUD;
	r3dScaleformMovie gfxHitEffect;

	bool isChatVisible() const { return ChatWindowVisible; }
	void ShowChatWindow(bool set);
	void ForceHideChatWindow();

	bool isInDeadMode() const { return inDeadMode; }

	void updateReticlePosition();

	enum HUDIconType
	{
		HUDIcon_Attack=0,
		HUDIcon_Defend,
		HUDIcon_Spotted,
	};
	struct HUDIcon
	{
		Scaleform::GFx::Value icon;
		r3dVector pos;
		float lifetime;
		float spawnTime;
		bool enabled;
		HUDIconType type;
		HUDIcon():enabled(false) {}
	};
	HUDIcon m_HUDIcons[32];
	void addHUDIcon(HUDIconType type, float lifetime, const r3dVector& pos);

	void	eventRegisterHUD(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
public:
	HUDDisplay();
	~HUDDisplay();

	bool 	Init(int tickets, bool inShootingRange=false);
	bool 	Unload();

	int 	Update();
	int 	Draw();

	void	SetYouHaveBomb(bool set); 

	void	SetTickets(int blue, int red);
	void	SetBombState(int idx, const char* state); // state: disabled, neutral, timed, exploded
	void	AddKillMessage(obj_AI_Player* killer, obj_AI_Player* victim, STORE_CATEGORIES damageType);
	void	ShowKillTag(obj_AI_Player* killer, STORE_CATEGORIES damageType); // show kill tag for a player that killed you
	void	ShowKilledTag(obj_AI_Player* victim); // show kill tag for a player that you killed
	void	ShowAchievement(const int achievementID );
	void	ShowAchievementCustom(const wchar_t* name, const char* desc, const char* icon, const char* reward);
	void	RequeustAchievement( int achievementID );
	int		popAchievementFromQueue();
	inline bool HasAchievements() { return m_NumAchievementsInQueue != 0; }
	int getCurrentAchievementDisplaying();

	void	ShowScore(int xp, int gp, int gd, int totalGD, int totalXP);
	void	ShowLootDropMsg(const wchar_t* lootName, const char* iconPath);

	void	showLeavingBattlezone(bool show, bool showTimer);
	void	setLeavingBattlezoneTimer(float progress); // progress = [0.0,1.0f]

	void	setHealth(int health);
	void	setPing(int ping);

	void	showYouGotCoolThing(const wchar_t* title, const char* icon);

	void	setMinimapBombTimer(const char* time, bool isVisible);
	
	void	switchToDead(bool state, bool force_spectator=false); // turns off many UI elements, shows when player is dead

	void	SetEnergyValue(float value); // value from [0.0..1.0]
	void	SetBreathValue(float value); // value from [0.0..1.0]

	void	AddMessage(const wchar_t *msg);
	void	SetBullets(int num_left, int num_in_clip, int num_clips);
	void	AddNewWeapon(int index, const char* name, const char* icon, STORE_CATEGORIES weaponType, WeaponFiremodeEnum firemode);
	void	SetItemInfo(int index, float cooldown, float maxcooldown, int quantity);
	void	setCurrentWeapon(int index);
	void	removeAllWeapons();
	void	setFireMode(WeaponFiremodeEnum firemode); // 1, 3, 5
	void	showReloading(bool set);
	void	SetReloadingProgress(float progress); // progress - 0-100
	void	showReticle(bool set);
	void	setReticleScale(float scale);
	void	setReticleColor(const char* color); // color="red", "blue", "green", "white"
	void	showUseItem(bool enable);

	void	showBombTimer(bool set);
	void	setBombTimerProgress(float progress); // progress = 0-1

	void addTargetIcon(Scaleform::GFx::Value& result);
	void addGrenadeIcon(Scaleform::GFx::Value& result);
	void addMineIcon(Scaleform::GFx::Value& result);
	void addMineEnemyIcon(Scaleform::GFx::Value& result);
	void addPingIcon(Scaleform::GFx::Value& result);
	void addWoundedIcon(Scaleform::GFx::Value& result);
	void addIconSpawnIcon(Scaleform::GFx::Value& result);
	void addHealingIcon(Scaleform::GFx::Value& result);
	void addMedicIcon(Scaleform::GFx::Value& result);
	void addNeedHelpIcon(Scaleform::GFx::Value& result);
	void addNeedOrdersIcon(Scaleform::GFx::Value& result);
	void addNeedMedicIcon(Scaleform::GFx::Value& result);
	void addNeedAmmoIcon(Scaleform::GFx::Value& result);
	void addControlPointIcon(Scaleform::GFx::Value& result);
	void setControlPointIconStatus(Scaleform::GFx::Value& icon, const char* status, const char* tag); // "red", "blue" or "grey", tag = a, b, c, d, z
	void setControlPointDistance(Scaleform::GFx::Value& icon, float distance, bool visible);
	void setVisibleControlPointCaptureProgress(Scaleform::GFx::Value& icon, bool visible);
	void setControlPointCaptureProgress(Scaleform::GFx::Value& icon, float progress, const char* type); // progress - 0-100, type="red" or "blue"
	void moveScreenIcon(Scaleform::GFx::Value& icon, const r3dPoint3D& pos, bool alwaysShow, bool force_invisible = false, bool pos_in_screen_space=false); 
	void setScreenIconAlpha(Scaleform::GFx::Value& icon, float alpha);
	void setScreenIconScale(Scaleform::GFx::Value& icon, float scale); // 1.0f - default size
	void deleteScreenIcon(Scaleform::GFx::Value& icon);

	void addPlayerTagIcon(const char* name, const char* team, Scaleform::GFx::Value& result); // team = "red" or "blue"
	void showPlayerNameTag(Scaleform::GFx::Value& icon, bool show);
	
	void	AddHitEffect(GameObject* from);
	void	SetBloodLevel(float health); // health 0-100
	void	HitOtherPlayer(); // animate weapon aim

	void	AddChatMessage(BYTE msgType, obj_AI_Player* from, const char* msg);
	void	AddChatMessage(BYTE msgType, obj_AI_Player* from, const wchar_t* msg);

	void	showPointCapturedInfo(const char* type, const wchar_t* text); // red, blue
	void	showSystemMessage(const wchar_t* text); // no reward message

	void	showActionMessage(const char* action, const char* key); // action: "blackops", "resupply", "fixer", "usekey", "needkey", "armBomb", "disarmBomb"

	void	showPickupMsg(const char* key, const char* wpnName, const char* wpnIcon,
						  const wchar_t* prop1, int Value1, int ModValue1,
						  const wchar_t* prop2, int Value2, int ModValue2,
						  const wchar_t* prop3, int Value3, int ModValue3,
						  const wchar_t* prop4, int Value4, int ModValue4,
						  const wchar_t* capacity, int cap);
	void	hidePickupMsg();
	void	hideActionMessage( const char* action );
};

#endif