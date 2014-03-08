#include "r3dPCH.h"

#include "r3d.h"
#include "d3dfont.h"

#include "GameCommon.h"
#include "UI\HUD_EditorGame.h"

#include "rendering/Deffered/CommonPostFX.h"

#include "HUDCameraEffects.h"

#include "ObjectsCode\AI\AI_Player.h"
#include "ObjectsCode\AI\AI_PlayerAnim.h"
#include "ObjectsCode\weapons\Weapon.h"
#include "ObjectsCode\weapons\WeaponArmory.h"

extern float GameFOV;

gobjid_t	EditorGameHUD::editorPlayerId = invalidGameObjectID;

void EditorGameHUD :: InitPure()
{
}

void EditorGameHUD :: DestroyPure()
{
}

void EditorGameHUD :: SetCameraDir (r3dPoint3D vPos )
{

}

r3dPoint3D EditorGameHUD :: GetCameraDir () const
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId);
	if ( pl )
		return pl->m_vVision;
	else
		return r3dVector(1,0,0);
}


	extern int g_RenderRadialBlur;

void SetFocusPoint(r3dPoint3D &P)
{
	extern int _FAR_DOF;
	extern int _NEAR_DOF;

	_FAR_DOF = 1;
	_NEAR_DOF = 1;

extern float DepthOfField_NearStart;
extern float DepthOfField_NearEnd;

extern float DepthOfField_FarStart;
extern float DepthOfField_FarEnd;

float FocusDist = (r3dRenderer->CameraPosition - P).Length();

DepthOfField_NearStart = 0.5f;
DepthOfField_NearEnd = R3D_MIN(2.0f, FocusDist/2);

DepthOfField_FarStart = FocusDist;
DepthOfField_FarEnd = FocusDist+25.0f;

}

	extern r3dPoint3D	UI_TargetPos;	

void RigEffect_Clear (float Lerp)
{
	PFX_RadialBlur::Settings sts = gPFX_RadialBlur.GetDefaultSettings();
	sts.BlurStrength = 0.015f*(1.0f-Lerp);
	gPFX_RadialBlur.SetDefaultSettings( sts );

	//SetFocusPoint(UI_TargetPos);



  if (Lerp > 0.95 ) 
      g_RenderRadialBlur = 0;
}



void RigEffect_SprintBlur (float Lerp)
{
	g_RenderRadialBlur = 1;

	PFX_RadialBlur::Settings sts = gPFX_RadialBlur.GetDefaultSettings();

	sts.BlurStart = 0.45f;
	sts.BlurStrength = 0.007f*Lerp;

	gPFX_RadialBlur.SetDefaultSettings( sts );
}

void PlayerStateVars_s::Lerp(obj_AI_Player* pl, PlayerStateVars_s& s1, PlayerStateVars_s& s2, float lerpV)
{
	if(s2.allowScope && pl->hasScopeMode() && pl->m_isAiming)
	{
		Position = R3D_LERP(s1.Position, s2.ScopePosition, lerpV);
		
		float ScopeFOV = s2.FOV;
		float ScopeMouseSensitivity = s2.MouseSensetivity;
		const Weapon* wpn = pl->m_Weapons[pl->m_SelectedWeapon];
		if(wpn)
		{
			ScopeFOV = R3D_LERP(s2.FOV, s2.ScopeFOV, wpn->getWeaponScopeZoom());
			ScopeMouseSensitivity = R3D_LERP(s2.MouseSensetivity, s2.ScopeMouseSensitivity, wpn->getWeaponScopeZoom());
		}
		
		FOV = R3D_LERP(s1.FOV, ScopeFOV, lerpV);
		MouseSensetivity = R3D_LERP(s1.MouseSensetivity, ScopeMouseSensitivity, lerpV);
		if(lerpV<0.3f) 
			allowScope = s1.allowScope;
		else 
			allowScope = s2.allowScope;
	}
	else
	{
		Position = R3D_LERP(s1.Position, s2.Position, lerpV);
		allowScope = false;
		FOV = R3D_LERP(s1.FOV, s2.FOV, lerpV);
		MouseSensetivity = R3D_LERP(s1.MouseSensetivity, s2.MouseSensetivity, lerpV);
	}

	FPS_FOV = R3D_LERP(s1.FPS_FOV, s2.FPS_FOV, lerpV);
	r_first_person_fov->SetFloat(FPS_FOV);

	LookUpLimit = R3D_LERP(s1.LookUpLimit, s2.LookUpLimit, lerpV);
	LookDownLimit = R3D_LERP(s1.LookDownLimit, s2.LookDownLimit, lerpV);
}

