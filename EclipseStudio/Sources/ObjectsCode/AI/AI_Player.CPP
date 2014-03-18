#include "r3dpch.h"
#include "r3d.h"

#include "APIScaleformGfx.h"
#include "AI_Player.h"
#include "AI_PlayerAnim.h"

#include "r3dPhysSkeleton.h"

#include "ObjectsCode\Effects\obj_ParticleSystem.h"

#include "Particle.h"

#include "ObjectsCode/world/DecalChief.h"
#include "ObjectsCode/world/MaterialTypes.h"

#include "ObjectsCode/weapons/Weapon.h"
#include "ObjectsCode/Airstrikes/Airstrike.h"

#include "ObjectsCode/weapons/Gear.h"
#include "ObjectsCode/WEAPONS/Ammo.h"
#include "ObjectsCode/weapons/WeaponArmory.h"
#include "ObjectsCode/weapons/ExplosionVisualController.h"
#include "ObjectsCode/WEAPONS/DroppedWeapon.h"
#include "ObjectsCode/WEAPONS/DroppedLootbox.h"
#include "ObjectsCode/WEAPONS/Mine.h"

#include "ObjectsCode/Gameplay/obj_SupplyCrate.h"
#include "ObjectsCode/Gameplay/obj_SiegeObjective.h"
#include "ObjectsCode/Gameplay/obj_BombPlacement.h"
#include "ObjectsCode/Gameplay/obj_DroppedBomb.h"
#include "ObjectsCode/Gameplay/obj_ControlPoint.h"
#include "ObjectsCode/Gameplay/obj_DamageArea.h"
#include "ObjectsCode/Gameplay/obj_UAV.h"
#include "../../GameEngine/gameobjects/obj_Vehicle.h"
#include "../Gameplay/obj_Zombie.h"

#include "ObjectsCode/WORLD/obj_LightMesh.h"

#include "multiplayer/ClientGameLogic.h"
#include "GameCode/UserProfile.h"
#include "GameCode/UserRewards.h"

#include "Gameplay_Params.h"

#include "CamouflageDataManager.h"

#include "../SF/CmdProcessor/CmdProcessor.h"
#include "../SF/CmdProcessor/CmdConsole.h"

#include "..\..\ui\HUDDisplay.h"
#include "..\..\ui\HUDMinimap.h"
#include "..\..\ui\HUD_EditorGame.h"
#include "..\..\ui\HUDRespawn.h"
#include "..\..\ui\FrontEndShared.h"
#include "..\..\ui\HUDCommCalls.h"
#include "..\..\ui\HUDLaserDesignator.h"
#include "..\..\ui\HUDPause.h"
#include "ui/m_EndRound.h"

extern HUDEndRound*	hudEndRound;
extern HUDDisplay*	hudMain;
extern HUDMinimap*	hudMinimap;
extern HUDRespawn*	hudRespawn;
extern HUDCommCalls* hudCommCalls;
extern HUDLaserDesignator* hudLaserDesignator;
extern HUDPause*	hudPause;

extern 	PlayerStateVars_s CurrentRig;

extern bool g_bEditMode;
extern int RUS_CLIENT;

extern void TPSGameHud_UnlockAchievement( int achievementID );

IMPLEMENT_CLASS(obj_AI_Player, "obj_AI_Player", "Object");
AUTOREGISTER_CLASS(obj_AI_Player);

float		_ai_fTurnSpeedIdle      = 360;
float		_ai_fTurnSpeedWalk	= 720;
float		_ai_NetPosUpdateDelta = 1.0f / 10;	// "30"

bool		g_CameraInsidePlayer = false;

#include "../../rendering/Deffered/PFX_ScopeEffect.h"
extern int g_RenderScopeEffect;
extern PFX_ScopeEffect gPFX_ScopeEffect;

r3dSec_type<CUberData*, 0x345fd82a> AI_Player_UberData = NULL;

r3dTexture* obj_AI_Player::s_damageHitColorCorrectionTex = NULL;
r3dTexture* obj_AI_Player::s_laserPointerTex = NULL;
r3dTexture* obj_AI_Player::s_flashlightFlashTex = NULL;
r3dTexture* obj_AI_Player::s_laserFlashTex = NULL;
r3dTexture* obj_AI_Player::s_lightHaloTex = NULL;
float obj_AI_Player::s_damageHitColorCorrectionVal = 0.0f;

r3dTexture* obj_AI_Player::s_UAVFriendlyBlinkTex = NULL;

r3dTexture* gPlayer_AuraTexture;
r3dTexture* gPlayer_CamoTexture;

gobjid_t m_LocalPlayer_CurrentAimAt;

#define TIME_FOR_SECOND_WIND 5.0f
#define KILLSTREAK_FOR_SECOND_WIND 3
static const float gDist_To_Allow_Vehicle_Entry = 5;
bool gDisableShortcuts = false;
struct PhysSkeletonCacheEntry
{
	int Taken ;
	r3dPhysSkeleton* Skel ;
};

typedef r3dTL::TArray< PhysSkeletonCacheEntry > PhysSkeletonCache ;

int g_bPhysSkeletonCacheInitialized ;
PhysSkeletonCache g_PhysSkeletonCache ;

// per shot
float SpreadIncrease[] = 
{
	0.15f, // ASR
	1.0f, // SNP
	0.5f, // SHTG
	0.45f, // MG
	0.0f, // SUPPORT
	0.5f, // HG
	0.1f, // SMG
	0.0f, // GRENADES
};
// per second
float SpreadDecrease[] = 
{
	4.0f, // ASR
	1.0f, // SNP
	4.0f, // SHTG
	2.0f, // MG
	4.0f, // SUPPORT
	1.0f, // HG
	4.0f, // SMG
	4.0f, // GRENADES
};
// percentage of weapon's spread
float SpreadMin[] = 
{
	.2f, // ASR
	1.0f, // SNP
	.25f, // SHTG
	.2f, // MG
	.1f, // SUPPORT
	.1f, // HG
	.15f, // SMG
	.1f, // GRENADES
};

void AddCachedPhysSkeleton()
{
	PhysSkeletonCacheEntry entry ;

	entry.Skel	= new r3dPhysSkeleton( "data/ObjectsDepot/Characters/RagDoll.RepX" ) ;
	entry.Taken	= 0 ;

	g_PhysSkeletonCache.PushBack( entry ) ;
}

void InitializePhysSkeletonCache( int count, float progressStart, float progressEnd )
{
	r3d_assert( !g_bPhysSkeletonCacheInitialized ) ;

	R3D_LOG_TIMESPAN_START(InitializeSkeletonCache);
	for( int i = 0, e = count ; i < e ; i ++ )
	{
		AddCachedPhysSkeleton() ;

		void SetLoadingProgress( float progress ) ;
		SetLoadingProgress( progressStart + ( progressEnd - progressStart ) * i / ( count - 1 ) ) ;
	}
	R3D_LOG_TIMESPAN_END(InitializeSkeletonCache);	

	g_bPhysSkeletonCacheInitialized = 1 ;
}

void DestroyPhysSkeletonCache()
{
	r3d_assert( g_bPhysSkeletonCacheInitialized ) ;

	for( int i = 0, e = g_PhysSkeletonCache.Count() ; i < e ; i ++ )
	{
		PhysSkeletonCacheEntry& entry = g_PhysSkeletonCache[ i ] ;
		delete entry.Skel ;
	}

	g_PhysSkeletonCache.Clear() ;

	g_bPhysSkeletonCacheInitialized = 0 ;
}

void ReleaseCachedPhysSkeleton( r3dPhysSkeleton* skel )
{
	if( skel )
	{
		for( int i = 0, e = g_PhysSkeletonCache.Count() ; i < e ; i ++ )
		{
			PhysSkeletonCacheEntry& entry = g_PhysSkeletonCache[ i ] ;
			if( entry.Skel == skel )
			{
				entry.Skel->unlink() ;
				entry.Skel->SwitchToRagdoll(false);
				entry.Taken = 0 ;
				return ;
			}
		}

		r3dError( "ReleaseCacheSkeleton: skeleton not found!" ) ;
	}
}

r3dPhysSkeleton* AquireCacheSkeleton()
{
	for( int i = 0, e = g_PhysSkeletonCache.Count() ; i < e ; i ++ )
	{
		PhysSkeletonCacheEntry& entry = g_PhysSkeletonCache[ i ] ;

		if( !entry.Taken )
		{
			entry.Taken = 1 ;
			return entry.Skel ;
		}
	}

	AddCachedPhysSkeleton() ;

	PhysSkeletonCacheEntry& entry = g_PhysSkeletonCache.GetLast() ;

	entry.Taken = 1 ;

	return entry.Skel ;
}

static void AdjustHitPosition(DecalParams& params, GameObject* target);

void AI_Player_FreeStuff()
{
	SAFE_DELETE(AI_Player_UberData);

	if(gPlayer_AuraTexture)
	{
		r3dRenderer->DeleteTexture(gPlayer_AuraTexture);

		gPlayer_AuraTexture = 0;
	}

	if(gPlayer_CamoTexture)
	{
		r3dRenderer->DeleteTexture(gPlayer_CamoTexture);

		gPlayer_CamoTexture = 0;
	}
}

#ifndef FINAL_BUILD
DECLARE_CMD(cw)
{
	if(ev.NumArgs() != 2)
	{
		ConPrint("cw {weapon id}");
		return;
	}
	
	obj_AI_Player* plr = gClientLogic().localPlayer_;
	if(plr == NULL)
	{
		ConPrint("not connected to server");
		return;
	}

	DWORD weapId = (DWORD)ev.GetInteger(1);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(weapId);
	if(wc == NULL)
	{
		ConPrint("there is no weapon id %d", weapId);
		return;
	}

	ConPrint("changing to weapon '%s', id:%d", wc->m_StoreName, weapId);
	ConPrint(" ammo damage: %f, decay: %f", wc->m_AmmoDamage, wc->m_AmmoDecay);
	
	PKT_C2S_TEST_PlayerSetWeapon_s n;
	n.weapId = (DWORD)weapId;
	p2pSendToHost(plr, &n, sizeof(n));

	// equip new player weapon
	SAFE_DELETE(plr->m_Weapons[0]);
	plr->m_Weapons[0] = gWeaponArmory.createWeapon(weapId, plr, false, false);
	plr->m_SelectedWeapon = -1;
	plr->ChangeWeaponByIndex(0);
	plr->SyncAnimation(true);
	return;
}
#endif

void _player_AdjustBoneCallback(DWORD dwData, int boneId, D3DXMATRIX &mp, D3DXMATRIX &anim)
{
	obj_AI_Player* npc = (obj_AI_Player*)dwData;
	if (!npc)
		return;

	if(npc->m_PhysSkeleton && npc->m_PhysSkeleton->IsRagdollMode())
		return;

	if(g_camera_mode->GetInt()==2 && npc->NetworkLocal) // FPS bone adjustments
	{
	}
	else // TPS bone adjustments
	{
		if(boneId == npc->boneId_Bip01_Spine1 || boneId == npc->boneId_Bip01_Neck) 
		{
			float xRot = npc->bodyAdjust_x;
			float yRot = npc->bodyAdjust_y[0];

			// add recoil for spine1
			if(boneId == npc->boneId_Bip01_Spine1)
			{
				if(!(npc->NetworkLocal && npc->hasScopeMode() && CurrentRig.allowScope)) // don't add recoil in scope mode to a weapon, as it messes up muzzle flash rendering
				{
					float recoil = npc->RecoilViewMod.y*0.5f;
					recoil = R3D_MIN(recoil, 0.0f);

					yRot += recoil;
				}
			}

			D3DXMATRIX r1, r2;

			// rotate previous matrix, with saved position
			r3dPoint3D vv = r3dPoint3D(mp._41, mp._42, mp._43);
			D3DXMatrixRotationY(&r2, xRot/2);
			D3DXMatrixRotationX(&r1, -yRot/2);
			mp = mp * (r1 * r2);
			mp._41 = vv.x;
			mp._42 = vv.y;
			mp._43 = vv.z;
		}
		else if(boneId == npc->boneId_Bip01_LUpperArm || boneId == npc->boneId_Bip01_RUpperArm)
		{
			float xRot = npc->RecoilViewMod.x*0.1f;
			float yRot = npc->RecoilViewMod.y*0.5f;

			xRot = 0;//-R3D_MIN(xRot, 0.5f); 
			yRot = -R3D_MIN(yRot, 0.1f);

			D3DXMATRIX r1, r2;

			// rotate previous matrix, with saved position
			r3dPoint3D vv = r3dPoint3D(mp._41, mp._42, mp._43);
			D3DXMatrixRotationY(&r2, xRot/2);
			D3DXMatrixRotationAxis(&r1, npc->GetvRightDraw().d3dx(), -yRot/2);
			mp = mp * (r1 * r2);
			mp._41 = vv.x;
			mp._42 = vv.y;
			mp._43 = vv.z;
		}
	}

	return;    
}

void obj_AI_Player::UpdateLoadoutSlot(const wiLoadoutSlot& slot, const wiWeaponAttachments& attms, bool canBeWithoutWeapon)
{
	R3DPROFILE_FUNCTION("obj_AI_Player::UpdateLoadoutSlot");

	// some meshes may change
	m_Loaded = 0 ;

	CurLoadout = slot; // save loadout
	CurWeaponAttachments = attms;
	m_GearWeight = 0;

	// always switch to primary weapon on update loadout (mimic server behavior)
	m_SelectedWeapon = 0;
	m_PrevSelectedWeapon = -1;

	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		SAFE_DELETE(m_Weapons[i]);

	bool isFirstPerson = g_camera_mode->GetInt()==2 && NetworkLocal;

	uberEquip_->isFirstPerson = isFirstPerson;

	if(slot.PrimaryWeaponID > 0)
		m_Weapons[0] = gWeaponArmory.createWeapon(slot.PrimaryWeaponID, this, isFirstPerson, true);
	if(slot.SecondaryWeaponID > 0)
		m_Weapons[1] = gWeaponArmory.createWeapon(slot.SecondaryWeaponID, this, isFirstPerson, true);
	if(slot.SidearmWeaponID > 0)
		m_Weapons[2] = gWeaponArmory.createWeapon(slot.SidearmWeaponID, this, isFirstPerson, true);

	if(slot.Item1 > 0 && gWeaponArmory.getWeaponConfig(slot.Item1))
		m_Weapons[3] = gWeaponArmory.createWeapon(slot.Item1, this, isFirstPerson, true);

	if(slot.Item2 > 0 && gWeaponArmory.getWeaponConfig(slot.Item2))
		m_Weapons[4] = gWeaponArmory.createWeapon(slot.Item2, this, isFirstPerson, true);

	if(slot.Item3 > 0 && gWeaponArmory.getWeaponConfig(slot.Item3))
		m_Weapons[5] = gWeaponArmory.createWeapon(slot.Item3, this, isFirstPerson, true);

	if(slot.Item4 > 0 && gWeaponArmory.getWeaponConfig(slot.Item4))
		m_Weapons[6] = gWeaponArmory.createWeapon(slot.Item4, this, isFirstPerson, true);

	if(!canBeWithoutWeapon)
	{
		if(m_Weapons[0] == 0 && m_Weapons[1] == 0 && m_Weapons[2] == 0)
		{
#ifndef FINAL_BUILD
			if(g_bEditMode)
			{
				m_Weapons[0] = gWeaponArmory.createWeapon(101035, this, isFirstPerson, true); // ak74
				m_Weapons[1] = gWeaponArmory.createWeapon(101109, this, isFirstPerson, true); // rpg7
				m_Weapons[2] = gWeaponArmory.createWeapon(101180, this, isFirstPerson, true); // desert eagle
				// added items is defined in EditorGameHUD::AddPlayer
				
				m_SelectedWeapon = 0;
			}
#endif
		}
	}

	UpdateLoadoutWeaponAttachments(attms);

	Weapon* selWeapon = m_Weapons[m_SelectedWeapon];

	// always add ak47 to primary weapon
	if(m_Weapons[0] == NULL)
		m_Weapons[0] = gWeaponArmory.createWeapon(101032, this, isFirstPerson, true);

	if(m_Weapons[m_SelectedWeapon] == NULL)
	{
		// this will happen only when server send selected weapon on spawn
		// that does not exists on client
		r3dError("Player does not have selected weapon\n");
	}

	if(NetworkLocal)
	{
		updateWeaponUI();
	}

	uberEquip_->SetSlot(SLOT_Weapon, m_Weapons[m_SelectedWeapon]);
	uberEquip_->SetSlot(SLOT_WeaponBackLeft, (Weapon*)NULL);
	uberEquip_->SetSlot(SLOT_WeaponBackLeftRPG, (Weapon*)NULL);
	uberEquip_->SetSlot(SLOT_WeaponBackRight, (Weapon*)NULL);
	uberEquip_->SetSlot(SLOT_WeaponSide, (Weapon*)NULL);

	if(m_Weapons[1] && m_SelectedWeapon != 1)
	{
		if(m_Weapons[1]->getCategory() == storecat_SUPPORT)
			uberEquip_->SetSlot(SLOT_WeaponBackLeftRPG, m_Weapons[1]);
		else
			uberEquip_->SetSlot(SLOT_WeaponBackLeft, m_Weapons[1]);
	}
	if(m_Weapons[2] && m_SelectedWeapon != 2)
		uberEquip_->SetSlot(SLOT_WeaponSide, m_Weapons[2]);

	Gear* bodyGear = gWeaponArmory.createGear(slot.BodyMeshID > 0 ? slot.BodyMeshID : 20000);
	bool hasHeroMesh = bodyGear->getCategory() == storecat_Heroes;

	const char* defaultWeaponMesh = 0;
	const char* defaultBodyMesh = 0;
	const char* defaultArmorMesh = 0;
	const char* defaultHead1Mesh = 0;
	const char* defaultHead2Mesh = 0;

	uberEquip_->SetSlot(SLOT_Body, bodyGear);

	if(uberEquip_->slots_[SLOT_Body].gear)
		m_GearWeight += uberEquip_->slots_[SLOT_Body].gear->GetWeight();

	if(slot.BodyArmorID>0 && !hasHeroMesh)
	{
		uberEquip_->SetSlot(SLOT_Armor, gWeaponArmory.createGear(slot.BodyArmorID));

		if(uberEquip_->slots_[SLOT_Armor].gear)
			m_GearWeight += uberEquip_->slots_[SLOT_Armor].gear->GetWeight();
	}
	else
		uberEquip_->SetSlot(SLOT_Armor, (Gear*)NULL);

	if(slot.BodyHeadID>0 && !hasHeroMesh)
		uberEquip_->SetSlot(SLOT_Head1, gWeaponArmory.createGear(slot.BodyHeadID));
	else
		uberEquip_->SetSlot(SLOT_Head1, (hasHeroMesh)?(Gear*)NULL:gWeaponArmory.createGear(20019));

	if(uberEquip_->slots_[SLOT_Head1].gear)
		m_GearWeight += uberEquip_->slots_[SLOT_Head1].gear->GetWeight();

	if(slot.BodyHeadGearID>0 && !hasHeroMesh)
	{
		uberEquip_->SetSlot(SLOT_Head2, gWeaponArmory.createGear(slot.BodyHeadGearID));

		if(uberEquip_->slots_[SLOT_Head2].gear)
			m_GearWeight += uberEquip_->slots_[SLOT_Head2].gear->GetWeight();
	}
	else
		uberEquip_->SetSlot(SLOT_Head2, (Gear*)NULL);

	int EngineerArmorWeightReductionSkill = CurLoadout.getSkillLevel(CUserSkills::SPEC_ArmorWeight);
	switch(EngineerArmorWeightReductionSkill)
	{
	case 1: m_GearWeight *= 0.85f; break;
	case 2: m_GearWeight *= 0.70f; break;
	case 3: m_GearWeight *= 0.55f; break;
	default: break;
	}
	int AssaultArmorWeightReductionSkill = CurLoadout.getSkillLevel(CUserSkills::ASSAULT_ArmorWeight);
	switch(AssaultArmorWeightReductionSkill)
	{
	case 1: m_GearWeight *= 0.95f; break;
	case 2: m_GearWeight *= 0.90f; break;
	case 3: m_GearWeight *= 0.85f; break;
	case 4: m_GearWeight *= 0.80f; break;
	case 5: m_GearWeight *= 0.70f; break;
	default: break;
	}

	// start loading meshes
	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
	{
		if(m_Weapons[i])
			m_Weapons[i]->getModel(true, isFirstPerson);
	}
	for(int i=0; i<SLOT_Max; ++i)
	{
		if(uberEquip_->slots_[i].gear)
			uberEquip_->slots_[i].gear->getModel(isFirstPerson);
	}

	uberAnim_->CurrentWeapon = NULL;
	SyncAnimation(true);
}

void obj_AI_Player::UpdateLoadoutWeaponAttachments(const struct wiWeaponAttachments& attms)
{
	if(m_Weapons[0])
		m_Weapons[0]->setWeaponAttachmentsByIDs(attms.primary_attachments.attachments);

	if(m_Weapons[1])
		m_Weapons[1]->setWeaponAttachmentsByIDs(attms.secondary_attachments.attachments);

	if(m_Weapons[2])
		m_Weapons[2]->setWeaponAttachmentsByIDs(attms.sidearm_attachments.attachments);
		
/*	// game mode, get attachments from local player profile
	// scan for primary-secondary-sidearm
	for(int wpnSlot = 0; wpnSlot < 3; wpnSlot++)
	{
		Weapon* wpn = m_Weapons[wpnSlot];
		if(!wpn) continue;
		
		const WeaponAttachmentConfig* wpnAttmConfigs[WPN_ATTM_MAX] = {0};

		// scan for player attachments inventory
		for(uint32_t i=0; i<gUserProfile.ProfileData.NumFPSAttachments; i++)
		{
			const wiUserProfile::temp_fps_attach& att = gUserProfile.ProfileData.FPSAttachments[i];
			if(att.isEquipped && att.WeaponID == wpn->getItemID())
			{
				const WeaponAttachmentConfig* wpnAttmConfig = gWeaponArmory.getAttachmentConfig(att.AttachmentID);
				if(!wpnAttmConfig) {
					r3dError("there is no attachment %d", att.AttachmentID);
					continue;
				}
				wpnAttmConfigs[wpnAttmConfig->m_type] = wpnAttmConfig;
			}
		}
		wpn->setWeaponAttachments(wpnAttmConfigs);
	}*/
}



