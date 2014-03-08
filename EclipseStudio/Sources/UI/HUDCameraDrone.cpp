#include "r3dPCH.h"
#include "r3d.h"

#include "../ObjectsCode/ai/AI_Player.H"
#include "ObjectsCode/Gameplay/obj_UAV.h"
#include "../multiplayer/clientgamelogic.h"

#include "HUDCameraDrone.h"

HUDCameraDrone::HUDCameraDrone()
: m_bInited ( false )
{
}

HUDCameraDrone::~HUDCameraDrone()
{
}

bool HUDCameraDrone::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\HUD_UAV_Camera.swf", false)) 
		return false;
    
//	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	Scaleform::GFx::Value var[4];
	var[0].SetString(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_UAV_UP));
	var[1].SetString(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_UAV_DOWN));
	var[2].SetString(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT));
	var[3].SetString(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_UAV_TARGET));
	gfxMovie.Invoke("_global.setButtonText", var, 4);

	m_bInited = true;
	for(int i=0; i<MAX_UAV_TARGETS; ++i)
		m_hasTargetIcon[i] = false;

	return true;
}

bool HUDCameraDrone::Unload()
{
	if(m_bInited)
		gfxMovie.Unload();
	m_bInited = false;
	return true;
}

static void updateAimTarget(GameObject* uav, GameObject** gameObj)
{
	r3d_assert(gameObj);

	const float MAX_CASTING_DISTANCE = 20000.f;
	r3dPoint3D dir;
	r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);
	dir.Normalize();

	*gameObj = NULL;

	{
		// shoot from camera :)
		//PxRaycastHit hits[32];
		PxSweepHit hits[32];
		PhysicsCallbackObject* target = NULL;
		// we only interested in players
		PxSceneQueryFilterData filter(PxFilterData((1<<PHYSCOLL_NETWORKPLAYER),0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC));
		bool blockingHit;
		int numHits = 0;

		PxSphereGeometry camSphere(1.0f);
		PxTransform camPose(PxVec3(gCam.x, gCam.y, gCam.z), PxQuat(0,0,0,1));
		numHits=g_pPhysicsWorld->PhysXScene->sweepMultiple(camSphere, camPose, PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlag::eIMPACT, hits, 32, blockingHit, filter);
//		numHits=g_pPhysicsWorld->PhysXScene->raycastMultiple(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hits, 32, blockingHit, filter);
		if(numHits > 0)
		{
			float closestHit = 99999999.0f;
			GameObject* closestObj = NULL;
			for(int i =0; i<numHits; ++i)
			{
				if(hits[i].shape && (target = static_cast<PhysicsCallbackObject*>(hits[i].shape->getActor().userData)))
				{
					GameObject* obj = target->isGameObject();
					if(obj != uav && hits[i].distance < closestHit)
					{
						closestObj = obj;
						closestHit = hits[i].distance;
					}
				}					
			}
			*gameObj = closestObj;
			return;
		}
	}
}

void HUDCameraDrone::moveScreenIcon(const char* name, const r3dPoint3D& pos, bool alwaysShow)
{
	if(!m_bInited)
		return;
	r3dPoint3D scrCoord;
	float x, y;
	int isVisible = 1;
	if(alwaysShow)
		isVisible = r3dProjectToScreenAlways(pos, &scrCoord, 20, 20);
	else
		isVisible = r3dProjectToScreen(pos, &scrCoord);

	x = scrCoord.x/r3dRenderer->ScreenW;
	y = scrCoord.y/r3dRenderer->ScreenH;

	Scaleform::GFx::Value var[3];
	var[0].SetString(name);
	var[1].SetNumber(x);
	var[2].SetNumber(y);
	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_global.setTargetIconPos"), var, 3);
}

