#include "r3dPCH.h"
#include "r3d.h"

#include "ClientGameLogic.h"

#include "GameObjects/ObjManag.h"
#include "GameObjects/EventTransport.h"
#include "MasterServerLogic.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/AI/AI_Player.h"
#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/Gameplay/obj_DroppedBomb.h"
#include "ObjectsCode/Gameplay/obj_BombPlacement.h"
#include "ObjectsCode/Gameplay/obj_UAV.h"
#include "ObjectsCode/weapons/Weapon.h"
#include "ObjectsCode/weapons/WeaponArmory.h"
#include "ObjectsCode/WEAPONS/DroppedWeapon.h"
#include "ObjectsCode/WEAPONS/DroppedLootbox.h"
#include "ObjectsCode/WEAPONS/DroppedMedKit.h"
#include "ObjectsCode/WEAPONS/DroppedMotionSensor.h"
#include "ObjectsCode/WEAPONS/DroppedRespawnBeacon.h"
#include "ObjectsCode/WEAPONS/obj_RiotShield.h"
#include "ObjectsCode/WEAPONS/obj_AutoTurret.h"
#include "ObjectsCode/weapons/Ammo.h"
#include "ObjectsCode/AIRSTRIKES/Airstrike.h"

#include "GameCode/UserProfile.h"
#include "Gameplay_Params.h"

#include "ui/m_LoadingScreen.h"
#include "ui/m_EndRound.h"
#include "ui/HUDDisplay.h"
#include "ui/HUDRespawn.h"
#include "ui/HUDCommCalls.h"
#include "ui/HUDLaserDesignator.h"

#include "GameObjects/obj_Vehicle.h"

extern HUDEndRound*	hudEndRound;
extern HUDDisplay*	hudMain;
extern HUDRespawn*	hudRespawn;
extern HUDCommCalls* hudCommCalls;
extern HUDLaserDesignator* hudLaserDesignator;
extern int g_RenderScopeEffect;

// VMProtect code block
#if USE_VMPROTECT
  #pragma optimize("g", off)
#endif

static r3dSec_type<ClientGameLogic*, 0xC7AA2FB5> g_pClientLogic = NULL;

void ClientGameLogic::CreateInstance()
{
	VMPROTECT_BeginVirtualization("ClientGameLogic::CreateInstance");
	r3d_assert(g_pClientLogic == NULL);
	g_pClientLogic = new ClientGameLogic();
	VMPROTECT_End();
}

void ClientGameLogic::DeleteInstance()
{
	VMPROTECT_BeginVirtualization("ClientGameLogic::DeleteInstance");
	SAFE_DELETE(g_pClientLogic);
	VMPROTECT_End();
}

ClientGameLogic* ClientGameLogic::GetInstance()
{
	VMPROTECT_BeginMutation("ClientGameLogic::GetInstance");
	r3d_assert(g_pClientLogic);
	return g_pClientLogic;
	VMPROTECT_End();
}

obj_AI_Player* ClientGameLogic::GetPlayer(int idx) const
{
	VMPROTECT_BeginMutation("ClientGameLogic::GetPlayer");
	r3d_assert(idx < MAX_NUM_PLAYERS);
	return players2_[idx].ptr;
	VMPROTECT_End();
}

void ClientGameLogic::SetPlayerPtr(int idx, obj_AI_Player* ptr)
{
	VMPROTECT_BeginMutation("ClientGameLogic::SetPlayerPtr");
	r3d_assert(idx < MAX_NUM_PLAYERS);
	players2_[idx].pad1 = (DWORD)ptr ^ 0x3462AB40;
	players2_[idx].pad2 = (DWORD)ptr ^ 0x6A296B48;
	players2_[idx].pad3 = (DWORD)ptr ^ 0x836543F2;
	players2_[idx].ptr  = ptr;

	if(idx >= CurMaxPlayerIdx) 
		CurMaxPlayerIdx = idx + 1;

	VMPROTECT_End();
}

#if USE_VMPROTECT
  #pragma optimize("g", on)
#endif



static const int HOST_TIME_SYNC_SAMPLES	= 20;

static void preparePacket(const GameObject* from, DefaultPacket* packetData)
{
	r3d_assert(packetData);
	//r3d_assert(packetData->EventID >= 0);

	if(from) {
		r3d_assert(from->NetworkID);
		//r3d_assert(from->NetworkLocal);

		packetData->FromID = toP2pNetId(from->NetworkID);
	} else {
		packetData->FromID = 0; // world event
	}

	return;
}

bool g_bDisableP2PSendToHost = false;
void p2pSendToHost(const GameObject* from, DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	extern bool g_bEditMode;
	if(g_bEditMode)
		return;
	if(g_bDisableP2PSendToHost)
		return;

	preparePacket(from, packetData);
	gClientLogic().net_->SendToHost(packetData, packetSize, guaranteedAndOrdered);
}

ClientGameLogic::ClientGameLogic()
{
	Reset();
	requestToJoinAsSpectator = false;
}

void ClientGameLogic::Reset()
{
	net_lastFreeId    = NETID_OBJECTS_START;

	serverConnected_ = false;
	gameFinished_ = false;
	gameShuttedDown_ = false;
	gameReadyToExit = false;
	gameReadyForNextRound = false;
	gameClosed_ = false;

	m_isSpectator = false;

	m_DroppedBomb = NULL;
	m_onJoinServerAssignedTeamId = 0;
	
	cheatAttemptReceived_ = false;
	cheatAttemptCheatId_  = 0;
	nextSecTimeReport_    = 0xFFFFFFFF;
	gppDataSeed_          = 0;
	d3dCheatSent_         = false;

	m_highPingTimer		  = 0;
	showCoolThingTimer	  = u_GetRandom(30.0f, 300.0f);

	m_gameLocalStartTime = 0;
	
	gameJoinAnswered_ = false;
	gameStartAnswered_ = false;
	serverVersionStatus_ = 0;
	m_gameHasStarted = false;
	m_gameInfo = GBGameInfo();
	m_sessionId = 0;

	localPlayerIdx_   = -1;
	localPlayer_      = NULL;
	localPlayerConnectedTime = 0;

	CurMaxPlayerIdx = 0;
	for(int i=0; i<MAX_NUM_PLAYERS; i++) {
		SetPlayerPtr(i, NULL);
	}
	CurMaxPlayerIdx = 0; // reset it after setting to NULLs

	for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
	{
		lobbyPlayers[i].peerID = -1;
	}

	for(int i=0; i<getNumAirstrikes(); ++i)
	{
		AirstrikeDataset* AS = getAirstrike(i);
		AS->CurrentCooldown[0] = 0;
		AS->CurrentCooldown[1] = 0;
	}
	AIRSTRIKE_Team_Cooldown[0] = 0;
	AIRSTRIKE_Team_Cooldown[1] = 0;

	tickets_[0] = 0;
	tickets_[1] = 0;

	// clearing scoping.  Particularly important for Spectator modes. 
	g_RenderScopeEffect = 0;

}

ClientGameLogic::~ClientGameLogic()
{
	g_net.Deinitialize();
}

void ClientGameLogic::OnNetPeerConnected(DWORD peerId)
{
#ifndef FINAL_BUILD
	r3dOutToLog("peer%02d connected\n", peerId);
#endif
	serverConnected_ = true;
	return;
}

