#pragma once

#include "GameCommon.h"

class obj_ServerLightMesh : public GameObject
{
	DECLARE_CLASS(obj_ServerLightMesh, GameObject)

public:
	obj_ServerLightMesh();
	~obj_ServerLightMesh();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();

	void				SyncLightStatus(DWORD peerID);

	virtual BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	bool				bLightOn;
};
