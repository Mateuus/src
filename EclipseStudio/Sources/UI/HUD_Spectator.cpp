#include "r3dPCH.h"
#include "r3d.h"
#include "d3dfont.h"
#include "d3dfont.h"

#include "../SF/Console/Config.h"
#include "UI\Hud_spectator.h"
#include "Editors\LevelEditor.h"

#include "GameCommon.h"
#include "GameLevel.h"

// selected objectID
gobjid_t	UI_SelectedObjID	= invalidGameObjectID;
// on-cursor selection
r3dPoint3D	UI_TargetPos;		// snapped to object position (object center)
r3dPoint3D	UI_TargetNorm;
r3dPoint3D	UI_TargetPos2;		// unsnapped position
r3dPoint3D	UI_TerraTargetPos;
gobjid_t	UI_TargetObjID		= invalidGameObjectID;
r3dMaterial *UI_TargetMaterial = NULL;

float GameFOV = VIEW_FOV;

static int enablemouselook = 1;

#include "Editors\LevelEditor.h"


BaseHUD  		*Hud = NULL;




void CameraHUD :: InitPure()
{
	CameraMode = hud_TopdownCamera;

	UI_SelectedObjID = invalidGameObjectID;
	UI_TargetObjID   = invalidGameObjectID;
	UI_TargetPos     = r3dPoint3D(0, 0, 0);
	FPS_Position.Assign(0,0,0);

#ifndef FINAL_BUILD
	ReadXMLEditorSettingsStartPosition( &FPS_Position );
#endif

	if ( Terrain && Terrain->IsLoaded() )
	{
		float h = terra_GetH(FPS_Position);
		if(FPS_Position.Y < h) FPS_Position.Y = h;

		// otherwise terrain update fails
		if( FPS_Position.X < 0 )
			FPS_Position.X = 0 ;

		if( FPS_Position.Z < 0 )
			FPS_Position.Z = 0 ;

		const r3dTerrainDesc& desc = Terrain->GetDesc() ;

		float totalW = desc.XSize ;
		float totalH = desc.ZSize ;

		if( FPS_Position.X > totalW )
			FPS_Position.X = totalW ;

		if( FPS_Position.Z > totalH )
			FPS_Position.Z = totalH ;
	}

	FPS_Acceleration.Assign(0,0,0);
	FPS_vViewOrig.Assign(0,0,0);
	FPS_ViewAngle.Assign(0,0,0);
	FPS_vVision.Assign(0, 0, 1);
}



void CameraHUD :: DestroyPure()
{
	//	floatTextMgrDestroy();
}

void CameraHUD :: SetCameraDir (r3dPoint3D vPos )
{
	FPS_vVision = vPos;
	FPS_vVision.Normalize();

	r3dVector v1 = FPS_vVision;
	v1.y = 0;
	v1.Normalize();

	FPS_vViewOrig.x = - R3D_RAD2DEG(atan2 ( v1.x, v1.z ));
	FPS_vViewOrig.y = R3D_RAD2DEG(asin ( FPS_vVision.y ));
	FPS_vViewOrig.z = 0;
	FPS_ViewAngle = FPS_vViewOrig;
}

r3dPoint3D CameraHUD :: GetCameraDir () const
{
	return FPS_vVision;
}


void CameraHUD :: SetCameraPure ( r3dCamera &Cam)
{
	switch (CameraMode)
	{
	case hud_FPSCamera:
	case hud_TPSCamera:
	case hud_TopdownCamera:
	default:
		SetFPSCamera(Cam);
		break;

	}

}  

extern int  g_usePlayerEditorCamera;
BaseHUD* editor_GetHudByIndex(int index);

void CameraHUD :: SetFPSCamera(r3dCamera &Cam )
{
	if(g_usePlayerEditorCamera)
		return editor_GetHudByIndex(1)->SetCamera(Cam);

	r3dPoint3D CamPos = FPS_Position;
	CamPos += GetCamOffset();

	r3dPoint3D ViewPos = CamPos + FPS_vVision*10.0f;

#ifndef FINAL_BUILD
	if(d_spectator_fov->GetFloat() > 0.0f)
		GameFOV = d_spectator_fov->GetFloat();
	else
#endif
		GameFOV = VIEW_FOV;

	Cam.FOV = GameFOV;
	Cam.SetPosition( CamPos );
	Cam.PointTo(ViewPos);
}


void CameraHUD :: Draw()
{
	assert(bInited);

	if ( !bInited ) return;

	r3dSetFiltering( R3D_POINT );

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA);

	//	floatTextMgrDraw();

	//Font_Label->PrintF(20,20,r3dColor(255,255,255), "fps %f\n%fms\n%d", 1.0f/r3dGetFrameTime(), r3dGetFrameTime()*1000.0f,Mod);

#ifndef FINAL_BUILD
	GameObject* TargetObj = GameWorld().GetObject(UI_TargetObjID);
	if(TargetObj && r_hide_icons->GetInt() == 0)
	{
		Font_Label->PrintF(20,60,r3dColor(255,255,255), "OBJ: %s", TargetObj->Name.c_str());
	}