obj_AI_Player::obj_AI_Player()
: footStepsSnd(0)
, currentFootStepsSoundID(-1)
, boneId_Bip01_Spine1(-1)
, boneId_Bip01_Neck(-1)
, boneId_Bip01_LUpperArm(-1)
, boneId_Bip01_RUpperArm(-1)
, boneId_Bip01_Head(-1)
, boneId_Bip01_R_Hand(-1)
, boneId_Bip01_L_Hand(-1)
, m_AuraType(AT_NONE)
, netMover(this, _ai_NetPosUpdateDelta, (float)PKT_C2C_MoveSetCell_s::PLAYER_CELL_RADIUS)
, m_Loaded( 0 )
, m_enableRendering(true)
#if ENABLE_ZOMBIES
, lifeProperties(this)
#endif // ENABLE_ZOMBIES
, m_RiotShieldMeshPlacement(NULL)
, m_RiotPersonalShieldMeshPlacement(NULL)
, m_AutoTurretMeshPlacement(NULL)
, m_RespawnBeaconMeshPlacement(NULL)
, m_ItemSimulateLeftClick(false)
{
	//explosionTime = 0;

	m_targetIconTime = 0;
	m_localTargetIconTime = 0;
	for(int i=0; i<MAX_UAV_TARGETS; ++i)
		m_localPlayer_lastUAVTarget[i] = invalidGameObjectID;

	m_visibleOnMinimap = false;
	uberEquip_ = NULL;
	m_PhysSkeleton = NULL;

	m_disablePhysSkeleton = false;

	lastTimeHit = 0;
	bOnGround =1;
	StartFallingHeight = 0;
	StartFallingTime = 0;
	bCrouch = 0;
	TimeOfDeath = 0;
	PosOfDeath.Assign(0,0,0);
	DisableKillerView = false;
	Dead_KillerID = 0;
	m_BloodTimer = 0;
	m_GearWeight = 0;
	m_haveBoostStamina = false;
	isPremiumAccount = false;
	ClanID         = 0;
	ClanTagColor   = 0xFFFFFFFF;
	r3dscpy(ClanTag, "CLAN");

	uavRequested_  = 0;
	uavViewActive_ = false;
	
	laserViewActive_ = false;

	viewTargetPos		= r3dPoint3D(0, 0, 0);
	viewTargetNorm		= r3dPoint3D(0, 1, 0);

	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		m_Weapons[i] = NULL;
	m_SelectedWeapon = 0;
	m_PrevSelectedWeapon = -1;
	
	rewards_.reserve(32);
	lastRewardShowTime_ = 0;


#if VEHICLES_ENABLED
	vehicleViewActive_ = VehicleView_None; 
#endif

#ifndef FINAL_BUILD
	// register "cw" command for changing player weapon online
	static bool cw_cmd_registered = false;
	if(!cw_cmd_registered)
	{
		cw_cmd_registered = true;
		REG_CCOMMAND(cw, 0, "Change current primary weapon");
	}
#endif
	
	if(g_pDecalChief && g_pDecalChief->GetTypeCount() > 254)
	{
		r3dError("more that 254 decals"); 
	}
	if(ImpactParticleEntry::NumRegistered > 254)
	{
		r3dError("more that 254 impact particles");
	}

	// create animation data first time player is created
	if(!AI_Player_UberData)
	{
		AI_Player_UberData = new CUberData();
	}

	if(!gPlayer_AuraTexture)
	{
		gPlayer_AuraTexture = r3dRenderer->LoadTexture("Data/Shaders/TEXTURE/Aura.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
	}

	if(!gPlayer_CamoTexture)
	{
		gPlayer_CamoTexture = r3dRenderer->LoadTexture("Data/Shaders/TEXTURE/plastic_nm.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_AI_Player::Load(const char *fname)
{
	if(!parent::Load(fname)) return FALSE;

	bPersistent 	= 0;

	ObjTypeFlags |= OBJTYPE_Human;

	Height 	 	= SRV_WORLD_SCALE (1.8f);
	TeamID		= TEAM_UNKNOWN;
	LoadoutSlot     = 0;
	m_EncryptedUserName.set("---");

	lastDamageRcvd = 0;
	lastTimeHitForce  = r3dPoint3D(1, 0, 0);
	lastTimeHitBone = 0;
	bDead                 = 0;
	BloodEffect = 0;
	m_ReticleCurrentScale = 1.0f;
	m_ReticleTargetScale = 1.0f;
	m_isAiming = false;
	m_isChangedAiming1 = false;
	m_isPressedFireTrigger = false;
	m_isFinishedAiming = false;
	m_isInScope = false;

	m_siegeArmingTimer = 0;

	grenadeReadyTime = 0;

	m_OrigBBox.Org.Assign( -0.5, 0, -0.5 );
	m_OrigBBox.Size.Assign( 1, 2, 1 );

	SetBBoxLocal( m_OrigBBox ) ;

	m_vVision.Assign(1, 0, 0);
	ViewAngle.Assign(0, 0, 0);
	RecoilViewMod.Assign(0,0,0);
	RecoilViewModTarget.Assign(0,0,0);
	RecoilViewModTarget2.Assign(0,0,0);
	SniperViewMod.Assign(0,0,0);
	RecoilCooldown = 0;
	m_fPlayerRotation      = 0;
	m_fPlayerRotationTarget     = 0;
	//UpdateRotation();

	D3DXMatrixIdentity(&DrawRotMatrix);
	D3DXMatrixIdentity(&DrawFullMatrix);
	D3DXMatrixIdentity(&MoveMatrix);

	JumpVelocity = 0;
	m_EnergyPenaltyTime = 0;
	//m_BreathPenaltyTime = 0;

	m_SpawnProtectedUntil = 0.0f;
	m_KillStreak = 0;
	m_LastKillTime = 0;
	m_RemainingSecondWindTime = 0.0f;
	m_isHoldingBreath = false;

	bodyAdjust_x    = 0;
	bodyAdjust_y[0] = 0;
	bodyAdjust_y[1] = 0;

	RealAcceleration    = r3dPoint3D(0, 0, 0);
	InputAcceleration    = r3dPoint3D(0, 0, 0);
	SetVelocity(r3dPoint3D(0, 0, 0));

	PlayerState	= PLAYER_IDLE;
	PlayerMoveDir   = 0;
	// we pack both vars into single byte in PKT_C2C_MoveRel.state, so check for overflow
	TL_STATIC_ASSERT(PLAYER_NUM_STATES <= 0xF);
	TL_STATIC_ASSERT(CUberData::ANIMDIR_COUNT <= 0xF);

	m_visibleOnMinimap = false;
	m_onMinimap = false;
	m_fVisibleTimer = 0;
	m_pingIconTimer = -1.0f;
	m_commRoseTimer = -1.0f;
	setHasSabotageBomb(false);

	m_HitMarkerFadeout = 0;
	m_HitMarkerTex = NULL;

	UpdateTransform();
	return TRUE;
}

BOOL obj_AI_Player::OnCreate()
{
	R3DPROFILE_FUNCTION("obj_AI_Player::OnCreate");

	R3D_LOG_TIMESPAN_START(Ai_Player_OnCreate);

	parent::OnCreate();

	afterRespawnTimer = 0;
	lastTimeWeaponSwitch = 0;
	TimeOfLastRespawn = r3dGetTime();
	m_QuickThrowGrenade = 0;
	m_BerserkerTime = 0;
	m_AuraType = AT_NONE;
	m_bombDropCooldown = 0;
	footsteps_timeSinceLastMissingHitResult = 0;

	m_needToDrawRiotShieldPlacement = 0;
	m_spawnedRiotShield = false;
	m_needToDrawAutoTurretPlacement = 0;
	m_spawnedAutoTurret = false;
	m_needToDrawRespawnBeaconPlacement = 0;

	m_canPlaceConsumableItem = false;

	m_SpeedBoost = 0;
	m_SpeedBoostTime = 0;
	m_MorhineShotTime =0;
	m_BandagesEffectTime = 0;

	CurrentSpread = 0;

	numLootBoxesPickedup = 0;
	numBombsPlanted = 0;
	numBombsDiffused = 0;
	numSaboRoundsWon = 0;

	shownWeaponPickup = false;

	if(NetworkLocal)
	{
		// because we can switch between TPS\FPS
		if(hudMain)
			hudMain->updateReticlePosition();

		ObjFlags |=	OBJFLAG_SkipOcclusionCheck // otherwise when you are in scope mode you might not see your own player, and so scope will not render. Anyway, we should see our player always
						| 
					OBJFLAG_ForceSceneBoxBBox ; 
		PhysicsConfig.group = PHYSCOLL_CHARACTERCONTROLLER;
		PhysicsConfig.type = PHYSICS_TYPE_CONTROLLER;
		PhysicsConfig.mass = 100.0f;
		PhysicsConfig.ready = true;
		PhysicsObject = BasePhysicsObject::CreateCharacterController(PhysicsConfig, this);

		if(hudRespawn && hudRespawn->isInit() && gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		{
			if(hudRespawn->getSelectedTeamID() != TeamID)
				hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_MovedToOppositeTeam"));
		}

		if(s_damageHitColorCorrectionTex == 0)
		{
			char tempstr[512];
			GetCCLUT3DTextureFullPath(tempstr, "DamegeHit.dds");
			s_damageHitColorCorrectionTex = r3dRenderer->LoadTexture( tempstr, D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem );
			r3d_assert(s_damageHitColorCorrectionTex != NULL);
			s_damageHitColorCorrectionVal = 0.0f;
		}

		if(s_UAVFriendlyBlinkTex == 0)
		{
			s_UAVFriendlyBlinkTex = r3dRenderer->LoadTexture("data/shaders/texture/PlayerBlinkSphere.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
			r3d_assert(s_UAVFriendlyBlinkTex!=NULL);
		}

		if(s_laserPointerTex == 0)
		{
			s_laserPointerTex = r3dRenderer->LoadTexture("data/shaders/texture/laserPointerDot.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem );
			r3d_assert(s_laserPointerTex!=NULL);
		}

		if(s_flashlightFlashTex == 0)
		{
			s_flashlightFlashTex = r3dRenderer->LoadTexture("data/shaders/texture/Light_Flashlight.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
			r3d_assert(s_flashlightFlashTex!=NULL);
		}

		if(s_laserFlashTex == 0)
		{
			s_laserFlashTex = r3dRenderer->LoadTexture("data/shaders/texture/Light_Laser.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
			r3d_assert(s_laserFlashTex!=NULL);
		}
	
		if(s_lightHaloTex == 0)
		{
			s_lightHaloTex = r3dRenderer->LoadTexture("data/shaders/texture/Light_Halo_01.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem);
			r3d_assert(s_lightHaloTex!=NULL);
		}

		m_HitMarkerTex = r3dRenderer->LoadTexture("data/weapons/hitmarker.dds", D3DFMT_UNKNOWN, false, 1, 1, 0, PlayerTexMem );
		r3d_assert(m_HitMarkerTex != NULL);
	}
	else
	{
		PhysicsConfig.group = PHYSCOLL_CHARACTERCONTROLLER;
		PhysicsConfig.type = PHYSICS_TYPE_RAYCAST_BOX;
		PhysicsConfig.mass = 100.0f;
		// NOTE : if you change this, sync with PHYSICS_TYPE_CONTROLLER offset...
		PhysicsConfig.offset = r3dPoint3D( 0, 0.9f, 0 );
		PhysicsObject = BasePhysicsObject::CreateDynamicObject(PhysicsConfig, this);
		wchar_t tempStr[64];
        char plrUserName[64]; GetUserName(plrUserName);
		swprintf(tempStr, 64, gLangMngr.getString("$HUD_Msg_JoinedGame"), utf8ToWide(plrUserName));
		if(hudMain)
			hudMain->AddMessage(tempStr);

		if(hudMain)
		{
			hudMain->addWoundedIcon(m_WoundedTeamIcon);
			hudMain->moveScreenIcon(m_WoundedTeamIcon, r3dPoint3D(-1, -1, -1), false, true);
		}

		if(hudMain)
		{
			hudMain->addPingIcon(m_PingTeamIcon);
			hudMain->moveScreenIcon(m_PingTeamIcon, r3dPoint3D(-1, -1, -1), false, true);
		}

		if( hudMain )
		{
			hudMain->addIconSpawnIcon(m_SpawnProtectIcon);
			hudMain->moveScreenIcon(m_SpawnProtectIcon, r3dPoint3D(-1, -1, -1), false, true);
			
		}
	}

	m_Energy = getMaxStamina();
	m_Breath = getMaxBreath();

	// init animation
	uberEquip_ = new CUberEquip(this);
	r3d_assert(AI_Player_UberData);
	uberAnim_ = new CUberAnim(this, AI_Player_UberData);

	boneId_Bip01_Spine1    = uberAnim_->GetBoneID("Bip01_Spine1");
	boneId_Bip01_Spine2    = uberAnim_->GetBoneID("Bip01_Spine2");
	boneId_Bip01_Neck      = uberAnim_->GetBoneID("Bip01_Neck");
	boneId_Bip01_LUpperArm = uberAnim_->GetBoneID("Bip01_L_UpperArm");
	boneId_Bip01_RUpperArm = uberAnim_->GetBoneID("Bip01_R_UpperArm");
	boneId_Bip01_Head      = uberAnim_->GetBoneID("Bip01_Head");
	boneId_Bip01_R_Hand	   = uberAnim_->GetBoneID("Bip01_R_Hand");
	boneId_Bip01_L_Hand	   = uberAnim_->GetBoneID("Bip01_L_Hand");

	// setup boosts
	if(NetworkLocal)
	{
		if(gUserProfile.getInventoryItemByID(301008) != -1)
			m_haveBoostStamina = true;
	}

	UpdateLoadoutSlot(CurLoadout, CurWeaponAttachments);

	PlayerState = PLAYER_IDLE;
	SyncAnimation(true);

	// force animation update here, so that phys skeleton can be updated, otherwise creating too many of them at 0,0,0 causing physX to crash (too many pairs created)
	{
		D3DXMATRIX mr;
		D3DXMatrixRotationY(&mr, R3D_PI);
		D3DXMATRIX CharDrawMatrix = mr;

		UpdateAnimations(CharDrawMatrix);
		UpdateRotation();
	}

	// create phys skeleton after we have animation data
	if(!m_disablePhysSkeleton)
	{
		m_PhysSkeleton = AquireCacheSkeleton() ;
		m_PhysSkeleton->linkParent(GetSkeleton(), DrawFullMatrix, this, NetworkLocal ? PHYSCOLL_LOCALPLAYER : PHYSCOLL_NETWORKPLAYER) ;
		m_PhysSkeleton->SwitchToRagdoll(false);
	}

	if(!NetworkLocal)
	{
		if(gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		{
			if(CurLoadout.hasItem(AbilityConfig::AB_GuardianAngel)) // guardian angel
			{
				if(hudMain)
				{
					hudMain->addMedicIcon(m_MedicTeamIcon);
					hudMain->moveScreenIcon(m_MedicTeamIcon, r3dPoint3D(-1, -1, -1), false, true);
					hudMain->setScreenIconScale(m_MedicTeamIcon, 0.5f);
				}
			} 
		}
	}

	// if we was dead on spawn
	if(bDead) 
	{
		r3d_assert(!NetworkLocal);

		PlayerState = PLAYER_DIE;
		uberAnim_->AnimPlayerState = PLAYER_DIE;
		uberAnim_->StartDeathAnim();
		DoRagdoll(true);

		// fucking physx
		SetPosition(GetPosition());
	}

	lastWeapDataRep = r3dGetTime();

	TeleportPlayer(GetPosition(), "OnCreate");
	m_Health	= getMaxHealth();

	R3D_LOG_TIMESPAN_END(Ai_Player_OnCreate);

	return TRUE;
}

BOOL obj_AI_Player::OnDestroy()
{
	wchar_t tempStr[64];
	char plrUserName[64]; GetUserName(plrUserName);
	swprintf(tempStr, 64, gLangMngr.getString("$HUD_Msg_LeftGame"), utf8ToWide(plrUserName));
	if(hudMain)
		hudMain->AddMessage(tempStr);

	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
	{
		SAFE_DELETE(m_Weapons[i]);
	}

	if(hudMain)
	{
		if(!m_MarkedTargetIcon.IsUndefined())
		{
			hudMain->deleteScreenIcon(m_MarkedTargetIcon);
		}

		if(!m_TagIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_TagIcon);

		if(!m_WoundedTeamIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_WoundedTeamIcon);

		if(!m_PingTeamIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_PingTeamIcon);

		if(m_commRoseTimer>0 && !m_CommRoseIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_CommRoseIcon);

		if(!m_MedicTeamIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_MedicTeamIcon);
		if(!m_SpawnProtectIcon.IsUndefined())
			hudMain->deleteScreenIcon(m_SpawnProtectIcon);
		
	}

	if(!NetworkLocal)
	{
		if(hudMinimap && hudMinimap->IsInited() && m_onMinimap)
		{
			hudMinimap->EraseUnit(m_MinimapTagIconName);
			m_onMinimap = false;
		}
	}

	m_RiotShieldMeshPlacement = NULL;
	m_RiotPersonalShieldMeshPlacement = NULL;
	m_AutoTurretMeshPlacement = NULL;
	m_RespawnBeaconMeshPlacement = NULL;

	if(NetworkLocal)
	{
		if(gClientLogic().localPlayer_ == this)
			gClientLogic().localPlayer_ = 0;

		r3dRenderer->DeleteTexture(s_damageHitColorCorrectionTex);
		s_damageHitColorCorrectionTex = NULL;

		r3dRenderer->DeleteTexture(s_UAVFriendlyBlinkTex);
		s_UAVFriendlyBlinkTex = NULL;

		r3dRenderer->DeleteTexture(s_laserPointerTex);
		s_laserPointerTex = NULL;

		r3dRenderer->DeleteTexture(s_flashlightFlashTex);
		s_flashlightFlashTex = NULL;

		r3dRenderer->DeleteTexture(s_laserFlashTex);
		s_laserFlashTex = NULL;

		r3dRenderer->DeleteTexture(s_lightHaloTex);
		s_lightHaloTex = NULL;
	}

	SAFE_DELETE(uberAnim_);
	SAFE_DELETE(uberEquip_);

	ReleaseCachedPhysSkeleton( m_PhysSkeleton ) ;
	m_PhysSkeleton = 0 ;
	
	for(uint32_t i=0; i<footStepsArr.Count(); ++i) 
	{
		SoundSys.Stop(footStepsArr[i].snd);
		SoundSys.Release(footStepsArr[i].snd);
	}
	footStepsArr.Clear();

	return parent::OnDestroy();
}

float getMinimumAngleDistance(float from, float to)
{
	//make sure the angles are in range [0..360)
	const float da = 1.0f/360.0f;
	from = from - floorf(from * da)*360.0f;
	to = to - floorf(to * da)*360.0f;

	float d = to - from;
	if(d <-180.0f)	d += 360.0f;
	if(d > 180.0f)	d -= 360.0f;
	return d;
}

void Fix_Angle(float& deg)
{
	if(deg < -R3D_PI)
		deg += R3D_PI * 2;
	else if(deg >= R3D_PI)
		deg -= R3D_PI * 2;
}

void obj_AI_Player::UpdateRotationToTarget(float fY, float fTurnSpeed)
{
	// if don't need to rotate
	if(R3D_ABS(fY) < 0.2f|| R3D_ABS(fY) > 120)
	{
		m_fPlayerRotation = m_fPlayerRotationTarget;
		uberAnim_->StopTurnInPlaceAnim();
		return;
	}

	float fTimePassed = r3dGetFrameTime();

	// turn character to the needed angle
	float step = fTurnSpeed * fTimePassed;

	m_fPlayerRotation += fY < 0.0f ? -step : +step;
		
	// check if we finished rotation, overrotated
	float fY2 = getMinimumAngleDistance(m_fPlayerRotation, m_fPlayerRotationTarget);
	if((fY > 0 && fY2 <= 0) || (fY < 0 && fY2 >= 0) 
		
		|| 
		// don't overreact - can get to any angle with this sort of step anyway (happens on low fps)
		step >= 180.f )
	{
		m_fPlayerRotation = m_fPlayerRotationTarget;
		uberAnim_->StopTurnInPlaceAnim();
	}
}

R3D_FORCEINLINE void UpdateBodyAdjustX( float* ioAdjust, float newadjust, float dt )
{
	float bodyAdjust_x = *ioAdjust ;
	bool was_less = bodyAdjust_x < newadjust ;

	bodyAdjust_x += (newadjust - bodyAdjust_x) * dt * 4.0f;

	if( was_less )
	{
		if( bodyAdjust_x > newadjust )
			bodyAdjust_x = newadjust ;
	}
	else
	{
		if( bodyAdjust_x < newadjust )
			bodyAdjust_x = newadjust ;
	}

	*ioAdjust = bodyAdjust_x ;
}

void obj_AI_Player::UpdateUpperBodyAngLegs()
{
	float fTimePassed = r3dGetFrameTime();
	const float fMaxBodyAngle = R3D_DEG2RAD(45.0f);

	// turn firstly upper body only and only then legs
	float fY = getMinimumAngleDistance(m_fPlayerRotation, m_fPlayerRotationTarget);
	float newadjust = R3D_DEG2RAD(fY);

	UpdateBodyAdjustX( &bodyAdjust_x, newadjust, fTimePassed ) ;

	if(NetworkLocal && g_camera_mode->GetInt()==2)
	{
		UpdateRotationToTarget(fY, _ai_fTurnSpeedIdle);
	}
	else
	{
		if(uberAnim_->turnInPlaceTrackID == CUberAnim::INVALID_TRACK_ID)
		{
			if(R3D_ABS(bodyAdjust_x) > fMaxBodyAngle)
			{
				uberAnim_->StartTurnInPlaceAnim();
				bodyAdjust_x = R3D_CLAMP(bodyAdjust_x, -fMaxBodyAngle, fMaxBodyAngle);
			}
		}
		else
		{
			UpdateRotationToTarget(fY, _ai_fTurnSpeedIdle);
			// set new body adjust after turning body
			float fY = getMinimumAngleDistance(m_fPlayerRotation, m_fPlayerRotationTarget);
			float newadjust = R3D_DEG2RAD(fY);
			bodyAdjust_x = newadjust;
		}
	}
}

void obj_AI_Player::UpdateRotation()
{
	R3DPROFILE_FUNCTION("UpdateRotation");

	uberAnim_->UpdateTurnInPlaceAnim();

	float fPrevRotation = m_fPlayerRotation;
	float fTimePassed = r3dGetFrameTime();
	if(PlayerState == PLAYER_IDLE || (PlayerState == PLAYER_MOVE_CROUCH && PlayerMoveDir == CUberData::ANIMDIR_Stand))
	{
		UpdateUpperBodyAngLegs();
	}
	else
	{
		uberAnim_->StopTurnInPlaceAnim();
		
		UpdateBodyAdjustX(&bodyAdjust_x, 0.f, fTimePassed * 4.0f);

		float fY = getMinimumAngleDistance(m_fPlayerRotation, m_fPlayerRotationTarget);
		UpdateRotationToTarget(fY, _ai_fTurnSpeedWalk);
	}
	
	// lerp body bending to target
	if(bodyAdjust_y[0] < bodyAdjust_y[1]) {
		bodyAdjust_y[0] = R3D_MIN(bodyAdjust_y[0] + fTimePassed * 1.8f, bodyAdjust_y[1]);
	} else if(bodyAdjust_y[0] > bodyAdjust_y[1]) {
		bodyAdjust_y[0] = R3D_MAX(bodyAdjust_y[0] - fTimePassed * 1.8f, bodyAdjust_y[1]);
	} else
		bodyAdjust_y[0] = bodyAdjust_y[1];

	// create rotation matrix
	D3DXMatrixRotationYawPitchRoll(&MoveMatrix, R3D_DEG2RAD(m_fPlayerRotation) + bodyAdjust_x, 0, 0);
	D3DXMatrixRotationYawPitchRoll(&DrawRotMatrix, R3D_DEG2RAD(m_fPlayerRotation), 0, 0);

	r3dPoint3D pos = GetPosition();

	extern int CurHUDID;
	if(g_camera_mode->GetInt()==2 && NetworkLocal && ((g_bEditMode && (CurHUDID==1 || CurHUDID==6) )||!g_bEditMode) && !uavViewActive_)
	{
		float hands_height = 0.0f;
		{
			D3DXMATRIX tmp, res;
			D3DXMatrixIdentity(&tmp);
			uberAnim_->anim.GetCurrentSkeleton()->GetBoneWorldTM(boneId_Bip01_Head, &res, tmp);
			hands_height = res._42;
		}
		// weapon movement when turning
		D3DXMATRIX m3, m4;
		float limit = 2.0f;
		if(m_isAiming)
			limit = 0.5f;
		// when firing, don't delay movement
		if(m_Weapons[m_SelectedWeapon] && (r3dGetTime() < m_Weapons[m_SelectedWeapon]->getLastTimeFired() + 0.5f))
			limit = 0.0f;
		float fRot = -R3D_CLAMP(getMinimumAngleDistance(fPrevRotation, m_fPlayerRotation)*10.0f, -limit, limit);
		static float EMAPrev = 0.0f;
		
		float EMACur = EMAPrev + 0.20f * (fRot - EMAPrev); // exponential moving average of about 15 prev.values
		EMAPrev = EMACur;

		D3DXMatrixRotationYawPitchRoll(&m3, R3D_DEG2RAD(EMACur), 0, 0);

		D3DXMatrixTranslation(&m4, 0, -hands_height, 0.1f);
		D3DXMatrixMultiply(&DrawFullMatrix, &m3, &m4);
		D3DXMATRIX m1, m2;
		D3DXMatrixMultiply(&m1, &DrawFullMatrix, &r3dRenderer->InvViewMatrix);
		D3DXMatrixTranslation(&m2, 0, 0.0f, 0.0f);
		D3DXMatrixMultiply(&DrawFullMatrix, &m1, &m2);
	}
	else
	{
		DrawFullMatrix = DrawRotMatrix;

		DrawFullMatrix._41 += pos.x;
		DrawFullMatrix._42 += pos.y;
		DrawFullMatrix._43 += pos.z;
	}

	// set physx rotation vectors
	SetRotationVector(r3dVector(m_fPlayerRotation, 0, 0));
}

static bool updateShootingTarget(obj_AI_Player* pl, const r3dPoint3D& shootFrom, const r3dPoint3D& muzzlePos, r3dPoint3D& sPos, r3dPoint3D& sNorm, r3dMaterial** sMat, GameObject** gameObj, const char** actorName, float spread, bool recheckCollisionAgainsMuzzler)
{
	r3d_assert(pl);
	r3d_assert(sMat);
	r3d_assert(gameObj);

	const float MAX_CASTING_DISTANCE = 20000.0f;
	r3dPoint3D dir;
	if(pl->m_isInScope || g_camera_mode->GetInt() != 1)
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
	else
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);

	*sMat = NULL;
	*gameObj = NULL;

	// convert spread from diameter at 50 meter range to radius at 1meter range
	spread = (spread*0.5f)/50.0f; // 0.5 - because spread initially set as diameter, and we need radius. 50-because spread is set at 50 meter range. formula is: tanA=opp/adj = so that is angle that we need to feed into raycast

	// limit spread to prevent ppl shooting behind, etc.
	spread = R3D_CLAMP(spread, -0.5f, 0.5f); 
	D3DXMATRIX rotMat;
	D3DXMatrixRotationYawPitchRoll(&rotMat, u_GetRandom(-spread, spread), u_GetRandom(-spread, spread), u_GetRandom(-spread, spread));
	r3dPoint3D dirWithSpread;
	D3DXVec3TransformNormal((D3DXVECTOR3*)&dirWithSpread, (D3DXVECTOR3*)&dir, &rotMat);
	{
		// shoot from camera :)
		PxRaycastHit hit;
		PhysicsCallbackObject* target = NULL;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
		g_pPhysicsWorld->raycastSingle(PxVec3(shootFrom.x, shootFrom.y, shootFrom.z), PxVec3(dirWithSpread.x, dirWithSpread.y, dirWithSpread.z), MAX_CASTING_DISTANCE, 
													PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL, hit, filter);
		// check if target position is still visible from muzzler (prevent against shooting from wall edges
		if( recheckCollisionAgainsMuzzler && hit.shape )
		{
			PxVec3 dir2(hit.impact.x - muzzlePos.x, hit.impact.y - muzzlePos.y, hit.impact.z - muzzlePos.z);
			dir2.normalize();

			PxRaycastHit hit2;
			g_pPhysicsWorld->raycastSingle(PxVec3(muzzlePos.x, muzzlePos.y, muzzlePos.z), dir2, MAX_CASTING_DISTANCE, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL, hit2, filter);
			if(hit2.shape != hit.shape)
			{
				// replace old collision data with new one
				hit   = hit2;
			}
		}
		
		if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
		{
			if(actorName)
				*actorName = hit.shape->getActor().getName();

			*gameObj = target->isGameObject();
			if(*gameObj)
			{
				if( (*gameObj)->isObjType( OBJTYPE_Mesh ) )
					*sMat = static_cast< MeshGameObject* > ( target )->MeshLOD[ 0 ]->GetFaceMaterial( hit.faceIndex );
			}
			else if(target->hasMesh())
			{
				*sMat = target->hasMesh()->GetFaceMaterial( hit.faceIndex );
			}
			if (!*sMat)
				*sMat = target->GetMaterial(hit.faceIndex);

			sPos.x = hit.impact.x;
			sPos.y = hit.impact.y;
			sPos.z = hit.impact.z;

			sNorm.x = hit.normal.x;
			sNorm.y = hit.normal.y;
			sNorm.z = hit.normal.z;
		
			return true;
		}
	}

	sPos = muzzlePos + dir * 500.0f;
	sNorm = -dir;

	// do not allow to shoot behind you, like when there a pole between your character and camera
	{
		r3dPoint3D dir2 = (sPos - muzzlePos).Normalize();
		if(dir.Dot(dir2) < 0.0f)
		{
			*sMat = NULL;
			*gameObj = NULL;
		}
	}

	return false;
}

static bool updateMeleeTarget(obj_AI_Player* pl, const r3dPoint3D& shootFrom, const r3dPoint3D& muzzlePos, r3dPoint3D& sPos, r3dPoint3D& sNorm, r3dMaterial** sMat, GameObject** gameObj, const char** actorName, float spread, bool recheckCollisionAgainsMuzzler)
{
	r3d_assert(pl);
	r3d_assert(sMat);
	r3d_assert(gameObj);

	*sMat = NULL;
	*gameObj = NULL;

	// for melee we need to check if we hit anyone in front of us. PhysX doesn't allow to do this check easily (overlaps do not return hit information, just shapes), so instead 
	// I'm going to do 9 ray casts in front of player. They are super short, so it shouldn't cause any performance issues 
	// and we going to hit first player, if any
	// TODO: we need to handle a case when we hit static object only and do not hit any player, but for melee this is minor
	const float MAX_CASTING_DISTANCE = 1.5f;
	float rayDegrees[9] = {-30.0f, -20.0f, -10.0f, -5.0f, 0.0f, 5.0f, 10.0f, 20.0f, 30.0f};
	for(int iRay = 0; iRay <9; ++iRay)
	{
		D3DXMATRIX rotMat;
		D3DXMatrixRotationYawPitchRoll(&rotMat, R3D_DEG2RAD(rayDegrees[iRay]), 0, 0);
		r3dPoint3D dirWithSpread;
		D3DXVec3TransformNormal((D3DXVECTOR3*)&dirWithSpread, (D3DXVECTOR3*)&gCam.vPointTo/*(D3DXVECTOR3*)&pl->GetvForw()*/, &rotMat);

		{
			PxRaycastHit hit;
			PhysicsCallbackObject* target = NULL;
			PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
			g_pPhysicsWorld->raycastSingle(PxVec3(shootFrom.x, shootFrom.y, shootFrom.z), PxVec3(dirWithSpread.x, dirWithSpread.y, dirWithSpread.z), MAX_CASTING_DISTANCE, 
				PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL, hit, filter);

			if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
			{
				*gameObj = target->isGameObject();
				if(!*gameObj)
					continue;
				if(!(*gameObj)->isObjType(OBJTYPE_Human))
					continue;

				*sMat = NULL;

				if(actorName)
					*actorName = hit.shape->getActor().getName();

				sPos.x = hit.impact.x;
				sPos.y = hit.impact.y;
				sPos.z = hit.impact.z;

				sNorm.x = hit.normal.x;
				sNorm.y = hit.normal.y;
				sNorm.z = hit.normal.z;

				return true;
			}
		}
	}

	r3dPoint3D dir;
	if(pl->m_isInScope || g_camera_mode->GetInt() != 1)
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
	else
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);

	sPos = muzzlePos + dir * 500.0f;
	sNorm = -dir;

	// do not allow to shoot behind you, like when there a pole between your character and camera
	{
		r3dPoint3D dir2 = (sPos - muzzlePos).Normalize();
		if(dir.Dot(dir2) < 0.0f)
		{
			*sMat = NULL;
			*gameObj = NULL;
		}
	}

	return false;
}

static bool isMuzzlerBlocked(obj_AI_Player* pl, const r3dPoint3D& muzzlePos)
{
	// check if muzzler is visible, and if not (inside of wall) then we do not shoot anything
	bool muzzlerBlocked = false;
	{
		r3dVector temp = muzzlePos-gCam;
		float len = temp.Length();
		temp.Normalize();
		if(len > 0)
		{
			PxRaycastHit hit;
			PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::eSTATIC));
			muzzlerBlocked = g_pPhysicsWorld->raycastSingle( PxVec3(gCam.x,gCam.y,gCam.z), PxVec3(temp.x, temp.y, temp.z), len, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter);
		}
	}
	if(muzzlerBlocked)
		return true;

	const float MAX_CASTING_DISTANCE = 20000.f;
	r3dPoint3D dir;
	if(pl->m_isInScope || g_camera_mode->GetInt() != 1)
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
	else
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);

	// check if camera->shoot_target position agains camera->muzzler (disable shooting when close to wall, but looking at step edge)
	float distToMuzzler = (gCam-muzzlePos).Length();
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::eSTATIC));
		if(g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlags(PxSceneQueryFlag::eDISTANCE), hit, filter))
		{
            if(hit.distance < distToMuzzler)
            {
                // shooting blocked before muzzler
                return true;
            }
		}
	}

	return false;
}

void obj_AI_Player::UpdateTargetting()
{
	R3DPROFILE_FUNCTION("UpdateTargetting");
	r3d_assert(NetworkLocal);

	m_LocalPlayer_CurrentAimAt = invalidGameObjectID;
	
	if(uavViewActive_ || laserViewActive_)
		return;

	r3dMaterial* temp;
	GameObject* tempObj;
	r3dPoint3D muzzlerPos = GetMuzzlerPosition();
	
	{
		r3dPoint3D tmpPos, tmpNorm;
		// we always accept this position now, sometimes this jiggles, if that happens we'll have to look at the angle of the aiming and maybe factor out parts of it. 
		updateShootingTarget(this, gCam, muzzlerPos, tmpPos, tmpNorm, &temp, &tempObj, NULL, 0.0f, false);

		{
			viewTargetPos = tmpPos;
			viewTargetNorm = tmpNorm;
		}
	}

	const char* reticleColor = "white";
	if(tempObj)
	{
		m_LocalPlayer_CurrentAimAt = tempObj->GetSafeID();
		if(tempObj->isObjType(OBJTYPE_Human))
		{
			obj_AI_Player* target = (obj_AI_Player*)tempObj;
			if(!target->bDead)
			{
				if(target->TeamID != TeamID)
				{
					reticleColor = "red";
				}
				else
					reticleColor = "blue";
			}
		}
	}
	if(hudMain)
		hudMain->setReticleColor(reticleColor);

	r3dPoint3D bendTarget = GetPosition();
	bendTarget.Y += Height * 0.9f;
	bendTarget   += GetvForw() * 0.2f;

	r3dPoint3D v = (viewTargetPos - bendTarget);
	float dist = v.Length();
	float bendUp = asinf(v.y / v.Length());
	if(dist<1.8f)
		bendUp = R3D_LERP(0.0f, bendUp, dist/1.8f);

	bodyAdjust_y[1] = bendUp;

#ifndef FINAL_BUILD
	if( g_char_spect->GetInt() )
	{
		bodyAdjust_y[1] = 0 ;
	}
#endif
}

r3dPoint3D obj_AI_Player::GetMuzzlerPosition()
{
	if(GetSkeleton() && m_Weapons[m_SelectedWeapon])
	{
		return m_Weapons[m_SelectedWeapon]->getMuzzlePos(uberEquip_->getWeaponBone(GetSkeleton(), DrawFullMatrix));
	}
	
	return GetPosition() + GetvForw() * 1.0f + r3dPoint3D(0, 0.5f, 0);
}

// 0 - body
// 1 - head
// 2 - hands
// 3 - legs
BYTE obj_AI_Player::getBodyNameFromBoneName(const char* boneName)
{
	struct boneToBodyPart_s {
		BYTE part;
		const char* bone;
	};
	const static boneToBodyPart_s map[] = {
	  {1, "Bip01_Head"},
	  {2, "Bip01_L_UpperArm"},
	  {2, "Bip01_L_Forearm"},
	  {2, "Bip01_L_Hand"},
	  {2, "Bip01_R_UpperArm"},
	  {2, "Bip01_R_Forearm"},
	  {2, "Bip01_R_Hand"},
	  {3, "Bip01_L_Thigh"},
	  {3, "Bip01_L_Calf"},
	  {3, "Bip01_L_Foot"},
	  {3, "Bip01_L_Toe0"},
	  {3, "Bip01_R_Thigh"},
	  {3, "Bip01_R_Calf"},
	  {3, "Bip01_R_Foot"},
	  {3, "Bip01_R_Toe0"},
	};
	
	// not optimal, but numbers CAN change, because of skeleton modification
	for(int i=0; i<R3D_ARRAYSIZE(map); i++)
	{
		if(strcmp(map[i].bone, boneName) == 0) {
#ifndef FINAL_BUILD
			r3dOutToLog("%s mapped to %d\n", boneName, map[i].part);
#endif
			return map[i].part;
		}
	}
	
	return 0;
}

void obj_AI_Player::CheckFireWeapon()
{
	r3d_assert(NetworkLocal);

	VMPROTECT_BeginMutation("obj_AI_Player::CheckFireWeapon");

	static bool wasReloading = false; // static is okay, this code is running for local player only
#if ENABLE_ZOMBIES	
	lifeProperties.weaponFired = false;
#endif

	Weapon* wpn = m_Weapons[m_SelectedWeapon];
	if(wpn == NULL)
		return;

	bool needExit = false;
#ifndef FINAL_BUILD
	if(d_video_spectator_mode->GetBool()) needExit = true;
	if(d_observer_mode->GetBool()) needExit = true;
#endif
	if(hudMain && hudMain->isChatVisible()) needExit = true;
	if(m_siegeArmingTimer > 0) needExit = true;
	if(hudPause && hudPause->isActive()) needExit = true;
	if(Mouse->GetMouseVisibility()) needExit = true;
	if(hudCommCalls && hudCommCalls->isCommRoseVisible()) needExit = true;
	if((PlayerState == PLAYER_MOVE_SPRINT && wpn->getCategory()!=storecat_MELEE) || bDead)  
	{
		if(wasReloading)
		{
			if(hudMain)
				hudMain->showReloading(false);
			wasReloading = false;
		}
		needExit = true;
	}
	
	if(uavViewActive_ || laserViewActive_) needExit = true;

	if(wpn->isUsableItem() && !needExit)
	{
		if(wpn->getItemID() == WeaponConfig::ITEMID_LLDR || wpn->getItemID() == WeaponConfig::ITEMID_Cypher2)
		{
		}
		else
		{
			// consumables
			if(m_needToDrawRiotShieldPlacement && InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_AIM))
				m_needToDrawRiotShieldPlacement = 0;
			if(m_needToDrawAutoTurretPlacement && InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_AIM))
				m_needToDrawAutoTurretPlacement = 0;
			if(m_needToDrawRespawnBeaconPlacement && InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_AIM))
				m_needToDrawRespawnBeaconPlacement = 0;

			if(InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_PRIMARY_FIRE) || m_ItemSimulateLeftClick)
			{
				m_ItemSimulateLeftClick = false;
				if(gClientLogic().m_gameHasStarted) // allow to use consumables only when game has started
				{
					int numCharges = 1;
					if(wpn->m_usableItemInventoryIdx >= 0)
						numCharges = gUserProfile.ProfileData.Inventory[wpn->m_usableItemInventoryIdx].quantity;
					if(numCharges > 0 && wpn->isReadyToFire(true, false)) // consumables have rate of fire (cooldown)
					{
						if(wpn->getItemID() == WeaponConfig::ITEMID_RiotShield || wpn->getItemID() == WeaponConfig::ITEMID_RiotShieldConsumable)
						{
							if(wpn->getItemID() == WeaponConfig::ITEMID_RiotShield && m_spawnedRiotShield)
							{
								return; // skill based riot shield - only one per player spawn
							}
							if(m_needToDrawRiotShieldPlacement == 0)
							{
								m_needToDrawRiotShieldPlacement = wpn->getItemID();
								return;
							}
							else
							{
								if(m_canPlaceConsumableItem)
									m_needToDrawRiotShieldPlacement = 0; // and let user place riot shield by sending packet to server
								else
									return;
							}
						}
						if(wpn->getItemID() == WeaponConfig::ITEMID_AutoTurret || wpn->getItemID() == WeaponConfig::ITEMID_AutoTurretCons)
						{
							if(wpn->getItemID() == WeaponConfig::ITEMID_AutoTurret && m_spawnedAutoTurret)
							{
								return; // skill based - only one per player spawn
							}
							if(m_needToDrawAutoTurretPlacement == 0)
							{
								m_needToDrawAutoTurretPlacement = wpn->getItemID();
								return;
							}
							else
							{
								if(m_canPlaceConsumableItem)
									m_needToDrawAutoTurretPlacement = 0; // and let user place it by sending packet to server
								else
									return;
							}
						}
						if(wpn->getItemID() == WeaponConfig::ITEMID_RespawnBeacon || wpn->getItemID() == WeaponConfig::ITEMID_RespawnBeaconCons)
						{
							if(m_needToDrawRespawnBeaconPlacement == 0)
							{
								m_needToDrawRespawnBeaconPlacement = wpn->getItemID();
								return;
							}
							else
							{
								if(m_canPlaceConsumableItem)
									m_needToDrawRespawnBeaconPlacement = 0; // and let user place it by sending packet to server
								else
									return;
							}
						}
						if((wpn->getItemID() == WeaponConfig::ITEMID_Bandages || wpn->getItemID() == WeaponConfig::ITEMID_Bandages2) && m_Health >= (getMaxHealth()-1.0f))
							return;

						uberAnim_->StartShootAnim();
						wpn->Fire(r3dPoint3D(0,0,0), D3DXMATRIX()); // to update last fire time

						// todo: we need to sync this crap over network, so that network players will see us using item
						// todo: also, we need to sync when we sent this packet, so that it is in sync with animation
						PKT_C2C_PlayerUseItem_s n;
						n.itemId  = wpn->getItemID();
						n.pos     = GetPosition() + GetvForw()*1.0f + r3dPoint3D(0,1,0);
						if(n.itemId == WeaponConfig::ITEMID_RiotShield || n.itemId == WeaponConfig::ITEMID_RiotShieldConsumable)
						{
							n.var1 = (m_fPlayerRotation) + R3D_RAD2DEG(bodyAdjust_x) + 180;
							if(n.itemId == WeaponConfig::ITEMID_RiotShield)
								m_spawnedRiotShield = true;
						}
						if(n.itemId == WeaponConfig::ITEMID_AutoTurret || n.itemId == WeaponConfig::ITEMID_AutoTurretCons)
						{
							n.var1 = (m_fPlayerRotation) + R3D_RAD2DEG(bodyAdjust_x) + 180;
							if(n.itemId == WeaponConfig::ITEMID_AutoTurret)
								m_spawnedAutoTurret = true;
						}
						if(n.itemId == WeaponConfig::ITEMID_RespawnBeacon || n.itemId == WeaponConfig::ITEMID_RespawnBeaconCons)
						{
							n.var1 = (m_fPlayerRotation) + R3D_RAD2DEG(bodyAdjust_x) + 180;
						}
						bool needCorrectPos = false;
						if(n.itemId == WeaponConfig::ITEMID_MedKit || 
							n.itemId == WeaponConfig::ITEMID_MotionSensor || 
							n.itemId == WeaponConfig::ITEMID_MotionSensorConsumable || 
							n.itemId == WeaponConfig::ITEMID_RiotShield || 
							n.itemId == WeaponConfig::ITEMID_RiotShieldConsumable || 
							n.itemId == WeaponConfig::ITEMID_AutoTurret || 
							n.itemId == WeaponConfig::ITEMID_AutoTurretCons || 
							n.itemId == WeaponConfig::ITEMID_RespawnBeacon || 
							n.itemId == WeaponConfig::ITEMID_RespawnBeaconCons  
							)
							needCorrectPos = true; // those objects require precise position, otherwise there will be desync between client\server (mainly spawn beacon problem)

						if(needCorrectPos)
						{
							PxRaycastHit hit;
							PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC));
							if(g_pPhysicsWorld->raycastSingle(PxVec3(n.pos.x, n.pos.y, n.pos.z), PxVec3(0,-1,0), 3.0f, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter))
								n.pos = r3dPoint3D(hit.impact.x, hit.impact.y+0.1f, hit.impact.z);
						}

						p2pSendToHost(this, &n, sizeof(n), true);
					}
				}
				else
				{
					if(hudMain)
						hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_CannotUseConsumableBeforeGameStart"));
				}
			}
		}
		needExit = true;
	}
	
	if(needExit)
	{
		wpn->isReadyToFire(false, false); // very important to call ready to fire with false to reset internal counter of how long trigger was pressed
		return;
	}

	float gamepadLeftTrigger, gamepadRightTrigger;
	Gamepad->GetTrigger(gamepadLeftTrigger, gamepadRightTrigger);

	m_isPressedFireTrigger = InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE) || gamepadRightTrigger > 0;
	// check muzzle block except when using scope
	if(!m_isInScope && wpn->getCategory()!=storecat_MELEE && isMuzzlerBlocked(this, GetMuzzlerPosition()))
		m_isPressedFireTrigger = false;

	bool oldAiming = m_isAiming;
	if(g_toggle_aim->GetBool())
	{
		if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_AIM))
			m_isAiming = !m_isAiming;
	}
	else
	{
		m_isAiming = InputMappingMngr->isPressed(r3dInputMappingMngr::KS_AIM) || gamepadLeftTrigger > 0;
	}
	
	// check if we just switched weapon and do not allow to fire
	if((r3dGetTime() - lastTimeWeaponSwitch) < 0.4f)
	{
		m_isPressedFireTrigger = false;
		m_isAiming = false;
	}

	// intentional, skip weapon switch check
	if(m_QuickThrowGrenade == 2 && !uberAnim_->GetGrenadeAnimState())
	{
		m_QuickThrowGrenade--;
		m_isPressedFireTrigger = true;
	} 
	else if( m_QuickThrowGrenade == 1 )
	{
		m_isPressedFireTrigger = false;
	}

	extern 	PlayerStateVars_s CurrentRig;
	m_isInScope = hasScopeMode() && CurrentRig.allowScope;

	if(wpn->isReloading())
		m_isAiming = false;
	if(wpn->getAnimType()== WPN_ANIM_MELEE) // no aiming in melee
		m_isAiming = false;

	// remember that aiming was changed
	if(m_isAiming != oldAiming)
		m_isChangedAiming1 = true;

	extern float LerpValue;
	if (wpn->getCategory() != storecat_SNP)
	{
		m_isFinishedAiming = m_isAiming && LerpValue==1.0f;
	}
	else 
	{
		m_isFinishedAiming = m_isAiming && LerpValue==1.0f && m_isInScope;
	}

	bool playerCrouching = PlayerState==PLAYER_MOVE_CROUCH || PlayerState==PLAYER_MOVE_CROUCH_AIM; 
	bool playerMoving  = InputAcceleration.Length() > 0.01f;
	bool playerRunning = PlayerState==PLAYER_MOVE_RUN;
	int triggerPressedCounter = wpn->getTriggerPressedCounter();

	float recoilCoolDownMod = 1.0f;
	if(triggerPressedCounter < 3)
		recoilCoolDownMod = 2.50f;
	else
		recoilCoolDownMod = 3.2f;

	// BEGIN SPREAD\RECOIL LOGIC

	if(!m_isPressedFireTrigger) // if user let go of trigger, start return recoil back to normal right away, so that it doesn't linger at the top for a second or two after stopping firing
	{
		if(wpn->getCategory() != storecat_SNP)
		{
			RecoilCooldown = 0;
		}
	}

	//[ST] trying to get higher recoil without changing database. hack :)
	float recoil = wpn->getRecoil()*1.1f;
		
	float spread = CurrentSpread; // spread will be affected now by player state and other modificators

	//[ST] - just in case - not sure why we needed it
	//spread *= GPP->c_fGlobalSpreadMod;

	// Okay by default in NON ADS mode we're increasing spread
	spread *= 1.1f;

	if (wpn->getCategory()==storecat_SHTG) spread *= 1.2f;


	if(playerCrouching)
	{
		//recoil *= GPP->c_fSpreadRecoilMod_Crouch;
		if(wpn->getCategory()==storecat_SHTG)
			spread *= GPP->c_fSpreadModShotgun_Crouch;
		else
			spread *= GPP->c_fSpreadRecoilMod_Crouch;
	}
	if(m_isFinishedAiming)
	{
		recoil *= GPP->c_fSpreadRecoilMod_Aim*2.0f;
		if(wpn->getCategory()==storecat_SHTG)
			spread *= GPP->c_fSpreadModShotgun_Aim;
		else
			spread *= GPP->c_fSpreadRecoilMod_Aim;
	}
	if(playerRunning)
	{
		recoil *= GPP->c_fSpreadRecoilMod_Running;
		spread *= GPP->c_fSpreadRecoilMod_Running;
	}
	//if(playerMoving)
	//{
	//	recoil *= GPP->c_fSpreadRecoilMod_Moving;
	//	spread *= GPP->c_fSpreadRecoilMod_Moving;
	//}

	/*if(wpn->getCategory() == storecat_SNP && !m_isAiming)
	{
		m_ReticleTargetScale = 25.0f;
	}*/

	// first bullet logic
	if(!playerRunning && m_isFinishedAiming && wpn->getCategory() != storecat_SHTG && bOnGround)
	{
		// spr_mod - to prevent player from shooting straight at target by just clicking on a mouse, rather than holding fire
		float spr_mod = 1.0f - R3D_CLAMP(((r3dGetTime() - wpn->getLastTimeFired())/GPP->c_fZeroSpreadDelayBetweenShots), 0.0f, 0.9f); // 0.9 - first bullet 10% spread
		// if shooting for 10 bullets straight you spread will slowly increase
		//[ST] - Removed this code
		//spread = R3D_LERP(spread*spr_mod, spread, R3D_CLAMP(float(triggerPressedCounter)/10.0f, 0.0f, 1.0f)); 

		// special case for sniper rifle
		if(wpn->getCategory() == storecat_SNP && m_isInScope && triggerPressedCounter==0)
			spread = 0;
	}

	// if in air
	if(!bOnGround)
	{
		if(wpn->getCategory() == storecat_HG)
			spread *= 2.0f;
		else
			spread *= 4.0f; // hard coded, if jumping = spread increased by X times
	}

	// special case for sniper rifle
	if(wpn->getCategory() == storecat_SNP && !m_isInScope)
		spread = 11; // fixed spread at 11 for sniper rifles if not aiming - per sergey's request

	if(m_BerserkerTime > 0)
		spread *= 2.5f; // 150% more spread when in berserker mode

	if(wpn->getCategory() == storecat_ASR)
	{
		if(CurLoadout.getSkillLevel(CUserSkills::ASSAULT_ASRWeapons) == 5)
		{
			spread *= 0.95f;
			recoil *= 0.95f;
		}
		else
		{
			if(CurLoadout.getSkillLevel(CUserSkills::ASSAULT_ASRWeapons)>=2)
			{
				recoil *= 0.98f;
			}
			if(CurLoadout.getSkillLevel(CUserSkills::ASSAULT_ASRWeapons)>=3)
			{
				spread *= 0.98f;
			}
		}
	}
	if(wpn->getCategory() == storecat_SNP)
	{
		if(CurLoadout.getSkillLevel(CUserSkills::RECON_SniperWeapons) == 5)
		{
			spread *= 0.95f;
			recoil *= 0.95f;
		}
		else
		{
			if(CurLoadout.getSkillLevel(CUserSkills::RECON_SniperWeapons)>=2)
			{
				recoil *= 0.98f;
			}
			if(CurLoadout.getSkillLevel(CUserSkills::RECON_SniperWeapons)>=3)
			{
				spread *= 0.98f;
			}
		}
	}
	if(wpn->getCategory() == storecat_MG)
	{
		if(CurLoadout.getSkillLevel(CUserSkills::SPEC_LMGWeapons) == 5)
		{
			spread *= 0.95f;
			recoil *= 0.95f;
		}
		else
		{
			if(CurLoadout.getSkillLevel(CUserSkills::SPEC_LMGWeapons)>=2)
			{
				recoil *= 0.98f;
			}
			if(CurLoadout.getSkillLevel(CUserSkills::SPEC_LMGWeapons)>=3)
			{
				spread *= 0.98f;
			}
		}
	}
	if(wpn->getCategory() == storecat_SMG)
	{
		if(CurLoadout.getSkillLevel(CUserSkills::MEDIC_SMGWeapons) == 5)
		{
			spread *= 0.95f;
			recoil *= 0.95f;
		}
		else
		{
			if(CurLoadout.getSkillLevel(CUserSkills::MEDIC_SMGWeapons)>=2)
			{
				recoil *= 0.98f;
			}
			if(CurLoadout.getSkillLevel(CUserSkills::MEDIC_SMGWeapons)>=3)
			{
				spread *= 0.98f;
			}
		}
	}


	// FINISHED SPREAD\RECOIL LOGIC

	{
		bool isReloading = wpn->isReloading();
		if(isReloading && !wasReloading)
		{
			if(hudMain)
				hudMain->showReloading(true);
			wasReloading = true;
		}
		if(!isReloading && wasReloading)
		{
			if(hudMain)
				hudMain->showReloading(false);
			wasReloading = false;
		}
		if(isReloading)
		{
			float progress = R3D_CLAMP((wpn->getReloadProgress()/wpn->getReloadTime()), 0.0f, 1.0f);
			if(hudMain)
				hudMain->SetReloadingProgress(progress*100.0f);
		}
	}

	if((InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_RELOAD)||Gamepad->WasReleased(gpX)) && !wpn->isReloading())
		wpn->Reload();

	if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_CHANGE_RATE_FIRE) && !wpn->isReloading())
	{
		wpn->switchFiremode();
		if(hudMain)
			hudMain->setFireMode(wpn->getFiremode());
	}

	// check for grenade throw, if successful it will go to normal fire code
	if(wpn->isGrenadeOrMineAnim())
	{
		if(!uberAnim_->IsGrenadePinPullActive())
		{
			if(wpn->isReadyToFire(m_isPressedFireTrigger, m_isFinishedAiming)) 
			{
				uberAnim_->StartGrenadePinPullAnimation();

				PKT_C2C_PlayerReadyGrenade_s n;
				n.wid = m_SelectedWeapon;
				p2pSendToHost(this, &n, sizeof(n));
				return;
			}
		}
	}

	bool isReadyToFire = wpn->isReadyToFire(m_isPressedFireTrigger, m_isFinishedAiming);
	float holdingDelay = 0;

	// for grenades, overide regular logic a little bit
	if(wpn->isGrenadeOrMineAnim() && uberAnim_->IsGrenadePinPullActive())
	{
		// override previous readytofire, grenade will be ready only after animation is finished
		isReadyToFire = false;
		if(uberAnim_->IsGrenadePinPullFinished() && IsPlantingMine() == false ) // ready to throw
		{
			if(grenadeReadyTime == 0)
				grenadeReadyTime = r3dGetTime(); // first frame when we finished playing pin pull animation, store time, as after that grenade timer is start to tick :)
			if(!m_isPressedFireTrigger || (r3dGetTime()-grenadeReadyTime)>wpn->getAmmoDelay()) // released trigger -> throw
			{
				holdingDelay = r3dGetTime()-grenadeReadyTime;
				grenadeReadyTime = 0;
				isReadyToFire = true;
			}
		}
	}

	// sniper aiming and holding breath logic
	{
		// reset flag that we're holding breath only when key is released. used for prevent sprinting (same key)
		if(m_isHoldingBreath && !InputMappingMngr->isPressed(r3dInputMappingMngr::KS_HOLD_BREATH))
			m_isHoldingBreath = false;
		
		static float last_timer = 0;
		if(wpn->getCategory() == storecat_SNP && m_isAiming) // sniper rifle and in scope mode
		{
			r3d_assert(NetworkLocal); // just to make sure, as this code should run only for local player (using statics here)

			if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_HOLD_BREATH) && m_Breath>0/* && m_BreathPenaltyTime<=0*/)
			{
				m_isHoldingBreath = true;
			
				//ViewAngle += SniperViewMod;
				//SniperViewMod.Assign(0,0,0);
				m_Breath = m_Breath - r3dGetFrameTime();
				if(m_Breath < 0)
					m_Breath = 0;
				//if(m_Breath <= 0)
				//	m_BreathPenaltyTime = GPP->c_fSniperRifleAfterHoldBounceTime;
			}
			else
			{
				float swayDecrease = 0.75f;
				int BreathControl = CurLoadout.getSkillLevel(CUserSkills::RECON_BreathControl);
				switch(BreathControl)
				{
				case 1: swayDecrease *= 0.9f; break;
				case 2: swayDecrease *= 0.8f; break;
				case 3: swayDecrease *= 0.6f; break;
				case 4: swayDecrease *= 0.5f; break;
				case 5: swayDecrease *= 0.25f; break;
				default:break;
				}

				SniperViewMod.Assign((r3dSin(last_timer/3)*3)*swayDecrease, (r3dCos(last_timer))*swayDecrease, 0);
				last_timer += r3dGetFrameTime();
			}
		}
		else
		{
			SniperViewMod.Assign(0,0,0);
			last_timer = 0;
		}
	}

	if(isReadyToFire) 
	{
		if(wpn->isGrenadeOrMineAnim())
		{
			uberAnim_->StartGrenadeThrowAnimation();
		}

		int numActualShots = wpn->getNumShotsRequired();
		int numVirtualShots = 0; // for shotgun
		if(numActualShots && wpn->getCategory() == storecat_SHTG)
			numVirtualShots = 7;

		while(numActualShots>0)
		{
			executeWeaponFireLocalPlayer(gCam, recoil, spread, recoilCoolDownMod, holdingDelay, true, !m_isInScope, 0);	
			--numActualShots;

			// update spread after shooting
			CurrentSpread += SpreadIncrease[wpn->getCategory()-storecat_ASR]*wpn->getSpread();
			CurrentSpread = R3D_MIN(CurrentSpread, wpn->getSpread());
#if ENABLE_ZOMBIES	
			lifeProperties.weaponFired = true;
#endif
		}
		while(numVirtualShots>0)
		{
			executeWeaponFireLocalPlayer(gCam, recoil, spread, recoilCoolDownMod, holdingDelay, false, !m_isInScope, 0);	
			--numVirtualShots;
#if ENABLE_ZOMBIES	
			lifeProperties.weaponFired = true;
#endif
		}

		uberAnim_->StartRecoilAnim();
	}

	m_ReticleTargetScale = R3D_MIN(1.0f + spread, 12.0f);

	// update reload anim
	if(wpn->isReloading())
	{
		uberAnim_->StartReloadAnim();
	}
	else
	{
		uberAnim_->StopReloadAnim();
	}

	VMPROTECT_End();
}

