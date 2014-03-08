#include "r3dPCH.h"
#include "r3d.h"

#include "Gameplay_Params.h"
#include "GameCommon.h"
#include "obj_BombPlacement.h"
#include "../../multiplayer/ClientGameLogic.h"
#include "../ai/AI_Player.H"
#include "ObjectsCode/world/DecalChief.h"
#include "ObjectsCode/world/MaterialTypes.h"
#include "ObjectsCode/WEAPONS/ExplosionVisualController.h"

#include "obj_ControlPoint.h"

#include "..\..\ui\HUDMinimap.h"
#include "..\..\ui\HUDDisplay.h"
extern HUDMinimap*	hudMinimap;
extern HUDDisplay*	hudMain;

#include "Editors/ObjectManipulator3d.h"

IMPLEMENT_CLASS(obj_BombPlacement, "obj_BombPlacement", "Object");
AUTOREGISTER_CLASS(obj_BombPlacement);

extern bool g_bEditMode;

BombPlacementMngr gBombPlacementMgr;

static r3dTexture *BombObjectiveIcon = NULL;

obj_BombPlacement::obj_BombPlacement()
{
	m_DestructionTimer = 45.0f;
	m_ArmingTimer = 3.0f;
	m_DisarmingTimer = 4.5f;
	m_ActivationRadius = 4.0f;
	bombSoundID = -1;
	timeUntilPlayBeep = -1;
	timeWhenArmed = 0;

	Status = SS_EMPTY;

	m_satchelChargeMesh = NULL;

	m_CurrentTimer = 0;

	needToAddToMinimap = 0;
	ControlName[0] = 0;
	UITagName = 0;
}

static int numBombPointsAddedToUI = 0;

obj_BombPlacement::~obj_BombPlacement()
{
	if(UITagName)
		numBombPointsAddedToUI--;
}

BOOL obj_BombPlacement::Load(const char *fname)
{
	if(!parent::Load(fname)) return FALSE;

#ifndef FINAL_BUILD
	if(g_bEditMode)
		if (!BombObjectiveIcon) BombObjectiveIcon = r3dRenderer->LoadTexture("Data\\Images\\BombObjective.dds");
#endif

	m_satchelChargeMesh = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\EXP_Satchel.sco", true, false, false, true );
	return TRUE;
}

BOOL obj_BombPlacement::OnCreate()
{
	parent::OnCreate();

	DrawOrder	= OBJ_DRAWORDER_LAST;

#ifndef FINAL_BUILD
	if(!g_bEditMode) 
#endif
	{
		NetworkLocal = false;
		NetworkID    = gClientLogic().net_lastFreeId++;
		gBombPlacementMgr.RegisterBombPlacement(this);

		sprintf(ControlName, "BombPlcmID%d", gBombPlacementMgr.NumBombPlacements());
		needToAddToMinimap = 30;

		// it should be safe to generate names here, as update order should be deterministic for all players
		switch(numBombPointsAddedToUI)
		{
		case 0:
			UITagName = "a";
			break;
		case 1:
			UITagName = "b";
			break;
		case 2:
			UITagName = "c";
			break;
		case 3:
			UITagName = "d";
			break;
		case 4:
			UITagName = "z";
			break;
		default:
			r3dError("Too many bomb points!\n"); // shouldn't happen in public build
		}
		++numBombPointsAddedToUI;
	}

	m_satchelChargePos = GetPosition();

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();

	return 1;
}