#endif


#ifndef FINAL_BUILD
	LevelEditor.Process(!enablemouselook);
#endif

	// display exposure/ luminance tab
	if (d_show_scene_luma->GetInt() > 0)
	{
		float luma = 0.5f, exp=0.0f ;
		IDirect3DTexture9* tex = NULL ;

		struct CreateDelTex
		{
			CreateDelTex()
			{
				D3D_V( r3dRenderer->pd3ddev->CreateTexture( 1, 1, 0, 0, D3DFMT_R32F, D3DPOOL_SYSTEMMEM, &tex, NULL ) ) ;
				D3D_V( tex->GetSurfaceLevel( 0, &surf ) ) ;
			}

			~CreateDelTex()
			{
				surf->Release() ;
				tex->Release() ;
			}

			IDirect3DSurface9* surf ;
			IDirect3DTexture9* tex ;

		} cdt ;

		char exp_val[ 512 ] ;

		D3DLOCKED_RECT lrect ;

		// can be device lost
		if( r3dRenderer->pd3ddev->GetRenderTargetData( SceneExposure1->GetTex2DSurface(), cdt.surf ) == D3D_OK ) 
		{
			D3D_V( cdt.tex->LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) ) ;
			exp=(*(float*)lrect.pBits);		
			D3D_V( cdt.tex->UnlockRect( 0 ) ) ;
		}


		if( r3dRenderer->pd3ddev->GetRenderTargetData( AvgSceneLuminanceBuffer->GetTex2DSurface(), cdt.surf ) == D3D_OK ) 
		{
			D3D_V( cdt.tex->LockRect( 0, &lrect, NULL, D3DLOCK_READONLY ) ) ;
			luma=(*(float*)lrect.pBits);		
			D3D_V( cdt.tex->UnlockRect( 0 ) ) ;
		}

		sprintf( exp_val, "Scene Exposure: %7.4f, Luma: %7.4f", exp, luma ) ;

		imgui_Static( 5, r3dRenderer->ScreenH-235, exp_val ) ;
	}

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );
}

/*virtual*/
r3dPoint3D
CameraHUD :: GetCamOffset() const /*OVERRIDE*/
{
	return r3dPoint3D( 0.f, 1.8f, 0.f );
}


void CameraHUD :: ProcessCheats()
{
}



//----------------------------------------------------------------
void CameraHUD :: Process()
//----------------------------------------------------------------
{
	ProcessCheats();

	// alt-8 spawn test players for sergey
	if(Keyboard->WasPressed(kbs8) && (Keyboard->IsPressed(kbsLeftAlt) || Keyboard->IsPressed(kbsRightAlt)))
	{
		extern void CreateDummyEditorPlayer();
		CreateDummyEditorPlayer();
	}

	switch (CameraMode)
	{
	case hud_FPSCamera:
	case hud_TPSCamera:
	case hud_TopdownCamera:
	default:
		ProcessFPSCamera();
		break;

	}

	if (!enablemouselook)
		ProcessPick();

	return;
}



void CameraHUD :: ProcessFPSCamera()
{
#ifndef FINAL_BUILD
	FPS_Acceleration.Assign(0, 0, 0);

	float	glb_MouseSens    = 0.5f;	// in range (0.1 - 1.0)
	float  glb_MouseSensAdj = 1.0f;	// in range (0.1 - 1.0)


	enablemouselook = (Mouse->IsPressed(r3dMouse::mRightButton));


	if (!enablemouselook) return;


	//
	//  Mouse controls are here
	//
	int mMX=Mouse->m_MouseMoveX, mMY=Mouse->m_MouseMoveY;

	FPS_vViewOrig.x += float(-mMX) * glb_MouseSensAdj;
	FPS_vViewOrig.y += float(-mMY) * glb_MouseSensAdj;

	// Player can't look too high!
	if(FPS_vViewOrig.y > 85)  FPS_vViewOrig.y = 85;
	if(FPS_vViewOrig.y < -85) FPS_vViewOrig.y = -85;

	FPS_ViewAngle = FPS_vViewOrig; // + FPS_vViewDistortion;

	if(FPS_ViewAngle.y > 360 ) FPS_ViewAngle.y = FPS_ViewAngle.y - 360;
	if(FPS_ViewAngle.y < 0 )   FPS_ViewAngle.y = FPS_ViewAngle.y + 360;


	D3DXMATRIX mr;

	D3DXMatrixIdentity(&mr);
	D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-FPS_ViewAngle.x), R3D_DEG2RAD(-FPS_ViewAngle.y), 0);

	FPS_vVision  = r3dVector(mr._31, mr._32, mr._33);

	D3DXMatrixIdentity(&mr);
	D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-FPS_ViewAngle.x), 0, 0);
	FPS_vRight = r3dVector(mr._11, mr._12, mr._13);
	FPS_vUp    = r3dVector(0, 1, 0);
	FPS_vForw  = r3dVector(mr._31, mr._32, mr._33);

	FPS_vForw.Normalize();
	FPS_vRight.Normalize();
	FPS_vVision.Normalize();

	// walk
	extern float __EditorWalkSpeed;
	float fSpeed = __EditorWalkSpeed;

	float mult = 1;
	if(Keyboard->IsPressed(kbsLeftShift)) mult = 30.0f; //64.0f;

	if(Keyboard->IsPressed(kbsW)) FPS_Acceleration.Z = fSpeed;
	if(Keyboard->IsPressed(kbsS)) FPS_Acceleration.Z = -fSpeed * 0.7f;
	if(Keyboard->IsPressed(kbsA)) FPS_Acceleration.X = -fSpeed * 0.7f;
	if(Keyboard->IsPressed(kbsD)) FPS_Acceleration.X = fSpeed * 0.7f;
	if(Keyboard->IsPressed(kbsQ)) FPS_Position.Y    += SRV_WORLD_SCALE(1.0f)* r3dGetFrameTime() * mult;
	if(Keyboard->IsPressed(kbsE)) FPS_Position.Y    -= SRV_WORLD_SCALE(1.0f)* r3dGetFrameTime() * mult;

	FPS_Position += FPS_vVision * FPS_Acceleration.Z * r3dGetFrameTime() * mult;
	FPS_Position += FPS_vRight * FPS_Acceleration.X * r3dGetFrameTime() *mult;

	//if (FPS_Position.Y < terra_GetH(FPS_Position)) FPS_Position.Y = terra_GetH(FPS_Position);

	if(Keyboard->WasPressed(kbsF6))
	{
		r3dPoint3D PP = FPS_Position + FPS_vVision*60.0f;

		r3dOutToLog("POINT at %f %f %f\n", PP.X, PP.Y, PP.Z);

		GameObject *Car = srv_CreateGameObject("obj_Phys", "Data\\Models\\Objects\\Box01.sco", PP);
	}