bool obj_AI_Player::IsPlantingMine()
{
	const Weapon* curWeapon = m_Weapons[m_SelectedWeapon];
	if(curWeapon && curWeapon->getConfig()->getGrenadeAnimType() != WeaponConfig::GRENADE_ANIM_Normal)
	{
		if(uberAnim_->GetGrenadeAnimState())
			return true;
	}
	
	return false;
}

float obj_AI_Player::GetGrenadeLaunchDelay()
{
	return uberAnim_->GetGrenadeLaunchFrame() / 25.0f;
}

// make sure this function is the same on server: bool ServerGameLogic::CanDamageThisObject(const GameObject* targetObj)
bool canDamageTarget(const GameObject* obj)
{
	if(obj->isObjType(OBJTYPE_Human))
		return true;
	if(obj->Class->Name == VMPROTECT_DecryptStringA("obj_UAV"))
	{
		// server will check if UAV killed, then it will say that object is not damageable
		// on a client once UAV is killed, object will be removed, so unless there is bad desync, it should be okay
		return true;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("obj_Mine"))
	{
		// same logic for mine as for UAV
		return true;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("obj_LightMesh"))
	{
		obj_LightMesh* lmesh = (obj_LightMesh*)obj;
		if(lmesh->isLightOn())
			return true;
		return false;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("DroppedMotionSensor"))
	{
		return true;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("DroppedRespawnBeacon"))
	{
		return true;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("obj_RiotShield"))
	{
		return true;
	}
	if(obj->Class->Name == VMPROTECT_DecryptStringA("obj_AutoTurret"))
	{
		return true;
	}

	return false;
}

float g_ScopeFeedbackTimer = 0.0f;

void obj_AI_Player::executeWeaponFireLocalPlayer(const r3dPoint3D& from, float recoil, float spread, float recoilCoolDownMod, float holdingDelay, bool executeWeaponFireCode, bool checkAgainsMuzzler, int damagePercFromPiercing)
{
	r3d_assert(NetworkLocal);

	Weapon* wpn = m_Weapons[m_SelectedWeapon];
	r3d_assert(wpn);
	r3d_assert(!wpn->isUsableItem());

	r3dPoint3D	shootTargetPos;
	r3dPoint3D	shootTargetNorm;
	GameObject*	shootTarget;
	r3dMaterial*	shootMaterial;
	const char*	shootActorName;

	// reset rotation if we're firing from idle mode
	if(PlayerState == PLAYER_IDLE)
	{
		bodyAdjust_x = 0;
		m_fPlayerRotation = m_fPlayerRotationTarget;
	}

	if(wpn->getCategory() == storecat_SNP)
	{
		// increase recoil for sniper rifle artificially
		recoil *= 10.0f;
		//if(!m_isFinishedAiming && spread>0) // if not aiming, make spread really huge
		//	spread = 50.0f;
	}

	r3dPoint3D muzzlerPos = GetMuzzlerPosition();
	{
		if(wpn->getCategory() == storecat_MELEE)
			updateMeleeTarget(this, GetPosition()+r3dPoint3D(0, bCrouch?0.6f:1.5f, 0.0f), muzzlerPos, shootTargetPos, shootTargetNorm, &shootMaterial, &shootTarget, &shootActorName, spread, checkAgainsMuzzler);
		else
			updateShootingTarget(this, from, muzzlerPos, shootTargetPos, shootTargetNorm, &shootMaterial, &shootTarget, &shootActorName, spread, checkAgainsMuzzler);
	}

	RecoilViewModTarget.y += recoil * u_GetRandom(0.85f, 1.75f);
	if(RecoilViewModTarget.y > GPP->c_MaxVerticalRecoil) // limit how much gun goes up
	{
		RecoilViewModTarget.y = u_GetRandom(GPP->c_MaxVerticalRecoil*0.95f, GPP->c_MaxVerticalRecoil*1.1f);
	}
	RecoilViewModTarget.x += recoil*u_GetRandom(-0.15f, 0.3f); // + to a side

	if(executeWeaponFireCode) // set cooldown only for actual fire
	{
		RecoilCooldown = R3D_MAX(wpn->getRateOfFire()*recoilCoolDownMod, r3dGetFrameTime()*2.50f);
	}

	if(wpn->getCategory()==storecat_SNP || wpn->getCategory()==storecat_SUPPORT || wpn->getCategory()==storecat_SHTG)
		if(RecoilCooldown > r3dGetFrameTime()*8.0f)
			RecoilCooldown = r3dGetFrameTime()*8.0f;



	// send weapon fired event
	if(executeWeaponFireCode && wpn->getAnimType() != WPN_ANIM_GRENADE )
	{
		PKT_C2C_PlayerFired_s n; 
		n.fire_from = GetPosition();
		n.fire_to = shootTargetPos;
		n.holding_delay = holdingDelay;
		n.debug_wid = m_SelectedWeapon;
		n.wasAiming = m_isAiming;
		p2pSendToHost(this, &n, sizeof(n), true);
	}

	if(wpn->isImmediateShooting())
	{
		if ( ProcessBulletHit(damagePercFromPiercing, this, shootTargetPos, shootTargetNorm, shootTarget, shootMaterial, shootActorName,  wpn->getConfig() )  == false ) 
		{	
			r3dPoint3D dir = shootTargetPos - from;
			dir.Normalize();
			executeWeaponFireLocalPlayer(shootTargetPos + dir*0.1f, 0.0f, 0.0f, 0.0f, holdingDelay, false, false, damagePercFromPiercing);
		}
	}
	if(executeWeaponFireCode)
	{
		wpn->Fire(shootTargetPos, uberEquip_->getWeaponBone(GetSkeleton(), DrawFullMatrix), holdingDelay);
		
		extern float g_shootCameraShakeTimer;
		g_shootCameraShakeTimer = 1.0f;
		if(m_isAiming && hasScopeMode())
		{
			g_ScopeFeedbackTimer = 1.0f;
		}
	}

	//	Change player noise value
#if ENABLE_ZOMBIES	
	lifeProperties.weaponFired = true;
#endif
}

void obj_AI_Player::UpdateAnimations(const D3DXMATRIX& CharDrawMatrix)
{
	const float TimePassed = r3dGetFrameTime();
#if ENABLE_RAGDOLL
	bool ragdoll = m_PhysSkeleton && m_PhysSkeleton->IsRagdollMode();
	if (!ragdoll)
#endif
	{
		uberAnim_->anim.Update(TimePassed, r3dPoint3D(0,0,0), CharDrawMatrix);
		uberAnim_->anim.Recalc();
	}

	if(m_PhysSkeleton)
		m_PhysSkeleton->syncAnimation(GetSkeleton(), DrawFullMatrix, GetAnimation());

#if ENABLE_RAGDOLL
	if (ragdoll)
	{
		r3dBoundBox bbox = m_PhysSkeleton->getWorldBBox();
		SetPosition(bbox.Org + bbox.Size * 0.5f);
		bbox.Org = -bbox.Size * 0.5;
		SetBBoxLocal(bbox);
	}
#endif
}

r3dPoint3D obj_AI_Player::GetHeadPosition()
{
	D3DXMATRIX m;
	uberAnim_->anim.GetCurrentSkeleton()->GetBoneWorldTM(boneId_Bip01_Head, &m, DrawFullMatrix);

	return r3dPoint3D(m._41, m._42, m._43);
}

void obj_AI_Player::DoDeath(int networdIDKiller, bool forced_by_server, STORE_CATEGORIES damageSource)
{
	//r3dOutToLog("%s: DoDeath\n", Name.c_str());
	r3d_assert(!bDead);

	snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/BulletHits/Hit_Death"), GetPosition());		

	CurrentSpread = 0;

	m_needToDrawRiotShieldPlacement = 0;
	m_spawnedRiotShield = false;
	m_needToDrawAutoTurretPlacement = 0;
	m_spawnedAutoTurret = false;
	m_needToDrawRespawnBeaconPlacement = 0;

	m_canPlaceConsumableItem = false;

	m_ItemSimulateLeftClick = false;

	m_SpeedBoost = 0;
	m_SpeedBoostTime = 0;
	m_MorhineShotTime = 0;
	m_BandagesEffectTime = 0;

	m_targetIconTime = 0;
	m_localTargetIconTime = 0;
	if(!m_MarkedTargetIcon.IsUndefined())
	{
		if(hudMain)
			hudMain->deleteScreenIcon(m_MarkedTargetIcon);
	}

	bCrouch = 0;
	m_isAiming = 0;
	m_isPressedFireTrigger = false;
	m_Health = 0;
	lastDamageRcvd = 0;
	BloodEffect = 0;
	bDead = 1;
	TimeOfDeath = r3dGetTime();
	DeathDamageSource = damageSource;
	PosOfDeath = GetPosition();
	Dead_KillerID = networdIDKiller;
	DisableKillerView = false;

	m_BerserkerTime = 0;
	m_AuraType = AT_NONE;
	grenadeReadyTime = 0;

	// make sure the player is not visible on the minimap. 
	m_fVisibleTimer = 0;
	m_visibleOnMinimap = false;

	setHasSabotageBomb(false);

	if(!(forced_by_server && gClientLogic().m_gameInfo.mapType==GBGameInfo::MAPT_Bomb)) // if end of round in sabotage, don't play death anim
	{
		PlayerState = PLAYER_DIE;
		uberAnim_->AnimPlayerState = PLAYER_DIE;
		uberAnim_->StartDeathAnim();
		DoRagdoll(true, lastTimeHitBone, &lastTimeHitForce);
	} 
	else
	{
		// in sabotage, switch player to idle
		PlayerState = PLAYER_IDLE;
	}
	
	RealAcceleration.Assign(0,0,0);
	InputAcceleration.Assign(0,0,0);

	if(!forced_by_server)
	{
		GameStats.Deaths++;
	}

	if(!NetworkLocal)
	{
		if(hudMinimap && hudMinimap->IsInited() && m_onMinimap)
		{
			hudMinimap->EraseUnit(m_MinimapTagIconName);
			m_onMinimap = false;
		}
		if(!m_MedicTeamIcon.IsUndefined() && hudMain)
		{
			hudMain->deleteScreenIcon(m_MedicTeamIcon);
		}

		// count kill streaks for local player
		const ClientGameLogic& CGL = gClientLogic();
		if(CGL.localPlayer_ && CGL.localPlayer_->NetworkID == Dead_KillerID)
		{
			if(!CGL.localPlayer_->bDead)
			{
				if((r3dGetTime() - CGL.localPlayer_->m_LastKillTime) < GPP->c_fKillStreakTimeout)
				{
					++CGL.localPlayer_->m_KillStreak;
					if(CGL.localPlayer_->m_KillStreak == 3 && CGL.localPlayer_->CurLoadout.hasItem(AbilityConfig::AB_NinjaKiller) && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb) // ninja killer ability
					{
						hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_TripleKillNinjaKiller"));

						ObjectManager& GW = GameWorld();
						for(GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
						{
							if(obj->ObjTypeFlags & OBJTYPE_Human)
							{
								((obj_AI_Player*)obj)->setMinimapVisibleTimer(GPP->c_fMinimapNinjaKillerTimer, true);
							}
						}

						if(CGL.localPlayer_->m_KillStreak >= KILLSTREAK_FOR_SECOND_WIND && CGL.localPlayer_->CurLoadout.hasItem(AbilityConfig::AB_SecondWind) ) // Second Wind ability
						{
							CGL.localPlayer_->m_RemainingSecondWindTime = TIME_FOR_SECOND_WIND;
						}
					
					}
				}
				else // if timer ran out, then reset kill streaks counter
					CGL.localPlayer_->m_KillStreak = 1;
				CGL.localPlayer_->m_LastKillTime = r3dGetTime();
			}
		}
	}
	else
	{
		
		if(shownWeaponPickup)
		{
			hudMain->hidePickupMsg();
			hudMain->hideActionMessage( "pickup" );
		}
		shownWeaponPickup = false;

		hudMain->setHealth(0);

		ToggleUAVView(true); // force disable
		ToggleLaserView(true); // force disable

		r_hud_filter_mode->SetInt(0); // turn off NVG

		if( CurLoadout.hasItem(AbilityConfig::AB_EyeSpy) && ( gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)) // eye spy ability
		{	
			for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
			{
				if( obj->ObjTypeFlags & OBJTYPE_Human )
				{
					obj_AI_Player* player  = static_cast< obj_AI_Player* > ( obj );

					player->setMinimapVisibleTimer( 0.0f, true );
				}
			}
		}
	}

	return;
}

void obj_AI_Player::DoRespawn(int newTeamId, int slotNum, const wiLoadoutSlot& lslot, const wiWeaponAttachments& attms, const r3dPoint3D& pos, float dir, float spawnProtection)
{
	R3DPROFILE_FUNCTION("obj_AI_Player::DoRespawn");
	//r3dOutToLog("%s: DoRespawn at %f %f %f\n", UserName, pos.x, pos.y, pos.z);
	r3d_assert(bDead);

	m_QuickThrowGrenade = 0;
	m_ItemSimulateLeftClick = false;

	CurrentSpread = 0;

	m_needToDrawRiotShieldPlacement = 0;
	m_spawnedRiotShield = false;
	m_needToDrawAutoTurretPlacement = 0;
	m_spawnedAutoTurret = false;
	m_needToDrawRespawnBeaconPlacement = 0;

	m_canPlaceConsumableItem = false;

	m_SpeedBoost = 0;
	m_SpeedBoostTime = 0;
	m_MorhineShotTime = 0;
	m_BandagesEffectTime = 0;

	// recreate player tag each time on respawn to fix a rare bug when player has blue tag but he is an enemy
	if(!m_TagIcon.IsUndefined())
	{
		hudMain->deleteScreenIcon(m_TagIcon);
	}

	if(TeamID != newTeamId) // switched teams
	{
		TeamID = newTeamId; // update it before updating CPs

		if(NetworkLocal) // if we changed a team, then we need to recreate tag icons for all players
		{
			const ClientGameLogic& CGL = gClientLogic();
			for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
			{
				obj_AI_Player* plr = CGL.GetPlayer(i);
				if(plr)
				{
					if(!plr->m_TagIcon.IsUndefined())
						hudMain->deleteScreenIcon(plr->m_TagIcon);
					obj_UAV* uav = (obj_UAV*)GameWorld().GetObject(plr->uavId_);
					if(uav && !uav->m_TagIcon.IsUndefined())
						hudMain->deleteScreenIcon(uav->m_TagIcon);
				}
			}
			for(int i=0; i<gCPMgr.NumCP(); ++i)
			{
				((obj_ControlPoint*)gCPMgr.GetCP(i))->SetControlPointStatusOnMinimap();
			}
		}
	}

	TeamID      = newTeamId;
	
	CurLoadout  = lslot;
	LoadoutSlot = slotNum;

	UpdateLoadoutSlot(CurLoadout, attms);
	
	const ClientGameLogic& CGL = gClientLogic();

	if(NetworkLocal)
	{
		// because we can switch between TPS\FPS
		if(hudMain)
			hudMain->updateReticlePosition();

		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		{
			if(m_Weapons[i]) 
				m_Weapons[i]->Reset();
		}

		uberEquip_->ResetSlots();

		if(hudRespawn && hudRespawn->isInit() && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		{
			if(hudRespawn->getSelectedTeamID() != TeamID)
				hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_MovedToOppositeTeam"));
		}

		if(hudRespawn && hudRespawn->isActive())
			hudRespawn->Deactivate();
	}
	else
	{
		if((CGL.localPlayer_ && CGL.localPlayer_->TeamID == TeamID))
		{
			hudMinimap->AddUnit(m_MinimapTagIconName, false, false, GetPosition());
			m_onMinimap = true;
		}
		if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		{
			if(CurLoadout.hasItem(AbilityConfig::AB_GuardianAngel)) // guardian angel
			{
				if(m_MedicTeamIcon.IsUndefined())
					hudMain->addMedicIcon(m_MedicTeamIcon);
				hudMain->moveScreenIcon(m_MedicTeamIcon, r3dPoint3D(-1, -1, -1), false, true);
				hudMain->setScreenIconScale(m_MedicTeamIcon, 0.5f);
			}
		}
	}

	m_visibleOnMinimap = false;
	m_onMinimap = false;
	m_fVisibleTimer = 0;

	m_Health      = getMaxHealth();
	lastDamageRcvd = 0;
	BloodEffect = 0;
	bDead      = 0;
	m_KillStreak = 0;
	m_LastKillTime = 0;
	m_Energy = getMaxStamina(); // reset sprint energy
	m_Breath = getMaxBreath();
	lastTimeWeaponSwitch = 0;
	m_RemainingSecondWindTime = 0.0f;  // reset it to 0 

	m_SpawnProtectedUntil = r3dGetTime() + spawnProtection;
	TimeOfLastRespawn = r3dGetTime();

	r3dVector spawnPosition = pos;
	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
	if(g_pPhysicsWorld->raycastSingle(PxVec3(spawnPosition.x, spawnPosition.y+1, spawnPosition.z), PxVec3(0, -1, 0), 2, PxSceneQueryFlag::eIMPACT, hit, filter))
	{
		spawnPosition.x = hit.impact.x;
		spawnPosition.y = hit.impact.y;
		spawnPosition.z = hit.impact.z;
	}

	//r3dOutToLog("respawning player %s at %0.2f, %0.2f, %0.2f\n", UserName, pos.x, pos.y, pos.z);
	TeleportPlayer(spawnPosition, "Respawn");

	SetVelocity(r3dPoint3D(0, 0, 0));
	ViewAngle.Assign(dir, 0, 0);

	Undie();

}

void obj_AI_Player::Undie()
{
	afterRespawnTimer = 2.0f;

	lastWeapDataRep = r3dGetTime();

	bDead = 0;

	// clear death anim
	uberAnim_->anim.AnimTracks.clear();

	DoRagdoll(false);

	PlayerState = PLAYER_IDLE;
	SyncAnimation(true);

}

static void dumpAnim(r3dAnimation& anim)
{
  for(size_t i=0; i<anim.AnimTracks.size(); i++) 
  {
    const r3dAnimation::r3dAnimInfo& ai = anim.AnimTracks[i];
          
    char st[256] = "";
    if(ai.GetStatus() & ANIMSTATUS_Playing) strcat(st, "Play ");
    if(ai.GetStatus() & ANIMSTATUS_Paused) strcat(st, "Pause ");
    if(ai.GetStatus() & ANIMSTATUS_Finished) strcat(st, "Finish ");
    if(ai.GetStatus() & ANIMSTATUS_Fading) strcat(st, "Fade ");
    if(ai.GetStatus() & ANIMSTATUS_Expiring) strcat(st, "Expire ");
          
    r3dOutToLog("%d: %s %f, %X %s\n", ai.GetTrackID(), ai.GetAnim()->GetAnimName(), ai.GetInfluence(), ai.GetStatus(), st);
  }
}

r3dSkeleton* obj_AI_Player::GetSkeleton()
{
	return uberAnim_->anim.pSkeleton;
}

r3dAnimation& obj_AI_Player::GetAnimation()
{
	return uberAnim_->anim;
}

void obj_AI_Player::SyncAnimation(bool force)
{
	R3DPROFILE_FUNCTION("obj_AI_Player::SyncAnimation");
	if(bDead)
		return;
		
	uberAnim_->SyncAnimation(PlayerState, PlayerMoveDir, force, m_Weapons[m_SelectedWeapon]); // firstly sync to update weapon pointer and only then update jump
	uberAnim_->UpdateJump(bOnGround);
}

static float accumLastDamageRcvd = 0;
void obj_AI_Player::ApplyDamage(const r3dPoint3D& dmgPos, float damage, GameObject* fromObj, int bodyBone, int dmgType)
{
	lastTimeHit = r3dGetTime();
	lastTimeHitBone = bodyBone;
	lastDamageRcvd = damage;
	accumLastDamageRcvd += damage;
	m_Health = m_Health - damage;
	lastTimeHit_OldHealth = m_Health;

	// sergey's code here. I guess he was trying to make it so that only if you shoot other player, you should hear sound confirmation that you hit him. 
	// so I fixed it. As prev.code was playing sound from the position of the player, which might be >100 meters away, and so you never hear hit confirmation.
	// still. retarded code. it shouldn't be here. should be two separate sound events: one hit player by player, and second hit confirmation played by local player only
	if (fromObj == gClientLogic().localPlayer_ && gClientLogic().localPlayer_!=NULL)
	{
		if(bodyBone == uberAnim_->GetBoneID("Bip01_Head"))
			snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/BulletHits/Hit_Headshot"), gClientLogic().localPlayer_->GetPosition());
		else
			snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/BulletHits/Hit_Body"), gClientLogic().localPlayer_->GetPosition());		
	}

	// apply ragdoll force from object position, or up if too close
	r3dPoint3D ragHitPos = GetPosition();
	lastTimeHitForce = r3dPoint3D(0, 1, 0);
	if(bodyBone < GetSkeleton()->NumBones)
		ragHitPos = uberEquip_->getBonePos(bodyBone, GetSkeleton(), DrawFullMatrix);
	if((ragHitPos - fromObj->GetPosition()).LengthSq() > 0.001f)
		lastTimeHitForce = (ragHitPos - fromObj->GetPosition()).NormalizeTo();

	float dmgForce = 10.0f;
	switch(dmgType)
	{
	case storecat_ASR:
		dmgForce = 30.0f;
		break;
	case storecat_SNP:
		dmgForce = 60.0f;
		break;
	case storecat_SHTG:
		dmgForce = 60.0f;
		break;
	case storecat_MG:
		dmgForce = 40.0f;
		break;
	case storecat_SUPPORT:
		dmgForce = 70.0f;
		break;
	case storecat_HG:
		dmgForce = 10.0f;
		break;
	case storecat_SMG:
		dmgForce = 20.0f;
		break;
	}
	lastTimeHitForce *= dmgForce;

#ifndef FINAL_BUILD
	if(NetworkLocal && m_isAiming)
	{
		float recoil = 3.5f;
		RecoilViewModTarget2.x = recoil * (random(2)==1?u_GetRandom(-1.0f, -0.5f):u_GetRandom(0.5f, 1.0f));
		RecoilViewModTarget2.y = recoil * (random(2)==1?u_GetRandom(-1.0f, -0.5f):u_GetRandom(0.5f, 1.0f));
	}
#endif
}

bool obj_AI_Player::ChangeWeaponByIndex(int index)
{
	r3d_assert(index>=0 && index < NUM_WEAPONS_ON_PLAYER);
	if(m_SelectedWeapon == index)
		return false;

	m_needToDrawRiotShieldPlacement = 0;
	m_needToDrawAutoTurretPlacement = 0;
	m_needToDrawRespawnBeaconPlacement = 0;
	m_canPlaceConsumableItem = false;

	m_PrevSelectedWeapon = m_SelectedWeapon;
	m_SelectedWeapon = index;

	uberAnim_->StopReloadAnim();

	if(m_PrevSelectedWeapon >= 0 && m_Weapons[m_PrevSelectedWeapon])
	{
		m_Weapons[m_PrevSelectedWeapon]->OnUnequip();
	}

	if(m_Weapons[m_SelectedWeapon])
	{
		m_Weapons[m_SelectedWeapon]->OnEquip();
	}
	
	UpdateCharWeaponMeshes();
	return true;
}

void obj_AI_Player::UpdateCharWeaponMeshes()
{
	//r3d_assert(!m_Weapons[m_SelectedWeapon].Empty());

	bool isFirstPerson = g_camera_mode->GetInt()==2 && NetworkLocal;
	uberEquip_->SetSlot(SLOT_Weapon, m_Weapons[m_SelectedWeapon]);
	if(m_SelectedWeapon==0)
	{
		uberEquip_->SetSlot(SLOT_WeaponBackRight, (Weapon*)NULL);
	}
	else
	{
		if(m_Weapons[0])
		{
			uberEquip_->SetSlot(SLOT_WeaponBackRight, m_Weapons[0]);
		}
	}
	if(m_SelectedWeapon==1)
	{
		uberEquip_->SetSlot(SLOT_WeaponBackLeft, (Weapon*)NULL);
		uberEquip_->SetSlot(SLOT_WeaponBackLeftRPG, (Weapon*)NULL);
	}
	else
	{
		if(m_Weapons[1])
		{
			if(m_Weapons[1]->getCategory() == storecat_SUPPORT)
			{
				uberEquip_->SetSlot(SLOT_WeaponBackLeft, (Weapon*)NULL);
				uberEquip_->SetSlot(SLOT_WeaponBackLeftRPG, m_Weapons[1]);
			}
			else
			{
				uberEquip_->SetSlot(SLOT_WeaponBackLeftRPG, (Weapon*)NULL);
				uberEquip_->SetSlot(SLOT_WeaponBackLeft, m_Weapons[1]);
			}
		}
	}
	if(m_SelectedWeapon==2)
		uberEquip_->SetSlot(SLOT_WeaponSide, (Weapon*)NULL);
	else
	{
		if(m_Weapons[2])
			uberEquip_->SetSlot(SLOT_WeaponSide, m_Weapons[2]);
	}
}

float obj_AI_Player::getMaxHealth()
{
	float maxHealth = 100.0f;
	if(gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
	{
		/*if(m_Skills[7] == 1)
			maxHealth *= 1.03f;
		else if(m_Skills[7] == 2)
			maxHealth *= 1.05f;
		else if(m_Skills[7] == 3)
			maxHealth *= 1.07f;
		else if(m_Skills[7] == 4)
			maxHealth *= 1.10f;
		else if(m_Skills[7] == 5)
			maxHealth *= 1.15f;*/
	}
	return maxHealth;
}

float obj_AI_Player::getMaxStamina()
{
	float maxStamina = GPP->c_fSprintMaxEnergy;
	if(gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
	{
		/*if(m_Skills[1] == 1)
			maxStamina *= 1.05f;
		else if(m_Skills[1] == 2)
			maxStamina *= 1.10f;
		else if(m_Skills[1] == 3)
			maxStamina *= 1.15f;
		else if(m_Skills[1] == 4)
			maxStamina *= 1.20f;
		else if(m_Skills[1] == 5)
			maxStamina *= 1.25f;*/
	}
	return maxStamina;
}

float obj_AI_Player::getMaxBreath()
{
	float maxBreath = GPP->c_fSniperRifleHoldBreathTime;
	switch(CurLoadout.getSkillLevel(CUserSkills::RECON_LongBreathing))
	{
	case 1: maxBreath += 1; break;
	case 2: maxBreath += 2; break;
	case 3: maxBreath += 3; break;
	case 4: maxBreath += 4; break;
	case 5: maxBreath += 5; break;
	}
	return maxBreath;
}

float obj_AI_Player::getMinimapDetectionRadius()
{
	float radius = GPP->c_fMinimapEnemyVisibileRadius;
	int EnemyRadar = CurLoadout.getSkillLevel(CUserSkills::RECON_EnemyRadar);
	switch(EnemyRadar)
	{
	case 1: radius += 1.0f; break;
	case 2: radius += 2.0f; break;
	case 3: radius += 3.0f; break;
	case 4: radius += 5.0f; break;
	default:break;
	}
	return radius;
}

float obj_AI_Player::getMinimapDetectionFireRadius()
{
    float radius = GPP->c_fMinimapEnemyShootVisibleRadius;
    if(gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
    {
        /*if(m_Skills[6] == 1)
            radius += 5;
        else if(m_Skills[6] == 2)
            radius += 10;
        else if(m_Skills[6] == 3)
            radius += 15;
        else if(m_Skills[6] == 4)
            radius += 20;
        else if(m_Skills[6] == 5)
            radius += 30;*/
    }
    return radius;
}

void obj_AI_Player::StartJump()
{
	r3d_assert(NetworkLocal);
	
	uberAnim_->StartJump();
	JumpVelocity  = 17;
	JumpStartTime = r3dGetTime() + uberAnim_->jumpStartTime;

	PKT_C2C_PlayerJump_s n;
	p2pSendToHost(this, &n, sizeof(n));
}

bool obj_AI_Player::IsJumpActive()
{
	// going up on jump, or in air.
	return (JumpVelocity > 0 || !bOnGround);
}

void obj_AI_Player::DetectIfOnGround()
{
	//static bool prevOnGround = bOnGround;
	
	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));
	if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y + 0.5f, GetPosition().z), PxVec3(0, -1, 0), 1.0f, PxSceneQueryFlags(PxSceneQueryFlag::eDISTANCE), hit, filter))
	{
		bOnGround = (hit.distance < 0.6f);
	}
	else
		bOnGround = false;

	// hack to check if player got glued to some geometry while jumping. Happen if he jumped on something that has some bumps and character controller thinks that he can stay on that tiny bump
	if(!bOnGround && R3D_ABS(oldstate.Position.y - GetPosition().y) < 0.01f)
		bOnGround = true;

	//if(prevOnGround != bOnGround)
	//{
	//	prevOnGround = bOnGround;
	//	r3dOutToLog("onGround=%d\n", bOnGround);
	//}
}
//////////////////Anticheat///////////////////////////////////////////

