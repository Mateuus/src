#include "r3dPCH.h"
#include "r3d.h"

#include "DroppedMotionSensor.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"


IMPLEMENT_CLASS(DroppedMotionSensor, "DroppedMotionSensor", "Object");
AUTOREGISTER_CLASS(DroppedMotionSensor);

DroppedMotionSensor::DroppedMotionSensor()
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	ObjTypeFlags = OBJTYPE_GameplayItem;
	nextScan_   = 0;
	scanRadius = 0;
}

DroppedMotionSensor::~DroppedMotionSensor()
{

}

BOOL DroppedMotionSensor::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_ItemID);
	r3d_assert(wc);

	scanRadius = wc->m_AmmoDamage; // scan radius is in damage field
	obj_AI_Player* owner = (obj_AI_Player*)GameWorld().GetObject(ownerID);
	if(owner)
	{
		switch(owner->CurLoadout.getSkillLevel(CUserSkills::RECON_MotionSensor))
		{
		case 2: 
		case 3:
			scanRadius += 1; break;
		case 4: 
		case 5:
			scanRadius += 2; break;
		}
	}

	m_PrivateModel = wc->getMesh(true, false);
	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
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

BOOL DroppedMotionSensor::OnDestroy()
{
	m_PrivateModel = NULL;
	return parent::OnDestroy();
}

BOOL DroppedMotionSensor::Update()
{
	if(r3dGetTime() > nextScan_)
	{
		nextScan_ = r3dGetTime() + 0.1f; // 10 times a sec
		obj_AI_Player* owner = (obj_AI_Player*)GameWorld().GetObject(ownerID);
		if(owner && owner->NetworkLocal) // sensor shows enemies only for person who spawned it
		{
			for(int i = 0; i < ClientGameLogic::MAX_NUM_PLAYERS; i++) 
			{
				obj_AI_Player* plr = gClientLogic().GetPlayer(i);
				if(plr == NULL)
					continue;

				if(plr->bDead || plr->TeamID == owner->TeamID)
					continue;

				// check radius
				float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
				if(distSq > (scanRadius * scanRadius))
					continue;

				plr->setMinimapVisibleTimer(1.0f, true);
			}
		}
	}
	return parent::Update();
}


struct SensorDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		SensorDeferredRenderable* This = static_cast< SensorDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	DroppedMotionSensor* Parent;
};

void DroppedMotionSensor::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		SensorDeferredRenderable& rend = static_cast<SensorDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
	}
}

/*virtual*/
r3dMesh* DroppedMotionSensor::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh* DroppedMotionSensor::GetObjectLodMesh()
{
	return m_PrivateModel;
}