BOOL obj_BombPlacement::Update()
{
	parent::Update();

	if(needToAddToMinimap>0 && hudMinimap && hudMinimap->IsInited() && gClientLogic().localPlayer_)
	{
		--needToAddToMinimap;
		if(needToAddToMinimap == 0) // need to skip frames to make sure that map is loaded, otherwise control point will not show up in minimap. weird flash
		{
			hudMinimap->AddControlPoint(ControlName, GetPosition());
			hudMinimap->SetControlPointStatus(ControlName, "grey", UITagName);
		}
	}
	if(timeUntilPlayBeep>=0 && bombSoundID!=-1)
	{
		timeUntilPlayBeep -= r3dGetFrameTime();
		float timeLeft = m_DestructionTimer - (r3dGetTime() - timeWhenArmed);
		char tempS[16];
		int minutes = int(timeLeft)/60;
		int seconds = int(timeLeft)-minutes*60;
		sprintf(tempS, "00:%.2d:%.2d", minutes, seconds);
		hudMain->setMinimapBombTimer(tempS, true);
		if(timeUntilPlayBeep < 0)
		{
			snd_PlaySound(bombSoundID, GetPosition());
			if(timeLeft > 30)
				timeUntilPlayBeep = 2.0f;
			else if(timeLeft > 15)
				timeUntilPlayBeep = 1.0f;
			else if(timeLeft > 10)
				timeUntilPlayBeep = 0.5f;
			else if(timeLeft > 5)
				timeUntilPlayBeep = 0.25f;
			else
				timeUntilPlayBeep = 0.2f;
		}
	}
	return TRUE;
}

void obj_BombPlacement::setStatus(BOMB_STATUS s)
{
	if(Status == s)
		return;
	Status = s;
	if(Status == SS_ARMED)
	{
		timeWhenArmed = r3dGetTime();
		if(bombSoundID==-1)
			bombSoundID = SoundSys.GetEventIDByPath("Sounds/Misc/BombBeep");
		timeUntilPlayBeep = 0;

		char tempS[16];
		int minutes = int(m_DestructionTimer)/60;
		int seconds = int(m_DestructionTimer)-minutes*60;
		sprintf(tempS, "00:%.2d:%.2d", minutes, seconds);
		hudMain->setMinimapBombTimer(tempS, true);
	}
	else
	{
		hudMain->setMinimapBombTimer("", false);
		timeUntilPlayBeep = -1;
	}
}

BOOL obj_BombPlacement::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
	default: return FALSE;

	case PKT_S2C_Bomb_BombPlaced:
		{
			const PKT_S2C_Bomb_BombPlaced_s& n = *(PKT_S2C_Bomb_BombPlaced_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			setStatus(SS_ARMED);
			m_satchelChargePos = n.pos;
			hudMain->ShowAchievementCustom(gLangMngr.getString("$HUD_Msg_BombArmed"), "", "$Data/Menu/achievements/hud/boomer.png", "");

			const ClientGameLogic& CGL = gClientLogic();
			for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
			{
				obj_AI_Player* plr = CGL.GetPlayer(i);
				if(plr)
					plr->setHasSabotageBomb(false);
			}

		}
		break;
	case PKT_S2C_Bomb_Exploded:
		{
			// spawn explosion
			DecalParams params;
			params.Dir		= r3dPoint3D(0,1,0);
			params.Pos		= GetPosition();
			params.TypeID	= GetDecalID( r3dHash::MakeHash(""), r3dHash::MakeHash("grenade") );
			if( params.TypeID != INVALID_DECAL_ID )
				g_pDecalChief->Add( params );
			SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("SatchelCharge"), GetPosition(), r3dPoint3D(0,1,0));

			//	Start radial blur effect
			gExplosionVisualController.AddExplosion(GetPosition(), 30.0f);

			setStatus(SS_EMPTY);
		}
		break;
	}

	return TRUE;
}

void obj_BombPlacement::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("BombPlaceObj");
	m_DestructionTimer = myNode.attribute("DestructionTimer").as_float();
	m_ArmingTimer = myNode.attribute("ArmingTimer").as_float();
	m_DisarmingTimer = myNode.attribute("DisarmingTimer").as_float();
	m_ActivationRadius = myNode.attribute("ActivationRadius").as_float();
}

void obj_BombPlacement::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node myNode = node.append_child();
	myNode.set_name("BombPlaceObj");
	myNode.append_attribute("DestructionTimer") = m_DestructionTimer;
	myNode.append_attribute("ArmingTimer") = m_ArmingTimer;
	myNode.append_attribute("DisarmingTimer") = m_DisarmingTimer;
	myNode.append_attribute("ActivationRadius") = m_ActivationRadius;
}

