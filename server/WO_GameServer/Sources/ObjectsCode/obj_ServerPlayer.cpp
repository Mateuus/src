#include "r3dpch.h"
#include "r3d.h"

#include "obj_ServerPlayer.h"

#include "ServerWeapons/ServerWeapon.h"
#include "ServerWeapons/ServerGear.h"
#include "obj_ServerMine.h"
#include "obj_ServerUAV.h"
#include "obj_ServerMedKit.h"
#include "obj_ServerMotionSensor.h"
#include "obj_ServerRespawnBeacon.h"
#include "obj_ServerRiotShield.h"
#include "obj_ServerAutoTurret.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"

#include "multiplayer/P2PMessages.h"
#include "ServerGameLogic.h"

#include "../EclipseStudio/Sources/Gameplay_Params.h"
extern CGamePlayParams		GPP_Data;
extern int RUS_CLIENT;

IMPLEMENT_CLASS(obj_ServerPlayer, "obj_ServerPlayer", "Object");
AUTOREGISTER_CLASS(obj_ServerPlayer);

CVAR_COMMENT("_ai_", "AI variables");

	const uint32_t	ITEMID_BLACKOPS_CRATE = 301000;
	const uint32_t	ITEMID_XPBONUSx2      = 301001;
	const uint32_t	ITEMID_GPBONUSx2      = 301003;
	const uint32_t	ITEMID_PREMIUMACC     = 301004;
	
obj_ServerPlayer::obj_ServerPlayer() : 
	netMover(this, 0.2f, (float)PKT_C2C_MoveSetCell_s::PLAYER_CELL_RADIUS)
{
	ObjTypeFlags = OBJTYPE_Human;

	respawnInvulnerableTime = 0;
	berserkerTime = 0;
  
	peerId_ = -1;
	startPlayTime_ = r3dGetTime();
  
	r3dscpy(userName, "unknown");
	NextSpawnNode = -1;
	m_SelectedLoadout = -1;
	m_SelectedWeapon = -1;
  
	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; i++)
		m_WeaponArray[i] = NULL;
    
	for(int i=0; i<SLOT_Max; i++)
		gears_[i] = NULL;
    
	lastTimeHit     = 0;
	lastHitBodyPart = 0;
  
	capturingPointId_ = 0;
	capturingPointStart_ = 0;
	capturingPointCooldown_ = -999;
	capturingPointStreak_ = 0;
  
	uavRequested_ = false;
  
	m_DamageReduction_LastTimeHit = 0;
	m_DamageReduction_BulletCounter = 0;

	m_LastTimeResupplied   = 0; // allow player to get resupply right after spawning
	m_NumberAssistedKills = 0;
	m_CanUseBlackopsCrate  = 0;
	m_RequestedResupply = 0;
  	m_DisableWeaponDrop = false;

	m_carryingBomb = false;

	m_BigSurpriseTimer = 0;
  
	isTargetDummy_ = false;
  
	lastChatTime_    = -1;
	numChatMessages_ = 0;
	
	m_PlayerRotation = 0;

	return;
}

BOOL obj_ServerPlayer::OnCreate()
{
	parent::OnCreate();
	DrawOrder	= OBJ_DRAWORDER_FIRST;

	SetVelocity(r3dPoint3D(0, 0, 0));

	r3d_assert(!NetworkLocal);
	TeleportPlayer(GetPosition());
	
	myPacketSequence = 0;
	clientPacketSequence = 0;
	packetBarrierReason = "";

	m_MorphineTime = 0;
	m_MorphineEffect = 1.0f;
	
	FireHitCount = 0;

	lastRespawnTimeAtBase = r3dGetTime();
	lastRespawnPosAtBase = GetPosition();
	m_Health = getMaxHealth();
	lastWeapDataRep = r3dGetTime();

	lastPlayerAction_ = r3dGetTime();

	m_RequestedWeaponPickup = 0;
	m_RequestWeaponPickupSecretCode = 0;
	m_RequestedWeaponPickUpPrevOwner = invalidGameObjectID;
	LastEnemyKillTime = 0;
	Killstreaks = 0;
	LastKilledBy = invalidGameObjectID;
	numKillWithoutDying = 0;
	inventoryIndexForKills = 0; 
	numOfUAVHits = 0;
	m_InDamageAreaTime = 0;

	m_RocketKills = 0;
	m_AirstrikeKills = 0;
	m_CampingKills = 0;
	m_SniperKillsWithoutMissing = 0;
	m_LastSniperShotZoomed = false;
	m_HasDamagedSelf = false;

	return TRUE;
}



BOOL obj_ServerPlayer::OnDestroy()
{
  return parent::OnDestroy();
}

BOOL obj_ServerPlayer::Load(const char *fname)
{
 if(!parent::Load(fname)) return FALSE;

 // Object won't be saved when level saved
 bPersistent 	= 0;

 Height 	 	= SRV_WORLD_SCALE (1.8f);
 TeamID		 	= TEAM_UNKNOWN;
 m_LastTimeTeamSwitch   = -9999;

 isDead                 = 0;
 bzOutTime              = -1;
 
 RecalcBoundBox();
 
 return TRUE;
}

void obj_ServerPlayer::SetProfile(const CServerUserProfile& in_profile)
{
	profile_ = in_profile;

	r3dscpy(userName, profile_.ScreenName);

	boostXPBonus_          = 0.0f; // % to add
	boostWPBonus_          = 0.0f; // % to add

	r3dOutToLog("SetProfile %s\n", userName); CLOG_INDENT;

	// check for special item that unlock black ops crates
	for(uint32_t i=0; i<profile_.ProfileData.NumItems; ++i)
	{
		//r3dOutToLog("item: %d\n", profile_.ProfileData.Inventory[i].itemID);
		switch(profile_.ProfileData.Inventory[i].itemID)
		{
		case ITEMID_BLACKOPS_CRATE:
			r3dOutToLog("Blackops crate unlocked\n");
			m_CanUseBlackopsCrate = true;
			break;

		case ITEMID_XPBONUSx2:
			// BoostXP +100%
			r3dOutToLog("XPx2 bonus\n");
			boostXPBonus_ += 1.0f;
			break;
		case ITEMID_GPBONUSx2:
			// BoostGP +100%
			r3dOutToLog("WPx2 bonus\n");
			boostWPBonus_ += 1.0f;
			break;
		case ITEMID_PREMIUMACC:
			// BoostXP/WP +50%
			if(RUS_CLIENT)
			{
				r3dOutToLog("PremiumAcc bonus RUS\n");
				boostXPBonus_ += 1.0f;
				boostWPBonus_ += 1.0f;
			}
			else
			{
				r3dOutToLog("PremiumAcc bonus\n");
				boostXPBonus_ += 0.5f;
				boostWPBonus_ += 0.5f;
			}
			break;
		}

		// no damage boosts in sabotage mode
// 		if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
// 		{
// 			switch(profile_.ProfileData.Inventory[i].itemID)
// 			{
// 			}
// 		}
	}
	
	// filter cybersport items
	if(gServerLogic.ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		for(int slot = 0; slot < profile_.ProfileData.NumSlots; slot++)
		{
			wiLoadoutSlot& ls = profile_.ProfileData.ArmorySlots[slot];
		
			//SABOTAGE_NoRpg rule: remove RPG
			const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(ls.SecondaryWeaponID);
			if(wc && wc->category == storecat_SUPPORT)
			{
				ls.SecondaryWeaponID = 0;
			}
		}
	}

	return;
}

void obj_ServerPlayer::DoDeath()
{
	SetLatePacketsBarrier("death");

	if(m_carryingBomb)
	{
		gServerLogic.bomb_isDropped = true;
		gServerLogic.bomb_droppedPos = GetPosition();

		PKT_S2C_Bomb_Dropped_s n;
		n.pos = gServerLogic.bomb_droppedPos;
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n), true);
	}

	m_carryingBomb = false;
	m_Health      = 0;
	isDead        = 1;
	deathTime     = r3dGetTime();
	NextSpawnNode = -1;
	numKillWithoutDying = 0;
	inventoryIndexForKills = 0;

	m_MorphineTime = 0;
	m_MorphineEffect = 1.0f;

	m_RequestedWeaponPickup = 0;
	m_RequestWeaponPickupSecretCode = 0;
	m_RequestedWeaponPickUpPrevOwner = invalidGameObjectID;
	m_DisableWeaponDrop = false;

	// reset current weapon selection on death
	ResetSelectedWeapon();	
  
	// reset capturing point, if any
	capturingPointId_ = 0;	

	// reset shot from history
	m_ShotFromHistory[0].networkID = -1;
	m_ShotFromHistory[1].networkID = -1;

	// ABILITY: Big Surprise ability
	if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(GetLoadoutData().hasItem(AbilityConfig::AB_BigSurprise))
		{
			m_BigSurpriseTimer = 3.0f;
			PKT_S2C_BigSurpriseAbility_s n;
			gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
		}
	}
  
	return;
}

