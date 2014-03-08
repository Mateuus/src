#include "r3dPCH.h"
#include "r3dProtect.h"

#include "HUDDisplay.h"

#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/Gameplay/obj_SiegeObjective.h"
#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/ai/AI_Player.H"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"

#include "HUDPause.h"
#include "HUDScoreboard.h"

const char* greyColor = "868686";
const char* orangeColor = "ff9e00";
const char* blueColor = "426886";
const char* redColor = "a52d26";

extern HUDScoreboard*	hudScore;
extern HUDPause*	hudPause;

static const float gTimeToShowAchievements = 2.0f;

struct NameHashFunc_T
{
	inline int operator () ( const char * szKey )
	{
		return r3dHash::MakeHash( szKey );// & ( LENGTH-1 );
	}
};
static HashTableDynamic<const char*, FixedString256, NameHashFunc_T, 1024> dictionaryHash_;

HUDDisplay :: HUDDisplay()
{
//	captVisible = 0;
//	captValue   = 0;
	Inited = false;
	ChatWindowVisible = false;
	TimerFlashVisible = false;
	m_PrevBloodLevel = 0;
	m_PrevEnergyValue = -1;
	m_PrevBreathValue = -1;
	lastAchievementTime = 0;
	
	for( int achievementIndex =0; achievementIndex < MAX_HUD_ACHIEVEMENT_QUEUE; achievementIndex++ )
	{
		m_AchievementQueue[achievementIndex] = 0;
	}
	m_NumAchievementsInQueue = 0;
	m_CurrentAchievement = 0;

	if(dictionaryHash_.Size() == 0)
	{
		r3dFile* f = r3d_open( "Data/LangPack/dictionary.txt", "rb" );
		char tmpStr[256];
		while(fgets(tmpStr, 256, f) != NULL)
		{
			size_t len = strlen(tmpStr);
			for(size_t i=0; i<len; ++i)
			{
				if(tmpStr[i]==13 || tmpStr[i]==10)
					tmpStr[i]=0;
			}
			dictionaryHash_.Add(tmpStr, tmpStr);	
		}
		fclose(f);
	}
}

HUDDisplay :: ~HUDDisplay()
{
	dictionaryHash_.Clear();
}

// this will leak on app exit
char* s_ChatIgnoreList[256] = {0};
int s_CharIgnoreListNum = 0;
char s_ChatLastWhisperFrom[64] = {0};
int s_prevChatMsgType = CHAT_MSGTYPE_TEAM;
char s_chatMsg[2048];

static void fsChatMessage(void* data, r3dScaleformMovie *pMovie, const char *arg)
{
	// really simple profanity filter
	{
		int counter = 0;
		char profanityFilter[2048];
		r3dscpy(profanityFilter, arg);
		char* word = strtok(profanityFilter, " ");
		while(word)
		{
			if(dictionaryHash_.IsExists(word))
			{
				r3dscpy(&s_chatMsg[counter], "*** ");
				counter +=4;
			}
			else
			{
				r3dscpy(&s_chatMsg[counter], word);
				counter +=strlen(word);
				s_chatMsg[counter++] = ' ';
			}
			word = strtok(NULL, " ");
		}
		s_chatMsg[counter++] = 0;
	}

	bool has_anything = false;

	size_t start_text=0;
	size_t argLen = strlen(s_chatMsg);

	HUDDisplay* This = (HUDDisplay*)data;
	bool require_local = true;
	bool team_msg = true;
	PKT_C2C_ChatMessage_s n;
	// check for chat commands
	if(s_chatMsg[start_text] == '/')
	{
		char* tokenizable = new char[argLen+1];
		r3dscpy(tokenizable, &s_chatMsg[start_text]);
		char* token = strtok(tokenizable, " ");
		if(token != NULL)
		{
			team_msg = false;
			if(stricmp(token, "/help")==0)
			{
				require_local = false;
				This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatHelp"));
			}
			else if(stricmp(token, "/g")==0 || stricmp(token, "/s")==0) // s and g are both global chat
			{
				s_prevChatMsgType = n.msgType = CHAT_MSGTYPE_GENERAL;
				start_text+=3;
				r3dscpy(n.msg, &s_chatMsg[start_text]);
				n.target = 0;
				if(n.msg[0]!='\0')
					p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
			}
			else if(stricmp(token, "/t")==0)
			{
				s_prevChatMsgType = n.msgType = CHAT_MSGTYPE_TEAM;
				start_text+=3;
				r3dscpy(n.msg, &s_chatMsg[start_text]);
				n.target = 0;
				if(n.msg[0]!='\0')
					p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
			}
			else if(stricmp(token, "/c")==0)
			{
				if(gUserProfile.ProfileData.ClanID == -1)
				{
					require_local = false;
					This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNotInClan"));
				}
				else
				{
					s_prevChatMsgType= n.msgType = CHAT_MSGTYPE_CLAN;
					start_text+=3;
					r3dscpy(n.msg, &s_chatMsg[start_text]);
					n.target = 0;
					if(n.msg[0]!='\0')
						p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
				}
			}
			else if(stricmp(token, "/w")==0)
			{
				require_local = true;
				token = strtok(NULL, " ");
				if(token!=0)
				{
					bool found_user = false;
					// try to find a user with that name
					for(int i=0; i<ClientGameLogic::MAX_NUM_PLAYERS; ++i)
					{
						if(gClientLogic().GetPlayer(i))
						{
                            char plrUserName[64];
                            gClientLogic().GetPlayer(i)->GetUserName(plrUserName);
							if(stricmp(plrUserName, token)==0)
							{
								found_user = true;
								if(!gClientLogic().GetPlayer(i)->NetworkLocal)
								{
									n.msgType = CHAT_MSGTYPE_WHISPER;
									start_text+=3+strlen(token);
									r3dscpy(n.msg, &s_chatMsg[start_text]);
									n.target = toP2pNetId(gClientLogic().GetPlayer(i)->GetSafeNetworkID());
									if(n.msg[0]!='\0')
										p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
								}
								else
								{
									require_local = false;
									This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatWhisperToYourself"));
								}
								break;
							}
						}
					}
					if(!found_user)
					{
						require_local = false;
						This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoSuchUser"));
					}
				}
				else
				{
					require_local = false;
					This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatWhisperFormat"));
				}
			}
			else if(stricmp(token, "/r")==0)
			{
				require_local = true;
				if(s_ChatLastWhisperFrom[0] == 0)
				{
					require_local = false;
					This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoOneToReply"));	
				}
				else
				{
					bool found_user = false;
					// try to find a user with that name
					for(int i=0; i<ClientGameLogic::MAX_NUM_PLAYERS; ++i)
					{
						if(gClientLogic().GetPlayer(i))
						{
                            char plrUserName[64];
                            gClientLogic().GetPlayer(i)->GetUserName(plrUserName);
							if(stricmp(plrUserName, s_ChatLastWhisperFrom)==0)
							{
								found_user = true;
								if(!gClientLogic().GetPlayer(i)->NetworkLocal)
								{
									n.msgType = CHAT_MSGTYPE_WHISPER;
									start_text+=3;
									r3dscpy(n.msg, &s_chatMsg[start_text]);
									n.target = toP2pNetId(gClientLogic().GetPlayer(i)->GetSafeNetworkID());
									if(n.msg[0]!='\0')
										p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
								}
								else
								{
									require_local = false;
									This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatWhisperToYourself"));
								}
								break;
							}
						}
					}
					if(!found_user)
					{
						require_local = false;
						This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoSuchUser"));
					}
				}
			}	
			else if(stricmp(token, "/ignore")==0)
			{
				require_local = false;
				token = strtok(NULL, "\n");
				if(token!=0)
				{
					bool found_user = false;
					// try to find a user with that name
					for(int i=0; i<ClientGameLogic::MAX_NUM_PLAYERS; ++i)
					{
						if(gClientLogic().GetPlayer(i))
						{
                            char plrUserName[64];
                            gClientLogic().GetPlayer(i)->GetUserName(plrUserName);
							if(stricmp(plrUserName, token)==0)
							{
								found_user = true;
								if(!gClientLogic().GetPlayer(i)->NetworkLocal)
								{
									s_ChatIgnoreList[s_CharIgnoreListNum] = new char[strlen(token)+1];
									r3dscpy(s_ChatIgnoreList[s_CharIgnoreListNum], token);
									s_CharIgnoreListNum++;
								}
								else
								{
									This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatIgnoreYourself"));
								}
								break;
							}
						}
					}
					if(!found_user)
					{
						require_local = false;
						This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoSuchUser"));
					}
				}
				else
				{
					require_local = false;
					This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatIgnoreFormat"));
				}
			}
			else if(stricmp(token, "/unignore")==0)
			{
				require_local = false;
				token = strtok(NULL, "\n");
				if(token!=0)
				{
					bool found_user = false;
					// try to find a user with that name
					for(int i=0; i<s_CharIgnoreListNum; ++i)
					{
						if(stricmp(s_ChatIgnoreList[i], token)==0)
						{
							found_user = true;
							delete [] s_ChatIgnoreList[i]; s_ChatIgnoreList[i] = 0;
							s_ChatIgnoreList[i] = s_ChatIgnoreList[s_CharIgnoreListNum-1];
							s_CharIgnoreListNum--;
						}
						break;
						}
					if(!found_user)
					{
						require_local = false;
						This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoSuchUserInIgnoreList"));
					}
				}
				else
				{
					require_local = false;
					This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatUnignoreFormat"));
				}
			}
			else if(stricmp(token, "/kick")==0)
			{
				require_local = false;
				token = strtok(NULL, "\n");
				if(token!=0)
				{
					bool found_user = false;
					// try to find a user with that name
					for(int i=0; i<ClientGameLogic::MAX_NUM_PLAYERS; ++i)
					{
						if(gClientLogic().GetPlayer(i))
						{
                            char plrUserName[64];
                            gClientLogic().GetPlayer(i)->GetUserName(plrUserName);
							if(stricmp(plrUserName, token)==0)
							{
								found_user = true;
								if(!gClientLogic().GetPlayer(i)->NetworkLocal)
								{
									// kick
									PKT_C2S_Admin_PlayerKick_s pkt;
									pkt.netID = toP2pNetId(gClientLogic().GetPlayer(i)->NetworkID);
									p2pSendToHost(NULL, &pkt, sizeof(pkt));
								}
								break;
							}
						}
					}
					if(!found_user)
					{
						This->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatNoSuchUser"));
					}
				}
			}	
			else if(stricmp(token, "/suicide")==0)
			{
				if( gClientLogic().localPlayer_ != NULL && gClientLogic().localPlayer_->bDead == false )
				{
					// regular team messag for death.
					n.msgType = CHAT_MSGTYPE_SUICIDE;
					r3dscpy(n.msg, "");
					p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
					require_local = false;
				}
			} 
			else 
			{
				team_msg = true; // if the token is invalid still send the message.
			}
		}
		delete [] tokenizable;
	}
	
	if(team_msg)
	{
		// regular team message
		n.msgType = s_prevChatMsgType;
		r3dscpy(n.msg, &s_chatMsg[start_text]);
		n.target = 0;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}

	if(require_local)
	{
		This->AddChatMessage(n.msgType, gClientLogic().localPlayer_, &s_chatMsg[start_text]);
	}

	memset(s_chatMsg, 0, sizeof(s_chatMsg));
}

