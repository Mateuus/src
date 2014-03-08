#include "r3dPCH.h"
#include "r3d.h"

#include "Gameplay_Params.h"
#include "GameCommon.h"
#include "obj_SiegeObjective.h"
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

IMPLEMENT_CLASS(obj_SiegeObjective, "obj_SiegeObjective", "Object");
AUTOREGISTER_CLASS(obj_SiegeObjective);

extern bool g_bEditMode;

static r3dTexture *SiegeObjectiveIcon = NULL;

SiegeObjectiveMngr gSiegeObjMgr;

obj_SiegeObjective::obj_SiegeObjective()
{
	m_DestructionTimer = 30.0f;
	m_ActivationTimer = 5.0f;

	m_BlueSpawnHashID = -1;
	m_RedSpawnHashID = -1;

	ControlName[0] = 0;
	UITagName = 0;
	UITagNameLong = 0;
	showCaptureProgressTimer = 0;
	CaptureProgressVisible = false;

	m_CurrentTimer = 0;
}

static int numControlPointsAddedToUI = 0;
obj_SiegeObjective::~obj_SiegeObjective()
{
	if(UITagName)
		numControlPointsAddedToUI--;
}

BOOL obj_SiegeObjective::Load(const char *fname)
{
	if(!parent::Load(fname)) return FALSE;

	if(g_bEditMode)
		if (!SiegeObjectiveIcon) SiegeObjectiveIcon = r3dRenderer->LoadTexture("Data\\Images\\SiegeObjective.dds");

	return TRUE;
}

BOOL obj_SiegeObjective::OnCreate()
{
	parent::OnCreate();

	DrawOrder	= OBJ_DRAWORDER_LAST;
	ObjFlags	|=	OBJFLAG_SkipOcclusionCheck | OBJFLAG_DisableShadows;

	if(!g_bEditMode) {
		NetworkLocal = false;
		int id = gSiegeObjMgr.RegisterSiegeObjective(this);
		NetworkID    = id + NETID_SIEGEOBJ_START;
		r3dOutToLog("SiegeObj %d created\n", NetworkID);
		sprintf(ControlName, "SiegeObjID%d", id);

		needToAddToMinimap = 30;
		// it should be safe to generate names here, as update order should be deterministic for all players
		switch(numControlPointsAddedToUI)
		{
		case 0:
			UITagName = "a";
			UITagNameLong = "ALPHA";
			break;
		case 1:
			UITagName = "b";
			UITagNameLong = "BRAVO";
			break;
		case 2:
			UITagName = "c";
			UITagNameLong = "CHARLIE";
			break;
		case 3:
			UITagName = "d";
			UITagNameLong = "DELTA";
			break;
		case 4:
			UITagName = "z";
			UITagNameLong = "ZULU";
			break;
		default:
			r3dError("Too many control points!\n"); // shouldn't happen in public build
		}
		++numControlPointsAddedToUI;
	}

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;
	SetBBoxLocal( bboxLocal ) ;
	UpdateTransform();

	return 1;
}

BOOL obj_SiegeObjective::Update()
{
	parent::Update();
	
	const ClientGameLogic& CGL = gClientLogic();

	if(showCaptureProgressTimer>0)
		showCaptureProgressTimer = R3D_MAX(showCaptureProgressTimer-r3dGetFrameTime(), 0.0f);

	if(showCaptureProgressTimer > 0 && !CaptureProgressVisible)
	{
		hudMain->setVisibleControlPointCaptureProgress(m_CPIcon, true);
		CaptureProgressVisible = true;
	}
	if(showCaptureProgressTimer == 0 && CaptureProgressVisible)
	{
		hudMain->setVisibleControlPointCaptureProgress(m_CPIcon, false);
		CaptureProgressVisible = false;
	}

	if(CaptureProgressVisible && CGL.localPlayer_)
	{
		hudMain->setControlPointCaptureProgress(m_CPIcon, (1.0f-((m_DestructionTimer-m_CurrentTimer)/m_DestructionTimer))*100.0f, "red");
	}

	if(needToAddToMinimap>0 && hudMinimap && hudMinimap->IsInited() && CGL.localPlayer_)
	{
		--needToAddToMinimap;
		if(needToAddToMinimap == 0) // need to skip frames to make sure that map is loaded, otherwise control point will not show up in minimap. weird flash
		{
			hudMinimap->AddControlPoint(ControlName, GetPosition());
			hudMain->addControlPointIcon(m_CPIcon);
			hudMain->setScreenIconScale(m_CPIcon, 0.75f);
			SetStatusOnMinimap();
			UpdateBombStatusHUD();
		}
	}
	if(needToAddToMinimap == 0 && CGL.localPlayer_)
	{
		float alpha = 1.0f;

		// points are always visible at the edge of the screen
		r3dPoint3D scrCoord;
		r3dProjectToScreenAlways(GetPosition(), &scrCoord, 20, 20);
		float xAxisLerp = 1.0f-powf((R3D_ABS(scrCoord.x - r3dRenderer->ScreenW2)/r3dRenderer->ScreenW2), 0.5f);
		float yAxisLerp = 1.0f-powf((R3D_ABS(scrCoord.y - r3dRenderer->ScreenH2)/r3dRenderer->ScreenH2), 0.5f);
		float axisLerp = R3D_MIN(xAxisLerp, yAxisLerp);
		alpha = R3D_LERP(1.0f, alpha, axisLerp);

		alpha = R3D_CLAMP(alpha, 0.2f, 1.0f);

		if(Status == SS_DESTROYED || Status == SS_DISABLED)
			alpha = 0;

		r3dPoint3D screenIconPos = GetPosition() + r3dPoint3D(0, 3, 0);
		if(CaptureProgressVisible && CGL.localPlayer_ && ((gClientLogic().localPlayer_->GetPosition() - GetPosition()).Length() < 5.0f)) // if local player capturing flag, move icon so that it's visible
		{
			hudMain->moveScreenIcon(m_CPIcon, r3dPoint3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.1f, 0.0f), true, false, true);
		}
		else
			hudMain->moveScreenIcon(m_CPIcon, screenIconPos, true);
		hudMain->setScreenIconAlpha(m_CPIcon, alpha);
	}

	return TRUE;
}


