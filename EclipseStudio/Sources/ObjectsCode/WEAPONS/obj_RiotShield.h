#pragma  once

#include "GameCommon.h"

class obj_RiotShield : public GameObject
{
	DECLARE_CLASS(obj_RiotShield, GameObject)
	friend struct RiotShieldDeferredRenderable;
public:
	obj_RiotShield();
	virtual ~obj_RiotShield();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
	float				m_RotX;
protected:
	r3dMesh* m_PrivateModel;
};
