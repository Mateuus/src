#include "r3dPCH.h"
#include "r3d.h"

#include "r3dBackgroundTaskDispatcher.h"

#include "d3dfont.h"

#include "GameCommon.h"
#include "Gameplay_Params.h"

#include "UI\HUD_TPSGame.h"
#include "ObjectsCode/AI/AI_Player.h"
#include "ObjectsCode/AI/AI_PlayerAnim.h"
#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/weapons/WeaponArmory.h"
#include "ObjectsCode/Gameplay/obj_UAV.h"

#include "APIScaleformGfx.h"

#include "multiplayer/ClientGameLogic.h"

#include "HUDCameraEffects.h"

#include "UI\HUDDisplay.h"
#include "UI\HUDRespawn.h"
#include "UI\HUDMinimap.h"
#include "UI\HUDScoreboard.h"
#include "UI\m_EndRound.h"
#include "UI\HUDPause.h"
#include "UI\HUDCameraDrone.h"
#include "UI\HUDCommCalls.h"
#include "UI\HUDLaserDesignator.h"

#include "..\GameEngine\gameobjects\obj_Vehicle.h"

extern float GameFOV;

HUDCommCalls* hudCommCalls = NULL;
HUDLaserDesignator* hudLaserDesignator = NULL;
HUDMinimap* hudMinimap = NULL;
HUDRespawn*	hudRespawn = NULL;
HUDDisplay*	hudMain = NULL;
HUDScoreboard*	hudScore = NULL;
HUDEndRound*	hudEndRound = NULL;
HUDPause*	hudPause = NULL;
HUDCameraDrone* hudDrone = NULL;

#define VEHICLE_CINEMATIC_MODE 0

void TPSGameHud_UnlockAchievement( int achievementID )
{
	if( gClientLogic().gameFinished_ == false )
	{
		hudMain->RequeustAchievement(achievementID);
	} else {
		hudEndRound->ShowAchievementRibbon(achievementID);
	}
}

void TPSGameHUD_AddHitEffect(GameObject* from)
{
	obj_AI_Player* pl = gClientLogic().localPlayer_;
	if(!pl) return;
	if(pl->bDead) return;

	hudMain->AddHitEffect(from);
	pl->BloodEffect = 3.0f;
}



TPSGameHUD::TPSGameHUD()
{
	FPS_Acceleration.Assign(0,0,0);
	FPS_vViewOrig.Assign(0,0,0);
	FPS_ViewAngle.Assign(0,0,0);
	FPS_vVision.Assign(0,0,0);
	FPS_vRight.Assign(0,0,0);
	FPS_vUp.Assign(0,0,0);
	FPS_vForw.Assign(0,0,0);
	cameraRayLen = 20000.0f;
}

TPSGameHUD::~TPSGameHUD()
{
}

static bool TPSGameHud_Inited;
void TPSGameHUD_OnStartGame()
{
	const GBGameInfo& ginfo = gClientLogic().m_gameInfo;

	hudCommCalls = new HUDCommCalls();
	hudLaserDesignator = new HUDLaserDesignator();
	hudMinimap = new HUDMinimap();
	hudRespawn = new HUDRespawn();
	hudMain = new HUDDisplay();
	hudScore = new HUDScoreboard();
	hudEndRound = new HUDEndRound();
	hudPause = new HUDPause();
	hudDrone = new HUDCameraDrone();

	hudMain->Init(ginfo.startTickets);
	hudRespawn->Init();
	hudScore->Init();
	hudCommCalls->Init();
	hudLaserDesignator->Init();
	hudMinimap->Init();
	hudEndRound->Init();
	hudPause->Init();
	hudDrone->Init();

	Mouse->Hide(true);
	// lock mouse to a window when playing a game
	d_mouse_window_lock->SetBool(true);
	Mouse->SetRange(r3dRenderer->HLibWin);


	extern int g_CCBlackWhite;
	extern float g_fCCBlackWhitePwr;
	g_CCBlackWhite = false;
	g_fCCBlackWhitePwr = 0.0f;

	TPSGameHud_Inited = true;
}

void TPSGameHUD :: DestroyPure()
{
	if(TPSGameHud_Inited)
	{
		TPSGameHud_Inited = false;

		hudRespawn->Unload();
		hudEndRound->Unload();
		hudPause->Unload();
		hudScore->Unload();
		hudCommCalls->Unload();
		hudLaserDesignator->Unload();
		hudDrone->Unload();
		hudMinimap->Unload();
		hudMain->Unload();

		SAFE_DELETE(hudCommCalls);
		SAFE_DELETE(hudLaserDesignator);
		SAFE_DELETE(hudMinimap);
		SAFE_DELETE(hudRespawn);
		SAFE_DELETE(hudMain);
		SAFE_DELETE(hudScore);
		SAFE_DELETE(hudEndRound);
		SAFE_DELETE(hudPause);
		SAFE_DELETE(hudDrone);
	}
}

void TPSGameHUD :: SetCameraDir (r3dPoint3D vPos )
{

}

r3dPoint3D TPSGameHUD :: GetCameraDir () const
{
	return r3dVector(1,0,0);
}


extern	PlayerStateVars_s TPSHudCameras[3][PLAYER_NUM_STATES];
extern	Playerstate_e ActiveCameraRigID;
extern	PlayerStateVars_s ActiveCameraRig;

extern 	Playerstate_e CurrentState;
extern 	PlayerStateVars_s CurrentRig;
extern 	PlayerStateVars_s SourceRig;
extern 	PlayerStateVars_s TargetRig;
extern 	float LerpValue;
extern	r3dPoint3D TPSHudCameraTarget;

extern  float	TPSCameraPointToAdj[3];
extern  float   TPSCameraPointToAdjCrouch[3];

void TPSGameHUD :: InitPure()
{
	// reinit hud rigs based on camera mode
	CurrentRig = TPSHudCameras[g_camera_mode->GetInt()][PLAYER_IDLE];
	SourceRig  = CurrentRig;
	TargetRig  = CurrentRig;
}


// camPos = current camera pos. target = player's head pos
bool CheckCameraCollision(r3dPoint3D& camPos, const r3dPoint3D& target, bool checkCamera)
{
	R3DPROFILE_FUNCTION("CheckCameraCollision");

	r3dPoint3D origCamPos = camPos;
    int LoopBreaker = 0;

	r3dPoint3D motion = (camPos - target);
	float motionLen = motion.Length();
	int MaxLoopBreaker = 10;
	if(motionLen > 0.1f)
	{
		motion.Normalize();
		MaxLoopBreaker = int(ceilf(motionLen/0.05f));

		PxSphereGeometry camSphere(0.3f);
		PxTransform camPose(PxVec3(target.x, target.y, target.z), PxQuat(0,0,0,1));

		PxSweepHit sweepResults[32];
		bool blockingHit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
		while(int numRes=g_pPhysicsWorld->PhysXScene->sweepMultiple(camSphere, camPose, PxVec3(motion.x, motion.y, motion.z), motionLen, PxSceneQueryFlag::eINITIAL_OVERLAP|PxSceneQueryFlag::eNORMAL, sweepResults, 32, blockingHit, filter) && LoopBreaker<MaxLoopBreaker)
		{
			if(numRes == -1)
			{
				r3d_assert(false);
				break;
			}
			/* PxVec3 collNormal = PxVec3(0,0,0);
			for(int i=0; i<numRes; ++i)
			{
			collNormal += sweepResults[i].normal;
			}

			collNormal.normalize();*/

			//r3dPoint3D tmp(collNormal.x, collNormal.y, collNormal.z);
			r3dPoint3D tmp = -motion;
			tmp.Normalize();
			camPos += tmp * 0.05f;

			LoopBreaker++;

			motion = (camPos - target);
			motionLen = motion.Length();
			if(motionLen < 0.005f)
			{
				LoopBreaker = MaxLoopBreaker;
				break;
			}
			motion.Normalize();
		}

		/*
		// OLD CODE!
		NxRay ray; 
		NxRaycastHit hit;
		ray.orig = target.asNxVec3();
		ray.dir = (camPos - target).asNxVec3();
		float rayLen = ray.dir.magnitude();
		ray.dir.normalize();
		r3dPoint3D rayDir(-ray.dir);
		if(rayLen > 0)
		{
		NxShape* shape = g_pPhysicsWorld->PhysXScene->raycastClosestShape(ray, NX_STATIC_SHAPES, hit, COLLIDABLE_STATIC_MASK, rayLen,NX_RAYCAST_IMPACT);
		if(shape)
		camPos = r3dPoint3D(hit.worldImpact.x-ray.dir.x*0.1f, hit.worldImpact.y-ray.dir.y*0.1f, hit.worldImpact.z-ray.dir.z*0.1f);
		}

		NxSphere camSphere;
		NxVec3 origCamPos = camPos.asNxVec3();
		camSphere.center = camPos.asNxVec3();
		camSphere.radius = 0.6f;
		int LoopBreaker = 0;
		float distance = 0;
		// simple test to make sure that camera isn't too close to geometry, so that Near Plane will not clip it, otherwise player can see through walls
		while(g_pPhysicsWorld->PhysXScene->checkOverlapSphere(camSphere, NX_STATIC_SHAPES, COLLIDABLE_STATIC_MASK) && LoopBreaker < 50) // && distance < rayLen
		{
		// check for collision shapes
		NxShape* collShapes[32] = {0};
		int numColl = g_pPhysicsWorld->PhysXScene->overlapSphereShapes(camSphere, NX_STATIC_SHAPES, 32, &collShapes[0], NULL, COLLIDABLE_STATIC_MASK, NULL, true);
		NxVec3 collNormal = NxVec3(0,0,0);
		for(int i=0; i<numColl; ++i)
		{
		NxBounds3 camAABB;
		camAABB.setCenterExtents(camSphere.center, NxVec3(0.5f));
		NxBox camBox;
		NxMat34 identMat; identMat.id();
		NxCreateBox(camBox, camAABB, identMat);
		NxSweepQueryHit hit;
		if(g_pPhysicsWorld->PhysXScene->linearOBBSpecificShapeSweep(camBox, collShapes[i], ray.dir*20.0f, hit))
		{
		collNormal += hit.normal;
		}
		}

		if(collNormal.magnitude() > 0)
		collNormal.normalize();
		else
		collNormal = rayDir.asNxVec3();

		r3dPoint3D tmp(collNormal.x, collNormal.y, collNormal.z);
		camPos += tmp * 0.02f;
		distance += 0.02f;
		camSphere.center = camPos.asNxVec3();
		LoopBreaker++;
		}*/

	}

	if(checkCamera)
	{
		extern bool g_CameraInsidePlayer;
		if((camPos - target).Length() < 0.6f)
			g_CameraInsidePlayer = true;
		else
			g_CameraInsidePlayer = false;

		if(g_camera_mode->GetInt()==2) // in FPS mode this check not needed
			g_CameraInsidePlayer = false;
	}

	return (LoopBreaker == MaxLoopBreaker);
}