BOOL obj_SiegeObjective::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
	default: return FALSE;

	case PKT_S2C_SiegeUpdate:
		{
			const PKT_S2C_SiegeUpdate_s& n = *(PKT_S2C_SiegeUpdate_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			SIEGE_STATUS prevStatus = Status;
			Status = (SIEGE_STATUS)n.status;
			m_CurrentTimer = n.destruction_timer;
			
			if(needToAddToMinimap == 0 && gClientLogic().localPlayer_ && m_CurrentTimer > 0)
			{
				showCaptureProgressTimer = 1.0f; // show progress update
			}

			if(prevStatus != Status)
			{
				UpdateBombStatusHUD();
				if(Status == SS_ARMED)
					hudMain->ShowAchievementCustom(gLangMngr.getString("$HUD_Msg_BombArmed"), "", "$Data/Menu/achievements/hud/boomer.png", "");
				if(Status == SS_ACTIVE && prevStatus == SS_ARMED)
					hudMain->ShowAchievementCustom(gLangMngr.getString("$HUD_Msg_BombDisarmed"), "", "$Data/Menu/achievements/hud/boomer.png", "");
				if(Status == SS_DESTROYED)
				{
					wchar_t tempStr[64];
					swprintf(tempStr, 64, gLangMngr.getString("$HUD_Msg_BombDestroyed"), UITagNameLong);
					hudMain->ShowAchievementCustom(tempStr, "", "$Data/Menu/achievements/hud/boomer.png", "");
					
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
				}
			}

			SetStatusOnMinimap();
			break;
		}
	}

	return TRUE;
}

void obj_SiegeObjective::UpdateBombStatusHUD()
{
	int objInd = gSiegeObjMgr.getObjectiveIndex(this);
	if(objInd>=0)
	{
		if(Status == SS_ACTIVE)
			hudMain->SetBombState(objInd, "neutral");
		else if(Status == SS_ARMED)
			hudMain->SetBombState(objInd, "timed");
		else if(Status == SS_DESTROYED)
			hudMain->SetBombState(objInd, "exploded");
		//else
		//	hudMain->SetBombState(objInd, "disabled");
	}
}

void obj_SiegeObjective::SetStatusOnMinimap()
{
	if(hudMinimap == NULL)
		return;

	if(!hudMinimap->IsInited() || gClientLogic().localPlayer_==0)
		return;

	if(Status == SS_ACTIVE)
	{
		hudMinimap->SetControlPointStatus(ControlName, "blue", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "blue", UITagName);
	}
	else if(Status == SS_ARMED)
	{
		hudMinimap->SetControlPointStatus(ControlName, "red", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "red", UITagName);
	}
	else
	{
		hudMinimap->SetControlPointStatus(ControlName, "grey", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "grey", UITagName);
	}
}


void obj_SiegeObjective::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("SiegeObj");
	m_DestructionTimer = myNode.attribute("DestructionTimer").as_float();
	m_ActivationTimer = myNode.attribute("ActivationTimer").as_float();
	m_BlueSpawnHashID = myNode.attribute("BlueHashID").as_uint();
	m_RedSpawnHashID = myNode.attribute("RedHashID").as_uint();
}