void obj_ServerPlayer::DoRespawn(const r3dPoint3D& pos, float dir, float spawnProtection)
{
	r3d_assert(isDead);

	SetLatePacketsBarrier("respawn");

	numOfUAVHits = 0;
	isDead    = 0;
	m_Health  = getMaxHealth();
	bzOutTime = -1;
	m_InDamageAreaTime = 0;

	TeleportPlayer(pos);
	m_PlayerRotation = dir;

	// ptumik: do not remove this one, otherwise kicks players from sabo games :)
	lastPlayerAction_ = r3dGetTime();

	m_MorphineTime = 0;
	m_MorphineEffect = 1.0f;

	respawnInvulnerableTime = spawnProtection; 

	LastEnemyKillTime = 0;
	Killstreaks = 0;

	m_DamageReduction_LastTimeHit = 0;
	m_DamageReduction_BulletCounter = 0;

	// resupply ammo
	DoResupply();
	
	lastWeapDataRep = r3dGetTime();

	m_RocketKills = 0;
	m_AirstrikeKills = 0;
	m_CampingKills = 0;
	m_SniperKillsWithoutMissing = 0;
	m_LastSniperShotZoomed = false;
	m_HasDamagedSelf = false;

}

float obj_ServerPlayer::getMaxHealth()
{
	float maxHealth = 100.0f;
	if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		/*if(profile_.ProfileData.Skills[7] == 1)
			maxHealth *= 1.03f;
		else if(profile_.ProfileData.Skills[7] == 2)
			maxHealth *= 1.05f;
		else if(profile_.ProfileData.Skills[7] == 3)
			maxHealth *= 1.07f;
		else if(profile_.ProfileData.Skills[7] == 4)
			maxHealth *= 1.10f;
		else if(profile_.ProfileData.Skills[7] == 5)
			maxHealth *= 1.15f;*/
	}
	return maxHealth;
}


wiStatsTracking obj_ServerPlayer::AddReward(const wiStatsTracking& in_rwd)
{
  float XPBonus = boostXPBonus_;
  float WPBonus = boostWPBonus_;

  wiStatsTracking rwd = in_rwd;
  // round up. basically if we award only 2 points, with +25% it would give us 0, so, let's make users more happy by round up
  // in case if that will be a balancing problem - we can always round it down with floorf
  rwd.HP += int(ceilf(R3D_ABS(rwd.HP)*XPBonus));
  rwd.GP += int(ceilf(rwd.GP*WPBonus));
  rwd.GD += int(ceilf(rwd.GD*WPBonus));
  
  // adjust player stats
  DetailedReward_ += rwd;
  RoundStats_.GamePoints += rwd.GP;
  RoundStats_.GameDollars += rwd.GD;
  RoundStats_.HonorPoints += rwd.getTotalHP();
  
  // loadout stats
  if(m_SelectedLoadout >= 0)
	loadoutUsage_[m_SelectedLoadout].HonorPoints += rwd.getTotalHP();
  
  return rwd;
}

float obj_ServerPlayer::CalcWeaponDamage(const r3dPoint3D& shootPos)
{
  // calc damaged based on weapon
  // decay damage based from distance from player to target
  float dist   = (GetPosition() - shootPos).Length();
  float damage = m_WeaponArray[m_SelectedWeapon]->calcDamage(dist);

  return damage;
}

void obj_ServerPlayer::DoResupply()
{
	m_LastTimeResupplied = r3dGetTime();

	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; i++)
	{
		if(m_WeaponArray[i])
			m_WeaponArray[i]->Resupply();
	}
  
	return;
}

bool obj_ServerPlayer::FireWeapon(int wid, bool wasAiming, int executeFire, DWORD targetId, const char* pktName)
{
	lastPlayerAction_ = r3dGetTime();

	if(isDead)
	{
		//packet sent from player while he wasn't dead yet
		//gServerLogic.LogInfo(peerId_, true, "%s isDead", pktName);
		return false;
	}

	if(targetId)
	{
		GameObject* targetObj = GameWorld().GetNetworkObject(targetId);
		if(targetObj == NULL) 
		{
			// target already disconnected (100% cases right now) or invalid.
			return false;
		}
	}

	if(wid < 0 || wid >= NUM_WEAPONS_ON_PLAYER)
	{
		gServerLogic.LogInfo(peerId_, "wid invalid", "%s %d", pktName, wid);
		return false;
	}

	if(wid != m_SelectedWeapon) 
	{
		// just log for now... we'll see how much mismatches we'll get
		gServerLogic.LogInfo(peerId_, "wid mismatch", "%s %d vs %d", pktName, wid, m_SelectedWeapon);
	}

	Weapon* wpn = m_WeaponArray[wid];
	if(wpn == NULL) 
	{
		gServerLogic.LogInfo(peerId_, "no wid", "%s %d", pktName, wid);
		return false;
	}

	// incr fire count, decremented on hit event
	//r3dOutToLog("FireHitCount++\n");
	FireHitCount++;
	if(wpn->getCategory() == storecat_SHTG) // shotgun will shoot 8 pieces of a bullet
	{
		FireHitCount += 7;
	}

	if( wpn->getCategory() == storecat_SUPPORT )  
	{
		m_RocketKills = 0;
	}

	if( wpn->getCategory() == storecat_SNP )
	{
		m_LastSniperShotZoomed = wasAiming;
	}
	
	// track ShotsFired
	RoundStats_.ShotsFired++;
	if(wpn->getCategory() == storecat_SHTG) // shotgun will shoot 8 pieces of a bullet
	{
		RoundStats_.ShotsFired += 7;
	}
	gServerLogic.TrackWeaponUsage(wpn->getConfig()->m_itemID, 1, 0, 0);

	if(executeFire && gServerLogic.weaponDataUpdates_ < 2)
	{
		// check if we fired more that we was able
		if(wpn->getCategory() != storecat_MELEE) // melee - infinite ammo
			wpn->m_NumBulletsLeft--;

		if(wpn->m_NumBulletsLeft < -1)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_NumShots, true, "bullethack",
				"%d/%d(%d(%d)x%d), %s, %s", 
				wpn->m_NumBulletsLeft,
				wpn->m_NumClips * wpn->getConfig()->m_clipSize,
				wpn->m_NumClips, wpn->getConfig()->m_numClips, wpn->getConfig()->m_clipSize,
				wpn->getConfig()->m_StoreName, pktName
				);
			return false;
		}
		

	}
	
	return true;
}

float obj_ServerPlayer::ReduceDamageByGear(int gslot, float damage)
{
  if(gears_[gslot] == NULL)
    return damage;
    
  float armorProtectionMul = 1.0f;
  float armorProtectionAdd = 0.0f;
  if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
  {
	 if(GetLoadoutData().hasItem(AbilityConfig::AB_BulkUp))
		  armorProtectionMul += 0.1f; //+10% to armor rating
  
	  // combat skill
	  /*if(profile_.ProfileData.Skills[0] == 5) 
		  armorProtectionAdd += 0.15f;
	  if(profile_.ProfileData.Skills[0] == 4) 
		  armorProtectionAdd += 0.10f;
	  if(profile_.ProfileData.Skills[0] == 3) 
		  armorProtectionAdd += 0.07f;
	  if(profile_.ProfileData.Skills[0] == 2) 
		  armorProtectionAdd += 0.05f;
	  if(profile_.ProfileData.Skills[0] == 1) 
		  armorProtectionAdd += 0.03f;*/
  }

  float new_damage = gears_[gslot]->GetDamage(damage, armorProtectionMul, armorProtectionAdd, isTargetDummy_);
  return new_damage;
}

// params should be in [0..360] range
float getMinimumAngleDistance(float from, float to)
{
	float d = to - from;
	if(d <-180.0f)	d += 360.0f;
	if(d > 180.0f)	d -= 360.0f;
	return d;
}


