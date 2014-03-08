#pragma once

#include "APIScaleformGfx.h"

class HUDCameraDrone
{
public:

public:
	r3dScaleformMovie 	gfxMovie;

public:
	HUDCameraDrone();
	~HUDCameraDrone();

	bool 	Init();
	bool	IsInited () const { return m_bInited; }
	bool 	Unload();

	void 	Update(class obj_UAV* uav); // uav may be null
	void 	Draw();

private:
	bool m_bInited;

	bool m_hasTargetIcon[MAX_UAV_TARGETS];
	void moveScreenIcon(const char* name, const r3dPoint3D& pos, bool alwaysShow);
};
