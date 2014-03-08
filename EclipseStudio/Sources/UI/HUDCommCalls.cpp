#include "r3dPCH.h"
#include "r3d.h"

#include "HUDCommCalls.h"

#include "LangMngr.h"
#include "..\multiplayer\ClientGameLogic.h"
#include "..\ObjectsCode\AI\AI_Player.H"

#include "HUDDisplay.h"
extern HUDDisplay*	hudMain;

HUDCommCalls::HUDCommCalls()
: m_bInited ( false )
{
}

HUDCommCalls::~HUDCommCalls()
{
}

bool HUDCommCalls::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\HUD_CommCalls.swf", false)) 
		return false;

	if(!gfxCommRose.Load("Data\\Menu\\HUD_CommRose.swf", false))
		return false;

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDCommCalls>(this, &HUDCommCalls::FUNC)
	gfxCommRose.RegisterEventHandler("eventPressButton", MAKE_CALLBACK(eventPressButton));


	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
	gfxMovie.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_CenterLeft);

	gfxCommRose.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
	gfxCommRose.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_Center);

	m_bInited = true;
	m_bVisible = false;
	m_CurrentMsgType = 0;
	for(int i=0; i<9; ++i)
	{
		m_CommandsZ[i].id = -1;
		m_CommandsX[i].id = -1;
		m_CommandsC[i].id = -1;
	}

	m_bCommRoseVisible = false;

	return true;
}

bool HUDCommCalls::Unload()
{
	if(m_bInited)
	{
		gfxMovie.Unload();
		gfxCommRose.Unload();
	}
	m_bInited = false;
	return true;
}

