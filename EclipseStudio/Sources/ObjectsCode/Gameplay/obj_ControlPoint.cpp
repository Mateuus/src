#include "r3dPCH.h"
#include "r3d.h"

#include "Gameplay_Params.h"
#include "GameCommon.h"
#include "../ai/AI_Player.H"

#include "Particle.h"
#include "obj_ControlPoint.h"
#include "gameobjects/obj_Mesh.h"

#include "multiplayer/P2PMessages.h"
#include "multiplayer/ClientGameLogic.h"

#include "Editors/ObjectManipulator3d.h"

#include "..\..\ui\HUDMinimap.h"
#include "..\..\ui\HUDDisplay.h"
extern HUDMinimap*	hudMinimap;
extern HUDDisplay*	hudMain;

IMPLEMENT_CLASS(obj_ControlPoint, "obj_ControlPoint", "Object");
AUTOREGISTER_CLASS(obj_ControlPoint);

extern bool g_bEditMode;

int g_bShowDistanceToControlPoint = 0;

obj_ControlPoint::obj_ControlPoint()
{
	Torch        = NULL;

	for( int i = 0, e = R3D_ARRAYSIZE( flagMesh_ ) ; i < e; i ++ )
	{
		flagMesh_[ i ] = 0 ;
		flagSkels_[ i ] = 0 ;
		flagSkelShared_[ i ] = 0 ;
	}

	needToAddToMinimap = 0;
	m_SelectedSpawnPoint = 0;
	showCaptureProgressTimer = 0;
	CaptureProgressVisible = false;

	ControlName[0] = 0;
	UITagName = 0;
	UITagNameFull = 0;
	UITagNameLong = 0;
}

static int numControlPointsAddedToUI = 0;

obj_ControlPoint::~obj_ControlPoint()
{
	SAFE_DELETE(Torch);
	if(UITagName)
		numControlPointsAddedToUI--;

	for( int i = 0, e = R3D_ARRAYSIZE(flagSkels_) ; i < e; i ++ )
	{
		if( !flagSkelShared_[ i ] )
		{
			SAFE_DELETE( flagSkels_[ i ] ) ;
		}
	}
}

BOOL obj_ControlPoint::Load(const char *fname)
{
#ifndef FINAL_BUILD
	if(!g_bEditMode)
#endif
	{
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
			return TRUE; // do not load meshes in deathmatch mode, not showing control points
	}
	const char* cpMeshName = "Data\\ObjectsDepot\\Capture_Points\\Flag_Pole_01.sco";
	const char* flag1 = "Data\\ObjectsDepot\\Capture_Points\\CR_Control_Point_Flag_Blue_01.sco";
	const char* flag2 = "Data\\ObjectsDepot\\Capture_Points\\CR_Control_Point_Flag_Red_01.sco";
	const char* flag3 = "Data\\ObjectsDepot\\Capture_Points\\CR_Control_Point_Flag_White_01.sco";
	const char* flagAnimFile = "Data\\ObjectsDepot\\Capture_Points\\Animations\\Flag_Wind_01.anm";

	if(!parent::Load(cpMeshName)) 
		return FALSE;

#ifndef FINAL_BUILD
	if(!g_bEditMode)
#endif
		ObjFlags |= OBJFLAG_SkipCastRay;

#ifndef FINAL_BUILD
	if(g_bEditMode) // to make it easier in editor to edit spawn points
	{
		ObjFlags |= OBJFLAG_AlwaysDraw | OBJFLAG_SkipOcclusionCheck | OBJFLAG_ForceSleep ;
	}
#endif

	const char* pathes[ 3 ] = { flag1, flag2, flag3 } ;

	for( int i = 0 ; i < 3 ; i ++ )
	{
		flagMesh_[ i ]	= r3dGOBAddMesh( pathes[ i ] );

		if( flagMesh_[ i ]->IsSkeletal() )
		{
			obj_Building::LoadSkeleton( pathes[ i ], flagSkels_ + i, flagSkelShared_ + i );
		}
	}

	animPool_.Add( "default", flagAnimFile ) ;

	for( int i = 0, e = R3D_ARRAYSIZE(flagSkels_) ; i < e; i ++ )
	{
		if( r3dSkeleton* skel = flagSkels_[ i ] )
		{
			skel->SetDefaultPose();
			skel->ResetTransform();
			skel->Recalc();

			flagAnimations_[ i ].Init( skel, &animPool_ ) ;
			flagAnimations_[ i ].StartAnimation( "default", ANIMFLAG_Looped ) ;
		}
	}

	return TRUE;
}

