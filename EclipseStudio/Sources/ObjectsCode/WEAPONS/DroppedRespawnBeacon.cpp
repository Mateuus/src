#include "r3dPCH.h"
#include "r3d.h"

#include "DroppedRespawnBeacon.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"
#include "..\..\ui\HUDRespawn.h"

extern HUDRespawn*	hudRespawn;

IMPLEMENT_CLASS(DroppedRespawnBeacon, "DroppedRespawnBeacon", "Object");
AUTOREGISTER_CLASS(DroppedRespawnBeacon);

DroppedRespawnBeacon::DroppedRespawnBeacon()
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	m_RotX = 0;
	ObjTypeFlags = OBJTYPE_GameplayItem;
}

DroppedRespawnBeacon::~DroppedRespawnBeacon()
{

}

BOOL DroppedRespawnBeacon::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_ItemID);
	if(!wc)
		return FALSE;

	m_PrivateModel = wc->getMesh(true, false);
	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
	PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// raycast and see where the ground is and place dropped box there
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
			SetRotationVector(r3dPoint3D(m_RotX, 0, 0));
		}
	}

	UpdateTransform();

	if(hudRespawn && hudRespawn->isActive())
		hudRespawn->onRespawnBeaconCreate(NetworkID);

	return parent::OnCreate();
}

BOOL DroppedRespawnBeacon::OnDestroy()
{
	m_PrivateModel = NULL;
	if(hudRespawn && hudRespawn->isActive())
		hudRespawn->onRespawnBeaconDestroy(NetworkID);

	return parent::OnDestroy();
}

BOOL DroppedRespawnBeacon::Update()
{
	return parent::Update();
}


struct BeaconDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BeaconDeferredRenderable* This = static_cast< BeaconDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	DroppedRespawnBeacon* Parent;
};

void DroppedRespawnBeacon::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		BeaconDeferredRenderable& rend = static_cast<BeaconDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
	}
}

r3dMesh* DroppedRespawnBeacon::GetObjectMesh() 
{
	return m_PrivateModel;
}

r3dMesh* DroppedRespawnBeacon::GetObjectLodMesh()
{
	return m_PrivateModel;
}