//////////////////////////////////////////////////////////////////////////

// THIRD PERSON CAMERA, CROSS HAIR OFFSET TO THE TOP OF THE SCREEN!
const r3dPoint3D gCameraPosIdle(0.0f, -0.3f, -3.0f);
const r3dPoint3D gCameraPosIdleAim(0.8f, -0.2f, -1.6f);
const r3dPoint3D gCameraPosScope(0.0f, -0.4f, 0.0f);
const r3dPoint3D gCameraPosSprint(0.0f,-0.3f,-3.5f);

const r3dPoint3D gCameraPosCrouch(0.0f, -0.4f,-2.7f);
const r3dPoint3D gCameraPosCrouchAim(0.8f, -0.3f, -1.47f);
const r3dPoint3D gCameraPosCrouchScope(0.0f, -0.6f, 0.0f);

// THIRD PERSON CAMERA, CROSS HAIR IN THE CENTER OF THE SCREEN!
const r3dPoint3D gCameraPosIdle2(0.5f, -0.2f, -2.0f);
const r3dPoint3D gCameraPosIdleAim2(0.73f, -0.32f, -1.05f);
const r3dPoint3D gCameraPosScope2(0.0f, -0.32f, 0.0f);
const r3dPoint3D gCameraPosSprint2(0.5f, -0.2f, -2.52f);

const r3dPoint3D gCameraPosCrouch2(0.5f,-0.7f,-1.5f);
const r3dPoint3D gCameraPosCrouchAim2(0.57f, -0.7f, -1.05f);
const r3dPoint3D gCameraPosCrouchScope2(0.15f, -0.7f, 0.0f);

// FIRST PERSON CAMERA, CROSS HAIR IN THE CENTER OF THE SCREEN!
const r3dPoint3D gCameraPosIdle3(0.0f, -.19f, 0.0f);
const r3dPoint3D gCameraPosIdleAim3(0.0f, -.19f, 0.0f);
const r3dPoint3D gCameraPosScope3(0.0f, -.19f, 0.0f);
const r3dPoint3D gCameraPosSprint3(0.0f, -.19f, 0.0f);

const r3dPoint3D gCameraPosCrouch3(0.0f, -.6f, 0.0f);
const r3dPoint3D gCameraPosCrouchAim3(0.0f, -.6f, 0.0f);
const r3dPoint3D gCameraPosCrouchScope3(0.0f, -.6f, 0.0f);

//////////////////////////////////////////////////////////////////////////

const float gMaximumScopeFov = 3;
const float gMaximumScopeSens = 0.005f;

r3dPoint3D TPSHudCameraTarget(0.0f, 0.0f, 2.0f); // pt: target should be the same for all states, otherwise your aim will move and make it very difficult to shoot someone

float	TPSCameraPointToAdj[3] = {0.0f, 0.0f, 0.0f };
float   TPSCameraPointToAdjCrouch[3] = {0.0f, 0.0f, 0.0f};