BOOL obj_ControlPoint::OnCreate()
{
	parent::OnCreate();

#ifndef FINAL_BUILD
	if(!g_bEditMode) 
#endif
	{
		m_HUDCPVisible = false;
		m_HUDCPDist = -1;
		NetworkLocal = false;
		int cpID = gCPMgr.RegisterControlPoint(this);
		NetworkID    = cpID + NETID_CONTROLS_START;
#ifndef FINAL_BUILD
		r3dOutToLog("ControlPoint %d created\n", NetworkID);
#endif
		sprintf(ControlName, "ControlPointID%d", cpID);
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
			needToAddToMinimap = -1;
		else
			needToAddToMinimap = 30;

		if(!m_isTeamBase)
		{
			// it should be safe to generate names here, as update order should be deterministic for all players
			switch(numControlPointsAddedToUI)
			{
			case 0:
				UITagName = "a";
				UITagNameFull = "alpha";
				UITagNameLong = gLangMngr.getString("HUD_Point_Alpha");
				break;
			case 1:
				UITagName = "b";
				UITagNameFull = "bravo";
				UITagNameLong = gLangMngr.getString("HUD_Point_Bravo");
				break;
			case 2:
				UITagName = "c";
				UITagNameFull = "charlie";
				UITagNameLong = gLangMngr.getString("HUD_Point_Charlie");
				break;
			case 3:
				UITagName = "d";
				UITagNameFull = "delta";
				UITagNameLong = gLangMngr.getString("HUD_Point_Delta");
				break;
			case 4:
				UITagName = "z";
				UITagNameFull = "zulu";
				UITagNameLong = gLangMngr.getString("HUD_Point_Zulu");
				break;
			default:
				r3dError("Too many control points!\n"); // shouldn't happen in public build
			}
			++numControlPointsAddedToUI;
		}
	}

	return 1;
}