#endif
}

bool hud_ProcessCameraPick(float mx, float my)
{
	R3DPROFILE_FUNCTION( "hud_ProcessCameraPick" ) ;

	float vx, vy, vw, vh;
	r3dRenderer->GetBackBufferViewport( &vx, &vy, &vw, &vh );

	mx = mx - vx;
	my = my - vy;

	r3dPoint3D dir;
	r3dScreenTo3D(mx, my, &dir);

	UI_TargetPos      = r3dPoint3D(0, 99999, 0); 
	UI_TargetObjID    = invalidGameObjectID;
	UI_TargetMaterial = NULL;

	CollisionInfo CL;
	const GameObject* target = GameWorld().CastRay(gCam, dir, 25000.0f, &CL); //bp, do bbox check so that clicks are better

	bool found = false;

	if(target) {
		UI_TargetObjID = target->ID;
		UI_TargetPos   = CL.NewPosition; //target->Position;
		UI_TargetPos2  = CL.NewPosition;
		UI_TargetNorm  = CL.Normal;

		if(CL.Material)
			UI_TargetMaterial  = CL.Material;

		found = true;
	}

	extern r3dMaterial* Get_Material_By_Ray(const r3dPoint3D& vStart, const r3dPoint3D& vRay, float& dist);

	float dist = 9999999;
	r3dMaterial* collectionsMaterial = Get_Material_By_Ray(gCam, dir, dist);

	if((!CL.Material || (CL.Material && dist < CL.Distance) ) && collectionsMaterial)
	{
		UI_TargetMaterial  = collectionsMaterial;
		found = true;
	}

	if ( Terrain && Terrain->IsLoaded() ) {
		extern BOOL terra_FindIntersection(r3dPoint3D &vFrom, r3dPoint3D &vTo, r3dPoint3D &colpos, int iterations);
		r3dPoint3D v3;

		if(terra_FindIntersection(gCam, gCam + dir*20000, v3, 20000)) 
		{
			UI_TerraTargetPos = v3;

			r3dVector oldDistance = UI_TargetPos - gCam;
			r3dVector newDistance = v3 - gCam;
			// if the new distance is closer than the hit, and the new distance is in the same direction as the hit. 
			if( !target || ( oldDistance.LengthSq() > newDistance.LengthSq() && newDistance.Dot(dir) > 0 ) )
			{
				UI_TargetObjID = invalidGameObjectID;
				UI_TargetPos   = v3;
				UI_TargetNorm  = Terrain->GetNormal( v3 ) ;
				UI_TargetPos2  = v3;
				UI_TargetMaterial = NULL;
				found = true;
			}
		}
	}

	return found;
}

void CameraHUD :: ProcessPick()
{
	int mx, my;
	Mouse->GetXY(mx,my);
	if(!hud_ProcessCameraPick((float)mx, (float)my)) {
		UI_TargetPos = FPS_Position;
	}

	// cheat: teleport player
	if((Keyboard->IsPressed(kbsLeftAlt) && Keyboard->WasPressed(kbsT)))
	{
		//r->AI_actor.SetPosition(UI_TargetPos);
		FPS_Position = UI_TargetPos;
	}
}