typedef ULONG(__stdcall *NtQueryPerformanceCounter)(PLARGE_INTEGER a1, PLARGE_INTEGER a2);

bool TimingCheck()
{
	LARGE_INTEGER delta1;
	LARGE_INTEGER delta2;

	NtQueryPerformanceCounter PerfQuery=(NtQueryPerformanceCounter)GetProcAddress(GetModuleHandle("ntdll"),"NtQueryPerformanceCounter");

	ULONG count=0;
	ULONG time=0;
	
	SwitchToThread();

	while(count<5)
	{

		QueryPerformanceCounter(&delta1);

		PerfQuery(&delta2,NULL);

		delta1.LowPart <<= 16;
	
		delta1.LowPart >>= 24;
	
		delta2.LowPart <<= 16;

		delta2.LowPart >>= 24;

		if(delta1.LowPart!=delta2.LowPart) time++;

		count++;

	}

	if(time==5) return true;

	return false;
}

#define JUNK_CODE_ONE        \
    __asm{push eax}            \
    __asm{xor eax, eax}        \
    __asm{setpo al}            \
    __asm{push edx}            \
    __asm{xor edx, eax}        \
    __asm{sal edx, 2}        \
    __asm{xchg eax, edx}    \
    __asm{pop edx}            \
    __asm{or eax, ecx}        \
    __asm{pop eax}


//////////////////////////////////////////////////////////////////////
void obj_AI_Player::UpdateLocalPlayerMovement()
{
	r3d_assert(NetworkLocal);
	
	VMPROTECT_BeginMutation("obj_AI_Player::UpdateLocalPlayerMovement");
	
	// important: limit movement to max for 1 sec. so server can check for teleports
	const float TimePassed = R3D_MIN(r3dGetFrameTime(), 0.1f);
	const float curTime    = r3dGetTime();

	RealAcceleration = InputAcceleration;

	// apply weight to moving speed before gravity and jumping
	float weightAdj = 1.0f + ((m_GearWeight/100.0f)); // each 100kg half your speed
	RealAcceleration = RealAcceleration/weightAdj;

	// locally add gravity to acceleration
	float Gravity = 9.81f;
	float deltaTimeInAir =(curTime-StartFallingTime);
	if(!bOnGround && JumpVelocity<=0) // don't accelerate falling while jumping, only when actually falling
		Gravity += Gravity * deltaTimeInAir;
	RealAcceleration.Y -= Gravity;
	
	if(JumpVelocity > 0 && curTime >= JumpStartTime)
	{
		JumpVelocity -= Gravity * TimePassed * 2.5f;
		RealAcceleration.Y += JumpVelocity;
		// reset falling time
		if(JumpVelocity<=0)
			StartFallingTime = curTime;
	}
	if(bOnGround)
		StartFallingTime = curTime; // keep reseting timer, otherwise when jump it will not be null

	r3dPoint3D vel    = r3dPoint3D(0,0,0);
	vel    += (GetvForw()  * RealAcceleration.Z);
	vel    += (GetvRight() * RealAcceleration.X);
	vel.Y  += RealAcceleration.Y;
	
	// send some info to server about fast movement
	if(!RUS_CLIENT)
	{
		static float lastUpd = -999;
		r3dPoint3D velXZ(vel.x, 0, vel.z);
		if(velXZ.Length() > 8.0f && curTime > lastUpd + 60.0f)
		{
			lastUpd = curTime;
			char buf[1024];
			sprintf(buf, "%.2f %.2f %.1f/%.1f %.2f %.1f", 
				velXZ.x, velXZ.z, GPP->AI_SPRINT_SPEED, GPP->AI_RUN_SPEED, TimePassed, weightAdj);

			PKT_C2S_DBG_LogMessage_s n;
			r3dscpy(n.msg, buf);
			p2pSendToHost(this, &n, sizeof(n));
		}
	}

	ControllerPhysObj* controller = (ControllerPhysObj*)PhysicsObject;

	/////////////////////////////////////////

	JUNK_CODE_ONE

	/////////////////////////////////////////

	controller->Move(vel * TimePassed, 0.5f);
	r3dPoint3D prevPos = GetPosition();
	parent::Update(); // manually update position after updating physics. Needed for camera, otherwise it will be lagging one frame behind
	SetVelocity(vel);

	float ll = (prevPos-GetPosition()).LengthSq();
	if(ll > 0.0001f)
	{
		extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
		WaterSplash(GetPosition(), 2.0f, 5.0f, 0.1f*ll, 3);
	}


	bool bWasOnGround = bOnGround;
	DetectIfOnGround();

	/////////////Anticheat/////////////////////

	JUNK_CODE_ONE

	//////////////////////////////////////////


	/////////////Anticheat//////////////////////////////
   	//RANDOME CODE 
	//========================================================================================

	if(IsDebuggerPresent()){
		r3dOutToLog("WARNING : ERROR 104");
		extern bool gKillFlag;
		gKillFlag  = true;
	}

	//========================================================================================

	////////////////////////////////////////////////////

	if(bWasOnGround && !bOnGround)
	{
		StartFallingHeight = GetPosition().y;
		StartFallingTime = r3dGetTime();
	}

	/////////////Anticheat/////////////////////

	JUNK_CODE_ONE

	//////////////////////////////////////////

	if(!bWasOnGround && bOnGround)
	{
		// calculate damage from falling if any
		float newHeight = GetPosition().y;
		if((StartFallingHeight - newHeight) > 5.0f) // more than 5 meters, we gonna break something
		{
			float damageAmount = ((StartFallingHeight - newHeight))*4.0f; // if falling more than 25 meters - dies immediatelly (damage >=100)
			damageAmount = R3D_CLAMP(damageAmount, 0.0f, 255.0f);

			// send damage to server
			PKT_C2S_FallingDamage_s n;
			n.damage = (BYTE)damageAmount;
			p2pSendToHost(this, &n, sizeof(n));
		}
	}
	
	/////////////Anticheat/////////////////////

	JUNK_CODE_ONE

	//////////////////////////////////////////

	if(PlayerState == PLAYER_MOVE_SPRINT)
	{
/*#ifdef FINAL_BUILD
		float decayPerSec = 1.0f; // minus one per each second
		//BOOST: Boost Stamina
		// Gives you 50% more sprinting time
		if(m_haveBoostStamina)
			decayPerSec *= 0.66f;

		m_Energy = m_Energy - (r3dGetFrameTime() * decayPerSec);
		if(m_Energy <= 0)
		{
			m_EnergyPenaltyTime = 3.0f;
		}
#endif*/
	}
	else if(bOnGround) // regen only if standing on ground
	{
		float regen_rate = 0.3f;
		// refill faster if idle or crouching
		if(PlayerState == PLAYER_IDLE || PlayerState == PLAYER_IDLEAIM || PlayerState == PLAYER_MOVE_CROUCH || PlayerState == PLAYER_MOVE_CROUCH_AIM)
			regen_rate = 0.5f;

		m_Energy = m_Energy + TimePassed*regen_rate; // regeneration rate
		if(m_EnergyPenaltyTime > 0.0f)
			m_EnergyPenaltyTime = m_EnergyPenaltyTime - TimePassed;
	}
	m_Energy = R3D_CLAMP((float)m_Energy, 0.0f, getMaxStamina());

	
	/////////////Anticheat/////////////////////

	JUNK_CODE_ONE

	//////////////////////////////////////////

	bool scopeRendering = hasScopeMode() && CurrentRig.allowScope;
	if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->getCategory() == storecat_SNP && m_isFinishedAiming) 
	{
		if(hudMain)
		{
			hudMain->SetEnergyValue(1.0f); // hide energy bar
			hudMain->SetBreathValue(R3D_CLAMP((m_Breath/getMaxBreath()), 0.0f, 1.0f));
		}
	}
	else
	{
		if(hudMain)
		{
			hudMain->SetBreathValue(2.0f); // hide breath
			hudMain->SetEnergyValue(m_Energy/getMaxStamina());
		}
	}

////////////////////Anticheat//////////////////////////////////////

	JUNK_CODE_ONE

	//TESTING ANTI SPEED HACk
	if(PlayerState == PLAYER_IDLE){
		if(plr_local_Velocity.Z >= 1.8){
			if(PlayerState == PLAYER_IDLE){
				r3dOutToLog("WARNING : ERROR 101-A");
				extern bool gKillFlag;
				gKillFlag  = true;
			}
		}
	}

	if(PlayerState == PLAYER_MOVE_RUN){
		if(plr_local_Velocity.Z >= 3.8){
			if(PlayerState == PLAYER_MOVE_RUN){
				r3dOutToLog("WARNING : ERROR 101-B");
				extern bool gKillFlag;
				gKillFlag  = true;
			}
		}
	}

	if(IsDebuggerPresent()){
			r3dOutToLog("WARNING : ERROR 104");
			extern bool gKillFlag;
			gKillFlag  = true;
	}
	
	if(bOnGround){
		if(plr_local_Velocity.Z >= 4.8){
			r3dOutToLog("WARNING : ERROR 101-C");
			extern bool gKillFlag;
			gKillFlag  = true;
		}
	}	
//////////////////////////////////////////////////////////////////////////

	VMPROTECT_End();
}

void obj_AI_Player::UpdateLoaded()
{
	int equipLoaded = uberEquip_->IsLoaded() ;

	int itemsLoaded = 1 ;

	for( int i = 0, e = NUM_WEAPONS_ON_PLAYER ; i < e ; i ++ )
	{
		if( Weapon* wpn = m_Weapons[ i ] )
		{
			if( !wpn->isLoaded() )
			{
				itemsLoaded = 0 ;
				break ;
			}
		}
	}

	if( equipLoaded && itemsLoaded )
	{
		m_Loaded = true ;

		for( int i = 0, e = NUM_WEAPONS_ON_PLAYER ; i < e ; i ++ )
		{
			if( Weapon* wpn = m_Weapons[ i ] )
			{
				wpn->getConfig()->updateMuzzleOffset(g_camera_mode->GetInt()==2 && NetworkLocal) ;
				if(g_camera_mode->GetInt()==2 && NetworkLocal)
					wpn->checkForSkeleton();
			}
		}

	}
}

BOOL obj_AI_Player::Update()
{
	if( !m_Loaded )
	{
		UpdateLoaded() ;
	}

	R3DPROFILE_FUNCTION("obj_AI_Player::Update");
	if(!NetworkLocal) // for local already done in UpdateLocalPlayerMovement
		parent::Update();
	r3d_assert(TeamID != TEAM_UNKNOWN);

	camoTimeLine.Update();

	// show pending rewards, even if game finished.
	if(NetworkLocal)
		ProcessRewardQueue();

	const ClientGameLogic& CGL = gClientLogic();
	if(CGL.gameFinished_)
		return TRUE;
		
	const float curTime = r3dGetTime();

#ifndef FINAL_BUILD
	if(NetworkLocal && gDisableShortcuts == false )
	{
		if( Keyboard->IsPressed( kbsLeftAlt ) && Keyboard->WasPressed( kbsK ) )
		{
			g_char_spect->SetInt( !g_char_spect->GetInt() ) ;
		}

		if(Keyboard->IsPressed(kbsLeftAlt) && hudEndRound && hudMain)
		{
			if(Keyboard->WasPressed(kbsF12))
			{
				hudEndRound->Activate();
			}

			if(Keyboard->WasPressed(kbsF5))
			{
				// send data update request to server
				PKT_C2S_DataUpdateReq_s n;
				p2pSendToHost(this, &n, sizeof(n), true);
				hudMain->AddMessage(L"Data update request sent");
			}

			if(Keyboard->WasPressed(kbsH))
			{
				static int selector = 0;
				const char* pressRTypes[] = {"blackops", "resupply", "fixer", "usekey", "needkey", "armBomb", "disarmBomb"};
				{
					hudMain->showActionMessage(pressRTypes[selector], "X");
					++selector;
					if(selector == 7) selector = 0;
				}
			}
			if(Keyboard->WasPressed(kbsJ))
			{
				TPSGameHud_UnlockAchievement( 1 );
			}
			if(Keyboard->WasPressed(kbsL))
			{
				hudMain->ShowScore(100, 200, 300, 500, 600);
			}
			if(Keyboard->WasPressed(kbsP))
			{
				hudMain->ShowKillTag(this, storecat_INVALID);
			}
			if(Keyboard->WasPressed(kbsO))
			{
				hudMain->AddKillMessage(this, this, storecat_SHTG);
			}
			if(Keyboard->WasPressed(kbs8))
			{
				PKT_C2S_TEST_SpawnDummyReq_s n;
				n.pos = GetPosition() + GetvForw() * 4.0f;
				PxRaycastHit hit;
				PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
				if(g_pPhysicsWorld->raycastSingle(PxVec3(n.pos.x, n.pos.y+2, n.pos.z), PxVec3(0, -1, 0), 4, PxSceneQueryFlag::eIMPACT, hit, filter))
				{
					n.pos.x = hit.impact.x;
					n.pos.y = hit.impact.y;
					n.pos.z = hit.impact.z;
				}

				p2pSendToHost(this, &n, sizeof(n));
			}
			if(Keyboard->WasPressed(kbsC))
			{
				hudMain->showYouGotCoolThing(L"YOU DESERVED IT!", /*L"200 WP", L"bla bla bla\n bla bla\n lalalala",*/ "$Data/Weapons/StoreIcons/ASR_AK108_PSO_Camo.dds");
			}
		}
	}
#endif

	// SPREAD/RECOIL LOGIC
	if(NetworkLocal && m_Weapons[m_SelectedWeapon])
	{
		if(CurrentSpread > 0 && !m_isPressedFireTrigger)
		{
			CurrentSpread -= r3dGetFrameTime()*SpreadDecrease[m_Weapons[m_SelectedWeapon]->getCategory()-storecat_ASR];
		}
		CurrentSpread = R3D_MAX(CurrentSpread, SpreadMin[m_Weapons[m_SelectedWeapon]->getCategory()-storecat_ASR]*m_Weapons[m_SelectedWeapon]->getSpread());

		//r3dOutToLog("1) Recoil=%.2f, cooldown=%.2f\n", RecoilViewModTarget.y, RecoilCooldown.get());
		// update recoil
		if(!RecoilViewMod.AlmostEqual((RecoilViewModTarget), 0.01f))
		{
			RecoilViewMod = R3D_LERP(RecoilViewMod, RecoilViewModTarget, R3D_MIN(r3dGetFrameTime()*GPP->c_fRecoilCooldownSpeed*4.0f * u_GetRandom(0.75f, 1.25f), 1.0f)); // lerp to make it smooth, less jerky
		}
		RecoilViewMod = R3D_LERP(RecoilViewMod, (RecoilViewMod+RecoilViewModTarget2), r3dGetFrameTime()*2.0f);

		if(RecoilCooldown>0)
			RecoilCooldown = RecoilCooldown-r3dGetFrameTime();

		if(RecoilCooldown<=0)
		{
			// slowly make recoil go back to zero
			RecoilViewModTarget = R3D_LERP(RecoilViewModTarget, r3dVector(0,0,0), R3D_MIN(r3dGetFrameTime()*GPP->c_fRecoilCooldownSpeed*8.0f, 1.0f));
		}
		// recoil effect when you are hit
		RecoilViewModTarget2 = R3D_LERP(RecoilViewModTarget2, r3dVector(0,0,0), R3D_MIN(r3dGetFrameTime()*GPP->c_fRecoilCooldownSpeed*2.0f, 1.0f));

		//r3dOutToLog("2) Recoil=%.2f, cooldown=%.2f\n", RecoilViewModTarget.y, RecoilCooldown.get());
	}
	// FINISHED SPREAD/RECOIL LOGIC

	// if mouse is visible - then we are in control menu mode, so DO NOT update player weapon (shooting) or player movement
	// update weapon selection
	if(NetworkLocal && !Mouse->GetMouseVisibility() && !uavViewActive_ && !laserViewActive_ && !bDead)
	{
		R3DPROFILE_FUNCTION("WeaponSelection");
		int prevAnimType = m_Weapons[m_SelectedWeapon]->getAnimType();
		int prevSelectedItem = m_SelectedWeapon;
		
		{
			int buttonPressed = 0;
			if(!(hudMain && hudMain->isChatVisible()) && m_QuickThrowGrenade==0 && !(hudCommCalls && hudCommCalls->isVisible()) 
				&& uberAnim_->bombPlantingTrackID == CUberAnim::INVALID_TRACK_ID && PlayerState != PLAYER_MOVE_SPRINT )
			{
				if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_PRIMARY_WEAPON))
					buttonPressed = 1;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_SECONDARY_WEAPON))
					buttonPressed = 2;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_HANDGUN_WEAPON))
					buttonPressed = 3;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_ITEM1))
					buttonPressed = 4;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_ITEM2))
					buttonPressed = 5;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_ITEM3))
					buttonPressed = 6;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_ITEM4))
					buttonPressed = 7;
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_QUICK_SWITCH_WEAPON))
				{
					if(m_PrevSelectedWeapon == -1)
						buttonPressed = 2;	// switch to second weapon if no previous weapon was selected
					else
						buttonPressed = m_PrevSelectedWeapon+1; // +1 to map from index to button pressed
				}
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_NEXTITEM))
				{
					// scan for 4 item slots and see if we can switch to next item
					int selWeapon = R3D_MAX(m_SelectedWeapon, 2);
					for(int i=0; i<4; i++)
					{
						if(++selWeapon >= NUM_WEAPONS_ON_PLAYER)
							selWeapon = 3;
						if(m_Weapons[selWeapon] != NULL && !isWeaponGrenadeOutofAmmo(selWeapon))
						{
							buttonPressed = selWeapon + 1;
							break;
						}
					}
				}
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_TOGGLE_NIGHTVISION))
				{
					// check if player has night vision goggles
					if(
						// helmets
						CurLoadout.isEquipped(20066) || CurLoadout.isEquipped(20067) 
						// heroes
						|| CurLoadout.isEquipped(20104) || CurLoadout.isEquipped(20105) || CurLoadout.isEquipped(20106) || CurLoadout.isEquipped(20107)
						|| CurLoadout.isEquipped(20164) || CurLoadout.isEquipped(20165) || CurLoadout.isEquipped(20166) || CurLoadout.isEquipped(20167) || CurLoadout.isEquipped(20168)
						)
						r_hud_filter_mode->SetInt(r_hud_filter_mode->GetInt() == HUDFilter_NightVision ? HUDFilter_Default : HUDFilter_NightVision);
				}
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_DROP_BOMB))
				{
					if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
					{
						if(hasSabotageBomb() && bOnGround && m_bombDropCooldown <= 0)
						{
							PKT_C2S_Bomb_RequestDrop_s n;
							p2pSendToHost(this, &n, sizeof(n));
							m_bombDropCooldown = 3.0f;
						}
					}
				}
				else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_THROW_GRENADE))
				{
					// check if player has grenades
					for(int i=3; i<NUM_WEAPONS_ON_PLAYER; i++)
					{
						if(m_Weapons[i] && m_Weapons[i]->getNumBulletsLeft() > 0 && strcmp(m_Weapons[i]->getConfig()->m_PrimaryAmmo->getBulletClass(), "obj_Grenade")==0)
						{
							buttonPressed = i+1; // switch to grenade
							m_QuickThrowGrenade = 2; // counter, on 2 it'll throw grenade. after that, it'll switch to previous item
							break;
						}
					}
				}
				else
				{
					// query mouse scroll
					int mouseScroll = 0;
					Mouse->QueryWheelMotion(&mouseScroll);
					if(Gamepad->WasPressed(gpY))
						mouseScroll = 1;

					if(mouseScroll!=0)
					{
						int selWeapon = m_SelectedWeapon;
						if(mouseScroll>0)
						{
							selWeapon++;
							while(1)
							{
								if(selWeapon>=NUM_WEAPONS_ON_PLAYER)
									selWeapon = 0;
								if(m_Weapons[selWeapon] == NULL || isWeaponGrenadeOutofAmmo(selWeapon))
								{
									selWeapon++;
									continue;
								}
								break;
							}
						}
						else if(mouseScroll<0)
						{
							selWeapon--;
							while(1)
							{
								if(selWeapon<0)
									selWeapon = NUM_WEAPONS_ON_PLAYER-1;
								if(m_Weapons[selWeapon] == NULL || isWeaponGrenadeOutofAmmo(selWeapon))
								{
									selWeapon--;
									continue;
								}
								break;
							}
						}
						// convert to buttonPressed
						buttonPressed = selWeapon+1;
					}
				}
			} 
			else if ( PlayerState == PLAYER_MOVE_SPRINT ) 
			{
				// clear the mouse wheel. 
				int mouseScroll = 0;
				Mouse->QueryWheelMotion(&mouseScroll);
			}

			
			// disable switch if we're cooking grenade or it wasn't launched yet
			int grState = uberAnim_->GetGrenadeAnimState();
			if(!(grState == 1 || (grState == 2 && m_Weapons[m_SelectedWeapon]->m_needDelayedAction)))
			{
				// check for empty grenades, and if empty - switch to primary weapon
				if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->isGrenadeOrMineAnim())
				{
					if( isWeaponGrenadeOutofAmmo(m_SelectedWeapon) )
					{
						m_QuickThrowGrenade = 0;
						buttonPressed = m_PrevSelectedWeapon+1;
						// need additional check
						if( buttonPressed > 0 && isWeaponGrenadeOutofAmmo(buttonPressed - 1) )
							buttonPressed = 1;
					}
					if(m_QuickThrowGrenade > 0 && buttonPressed == 0)
					{
						m_QuickThrowGrenade = 0;
						buttonPressed = m_PrevSelectedWeapon+1;
						m_Weapons[m_SelectedWeapon]->isReadyToFire(false, false); // workaround
					}
				}

				r3d_assert(buttonPressed>=0 && buttonPressed <=NUM_WEAPONS_ON_PLAYER);
				if(buttonPressed && m_Weapons[buttonPressed-1])
				{
					if( isWeaponGrenadeOutofAmmo(buttonPressed - 1) )
					{
						// do nothing, no grenades - so do not switch
					}
					else if(m_Weapons[buttonPressed-1]->isUsableItem() && InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE))
					{
						// do not allow switching to usable item while fire key is pressed
					}
					else
					{
						if(ChangeWeaponByIndex(buttonPressed-1))
						{
							lastTimeWeaponSwitch = curTime;
							if(hudMain)
							{
								hudMain->setCurrentWeapon(buttonPressed-1);
								if(m_Weapons[m_SelectedWeapon])
								{
									hudMain->showUseItem(false);
									hudMain->setFireMode(m_Weapons[m_SelectedWeapon]->getFiremode());
								}
								else
								{
									hudMain->showUseItem(true);
								}
							}
						}
						else if(buttonPressed >= 4 && buttonPressed <=7) // sergey's request: already was pressed and this is an item, simulate left click!
						{
							if(m_Weapons[buttonPressed-1]->isUsableItem())
								m_ItemSimulateLeftClick = true;
						}
					}

				}
			}
		}
		r3d_assert(m_Weapons[m_SelectedWeapon]);
		if(prevAnimType != m_Weapons[m_SelectedWeapon]->getAnimType())
			SyncAnimation(true);
		
		if(prevSelectedItem != m_SelectedWeapon)
		{
			uberAnim_->StopGrenadeAnimations(); // cancel the mine placement. 
			PKT_C2C_PlayerSwitchWeapon_s n;
			n.wid       = m_SelectedWeapon;
			p2pSendToHost(this, &n, sizeof(n), true);
		}
	}

	const float lastFrameTime = r3dGetFrameTime();

	// WARNING: current model is rotated 180 around y
	D3DXMATRIX mr;
	D3DXMatrixRotationY(&mr, R3D_PI);
	D3DXMATRIX CharDrawMatrix = mr;

	if(NetworkLocal && !Mouse->GetMouseVisibility()) 
	{
		if(m_HitMarkerFadeout > 0)
			m_HitMarkerFadeout -= lastFrameTime * GPP->c_fHitMarkerFadeoutSpeed;

		UpdateTargetting();
	}

	// detect if UAV is still here (it can be killed)
	if(NetworkLocal)
	{
		if(uavId_ != invalidGameObjectID)
		{
			if(GameWorld().GetObject(uavId_) == NULL)
			{
				//OnUAVKill();
				if(hudMain)
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_UAVKilled"));
				r_hud_filter_mode->SetInt(HUDFilter_Default);

				uavId_ = invalidGameObjectID;
				uavViewActive_ = false;
				uavRequested_  = 3;
			}
		}
	}

	plr_local_moving_speed = (GetPosition() - oldstate.Position).Length();
	if(NetworkLocal) 
	{
		// moved to separate function and called from UpdatePlayerMovement, to make sure that player position and camera position are in sync
	} 
	else if(GetVelocity().LengthSq() > 0.0001f)
	{
		R3DPROFILE_FUNCTION("NetPlayer_Move");
		r3dPoint3D prevPos = GetPosition();
		SetPosition(GetPosition() + GetVelocity() * lastFrameTime);

		DetectIfOnGround();

		// check if we overmoved to target position
		r3dPoint3D v = netMover.GetNetPos() - GetPosition();
		float d = GetVelocity().Dot(v);
		if(d < 0) {
			SetPosition(netMover.GetNetPos());
			SetVelocity(r3dPoint3D(0, 0, 0));
		}

		float ll = (prevPos-GetPosition()).LengthSq();
		if(ll > 0.0001f)
		{
			extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
			WaterSplash(GetPosition(), 2.0f, 5.0f, 0.1f*ll, 3);
		}
	}

	if(afterRespawnTimer>0)
		afterRespawnTimer -= lastFrameTime;
	//r3dOutToLog("Player Update: %s to %.2f, %.2f, %.2f\n", UserName, GetPosition().x, GetPosition().y, GetPosition().z);

	// update footsteps after moving player
	UpdateFootsteps();
	// update camo only if player has special items
	if(CurLoadout.BodyMeshID==20073 || CurLoadout.BodyArmorID==20074 || CurLoadout.BodyArmorID==20075)
		gCamouflageDataManager.UpdateCamouflageData(*this);


	// even if dead, still need to send pos, otherwise if died in the air - you will still be in the air.
	if(NetworkLocal) 
	{
		CNetCellMover::moveData_s md;
		md.pos       = GetPosition();
		md.turnAngle = m_fPlayerRotationTarget;
		md.bendAngle = bodyAdjust_y[1];
		md.state     = (PlayerState&0xF) | ((PlayerMoveDir&0xF) << 4);
		netMover.SendPosUpdate(md);
	}
	
	//
	// send hash of currently equipped weapon configs
	//
	if(NetworkLocal && !bDead && curTime >= (lastWeapDataRep + PKT_C2S_PlayerWeapDataRep_s::REPORT_PERIOD))
	{
		r3d_assert(!bDead);
		lastWeapDataRep = curTime;
		
		PKT_C2S_PlayerWeapDataRep_s n;
		r3d_assert(NUM_WEAPONS_ON_PLAYER == R3D_ARRAYSIZE(n.weaponsDataHash));
		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; i++)
		{
			n.weaponsDataHash[i] = 0;
			if(m_Weapons[i] == NULL)
				continue;
			n.weaponsDataHash[i] = m_Weapons[i]->getConfig()->GetClientParametersHash();

			n.debug_wid[i] = m_Weapons[i]->getItemID();
			m_Weapons[i]->getConfig()->copyParametersTo(n.debug_winfo[i]);
		}
		
		p2pSendToHost(this, &n, sizeof(n), true);
	}

	// TARGET ICON
	if(m_targetIconTime > 0 && m_MarkedTargetIcon.IsUndefined())
	{
		hudMain->addTargetIcon(m_MarkedTargetIcon);
	}
	else if(!m_MarkedTargetIcon.IsUndefined() && m_targetIconTime <= 0)
	{
		hudMain->deleteScreenIcon(m_MarkedTargetIcon);
	}
	if(m_targetIconTime > 0)
	{
		m_targetIconTime -= lastFrameTime;
	}
	if( m_localTargetIconTime > 0 )
	{
		m_localTargetIconTime -= lastFrameTime;
	}
	if(!m_MarkedTargetIcon.IsUndefined())
		hudMain->moveScreenIcon(m_MarkedTargetIcon, GetPosition()+r3dPoint3D(0,1,0), true);

	//// PLAYER TAG ///////
	if(!NetworkLocal && m_TagIcon.IsUndefined() && (CGL.localPlayer_ || CGL.m_isSpectator))
	{
		char* team = "";
		if(CGL.localPlayer_)
			team = CGL.localPlayer_->TeamID == TeamID?"blue":"red";
		else
			team = TeamID==1?"blue":"red";

        char plrUserName[256]; GetUserNameAndClanTag(plrUserName);
		hudMain->addPlayerTagIcon(plrUserName, team, m_TagIcon);
	}

	bool spawnProtected = r3dGetTime() < m_SpawnProtectedUntil;
	if(!m_TagIcon.IsUndefined())
	{
		R3DPROFILE_FUNCTION("moveScreenIcon");
		r3dPoint3D scrCoord;
		r3dProjectToScreen(GetPosition()+r3dPoint3D(0,2.0f,0), &scrCoord);

		r3dPoint3D dir;
		if(m_isInScope || g_camera_mode->GetInt() != 1)
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
		else
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);

		bool showTag = false;
		bool showName = false;
		bool alwaysShow = false;
		// if same team, or withing 10meters, or aim is on player
		if(CGL.localPlayer_)
		{
			float dist = (CGL.localPlayer_->GetPosition()-GetPosition()).Length();
			if(CGL.localPlayer_->bDead) // if local player is dead, use camera instead of player position
				dist = (gCam - GetPosition()).Length();

			if((CGL.localPlayer_->TeamID == TeamID) ||
				(dist < 10.0f) ||
				this->GetSafeID() == m_LocalPlayer_CurrentAimAt ||
				(uavId_ == m_LocalPlayer_CurrentAimAt && GameWorld().GetObject(uavId_))
				)
			{
				showTag = true;
			}

			
			if( (CGL.localPlayer_->TeamID == TeamID && dist < 20.0f) ||
				(this->GetSafeID() == m_LocalPlayer_CurrentAimAt) ||
				(uavId_ == m_LocalPlayer_CurrentAimAt && GameWorld().GetObject(uavId_))
				)
			{
				showName = true;
			}

			if(CGL.localPlayer_->TeamID != TeamID)
			{
				// disable enemy tag if behind obstacle and not targetted
				if(showTag && this->GetSafeID() != m_LocalPlayer_CurrentAimAt)
				{
					r3dPoint3D viewTrg = GetPosition();
					viewTrg.y = GetMuzzlerPosition().y;
					r3dPoint3D viewVec = (viewTrg - gCam);
					float rayLen = viewVec.Length();
					viewVec.Normalize();

					// check if we hit any static geometry shape on our view from camera to this player
					PxRaycastHit hit;
					PhysicsCallbackObject* target = NULL;
					PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
					if(g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(viewVec.x, viewVec.y, viewVec.z), rayLen, PxSceneQueryFlag::eIMPACT, hit, filter))
						showTag = false;
				}

				// if detected, show triangle on top
				if(/*m_onMinimap ||*/ !m_MarkedTargetIcon.IsUndefined())
					showTag = true;

				// if inside death zone, always show his icon (todo: optimize this, as it might be slow)
				R3DPROFILE_START( "Damage Area" ) ;

				const obj_DamageArea::Arr& damageAreas = obj_DamageArea::GetDamageAreaArr() ;
				for( int i = 0, e = damageAreas.Count() ; i < e; i ++ )
				{
					obj_DamageArea* damageArea = (obj_DamageArea*)damageAreas[ i ];
					if(((GetPosition() - damageArea->GetPosition()).LengthSq() < (damageArea->GetRadius()*damageArea->GetRadius())) && damageArea->getTeamID() == TeamID)
					{
						showTag = true;
						alwaysShow = true;
						break;
					}
				}

				R3DPROFILE_END( "Damage Area" ) ;
			}

			// if local player is dead, always show tags, so that when you are in dead mode, you can see who is who
			if(CGL.localPlayer_->bDead)
			{
				showTag = CGL.localPlayer_->TeamID == TeamID;
			}
		}
		else if(CGL.m_isSpectator)
		{
			extern int spectator_observingPlrIdx;
			extern r3dPoint3D spectator_cameraPos;
			if(CGL.GetPlayer(spectator_observingPlrIdx) == this || (spectator_cameraPos-GetPosition()).Length() < 25.0f)
			{
				showTag = true;
				showName = true;
			}
		}
		if(bDead)
			showTag = false;

#ifndef FINAL_BUILD
		if(d_observer_mode->GetBool())
		{
			showTag = true;
			showName = true;
		}