extern float uav_LevelBounds[5];
void HUDCameraDrone::Update(obj_UAV* uav)
{
	if(uav && gClientLogic().localPlayer_)
	{
		float compass = atan2f(uav->vVision.z, uav->vVision.x)/R3D_PI;
		compass = R3D_CLAMP(compass, -1.0f, 1.0f);
		gfxMovie.Invoke("_global.setCompass", compass);

		{
			Scaleform::GFx::Value var[2];
			char tempS[16];
			sprintf(tempS, "%.0f", uav->GetPosition().y);
			var[0].SetString(tempS);
			static float baseHeight = uav->GetPosition().y;
			float scale = uav->GetPosition().y - baseHeight;
			if(scale < -100 || scale > 100)
			{
				baseHeight = uav->GetPosition().y;
				scale = 0;
			}
			var[1].SetNumber(scale/100.0f);
			gfxMovie.Invoke("_global.setHeight", var, 2);
		}

		int maxNumTargets = 1;
		switch(gClientLogic().localPlayer_->CurLoadout.getSkillLevel(CUserSkills::RECON_UAVTagging))
		{
		case 1: maxNumTargets = 2; break;
		case 2: maxNumTargets = 3; break;
		}
		r3d_assert(maxNumTargets<=MAX_UAV_TARGETS);
		for(int i=0; i<maxNumTargets; ++i)
		{
			obj_AI_Player* prevTarget = (obj_AI_Player*)GameWorld().GetObject(gClientLogic().localPlayer_->m_localPlayer_lastUAVTarget[i]);
			char uavTargetUIName[32];
			sprintf(uavTargetUIName, "uavTarget%d", i);
			if(prevTarget && prevTarget->m_localTargetIconTime > 0 && !prevTarget->bDead)
			{
				if(!m_hasTargetIcon[i])
				{
					gfxMovie.Invoke(VMPROTECT_DecryptStringA("_global.addTargetIcon"), uavTargetUIName);
					m_hasTargetIcon[i] = true;
				}
				moveScreenIcon(uavTargetUIName, prevTarget->GetPosition()+r3dPoint3D(0,1,0), true);

				// check if target is visible and if yes, send auto target when time on target is less than 1 second 
				// NOTE: bumping this to 2 seconds since we now have potential latency issues.
				if(r3dProjectToScreen(prevTarget->GetPosition(), NULL) && prevTarget->m_targetIconTime < 2.0f)
				{
					static float lastSent = 0;
					if((r3dGetTime() - lastSent) > 1.0f)
					{
						lastSent = r3dGetTime();
						PKT_C2S_MarkTarget_s n;
						n.targetID = toP2pNetId(prevTarget->NetworkID);
						p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
					}
				}
			}
			else if((prevTarget == NULL || (prevTarget && (prevTarget->m_localTargetIconTime <= 0 || prevTarget->bDead))) && m_hasTargetIcon[i] )
			{
				gfxMovie.Invoke(VMPROTECT_DecryptStringA("_global.deleteScreenIcon"), uavTargetUIName);
				m_hasTargetIcon[i] = false;
			}

			if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_UAV_TARGET))
			{
				GameObject* tempObj;
				updateAimTarget(uav, &tempObj);

				if(tempObj && tempObj->isObjType(OBJTYPE_Human))
				{
					// check that target isn't already selected by us
					bool alreadyTargeted = false;
					for(int j=0; j<maxNumTargets; ++j)
					{
						obj_AI_Player* plr = (obj_AI_Player*)GameWorld().GetObject(gClientLogic().localPlayer_->m_localPlayer_lastUAVTarget[j]);
						if(plr && plr->m_localTargetIconTime > 0 && plr==tempObj)
						{
							alreadyTargeted = true;
							break;
						}
					}
					obj_AI_Player* target = (obj_AI_Player*)tempObj;
					if(!target->bDead && target->TeamID != gClientLogic().localPlayer_->TeamID && (prevTarget==NULL || (prevTarget && prevTarget->m_localTargetIconTime <= 0)) && !alreadyTargeted)
					{
						static float localTimer = 0;
						if(r3dGetTime() - localTimer > 1.0f) // do not allow to spam
						{
							localTimer = r3dGetTime();
							PKT_C2S_MarkTarget_s n;
							n.targetID = toP2pNetId(target->NetworkID);
							p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);

							gClientLogic().localPlayer_->m_localPlayer_lastUAVTarget[i] = target->GetSafeID();
							target->m_localTargetIconTime = 10.0f;
						}
					}
				}
			}
		}
	}
}

void HUDCameraDrone::Draw()
{
	gfxMovie.UpdateAndDraw();
}