void HUDDisplay::ShowChatWindow(bool set)
{
	if(ChatWindowVisible == set) 
		return;

	ChatWindowVisible = set;
	if(ChatWindowVisible)
	{
		gfxHUD.Invoke("_global.showChatInputWindow", "");
		gfxHUD.SetKeyboardCapture(); // just to make sure
	}
}

void HUDDisplay::ForceHideChatWindow()
{
	ChatWindowVisible = false;
	gfxHUD.Invoke("_global.hideChatInputWindow", "");
}

void HUDDisplay::AddChatMessage(BYTE msgType, obj_AI_Player* from, const char* msg)
{
	if(!Inited)
		return;
	if(gClientLogic().localPlayer_==NULL) // no chat until you entered game
		return;

	wchar_t temp[2048];
	r3dscpy(temp, utf8ToWide(msg));
	AddChatMessage(msgType, from, temp);
}

void HUDDisplay::AddChatMessage(BYTE msgType, obj_AI_Player* from, const wchar_t* msg)
{
	if(!Inited)
		return;
	if(gClientLogic().localPlayer_==NULL) // no chat until you entered game
		return;

	if(msg[0] == '\0')
		return;

	// check ignore list
    char plrFromUserName[64];
	if(from)
	{
        from->GetUserName(plrFromUserName);
		for(int i=0; i<s_CharIgnoreListNum; ++i)
		{
			if(stricmp(s_ChatIgnoreList[i], plrFromUserName)==0)
				return; // ignore msg
		}
		if(msgType == CHAT_MSGTYPE_WHISPER)
			r3dscpy(s_ChatLastWhisperFrom, plrFromUserName);
	}

	Scaleform::GFx::Value var[4];
	const wchar_t* tempPlayerName = NULL;
	if(from)
	{
		wchar_t* plrName = utf8ToWide(plrFromUserName);
		tempPlayerName = plrName;
	}
	else
		tempPlayerName = gLangMngr.getString("$HUD_Msg_System");

	var[0].SetStringW(tempPlayerName);
	if(from)
	{
		if(from->TeamID == gClientLogic().localPlayer_->TeamID)
			var[1].SetString("blue");
		else
			var[1].SetString("red");
	}
	else
		var[1].SetString("white");

	var[2].SetStringW(msg);
	if(msgType == CHAT_MSGTYPE_GENERAL)
		var[3].SetString("white");
	else if(msgType == CHAT_MSGTYPE_TEAM)
		var[3].SetString("green");
	else if(msgType == CHAT_MSGTYPE_WHISPER)
		var[3].SetString("purple");
	else if(msgType == CHAT_MSGTYPE_CLAN)
		var[3].SetString("orange");

	gfxHUD.Invoke("_global.AddChatMessage", var, 4);
}

void HUDDisplay::eventRegisterHUD(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount ==1);
	m_SF_HUD = args[0];

	m_SF_HUD.GetMember("Ping", &m_SF_HUD_Ping);
	m_SF_HUD_Ping.GetMember("Text", &m_SF_HUD_PingText);

	m_SF_HUD.GetMember("reticles", &m_SF_HUD_Reticles);

	m_SF_HUD.GetMember("YouHaveBomb", &m_SF_HUD_YouHaveBomb);
	m_YouHaveBombSet = false;
}