#endif

		hudMain->showPlayerNameTag(m_TagIcon, showName);

		if ( CGL.localPlayer_ && CGL.localPlayer_->TeamID != TeamID )
		{
			if(!m_SpawnProtectIcon.IsUndefined() && showName && spawnProtected )
			{
				hudMain->moveScreenIcon(m_SpawnProtectIcon, GetPosition()+r3dPoint3D(0,2.5,0), false);
			} 
			else 
			{
				hudMain->moveScreenIcon(m_SpawnProtectIcon, r3dPoint3D(-1, -1, -1), false, true);
			}
		}

		if(uavId_ == m_LocalPlayer_CurrentAimAt && GameWorld().GetObject(uavId_))
		{			
			GameObject* uav = GameWorld().GetObject(uavId_);
			obj_AI_Player* uavOwner = (obj_AI_Player*)GameWorld().GetObject(uav->ownerID);
			if(!uavOwner || (uavOwner && uavOwner->TeamID == TeamID) ) // uav handles friendly tag itself
			{
				alwaysShow = false;
				showTag = false;
			}
			hudMain->moveScreenIcon(m_TagIcon, uav->GetPosition()+r3dPoint3D(0,2.0f,0), alwaysShow, !showTag);
		}
		else
			hudMain->moveScreenIcon(m_TagIcon, GetPosition()+r3dPoint3D(0,2.0f,0), alwaysShow, !showTag);
	}

	// guardian angel
	if(!NetworkLocal)
	{
		if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		{	
			if(CGL.localPlayer_ )
			{
				if(CGL.localPlayer_->TeamID == TeamID)
				{
					if(CGL.localPlayer_->CurLoadout.hasItem(AbilityConfig::AB_GuardianAngel)) // if local player has guardian angel ability, show injured team mates
					{
						if((GetPosition() - CGL.localPlayer_->GetPosition()).Length() < 50.0f && m_Health < getMaxHealth() && !bDead)
							hudMain->moveScreenIcon(m_WoundedTeamIcon, GetPosition()+r3dPoint3D(0,3,0), false);
						else
							hudMain->moveScreenIcon(m_WoundedTeamIcon, GetPosition()+r3dPoint3D(0,3,0), false, true);
					}
					if(CurLoadout.hasItem(AbilityConfig::AB_GuardianAngel)) // if player has guardina angel ability - show medic icon
					{
						if(!m_MedicTeamIcon.IsUndefined())
						{
							if((GetPosition() - CGL.localPlayer_->GetPosition()).Length() < 50.0f)
								hudMain->moveScreenIcon(m_MedicTeamIcon, GetPosition()+r3dPoint3D(0,3,0), false);
							else
								hudMain->moveScreenIcon(m_MedicTeamIcon, GetPosition()+r3dPoint3D(0,3,0), false, true);
						}
					}
				} 
			}
		}
	}

	// ping icon
	if(!NetworkLocal)
	{
		if(m_pingIconTimer>0)
		{
			m_pingIconTimer -= lastFrameTime;
			if(m_pingIconTimer <=0)
			{
				m_pingIconTimer = -1;
				hudMain->moveScreenIcon(m_PingTeamIcon, GetPosition()+r3dPoint3D(0,2,0), false, true); // hide icon
			}
			else
			{
				hudMain->moveScreenIcon(m_PingTeamIcon, GetPosition()+r3dPoint3D(0,2,0), true);
			}
		}
	}
	// comm rose icon
	if(!NetworkLocal)
	{
		if(m_commRoseTimer>0)
		{
			m_commRoseTimer -= lastFrameTime;
			if(m_commRoseTimer <=0)
			{
				m_commRoseTimer = -1;
				hudMain->deleteScreenIcon(m_CommRoseIcon);
			}
			else
			{
				hudMain->moveScreenIcon(m_CommRoseIcon, GetPosition()+r3dPoint3D(0,2,0), true);
			}
		}
	}

	// check for dead after moving player, otherwise might die in the air and be stuck in the air (dead body)
	if(bDead)
	{
		if(NetworkLocal)
			s_damageHitColorCorrectionVal = 0;

		UpdateAnimations(CharDrawMatrix);

		if(m_Weapons[m_SelectedWeapon])
			m_Weapons[m_SelectedWeapon]->Update(uberEquip_->getWeaponBone(GetSkeleton(), DrawFullMatrix));

		return TRUE;
	}

	SyncAnimation();

	UpdateRotation();

	UpdateAnimations(CharDrawMatrix);

	if(m_Weapons[m_SelectedWeapon])
	{
		m_Weapons[m_SelectedWeapon]->Update(uberEquip_->getWeaponBone(GetSkeleton(), DrawFullMatrix));
	}

	// simple regeneration
	bool guardingAngelHealing = false;
	if(m_Health < getMaxHealth())
	{
		// THIS CODE HAS TO BE IN SYNC WITH THE SAME CODE ON SERVER
		if((curTime-lastTimeHit) > 5.0f || m_BerserkerTime > 0) // no regeneration for 5 sec after someone hit you
		{
			float regenSpeed = GPP->c_fRegenSpeed;

			/* - disabled 2012-05-23 by Syncopate request. NOTE: must be synched with SERVER
			// disable regen on sabotage on russian server
			extern int RUS_CLIENT;
			if(RUS_CLIENT && CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
				regenSpeed = 0.0f;
			*/

			int HealthRegenSkill = CurLoadout.getSkillLevel(CUserSkills::MEDIC_HealthRegeneration);
			switch(HealthRegenSkill)
			{
			case 1: regenSpeed *= 1.05f; break;
			case 2: regenSpeed *= 1.10f; break;
			case 3: regenSpeed *= 1.15f; break;
			case 4: regenSpeed *= 1.20f; break;
			case 5: regenSpeed *= 1.25f; break;
			default:break;
			}
			if(m_BerserkerTime > 0)
			{
				int berserkerRage = CurLoadout.getSkillLevel(CUserSkills::ASSAULT_BerserkersRage);
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

			m_Health = m_Health + lastFrameTime*regenSpeed; //X hit points in one second
			// guardian angel
			float guardianAngelBoostRadius = 0.0f;
			int GuardianAngelSkill = CurLoadout.getSkillLevel(CUserSkills::MEDIC_GuardianAngel);
			switch(GuardianAngelSkill)
			{
			case 1: guardianAngelBoostRadius = 1.0f; break;
			case 2: guardianAngelBoostRadius = 2.0f; break;
			case 3: guardianAngelBoostRadius = 3.0f; break;
			case 4: guardianAngelBoostRadius = 4.0f; break;
			default:break;
			}
			if(CGL.CheckTeamAbilityAvailable(this, AbilityConfig::AB_GuardianAngel, 10.0f+guardianAngelBoostRadius))
			{
				if(lastTimeHit_OldHealth < 70) // show guardian angel heal only if you were damaged substantially, otherwise the effect will show up just for a second, which makes it look like a random green effect
					guardingAngelHealing = true;
				float healthNutBoost = 1.0f;
				switch(CurLoadout.getSkillLevel(CUserSkills::RECON_HealthNut))
				{
				case 1: healthNutBoost = 1.1f; break;
				case 2: healthNutBoost = 1.2f; break;
				case 3: healthNutBoost = 1.3f; break;
				}
				m_Health = m_Health + lastFrameTime*(regenSpeed*4.0f)*healthNutBoost; // 20 points per second
			}

			m_Health = R3D_MIN((float)m_Health, getMaxHealth());
		}
	}

	if(NetworkLocal)
	{
		if(hudMain)
			hudMain->setHealth(int((m_Health/getMaxHealth())*100.0f));
	}

	if(NetworkLocal)
	{
		// breath timer
		/*if(m_BreathPenaltyTime > 0)
		{
			m_BreathPenaltyTime = m_BreathPenaltyTime - r3dGetFrameTime();
			if(m_BreathPenaltyTime < 0)
				m_BreathPenaltyTime = 0;
		}
		else*/
		{
			if(!InputMappingMngr->isPressed(r3dInputMappingMngr::KS_HOLD_BREATH))
			{
				if(m_Breath < getMaxBreath())
					m_Breath = m_Breath + lastFrameTime;
			}
		}
	}

	{
		if(guardingAngelHealing && m_AuraType == AT_NONE)
			m_AuraType = AT_HEALING;
		else if(guardingAngelHealing==false && m_AuraType == AT_HEALING)
		{
			// so that debug works in editor correctly
			if( !g_bEditMode )
			{
				m_AuraType = AT_NONE;
			}
		}

		if(m_BerserkerTime > 0)
		{
			m_BerserkerTime -= lastFrameTime;
			if(m_BerserkerTime <= 0)
				m_AuraType = AT_NONE;
		}
		if(r3dGetTime() < m_MorhineShotTime && m_AuraType == AT_NONE)
			m_AuraType = AT_MORHINE;
		if(m_AuraType == AT_MORHINE && r3dGetTime() > m_MorhineShotTime)
			m_AuraType = AT_NONE;
		if(m_AuraType == AT_NONE && spawnProtected)
			m_AuraType = AT_SPAWNPROTECTION;
		if(m_AuraType == AT_SPAWNPROTECTION && !spawnProtected)
			m_AuraType = AT_NONE;
	}

	if(NetworkLocal)
	{
		extern r3dVector g_vBattleZonePlayerPos;
		g_vBattleZonePlayerPos = GetPosition();

		if(BloodEffect > 0)
		{
			BloodEffect -= lastFrameTime;
			if(BloodEffect < 0) BloodEffect = 0;
		}
	}
	
	// update fire
	if(NetworkLocal && !Mouse->GetMouseVisibility() && !uavViewActive_ && !laserViewActive_)
	{
		if(fabs(m_ReticleCurrentScale - m_ReticleTargetScale)>0.01f)
			m_ReticleCurrentScale = R3D_LERP(m_ReticleCurrentScale, m_ReticleTargetScale, lastFrameTime * 5.0f);

		bool scopeRendering = (hasScopeMode() && CurrentRig.allowScope && m_isAiming) || (g_camera_mode->GetInt()==2 && m_isAiming);
		if(hudMain)
		{
			hudMain->showReticle(!scopeRendering && m_Weapons[m_SelectedWeapon] && !m_Weapons[m_SelectedWeapon]->isReloading() && PlayerState!=PLAYER_MOVE_SPRINT);
			hudMain->setReticleScale(m_ReticleCurrentScale);
		}
	}

	// if low health - start dropping blood every second
	m_BloodTimer += lastFrameTime;
	if(m_Health < 30 && m_BloodTimer > 1.0f)
	{
		r3dPoint3D pos = GetPosition(); pos.y += 2.0f;
		AddBloodOnGround(pos);
		m_BloodTimer = 0;
	}

	{
		R3DPROFILE_FUNCTION("update hud");
		if(NetworkLocal)
		{
			if(m_bombDropCooldown > 0)
				m_bombDropCooldown -= lastFrameTime;
			// update hud
			{
				const Weapon* wpn = m_Weapons[m_SelectedWeapon];
				if(wpn && !wpn->isUsableItem() )
				{
					if(hudMain) hudMain->SetBullets(m_Weapons[m_SelectedWeapon]->getNumBulletsLeftInClip(), m_Weapons[m_SelectedWeapon]->getNumBulletsInClip(), m_Weapons[m_SelectedWeapon]->getNumClipsLeft());
				}
				else if(wpn && wpn->isUsableItem())
				{
					int numCharges = 1;
					if(wpn->m_usableItemInventoryIdx >= 0)
					{
						numCharges = gUserProfile.ProfileData.Inventory[wpn->m_usableItemInventoryIdx].quantity;
					}
					
					if(wpn->getItemID() == WeaponConfig::ITEMID_Cypher2)
					{
						if(uavRequested_) // destroyed or spawned UAV
							numCharges = 0;
					}
					if(wpn->getItemID() == WeaponConfig::ITEMID_RiotShield && m_spawnedRiotShield)
						numCharges = 0;
					if(wpn->getItemID() == WeaponConfig::ITEMID_AutoTurret && m_spawnedAutoTurret)
						numCharges = 0;
					
					if(hudMain)
						hudMain->SetBullets(numCharges, 0, 0);
				}
				// update items
				if(hudMain)
				{
					for(int i=3; i<7; ++i)
					{
						const Weapon* wpn = m_Weapons[i];
						if(wpn)
						{
							if(wpn->isUsableItem())
							{
								int numCharges = 1;
								if(wpn->m_usableItemInventoryIdx >= 0)
								{
									numCharges = gUserProfile.ProfileData.Inventory[wpn->m_usableItemInventoryIdx].quantity;
								}

								if(wpn->getItemID() == WeaponConfig::ITEMID_Cypher2)
								{
									if(uavRequested_) // destroyed or spawned UAV
										numCharges = 0;
								}
								if(wpn->getItemID() == WeaponConfig::ITEMID_RiotShield && m_spawnedRiotShield)
									numCharges = 0;
								if(wpn->getItemID() == WeaponConfig::ITEMID_AutoTurret && m_spawnedAutoTurret)
									numCharges = 0;
								if(hudMain)
								{
									float cooldown = wpn->getRateOfFire() - (r3dGetTime() - wpn->getLastTimeFired());
									if(cooldown < 0 || numCharges == 0) cooldown = 0;
									hudMain->SetItemInfo(i, cooldown, wpn->getRateOfFire(), numCharges);
								}
							}
							else
							{
								hudMain->SetItemInfo(i, 0, 1, wpn->getNumBulletsLeft());
							}
						}
					}
				}
	
				float bloodLevel = 0;
				if(m_Health < 80)
					bloodLevel = 100.0f - ((m_Health/80.0f)*100.0f); // show blood effect when health is between 0 and 80
				s_damageHitColorCorrectionVal = (bloodLevel/100.0f);
				if(hudMain) hudMain->SetBloodLevel(bloodLevel);

				if(CGL.localPlayer_ && !(hudMain && hudMain->isChatVisible()))
				{
					// THE FIXER ability
					if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
					{
						if(CurLoadout.hasItem(AbilityConfig::AB_TheFixer))
						{
							static bool showMessage = false;
							GameObject* obj = GameWorld().GetObject(m_LocalPlayer_CurrentAimAt);
							if(obj && obj->isObjType(OBJTYPE_Mine) && (GetPosition()-obj->GetPosition()).Length() < 2.0f) // should be within 2meters to switch mine
							{
								obj_Mine* mine = (obj_Mine*)obj;
								GameObject* owner = GameWorld().GetObject(mine->ownerID);
								if(owner && owner->isObjType(OBJTYPE_Human))
								{
									obj_AI_Player* ownPl = (obj_AI_Player*)owner;
									if(ownPl->TeamID != TeamID)
									{
										// ok, finally here. Show player an option to switch mine
										if(!showMessage)
										{
											showMessage = true;
											hudMain->showActionMessage("fixer", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
										}
										if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_INTERACT) || Gamepad->WasReleased(gpRightShoulder))
										{
											// send request to switch mine
											PKT_C2C_PlayerSwitchMine_s n;
											n.mineID = toP2pNetId(mine->NetworkID);
											p2pSendToHost(this, &n, sizeof(n));
										}
									}
									else
										showMessage = false;
								}
								else 
									showMessage = false;
							}
							else
								showMessage = false;
						}
					}
					// WEAPON PICKUP LOGIC
					//if(g_camera_mode->GetInt()!=2) // disabled in FPS for now, until fixed
					{
						// slow, need to rewrite
						DroppedWeapon* dropWpn = 0;
						float distance = 999999999.0f;

						ObjectManager& GW = GameWorld();
						for(GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
						{
							if(obj->isObjType(OBJTYPE_WeaponDrop))
							{
								float d = (obj->GetPosition() - GetPosition()).LengthSq();
								if(!((DroppedWeapon*)obj)->m_IsPicked && distance > d && ((DroppedWeapon*)obj)->m_isGood)
								{
									dropWpn = (DroppedWeapon*)obj;
									distance = d;
								}
							}
						}
						float maxDist = PKT_C2S_RequestWeaponPickup_s::PICKUP_RADIUS*0.5f;
						static float pressedKeyTimer = 0.0f;
						static bool movedOff = true; // have we moved off the object? 
						const static float maxTimeForDisplay = 2.0f;
						static float timeForDisplay = 2.0f;
						if(distance < (maxDist*maxDist))
						{
							
							if(!shownWeaponPickup )
							{
								if ( movedOff )
								{
									timeForDisplay = maxTimeForDisplay;
									movedOff = false;
									shownWeaponPickup = true;
									const WeaponConfig* config = gWeaponArmory.getWeaponConfig(dropWpn->m_WeaponItemID);
									int damage   = 0;
									int accuracy = 0;
									int recoil = 0;
									int firerate = 0;
									getWeaponParamForUI(config, &damage, &accuracy, &firerate, &recoil);

									const WeaponConfig* currentConfig = 0;
									if(config->category == storecat_ASR || config->category == storecat_SNP || config->category == storecat_MG)
									{
										if(m_Weapons[0])
											currentConfig = m_Weapons[0]->getConfig();
									}
									else if(config->category == storecat_SUPPORT || config->category == storecat_SHTG || config->category == storecat_SMG)
									{
										if(m_Weapons[1])
											currentConfig = m_Weapons[1]->getConfig();
									}
									else if(config->category == storecat_HG)
									{
										if(m_Weapons[2])
											currentConfig = m_Weapons[2]->getConfig();
									}
									int mydamage   = 1;
									int myaccuracy = 1;
									int myrecoil = 1;
									int myfirerate = 1;
									if(currentConfig)
										getWeaponParamForUI(currentConfig, &mydamage, &myaccuracy, &myfirerate, &myrecoil);


									hudMain->showPickupMsg(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT),
										config->m_StoreName, config->m_StoreIcon,
										gLangMngr.getString("$StatDamageCAP"), mydamage, damage - mydamage,
										gLangMngr.getString("$StatAccuracyCAP"), myaccuracy, accuracy - myaccuracy,
										gLangMngr.getString("$StatRecoilCAP"), myrecoil, recoil - myrecoil,
										gLangMngr.getString("$StatFireRateCAP"), myfirerate, firerate - myfirerate,
										gLangMngr.getString("$StatCapacityCAP"), dropWpn->m_NumBullets);
								
								}
							}
							else 
							{
								if( timeForDisplay > 0 ) 
								{
									timeForDisplay-= r3dGetFrameTime();
									if( timeForDisplay <= 0 ) 
									{
										timeForDisplay = 0;
										hudMain->hidePickupMsg();
										hudMain->hideActionMessage( "pickup" );	
										shownWeaponPickup = false;
									}
								}
							}

							if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_INTERACT) || Gamepad->IsPressed(gpRightShoulder))
							{
								pressedKeyTimer += lastFrameTime;
								// send request to pickup weapon
								if(pressedKeyTimer > 1.0f) 
								{
									dropWpn->m_IsPicked = true;

									pressedKeyTimer = 0.0f;
									PKT_C2S_RequestWeaponPickup_s n;
									n.spawnID = toP2pNetId(dropWpn->NetworkID);
									p2pSendToHost(this, &n, sizeof(n));
								}
							}
							else
								pressedKeyTimer = 0.0f;
						}
						else
						{
							if(shownWeaponPickup)
							{
								hudMain->hidePickupMsg();
								hudMain->hideActionMessage( "pickup" );	
							}
							shownWeaponPickup = false;
							movedOff = true;
							pressedKeyTimer = 0.0f;
						}
					}
					
					// SUPPLY CRATES logic
					static bool shownCrateMessage = false; // it is a local code only, so static here should be safe
					if(obj_SupplyCrate* crate = g_SupplyCrateMngr.getCrateInRadius(GetPosition(), 3.0f))
					{
						if(!shownCrateMessage)
						{
							shownCrateMessage = true;
							hudMain->showActionMessage(crate->SupplyType==0?"resupply":"blackops", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
						}
						if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_INTERACT) || Gamepad->WasReleased(gpRightShoulder))
						{
							// send request for resupply
							PKT_C2S_ResupplyRequest_s n;
							p2pSendToHost(this, &n, sizeof(n));
						}
					}
					else
						shownCrateMessage = false;

					// Siege objective logic
					if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
					{
						static bool shownSiegeMessage = false; // it is a local code only, so static here should be safe
						obj_SiegeObjective* sobj = gSiegeObjMgr.GetActiveObjective();
						if(sobj && (sobj->GetPosition() - GetPosition()).Length() < 2.0f && ((CGL.localPlayer_->TeamID == 0 && sobj->Status == obj_SiegeObjective::SS_ACTIVE) || (CGL.localPlayer_->TeamID == 1 && sobj->Status == obj_SiegeObjective::SS_ARMED))) // within 1 meter and not armed already
						{
							if(!shownSiegeMessage)
							{
								shownSiegeMessage = true;
								if(sobj->Status == obj_SiegeObjective::SS_ACTIVE)
									hudMain->showActionMessage("armBomb", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
								else
									hudMain->showActionMessage("disarmBomb", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
							}
							if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_INTERACT))
							{
								if(m_siegeArmingTimer == 0)
									hudMain->showReloading(true);
								m_siegeArmingTimer += lastFrameTime;
								hudMain->SetReloadingProgress((1.0f-(sobj->m_ActivationTimer-m_siegeArmingTimer)/sobj->m_ActivationTimer)*100.0f);
								// for now, just sent a packet when we think we finished arming bomb, later, send two packets, when started arming and when done to prevent cheating
								if(m_siegeArmingTimer > sobj->m_ActivationTimer)
								{
									PKT_C2S_Siege_Activate_s n;
									n.ObjectiveID = toP2pNetId(sobj->NetworkID);
									p2pSendToHost(this, &n, sizeof(n));
								}
							}
							else
							{
								if(m_siegeArmingTimer > 0)
									hudMain->showReloading(false);
								m_siegeArmingTimer = 0;
							}
						}
						else
						{
							shownSiegeMessage = false;
							if(m_siegeArmingTimer > 0)
								hudMain->showReloading(false);
							m_siegeArmingTimer = 0;
						}
					}
					// BOMB CYBERSPORT LOGIC
					if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
					{
						// check if we are able to pick up dropped bomb
						if(CGL.m_DroppedBomb->isBombDropped() && TeamID == 0 && m_bombDropCooldown <= 0)
						{
							static float lastRequest = 0.0f;
							if((CGL.m_DroppedBomb->GetPosition() - GetPosition()).Length() < 0.5f)
							{
								if((r3dGetTime() - lastRequest) > 2.0f)
								{
									lastRequest = r3dGetTime();
									PKT_C2S_Bomb_RequestBombPickup_s n;
									p2pSendToHost(this, &n, sizeof(n));
								}
							}
						}

						if(!uberAnim_->GetGrenadeAnimState()) // do not allow to arm/disarm bomb if throwing grenade or arming mines.
						{
							static bool shownBombMessage = false; // it is a local code only, so static here should be safe
							static int  playBombAnim = 0;
							bool found = false;
							for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
							{
								obj_BombPlacement* bobj = gBombPlacementMgr.GetBombPlacement(i);
								float actRadius = bobj->getActivationRadius();
								r3dPoint3D bombPos = bobj->GetPosition();
								if(bobj->getStatus() == obj_BombPlacement::SS_ARMED)
								{
									actRadius = 1.0f; // you have to stand on top of bomb to start disarming it
									bombPos = bobj->m_satchelChargePos;
								}
								if((bombPos - GetPosition()).Length() < actRadius && ((CGL.localPlayer_->TeamID == 0 && bobj->getStatus() == obj_BombPlacement::SS_EMPTY && CGL.localPlayer_->hasSabotageBomb()) || (CGL.localPlayer_->TeamID == 1 && bobj->getStatus() == obj_BombPlacement::SS_ARMED))) 
								{
									if(!shownBombMessage)
									{
										shownBombMessage = true;
										if(bobj->getStatus() == obj_BombPlacement::SS_EMPTY)
											hudMain->showActionMessage("armBomb", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
										else
											hudMain->showActionMessage("disarmBomb", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
									}
									if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_INTERACT))
									{
										if(m_siegeArmingTimer == 0)
											hudMain->showBombTimer(true);

										m_siegeArmingTimer += lastFrameTime;
										float activTimer = 0;
										if(TeamID == 0)
											activTimer = bobj->m_ArmingTimer;
										else
											activTimer = bobj->m_DisarmingTimer;

										if(playBombAnim == 0)
										{
											uberAnim_->StartBombPlantingAnimation(0);
											playBombAnim = 1;
										}
										else if(playBombAnim == 1 && m_siegeArmingTimer > 0.4f)
										{
											uberAnim_->StartBombPlantingAnimation(1);
											playBombAnim = 2;
										}
										else if(playBombAnim == 2 && (activTimer-m_siegeArmingTimer) < 0.4f)
										{
											uberAnim_->StartBombPlantingAnimation(2);
											playBombAnim = 3;
										}

										hudMain->setBombTimerProgress((1.0f-(activTimer-m_siegeArmingTimer)/activTimer));
										// for now, just sent a packet when we think we finished arming bomb, later, send two packets, when started arming and when done to prevent cheating
										if(m_siegeArmingTimer > activTimer)
										{
											static float timerAntiSpam = 0.0f;
											if((r3dGetTime() - timerAntiSpam) > 1.0f) // send only once per second
											{
												timerAntiSpam = r3dGetTime();
												PKT_C2S_Bomb_RequestBombPlacement_s n;
												n.bombID = toP2pNetId(bobj->NetworkID);
												n.pos = GetPosition();
												p2pSendToHost(this, &n, sizeof(n));
											}
										}
									}
									else
									{
										if(m_siegeArmingTimer > 0)
										{
											hudMain->showBombTimer(false);
											m_siegeArmingTimer = 0;
											playBombAnim = 0;
											uberAnim_->StartBombPlantingAnimation(3);
										}
									}
									found = true;
									break;
								}
							}
							if(!found)
							{
								shownBombMessage = false;
								if(m_siegeArmingTimer > 0)
								{
									hudMain->showBombTimer(false);
									m_siegeArmingTimer = 0;
									playBombAnim = 0;
									uberAnim_->StartBombPlantingAnimation(3);
								}
							}
						}
					}
				}

			}
			if(hudMinimap && hudMinimap->IsInited())
			{
				hudMinimap->SetCameraPosition(GetPosition(), GetvForw());
			}
		}
		else // Not network local
		{
			if(hudMinimap && hudMinimap->IsInited())
			{
				if(CGL.localPlayer_)
				{
					if(CGL.localPlayer_->TeamID != TeamID)
					{
						// check timer
						if(m_fVisibleTimer > 0)
						{
							m_visibleOnMinimap = true;
							m_fVisibleTimer -= lastFrameTime;
							if(m_fVisibleTimer<=0)
							{
								m_fVisibleTimer = 0;
								m_visibleOnMinimap = false;
							}
						}
						// check distance
						if((CGL.localPlayer_->GetPosition() - GetPosition()).Length() < CGL.localPlayer_->getMinimapDetectionRadius() && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
							m_visibleOnMinimap = true;
						else if(m_fVisibleTimer == 0)
							m_visibleOnMinimap = false;

						if(m_visibleOnMinimap && !m_onMinimap) // need to add
						{
							hudMinimap->AddUnit(m_MinimapTagIconName, true, CGL.localPlayer_->Dead_KillerID == NetworkID, GetPosition());
							m_onMinimap = true;
						}
						if(!m_visibleOnMinimap && m_onMinimap) // remove
						{
							hudMinimap->EraseUnit(m_MinimapTagIconName);
							m_onMinimap = false;
						}
					}
					else
					{
						// check that ally is visible, sometimes during joining game ally might be created before you and so will not be added to minimap automatically
						if(!m_onMinimap)
						{
							hudMinimap->AddUnit(m_MinimapTagIconName, false, false, GetPosition());
							m_onMinimap = true;
						}
					}
				}
				else if(CGL.m_isSpectator)
				{
					if(!m_onMinimap)
					{
						if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // due to legacy shenanigans in sabo teams are switched
							hudMinimap->AddUnit(m_MinimapTagIconName, TeamID==0, false, GetPosition());
						else
							hudMinimap->AddUnit(m_MinimapTagIconName, TeamID==1, false, GetPosition());
						m_onMinimap = true;
					}
				}
				if(m_onMinimap)
				{
					bool visible = true;
					if(CGL.localPlayer_)
						visible = (CGL.localPlayer_->GetPosition()-GetPosition()).Length() < 100.0f;
					hudMinimap->MoveUnit(m_MinimapTagIconName, GetPosition(), visible);		
					float angle = 360.0f + GetRotationVector().x;
					//angle = angle / 360.0f;
					hudMinimap->RotateUnit(m_MinimapTagIconName, angle);
				}
			}
		}
	}

	return TRUE;
}

void obj_AI_Player::ProcessSpawnUAV()
{
	// can spawn UAV only one time per session
	if(uavRequested_ == 1 || uavRequested_ == 2)
	{
		if(hudMain) hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_UAVOnline"));
		return;
	}
	if(uavRequested_ == 3)
	{
		if(hudMain) hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_UAVOffline"));
		return;
	}
	
	// dont spawn UAV if no item
	if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->getItemID() != WeaponConfig::ITEMID_Cypher2) {
		return;
	}

	r3d_assert(!uavRequested_);

	r3dPoint3D pos;
	if(!GetUAVSpawnPos(&pos))
	{
		if(hudMain) hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_NoUAVSpace"));
		return;
	}

	uavRequested_ = 1;

#ifndef FINAL_BUILD
	if(g_bEditMode)
	{
		// spawn test uav in editor
		GameObject* uav = srv_CreateGameObject("obj_UAV", "uav", pos);
		uav->ownerID = GetSafeID();
		uav->NetworkLocal = true;
		uav->NetworkID    = 0x454444;
		uavId_ = uav->GetSafeID();
		return;
	}
#endif

	r3d_assert(NetworkLocal);
	PKT_C2C_PlayerUseItem_s n;
	n.itemId  = WeaponConfig::ITEMID_Cypher2;
	n.pos     = pos;
	n.var1    = m_fPlayerRotationTarget;
	p2pSendToHost(this, &n, sizeof(n), true);
	return;
}

void obj_AI_Player::ToggleUAVView(bool force_disable)
{
	r3d_assert(NetworkLocal);
	if(laserViewActive_)
		return;
	if(hudPause && hudPause->isActive())
		return;

	GameObject* uav = GameWorld().GetObject(uavId_);
	if(uavViewActive_ || force_disable)
	{
		uavViewActive_ = false;
		r_hud_filter_mode->SetInt(HUDFilter_Default);
		if(uav) uav->ObjFlags &= ~OBJFLAG_DisableShadows;
		if(uav) uav->ObjFlags &= ~OBJFLAG_SkipDraw;	// need that
	} 
	else if(uav)
	{
		uavViewActive_ = true;
		r_hud_filter_mode->SetInt(HUDFilter_CameraDrone);
		uav->ObjFlags |= OBJFLAG_DisableShadows;
		uav->ObjFlags |= OBJFLAG_SkipDraw;	// need that, because camera is inside body
	}
}

void obj_AI_Player::ToggleLaserView(bool force_disable)
{
	r3d_assert(NetworkLocal);
	if(force_disable)
		laserViewActive_ = false;
	else
	{
		if(hudPause && hudPause->isActive())
			return;

		if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->getItemID() == WeaponConfig::ITEMID_LLDR) 
		{
			if(!uavViewActive_)
				laserViewActive_ = !laserViewActive_;
		}
		else
			laserViewActive_ = false;
	}
}

bool obj_AI_Player::GetUAVSpawnPos(r3dPoint3D* pos)
{
	r3dPoint3D uavPos = GetPosition() + GetvForw() * 1.0f;
	uavPos.y += 4;

	// loop 100m up to check if we can spawn uav
	for(int i=0; i<100; i++)
	{
		// half sizes from obj_Uav mesh
		float uavSizeX = 4.7f/2;
		float uavBelowSpace = 3.0f;  // needed free meters below UAV
		float uavTopSpace   = 10.0f; // needed free meters above UAV
		
		PxBoxGeometry bbox(uavSizeX, (uavBelowSpace+uavTopSpace)*0.5f, uavSizeX);
		PxTransform pose(PxVec3(uavPos.x, uavPos.y+3.0f, uavPos.z), PxQuat(0,0,0,1));
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));
		PxShape* shape;
		if(!g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, shape, filter))
		{
			*pos = uavPos;
			return true;
		}

		uavPos.y += 1.0f;
	}

	*pos = uavPos;
	return false;
}

float dist_Point_to_Line( const r3dPoint3D& p, const r3dPoint3D& l1, const r3dPoint3D& l2)
{
	r3dVector v = l2 - l1;
	r3dVector w = p - l1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);
	float b = c1 / c2;

	r3dPoint3D Pb = l1 + b * v;
	return (p-Pb).Length();
}

void obj_AI_Player::AddBloodOnGround(const r3dPoint3D& posOfShot)
{
	{
		r3dPoint3D blood_pos;
		r3dPoint3D blood_norm;
		bool has_collision = false;
		{
			PxRaycastHit hit;
			PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));
			if(g_pPhysicsWorld->raycastSingle(PxVec3(posOfShot.x, posOfShot.y, posOfShot.z), PxVec3(0, -1, 0), 10.0f, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL), hit, filter))
			{
				blood_pos = r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z);
				blood_norm = r3dPoint3D(hit.normal.x, hit.normal.y, hit.normal.z);
				has_collision = true;
			}
			else
			{
				//WTF??? How no collision? Is player up in the sky??
			}
		}
		if(has_collision)
		{
			DecalParams params;
			params.Pos		= blood_pos;
			params.Dir		= blood_norm;
			params.TypeID	= GetDecalID( r3dHash::MakeHash("default"), r3dHash::MakeHash("blood_ground") );
			if( params.TypeID != INVALID_DECAL_ID )
				g_pDecalChief->Add( params );
		}
	}
}

void obj_AI_Player::setMinimapVisibleTimer(float time, bool force)
{
	const ClientGameLogic& CGL = gClientLogic();
	if(CGL.localPlayer_ == 0)
	{
		m_fVisibleTimer = time;
		return;
	}
	if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		return;

	// detect only if within X meters
	if((CGL.localPlayer_->GetPosition() - GetPosition()).Length() > CGL.localPlayer_->getMinimapDetectionFireRadius() && !force)
		return;

	float mod = 0;
	// check for Recon Stealth Boost which will lower time you are visible on minimap by 4% for each level
	/*if(m_Skills[1] == 1)
		mod -= 5;
	else if(m_Skills[1] == 2)
		mod -= 10;
	else if(m_Skills[1] == 3)
		mod -= 15;
	else if(m_Skills[1] == 4)
		mod -= 20;
	else if(m_Skills[1] == 5)
		mod -= 25;*/
	// check for tactician stealth detection
	/*if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(CGL.localPlayer_->m_Skills[6] == 3)
			mod += 1;
		else if(CGL.localPlayer_->m_Skills[6] == 4)
			mod += 3;
		else if(CGL.localPlayer_->m_Skills[6] == 5)
			mod += 5;
	}*/
	time = time + mod;

	if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(CurLoadout.hasItem(AbilityConfig::AB_SilentStrike)) // silent strike, do not show this player on a map
			return;
	}
	m_fVisibleTimer = time;
}

void obj_AI_Player::OnNetPacket(const PKT_S2C_MoveTeleport_s& n)
{
	TeleportPlayer(n.teleport_pos, "PlayerMoveTeleport");
}

void obj_AI_Player::OnNetPacket(const PKT_C2C_MoveSetCell_s& n)
{
	netMover.SetCell(n);
}

void obj_AI_Player::OnNetPacket(const PKT_C2C_MoveRel_s& n)
{
	const CNetCellMover::moveData_s& md = netMover.DecodeMove(n);
	
	if(afterRespawnTimer>0) // check for old player moves and discard them
	{
		if((md.pos - GetPosition()).Length() > 10.0f)
			return;
	}
			
	m_fPlayerRotationTarget = md.turnAngle;
	bodyAdjust_y[1]         = md.bendAngle;
	PlayerState             = (md.state & 0xF);
	PlayerMoveDir           = (md.state >> 4) & 0xF;

	// calc velocity to reach position on time for next update
	r3dPoint3D vel = netMover.GetVelocityToNetTarget(
		GetPosition(),
		GPP->AI_SPRINT_SPEED * 1.5f,
		1.0f);

	SetVelocity(vel);
}

void obj_AI_Player::OnNetPacket(const PKT_C2C_PlayerJump_s& n)
{
	uberAnim_->StartJump();
	return;
}

void obj_AI_Player::ProcessRewardQueue()
{
	r3d_assert(NetworkLocal);
	
	if(rewards_.size() == 0)
		return;
		
	// one reward show per sec
	const float curTime = r3dGetTime();
	if(curTime < lastRewardShowTime_ + 1.0f)
		return;
	lastRewardShowTime_ = curTime;
	
	PKT_S2C_AddScore_s n = *rewards_.begin();
	rewards_.erase(rewards_.begin());

	if((n.ID & 0x8000) == 0)
	{
		GameStats.Score += n.HP;
		GameStats.GD    += n.GD;
	}
	else
	{
		// reward without adding score/gd
		n.ID &= ~0x8000;
	}
	
	const CGameRewards::rwd_s& rwd = g_GameRewards->GetRewardData(n.ID);

	char rewardName[128] = "";	// localized reward name
	char iconName[128] = "";
	if(rwd.IsSet)
	{
		sprintf(rewardName, "Reward_%s", rwd.Name.c_str());
		sprintf(iconName, "$Data/Menu/Rewards/RWD_%s.png", rwd.Name.c_str());
	}
	else
	{
		sprintf(rewardName, "RWD_%d", n.ID);
	}

	// and always show some crap about that reward
	const wchar_t* rewardString=gLangMngr.getString(rewardName);
	if(n.ID == RWD_KillAtSpawn) // no reward message
	{
		hudMain->showSystemMessage(rewardString);
	}
	else
	{
		hudMain->ShowAchievementCustom(rewardString, "", iconName, "");
	}

	hudMain->ShowScore(n.HP, 0, n.GD, GameStats.GD, GameStats.Score);
}

void obj_AI_Player::OnNetPacket(const PKT_S2C_AddScore_s& n)
{
	if(n.ID == RWD_Bomb_PlantBomb) // plant bomb
		++numBombsPlanted;
	if(n.ID == RWD_Bomb_DiffuseBomb) // diffuse bomb
		++numBombsDiffused;
	if(n.ID == RWD_Bomb_WinRound || n.ID == RWD_Bomb_WinRoundKills) // sabo won round
		++numSaboRoundsWon;
		
	if(NetworkLocal)
	{
		rewards_.push_back(n);
	}
	else
	{
		GameStats.Score += n.HP;
		GameStats.GD    += n.GD;
	}
}

