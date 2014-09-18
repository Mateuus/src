#pragma once

#include "GameCommon.h"

#include "ObjectsCode/Gameplay/BaseControlPoint.h"

class obj_ServerControlPoint : public BaseControlPoint
{
	DECLARE_CLASS(obj_ServerControlPoint, GameObject)
private:
public:
	void		HostSendUpdate();

	float		nextScan_;
	float		last_send_status;
	void		ScanForPlayers();
	void		 RewardCapturingPlayers(int teamId);

public:
	obj_ServerControlPoint();
	~obj_ServerControlPoint();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
};
