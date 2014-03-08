#include "r3dPCH.h"
#include "r3d.h"
#include "shellapi.h"

#include "UserSkills.h"

// there is no data here yet..

const char* CUserSkills::SkillNames[CUserSkills::SKILL_ID_END] = {0};

static int setSkillNames()
{
	#define SET_NAME(xx) CUserSkills::SkillNames[CUserSkills::xx] = #xx
		SET_NAME(ASSAULT_ASRWeapons);
		SET_NAME(ASSAULT_FastCap);
		SET_NAME(ASSAULT_BlazingSpeed);
		SET_NAME(ASSAULT_Resilience);
		SET_NAME(ASSAULT_RiotShield);
		SET_NAME(ASSAULT_BiggerBang);
		SET_NAME(ASSAULT_Walker);

		SET_NAME(ASSAULT_ArmorWeight);
		SET_NAME(ASSAULT_BerserkersRage);
		SET_NAME(ASSAULT_EagleEye);
		SET_NAME(ASSAULT_C4);
		SET_NAME(ASSAULT_SlightOfHand);

		SET_NAME(ASSAULT_AdrenalineShot);
		SET_NAME(ASSAULT_SpawnBeacon);
		SET_NAME(ASSAULT_AutoTurret);
		
		// specialist skills
		SET_NAME(SPEC_LMGWeapons);
		SET_NAME(SPEC_ExplosiveRadius);
		SET_NAME(SPEC_RepairVehicles);
		SET_NAME(SPEC_RiotShield);
		SET_NAME(SPEC_ArmorWeight);
		SET_NAME(SPEC_WellArmored);
		SET_NAME(SPEC_WhoPointingGun);

		SET_NAME(SPEC_ArmoredUp);
		SET_NAME(SPEC_EnhancedSniffer);
		SET_NAME(SPEC_RepairVehicleSpeed);
		SET_NAME(SPEC_C4);
		SET_NAME(SPEC_AutoTurret);

		SET_NAME(SPEC_VehicleHealth);
		SET_NAME(SPEC_EnhancedFixer);
		SET_NAME(SPEC_AdrenalineShot);
		SET_NAME(SPEC_SpawnBeacon);
		
		// recon skills
		SET_NAME(RECON_SniperWeapons);
		SET_NAME(RECON_BreathControl);
		SET_NAME(RECON_EnemyRadar);
		SET_NAME(RECON_FastCap);
		SET_NAME(RECON_InvisibleCamo);
		SET_NAME(RECON_MotionSensor);
		SET_NAME(RECON_HealthNut);

		SET_NAME(RECON_LongBreathing);
		SET_NAME(RECON_UAVTagging);
		SET_NAME(RECON_CoatedOptics);
		SET_NAME(RECON_BlazingSpeed);
		SET_NAME(RECON_RespawnBeacon);

		SET_NAME(RECON_C4);
		SET_NAME(RECON_AdvancedStealth);
		SET_NAME(RECON_AdrenalineShot);
		SET_NAME(RECON_AutoTurret);
		SET_NAME(RECON_MorphineShot);

		// medic skill
		SET_NAME(MEDIC_SMGWeapons);
		SET_NAME(MEDIC_Medkit);
		SET_NAME(MEDIC_MorphineShot);
		SET_NAME(MEDIC_BlazingSpeed);
		SET_NAME(MEDIC_HealthRegeneration);
		SET_NAME(MEDIC_ImprovedFlashbangRadius);
		SET_NAME(MEDIC_GuardianAngel);
		
		SET_NAME(MEDIC_HealingSpeed);
		SET_NAME(MEDIC_ImprovedMorphine);
		SET_NAME(MEDIC_ImprovedFlashbangPower);
		SET_NAME(MEDIC_AdrenalineShot);
		SET_NAME(MEDIC_Defibrilator);

		SET_NAME(MEDIC_RevivalHealth);
		SET_NAME(MEDIC_RiotShield);
		SET_NAME(MEDIC_C4);
	#undef SET_NAME
	return 1;
}

static int skillNamesInited = setSkillNames();