bool HUDDisplay::Init(int tickets, bool inShootingRange)
{
	if(!gfxHUD.Load("Data\\Menu\\HUD_main.swf", true)) 
		return false;

	if(!gfxHitEffect.Load("Data\\Menu\\HUD_BloodStreak.swf", false)) 
		return false;

	// set rank data
	{
		char tempB[64];
		Scaleform::GFx::Value var[2];
		for(int i=0; i<MAX_NUM_RANKS; ++i)
		{
			sprintf(tempB, "PlayerRank%d", i);
			var[0].SetStringW(gLangMngr.getString(tempB));
			if(i==0)
				var[1].SetNumber(0);
			else
				var[1].SetNumber(g_RanksPoints[i-1]);
			gfxHUD.Invoke("_global.RanksData.addRank", var, 2);
		}
	}


	gfxHUD.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
	gfxHitEffect.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDDisplay>(this, &HUDDisplay::FUNC)
	gfxHUD.RegisterEventHandler("eventRegisterHUD", MAKE_CALLBACK(eventRegisterHUD));
	gfxHUD.RegisterEventHandler("callbackEnteredChatMessage", this, &fsChatMessage);

	Scaleform::GFx::Value vars[2];
	vars[0].SetNumber(r3dRenderer->ScreenW);
	vars[1].SetNumber(r3dRenderer->ScreenH);
	r3dSetCurrentActiveMovie(&gfxHUD); // to make callback work
	gfxHUD.Invoke("_global.arrangeHUD", vars, 2);
	r3dSetCurrentActiveMovie(NULL);
	updateReticlePosition();

	// init tickets
	tickets_[0] = tickets;
	tickets_[1] = tickets;
	tickets_[2] = -1;
	tickets_[3] = -1;

	ReticleVisible = true;
	inDeadMode = false;
	bombTimerVisible = false;
	s_prevChatMsgType = CHAT_MSGTYPE_TEAM; // team by default

	if(!inShootingRange)
	{
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
		{
			gfxHUD.Invoke("_global.SetFlagIndicator", "conquest");
			int tmp1, tmp2;
			int numCapturableCP = gCPMgr.GetControlledPoints(tmp1, tmp2, 0);
			gfxHUD.Invoke("_global.CreateFlagIndicators", numCapturableCP);
		}
		else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		{
			gfxHUD.Invoke("_global.SetFlagIndicator", "dm");
		}
		else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
		{
			gfxHUD.Invoke("_global.SetFlagIndicator", "siege");
			gfxHUD.Invoke("_global.CreateBombIndicators", gSiegeObjMgr.NumObjectives());
		}
	}

	gfxHUD.SetVariable("_root.HUD.ClearTextBox._alpha", 90);

	cpoints_[0] = -1;
	cpoints_[1] = -1;

	Inited = true;

	setMinimapBombTimer("", false);

	m_PrevBloodLevel = 0;
	m_PrevEnergyValue = -1;
	m_PrevBreathValue = -1;

	gfxHUD.Invoke("_global.flashMainTimerOff", "");
	TimerFlashVisible = false;

	showTimeMessageDelay = 0;

	ShowWaitingForPlayersMsg = false;

	if(inShootingRange)
	{
		Scaleform::GFx::Value tmp;
		Scaleform::GFx::Value::DisplayInfo dinfo;
		dinfo.SetVisible(true);
		m_SF_HUD.GetMember("ShootingRangeText", &tmp);
		tmp.SetDisplayInfo(dinfo);

		// disable some crap
		dinfo.SetVisible(false);
		m_SF_HUD.GetMember("timerMain", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("BottomLeft", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("Ping", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("Chat", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("ScoreCounterSiege", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("ScoreCounterDM", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("ScoreCounterConquest", &tmp);
		tmp.SetDisplayInfo(dinfo);
		m_SF_HUD.GetMember("ItemToolbarVertical", &tmp);
		tmp.SetDisplayInfo(dinfo);
	}
	return true;
}

bool HUDDisplay::Unload()
{
	for(int i=0; i<32; ++i)
	{
		if(m_HUDIcons[i].enabled)
		{
			m_HUDIcons[i].enabled = false;
			deleteScreenIcon(m_HUDIcons[i].icon);
		}
	}

	hitEffectList.clear();

	m_SF_HUD.SetUndefined();
	m_SF_HUD_Ping.SetUndefined();
	m_SF_HUD_PingText.SetUndefined();
	m_SF_HUD_Reticles.SetUndefined();
	m_SF_HUD_YouHaveBomb.SetUndefined();

	m_SF_HUD_ItemSlots[0].SetUndefined();
	m_SF_HUD_ItemSlots[1].SetUndefined();
	m_SF_HUD_ItemSlots[2].SetUndefined();
	m_SF_HUD_ItemSlots[3].SetUndefined();

	gfxHUD.Unload();
	gfxHitEffect.Unload();

	Inited = false;
	return true;
}

void HUDDisplay::updateReticlePosition()
{
	// true - center, false - offcenter
	gfxHUD.Invoke("_global.setReticlePosition", g_camera_mode->GetInt()!=1);
}

int HUDDisplay::Update()
{
	if(!Inited)
		return 1;
		
	const ClientGameLogic& CGL = gClientLogic();
	// update controlled points
	if(CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
	{
		if(CGL.localPlayer_)
		{
			int ally=0, enemy=0;
			gCPMgr.GetControlledPoints(ally, enemy, CGL.localPlayer_->TeamID);
			if(cpoints_[0] != ally || cpoints_[1] != enemy)
			{
				Scaleform::GFx::Value args[2];
				cpoints_[0] = ally;
				cpoints_[1] = enemy;
				args[0].SetNumber(ally);
				args[1].SetNumber(enemy);
				gfxHUD.Invoke("_global.SetFlagIndicators", args, 2);
			}
		}
	}

	{
		float timeLeft = CGL.gameTimeEnd_ - r3dGetTime();
		if(!CGL.m_gameHasStarted)
		{
			timeLeft = 0; // show timer as 0 until game has started
			showTimeMessageDelay = 0;
		}

		if(timeLeft < 120 && !TimerFlashVisible && CGL.m_gameHasStarted && CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb) // less than two minutes
		{
			gfxHUD.Invoke("_global.flashMainTimerOn", "");
			TimerFlashVisible = true;
			showTimeMessageDelay = 0;
		}

		if(TimerFlashVisible && timeLeft < 60.0f)
		{
			if(showTimeMessageDelay <= 0.1f)
			{
				showTimeMessageDelay = 10.0f; // each 10 seconds
				wchar_t tempStr[256];
				swprintf(tempStr, 256, gLangMngr.getString("$HUD_Msg_SecondsLeft"), (int)timeLeft);
				ShowAchievementCustom(tempStr, "", "$Data/Menu/achievements/hud/on_my_pos.png", "");
			}
			showTimeMessageDelay -= r3dGetFrameTime();
		}

		int hours = int(timeLeft/3600.0f);
		timeLeft -= hours*3600;
		int minutes = int(timeLeft/60.0f);
		timeLeft -= minutes*60;
		int seconds = int(timeLeft);
		char tempStr[32];
		sprintf(tempStr, "%02d:%02d:%02d", hours, minutes, seconds);
		gfxHUD.Invoke("_global.setMainTimer", tempStr);
	}

	for(std::list<hitEffectTracker>::iterator it = hitEffectList.begin(); it!=hitEffectList.end(); )
	{
		hitEffectTracker& tracker = *it;
		if(tracker.hitTimeEnd >= r3dGetTime())
		{
			it = hitEffectList.erase(it);
			continue;
		}
		GameObject* from = GameWorld().GetObject((gobjid_t)tracker.fromID);
		if(from==NULL)
		{
			it = hitEffectList.erase(it);
			continue;
		}
		float angle = getHitEffectAngle(from);
		Scaleform::GFx::Value::DisplayInfo dinfo;
		dinfo.SetRotation(angle);
		tracker.hitEffect.SetDisplayInfo(dinfo);

		++it;
	}

	for(int i=0; i<32; ++i)
	{
		if(m_HUDIcons[i].enabled)
		{
			if((r3dGetTime()-m_HUDIcons[i].spawnTime)>m_HUDIcons[i].lifetime)
			{
				m_HUDIcons[i].enabled = false;
				deleteScreenIcon(m_HUDIcons[i].icon);
			}
			else
			{
				moveScreenIcon(m_HUDIcons[i].icon, m_HUDIcons[i].pos, true);
			}
		}
	}

	if(CGL.localPlayer_ && CGL.m_gameInfo.autoBalance)
	{
		if(!CGL.m_gameHasStarted && !ShowWaitingForPlayersMsg)
		{
			ShowWaitingForPlayersMsg = true;
			wchar_t tempBuf[64];
			swprintf(tempBuf, 64, gLangMngr.getString("$HUD_Msg_WaitingForPlayers"), CGL.m_gameInfo.minPlayers);
			Scaleform::GFx::Value var[2];
			var[0].SetBoolean(true);
			var[1].SetStringW(tempBuf);
			gfxHUD.Invoke("_global.setWaitingForPlayersMsg", var, 2);
		}
		else if(CGL.m_gameHasStarted && ShowWaitingForPlayersMsg)
		{
			ShowWaitingForPlayersMsg = false;
			Scaleform::GFx::Value var[2];
			var[0].SetBoolean(false);
			var[1].SetStringW(L"");
			gfxHUD.Invoke("_global.setWaitingForPlayersMsg", var, 2);
		}
	}

	ProcessAchievementQueue();

	return 1;
}


int HUDDisplay::Draw()
{
	if(!Inited)
		return 1;

	{
		R3DPROFILE_FUNCTION("gfxHitEffect.UpdateAndDraw");
		gfxHitEffect.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );
		gfxHitEffect.UpdateAndDraw();
	}

	{
		R3DPROFILE_FUNCTION("gfxHUD.UpdateAndDraw");
		float X, Y, W, H;
		r3dRenderer->GetBackBufferViewport( &X, &Y, &W, &H );
		float fY = r3dRenderer->ScreenH / H;
		float fX = r3dRenderer->ScreenW / W;
		r3dScaleformSetUserMatrix(fX, fY, 0, 0);
#ifndef FINAL_BUILD
		gfxHUD.UpdateAndDraw(d_disable_render_hud->GetBool());
#else
		gfxHUD.UpdateAndDraw();
#endif
		r3dScaleformSetUserMatrix();
	}


	return 1;
}

void HUDDisplay::SetBombState(int idx, const char* state)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetNumber(idx);
	var[1].SetString(state);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.SetBombIndicator"), var, 2);
}

void HUDDisplay::SetYouHaveBomb(bool state)
{
	if(!Inited)
		return;

	if(m_YouHaveBombSet && state)
		return;
	if(!m_YouHaveBombSet && !state)
		return;

	if(m_YouHaveBombSet && !state)
		m_SF_HUD_YouHaveBomb.GotoAndPlay("out");
	else
		m_SF_HUD_YouHaveBomb.GotoAndPlay("start");
	m_YouHaveBombSet = state;
}

float HUDDisplay::getHitEffectAngle(GameObject* from)
{
	obj_AI_Player* pl = gClientLogic().localPlayer_;
	if(!pl)
		return 0;

	r3dVector a = (from->GetPosition() - pl->GetPosition()).NormalizeTo();
	r3dVector b = pl->GetvForw();
	// 2d angle
	float angle = atan2(a.x*b.z - b.x*a.z, a.x*b.x + a.z*b.z);

	angle = R3D_RAD2DEG(angle);
	if(angle < 0) angle += 360;

	// 0 degrees in flash is hit from left.
	angle += 90;
	if(angle > 360) angle -= 360;

	return angle;
}

void HUDDisplay::AddHitEffect(GameObject* from)
{
	if(!Inited)
		return;

	float angle = getHitEffectAngle(from);
	Scaleform::GFx::Value args[2];
	args[0].SetNumber(angle);
	hitEffectTracker tracker;
	tracker.hitTimeEnd = r3dGetTime() + 1.0f; // effect lasts 1 second. hard coded in flash
	tracker.fromID = from->GetSafeID().get();
	gfxHitEffect.Invoke(VMPROTECT_DecryptStringA("_root.api.addHit"), &tracker.hitEffect, args, 1);
	hitEffectList.push_back(tracker);
}

void HUDDisplay::SetBloodLevel(float bloodLevel)
{
	if(!Inited)
		return;
	// bug in flash, it doesn't set blood level correctly from first call
//	if(m_PrevBloodLevel != bloodLevel)
	{
		m_PrevBloodLevel = bloodLevel;
		Scaleform::GFx::Value args[2];
		args[0].SetNumber(bloodLevel);
		gfxHitEffect.Invoke(VMPROTECT_DecryptStringA("_root.api.setBloodLevel"), args, 1);
	}
}

void HUDDisplay::SetTickets(int blue, int red)
{
	if(!Inited)
		return;

	if(blue == tickets_[2] && red == tickets_[3])
		return;

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch)
	{
		R3D_SWAP(blue, red); // in deathmatch swap tickets, as we need to show that the team the reaches 100 first should win.
	}

	bool blueChanged = blue != tickets_[2];
	bool redChanged = red != tickets_[3];

	tickets_[2] = blue;
	tickets_[3] = red;

	int k1 = (blue * 100 / tickets_[0]);
	int k2 = (red  * 100 / tickets_[1]);
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch)
	{
		k1 = 100 - k1;
		k2 = 100 - k2;
		blue = 100 - blue;
		red = 100 - red;
	}

	char buf1[128];
	char buf2[128];
	sprintf(buf1, "%d", blue);
	sprintf(buf2, "%d", red);

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
	{
		gfxHUD.SetVariable("_root.HUD.ScoreCounterConquest.BlueScore.Bar._xscale", k1);
		gfxHUD.SetVariable("_root.HUD.ScoreCounterConquest.BlueScore.Digits.Text.text", buf1);
		gfxHUD.SetVariable("_root.HUD.ScoreCounterConquest.RedScore.Bar._xscale", k2);
		gfxHUD.SetVariable("_root.HUD.ScoreCounterConquest.RedScore.Digits.Text.text", buf2);
		if(blueChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterConquest.BlueScore.gotoAndPlay", "start");
		if(redChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterConquest.RedScore.gotoAndPlay", "start");
	}
	else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch)
	{
		gfxHUD.SetVariable("_root.HUD.ScoreCounterDM.BlueScore.Digits.Text.text", buf1);
		gfxHUD.SetVariable("_root.HUD.ScoreCounterDM.RedScore.Digits.Text.text", buf2);
		if(blueChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterDM.BlueScore.gotoAndPlay", "start");
		if(redChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterDM.RedScore.gotoAndPlay", "start");
	}
	else if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		gfxHUD.SetVariable("_root.HUD.ScoreCounterDM.BlueScore.Digits.Text.text", buf1);
		gfxHUD.SetVariable("_root.HUD.ScoreCounterDM.RedScore.Digits.Text.text", buf2);
		if(blueChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterDM.BlueScore.gotoAndPlay", "start");
		if(redChanged)
			gfxHUD.Invoke("_root.HUD.ScoreCounterDM.RedScore.gotoAndPlay", "start");
	}
}

void HUDDisplay::ShowAchievement(int achievementID )
{
	if(!Inited)
		return;

	const AchievementConfig* ach = gWeaponArmory.getAchievementByID( achievementID );
	r3d_assert(ach);
	Scaleform::GFx::Value var[4];
	var[0].SetString(ach->hud_icon);
	var[1].SetStringW(gLangMngr.getString(ach->name));
	var[2].SetStringW(gLangMngr.getString(ach->desc));
	var[3].SetString(""); // don't use this. 

	lastAchievementTime = r3dGetTime();
	m_CurrentAchievement = achievementID;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showAchievementRibbon"), var, 4);
}

void HUDDisplay::ShowAchievementCustom(const wchar_t* name, const char* desc, const char* icon, const char* reward)
{
	if(!Inited)
		return;
	if(wcslen(name)<3)
		return;
	if( lastAchievementTime + gTimeToShowAchievements < r3dGetFrameTime() )
	{
		return;
	}

	Scaleform::GFx::Value var[4];
	var[0].SetString(icon);
	var[1].SetStringW(name);
	var[2].SetString(desc);
	var[3].SetString(reward);

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showAchievementRibbon"), var, 4);
}

void HUDDisplay::setHealth(int health)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value var[1];
	var[0].SetNumber(health);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setHealth"), var, 1);
}

void HUDDisplay::showLeavingBattlezone(bool show, bool showTimer)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(show);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showLeavingBattleZone"), var, 1);

	if(show)
	{
		gfxHUD.SetVariable("_root.HUD.LeavingBZ.Msg.Back._visible", showTimer);
		gfxHUD.SetVariable("_root.HUD.LeavingBZ.Msg.Bar._visible", showTimer);
	}
}

void HUDDisplay::setLeavingBattlezoneTimer(float progress)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value var[1];
	var[0].SetNumber(progress);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setLeavingBattleZoneBar"), var, 1);
}

void HUDDisplay::ShowScore(int xp, int gp, int gd, int totalGD, int totalXP)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value var[3];
	var[2].SetString(""); // description of reward. not user right now
	if(xp>0)
	{
		var[0].SetNumber(R3D_ABS(xp));
		var[1].SetString(xp>0?"+":"-");
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.activateXPDisplay"), var, 3);
	}

	if(gp>0)
	{
		var[0].SetNumber(R3D_ABS(gp));
		var[1].SetString(gp>0?"+":"-");
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.activateGPDisplay"), var, 3);
	}

	if(gd>0)
	{
		var[0].SetNumber(R3D_ABS(gd));
		var[1].SetString(gd>0?"+":"-");
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.activateGDDisplay"), var, 3);
	}

	if(xp>0 || gd>0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetNumber(totalGD);
		var[1].SetNumber(totalXP);
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showScorePlate"), var, 2);
	}
}

void HUDDisplay::ShowLootDropMsg(const wchar_t* lootName, const char* iconPath)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(lootName);
	var[1].SetString(iconPath);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showLootDropPickup"), var, 2);
}

void HUDDisplay::switchToDead(bool state, bool force_spectator)
{
	if(!Inited)
		return;
	if(inDeadMode==state && !force_spectator)
		return;
	inDeadMode = state;
	Scaleform::GFx::Value var[2];
	var[0].SetNumber((int)inDeadMode);
	var[1].SetNumber(((int)gClientLogic().m_isSpectator || force_spectator)?1:0);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.SwitchDeadMode"), var, 2);

	if(inDeadMode)
		showBombTimer(false);
}

void HUDDisplay::setPing(int ping)
{
	if(!Inited)
		return;

	static int prevPing = -1;
	if(prevPing != ping)
	{
		if(ping < 150)
			m_SF_HUD_Ping.GotoAndStop(1);
		else if(ping < 350)
			m_SF_HUD_Ping.GotoAndStop(2);
		else
			m_SF_HUD_Ping.GotoAndStop(3);
		char tmpStr[16];
		sprintf(tmpStr, "%d", ping);
		m_SF_HUD_PingText.SetText(tmpStr);
	}
	/*Scaleform::GFx::Value var[1];
	var[0].SetNumber(ping);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setPing"), var, 1);*/
}

void HUDDisplay::showYouGotCoolThing(const wchar_t* title, const char* icon)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetStringW(title);
//	var[1].SetStringW(item);
//	var[2].SetStringW(text);
	var[1].SetString(icon);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showYouGotCoolThing"), var, 2);
}

