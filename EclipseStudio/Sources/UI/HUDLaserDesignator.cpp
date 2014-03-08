#include "r3dPCH.h"
#include "r3d.h"

#include "HUDLaserDesignator.h"

#include "LangMngr.h"
#include "..\multiplayer\ClientGameLogic.h"
#include "..\ObjectsCode\AI\AI_Player.H"
#include "..\ObjectsCode\Airstrikes\Airstrike.h"


#include "HUDDisplay.h"
extern HUDDisplay*	hudMain;

HUDLaserDesignator::HUDLaserDesignator()
: m_bInited ( false )
{
}

HUDLaserDesignator::~HUDLaserDesignator()
{
}

bool HUDLaserDesignator::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\HUD_LaserDesignator.swf", false)) 
		return false;

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDLaserDesignator>(this, &HUDLaserDesignator::FUNC)
	gfxMovie.RegisterEventHandler("eventLockSuccess", MAKE_CALLBACK(eventLockSuccess));

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
	gfxMovie.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_Center);

	for(int i=0; i<getNumAirstrikes(); ++i)
	{
		const AirstrikeDataset* ASData = getAirstrike(i);
		if(!ASData->isAvailable)
			continue;

		// find price in store
		for(uint32_t j=0; j<g_NumStoreItems; ++j)
		{
			if(g_StoreItems[j].itemID == ASData->itemID)
			{
				if(g_StoreItems[j].price1day == 0)
					break;

				Scaleform::GFx::Value var[14];
				var[0].SetStringW(gLangMngr.getString(ASData->Name));
				var[1].SetNumber(g_StoreItems[j].price1day); // price
				var[2].SetNumber(ASData->itemID); // itemID
				var[3].SetNumber(ASData->CurrentCooldown[0]); // cooldown, if any
				char tmpStr1[16], tmpStr2[16], tmpStr3[16], tmpStr4[16];
				sprintf(tmpStr1, "%.1f", ASData->DamageAmount);
				sprintf(tmpStr2, "%.1f m", ASData->DamageRadius);
				sprintf(tmpStr3, "%.1f m", ASData->StrikeRadius);
				sprintf(tmpStr4, "%.1f sec", ASData->StrikeDelay);
				var[4].SetString(tmpStr1);
				var[5].SetString(tmpStr2);
				var[6].SetString(tmpStr3);
				var[7].SetString(tmpStr4);
				var[8].SetStringW(gLangMngr.getString(ASData->DescName));
				var[9].SetString(ASData->icon);
				var[10].SetNumber(ASData->DamageAmountProgrUI);
				var[11].SetNumber(ASData->DamageRadiusProgrUI);
				var[12].SetNumber(ASData->StrikeRadiusProgrUI);
				var[13].SetNumber(ASData->StrikeDelayProgrUI);

				gfxMovie.Invoke("_root.api.addAirstrike", var, 14);
				break;
			}
		}
	}

	gfxMovie.Invoke("_root.api.populateList", "");

	gfxMovie.SetVariable("_root.Scroller.UpText.Text.text", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_LASER_VIEW_UP));
	gfxMovie.SetVariable("_root.Scroller.DownText.Text.text", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_LASER_VIEW_DOWN));
	{
		char tmpStr[16];
		sprintf(tmpStr, "'%s'", InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_PRIMARY_FIRE));
		gfxMovie.SetVariable("_root.Warning.Key.text", tmpStr);
	}

	gfxMovie.Invoke("_root.api.enableAirstrikeSelection", true);
	gfxMovie.SetVariable("_root.ArmedWarning._visible", false);

	m_bInited = true;
	m_bActive = false;

	m_bTargeting = false;
	m_bSelectingAirstrike = false;
	m_TargetPos.Assign(0,0,0);

	return true;
}

bool HUDLaserDesignator::Unload()
{
	if(m_bInited)
		gfxMovie.Unload();
	m_bInited = false;
	return true;
}

void HUDLaserDesignator::showWarning(const wchar_t* text)
{
	gfxMovie.SetVariable("_root.WarningBox.Text.Text.Text.text", text);
	gfxMovie.Invoke("_root.WarningBox.gotoAndPlay", 2);
}