BOOL obj_AI_Player::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	R3DPROFILE_FUNCTION("obj_AI_Player::OnNetReceive");
	r3d_assert(!(ObjFlags & OBJFLAG_JustCreated)); // make sure that object was actually created before processing net commands

	const ClientGameLogic& CGL = gClientLogic();

	switch(EventID)
	{
	default: return FALSE;
	case PKT_S2C_TeamSwitchError:
		{
			const PKT_S2C_TeamSwitchError_s& n = *(PKT_S2C_TeamSwitchError_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			r3d_assert(NetworkLocal); // should come only to local player
			if(NetworkLocal)
			{
				switch(n.errorType)
				{
				case PKT_S2C_TeamSwitchError_s::ERR_TOO_EARLY:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_TeamSwitch_TooEarly"));
					break;
				case PKT_S2C_TeamSwitchError_s::ERR_ALMOST_FINISHED:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_TeamSwitch_AlmostFinished"));
					break;
				case PKT_S2C_TeamSwitchError_s::ERR_JOINING_WINNING_TEAM:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_TeamSwitch_WinTeam"));
					break;
				case PKT_S2C_TeamSwitchError_s::ERR_JOINING_TEAM_WITH_MORE_PLAYERS:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_TeamSwitch_MorePlayers"));
					break;
					
				// special rebalancing codes, server will kill player after it
				case PKT_S2C_TeamSwitchError_s::OK_MOVED_TEAM0:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_MovedToOppositeTeam"));
					TeamID = 0;
					break;
				case PKT_S2C_TeamSwitchError_s::OK_MOVED_TEAM1:
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_MovedToOppositeTeam"));
					TeamID = 1;
					break;
					
				default:
					r3d_assert(false);
					break;
				}
			}
			break;
		}
	case PKT_S2C_WeaponPickedUp:
		{
			const PKT_S2C_WeaponPickedUp_s& n = *(PKT_S2C_WeaponPickedUp_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			r3d_assert(NetworkLocal);

			// change our current weapon to the new one, depending on which slot it is gonna go
			WeaponConfig* wpn = const_cast<WeaponConfig*>(gWeaponArmory.getWeaponConfig(n.itemID));
			if(wpn==NULL)
			{
				r3dOutToLog("Unknown weapon received from server, %d\n", n.itemID);
				break;
			}

			// update weapon data
			wpn->copyParametersFrom(n.wi);

			bool isFirstPerson = g_camera_mode->GetInt()==2 && NetworkLocal;

			STORE_CATEGORIES cat = wpn->category;
			// primary slot
			if(cat == storecat_ASR || cat == storecat_SNP || cat == storecat_MG)
			{
				SAFE_DELETE(m_Weapons[0]);
				m_Weapons[0] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
				m_Weapons[0]->setWeaponAttachmentsByIDs(n.attms.attachments);
			}
			else if(cat == storecat_SUPPORT || cat == storecat_SHTG || cat == storecat_SMG)
			{
				SAFE_DELETE(m_Weapons[1]);
				m_Weapons[1] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
				m_Weapons[1]->setWeaponAttachmentsByIDs(n.attms.attachments);
			}
			else if(cat == storecat_HG)
			{
				SAFE_DELETE(m_Weapons[2]);
				m_Weapons[2] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
				m_Weapons[2]->setWeaponAttachmentsByIDs(n.attms.attachments);
			}
			else
			{
				r3dOutToLog("Non pick able weapon rcvd from server, %d, %d\n", n.itemID, cat);
				break;
			}

			// update UI
			updateWeaponUI();

			UpdateCharWeaponMeshes();

			m_Loaded = false; // force check for loaded meshes, so that new weapon will load it's skeleton once mesh was loaded

			// send confirm to server
			PKT_C2C_ConfirmWeaponPickup_s confirm;
			confirm.secretCode = n.secretCode;
			confirm.itemID = n.itemID;
			p2pSendToHost(this, &confirm, sizeof(confirm));
		}
		break;
	case PKT_C2C_ConfirmWeaponPickup:
		{
			const PKT_C2C_ConfirmWeaponPickup_s& n = *(PKT_C2C_ConfirmWeaponPickup_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			r3d_assert(!NetworkLocal);

			// change our current weapon to the new one, depending on which slot it is gonna go
			const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(n.itemID);
			if(wpn==NULL)
			{
				r3dOutToLog("Unknown weapon received from server (confirm), %d\n", n.itemID);
				break;
			}

			bool isFirstPerson = g_camera_mode->GetInt()==2 && NetworkLocal;

			STORE_CATEGORIES cat = wpn->category;
			// primary slot
			if(cat == storecat_ASR || cat == storecat_SNP || cat == storecat_MG)
			{
				SAFE_DELETE(m_Weapons[0]);
				m_Weapons[0] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
			}
			else if(cat == storecat_SUPPORT || cat == storecat_SHTG || cat == storecat_SMG)
			{
				SAFE_DELETE(m_Weapons[1]);
				m_Weapons[1] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
			}
			else if(cat == storecat_HG)
			{
				SAFE_DELETE(m_Weapons[2]);
				m_Weapons[2] = gWeaponArmory.createWeapon(n.itemID, this, isFirstPerson, true);
			}
			else
			{
				r3dOutToLog("Non pick able weapon rcvd from server (confirm), %d, %d\n", n.itemID, cat);
				break;
			}
			UpdateCharWeaponMeshes();
		}
		break;
	case PKT_S2C_LootBoxPickedUp:
		{
			const PKT_S2C_LootBoxPickedUp_s& n = *(PKT_S2C_LootBoxPickedUp_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			r3d_assert(NetworkLocal);

			++numLootBoxesPickedup;

			// show message
			const ItemConfig* ic = gWeaponArmory.getItemConfig(n.itemID);
			if(ic)
				hudMain->ShowLootDropMsg(ic->m_StoreNameW, ic->m_StoreIcon);
		}
		break;

	case PKT_S2C_MoveTeleport:
		{
			const PKT_S2C_MoveTeleport_s& n = *(PKT_S2C_MoveTeleport_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			OnNetPacket(n);
			break;
		}
	case PKT_C2C_MoveSetCell:
		{
			const PKT_C2C_MoveSetCell_s& n = *(PKT_C2C_MoveSetCell_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			OnNetPacket(n);
			break;
		}
	case PKT_C2C_MoveRel:
		{
			const PKT_C2C_MoveRel_s& n = *(PKT_C2C_MoveRel_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			OnNetPacket(n);
			break;
		}

	case PKT_C2C_PlayerJump:
		{
			const PKT_C2C_PlayerJump_s& n = *(PKT_C2C_PlayerJump_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			OnNetPacket(n);
			break;
		}
		
	case PKT_C2C_PlayerSwitchWeapon:
		{
			const PKT_C2C_PlayerSwitchWeapon_s& n = *(PKT_C2C_PlayerSwitchWeapon_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			r3d_assert(!NetworkLocal);
			r3d_assert(m_Weapons[n.wid]);
			
			if(m_Weapons[n.wid] == NULL)
			{
				// do not crash in public build
				char plrUserName[64]; GetUserName(plrUserName);
#ifndef FINAL_BUILD
				r3dOutToLog("NET_DEBUG: %s tried to change to empty weapon slot %d", plrUserName, n.wid);
#endif
			}
			else
			{
				uberAnim_->StopGrenadeAnimations();	// we now can switch weapon while grenade animation active, so need to abort

				ChangeWeaponByIndex(n.wid);
				SyncAnimation(true);
			}
			break;
		}

	case PKT_C2C_PlayerReadyGrenade:
		{
			//r3dOutToLog("PKT_C2C_PlayerReadyGrenade\n");
			const PKT_C2C_PlayerReadyGrenade_s& n = *(PKT_C2C_PlayerReadyGrenade_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			m_SelectedWeapon = n.wid;
			r3d_assert(m_Weapons[m_SelectedWeapon]);

			uberAnim_->StartGrenadePinPullAnimation();
			break;
		}
	case PKT_C2C_PlayerFired:
		{
			//r3dOutToLog("PKT_C2C_PlayerFired\n");
			const PKT_C2C_PlayerFired_s& n = *(PKT_C2C_PlayerFired_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			m_SelectedWeapon = n.debug_wid;
			r3d_assert(m_Weapons[m_SelectedWeapon]);

			D3DXMATRIX fireFrom ;

			fireFrom = DrawRotMatrix ;

			fireFrom._41 = n.fire_from.x ;
			fireFrom._42 = n.fire_from.y ;
			fireFrom._43 = n.fire_from.z ;

			// get the weapon bone.  Fire requires it. 
			D3DXMATRIX weaponBone = uberEquip_->getWeaponBone(GetSkeleton(),fireFrom);

			if( m_Weapons[m_SelectedWeapon]->getCategory() == storecat_GRENADES ) {

				// bash back the translation, the rotation is all we wanted for grenades.
				weaponBone._41 = n.fire_from.x ;
				weaponBone._42 = n.fire_from.y ;
				weaponBone._43 = n.fire_from.z ;
			}
			
			m_Weapons[m_SelectedWeapon]->Fire(n.fire_to, weaponBone, n.holding_delay, n.fire_from);

			int muzzleAttm = m_Weapons[m_SelectedWeapon]->getWeaponAttachmentID(WPN_ATTM_MUZZLE);
			if(muzzleAttm != 400013 && muzzleAttm != 400125) // if using silencer - do not show up on minimap at all
			{
				float timeToShow = m_Weapons[m_SelectedWeapon]->getCategory()==storecat_SNP?GPP->c_fMinimapEnemyShootVisibleTimerSniper:GPP->c_fMinimapEnemyShootVisibleTimer;
				if(muzzleAttm == 400012)
					timeToShow *= 0.5f; // half time if using flash hider
				setMinimapVisibleTimer(timeToShow, m_Weapons[m_SelectedWeapon]->getCategory()==storecat_SNP); // sniper ALWAYS show up on minimap (Sergey's request)
			}

			if(m_Weapons[m_SelectedWeapon]->isGrenadeOrMineAnim())
				uberAnim_->StartGrenadeThrowAnimation();
			else
				uberAnim_->StartRecoilAnim();

			break;
		}
	case PKT_C2C_PlayerUseItem:
		{
			const PKT_C2C_PlayerUseItem_s& n = *(PKT_C2C_PlayerUseItem_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			// assumption: current weapon is set to whatever player is about to use so just play it's animation.
			if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->isUsableItem() && 
				m_Weapons[m_SelectedWeapon]->getItemID() != WeaponConfig::ITEMID_Cypher2 && 
				m_Weapons[m_SelectedWeapon]->getItemID() != WeaponConfig::ITEMID_LLDR)
				uberAnim_->StartShootAnim();
		}
		break;
	case PKT_S2C_PlayerUsedItemAns:
		{
			const PKT_S2C_PlayerUsedItemAns_s& n = *(PKT_S2C_PlayerUsedItemAns_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			// simulate what server does
			if(n.itemId == WeaponConfig::ITEMID_Bandages || n.itemId == WeaponConfig::ITEMID_Bandages2)
			{
				m_Health = m_Health + getMaxHealth()*n.var1;
				m_Health = R3D_MIN(m_Health.get(), getMaxHealth());
				m_BandagesEffectTime = r3dGetTime() + 0.5f;
			}
			else if(n.itemId == WeaponConfig::ITEMID_AdrenalineShot || n.itemId == WeaponConfig::ITEMID_EpinephrineShot)
			{
				m_SpeedBoost = n.var1;
				m_SpeedBoostTime = r3dGetTime() + n.var2;
				m_BandagesEffectTime = r3dGetTime() + 0.5f;
			}
			else if(n.itemId == WeaponConfig::ITEMID_MorphineShot || n.itemId == WeaponConfig::ITEMID_MethylmorphineShot)
			{
				m_MorhineShotTime = r3dGetTime() + n.var1;
				m_BandagesEffectTime = r3dGetTime() + 0.5f;
			}
		}
		break;

	case PKT_C2C_PlayerHitNothing:
		{
			break; 
		}
	case PKT_C2C_PlayerHitStatic:
	case PKT_C2C_PlayerHitStaticPierced:
		{
			const PKT_C2C_PlayerHitStatic_s& n = *(PKT_C2C_PlayerHitStatic_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			r3d_assert(m_Weapons[m_SelectedWeapon]);

			// create decal
			if(n.decalIdx != 0 )
			{
				DecalParams params;
				params.Dir	= n.hit_norm;
				params.Pos	= n.hit_pos;
				params.TypeID	= (uint32_t)(n.decalIdx) - 1;
				if(params.TypeID >= 0 && params.TypeID < (int)g_pDecalChief->GetTypeCount())
				{
					g_pDecalChief->Add( params );
				}
				else
				{
					// some cheat or invalid data
				}
			}

			// impact particle
			if(n.particleIdx != 0)
			{
				uint32_t particleIdx = (uint32_t)(n.particleIdx) - 1;
				if(particleIdx < ImpactParticleEntry::NumRegistered)
				{
					SpawnImpactParticle(particleIdx, n.hit_pos, n.hit_norm);
				}
				else
				{
					// some cheat or invalid data
				}
			}

			{
				if(GameObject* from = GameWorld().GetNetworkObject(n.FromID))
				{
					r3dPoint3D waterSplashPos;
					extern bool TraceWater(const r3dPoint3D& start, const r3dPoint3D& finish, r3dPoint3D& waterSplashPos);
					if( TraceWater( from->GetPosition(), n.hit_pos, waterSplashPos))
					{	
						extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
						waterSplashPos.y -= 0.1f;
						WaterSplash(waterSplashPos, 0.1f, 30.0f, 0.04f, 2);
					}
				}
			}

			// add impulse
			GameObject* target = GameWorld().GetObjectByHash(n.hash_obj);
			if(m_Weapons[m_SelectedWeapon]->isImmediateShooting() && target)
				if(target->PhysicsObject)
					target->PhysicsObject->AddImpulseAtPos(-n.hit_norm*m_Weapons[m_SelectedWeapon]->getAmmoSpeed()*m_Weapons[m_SelectedWeapon]->getAmmoMass()/150.0f, n.hit_pos);

			// play near miss sound
			if(m_Weapons[m_SelectedWeapon]->isImmediateShooting())
			{
				if(CGL.localPlayer_)
				{
					float dist = dist_Point_to_Line(CGL.localPlayer_->GetPosition(), GetPosition(), n.hit_pos);
					if(dist < 5.0f && rand()%5==0)
					{
						static int bulletHitMissSoundID = -1;
						if(bulletHitMissSoundID==-1)
							bulletHitMissSoundID = SoundSys.GetEventIDByPath("Sounds/BulletHits/Miss");
						snd_PlaySound(bulletHitMissSoundID, CGL.localPlayer_->GetPosition());
					}
				}
			}

			break;
		}

	case PKT_C2C_PlayerHitDynamic:
		{
			const PKT_C2C_PlayerHitDynamic_s& n = *(PKT_C2C_PlayerHitDynamic_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			r3d_assert(m_Weapons[m_SelectedWeapon]);

			r3dPoint3D dir = (GetPosition()-n.hit_pos).NormalizeTo();

			GameObject* target = GameWorld().GetNetworkObject(n.targetId);
			if(!target) 
			{
				r3d_assert(target);
				break;
			}

			// if we hit local player, display hit effect
			if(target->isObjType(OBJTYPE_Human))
			{
				if(!(CGL.m_gameInfo.friendlyFire == 0 && (TeamID == ((obj_AI_Player*)target)->TeamID)))
				{
					AddBloodOnGround(n.hit_pos);
					SpawnImpactParticle(r3dHash::MakeHash("player"), r3dHash::MakeHash(m_Weapons[m_SelectedWeapon]->getDecalSource()), n.hit_pos, dir);
				}

				if(target->NetworkLocal == false || CGL.localPlayer_==NULL) 
					break;

				// if hit local player
				extern void TPSGameHUD_AddHitEffect(GameObject* from);
				if(!(CGL.m_gameInfo.friendlyFire == 0 && (TeamID == ((obj_AI_Player*)target)->TeamID)))
					TPSGameHUD_AddHitEffect(this);

			}
			else // hit something else, for now that is UAV only
			{
				SpawnImpactParticle(r3dHash::MakeHash("Metal"),r3dHash::MakeHash(m_Weapons[m_SelectedWeapon]->getDecalSource()), n.hit_pos, dir);
			}

			break;
		}

	case PKT_S2C_ResupplyPlayer:
		{
			//r3dOutToLog("PKT_S2C_ResupplyPlayer\n");
			const PKT_S2C_ResupplyPlayer_s& n = *(PKT_S2C_ResupplyPlayer_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			if(n.result==1)
			{
				for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
				{
					if(m_Weapons[i])
						m_Weapons[i]->Resupply();
				}
	
				static int resupplySoundID = -1;
				if(resupplySoundID==-1)
					resupplySoundID = SoundSys.GetEventIDByPath("Sounds/Misc/Resupply");
				// play resupply sound
				snd_PlaySound(resupplySoundID, GetPosition());
			}
			else if(NetworkLocal) // show message
			{	
				if(n.result ==2)
				{
					wchar_t tempBuf[256];
					swprintf(tempBuf, 256, gLangMngr.getString("$HUD_Msg_TimeUntilResupply"), n.timeLeft);
					hudMain->AddMessage(tempBuf);
				}
				else if(n.result ==3)
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_ResupplyTooFarAway"));
				else if(n.result == 4)
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_NeedBlackOpsKey"));
				else
					hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_FailedResupply"));
			}

			break;
		}
	
	case PKT_S2C_AddScore:
		{
			const PKT_S2C_AddScore_s& n = *(PKT_S2C_AddScore_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			OnNetPacket(n);
			break;
		}
		
	case PKT_S2C_SetPlayerScore:
		{
			const PKT_S2C_SetPlayerScore_s& n = *(PKT_S2C_SetPlayerScore_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			GameStats.Score = n.score;
			GameStats.Kills = n.kills;
			GameStats.Deaths = n.deaths;

			// mark all queued rewards as already added to score
			for(size_t i=0, e=rewards_.size(); i<e; i++)
				rewards_[i].ID |= 0x8000;

			break;
		}

	case PKT_S2C_UnlockAchievement:
		{
			const PKT_S2C_UnlockAchievement_s& n = *(PKT_S2C_UnlockAchievement_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
			
			TPSGameHud_UnlockAchievement( n.achiIdx );
			break;
		}

	case PKT_S2C_BerserkerAbility:
		{
			const PKT_S2C_BerserkerAbility_s& n = *(PKT_S2C_BerserkerAbility_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			m_BerserkerTime = n.time;
			m_AuraType = AT_BERSERK;
			break;
		}
	case PKT_S2C_BigSurpriseAbility:
		{
			const PKT_S2C_BigSurpriseAbility_s& n = *(PKT_S2C_BigSurpriseAbility_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			/*GameObject* from = GameWorld().GetNetworkObject(n.FromID);
			if(from)
			{
				static int bigSurpriseAbilitySoundID = -1;
				if(bigSurpriseAbilitySoundID==-1)
					bigSurpriseAbilitySoundID = SoundSys.GetEventIDByPath("Sounds/Misc/BigSurpriseAbility");
				snd_PlaySound(bigSurpriseAbilitySoundID, from->GetPosition());
			}*/
			
			break;
		}
	case PKT_S2C_SpawnExplosion:
		{
			const PKT_S2C_SpawnExplosion_s& n = *(PKT_S2C_SpawnExplosion_s*)packetData;
			r3d_assert(packetSize == sizeof(n));
		

			GameObject* from = GameWorld().GetNetworkObject(n.FromID);
			if(from)
			{
				// add decal
				DecalParams params;
				params.Dir		= r3dPoint3D(0,1,0);
				params.Pos		= from->GetPosition();
				params.TypeID	= GetDecalID( r3dHash::MakeHash(""), r3dHash::MakeHash("grenade") );
				if( params.TypeID != INVALID_DECAL_ID )
					g_pDecalChief->Add( params );
				SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("grenade"), from->GetPosition(), r3dPoint3D(0,1,0));

				//	Start radial blur effect
				gExplosionVisualController.AddExplosion(from->GetPosition(), n.radius);
			}
			break;
		}
	}

	return TRUE;
}

bool obj_AI_Player::hasScopeMode() const 
{
	if(bDead)
		return false;
////////Anticheat/////////////////////////

	//RANDOME CODE 
	//========================================================================================

	if(IsDebuggerPresent()){
		r3dOutToLog("WARNING : ERROR 104");
		extern bool gKillFlag;
		gKillFlag  = true;
	}

	//========================================================================================


/////////////////////////////////////////////


	return (m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->hasScopeMode(uberEquip_->isFirstPerson)) || laserViewActive_; 
}

bool obj_AI_Player::hasScopeMode_hidePlayerModel() const 
{
	if(m_Weapons[m_SelectedWeapon] && m_Weapons[m_SelectedWeapon]->getScopeConfig(uberEquip_->isFirstPerson))
		return m_Weapons[m_SelectedWeapon]->getScopeConfig(uberEquip_->isFirstPerson)->hide_player_model;
	
	return false;
}

void obj_AI_Player::DrawReticle()
{
	if(!NetworkLocal)
		return;
	
	g_RenderScopeEffect = 0;
	
	if(bDead || gClientLogic().gameFinished_ || uavViewActive_ || laserViewActive_ || m_Weapons[m_SelectedWeapon] == NULL)
	{
		CurrentRig.allowScope = false; // to make sure that scope will not be rendered when you are dead
		g_ScopeFeedbackTimer = 0.0f;
		return;
	}
	
	r3dRenderer->SetRenderingMode(R3D_BLEND_PUSH|R3D_BLEND_NZ|R3D_BLEND_ALPHA);
	const ScopeConfig* scopeConfig = m_Weapons[m_SelectedWeapon]->getScopeConfig(uberEquip_->isFirstPerson);
	r3d_assert(scopeConfig);
	float cx = r3dRenderer->ScreenW/2;
	float cy = r3dRenderer->ScreenH/2;
	float cy2 = r3dRenderer->ScreenH*0.32f;

	// draw laser pointer if any
	if(PlayerState != PLAYER_MOVE_SPRINT && !m_Weapons[m_SelectedWeapon]->isReloading() && g_camera_mode->GetInt() == 2)
	{
		r3dPoint3D laserPos;
		if(m_Weapons[m_SelectedWeapon]->hasLaserPointer(laserPos))
		{
			r3dRenderer->SetRenderingMode( R3D_BLEND_PUSH | R3D_BLEND_ADD | R3D_BLEND_ZC );

			float size  = R3D_MIN(r3dRenderer->ScreenW*0.02f, r3dRenderer->ScreenH*0.02f);

			// gradually decrease size over distance
			const float maxDecDist = 20.0f;
			float dist = (gCam - laserPos).Length();
			if(dist >= maxDecDist) dist = maxDecDist;
			size = R3D_LERP(size, size*0.25f, dist / maxDecDist);

			r3dPoint3D laserPosScreen;
			r3dProjectToScreen(laserPos, &laserPosScreen);
			if(m_isAiming)
			{
				laserPosScreen.x = cx;
				laserPosScreen.y = cy;
			}
			r3dColor laserColor = r3dColor::red;
			laserColor.A = (int)R3D_LERP(196.0f, 0.0f, dist/400.0f);
			r3dDrawBox2D(float(laserPosScreen.x-(size/2)), float(laserPosScreen.y-(size/2)), size, size, laserColor, s_laserPointerTex);

			r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
		}
	}

	// check if blinded by flash light or laser
	{
		// check all other players, see if they are close enough, have attachment and pointing it at you
		ClientGameLogic& CGL = gClientLogic();
		for(int i=0; i<CGL.MAX_NUM_PLAYERS; ++i)
		{
			obj_AI_Player* plr = CGL.GetPlayer(i);
			if(g_bEditMode)
			{
				plr = this;
				i = CGL.MAX_NUM_PLAYERS;
			}
			if(!plr)
				continue;
			if(plr == this && !g_bEditMode)
				continue;
			if(plr->m_Weapons[plr->m_SelectedWeapon] == NULL)
				continue;
			int attmID = plr->m_Weapons[plr->m_SelectedWeapon]->getWeaponAttachmentID(WPN_ATTM_LEFT_RAIL);
			if(attmID == 400004 || attmID == 400018 || attmID == 400021 || attmID == 400022) // laser, flashlight
			{
				float dist = (plr->GetPosition() - gCam).Length();

				// other players do not have FPS model, so just use muzzle pos
				D3DXMATRIX weaponBone = plr->uberEquip_->getWeaponBone(plr->GetSkeleton(), plr->DrawFullMatrix);
				r3dPoint3D attmPos = plr->m_Weapons[plr->m_SelectedWeapon]->getMuzzlePos(weaponBone);
				{
					// check if attachment is visible
					r3dPoint3D attmScreenPos;
					if(!r3dProjectToScreen(attmPos, &attmScreenPos))
						continue;

					r3dPoint3D attmDir = r3dPoint3D(weaponBone._11, weaponBone._12, weaponBone._13);
					if(attmDir.Dot(gCam.vPointTo)>0.0f)
						continue;
					float distToCamera = (attmPos - gCam).Length();

					// check that attachment is visible and not blocked
					PxRaycastHit hit;
					PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER),0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));
					r3dPoint3D dirToAttm = (attmPos - gCam).NormalizeTo();
					bool hitResult = g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dirToAttm.x, dirToAttm.y, dirToAttm.z), distToCamera, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter);
					if(!hitResult)
					{
						/*{
							float size = r3dRenderer->ScreenH*0.1f;
							r3dDrawBox2DRotate(attmScreenPos.x, attmScreenPos.y, size, size, 0.0f, r3dColor::white, s_lightHaloTex);
						}*/

						float viewDot = -attmDir.Dot(dirToAttm);
						if(attmID == 400018 || attmID == 400022) // flashlight
						{
							if(viewDot < 0.75f)
								viewDot = 0;
							else
								viewDot = viewDot*4-3.0f;
						}
						else // laser
						{
							if(viewDot < 0.997f)
								viewDot = 0;
							else
								viewDot = ((viewDot*100.0f)-99.7f); // convert from [0.99, 1.0] to [0, 1]
						}
						
						{
							r3dTexture* tex = (attmID==400004||attmID==400021)?s_laserFlashTex:s_flashlightFlashTex;
							float size = R3D_MIN(r3dRenderer->ScreenW, r3dRenderer->ScreenH);
							float scale = viewDot*0.5f;
							if(distToCamera < 10.0f)
								scale = scale * (1.0f+2.0f*R3D_CLAMP((distToCamera/10.0f), 0.0f, 1.0f));
							else if(attmID == 400018 || attmID == 400022) // flashlight, max range 20m
								scale = scale * (3.0f-(3*(R3D_CLAMP((distToCamera-10.0f)/10.0f, 0.0f, 0.95f))));
							else // laser, max range 40m
								scale = scale * (3.0f-(3*(R3D_CLAMP((distToCamera-10.0f)/30.0f, 0.0f, 1.0f))));
							size = size * scale;
							r3dDrawBox2DRotate(attmScreenPos.x, attmScreenPos.y, size, size, r3dSin(r3dGetTime()*0.01f)*360.0f, r3dColor::white, tex);
							r3dDrawBox2DRotate(attmScreenPos.x, attmScreenPos.y, size, size, r3dCos(r3dGetTime()*0.012f)*360.0f, r3dColor::white, tex);
						}
					}
				}
			}
		}
	}

	bool scopeRendering = hasScopeMode() && CurrentRig.allowScope;
	if(scopeRendering) 	// draw sniper scope
	{
		g_RenderScopeEffect = 1;
		gPFX_ScopeEffect.SetScopeReticle(scopeConfig->scope_reticle);
		gPFX_ScopeEffect.SetScopeNormal(scopeConfig->scope_normal);
		gPFX_ScopeEffect.SetScopeMask(scopeConfig->scope_mask);
		gPFX_ScopeEffect.SetScopeBlurMask(scopeConfig->scopeBlur_mask);

		ScopeMaterialParams params = gPFX_ScopeEffect.GetMaterialParams() ;
		params.Lighting = scopeConfig->lighting ;
		gPFX_ScopeEffect.SetMaterialParams( params ) ;

		float scaleScope = 1.0f;
		float scopeScaleMod = 0.20f;
		if(g_camera_mode->GetInt() == 2)
			scopeScaleMod = 0.10f;

		scaleScope += g_ScopeFeedbackTimer*scopeScaleMod;
		if(g_ScopeFeedbackTimer>0)
		{
			g_ScopeFeedbackTimer = R3D_MAX(g_ScopeFeedbackTimer-r3dGetFrameTime()*8.0f, 0.0f);
		}
		gPFX_ScopeEffect.SetScopeScale(scaleScope);
	}
#ifndef FINAL_BUILD
	else if(g_bEditMode && r_show_player_debug_data->GetBool()) // draw reticule only in editor, in game reticule is in flash
	{
		float rw = (float)scopeConfig->reticule->GetWidth();
		float rh = (float)scopeConfig->reticule->GetHeight();
		float yy = g_camera_mode->GetInt() == 1?cy2:cy;
		r3dDrawBox2D(float(cx-(rw/2)), float(yy-(rh/2)), rw, rh, r3dColor::white, scopeConfig->reticule);
	}
#endif

	if(!scopeRendering)
		g_ScopeFeedbackTimer = 0.0f;

	if(m_HitMarkerFadeout>0)
	{
		float rw = (float)m_HitMarkerTex->GetWidth();
		float rh = (float)m_HitMarkerTex->GetHeight();
		r3dColor clr = r3dColor::white; clr.A = BYTE(m_HitMarkerFadeout*255.0f);
		if(m_isInScope)
			r3dDrawBox2D(float(cx-(rw/2)), float(cy-(rh/2)), rw, rh, r3dColor::white, m_HitMarkerTex);
		else
		{
			float yy = g_camera_mode->GetInt() == 1?cy2:cy;
			r3dDrawBox2D(float(cx-(rw/2)), float(yy-(rh/2)), rw, rh, r3dColor::white, m_HitMarkerTex);
		}
	}

	if(r3dGetTime() < m_BandagesEffectTime)
	{
		// !! hardcoded for 0.5 duration
		float time = m_BandagesEffectTime - r3dGetTime(); // values: 0.5...0.0
		float infl = 0;
		time -= 0.25f;  // 0.25...-0.25
		time *= 4.0f; // 1.0...-1.0
		time = R3D_ABS(time); // 1.0...0.0...1.0
		time = 1.0f-time; // 0.0...1.0...0.0
		time = pow(time, 2.0f); // curve
		infl = R3D_CLAMP(time, 0.0f, 1.0f);

		r3dColor24 effectColor(153, 217, 255, 196);
		effectColor = effectColor * infl;
		r3dDrawBox2D(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, effectColor);
	}
	
	r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
}

void obj_AI_Player::DrawLabel()
{
	r3dRenderer->SetMaterial(NULL);

	if(NetworkLocal && gClientLogic().localPlayer_)
	{
		// show a warning if FPS or PING is low
		float fps = r3dGetAvgFPS();
		if(fps < 20.0f)
			Font_Label->PrintF(r3dRenderer->ScreenW-80, 0, r3dColor(243,43,37), "FPS: %.1f", fps);
	}

	r3dPoint3D off(0, Height, 0);
	r3dColor team_clr = TeamID == 0 ? r3dColor(0, 0, 255) : r3dColor(255, 0, 0);
	
	
	/*
	{
	  // muzzler position debug
	  D3DXMATRIX mm = uberEquip_->getWeaponBone(GetSkeleton(),GetPosition());
	  r3dPoint3D pos = m_Weapons[m_SelectedWeapon]->getMuzzlePos(mm);
	  r3dDrawLine3D(pos, pos + r3dPoint3D(mm._11, mm._12, mm._13), gCam, 0.005f, r3dColor(255, 255, 255));
	  r3dDrawLine3D(pos, pos + r3dPoint3D(0, 1, 0), gCam, 0.005f, r3dColor(255, 255, 255));

	 // r3dDrawLine3D(gTempTargetPos, gTempTargetPos + r3dPoint3D(mm._11, mm._12, mm._13), gCam, 0.005f, r3dColor(255, 0, 0));
	 // r3dDrawLine3D(gTempTargetPos, gTempTargetPos + r3dPoint3D(0, 1, 0), gCam, 0.005f, r3dColor(255, 0, 0));
	  
	 // r3dDrawLine3D(gTempTargetPos, gTempMuzzlePos, gCam, 0.005f, r3dColor(0,255,0));
	  r3dRenderer->Flush();
	}
	*/

#ifndef FINAL_BUILD
	if(NetworkLocal && r_show_player_debug_data->GetBool() && m_Weapons[m_SelectedWeapon])
	{
		Font_Label->PrintF(10, r3dRenderer->ScreenH-240,r3dColor(255,255,255), "m_Health %.2f", m_Health.get());
		Font_Label->PrintF(10, r3dRenderer->ScreenH-220,r3dColor(255,255,255), "DmgRcvd %.2f", lastDamageRcvd);
		Font_Label->PrintF(10, r3dRenderer->ScreenH-200,r3dColor(255,255,255), "AccumDmgRcvd %.2f", accumLastDamageRcvd);
		Font_Label->PrintF(10, r3dRenderer->ScreenH-180,r3dColor(255,255,255), "Recoil %.2f, %.2f", RecoilViewModTarget.x, RecoilViewModTarget.y);

		if(Keyboard->WasPressed(kbsO))
			accumLastDamageRcvd = 0;

		/*Font_Label->PrintF(10, r3dRenderer->ScreenH-280,r3dColor(255,255,255), "%s %d %d (%dx%d)", 
			m_Weapons[m_SelectedWeapon]->getConfig()->m_StoreName,
			m_Weapons[m_SelectedWeapon]->m_numBulletsLeftInClip,
			m_Weapons[m_SelectedWeapon]->m_numBulletsLeft,
			m_Weapons[m_SelectedWeapon]->m_pConfig->m_clipSize,
			m_Weapons[m_SelectedWeapon]->m_ModifiedNumClips);*/
	}
#endif

#ifndef FINAL_BUILD
	// editing rig animation speed
	if(0)
	{
		int astate = uberAnim_->AnimPlayerState;
		Font_Label->PrintF(10, r3dRenderer->ScreenH-300, r3dColor(255,255,255), "AnimState:%d, Speed:%f", astate, uberAnim_->AnimSpeedStates[astate]);
		
		if(Keyboard->IsPressed(kbsT)) uberAnim_->AnimSpeedStates[astate] -= r3dGetFrameTime() * 0.2f;
		if(Keyboard->IsPressed(kbsY)) uberAnim_->AnimSpeedStates[astate] += r3dGetFrameTime() * 0.2f;
	}
#endif

#ifndef FINAL_BUILD
	char plrUserName[64]; GetUserName(plrUserName);
	if(strncmp(plrUserName, "@@TestDummy", 10) == 0)
	{
		r3dPoint3D scrCoord;
		if(r3dProjectToScreen(GetPosition() + r3dPoint3D(0, 1, 0), &scrCoord))
		{
			float dist = gClientLogic().localPlayer_ ? (gClientLogic().localPlayer_->GetPosition() - GetPosition()).Length() : 0;
			Font_Label->PrintF(scrCoord.x+20, scrCoord.y,   r3dColor(255,255,255), "LastDmg %.2f", lastDamageRcvd);
			Font_Label->PrintF(scrCoord.x+20, scrCoord.y+25,r3dColor(255,255,255), "Distance %.2f", dist);
		}
	}
#endif

	//r3dDrawLine3D(GetPosition() + off, GetPosition() + off + r3dPoint3D(0, 0.03f, 0) * m_Health, Cam, 0.1f, team_clr);
#ifndef FINAL_BUILD
	if(r_show_player_debug_data->GetBool())
	{
		float x = 10;
		float y = 100;
		for(size_t i=0; i<uberAnim_->anim.AnimTracks.size(); i++) 
		{
			const r3dAnimation::r3dAnimInfo& ai = uberAnim_->anim.AnimTracks[i];

			char st[256] = "";
			if(ai.dwStatus & ANIMSTATUS_Playing) strcat(st, "Play ");
			if(ai.dwStatus & ANIMSTATUS_Paused) strcat(st, "Pause ");
			if(ai.dwStatus & ANIMSTATUS_Finished) strcat(st, "Finish ");
			if(ai.dwStatus & ANIMSTATUS_Fading) strcat(st, "Fade ");
			if(ai.dwStatus & ANIMSTATUS_Expiring) strcat(st, "Expire ");

			_r3dSystemFont->PrintF(x, y, r3dColor(255, 255, 255), "%s: %02.1f, %X %s", 
				ai.pAnim->pAnimName, ai.fInfluence, ai.dwStatus, st);

			y += 20;
		}
	}
#endif
#ifndef FINAL_BUILD
	if(r_show_player_health->GetBool())
	{
		r3dPoint3D scrCoord;
		if(r3dProjectToScreen(GetPosition() + r3dPoint3D(0, 1, 0), &scrCoord))
		{
			Font_Label->PrintF(scrCoord.x, scrCoord.y,   r3dColor(255,255,255), "%.1f", m_Health.get());
		}
	}
#endif

}

//------------------------------------------------------------------------
struct PlayerRenderable : Renderable
{
	void Init( obj_AI_Player::AuraType at, int Transparent )
	{
		DrawFunc = Draw;
		PlayerAuraType = at ;

		this->Transparent = Transparent ;
		matFlagsModifier = 0;
		FirstPerson = 0 ;
	}

	void ApplyCamouflageSettings()
	{
		if (!Parent)
			return;

		gCamouflageDataManager.SetCurrentData(*Parent);
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PlayerRenderable* This = static_cast<PlayerRenderable*>( RThis );

		bool draw_weapon = true;

		struct RenderEvents
		{
			RenderEvents()
			{
				D3DPERF_BeginEvent( 0, L"PlayerRenderable::Draw" ) ;
			}

			~RenderEvents()
			{
				D3DPERF_EndEvent() ;
			}

		} renderEvents; renderEvents ;

		CUberEquip::DrawType dt;

		extern int PS_TRANSPARENT_ID		;
		extern int PS_TRANSPARENT_AURA_ID	;

		struct RestoreCamOnExit
		{
			RestoreCamOnExit()
			{
				needRestore = false ;
			}

			~RestoreCamOnExit()
			{
				if( needRestore )
				{
					r3dRenderer->SetCamera( gCam ) ;
					if( r_particles_quality->GetInt() > 3 )
					{
						D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_ZENABLE, TRUE ) ) ;
					}
				}
			}

			bool needRestore ;
		} restoreCamOnExit ;

		if( ( This->Transparent || This->PlayerAuraType != obj_AI_Player::AT_NONE )
				&&
			This->FirstPerson			
			)
		{
			r3dCamera firstPersonCam = gCam ;
			firstPersonCam.NearClip = r_first_person_render_z_start->GetFloat();
			firstPersonCam.FarClip = r_first_person_render_z_end->GetFloat();
			firstPersonCam.FOV = r_first_person_fov->GetFloat();

			restoreCamOnExit.needRestore = true ;

			r3dRenderer->SetCamera( firstPersonCam ) ;
		}

		if( This->PlayerAuraType != obj_AI_Player::AT_NONE )
		{
			draw_weapon = false ;
			dt = CUberEquip::DT_AURA ;
			r3dMaterial::SetupTransparentStates( 1 );

			void SetParticleDepth();
			SetParticleDepth();

			float r(1.0f), g(1.0), b(1.0f);

			if( This->PlayerAuraType == obj_AI_Player::AT_HEALING )
			{
				r = 0.f ;
				g = 1.f ;
				b = 0.25f ;
			}
			else if(This->PlayerAuraType == obj_AI_Player::AT_MORHINE)
			{
				r = 0.f ;
				g = 0.f ;
				b = 1.0f ;
			}
			else if(This->PlayerAuraType == obj_AI_Player::AT_BERSERK)
			{
				r = 1.f ;
				g = 0.f ;
				b = 0.f ;
			}
			else if(This->PlayerAuraType == obj_AI_Player::AT_SPAWNPROTECTION) // golden color
			{
				r = 1.0f ;
				g = 0.62f ;
				b = 0.15f ;
			}

			r3dRenderer->SetTex( gPlayer_AuraTexture ) ;

			{
				float vConst[ 4 ] = { r, g, b, r3dGetTime()  } ;
				D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_AURA, vConst, 1 ) );
			}

			void SetupFilmToneConstants( int reg ) ;
			SetupFilmToneConstants( 24 ) ;

			r3dRenderer->SetPixelShader( PS_TRANSPARENT_AURA_ID ) ;
			r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_ZC);
			r3dRenderer->pd3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
			r3dRenderer->pd3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		else
		{
			const Weapon* wpn = This->Parent->m_Weapons[This->Parent->m_SelectedWeapon];
			if(wpn && wpn->isGrenadeOrMineAnim())
			{
				if(This->Parent->uberAnim_->IsGrenadeLaunched())
					draw_weapon = false; // throwing grenade, so do not render grenade in hand
			}

			if( This->Transparent )
			{
				const CamoTimeLine &ctl = This->Parent->camoTimeLine;
				float s = ctl.transparencyOnShock;

				float vConst[ 4 ][ 4 ] =
				{
					// float4 Aura				: register(c14); // (c65);
					{ 1.0f - ctl.camoMasterTransparency,	s * (1 - s) * 4, ctl.transparentCamoAlpha, r3dGetTime() * 0.25f * gCamoSettings.animSpeed },
					// float4 TCamoExtra		: register(c15);
					{ gCamoSettings.texScale * 4, gCamoSettings.distortScale, gCamoSettings.reflectionAmmount, 0.f },
					// float4 TCamoColor0			: register(c16);
					{ gCamoSettings.color0.R / 255.f, gCamoSettings.color0.G / 255.f, gCamoSettings.color0.B / 255.f, 0.f },
					// float4 TCamoColor1			: register(c17);
					{ gCamoSettings.color1.R / 255.f, gCamoSettings.color1.G / 255.f, gCamoSettings.color1.B / 255.f, gCamoSettings.color1.A / 255.f }
				};
				D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_AURA, (float*)vConst, 4 ) );
				
				vConst[0][3] = 0.01f;
				D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 23, (float*)vConst, 1 ) ) ;

				for(int i=0; i<=SLOT_Head3; i++)
				{
					if(This->Parent->uberEquip_->slots_[i].gear)
					{
						if( r3dMesh* mesh = This->Parent->uberEquip_->slots_[i].gear->getModel(This->Parent->uberEquip_->isFirstPerson) )
						{
							mesh->ExtrudeAmmount = 0.01f ;
						}
					}
				}

				r3dRenderer->SetTex( gPlayer_CamoTexture, 9 ) ;

				r3dRenderer->pd3ddev->SetSamplerState(9, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				r3dRenderer->pd3ddev->SetSamplerState(9, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

			}

			if(This->Parent->bDead)
				draw_weapon = false;

			This->ApplyCamouflageSettings();
			dt = CUberEquip::DT_DEFERRED;
		}

		float vCData[ 4 ] = { 1, 1, 1, 0 } ;
#if 0
 		if(gClientLogic().localPlayer_ && This->Parent->TeamID != gClientLogic().localPlayer_->TeamID)
 		{
 			vCData[ 0 ] = 1 ; vCData[ 1 ] = 0 ; vCData[ 2 ] = 0 ; 
 		}
 		else
 		{
 			vCData[ 0 ] = 0 ; vCData[ 1 ] = 0 ; vCData[ 2 ] = 1 ; 
 		}
#endif
		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_MASKCOLOR, vCData, 1 ) );

		if (This->matFlagsModifier)
		{
			for (int i = 0; i < SLOT_Max; ++i)
			{
				r3dMesh *m = This->Parent->uberEquip_->getSlotMesh((ESlot)i);
				if(!m) continue;
				if(!m->IsDrawable()) continue;

				for (int j = 0, e = m->GetNumMaterials(); j < e; ++j)
				{
					r3dTriBatch &tb = m->MatChunks[j];
					r3dMaterial *mat = tb.Mat;
					if (!mat) continue;

					This->matFlagsModifier(mat);
				}
			}
		}

		if( This->Transparent )
		{
			void SetVolumeFogParams() ;
			SetVolumeFogParams() ;

			// if it's local, draw depth prepass to disable backfaces for camo so that everything is not uggly.
			// TODO : if we spectate we should do this on spectate target!
			if( This->Parent->NetworkLocal && r3dRenderer->SupportsR32FBlending && !This->FirstPerson )
			{
				void ActivateTransparentBuffer();
				void ActivateParticleDepth();
				void DeactivateTransparentBuffer();
				void DeactivateParticleDepth();
				void SetParticleDepth();
				void RemoveParticleDepth();

				RemoveParticleDepth();
				r3dTransparentDistortFlush() ;
				DeactivateTransparentBuffer();
				ActivateParticleDepth();

				r3dRenderer->SetPixelShader( PS_DEPTH_ID ) ;

				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHABLENDENABLE,  TRUE ) );

				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE ) );
				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ) );

				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_MIN ) );

				This->Parent->uberEquip_->Draw( This->Parent->GetSkeleton(), This->Parent->DrawFullMatrix, draw_weapon, CUberEquip::DT_DEPTH, This->FirstPerson ? true : false );

				DeactivateParticleDepth();
				ActivateTransparentBuffer();
				r3dTransparentSetDistort( 1 ) ;
				SetParticleDepth();

				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD ) );
			}

			{
				D3DXMATRIX sm0, sm1 ;

				D3DXMatrixTranspose( &sm0, &This->Parent->DrawFullMatrix ) ;

				sm1 = This->Parent->DrawFullMatrix * r3dRenderer->ViewMatrix ;

				D3DXMatrixTranspose( &sm1, &sm1 ) ;

				float consts[ 6 ][ 4 ] =
				{
					// float4x3 mWorld : register( c32 ) ;
					{ sm0._11, sm0._12, sm0._13, sm0._14 },
					{ sm0._21, sm0._22, sm0._23, sm0._24 },
					{ sm0._31, sm0._32, sm0._33, sm0._34 },
					// float4x3 mWorldView : register( c35 ) ;
					{ sm1._11, sm1._12, sm1._13, sm1._14 },
					{ sm1._21, sm1._22, sm1._23, sm1._24 },
					{ sm1._31, sm1._32, sm1._33, sm1._34 },
				} ;

				D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( 32, (float*)consts, 6 ) ) ;
			}

			r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );
			D3D_V( r3dRenderer->pd3ddev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE ) );
			D3D_V( r3dRenderer->pd3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );

			r3dTransparentSetDistort( 1 ) ;

			if( This->FirstPerson && r_particles_quality->GetInt() > 3 )
			{
				D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_ZENABLE, FALSE ) ) ;
			}

		}

		This->Parent->uberEquip_->Draw( This->Parent->GetSkeleton(), This->Parent->DrawFullMatrix, draw_weapon, dt, This->FirstPerson ? true : false );

		if( This->PlayerAuraType != obj_AI_Player::AT_NONE )
		{
			r3dRenderer->SetPixelShader( PS_TRANSPARENT_ID ) ;
		}

		if(This->Parent->s_UAVFriendlyBlinkTex && gClientLogic().localPlayer_ && gClientLogic().localPlayer_->uavViewActive_ && gClientLogic().localPlayer_->TeamID == This->Parent->TeamID)
		{
			r3dRenderer->SetTex(NULL);
			r3dRenderer->SetMaterial(NULL);
			r3dRenderer->SetRenderingMode( R3D_BLEND_PUSH | R3D_BLEND_ADD | R3D_BLEND_ZC );

			void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
			int alpha = int(floorf(R3D_ABS(r3dSin(r3dGetTime()*6.0f))*255.0f));
			r3dDrawIcon3D(This->Parent->GetPosition() + r3dPoint3D(0,2,0), This->Parent->s_UAVFriendlyBlinkTex, r3dColor24(alpha,alpha,alpha, alpha), 18);

			r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
		}
	}

	obj_AI_Player* Parent ;
	typedef void (*MaterialModifyFlags)(r3dMaterial *mat);
	MaterialModifyFlags matFlagsModifier;

	char PlayerAuraType ;
	char Transparent ;
	char FirstPerson ;
};