void HUDDisplay::setMinimapBombTimer(const char* time, bool isVisible)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetString(time);
	var[1].SetBoolean(isVisible);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setMinimapBombTimer"), var, 2);
}

void HUDDisplay::SetEnergyValue(float value)
{
	if(!Inited)
		return;
	if(m_PrevEnergyValue != value)
	{
		m_PrevEnergyValue = value;
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setEnergyBarValue"), value);
	}
}

void HUDDisplay::SetBreathValue(float value)
{
	if(!Inited)
		return;
	if(m_PrevBreathValue != value)
	{
		m_PrevBreathValue = value;
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setBreathBarValue"), value);
	}
}

void getWeaponParamForUI(const WeaponConfig* wc, int* damage, int* spread, int* firerate, int *recoil);
void HUDDisplay::ShowKillTag(obj_AI_Player* killer, STORE_CATEGORIES damageType)
{
	if(!Inited)
		return;
	r3d_assert(killer);

	Scaleform::GFx::Value var[32];
    char plrUserName[256];
    killer->GetUserNameAndClanTag(plrUserName);
    var[0].SetString(plrUserName);
	var[1].SetNumber(killer->GameStats.TotalScore);

	// find weapon that killer used to kill us by checking damage type
	int wpnIdx = -1;
	if(damageType == storecat_GRENADES) // we cannot properly select mine or grenade, so for now if killed via mine\greande show primary weapon of killer
		damageType = storecat_INVALID;
	for(int i=0; i<NUM_WEAPONS_ON_PLAYER; ++i)
	{
		if(killer->m_Weapons[i] && killer->m_Weapons[i]->getCategory() == damageType)
		{
			wpnIdx = i;
			break;
		}
	}
	if(wpnIdx == -1) // if not found, just use primary
		wpnIdx = 0;

	const Weapon* wpn = killer->m_Weapons[wpnIdx];
	r3d_assert(wpn);
	var[2].SetStringW(wpn->getConfig()->m_StoreNameW);
	const GearConfig* gc = gWeaponArmory.getGearConfig(killer->CurLoadout.BodyArmorID);
	var[3].SetStringW(gc?gc->m_StoreNameW:L"");
	var[4].SetString(wpn->getConfig()->m_StoreIcon);
	var[5].SetString(gc?gc->m_StoreIcon:"");

	var[6].SetString(gWeaponArmory.getIconByItemId(killer->CurLoadout.Item1, true));
	var[7].SetString(gWeaponArmory.getIconByItemId(killer->CurLoadout.Item2, true));
	var[8].SetString(gWeaponArmory.getIconByItemId(killer->CurLoadout.Item3, true));
	var[9].SetString(gWeaponArmory.getIconByItemId(killer->CurLoadout.Item4, true));

	// skills are disabled for now
	var[10].SetNumber(0);//killer->m_Skills[0]);
	var[11].SetNumber(0);//killer->m_Skills[1]);
	var[12].SetNumber(0);//killer->m_Skills[2]);
	var[13].SetNumber(0);//killer->m_Skills[3]);
	var[14].SetNumber(0);//killer->m_Skills[4]);
	var[15].SetNumber(0);//killer->m_Skills[5]);
	var[16].SetNumber(0);//killer->m_Skills[6]);
	var[17].SetNumber(0);//killer->m_Skills[7]);

	var[18].SetNumber(ceilf(killer->m_Health));
	var[19].SetNumber(killer->m_Health/killer->getMaxHealth());

	var[20].SetNumber(ceilf(wpn->getConfig()->m_AmmoDamage));
	int damage = 0;
	getWeaponParamForUI(wpn->getConfig(), &damage, NULL, NULL, NULL);
	var[21].SetNumber(float(damage)/100.0f);

	var[22].SetNumber(gc?ceilf(gc->m_damagePerc*100.0f):0.0f);
	var[23].SetNumber(gc?gc->m_damagePerc:0.0f);

	// TO BE REMOVED, OLD UPGRADE SYSTEM
	var[24].SetNumber(1);
	var[25].SetString("");
	var[26].SetNumber(1);
	var[27].SetString("");
	var[28].SetNumber(1);
	var[29].SetString("");
	var[30].SetNumber(1);
	var[31].SetString("");
	// END

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.displayKillTag"), var, 32);
}