float obj_ServerPlayer::ApplyDamage(float damage, GameObject* fromObj, int bodyPart, STORE_CATEGORIES damageSource)
{
	if(respawnInvulnerableTime > 0 || berserkerTime > 0)
		return 0.0f;

  lastTimeHit     = r3dGetTime();
  lastHitBodyPart = bodyPart;

  //r3dOutToLog("Player(%s) received gamage\n", userName); CLOG_INDENT;
  //r3dOutToLog("raw damage(%.2f) at part (%d)\n", damage, bodyPart);

  // adjust damage based on hit part
  if(damageSource != storecat_MELEE)
  {
	  switch(bodyPart) 
	  {
	  case 1: // head
		  damage *= 2;
		  break;

		  // case 2: // hands
	  case 3: // legs
		  damage *= 0.75f;
		  break;
	  }
  }

  // melee logic
  if(fromObj && fromObj->isObjType(OBJTYPE_Human) && damageSource == storecat_MELEE)
  {
	  const obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
	 
	  float fromPlrRot = fromPlr->m_PlayerRotation;
	  if(fromPlrRot > 360) fromPlrRot -= 360;
	  float angleDiff = getMinimumAngleDistance(fromPlrRot, m_PlayerRotation);
	  //r3dOutToLog("angle:%.2f\n", angleDiff);
	  // receiving damage from behind
	  if(R3D_ABS(angleDiff) < 60)
	  {
		  damage = 1000; // insta kill if from behind!
	  }
  }

  // EOD skill, protection from explosives
  if(damageSource == storecat_SUPPORT || damageSource == storecat_GRENADES)
  {
	  int WellArmoredSkill = GetLoadoutData().getSkillLevel(CUserSkills::SPEC_WellArmored);
	  switch(WellArmoredSkill)
	  {
	  case 1: damage *= 0.95f; break;
	  case 2: damage *= 0.90f; break;
	  case 3: damage *= 0.85f; break;
	  case 4: damage *= 0.80f; break;
	  default:break;
	  }
  }
  if(damageSource == storecat_GRENADES) // EOD skill, team explosive resistance
  {
	  /*int EODSkillLevel = gServerLogic.CheckTeamSkillAvailable(this, 3);
	  if(EODSkillLevel==4)
		  damage *= 0.96f;
	  else if(EODSkillLevel ==5)
		  damage *= 0.92f;*/
  }

  // small arms resistance from tactician team skill. look up small arms definition in wikipedia :)
  if(damageSource == storecat_ASR || damageSource == storecat_SNP || damageSource == storecat_HG || damageSource == storecat_SMG)
  {
	/*  int TacticianSkillLevel = gServerLogic.CheckTeamSkillAvailable(this, 6);
	  if(TacticianSkillLevel==4)
		  damage *= 0.96f;
	  else if(TacticianSkillLevel ==5)
		  damage *= 0.92f;*/
  }

  // reduce damage by armor		
  switch(bodyPart)
  {
    case 1: // head
      damage = ReduceDamageByGear(SLOT_Head2, damage);
      break;
    
    default:
      damage = ReduceDamageByGear(SLOT_Body, damage);
      damage = ReduceDamageByGear(SLOT_Armor, damage);
      break;
  }

 // r3dOutToLog("gear adjusted damage(%.2f)\n", damage);

  if(r3dGetTime() < m_MorphineTime)
	  damage *= m_MorphineEffect;
  
  if(damage < 0)
    damage = 0;
    
  // reduce health, not works for target dummy as well!
  if(!isTargetDummy_ || 1)
	m_Health -= damage;

  if( damage > 0 && fromObj == this ) 
  {
	  m_HasDamagedSelf = true;
  }
  
  //r3dOutToLog("%s damaged by %s by %.1f points, %.1f left\n", userName, fromObj->Name.c_str(), damage, m_Health);

  // assisted kill tracking
  if(fromObj) 
  {
    if(m_ShotFromHistory[0].networkID != fromObj->NetworkID)
    {
      m_ShotFromHistory[1] = m_ShotFromHistory[0];
      m_ShotFromHistory[0].networkID = fromObj->NetworkID;
      m_ShotFromHistory[0].timestamp = r3dGetTime();
    }
  }

  // BERSERKER ability
  if(damageSource!=storecat_INVALID) // if server is dealing damage - do not trigger berserker
  {
	  if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	  {
		  if(GetLoadoutData().hasItem(AbilityConfig::AB_Berserker))
		  {
			  if(m_Health < 20.0f && m_Health > 1.0f) // health between 1% and 20%
			  {
				  bool chance = u_GetRandom() < 0.1f; // 10% chance of having invincibility
				  if(chance)
				  {
					  r3dOutToLog("%s became Berserker!\n", userName);
					  berserkerTime = 5.0f; // X seconds of invincibility

					  PKT_S2C_BerserkerAbility_s n;
					  n.time = 5.0f;
					  gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
				  }
			  }
		  }
	  }
  }

  return damage;    
}

void obj_ServerPlayer::SetWeaponSlot(int wslot, int* weapId)
{
  r3d_assert(wslot < NUM_WEAPONS_ON_PLAYER);
  r3d_assert(m_WeaponArray[wslot] == NULL);

  if(*weapId == 0)
    return;

  if(gWeaponArmory.getWeaponConfig(*weapId)==NULL)
	  return;
    
  m_WeaponArray[wslot] = gWeaponArmory.createWeapon(*weapId, this, false, false);
  if(m_WeaponArray[wslot] != NULL) {
    //r3dOutToLog("weap%d: %s\n", wslot, m_WeaponArray[wslot]->getConfig()->m_StoreName);
    return;
  }
  
  // fix invalid slot
  *weapId = 0;
  r3dOutToLog("!!! %s does not have weapon id %d\n", userName, *weapId);
  return;
}

void obj_ServerPlayer::SetGearSlot(int gslot, int* gearId)
{
  r3d_assert(gslot >= 0 && gslot < SLOT_Max);
  r3d_assert(gears_[gslot] == NULL);
  
  if(*gearId == 0)
    return;
  
  gears_[gslot] = gWeaponArmory.createGear(*gearId);
  if(gears_[gslot] != NULL) {
    //r3dOutToLog("gear%d: %s\n", gslot, gears_[gslot]->getConfig()->m_StoreName);
    return;
  }

  *gearId = 0;
  r3dOutToLog("!!! %s does not have gear id %d\n", userName, *gearId);
  return;
}

int obj_ServerPlayer::SetLoadoutSlot(int lslot)
{
  r3dOutToLog("loadout: %s -> %d\n", userName, lslot); CLOG_INDENT;
  
  // ptumik: removed this, as player can pickup a weapon
//  if(m_SelectedLoadout == lslot)
  //  return lslot;

  if(lslot < 0 || lslot >= wiUserProfile::MAX_LOADOUT_SLOTS) {
    r3dOutToLog("!!! %s used wrong loadout slot %d\n", userName, lslot);
    lslot = 0;
  }
  
  if(profile_.ProfileData.ArmorySlots[lslot].LoadoutID == 0) {
    r3dOutToLog("!!! %s used not unlocked loadout slot %d\n", userName, lslot);
    lslot = 0;
  }
  
  // remember loadout play time
  if(m_SelectedLoadout != lslot)
  {
	// add time for previously selected layout
	if(m_SelectedLoadout >= 0)
	{
		loadoutUsage_[m_SelectedLoadout].AddPlayTime();
	}

	// start timer for new loadout
	loadoutUsage_[lslot].StartPlayTime = r3dGetTime();
  }
  
  m_SelectedLoadout = lslot;
  
  for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
    SAFE_DELETE(m_WeaponArray[i]);
  for(int i=0; i<SLOT_Max; ++i)
    SAFE_DELETE(gears_[i]);

  wiLoadoutSlot& slot = profile_.ProfileData.ArmorySlots[lslot];

  SetWeaponSlot(6, &slot.Item4);
  SetWeaponSlot(5, &slot.Item3);
  SetWeaponSlot(4, &slot.Item2);
  SetWeaponSlot(3, &slot.Item1);
  SetWeaponSlot(2, &slot.SidearmWeaponID);
  SetWeaponSlot(1, &slot.SecondaryWeaponID);
  SetWeaponSlot(0, &slot.PrimaryWeaponID);

  // if player have empty primary loadout slot, give player only AK74
  if(m_WeaponArray[0] == NULL)
  {
    r3dOutToLog("Giving AK47 as primary weapon\n");
    
    slot.PrimaryWeaponID = 101032;
    SetWeaponSlot(0, &slot.PrimaryWeaponID);
    if(m_WeaponArray[0] == NULL) {
      r3dError("there is no default AK74 weapon");
    }
  }

  ResetSelectedWeapon();
  
  if(slot.BodyMeshID == 0)
	  slot.BodyMeshID = 20000;
  if(slot.BodyHeadID == 0)
	  slot.BodyHeadID = 20019;

  bool hasHeroMesh = gWeaponArmory.getCategoryByItemId(slot.BodyMeshID)==storecat_Heroes;
  if(hasHeroMesh)
  {
	  slot.BodyArmorID = 0;
	  slot.BodyHeadID = 0;
	  slot.BodyHeadGearID = 0;
  }

  SetGearSlot(SLOT_Body,  &slot.BodyMeshID);
  SetGearSlot(SLOT_Armor, &slot.BodyArmorID);
  SetGearSlot(SLOT_Head1, &slot.BodyHeadID);
  SetGearSlot(SLOT_Head2, &slot.BodyHeadGearID);

  // check for upgrades
  r3dOutToLog("Checking for weapon upgrades\n");
  for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
  {
	  if(m_WeaponArray[i] && !isTargetDummy_)
	  {
		  // find that item in inventory
		  if(!profile_.isValidInventoryItem(m_WeaponArray[i]->getConfig()->m_itemID))
		  {
			  gServerLogic.LogInfo(peerId_, "Has item in loadout but not in inventory", "%d", m_WeaponArray[i]->getConfig()->m_itemID);
			  continue;
		  }
		  else
		  {
			  // set attachments
			  {
				  Weapon* wpn = m_WeaponArray[i];

				  const WeaponAttachmentConfig* wpnAttmConfigs[WPN_ATTM_MAX] = {0};
				  // scan for player attachments inventory
				  for(uint32_t i=0; i<profile_.ProfileData.NumFPSAttachments; i++)
				  {
					  const wiUserProfile::temp_fps_attach& att = profile_.ProfileData.FPSAttachments[i];
					  if(att.isEquipped && att.WeaponID == wpn->getConfig()->m_itemID)
					  {
						  const WeaponAttachmentConfig* wpnAttmConfig = gWeaponArmory.getAttachmentConfig(att.AttachmentID);
						  if(!wpnAttmConfig) {
							  r3dOutToLog("there is no attachment %d\n", att.AttachmentID);
							  continue;
						  }
						  wpnAttmConfigs[wpnAttmConfig->m_type] = wpnAttmConfig;
					  }
				  }
				  wpn->setWeaponAttachments(wpnAttmConfigs);
			  }
		  }
	  }
  }

  return lslot;
}