void ClientGameLogic::OnNetPeerDisconnected(DWORD peerId)
{
	r3dOutToLog("***** disconnected from game server\n");
	serverConnected_ = false;
	return;
}

void ClientGameLogic::OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize)
{
	r3d_assert(packetSize >= sizeof(DefaultPacket));
	const DefaultPacket* evt = static_cast<const DefaultPacket*>(packetData);

	GameObject* fromObj = NULL;
	if(evt->FromID != 0) 
	{
		fromObj = GameWorld().GetNetworkObject(evt->FromID);
	}

	//r3dOutToLog("OnNetData from peer%d, obj:%d(%s), event:%d\n", peerId, evt->FromID, fromObj ? fromObj->Name.c_str() : "", evt->EventID);

	if(evt->FromID && fromObj == NULL) 
	{
		r3dOutToLog("!!!!!!!!!! bad event sent from non registered object %d\n", evt->FromID);
		return; 
	}

	if(fromObj)
	{
		r3d_assert(!(fromObj->ObjFlags & OBJFLAG_JustCreated)); // just to make sure
	}

	// pass to world even processor first.
	if(ProcessWorldEvent(fromObj, evt->EventID, peerId, packetData, packetSize)) 
		return;

	if(fromObj) 
	{
		if(!fromObj->OnNetReceive(evt->EventID, packetData, packetSize)) 
			r3dError("bad event %d for %s", evt->EventID, fromObj->Class->Name.c_str());
		return;
	}

	r3dError("bad world event %d", evt->EventID);
	return;
}

r3dPoint3D ClientGameLogic::AdjustSpawnPositionToGround(const r3dPoint3D& pos)
{
	//
	// detect 'ground' under spawn position. 
	// because server now send exact position and it might be under geometry if it was changed
	//
	PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK,0,0,0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC));
	if(!g_pPhysicsWorld->raycastSingle(PxVec3(pos.x, pos.y+1.0f, pos.z), PxVec3(0,-1,0), 1.2f, PxSceneQueryFlags(PxSceneQueryFlag::eIMPACT), hit, filter))
		return pos + r3dPoint3D(0, 1.0f, 0);
		
	return r3dPoint3D(hit.impact.x, hit.impact.y + 0.1f, hit.impact.z);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_C2S_ValidateConnectingPeer)
{
	serverVersionStatus_ = 1;
	if(n.protocolVersion != P2PNET_VERSION)
	{
		r3dOutToLog("Version mismatch our:%d, server:%d\n", P2PNET_VERSION, n.protocolVersion);
		serverVersionStatus_ = 2;
	}
		
	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_C2C_PacketBarrier)
{
	// sanity check: must be only for local networked objects
	if(fromObj && !fromObj->NetworkLocal) {
		r3dError("PKT_C2C_PacketBarrier for %s, %s, %d\n", fromObj->Name.c_str(), fromObj->Class->Name.c_str(), fromObj->NetworkID);
	}

	// reply back
	PKT_C2C_PacketBarrier_s n2;
	p2pSendToHost(fromObj, &n2, sizeof(n2));
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_LevelInfo)
{
	m_gameInfo = n.gameInfo;
	g_num_matches_played->SetInt(g_num_matches_played->GetInt()+1);
	void writeGameOptionsFile();
	writeGameOptionsFile();
}


IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_JoinGameAns)
{
#ifndef FINAL_BUILD
	r3dOutToLog("PKT_S2C_JoinGameAns: %d %d\n", n.success, n.playerIdx);
#endif

	if(n.success != 1) {
		r3dOutToLog("Can't join to game server - session is full");
		return;
	}

	localPlayerIdx_   = n.playerIdx;
	gameTimeEnd_      = r3dGetTime() + n.gameTimeLeft;
	m_gameStartTime = r3dGetTime() - n.gameStartTime;
	m_gameLocalStartTime = r3dGetTime();
	gameJoinAnswered_ = true;

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_ShutdownNote)
{
	gameShuttedDown_ = true;
	
	char msg[128];
	sprintf(msg, "SERVER SHUTDOWN in %.0f sec", n.timeLeft);
	hudMain->AddChatMessage(0, NULL, msg);
	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_SetGamePlayParams)
{
	// replace our game parameters with ones from server.
	r3d_assert(GPP);
	*const_cast<CGamePlayParams*>(GPP) = n.GPP_Data;
	gppDataSeed_ = n.GPP_Seed;

	for(int i=0; i<getNumAirstrikes(); ++i)
	{
		getAirstrike(i)->CurrentCooldown[0] = n.airstrike_cooldowns[(i*2)];
		getAirstrike(i)->CurrentCooldown[1] = n.airstrike_cooldowns[(i*2)+1];
	}
	AIRSTRIKE_Team_Cooldown[0] = n.airstrike_cooldowns[getNumAirstrikes()*2];
	AIRSTRIKE_Team_Cooldown[1] = n.airstrike_cooldowns[getNumAirstrikes()*2+1];

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_StartGameAns)
{
#ifndef FINAL_BUILD
	r3dOutToLog("OnPKT_S2C_StartGameAns, %d\n", n.result);
#endif

	if(n.result == n.RES_UNSYNC)
	{
		r3dOutToLog("Client desync. Please get update\n");
		gMasterServerLogic.badClientVersion_ = true;
	}

	r3d_assert(gameStartAnswered_ == false);
	gameStartAnswered_ = true;
	gameStartResult_   = n.result;
	m_onJoinServerAssignedTeamId = n.autoplacedTeamId;
	requestToJoinAsSpectator = false; // reset

	m_gameHasStarted = n.gameStarted ? true : false;
	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_GameAboutToStart)
{
	if(localPlayer_ && m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
		hudMain->ShowAchievementCustom(gLangMngr.getString("$HUD_Msg_GameAboutToStart"), "", "$Data/Menu/Rewards/Rewards_timer.png", "");

	if( m_gameInfo.mapType != GBGameInfo::MAPT_Bomb )
	{
		// reset local stats
		for(int i=0; i<MAX_NUM_PLAYERS; ++i)
		{
			obj_AI_Player* pl = GetPlayer(i);
			if(pl)
			{
				pl->GameStats.Score = 0;
				pl->GameStats.GD = 0;
				pl->GameStats.Kills = 0;
				pl->GameStats.Deaths = 0;

				// reset UAV, the server will be destroying it. 
				pl->uavRequested_ = 0;
				pl->uavId_ = invalidGameObjectID;
				
			}
		}
	}
	
	gameTimeEnd_      = r3dGetTime() + n.gameTimeLeft;
	m_gameStartTime		= r3dGetTime(); // game just started
	m_gameLocalStartTime = r3dGetTime();
	m_gameHasStarted = true;

	if(m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
		{
			gBombPlacementMgr.GetBombPlacement(i)->setStatus(obj_BombPlacement::SS_EMPTY);
		}

		if(localPlayer_==NULL)
		{
			if(!hudRespawn->isWaitingForNextRound())
				hudRespawn->StartLobbyCountdown();
			else
			{
				if(hudRespawn->isActive())
					hudRespawn->Deactivate();
				hudRespawn->Activate(m_onJoinServerAssignedTeamId, 0, true);
			}
		}
		else if(!hudRespawn->isActive())
			hudRespawn->Activate(localPlayer_->TeamID, localPlayer_->LoadoutSlot);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_GameStarted)
{
	if(m_gameInfo.mapType != GBGameInfo::MAPT_Bomb)
	{
		r3d_assert(fromObj->isObjType(OBJTYPE_Human));
		obj_AI_Player* pl = (obj_AI_Player*)fromObj;

		r3dPoint3D spawnPos = AdjustSpawnPositionToGround(n.teleport_pos);
		pl->TeleportPlayer(spawnPos, "GameStarted");
		pl->ViewAngle.Assign(n.dir, 0, 0);
		pl->m_Health = pl->getMaxHealth();
		pl->m_Energy = pl->getMaxStamina();

		pl->UpdateLoadoutSlot(pl->CurLoadout, pl->CurWeaponAttachments);
	}
	else
	{
		// spectator mode
		m_isSpectator = true;
		if(hudRespawn->isActive())
			hudRespawn->Deactivate();
		hudMain->switchToDead(true);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_ConnectedPlayer)
{
	if(n.isMaster)
		hudRespawn->Lobby_UnlockCreatorOptions();

	bool found = false;
	for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
	{
		if(lobbyPlayers[i].peerID == n.peerID)
		{
			lobbyPlayers[i].desiredTeam = n.teamID;
			lobbyPlayers[i].isReady = n.isReady;
			lobbyPlayers[i].isSpectator = n.isSpectator;
			found = true;
			break;
		}
	}

	// find empty slot
	if(!found)
	{
		for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
		{
			if(lobbyPlayers[i].peerID == -1)
			{
				lobbyPlayers[i].peerID = n.peerID;
				lobbyPlayers[i].desiredTeam = n.teamID;
				lobbyPlayers[i].isReady = n.isReady;
				lobbyPlayers[i].isSpectator = n.isSpectator;
				lobbyPlayers[i].level = n.plrLevel;
				r3dscpy(lobbyPlayers[i].userName, n.userName);
				break;
			}
		}
	}
	hudRespawn->UpdateLobbyPlayerList();
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_PlayerReady)
{
	for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
	{
		if(lobbyPlayers[i].peerID == n.peerID)
		{
			lobbyPlayers[i].isReady = n.isReady;
			hudRespawn->Lobby_SetPlayerReady(lobbyPlayers[i].userName, n.isReady==1);
			break;
		}
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_PlayerHasBomb)
{
	obj_AI_Player* plr = (obj_AI_Player*)GameWorld().GetNetworkObject(n.playerID);
	if(plr)
		plr->setHasSabotageBomb(true);
	m_DroppedBomb->BombDropped(false);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_Dropped)
{
	m_DroppedBomb->SetPosition(n.pos);
	m_DroppedBomb->BombDropped(true);

	for(int i=0; i<CurMaxPlayerIdx; ++i)
	{
		obj_AI_Player* plr = GetPlayer(i);
		if(plr)
			plr->setHasSabotageBomb(false);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_ChatMsg)
{
	if(hudRespawn->isActive())
	{
		const char* from = 0;
		for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
		{
			if(lobbyPlayers[i].peerID == n.senderPeerId)
			{
				from = lobbyPlayers[i].userName;
				break;
			}
		}

		if(from)
		{
			hudRespawn->showChatMsg(n.isTeam, from, n.isAlly, n.msg);
		}
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_TargetMarked)
{
	obj_AI_Player* plr = (obj_AI_Player*)GameWorld().GetNetworkObject(n.targetID);
	if(plr)
		plr->m_targetIconTime = n.time;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_WonRound)
{
	hudMain->ShowAchievementCustom((n.teamID==0)?gLangMngr.getString("TerroristWon"):gLangMngr.getString("CounterTerroristWon"), "", "$Data/Menu/achievements/hud/boomer.png", "");

	for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
	{
		gBombPlacementMgr.GetBombPlacement(i)->setStatus(obj_BombPlacement::SS_EMPTY);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Bomb_DisconnectedPlayer)
{
	for(int i=0; i<MAX_NUM_LOBBY_PLAYERS; ++i)
	{
		if(lobbyPlayers[i].peerID == n.peerID)
		{
			lobbyPlayers[i].peerID = -1;
			break;
		}
	}
	hudRespawn->UpdateLobbyPlayerList();
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_C2C_ChatMessage)
{
	hudMain->AddChatMessage(n.msgType, (obj_AI_Player*)fromObj, n.msg);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_C2C_VoiceCommand)
{
	obj_AI_Player* fromPl = (obj_AI_Player*)fromObj;
	hudCommCalls->PlayMessage(fromPl, n.id);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_C2C_CommRoseCommand)
{
	obj_AI_Player* fromPl = (obj_AI_Player*)fromObj;
	hudCommCalls->showHUDIcon(fromPl, n.id, n.pos);
}

void Airstrike_Fire(const r3dPoint3D& hitPos, int itemID, float heightOffset, int teamID);
void Airstrike_Spawn_Explosion(const r3dPoint3D& hitPos, int itemID);
IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Airstrike)
{
	obj_AI_Player* fromPl = (obj_AI_Player*)fromObj;
	if(n.itemID == 0)
		hudLaserDesignator->showWarning(gLangMngr.getString("HUD_Airstrike_GeneralError"));
	else if(n.itemID == 1)
		hudLaserDesignator->showWarning(gLangMngr.getString("HUD_Airstrike_NoGP"));
	else if(n.itemID == 2)
		hudLaserDesignator->showWarning(gLangMngr.getString("HUD_Airstrike_Cooldown"));
	else if(n.itemID == 3)
		hudLaserDesignator->showWarning(gLangMngr.getString("HUD_Airstrike_NotAuthorized"));
	else if(n.itemID == 100)
	{
		int price = 0;
		for(uint32_t j=0; j<g_NumStoreItems; ++j)
		{
			if(g_StoreItems[j].itemID == n.itemID)
			{
				price = g_StoreItems[j].price1day;
				break;
			}
		}

		gUserProfile.ProfileData.Stats.GamePoints -= price;
	}
	else if(n.itemID > 100)
	{
		if(n.heightOffset < 1000)
		{
			extern int RUS_CLIENT;
			if(!RUS_CLIENT)
			{
				if(localPlayer_)
				{
					if(fromPl->TeamID == localPlayer_->TeamID)
						snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly artillary strike incoming"), r3dPoint3D(0,0,0));
					else
						snd_PlaySound(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/incoming take cover"), r3dPoint3D(0,0,0));

				}
			}
			Airstrike_Fire(n.pos, n.itemID, n.heightOffset, fromPl->TeamID);
		}
		else
			Airstrike_Spawn_Explosion(n.pos, n.itemID);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_UpdateWeaponData)
{
	WeaponConfig* wc = const_cast<WeaponConfig*>(gWeaponArmory.getWeaponConfig(n.itemId));
#ifdef FINAL_BUILD
	if(wc)
		wc->copyParametersFrom(n.wi);
	return;
#endif

	if(wc == NULL) {
		r3dOutToLog("!!! got update for not existing weapon %d\n", n.itemId);
		return;
	}

	wc->copyParametersFrom(n.wi);

	//r3dOutToLog("got update for weapon %s\n", wc->m_StoreName);

	static float lastMsgTime = 0;
	if(r3dGetTime() > lastMsgTime + 1.0f) {
		lastMsgTime = r3dGetTime();
		hudMain->AddMessage(L"weapons data updated");
	}

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_UpdateGearData)
{
	GearConfig* gc = const_cast<GearConfig*>(gWeaponArmory.getGearConfig(n.itemId));
#ifdef FINAL_BUILD
	if(gc)
		gc->copyParametersFrom(n.gi);
	return;
#endif

	if(gc == NULL) {
		r3dOutToLog("!!! got update for not existing gear %d\n", n.itemId);
		return;
	}

	gc->copyParametersFrom(n.gi);

	r3dOutToLog("got update for gear %s\n", gc->m_StoreName);
	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_CreateNetObject)
{
	
	// HACK! 2 means vehicle. 
	if ( n.var5 == 2.0f )
	{

#if VEHICLES_ENABLED
		// spawn uav with manual OnCreate call
		char name[128];
		// HACK: Just hardcode the type;
		sprintf(name, "Data\\ObjectsDepot\\Vehicles\\Drivable_Stryker.sco");
		obj_Vehicle* spawnedVehicle = (obj_Vehicle*)srv_CreateGameObject("obj_Vehicle", name, n.pos);
		spawnedVehicle->NetworkLocal = false;
		spawnedVehicle->NetworkID   = n.spawnID;
		spawnedVehicle->SetRotationVector(r3dPoint3D(n.var1, 0, 0));
		spawnedVehicle->OnCreate();

		// set base cell for movement data (must do it AFTER OnCreate())
		spawnedVehicle->netMover.SetNetCell(r3dPoint3D(n.var2, n.var3, n.var4));
#endif
		return;
	} 

	r3d_assert(fromObj);
	r3d_assert(fromObj->isObjType(OBJTYPE_Human));
	obj_AI_Player* plr = (obj_AI_Player*)fromObj;
	if(n.itemID == WeaponConfig::ITEMID_Cypher2)
	{
		r3d_assert(plr->uavId_ == invalidGameObjectID);
		//r3dOutToLog("PKT_S2C_UAVSpawnAns for %s\n", plr->UserName); CLOG_INDENT;

		// spawn uav with manual OnCreate call
		char name[128];
		sprintf(name, "uav_%p", plr);
		obj_UAV* uav = (obj_UAV*)srv_CreateGameObject("obj_UAV", name, n.pos);
		uav->ownerID      = plr->GetSafeID();
		uav->NetworkLocal = plr->NetworkLocal;
		uav->NetworkID    = n.spawnID;
		uav->SetRotationVector(r3dPoint3D(n.var1, 0, 0));
		uav->OnCreate();

		// start damage particle
		if(n.var5 >= 1) uav->SetDamagedState();

		// set base cell for movement data (must do it AFTER OnCreate())
		if(!uav->NetworkLocal) uav->netMover.SetNetCell(r3dPoint3D(n.var2, n.var3, n.var4));

		// set that player have it
		plr->uavId_ = uav->GetSafeID();

		hudMain->AddMessage(gLangMngr.getString("$HUD_Msg_UAVSpawned"));
	}
	else if(n.itemID == WeaponConfig::ITEMID_MedKit)
	{
		char name[128];
		sprintf(name, "medkit_%p", plr);
		DroppedMedKit* medkit = (DroppedMedKit*)srv_CreateGameObject("DroppedMedKit", name, n.pos);
		medkit->ownerID      = plr->GetSafeID();
		medkit->m_ItemID	= n.itemID;
		medkit->NetworkLocal = plr->NetworkLocal;
		medkit->NetworkID    = n.spawnID;
		medkit->TeamID		= (int)n.var1;
		medkit->SetRotationVector(r3dPoint3D(0, 0, 0));
		medkit->OnCreate();
	}
	else if(n.itemID == WeaponConfig::ITEMID_MotionSensor || n.itemID == WeaponConfig::ITEMID_MotionSensorConsumable)
	{
		char name[128];
		sprintf(name, "sensor_%p", plr);
		DroppedMotionSensor* sensor= (DroppedMotionSensor*)srv_CreateGameObject("DroppedMotionSensor", name, n.pos);
		sensor->ownerID      = plr->GetSafeID();
		sensor->m_ItemID	= n.itemID;
		sensor->NetworkLocal = plr->NetworkLocal;
		sensor->NetworkID    = n.spawnID;
		sensor->SetRotationVector(r3dPoint3D(0, 0, 0));
		sensor->OnCreate();
	}
	else if(n.itemID == WeaponConfig::ITEMID_RespawnBeacon || n.itemID == WeaponConfig::ITEMID_RespawnBeaconCons)
	{
		char name[128];
		sprintf(name, "beacon_%p", plr);
		DroppedRespawnBeacon* beacon= (DroppedRespawnBeacon*)srv_CreateGameObject("DroppedRespawnBeacon", name, n.pos);
		beacon->ownerID      = plr->GetSafeID();
		beacon->m_ItemID	= n.itemID;
		beacon->NetworkLocal = plr->NetworkLocal;
		beacon->NetworkID    = n.spawnID;
		beacon->m_RotX		= n.var1;
		beacon->SetRotationVector(r3dPoint3D(n.var1, 0, 0));
		beacon->OnCreate();
	}
	else if(n.itemID == WeaponConfig::ITEMID_RiotShield || n.itemID == WeaponConfig::ITEMID_RiotShieldConsumable)
	{
		char name[128];
		sprintf(name, "shield_%p", plr);
		obj_RiotShield* shield= (obj_RiotShield*)srv_CreateGameObject("obj_RiotShield", name, n.pos);
		shield->ownerID      = plr->GetSafeID();
		shield->m_ItemID	= n.itemID;
		shield->NetworkLocal = plr->NetworkLocal;
		shield->NetworkID    = n.spawnID;
		shield->m_RotX		= n.var1;
		shield->SetRotationVector(r3dPoint3D(n.var1, 0, 0));
		shield->OnCreate();
	}
	else if(n.itemID == WeaponConfig::ITEMID_AutoTurret || n.itemID == WeaponConfig::ITEMID_AutoTurretCons)
	{
		char name[128];
		sprintf(name, "turret_%p", plr);
		obj_AutoTurret* turret= (obj_AutoTurret*)srv_CreateGameObject("obj_AutoTurret", name, n.pos);
		turret->ownerID      = plr->GetSafeID();
		turret->m_ItemID	= n.itemID;
		turret->NetworkLocal = plr->NetworkLocal;
		turret->NetworkID    = n.spawnID;
		turret->m_RotX		= n.var1;
		turret->SetRotationVector(r3dPoint3D(n.var1, 0, 0));
		turret->OnCreate();
	}

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_DestroyNetObject)
{
	GameObject* obj = GameWorld().GetNetworkObject(n.spawnID);
	if(obj)
		obj->setActiveFlag(0);

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_SpawnDroppedWeapon)
{
	char tempName[128];
	sprintf(tempName, "DroppedWeapon_%d", n.itemID);
	DroppedWeapon* wpn = (DroppedWeapon*)srv_CreateGameObject("DroppedWeapon", tempName, n.pos);
	wpn->SetRotationVector(n.rot);
	wpn->m_WeaponItemID = n.itemID;
	wpn->m_NumBullets = n.numBullets;
	wpn->m_Attms = n.attms;
	wpn->NetworkLocal = false;
	wpn->NetworkID = n.spawnID;
	wpn->OnCreate();

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_DestroyDroppedWeapon)
{
	GameObject* obj = GameWorld().GetNetworkObject(n.spawnID);
	if(obj)
		obj->setActiveFlag(0);

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_SpawnDroppedLootBox)
{
	char tempName[128];
	sprintf(tempName, "DroppedLootbox_%d", n.itemID);
	DroppedLootbox* box = (DroppedLootbox*)srv_CreateGameObject("DroppedLootbox", tempName, n.pos);
	box->SetRotationVector(n.rot);
	box->m_ItemID = n.itemID;
	box->NetworkLocal = false;
	box->NetworkID = n.spawnID;
	box->OnCreate();

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_DestroyDroppedLootBox)
{
	GameObject* obj = GameWorld().GetNetworkObject(n.spawnID);
	if(obj)
		obj->setActiveFlag(0);

	return;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_SpawnMine)
{
	r3d_assert(fromObj);
	r3d_assert(fromObj->isObjType(OBJTYPE_Human));
	obj_AI_Player* plr = (obj_AI_Player*)fromObj;

	AmmoShared* ammoSh = (AmmoShared*)srv_CreateGameObject("obj_Mine", "mine", n.pos);
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(n.itemID);
	ammoSh->m_Ammo = wc->m_PrimaryAmmo;
	ammoSh->m_Weapon = wc;
	ammoSh->ownerID = plr->GetSafeID();
	ammoSh->SetRotationVector(n.rot);
	ammoSh->NetworkLocal = false;
	ammoSh->NetworkID = n.spawnID;

	ammoSh->OnCreate(); // create right away networked object
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_CreatePlayer)
{
	R3DPROFILE_FUNCTION("ClientGameLogic::OnPKT_S2C_CreatePlayer");
	//r3dOutToLog("PKT_S2C_CreatePlayer: %d at %f %f %f\n", n.playerIdx, n.spawnPos.x, n.spawnPos.y, n.spawnPos.z);
#ifndef FINAL_BUILD
	r3dOutToLog("Create player: %s\n", n.userName);
#endif

	r3dPoint3D spawnPos = AdjustSpawnPositionToGround(n.spawnPos);

	char name[128];
	sprintf(name, "player%02d", n.playerIdx);
	obj_AI_Player* plr = (obj_AI_Player*)srv_CreateGameObject("obj_AI_Player", name, spawnPos);
	plr->ViewAngle.Assign(n.spawnDir, 0, 0);
	plr->m_fPlayerRotationTarget = n.spawnDir;
	plr->m_fPlayerRotation = n.spawnDir;
	plr->TeamID       = n.teamId;
	plr->isPremiumAccount = n.isPremium>0;
	plr->LoadoutSlot  = n.slotNum;
	plr->m_SelectedWeapon = n.weapIndex;
	plr->m_PrevSelectedWeapon = -1;
	plr->CurLoadout   = n.lslot;
	plr->CurWeaponAttachments = n.attms;
	plr->bDead        = n.bDying; // set so OnCreate() will know that we already dead
	plr->NetworkID    = n.playerIdx + NETID_PLAYERS_START;
	plr->NetworkLocal = false;
	plr->GameStats.TotalScore = n.score;
	plr->GameStats.Wins = n.wins;
	plr->GameStats.Loss = n.losses;
	plr->m_EncryptedUserName.set(n.userName);
	plr->m_SpawnProtectedUntil = r3dGetTime() + n.spawnProteciton;
	// should be safe to use playerIdx, as it should be uniq to each player
	sprintf_s(plr->m_MinimapTagIconName, 64, "pl_%u", n.playerIdx);
	plr->ClanID = n.ClanID;
	r3dscpy(plr->ClanTag, n.ClanTag);
	plr->ClanTagColor = n.ClanTagColor;

	if(n.playerIdx == localPlayerIdx_) 
	{
		localPlayer_      = plr;
		plr->NetworkLocal = true;

		localPlayerConnectedTime = r3dGetTime();
		
		// start time reports for speedhack detection
		nextSecTimeReport_ = GetTickCount();

		// add chat msg
		hudMain->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_ChatTypeHelp"));

// 		if(m_gameInfo.practiceGame)
// 		{
// 			hudMain->AddChatMessage(0, NULL, gLangMngr.getString("$HUD_Msg_JoinedNonRankedGame"));
// 		}
	}
	plr->OnCreate(); // call OnCreate manually to init player right away
	// call change weapon manually because UpdateLoadoutSlot that is called from OnCreate always resets CurWeapon to 0
	plr->ChangeWeaponByIndex(n.weapIndex);
	plr->SyncAnimation(true);
	
	// set base cell for movement data (must do it AFTER OnCreate())
	if(!plr->NetworkLocal) plr->netMover.SetNetCell(n.moveCell);

	r3d_assert(GetPlayer(n.playerIdx) == NULL);
	SetPlayerPtr(n.playerIdx, plr);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_DropPlayer)
{
	//r3dOutToLog("PKT_S2C_DropPlayer: %d, reason:%d\n", n.playerIdx, n.reason);

	obj_AI_Player* plr = GetPlayer(n.playerIdx);
	if(plr == NULL) {
		r3dOutToLog("!!!warning!!! dropping not existing player %d\n", n.playerIdx);
		return;
	}

	//@TODO: do something when local player was dropped
	if(plr == localPlayer_)
	{
		r3dError("local player dropped by server");
	}

	SetPlayerPtr(n.playerIdx, NULL);
	plr->setActiveFlag(0);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_Damage)
{
	GameObject* targetObj = GameWorld().GetNetworkObject(n.targetId);
	r3d_assert(fromObj);
	r3d_assert(targetObj);

	//r3dOutToLog("PKT_S2C_Damage: from:%s, to:%s, damage:%d\n", fromObj->Name.c_str(), targetObj->Name.c_str(), n.damage);

	r3d_assert(targetObj->isObjType(OBJTYPE_Human));
	obj_AI_Player* targetPlr = (obj_AI_Player*)targetObj;
	targetPlr->ApplyDamage(n.dmgPos, n.damage, fromObj, n.bodyBone, n.dmgType);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_KillPlayer)
{
	R3DPROFILE_FUNCTION("ClientGameLogic::OnPKT_S2C_KillPlayer");
	GameObject* killerObj = fromObj;
	GameObject* targetObj = GameWorld().GetNetworkObject(n.targetId);
	r3d_assert(targetObj);	
	r3d_assert(killerObj);

	//r3dOutToLog("PKT_S2C_KillPlayer: killed %s by %s\n", targetObj->Name.c_str(), killerObj->Name.c_str());

	r3d_assert(targetObj->isObjType(OBJTYPE_Human));
	obj_AI_Player* targetPlr = (obj_AI_Player*)targetObj;
	obj_AI_Player* fromPlr   = (fromObj->isObjType(OBJTYPE_Human)) ? (obj_AI_Player*)fromObj : NULL;

	int killedID = invalidGameObjectID;
	if(fromPlr != targetPlr && fromPlr!=0)
		killedID = fromPlr->NetworkID;
	targetPlr->DoDeath(killedID, n.forced_by_server, (STORE_CATEGORIES)n.killerWeaponCat);
	
	if(n.forced_by_server)
		return;
		
	if(fromPlr != targetPlr) // do not count suicide as kills
	{
		// increase kill count (death will be increased in DoDeath)
		if(fromPlr && fromPlr->TeamID != targetPlr->TeamID)
			fromPlr->GameStats.Kills++;

		hudMain->AddKillMessage(fromPlr, targetPlr, (STORE_CATEGORIES)n.killerWeaponCat);
		if(fromPlr == localPlayer_)
		{
			hudMain->ShowKilledTag(targetPlr);
		}
	}
	else
	{
		hudMain->AddKillMessage(0, targetPlr, storecat_INVALID);
	}
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_RespawnPlayer)
{
	GameObject* targetObj = fromObj;
	r3d_assert(targetObj->isObjType(OBJTYPE_Human));
	obj_AI_Player* targetPlr = (obj_AI_Player*)targetObj;

	//r3dOutToLog("PKT_S2C_RespawnPlayer: %s\n", targetObj->Name.c_str());
	r3dPoint3D spawnPos = AdjustSpawnPositionToGround(n.pos);
	targetPlr->DoRespawn(n.teamId, n.slotNum, n.lslot, n.attms, spawnPos, n.dir, n.spawnProtection);
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_TicketsUpdate)
{
	tickets_[0] = n.tickets[0];
	tickets_[1] = n.tickets[1];
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_GameAbort)
{
	//@TODO: make game abort gracefully
	r3dError("Game Aborted By Server");
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_GameFinish)
{
	//r3dOutToLog("OnPKT_S2C_GameFinish\n");
	gameFinished_ = true;
	finishData_   = n;
	hudEndRound->Activate();
	GameWorld().OnGameEnded();

	int achievementID = hudMain->getCurrentAchievementDisplaying();
	if ( achievementID != 0 )
	{
		hudEndRound->ShowAchievementRibbon( achievementID );
	}

	while( hudMain->HasAchievements() )
	{
		achievementID = hudMain->popAchievementFromQueue();
		hudEndRound->ShowAchievementRibbon( achievementID );
	} 
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_GameClose)
{
	gameClosed_ = true;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_RoundStats)
{
	curRoundScore_ = n.rscore;
	curRoundStat_  = n.rstat;
	curRoundLevelUpMin = n.levelUpMin;
	curRoundLevelUpMax = n.levelUpMax;
	curRoundRewardIdx = n.oneTimeReward;
}

IMPL_PACKET_FUNC(ClientGameLogic, PKT_S2C_CheatWarning)
{
	cheatAttemptReceived_ = true;
	cheatAttemptCheatId_  = n.cheatId;
}


int ClientGameLogic::ProcessWorldEvent(GameObject* fromObj, DWORD eventId, DWORD peerId, const void* packetData, int packetSize)
{
	R3DPROFILE_FUNCTION("ClientGameLogic::ProcessWorldEvent");
	switch(eventId) 
	{
		DEFINE_PACKET_HANDLER(PKT_C2S_ValidateConnectingPeer);
		DEFINE_PACKET_HANDLER(PKT_C2C_PacketBarrier);
		DEFINE_PACKET_HANDLER(PKT_S2C_LevelInfo);
		DEFINE_PACKET_HANDLER(PKT_S2C_JoinGameAns);
		DEFINE_PACKET_HANDLER(PKT_S2C_ShutdownNote);
		DEFINE_PACKET_HANDLER(PKT_S2C_SetGamePlayParams);
		DEFINE_PACKET_HANDLER(PKT_S2C_StartGameAns);
		DEFINE_PACKET_HANDLER(PKT_S2C_CreatePlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_DropPlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_Damage);
		DEFINE_PACKET_HANDLER(PKT_S2C_KillPlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_RespawnPlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_TicketsUpdate);
		DEFINE_PACKET_HANDLER(PKT_S2C_GameAbort);
		DEFINE_PACKET_HANDLER(PKT_S2C_GameFinish);
		DEFINE_PACKET_HANDLER(PKT_S2C_GameClose);
		DEFINE_PACKET_HANDLER(PKT_S2C_RoundStats);
		DEFINE_PACKET_HANDLER(PKT_S2C_GameAboutToStart);
		DEFINE_PACKET_HANDLER(PKT_S2C_GameStarted);
		DEFINE_PACKET_HANDLER(PKT_C2C_ChatMessage);
		DEFINE_PACKET_HANDLER(PKT_C2C_VoiceCommand);
		DEFINE_PACKET_HANDLER(PKT_C2C_CommRoseCommand);
		DEFINE_PACKET_HANDLER(PKT_S2C_Airstrike);
		DEFINE_PACKET_HANDLER(PKT_S2C_UpdateWeaponData);
		DEFINE_PACKET_HANDLER(PKT_S2C_UpdateGearData);
		DEFINE_PACKET_HANDLER(PKT_S2C_CreateNetObject);
		DEFINE_PACKET_HANDLER(PKT_S2C_DestroyNetObject);
		DEFINE_PACKET_HANDLER(PKT_S2C_CheatWarning);
		DEFINE_PACKET_HANDLER(PKT_S2C_SpawnDroppedWeapon);
		DEFINE_PACKET_HANDLER(PKT_S2C_DestroyDroppedWeapon);
		DEFINE_PACKET_HANDLER(PKT_S2C_SpawnDroppedLootBox);
		DEFINE_PACKET_HANDLER(PKT_S2C_DestroyDroppedLootBox);
		DEFINE_PACKET_HANDLER(PKT_S2C_SpawnMine);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_ConnectedPlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_DisconnectedPlayer);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_PlayerReady);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_PlayerHasBomb);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_WonRound);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_Dropped);
		DEFINE_PACKET_HANDLER(PKT_S2C_Bomb_ChatMsg);
		DEFINE_PACKET_HANDLER(PKT_S2C_TargetMarked);
	}

	return FALSE;
}

int ClientGameLogic::WaitFunc(fn_wait fn, float timeout, const char* msg)
{
	float endWait = r3dGetTime() + timeout;
	while(1) 
	{
		r3dEndFrame();
		r3dStartFrame();

		extern void tempDoMsgLoop();
		tempDoMsgLoop();

		if((this->*fn)())
			break;

		r3dRenderer->StartRender();
		r3dRenderer->StartFrame();
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA);
		Font_Label->PrintF(10, 10, r3dColor::white, "%s", msg);
		r3dRenderer->EndFrame();
		r3dRenderer->EndRender( true );

		if(r3dGetTime() > endWait) {
			return 0;
		}
	}

	return 1;
}

bool ClientGameLogic::Connect(const char* host, int port)
{
	r3d_assert(!serverConnected_);
	r3d_assert(!gameFinished_);

	g_net.Initialize(this, "p2pNet");
	g_net.CreateClient();
	g_net.Connect(host, port);

	if( !DoConnectScreen( this, &ClientGameLogic::wait_IsConnected, gLangMngr.getString("WaitConnectingToServer"), 30.f ) )
		return false;

	return true;
}

void  ClientGameLogic::Disconnect()
{
	g_net.Deinitialize();
	serverConnected_ = false;
}

int ClientGameLogic::ValidateServerVersion(__int64 sessionId)
{
	serverVersionStatus_ = 0;
	
	PKT_C2S_ValidateConnectingPeer_s n;
	n.protocolVersion = P2PNET_VERSION;
	n.sessionId       = sessionId;
	p2pSendToHost(NULL, &n, sizeof(n));

	if( !DoConnectScreen( this, &ClientGameLogic::wait_ValidateVersion, gLangMngr.getString("WaitValidatingClientVersion"), 30.f ) )
	{
		r3dOutToLog("can't check game server version");
		return 0;
	}
	
	// if invalid version
	if(serverVersionStatus_ == 2)
	{
		return 0;
	}
	
	m_sessionId = sessionId;

	return 1;
}

int ClientGameLogic::WaitForLevelName()
{
	if( !DoConnectScreen( this, &ClientGameLogic::wait_GetLevelName, gLangMngr.getString("WaitRetrievengLevelName"), 10.f ) )
	{
		r3dOutToLog("can't get level name\n");
		return 0;
	}

	return 1;
}

int ClientGameLogic::RequestToJoinGame()
{
	r3d_assert(!gameJoinAnswered_);
	r3d_assert(localPlayer_ == NULL);
	r3d_assert(localPlayerIdx_ == -1);

	PKT_C2S_JoinGameReq_s n;
	n.CustomerID  = gUserProfile.CustomerID;
	n.SessionID   = gUserProfile.SessionID;
	p2pSendToHost(NULL, &n, sizeof(n));

	if( !DoConnectScreen( this, &ClientGameLogic::wait_GameJoin, gLangMngr.getString("WaitJoinGame"), 10.f ) )
	{
		r3dOutToLog("RequestToJoinGame failed\n");
		return 0;
	}

	r3d_assert(localPlayerIdx_ != -1);
#ifndef FINAL_BUILD
	r3dOutToLog("joined as player %d\n", localPlayerIdx_);
#endif

	return 1;
}

bool ClientGameLogic::wait_GameStart() 
{
	if(!gameStartAnswered_)
		return false;
		
	if(gameStartResult_ == PKT_S2C_StartGameAns_s::RES_Pending)
	{
		gameStartAnswered_ = false; // reset flag to force wait for next answer

		//TODO: make a separate timer to send new queries
		r3dOutToLog("retrying start game request\n");
		::Sleep(500);

		PKT_C2S_StartGameReq_s n;
		n.lastNetID = net_lastFreeId;
		n.requestSpectator = requestToJoinAsSpectator;
		p2pSendToHost(NULL, &n, sizeof(n));
		return false;
	}

	return true;
}

int ClientGameLogic::RequestToStartGame()
{
	r3d_assert(localPlayerIdx_ != -1);
	r3d_assert(localPlayer_ == NULL);

	PKT_C2S_StartGameReq_s n;
	n.lastNetID = net_lastFreeId;
	n.requestSpectator = requestToJoinAsSpectator;
	p2pSendToHost(NULL, &n, sizeof(n));

	if( !DoConnectScreen( this, &ClientGameLogic::wait_GameStart, gLangMngr.getString("WaitGameStart"), 20.f ) )
	{
		r3dOutToLog("can't start game, timeout\n");
		return 0;
	}
	
	if(gameStartResult_ != PKT_S2C_StartGameAns_s::RES_Ok)
	{
		r3dOutToLog("can't start game, res: %d\n", gameStartResult_);
		return 0;
	}

	return 1;
}


void ClientGameLogic::SendScreenshotFailed(int code)
{
	r3d_assert(code > 0);
	
	PKT_C2S_ScreenshotData_s n;
	n.errorCode = (BYTE)code;
	n.dataSize  = 0;
	p2pSendToHost(NULL, &n, sizeof(n));
}

extern IDirect3DTexture9* _r3d_screenshot_copy;
void ClientGameLogic::SendScreenshot()
{
	r3d_assert(_r3d_screenshot_copy);

	HRESULT hr;
	IDirect3DSurface9* pSurf0 = NULL;
	hr = _r3d_screenshot_copy->GetSurfaceLevel(0, &pSurf0);

	ID3DXBuffer* pData = NULL;
	hr = D3DXSaveSurfaceToFileInMemory(&pData, D3DXIFF_JPG, pSurf0, NULL, NULL);
	SAFE_RELEASE(pSurf0);
	
	// release saved screenshot copy
	SAFE_RELEASE(_r3d_screenshot_copy);
	
	if(pData == NULL || pData->GetBufferSize() > 0xF000) {
		SAFE_RELEASE(pData);
		SendScreenshotFailed(4);
		return;
	}

  // assemble packet and send it to host
	int   pktsize = sizeof(PKT_C2S_ScreenshotData_s) + pData->GetBufferSize();
	char* pktdata = new char[pktsize + 1];

	PKT_C2S_ScreenshotData_s n;
	n.errorCode = 0;
	n.dataSize  = (WORD)pData->GetBufferSize();
	memcpy(pktdata, &n, sizeof(n));
	memcpy(pktdata + sizeof(n), pData->GetBufferPointer(), n.dataSize);
	p2pSendToHost(NULL, (DefaultPacket*)pktdata, pktsize);

	delete[] pktdata;
	SAFE_RELEASE(pData);
	return;
}

void ClientGameLogic::Tick()
{
	R3DPROFILE_FUNCTION("ClientGameLogic::Tick");
	if(net_)
		net_->Update();

	if(!serverConnected_)
		return;

	// update ping
	if(hudMain && net_ && net_->lastPing_ >= 0)
	{
		hudMain->setPing(net_->lastPing_/2); // ptumik: div by 2 to show ping one way (as it is in browse games), otherwise it is round trip time :)
	}

	// update airstrike cooldowns
	{
		for(int i=0; i<getNumAirstrikes(); ++i)
		{
			AirstrikeDataset* AS = getAirstrike(i);
			if(AS->CurrentCooldown[0]>0)
				AS->CurrentCooldown[0] = R3D_MAX((AS->CurrentCooldown[0]-r3dGetFrameTime()), 0.0f);
			if(AS->CurrentCooldown[1]>0)
				AS->CurrentCooldown[1] = R3D_MAX((AS->CurrentCooldown[1]-r3dGetFrameTime()), 0.0f);
		}
		if(AIRSTRIKE_Team_Cooldown[0]>0)
			AIRSTRIKE_Team_Cooldown[0] = R3D_MAX((AIRSTRIKE_Team_Cooldown[0]-r3dGetFrameTime()), 0.0f);
		if(AIRSTRIKE_Team_Cooldown[1]>0)
			AIRSTRIKE_Team_Cooldown[1] = R3D_MAX((AIRSTRIKE_Team_Cooldown[1]-r3dGetFrameTime()), 0.0f);
	}

	// every <N> sec client must send his security report
	const DWORD curTicks = GetTickCount();
	if(curTicks >= nextSecTimeReport_) 
	{
		nextSecTimeReport_ = curTicks + (PKT_C2S_SecurityRep_s::REPORT_PERIOD * 1000);
		PKT_C2S_SecurityRep_s n;
		// store current time for speed hack detection
		n.gameTime = (float)curTicks / 1000.0f;
		n.detectedWireframeCheat = 0;
		n.GPP_Crc32 = GPP->GetCrc32() ^ gppDataSeed_;

		p2pSendToHost(NULL, &n, sizeof(n), true);
	}

	if(m_gameHasStarted && localPlayer_ && net_ && net_->lastPing_>500)
	{
		m_highPingTimer += r3dGetFrameTime();
		if(m_highPingTimer>60) // ping > 500 for more than 60 seconds -> disconnect
		{
			Disconnect();
			return;
		}
	}
	else
		m_highPingTimer = 0;

	/*if(showCoolThingTimer > 0 && m_gameHasStarted && localPlayer_ && !localPlayer_->bDead && ((r3dGetTime()-m_gameLocalStartTime))>showCoolThingTimer && m_gameInfo.practiceGame == false )
	{
		showCoolThingTimer = 0;
		char titleID[64];
		char textID[64];
		int tID = int(u_GetRandom(1.0f, 10.99f));
		int txtID = int(u_GetRandom(1.0f, 2.99f));
		sprintf(titleID, "tmpYouGotCoolThingTitle%d", tID);
		sprintf(textID, "tmpYouGotCoolThingText%d_%d", tID, txtID);
		hudMain->showYouGotCoolThing(gLangMngr.getString(titleID), "$Data/Weapons/StoreIcons/WPIcon.dds");
	}*/
	
	// send d3d cheat screenshot once per game
	if(_r3d_screenshot_copy && !d3dCheatSent_)
	{
		d3dCheatSent_ = true;
		SendScreenshot();
	}
	
	return;
}

bool canDamageTarget(const GameObject* obj);
// applies damage from local player
// Note: Direction tells us if it's a directional attack as well as how much. Remember to half the arc you want. (180 degree arc, is 90 degrees)  
// 		ForwVector is the direction.  ForwVector needs to be normalized if used.
void ClientGameLogic::ApplyExplosionDamage( const r3dVector& pos, float radius, int wpnIdx, const r3dVector& forwVector /*= FORWARDVECTOR*/, float direction /*= 360*/ )
{

	// WHEN WE MOVE THIS TO THE SERVER: Check the piercability of the gameobjects hit, and apply that to the explosion. 
	if(localPlayer_ == NULL)
		return;

	float radius_incr = 1.0f;
	int ExpRadiusSkill = localPlayer_->CurLoadout.getSkillLevel(CUserSkills::SPEC_ExplosiveRadius);
	switch(ExpRadiusSkill)
	{
	case 1: radius_incr = 1.10f; break;
	case 2: radius_incr = 1.15f; break;
	case 3: radius_incr = 1.20f; break;
	case 4: radius_incr = 1.25f; break;
	default:break;
	}

	// apply damage within a radius
	ObjectManager& GW = GameWorld();
	for(GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
	{
		if(canDamageTarget(obj))
		{
			r3d_assert(obj->NetworkID);
			obj_AI_Player* objPlayer = NULL;
			if(obj->ObjTypeFlags & OBJTYPE_Human)
				objPlayer = (obj_AI_Player*)obj;

			// skip friendly fire, but deal damage to yourself to prevent obvious shooting RPG when enemy is right next to you
			if(objPlayer)
				if((m_gameInfo.friendlyFire == 0) && (localPlayer_->TeamID == objPlayer->TeamID) && (objPlayer!=localPlayer_))
					continue;

			float dist_to_obj_sq = (obj->GetPosition() - pos).LengthSq();
			if(dist_to_obj_sq < ( radius * radius_incr ) * ( radius * radius_incr ) )
			{
				// raycast to make sure that player isn't behind a wall
				r3dPoint3D orig = r3dPoint3D(obj->GetPosition().x, obj->GetPosition().y+2.0f, obj->GetPosition().z);
				r3dPoint3D dir = r3dPoint3D(pos.x-obj->GetPosition().x, pos.y-(obj->GetPosition().y + 2.0f), pos.z - obj->GetPosition().z);
				float rayLen = dir.Length();
				dir.Normalize();
				BYTE damagePercentage = 100;
				float minDotDirection = cos( R3D_DEG2RAD( direction ) );
				if( direction == FULL_AREA_EXPLOSION || dir.Dot( forwVector ) > minDotDirection ) {

					if(rayLen > 0)
					{
						PxRaycastHit hit;
						PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK,0,0,0), PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::eSTATIC);
						if(g_pPhysicsWorld->raycastSingle(PxVec3(orig.x, orig.y, orig.z), PxVec3(dir.x, dir.y, dir.z), rayLen, PxSceneQueryFlag::eIMPACT, hit, filter))
						{
							// check distance to collision
							float len = r3dPoint3D(hit.impact.x-obj->GetPosition().x, hit.impact.y-(obj->GetPosition().y+2.0f), hit.impact.z-obj->GetPosition().z).Length();
							if((len+0.01f) < rayLen)
							{
								// human is behind a wall						
								PhysicsCallbackObject* target;
								if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
								{
									// this currently only handles one piercable object between the player and explosion.  More complexity might be valid here. 
									GameObject* obj = target->isGameObject();
									if ( obj )
									{
										damagePercentage = obj->m_BulletPierceable;
									}
								}
							}
						}
					}

					// send damage to server
					PKT_C2S_Temp_Damage_s n;
					n.targetId = toP2pNetId(obj->NetworkID);
					n.wpnIdx = (BYTE)wpnIdx;
					n.damagePercentage = damagePercentage; 
					n.explosion_pos = pos;
					p2pSendToHost(localPlayer_, &n, sizeof(n));
				}
			}
		}
	}
}
/*
int	ClientGameLogic::CheckTeamSkillAvailable(obj_AI_Player* plr, int skill)
{
	r3d_assert(plr);
	r3d_assert(skill >= 0 && skill < 10);
	
	int highestSkillAvalable = 0;
	for(int i=0; i<CurMaxPlayerIdx; ++i)
	{
		if(players_[i] && players_[i]->TeamID == plr->TeamID)
		{
			float radius = 0;
			if(players_[i]->m_Skills[skill] == 4)
				radius = 15.0f;
			else if(players_[i]->m_Skills[skill] == 5)
				radius = 25.0f;
			if(radius>0)
			{
				if((players_[i]->GetPosition() - plr->GetPosition()).LengthSq() < (radius*radius))
					highestSkillAvalable = R3D_MAX(highestSkillAvalable, players_[i]->m_Skills[skill]); // save skill level
			}			
		}
	}
	return highestSkillAvalable;
}*/

bool ClientGameLogic::CheckTeamAbilityAvailable(const obj_AI_Player* teamPlayer, int ability, float radius) const
{
	r3d_assert(teamPlayer);
	if(m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		return false;

	for(int i=0; i<CurMaxPlayerIdx; ++i)
	{
		const obj_AI_Player* plr = GetPlayer(i);
		if(plr && plr->TeamID == teamPlayer->TeamID && plr!=teamPlayer) // same team and not you
		{
			if((plr->GetPosition() - teamPlayer->GetPosition()).LengthSq() < (radius*radius))
				if( plr->CurLoadout.hasItem(ability))
					return true;
		}
	}
	return false;
}
