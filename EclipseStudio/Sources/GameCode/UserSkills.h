#pragma once

class CUserSkills
{
  public:
	//
	// hardcoded vars used to generate skill Ids.
	//
	
	static const int NUM_TIERS = 3;
	static const int NUM_SKILLS_PER_TIER = 10;
	static const int SKILL_CLASS_MULT = 100;	// multiplier used in SkillID to detect skill class
	static const int NUM_RANKS = 5;

	// some random numbers. amount of SP we need to spend on tier to unlock next one
	static const int TIER2_UNLOCK = 20;
	static const int TIER3_UNLOCK = 20;
	
	enum EClassID
	{
		CLASS_Assault = 0,
		CLASS_Spec    = 1,
		CLASS_Recon   = 2,
		CLASS_Medic   = 3,
		CLASS_MAX,
	};

	// note: adjust setSkillNames when changing skill ids
	enum ESkillIDs
	{
		// assault skills 
		ASSAULT_ASRWeapons    = 0,
		ASSAULT_FastCap,
		ASSAULT_BlazingSpeed,
		ASSAULT_Resilience,
		ASSAULT_RiotShield,
		ASSAULT_BiggerBang,
		ASSAULT_Walker,

		ASSAULT_ArmorWeight    = 10,
		ASSAULT_BerserkersRage,
		ASSAULT_EagleEye,
		ASSAULT_C4,
		ASSAULT_SlightOfHand,

		ASSAULT_AdrenalineShot = 20,
		ASSAULT_SpawnBeacon,
		ASSAULT_AutoTurret,
		
		// specialist skills
		SPEC_LMGWeapons        = 100,
		SPEC_ExplosiveRadius,
		SPEC_RepairVehicles,
		SPEC_RiotShield,
		SPEC_ArmorWeight,
		SPEC_WellArmored,
		SPEC_WhoPointingGun,

		SPEC_ArmoredUp         = 110,
		SPEC_EnhancedSniffer,
		SPEC_RepairVehicleSpeed,
		SPEC_C4,
		SPEC_AutoTurret,

		SPEC_VehicleHealth     = 120,
		SPEC_EnhancedFixer,
		SPEC_AdrenalineShot,
		SPEC_SpawnBeacon,
		
		// recon skills
		RECON_SniperWeapons    = 200,
		RECON_BreathControl,
		RECON_EnemyRadar,
		RECON_FastCap,
		RECON_InvisibleCamo,
		RECON_MotionSensor,
		RECON_HealthNut,

		RECON_LongBreathing    = 210,
		RECON_UAVTagging,
		RECON_CoatedOptics,
		RECON_BlazingSpeed,
		RECON_RespawnBeacon,

		RECON_C4               = 220,
		RECON_AdvancedStealth,
		RECON_AdrenalineShot,
		RECON_AutoTurret,
		RECON_MorphineShot,

		// medic skill
		MEDIC_SMGWeapons       = 300,
		MEDIC_Medkit,
		MEDIC_MorphineShot,
		MEDIC_BlazingSpeed,
		MEDIC_HealthRegeneration,
		MEDIC_ImprovedFlashbangRadius,
		MEDIC_GuardianAngel,
		
		MEDIC_HealingSpeed     = 310,
		MEDIC_ImprovedMorphine,
		MEDIC_ImprovedFlashbangPower,
		MEDIC_AdrenalineShot,
		MEDIC_Defibrilator,

		MEDIC_RevivalHealth    = 320,
		MEDIC_RiotShield,
		MEDIC_C4,

		SKILL_ID_END           = 400,
	};
	
	const static char* SkillNames[SKILL_ID_END];
};