void obj_ServerPlayer::getAttachmentData(wiWeaponAttachments& attms) const
{
	attms.Reset();
	// only first three weapons have attachment info
	if(m_WeaponArray[0])
		m_WeaponArray[0]->getWeaponAttachmentIDs(attms.primary_attachments.attachments);
	if(m_WeaponArray[1])
		m_WeaponArray[1]->getWeaponAttachmentIDs(attms.secondary_attachments.attachments);
	if(m_WeaponArray[2])
		m_WeaponArray[2]->getWeaponAttachmentIDs(attms.sidearm_attachments.attachments);
}

int obj_ServerPlayer::getSelectedWeaponItemID() const 
{
	r3d_assert(m_SelectedWeapon >= 0);
	r3d_assert(m_WeaponArray[m_SelectedWeapon]);
	return m_WeaponArray[m_SelectedWeapon]->getConfig()->m_itemID;
}

BOOL obj_ServerPlayer::Update()
{
	r3d_assert(TeamID != TEAM_UNKNOWN);

	parent::Update();
  
	const float timePassed = r3dGetFrameTime();
	const float curTime = r3dGetTime();

	if(respawnInvulnerableTime > 0)
	{
		respawnInvulnerableTime -= timePassed;
		if(respawnInvulnerableTime < 0)
			respawnInvulnerableTime = 0;
	}
	if(berserkerTime > 0)
	{
		berserkerTime -= timePassed;
		if(berserkerTime < 0)
			berserkerTime = 0;
	}

	if(isDead) 
	{
		if(m_BigSurpriseTimer > 0) // only while dead
		{
			m_BigSurpriseTimer -= timePassed;
			if(m_BigSurpriseTimer <= 0)
			{
				m_BigSurpriseTimer = 0;
				PKT_S2C_SpawnExplosion_s n;
				n.pos =  GetPosition() + r3dPoint3D(0,0.25f,0);
				float BiggerBangRadius = 0.0f;
				int BiggerBangSkill = GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_BiggerBang);
				switch(BiggerBangSkill)
				{
				case 1: BiggerBangRadius = 1.0f; break;
				case 2: BiggerBangRadius = 2.0f; break;
				case 3: BiggerBangRadius = 3.0f; break;
				default:break;
				}
				n.radius = 3.0f + BiggerBangRadius;
				gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));

				// apply damage to everyone
				for(int i=0; i<ServerGameLogic::MAX_NUM_PLAYERS; ++i)
				{
					obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
					if(plr && !plr->isDead && ((GetPosition()-plr->GetPosition()).Length() < n.radius))
						if((plr->TeamID == TeamID && gServerLogic.ginfo_.friendlyFire) || plr->TeamID != TeamID) // friendly fire check 
							gServerLogic.ApplyDamage(this, plr, GetPosition(), 300.0f, true, storecat_GRENADES); // apply damage
				}
			}
		}
		return TRUE; 
	}
	else
		m_BigSurpriseTimer = 0; // reset

	// simple regeneration
	if(m_Health < getMaxHealth()) 
	{	
		// THIS CODE HAS TO BE IN SYNC WITH THE SAME CODE ON CLIENT
		if((curTime - lastTimeHit) > 5.0f || berserkerTime > 0)	// no regeneration for X sec after someone hit you
		{
			float regenSpeed = GPP_Data.c_fRegenSpeed;

			/* - disabled 2012-05-23 by Syncopate request
			// disable regen on sabotage on russian server
			if(RUS_CLIENT && gServerLogic.ginfo_.mapType == GBGameInfo::MAPT_Bomb)
				regenSpeed = 0.0f;
			*/

			int HealthRegenSkill = GetLoadoutData().getSkillLevel(CUserSkills::MEDIC_HealthRegeneration);
			switch(HealthRegenSkill)
			{
			case 1: regenSpeed *= 1.05f; break;
			case 2: regenSpeed *= 1.10f; break;
			case 3: regenSpeed *= 1.15f; break;
			case 4: regenSpeed *= 1.20f; break;
			case 5: regenSpeed *= 1.25f; break;
			default:break;
			}
			if(berserkerTime > 0)
			{
				int berserkerRage = GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_BerserkersRage);
				switch(berserkerRage)
				{
				case 1: regenSpeed *= 1.05f; break;
				case 2: regenSpeed *= 1.10f; break;
				case 3: regenSpeed *= 1.15f; break;
				case 4: regenSpeed *= 1.20f; break;
				case 5: regenSpeed *= 1.25f; break;
				default:break;
				}
			}

			m_Health = m_Health + timePassed*regenSpeed; //X hit points in one second

			// guardian angel
			float guardianAngelBoostRadius = 0.0f;
			int GuardianAngelSkill = GetLoadoutData().getSkillLevel(CUserSkills::MEDIC_GuardianAngel);
			switch(GuardianAngelSkill)
			{
			case 1: guardianAngelBoostRadius = 1.0f; break;
			case 2: guardianAngelBoostRadius = 2.0f; break;
			case 3: guardianAngelBoostRadius = 3.0f; break;
			case 4: guardianAngelBoostRadius = 4.0f; break;
			default:break;
			}

			if(gServerLogic.CheckTeamAbilityAvailable(this, AbilityConfig::AB_GuardianAngel, 10.0f+guardianAngelBoostRadius))
			{
				float healthNutBoost = 1.0f;
				switch(GetLoadoutData().getSkillLevel(CUserSkills::RECON_HealthNut))
				{
				case 1: healthNutBoost = 1.1f; break;
				case 2: healthNutBoost = 1.2f; break;
				case 3: healthNutBoost = 1.3f; break;
				}
				m_Health = m_Health + timePassed*(regenSpeed*4.0f)*healthNutBoost; // 20 points per second
			}

			m_Health = R3D_MIN(m_Health, getMaxHealth());
			if ( m_Health == getMaxHealth() )
			{
				m_HasDamagedSelf = false;
			}
		}
	}

	// give x4 time for weapon packet to arrive (make sure it's bigger that r3dNetwork peers disconnect)
	if(!isTargetDummy_ && !isDead && curTime > (lastWeapDataRep + PKT_C2S_PlayerWeapDataRep_s::REPORT_PERIOD * 4))
	{
		gServerLogic.DisconnectPeer(peerId_, true, "no weapdatarep");
		return TRUE;
	}
	
	// afk kick
	const float AFK_KICK_TIME_IN_SEC = 90.0f;
	if(!isTargetDummy_ && !isDead && curTime > lastPlayerAction_ + AFK_KICK_TIME_IN_SEC)
	{
		if(profile_.ProfileData.isDevAccount)
		{
			// do nothing for admin accs
		}
		else
		{
			PKT_S2C_CheatWarning_s n;
			n.cheatId = PKT_S2C_CheatWarning_s::CHEAT_AFK;
			gServerLogic.p2pSendToPeer(peerId_, this, &n, sizeof(n), true);

			gServerLogic.DisconnectPeer(peerId_, false, "afk_kick");
			return TRUE;
		}
	}


	return TRUE;
}

void obj_ServerPlayer::RecalcBoundBox()
{
  float	x_size = 0.8f;
  float	z_size = x_size;
  float	y_size = Height;

  r3dPoint3D pos = GetPosition();
  r3dBoundBox bboxlocal;
  bboxlocal.Org.Assign(pos.X - x_size / 2, pos.Y, pos.Z - z_size / 2);
  bboxlocal.Size.Assign(x_size, y_size, z_size);
  SetBBoxLocal(bboxlocal);

  return;
}

BOOL obj_ServerPlayer::OnCollide(GameObject *tobj, CollisionInfo &trace)
{
  return TRUE;
}