float g_shootCameraShakeTimer = 0.0f;
void Get_Camera_Bob(r3dPoint3D& camBob, r3dPoint3D& camUp, const obj_AI_Player* player)
{
	r3d_assert(player);
	camBob.Assign(0,0,0);
	camUp.Assign(0,1,0);

	static float accumul = 0.0f;
	accumul += r3dGetFrameTime()*1.0f*u_GetRandom(0.75f, 1.25f);

	
	// only use this in FPS, but calculate out here.

	float wave = r3dSin(accumul) * g_shootCameraShakeTimer;
	if(g_shootCameraShakeTimer>0)
	{
		g_shootCameraShakeTimer = R3D_MAX(g_shootCameraShakeTimer-r3dGetFrameTime()*3.f, 0.0f);
	}

	if(g_camera_mode->GetInt()==2)
	{
		r3dPoint3D up(0,1,0);
		r3dPoint3D rightVector = player->m_vVision.Cross( up );
		up.RotateAroundVector(rightVector, wave*20.0f );

		camUp = up;
		
		float mStepDist = 0.1f;
		r3dPoint3D mCurPos = player->GetPosition();
		mCurPos -= player->oldstate.Position;
		float len = mCurPos.Length();

		//BP ok, make step distance the lenth of the anim
		// then len, is the current frame converted to percent :)
		std::vector<r3dAnimation::r3dAnimInfo>::iterator it;
		float curframe = 0;
		float numframe = 0;
		for(it = player->uberAnim_->anim.AnimTracks.begin(); it != player->uberAnim_->anim.AnimTracks.end(); ++it) 
		{
			r3dAnimation::r3dAnimInfo &ai = *it;
			if(!(ai.dwStatus & ANIMSTATUS_Playing)) 
				continue;
			if(!(ai.dwStatus & ANIMSTATUS_Paused)) 
			{
				if(ai.pAnim && ai.pAnim->NumFrames < 60)
				{
					curframe = ai.fCurFrame;
					numframe = (float)ai.pAnim->NumFrames;
					break;
				}
			}
		}
		if(player->PlayerState == PLAYER_MOVE_SPRINT)
			mStepDist *=2;

		float mWave = 0;
		bool rightlean = true;
		if(numframe > 0)
		{
			mWave = curframe / numframe; 
			
			// want to go 0-1-0 at 0,50,100
			// want to go 0-1-0-(-1)-0 at 0,25,50,75,100
 			if(mWave >= 0.5f)
			{	
				mWave -= 0.5f;
				rightlean = false; 
			} else 
			{
				rightlean = true; 

			}

 			mWave *=2;
			// if greater than .5, subtract .5 to make 0.5
			// now go back down to zero if > .5
			if(mWave >= 0.5f)
				mWave = 1.0f - mWave;
			mWave *=2;
		}

		float boba = 0.1f;
		float rolla = 0.2f;

		// in aim mode no bob
		switch ( player->PlayerState)
		{
		case PLAYER_MOVE_WALK_AIM:
			// crouch mode no bob
		case PLAYER_MOVE_CROUCH: // intentional fallthrough
		case PLAYER_MOVE_CROUCH_AIM:// intentional fallthrough
		case PLAYER_IDLE:// intentional fallthrough
		case PLAYER_IDLEAIM:// intentional fallthrough
			// no bob if turn in place or idle
			{
				mWave = 0; 
				boba = 0.0f;
			}
			break;
		// lower bob on low speed
		case PLAYER_MOVE_RUN:
			{
				boba *= .6f; // yeah this doesn't do anything, it's here in case we want to tweak it. 
				rolla = 1;
			}
			break;
		case PLAYER_MOVE_SPRINT:

			{
				boba *=1.0f; 
				rolla = 2;
			}
			break;
		}

		//boba = 0; // disable bob
		camBob.y = boba * sin(mWave * R3D_PI_2);

		r3dPoint3D p(0,1,0);

		float _angle = sin(mWave * R3D_PI_2 ) * rolla;
		if(_angle < 0)
			_angle += 360.0f;
		else if(_angle > 360.0f)
			_angle -=360.0f;
	
		if ( rightlean == false ) 
		{
			_angle = -(_angle);
		}

		p.RotateAroundZ(_angle);
		p.Normalize();
		
		// this currently will half the camUp's lean.  But the system works with this. 
		camUp = camUp + p;
		camUp.Normalize();
	}

	return;

}

static bool g_CameraPointToAdj_HasAdjustedVec = false;
static r3dPoint3D g_CameraPointToAdj_adjVec(0,0,0);
static r3dPoint3D g_CameraPointToAdj_nextAdjVec(0,0,0);

float		g_CameraLeftSideSource = -0.7f;
float		g_CameraLeftSideTarget = 1.0f;
float		g_CameraLeftSideLerp = 1.0f;

float		getCameraLeftSide()
{
	return R3D_LERP(g_CameraLeftSideSource, g_CameraLeftSideTarget, g_CameraLeftSideLerp);
}

void		updateCameraLeftSide()
{
	if(g_CameraLeftSideLerp < 1.0f)
		g_CameraLeftSideLerp = R3D_CLAMP(g_CameraLeftSideLerp+r3dGetFrameTime()*5.0f, 0.0f, 1.0f);
}

r3dPoint3D getAdjustedPointTo(obj_AI_Player* pl, const r3dPoint3D& PointTo, const r3dPoint3D& CamPos)
{
	if(g_camera_mode->GetInt()==2)
		return R3D_ZERO_VECTOR;

	static r3dPoint3D currentLookAt(0,0,0);
	static float	  currentLookAtDist = 0.0f;
	if(LerpValue == 1.0f)
	{
		//r3dOutToLog("Lerp finished\n");
		g_CameraPointToAdj_adjVec = g_CameraPointToAdj_nextAdjVec;
	}
	else if(!g_CameraPointToAdj_HasAdjustedVec)
	{
        {
            r3dPoint3D dir;
            if(pl->m_isInScope || g_camera_mode->GetInt() != 1 )
                r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
            else
                r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);

            PxRaycastHit hit;
            PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
            if(g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), 2000.0f, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eDISTANCE, hit, filter))
            {
                currentLookAt.Assign(hit.impact.x, hit.impact.y, hit.impact.z);
                currentLookAtDist = hit.distance;
            }
            else
            {
                currentLookAt = CamPos + dir * 1000.0f;
                currentLookAtDist = 1000.0f;
            }
        }


		g_CameraPointToAdj_HasAdjustedVec = true;
		r3dPoint3D DestCamPos = pl->GetPosition();
		r3dPoint3D offset;
		if(pl->hasScopeMode())
		{
			offset =  r3dPoint3D( 0, (pl->getPlayerHeightForCamera() +  TargetRig.ScopePosition.Y), 0 );
			offset += pl->GetvRight() * TargetRig.ScopePosition.X;
			offset += pl->m_vVision * (TargetRig.ScopePosition.Z);
		}
		else
		{
			offset =  r3dPoint3D( 0, (pl->getPlayerHeightForCamera() +  TargetRig.Position.Y), 0 );
			offset += pl->GetvRight() * TargetRig.Position.X * getCameraLeftSide();
			offset += pl->m_vVision * (TargetRig.Position.Z);
		}

		DestCamPos += offset;

        r3dPoint3D playerPosHead = pl->GetPosition(); playerPosHead.y += pl->getPlayerHeightForCamera();
        CheckCameraCollision(DestCamPos, playerPosHead, false);

		r3dPoint3D curViewVec = PointTo - CamPos;
		curViewVec.Normalize();

		r3dPoint3D destViewVec = PointTo - DestCamPos;
		destViewVec.Normalize();

		r3dPoint3D curLookAt = currentLookAt;
		if(!pl->hasScopeMode() &&  g_camera_mode->GetInt() == 1 ) // we only need this offset in offcenter mode.
		{
			float fHeight = currentLookAtDist * tan(R3D_DEG2RAD(TargetRig.FOV) * 0.5f);
			curLookAt.y -= fHeight * 0.35f;
		}
		r3dPoint3D destViewVec2 = curLookAt - DestCamPos;
		destViewVec2.Normalize();

       /* float d1 = pl->m_vVision.Dot(destViewVec);
        float d2 = pl->m_vVision.Dot(destViewVec2);
        if(!pl->hasScopeMode())
            if(d2 < 0.99f)
                destViewVec2 = destViewVec;*/

		//r3dOutToLog("Lerp=%.2f, pl_state=%d, aiming=%d\n", LerpValue, pl->PlayerState, pl->m_isAiming);

		static bool wasAiming = false;
		if(pl->m_isAiming || pl->laserViewActive_)
		{
			if(!wasAiming)
			{
				wasAiming = true;
				if(currentLookAtDist < 5.0f && pl->hasScopeMode())
					g_CameraPointToAdj_nextAdjVec = r3dPoint3D(0,0.25f,0);
				else
					g_CameraPointToAdj_nextAdjVec = destViewVec2 - destViewVec;
			}
		}
		else
		{
			wasAiming = false;
			g_CameraPointToAdj_nextAdjVec = r3dPoint3D(0,0,0);
		}

        //r3dOutToLog("(%d): %.2f, %.2f\n", pl->m_isAiming, d1, d2);
		//r3dOutToLog("switching (%d): %.2f, %.2f, %.2f; %.2f\n", pl->m_isAiming, g_CameraPointToAdj_nextAdjVec.x, g_CameraPointToAdj_nextAdjVec.y, g_CameraPointToAdj_nextAdjVec.z, currentLookAtDist);
        //r3dOutToLog("vec: %.2f, %.2f, %.2f; %.2f, %.2f, %.2f\n", destViewVec.x, destViewVec.y, destViewVec.z, destViewVec2.x, destViewVec2.y, destViewVec2.z);
	}

	return R3D_LERP(g_CameraPointToAdj_adjVec, g_CameraPointToAdj_nextAdjVec, LerpValue);
}