PlayerStateVars_s TPSHudCameras[3][PLAYER_NUM_STATES] = 
{
	//////////////////////////////////////////////////////////////
	// 0 - TPS, CROSSHAIR IN CENTER
	{
		// PLAYER_IDLE
		{
			gCameraPosIdle2,
			gCameraPosScope2,
			60, 60, 60,
			0.26f,0.26f, 
			70.0f, -70.0f, false, RigEffect_Clear
		},	
		// PLAYER_IDLEAIM
		{
			gCameraPosIdleAim2,
			gCameraPosScope2,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			70.0f, -70.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH
		{
			gCameraPosCrouch2,
			gCameraPosCrouchScope2,
			60, 60, 60,
			0.26f,0.26f, 
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH_AIM
		{
			gCameraPosCrouchAim2,
			gCameraPosCrouchScope2,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_WALK_AIM
		{
			gCameraPosIdleAim2,
			gCameraPosScope2,
			30, gMaximumScopeFov, 30,
			0.1f,gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_RUN
		{
			gCameraPosIdle2,
			gCameraPosScope2,
			60, 60, 60,
			0.26f,0.26f,
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_SPRINT
		{
			gCameraPosSprint2,
			gCameraPosScope2,
			65.0f, 65.0f, 50,
			0.15f,0.15f,
			20.0f, -20.0f, false, RigEffect_SprintBlur
		},

		// PLAYER_DIE
		{
			gCameraPosSprint2,
			gCameraPosScope2,
			47, 47, 50,
			0.26f,0.26f,
			20.0f, -20.0f, false, RigEffect_Clear
		}
	},
	//////////////////////////////////////////////////////////////
	// 1 - TPS, CROSSHAIR OFFCENTER
	{
		// PLAYER_IDLE
		{
			gCameraPosIdle,
			gCameraPosScope,
			50, 50, 50,
			0.26f,0.26f, 
			70.0f, -70.0f, false, RigEffect_Clear
		},	
		// PLAYER_IDLEAIM
		{
			gCameraPosIdleAim,
			gCameraPosScope,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			70.0f, -70.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH
		{
			gCameraPosCrouch,
			gCameraPosCrouchScope,
			50, 50, 50,
			0.26f,0.26f, 
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH_AIM
		{
			gCameraPosCrouchAim,
			gCameraPosCrouchScope,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_WALK_AIM
		{
			gCameraPosIdleAim,
			gCameraPosScope,
			30, gMaximumScopeFov, 30,
			0.1f,gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_RUN
		{
			gCameraPosIdle,
			gCameraPosScope,
			50, 50, 50,
			0.26f,0.26f,
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_SPRINT
		{
			gCameraPosSprint,
			gCameraPosScope,
			55.0f, 55.0f, 50,
			0.15f,0.15f,
			20.0f, -20.0f, false, RigEffect_SprintBlur
		},

		// PLAYER_DIE
		{
			gCameraPosSprint,
			gCameraPosScope,
			47, 47, 50,
			0.26f,0.26f,
			20.0f, -20.0f, false, RigEffect_Clear
		}
	},
	//////////////////////////////////////////////////////////////
	// 2 - FPS, CROSSHAIR IN CENTER
	{
		// PLAYER_IDLE
		{
			gCameraPosIdle3,
			gCameraPosScope3,
			60, 60, 50,
			0.26f,0.26f, 
			70.0f, -70.0f, false, RigEffect_Clear
		},	
		// PLAYER_IDLEAIM
		{
			gCameraPosIdleAim3,
			gCameraPosScope3,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			70.0f, -70.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH
		{
			gCameraPosCrouch3,
			gCameraPosCrouchScope3,
			60, 60, 50,
			0.26f,0.26f, 
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_CROUCH_AIM
		{
			gCameraPosCrouchAim3,
			gCameraPosCrouchScope3,
			30, gMaximumScopeFov, 30,
			0.1f, gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_WALK_AIM
		{
			gCameraPosIdleAim3,
			gCameraPosScope3,
			30, gMaximumScopeFov, 30,
			0.1f,gMaximumScopeSens,
			60.0f, -60.0f, true, RigEffect_Clear
		},

		// PLAYER_MOVE_RUN
		{
			gCameraPosIdle3,
			gCameraPosScope3,
			60, 60, 50,
			0.26f,0.26f,
			60.0f, -60.0f, false, RigEffect_Clear
		},

		// PLAYER_MOVE_SPRINT
		{
			gCameraPosSprint3,
			gCameraPosScope3,
			65.0f, 65.0f, 50,
			0.15f,0.15f,
			20.0f, -20.0f, false, RigEffect_SprintBlur
		},

		// PLAYER_DIE
		{
			gCameraPosSprint3,
			gCameraPosScope3,
			47, 47, 50,
			0.26f,0.26f,
			20.0f, -20.0f, false, RigEffect_Clear
		}
	},

};

struct CheckTPSCameras
{
	CheckTPSCameras()
	{
		TL_STATIC_ASSERT( R3D_ARRAYSIZE(TPSHudCameras[0]) == PLAYER_NUM_STATES ) ;
	}
};

Playerstate_e ActiveCameraRigID = PLAYER_IDLE;
PlayerStateVars_s ActiveCameraRig = TPSHudCameras[0][ActiveCameraRigID];

int  g_usePlayerEditorCamera = false;

void ProcessCameraRigEditor ()
{
	float SliderX = r3dRenderer->ScreenW-375;
	float SliderY = 85;

	static int RigID = PLAYER_IDLE;

	if (imgui_Button(SliderX, SliderY,80,35, "RIG ID", 0, false)) RigID ++;
	if (RigID > PLAYER_DIE) RigID = PLAYER_IDLE;

	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(EditorGameHUD::editorPlayerId);
	if(pl)
		pl->PlayerState = RigID;

	static const char* RigNames[PLAYER_NUM_STATES] = {
		"PLAYER_IDLE",
		"PLAYER_IDLEAIM",
		"PLAYER_MOVE_CROUCH",
		"PLAYER_MOVE_CROUCH_AIM",
		"PLAYER_MOVE_WALK_AIM",
		"PLAYER_MOVE_RUN",
		"PLAYER_MOVE_SPRINT",
		"PLAYER_DIE",
	};

	SliderY += imgui_Static(SliderX+90, SliderY, RigNames[RigID]);
	SliderY +=10;

	SliderY += imgui_Checkbox(SliderX, SliderY, "Use Player Camera", &g_usePlayerEditorCamera, 1);
	if(g_camera_mode->GetInt()!=2)
	{
		int camMode = g_camera_mode->GetInt();
		SliderY += imgui_Checkbox(SliderX, SliderY, "TPS: Off-center aim", &camMode, 1);
		g_camera_mode->SetInt(camMode);
	}

	SliderY += 10;
	SliderY += imgui_Static(SliderX+40, SliderY, "CAMERA");
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Height", &TPSHudCameras[g_camera_mode->GetInt()][RigID].Position.Y, -1.0f, 2.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Distance", &TPSHudCameras[g_camera_mode->GetInt()][RigID].Position.Z, -20.0f, 20.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Offset", &TPSHudCameras[g_camera_mode->GetInt()][RigID].Position.X, -3.0f, 3.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "FOV", &TPSHudCameras[g_camera_mode->GetInt()][RigID].FOV, 20.0f, 100.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Sensetivity", &TPSHudCameras[g_camera_mode->GetInt()][RigID].MouseSensetivity, 0.01f, 2.0f, "%.2f", 1 );
	SliderY +=5;

	/*SliderY += imgui_Static(SliderX+40, SliderY, "CAMERA TARGET");
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Height", &TPSHudCameraTarget.Y, -1.0f, 2.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Offset", &TPSHudCameraTarget.X, -3.0f, 3.0f, "%.2f", 1 );
	SliderY +=5;*/
	SliderY += imgui_Static(SliderX+40, SliderY, "CAMERA POINT TO");
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Height", &TPSCameraPointToAdj[g_camera_mode->GetInt()], -2.0f, 2.0f, "%.2f", 1 );
	SliderY += imgui_Value_Slider( SliderX, SliderY, "Height Crouch", &TPSCameraPointToAdjCrouch[g_camera_mode->GetInt()], -2.0f, 2.0f, "%.2f", 1 );

}



Playerstate_e CurrentState = PLAYER_IDLE;
PlayerStateVars_s CurrentRig = TPSHudCameras[0][0];
PlayerStateVars_s SourceRig = TPSHudCameras[0][0];
PlayerStateVars_s TargetRig = TPSHudCameras[0][0];
float LerpValue = 0;

void ProcessPlayerMovement(obj_AI_Player* pl, bool editor_debug );

// runs only in editor!
bool CheckCameraCollision(r3dPoint3D& camPos, const r3dPoint3D& target, bool checkCamera);
void Get_Camera_Bob(r3dPoint3D& camBob, r3dPoint3D& camUp, const obj_AI_Player* player);
void EditorGameHUD :: SetCameraPure ( r3dCamera &Cam)
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId);
	if (!pl) return;

	if(g_usePlayerEditorCamera)
	{
		TargetRig  = TPSHudCameras[g_camera_mode->GetInt()][ActiveCameraRigID];
		pl->m_siegeArmingTimer = 1.0f; // hack to prevent player from moving
		ProcessPlayerMovement(pl, true);
		pl->m_siegeArmingTimer = 0.0f;
	}

#ifndef FINAL_BUILD
	if(d_spectator_fov->GetFloat() > 0.0f)
		GameFOV = d_spectator_fov->GetFloat();
	else
#endif
		GameFOV = CurrentRig.FOV;

	// uav camera
	extern bool SetCameraPlayerUAV(const obj_AI_Player* pl, r3dCamera &Cam);
	if(SetCameraPlayerUAV(pl, Cam))
	{
		FPS_Position = Cam;
		return;
	}

	// vehicle camera
	extern bool SetCameraPlayerVehicle(const obj_AI_Player* pl, r3dCamera &Cam);
	if(SetCameraPlayerVehicle(pl, Cam))
	{
		FPS_Position = Cam;
		return;
	}

	r3dPoint3D CamPos = pl->GetPosition();
	
	r3dPoint3D offset = GetCamOffset();
	CamPos += offset ;

	float CharacterHeight = pl->getPlayerHeightForCamera();
	
	r3dPoint3D playerPos = pl->GetPosition();	
	r3dPoint3D PointTo = playerPos;
	PointTo.Y += (CharacterHeight + TPSHudCameraTarget.Y);
	PointTo += pl->GetvRight() * TPSHudCameraTarget.X;

	// check for collision
	r3dPoint3D playerPosHead = playerPos; playerPosHead.y += CharacterHeight;
	{
		r3dPoint3D savedCamPos = CamPos;
		if(CheckCameraCollision(CamPos, playerPosHead, true) && (pl->PlayerState == PLAYER_MOVE_CROUCH || pl->PlayerState == PLAYER_MOVE_CROUCH_AIM)) 
		{
			CamPos = savedCamPos;
			playerPosHead = playerPos;
			playerPosHead.y += CharacterHeight-0.8f;
			CheckCameraCollision(CamPos, playerPosHead, true);
		}
	}
	PointTo += (pl->m_vVision+r3dPoint3D(0, (pl->bCrouch?TPSCameraPointToAdjCrouch[g_camera_mode->GetInt()]:TPSCameraPointToAdj[g_camera_mode->GetInt()]), 0.0f)) * 50;//cameraRayLen;//CurrentRig.Target.Z;

	r3dPoint3D getAdjustedPointTo(obj_AI_Player* pl, const r3dPoint3D& PointTo, const r3dPoint3D& CamPos);
	r3dPoint3D adjPointTo(0,0,0);
	adjPointTo = getAdjustedPointTo(pl, PointTo, CamPos);

	r3dPoint3D camBob, camUp;
	Get_Camera_Bob(camBob, camUp, pl);

	Cam.FOV = GameFOV;
	Cam.SetPosition( CamPos + camBob);
	Cam.PointTo(PointTo + camBob);

	Cam.vPointTo += adjPointTo;
	Cam.vUP = camUp;

#ifndef FINAL_BUILD
	if( g_pHUDCameraEffects )
	{
		g_pHUDCameraEffects->Update( &Cam, pl->GetPosition() ) ;
	}
#endif
	
	FPS_Position = Cam;
}  

void EditorGameHUD :: Draw()
{
#ifdef FINAL_BUILD
	return;
#else
	assert(bInited);

	r3dSetFiltering( R3D_POINT );

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA);

	//r3dDrawBox2D(r3dRenderer->ScreenW/2-8, r3dRenderer->ScreenH/2-8, 16, 16, r3dColor(255,155,0,100));

	if( r_show_player_debug_data->GetBool() )
	{
		Font_Label->PrintF(10, r3dRenderer->ScreenH-25,r3dColor(255,255,255), "FPS %3.1f[%02.2fms]", 1.0f/r3dGetFrameTime(), r3dGetFrameTime()*1000.0f );
	}

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

	void DEBUG_DrawZombieModHelpers();
	DEBUG_DrawZombieModHelpers();
#endif
}

float		getCameraLeftSide();
void		updateCameraLeftSide();

/*virtual*/ r3dPoint3D EditorGameHUD :: GetCamOffset() const
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId);
	if (!pl) return r3dPoint3D( 0, 0, 0 );

	r3dPoint3D offset;

	const PlayerStateVars_s& state = CurrentRig ;

	offset =  r3dPoint3D( 0, (pl->Height +  state.Position.Y), 0 );
	updateCameraLeftSide();
	offset += pl->GetvRight() * state.Position.X * getCameraLeftSide();
	offset += pl->m_vVision * (state.Position.Z);

	return offset;
}

/*virtual*/ void EditorGameHUD :: SetCamPos( const r3dPoint3D& pos )
{
	if( obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId) )
	{
		pl->SetPosition( pos - GetCamOffset() );
	}
}


//----------------------------------------------------------------
void EditorGameHUD :: Process()
//----------------------------------------------------------------
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId);

	if(!pl) 
		pl = AddPlayer( 1 );

	if( int s = d_spawn_players->GetInt() )
	{
		static float SinceLastSpawn = r3dGetTime() ;

		if( r3dGetTime() - SinceLastSpawn > d_spawn_player_delay->GetFloat() )
		{
			r3dPoint3D rpos ;

			r3dVector to = gCam.vPointTo ;

			to.Normalize() ;

			r3dVector binorm = to.Cross( r3dVector( 0,1,0) ) ;

			binorm.Normalize() ;
			
			r3dVector toAdd = to * ( float( rand() ) / RAND_MAX + 0.5f ) * 10 + binorm * ( float( rand() ) / RAND_MAX - 0.5f ) * 10 ;

			rpos.x = gCam.x + toAdd.x ;
			rpos.y = gCam.y ;
			rpos.z = gCam.z + toAdd.z ;

			AddPlayer( 0 )->SetPosition( rpos ) ;
			d_spawn_players->SetInt( s - 1 ) ;
			SinceLastSpawn = r3dGetTime() ;
		}
	}

	ProcessPlayerMovement(pl, false);
}

void EditorGameHUD::OnHudSelected()
{
	CurrentRig = TPSHudCameras[g_camera_mode->GetInt()][PLAYER_IDLE];
	SourceRig  = CurrentRig;
	TargetRig  = CurrentRig;

	g_usePlayerEditorCamera = false;

	Mouse->Hide();
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(editorPlayerId);
	if ( !pl )
	{
		pl = AddPlayer( 1 );
	}
	else
	{
		for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
		{
			if(pl->m_Weapons[i]) 
				pl->m_Weapons[i]->Reset();
		}
	}
	extern r3dPoint3D UI_TargetPos;
	pl->SetPosition( UI_TargetPos + r3dPoint3D( 0, 5, 0 ) );

	if( pl->bDead )
		pl->Undie();
}

void EditorGameHUD::OnHudUnselected()
{
	Mouse->Show();
}

static int SelectRandomGear( STORE_CATEGORIES cat )
{
	int catCount = 0 ;

	for( int i = 0, e = gWeaponArmory.getNumGears(); i < e; i ++ )
	{
		const GearConfig* cfg = gWeaponArmory.getGearConfigByIndex( i ) ;

		if( cfg->category == cat )
		{
			catCount ++ ;
		}
	}

	int idx = rand() % catCount ;

	catCount = 0 ;

	for( int i = 0, e = gWeaponArmory.getNumGears(); i < e; i ++ )
	{
		const GearConfig* cfg = gWeaponArmory.getGearConfigByIndex( i ) ;

		if( cfg->category == cat )
		{
			if( catCount == idx )
			{
				if( r3dMesh::CanLoad( cfg->m_ModelPath ) )
				{
					return cfg->m_itemID ;
				}
				else
				{
					r3dOutToLog( "Couldn't load random gear %s!\n", cfg->m_ModelPath ) ;
					return 0 ;
				}
			}
			catCount ++ ;
		}
	}

	r3dOutToLog( "Couldn't select random gear!\n" ) ;

	return 0 ;
}

static int SelectRandomWeapon( STORE_CATEGORIES cat )
{
	int catCount = 0 ;

	for( int i = 0, e = gWeaponArmory.getNumWeapons(); i < e; i ++ )
	{
		const WeaponConfig* cfg = gWeaponArmory.getWeaponConfigByIndex( i ) ;

		if( cfg->category == cat && cfg->IsFPS)
		{
			catCount ++ ;
		}

	}

	int idx = rand() % catCount ;

	catCount = 0 ;

	for( int i = 0, e = gWeaponArmory.getNumWeapons(); i < e; i ++ )
	{
		const WeaponConfig* cfg = gWeaponArmory.getWeaponConfigByIndex( i ) ;

		if( cfg->category == cat && cfg->IsFPS)
		{
			if( catCount == idx )
			{
				if( r3dMesh::CanLoad( cfg->m_ModelPath) )
				{
					return cfg->m_itemID ;
				}
				else
				{
					r3dOutToLog( "Couldn't load random weapon %s!\n", cfg->m_ModelPath ) ;
					return 0 ;
				}
			}
			catCount ++ ;
		}
	}

	r3dOutToLog( "Couldn't select random weapon!\n" ) ;

	return 0 ;
}

void CreateDummyEditorPlayer()
{
	obj_AI_Player* pl = (obj_AI_Player *)srv_CreateGameObject("obj_AI_Player", "Player", UI_TargetPos + r3dPoint3D(0, 0.01f, 0));
	pl->TeamID       = TEAM_RED;
	pl->NetworkLocal = false; // so it will be always in 3rd person mode. with disabled physics

	extern int SelectRandomGear( STORE_CATEGORIES cat );
	extern int SelectRandomWeapon( STORE_CATEGORIES cat );

	wiWeaponAttachments Attm;
	pl->CurLoadout.BodyMeshID		= SelectRandomGear( storecat_Characters ) ;
	pl->CurLoadout.BodyHeadID		= SelectRandomGear( storecat_Heads ) ;
	pl->CurLoadout.BodyHeadGearID		= SelectRandomGear( storecat_HeadGear ) ;
	pl->CurLoadout.BodyArmorID		= SelectRandomGear( storecat_Gear ) ;
	pl->CurLoadout.BodyVoiceID		= 0 ;

	pl->CurLoadout.PrimaryWeaponID		= SelectRandomWeapon( storecat_ASR ) ;
	pl->CurLoadout.SecondaryWeaponID	= SelectRandomWeapon( storecat_SNP ) ;
	pl->CurLoadout.SidearmWeaponID		= SelectRandomWeapon( storecat_HG ) ;

	pl->CurLoadout.Item1			= SelectRandomWeapon( storecat_GRENADES ) ;
	pl->CurLoadout.Item2			= SelectRandomWeapon( storecat_GRENADES ) ;
	pl->CurLoadout.Item3			= 0 ;	
	pl->CurLoadout.Item4			= 0 ;

	pl->m_fPlayerRotationTarget = pl->m_fPlayerRotation = u_GetRandom(0.0f, 360.0f);
	pl->OnCreate() ;
	pl->UpdateLoadoutSlot(pl->CurLoadout, Attm);
}

obj_AI_Player* EditorGameHUD::AddPlayer( int bControllable )
{
	obj_AI_Player* pl;

	extern r3dPoint3D UI_TargetPos;

	pl = (obj_AI_Player *)srv_CreateGameObject("obj_AI_Player", "Player", UI_TargetPos + r3dPoint3D(0,5,0));
	
	if( bControllable )
	{
		pl->TeamID = TEAM_BLUE;

		pl->NetworkLocal = true;
		pl->NetworkID    = 1;
		editorPlayerId = pl->ID;
	}
	else
	{
		pl->TeamID = TEAM_RED;
	}

	wiWeaponAttachments Attm;
	if(g_camera_mode->GetInt() == 2)
	{
		pl->CurLoadout.BodyMeshID			= 20011; // slickman
		pl->CurLoadout.BodyHeadID			= SelectRandomGear( storecat_Heads ) ;
		pl->CurLoadout.BodyHeadGearID		= SelectRandomGear( storecat_HeadGear ) ;
		pl->CurLoadout.BodyArmorID			= SelectRandomGear( storecat_Gear ) ;

		pl->CurLoadout.PrimaryWeaponID		= 101193; //ASR_Fn_scar
		pl->CurLoadout.SecondaryWeaponID	= 101068;
		pl->CurLoadout.SidearmWeaponID		= 101115;
	}

	pl->CurLoadout.Item1 = 101140; //EXP_VS50
	pl->CurLoadout.Item2 = 101147; //Exp_SmokeG_Red
	pl->CurLoadout.Item3 = WeaponConfig::ITEMID_Cypher2;
	pl->CurLoadout.Item4 = WeaponConfig::ITEMID_LLDR;

	if( d_random_editor_players->GetInt() )
	{
		pl->CurLoadout.BodyMeshID			= SelectRandomGear( storecat_Characters ) ;
		pl->CurLoadout.BodyHeadID			= SelectRandomGear( storecat_Heads ) ;
		pl->CurLoadout.BodyHeadGearID		= SelectRandomGear( storecat_HeadGear ) ;
		pl->CurLoadout.BodyArmorID			= SelectRandomGear( storecat_Gear ) ;
		pl->CurLoadout.BodyVoiceID			= 0 ;

		pl->CurLoadout.PrimaryWeaponID		= SelectRandomWeapon( storecat_ASR ) ;
		pl->CurLoadout.SecondaryWeaponID	= SelectRandomWeapon( storecat_SNP ) ;
		pl->CurLoadout.SidearmWeaponID		= SelectRandomWeapon( storecat_HG ) ;

		pl->CurLoadout.Item1				= SelectRandomWeapon( storecat_GRENADES ) ;
		pl->CurLoadout.Item2				= SelectRandomWeapon( storecat_GRENADES ) ;
		pl->CurLoadout.Item3				= 0 ;	
		pl->CurLoadout.Item4				= 0 ;
	}

	pl->OnCreate() ;
	pl->UpdateLoadoutSlot(pl->CurLoadout, Attm);

	return pl;
}


