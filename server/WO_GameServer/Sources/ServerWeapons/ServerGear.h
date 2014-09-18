#pragma once

#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"

class Gear
{
	friend class WeaponArmory;
public:
	Gear(const GearConfig* conf);
	~Gear();

	void Reset();

	const GearConfig* getConfig() const { return m_pConfig; }
	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }

	float GetDamage(float org_damage, float armorModMul, float armorModAdd, bool skipConsumedDamage); // calculates how much damage you will get with that gear. mod = affect maximum damage absorbed
private:
	const GearConfig* m_pConfig;

	// data
	float m_ConsumedDamage;
};