BOOL obj_ControlPoint::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_ControlPoint::Update()
{
	if(Torch)
	{ 
		if(!Torch->bEmit && !Torch->NumAliveParticles) 
			Torch->Restart(r3dGetTime());

		Torch->Position = GetPosition();
		Torch->Update(r3dGetTime());
	}

	int flagIdx = GetFlagIdx() ;

	if( flagSkels_[ flagIdx ] )
	{
		D3DXMATRIX ident ;
		D3DXMatrixIdentity( &ident ) ;
		flagAnimations_[ flagIdx ].Update( r3dGetAveragedFrameTime(), r3dPoint3D(0,0,0), ident ) ;
	}

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

	const obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(CaptureProgressVisible && localPlayer)
	{
		const char* type=0;
		if(localPlayer->TeamID == 0)
			type = status_<0.0f?"blue":"red";
		else
			type = status_<0.0f?"red":"blue";
		hudMain->setControlPointCaptureProgress(m_CPIcon, R3D_ABS(status_*100), type);
	}

	if(needToAddToMinimap>0 && hudMinimap && hudMinimap->IsInited() && localPlayer)
	{
		--needToAddToMinimap;
		if(needToAddToMinimap == 0) // need to skip frames to make sure that map is loaded, otherwise control point will not show up in minimap. weird flash
		{
			// don't show base flags on minimap, you cannot capture them anyway
			if(m_isTeamBase)
			{
				needToAddToMinimap = -1;
			}
			else
			{
				hudMinimap->AddControlPoint(ControlName, GetPosition());
				hudMain->addControlPointIcon(m_CPIcon);
				hudMain->setScreenIconScale(m_CPIcon, 0.5f);
				SetControlPointStatusOnMinimap();
			}
		}
	}
	if(needToAddToMinimap == 0 && localPlayer)
	{
		// find closest and furtherest point
		float minPoint = 1000000;
		float maxPoint = 0;
		{
			for(int i=0; i<gCPMgr.NumCP(); ++i)
			{
				BaseControlPoint* cp = gCPMgr.GetCP(i);
				if(cp->GetSpawnTeamId() == localPlayer->TeamID) // skip friendly
					continue;
				if(cp->m_isTeamBase)
					continue;
				float dist = (cp->GetPosition() - localPlayer->GetPosition()).Length();
				minPoint = R3D_MIN(dist, minPoint);
				maxPoint = R3D_MAX(dist, maxPoint);
			}
		}

		float distance = (GetPosition() - localPlayer->GetPosition()).Length();
		float alpha = 1.0f-R3D_CLAMP((distance-minPoint)/(maxPoint-minPoint+0.0001f), 0.0f, 0.75f);
		
		// points are always visible at the edge of the screen
		r3dPoint3D scrCoord;
		r3dProjectToScreenAlways(GetPosition(), &scrCoord, 20, 20);
		float xAxisLerp = 1.0f-powf((R3D_ABS(scrCoord.x - r3dRenderer->ScreenW2)/r3dRenderer->ScreenW2), 0.5f);
		float yAxisLerp = 1.0f-powf((R3D_ABS(scrCoord.y - r3dRenderer->ScreenH2)/r3dRenderer->ScreenH2), 0.5f);
		float axisLerp = R3D_MIN(xAxisLerp, yAxisLerp);
		alpha = R3D_LERP(1.0f, alpha, axisLerp);
		
		alpha = R3D_CLAMP(alpha, 0.2f, 1.0f);

		r3dPoint3D screenIconPos = GetPosition() + r3dPoint3D(0, 10, 0);
		float cpdist = 0;
		bool cpdistvis = false;

		if(CaptureProgressVisible && localPlayer && ((localPlayer->GetPosition() - GetPosition()).Length() < capture_radius)) // if local player capturing flag, move icon so that it's visible
		{
			hudMain->moveScreenIcon(m_CPIcon, r3dPoint3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.1f, 0.0f), true, false, true);
			cpdist = 0.0f;
			cpdistvis = false;
		}
		else
		{
			hudMain->moveScreenIcon(m_CPIcon, screenIconPos, true);
			float dist = (localPlayer->GetPosition() - GetPosition()).Length();
			cpdist = dist;
			cpdistvis = r3dProjectToScreen(screenIconPos, NULL)==0?false:true;
		}
		if(cpdistvis != m_HUDCPVisible || (cpdistvis && R3D_ABS(cpdist-m_HUDCPDist)>1.0f))
		{
			m_HUDCPVisible = cpdistvis;
			m_HUDCPDist = cpdist;
			hudMain->setControlPointDistance(m_CPIcon, m_HUDCPDist, m_HUDCPVisible);
		}
		hudMain->setScreenIconAlpha(m_CPIcon, alpha);
	}

	return parent::Update();
}

int obj_ControlPoint::GetFlagIdx()
{
	int flagIdx = GetSpawnTeamId();
	if(gClientLogic().localPlayer_ && flagIdx != 2)
	{
		if(flagIdx == gClientLogic().localPlayer_->TeamID)
			flagIdx = 0; // blue, ally
		else
			flagIdx = 1; // red, enemy
	}

	return flagIdx ;
}


void obj_ControlPoint::DrawFlagMesh(eRenderStageID DrawState)
{
	static r3dPoint3D flagOff    = r3dPoint3D(-0.024029f, 2.331320f, -0.041291f);
	static float      flagRaiseY = 10.5f;

	/*
	float neg = Keyboard->IsPressed(kbsLeftShift) ? -1.0f : 1.0f;
	float spd = 0.1f;
	r3dPoint3D& off = flagOff;
	if(Keyboard->IsPressed(kbs1)) off.x += r3dGetFrameTime() * spd * neg;
	if(Keyboard->IsPressed(kbs2)) off.y += r3dGetFrameTime() * spd * neg;
	if(Keyboard->IsPressed(kbs3)) off.z += r3dGetFrameTime() * spd * neg;
	r3dOutToLog("%f %f %f\n", off.x, off.y, off.z);
	*/

	D3DXMATRIX mr;

	// draw flag
	{
		int flagIdx = GetFlagIdx() ;

		r3dPoint3D p = flagOff;
		p.y += fabs(status_) * flagRaiseY;
		D3DXMatrixTranslation(&mr, p.x, p.y, p.z);
		mr *= GetRotationMatrix();
		mr._41 += GetPosition().x;
		mr._42 += GetPosition().y;
		mr._43 += GetPosition().z;

		if( flagMesh_[flagIdx]->IsSkeletal() )
		{
			if( flagSkels_[flagIdx] )
			{
				r3dAnimation* an = flagAnimations_ + flagIdx ;
				an->Recalc() ;
				r3dMeshSetVSConsts( mr, NULL, NULL );
				an->pSkeleton->SetShaderConstants() ;
			}
			else
			{
				flagMesh_[flagIdx]->SetVSConsts(mr);
				r3dSkeleton::SetDefaultShaderConstants() ;
			}
		}

		switch( DrawState )
		{
		case rsFillGBuffer:
			flagMesh_[flagIdx]->DrawMeshDeferred( r3dColor::white, 0 ) ;
			break ;

		case rsCreateSM:
			flagMesh_[flagIdx]->DrawMeshShadows();
			break ;

		default:
			flagMesh_[flagIdx]->DrawMeshSimple( 0 );
			break ;
		}
			
	}

	return;
}
//------------------------------------------------------------------------

struct ControlPointShadowGBufferRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		ControlPointShadowGBufferRenderable *This = static_cast<ControlPointShadowGBufferRenderable*>( RThis );

		This->Parent->MeshGameObject::Draw( Cam, This->DrawState );
		This->Parent->DrawFlagMesh( This->DrawState );
	}

	obj_ControlPoint*	Parent;
	eRenderStageID		DrawState;
};

struct ControlPointParticleRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		ControlPointParticleRenderable *This = static_cast<ControlPointParticleRenderable*>( RThis );

		This->Parent->GetTorch()->Draw( Cam, false );
	}

	obj_ControlPoint*	Parent;	
};

struct ControlPointCompositeRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		ControlPointCompositeRenderable *This = static_cast<ControlPointCompositeRenderable*>( RThis );

		This->Parent->DoDrawComposite( Cam );
	}

	obj_ControlPoint*	Parent;	
};



#define RENDERABLE_CTRLPOINT_SORT_VALUE (6*RENDERABLE_USER_SORT_VALUE)

/*virtual*/

void
obj_ControlPoint::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) /*OVERRIDE*/
{
#ifndef FINAL_BUILD
	if(!g_bEditMode)
#endif
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
			return;

	ControlPointShadowGBufferRenderable rend;

	rend.Init();
	rend.Parent		= this;
	rend.SortValue	= RENDERABLE_CTRLPOINT_SORT_VALUE;
	rend.DrawState	= rsCreateSM;

	rarr.PushBack( rend );
}

//------------------------------------------------------------------------
/*virtual*/

void
obj_ControlPoint::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) /*OVERRIDE*/
{
	// not drawing CP in deathmatch
#ifndef FINAL_BUILD
	if(!g_bEditMode)
#endif
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
			return;

	// gbuffer
	{
		ControlPointShadowGBufferRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_CTRLPOINT_SORT_VALUE;
		rend.DrawState	= rsFillGBuffer;

		render_arrays[ rsFillGBuffer ].PushBack( rend );
	}

	// particles
	if( Torch )
	{
		ControlPointParticleRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_CTRLPOINT_SORT_VALUE;

		render_arrays[ rsDrawTransparents ].PushBack( rend );
	}
#ifdef FINAL_BUILD
	return;
#else
	if(r_hide_icons->GetInt())
		return;

	// composite
	if(g_bEditMode)
	{
		ControlPointCompositeRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_CTRLPOINT_SORT_VALUE;

		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}
#endif

}