void obj_ServerPlayer::TeleportPlayer(const r3dPoint3D& pos)
{
	SetPosition(pos);
	netMover.SrvSetCell(GetPosition());
	movePrevTime  = -1;
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PacketBarrier_s& n)
{
	// client switched to next sequence
	clientPacketSequence++;
	r3dOutToLog("peer%02d PKT_C2C_PacketBarrier_s %s %d vs %d\n", peerId_, packetBarrierReason, myPacketSequence, clientPacketSequence);
	packetBarrierReason = "";
	
	// reset move cheat detection
	movePrevTime = -1;
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_MoveSetCell_s& n)
{
	const float curTime = r3dGetTime();
	
	if(movePrevTime >= 0)
	{
		// for now we will check ONLY ZX, because somehow players is able to fall down
		r3dPoint3D p1 = netMover.SrvGetCell();
		r3dPoint3D p2 = n.pos;
		p1.y = 0;
		p2.y = 0;
		float dist = (p1 - p2).Length();
		moveAccumDist += dist;
		moveAccumTime += curTime - movePrevTime;
		movePrevTime   = curTime;
		
		float avgSpeed = moveAccumDist / moveAccumTime;
		
		// check every 10 sec and check against sprint speed with bigger delta
		if(moveAccumTime > 10.0f)
		{
			if(!isDead && avgSpeed > GPP_Data.AI_SPRINT_SPEED * 1.3f)
			{
				gServerLogic.LogInfo(peerId_, "CheatFastMove", 
					"dist: %f for %f, speed:%f\n", 
					moveAccumDist, 
					moveAccumTime, 
					avgSpeed
					);
			}
			
			movePrevTime = -1;
		}
		
		// check for teleport - more that 1.3 sec of sprint speed.
		if(!isDead && dist > GPP_Data.AI_SPRINT_SPEED * 1.3f)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_FastMove, false, "teleport",
				"%f, srvGetCell: %.2f, %.2f, %.2f; n.pos: %.2f, %.2f, %.2f", 
				dist, 
				netMover.SrvGetCell().x, netMover.SrvGetCell().y, netMover.SrvGetCell().z, 
				n.pos.x, n.pos.y, n.pos.z
				);
		}
	}
	
	// check if we need to reset accomulated speed
	if(movePrevTime < 0) 
	{
		movePrevTime  = curTime;
		moveAccumTime = 0.0f;
		moveAccumDist = 0.0f;
	}

	netMover.SetCell(n);

	// keep them guaranteed
	RelayPacket(&n, sizeof(n), true);
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_MoveRel_s& n)
{
	const CNetCellMover::moveData_s& md = netMover.DecodeMove(n);

	// update last action if we moved or rotated
	if((GetPosition() - md.pos).Length() > 0.01f || m_PlayerRotation != md.turnAngle)
	{
		lastPlayerAction_ = r3dGetTime();
	}
	
	if ( (GetPosition() - md.pos).Length() > 0.01f ) 
	{
		m_CampingKills = 0;
	}

	SetPosition(md.pos);
	m_PlayerRotation = md.turnAngle;

	
	
	RelayPacket(&n, sizeof(n), false);
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerJump_s& n)
{
	RelayPacket(&n, sizeof(n), true);
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerSwitchWeapon_s& n)
{
	if(n.wid >= NUM_WEAPONS_ON_PLAYER) {
		gServerLogic.LogInfo(peerId_, "SwitchWeapon", "wrong weaponslot %d", n.wid);
		return;
	}

	if(m_WeaponArray[n.wid] == NULL) {
		gServerLogic.LogInfo(peerId_, "SwitchWeapon", "empty weaponslot %d", n.wid);
		return;
	}

	if(isDead) {
		gServerLogic.LogInfo(peerId_, "SwitchWeapon", "when dead");
		return;
	}

	m_SelectedWeapon = n.wid;

	RelayPacket(&n, sizeof(n));
}

void obj_ServerPlayer::UseItem_UAV(const r3dPoint3D& pos, float rotX)
{
	r3dOutToLog("%s requested UAV spawn\n", userName);
	if(gServerLogic.ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		r3dOutToLog("UAV Spawn failed due to being on Sabotage\n");
		return;
	}
	if(uavRequested_)
	{
		r3dOutToLog("UAV Spawn already requested\n");
		return;
	}

	char name[128];
	sprintf(name, "uav_%s", userName);
	obj_ServerUAV* uav = (obj_ServerUAV*)srv_CreateGameObject("obj_ServerUAV", name, pos);
	uav->ownerID      = GetSafeID();
	uav->peerId_      = peerId_;
	uav->NetworkID    = gServerLogic.net_lastFreeId++;
	uav->NetworkLocal = false;
	uav->SetRotationVector(r3dPoint3D(rotX, 0, 0));
	uav->OnCreate();
	
	// set that we have UAV
	uavRequested_ = true;
	uavId_        = uav->GetSafeID();

	gServerLogic.AddPlayerReward(this, RWD_UseUAV);
	
	// broadcast UAV creation
	{
		PKT_S2C_CreateNetObject_s n1;
		uav->fillInSpawnData(n1);
		gServerLogic.p2pBroadcastToActive(this, &n1, sizeof(n1), true);
	}
}