//------------------------------------------------------------------------
struct PlayerShadowRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PlayerShadowRenderable* This = static_cast<PlayerShadowRenderable*>(RThis);

		bool draw_weapon = true;
		const Weapon* wpn = This->Parent->m_Weapons[This->Parent->m_SelectedWeapon];
		if(wpn && wpn->isGrenadeOrMineAnim())
		{
			if(This->Parent->uberAnim_->IsGrenadeLaunched())
				draw_weapon = false; // throwing grenade, so do not render grenade in hand
		}

		This->Parent->uberEquip_->Draw(This->Parent->GetSkeleton(), This->Parent->DrawFullMatrix, draw_weapon, CUberEquip::DT_SHADOWS, false );		
	}

	obj_AI_Player* Parent;
};

struct PlayerFlashRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PlayerFlashRenderable* This = static_cast<PlayerFlashRenderable*>( RThis );

		This->Parent->DrawLabel();
		This->Parent->DrawReticle();
	}

	obj_AI_Player* Parent;
};

struct PlayerDebugRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PlayerDebugRenderable* This = static_cast<PlayerDebugRenderable*>( RThis );
		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		This->Parent->GetSkeleton()->DrawSkeleton(Cam, This->Parent->GetPosition());

		/*if(This->Parent->explosionTime > 0)
		{
			This->Parent->explosionTime-=r3dGetFrameTime();
			r3dDrawSphereSolid(This->Parent->explosionCenter, This->Parent->explosionRadius, Cam, r3dColor::red);
		}*/
		//r3dDrawBoundBox(This->Parent->GetBBoxWorld(), Cam, r3dColor24::green/*debugColorLevels[m_Level]*/, 0.05f);
	}

	obj_AI_Player* Parent;
};

#define	RENDERABLE_AI_PLAYER_SORT_VALUE (2*RENDERABLE_USER_SORT_VALUE)

extern RenderArray	g_render_arrays[ rsCount ];

void
obj_AI_Player::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam )
{
	r3d_assert( &rarr - g_render_arrays - rsCreateSM < NumShadowSlices ) ;

	if( !m_Loaded )
		return ;

	// HACK: For the moment. 
#if VEHICLES_ENABLED
	if ( isInVehicle() ) 
	{
		return; 
	}
#endif

	int firstPerson = g_camera_mode->GetInt() == 2 && NetworkLocal && !bDead && !uavViewActive_;

	if(firstPerson) // do not render our own shadow in FPS mode (same as in BF3)
		return;

	if( !gDisableDynamicObjectShadows && &rarr - g_render_arrays - rsCreateSM != r_active_shadow_slices->GetInt() - 1 )
	{
		PlayerShadowRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_AI_PLAYER_SORT_VALUE;

		rarr.PushBack( rend );
	}
}

//////////////////////////////////////////////////////////////////////////

void SetTransparentCamoMatFlags(r3dMaterial *mat )
{
	mat->Flags |= R3D_MAT_TRANSPARENT | R3D_MAT_TRANSPARENT_CAMOUFLAGE ;
	if (strstr(mat->Name, "nocamo"))
		mat->Flags |= R3D_MAT_SKIP_DRAW;
}

void SetTransparentCamoMatFlags_FP(r3dMaterial *mat )
{
	mat->Flags |= R3D_MAT_TRANSPARENT | R3D_MAT_TRANSPARENT_CAMO_FP ;
	if (strstr(mat->Name, "nocamo"))
		mat->Flags |= R3D_MAT_SKIP_DRAW;
}

//////////////////////////////////////////////////////////////////////////

void UnsetTransparentCamoMatFlags(r3dMaterial *mat)
{
	mat->Flags &= ~(R3D_MAT_TRANSPARENT | R3D_MAT_TRANSPARENT_CAMOUFLAGE | R3D_MAT_TRANSPARENT_CAMO_FP |  R3D_MAT_SKIP_DRAW);
}

//////////////////////////////////////////////////////////////////////////
struct PlaceableConsumableItemRenderable : MeshDeferredRenderable
{
	void Init( int a_transparent )
	{
		DrawFunc = Draw;
		Transparent = a_transparent ;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PlaceableConsumableItemRenderable* This = static_cast< PlaceableConsumableItemRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts( This->Parent->m_ConsumableItemPlacementMeshVSConsts );

		int wasTransparent = 0 ;

		float oldTranspMultiplier = 1.0f ;

		bool hasMatForTrasnparency = This && This->Transparent && This->Mesh && This->Mesh->MatChunks[This->BatchIdx].Mat ;

		if ( hasMatForTrasnparency )
		{
			// because previous material may be the same
			// and thus it will filter our transparency hacks that follow
			r3dMaterial::ResetMaterialFilter() ;

			r3dMaterial* mat = This->Mesh->MatChunks[This->BatchIdx].Mat ;

			wasTransparent = mat->Flags & R3D_MAT_TRANSPARENT ;
			oldTranspMultiplier = mat->TransparencyMultiplier ;

			if( !wasTransparent )
				mat->TransparencyMultiplier = 0.75f ;

			mat->Flags |= R3D_MAT_TRANSPARENT ;

			SetLightsIfTransparent( mat, This->Parent->GetBBoxWorld() ) ;
		}

		if(This->Mesh->IsSkeletal())
			r3dSkeleton::SetDefaultShaderConstants();

		MeshDeferredRenderable::Draw( RThis, Cam );

		if ( hasMatForTrasnparency )
		{
			r3dMaterial* mat = This->Mesh->MatChunks[This->BatchIdx].Mat ;

			int& flags = mat->Flags ;
			flags &= ~R3D_MAT_TRANSPARENT ;
			flags |= wasTransparent ;

			mat->TransparencyMultiplier = oldTranspMultiplier ;
		}
	}

	obj_AI_Player* Parent ;
	int Transparent ;
};

//------------------------------------------------------------------------
// special callback to ignore terrain for beacon placement, as otherwise it is almost impossible to place beacon on uneven terrain
class beaconFilterCallback : public PxSceneQueryFilterCallback
{
public:
	beaconFilterCallback(){};

	virtual PxSceneQueryHitType::Enum preFilter(const PxFilterData& filterData, PxShape* shape, PxSceneQueryFilterFlags& filterFlags)
	{
		if(shape)
		{
			PxRigidActor& actor = shape->getActor();
			PhysicsCallbackObject* target = static_cast<PhysicsCallbackObject*>(actor.userData);
			if(target)
			{
				GameObject *gameObj = target->isGameObject();
				if(gameObj)
				{
					if(gameObj->isObjType(OBJTYPE_Terrain))
						return PxSceneQueryHitType::eNONE;
				}
			}
		}
		return PxSceneQueryHitType::eBLOCK;
	}

	virtual PxSceneQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxSceneQueryHit& hit)
	{
		// shouldn't be called!
		return PxSceneQueryHitType::eBLOCK;
	}
};


void
obj_AI_Player::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if( !m_Loaded )
		return ;

	if(!m_enableRendering)
		return;

	// HACK: For the moment. 
#if VEHICLES_ENABLED
	if ( isInVehicle() ) 
	{
		return; 
	}
#endif
	

	int firstPerson = g_camera_mode->GetInt() == 2 && NetworkLocal && !bDead && !uavViewActive_;

	if(firstPerson)
	{
		if(hasScopeMode() && CurrentRig.allowScope)
		{
			r_first_person_render_z_start->SetFloat(0.01f);
			r_first_person_render_z_end->SetFloat(0.7f);
		}
		else
		{
			r_first_person_render_z_start->SetFloat(0.1f);
			r_first_person_render_z_end->SetFloat(1.5f);
		}
	}

	// deferred
	if( !NetworkLocal || (NetworkLocal && (!g_CameraInsidePlayer || g_camera_mode->GetInt()==2) && !laserViewActive_) ) // do not render player if camera is inside of player
	{
		if(!(NetworkLocal && hasScopeMode() && hasScopeMode_hidePlayerModel() && CurrentRig.allowScope)) // do not render yourself in scope mode
		{
			bool transparentCamo = camoTimeLine.camoMasterTransparency < 1.0f;
			if (camoTimeLine.camoMasterTransparency > 0)
			{
				PlayerRenderable rend;

				rend.Init( AT_NONE, 0 );
				rend.Parent		= this;
				rend.SortValue	= RENDERABLE_AI_PLAYER_SORT_VALUE;
				rend.matFlagsModifier = &UnsetTransparentCamoMatFlags;
				rend.FirstPerson = firstPerson ;

				render_arrays[ firstPerson ? rsFillGBufferFirstPerson : rsFillGBuffer ].PushBack( rend );
			}
			
			if ( transparentCamo )
			{
				PlayerRenderable rend ;

				rend.Init( AT_NONE, 1 ) ;
				rend.Parent		= this ;
				rend.FirstPerson = firstPerson ;

				rend.matFlagsModifier = firstPerson ? 
											&SetTransparentCamoMatFlags_FP
												:
											&SetTransparentCamoMatFlags ;

				float len = ( Cam - GetPosition() ).Length() ;
				rend.SortValue	= RENDERABLE_EMITTER_USER_SORT_VALUE | GetRevIDist( len );

				render_arrays[ rsDrawTransparents ].PushBack( rend ) ;			
			}

			if ( m_AuraType != AT_NONE )
			{
				PlayerRenderable rend ;

				rend.Init( m_AuraType, 0 ) ;
				//rend.Init( AT_HEALING , 0 ) ;
				rend.Parent		= this ;
				rend.FirstPerson = firstPerson ;

				rend.matFlagsModifier = &UnsetTransparentCamoMatFlags;

				float len = ( Cam - GetPosition() ).Length() ;

				// render after tcamo hecne +1
				rend.SortValue	= RENDERABLE_EMITTER_USER_SORT_VALUE | ( GetRevIDist( len ) + 1 );

				render_arrays[ rsDrawTransparents ].PushBack( rend ) ;			
			}

			if(m_needToDrawRiotShieldPlacement)
			{
				r3dMesh* riotMesh = NULL;
				if(m_needToDrawRiotShieldPlacement == WeaponConfig::ITEMID_RiotShield)
				{
					if(m_RiotShieldMeshPlacement == NULL)
						m_RiotShieldMeshPlacement = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_01.sco", true, false, true, true );
					riotMesh = m_RiotShieldMeshPlacement;
				}
				else
				{
					if(m_RiotPersonalShieldMeshPlacement == NULL)
						m_RiotPersonalShieldMeshPlacement = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_02.sco", true, false, true, true );
					riotMesh = m_RiotPersonalShieldMeshPlacement;
				}
				if(riotMesh->IsDrawable())
				{
					r3dPoint3D *scale ;
					r3dPoint2D *texcScale ;
					scale = riotMesh->IsSkeletal() ? 0 : &riotMesh->unpackScale;
					texcScale = &riotMesh->texcUnpackScale ;

					D3DXMATRIX m1, m2, m3, world;
					r3dPoint3D pos = GetPosition() + GetvForw() * 1.0f;
					D3DXMatrixTranslation(&m1, pos.x, pos.y, pos.z);
					D3DXMatrixRotationY(&m2, R3D_PI);
					D3DXMatrixMultiply(&m3, &DrawRotMatrix, &m2);
					D3DXMatrixMultiply(&world, &m3, &m1);
					r3dPrepareMeshVSConsts(m_ConsumableItemPlacementMeshVSConsts, world, scale, texcScale );

					// check if we can place our riot shield. I don't want to do this in update, as anyway this function is called every frame, so should be fine
					{
						PxBoxGeometry bbox(riotMesh->localBBox.Size.x*0.5f, riotMesh->localBBox.Size.y*0.5f, riotMesh->localBBox.Size.z*0.5f);
						PxTransform pose(PxVec3(pos.x, pos.y+(riotMesh->localBBox.Size.y*0.5f)+0.5f, pos.z), PxQuat(0,0,0,1));
						PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
						PxShape* shape;
						m_canPlaceConsumableItem = !g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, shape, filter) && bOnGround;
						if(m_canPlaceConsumableItem) // check if we are trying to place it on top of usable item
						{
							bbox.halfExtents.y *= 4.0f;
							PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
							PxShape* results[64] = {0};
							int numRes = g_pPhysicsWorld->PhysXScene->overlapMultiple(bbox, pose, results, 64, filter);
							for(int i=0; i<numRes; ++i)
							{
								PhysicsCallbackObject* clbObj = NULL;
								if( results[i] && (clbObj = static_cast<PhysicsCallbackObject*>(results[i]->getActor().userData)))
								{
									GameObject* gameObj = clbObj->isGameObject();
									if(gameObj && gameObj->isObjType(OBJTYPE_GameplayItem))
									{
										m_canPlaceConsumableItem = false;
										break;
									}
								}
							}
						}
					}

					if( 0 )
					{
						uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
						riotMesh->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
						for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
							rend.Init( 0 );
							rend.Parent = this ;
						}
					}
					else
					{
						float dist = (Cam - GetPosition()).Length();

						uint32_t prevTranspCount = render_arrays[rsDrawTransparents].Count();

						riotMesh->AppendTransparentRenderables( render_arrays[rsDrawTransparents], m_canPlaceConsumableItem?r3dColor::white:r3dColor::red, dist, 1 );

						for( uint32_t i = prevTranspCount, e = render_arrays[rsDrawTransparents].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[rsDrawTransparents][ i ] ) ;
							rend.Init( 1 ) ;
							rend.Parent = this;
						}
					}
				}
			}
			if(m_needToDrawAutoTurretPlacement)
			{
				r3dMesh* turretMesh = NULL;
				{
					if(m_AutoTurretMeshPlacement== NULL)
						m_AutoTurretMeshPlacement = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\item_turret_01_m60.sco", true, false, true, true );
					turretMesh = m_AutoTurretMeshPlacement;
				}
				if(turretMesh->IsDrawable())
				{
					r3dPoint3D *scale ;
					r3dPoint2D *texcScale ;
					scale = turretMesh->IsSkeletal() ? 0 : &turretMesh->unpackScale;
					texcScale = &turretMesh->texcUnpackScale ;

					D3DXMATRIX m1, m2, m3, world;
					r3dPoint3D pos = GetPosition() + GetvForw() * 1.0f + r3dPoint3D(0,1,0);
					D3DXMatrixTranslation(&m1, pos.x, pos.y, pos.z);
					D3DXMatrixRotationY(&m2, R3D_PI);
					D3DXMatrixMultiply(&m3, &DrawRotMatrix, &m2);
					D3DXMatrixMultiply(&world, &m3, &m1);
					r3dPrepareMeshVSConsts(m_ConsumableItemPlacementMeshVSConsts, world, scale, texcScale );

					// check if we can place our riot shield. I don't want to do this in update, as anyway this function is called every frame, so should be fine
					{
						PxBoxGeometry bbox(turretMesh->localBBox.Size.x*0.5f, turretMesh->localBBox.Size.y*0.5f, turretMesh->localBBox.Size.z*0.5f);
						PxTransform pose(PxVec3(pos.x, pos.y+(turretMesh->localBBox.Size.y*0.5f)+0.5f - 1.0f, pos.z), PxQuat(0,0,0,1));
						PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
						PxShape* shape;
						m_canPlaceConsumableItem = !g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, shape, filter) && bOnGround;
						if(m_canPlaceConsumableItem) // check if we are trying to place it on top of usable item
						{
							bbox.halfExtents.y *= 4.0f;
							PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
							PxShape* results[64] = {0};
							int numRes = g_pPhysicsWorld->PhysXScene->overlapMultiple(bbox, pose, results, 64, filter);
							for(int i=0; i<numRes; ++i)
							{
								PhysicsCallbackObject* clbObj = NULL;
								if( results[i] && (clbObj = static_cast<PhysicsCallbackObject*>(results[i]->getActor().userData)))
								{
									GameObject* gameObj = clbObj->isGameObject();
									if(gameObj && gameObj->isObjType(OBJTYPE_GameplayItem))
									{
										m_canPlaceConsumableItem = false;
										break;
									}
								}
							}
						}
					}

					if( 0 )
					{
						uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
						turretMesh->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
						for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
							rend.Init( 0 );
							rend.Parent = this ;
						}
					}
					else
					{
						float dist = (Cam - GetPosition()).Length();

						uint32_t prevTranspCount = render_arrays[rsDrawTransparents].Count();

						turretMesh->AppendTransparentRenderables( render_arrays[rsDrawTransparents], m_canPlaceConsumableItem?r3dColor::white:r3dColor::red, dist, 1 );

						for( uint32_t i = prevTranspCount, e = render_arrays[rsDrawTransparents].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[rsDrawTransparents][ i ] ) ;
							rend.Init( 1 ) ;
							rend.Parent = this;
						}
					}
				}
			}
			if(m_needToDrawRespawnBeaconPlacement)
			{
				r3dMesh* beaconMesh = NULL;
				{
					if(m_RespawnBeaconMeshPlacement== NULL)
						m_RespawnBeaconMeshPlacement = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_RespawnBeacon_01.sco", true, false, true, true );
					beaconMesh = m_RespawnBeaconMeshPlacement;
				}
				if(beaconMesh->IsDrawable())
				{
					r3dPoint3D *scale ;
					r3dPoint2D *texcScale ;
					scale = beaconMesh->IsSkeletal() ? 0 : &beaconMesh->unpackScale;
					texcScale = &beaconMesh->texcUnpackScale ;

					D3DXMATRIX m1, m2, m3, world;
					r3dPoint3D pos = GetPosition() + GetvForw() * 1.0f;
					D3DXMatrixTranslation(&m1, pos.x, pos.y, pos.z);
					D3DXMatrixRotationY(&m2, R3D_PI);
					D3DXMatrixMultiply(&m3, &DrawRotMatrix, &m2);
					D3DXMatrixMultiply(&world, &m3, &m1);
					r3dPrepareMeshVSConsts(m_ConsumableItemPlacementMeshVSConsts, world, scale, texcScale );

					// check if we can place our riot shield. I don't want to do this in update, as anyway this function is called every frame, so should be fine
					{
						PxBoxGeometry bbox(1.0f, 1.0f, 1.0f); // make bbox big enough to prevent players from spawning inside of geometry
						PxTransform pose(PxVec3(pos.x, pos.y+1.0f, pos.z), PxQuat(0,0,0,1));
						PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::ePREFILTER);
						PxShape* shape;
						beaconFilterCallback filterCallback;
						m_canPlaceConsumableItem = !g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, shape, filter, &filterCallback) && bOnGround;
						if(m_canPlaceConsumableItem) // check if we are trying to place it on top of usable item
						{
							bbox.halfExtents.y *= 4.0f;
							PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
							PxShape* results[64] = {0};
							int numRes = g_pPhysicsWorld->PhysXScene->overlapMultiple(bbox, pose, results, 64, filter);
							for(int i=0; i<numRes; ++i)
							{
								PhysicsCallbackObject* clbObj = NULL;
								if( results[i] && (clbObj = static_cast<PhysicsCallbackObject*>(results[i]->getActor().userData)))
								{
									GameObject* gameObj = clbObj->isGameObject();
									if(gameObj && gameObj->isObjType(OBJTYPE_GameplayItem))
									{
										m_canPlaceConsumableItem = false;
										break;
									}
								}
							}
						}
					}

					if( 0 )
					{
						uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
						beaconMesh->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
						for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
							rend.Init( 0 );
							rend.Parent = this ;
						}
					}
					else
					{
						float dist = (Cam - GetPosition()).Length();

						uint32_t prevTranspCount = render_arrays[rsDrawTransparents].Count();

						beaconMesh->AppendTransparentRenderables( render_arrays[rsDrawTransparents], m_canPlaceConsumableItem?r3dColor::white:r3dColor::red, dist, 1 );

						for( uint32_t i = prevTranspCount, e = render_arrays[rsDrawTransparents].Count(); i < e; i ++ )
						{
							PlaceableConsumableItemRenderable& rend = static_cast<PlaceableConsumableItemRenderable&>( render_arrays[rsDrawTransparents][ i ] ) ;
							rend.Init( 1 ) ;
							rend.Parent = this;
						}
					}
				}
			}		
		}
	}

	// debug
	{
		PlayerDebugRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_AI_PLAYER_SORT_VALUE;

		render_arrays[ rsDrawBoundBox ].PushBack( rend );
	}

	// Flash UI
#ifndef FINAL_BUILD
	if((!d_video_spectator_mode->GetBool() && !d_disable_game_hud->GetBool() && !d_observer_mode->GetBool()) || d_force_scope_rendering->GetBool())
#endif
	{
		PlayerFlashRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_AI_PLAYER_SORT_VALUE;

		render_arrays[ rsDrawFlashUI ].PushBack( rend );
	}
}

void obj_AI_Player::UpdateTransform()
{
	GameObject::UpdateTransform();
}

//////////////////////////////////////////////////////////////////////////
void obj_AI_Player::stopFootstepsSound()
{
	if (SoundSys.IsHandleValid(footStepsSnd))
	{
		SoundSys.Stop(footStepsSnd);
	}
}

void obj_AI_Player::UpdateFootsteps()
{
	R3DPROFILE_FUNCTION("UpdateFootsteps");
	R3DPROFILE_START("RayCast");
	// cast ray to see on what material player is standing
	PhysicsCallbackObject* target = NULL;
	const MaterialType *mt = 0;

	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));

	// did we hit anything?
	bool hitResult = g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y + 0.5f, GetPosition().z), PxVec3(0, -1, 0), 1.0f, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter);
	if( hitResult )
	{
		if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
		{
			r3dMaterial* material = 0;
			GameObject *gameObj = target->isGameObject();
			if(gameObj)
			{
				if( gameObj->isObjType( OBJTYPE_Mesh ) )
					material = static_cast< MeshGameObject* > ( target )->MeshLOD[ 0 ]->GetFaceMaterial( hit.faceIndex );
				if(gameObj->isObjType(OBJTYPE_Terrain))
				{
					mt = Terrain->GetMaterialType(GetPosition());
				}
			}
			else if(target->hasMesh())
			{
				material = target->hasMesh()->GetFaceMaterial( hit.faceIndex );
			}
			if (!material)
				material = target->GetMaterial(hit.faceIndex);

			if(material && !mt) // get material type from mesh material
			{
				mt = g_pMaterialTypes->Get(r3dHash::MakeHash(material->TypeName));
			}
		}
	}
	R3DPROFILE_END("RayCast");

	{
		R3DPROFILE_FUNCTION("play sound");
		// hack: right now we have a problem with faceID in raycast not matching faceID of the actual mesh (collision mesh vs regular mesh). 
		// So, in case if we failed to find a proper material because of that, let's just use old foot step sound and that's it
		int fts = currentFootStepsSoundID; 
		if(mt && mt->footStepsSound>=0)
		{
			fts = mt->footStepsSound;
		}
		if (!SoundSys.IsHandleValid(footStepsSnd) || fts != currentFootStepsSoundID)
		{
			SoundSys.Stop(footStepsSnd);
			footStepsSnd = NULL;
			
			for(uint32_t i=0; i<footStepsArr.Count(); ++i) 
			{
				if(footStepsArr[i].id == fts)
				{
					footStepsSnd = footStepsArr[i].snd;
					// check if handle is valid, sometimes it becomes invalid for some reason
					if(!SoundSys.IsHandleValid(footStepsSnd))
					{
						// handle is broken, remove it from the list, and remove it from FMOD
						SoundSys.Stop(footStepsArr[i].snd);
						SoundSys.Release(footStepsArr[i].snd);
						footStepsArr.Erase(i);	
						footStepsSnd = NULL;
					}
					break;
				}
			}
			
			if(footStepsSnd == NULL)
			{
				footStepsSnd = SoundSys.Play(fts, GetPosition());
				if (!SoundSys.IsHandleValid(footStepsSnd))
					return;
				footStepsArr.PushBack(footStepEntry_s(fts, footStepsSnd));
			}

			currentFootStepsSoundID = fts;
		}

		SoundSys.Set3DAttributes(footStepsSnd, &GetPosition(), 0, 0);
		
		static const float SpawnRatesByState[] = {
			0,	//PLAYER_IDLE,
			0,	//PLAYER_IDLEAIM,
			0.76f,	//PLAYER_MOVE_CROUCH,
			0.54f,	//PLAYER_MOVE_CROUCH_AIM,
			0.58f,	//PLAYER_MOVE_WALK_AIM,
			1.0f,	//PLAYER_MOVE_RUN,
			1.17f,	//PLAYER_MOVE_SPRINT,
			0,	//PLAYER_DIE,
		};
		TL_STATIC_ASSERT( R3D_ARRAYSIZE(SpawnRatesByState) == PLAYER_NUM_STATES ) ;
		float spawnRate = SpawnRatesByState[PlayerState];
		if(PlayerMoveDir == CUberData::ANIMDIR_Stand || hitResult == false )
			spawnRate = 0.0f;

		if(spawnRate > 0)
		{
			footsteps_timeSinceLastMissingHitResult = r3dGetTime();
			if (SoundSys.GetState(footStepsSnd) != FMOD_EVENT_STATE_PLAYING)
			{
				SoundSys.Start(footStepsSnd);
			}

			SoundSys.SetProperty(footStepsSnd, FMOD_EVENTPROPERTY_SPAWNINTENSITY, &spawnRate);
		}
		else
		{
			if((r3dGetTime() - footsteps_timeSinceLastMissingHitResult)>0.25f) // workaround when you are running across wooden planks and sounds restart each time you step on new wooden plank
				SoundSys.Stop(footStepsSnd);
		}
	}
}


void obj_AI_Player::SetTransparentCamouflage(float alpha)
{
	if (uberEquip_)
	{
		camoTimeLine.SetNewCamoTarget(alpha);
	}

	if( alpha < camoTimeLine.transparentCamoAlpha )
	{
		camoTimeLine.transparencyOnShock = 1.0f ;
	}
}

void obj_AI_Player::updateWeaponUI()
{
	if(!hudMain)
		return;

	hudMain->removeAllWeapons();
	if(m_Weapons[0])
		hudMain->AddNewWeapon(0, m_Weapons[0]->getStoreName(), m_Weapons[0]->getStoreIcon(), m_Weapons[0]->getConfig()->category, m_Weapons[0]->getFiremode());
	if(m_Weapons[1])
		hudMain->AddNewWeapon(1, m_Weapons[1]->getStoreName(), m_Weapons[1]->getStoreIcon(), m_Weapons[1]->getConfig()->category, m_Weapons[1]->getFiremode());
	if(m_Weapons[2])
		hudMain->AddNewWeapon(2, m_Weapons[2]->getStoreName(), m_Weapons[2]->getStoreIcon(), m_Weapons[2]->getConfig()->category, m_Weapons[2]->getFiremode());

	if(m_Weapons[3])
		hudMain->AddNewWeapon(3, m_Weapons[3]->getStoreName(), m_Weapons[3]->getStoreIcon(), m_Weapons[3]->getConfig()->category, m_Weapons[3]->getFiremode());
	else
		hudMain->AddNewWeapon(3, NULL, "", storecat_INVALID, WPN_FRM_SINGLE);

	if(m_Weapons[4])
		hudMain->AddNewWeapon(4, m_Weapons[4]->getStoreName(), m_Weapons[4]->getStoreIcon(), m_Weapons[4]->getConfig()->category, m_Weapons[4]->getFiremode());
	else
		hudMain->AddNewWeapon(4, NULL, "", storecat_INVALID, WPN_FRM_SINGLE);

	if(m_Weapons[5])
		hudMain->AddNewWeapon(5, m_Weapons[5]->getStoreName(), m_Weapons[5]->getStoreIcon(), m_Weapons[5]->getConfig()->category, m_Weapons[5]->getFiremode());
	else
		hudMain->AddNewWeapon(5, NULL, "", storecat_INVALID, WPN_FRM_SINGLE);

	if(m_Weapons[6])
		hudMain->AddNewWeapon(6, m_Weapons[6]->getStoreName(), m_Weapons[6]->getStoreIcon(), m_Weapons[6]->getConfig()->category, m_Weapons[6]->getFiremode());
	else
		hudMain->AddNewWeapon(6, NULL, "", storecat_INVALID, WPN_FRM_SINGLE);

	hudMain->setCurrentWeapon(m_SelectedWeapon);
	if(m_Weapons[m_SelectedWeapon])
	{
		hudMain->showUseItem(false);
		hudMain->setFireMode(m_Weapons[m_SelectedWeapon]->getFiremode());
	}
	else
		hudMain->showUseItem(true);

	hudMain->showReticle(true);
	hudMain->setReticleColor("white");
}

float obj_AI_Player::getPlayerHeightForCamera() const
{
	// ptumik: not working :(
//	if(PlayerState == PLAYER_MOVE_CROUCH || PlayerState == PLAYER_MOVE_CROUCH_AIM)
//		return 1.0f;
	
	return Height;
}

//////////////////////////////////////////////////////////////////////////

