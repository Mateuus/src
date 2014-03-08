#include "r3dPCH.h"
#include "r3d.h"

#include "DroppedLootbox.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"


IMPLEMENT_CLASS(DroppedLootbox, "DroppedLootbox", "Object");
AUTOREGISTER_CLASS(DroppedLootbox);

DroppedLootbox::DroppedLootbox()
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	m_IsPicked     = false;
	ObjTypeFlags = OBJTYPE_LootBoxDrop;
}

DroppedLootbox::~DroppedLootbox()
{

}

BOOL DroppedLootbox::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const ItemConfig* ic = gWeaponArmory.getItemConfig(m_ItemID);
	r3d_assert(ic);

	m_PrivateModel = ic->getMesh();
	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	PhysicsConfig.group = PHYSCOLL_TINY_GEOMETRY;
	PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// raycast and see where the ground is and place dropped box there
	if(GetRotationVector().AlmostEqual(r3dPoint3D(0,0,0)))
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
			SetRotationVector(r3dPoint3D(0, 0, 0));
		}
		else
		{
			r3d_assert(false);
		}
	}

	UpdateTransform();

	return parent::OnCreate();
}

BOOL DroppedLootbox::OnDestroy()
{
	m_PrivateModel = NULL;
	return parent::OnDestroy();
}

BOOL DroppedLootbox::Update()
{
	return parent::Update();
}


struct LootDropDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		LootDropDeferredRenderable* This = static_cast< LootDropDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	DroppedLootbox* Parent;
};

void DroppedLootbox::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		LootDropDeferredRenderable& rend = static_cast<LootDropDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
	}
}

/*virtual*/
r3dMesh* DroppedLootbox::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh* DroppedLootbox::GetObjectLodMesh()
{
	return m_PrivateModel ;
}