void HUDDisplay::ShowKilledTag(obj_AI_Player* victim)
{
	if(!Inited)
		return;
	r3d_assert(victim);

	Scaleform::GFx::Value var[10];
	char plrUserName[256];
	victim->GetUserNameAndClanTag(plrUserName);
	var[0].SetString(plrUserName);
	var[1].SetNumber(victim->GameStats.TotalScore);

	const GearConfig* gc = gWeaponArmory.getGearConfig(victim->CurLoadout.BodyArmorID);
	var[2].SetStringW(gc?gc->m_StoreNameW:L"");
	var[3].SetString(gc?gc->m_StoreIcon:"");

	var[4].SetString(gWeaponArmory.getIconByItemId(victim->CurLoadout.Item1, true));
	var[5].SetString(gWeaponArmory.getIconByItemId(victim->CurLoadout.Item2, true));
	var[6].SetString(gWeaponArmory.getIconByItemId(victim->CurLoadout.Item3, true));
	var[7].SetString(gWeaponArmory.getIconByItemId(victim->CurLoadout.Item4, true));

	var[8].SetNumber(gc?ceilf(gc->m_damagePerc*100.0f):0.0f);
	var[9].SetNumber(gc?gc->m_damagePerc:0.0f);

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.displayKilledTag"), var, 10);
}

void HUDDisplay::AddKillMessage(obj_AI_Player* killer, obj_AI_Player* victim, STORE_CATEGORIES damageType)
{
	if(!Inited)
		return;

	// if HUD isn't visible right now, then skip messages
	if(hudPause->isActive() || hudScore->isActive())
		return;

	r3d_assert(victim);

	int localPlayerTeamID = 0;
	if(gClientLogic().localPlayer_)
		localPlayerTeamID = gClientLogic().localPlayer_->TeamID;

	Scaleform::GFx::Value vars[5];
    char killerUserName[64] = "";
    char victimUserName[64] = "";
    if(victim) victim->GetUserName(victimUserName);
    if(killer) killer->GetUserName(killerUserName);

    vars[0].SetString(killer?(killerUserName):"");
	const char* killerColor = killer?((killer->TeamID!=localPlayerTeamID)?redColor:blueColor):greyColor;
	if(killer && gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		killerColor = killer->TeamID==0?redColor:blueColor;
	vars[1].SetString(killerColor);
	vars[2].SetString(victimUserName);

	const char* victimColor = (victim->TeamID!=localPlayerTeamID)?redColor:blueColor;
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		victimColor = victim->TeamID==0?redColor:blueColor;
	vars[3].SetString(victimColor);
	const char* how_killed=VMPROTECT_DecryptStringA("suicide");
	{
		if(damageType==storecat_ASR)
			how_killed = VMPROTECT_DecryptStringA("rifle");
		else if(damageType==storecat_SHTG)
			how_killed = VMPROTECT_DecryptStringA("shotgun");
		else if(damageType==storecat_HG)
			how_killed = VMPROTECT_DecryptStringA("pistol");
		else if(damageType==storecat_SMG)
			how_killed = VMPROTECT_DecryptStringA("smg");
		else if(damageType == storecat_SUPPORT)
			how_killed = VMPROTECT_DecryptStringA("rpg");
		else if(damageType == storecat_GRENADES)
			how_killed = VMPROTECT_DecryptStringA("grenade");
		else if(damageType==storecat_SNP)
			how_killed = VMPROTECT_DecryptStringA("sniper");
		else if(damageType==storecat_MG)
			how_killed = VMPROTECT_DecryptStringA("mg");
		else if(damageType==storecat_MELEE)
			how_killed = VMPROTECT_DecryptStringA("knife");
		else if(damageType==storecat_UsableItem) // turret is the only usable item that kills right now
			how_killed = VMPROTECT_DecryptStringA("turret");
	}

	vars[4].SetString(how_killed);

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addKillMessage"), vars, 5);
}

void HUDDisplay::AddMessage(const wchar_t *msg)
{
	if(!Inited)
		return;
	if(gClientLogic().localPlayer_ == NULL)
		return;

	// if HUD isn't visible right now, then skip messages
	if(hudPause->isActive() || hudScore->isActive())
		return;

	Scaleform::GFx::Value vars[2];
	vars[0].SetStringW(msg);
	vars[1].SetString(orangeColor);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addGeneralMessage"), vars, 2);
}

void HUDDisplay::SetBullets(int num_left, int num_in_clip, int num_clips)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value args[4];
	args[0].SetNumber(num_left);
	args[1].SetNumber(num_in_clip);
	args[2].SetNumber(num_clips);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.SetBullets"), args, 3);
}

