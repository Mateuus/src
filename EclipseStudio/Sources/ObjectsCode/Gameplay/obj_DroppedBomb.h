#pragma once

#include "GameCommon.h"

class obj_DroppedBomb : public GameObject
{
	DECLARE_CLASS(obj_DroppedBomb, GameObject)
	bool			isDropped;
	char			BombName[32];
public:
	r3dMesh*	m_satchelChargeMesh;
public:
	obj_DroppedBomb();
	virtual ~obj_DroppedBomb();

	virtual	BOOL		OnCreate();
	virtual	BOOL		Load(const char *name);

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

	void				DrawMyMesh(eRenderStageID DrawState);

	void				BombDropped(bool state);
	bool				isBombDropped() const { return isDropped; }
};