#include "r3dPCH.h"

#include "r3d.h"
#include "d3dfont.h"

#include "GameCommon.h"
#include "HUD_ShootingGallery.h"
#include "HUDDisplay.h"

#include "rendering/Deffered/CommonPostFX.h"

#include "ObjectsCode\AI\AI_Player.h"
#include "ObjectsCode\AI\AI_PlayerAnim.h"
#include "ObjectsCode\weapons\Weapon.h"
#include "ObjectsCode\weapons\WeaponArmory.h"

extern float GameFOV;

gobjid_t	ShootingGalleryHUD::targetPlayerId = invalidGameObjectID;

void ShootingGalleryHUD :: InitPure()
{
}

void ShootingGalleryHUD :: DestroyPure()
{
}

void ShootingGalleryHUD :: SetCameraDir (r3dPoint3D vPos )
{

}

r3dPoint3D ShootingGalleryHUD :: GetCameraDir () const
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId);
	if ( pl )
		return pl->m_vVision;
	else
		return r3dVector(1,0,0);
}


	extern int g_RenderRadialBlur;

extern void SetFocusPoint(r3dPoint3D &P);

extern void RigEffect_Clear (float Lerp);



extern void RigEffect_SprintBlur (float Lerp);

//////////////////////////////////////////////////////////////////////////

extern r3dPoint3D TPSHudCameraTarget; // pt: target should be the same for all states, otherwise your aim will move and make it very difficult to shoot someone

extern PlayerStateVars_s TPSHudCameras[3][PLAYER_NUM_STATES];

extern  float	TPSCameraPointToAdj[3];
extern  float   TPSCameraPointToAdjCrouch[3];

extern Playerstate_e ActiveCameraRigID;
extern PlayerStateVars_s ActiveCameraRig;

extern int  g_usePlayerEditorCamera;


extern Playerstate_e CurrentState;
extern PlayerStateVars_s CurrentRig ;
extern PlayerStateVars_s SourceRig;
extern PlayerStateVars_s TargetRig;
extern float LerpValue;

void ProcessPlayerMovement(obj_AI_Player* pl, bool editor_debug );

// runs only in editor!
bool CheckCameraCollision(r3dPoint3D& camPos, const r3dPoint3D& target, bool checkCamera);
void Get_Camera_Bob(r3dPoint3D& camBob, r3dPoint3D& camUp, const obj_AI_Player* player);
void ShootingGalleryHUD :: SetCameraPure ( r3dCamera &Cam)
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId);
	if (!pl) return;

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
	
	FPS_Position = Cam;
}  

void ShootingGalleryHUD :: Draw()
{
	extern HUDDisplay* hudMain;
	hudMain->Update();
	hudMain->Draw();

#ifndef FINAL_BUILD
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
#endif
}

float		getCameraLeftSide();
void		updateCameraLeftSide();

/*virtual*/ r3dPoint3D ShootingGalleryHUD :: GetCamOffset() const
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId);
	if (!pl) return r3dPoint3D( 0, 0, 0 );

	r3dPoint3D offset;

	const PlayerStateVars_s& state = CurrentRig ;

	offset =  r3dPoint3D( 0, (pl->Height +  state.Position.Y), 0 );
	updateCameraLeftSide();
	offset += pl->GetvRight() * state.Position.X * getCameraLeftSide();
	offset += pl->m_vVision * (state.Position.Z);

	return offset;
}

/*virtual*/ void ShootingGalleryHUD :: SetCamPos( const r3dPoint3D& pos )
{
	if( obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId) )
	{
		pl->SetPosition( pos - GetCamOffset() );
	}
}


//----------------------------------------------------------------
void ShootingGalleryHUD :: Process()
//----------------------------------------------------------------
{
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId);

	// fail
	if(!pl)
	{
		return;
	}

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

			d_spawn_players->SetInt( s - 1 ) ;
			SinceLastSpawn = r3dGetTime() ;
		}
	}

	ProcessPlayerMovement(pl, false);
}

void ShootingGalleryHUD::OnHudSelected()
{
	CurrentRig = TPSHudCameras[g_camera_mode->GetInt()][PLAYER_IDLE];
	SourceRig  = CurrentRig;
	TargetRig  = CurrentRig;

	g_usePlayerEditorCamera = false;

	Mouse->Hide();
	obj_AI_Player* pl = (obj_AI_Player *)GameWorld().GetObject(targetPlayerId);

	if ( pl )
	{
		pl->SetPosition( r3dPoint3D( 212,100,210 ) );

		if( pl->bDead )
			pl->Undie();
	}
	
}

void ShootingGalleryHUD::OnHudUnselected()
{
	Mouse->Show();
}


void ShootingGalleryHUD::TargetPlayer( obj_AI_Player* targetPlayer )
{
	if ( targetPlayer != NULL ) 
	{
		targetPlayerId = targetPlayer->ID;
	}
}