extern float DepthOfField_NearStart;
extern float DepthOfField_NearEnd;
extern float DepthOfField_FarStart;
extern float DepthOfField_FarEnd;
extern int _FAR_DOF;
extern int _NEAR_DOF;
extern int LevelDOF;

// runs in actual game
int spectator_observingPlrIdx = 0;
r3dPoint3D spectator_cameraPos(0,0,0);
void TPSGameHUD :: SetCameraPure ( r3dCamera &Cam)
{
#ifndef FINAL_BUILD
	if(d_video_spectator_mode->GetBool() || d_observer_mode->GetBool())
	{
		r3dPoint3D CamPos = FPS_Position;
		CamPos.Y += 1.8f;
		r3dPoint3D ViewPos = CamPos + FPS_vVision*10.0f;

		Cam.FOV = r_video_fov->GetFloat();
		Cam.SetPosition( CamPos );
		Cam.PointTo(ViewPos);

		LevelDOF = 1;
		_NEAR_DOF = 1;
		_FAR_DOF = 1;
		DepthOfField_NearStart = r_video_nearDOF_start->GetFloat();
		DepthOfField_NearEnd = r_video_nearDOF_end->GetFloat();
		DepthOfField_FarStart = r_video_farDOF_start->GetFloat();
		DepthOfField_FarEnd = r_video_farDOF_end->GetFloat();
		
		return;
	}
#endif

	if(hudRespawn->isActive() && !hudRespawn->isWaitingForNextRound())
	{
		int spawnID = hudRespawn->getSpawnID();
		r3dPoint3D cpPos(0,0,0);
		if(spawnID >= gCPMgr.numControlPoints_)
		{
			GameObject* beacon = GameWorld().GetNetworkObject(spawnID);
			if(beacon)
				cpPos = beacon->GetPosition();
			else
				cpPos = gClientLogic().localPlayer_->GetPosition(); // should be ok, beacons visible only after first spawn
		}
		else
			cpPos = gCPMgr.GetCP(spawnID)->GetPosition();
		
		r3dPoint3D camPos = cpPos + r3dPoint3D(1, 40, 0);
		
		Cam.FOV = 60;
		Cam.SetPosition( camPos );
		Cam.PointTo(cpPos);
		FPS_Position = Cam;
		return;
	}
	if(hudEndRound->isActive() || hudPause->isActive())
		return;

	int mMX=Mouse->m_MouseMoveX, mMY=Mouse->m_MouseMoveY;
	if(g_vertical_look->GetBool()) // invert mouse
		mMY = -mMY;

	const ClientGameLogic& CGL = gClientLogic();
	obj_AI_Player* pl = CGL.localPlayer_;
	if(pl == 0)
	{
		r3dPoint3D camPos;
		r3dPoint3D camPointTo;
		bool do_camera = false;
		if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb && (hudRespawn->isWaitingForNextRound() || gClientLogic().m_isSpectator)) // spectator mode
		{
			bool find_new_player = false;
			obj_AI_Player* player = CGL.GetPlayer(spectator_observingPlrIdx);
			if(player == NULL) 
				find_new_player = true;
			else if(player->bDead && (r3dGetTime()-player->TimeOfDeath)>5.0f) // allow X seconds to look at dead body
				find_new_player = true;
			if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE))
				find_new_player = true;

			if(find_new_player)
			{
				bool foundPlr = false;
				for(int i=spectator_observingPlrIdx+1; i<CGL.CurMaxPlayerIdx; ++i)
				{
					obj_AI_Player* plr = CGL.GetPlayer(i);
					if(plr && !plr->bDead)
					{
						spectator_observingPlrIdx = i;
						player = plr;
						foundPlr = true;
						break;
					}
				}
				if(!foundPlr)
				{
					for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
					{
						obj_AI_Player* plr = CGL.GetPlayer(i);
						if(plr && !plr->bDead)
						{
							spectator_observingPlrIdx = i;
							player = plr;
							break;
						}
					}
				}
			}

			if(player)
			{
				float  glb_MouseSensAdj = CurrentRig.MouseSensetivity * g_mouse_sensitivity->GetFloat();	
				//  Mouse controls are here
				static r3dPoint3D camViewAngle(0,0,0);
				camViewAngle.x += float(-mMX) * glb_MouseSensAdj;
				camViewAngle.y += float(-mMY) * glb_MouseSensAdj;

				if(camViewAngle.x > 360.0f ) camViewAngle.x = camViewAngle.x - 360.0f;
				if(camViewAngle.x < 0.0f )   camViewAngle.x = camViewAngle.x + 360.0f;

				// Player can't look too high!
				if(camViewAngle.y > 50 ) camViewAngle.y = 50;
				if(camViewAngle.y < -60) camViewAngle.y = -60;

				D3DXMATRIX mr;
				D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-camViewAngle.x), R3D_DEG2RAD(-camViewAngle.y), 0);
				r3dPoint3D vVision  = r3dVector(mr._31, mr._32, mr._33);

				camPos = player->GetPosition();
				camPos.Y +=  (player->getPlayerHeightForCamera()+  TPSHudCameras[0][0].Position.Y);
				camPos += player->GetvRight() * TPSHudCameras[0][0].Position.X * getCameraLeftSide();
				camPos += vVision * TPSHudCameras[0][0].Position.Z;

				r3dPoint3D playerPosHead = player->GetPosition(); playerPosHead.y += player->getPlayerHeightForCamera();
				camPointTo = player->GetPosition();
				camPointTo.Y += (player->getPlayerHeightForCamera() + TPSHudCameraTarget.Y);
				camPointTo += player->GetvRight() * TPSHudCameraTarget.X;

				CheckCameraCollision(camPos, playerPosHead, false);

				camPointTo += vVision * 50;

				do_camera = true;

				// update minimap
				hudMinimap->SetCameraPosition(camPos, (camPointTo-camPos).Normalize());
			}
		}

		if(do_camera)
		{
			spectator_cameraPos = camPos;
			Cam.FOV = 60;
			Cam.SetPosition( camPos );
			Cam.PointTo( camPointTo );
			FPS_Position = Cam;
		}
		return;
	}

	// uav camera
	extern bool SetCameraPlayerUAV(const obj_AI_Player* pl, r3dCamera &Cam);
	if(SetCameraPlayerUAV(pl, Cam))
	{
		FPS_Position = Cam;
		return;
	}

	extern bool SetCameraPlayerVehicle(const obj_AI_Player* pl, r3dCamera &Cam);
	if(SetCameraPlayerVehicle(pl, Cam))
	{
		FPS_Position = Cam;
		return;
	}


	// dead camera
	if(pl->bDead)
	{
		obj_AI_Player* killer = (obj_AI_Player*)GameWorld().GetNetworkObject(pl->Dead_KillerID);
		r3dPoint3D camPos, camPointTo;
		bool do_camera = false;
		bool check_cam_collision = true;
		if((r3dGetTime() - pl->TimeOfDeath)<3.0f) // show dead player for 2 seconds
		{
			static r3dPoint3D oldPlayerPos(0,0,0);
			static r3dPoint3D camPosOffset(0,0,0);
			camPointTo = pl->GetPosition();

			obj_AI_Player* killer = (obj_AI_Player*)GameWorld().GetNetworkObject(pl->Dead_KillerID);
			if(killer)
				hudMain->ShowKillTag(killer, pl->DeathDamageSource);

			// find a cam position
			if(!oldPlayerPos.AlmostEqual(pl->GetPosition())) // make sure to do that check only once
			{
				oldPlayerPos = pl->GetPosition();
				r3dPoint3D possible_cam_offset[4] = {r3dPoint3D(-3, 5, -3), r3dPoint3D(3, 5, -3), r3dPoint3D(-3, 5, 3), r3dPoint3D(3, 5, 3)};
				int found=-1;
				for(int i=0; i<4; ++i)
				{
					r3dPoint3D raydir = ((pl->GetPosition()+possible_cam_offset[i]) - camPointTo);
					float rayLen = raydir.Length();
					if(rayLen > 0)
					{
						raydir.Normalize();
						PxRaycastHit hit;
						PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
						if(!g_pPhysicsWorld->raycastSingle(PxVec3(camPointTo.x, camPointTo.y, camPointTo.z), PxVec3(raydir.x, raydir.y, raydir.z), rayLen, PxSceneQueryFlag::eIMPACT, hit, filter))
						{
							found = i;
							break;
						}
					}
				}
				if(found!=-1)
				{
					camPosOffset = possible_cam_offset[found];
				}
				else
				{
					camPosOffset = r3dPoint3D(-0.1f, 5, -0.1f);
				}
			}

			camPos = pl->GetPosition() + camPosOffset; 
			do_camera = true;
			check_cam_collision = false;
		}
		else if(killer && (r3dGetTime() - pl->TimeOfDeath)<4.0f && !pl->DisableKillerView && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb) // for next 2 seconds show enemy
		{
			pl->DisableKillerView = InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE);
			camPointTo = killer->GetPosition(); camPointTo.y += pl->Height;
			r3dPoint3D viewVector(0,0,5);
			D3DXVECTOR3 res;
			D3DXVec3TransformNormal(&res, viewVector.d3dx(), &killer->MoveMatrix);
			viewVector.x = res.x;
			viewVector.y = -1.0f;
			viewVector.z = res.z;

			camPos = camPointTo - viewVector; 
			do_camera = true;
		}
		else if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb && (r3dGetTime() - pl->TimeOfDeath)>=2.0f && !hudRespawn->isActive()) // after that in bomb mode show your allies until respawn time
		{
			hudMain->switchToDead(true, true); // force spectator mode
			static int friendlyIndx = 0;
			bool find_new_ally = false;
			obj_AI_Player* ally = CGL.GetPlayer(friendlyIndx);
			if(ally == NULL) 
				find_new_ally = true;
			else if(ally->bDead && (r3dGetTime()-ally->TimeOfDeath)>5.0f) // allow X seconds to look at dead body
				find_new_ally = true;
			else if(ally->TeamID != CGL.localPlayer_->TeamID)
				find_new_ally = true;
			if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE))
				find_new_ally = true;

			if(find_new_ally)
			{
				bool foundPlr = false;
				for(int i=friendlyIndx+1; i<CGL.CurMaxPlayerIdx; ++i)
				{
					obj_AI_Player* plr = CGL.GetPlayer(i);
					if(plr && !plr->bDead && 
					   plr->TeamID == CGL.localPlayer_->TeamID && 
					   plr != CGL.localPlayer_)
					{
						friendlyIndx = i;
						ally = plr;
						foundPlr = true;
						break;
					}
				}
				if(!foundPlr)
				{
					for(int i=0; i<CGL.CurMaxPlayerIdx; ++i)
					{
						obj_AI_Player* plr = CGL.GetPlayer(i);
						if(plr && !plr->bDead && 
						   plr->TeamID == CGL.localPlayer_->TeamID && 
						   plr != CGL.localPlayer_)
						{
							friendlyIndx = i;
							ally = plr;
							break;
						}
					}
				}
			}

			if(ally)
			{
				float  glb_MouseSensAdj = CurrentRig.MouseSensetivity * g_mouse_sensitivity->GetFloat();	
				//  Mouse controls are here
				static r3dPoint3D camViewAngle(0,0,0);
				camViewAngle.x += float(-mMX) * glb_MouseSensAdj;
				camViewAngle.y += float(-mMY) * glb_MouseSensAdj;

				if(camViewAngle.x > 360.0f ) camViewAngle.x = camViewAngle.x - 360.0f;
				if(camViewAngle.x < 0.0f )   camViewAngle.x = camViewAngle.x + 360.0f;

				// Player can't look too high!
				if(camViewAngle.y > 50 ) camViewAngle.y = 50;
				if(camViewAngle.y < -60) camViewAngle.y = -60;

				D3DXMATRIX mr;
				//D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-camViewAngle.x), R3D_DEG2RAD(-camViewAngle.y), 0);
				D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(ally->m_fPlayerRotation), R3D_DEG2RAD(-ally->bodyAdjust_y[1]), 0);
				r3dPoint3D vVision  = r3dVector(mr._31, mr._32, mr._33);

				camPos = ally->GetPosition();
				camPos.Y +=  (ally->getPlayerHeightForCamera()+  TPSHudCameras[0][0].Position.Y);
				camPos += ally->GetvRight() * TPSHudCameras[0][0].Position.X * getCameraLeftSide();
				camPos += vVision * TPSHudCameras[0][0].Position.Z;

				r3dPoint3D playerPosHead = ally->GetPosition(); playerPosHead.y += ally->getPlayerHeightForCamera();
				camPointTo = ally->GetPosition();
				camPointTo.Y += (ally->getPlayerHeightForCamera() + TPSHudCameraTarget.Y);
				camPointTo += ally->GetvRight() * TPSHudCameraTarget.X;



				check_cam_collision = false;
				CheckCameraCollision(camPos, playerPosHead, false);

				camPointTo += vVision * 50;

				do_camera = true;
			}
		}

		if(do_camera)
		{
			extern int g_CCBlackWhite;
			extern float g_fCCBlackWhitePwr;
			g_CCBlackWhite = 1;
			g_fCCBlackWhitePwr = R3D_CLAMP((r3dGetTime() - pl->TimeOfDeath)/2.0f, 0.0f, 1.0f); // go to black and white while look at our dead body

			// check for collision
			if(check_cam_collision)
				CheckCameraCollision(camPos, camPointTo, false);

			Cam.FOV = 60;
			Cam.SetPosition( camPos );
			Cam.PointTo( camPointTo );
			FPS_Position = Cam;
			return;
		}
	}

	GameFOV = CurrentRig.FOV;

	float CharacterHeight = pl->getPlayerHeightForCamera();
	r3dPoint3D CamPos = pl->GetPosition();
	CamPos.Y +=  (CharacterHeight+  CurrentRig.Position.Y);
	updateCameraLeftSide();
	CamPos += pl->GetvRight() * CurrentRig.Position.X * getCameraLeftSide();
	CamPos += pl->m_vVision * CurrentRig.Position.Z;

	r3dPoint3D playerPos = pl->GetPosition();
	r3dPoint3D playerPosHead = playerPos; playerPosHead.y += CharacterHeight;
	r3dPoint3D PointTo = playerPos;
	PointTo.Y += (CharacterHeight + TPSHudCameraTarget.Y);
	PointTo += pl->GetvRight() * TPSHudCameraTarget.X;

	// check for collision
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

	r3dPoint3D adjPointTo(0,0,0);
	adjPointTo = getAdjustedPointTo(pl, PointTo, CamPos);

	r3dPoint3D camBob, camUp;
	Get_Camera_Bob(camBob, camUp, pl);

	Cam.FOV = GameFOV;
	Cam.SetPosition( CamPos + camBob );
	Cam.PointTo(PointTo + camBob);
	Cam.vUP = camUp;

	Cam.vPointTo += adjPointTo;

