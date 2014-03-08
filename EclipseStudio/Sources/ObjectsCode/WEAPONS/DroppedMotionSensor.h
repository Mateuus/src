#pragma  once

#include "GameCommon.h"

class DroppedMotionSensor : public GameObject
{
	DECLARE_CLASS(DroppedMotionSensor, GameObject)
public:
	DroppedMotionSensor();
	virtual ~DroppedMotionSensor();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
private:
	r3dMesh* m_PrivateModel;
	float		nextScan_;
	float		scanRadius;
};