void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
struct BombPlacemenetObjRenderableHelper : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BombPlacemenetObjRenderableHelper *This = static_cast<BombPlacemenetObjRenderableHelper*>( RThis );

		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZC );

		if((This->Parent->GetPosition() - Cam).Length() < 100)
			r3dDrawIcon3D(This->Parent->GetPosition(), BombObjectiveIcon, r3dColor(255,255,255), 24);
	}

	obj_BombPlacement* Parent;	
};

struct BombPlacemenetObjCompositeRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BombPlacemenetObjCompositeRenderable *This = static_cast<BombPlacemenetObjCompositeRenderable*>( RThis );

		r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

		r3dDrawLine3D(This->Parent->GetPosition(), This->Parent->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor24::grey);
		r3dDrawCircle3D(This->Parent->GetPosition(), This->Parent->m_ActivationRadius, Cam, 0.1f, r3dColor(0,255,0));

		r3dRenderer->Flush();
		r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
	}

	obj_BombPlacement*	Parent;	
};

void obj_BombPlacement::DrawSatchelMesh(eRenderStageID DrawState)
{
	D3DXMATRIX mr;
	{
		D3DXMatrixTranslation(&mr, m_satchelChargePos.x, m_satchelChargePos.y, m_satchelChargePos.z);
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

struct BombShadowGBufferRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BombShadowGBufferRenderable *This = static_cast<BombShadowGBufferRenderable*>( RThis );

		This->Parent->DrawSatchelMesh( This->DrawState );
	}

	obj_BombPlacement*	Parent;
	eRenderStageID		DrawState;
};



#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_BombPlacement::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) /*OVERRIDE*/
{
	if(Status == SS_ARMED)
	{
		BombShadowGBufferRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		rend.DrawState	= rsCreateSM;

		rarr.PushBack( rend );
	}
}

void obj_BombPlacement::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if( Status == SS_ARMED)
	{
		// gbuffer
		{
			BombShadowGBufferRenderable rend;

			rend.Init();
			rend.Parent		= this;
			rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
			rend.DrawState	= rsFillGBuffer;

			render_arrays[ rsFillGBuffer ].PushBack( rend );
		}
	}
	
	// EDITOR RENDERING BELOW
#ifdef FINAL_BUILD
	return;
#else
	// don't draw debug info if we're not in editor mode
	if ( !g_bEditMode )
		return;

	if( r_hide_icons->GetInt() )
		return ;

	// helper
	extern int CurHUDID;
	if(CurHUDID == 0)
	{
		BombPlacemenetObjRenderableHelper rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}

	// composite
	{
		BombPlacemenetObjCompositeRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;

		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}
#endif
}

#ifndef FINAL_BUILD
float obj_BombPlacement::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected )
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		starty += imgui_Static ( scrx, starty, "Bomb Placement Properties" );

		static float destrTimerVal = 0 ;
		destrTimerVal = m_DestructionTimer ;
		starty += imgui_Value_Slider(scrx, starty, "Destruction Timer", &destrTimerVal,	0.0f,300,	"%.2f",1);
		PropagateChange( destrTimerVal, &obj_BombPlacement::m_DestructionTimer, this, selected ) ;

		static float armingTimerVal ;
		armingTimerVal = m_ArmingTimer ;
		starty += imgui_Value_Slider(scrx, starty, "Arming Timer", &armingTimerVal,	0.0f,30.0f,	"%.2f",1);
		PropagateChange( armingTimerVal, &obj_BombPlacement::m_ArmingTimer, this, selected ) ;

		static float disarmingTimerVal ;
		disarmingTimerVal = m_DisarmingTimer ;
		starty += imgui_Value_Slider(scrx, starty, "Disarming Timer", &disarmingTimerVal,	0.0f,30.0f,	"%.2f",1);
		PropagateChange( disarmingTimerVal, &obj_BombPlacement::m_DisarmingTimer, this, selected ) ;

		static float activationRadiusVal ;
		activationRadiusVal = m_ActivationRadius ;
		starty += imgui_Value_Slider(scrx, starty, "Activation Radius", &activationRadiusVal,	0.0f,30.0f,	"%.2f",1);
		PropagateChange( activationRadiusVal, &obj_BombPlacement::m_ActivationRadius, this, selected ) ;
	}

	return starty-scry;
}
#endif