void obj_SiegeObjective::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node myNode = node.append_child();
	myNode.set_name("SiegeObj");
	myNode.append_attribute("DestructionTimer") = m_DestructionTimer;
	myNode.append_attribute("ActivationTimer") = m_ActivationTimer;
	myNode.append_attribute("BlueHashID") = m_BlueSpawnHashID;
	myNode.append_attribute("RedHashID") = m_RedSpawnHashID;
}

void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
struct SiegeObjRenderableHelper : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		SiegeObjRenderableHelper *This = static_cast<SiegeObjRenderableHelper*>( RThis );

		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZC );

		if((This->Parent->GetPosition() - Cam).Length() < 100)
			r3dDrawIcon3D(This->Parent->GetPosition(), SiegeObjectiveIcon, r3dColor(255,255,255), 24);
	}

	obj_SiegeObjective* Parent;	
};

struct SiegeObjCompositeRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		SiegeObjCompositeRenderable *This = static_cast<SiegeObjCompositeRenderable*>( RThis );

		r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

		if(This->Parent->m_BlueSpawnHashID != -1)
		{
			GameObject* obj = GameWorld().GetObjectByHash(This->Parent->m_BlueSpawnHashID);
			if(obj)
				r3dDrawLine3D(This->Parent->GetPosition(), obj->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor(0, 0, 255));
			else
				This->Parent->m_BlueSpawnHashID = -1;
		}
		if(This->Parent->m_RedSpawnHashID != -1)
		{
			GameObject* obj = GameWorld().GetObjectByHash(This->Parent->m_RedSpawnHashID);
			if(obj)
				r3dDrawLine3D(This->Parent->GetPosition(), obj->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor(255, 0, 0));
			else
				This->Parent->m_RedSpawnHashID = -1;
		}

		r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		r3dRenderer->Flush();
	}

	obj_SiegeObjective*	Parent;	
};


#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_SiegeObjective::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
#ifdef FINAL_BUILD
	return;
#else
	// don't draw debug info if we're not in editor mode
	if ( !g_bEditMode )
		return;

	if( r_hide_icons->GetInt() )
		return ;

// 	if(g_Manipulator3d.PickedObject() == this)
// 	{
// 		DamageAreaRenderable rend;
// 		rend.Init();
// 		rend.SortValue = RENDERABLE_OBJ_USER_SORT_VALUE;
// 		rend.Parent = this;
// 		render_arrays[ rsDrawDebugData ].PushBack( rend );
// 	}

	// helper
	extern int CurHUDID;
	if(CurHUDID == 0)
	{
		SiegeObjRenderableHelper rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}

	// composite
	if(g_bEditMode)
	{
		SiegeObjCompositeRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;

		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}
#endif
}

#ifndef FINAL_BUILD
float obj_SiegeObjective::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		starty += imgui_Static ( scrx, starty, "Siege Objective Properties" );

		static float destructionTimerVal = 0.f ;
		destructionTimerVal = m_DestructionTimer ;
		starty += imgui_Value_Slider(scrx, starty, "Destruction Timer", &destructionTimerVal,	0.0f,300,	"%.2f",1);
		PropagateChange( destructionTimerVal, &obj_SiegeObjective::m_DestructionTimer, this, selected ) ;
		
		static float activationTimerVal ;
		activationTimerVal = m_ActivationTimer ;
		starty += imgui_Value_Slider(scrx, starty, "Activation Timer", &activationTimerVal,	0.0f,30.0f,	"%.2f",1);
		PropagateChange( activationTimerVal, &obj_SiegeObjective::m_ActivationTimer, this, selected ) ;

		// find a spawn point connected to this siege objective

		if( selected.Count() <= 1 )
		{
			extern r3dPoint3D UI_TargetPos;
			if((Mouse->IsPressed(r3dMouse::mLeftButton)) && Keyboard->IsPressed(kbsLeftControl))
			{
				obj_ControlPoint* closestCP = NULL;
				float dist = 9999999999.0f;
				for(GameObject *obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj))
				{
					if(obj->Class->Name == "obj_ControlPoint")
					{
						obj_ControlPoint* cp = (obj_ControlPoint*)obj;
						float d = (UI_TargetPos - cp->GetPosition()).Length();
						if(d < dist)
						{
							closestCP = cp;
							dist = d;
						}
					}
				}
				if(closestCP)
				{
					if(closestCP->spawnType_ == 0) // blue
						m_BlueSpawnHashID = closestCP->GetHashID();
					else if(closestCP->spawnType_ == 1) // red
						m_RedSpawnHashID = closestCP->GetHashID();
				}
			}
		}
	}

	return starty-scry;
}
#endif