int convertFireModeIntoInt(WeaponFiremodeEnum firemode)
{
	if(firemode == WPN_FRM_SINGLE)
		return 1;
	else if(firemode == WPN_FRM_TRIPLE)
		return 3;
	else if(firemode == WPN_FRM_AUTO)
		return 5;

	return 1;
}

void HUDDisplay::AddNewWeapon(int index, const char* name, const char* icon, STORE_CATEGORIES weaponType, WeaponFiremodeEnum firemode)
{
	if(!Inited)
		return;

	if(name != NULL)
	{
		char* weapon_name = strdup(name);
		strupr(weapon_name); // per Max request, weapon name should be in capital letter

		Scaleform::GFx::Value var[5];
		var[0].SetNumber(index);
		var[1].SetString(weapon_name);
		var[2].SetString(icon);
		if(weaponType == storecat_ASR)
			var[3].SetString(VMPROTECT_DecryptStringA("assault_rifle"));
		else if(weaponType == storecat_SNP)
			var[3].SetString(VMPROTECT_DecryptStringA("assault_rifle"));
		else if(weaponType == storecat_SHTG)
			var[3].SetString(VMPROTECT_DecryptStringA("shotgun"));
		else if(weaponType == storecat_SUPPORT)
			var[3].SetString(VMPROTECT_DecryptStringA("rocket"));
		else if(weaponType == storecat_HG)
			var[3].SetString(VMPROTECT_DecryptStringA("pistol"));
		else if(weaponType == storecat_SMG)
			var[3].SetString(VMPROTECT_DecryptStringA("smg"));
		else if(weaponType == storecat_MG)
			var[3].SetString(VMPROTECT_DecryptStringA("machinegun"));
		else if(weaponType == storecat_GRENADES)
			var[3].SetString(VMPROTECT_DecryptStringA("grenade"));
		else if(weaponType == storecat_MELEE)
			var[3].SetString(VMPROTECT_DecryptStringA("pistol"));
		else
			var[3].SetString(VMPROTECT_DecryptStringA("none"));
		var[4].SetNumber(convertFireModeIntoInt(firemode));
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addNewWeapon"), var, 5);

		free(weapon_name);
	}
	if(index >= 3 && index <= 6)
	{
		//_global.setItemInfo = function(idx:Number, icon:String) : MovieClip
		Scaleform::GFx::Value var[2];
		var[0].SetNumber(index-2); // flash accept idx from 1
		var[1].SetString(icon);
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setItemInfo"), &m_SF_HUD_ItemSlots[index-3], var, 2);

		Scaleform::GFx::Value SlotText;
		m_SF_HUD_ItemSlots[index-3].GetMember("Slot", &SlotText);
		char tmpStr[8];
		sprintf(tmpStr, "%d", index+1);
		SlotText.SetText(tmpStr);
		SlotText.SetUndefined();

		if(name == NULL)
		{
			m_SF_HUD_ItemSlots[index-3].GotoAndStop("empty");
		}
		else
		{
			m_SF_HUD_ItemSlots[index-3].GotoAndStop("regular");
		}
	}
}

void HUDDisplay::SetItemInfo(int index, float cooldown, float maxcooldown, int quantity)
{
	Scaleform::GFx::Value AmmoText, AmmoLowText, TimeText, TimeShadowText, TimeTimeText, TimeTimeShadowText;
	m_SF_HUD_ItemSlots[index-3].GetMember("Ammo", &AmmoText);
	m_SF_HUD_ItemSlots[index-3].GetMember("AmmoLow", &AmmoLowText);
	m_SF_HUD_ItemSlots[index-3].GetMember("Time", &TimeText);
	m_SF_HUD_ItemSlots[index-3].GetMember("TimeShadow", &TimeShadowText);
	TimeText.GetMember("Time", &TimeTimeText);
	TimeShadowText.GetMember("Time", &TimeTimeShadowText);
	
	char tmpStr[8];
	sprintf(tmpStr, "%d", quantity);
	AmmoText.SetText(tmpStr);
	AmmoLowText.SetText(tmpStr);
	if(cooldown > 0)
	{
		int frame = R3D_CLAMP(100-int((cooldown/maxcooldown)*100)+1, 1, 99);
		m_SF_HUD_ItemSlots[index-3].GotoAndStop(frame);

		char tmpStr2[16];
		sprintf(tmpStr2, "00:%02d", (int)cooldown);
		TimeTimeText.SetText(tmpStr2);
		TimeTimeShadowText.SetText(tmpStr2);
	}
	else
		m_SF_HUD_ItemSlots[index-3].GotoAndStop("regular");

	{
		bool emptyAmmo = quantity == 0;

		Scaleform::GFx::Value tmp;
		Scaleform::GFx::Value::DisplayInfo dinfo;
		dinfo.SetVisible(emptyAmmo);
		m_SF_HUD_ItemSlots[index-3].GetMember("LowAmmoLight2", &tmp); tmp.SetDisplayInfo(dinfo);
		m_SF_HUD_ItemSlots[index-3].GetMember("LowAmmoLight", &tmp); tmp.SetDisplayInfo(dinfo);
		m_SF_HUD_ItemSlots[index-3].GetMember("PlateLowAmmo", &tmp); tmp.SetDisplayInfo(dinfo);
		m_SF_HUD_ItemSlots[index-3].GetMember("AmmoLow", &tmp); tmp.SetDisplayInfo(dinfo);
		dinfo.SetVisible(!emptyAmmo);
		m_SF_HUD_ItemSlots[index-3].GetMember("Backlight", &tmp); tmp.SetDisplayInfo(dinfo);
		m_SF_HUD_ItemSlots[index-3].GetMember("Ammo", &tmp); tmp.SetDisplayInfo(dinfo);
		m_SF_HUD_ItemSlots[index-3].GetMember("Plate", &tmp); tmp.SetDisplayInfo(dinfo);
	}
}

void HUDDisplay::setCurrentWeapon(int index)
{
	if(!Inited)
		return;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setCurrentWeapon"), index);
}

void HUDDisplay::removeAllWeapons()
{
	if(!Inited)
		return;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.removeAllWeapons"), "");
}

void HUDDisplay::setFireMode(WeaponFiremodeEnum firemode)
{
	if(!Inited)
		return;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setFireMode"), convertFireModeIntoInt(firemode));
}

void HUDDisplay::showUseItem(bool enable)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetBoolean(enable);
	wchar_t temp[256];
	swprintf(temp, 256, gLangMngr.getString("HUD_ShowUseItemText"), utf8ToWide(InputMappingMngr->getKeyName(r3dInputMappingMngr::KS_INTERACT)));
	var[1].SetStringW(temp);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showItemUseInfo"), var, 2);
}

void HUDDisplay::showReloading(bool set)
{
	if(!Inited)
		return;
	if(set)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showReload"), "");
	else
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.hideReload"), "");
}

void HUDDisplay::showBombTimer(bool set)
{
	if(!Inited)
		return;
	if(bombTimerVisible == set)
		return;
	bombTimerVisible = set;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showBombTimer"), (int)set);
}

void HUDDisplay::setBombTimerProgress(float progress)
{
	if(!Inited)
		return;

	progress = R3D_CLAMP(progress, 0.0f, 1.0f);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setBombTimerProgress"), progress);
}

void HUDDisplay::SetReloadingProgress(float progress)
{
	if(!Inited)
		return;
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.updateReload"), progress);
}

void HUDDisplay::showReticle(bool set)
{
	if(!Inited)
		return;
	if(ReticleVisible == set)
		return;
	ReticleVisible = set;
	if(set)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showReticle"), "");
	else
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.hideReticle"), "");
}

void HUDDisplay::setReticleScale(float scale)
{
	if(!Inited)
		return;
	static float prevScale = 1.0f;
	if(R3D_ABS(prevScale-scale) < 0.01f)
		return;
	prevScale = scale;

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.scaleReticle"), scale);
}

void HUDDisplay::setReticleColor(const char* color)
{
	if(!Inited)
		return;
	static const char* prevColor = "none";
	if(prevColor == color) // ok to check pointers, as color comes in as static text
		return;
	prevColor = color;
	
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.setReticleColor"), color);
}