#ifndef FINAL_BUILD
	if( g_pHUDCameraEffects )
	{
		g_pHUDCameraEffects->Update( &Cam, CamPos ) ;
	}
#endif

	FPS_Position = Cam;
}  

static void DrawMenus()
{
#ifndef FINAL_BUILD
	if(d_video_spectator_mode->GetBool() && !d_observer_mode->GetBool()) // no UI in spectator mode
		return;
	if(d_disable_game_hud->GetBool())
		return;
#endif

#if 0
	typedef std::vector<std::string> stringlist_t;
	extern stringlist_t currentMovies ;

	typedef std::vector< float > floats ;
	extern floats movieDurations ;

	char buff[ 512 ] ;
	sprintf( buff, "%d - Num Drawcalls", r3dRenderer->Stats.NumDraws );

	currentMovies.push_back( buff );
	movieDurations.push_back( 0.1f );

	typedef std::vector< int > sorties ;
	static sorties ss ;

	ss.resize( movieDurations.size() );

	for( int i = 0, e = movieDurations.size(); i < e; i ++ )
	{
		ss[ i ] = i ;
	}

	for( int i = 0, e = movieDurations.size(); i < e; i ++ )
	{
		for( int j = 0, e = movieDurations.size() - 1 ; j < e; j ++ )
		{
			if( movieDurations[ ss[ j ] ] > movieDurations[ ss[ j + 1 ] ] )
			{
				std::swap( ss[ j ], ss[ j + 1 ] );
			}
		}
	}

	r3dSetFiltering( R3D_POINT );

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA);

	for( int i = 0, e = (int)currentMovies.size(); i < e; i ++ )
	{
		Font_Label->PrintF(r3dRenderer->ScreenW - 330, r3dRenderer->ScreenH-e*22 - 220 + i*22,r3dColor(255,255,255), "%.1f - %s", movieDurations[ ss[ i ] ] * 1000.f, currentMovies[ ss[ i ] ].c_str() );
	}

	currentMovies.clear();
	movieDurations.clear();

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
#endif

	if(!win::bSuspended && Keyboard->WasPressed(kbsEsc) && !hudEndRound->isActive() && !hudCommCalls->isVisible()) 
	{
		// close chat on Esc
		if(hudMain->isChatVisible())
			hudMain->ForceHideChatWindow();
		else
		{
			if(!hudPause->isActive())
				hudPause->Activate();
			else
				hudPause->Deactivate();
		}
	}

	if(hudPause->isActive())
	{
		// make sure that we close chat
		if(hudMain->isChatVisible())
			hudMain->ForceHideChatWindow();

		r3dMouse::Show(); // make sure that mouse is visible

		R3DPROFILE_START( "hudPause->" );

		hudPause->Update();
		hudPause->Draw();

		R3DPROFILE_END( "hudPause->" );

		return;
	}

	if(hudEndRound->isActive())
	{
		// make sure that we close chat
		if(hudMain->isChatVisible())
			hudMain->ForceHideChatWindow();

		r3dMouse::Show(); // make sure that mouse is visible

		R3DPROFILE_START( "hudEndRound->" );

		hudEndRound->Update();
		hudEndRound->Draw();

		R3DPROFILE_END( "hudEndRound->" );

		return;
	}

	bool scoreSwitch = InputMappingMngr->isPressed(r3dInputMappingMngr::KS_SHOW_SCORE);
	bool ChatWindowSwitch = InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_CHAT);

	const ClientGameLogic& CGL = gClientLogic();
	const obj_AI_Player* pl = CGL.localPlayer_; // can be null
	if(pl == NULL) // no player, we need to show respawn menu and let player enter game
	{
		if(!CGL.m_isSpectator)
		{
			if(hudRespawn->isActive() == false)
				hudRespawn->Activate(CGL.m_onJoinServerAssignedTeamId, 0);

			if(hudRespawn->isSpawning())
				scoreSwitch = 0;
			if(scoreSwitch && !hudScore->isActive())
				hudScore->Activate();

			hudRespawn->Update();
			if(hudScore->isActive())
			{
				if(!scoreSwitch)
					hudScore->Deactivate();

				R3DPROFILE_START( "hudScore->" );

				hudScore->Update();
				hudScore->Draw();

				R3DPROFILE_END( "hudScore->" );
			}
			else
			{
				R3DPROFILE_START( "hudRespawn->" );

				if( CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb || CGL.m_gameHasStarted == false ){ // don't use a mouse if the showing "Waiting for next round"
					r3dMouse::Show(); // make sure that mouse is visible
				}
				hudRespawn->Draw();

				R3DPROFILE_END( "hudRespawn->" );
			}
		}
		else // spectator mode
		{
			if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // in sabotage teams are switched due to legacy shenanigans
				hudMain->SetTickets(CGL.tickets_[1], CGL.tickets_[0]);
			else
				hudMain->SetTickets(CGL.tickets_[0], CGL.tickets_[1]);
			hudMain->Update();
			hudMain->Draw();

			// show minimap
			if (InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_SWITCH_MINIMAP))
				hudMinimap->SwitchMinimap();

			hudMinimap->Update();
			hudMinimap->Draw();

			// show score in spectator mode
			if(scoreSwitch && !hudScore->isActive())
				hudScore->Activate();

			if(hudScore->isActive())
			{
				if(!scoreSwitch)
					hudScore->Deactivate();

				R3DPROFILE_START( "hudScore->" );

				hudScore->Update();
				hudScore->Draw();

				R3DPROFILE_END( "hudScore->" );
			}


		}
	}
	else
	{
		// check for respawn screen
		if(pl->bDead) 
		{
			hudMain->switchToDead(hudRespawn->isActive());

			obj_AI_Player* killer = (obj_AI_Player*)GameWorld().GetNetworkObject(pl->Dead_KillerID);
			
			// show respawn: if no killer - after 2 seconds. If killer - after 4 seconds (2sec to see our body, 2 sec to see killer)
			if(!(((r3dGetTime() - pl->TimeOfDeath)<3.0f) || (killer && (r3dGetTime() - pl->TimeOfDeath)<8.0f && !pl->DisableKillerView)))
			{
				if(hudRespawn->isActive() == false && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
				{
					hudRespawn->Activate(pl->TeamID, pl->LoadoutSlot);
				}
			}
			else
			{
				// otherwise when you spawn you will see for a second blood on your screen
				hudMain->SetBloodLevel(0);
			}
		}
		else
		{
			hudMain->switchToDead(pl->uavViewActive_);
		}

		if(!pl->bDead && hudRespawn->isActive())
			hudRespawn->Deactivate();

		// render flash UI for objects

		R3DPROFILE_START( "GameWorld().Draw(rsDrawFlashUI)" );
		GameWorld().Draw(rsDrawFlashUI);
		R3DPROFILE_END( "GameWorld().Draw(rsDrawFlashUI)" );

		bool chatVisible = hudMain->isChatVisible();
		if (!pl->uavViewActive_ && !pl->laserViewActive_)
		{
			if(ChatWindowSwitch && !hudMinimap->isShowingBigMap())
			{
				hudMain->ShowChatWindow(!chatVisible);
			}
			{
				if(CGL.localPlayer_)
				{
					if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
						hudMain->SetTickets(CGL.tickets_[CGL.localPlayer_->TeamID], CGL.tickets_[1-CGL.localPlayer_->TeamID]);
					else
						hudMain->SetTickets(CGL.tickets_[1], CGL.tickets_[0]);
				}

				R3DPROFILE_START( "hudMain->" );

				hudMain->Update();
				hudMain->Draw();

				R3DPROFILE_END( "hudMain->" );
			}

			// draw minimap on top of hud
			if(!hudMain->isInDeadMode() && !g_hide_minimap->GetInt() )
			{
				R3DPROFILE_START( "hudMinimap" );

				D3DPERF_BeginEvent( 0, L"Minimap" );

				if(!chatVisible)
				{
					D3DPERF_BeginEvent( 0, L"Minimap:Update" );

					if (InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_SWITCH_MINIMAP))
						hudMinimap->SwitchMinimap();
					hudMinimap->Update();

					D3DPERF_EndEvent();
				}

				D3DPERF_BeginEvent( 0, L"Minimap:Draw" );

				hudMinimap->Draw();

				D3DPERF_EndEvent();

				D3DPERF_EndEvent();

				R3DPROFILE_END( "hudMinimap" );
			}
			
			// issue d3d cheat check on some frames
			// must be done after minimap 
			if(!pl->bDead && (r3dGetTime() - pl->TimeOfLastRespawn)>2.0f && !hudMain->isInDeadMode() && !hudScore->isActive() && !hudMinimap->isShowingBigMap() && (pl->m_SelectedWeapon>=0 && pl->m_SelectedWeapon<=2))
			{
				static float nextCheck = r3dGetTime() + u_GetRandom(10.0f, 30.0f);
				if(r3dGetTime() > nextCheck)
				{
					extern void issueD3DAntiCheatCodepath();
					issueD3DAntiCheatCodepath();
					nextCheck = r3dGetTime() + u_GetRandom(10.0f, 30.0f);
				}
			}
		}
		else if(pl->uavViewActive_) 
		{
				obj_UAV* uav = (obj_UAV *)GameWorld().GetObject(pl->uavId_);
				if(uav)
				{
					hudMinimap->SetCameraPosition(uav->GetPosition(), uav->vVision );
				}
				
				if(CGL.localPlayer_)
				{
					if(CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
						hudMain->SetTickets(CGL.tickets_[CGL.localPlayer_->TeamID], CGL.tickets_[1-CGL.localPlayer_->TeamID]);
					else
						hudMain->SetTickets(CGL.tickets_[1], CGL.tickets_[0]);
				}

				R3DPROFILE_START( "hudMain->" );
				hudMain->Update();
				hudMain->Draw();
				R3DPROFILE_END( "hudMain->" );

				hudMinimap->Update();
				hudMinimap->Draw();
				hudDrone->Update(uav);
				hudDrone->Draw();

		}

		if(!pl->bDead) {
			R3DPROFILE_START( "hudBattleZone->" );
			void BattleZoneWork();
			BattleZoneWork();
			R3DPROFILE_END( "hudBattleZone->" );

			R3DPROFILE_START( "hudCommCalls->" );
			hudCommCalls->Update();
			hudCommCalls->Draw();
			R3DPROFILE_END( "hudCommCalls->" );

			if(hudLaserDesignator->isActive() && !pl->laserViewActive_)
				hudLaserDesignator->Deactivate();
			else if(!hudLaserDesignator->isActive() && pl->laserViewActive_)
				hudLaserDesignator->Activate();
			
			if(pl->laserViewActive_)
			{
				R3DPROFILE_START( "hudLaserDesignator->" );
				hudLaserDesignator->Update();
				hudLaserDesignator->Draw();
				R3DPROFILE_END( "hudLaserDesignator->" );
			}
		}

		if(hudRespawn->isActive())
		{
			if(!hudScore->isActive())
				r3dMouse::Show(); // make sure that mouse is visible

			R3DPROFILE_START( "hudRespawn->" );

			hudRespawn->Update();
			hudRespawn->Draw();

			R3DPROFILE_END( "hudRespawn->" );
		}

		if (!pl->uavViewActive_ && !pl->laserViewActive_)
		{
			// main screen active
			if(scoreSwitch && !hudScore->isActive() && !hudMain->isChatVisible())
			{
				hudScore->Activate();
				return; // it'll activate on next frame
			}


			if(hudScore->isActive())
			{
				if(!scoreSwitch)
				{
					// tab was depressed, deactivate it
					hudScore->Deactivate();
				}

				R3DPROFILE_START( "hudScore->" );

				hudScore->Update();
				hudScore->Draw();

				R3DPROFILE_END( "hudScore->" );

				return;
			}
		}

		// draw main hud with hidden mouse
		// this call is FREE if mouse was hidden already
		// [denis]: do not remove for now, this is minor hack for situation when app was started inactive. 
		// [pavel]: that fucks up controls, when big map is on screen, or scoreboard is, you shouldn't be able to move character, as in that mode you are actually using mouse
		//			if app was started inactive, just press M twice and that's it. 
		// [pavel]: ok, that should fix a problem. If non of modal windows are active, then hide mouse.
		if(!win::bSuspended && !hudRespawn->isActive() && !g_cursor_mode->GetInt() && !hudCommCalls->isCommRoseVisible() )
			r3dMouse::Hide();
	}
}