void HUDLaserDesignator::Update()
{
	if(gClientLogic().localPlayer_ == NULL)
		return;

	const float MAX_CASTING_DISTANCE = 20000.f;
	r3dPoint3D dir;
	r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);

	float distance = 20000;
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			m_TargetPos.Assign(hit.impact.x, hit.impact.y, hit.impact.z);
			distance = (gCam - m_TargetPos).Length();
		}
	}
	char tempStr[32];
	sprintf(tempStr, "%.1f", distance);
	gfxMovie.Invoke("_root.api.setDistance", tempStr);

	gfxMovie.Invoke("_root.api.setZoom", "1.0");

	float compass = atan2f(gClientLogic().localPlayer_->m_vVision.z, gClientLogic().localPlayer_->m_vVision.x)/R3D_PI;
	compass = ceilf(R3D_CLAMP(compass, -1.0f, 1.0f)*180);
	gfxMovie.Invoke("_root.api.setCompass", compass);

	if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_LASER_VIEW_UP))
		gfxMovie.Invoke("_root.api.slotUp", "");
	if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_LASER_VIEW_DOWN))
		gfxMovie.Invoke("_root.api.slotDown", "");

	int mouseScroll = 0;
	Mouse->QueryWheelMotion(&mouseScroll);
	if(mouseScroll!=0)
	{
		if(mouseScroll>0)
			gfxMovie.Invoke("_root.api.slotUp", "");
		else
			gfxMovie.Invoke("_root.api.slotDown", "");
	}

	int plrTeamID = gClientLogic().localPlayer_->TeamID;
	if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_PRIMARY_FIRE) && (AIRSTRIKE_Team_Cooldown[plrTeamID]<=0))
	{
		if(!m_bTargeting)
		{
			m_bTargeting = true;
			Scaleform::GFx::Value result;
			gfxMovie.Invoke("_root.api.startLock", &result, NULL, 0);
			if(!result.GetBool())
				m_bTargeting = false; // active cooldown
			else
			{
				gfxMovie.Invoke("_root.api.enableAirstrikeSelection", false);
				gfxMovie.SetVariable("_root.ArmedWarning._visible", true);
				m_bSelectingAirstrike = false;
			}
		}
	}
	else if(m_bTargeting)
	{
		m_bTargeting = false;
		gfxMovie.Invoke("_root.api.interruptLock", "");
	}

	// update cooldowns, if any
	for(int i=0; i<getNumAirstrikes(); ++i)
	{
		const AirstrikeDataset* ASData = getAirstrike(i);
		if(!ASData->isAvailable)
			continue;

		float cooldown = ceilf(R3D_MAX(ASData->CurrentCooldown[plrTeamID], AIRSTRIKE_Team_Cooldown[plrTeamID])); // to prevent flash to show number like 23.8343748374384
		Scaleform::GFx::Value var[2];
		var[0].SetNumber(ASData->itemID);
		var[1].SetNumber(cooldown);
		gfxMovie.Invoke("_root.api.setCooldown", var, 2);
	}
	gfxMovie.Invoke("_root.api.updateSlots", "");

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
}

void HUDLaserDesignator::Draw()
{
	gfxMovie.UpdateAndDraw();
}

void HUDLaserDesignator::Activate()
{
	m_bActive = true;
	m_bTargeting = false;
	m_bSelectingAirstrike = true;
	gfxMovie.Invoke("_root.api.enableAirstrikeSelection", true);
	gfxMovie.SetVariable("_root.ArmedWarning._visible", false);
}

void HUDLaserDesignator::Deactivate()
{
	m_bActive = false;
	m_bTargeting = false;
	m_bSelectingAirstrike = false;
}

void HUDLaserDesignator::eventLockSuccess(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	int itemID = (int)args[0].GetNumber();

	// check if user has enough GP
	int price = 0;
	for(uint32_t j=0; j<g_NumStoreItems; ++j)
	{
		if(g_StoreItems[j].itemID == itemID)
		{
			price = g_StoreItems[j].price1day;
			break;
		}
	}

	if(price == 0)
	{
		showWarning(gLangMngr.getString("HUD_Airstrike_GeneralError"));
		return;
	}

	if((gUserProfile.ProfileData.Stats.GamePoints - price)<0)
	{
		showWarning(gLangMngr.getString("HUD_Airstrike_NoGP"));
		return;
	}

	// check where it should collide and send server pos (todo: fix this when server will have physics enabled)
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(m_TargetPos.x, m_TargetPos.y+200, m_TargetPos.z), PxVec3(0, -1, 0), 2000, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			r3dVector hitPos(hit.impact.x, hit.impact.y, hit.impact.z);
			
			PKT_C2S_RequestAirstrike_s n;
			n.pos = hitPos; 
			n.itemID = itemID;
			p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
		}
	}

	m_bTargeting = false;
	gClientLogic().localPlayer_->ToggleLaserView(true);
}