void obj_AI_Player::DoRagdoll(bool toRagdoll, int hitBone, const r3dPoint3D* impulse)
{
#if ENABLE_RAGDOLL
	if (m_PhysSkeleton)
	{
		if (impulse)
		{
			m_PhysSkeleton->SwitchToRagdollWithForce(toRagdoll, hitBone, *impulse);
		}
		else
		{
			m_PhysSkeleton->SwitchToRagdoll(toRagdoll);
		}

		if (!m_PhysSkeleton->IsRagdollMode())
		{
			SetBBoxLocal(m_OrigBBox);
		}
    }
#endif
}

void obj_AI_Player::OnGameEnded()
{
	if(m_SelectedWeapon >= 0 && m_Weapons[m_SelectedWeapon] )
	{
		// currently we only need to do this to the equipped weapon, it can be expanded as needed.
		m_Weapons[m_SelectedWeapon]->OnGameEnded();
	}
}

#if VEHICLES_ENABLED
obj_Vehicle* obj_AI_Player::canEnterVehicle()
{
	// Fast version from Frank
	for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
	{
		if( obj->ObjTypeFlags & OBJTYPE_Vehicle )
		{
			obj_Vehicle* vehicle= static_cast< obj_Vehicle* > ( obj );

			float distSq = (GetPosition() - vehicle->GetPosition()).LengthSq();
			if( distSq < gDist_To_Allow_Vehicle_Entry * gDist_To_Allow_Vehicle_Entry )
			{
				return vehicle;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void obj_AI_Player::TogglePhysicsSimulation(bool on)
{
	if (PhysicsObject)
	{
		PxActor *a = PhysicsObject->getPhysicsActor();
		if (a)
		{
			on ? g_pPhysicsWorld->AddActor(*a) : g_pPhysicsWorld->RemoveActor(*a);
		}
		if (m_PhysSkeleton)
		{
			m_PhysSkeleton->TogglePhysicsSimulation(on);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_AI_Player::enterVehicle( obj_Vehicle* target_Vehicle )
{
	// Fast version from Frank
	target_Vehicle->SwitchToDrivable( true );
	vehicleViewActive_ = VehicleView_Driver; 
	d_drive_vehicles->SetBool( true ); // to gain control currently. 
	// HACK: bash this currently. 
	target_Vehicle->NetworkLocal = true;

	//	Exclude player physics shapes from simulation
	TogglePhysicsSimulation(false);
}

void obj_AI_Player::exitVehicle()
{

	// first let's look for a good location
	obj_Vehicle* currentCar = g_pPhysicsWorld->m_VehicleManager->getRealDrivenVehicle();
	r3d_assert(currentCar);


	r3dVector rotationVector = currentCar->GetRotationVector();
	r3dMatrix Matrix;
	// NOTE: We might want to take out rotation in the x or z direction and only focus on the Y direction... 
	r3dBuildRotationMatrix(Matrix, rotationVector );

	bool success = false;
	for ( int exitIndex = 0; exitIndex < 4; exitIndex++  )
	{
		r3dVector exitPosition;
		bool returnValue = currentCar->getExitSpace( exitPosition, exitIndex );
		r3d_assert( returnValue );

		exitPosition *= Matrix;
		r3dVector targetPosition = currentCar->GetPosition() + exitPosition; 

		if ( TestExitPosition( targetPosition) )
		{
			TeleportPlayer( targetPosition, "Exiting vehicle");
			success = true;
			break;
		}
	}

	if( success == false )
	{
		// don't know the height right now. 
		r3dVector targetPosition = currentCar->GetPosition() + r3dVector( 0, 3, 0 );
		if ( TestExitPosition( targetPosition, true ) )
		{
			TeleportPlayer( targetPosition, "Exiting vehicle");
			success = true;
		}
	}

	if( success == true ) 
	{
		// Fast version from Frank
		vehicleViewActive_ = VehicleView_None; 
		d_drive_vehicles->SetBool( false ); // to release control currently. 
		TogglePhysicsSimulation(true);
	}
	
}

bool obj_AI_Player::isDriving()
{
	return vehicleViewActive_ == VehicleView_Driver;
}

bool obj_AI_Player::isInVehicle() 
{
	return vehicleViewActive_ != VehicleView_None;
}

bool obj_AI_Player::TestExitPosition( r3dVector& exitPosition, bool ignoreGround)
{

	// snap to ground
	if ( ignoreGround == false ) {
		// first find out if there's ground. 
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(exitPosition.x, exitPosition.y+1, exitPosition.z), PxVec3(0, -1, 0), 2, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			exitPosition.x = hit.impact.x;
			exitPosition.y = hit.impact.y;
			exitPosition.z = hit.impact.z;
		} 
		else 
		{
			return false;
		}
	}

	// check spawn area. 
	PxShape* overlapHit = NULL;
	PxU32 collisionFlag = COLLIDABLE_STATIC_MASK ;
	PxBoxGeometry playerBox( 0.2f, 0.9f, 0.2f) ;
	PxTransform pose(PxVec3(exitPosition.x, exitPosition.y +1.1f, exitPosition.z), PxQuat(0,0,0,1));
	PxSceneQueryFilterData capsulefilter(PxFilterData(collisionFlag, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
	if ( g_pPhysicsWorld->PhysXScene->overlapAny( playerBox, pose, overlapHit, capsulefilter))
	{
		return false;
	}

	return true;
}

#endif


// this weird function (made by our friend Frank) is used to return true only for grenades that is out of ammo.
bool obj_AI_Player::isWeaponGrenadeOutofAmmo(int weaponIndex)
{
	r3d_assert(weaponIndex >= 0 && weaponIndex < NUM_WEAPONS_ON_PLAYER);
	const Weapon* wpn = m_Weapons[weaponIndex];

	if(wpn->getCategory() == storecat_GRENADES)
	{
		if(wpn->getNumBulletsLeft() == 0 && !wpn->m_needDelayedAction)
			return true;
	}
	if(wpn->isUsableItem())
	{
		if(wpn->m_usableItemInventoryIdx >= 0)
		{
			int numCharges = gUserProfile.ProfileData.Inventory[wpn->m_usableItemInventoryIdx].quantity;
			if(numCharges == 0)
				return true;
		}
	}
		
	return false;
}

void obj_AI_Player::setHasSabotageBomb(bool set)
{
	m_hasBomb = set;
	if(NetworkLocal && hudMain)
		hudMain->SetYouHaveBomb(set);
}

//////////////////////////////////////////////////////////////////////////

void ProcessCharacterEditor(obj_AI_Player* pl, float left, float top, float height);
void ProcessCharacterEditor()
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(EditorGameHUD::editorPlayerId);
	if (!pl)
	{
		imgui_Static(r3dRenderer->ScreenW-375, 85, "Spawn player firstly by pressing F8");
		return;
	}

	ProcessCharacterEditor(pl, 100, 100, 150);
}

void ProcessCharacterEditor(obj_AI_Player* pl, float left, float top, float height)
{
	stringlist_t weaponDB_nameList;
	uint32_t* weaponDB_idList = 0;

	stringlist_t attachmentDB_muzzle_nameList;
	uint32_t* attachmentDB_muzzle_idList = 0;
	stringlist_t attachmentDB_upperRail_nameList;
	uint32_t* attachmentDB_upperRail_idList = 0;
	stringlist_t attachmentDB_leftRail_nameList;
	uint32_t* attachmentDB_leftRail_idList = 0;
	stringlist_t attachmentDB_bottomRail_nameList;
	uint32_t* attachmentDB_bottomRail_idList = 0;
	stringlist_t attachmentDB_clip_nameList;
	uint32_t* attachmentDB_clip_idList = 0;

	static int selectedPrimaryWeaponIndex = -1;
	static int selectedSecondaryWeaponIndex = -1;
	static int selectedSideWeaponIndex = -1;
	static int selectedBodyIndex = 0;
	static int selectedArmorIndex = 0;
	static int selectedHeadIndex = 0;
	static int selectedHeadArmorIndex = 0;

	stringlist_t armorDBnames, bodyDBnames, headBDnames, headarmorBDnames;
	uint32_t *armorDBids = 0, *bodyDBids = 0, *headDBids = 0, *headarmorDBids = 0;
	if(weaponDB_nameList.empty()) // fill in list
	{
		weaponDB_nameList.push_back("empty");
		attachmentDB_muzzle_nameList.push_back("empty");
		attachmentDB_upperRail_nameList.push_back("empty");
		attachmentDB_leftRail_nameList.push_back("empty");
		attachmentDB_bottomRail_nameList.push_back("empty");
		attachmentDB_clip_nameList.push_back("empty");

		// sort item names there.
		struct CWpnNameSort {
			static int sort(const void* P1, const void* P2)	{
				const WeaponConfig* g1 = *(const WeaponConfig**)P1;
				const WeaponConfig* g2 = *(const WeaponConfig**)P2;
				return strcmp(g1->m_StoreIcon, g2->m_StoreIcon);
			}
		};
		struct CWpnAttachmentNameSort {
			static int sort(const void* P1, const void* P2)	{
				const WeaponAttachmentConfig* g1 = *(const WeaponAttachmentConfig**)P1;
				const WeaponAttachmentConfig* g2 = *(const WeaponAttachmentConfig**)P2;
				return strcmp(g1->m_StoreIcon, g2->m_StoreIcon);
			}
		};
		std::vector<const WeaponConfig*> allWpns;
		for(int i=0; i<gWeaponArmory.getNumWeapons(); ++i) {
			allWpns.push_back(gWeaponArmory.getWeaponConfigByIndex(i));
		}
		qsort(&allWpns[0], allWpns.size(), sizeof(WeaponConfig*), CWpnNameSort::sort);

		int numItemsInWeaponList = gWeaponArmory.getNumWeapons() + 1;
		weaponDB_idList = new uint32_t[numItemsInWeaponList];
		weaponDB_idList[0] = -1;
		for(int i=1; i<numItemsInWeaponList; ++i)
		{
			const WeaponConfig* config = allWpns[i-1]; //gWeaponArmory.getWeaponConfigByIndex(i-1);

			// get item FNAME without .dds
			char wname[256];
			sprintf(wname, strrchr(config->m_StoreIcon, '/') + 1);
			wname[strlen(wname)-4] = 0;

			weaponDB_nameList.push_back(wname);
			weaponDB_idList[i] = config->m_itemID;

			if(selectedPrimaryWeaponIndex==-1 && config->m_itemID == pl->CurLoadout.PrimaryWeaponID)
				selectedPrimaryWeaponIndex = i;
			if(selectedSecondaryWeaponIndex==-1 && config->m_itemID == pl->CurLoadout.SecondaryWeaponID)
				selectedSecondaryWeaponIndex = i;
			if(selectedSideWeaponIndex==-1 && config->m_itemID == pl->CurLoadout.SidearmWeaponID)
				selectedSideWeaponIndex = i;
		}

		if(selectedPrimaryWeaponIndex==-1)
			selectedPrimaryWeaponIndex = 0;
		if(selectedSecondaryWeaponIndex==-1)
			selectedSecondaryWeaponIndex = 0;
		if(selectedSideWeaponIndex==-1)
			selectedSideWeaponIndex = 0;

		int numItemsInAttachmentList = gWeaponArmory.getNumAttachments() + 1;
		attachmentDB_muzzle_idList = new uint32_t[numItemsInAttachmentList]; int numMuzzles = 0;
		attachmentDB_muzzle_idList[0] = 0;
		attachmentDB_upperRail_idList = new uint32_t[numItemsInAttachmentList]; int numUpperRails = 0;
		attachmentDB_upperRail_idList[0] = 0;
		attachmentDB_leftRail_idList = new uint32_t[numItemsInAttachmentList]; int numLeftRails = 0;
		attachmentDB_leftRail_idList[0] = 0;
		attachmentDB_bottomRail_idList = new uint32_t[numItemsInAttachmentList]; int numBottomRails = 0;
		attachmentDB_bottomRail_idList[0] = 0;
		attachmentDB_clip_idList = new uint32_t[numItemsInAttachmentList]; int numClips = 0;
		attachmentDB_clip_idList[0] = 0;
		for(int i=1; i<numItemsInAttachmentList; ++i)
		{
			const WeaponAttachmentConfig* config = gWeaponArmory.getAttachmentConfigByIndex(i-1);
			switch(config->m_type)
			{
			case WPN_ATTM_MUZZLE:
				attachmentDB_muzzle_nameList.push_back(config->m_StoreName);
				attachmentDB_muzzle_idList[++numMuzzles] = config->m_itemID;
				break;
			case WPN_ATTM_UPPER_RAIL:
				attachmentDB_upperRail_nameList.push_back(config->m_StoreName);
				attachmentDB_upperRail_idList[++numUpperRails] = config->m_itemID;
				break;
			case WPN_ATTM_LEFT_RAIL:
				attachmentDB_leftRail_nameList.push_back(config->m_StoreName);
				attachmentDB_leftRail_idList[++numLeftRails] = config->m_itemID;
				break;
			case WPN_ATTM_BOTTOM_RAIL:
				attachmentDB_bottomRail_nameList.push_back(config->m_StoreName);
				attachmentDB_bottomRail_idList[++numBottomRails] = config->m_itemID;
				break;
			case WPN_ATTM_CLIP:
				attachmentDB_clip_nameList.push_back(config->m_StoreName);
				attachmentDB_clip_idList[++numClips] = config->m_itemID;
				break;
			default:
				break;
			}
		}

		int numItemsInGearList = gWeaponArmory.getNumGears() + 1;
		armorDBids = new uint32_t[numItemsInGearList]; int numArmor = 0;
		bodyDBids = new uint32_t[numItemsInGearList]; int numBody = 0;
		headDBids = new uint32_t[numItemsInGearList]; int numHead = 0;
		headarmorDBids = new uint32_t[numItemsInGearList]; int numHeadarmor = 0;

		headarmorBDnames.push_back("empty");
		headBDnames.push_back("empty");
		armorDBnames.push_back("empty");
		bodyDBnames.push_back("empty");
		armorDBids[0] = 0;
		bodyDBids[0] = 0; 
		headDBids[0] = 0;
		headarmorDBids[0] = 0;
		for(int i=1; i<numItemsInGearList; ++i)
		{
			const GearConfig* config = gWeaponArmory.getGearConfigByIndex(i-1);
			switch(config->category)
			{
			case storecat_Characters:
			case storecat_Heroes:
				bodyDBnames.push_back(config->m_StoreName);
				bodyDBids[++numBody] = config->m_itemID;
				if(selectedBodyIndex==-1 && config->m_itemID == pl->CurLoadout.BodyMeshID)
					selectedBodyIndex = numBody;
				break;
			case storecat_Gear:
				armorDBnames.push_back(config->m_StoreName);
				armorDBids[++numArmor] = config->m_itemID;
				if(selectedArmorIndex==-1 && config->m_itemID == pl->CurLoadout.BodyArmorID)
					selectedArmorIndex = numArmor;
				break;
			case storecat_Heads:
				headBDnames.push_back(config->m_StoreName);
				headDBids[++numHead] = config->m_itemID;
				if(selectedHeadIndex==-1 && config->m_itemID == pl->CurLoadout.BodyHeadID)
					selectedHeadIndex = numHead;
				break;
			case storecat_HeadGear:
				headarmorBDnames.push_back(config->m_StoreName);
				headarmorDBids[++numHeadarmor] = config->m_itemID;
				if(selectedHeadArmorIndex==-1 && config->m_itemID == pl->CurLoadout.BodyHeadGearID)
					selectedHeadArmorIndex = numHeadarmor;
				break;
			default:
				break;
			}
		}
	}

	static const int width = 150;
//	static const int height = 150;
	static const int shift = 25;

	static int selectedPrimaryWeapon_Muzzle = 0;
	static int selectedPrimaryWeapon_UpperRail = 0;
	static int selectedPrimaryWeapon_LeftRail = 0;
	static int selectedPrimaryWeapon_BottomRail= 0;
	static int selectedPrimaryWeapon_Clip = 0;

	static int selectedSecondaryWeapon_Muzzle = 0;
	static int selectedSecondaryWeapon_UpperRail = 0;
	static int selectedSecondaryWeapon_LeftRail = 0;
	static int selectedSecondaryWeapon_BottomRail= 0;
	static int selectedSecondaryWeapon_Clip = 0;

	static int selectedSidearmWeapon_Muzzle = 0;
	static int selectedSidearmWeapon_UpperRail = 0;
	static int selectedSidearmWeapon_LeftRail = 0;
	static int selectedSidearmWeapon_BottomRail= 0;
	static int selectedSidearmWeapon_Clip = 0;

	float offset = top + shift + height - 20;
	{
		static float primWeaponOffset = 0;
		imgui_Static(left + width * 6, top, "Primary Weapon", width);
		imgui_DrawList(left + width * 6, top + shift, (float)width, (float)height, weaponDB_nameList, &primWeaponOffset, &selectedPrimaryWeaponIndex);

		if(g_camera_mode->GetInt() == 2)
		{
			static float primWeaponAttachmentMuzzleOffset = 0;
			imgui_Static(left + width * 6, offset, "Muzzle", width);
			imgui_DrawList(left + width * 6, offset + 20, (float)width, (float)100.0f, attachmentDB_muzzle_nameList, &primWeaponAttachmentMuzzleOffset, &selectedPrimaryWeapon_Muzzle);

			static float primWeaponAttachmentUpperRailOffset = 0;
			imgui_Static(left + width * 6, offset + 100, "Upper Rail", width);
			imgui_DrawList(left + width * 6, offset + 20 + 100, (float)width, (float)100.0f, attachmentDB_upperRail_nameList, &primWeaponAttachmentUpperRailOffset, &selectedPrimaryWeapon_UpperRail);

			static float primWeaponAttachmentLeftRailOffset = 0;
			imgui_Static(left + width * 6, offset + 200, "Left Rail", width);
			imgui_DrawList(left + width * 6, offset + 20 + 200, (float)width, (float)100.0f, attachmentDB_leftRail_nameList, &primWeaponAttachmentLeftRailOffset, &selectedPrimaryWeapon_LeftRail);

			static float primWeaponAttachmentBottomRailOffset = 0;
			imgui_Static(left + width * 6, offset + 300, "Bottom Rail", width);
			imgui_DrawList(left + width * 6, offset + 20 + 300, (float)width, (float)100.0f, attachmentDB_bottomRail_nameList, &primWeaponAttachmentBottomRailOffset, &selectedPrimaryWeapon_BottomRail);

			static float primWeaponAttachmentClipOffset = 0;
			imgui_Static(left + width * 6, offset + 400, "Clip", width);
			imgui_DrawList(left + width * 6, offset + 20 + 400, (float)width, (float)100.0f, attachmentDB_clip_nameList, &primWeaponAttachmentClipOffset, &selectedPrimaryWeapon_Clip);
		}
	}

	{
		static float secWeaponOffset = 0;
		imgui_Static(left + width * 5, top, "Secondary Weapon", width);
		imgui_DrawList(left + width * 5, top + shift, (float)width, (float)height, weaponDB_nameList, &secWeaponOffset, &selectedSecondaryWeaponIndex);

		if(g_camera_mode->GetInt() == 2)
		{
			static float primWeaponAttachmentMuzzleOffset = 0;
			imgui_Static(left + width * 5, offset, "Muzzle", width);
			imgui_DrawList(left + width * 5, offset + 20, (float)width, (float)100.0f, attachmentDB_muzzle_nameList, &primWeaponAttachmentMuzzleOffset, &selectedSecondaryWeapon_Muzzle);

			static float primWeaponAttachmentUpperRailOffset = 0;
			imgui_Static(left + width * 5, offset + 100, "Upper Rail", width);
			imgui_DrawList(left + width * 5, offset + 20 + 100, (float)width, (float)100.0f, attachmentDB_upperRail_nameList, &primWeaponAttachmentUpperRailOffset, &selectedSecondaryWeapon_UpperRail);

			static float primWeaponAttachmentLeftRailOffset = 0;
			imgui_Static(left + width * 5, offset + 200, "Left Rail", width);
			imgui_DrawList(left + width * 5, offset + 20 + 200, (float)width, (float)100.0f, attachmentDB_leftRail_nameList, &primWeaponAttachmentLeftRailOffset, &selectedSecondaryWeapon_LeftRail);

			static float primWeaponAttachmentBottomRailOffset = 0;
			imgui_Static(left + width * 5, offset + 300, "Bottom Rail", width);
			imgui_DrawList(left + width * 5, offset + 20 + 300, (float)width, (float)100.0f, attachmentDB_bottomRail_nameList, &primWeaponAttachmentBottomRailOffset, &selectedSecondaryWeapon_BottomRail);

			static float primWeaponAttachmentClipOffset = 0;
			imgui_Static(left + width * 5, offset + 400, "Clip", width);
			imgui_DrawList(left + width * 5, offset + 20 + 400, (float)width, (float)100.0f, attachmentDB_clip_nameList, &primWeaponAttachmentClipOffset, &selectedSecondaryWeapon_Clip);
		}
	}

	{
		static float sideWeaponOffset = 0;
		imgui_Static(left + width * 4, top, "Side Weapon", width);
		imgui_DrawList(left + width * 4, top + shift, (float)width, (float)height, weaponDB_nameList, &sideWeaponOffset, &selectedSideWeaponIndex);

		if(g_camera_mode->GetInt() == 2)
		{
			static float primWeaponAttachmentMuzzleOffset = 0;
			imgui_Static(left + width * 4, offset, "Muzzle", width);
			imgui_DrawList(left + width * 4, offset + 20, (float)width, (float)100.0f, attachmentDB_muzzle_nameList, &primWeaponAttachmentMuzzleOffset, &selectedSidearmWeapon_Muzzle);

			static float primWeaponAttachmentUpperRailOffset = 0;
			imgui_Static(left + width * 4, offset + 100, "Upper Rail", width);
			imgui_DrawList(left + width * 4, offset + 20 + 100, (float)width, (float)100.0f, attachmentDB_upperRail_nameList, &primWeaponAttachmentUpperRailOffset, &selectedSidearmWeapon_UpperRail);

			static float primWeaponAttachmentLeftRailOffset = 0;
			imgui_Static(left + width * 4, offset + 200, "Left Rail", width);
			imgui_DrawList(left + width * 4, offset + 20 + 200, (float)width, (float)100.0f, attachmentDB_leftRail_nameList, &primWeaponAttachmentLeftRailOffset, &selectedSidearmWeapon_LeftRail);

			static float primWeaponAttachmentBottomRailOffset = 0;
			imgui_Static(left + width * 4, offset + 300, "Bottom Rail", width);
			imgui_DrawList(left + width * 4, offset + 20 + 300, (float)width, (float)100.0f, attachmentDB_bottomRail_nameList, &primWeaponAttachmentBottomRailOffset, &selectedSidearmWeapon_BottomRail);

			static float primWeaponAttachmentClipOffset = 0;
			imgui_Static(left + width * 4, offset + 400, "Clip", width);
			imgui_DrawList(left + width * 4, offset + 20 + 400, (float)width, (float)100.0f, attachmentDB_clip_nameList, &primWeaponAttachmentClipOffset, &selectedSidearmWeapon_Clip);
		}
	}

	static float bodyOffset = 0;
	imgui_Static(left + width * 3, top, "Body", width);
	imgui_DrawList(left + width * 3, top + shift, (float)width, (float)height, bodyDBnames, &bodyOffset, &selectedBodyIndex);

	static float armorOffset = 0;
	imgui_Static(left + width * 2, top, "Armor", width);
	imgui_DrawList(left + width * 2, top + shift, (float)width, (float)height, armorDBnames, &armorOffset, &selectedArmorIndex);

	static float headOffset = 0;
	imgui_Static(left + width * 1, top, "Head", width);
	imgui_DrawList(left + width * 1, top + shift, (float)width, (float)height, headBDnames, &headOffset, &selectedHeadIndex);

	static float headArmorOffset = 0;
	imgui_Static(left + width * 0, top, "Head Armor", 150);
	imgui_DrawList(left + width * 0, top + shift, (float)width, (float)height, headarmorBDnames, &headArmorOffset, &selectedHeadArmorIndex);


	//if(imgui_Button(500, 550, 300, 30, "Apply Changes"))
	{
		wiLoadoutSlot loadout;
		loadout.PrimaryWeaponID = weaponDB_idList[selectedPrimaryWeaponIndex];
		loadout.SecondaryWeaponID = weaponDB_idList[selectedSecondaryWeaponIndex];
		loadout.SidearmWeaponID = weaponDB_idList[selectedSideWeaponIndex];

		loadout.BodyMeshID = bodyDBids[selectedBodyIndex];
		loadout.BodyArmorID = armorDBids[selectedArmorIndex];
		loadout.BodyHeadID = headDBids[selectedHeadIndex];
		loadout.BodyHeadGearID = headarmorDBids[selectedHeadArmorIndex];

		wiWeaponAttachments Attm;
		Attm.primary_attachments.attachments[WPN_ATTM_MUZZLE] = attachmentDB_muzzle_idList[selectedPrimaryWeapon_Muzzle];
		Attm.primary_attachments.attachments[WPN_ATTM_UPPER_RAIL] = attachmentDB_upperRail_idList[selectedPrimaryWeapon_UpperRail];
		Attm.primary_attachments.attachments[WPN_ATTM_LEFT_RAIL] = attachmentDB_leftRail_idList[selectedPrimaryWeapon_LeftRail];
		Attm.primary_attachments.attachments[WPN_ATTM_BOTTOM_RAIL] = attachmentDB_bottomRail_idList[selectedPrimaryWeapon_BottomRail];
		Attm.primary_attachments.attachments[WPN_ATTM_CLIP] = attachmentDB_clip_idList[selectedPrimaryWeapon_Clip];

		Attm.secondary_attachments.attachments[WPN_ATTM_MUZZLE] = attachmentDB_muzzle_idList[selectedSecondaryWeapon_Muzzle];
		Attm.secondary_attachments.attachments[WPN_ATTM_UPPER_RAIL] = attachmentDB_upperRail_idList[selectedSecondaryWeapon_UpperRail];
		Attm.secondary_attachments.attachments[WPN_ATTM_LEFT_RAIL] = attachmentDB_leftRail_idList[selectedSecondaryWeapon_LeftRail];
		Attm.secondary_attachments.attachments[WPN_ATTM_BOTTOM_RAIL] = attachmentDB_bottomRail_idList[selectedSecondaryWeapon_BottomRail];
		Attm.secondary_attachments.attachments[WPN_ATTM_CLIP] = attachmentDB_clip_idList[selectedSecondaryWeapon_Clip];

		Attm.sidearm_attachments.attachments[WPN_ATTM_MUZZLE] = attachmentDB_muzzle_idList[selectedSidearmWeapon_Muzzle];
		Attm.sidearm_attachments.attachments[WPN_ATTM_UPPER_RAIL] = attachmentDB_upperRail_idList[selectedSidearmWeapon_UpperRail];
		Attm.sidearm_attachments.attachments[WPN_ATTM_LEFT_RAIL] = attachmentDB_leftRail_idList[selectedSidearmWeapon_LeftRail];
		Attm.sidearm_attachments.attachments[WPN_ATTM_BOTTOM_RAIL] = attachmentDB_bottomRail_idList[selectedSidearmWeapon_BottomRail];
		Attm.sidearm_attachments.attachments[WPN_ATTM_CLIP] = attachmentDB_clip_idList[selectedSidearmWeapon_Clip];

		if(pl->CurLoadout.PrimaryWeaponID != loadout.PrimaryWeaponID ||
		   pl->CurLoadout.SecondaryWeaponID != loadout.SecondaryWeaponID ||
		   pl->CurLoadout.SidearmWeaponID != loadout.SidearmWeaponID ||
		   pl->CurLoadout.BodyMeshID != loadout.BodyMeshID ||
		   pl->CurLoadout.BodyArmorID != loadout.BodyArmorID ||
		   pl->CurLoadout.BodyHeadID != loadout.BodyHeadID ||
		   pl->CurLoadout.BodyHeadGearID != loadout.BodyHeadGearID ||
		   pl->CurWeaponAttachments != Attm)
		{
			pl->UpdateLoadoutSlot(loadout, Attm, true);
		}
	}

	delete [] weaponDB_idList;
	delete [] attachmentDB_muzzle_idList;
	delete [] attachmentDB_upperRail_idList;
	delete [] attachmentDB_leftRail_idList;
	delete [] attachmentDB_bottomRail_idList;
	delete [] attachmentDB_clip_idList;
	delete [] armorDBids;
	delete [] bodyDBids;
	delete [] headDBids;
	delete [] headarmorDBids;
}

//------------------------------------------------------------------------

static void AdjustHitPosition( DecalParams& params, GameObject* target )
{
	R3DPROFILE_FUNCTION( "AdjustHitPosition" );
	if( target && target->isObjType( OBJTYPE_Mesh ) )
	{
		MeshGameObject* mobj = static_cast< MeshGameObject* >(target);

		float interSearchLength = mobj->GetObjectsRadius() * 0.33f;

		float dist;

		r3dMaterial* mtl;
		int minFace;

		r3dPoint3D start = params.Pos + params.Dir * interSearchLength * 0.5f;

		if( mobj->MeshLOD[ 0 ]->ContainsRay( start, -params.Dir, interSearchLength, &dist, &mtl, mobj->GetPosition(), mobj->GetRotationMatrix(), &minFace ) )
		{
			params.Pos = start - params.Dir * dist;
		}
	}
}

//------------------------------------------------------------------------

TCamoSettings::TCamoSettings()
: texScale( 1.0f )
, distortScale( 1.0f )
, animSpeed( 1.0f )
, reflectionAmmount( 0.33f )
, color0( 0, 0, 255 )
, color1( 0, 255, 255, 127 )
{

}

//------------------------------------------------------------------------

TCamoSettings gCamoSettings;



bool ProcessBulletHit( int &damageFromPiercable, GameObject* owner, const r3dPoint3D &hitPoint, const r3dPoint3D &hitNormal, GameObject* shootTarget, const r3dMaterial* shootMaterial, const char* hitActorName, const WeaponConfig *weaponInfo) 
{	
	r3d_assert(owner);
	r3d_assert(owner->NetworkLocal==true);

	obj_AI_Player* ownerPlayer = NULL;
	if(owner->isObjType(OBJTYPE_Human))
		ownerPlayer = (obj_AI_Player*)owner;

	if( shootTarget == NULL && shootMaterial == NULL) // hit nothing
	{
		PKT_C2C_PlayerHitNothing_s n;
		p2pSendToHost(owner, &n, sizeof(n), true);
	} 
	else if(shootTarget && shootTarget->isObjType(OBJTYPE_Human)) // hit other player
	{
		r3d_assert(shootTarget->NetworkID);
		obj_AI_Player* trgt = (obj_AI_Player*)shootTarget;

		if(!trgt->bDead)
		{
			PKT_C2C_PlayerHitDynamic_s n;
			n.hit_pos = hitPoint;
			n.targetId = toP2pNetId(shootTarget->NetworkID);
			n.hit_body_part = trgt->getBodyNameFromBoneName(hitActorName);
			n.hit_body_bone = trgt->uberAnim_->GetBoneID(hitActorName);
			n.damageFromPiercing = damageFromPiercable;
			// NOTE: This can be broken if the player jumps after firing a sniper rifle, but shouldn't happen as it's too fast.
			n.state = 0;
			if(ownerPlayer)
				n.state |= (!ownerPlayer->bOnGround)?(0x1): 0 ; // [0]-in air 
			n.state |= (!trgt->bOnGround)?0x2:0 ; // [1]-in air
			p2pSendToHost( owner, &n, sizeof(n), true);

			if(!trgt->bDead && ownerPlayer) 
			{
				ownerPlayer->m_HitMarkerFadeout = 1.0f;
				if(ownerPlayer->CurLoadout.hasItem(AbilityConfig::AB_EyeSpy) && ( gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)) // eye spy ability
				{	
					trgt->setMinimapVisibleTimer(999999999.0f, true); // just make it super long
				}

			}
		}
		else
		{
			PKT_C2C_PlayerHitNothing_s n;
			p2pSendToHost(owner, &n, sizeof(n), true);
		}

		if(ownerPlayer)
		{
			if(!(gClientLogic().m_gameInfo.friendlyFire == 0 && (ownerPlayer->TeamID == ((obj_AI_Player*)shootTarget)->TeamID)))
			{
				ownerPlayer->AddBloodOnGround(hitPoint);
				SpawnImpactParticle(r3dHash::MakeHash("player"),r3dHash::MakeHash(weaponInfo->m_PrimaryAmmo->getDecalSource()), hitPoint, hitNormal);
			}
		}
	} 
	else if(shootTarget && shootTarget->NetworkID && canDamageTarget(shootTarget))
	{
		// hit something that have network Id
		r3d_assert(shootTarget->NetworkID);
		PKT_C2C_PlayerHitDynamic_s n;
		n.hit_pos = hitPoint;
		n.targetId = toP2pNetId(shootTarget->NetworkID);
		n.hit_body_bone = 0;
		n.hit_body_part = 0;
		n.state = 0;
		n.damageFromPiercing = damageFromPiercable; 
		p2pSendToHost( owner, &n, sizeof(n), true);

		//TODO: add decals
		SpawnImpactParticle(r3dHash::MakeHash("Metal"),r3dHash::MakeHash(weaponInfo->m_PrimaryAmmo->getDecalSource()), hitPoint, hitNormal);
	} 
#if ENABLE_ZOMBIES
	else if (shootTarget && shootTarget->isObjType(OBJTYPE_Zombie) && strcmp("Bip01_Head", hitActorName) == 0)
	{
		obj_Zombie *z = static_cast<obj_Zombie*>(shootTarget);
		z->Die((z->GetPosition() - owner->GetPosition()).Normalize());
	}
#endif // ENABLE_ZOMBIES
	else 
	{
		DecalParams params;
		params.Dir	= hitNormal;
		params.Pos	= hitPoint;
		params.TypeID	= INVALID_DECAL_ID;

		AdjustHitPosition( params, shootTarget );

		uint32_t sourceHash = r3dHash::MakeHash( weaponInfo->m_PrimaryAmmo->getDecalSource());
		uint32_t matHash    = 0;
		int      particleIdx= -1;

		r3dPoint3D waterSplashPos;
		extern bool TraceWater(const r3dPoint3D& start, const r3dPoint3D& finish, r3dPoint3D& waterSplashPos);
		if( TraceWater( r3dPoint3D(gCam.x, gCam.y, gCam.z), hitPoint, waterSplashPos))
		{	
			matHash = r3dHash::MakeHash("Water");
			particleIdx = SpawnImpactParticle(matHash, sourceHash, waterSplashPos, r3dPoint3D(0,1,0));
			extern void WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);
			waterSplashPos.y -= 0.1f;
			WaterSplash(waterSplashPos, 0.1f, 30.0f, 0.04f, 0);
		}
		else
		{
			{
				if( shootTarget && shootTarget->isObjType(OBJTYPE_Terrain) )
				{
					params.TypeID	= Terrain->GetDecalID(params.Pos, sourceHash);
					particleIdx = SpawnImpactParticle(matHash, sourceHash, hitPoint, hitNormal);
				}
				else
				{
					if( shootMaterial )
					{
						matHash = r3dHash::MakeHash(shootMaterial->TypeName);
						params.TypeID	= GetDecalID(matHash, sourceHash);
						particleIdx = SpawnImpactParticle(matHash, sourceHash, hitPoint, hitNormal);
					}
				}

				if( params.TypeID != INVALID_DECAL_ID )
				{
					g_pDecalChief->Add( params );
				}

				// add impulse
				if(shootTarget && shootTarget->PhysicsObject)
				{
					shootTarget->PhysicsObject->AddImpulseAtPos(-hitNormal*weaponInfo->m_AmmoSpeed*weaponInfo->m_AmmoMass/150.0f, hitPoint);
				}
			}
		}
		int hitPierceableObject = 0;
		if(shootTarget && shootTarget->m_BulletPierceable > 0  && damageFromPiercable == 0 && weaponInfo->category!=storecat_MELEE ) // melee cannot pierce objects
		{
			hitPierceableObject = shootTarget->m_BulletPierceable;
		}

		if(hitPierceableObject)
		{
			PKT_C2C_PlayerHitStaticPierced_s n; 
			n.hit_pos  = hitPoint;
			n.hit_norm = hitNormal;
			n.decalIdx = (BYTE)(params.TypeID + 1);  // -1 is invalid decal, bring to 0-xx range
			n.particleIdx = (BYTE)(particleIdx + 1); // same here
			n.hash_obj = shootTarget?shootTarget->GetHashID():0;
			p2pSendToHost( owner, &n, sizeof(n));

			damageFromPiercable = shootTarget->m_BulletPierceable;

			return false;

		}
		else
		{
			PKT_C2C_PlayerHitStatic_s n;
			n.hit_pos  = hitPoint;
			n.hit_norm = hitNormal;
			n.decalIdx = (BYTE)(params.TypeID + 1);  // -1 is invalid decal, bring to 0-xx range
			n.particleIdx = (BYTE)(particleIdx + 1); // same here
			n.hash_obj = shootTarget?shootTarget->GetHashID():0;
			p2pSendToHost( owner, &n, sizeof(n));
		}
	}

	return true;
}