void HUDDisplay::HitOtherPlayer()
{
	if(!Inited)
		return;
}

void HUDDisplay::addGrenadeIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addGrenadeIcon"), &result, NULL, 0);
}

void HUDDisplay::addTargetIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());

	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addTargetIcon"), &result, NULL, 0);
}

void HUDDisplay::addMineIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addMineIcon"), &result, NULL, 0);
}

void HUDDisplay::addMineEnemyIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addMineEnemyIcon"), &result, NULL, 0);
}

void HUDDisplay::addPingIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addPingIcon"), &result, NULL, 0);
}

void HUDDisplay::addWoundedIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addWoundedIcon"), &result, NULL, 0);
}

void HUDDisplay::addIconSpawnIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addIconSpawnIcon"), &result, NULL, 0);
}

void HUDDisplay::addHealingIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addHealingIcon"), &result, NULL, 0);
}

void HUDDisplay::addMedicIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addMedicIcon"), &result, NULL, 0);
}

void HUDDisplay::addNeedHelpIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addNeedHelpIcon"), &result, NULL, 0);
}

void HUDDisplay::addNeedOrdersIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addNeedOrdersIcon"), &result, NULL, 0);
}

void HUDDisplay::addNeedMedicIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addNeedMedicIcon"), &result, NULL, 0);
}

void HUDDisplay::addNeedAmmoIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addNeedAmmoIcon"), &result, NULL, 0);
}

void HUDDisplay::addHUDIcon(HUDIconType type, float lifetime, const r3dVector& pos)
{
	if(!Inited)
		return;

	int i=0;
	bool found = false;
	// search for icon within X radius firstly
	for(i=0;i<32; ++i)
	{
		if(m_HUDIcons[i].enabled && m_HUDIcons[i].type == type && (pos-m_HUDIcons[i].pos).Length() < 10.0f) // same point, just update lifetime
		{
			m_HUDIcons[i].spawnTime = r3dGetTime();
			return;
		}
	}
	for(i=0;i<32; ++i)
	{
		if(!m_HUDIcons[i].enabled)
		{
			found = true;
			break;
		}
	}
	if(!found)
		return;

	r3d_assert(m_HUDIcons[i].icon.IsUndefined());

	if(type == HUDIcon_Attack)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addAttackIcon"), &m_HUDIcons[i].icon, NULL, 0);
	else if(type == HUDIcon_Defend)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addDefendIcon"), &m_HUDIcons[i].icon, NULL, 0);
	else if(type == HUDIcon_Spotted)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addSpottedIcon"), &m_HUDIcons[i].icon, NULL, 0);
	else
		r3d_assert(false);
	
	m_HUDIcons[i].enabled = true;
	m_HUDIcons[i].type = type;
	m_HUDIcons[i].lifetime = lifetime;
	m_HUDIcons[i].spawnTime = r3dGetTime();
	m_HUDIcons[i].pos = pos;

	moveScreenIcon(m_HUDIcons[i].icon, pos, true, false);
}

void HUDDisplay::addControlPointIcon(Scaleform::GFx::Value& result)
{
	if(!Inited)
		return;
	r3d_assert(result.IsUndefined());
	gfxHUD.GetMovie()->Invoke(VMPROTECT_DecryptStringA("_global.addControlPointIcon"), &result, NULL, 0);
	Scaleform::GFx::Value::DisplayInfo dinfo;
	Scaleform::GFx::Value Ring;
	result.GetMember("Ring", &Ring);
	Ring.GetDisplayInfo(&dinfo);
	dinfo.SetAlpha(0.0f);
	Ring.SetDisplayInfo(dinfo);

	Scaleform::GFx::Value RangeToFlag;
	result.GetMember("RangeToFlag", &RangeToFlag);
	RangeToFlag.GetDisplayInfo(&dinfo);
	dinfo.SetVisible(false);
	RangeToFlag.SetDisplayInfo(dinfo);
}

void HUDDisplay::addPlayerTagIcon(const char* name, const char* team, Scaleform::GFx::Value& icon)
{
	if(!Inited)
		return;
	// quickfix: when we are creating a player in Respawn screen, do not allow it to register player tag icon. Need to fix it properly
	if(strcmp(name, "unknown")==0)
		return;

	if(strcmp(name, "---")==0)
		return;

	r3d_assert(icon.IsUndefined());
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.addPlayerTag"), &icon, NULL, 0);

	Scaleform::GFx::Value chevron, PlayerName, Backlight, Plate, Frame1, Frame2, tmpVal;
	icon.GetMember("chevron", &chevron);
	chevron.GetMember("PlayerName", &PlayerName);
	chevron.GetMember("Backlight", &Backlight);
	chevron.GetMember("Plate", &Plate);
	chevron.GetMember("Frame1", &Frame1);
	chevron.GetMember("Frame2", &Frame2);

	PlayerName.SetTextHTML(name);
	tmpVal.SetString("center");
	PlayerName.SetMember("autoSize", tmpVal);

	PlayerName.GetMember("textWidth", &tmpVal);
	float plrNameTextWidth = (float)tmpVal.GetNumber();

	float backlightWidth = plrNameTextWidth * 1.5f;
	tmpVal.SetNumber(backlightWidth);
	Backlight.SetMember("_width", tmpVal);
	tmpVal.SetNumber(-(backlightWidth*0.5f));
	Backlight.SetMember("_x", tmpVal);

	float plateWidth = plrNameTextWidth + 20.0f;
	float plateX = -(plateWidth*0.5f);
	tmpVal.SetNumber(plateWidth);
	Plate.SetMember("_width", tmpVal);
	tmpVal.SetNumber(plateX);
	Plate.SetMember("_x", tmpVal);
	Frame1.SetMember("_x", tmpVal);
	tmpVal.SetNumber(plateX + plateWidth);
	Frame2.SetMember("_x", tmpVal);

	icon.GotoAndStop(team);
	Backlight.GotoAndStop(team);
	Frame1.GotoAndStop(team);
	Frame2.GotoAndStop(team);

	showPlayerNameTag(icon, false);
}

void HUDDisplay::showPlayerNameTag(Scaleform::GFx::Value& icon, bool show)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value chevron;
	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetMember("chevron", &chevron);
	chevron.GetDisplayInfo(&dinfo);
	dinfo.SetVisible(show);
	chevron.SetDisplayInfo(dinfo);
}

void HUDDisplay::setControlPointIconStatus(Scaleform::GFx::Value& icon, const char* status, const char* tag)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	char frame[64];
	sprintf(frame, "%s_%s", status, tag);
	icon.GotoAndStop(frame);
	Scaleform::GFx::Value RangeToFlag, BacklightBlue, BacklightRed, BacklightGrey, Frame1, Frame2;
	icon.GetMember("RangeToFlag", &RangeToFlag);
	RangeToFlag.GetMember("BacklightBlue", &BacklightBlue);
	RangeToFlag.GetMember("BacklightRed", &BacklightRed);
	RangeToFlag.GetMember("BacklightGrey", &BacklightGrey);
	RangeToFlag.GetMember("Frame1", &Frame1);
	RangeToFlag.GetMember("Frame2", &Frame2);
	Scaleform::GFx::Value::DisplayInfo dinfo;
	BacklightBlue.GetDisplayInfo(&dinfo);
	if(strcmp(status, "blue")==0)
	{
		dinfo.SetVisible(false);
		BacklightRed.SetDisplayInfo(dinfo);
		BacklightGrey.SetDisplayInfo(dinfo);
		dinfo.SetVisible(true);
		BacklightBlue.SetDisplayInfo(dinfo);
	}
	else if(strcmp(status, "red")==0)
	{
		dinfo.SetVisible(false);
		BacklightBlue.SetDisplayInfo(dinfo);
		BacklightGrey.SetDisplayInfo(dinfo);
		dinfo.SetVisible(true);
		BacklightRed.SetDisplayInfo(dinfo);
	}
	else // grey
	{
		dinfo.SetVisible(false);
		BacklightRed.SetDisplayInfo(dinfo);
		BacklightBlue.SetDisplayInfo(dinfo);
		dinfo.SetVisible(true);
		BacklightGrey.SetDisplayInfo(dinfo);
	}
	Frame1.GotoAndStop(status);
	Frame2.GotoAndStop(status);
}

