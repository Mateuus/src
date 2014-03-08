#include "r3dPCH.h"
#include "r3d.h"

#include "DroppedWeapon.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"


IMPLEMENT_CLASS(DroppedWeapon, "DroppedWeapon", "Object");
AUTOREGISTER_CLASS(DroppedWeapon);

DroppedWeapon::DroppedWeapon()
{
	m_isGood = false;
	m_PrivateModel = NULL;
	m_WeaponItemID = -1;
	m_IsPicked     = false;
	ObjTypeFlags = OBJTYPE_WeaponDrop;
}

DroppedWeapon::~DroppedWeapon()
{
	
}

BOOL DroppedWeapon::OnCreate()
{
	r3d_assert(m_WeaponItemID > 0);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_WeaponItemID);
	if(wc == NULL)
		return FALSE;

	wc->aquireMesh( true ) ;

	m_PrivateModel = wc->getMesh( false, false );
	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	PhysicsConfig.group = PHYSCOLL_TINY_GEOMETRY;
	PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	// raycast and see where the ground is and place dropped weapon there
	if(GetRotationVector().AlmostEqual(r3dPoint3D(0,0,0)))
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z) + r3dPoint3D(0, GetBBoxLocal().Size.z, 0));
			SetRotationVector(r3dPoint3D(0, 90, 0));
		}
		else
		{
			r3d_assert(false);
		}
	}

	UpdateTransform();

	m_isGood = true;
	return parent::OnCreate();
}

BOOL DroppedWeapon::OnDestroy()
{
	gWeaponArmory.getWeaponConfig( m_WeaponItemID )->releaseMesh() ;

	m_PrivateModel = NULL;
	return parent::OnDestroy();
}

BOOL DroppedWeapon::Update()
{
	return parent::Update();
}


struct WeaponDropDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		WeaponDropDeferredRenderable* This = static_cast< WeaponDropDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	DroppedWeapon* Parent;
};

void DroppedWeapon::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if(!m_isGood)
		return;

	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	if(m_PrivateModel)
		m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		WeaponDropDeferredRenderable& rend = static_cast<WeaponDropDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
	}
}

/*virtual*/
r3dMesh*
DroppedWeapon::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh*
DroppedWeapon::GetObjectLodMesh()
{
	return m_PrivateModel ;
}