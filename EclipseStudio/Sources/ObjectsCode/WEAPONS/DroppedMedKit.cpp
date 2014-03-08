#include "r3dPCH.h"
#include "r3d.h"

#include "DroppedMedKit.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"


IMPLEMENT_CLASS(DroppedMedKit, "DroppedMedKit", "Object");
AUTOREGISTER_CLASS(DroppedMedKit);

DroppedMedKit::DroppedMedKit()
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	ObjTypeFlags = OBJTYPE_GameplayItem;
	nextScan_   = 0;
}

DroppedMedKit::~DroppedMedKit()
{

}

BOOL DroppedMedKit::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_ItemID);
	r3d_assert(wc);

	m_PrivateModel = wc->getMesh(true, false);
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

BOOL DroppedMedKit::OnDestroy()
{
	m_PrivateModel = NULL;
	return parent::OnDestroy();
}

BOOL DroppedMedKit::Update()
{
	const float radius = 7.0f;
	obj_AI_Player* plrOwner = (obj_AI_Player*)GameWorld().GetObject(ownerID);
	if(!plrOwner)
		return parent::Update();

	if(r3dGetTime() > nextScan_)
	{
		nextScan_ = r3dGetTime() + 0.1f; // 10 times a sec

		for(int i = 0; i < ClientGameLogic::MAX_NUM_PLAYERS; i++) 
		{
			obj_AI_Player* plr = gClientLogic().GetPlayer(i);
			if(plr == NULL)
				continue;

			if(plr->bDead || plr->TeamID != TeamID)
				continue;

			// check radius
			float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
			if(distSq > (radius * radius))
				continue;

			if(plr->m_Health < plr->getMaxHealth())
			{
				//!!! this code is duplicated on client !!!
				float healthNutBoost = 1.0f;
				switch(plr->CurLoadout.getSkillLevel(CUserSkills::RECON_HealthNut))
				{
				case 1: healthNutBoost = 1.1f; break;
				case 2: healthNutBoost = 1.2f; break;
				case 3: healthNutBoost = 1.3f; break;
				}
				float healingSpeed = 1.0f;
				switch(plrOwner->CurLoadout.getSkillLevel(CUserSkills::MEDIC_HealingSpeed))
				{
				case 1: healingSpeed = 1.1f; break;
				case 2: healingSpeed = 1.2f; break;
				case 3: healingSpeed = 1.3f; break;
				}

				plr->m_Health = plr->m_Health + GPP->c_fRegenSpeed * 0.1f * 2 * healthNutBoost * healingSpeed; // 0.1f - rate of update. design: The medkit emits heals at 2x the players base regeneration rate
				plr->m_Health = R3D_MIN(plr->m_Health.get(), plr->getMaxHealth());
			}
		}
	}
	return parent::Update();
}


struct MedKitDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		MedKitDeferredRenderable* This = static_cast< MedKitDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);
		MeshDeferredRenderable::Draw( RThis, Cam );
	}

	DroppedMedKit* Parent;
};

void DroppedMedKit::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		MedKitDeferredRenderable& rend = static_cast<MedKitDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
	}
}

/*virtual*/
r3dMesh* DroppedMedKit::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh* DroppedMedKit::GetObjectLodMesh()
{
	return m_PrivateModel;
}