void HUDDisplay::setControlPointDistance(Scaleform::GFx::Value& icon, float distance, bool visible)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value RangeToFlag, Range, BacklightGrey, BacklightRed, BacklightBlue, Plate, Frame1, Frame2;
	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetMember("RangeToFlag", &RangeToFlag);
	RangeToFlag.GetDisplayInfo(&dinfo);
	dinfo.SetVisible(visible);
	RangeToFlag.SetDisplayInfo(dinfo);
	if(visible)
	{
		RangeToFlag.GetMember("Range", &Range);
		RangeToFlag.GetMember("BacklightBlue", &BacklightBlue);
		RangeToFlag.GetMember("BacklightRed", &BacklightRed);
		RangeToFlag.GetMember("BacklightGrey", &BacklightGrey);
		RangeToFlag.GetMember("Frame1", &Frame1);
		RangeToFlag.GetMember("Frame2", &Frame2);
		RangeToFlag.GetMember("Plate", &Plate);

		char tmpStr[16];
		sprintf(tmpStr, "%.0f", distance);
		Range.SetText(tmpStr);
		Scaleform::GFx::Value tmpVal;
		tmpVal.SetString("center");
		Range.SetMember("autoSize", tmpVal);

		Range.GetMember("textWidth", &tmpVal);
		float textWidth = (float)tmpVal.GetNumber();
		float backlightWidth = textWidth * 2.5f;
		float backlightX = -(backlightWidth*0.5f);
		tmpVal.SetNumber(backlightWidth);
		BacklightBlue.SetMember("_width", tmpVal);
		BacklightRed.SetMember("_width", tmpVal);
		BacklightGrey.SetMember("_width", tmpVal);
		tmpVal.SetNumber(backlightX);
		BacklightBlue.SetMember("_x", tmpVal);
		BacklightRed.SetMember("_x", tmpVal);
		BacklightGrey.SetMember("_x", tmpVal);

		float plateWidth = textWidth + 20.0f;
		float plateX = -(plateWidth*0.5f);
		tmpVal.SetNumber(plateWidth);
		Plate.SetMember("_width", tmpVal);
		tmpVal.SetNumber(plateX);
		Plate.SetMember("_x", tmpVal);
		Frame1.SetMember("_x", tmpVal);
		tmpVal.SetNumber(plateX + plateWidth);
		Frame2.SetMember("_x", tmpVal);		
	}
}

// optimized version
void HUDDisplay::moveScreenIcon(Scaleform::GFx::Value& icon, const r3dPoint3D& pos, bool alwaysShow, bool force_invisible /* = false */, bool pos_in_screen_space/* =false */)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	r3dPoint3D scrCoord;
	float x, y;
	int isVisible = 1;
	if(!pos_in_screen_space)
	{
		if(alwaysShow)
			isVisible = r3dProjectToScreenAlways(pos, &scrCoord, 20, 20);
		else
			isVisible = r3dProjectToScreen(pos, &scrCoord);
	}
	else
		scrCoord = pos;

	x = scrCoord.x;
	y = scrCoord.y;

	/*Scaleform::GFx::Value var[4];
	var[0].SetString(name);
	var[1].SetNumber(x);
	var[2].SetNumber(y);
	var[3].SetBoolean((isVisible && !force_invisible));
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.moveScreenIcon"), var, 4);*/

	Scaleform::GFx::Value::DisplayInfo displayInfo;
	icon.GetDisplayInfo(&displayInfo);
	displayInfo.SetVisible(isVisible && !force_invisible);
	displayInfo.SetX(x);
	displayInfo.SetY(y);
	icon.SetDisplayInfo(displayInfo);
}

void HUDDisplay::setScreenIconAlpha(Scaleform::GFx::Value& icon, float alpha)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetDisplayInfo(&dinfo);
	dinfo.SetAlpha(R3D_CLAMP(alpha * 100.0f, 0.0f, 100.0f)); // 0..100
	icon.SetDisplayInfo(dinfo);
}

void HUDDisplay::setScreenIconScale(Scaleform::GFx::Value& icon, float scale)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());
	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetDisplayInfo(&dinfo);
	scale = R3D_CLAMP(scale*100.0f, 0.0f, 100.0f);
	dinfo.SetScale(scale, scale);
	icon.SetDisplayInfo(dinfo);
}

void HUDDisplay::deleteScreenIcon(Scaleform::GFx::Value& icon)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	icon.Invoke("removeMovieClip");
	icon.SetUndefined();
}

void HUDDisplay::showPointCapturedInfo(const char* type, const wchar_t* text)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value vars[2];
	vars[0].SetString(type);
	vars[1].SetStringW(text);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showPointCapturedMessage"), vars, 2);
}

void HUDDisplay::showSystemMessage(const wchar_t* text)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value vars[1];
	vars[0].SetStringW(text);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showNoRewardMessage"), vars, 1);
}

void HUDDisplay::showActionMessage(const char* action, const char* key)
{
	if(!Inited)
		return;
	Scaleform::GFx::Value vars[3];
	vars[0].SetString(action);
	vars[1].SetString(key);
	vars[2].SetString(""); // how much
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showActionMessage"), vars, 3);
}


void HUDDisplay::hideActionMessage( const char* action )
{
	if(!Inited)
		return;

	Scaleform::GFx::Value vars[1];
	vars[0].SetString(action);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.hideActionMessage"), vars, 1);
}

void HUDDisplay::setVisibleControlPointCaptureProgress(Scaleform::GFx::Value& icon, bool visible)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());
	if(visible)
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showCaptureProgress"), NULL, &icon, 1);
	else
		gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.hideCaptureProgress"), NULL, &icon, 1);
}

void HUDDisplay::setControlPointCaptureProgress(Scaleform::GFx::Value& icon, float progress, const char* type)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());
	Scaleform::GFx::Value var[3];
	var[0] = icon;
	var[1].SetNumber(progress);
	var[2].SetString(type);
	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.updateCaptureProgress"), var, 3);
}

void HUDDisplay::showPickupMsg(const char* key, const char* wpnName, const char* wpnIcon,
					  const wchar_t* prop1, int Value1, int ModValue1,
					  const wchar_t* prop2, int Value2, int ModValue2,
					  const wchar_t* prop3, int Value3, int ModValue3,
					  const wchar_t* prop4, int Value4, int ModValue4,
					  const wchar_t* capacity, int cap)
{
	if(!Inited)
		return;

	Scaleform::GFx::Value var[16];
	var[0].SetString(wpnName);
	var[1].SetString(wpnIcon);
	var[2].SetStringW(prop1);
	var[3].SetNumber(Value1);
	var[4].SetNumber(ModValue1);
	var[5].SetStringW(prop2);
	var[6].SetNumber(Value2);
	var[7].SetNumber(ModValue2);
	var[8].SetStringW(prop3);
	var[9].SetNumber(Value3);
	var[10].SetNumber(ModValue3);
	var[11].SetStringW(prop4);
	var[12].SetNumber(Value4);
	var[13].SetNumber(ModValue4);
	var[14].SetStringW(capacity);
	var[15].SetNumber(cap);

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.showPickupMsg"), var, 16);

	showActionMessage("pickup", key);
}

void HUDDisplay::hidePickupMsg()
{
	if(!Inited)
		return;

	gfxHUD.Invoke(VMPROTECT_DecryptStringA("_global.hidePickupMsg"), "");
}

void HUDDisplay::RequeustAchievement( int achievementID )
{
	if( m_NumAchievementsInQueue >= MAX_HUD_ACHIEVEMENT_QUEUE )
	{
		r3dOutToLog("Requeusting Achievement when already maxed. Ignoring achievement. ");
		return;
	}

	m_AchievementQueue[m_NumAchievementsInQueue] = achievementID;
	m_NumAchievementsInQueue++;
}

void  HUDDisplay::ProcessAchievementQueue()
{
	if( lastAchievementTime + gTimeToShowAchievements  < r3dGetTime() && HasAchievements())
	{
		int achievementID = popAchievementFromQueue();
		ShowAchievement(achievementID);	
	}
}

int HUDDisplay::popAchievementFromQueue()
{
	r3d_assert( m_AchievementQueue[0] != 0 && HasAchievements() );
	int retValue = m_AchievementQueue[0];
	for( int achievementIndex = 1; achievementIndex < m_NumAchievementsInQueue; achievementIndex++ )
	{
		m_AchievementQueue[achievementIndex -1] = m_AchievementQueue[achievementIndex];
	}
	m_NumAchievementsInQueue--;

	return retValue;

}

int HUDDisplay::getCurrentAchievementDisplaying()
{
	if( lastAchievementTime + gTimeToShowAchievements  > r3dGetTime() )
	{
		return m_CurrentAchievement;
	}

	return 0;
}

