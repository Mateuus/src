#pragma once

#include "APIScaleformGfx.h"

class HUDPause
{
	bool	isActive_;
	bool	isInit;

	int			m_waitingForKeyRemap; // if -1 - then not waiting, otherwise it is a key remap index that we are about to change

public:
	r3dScaleformMovie gfxMovie;

public:
	HUDPause();
	~HUDPause();

	bool 	Init();
	bool 	Unload();

	bool	IsInited() const { return isInit; }

	void 	Update();
	void 	Draw();

	bool	isActive() const { return isActive_; }
	void	Activate();
	void	Deactivate();

	void	eventReturnClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsApplyClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventExitClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
};
