#pragma  once

#include "GameCommon.h"

class DroppedRespawnBeacon : public GameObject
{
	DECLARE_CLASS(DroppedRespawnBeacon, GameObject)
public:
	DroppedRespawnBeacon();
	virtual ~DroppedRespawnBeacon();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
	float				m_RotX;
private:
	r3dMesh* m_PrivateModel;
};
