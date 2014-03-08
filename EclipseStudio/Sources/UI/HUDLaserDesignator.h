#pragma once

#include "APIScaleformGfx.h"

class HUDLaserDesignator
{
private:
	r3dScaleformMovie gfxMovie;

	void	eventLockSuccess(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
public:
	HUDLaserDesignator();
	~HUDLaserDesignator();

	bool 	Init();
	bool	IsInited () const { return m_bInited; }
	bool 	Unload();

	void 	Update();
	void 	Draw();

	void	Activate();
	void	Deactivate();
	bool	isActive() const {return m_bActive;}

	bool	disablePlayerMovement() const { return m_bTargeting||m_bSelectingAirstrike; }

	void	showWarning(const wchar_t* text);
private:
	bool m_bInited;
	bool m_bActive;

	bool m_bTargeting;
	bool m_bSelectingAirstrike;
	r3dPoint3D	m_TargetPos;
};