void obj_ServerPlayer::UseItem_MedKit(const r3dPoint3D& pos)
{
	{
		bool hasSkill = false;

		const wiLoadoutSlot& loadout = GetLoadoutData();
		if(loadout.getSkillLevel(CUserSkills::MEDIC_Medkit)>0)
			hasSkill = true;

		if(!hasSkill)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
				"%d", 
				WeaponConfig::ITEMID_MedKit
				);
			return;
		}
	}

	// spawn med kit
	obj_ServerMedKit* medkit = (obj_ServerMedKit*)srv_CreateGameObject("obj_ServerMedKit", "medkit", pos);
	medkit->itemID = WeaponConfig::ITEMID_MedKit;
	medkit->ownerID = GetSafeID();
	medkit->teamID = TeamID;
	medkit->spawnTime = r3dGetTime();
	medkit->SetRotationVector(r3dPoint3D(0,0,0));
	medkit->NetworkID = gServerLogic.net_lastFreeId++;

	PKT_S2C_CreateNetObject_s n; 
	medkit->fillInSpawnData(n);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerPlayer::UseItem_MotionSensor(const r3dPoint3D& pos, uint32_t itemID)
{
	if((GetPosition() - pos).Length() > 5.0f)
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "distance",
			"%d", 
			itemID
			);
		return;
	}
	if(itemID == WeaponConfig::ITEMID_MotionSensor)
	{
		bool hasSkill = false;

		const wiLoadoutSlot& loadout = GetLoadoutData();
		if(loadout.getSkillLevel(CUserSkills::RECON_MotionSensor)>0)
			hasSkill = true;

		if(!hasSkill)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
				"%d", 
				itemID
				);
			return;
		}
	}
	// spawn
	obj_ServerMotionSensor* sensor= (obj_ServerMotionSensor*)srv_CreateGameObject("obj_ServerMotionSensor", "sensor", pos);
	sensor->itemID = itemID;
	sensor->ownerID = GetSafeID();
	sensor->isConsumableVersion= itemID == WeaponConfig::ITEMID_MotionSensorConsumable;
	sensor->SetRotationVector(r3dPoint3D(0,0,0));
	sensor->NetworkID = gServerLogic.net_lastFreeId++;

	PKT_S2C_CreateNetObject_s n; 
	sensor->fillInSpawnData(n);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerPlayer::UseItem_RespawnBeacon(const r3dPoint3D& pos, float RotX, uint32_t itemID)
{
	if((GetPosition() - pos).Length() > 5.0f)
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "distance",
			"%d", 
			itemID
			);
		return;
	}
	if(itemID == WeaponConfig::ITEMID_RespawnBeacon)
	{
		bool hasSkill = false;

		const wiLoadoutSlot& loadout = GetLoadoutData();
		if(loadout.getSkillLevel(CUserSkills::ASSAULT_SpawnBeacon)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::RECON_RespawnBeacon)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::SPEC_SpawnBeacon)>0)
			hasSkill = true;

		if(!hasSkill)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
				"%d", 
				itemID
				);
			return;
		}
	}

	// spawn
	obj_ServerRespawnBeacon* beacon= (obj_ServerRespawnBeacon*)srv_CreateGameObject("obj_ServerRespawnBeacon", "beacon", pos);
	beacon->itemID = itemID;
	beacon->ownerID = GetSafeID();
	beacon->isConsumableVersion= itemID == WeaponConfig::ITEMID_RespawnBeaconCons;
	beacon->SetRotationVector(r3dPoint3D(RotX,0,0));
	beacon->NetworkID = gServerLogic.net_lastFreeId++;

	PKT_S2C_CreateNetObject_s n; 
	beacon->fillInSpawnData(n);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerPlayer::UseItem_RiotShield(const r3dPoint3D& pos, float rotX, uint32_t itemID)
{
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(itemID);
	if(!wc)
		return;

	if((GetPosition() - pos).Length() > 5.0f)
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "distance",
			"%d", 
			itemID
			);
		return;
	}

	if(itemID == WeaponConfig::ITEMID_RiotShield)
	{
		bool hasSkill = false;
		
		const wiLoadoutSlot& loadout = GetLoadoutData();
		if(loadout.getSkillLevel(CUserSkills::ASSAULT_RiotShield)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::SPEC_RiotShield)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::MEDIC_RiotShield)>0)
			hasSkill = true;

		if(!hasSkill)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
				"%d", 
				itemID
				);
			return;
		}
	}

	if(itemID == WeaponConfig::ITEMID_RiotShield)
	{
		for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_GameplayItem))
			{
				if(obj->Class->Name == "obj_ServerRiotShield")
				{
					obj_ServerRiotShield* shield = (obj_ServerRiotShield*)obj;
					if(shield->ownerID == GetSafeID() && !shield->isConsumableVersion)
					{
						gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "double use",
							"%d", 
							itemID
							);
						return;
					}
				}
			}
		}
	}

	// spawn
	obj_ServerRiotShield* shield= (obj_ServerRiotShield*)srv_CreateGameObject("obj_ServerRiotShield", "riotshield", pos);
	shield->itemID = itemID;
	shield->ownerID = GetSafeID();
	shield->isConsumableVersion= itemID == WeaponConfig::ITEMID_RiotShieldConsumable;
	if(itemID == WeaponConfig::ITEMID_RiotShield)
	{
		float baseHealth = wc->m_AmmoDamage;
		switch(GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_RiotShield))
		{
		case 2:
			shield->Health = baseHealth * 1.10f; break;
		case 3:
			shield->Health = baseHealth * 1.15f; break;
		case 4:
			shield->Health = baseHealth * 1.20f; break;
		case 5:
			shield->Health = baseHealth * 1.25f; break;
		default:
			shield->Health = baseHealth; break;
		}
		
		switch(GetLoadoutData().getSkillLevel(CUserSkills::SPEC_RiotShield))
		{
		case 2:
			shield->Health = baseHealth * 1.10f; break;
		case 3:
			shield->Health = baseHealth * 1.15f; break;
		case 4:
			shield->Health = baseHealth * 1.20f; break;
		case 5:
			shield->Health = baseHealth * 1.25f; break;
		default:
			shield->Health = baseHealth; break;
		}
		
		switch(GetLoadoutData().getSkillLevel(CUserSkills::SPEC_ArmoredUp))
		{
		case 1:
			shield->Health *= 1.10f; break;
		case 2:
			shield->Health *= 1.15f; break;
		case 3:
			shield->Health *= 1.20f; break;
		case 4:
			shield->Health *= 1.25f; break;
		default:
			break;
		}
	}
	else
		shield->Health = wc->m_AmmoDamage;

	shield->SetRotationVector(r3dPoint3D(rotX,0,0));
	shield->NetworkID = gServerLogic.net_lastFreeId++;

	PKT_S2C_CreateNetObject_s n; 
	shield->fillInSpawnData(n);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerPlayer::UseItem_AutoTurret(const r3dPoint3D& pos, float rotX, uint32_t itemID)
{
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(itemID);
	if(!wc)
		return;

	if((GetPosition() - pos).Length() > 5.0f)
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "distance",
			"%d", 
			itemID
			);
		return;
	}
	if(itemID == WeaponConfig::ITEMID_AutoTurret)
	{
		bool hasSkill = false;

		const wiLoadoutSlot& loadout = GetLoadoutData();
		if(loadout.getSkillLevel(CUserSkills::ASSAULT_AutoTurret)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::RECON_AutoTurret)>0)
			hasSkill = true;
		if(loadout.getSkillLevel(CUserSkills::SPEC_AutoTurret)>0)
			hasSkill = true;

		if(!hasSkill)
		{
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
				"%d", 
				itemID
				);
			return;
		}
	}

	if(itemID == WeaponConfig::ITEMID_AutoTurret)
	{
		for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_GameplayItem))
			{
				if(obj->Class->Name == "obj_ServerAutoTurret")
				{
					obj_ServerAutoTurret* turret = (obj_ServerAutoTurret*)obj;
					if(turret->ownerID == GetSafeID() && !turret->isConsumableVersion)
					{
						gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "double use",
							"%d", 
							itemID
							);
						return;
					}
				}
			}
		}
	}

	// spawn
	obj_ServerAutoTurret* turret= (obj_ServerAutoTurret*)srv_CreateGameObject("obj_ServerAutoTurret", "autoturret", pos);
	turret->peerId_      = peerId_;
	turret->itemID = itemID;
	turret->ownerID = GetSafeID();
	turret->isConsumableVersion= itemID == WeaponConfig::ITEMID_AutoTurretCons;
	turret->Health = wc->m_AmmoTimeout;
	turret->Damage = wc->m_AmmoDamage;
	turret->Range = wc->m_AmmoDecay;
	turret->SetRotationVector(r3dPoint3D(rotX,0,0));
	turret->NetworkID = gServerLogic.net_lastFreeId++;

	if(itemID == WeaponConfig::ITEMID_AutoTurret)
	{
		switch(GetLoadoutData().getSkillLevel(CUserSkills::SPEC_ArmoredUp))
		{
		case 1:
			turret->Health *= 1.10f; break;
		case 2:
			turret->Health *= 1.15f; break;
		case 3:
			turret->Health *= 1.20f; break;
		case 4:
			turret->Health *= 1.25f; break;
		default:
			break;
		}
	}

	PKT_S2C_CreateNetObject_s n; 
	turret->fillInSpawnData(n);
	gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerUseItem_s& n)
{
	// check if user have that item in loadout.
	if(!GetLoadoutData().hasItem(n.itemId))
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "noloadout",
			"%d", 
			n.itemId
			);
		return;
	}
	
	// NOTE: all multi-usage items must call and check for gServerLogic.RemoveItemFromPlayer
	switch(n.itemId)
	{
		case WeaponConfig::ITEMID_Cypher2:
			// var1 is rotX
			UseItem_UAV(n.pos, n.var1);
			break;

		case WeaponConfig::ITEMID_MedKit:
			// we cannot drop item too far away from player, so verify distance
			if((GetPosition() - n.pos).Length() > 5.0f)
			{
				gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "distance",
					"%d", 
					n.itemId
					);
				return;
			}
			UseItem_MedKit(n.pos);
			break;

		case WeaponConfig::ITEMID_Bandages:
		case WeaponConfig::ITEMID_Bandages2:
			{
				const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(n.itemId);
				if(wc)
				{
					if(gServerLogic.RemoveItemFromPlayer(this, n.itemId))
					{
						float bandageEffect = wc->m_AmmoDamage*0.01f;
						m_Health = m_Health + getMaxHealth()*bandageEffect; // ammoDamage sets % of restore
						m_Health = R3D_MIN(m_Health, getMaxHealth());

						PKT_S2C_PlayerUsedItemAns_s ans;
						ans.itemId = n.itemId;
						ans.var1 = bandageEffect;
						gServerLogic.p2pBroadcastToActive(this, &ans, sizeof(ans), true);
					}	
				}
			}
			break;

		case WeaponConfig::ITEMID_MotionSensorConsumable:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_MotionSensor:
			UseItem_MotionSensor(n.pos, n.itemId);
			break;

		case WeaponConfig::ITEMID_RespawnBeaconCons:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_RespawnBeacon:
			UseItem_RespawnBeacon(n.pos, n.var1, n.itemId);
			break;

		case WeaponConfig::ITEMID_RiotShieldConsumable:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_RiotShield:
			UseItem_RiotShield(n.pos, n.var1, n.itemId);
			break;

		case WeaponConfig::ITEMID_EpinephrineShot:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_AdrenalineShot:
			if(n.itemId == WeaponConfig::ITEMID_AdrenalineShot)
			{
				bool hasSkill = false;

				const wiLoadoutSlot& loadout = GetLoadoutData();
				if(loadout.getSkillLevel(CUserSkills::ASSAULT_AdrenalineShot)>0)
					hasSkill = true;
				if(loadout.getSkillLevel(CUserSkills::RECON_AdrenalineShot)>0)
					hasSkill = true;
				if(loadout.getSkillLevel(CUserSkills::SPEC_AdrenalineShot)>0)
					hasSkill = true;
				if(loadout.getSkillLevel(CUserSkills::MEDIC_AdrenalineShot)>0)
					hasSkill = true;

				if(!hasSkill)
				{
					gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
						"%d", 
						n.itemId);
					return;
				}
			}
			{
				PKT_S2C_PlayerUsedItemAns_s ans;
				ans.itemId = n.itemId;
				ans.var1 = n.itemId==WeaponConfig::ITEMID_EpinephrineShot?1.3f:1.1f; // speed boost
				ans.var2 = n.itemId==WeaponConfig::ITEMID_EpinephrineShot?5.0f:15.0f; // time
				gServerLogic.p2pBroadcastToActive(this, &ans, sizeof(ans), true);
			}

			break;

		case WeaponConfig::ITEMID_MethylmorphineShot:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_MorphineShot:
			if(n.itemId == WeaponConfig::ITEMID_MorphineShot)
			{
				bool hasSkill = false;

				const wiLoadoutSlot& loadout = GetLoadoutData();
				if(loadout.getSkillLevel(CUserSkills::RECON_MorphineShot)>0)
					hasSkill = true;
				if(loadout.getSkillLevel(CUserSkills::MEDIC_MorphineShot)>0)
					hasSkill = true;

				if(!hasSkill)
				{
					gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "no skill",
						"%d", 
						n.itemId);
					return;
				}
			}

			{
				if(n.itemId == WeaponConfig::ITEMID_MethylmorphineShot)
					m_MorphineTime = r3dGetTime() + 5.0f;
				else
					m_MorphineTime = r3dGetTime() + 15.0f;
				
				const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(n.itemId);
				if(wc)
				{
					m_MorphineEffect = 1.0f - R3D_CLAMP(wc->m_AmmoDamage*0.01f, 0.0f, 1.0f); // convert from 100 to 1.0f
				}
			}

			{
				PKT_S2C_PlayerUsedItemAns_s ans;
				ans.itemId = n.itemId;
				ans.var1 = n.itemId==WeaponConfig::ITEMID_MethylmorphineShot?5.0f:15.0f; // time
				gServerLogic.p2pBroadcastToActive(this, &ans, sizeof(ans), true);
			}

			break;

		case WeaponConfig::ITEMID_AutoTurretCons:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;
		case WeaponConfig::ITEMID_AutoTurret:
			UseItem_AutoTurret(n.pos, n.var1, n.itemId);
			break;

		case 101255:
			if(!gServerLogic.RemoveItemFromPlayer(this, n.itemId))
				return;

			// bah, bah.
			r3dOutToLog("SUPER ITEM ENABLED!\n");
			break;

		default:
			gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "baditemid",
				"%d", 
				n.itemId
				);
			break;
	}

	RelayPacket(&n, sizeof(n));
	return;
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerFired_s& n)
{
	
	if(!FireWeapon(n.debug_wid, n.wasAiming != 0, true, 0, "PKT_C2C_PlayerFired_s"))
	{
		return;
	}

	RelayPacket(&n, sizeof(n));
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerHitStatic_s& n)
{

	m_SniperKillsWithoutMissing = 0;

	//r3dOutToLog("FireHitCount--: PKT_C2C_PlayerHitStatic_s\n");
	FireHitCount--;
	if(FireHitCount < -10) // -10 - to allow some buffer
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_NumShots, true, "bullethack2");
		return;
	}

	RelayPacket(&n, sizeof(n), false);
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerHitStaticPierced_s& n)
{
	// just relay packet. not a real hit, just idintication that we pierced some static geometry, will be followed up by real HIT packet
	RelayPacket(&n, sizeof(n), false);
}


