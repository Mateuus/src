#include "r3dPCH.h"
#include "r3d.h"

#include "obj_RiotShield.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"


IMPLEMENT_CLASS(obj_RiotShield, "obj_RiotShield", "Object");
AUTOREGISTER_CLASS(obj_RiotShield);

obj_RiotShield::obj_RiotShield()
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	m_RotX = 0;
	ObjTypeFlags = OBJTYPE_GameplayItem;
}

obj_RiotShield::~obj_RiotShield()
{

}

BOOL obj_RiotShield::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	if(m_ItemID == WeaponConfig::ITEMID_RiotShield)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_01.sco", true, false, true, true );
	}
	else
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_02.sco", true, false, true, true );
	}

	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	//PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
	//PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// raycast and see where the ground is and place dropped box there
	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
	if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
	{
		SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
		SetRotationVector(r3dPoint3D(m_RotX, 0, 0));
	}

	UpdateTransform();

	return parent::OnCreate();
}

BOOL obj_RiotShield::OnDestroy()
{
	m_PrivateModel = NULL;
	return parent::OnDestroy();
}

BOOL obj_RiotShield::Update()
{
	return parent::Update();
}


struct RiotShieldDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		RiotShieldDeferredRenderable* This = static_cast< RiotShieldDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		if(This->DrawState != rsCreateSM)
			This->Parent->m_PrivateModel->DrawMeshDeferred(r3dColor::white, 0);
		else
			This->Parent->m_PrivateModel->DrawMeshShadows();
	}

	obj_RiotShield* Parent;
	eRenderStageID DrawState;
};

void obj_RiotShield::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam )
{
	uint32_t prevCount = rarr.Count();
	m_PrivateModel->AppendShadowRenderables( rarr );
	for( uint32_t i = prevCount, e = rarr.Count(); i < e; i ++ )
	{
		RiotShieldDeferredRenderable& rend = static_cast<RiotShieldDeferredRenderable&>( rarr[ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsCreateSM;
	}
}

void obj_RiotShield::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		RiotShieldDeferredRenderable& rend = static_cast<RiotShieldDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsFillGBuffer;
	}
}

/*virtual*/
r3dMesh* obj_RiotShield::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh* obj_RiotShield::GetObjectLodMesh()
{
	return m_PrivateModel;
}