//------------------------------------------------------------------------
void obj_ControlPoint::DoDrawComposite( const r3dCamera& Cam )
{
#ifndef FINAL_BUILD
	// draw them, so we can see them
	r3dPoint3D off(0, 4, 0);
	r3dColor   clr(255, 255, 255);
	if(GetSpawnTeamId() == 0) clr = r3dColor(0, 0, 255);
	if(GetSpawnTeamId() == 1) clr = r3dColor(255, 0, 0);
	
	r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);
	
	r3dDrawLine3D(GetPosition(), GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, clr);
	r3dDrawCircle3D(GetPosition(), capture_radius, Cam, 0.1f, r3dColor(0,255,0));

	
	//r3dDrawBoundBox(GetBBoxWorld(), Cam, clr);

	// draw circles
	for(int i=0; i<m_NumSpawnPoints; ++i)
	{
		r3dDrawCircle3D(m_SpawnPoints[i], 2.0f, Cam, 0.4f, ((i==m_SelectedSpawnPoint&&g_Manipulator3d.PickedObject() == this)?r3dColor24::red:r3dColor24::grey));
		r3dDrawLine3D(m_SpawnPoints[i], m_SpawnPoints[i]+r3dVector(m_SpawnDir[i].x, 0.0f, m_SpawnDir[i].z)*2.0f, Cam, 0.4f, r3dColor24::blue);
	}
	r3dRenderer->Flush();
	r3dRenderer->SetRenderingMode(R3D_BLEND_POP);

	if(g_bShowDistanceToControlPoint)
	{
		r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_ALPHA | R3D_BLEND_PUSH);
		r3dPoint3D scrCoord;
		if(r3dProjectToScreen(GetPosition()+r3dPoint3D(0,15,0), &scrCoord))
			Font_Label->PrintF(scrCoord.x, scrCoord.y, r3dColor24::white, "%.2f", (Cam-GetPosition()).Length());
		r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		r3dRenderer->Flush();
	}
#endif
}

//------------------------------------------------------------------------
#ifndef FINAL_BUILD
float obj_ControlPoint::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{		
		starty += imgui_Static ( scrx, starty, "Control Point Parameters" );
		starty += imgui_Value_Slider(scrx, starty, "Capture Radius", &capture_radius, 1.0f, 100.0f, "%.2f");
		starty += imgui_Value_Slider(scrx, starty, "Spawn Protection Time(sec)", &spawnProtectionTime, 0.1f, 50.0f, "%.1f");
		starty += imgui_Checkbox(scrx, starty, "Show Distances", &g_bShowDistanceToControlPoint, 1);

		static const char* spawntypes[] = { "BLUE", "RED", "FREE"};

		int sel = spawnType_ ;
		Edit_Value_List(scrx, starty, "TYPE", &sel, spawntypes, R3D_ARRAYSIZE(spawntypes));
		PropagateChange( sel, &obj_ControlPoint::spawnType_, this, selected ) ;

		// don't allow multi edit of this
		if( selected.Count() <= 1 )
		{
			if(m_NumSpawnPoints < MAX_SPAWN_POINTS)
			{
				if(imgui_Button(scrx+110, starty, 100, 25, "Add Location"))
				{
					m_SpawnPoints[m_NumSpawnPoints] = GetPosition() + r3dPoint3D(2, 0, 2);
					m_SpawnDir[m_NumSpawnPoints].Assign(0,0,1);
					m_NumSpawnPoints++;
				}
			}

			starty += 25;

			for(int i=0; i<m_NumSpawnPoints; ++i)
			{
				// selection button
				char tempStr[32];
				sprintf(tempStr, "Location %d", i+1);
				if(imgui_Button(scrx, starty, 100, 25, tempStr, i==m_SelectedSpawnPoint))
				{
					// shift click on location will set camera to it
					if(Keyboard->IsPressed(kbsLeftShift))
					{
						extern BaseHUD* HudArray[6];
						extern int CurHUDID;
						HudArray[CurHUDID]->FPS_Position = m_SpawnPoints[i];
						HudArray[CurHUDID]->FPS_Position.y += 0.1f;
					}
					
					m_SelectedSpawnPoint = i;
				}
				
				// delete button
				if(m_NumSpawnPoints > 1)
				{
					if(imgui_Button(scrx + 110, starty, 100, 25, "DEL"))
					{
						if(i!=m_NumSpawnPoints-1)
						{
							m_SpawnPoints[i] = m_SpawnPoints[m_NumSpawnPoints-1]; // just move last point into this one
							m_SpawnDir[i] = m_SpawnDir[m_NumSpawnPoints-1];
						}
						
						m_NumSpawnPoints--;
					}
				}
				imgui_Value_Slider(scrx+220, starty, "Direction", &m_SpawnDir[i].y, 0.0f, 360.0f, "%.2f", false, true, 160);
				D3DXMATRIX tempM;
				D3DXMatrixRotationY(&tempM, R3D_DEG2RAD(-m_SpawnDir[i].y));
				D3DXVECTOR3 tempV, tempV2; tempV2 = D3DXVECTOR3(0,0,1);
				D3DXVec3TransformNormal(&tempV, &tempV2, &tempM);
				m_SpawnDir[i].x = tempV.x; m_SpawnDir[i].z = tempV.z;

				m_SelectedSpawnPoint = R3D_CLAMP(m_SelectedSpawnPoint, 0, m_NumSpawnPoints-1);
				starty += 25;
			}

			extern r3dPoint3D UI_TargetPos;
			if((Mouse->IsPressed(r3dMouse::mLeftButton)) && Keyboard->IsPressed(kbsLeftControl))
				m_SpawnPoints[m_SelectedSpawnPoint] = UI_TargetPos;

			UpdateStatusByType();
		}
	}

	return starty-scry;
}
#endif