void TPSGameHUD :: Draw()
{
	if(!TPSGameHud_Inited) r3dError("!TPSGameHud_Inited");

	assert(bInited);
	if ( !bInited ) return;

	R3DPROFILE_D3DSTART( D3DPROFILE_SCALEFORM ) ;

	r3dSetFiltering( R3D_POINT );

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHATESTENABLE, 	FALSE );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_ALPHAREF,        	1 );

	r3dRenderer->SetMaterial(NULL);
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

	DrawMenus();

	R3DPROFILE_D3DEND( D3DPROFILE_SCALEFORM ) ;

	return;  
}

static void ProcessUAVMovement(obj_UAV* uav, int mMX, int mMY)
{
	float sens = 0.3f * g_mouse_sensitivity->GetFloat();

	uav->ViewAngle.x += float(-mMX) * sens;
	uav->ViewAngle.y += float(-mMY) * sens;

	if(Gamepad->IsConnected()) // overwrite mouse
	{
		float X, Y;
		Gamepad->GetRightThumb(X, Y);
		uav->ViewAngle.x += float(-X) * sens * r_gamepad_view_sens->GetFloat();
		uav->ViewAngle.y += float(Y) * sens * r_gamepad_view_sens->GetFloat() * (g_vertical_look->GetBool()?-1.0f:1.0f);
	}


	if(uav->ViewAngle.x > 360.0f ) uav->ViewAngle.x = uav->ViewAngle.x - 360.0f;
	if(uav->ViewAngle.x < 0.0f )   uav->ViewAngle.x = uav->ViewAngle.x + 360.0f;
	if(uav->ViewAngle.y > -50 ) uav->ViewAngle.y = -50;
	if(uav->ViewAngle.y < -80) uav->ViewAngle.y = -80;

	D3DXMATRIX mr;
	D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-uav->ViewAngle.x), R3D_DEG2RAD(-uav->ViewAngle.y), 0);
	const r3dPoint3D vVision  = r3dVector(mr._31, mr._32, mr._33);
	const r3dPoint3D vForw    = r3dVector(mr._31, 0, mr._33).NormalizeTo(); // need to be normalized, otherwise UAV speed depends on your camera view angle
	const r3dPoint3D vRight   = r3dVector(mr._11, 0, mr._13);
	const r3dPoint3D vUp      = r3dVector(0, 1, 0);

	uav->vVision = vVision;
	
	r3dPoint3D acc = r3dPoint3D(0, 0, 0);
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_FORWARD)) acc.z = GPP->UAV_FLY_SPEED_V;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_BACKWARD)) acc.z = -GPP->UAV_FLY_SPEED_V;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_LEFT)) acc.x = -GPP->UAV_FLY_SPEED_V;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_RIGHT)) acc.x = GPP->UAV_FLY_SPEED_V;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_UAV_UP)) acc.y = GPP->UAV_FLY_SPEED_H;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_UAV_DOWN)) acc.y = -GPP->UAV_FLY_SPEED_H;

	if(Gamepad->IsConnected())
	{
		float RX, RY, TL, TR;
		Gamepad->GetLeftThumb(RX, RY);
		Gamepad->GetTrigger(TL, TR);

		acc.Z = -RY*r_gamepad_move_speed->GetFloat();
		acc.Z = RY*r_gamepad_move_speed->GetFloat();
		acc.X = -RX*r_gamepad_move_speed->GetFloat();
		acc.X = RX*r_gamepad_move_speed->GetFloat();
		acc.Y = -TR * r_gamepad_move_speed->GetFloat();
		acc.Y = TL * r_gamepad_move_speed->GetFloat();
	}

	
	r3dPoint3D vel = r3dPoint3D(0, 0, 0);
	vel += vForw  * acc.z;
	vel += vRight * acc.x;
	vel += vUp    * acc.y;
	if(vel.Length() > 0.001f)
		uav->SetVelocity(vel);
	
	return;
}

