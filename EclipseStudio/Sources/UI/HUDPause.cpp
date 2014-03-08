#include "r3dPCH.h"
#include "r3d.h"

#include "../../../Eternity/sf/Console/config.h"
#include "HUDPause.h"
#include "m_EndRound.h"
#include "HUDDisplay.h"

#include "FrontendShared.h"

extern HUDEndRound*	hudEndRound;
extern HUDDisplay*	hudMain;

HUDPause::HUDPause()
{
	isActive_ = false;
	isInit = false;
	m_waitingForKeyRemap = -1;
}

HUDPause::~HUDPause()
{
}

void HUDPause::eventReturnClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Deactivate();
}

void writeGameOptionsFile();
void HUDPause::eventOptionsApplyClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args!=NULL);
	r3d_assert(argCount==13);
	r_overall_quality->SetInt((int)args[0].GetNumber()+1);
	
	r_brightness->SetFloat(R3D_CLAMP((float)args[1].GetNumber(), 0.25f, 0.75f));
	r_contrast->SetFloat(R3D_CLAMP((float)args[2].GetNumber(), 0.25f, 0.75f));
	
	s_sound_volume->SetFloat(R3D_CLAMP((float)args[3].GetNumber(), 0.0f, 1.0f));
	s_music_volume->SetFloat(R3D_CLAMP((float)args[4].GetNumber(), 0.0f, 1.0f));
	s_comm_volume->SetFloat(R3D_CLAMP((float)args[5].GetNumber(), 0.0f, 1.0f));
	
	bool cam_off_center = args[6].GetBool();
	{
		if(g_camera_mode->GetInt()!=2)
		{
			g_tps_camera_mode->SetInt(cam_off_center?1:0);
			g_camera_mode->SetInt(cam_off_center?1:0);
		}
		hudMain->updateReticlePosition();
	}
	g_enable_voice_commands->SetBool(args[7].GetBool());
	g_vertical_look->SetBool(args[8].GetBool());
	g_mouse_wheel->SetBool(args[10].GetBool());
	g_mouse_sensitivity->SetFloat((float)args[11].GetNumber());
	g_mouse_acceleration->SetBool(args[12].GetBool());

	DWORD settingsChangedFlags = 0;
	switch( r_overall_quality->GetInt() )
	{
	case 1:
		settingsChangedFlags = SetDefaultSettings( S_WEAK );
		break;
	case 2:
		settingsChangedFlags = SetDefaultSettings( S_MEDIUM );
		break;
	case 3:
		settingsChangedFlags = SetDefaultSettings( S_STRONG );
		break;
	case 4:
		settingsChangedFlags = SetDefaultSettings( S_ULTRA );
		break;
	case 5:
		break;
	default:
		r3d_assert( false ); // alles's ein bissn schlecht
	}	

	r3dRenderer->UpdateSettings();
	void applyGraphicsOptions( uint32_t settingsFlags );
	applyGraphicsOptions( settingsChangedFlags );

	// write to ini file
	writeGameOptionsFile();

	Deactivate();
}

void HUDPause::eventExitClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Deactivate();
	if(!hudEndRound->isActive()) // in case if game finished while player was on pause menu
		hudEndRound->Activate();
}

void HUDPause::eventRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int remapIndex = (int)args[0].GetNumber();
	r3d_assert(m_waitingForKeyRemap == -1);

	m_waitingForKeyRemap = remapIndex;
}

bool HUDPause::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\PauseMenu.swf", true)) 
		return false;

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDPause>(this, &HUDPause::FUNC)

	gfxMovie.RegisterEventHandler("eventReturnClick", MAKE_CALLBACK(eventReturnClick));
	gfxMovie.RegisterEventHandler("eventOptionsApplyClick", MAKE_CALLBACK(eventOptionsApplyClick));
	gfxMovie.RegisterEventHandler("eventExitClick", MAKE_CALLBACK(eventExitClick));
	gfxMovie.RegisterEventHandler("eventRequestKeyRemap", MAKE_CALLBACK(eventRequestKeyRemap));

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	m_waitingForKeyRemap = -1;

	// set options
	{
		Scaleform::GFx::Value args[13];
		args[0].SetNumber(r_overall_quality->GetInt()-1);
		args[1].SetNumber(r_brightness->GetFloat());
		args[2].SetNumber(r_contrast->GetFloat());
		args[3].SetNumber(s_sound_volume->GetFloat());
		args[4].SetNumber(s_music_volume->GetFloat());
		args[5].SetNumber(s_comm_volume->GetFloat());
		args[6].SetBoolean( g_tps_camera_mode->GetInt() == 1?true:false); // TODO: this needs to be changed to a number when we change the flash file. 
		args[7].SetBoolean(g_enable_voice_commands->GetBool());
		args[8].SetBoolean(g_vertical_look->GetBool());
		args[9].SetBoolean(false); // not used
		args[10].SetBoolean(g_mouse_wheel->GetBool());
		args[11].SetNumber(g_mouse_sensitivity->GetFloat());
		args[12].SetBoolean(g_mouse_acceleration->GetBool());
		gfxMovie.Invoke("_root.api.setOptionsParams", args, 13);
	}

	// add keyboard shortcuts
	for(int i=0; i<r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetStringW(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxMovie.Invoke("_root.api.addKeyboardShortcut", args, 2);
	}

	isActive_ = false;
	isInit = true;
	return true;
}

bool HUDPause::Unload()
{
	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDPause::Update()
{
	if(m_waitingForKeyRemap != -1)
	{
		// query input manager for any input
		if(InputMappingMngr->attemptRemapKey((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetNumber(m_waitingForKeyRemap);
			var[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap));
			gfxMovie.Invoke("_root.api.updateKeyboardShortcut", var, 2);
			m_waitingForKeyRemap = -1;

			void writeInputMap();
			writeInputMap();
		}
	}

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
}

void HUDPause::Draw()
{
	gfxMovie.UpdateAndDraw();
}

void HUDPause::Deactivate()
{
	if( !g_cursor_mode->GetInt() )
	{
		r3dMouse::Hide();
	}

	isActive_ = false;
	r_gameplay_blur_strength->SetFloat(0.0f);
}

void HUDPause::Activate()
{
	r3d_assert(!isActive_);
	r3dMouse::Show();
	isActive_ = true;
	r_gameplay_blur_strength->SetFloat(1.0f);
	gfxMovie.Invoke("_root.api.Activate", "");
}


