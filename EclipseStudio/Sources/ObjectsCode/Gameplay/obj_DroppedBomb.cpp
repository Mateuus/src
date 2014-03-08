#include "r3dPCH.h"
#include "r3d.h"

#include "Gameplay_Params.h"
#include "GameCommon.h"
#include "obj_DroppedBomb.h"
#include "../../multiplayer/ClientGameLogic.h"
#include "../ai/AI_Player.H"

#include "..\..\ui\HUDMinimap.h"
extern HUDMinimap*	hudMinimap;

IMPLEMENT_CLASS(obj_DroppedBomb, "obj_DroppedBomb", "Object");
AUTOREGISTER_CLASS(obj_DroppedBomb);

obj_DroppedBomb::obj_DroppedBomb()
{
	m_satchelChargeMesh = NULL;
	isDropped = false;
}

obj_DroppedBomb::~obj_DroppedBomb()
{
}

BOOL obj_DroppedBomb::Load(const char *fname)
{
	if(!parent::Load(fname)) return FALSE;

	m_satchelChargeMesh = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\EXP_Satchel.sco", true, false, false, true );

	r3dscpy(BombName, "DroppedBomb");

	return TRUE;
}

BOOL obj_DroppedBomb::OnCreate()
{
	parent::OnCreate();

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();

	// because we decide where to render this object in the actual render, disable any kind of occlusion checks
	ObjFlags |= OBJFLAG_AlwaysDraw | OBJFLAG_SkipOcclusionCheck;

	return 1;
}

void obj_DroppedBomb::DrawMyMesh(eRenderStageID DrawState)
{
	D3DXMATRIX mr;
	{
		if(isDropped)
			D3DXMatrixTranslation(&mr, GetPosition().x, GetPosition().y, GetPosition().z);
		else
		{
			bool found = false;
			// find a player that is carrying a bomb
			const ClientGameLogic& CGL = gClientLogic();
			for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
			{
				obj_AI_Player* plr = CGL.GetPlayer(i);
				if(plr && plr->hasSabotageBomb())
				{
					plr->GetSkeleton()->GetBoneWorldTM("Weapon_BackRPG", &mr, plr->DrawFullMatrix);
					found = true;
					break;
				}
			}
			if(!found)
				return;
		}
		m_satchelChargeMesh->SetVSConsts(mr);

		switch( DrawState )
		{
		case rsFillGBuffer:
			m_satchelChargeMesh->DrawMeshDeferred( r3dColor::white, 0 ) ;
			break ;

		case rsCreateSM:
			m_satchelChargeMesh->DrawMeshShadows();
			break ;

		default:
			m_satchelChargeMesh->DrawMeshSimple( 0 );
			break ;
		}
	}
}

struct DroppedBombShadowGBufferRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		DroppedBombShadowGBufferRenderable *This = static_cast<DroppedBombShadowGBufferRenderable*>( RThis );

		This->Parent->DrawMyMesh( This->DrawState );
	}

	obj_DroppedBomb*	Parent;
	eRenderStageID		DrawState;
};



#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_DroppedBomb::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) /*OVERRIDE*/
{
	DroppedBombShadowGBufferRenderable rend;

	rend.Init();
	rend.Parent		= this;
	rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
	rend.DrawState	= rsCreateSM;

	rarr.PushBack( rend );
}

void obj_DroppedBomb::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
		// gbuffer
		{
			DroppedBombShadowGBufferRenderable rend;

			rend.Init();
			rend.Parent		= this;
			rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
			rend.DrawState	= rsFillGBuffer;

			render_arrays[ rsFillGBuffer ].PushBack( rend );
		}
}


void obj_DroppedBomb::BombDropped(bool state)
{
	if(isDropped == state)
		return;

	isDropped = state;
	if(isDropped)
		hudMinimap->AddBomb(BombName, GetPosition());
	else
		hudMinimap->EraseUnit(BombName);
}