#pragma once

#include "GameCommon.h"

class obj_ServerPlayer;

class obj_ServerSupplyCrate : public GameObject
{
	DECLARE_CLASS(obj_ServerSupplyCrate, GameObject)

  protected:
	bool		isBlackops_;
	float		nextScan_;
	void		ScanForPlayers();

virtual	bool		CanResupply(obj_ServerPlayer* plr) { return true; }

  public:
	obj_ServerSupplyCrate();
	~obj_ServerSupplyCrate();

virtual	BOOL		Load(const char *name);

virtual	BOOL		OnCreate();
virtual	BOOL		OnDestroy();

virtual	BOOL		Update();
};

class obj_ServerBlackopsCrate : public obj_ServerSupplyCrate
{
	DECLARE_CLASS(obj_ServerBlackopsCrate, GameObject)
  private:
	obj_ServerBlackopsCrate()
	{
		isBlackops_ = true;
	}
virtual	bool		CanResupply(obj_ServerPlayer* plr);
};