bool SetCameraPlayerUAV(const obj_AI_Player* pl, r3dCamera &Cam)
{
	if(!pl->uavViewActive_)
		return false;

	obj_UAV* uav = (obj_UAV*)GameWorld().GetObject(pl->uavId_);
	if(!uav)
		return false;

	r3dPoint3D Pos = uav->GetPosition(); /*+r3dPoint3D(0,-1,0)*/;
	Cam.FOV = 35;
	Cam.SetPosition(Pos);
	Cam.PointTo(Pos + uav->vVision);
	Cam.vUP = r3dPoint3D(0, 1, 0);
	return true;
}


bool SetCameraPlayerVehicle(const obj_AI_Player* pl, r3dCamera &Cam)
{
	static bool wasDrivenByPlayer = false;
#if VEHICLES_ENABLED
	if ( g_pPhysicsWorld && g_pPhysicsWorld->m_VehicleManager->GetDrivenCar() && d_drive_vehicles->GetBool() == true )
	{
		obj_Vehicle* vehicle = g_pPhysicsWorld->m_VehicleManager->getRealDrivenVehicle();
		if( vehicle ) 
		{
#if	VEHICLE_CINEMATIC_MODE
			r3dVector CamPos = vehicle->GetPosition();
			CamPos += r3dPoint3D( 0, ( 5 ), 0 );

			int mMX=Mouse->m_MouseMoveX, mMY=Mouse->m_MouseMoveY;
			float  glb_MouseSensAdj = CurrentRig.MouseSensetivity * g_mouse_sensitivity->GetFloat();	

			static float camangle = 0;
			camangle += float(-mMX) * glb_MouseSensAdj;

			if(camangle > 360.0f ) camangle = camangle - 360.0f;
			if(camangle < 0.0f )   camangle = camangle + 360.0f;

			D3DXMATRIX mr;
			D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-camangle), 0.0f, 0);
			r3dVector vehicleForwardVector = r3dVector(mr ._31, mr ._32, mr ._33);

			CamPos += -vehicleForwardVector * 8 ;

			Cam.SetPosition(CamPos);
			Cam.PointTo( CamPos + vehicleForwardVector * 3 + r3dVector ( 0, -1, 0) );
			Cam.vUP = r3dPoint3D(0, 1, 0);
#else 
			g_pPhysicsWorld->m_VehicleManager->ConfigureCamera(Cam);
#endif
			wasDrivenByPlayer = true;
		}
		else
		{
			wasDrivenByPlayer = false;
		}
	}
	else
	{
		wasDrivenByPlayer = false;
	}
#endif 
	return wasDrivenByPlayer;

}