void HUDCommCalls::Update()
{
	if(gClientLogic().localPlayer_ && m_CommandsZ[0].id == -1)
	{
		// fill in command list
		extern int RUS_CLIENT;
		if(RUS_CLIENT)
		{
			m_CommandsZ[0] = CallCommand(0, gLangMngr.getString("CommandZ1"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ1_RUS"));
			m_CommandsZ[1] = CallCommand(1, gLangMngr.getString("CommandZ2"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ2_RUS"));
			m_CommandsZ[2] = CallCommand(2, gLangMngr.getString("CommandZ3"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ3_RUS"));
			m_CommandsZ[3] = CallCommand(3, gLangMngr.getString("CommandZ4"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ4_RUS"));
			m_CommandsZ[4] = CallCommand(4, gLangMngr.getString("CommandZ5"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ5_RUS"));
			m_CommandsZ[5] = CallCommand(5, gLangMngr.getString("CommandZ6"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandZ6_RUS"));

			m_CommandsX[0] = CallCommand(10, gLangMngr.getString("CommandX1"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX1_RUS"));
			m_CommandsX[1] = CallCommand(11, gLangMngr.getString("CommandX2"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX2_RUS"));
			m_CommandsX[2] = CallCommand(12, gLangMngr.getString("CommandX3"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX3_RUS"));
			m_CommandsX[3] = CallCommand(13, gLangMngr.getString("CommandX4"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX4_RUS"));
			m_CommandsX[4] = CallCommand(14, gLangMngr.getString("CommandX5"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX5_RUS"));
			m_CommandsX[5] = CallCommand(15, gLangMngr.getString("CommandX6"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX6_RUS"));
			m_CommandsX[6] = CallCommand(16, gLangMngr.getString("CommandX7"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandX7_RUS"));

			m_CommandsC[0] = CallCommand(20, gLangMngr.getString("CommandC1"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC1_RUS"));
			m_CommandsC[1] = CallCommand(21, gLangMngr.getString("CommandC2"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC2_RUS"));
			m_CommandsC[2] = CallCommand(22, gLangMngr.getString("CommandC3"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC3_RUS"));
			m_CommandsC[3] = CallCommand(23, gLangMngr.getString("CommandC4"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC4_RUS"));
			m_CommandsC[4] = CallCommand(24, gLangMngr.getString("CommandC5"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC5_RUS"));
			m_CommandsC[5] = CallCommand(25, gLangMngr.getString("CommandC6"), SoundSys.GetEventIDByPath("Sounds/VoiceOver/RUS/CommandC6_RUS"));
		}
		else
		{
			m_CommandsZ[0] = CallCommand(0, gLangMngr.getString("CommandZ1"), -1);
			m_CommandsZ[1] = CallCommand(1, gLangMngr.getString("CommandZ2"), -1);
			m_CommandsZ[2] = CallCommand(2, gLangMngr.getString("CommandZ3"), -1);
			m_CommandsZ[3] = CallCommand(3, gLangMngr.getString("CommandZ4"), -1);
			m_CommandsZ[4] = CallCommand(4, gLangMngr.getString("CommandZ5"), -1);
			m_CommandsZ[5] = CallCommand(5, gLangMngr.getString("CommandZ6"), -1);

			m_CommandsX[0] = CallCommand(10, gLangMngr.getString("CommandX1"), -1);
			m_CommandsX[1] = CallCommand(11, gLangMngr.getString("CommandX2"), -1);
			m_CommandsX[2] = CallCommand(12, gLangMngr.getString("CommandX3"), -1);
			m_CommandsX[3] = CallCommand(13, gLangMngr.getString("CommandX4"), -1);
			m_CommandsX[4] = CallCommand(14, gLangMngr.getString("CommandX5"), -1);
			m_CommandsX[5] = CallCommand(15, gLangMngr.getString("CommandX6"), -1);

			if(gClientLogic().m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
			{
				m_CommandsC[0] = CallCommand(20, gLangMngr.getString("CommandC1"), -1);
				m_CommandsC[1] = CallCommand(21, gLangMngr.getString("CommandC2"), -1);
				m_CommandsC[2] = CallCommand(22, gLangMngr.getString("CommandC3"), -1);
				m_CommandsC[3] = CallCommand(23, gLangMngr.getString("CommandC4"), -1);
				m_CommandsC[4] = CallCommand(24, gLangMngr.getString("CommandC5"), -1);
				m_CommandsC[5] = CallCommand(25, gLangMngr.getString("CommandC6"), -1);
				m_CommandsC[6] = CallCommand(26, gLangMngr.getString("CommandC7"), -1);
				m_CommandsC[7] = CallCommand(27, gLangMngr.getString("CommandC8"), -1);
				m_CommandsC[8] = CallCommand(28, gLangMngr.getString("CommandC9"), -1);
			}
			else
			{
				m_CommandsC[0] = CallCommand(20, gLangMngr.getString("CommandC1S"), -1);
				m_CommandsC[1] = CallCommand(21, gLangMngr.getString("CommandC2S"), -1);
				m_CommandsC[2] = CallCommand(22, gLangMngr.getString("CommandC3S"), -1);
				m_CommandsC[3] = CallCommand(23, gLangMngr.getString("CommandC4S"), -1);
				m_CommandsC[4] = CallCommand(24, gLangMngr.getString("CommandC5S"), -1);
				m_CommandsC[5] = CallCommand(25, gLangMngr.getString("CommandC6S"), -1);
				m_CommandsC[6] = CallCommand(26, gLangMngr.getString("CommandC7S"), -1);
				m_CommandsC[7] = CallCommand(27, gLangMngr.getString("CommandC8S"), -1);
				m_CommandsC[8] = CallCommand(28, gLangMngr.getString("CommandC9S"), -1);
			}
		}
	}

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );

	if(gClientLogic().localPlayer_)
	{
		if(!gClientLogic().localPlayer_->bDead && !hudMain->isChatVisible() && !gClientLogic().localPlayer_->uavViewActive_ && !gClientLogic().localPlayer_->laserViewActive_)	
		{
			if(InputMappingMngr->isPressed(r3dInputMappingMngr::KS_COMMROSE))
			{
				if(!m_bCommRoseVisible)
					Mouse->MoveTo((int)r3dRenderer->ScreenW2, (int)r3dRenderer->ScreenH2);
				m_bCommRoseVisible = true;
				r3dMouse::Show(true);
				gfxCommRose.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
			}
			else
			{
				m_bCommRoseVisible = false;
			}

			if(m_bVisible) // check for messages
			{
				if(Keyboard->WasPressed(kbs1))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+0);
				else if(Keyboard->WasPressed(kbs2))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+1);
				else if(Keyboard->WasPressed(kbs3))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+2);
				else if(Keyboard->WasPressed(kbs4))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+3);
				else if(Keyboard->WasPressed(kbs5))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+4);
				else if(Keyboard->WasPressed(kbs6))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+5);
				else if(Keyboard->WasPressed(kbs7))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+6);
				else if(Keyboard->WasPressed(kbs8))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+7);
				else if(Keyboard->WasPressed(kbs9))
					PlayMessage(gClientLogic().localPlayer_, m_CurrentMsgType*10+8);
				else if(Keyboard->WasPressed(kbsEsc))
					HideMessages();
			}
			if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_COMMAND_Z))
				ShowMessages(0);
			else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_COMMAND_X))
				ShowMessages(1);
			else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_COMMAND_C))
				ShowMessages(2);
		}
		else
		{
			m_bCommRoseVisible = false;
			if(m_bVisible)
				HideMessages();
		}
	}
}

extern int RUS_CLIENT;
void HUDCommCalls::Draw()
{
	if(m_bVisible)
		gfxMovie.UpdateAndDraw();

	if(m_bCommRoseVisible)
		gfxCommRose.UpdateAndDraw();
}

void HUDCommCalls::eventPressButton(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==1);
	int btnID = (int)args[0].GetNumber();

	if(gClientLogic().localPlayer_ == NULL)
		return;

	if(btnID == 5 || btnID==6 || btnID==7)
	{
		const float MAX_CASTING_DISTANCE = 20000.f;
		r3dPoint3D dir;
		if(g_camera_mode->GetInt() == 1)
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);
		else
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);

		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			r3dVector pos(hit.impact.x, hit.impact.y, hit.impact.z);
			showHUDIcon(gClientLogic().localPlayer_, btnID, pos);

			// send msg to server
			PKT_C2C_CommRoseCommand_s n;
			n.id = btnID;
			n.pos = pos;
			p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
		}
	}
	else
	{
		showHUDIcon(gClientLogic().localPlayer_, btnID, r3dVector(0,0,0));
		
		// send msg to server
		PKT_C2C_CommRoseCommand_s n;
		n.id = btnID;
		n.pos = r3dVector(0,0,0);
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}
}

void HUDCommCalls::showHUDIcon(class obj_AI_Player* from, int id, const r3dVector& pos)
{
	if(id==5) // attack
	{
		snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/all units attack"), r3dPoint3D(0,0,0));
		hudMain->addHUDIcon(HUDDisplay::HUDIcon_Attack, 20.0f, pos);
	}
	else if(id == 6)
	{
		snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/keep defending"), r3dPoint3D(0,0,0));
		hudMain->addHUDIcon(HUDDisplay::HUDIcon_Defend, 20.0f, pos);
	}
	else if(id == 7)
	{
		snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/enemy has been spotted"), r3dPoint3D(0,0,0));
		hudMain->addHUDIcon(HUDDisplay::HUDIcon_Spotted, 10.0f, pos);
	}
	else if(id >= 1 && id <= 4)
	{
		switch(id)
		{
		case 1:
			break;
		case 2:
			snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need medic"), r3dPoint3D(0,0,0));
			break;
		case 3:
			snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need ammo"), r3dPoint3D(0,0,0));
			break;
		case 4:
			snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need help"), r3dPoint3D(0,0,0));
			break;
		}

		if(from == gClientLogic().localPlayer_) // do not show icon above our own head
			return;

		if(from->m_commRoseTimer > 0 && !from->m_CommRoseIcon.IsUndefined())
			hudMain->deleteScreenIcon(from->m_CommRoseIcon);
		from->m_commRoseTimer = 5.0f;
		switch(id)
		{
		case 1:
			hudMain->addNeedOrdersIcon(from->m_CommRoseIcon);
			break;
		case 2:
			hudMain->addNeedMedicIcon(from->m_CommRoseIcon);
			break;
		case 3:
			hudMain->addNeedAmmoIcon(from->m_CommRoseIcon);
			break;
		case 4:
			hudMain->addNeedHelpIcon(from->m_CommRoseIcon);
			break;
		}
		hudMain->moveScreenIcon(from->m_CommRoseIcon, pos, false, true); // hide until player update
	}
}

extern char* s_ChatIgnoreList[256];
extern int s_CharIgnoreListNum;

void HUDCommCalls::PlayMessage(obj_AI_Player* from, int id)
{
	if(!from)
		return;

	// anti spam check
	if(from == gClientLogic().localPlayer_)
	{
		static float lastTimeMessage = 0;
		if(r3dGetTime() - lastTimeMessage < 2.0f)
		{
			return; // anti spam
		}
		lastTimeMessage = r3dGetTime();
	}
	else
	{
		// check ignore list
		char plrFromUserName[64];
		from->GetUserName(plrFromUserName);
		for(int i=0; i<s_CharIgnoreListNum; ++i)
		{
			if(stricmp(s_ChatIgnoreList[i], plrFromUserName)==0)
				return; // ignore msg
		}
		from->m_pingIconTimer = 1.0f;
	}

	if(id>=0 && id<9)
	{
		int i = id;
		if(m_CommandsZ[i].id != -1)
		{
			hudMain->AddChatMessage(1, from, m_CommandsZ[i].txtCommand);
			if(g_enable_voice_commands->GetBool())
				snd_PlaySound(m_CommandsZ[i].voiceCmd, r3dPoint3D(0,0,0));
		}
	}
	else if(id>=10 && id < 19)
	{
		int i = id-10;
		if(m_CommandsX[i].id != -1)
		{
			hudMain->AddChatMessage(1, from, m_CommandsX[i].txtCommand);
			if(g_enable_voice_commands->GetBool())
				snd_PlaySound(m_CommandsX[i].voiceCmd, r3dPoint3D(0,0,0));
		}
	}
	else if(id>=20 && id<29)
	{
		int i = id-20;
		if(m_CommandsC[i].id != -1)
		{
			hudMain->AddChatMessage(1, from, m_CommandsC[i].txtCommand);
			if(g_enable_voice_commands->GetBool())
				snd_PlaySound(m_CommandsC[i].voiceCmd, r3dPoint3D(0,0,0));
		}
	}

	if(from == gClientLogic().localPlayer_)
	{
		HideMessages();
		// send msg to server
		PKT_C2C_VoiceCommand_s n;
		n.id = id;
		p2pSendToHost(from, &n, sizeof(n));
	}
}

void HUDCommCalls::ShowMessages(int msgType)
{
	if(m_bVisible && m_CurrentMsgType == msgType)
	{
		HideMessages();
		return;
	}
	m_CurrentMsgType = msgType;

	CallCommand* commandsArray = 0;
	if(msgType == 0)
		commandsArray = &m_CommandsZ[0];
	else if(msgType == 1)
		commandsArray = &m_CommandsX[0];
	else 
		commandsArray = &m_CommandsC[0];
	for(int i=0; i<9; ++i)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetNumber(i+1);
		if(commandsArray[i].id!=-1)
		{
			var[1].SetNumber(i+1);
			var[2].SetStringW(commandsArray[i].txtCommand);
			gfxMovie.Invoke("_global.setCommandText", var, 3);
		}
		else
		{
			var[1].SetNumber(-1);
			var[2].SetString("");
			gfxMovie.Invoke("_global.setCommandText", var, 3);
		}
	}

	if(!m_bVisible)
		gfxMovie.Invoke("_root.Block01.gotoAndPlay", "in");
	m_bVisible = true;
}

void HUDCommCalls::HideMessages()
{
	m_bVisible = false;
	gfxMovie.Invoke("_root.Block01.gotoAndPlay", "out");
}