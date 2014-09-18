#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "ServerGear.h"

Gear::Gear(const GearConfig* conf) : m_pConfig(conf)
{
	Reset();
}

Gear::~Gear()
{
}

void Gear::Reset()
{
	m_ConsumedDamage = 0;
}

float Gear::GetDamage(float org_damage, float armorModMul, float armorModAdd, bool skipConsumedDamage)
{
	//r3dOutToLog("Gear adjusting damage fn: org_damage(%.2f), armorModMul(%.2f), armorModAdd(%.2f)\n", org_damage, armorModMul, armorModAdd); CLOG_INDENT;
	if(org_damage < 0 ) org_damage = 0; 
	
	if(m_ConsumedDamage < (m_pConfig->m_damageMax) || skipConsumedDamage)
	{
		float cons_dam = org_damage * R3D_MIN((m_pConfig->m_damagePerc+armorModAdd)*armorModMul, 1.0f);
		//r3dOutToLog("Consumed damage(%.2f)\n", cons_dam);
		m_ConsumedDamage += cons_dam;
		return org_damage-cons_dam;
	}
	else
	{
		//r3dOutToLog("Gear received max damage already\n");
	}
	return org_damage;
}