void ProcessPlayerMovement(obj_AI_Player* pl, bool editor_debug )
{
	r3d_assert(pl->NetworkLocal);

	// check fire weapon should be called all the time, as it will reset weapon fire in case if you are sitting on the menu, etc
	{
		R3DPROFILE_FUNCTION("update fire");
		pl->CheckFireWeapon();
	}

	r3dPoint3D prevAccel = pl->InputAcceleration;
	pl->InputAcceleration.Assign(0, 0, 0);
	
	static int shiftWasPressed = 0;
	float movingSpeed = pl->plr_local_moving_speed * (1.0f/r3dGetFrameTime());

	// query mouse distance, so it will not be accumulated
	int mMX=Mouse->m_MouseMoveX, mMY=Mouse->m_MouseMoveY;
	if(g_vertical_look->GetBool()) // invert mouse
		mMY = -mMY;

	bool disablePlayerMovement = false;
	if(Mouse->GetMouseVisibility()) // do not update player if we are in menu control mode!
		disablePlayerMovement = true;
	if(hudMain && hudMain->isChatVisible())
		disablePlayerMovement = true;
	if(pl->bDead)
		return;

	if(pl->m_siegeArmingTimer>0)
		disablePlayerMovement = true;

	if(hudLaserDesignator && hudLaserDesignator->disablePlayerMovement())
		disablePlayerMovement = true;

	const Weapon* wpn = pl->m_Weapons[pl->m_SelectedWeapon];

	if(!(hudMinimap && hudMinimap->isShowingBigMap()) 
		&& !(hudMain && hudMain->isChatVisible())
		&& !Mouse->GetMouseVisibility()
		&& wpn)
	{
		if(wpn->getItemID() == WeaponConfig::ITEMID_LLDR && !pl->laserViewActive_) // LLDR
		{
			// flag used to prevent entering to LLDR again while fire key is still pressed from inside lldr menu
			static bool lldrWasActivated = false;
			if(lldrWasActivated && !InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE))
			{
				lldrWasActivated = false;
				Mouse->ClearPressed();
			}

			if(!lldrWasActivated && InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_PRIMARY_FIRE))
			{
				lldrWasActivated = true;
				pl->ToggleLaserView();
			}
		}

		if(wpn->getItemID() == WeaponConfig::ITEMID_Cypher2 && !pl->uavViewActive_) // UAV
		{
			if(InputMappingMngr->wasReleased(r3dInputMappingMngr::KS_PRIMARY_FIRE))
			{
				if(pl->uavRequested_ == 0) // spawn UAV firstly
					pl->ProcessSpawnUAV();
				else
					pl->ToggleUAVView();
			}
		}

		// vehicles
		if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_INTERACT) )
		{
#if VEHICLES_ENABLED
			obj_Vehicle* target_Vehicle = pl->canEnterVehicle();
		
			if( pl->isInVehicle() ) {
				pl->exitVehicle();
			}
			else if ( target_Vehicle  ) // now we're going to try to use vehicles (otherwise UAV characters can't use vehicles).
			{
				pl->enterVehicle( target_Vehicle );
			}
#endif
		}
	}

	// exit from laser view.
	if(pl->laserViewActive_)
	{
		if(Keyboard->WasPressed(kbsEsc) || InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_INTERACT))
		{
			Keyboard->ClearPressed(); // eat up ESC key so exit game screen will not be activated
			pl->ToggleLaserView();
		}
	}
	// exit for uav view
	if(pl->uavViewActive_)
	{
		if(Keyboard->WasPressed(kbsEsc) || InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_INTERACT))
		{
			Keyboard->ClearPressed(); // eat up ESC key so exit game screen will not be activated
			pl->ToggleUAVView();
		}
	}
	// uav first time spawn check, automatically switch
	if(pl->uavRequested_ == 1 && pl->uavId_ != invalidGameObjectID && GameWorld().GetObject(pl->uavId_))
	{
		pl->ToggleUAVView();
		pl->uavRequested_ = 2;
	}
	// uav control
	if(pl->uavViewActive_)
	{
		// UAV might be destroyed at the moment
		obj_UAV* uav = (obj_UAV*)GameWorld().GetObject(pl->uavId_);
		if(uav)
		{
			ProcessUAVMovement(uav, mMX, mMY);
		}

		disablePlayerMovement = true; // do not exit from function, let it go, as otherwise character will be stuck in prev.state
	}

	/*if(pl->m_isFinishedAiming && !pl->m_isInScope)
	{
		if(Keyboard->WasPressed(kbsLeftShift))
		{
			R3D_SWAP(g_CameraLeftSideSource, g_CameraLeftSideTarget);
			g_CameraLeftSideLerp = 0.0f;
		}
	}*/

	//if(pl->m_isFinishedAiming && !pl->m_isInScope)//Mateuus
	//{
		if(Keyboard->WasPressed(kbsLeftAlt))
		{
			R3D_SWAP(g_CameraLeftSideSource, g_CameraLeftSideTarget);
			g_CameraLeftSideLerp = 0.0f;
		}
	//}

	bool  aiming      = pl->m_isAiming || pl->laserViewActive_;
	int   playerState = aiming ? PLAYER_IDLEAIM : PLAYER_IDLE;

	if(!(g_camera_mode->GetInt()==2 && pl->NetworkLocal))
	{
		if(pl->IsJumpActive()) 
		{
			// in jump, keep current state  (so strafe will stay, for example) and disable movement
			playerState = pl->PlayerState;
			//disablePlayerMovement = true;
		}
	}
	
	// not able to sprint with equipped RPG
	bool disableSprint = false;
	r3dAnimation::r3dAnimInfo* animInfo = pl->uberAnim_->anim.GetTrack(pl->uberAnim_->grenadeThrowTrackID);
	if(!(pl->uberAnim_->grenadePinPullTrackID==CUberAnim::INVALID_TRACK_ID && !(animInfo && (animInfo->GetStatus()&ANIMSTATUS_Playing))))
		disableSprint = true;

	// check if player can straighten up, in case if there is something above his head he will not be able to stop crouching
	bool force_crouch = false;
	if(pl->bCrouch)
	{
		PxBoxGeometry bbox(0.2f, 0.9f, 0.2f);
		PxTransform pose(PxVec3(pl->GetPosition().x, pl->GetPosition().y+1.1f, pl->GetPosition().z), PxQuat(0,0,0,1));
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		PxShape* shape;
		force_crouch = g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, shape, filter);
	}
	bool crouching = pl->bCrouch;
	if(pl->bOnGround)
	{
		if(g_toggle_crouch->GetBool())
		{
			if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_CROUCH) || Gamepad->WasReleased(gpB))
				crouching = !crouching;
		}
		else
			crouching = InputMappingMngr->isPressed(r3dInputMappingMngr::KS_CROUCH) || Gamepad->IsPressed(gpB);
	}
	else
		crouching = false;

	if(disablePlayerMovement)
		crouching = false;

	if(force_crouch)
		crouching = true;

	if(crouching) 
		playerState = aiming ? PLAYER_MOVE_CROUCH_AIM : PLAYER_MOVE_CROUCH;

	VMPROTECT_BeginMutation("ProcessPlayerMovement_Accel");	
	{

	r3dPoint3D accelaration(0,0,0);
	if(!disablePlayerMovement && !pl->IsPlantingMine())
	{
		if(pl->bOnGround && (InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_JUMP)||Gamepad->WasPressed(gpA)) 
		   && !crouching /*&& (pl->m_Energy>1.0f)*/ 
		   && !pl->IsJumpActive()
		   && prevAccel.z >= 0 /* prevent jump backward*/
		   )
		{
			pl->StartJump();
		}

		// if facing a wall and cannot sprint - stop sprint
		bool canSprint = (shiftWasPressed<3) || (shiftWasPressed>=3 && movingSpeed > 1.0f);
		if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_SPRINT) || Gamepad->IsPressed(gpLeftShoulder))
			shiftWasPressed++;
		else
			shiftWasPressed = 0;
		
		if(aiming || pl->m_isHoldingBreath) // cannot spring and aim. also, in default key binding spring and hold breath are on the same key
			shiftWasPressed = 0;

		// due to animation, firstly check left and right movement, so that if you move diagonally we will play moving forward animation
		float thumbX, thumbY;
		Gamepad->GetLeftThumb(thumbX, thumbY);
		if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_LEFT)) 
		{
			accelaration += (aiming)?r3dPoint3D(-GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_SIDE,0,0):r3dPoint3D(-GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_SIDE,0,0);
		}
		else if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_RIGHT)) 
		{
			accelaration += (aiming)?r3dPoint3D(GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_SIDE,0,0):r3dPoint3D(GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_SIDE,0,0);
		}
		else if(thumbX!=0.0f)
		{
			accelaration += (aiming)?r3dPoint3D(GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_SIDE*thumbX,0,0):r3dPoint3D(GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_SIDE*thumbX,0,0);
		}

		//r3dOutToLog("sprint: %d, canSprint: %d, speed: %.3f\n", (int)shiftWasPressed, (int)canSprint, movingSpeed);
		if(shiftWasPressed && canSprint /*&& pl->bOnGround*/ && !crouching && !disableSprint && (pl->m_Energy>0.0f) && pl->m_EnergyPenaltyTime<=0 ) 
		{
			playerState = PLAYER_MOVE_SPRINT;
			accelaration *= 0.5f; // half side movement when sprinting
			accelaration += r3dPoint3D(0,0,GPP->AI_SPRINT_SPEED);
			accelaration  = accelaration.NormalizeTo() * GPP->AI_SPRINT_SPEED;
		}
		else
		{
			if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_FORWARD) || shiftWasPressed) 
			{
				accelaration += (aiming)?r3dPoint3D(0,0,GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_FORWARD):r3dPoint3D(0,0,GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_FORWARD);
			}
			else if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_MOVE_BACKWARD))
			{
				accelaration += (aiming)?r3dPoint3D(0,0,-GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_BACKWARD):r3dPoint3D(0,0,-GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_BACKWARD);
			}
			else if(thumbY!=0.0f)
			{
				if(thumbY>0)
					accelaration += (aiming)?r3dPoint3D(0,0,GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_FORWARD*thumbY):r3dPoint3D(0,0,GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_FORWARD*thumbY);
				else
					accelaration += (aiming)?r3dPoint3D(0,0,GPP->AI_WALK_SPEED*GPP->AI_BASE_MOD_BACKWARD*thumbY):r3dPoint3D(0,0,GPP->AI_RUN_SPEED*GPP->AI_BASE_MOD_BACKWARD*thumbY);
			}
		}

		// set walk/run state
		if(playerState != PLAYER_MOVE_SPRINT && !crouching && (accelaration.x || accelaration.z))
			playerState = aiming ? PLAYER_MOVE_WALK_AIM : PLAYER_MOVE_RUN;

		if(playerState == PLAYER_MOVE_WALK_AIM)
		{
			int Walker = pl->CurLoadout.getSkillLevel(CUserSkills::ASSAULT_Walker);
			switch(Walker)
			{
			case 1: accelaration *= 1.10f; break;
			case 2: accelaration *= 1.15f; break;
			case 3: accelaration *= 1.20f; break;
			case 4: accelaration *= 1.25f; break;
			case 5: accelaration *= 1.30f; break;
			default:break;
			}
		}
		if(playerState == PLAYER_MOVE_SPRINT)
		{
			int AssaultBlazingSpeed = pl->CurLoadout.getSkillLevel(CUserSkills::ASSAULT_BlazingSpeed);
			switch(AssaultBlazingSpeed)
			{
			case 1: accelaration *= 1.02f; break;
			case 2: accelaration *= 1.03f; break;
			case 3: accelaration *= 1.05f; break;
			case 4: accelaration *= 1.07f; break;
			case 5: accelaration *= 1.10f; break;
			default:break;
			}
			int MedicBlazingSpeed = pl->CurLoadout.getSkillLevel(CUserSkills::MEDIC_BlazingSpeed);
			switch(MedicBlazingSpeed)
			{
			case 1: accelaration *= 1.02f; break;
			case 2: accelaration *= 1.03f; break;
			case 3: accelaration *= 1.05f; break;
			case 4: accelaration *= 1.07f; break;
			case 5: accelaration *= 1.10f; break;
			default:break;
			}
			int SniperBlazingSpeed = pl->CurLoadout.getSkillLevel(CUserSkills::RECON_BlazingSpeed);
			switch(SniperBlazingSpeed)
			{
			case 1: accelaration *= 1.05f; break;
			case 2: accelaration *= 1.10f; break;
			case 3: accelaration *= 1.15f; break;
			case 4: accelaration *= 1.20f; break;
			case 5: accelaration *= 1.25f; break;
			default:break;
			}
		}

		if((playerState == PLAYER_MOVE_RUN || playerState == PLAYER_MOVE_SPRINT) && (r3dGetTime() < pl->m_SpeedBoostTime))
		{
			accelaration *= pl->m_SpeedBoost;
		}

		STORE_CATEGORIES equippedItemCat = wpn ? wpn->getCategory() : storecat_INVALID;;
		if(equippedItemCat == storecat_SUPPORT || equippedItemCat == storecat_MG)
			accelaration *= 0.8f; // 20% slow down
	}


	if( pl->CurLoadout.hasItem( AbilityConfig::AB_SecondWind ) && pl->m_RemainingSecondWindTime != 0 ) { 
		pl->m_RemainingSecondWindTime -= r3dGetFrameTime();
		pl->m_RemainingSecondWindTime = R3D_MAX( pl->m_RemainingSecondWindTime, 0.0f );
		accelaration *= 1.10f;
	}

	if(crouching)
		accelaration *= 0.4f;

	/*if(pl->IsJumpActive()) // don't allow to change direction when jumping
		pl->InputAcceleration = prevAccel;
	else*/
		pl->InputAcceleration = accelaration;

	if(!editor_debug)
		pl->PlayerState   = playerState;

	pl->PlayerMoveDir = CUberData::GetMoveDirFromAcceleration(pl->InputAcceleration);
	
	} VMPROTECT_End();	

	// adjust player physx controller size
	// TODO: we need to adjust size only when animation blending was finished! ask Denis how.
	if(crouching != pl->bCrouch)
	{
		// GetPosition()/SetPosition() to keep player on the ground.
		// because capsule controller height offset will be changed in AdjustControllerSize()
		r3dPoint3D pos = pl->PhysicsObject->GetPosition();
		if(crouching)
			pl->PhysicsObject->AdjustControllerSize(0.3f, 0.2f, 0.4f);
		else
			pl->PhysicsObject->AdjustControllerSize(0.3f, 1.1f, 0.85f);
		pl->PhysicsObject->SetPosition(pos + r3dPoint3D(0, 0.01f, 0));
		g_pPhysicsWorld->CharacterManager->updateControllers();// need to update internal position on character controller, otherwise camera jerks a little bit
	}
	
	pl->bCrouch = crouching;

	ActiveCameraRigID = (Playerstate_e)pl->PlayerState;
	ActiveCameraRig   = TPSHudCameras[g_camera_mode->GetInt()][ActiveCameraRigID];
	
	// use this to update the camera from the options. 
	static int currentCameraMode = g_camera_mode->GetInt();

	// if we arn't in the correct view mode currently.   And we are not doing a aim zoom, or the previous lerp is done. 
	if ( ( CurrentState != pl->PlayerState || currentCameraMode != g_camera_mode->GetInt()) && (LerpValue >= 1.0f || ( !pl->m_isAiming && !pl->laserViewActive_ ) ) )
	{
		currentCameraMode = g_camera_mode->GetInt();
		//set new target
		SourceRig = CurrentRig;
		TargetRig = ActiveCameraRig;

		if(SourceRig.Position.AlmostEqual(TargetRig.Position) && SourceRig.ScopePosition.AlmostEqual(TargetRig.ScopePosition))
		{
			// workaround for a quickscoping and firing at the same time and causing a camera to tilt up.
			if((TPSHudCameras[g_camera_mode->GetInt()][CurrentState].allowScope && !TargetRig.allowScope) || (!TPSHudCameras[g_camera_mode->GetInt()][CurrentState].allowScope && TargetRig.allowScope))
			{
				g_CameraPointToAdj_HasAdjustedVec = false;
			}
		}
		else
		{
			g_CameraPointToAdj_adjVec = R3D_LERP(g_CameraPointToAdj_adjVec, g_CameraPointToAdj_nextAdjVec, LerpValue);
			g_CameraPointToAdj_HasAdjustedVec = false;
		}

		LerpValue = 0;

		CurrentState = (Playerstate_e)pl->PlayerState;	
	}
	else
	{
		// just lerp
		if (LerpValue < 1.0f)
		{
			float lerpMOD = 1.0f;
			STORE_CATEGORIES equippedItemCat = wpn ? wpn->getCategory() : storecat_INVALID;;
			if(TargetRig.allowScope) // slow down aiming for those categories
				if(equippedItemCat == storecat_MG || equippedItemCat == storecat_SUPPORT)
					lerpMOD = 0.5f;

			LerpValue += r3dGetFrameTime()*6.5f*lerpMOD;
			if (LerpValue >1.0f) LerpValue = 1.0f;

			CurrentRig.Lerp(pl, SourceRig, TargetRig, LerpValue);
		}
		else
			CurrentRig.Lerp(pl, SourceRig, TargetRig, 1.0f);
	}

	if(!disablePlayerMovement)
	{
		float  glb_MouseSensAdj = CurrentRig.MouseSensetivity * g_mouse_sensitivity->GetFloat();	
		//  Mouse controls are here

		float mmoveX = float(-mMX) * glb_MouseSensAdj;
		float mmoveY = float(-mMY) * glb_MouseSensAdj;

		// fight only vertical recoil, apply adjustment leftover to viewvector
		if(pl->RecoilViewModTarget.y > 0.01f && mmoveY < 0) {
			pl->RecoilViewModTarget.y += mmoveY;
			if(pl->RecoilViewModTarget.y < 0) {
				mmoveY = pl->RecoilViewModTarget.y;
				pl->RecoilViewModTarget.y = 0;
			} else {
				mmoveY = 0;
			}
		}

		pl->ViewAngle.x += mmoveX;
		pl->ViewAngle.y += mmoveY;

		if(Gamepad->IsConnected()) // overwrite mouse
		{
			float X, Y;
			Gamepad->GetRightThumb(X, Y);
			pl->ViewAngle.x += float(-X) * glb_MouseSensAdj * r_gamepad_view_sens->GetFloat();
			pl->ViewAngle.y += float(Y) * glb_MouseSensAdj * r_gamepad_view_sens->GetFloat() * (g_vertical_look->GetBool()?-1.0f:1.0f);
		}

		if(pl->ViewAngle.x > 360.0f ) pl->ViewAngle.x = pl->ViewAngle.x - 360.0f;
		if(pl->ViewAngle.x < 0.0f )   pl->ViewAngle.x = pl->ViewAngle.x + 360.0f;

		// Player can't look too high!
		if(pl->ViewAngle.y > CurrentRig.LookUpLimit )  pl->ViewAngle.y = CurrentRig.LookUpLimit;
		if(pl->ViewAngle.y < CurrentRig.LookDownLimit) pl->ViewAngle.y = CurrentRig.LookDownLimit;

		// set player rotation (except when planting mines)
		if(!pl->IsPlantingMine())
			pl->m_fPlayerRotationTarget = -pl->ViewAngle.x;

		// calculate player vision
		r3dVector FinalViewAngle = pl->ViewAngle + pl->RecoilViewMod + pl->SniperViewMod;
		if(FinalViewAngle.x > 360.0f ) FinalViewAngle.x = FinalViewAngle.x - 360.0f;
		if(FinalViewAngle.x < 0.0f )   FinalViewAngle.x = FinalViewAngle.x + 360.0f;
		// Player can't look too high!
		if(FinalViewAngle.y > CurrentRig.LookUpLimit )  FinalViewAngle.y = CurrentRig.LookUpLimit;
		if(FinalViewAngle.y < CurrentRig.LookDownLimit) FinalViewAngle.y = CurrentRig.LookDownLimit;

		D3DXMATRIX mr;
		D3DXMatrixRotationYawPitchRoll(&mr, R3D_DEG2RAD(-FinalViewAngle.x), R3D_DEG2RAD(-FinalViewAngle.y), 0);
		pl->m_vVision  = r3dVector(mr._31, mr._32, mr._33);
	}

	if (TargetRig.FXFunc) TargetRig.FXFunc(LerpValue);

	pl->UpdateLocalPlayerMovement();
}