void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerHitNothing_s& n)
{
	m_SniperKillsWithoutMissing = 0;
	//r3dOutToLog("FireHitCount--: PKT_C2C_PlayerHitNothing_s\n");
	FireHitCount--;
	if(FireHitCount < -10) // -10 - to allow some buffer
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_NumShots, true, "bullethack2");
		return;
	}

	//RelayPacket(&n, sizeof(n), false);
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerHitDynamic_s& n)
{
	//r3dOutToLog("FireHitCount--: PKT_C2C_PlayerHitDynamic_s\n");
	FireHitCount--;
	if(FireHitCount < -10) // -10 - to allow some buffer
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_NumShots, true, "bullethack2");
		return;
	}

	// make sure we're shooting to another player
	GameObject* targetObj = GameWorld().GetNetworkObject(n.targetId);
	if(!targetObj)
	{
		m_SniperKillsWithoutMissing = 0;
		gServerLogic.LogInfo(peerId_, "HitBody0", "not valid targetId");
		return;
	}
	if(obj_ServerPlayer* targetPlr = IsServerPlayer(targetObj))
	{
		//@TODO: check and validate hitting

		//r3dOutToLog("hit from %s to %s\n", fromObj->Name.c_str(), targetObj->Name.c_str()); CLOG_INDENT;

		// check that shoot_pos is close to the targetObj, if not, that it is a hack
		if(n.damageFromPiercing == 0) // 0 - bullet didn't pierce anything
		{
			const float dist  = (n.hit_pos - targetObj->GetPosition()).Length();
			const float allow = GPP_Data.AI_SPRINT_SPEED*2.0f;
			if(dist > allow) // if more than Xsec of sprint
			{
				// ptumik: disabled cheat report, as we might receive packet for a player that is dead for client, but respawned on server -> distance difference
				//gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_ShootDistance, false, "HitBodyBigDistance",
				//	"hit %s, dist %f vs %f", 
				//	targetObj->Name.c_str(), dist, allow
				//	);
				return;
			}
		}
		if(m_WeaponArray[m_SelectedWeapon]->getCategory()==storecat_MELEE)
		{
			if(((GetPosition() - targetObj->GetPosition()).Length()) > 3.0f)
			{
				gServerLogic.LogInfo(peerId_, "HitBody0", "knife cheat");
				return;
			}
		}

		RelayPacket(&n, sizeof(n));

		float damage = CalcWeaponDamage(targetObj->GetPosition());
		if(n.damageFromPiercing > 0)
		{
			float dmod = float(n.damageFromPiercing)/100.0f;
			damage *= dmod;
		}
		if(gServerLogic.ApplyDamageToPlayer(this, (obj_ServerPlayer*)targetObj, this->GetPosition()+r3dPoint3D(0,1,0), damage, n.hit_body_bone, n.hit_body_part, false, m_WeaponArray[m_SelectedWeapon]->getCategory()))
		{
			RoundStats_.ShotsHits++;

			//HACK: track Kill here, because we can't pass weapon ItemID to ApplyDamageToPlayer yet
			int isKill = ((obj_ServerPlayer*)targetObj)->isDead ? 1 : 0;
			gServerLogic.TrackWeaponUsage(m_WeaponArray[m_SelectedWeapon]->getConfig()->m_itemID, 0, 1, isKill);
		}
	}
	else 
	{
		m_SniperKillsWithoutMissing = 0;

		if(gServerLogic.CanDamageThisObject(targetObj))
		{
			// track ShotsHits
			RoundStats_.ShotsHits++;
			gServerLogic.TrackWeaponUsage(m_WeaponArray[m_SelectedWeapon]->getConfig()->m_itemID, 0, 1, 0);

			r3dOutToLog("hit from %s to %s\n", Name.c_str(), targetObj->Name.c_str()); CLOG_INDENT;

			//@TODO: check and validate hitting

			RelayPacket(&n, sizeof(n));

			// apply damage
			float damage = CalcWeaponDamage(n.hit_pos);
			if(n.damageFromPiercing > 0)
			{
				float dmod = float(n.damageFromPiercing)/100.0f;
				damage *= dmod;
			}
			gServerLogic.ApplyDamage(this, targetObj, this->GetPosition()+r3dPoint3D(0,1,0), damage, false, m_WeaponArray[m_SelectedWeapon]->getCategory() );
		}
		else
		{
			gServerLogic.LogInfo(peerId_, "HitBody1", "hit object that is not damageable!");
		}
	}
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerReadyGrenade_s& n)
{
	RelayPacket(&n, sizeof(n));
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2S_PlayerCreateMine_s& n)
{
	//r3dOutToLog("FireHitCount--: PKT_C2S_PlayerCreateMine_s\n");
	FireHitCount--;
	if(FireHitCount < -10) // -10 - to allow some buffer
	{
		gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_NumShots, true, "bullethack2");
		return;
	}

	// check that we are trying to create mine
	if(m_WeaponArray[n.wid]->getConfig()->category != storecat_GRENADES)
	{
		gServerLogic.LogInfo(peerId_, "CreateMine1", "wid %d isn't a grenade type", n.wid);
		return;
	}
	uint32_t itemID = m_WeaponArray[n.wid]->getConfig()->m_itemID;

	char name[128];
	sprintf(name, "mine_%s", userName);
	obj_ServerMine* mine = (obj_ServerMine*)srv_CreateGameObject("obj_ServerMine", name, n.pos);
	mine->ownerID      = GetSafeID();
	mine->NetworkID    = gServerLogic.net_lastFreeId++;
	mine->NetworkLocal = true;
	mine->m_wc         = gWeaponArmory.getWeaponConfig(itemID);
	mine->SetRotationVector(n.rot);
	if(mine->m_wc == NULL)
	{
		r3dOutToLog("ERRROR!!!! Unknown weapon itemID %d\n", itemID);
		mine->setActiveFlag(0);
		return;
	}

	mine->OnCreate();

	// broadcast mine creation
	{
		PKT_S2C_SpawnMine_s n1;
		mine->fillInSpawnData(n1);
		gServerLogic.p2pBroadcastToActive(this, &n1, sizeof(n1), true);
	}
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2C_PlayerSwitchMine_s& n)
{
	// firstly, check that player has ability to switch mine owner
	if(!GetLoadoutData().hasItem(AbilityConfig::AB_TheFixer))// fixer ability
	{
		gServerLogic.LogInfo(peerId_, "PlayerSwitchMine", "has no fixer ability");
		return;
	}

	GameObject* obj = GameWorld().GetNetworkObject(n.mineID);
	if(!(obj && obj->isObjType(OBJTYPE_Mine)))
	{
		gServerLogic.LogInfo(peerId_, "PlayerSwitchMine", "mineID %d is invalid", n.mineID);
		return;
	}

	obj_ServerMine* mine = (obj_ServerMine*)obj;
	obj_ServerPlayer* mineOwner = IsServerPlayer(GameWorld().GetObject(mine->ownerID));
	if(mineOwner == NULL)
		return;

	if(mineOwner->TeamID != TeamID) // switch mine owner
	{
		int chance = 50;
		switch(GetLoadoutData().getSkillLevel(CUserSkills::SPEC_EnhancedFixer))
		{
		case 1: chance = 75; break;
		case 2: chance = 100; break;
		}

		if(random(99) < chance)
		{
			markAchievementComplete( ACHIEVEMENT_DISARM_REWIRE_EXPLOSIVE_DEVICE );
			mine->switchOwner(this);
		}
		else // failed, mine triggered
			mine->onExplode(false); 
	}
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2S_PlayerWeapDataRep_s& n)
{
	//r3dOutToLog("PKT_C2S_PlayerWeapDataRep\n");
	
	lastWeapDataRep = r3dGetTime();
	
	// if weapon data was updated more that once it mean that updated happened in middle of the game
	// so skip validation
	if(gServerLogic.weaponDataUpdates_ >= 2)
		return;

	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; i++)
	{
		if(m_WeaponArray[i] == NULL)
			continue;
		DWORD hash = m_WeaponArray[i]->getConfig()->GetClientParametersHash();
		
		if(hash == n.weaponsDataHash[i])
			continue;

		const WeaponConfig& wc1 = *m_WeaponArray[i]->getConfig();
		WeaponConfig wc2(n.debug_wid[i]); 
		wc2.copyParametersFrom(n.debug_winfo[i]);
			
		if(wc1.m_itemID != wc2.m_itemID)
		{
			gServerLogic.LogInfo(peerId_, "weapDataRep mismatch",
				"%d - %d", wc1.m_itemID, wc2.m_itemID);
		}
		else
		{
			// create diffs string for logging
			char diffs[4096] = "";
			if(fabs((float)wc1.m_spread - (float)wc2.m_spread) > 0.01f)
				sprintf(diffs + strlen(diffs), "s:%.2f/%.2f ", (float)wc1.m_spread, (float)wc2.m_spread);
			if(fabs((float)wc1.m_recoil - (float)wc2.m_recoil) > 0.01f)
				sprintf(diffs + strlen(diffs), "r:%.2f/%.2f ", (float)wc1.m_recoil, (float)wc2.m_recoil);
			if((int)wc1.m_numClips != (int)wc2.m_numClips)
				sprintf(diffs + strlen(diffs), "n:%d/%d ", (int)wc1.m_numClips, (int)wc2.m_numClips);
			if((int)wc1.m_clipSize != (int)wc2.m_clipSize)
				sprintf(diffs + strlen(diffs), "c:%d/%d ", (int)wc1.m_clipSize, (int)wc2.m_clipSize);
			if(fabs((float)wc1.m_reloadTime - (float)wc2.m_reloadTime) > 0.01f)
				sprintf(diffs + strlen(diffs), "r:%.2f/%.2f ", (float)wc1.m_reloadTime, (float)wc2.m_reloadTime);
			if(fabs((float)wc1.m_fireDelay - (float)wc2.m_fireDelay) > 0.01f)
				sprintf(diffs + strlen(diffs), "f:%.2f/%.2f ", (float)wc1.m_fireDelay, (float)wc2.m_fireDelay);
			
			if(diffs[0])
			{
				gServerLogic.LogCheat(peerId_, PKT_S2C_CheatWarning_s::CHEAT_BadWeapData, false, "weapDataRep different",
					"id:%d, diff:%s", wc1.m_itemID, diffs
					);
			}
		}

	}
}

