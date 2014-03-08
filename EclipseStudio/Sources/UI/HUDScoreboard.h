#pragma once

#include "APIScaleformGfx.h"

class obj_AI_Player;
class HUDScoreboard
{
	bool	isActive_;
	bool	isInit;
public:
	r3dScaleformMovie gfxMovie;

public:
	HUDScoreboard();
	~HUDScoreboard();

	bool 	Init();
	bool 	Unload();

	bool	IsInited() const { return isInit; }

	void 	Update();
	void 	Draw();

	bool	isActive() const { return isActive_; }
	void	Activate();
	void	Deactivate();
};