//----------------------------------------------------------------
void TPSGameHUD :: Process()
//----------------------------------------------------------------
{
	if( g_cursor_mode->GetInt() )
	{
		imgui_Update();
		imgui2_Update();
	}

	if( hudRespawn->isActive() )
	{
		r3dSetAsyncLoading( 0 ) ;
	}
	else
	{
		r3dSetAsyncLoading( 1 ) ;
	}

	obj_AI_Player* pl = gClientLogic().localPlayer_;
	if(!pl) return;

#ifndef FINAL_BUILD

	bool allow_specator_mode = true;
	if(Keyboard->WasPressed(kbsF8) && allow_specator_mode)
	{
		d_video_spectator_mode->SetBool(!d_video_spectator_mode->GetBool());
		static float DOF_NS=0, DOF_NE=0, DOF_FS=0, DOF_FE=0;
		static int DOF_N=0, DOF_F=0, DOF_ENABLE=0;
		if(d_video_spectator_mode->GetBool())
		{
			FPS_vViewOrig.Assign(pl->ViewAngle);
			// save
			DOF_NS=DepthOfField_NearStart;
			DOF_NE=DepthOfField_NearEnd;
			DOF_FS=DepthOfField_FarStart;
			DOF_FE=DepthOfField_FarEnd;
			DOF_N=_NEAR_DOF;
			DOF_F=_FAR_DOF;
			DOF_ENABLE=LevelDOF;
		}
		else
		{
			// restore
			DepthOfField_NearStart=DOF_NS;
			DepthOfField_NearEnd=DOF_NE;
			DepthOfField_FarStart=DOF_FS;
			DepthOfField_FarEnd=DOF_FE;
			_NEAR_DOF=DOF_N;
			_FAR_DOF=DOF_F;
			LevelDOF=DOF_ENABLE;
		}
	}

	bool allow_observer_mode = true;
	allow_observer_mode = false;

	if(Keyboard->WasPressed(kbsF9) && allow_observer_mode)
	{
		d_observer_mode->SetBool(!d_observer_mode->GetBool());
		if(d_observer_mode->GetBool())
		{
			FPS_vViewOrig.Assign(pl->ViewAngle);
		}
	}

	if(d_video_spectator_mode->GetBool() || d_observer_mode->GetBool())
	{
		FPS_Acceleration.Assign(0, 0, 0);

		float  glb_MouseSensAdj = g_mouse_sensitivity->GetFloat();	
		// camera view
		if(Gamepad->IsConnected())
		{
			float X, Y;
			Gamepad->GetRightThumb(X, Y);
			FPS_vViewOrig.x += float(-X) * r_gamepad_view_sens->GetFloat();
			FPS_vViewOrig.y += float(Y) * r_gamepad_view_sens->GetFloat() * (g_vertical_look->GetBool()?-1.0f:1.0f);
		}
		else // mouse fallback
		{
			int mMX=Mouse->m_MouseMoveX, mMY=Mouse->m_MouseMoveY;

			FPS_vViewOrig.x += float(-mMX) * glb_MouseSensAdj;
			FPS_vViewOrig.y += float(-mMY) * glb_MouseSensAdj * (g_vertical_look->GetBool()?-1.0f:1.0f);
		}

		if(FPS_vViewOrig.y > 85)  FPS_vViewOrig.y = 85;
		if(FPS_vViewOrig.y < -85) FPS_vViewOrig.y = -85;

		FPS_ViewAngle = FPS_vViewOrig;

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
		if(Keyboard->IsPressed(kbsLeftShift)) mult = d_spectator_fast_move_mul->GetFloat();
		if(Keyboard->IsPressed(kbsLeftControl)) mult = d_spectator_slow_move_mul->GetFloat();

		if(Keyboard->IsPressed(kbsW)) FPS_Acceleration.Z = fSpeed;
		if(Keyboard->IsPressed(kbsS)) FPS_Acceleration.Z = -fSpeed * 0.7f;
		if(Keyboard->IsPressed(kbsA)) FPS_Acceleration.X = -fSpeed * 0.7f;
		if(Keyboard->IsPressed(kbsD)) FPS_Acceleration.X = fSpeed * 0.7f;
		if(Keyboard->IsPressed(kbsQ)) FPS_Position.Y    += SRV_WORLD_SCALE(1.0f)* r3dGetFrameTime() * mult;
		if(Keyboard->IsPressed(kbsE)) FPS_Position.Y    -= SRV_WORLD_SCALE(1.0f)* r3dGetFrameTime() * mult;

		if(Gamepad->IsConnected())
		{
			float RX, RY, TL, TR;
			Gamepad->GetLeftThumb(RX, RY);
			Gamepad->GetTrigger(TL, TR);

			FPS_Acceleration.Z = -RY*r_gamepad_move_speed->GetFloat();
			FPS_Acceleration.Z = RY*r_gamepad_move_speed->GetFloat();
			FPS_Acceleration.X = -RX*r_gamepad_move_speed->GetFloat();
			FPS_Acceleration.X = RX*r_gamepad_move_speed->GetFloat();
			FPS_Position.Y    += r3dGetFrameTime() * TR * r_gamepad_move_speed->GetFloat();
			FPS_Position.Y    -= r3dGetFrameTime() * TL * r_gamepad_move_speed->GetFloat();
		}

		FPS_Position += FPS_vVision * FPS_Acceleration.Z * r3dGetFrameTime() * mult;
		FPS_Position += FPS_vRight * FPS_Acceleration.X * r3dGetFrameTime() *mult;

		return;
	}
#endif
	ProcessPlayerMovement(pl, false);
}