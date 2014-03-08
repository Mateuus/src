#pragma  once

#include "GameCommon.h"

class DroppedMedKit : public GameObject
{
	DECLARE_CLASS(DroppedMedKit, GameObject)
public:
	DroppedMedKit();
	virtual ~DroppedMedKit();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
	int					TeamID;
private:
	r3dMesh* m_PrivateModel;
	float		nextScan_;
};
