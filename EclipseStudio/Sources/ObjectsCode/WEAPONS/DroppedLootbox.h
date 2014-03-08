#pragma  once

#include "GameCommon.h"

class DroppedLootbox : public GameObject
{
	DECLARE_CLASS(DroppedLootbox, GameObject)
public:
	DroppedLootbox();
	virtual ~DroppedLootbox();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
	bool		m_IsPicked;
private:
	r3dMesh* m_PrivateModel;
};