void obj_ControlPoint::SetControlPointStatusOnMinimap()
{
	const obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(hudMinimap == NULL || hudMain == NULL)
		return;
	
	if(!hudMinimap->IsInited() || localPlayer==0 || m_CPIcon.IsUndefined())
		return;

	int teamID = GetSpawnTeamId();
	if(teamID == localPlayer->TeamID)
	{
		hudMinimap->SetControlPointStatus(ControlName, "blue", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "blue", UITagName);
	}
	else if(teamID == 2)
	{
		hudMinimap->SetControlPointStatus(ControlName, "grey", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "grey", UITagName);
	}
	else
	{
		hudMinimap->SetControlPointStatus(ControlName, "red", UITagName);
		hudMain->setControlPointIconStatus(m_CPIcon, "red", UITagName);
	}
}

BOOL obj_ControlPoint::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	switch(EventID)
	{
	default: return FALSE;

	case PKT_S2C_ControlPointUpdate:
		{
			const PKT_S2C_ControlPointUpdate_s& n = *(PKT_S2C_ControlPointUpdate_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			float prevStatus = status_;
			status_ = n.unpack();
			if(needToAddToMinimap == 0 && gClientLogic().localPlayer_ && R3D_ABS(prevStatus-status_)>0.001f) // otherwise when you join a game it will show that enemy base was captured :)
			{
				extern int RUS_CLIENT;
				if(!RUS_CLIENT)
				{
					if(!CaptureProgressVisible)
					{
						float status_diff = prevStatus - status_;
						if(g_enable_voice_commands->GetBool())
						{
							if(status_diff < 0) // going towards 1.0
							{
								if(gClientLogic().localPlayer_->TeamID == 0) // enemy capturing
								{
									char tmpStr[64];
									sprintf(tmpStr, "Sounds/VoiceOver/ENG/we are losing %s", UITagNameFull);								
									snd_PlaySound(SoundSys.GetEventIDByPath(tmpStr), r3dPoint3D(0,0,0));
								}
								else // friendly capturing
								{
									char tmpStr[64];
									sprintf(tmpStr, "Sounds/VoiceOver/ENG/capturing %s point", UITagNameFull);								
									snd_PlaySound(SoundSys.GetEventIDByPath(tmpStr), r3dPoint3D(0,0,0));
								}
							}
							else // going towards -1.0f
							{
								if(gClientLogic().localPlayer_->TeamID == 0) // friendly capturing
								{
									char tmpStr[64];
									sprintf(tmpStr, "Sounds/VoiceOver/ENG/capturing %s point", UITagNameFull);								
									snd_PlaySound(SoundSys.GetEventIDByPath(tmpStr), r3dPoint3D(0,0,0));
								}
								else // enemy capturing
								{
									char tmpStr[64];
									sprintf(tmpStr, "Sounds/VoiceOver/ENG/we are losing %s", UITagNameFull);								
									snd_PlaySound(SoundSys.GetEventIDByPath(tmpStr), r3dPoint3D(0,0,0));
								}
							}
						}
					}
				}

				showCaptureProgressTimer = 3.0f; // show progress update
				if(status_ == -1.0f || status_ == 1.0f)
				{
					const char* type=0;
					if(gClientLogic().localPlayer_->TeamID == 0)
						type = status_==-1.0f?"blue":"red";
					else
						type = status_==1.0f?"blue":"red";

					wchar_t tempStr[128];
					swprintf(tempStr, 128, gLangMngr.getString("HUD_Point_Captured"), UITagNameLong);
					hudMain->showPointCapturedInfo(type, tempStr);
				}
			}

			SetControlPointStatusOnMinimap();
			//r3dOutToLog("CP%p status updated to %f\n", this, status_);
			break;
		}
	}

	return TRUE;
}