void obj_ServerPlayer::OnNetPacket(const PKT_C2S_TEST_PlayerSetWeapon_s& n)
{
	// temporary code to change primary player weapon
	int weapId = n.weapId;
	SAFE_DELETE(m_WeaponArray[0]);
	SetWeaponSlot(0, &weapId);

	r3dOutToLog("%s changed primary weapon to %s\n", userName, m_WeaponArray[0]->getConfig()->m_StoreName);
}

void obj_ServerPlayer::SetLatePacketsBarrier(const char* reason)
{
	if(isTargetDummy_)
		return;

	r3dOutToLog("peer%02d, SetLatePacketsBarrier: %s\n", peerId_, reason);
	
	packetBarrierReason = reason;
	myPacketSequence++;
	lastWeapDataRep = r3dGetTime();
		
	PKT_C2C_PacketBarrier_s n;
	gServerLogic.p2pSendToPeer(peerId_, this, &n, sizeof(n));

	// NOTE:
	// from now on, we'll ignore received packets until client ack us with same barrier packet.
	// so, any fire/move/etc requests that will invalidate logical state of player will be automatically ignored
}


#undef DEFINE_GAMEOBJ_PACKET_HANDLER
#define DEFINE_GAMEOBJ_PACKET_HANDLER(xxx) \
	case xxx: { \
		const xxx##_s&n = *(xxx##_s*)packetData; \
		if(packetSize != sizeof(n)) { \
			r3dOutToLog("!!!!errror!!!! %s packetSize %d != %d\n", #xxx, packetSize, sizeof(n)); \
			return TRUE; \
		} \
		OnNetPacket(n); \
		return TRUE; \
	}

BOOL obj_ServerPlayer::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	//@TODO somehow check that originator of that packet have playerIdx that match peer

	// packets that ignore packet sequence
	switch(EventID)
	{
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PacketBarrier);
	}
	
	if(myPacketSequence != clientPacketSequence)
	{
		// we get late packet after late packet barrier, skip it
		r3dOutToLog("peer%02d, CustomerID:%d LatePacket %d %s\n", peerId_, profile_.CustomerID, EventID, packetBarrierReason);
		return TRUE;
	}

	switch(EventID)
	{
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2S_PlayerWeapDataRep);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveSetCell);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_MoveRel);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerJump);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerFired);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitNothing);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitStatic);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitStaticPierced);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerHitDynamic);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerReadyGrenade);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerSwitchWeapon);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerUseItem);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2S_PlayerCreateMine);
		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2C_PlayerSwitchMine);

		DEFINE_GAMEOBJ_PACKET_HANDLER(PKT_C2S_TEST_PlayerSetWeapon);
	}
  
	return FALSE;
}
#undef DEFINE_GAMEOBJ_PACKET_HANDLER

void obj_ServerPlayer::RelayPacket(const DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	gServerLogic.RelayPacket(peerId_, packetData, packetSize, guaranteedAndOrdered);
}

void obj_ServerPlayer::Draw(r3dCamera& Cam, eRenderStageID DrawState)
{
  return;
}

void obj_ServerPlayer::markAchievementComplete( int whichAchievement, bool ignorePractice )
{
	if ( ignorePractice || gServerLogic.canRecordAchievements() )
	{
		if( profile_.MarkAchievementComplete( whichAchievement ) == true ) 
		{
			const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
			PKT_S2C_UnlockAchievement_s n;
			n.achiIdx = achievementInfo->id;
			gServerLogic.p2pSendToPeer(peerId_, this, &n, sizeof(n), true);
		}
	}
}

void obj_ServerPlayer::checkAchievementByValue( int whichAchievement, int value )
{
	if ( gServerLogic.canRecordAchievements() )
	{
		if( profile_.CheckAchievementByValue( whichAchievement, value ) == true ) 
		{
			const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
			PKT_S2C_UnlockAchievement_s n;
			n.achiIdx = achievementInfo->id;
			gServerLogic.p2pSendToPeer(peerId_, this, &n, sizeof(n), true);
		}
	}
}

void obj_ServerPlayer::incrementAchievement( int whichAchievement, int value, bool ignorePractice )
{

	if ( ignorePractice || gServerLogic.canRecordAchievements() )
	{	
		if( profile_.IncrementAchievement( whichAchievement, value ) == true ) 
		{
			const AchievementConfig* achievementInfo = gWeaponArmory.getAchievementByIndex( whichAchievement);
			PKT_S2C_UnlockAchievement_s n;
			n.achiIdx = achievementInfo->id;
			gServerLogic.p2pSendToPeer(peerId_, this, &n, sizeof(n), true);
		}
	}
}
