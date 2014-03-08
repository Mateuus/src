#ifndef __GEAR_H__
#define __GEAR_H__

#ifdef WO_SERVER
  #error "client gear.h included in SERVER"
#endif

#include "..\..\GameCode\UserProfile.h"
#include "WeaponConfig.h"

class Gear
{
	friend class WeaponArmory;
public:
	Gear(const GearConfig* conf);
	~Gear();

	void Reset();

	r3dMesh*	getModel(bool firstPersonModel) const 
	{
		// always load regular model, as we need it in FPS mode too
		r3dMesh* res = m_pConfig->getMesh();
		if(firstPersonModel && (m_pConfig->category == storecat_Characters || m_pConfig->category == storecat_Heroes)) // only body/heroes have FPS models, armor,heads,etc isn't rendered in fps mode
		{
			r3dMesh* fres = m_pConfig->getFirstPersonMesh();
			if(fres)
				return fres;
		}

		return res;
	}
	const char*			getStoreIcon() const { return m_pConfig->m_StoreIcon; }

	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }

	float GetWeight() const { return m_pConfig->m_Weight; }

private:
	const GearConfig* m_pConfig;

};

#endif //__GEAR_H__
