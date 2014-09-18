#include "r3dPCH.h"
#include "r3d.h"
#include "GameLevel.h"

#include "ServerGameLogic.h"
#include "MasterServerLogic.h"
#include "GameObjects/ObjManag.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "ObjectsCode/obj_ServerControlPoint.h"
#include "ObjectsCode/obj_ServerUAV.h"
#include "ObjectsCode/obj_ServerVehicle.h"
#include "ObjectsCode/obj_ServerMine.h"
#include "ObjectsCode/obj_ServerLightMesh.h"
#include "ObjectsCode/obj_ServerMedKit.h"
#include "ObjectsCode/obj_ServerMotionSensor.h"
#include "ObjectsCode/obj_ServerRespawnBeacon.h"
#include "ObjectsCode/Gameplay/BaseControlPoint.h"
#include "ObjectsCode/obj_ServerSiegeObjective.h"
#include "ObjectsCode/obj_ServerBombPlacement.h"
#include "ObjectsCode/obj_ServerPermWeaponDrop.h"
#include "ObjectsCode/obj_ServerWeaponDrop.h"
#include "ObjectsCode/obj_ServerLootBox.h"
#include "ObjectsCode/obj_ServerRiotShield.h"
#include "ObjectsCode/obj_ServerAutoTurret.h"

#include "../EclipseStudio/Sources/GameCode/UserProfile.h"
#include "../EclipseStudio/Sources/ObjectsCode/Gameplay/BattleZone.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponArmory.h"
#include "../EclipseStudio/Sources/ObjectsCode/AIRSTRIKES/Airstrike.h"

#include "ServerWeapons/ServerWeapon.h"

#include "AsyncFuncs.h"

ServerGameLogic	gServerLogic;

static	const float	HOST_TIME_SYNC_DELAY	= 1.0f;

#define MIN_SPAWN_DISTANCE_TO_ENEMY (25.0f) 

CVAR_FLOAT(	_glm_SpawnRadius,	 1.0f, "");
CVAR_FLOAT(	_glm_bzOutKillTime,	 5.0f, "battle zone out kill time");

CVAR_FLOAT(	_glmConq_BleedDelay,	 1.0f,	""); // if you change this varialbe, update this fn: UpdateGameLogic_UpdateTickets()

extern	__int64 cfg_sessionId;

#include "../EclipseStudio/Sources/Gameplay_Params.h"
	CGamePlayParams		GPP_Data;
	DWORD			GPP_Seed = GetTickCount();	// seed used to xor CRC of gpp_data



static bool IsNullTerminated(const char* data, int size)
{
  for(int i=0; i<size; i++) {
    if(data[i] == 0)
      return true;
  }

  return false;
}

//
//
//
//
static void preparePacket(const GameObject* from, DefaultPacket* packetData)
{
	r3d_assert(packetData);
	r3d_assert(packetData->EventID >= 0);

	if(from) {
		r3d_assert(from->NetworkID);
		//r3d_assert(from->NetworkLocal);

		packetData->FromID = toP2pNetId(from->NetworkID);
	} else {
		packetData->FromID = 0; // world event
	}

	return;
}

ServerGameLogic::ServerGameLogic()
{
	maxPlayers_ = 0;
	curPlayers_ = 0;
	curPeersConnected = 0;

	// init index to players table
	for(int i=0; i<MAX_NUM_PLAYERS; i++) {
		plrToPeer_[i] = NULL;
	}

	// init peer to player table
	for(int i=0; i<MAX_PEERS_COUNT; i++) {
		peers_[i].Clear();
	}
	
	memset(&netRecvPktSize, 0, sizeof(netRecvPktSize));
	memset(&netSentPktSize, 0, sizeof(netSentPktSize));

	net_lastFreeId    = NETID_OBJECTS_START;
	net_mapLoaded_LastNetID = 0;
	
	for(int i=0; i<4; i++)
		testDamageDummy_[i] = NULL;
	
	chatLogFile_ = NULL;
	
	weaponStats_.reserve(128);
	

}

ServerGameLogic::~ServerGameLogic()
{
}

void ServerGameLogic::Init(const GBGameInfo& ginfo, uint32_t creatorID)
{
	r3dOutToLog("Game: Initializing with %d players\n", ginfo.maxPlayers); CLOG_INDENT;
	r3d_assert(curPlayers_ == 0);
	r3d_assert(curPeersConnected == 0);

	creatorID_	= creatorID;
	ginfo_      = ginfo;
	maxPlayers_ = ginfo.maxPlayers;
	curPlayers_ = 0;
	curPeersConnected = 0;

	bomb_droppedPos.Assign(0,0,0);
	bomb_isDropped = false;
	bomb_nextRoundStartTimer = 0;

	// init GLM_Conquest vars
	tickets_[0] = ginfo.startTickets;
	tickets_[1] = ginfo.startTickets;
	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		tickets_[0] = 0;
		tickets_[1] = 0;
	}
	nextTicketUpdate_ = r3dGetTime() + _glmConq_BleedDelay;
	nextScoreUpdate_  = r3dGetTime();

	gameNoPeersConnectedTime = 0;
	gameStartCountdown = 0;
	m_requestGameStart = false;
	gameStartTime_ = r3dGetTime();
	gameEndTime_   = r3dGetTime() + (float)ginfo_.timeLimit * 60.0f;
	gameFinished_  = false;
	respawnDelay_  = (float)ginfo_.respawnDelay;
	m_isGameHasStarted   = ginfo_.autoBalance ? false : true;
	m_isTestingGame= false;
	
	weaponDataUpdates_ = 0;

	for(int i=0; i<MAX_AIRSTRIKES_IN_FLIGHT; ++i)
		g_AirstrikesInFlight[i].Reset();
	AIRSTRIKE_Team_Cooldown[0] = 0;
	AIRSTRIKE_Team_Cooldown[1] = 0;
	
	//m_isTestingGame= true; //@

	return;
}

// this function is also called when enough players are joined a game to reset counter
void ServerGameLogic::StartGame(bool resetPlayerTime)
{
	// reset game start time.
	gameStartTime_ = r3dGetTime();
	gameEndTime_   = r3dGetTime() + (float)ginfo_.timeLimit * 60.0f;

	nextTicketUpdate_ = r3dGetTime() + _glmConq_BleedDelay;

	// mark playtime only after match started
	if(resetPlayerTime)
	{
		for(int i=0; i<maxPlayers_; i++) 
		{
			obj_ServerPlayer* plr = GetPlayer(i);
			if(!plr) continue;

			plr->startPlayTime_ = r3dGetTime();
			// also reset stats when game has started
			plr->RoundStats_.Reset();
			plr->DetailedReward_ = wiStatsTracking();

		}
	}

	return;
}

void ServerGameLogic::CreateHost(int port)
{
	r3dOutToLog("Starting server on port %d\n", port);

	g_net.Initialize(this, "p2pNet");
	g_net.CreateHost(port, MAX_PEERS_COUNT);
	//g_net.dumpStats_ = 2;

	return;
}

void ServerGameLogic::Disconnect()
{
	r3dOutToLog("Disconnect server\n");
	g_net.Deinitialize();

	return;
}

void ServerGameLogic::CheckClientsSecurity()
{
  const float PEER_CHECK_DELAY = 0.2f;	// do checks every N seconds
  const float IDLE_PEERS_DELAY = 5.0f;	// peer have this N seconds to validate itself
  const float SECREP1_DELAY    = PKT_C2S_SecurityRep_s::REPORT_PERIOD*4; // x4 time of client reporting time (15sec) to receive security report

  static float nextCheck = -1;
  const float curTime = r3dGetTime();
  if(curTime < nextCheck)
    return;
  nextCheck = curTime + PEER_CHECK_DELAY;
  
  for(int peerId=0; peerId<MAX_PEERS_COUNT; peerId++) 
  {
    const peerInfo_s& peer = peers_[peerId];
    if(peer.status_ == PEER_FREE)
      continue;
      
    // check againts not validated peers
    if(peer.status_ == PEER_CONNECTED)
    {
      if(curTime < peer.startTime + IDLE_PEERS_DELAY)
        continue;
      
      DisconnectPeer(peerId, false, "no validation, last:%f/%d", peer.lastPacketTime, peer.lastPacketId);
      continue;
    }
    
    // check for receiveing security report
    if(peer.player != NULL)
    {
      if(curTime > peer.secRepRecvTime + SECREP1_DELAY) {
        DisconnectPeer(peerId, false, "no SecRep, last:%f/%d", peer.lastPacketTime, peer.lastPacketId);
        continue;
      }
    }
  }
  
  return;
}

void ServerGameLogic::LogInfo(DWORD peerId, const char* msg, const char* fmt, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap, fmt);
	StringCbVPrintfA(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	
	LogCheat(peerId, 0, false, msg, buf);
}

void ServerGameLogic::LogCheat(DWORD peerId, int LogID, int disconnect, const char* msg, const char* fmt, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap, fmt);
	StringCbVPrintfA(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	const peerInfo_s& peer = GetPeer(peerId);
	DWORD IP = net_->GetPeerIp(peerId);

	extern int cfg_uploadLogs;
	if(cfg_uploadLogs)
	{
		// report that string to server, 'li' will be deleted in thread
		LogInfo_s* li = new LogInfo_s();
		li->CheatID    = LogID;
		li->CustomerID = peer.CustomerID;
		li->IP         = IP;
		r3dscpy(li->Msg, msg);
		r3dscpy(li->Data, buf);
  		CreateThread(NULL, 0, AddLogInfoThread, li, 0, NULL);
  	}
  	
  	const char* screenname = "<NOT_CONNECTED>";
  	if(peer.status_ == PEER_PLAYING)
  		screenname = peer.temp_profile.ScreenName;

	r3dOutToLog("%s: peer%02d, r:%s %s, CID:%d [%s], ip:%s\n", 
		LogID > 0 ? "!!! cheat" : "LogInfo",
		peerId, 
		msg, buf,
		peer.CustomerID, screenname,
		inet_ntoa(*(in_addr*)&IP));

	if(disconnect && peer.player)
	{
		// tell client he's cheating.
		// ptumik: no need to make it easier to hack
		//PKT_S2C_CheatWarning_s n2;
		//n2.cheatId = (BYTE)cheatId;
		//p2pSendToPeer(peerId, NULL, &n2, sizeof(n2));

		net_->DisconnectPeer(peerId);
		// fire up disconnect event manually, enet might skip if if other peer disconnect as well
		OnNetPeerDisconnected(peerId);
	}
  
	return;
}

void ServerGameLogic::DisconnectPeer(DWORD peerId, bool cheat, const char* fmt, ...)
{
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  StringCbVPrintfA(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  LogCheat(peerId, cheat ? 99 : 0, false, "DisconnectPeer", buf);

  net_->DisconnectPeer(peerId);
  
  // fire up disconnect event manually, enet might skip if if other peer disconnect as well
  OnNetPeerDisconnected(peerId);
}

void ServerGameLogic::OnNetPeerConnected(DWORD peerId)
{
	if(gameFinished_)
	{
		r3dOutToLog("peer connected while game is finished\n");
		return;
	}
		
	r3d_assert(maxPlayers_ > 0);

	r3dOutToLog("peer%02d connected\n", peerId);
	CLOG_INDENT;

	peerInfo_s& peer = GetPeer(peerId);
	peer.SetStatus(PEER_CONNECTED);

	curPeersConnected++;
	return;
}

void ServerGameLogic::OnNetPeerDisconnected(DWORD peerId)
{
	r3dOutToLog("peer%02d disconnected\n", peerId);
	CLOG_INDENT;

	peerInfo_s& peer = GetPeer(peerId);

	// debug validation
	switch(peer.status_)
	{
	default: 
		r3dError("!!! Invalid status %d in disconnect !!!", peer.status_);
		break;
	case PEER_FREE:
		break;

	case PEER_CONNECTED:
	case PEER_VALIDATED1:
		r3d_assert(peer.player == NULL);
		r3d_assert(peer.playerIdx == -1);
		break;

	case PEER_LOADING:
		r3d_assert(peer.playerIdx != -1);
		r3d_assert(plrToPeer_[peer.playerIdx] != NULL);
		r3d_assert(peer.player == NULL);
		
		//@TODO: make a event to stop each getprofile thread
		::TerminateThread(peer.getProfileH, 0);

		plrToPeer_[peer.playerIdx] = NULL;
		break;

	case PEER_PLAYING:
		r3d_assert(peer.playerIdx != -1);
		r3d_assert(plrToPeer_[peer.playerIdx] != NULL);
		r3d_assert(plrToPeer_[peer.playerIdx] == &peer);
		//r3d_assert(peer.player != NULL); if disconnected on respawn menu before entering game
		if(peer.player)
		{
			if(ginfo_.mapType == GBGameInfo::MAPT_Bomb && m_isGameHasStarted && peer.player->m_carryingBomb)
			{
				// player that was carrying bomb was disconnected, so give bomb to the next player. we cannot drop it, as player might be in the air
				obj_ServerPlayer* nxtPlr = NULL;
				for(int j=0; j<MAX_NUM_PLAYERS; ++j)
				{
					obj_ServerPlayer* pl = GetPlayer(j);
					if(pl && pl->TeamID==0 && pl!=peer.player && !pl->isDead) // new player should be terrorist and not dead to get bomb
					{
						nxtPlr = pl;
						break;
					}
				}
				if(nxtPlr)
				{
					PKT_S2C_Bomb_PlayerHasBomb_s pkt;
					pkt.playerID = toP2pNetId(nxtPlr->NetworkID);
					nxtPlr->m_carryingBomb = true;

					for(int j=0; j<MAX_PEERS_COUNT; ++j)
					{
						peerInfo_s& peerInfo = GetPeer(j);
						obj_ServerPlayer* pl = peerInfo.player;
						if(peerInfo.status_ == PEER_PLAYING && pl!=peer.player) 
							p2pSendToPeer(j, NULL, &pkt, sizeof(pkt), true);
					}
				}
			}
			DeletePlayer(peer.playerIdx, peer.player);
		}

		if(ginfo_.mapType == GBGameInfo::MAPT_Bomb && !m_isGameHasStarted) // if still in lobby
		{
			// notify other peers that this one disconnected
			PKT_S2C_Bomb_DisconnectedPlayer_s n;
			n.peerID = peerId;
			for(int i=0; i<MAX_PEERS_COUNT; ++i)
			{
				if(i==peerId)
					continue;

				peerInfo_s& peerInfo = GetPeer(i);
				if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL)
				{
					p2pSendToPeer(i, NULL, &n, sizeof(n));
				}
			}

			// check if it was creator, if yes, designate other player as creator
			if(peer.CustomerID == creatorID_)
			{
				for(int i=0; i<MAX_PEERS_COUNT; ++i)
				{
					if(i==peerId)
						continue;

					peerInfo_s& peerInfo = GetPeer(i);
					if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL)
					{
						r3dOutToLog("Changed creatorID to %u, previous disconnected\n", peerInfo.CustomerID);
						creatorID_ = peerInfo.CustomerID;

						PKT_S2C_Bomb_ConnectedPlayer_s new_player_packet;
						new_player_packet.peerID = i;
						new_player_packet.teamID = peerInfo.desiredTeam;
						new_player_packet.isReady = peerInfo.isPlayerReady;
						new_player_packet.isSpectator = peerInfo.isPlayerSpectator;
						new_player_packet.plrLevel = (BYTE)peerInfo.temp_profile.ProfileData.Stats.getRankLevel();
						r3dscpy(new_player_packet.userName, peerInfo.temp_profile.ScreenName);
						new_player_packet.isMaster = 1;
						
						p2pSendToPeer(i, NULL, &new_player_packet, sizeof(new_player_packet));
						break;
					}
				}
			}
		}

		plrToPeer_[peer.playerIdx] = NULL;
		break;
	}

	if(peer.status_ != PEER_FREE)
	{
		// OnNetPeerDisconnected can fire multiple times, because of forced disconnect
		curPeersConnected--;
	}

	// clear peer status
	peer.Clear();

	return;
}

void ServerGameLogic::OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize)
{
	// we can receive late packets from logically disconnected peer.
	peerInfo_s& peer = GetPeer(peerId);
	if(peer.status_ == PEER_FREE)
		return;
		
	r3d_assert(packetSize >= sizeof(DefaultPacket));
	const DefaultPacket* evt = static_cast<const DefaultPacket*>(packetData);

	// store last packet data for debug
	peer.lastPacketTime = r3dGetTime();
	peer.lastPacketId   = evt->EventID;
	
	// store received sizes by packets
	if(evt->EventID < 256)
		netRecvPktSize[evt->EventID] += packetSize;

	if(gameFinished_)
	{
		r3dOutToLog("!!! peer%02d got packet %d while game is finished\n", peerId, evt->EventID);
		return;
	}

	GameObject* fromObj = NULL;
	if(evt->FromID != 0) {
		for(GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj)) {
			if(obj->NetworkID == evt->FromID) {
				fromObj = obj;
				break;
			}
		}
	}

	//r3dOutToLog("OnNetData from peer%d, obj:%d(%s), event:%d\n", peerId, evt->FromID, fromObj ? fromObj->Name.c_str() : "", evt->EventID);

	// pass to world even processor first.
	if(ProcessWorldEvent(fromObj, evt->EventID, peerId, packetData, packetSize)) {
		return;
	}

	if(evt->FromID && fromObj == NULL) {
		r3dOutToLog("!!! bad event %d sent from non registered object %d\n", evt->EventID, evt->FromID);
		return;
	}

	if(fromObj) 
	{
		if(IsServerPlayer(fromObj)) {
			// make sure that sender of that packet is same player on server
			if(((obj_ServerPlayer*)fromObj)->peerId_ != peerId) {
				DisconnectPeer(peerId, true, "message from different obj_ServerPlayer. Rcvd peerID: %d, player's peerID: %d, packetID: %d", peerId, ((obj_ServerPlayer*)fromObj)->peerId_, evt->EventID);
				return;
			}
		}

		if(!fromObj->OnNetReceive(evt->EventID, packetData, packetSize)) {
			r3dError("bad event %d for %s", evt->EventID, fromObj->Class->Name.c_str());
		}
		return;
	}

	r3dError("bad world event %d", evt->EventID);
	return;
}

void ServerGameLogic::RewardKillPlayer(obj_ServerPlayer* fromPlr, obj_ServerPlayer* targetPlr, STORE_CATEGORIES dmgCat)
{
	r3d_assert(fromPlr);
	r3d_assert(targetPlr);
	r3dOutToLog("RewardKillPlayer %s->%s\n", fromPlr->userName, targetPlr->userName); CLOG_INDENT;

	if(!m_isGameHasStarted)
		return; // do not reward players while game hasn't started
	
	if(targetPlr->TeamID == fromPlr->TeamID)
	{
		AddPlayerReward(fromPlr, RWD_FriendlyKill);
		return; // do not give any other rewards if this is a friendly kill do not count it as a kill. 
	}
	
	fromPlr->RoundStats_.Kills++;
	if(targetPlr->lastHitBodyPart == 1)
		fromPlr->RoundStats_.Headshots++;

	// check if KD ratio is higher than 70, if yes - kick from server
	{
		float kd = 0.0f;
		if(fromPlr->RoundStats_.Deaths > 0)
			kd = (float)fromPlr->RoundStats_.Kills / (float)fromPlr->RoundStats_.Deaths;
		else 
			kd = (float)fromPlr->RoundStats_.Kills;

		if(kd >= 70.0f)
		{
			LogCheat(fromPlr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_HighKDRatio, true, "HighKDRatio",
				"k:%d d:%d",
				fromPlr->RoundStats_.Kills, fromPlr->RoundStats_.Deaths
				);
			return;
		}
	}

	fromPlr->numKillWithoutDying++;

	// check if killed within base spawn point
	// if within 30 meter radius of spawn point and within 15 seconds
	if(ginfo_.mapType == GBGameInfo::MAPT_Conquest)
	{
		if((targetPlr->GetPosition()-targetPlr->lastRespawnPosAtBase).Length() < 30.0f && (r3dGetTime()-targetPlr->lastRespawnTimeAtBase)<15.0f)
		{
			AddPlayerReward(fromPlr, RWD_KillAtSpawn);
			return;
		}
	}

	for ( int targetIndex = 0; targetIndex < targetPlr->numOfUAVHits; targetIndex++ ) 
	{
		// UAV assist (do not award assist if killer was the one who marked target)
		if( targetPlr->listOfUAVHits[targetIndex].UAVTargetedBy !=invalidGameObjectID && targetPlr->listOfUAVHits[targetIndex].UAVTargetedBy != fromPlr->GetSafeID() && ( r3dGetTime() - targetPlr->listOfUAVHits[targetIndex].UAVTargetedByTime )<10.0f)
		{
			obj_ServerPlayer* assistedPlr = IsServerPlayer(GameWorld().GetObject(targetPlr->listOfUAVHits[targetIndex].UAVTargetedBy ));
			if( assistedPlr )
			{
				AddPlayerReward(assistedPlr, RWD_UAVAssist);
			}
		}	
	}

	
	if(fromPlr->numKillWithoutDying == 5)
		AddPlayerReward(fromPlr, RWD_Kill5NotDying);
	else if(fromPlr->numKillWithoutDying == 10)
		AddPlayerReward(fromPlr, RWD_Kill10NotDying);
	else if(fromPlr->numKillWithoutDying == 25)
		AddPlayerReward(fromPlr, RWD_Kill25NotDying);
	else if(fromPlr->numKillWithoutDying == 50)
		AddPlayerReward(fromPlr, RWD_Kill50NotDying);
	


	if(fromPlr->m_RequestedWeaponPickup == 0 && fromPlr->m_RequestedWeaponPickUpPrevOwner == targetPlr->GetSafeID())
	{	
		// reset, give award only once
		fromPlr->m_RequestedWeaponPickUpPrevOwner = invalidGameObjectID;
		AddPlayerReward(fromPlr, RWD_KillEnemyWithHisGun);
	}
	
	int gotKillReward = 0;
	if(targetPlr->TeamID != fromPlr->TeamID)
	{
		// count kill streaks
		if((r3dGetTime() - fromPlr->LastEnemyKillTime) < GPP_Data.c_fKillStreakTimeout)
			fromPlr->Killstreaks++;
		else
			fromPlr->Killstreaks = 1;

		if(fromPlr->Killstreaks == 2)
		{
			AddPlayerReward(fromPlr, RWD_Killstreak2);
			gotKillReward++;
		}
		else if(fromPlr->Killstreaks == 3)
		{
			AddPlayerReward(fromPlr, RWD_Killstreak3);
			gotKillReward++;
		}
		else if(fromPlr->Killstreaks == 4)
		{
			AddPlayerReward(fromPlr, RWD_Killstreak4);
			gotKillReward++;
		}
		else if(fromPlr->Killstreaks == 5)
		{
			AddPlayerReward(fromPlr, RWD_Killstreak5);
			gotKillReward++;
		}
		else if(fromPlr->Killstreaks >= 6)
		{
			AddPlayerReward(fromPlr, RWD_Killstreak6);
			gotKillReward++;
		}

		// update timer
		fromPlr->LastEnemyKillTime = r3dGetTime();
	}

	if(fromPlr->LastKilledBy == targetPlr->GetSafeID())
	{
		AddPlayerReward(fromPlr, RWD_RevengeKill);
		fromPlr->LastKilledBy = invalidGameObjectID; // reset, otherwise if you kill him again, you will get revenge kill again
		gotKillReward++;
	}
	targetPlr->LastKilledBy = fromPlr->GetSafeID();

	if((r3dGetTime() - targetPlr->LastEnemyKillTime) < 10.0f)
	{
		AddPlayerReward(fromPlr, RWD_AvengeKill);
		gotKillReward++;
	}

	CheckAchievementsOnKill(fromPlr, targetPlr, dmgCat);


	if(dmgCat==storecat_SUPPORT || dmgCat==storecat_GRENADES)
	{
		AddPlayerReward(fromPlr, RWD_ExplosionKill);
	}
	else if(targetPlr->lastHitBodyPart == 1 && dmgCat!=storecat_MELEE) // headshot
	{
		AddPlayerReward(fromPlr, RWD_Headshot);

		fromPlr->incrementAchievement( ACHIEVEMENT_300_HEADSHOTS, 1 );
	}
	else
	{
		if(!gotKillReward)
			AddPlayerReward(fromPlr, RWD_Kill);
	}

	if(fromPlr->TeamID != targetPlr->TeamID && ginfo_.mapType == GBGameInfo::MAPT_Conquest)
	{
		// Kill An Enemy within 25 meters of Your Flag
		for(int i=0; i<gCPMgr.NumCP(); ++i)
		{
			if(gCPMgr.GetCP(i)->GetSpawnTeamId() == fromPlr->TeamID && (gCPMgr.GetCP(i)->GetPosition()-targetPlr->GetPosition()).Length() < 25.0f)
			{
				AddPlayerReward(fromPlr, RWD_KillCloseToYourFlag);
				fromPlr->incrementAchievement( ACHIEVEMENT_DEFEND_5000_FLAGS, 1 );

				break;
			}
		}

		// Kill An Enemy within 35 meters of Their Flag 
		for(int i=0; i<gCPMgr.NumCP(); ++i)
		{
			if(gCPMgr.GetCP(i)->GetSpawnTeamId() == targetPlr->TeamID && (gCPMgr.GetCP(i)->GetPosition()-targetPlr->GetPosition()).Length() < 35.0f)
			{
				AddPlayerReward(fromPlr, RWD_KillCloseToTheirFlag);
				break;
			}
		}
	}

	// if you killed a higher ranking player
	/*int fromLevel = fromPlr->profile_.ProfileData.Stats.getRankLevel();
	int targetLevel = targetPlr->profile_.ProfileData.Stats.getRankLevel();
	int lvlDiff = targetLevel - fromLevel;
	if(lvlDiff>=10 && lvlDiff<20)
		reward.OffenseHP += 100;
	else if(lvlDiff>=20 && lvlDiff<30)
		reward.OffenseHP += 200;
	else if(lvlDiff>=30)
		reward.OffenseHP += 300;
	AddPlayerReward(fromPlr, reward, "KillReward");
	*/
}

// Returns: true if there was an assist, false if not. 
bool ServerGameLogic::RewardAssistedKills(obj_ServerPlayer* killedPlr)
{
	if( killedPlr == NULL ) {
		return false;
	}

	const float ASSISTED_KILL_TIMEOUT = 15.0f; // 15 seconds
	if(killedPlr->m_ShotFromHistory[1].networkID == -1)
		return false;
	if((r3dGetTime() - killedPlr->m_ShotFromHistory[1].timestamp) > ASSISTED_KILL_TIMEOUT)
		return false;
	
	GameObject* assistObj = GameWorld().GetNetworkObject(killedPlr->m_ShotFromHistory[1].networkID);
	if(!assistObj || assistObj == killedPlr)
		return false;
	
	if(!IsServerPlayer(assistObj))
		return false;
		
	obj_ServerPlayer* assistedKiller = (obj_ServerPlayer*)assistObj;
	if( assistedKiller->TeamID == killedPlr->TeamID )
	{
		return false;
	}
	
	assistedKiller->RoundStats_.AssistKills++;
	AddPlayerReward(assistedKiller, RWD_AssistedKill);
	
	return true;
}

static int plrScoreSort(const void* Idx1, const void* Idx2)
{
	int A = *(int*)Idx1;
	int B = *(int*)Idx2;
	return B-A;
}

bool ServerGameLogic::DropLootBox(obj_ServerPlayer* killedPlr)
{
	if(killedPlr == NULL)
		return false;
		
	if(killedPlr->isTargetDummy_)
		return false;

	// fixed list of maps where we allow to drop loot boxes
	if(!(ginfo_.mapId == GBGameInfo::MAPID_WO_Crossroads16 ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Crossroads2 ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Grozny ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_EasternBunkerTDM ||
		ginfo_.mapId == GBGameInfo::MAPID_BurningSea ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Torn ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Nightfall_CQ ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_TornTown)
		)
		return false;

	if(curPlayers_ < 10)
		return false;

	// hard coded for now
	const uint32_t PREMIUM_LOOT_BOX1 = 301118;
	const uint32_t PREMIUM_LOOT_BOX2 = 301119;
	const uint32_t PREMIUM_LOOT_BOX3 = 301120;
	const uint32_t REGULAR_LOOT_BOX1 = 301121;
	const uint32_t REGULAR_LOOT_BOX2 = 301122;
	const uint32_t REGULAR_LOOT_BOX3 = 301123;

	int numPlrsInTeam = 0;
	int playersScore[MAX_NUM_PLAYERS] = {0};
	for(int i=0; i<MAX_NUM_PLAYERS; ++i)
	{
		if(plrToPeer_[i] && plrToPeer_[i]->player && plrToPeer_[i]->player->TeamID == killedPlr->TeamID)
		{
			playersScore[numPlrsInTeam] = plrToPeer_[i]->player->RoundStats_.HonorPoints;
			numPlrsInTeam++;
		}
	}
	// one more sanity check
	if(numPlrsInTeam < 3)
		return false;

	qsort(&playersScore[0], numPlrsInTeam, sizeof(int), plrScoreSort);
	
	// if player in top 20% on his team - then drop premium box
	int numPlrsInTop20 = (int)ceilf(float(numPlrsInTeam)*0.2f); // min 1 plr
	// sanity check
	if(numPlrsInTop20 < 1)
	{
		r3dOutToLog("!!! impossible shit happened\n");
		return false;
	}
	bool top20 = killedPlr->RoundStats_.HonorPoints >= playersScore[numPlrsInTop20-1];

	// roll dices
	uint32_t crateID = 0;
	if(top20)
	{
		while(1) // fake
		{
			const ItemConfig* ic = NULL;
			ic = gWeaponArmory.getItemConfig(PREMIUM_LOOT_BOX1);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = PREMIUM_LOOT_BOX1;
				break;
			}
			ic = gWeaponArmory.getItemConfig(PREMIUM_LOOT_BOX2);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = PREMIUM_LOOT_BOX2;
				break;
			}
			ic = gWeaponArmory.getItemConfig(PREMIUM_LOOT_BOX3);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = PREMIUM_LOOT_BOX3;
				break;
			}
			break; // exit while()
		}
	}
	else
	{
		while(1) // fake
		{
			const ItemConfig* ic = NULL;
			ic = gWeaponArmory.getItemConfig(REGULAR_LOOT_BOX1);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = REGULAR_LOOT_BOX1;
				break;
			}
			ic = gWeaponArmory.getItemConfig(REGULAR_LOOT_BOX2);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = REGULAR_LOOT_BOX2;
				break;
			}
			ic = gWeaponArmory.getItemConfig(REGULAR_LOOT_BOX3);
			if(ic && random(100) < ic->m_LevelRequired)
			{
				crateID = REGULAR_LOOT_BOX3;
				break;
			}
			break; // exit while()
		}
	}

	if(crateID == 0)
		return false;

	// spawn loot box
	obj_ServerLootBox* lootBox = (obj_ServerLootBox*)srv_CreateGameObject("obj_ServerLootBox", "lootBox", killedPlr->GetPosition());
	lootBox->itemID = crateID;
	lootBox->spawnTime = r3dGetTime();
	lootBox->SetRotationVector(r3dPoint3D(0,0,0));
	lootBox->NetworkID = net_lastFreeId++;

	PKT_S2C_SpawnDroppedLootBox_s n;
	n.spawnID = toP2pNetId(lootBox->NetworkID);
	n.pos = lootBox->GetPosition();
	n.rot = lootBox->GetRotationVector();
	n.itemID = crateID;
	p2pBroadcastToActive(NULL, &n, sizeof(n));

	return true;
}

void ServerGameLogic::DropWeapon(uint32_t itemID, const r3dVector& pos, const r3dVector& rot, uint32_t numBullets, gobjid_t prevOwner, bool isPermDrop, const wiWeaponAttachment& attms)
{
	r3dOutToLog("Dropped weapon %d with %d bullets\n", itemID, numBullets);

	obj_ServerWeaponDrop* droppedWeapon = (obj_ServerWeaponDrop*)srv_CreateGameObject("obj_ServerWeaponDrop", "weaponDrop", pos);
	droppedWeapon->itemID = itemID;
	droppedWeapon->spawnTime = r3dGetTime();
	droppedWeapon->SetRotationVector(rot);
	droppedWeapon->NetworkID = net_lastFreeId++;
	droppedWeapon->numBullets = numBullets;
	droppedWeapon->prevOwner = prevOwner;
	droppedWeapon->isPermanentDrop = isPermDrop;
	droppedWeapon->attms = attms;

	PKT_S2C_SpawnDroppedWeapon_s n;
	n.spawnID = toP2pNetId(droppedWeapon->NetworkID);
	n.pos = pos;
	n.rot = rot;
	n.itemID = itemID;
	n.numBullets = numBullets;
	n.attms = attms;
	p2pBroadcastToActive(NULL, &n, sizeof(n));
}

void ServerGameLogic::DoKillPlayer(GameObject* sourceObj, obj_ServerPlayer* targetPlr, STORE_CATEGORIES weaponCat, bool forced_by_server, bool fromPlayerInAir, bool targetPlayerInAir )
{
	r3dOutToLog("%s killed by %s, forced: %d\n", targetPlr->userName, sourceObj->Name.c_str(), (int)forced_by_server);

	// sent kill packet
	{		     
		PKT_S2C_KillPlayer_s n;
		n.targetId = toP2pNetId(targetPlr->NetworkID);
		n.killerWeaponCat = (BYTE)weaponCat;
		n.forced_by_server = forced_by_server;
		p2pBroadcastToActive(sourceObj, &n, sizeof(n));
	}

	// we going to give at least one full clip of bullets on weapon drop
	if(!forced_by_server && sourceObj != targetPlr) // do not reward suicides
	{
		if(!DropLootBox(targetPlr)) // if didn't drop loot box, then drop weapon
		{
			if(targetPlr->GetSelectedWeapon() < 3 && !targetPlr->m_DisableWeaponDrop) // can only drop weapon, not grenades
			{
				const Weapon* wpn = targetPlr->m_WeaponArray[targetPlr->GetSelectedWeapon()];
				int numBulletsLeftInWeapon = R3D_MAX(wpn->m_NumBulletsLeft, wpn->getClipSize());
				wiWeaponAttachment wpnAttms;
				wpn->getWeaponAttachmentIDs(wpnAttms.attachments);
				DropWeapon(targetPlr->getSelectedWeaponItemID(), targetPlr->GetPosition() + r3dVector(0,1,0), r3dVector(0,0,0), numBulletsLeftInWeapon, targetPlr->GetSafeID(), false, wpnAttms);
			}
		}
	}

	bool assisted = false;
	if(!forced_by_server)
	{
		assisted = RewardAssistedKills(targetPlr); // reward assist before DoDeath, as in DoDeath we will reset shot history
	}

	targetPlr->DoDeath();

	if(forced_by_server)
		return;
	
	targetPlr->RoundStats_.Deaths++;
	//AddPlayerReward(targetPlr, RWD_Death, "RWD_Death");

	// - Each time team member dies, team loses one ticket.   
	if(m_isGameHasStarted)
	{
		int loseteam = targetPlr->TeamID;
		if(ginfo_.mapType == GBGameInfo::MAPT_Deathmatch && m_isGameHasStarted)
		{
			// scale tickets according to how many players are in game, otherwise 16v16 are finishing very quickly
			if(curPlayers_ >= 24)
				tickets_[loseteam] -= 0.5f;
			else if(curPlayers_ >= 16)
				tickets_[loseteam] -= 1.0f;
			else if(curPlayers_ >= 8)
				tickets_[loseteam] -= 1.5f;
			else
				tickets_[loseteam] -= 2.0f;
		}
		else if(ginfo_.mapType == GBGameInfo::MAPT_Conquest) 
			tickets_[loseteam] -= 1.0f;

		if(ginfo_.mapType != GBGameInfo::MAPT_Bomb)
		{
			if(tickets_[loseteam] < 1.0f) tickets_[loseteam] = 0;
		}
	}
	
	// do not count suicide kills
	if(sourceObj == targetPlr)
		return;

	if(IsServerPlayer(sourceObj))
	{
		obj_ServerPlayer * fromPlr = ((obj_ServerPlayer*)sourceObj);
		if(m_FirstKill) {
			fromPlr->markAchievementComplete( ACHIEVEMENT_FIRST_KILL );
			m_FirstKill = false;
		}

		if( ginfo_.mapType == GBGameInfo::MAPT_Deathmatch ) {

			if( assisted ) {
				fromPlr->m_NumberAssistedKills++;
				fromPlr->checkAchievementByValue( ACHIEVEMENT_DM_KILL_7_ENEMIES_ASSISTED, fromPlr->m_NumberAssistedKills );
			}
		}

		// we're only looking at the first three weapons for this achievement.
		int usedWeaponIndex = -1 ;
		for(int weaponIndex=0; weaponIndex < 3; weaponIndex++)
		{
			if( fromPlr->m_WeaponArray[weaponIndex] == NULL )
				continue;
			
			if( fromPlr->m_WeaponArray[weaponIndex]->getCategory() == weaponCat )
			{
				fromPlr->inventoryIndexForKills |= 2 << weaponIndex;
			}
		}

		if ( fromPlr->inventoryIndexForKills == ((2 << 0) | (2 << 1)  | (2 << 2)) )
		{
			fromPlr->markAchievementComplete( ACHIEVEMENT_KILL_WITH_EVERY_WEAPON );
		}

		if ( targetPlr->m_HasDamagedSelf == true )
		{
			fromPlr->markAchievementComplete( ACHIEVEMENT_KILL_SELF_DAMAGED_ENEMY );
		}

		if ( targetPlayerInAir ) {
			fromPlr->markAchievementComplete( ACHIEVEMENT_KILL_ENEMY_IN_AIR );
		}

		if ( fromPlayerInAir ) {
			fromPlr->markAchievementComplete( ACHIEVEMENT_KILL_WHILE_IN_AIR );
		}


		m_LastKiller_PeerID = fromPlr->peerId_;

		RewardKillPlayer( fromPlr, targetPlr, weaponCat );

	}
	else if(sourceObj->isObjType(OBJTYPE_GameplayItem) && sourceObj->Class->Name == "obj_ServerAutoTurret")
	{
		// award kill to owner of the turret
		obj_ServerPlayer* turretOwner = IsServerPlayer(GameWorld().GetObject(sourceObj->ownerID));
		if(turretOwner)
		{
			turretOwner->RoundStats_.Kills++;
		}
	}

	return;
}

// make sure this function is the same on client: AI_Player.cpp bool canDamageTarget(const GameObject* obj)
bool ServerGameLogic::CanDamageThisObject(const GameObject* targetObj)
{
	if(IsServerPlayer(targetObj))
	{
		return true;
	}
	
	if(targetObj->Class->Name == "obj_ServerUAV")
	{
		// can only damage when it's not destroyed.
		// otherwise we can send hit packet to already destroyed object on client
		obj_ServerUAV* targetUav = (obj_ServerUAV*)targetObj;
		if(targetUav->state_ == obj_ServerUAV::UAV_Killed)
			return false;
		return true;
	}
	else if(targetObj->Class->Name == "obj_ServerMine")
	{
		obj_ServerMine* mine = (obj_ServerMine*)targetObj;
		if(mine->bDestroyed)
			return false;
		return true;
	}
	else if(targetObj->Class->Name == "obj_LightMesh")
	{
		obj_ServerLightMesh* lightM = (obj_ServerLightMesh*)targetObj;
		if(lightM->bLightOn)
			return true;
		return false;
	}
	else if(targetObj->Class->Name == "obj_ServerMotionSensor")
	{
		return true;
	}
	else if(targetObj->Class->Name == "obj_ServerRespawnBeacon")
	{
		return true;
	}
	else if(targetObj->Class->Name == "obj_ServerRiotShield")
	{
		return true;
	}
	else if(targetObj->Class->Name == "obj_ServerAutoTurret")
	{
		return true;
	}
	
	return false;
}

void ServerGameLogic::ApplyDamage(GameObject* fromObj, GameObject* targetObj, const r3dPoint3D& dmgPos, float damage, bool force_damage, STORE_CATEGORIES damageSource)
{
	r3d_assert(fromObj);
	r3d_assert(targetObj);
	
	if(IsServerPlayer(targetObj))
	{
		ApplyDamageToPlayer(fromObj, (obj_ServerPlayer*)targetObj, dmgPos, damage, -1, 0, force_damage, damageSource);
		return;
	}

	if(targetObj->Class->Name == "obj_ServerUAV")
	{
		obj_ServerUAV* targetUav = (obj_ServerUAV*)targetObj;
		targetUav->DoDamage(damage, fromObj->NetworkID);
		return;
	}
	else if(targetObj->Class->Name == "obj_ServerMine")
	{
		obj_ServerMine* mine = (obj_ServerMine*)targetObj;
		if(mine->bDestroyed)
			return;

		mine->onExplode(false);

		obj_ServerPlayer* mineOwner = IsServerPlayer(GameWorld().GetObject(mine->ownerID));
		obj_ServerPlayer* fromPl = IsServerPlayer(fromObj);
		if(mineOwner && fromPl && mineOwner->TeamID!=fromPl->TeamID)
		{
			AddPlayerReward(fromPl, RWD_DestroyMine);
		}

		return;
	}
	else if(targetObj->Class->Name == "obj_LightMesh")
	{
		obj_ServerLightMesh* lightM = (obj_ServerLightMesh*)targetObj;
		if(!lightM->bLightOn || !m_isGameHasStarted) // do not allow to shoot lights in pre game
			return;

		lightM->bLightOn = false;
		lightM->SyncLightStatus(0);
		
		return;
	}
	else if(targetObj->Class->Name == "obj_ServerMotionSensor")
	{
		obj_ServerMotionSensor* sensor = (obj_ServerMotionSensor*)targetObj;
		GameObject* sensorOwner = GameWorld().GetObject(sensor->ownerID);
		if(IsServerPlayer(fromObj) && IsServerPlayer(sensorOwner))
		{
			obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
			obj_ServerPlayer* ownerPlr = (obj_ServerPlayer*)sensorOwner;
			if(fromPlr->TeamID != ownerPlr->TeamID)
				sensor->requestKill = true;// sensors are killed with one hit
		}
		return;
	}
	else if(targetObj->Class->Name == "obj_ServerRespawnBeacon")
	{
		obj_ServerRespawnBeacon* beacon = (obj_ServerRespawnBeacon*)targetObj;
		GameObject* beaconOwner = GameWorld().GetObject(beacon->ownerID);
		if(IsServerPlayer(fromObj) && IsServerPlayer(beaconOwner))
		{
			obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
			obj_ServerPlayer* ownerPlr = (obj_ServerPlayer*)beaconOwner;
			if(fromPlr->TeamID != ownerPlr->TeamID)
				beacon->requestKill = true;// beacons are killed with one hit
		}
		return;
	}
	else if(targetObj->Class->Name == "obj_ServerRiotShield")
	{
		obj_ServerRiotShield* shield = (obj_ServerRiotShield*)targetObj;
		GameObject* shieldOwner = GameWorld().GetObject(shield->ownerID);
		if(IsServerPlayer(fromObj) && IsServerPlayer(shieldOwner))
		{
			obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
			obj_ServerPlayer* ownerPlr = (obj_ServerPlayer*)shieldOwner;
			if(fromPlr->TeamID != ownerPlr->TeamID)
				shield->DoDamage(damage);
		}
		return;
	}
	else if(targetObj->Class->Name == "obj_ServerAutoTurret")
	{
		obj_ServerAutoTurret* turret = (obj_ServerAutoTurret*)targetObj;
		GameObject* turretOwner = GameWorld().GetObject(turret->ownerID);
		if(IsServerPlayer(fromObj) && IsServerPlayer(turretOwner))
		{
			obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
			obj_ServerPlayer* ownerPlr = (obj_ServerPlayer*)turretOwner;
			if(fromPlr->TeamID != ownerPlr->TeamID)
				turret->DoDamage(damage);
		}
		return;
	}

	r3dOutToLog("!!! error !!! was trying to damage object %s [%s]\n", targetObj->Name.c_str(), targetObj->Class->Name.c_str());
}

// return true if hit was registered, false otherwise
// state is grabbed from the dynamics.  [0] is from player in the air, [1] is target player in the air
bool ServerGameLogic::ApplyDamageToPlayer(GameObject* fromObj, obj_ServerPlayer* targetPlr, const r3dPoint3D& dmgPos, float damage, int bodyBone, int bodyPart, bool force_damage, STORE_CATEGORIES damageSource, int airState )
{
	r3d_assert(fromObj);
	r3d_assert(targetPlr);

	if(targetPlr->isDead)
		return false;

	// check for friendly fire
	if(!ginfo_.friendlyFire && !force_damage && !targetPlr->isTargetDummy_)
	{
		if(IsServerPlayer(fromObj))
		{
			const obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
			if(fromPlr->TeamID == targetPlr->TeamID && fromPlr!=targetPlr) // we still need to be able to damage ourself, (client sends damage to itself when falling or in other cases)
			{
				//r3dOutToLog("friendly fire detected\n");
				return false;
			}
		}
	}

	if(targetPlr->respawnInvulnerableTime > 0 || targetPlr->berserkerTime > 0)
		return true;

	// if in death zone, double damage
	if(targetPlr->m_InDamageAreaTime > r3dGetTime())
		damage *= 2;
	
	damage = targetPlr->ApplyDamage(damage, fromObj, bodyPart, damageSource);

	// send damage packet, originating from the firing dude
	PKT_S2C_Damage_s a;
	a.dmgPos = dmgPos;
	a.targetId = toP2pNetId(targetPlr->NetworkID);
	a.damage   = R3D_MIN((BYTE)damage, (BYTE)255);
	a.dmgType = damageSource;
	a.bodyBone = bodyBone;
	p2pBroadcastToActive(fromObj, &a, sizeof(a));

	// check if we killed player
	if(targetPlr->m_Health <= 0) 
	{
		bool fromPlayerInAir = ((airState & 0x1) != 0);
		bool targetPlayerInAir = ((airState & 0x2) != 0);

		DoKillPlayer(fromObj, targetPlr, damageSource, false, fromPlayerInAir, targetPlayerInAir);
	}

	return true;
}

void ServerGameLogic::RelayPacket(DWORD peerId, const DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	for(int i=0; i<MAX_PEERS_COUNT; i++) {
		if(peers_[i].status_ >= PEER_PLAYING && i != peerId) {
			net_->SendToPeer(packetData, packetSize, i, guaranteedAndOrdered);
			netSentPktSize[packetData->EventID] += packetSize;
		}
	}

	return;
}

void ServerGameLogic::p2pBroadcastToActive(const GameObject* from, DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	preparePacket(from, packetData);

	for(int i=0; i<MAX_PEERS_COUNT; i++) {
		if(peers_[i].status_ >= PEER_PLAYING) {
			net_->SendToPeer(packetData, packetSize, i, guaranteedAndOrdered);
			netSentPktSize[packetData->EventID] += packetSize;
		}
	}

	return;
}

void ServerGameLogic::p2pBroadcastToAll(DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	preparePacket(NULL, packetData);

	for(int i=0; i<MAX_PEERS_COUNT; i++) {
		if(peers_[i].status_ >= PEER_VALIDATED1) {
			net_->SendToPeer(packetData, packetSize, i, guaranteedAndOrdered);
			netSentPktSize[packetData->EventID] += packetSize;
		}
	}

	return;
}

void ServerGameLogic::p2pSendToPeer(DWORD peerId, const GameObject* from, DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered)
{
	const peerInfo_s& peer = GetPeer(peerId);
	r3d_assert(peer.status_ != PEER_FREE);

	preparePacket(from, packetData);
	net_->SendToPeer(packetData, packetSize, peerId, guaranteedAndOrdered);
	netSentPktSize[packetData->EventID] += packetSize;
}


void ServerGameLogic::RespawnPlayer(obj_ServerPlayer* plr)
{
	r3dOutToLog("respawning %s, teamID:%d\n", plr->userName, plr->TeamID); CLOG_INDENT;
	if(!plr->isDead) 
	{
		LogInfo(plr->peerId_, "RespawnPlayer", "was not dead");
		return;
	}

	// check if we can respawn yet. give a little grace period
	if(ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		const float curTime = r3dGetTime();
		if(curTime < plr->deathTime + respawnDelay_ - 1.0f) 
		{
			r3dOutToLog("!!! too early respawn, delta: %f\n", curTime - (plr->deathTime + respawnDelay_));
		}
	}

	// check if beacon and that it is still valid
	if(plr->NextSpawnNode >= gCPMgr.numControlPoints_)
	{
		GameObject* beacon = GameWorld().GetNetworkObject(plr->NextSpawnNode);
		r3dOutToLog("beacon: id=%d, found=%d\n", plr->NextSpawnNode, beacon!=0?1:0);
		
		if(!(beacon && beacon->isObjType(OBJTYPE_GameplayItem) && beacon->Class->Name=="obj_ServerRespawnBeacon"))
			plr->NextSpawnNode = -1;
	}

	r3dPoint3D pos;
	float dir;
	float respawnProtection = 0.0f;
	if(plr->NextSpawnNode < gCPMgr.numControlPoints_)
	{
		int cpIdx;
		int cpLocIdx;
		FindRespawnPoint( cpIdx, cpLocIdx, plr );

		const BaseControlPoint* cp = gCPMgr.GetCP(cpIdx);
		respawnProtection = cp->spawnProtectionTime;
		if( cpLocIdx == -1 ) 
			GetSpawnPosition(cpIdx, pos, dir, plr);
		else 
			cp->getSpawnPointByIdx( cpLocIdx, pos, dir );
		
		if(gCPMgr.GetBaseIndex(plr->TeamID) == cpIdx)
		{
			plr->lastRespawnTimeAtBase = r3dGetTime();
			plr->lastRespawnPosAtBase = pos;
		}
	}
	else // beacon
	{
		respawnProtection = 0.0f; // no spawn protection from beacon!
		obj_ServerRespawnBeacon* beacon = (obj_ServerRespawnBeacon*)GameWorld().GetNetworkObject(plr->NextSpawnNode);
		if(beacon->itemID == WeaponConfig::ITEMID_RespawnBeaconCons)
		{
			beacon->requestKill = true;
		}
		pos = beacon->GetPosition();
		dir = (float)random(360);
	}

	plr->DoRespawn(pos, dir, respawnProtection);

	PKT_S2C_RespawnPlayer_s n;
	n.pos		= plr->GetPosition();
	n.dir		= plr->m_PlayerRotation;
	n.spawnProtection = respawnProtection;
	n.teamId	= (BYTE)plr->TeamID;
	n.slotNum	= plr->GetLoadoutSlot();
	n.lslot		= plr->GetLoadoutData();
	plr->getAttachmentData(n.attms);
	p2pBroadcastToActive(plr, &n, sizeof(n));

	return;
}

wiStatsTracking ServerGameLogic::GetRewardData(obj_ServerPlayer* plr, EPlayerRewardID rewardID)
{
	r3d_assert(g_GameRewards);
	const CGameRewards::rwd_s& rwd = g_GameRewards->GetRewardData(rewardID);
	if(!rwd.IsSet)
	{
		LogInfo(plr->peerId_, "GetReward", "%d not set", rewardID);
		return wiStatsTracking();
	}
	
	wiStatsTracking stat;
	stat.RewardID = (int)rewardID;
	stat.GP       = 0;
	switch(ginfo_.mapType)
	{
		default:
			r3d_assert(0 && "bad mapType");
			break;
		case GBGameInfo::MAPT_Conquest:
			stat.GD = rwd.GD_CQ;
			stat.HP = rwd.HP_CQ;
			break;
		case GBGameInfo::MAPT_Deathmatch:
			stat.GD = rwd.GD_DM;
			stat.HP = rwd.HP_DM;
			break;
		case GBGameInfo::MAPT_Bomb:
			stat.GD = rwd.GD_SB;
			stat.HP = rwd.HP_SB;
			break;
	}
	
	return stat;
}	
	
void ServerGameLogic::AddPlayerReward(obj_ServerPlayer* plr, EPlayerRewardID rewardID)
{
	wiStatsTracking stat = GetRewardData(plr, rewardID);
	if(stat.RewardID == 0)
		return;

	const CGameRewards::rwd_s& rwd = g_GameRewards->GetRewardData(rewardID);
	AddDirectPlayerReward(plr, stat, rwd.Name.c_str());
}

void ServerGameLogic::AddDirectPlayerReward(obj_ServerPlayer* plr, const wiStatsTracking& in_rwd, const char* rewardName)
{
	// do not add rewards if game not started yet.
	if(!m_isGameHasStarted && !m_isTestingGame)
		return;

	// if this is a non ranked game, do not give XP or GP rewards to players
	// ptumik: show rewards to player, as at the end of round their will be reset anyway
	//if(ginfo_.practiceGame)
	//	return;
	

	float rewardMult = 1.0f;
	switch(ginfo_.mapId)
	{
	case GBGameInfo::MAPID_WO_Nightfall_CQ:
	case GBGameInfo::MAPID_WO_NightfallPAX:
	case GBGameInfo::MAPID_WO_Crossroads16:
	case GBGameInfo::MAPID_WO_Crossroads2:
	case GBGameInfo::MAPID_BurningSea:
		rewardMult = 1.0f;
		break;
	case GBGameInfo::MAPID_WO_Grozny:
	case GBGameInfo::MAPID_WO_Jungle02:
	case GBGameInfo::MAPID_WO_Citadel_DM:
	case GBGameInfo::MAPID_WO_EasternBunkerTDM:
		rewardMult = 1.0f;
		break;
	case GBGameInfo::MAPID_WO_Shipyard:
	case GBGameInfo::MAPID_WO_Wasteland:
		rewardMult = 0.5f;
		break;
	}
	
	// modify reward with reward multiplier
	//@@@@@@@@@@@TODO: move it to plr->AddReward, fix that R3D_ABS crap there.
	wiStatsTracking rwd1;
	rwd1.RewardID = in_rwd.RewardID;
	rwd1.HP       = int(ceilf(in_rwd.HP * rewardMult));
	rwd1.GP       = int(ceilf(in_rwd.GP * rewardMult));
	rwd1.GD       = int(ceilf(in_rwd.GD * rewardMult));

	// add reward to player
	wiStatsTracking rwd2 = plr->AddReward(rwd1);
	int hp = rwd2.getTotalHP();
	int gp = rwd2.GP;
	int gd = rwd2.GD;
	if(hp == 0 && gp == 0 && gd == 0 && rwd2.RewardID != RWD_KillAtSpawn)
		return;
		
	r3dOutToLog("reward: %s got %dhp %dgp %dgd RWD_%s, mult:%f\n", plr->userName, hp, gp, gd, rewardName ? rewardName : "", rewardMult);
  
	// send it to him
	PKT_S2C_AddScore_s n;
	n.ID = (WORD)in_rwd.RewardID;
	n.HP = R3D_CLAMP(hp, -30000, 30000);
	n.GD = (WORD)gd;
	p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
  
	return;
}

void ServerGameLogic::GiveItemToPlayer(obj_ServerPlayer* plr, uint32_t itemID)
{
	r3d_assert(plr);
	r3dOutToLog("GiveItemToPlayer: %d to %s[%d]\n", itemID, plr->userName, plr->profile_.CustomerID);
	
	PlayerGiveItem_s* data = new PlayerGiveItem_s();
	data->CustomerID = plr->profile_.CustomerID;
	data->SessionID  = plr->profile_.SessionID;
	data->itemID     = itemID;
	
  	CreateThread(NULL, 0, PlayerGiveItemThread, data, 0, NULL);
}

void ServerGameLogic::GiveItemToPlayerInMinutes(obj_ServerPlayer* plr, uint32_t itemID, uint32_t numMin)
{
	r3d_assert(plr);
	r3dOutToLog("GiveItemToPlayerInMinutes: %d to %s[%d] for %d minutes\n", itemID, plr->userName, plr->profile_.CustomerID, numMin);

	PlayerGiveItemInMin_s* data = new PlayerGiveItemInMin_s();
	data->CustomerID = plr->profile_.CustomerID;
	data->SessionID  = plr->profile_.SessionID;
	data->itemID     = itemID;
	data->minutes	 = numMin;

	CreateThread(NULL, 0, PlayerGiveItemInMinThread, data, 0, NULL);
}

bool ServerGameLogic::RemoveItemFromPlayer(obj_ServerPlayer* plr, uint32_t itemID)
{
	r3d_assert(plr);
	
	int itemsLeft = 0;
	
	// search if user have that item and decrease quantity
	wiUserProfile& prof = plr->profile_.ProfileData;
	for(uint32_t i=0; i<prof.NumItems; ++i)
	{
		if(prof.Inventory[i].itemID != itemID)
			continue;

		itemsLeft = prof.Inventory[i].quantity;
		if(itemsLeft == 0)
		{
			gServerLogic.LogCheat(plr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "usemoreitems",
				"%d", 
				itemID
				);
			return false;
		}
		
		prof.Inventory[i].quantity--;
		break;
	}
	
	if(itemsLeft == 0)
	{
		gServerLogic.LogCheat(plr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_UseItem, false, "noitem",
			"%d", 
			itemID
			);
		return false;
	}
	
	
	//r3dOutToLog("RemoveItemFromPlayer: %d from %s[%d], %d left\n", itemID, plr->userName, plr->profile_.CustomerID, itemsLeft - 1);

	PlayerRemoveItem_s* data = new PlayerRemoveItem_s();
	data->CustomerID = plr->profile_.CustomerID;
	data->SessionID  = plr->profile_.SessionID;
	data->itemID     = itemID;

	CreateThread(NULL, 0, PlayerRemoveItemThread, data, 0, NULL);
	return true;
}

void ServerGameLogic::CheckForRespawnPlayers()
{
/*
	const float curTime = r3dGetTime();

	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		if(plrToPeer_[i] == NULL)
			continue;

		obj_ServerPlayer* plr = plrToPeer_[i]->player;
		if(plr == NULL) 
			continue;

		// PT: do not respawn automatically. Player has to click on enter battle again, or it will be driven by client anyway
		//if(plr->isDead && curTime > plr->deathTime + respawnDelay_) {
		//	RespawnPlayer(plr);
		//}
	}
*/	

	return;
}

int ServerGameLogic::GetRandomControlPoint(int teamId)
{
	int ids[ControlPointsMgr::MAX_CONTROL_POINTS + 1];
	int n = 0;

	for(int i=0; i<gCPMgr.NumCP(); i++) {
		if(gCPMgr.GetCP(i)->GetSpawnTeamId() == teamId) {
			ids[n++] = i;
		}
	}

	if(n == 0) {
		r3dOutToLog("!!!warning!!! there is no control points for team %d\n", teamId);
		ids[n++] = 0;
	}

	// there must be at least one available control point
	r3d_assert(n != 0);

	int idx = u_random(n);
	idx = ids[idx];
	return idx;
}

int ServerGameLogic::GetClosestControlPoint(int teamId, const r3dPoint3D& pos)
{
	int   idx     = -1;
	float minDist = 999999;

	for(int i=0; i<gCPMgr.NumCP(); i++) 
	{
		const BaseControlPoint* cp = gCPMgr.GetCP(i);
		if(cp->GetSpawnTeamId() != teamId) 
			continue;

		float dist = (cp->GetPosition() - pos).Length();
		if(dist < minDist) {
			minDist = dist;
			idx     = i;
		}
	}

	if(idx == -1) {
		r3dOutToLog("!!!warning!!! there is no control points for team %d\n", teamId);
		idx = 0;
	}

	r3d_assert(idx != -1);
	return idx;
}

void ServerGameLogic::GetSpawnPosition(int cpIdx, r3dPoint3D& pos, float& dir, obj_ServerPlayer* spawnedPlayer)
{
	const BaseControlPoint* cp = gCPMgr.GetCP(cpIdx);
	// get spawn point that has least amount of enemies around it
	if(spawnedPlayer != NULL)
	{
		int cpIndex = 0;
		float closestEnemy[BaseControlPoint::MAX_SPAWN_POINTS];
		for(int i =0; i<BaseControlPoint::MAX_SPAWN_POINTS; ++i)
			closestEnemy[i] = 9999999999999.0f;

		for(int i=0; i<cp->m_NumSpawnPoints; ++i)
		{
			for(int j=0; j<MAX_NUM_PLAYERS; ++j)
			{
				obj_ServerPlayer* plr = GetPlayer(j);
				if(plr == NULL)
					continue;
				if(plr->isDead) 
					continue;
				if(plr->TeamID == spawnedPlayer->TeamID)
					continue;

				float distSq = (plr->GetPosition() - cp->m_SpawnPoints[i]).LengthSq();
				if(distSq < closestEnemy[i])
					closestEnemy[i] = distSq;
			}
		}

		// find points where enemies are not around and choose randomly from them
		int possiblePoints[BaseControlPoint::MAX_SPAWN_POINTS];
		memset(possiblePoints, 0, sizeof(possiblePoints));
		int numPossiblePoints = 0;
		float leastEnemyAround = 0;
		for(int i=0; i<cp->m_NumSpawnPoints; ++i)
		{
			if(closestEnemy[i] > leastEnemyAround)
			{
				leastEnemyAround = closestEnemy[i];
				cpIndex = i;
			}
			if(closestEnemy[i] > 30.0f) // 30 meters away from point -> consider this point
			{
				possiblePoints[numPossiblePoints++] = i;
			}
		}
		if(numPossiblePoints > 1)
		{
			cpIndex = possiblePoints[u_random(numPossiblePoints)];
		}
		cp->getSpawnPointByIdx(cpIndex, pos, dir);
	}
	else // just get random point
		cp->getSpawnPoint(pos, dir);

	// move spawn pos at radius
	pos.x += u_GetRandom(-_glm_SpawnRadius, _glm_SpawnRadius);
	pos.z += u_GetRandom(-_glm_SpawnRadius, _glm_SpawnRadius);
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_ValidateConnectingPeer)
{
	// reply back with our version
	PKT_C2S_ValidateConnectingPeer_s n1;
	n1.protocolVersion = P2PNET_VERSION;
	n1.sessionId       = 0;
	p2pSendToPeer(peerId, NULL, &n1, sizeof(n1));

	if(n.protocolVersion != P2PNET_VERSION) 
	{
		DisconnectPeer(peerId, false, "Version mismatch");
		return;
	}
	extern __int64 cfg_sessionId;
	if(n.sessionId != cfg_sessionId)
	{
		DisconnectPeer(peerId, true, "Wrong sessionId");
		return;
	}

	// switch to Validated state
	peerInfo_s& peer = GetPeer(peerId);
	peer.SetStatus(PEER_VALIDATED1);

	// send level name
	{
		PKT_S2C_LevelInfo_s n;
		n.gameInfo = gServerLogic.ginfo_;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n));
	}

	return;
}

void ServerGameLogic::FillNetCreatePlayer(const obj_ServerPlayer* plr, PKT_S2C_CreatePlayer_s& n)
{
	r3d_assert(plr);
	r3d_assert(plr->NetworkID);

	n.playerIdx = BYTE(plr->NetworkID - NETID_PLAYERS_START);
	n.teamId    = plr->TeamID;
	n.spawnPos  = plr->GetPosition();
	n.moveCell  = plr->netMover.SrvGetCell();
	n.spawnDir  = plr->m_PlayerRotation;
	n.spawnProteciton = plr->respawnInvulnerableTime;
	n.slotNum   = plr->GetLoadoutSlot();
	n.lslot     = plr->GetLoadoutData();
	plr->getAttachmentData(n.attms);
	// hot fix loadout slot in case if player pickup any guns
	n.lslot.PrimaryWeaponID = plr->m_WeaponArray[0]->getConfig()->m_itemID;
	if(plr->m_WeaponArray[1])
		n.lslot.SecondaryWeaponID = plr->m_WeaponArray[1]->getConfig()->m_itemID;
	if(plr->m_WeaponArray[2])
		n.lslot.SidearmWeaponID = plr->m_WeaponArray[2]->getConfig()->m_itemID;
	n.weapIndex = plr->GetSelectedWeapon();
	n.bDying    = plr->isDead;
	
	bool hasPremium = false;
	for(uint32_t i=0; i<plr->profile_.ProfileData.NumItems; ++i)
	{
		hasPremium = plr->profile_.ProfileData.Inventory[i].itemID == 301004;
		if(hasPremium)
			break;
	}
	n.isPremium = hasPremium;
	
	r3dscpy(n.userName, plr->userName);
	n.score = plr->profile_.ProfileData.Stats.HonorPoints;
	n.wins = plr->profile_.ProfileData.Stats.Wins;
	n.losses = plr->profile_.ProfileData.Stats.Losses;
	n.ClanID = plr->profile_.ProfileData.ClanID;
	r3dscpy(n.ClanTag, plr->profile_.ProfileData.ClanTag);
	n.ClanTagColor = plr->profile_.ProfileData.ClanTagColor;
	
	return;
}

obj_ServerPlayer* ServerGameLogic::CreateNewPlayer(DWORD peerId, int teamId, const r3dPoint3D& spawnPos, float dir, int loadoutSlot, float spawnProtection)
{
	peerInfo_s& peer = GetPeer(peerId);
	const int playerIdx = peer.playerIdx;

	r3d_assert(playerIdx >= 0 && playerIdx < maxPlayers_);
	r3d_assert(teamId >= TEAM_BLUE && teamId < TEAM_MAX_ID);
	r3d_assert(peer.haveProfile);

	// add to players table
	char name[128];
	//sprintf(name, "player%02d", playerIdx);
	sprintf(name, "%s", peer.temp_profile.ScreenName);
	obj_ServerPlayer* plr = (obj_ServerPlayer*)srv_CreateGameObject("obj_ServerPlayer", name, spawnPos);
	plr->m_PlayerRotation = dir;
	plr->peerId_      = peerId;
	plr->TeamID       = teamId;
	plr->NetworkID    = playerIdx + NETID_PLAYERS_START;
	plr->NetworkLocal = false;
	plr->respawnInvulnerableTime = spawnProtection;
	plr->SetProfile(peer.temp_profile);
	plr->SetLoadoutSlot(loadoutSlot);
	plr->OnCreate();

	// add to peer-to-player table
	r3d_assert(plrToPeer_[playerIdx] != NULL);
	r3d_assert(plrToPeer_[playerIdx]->player == NULL);
	plrToPeer_[playerIdx]->player = plr;

	// mark that we're active
	peer.player = plr;
	
	// from this point we do expect security report packets
	peer.secRepRecvTime = r3dGetTime();
	peer.secRepGameTime = -1;
	peer.secRepRecvAccum = 0;

	r3d_assert(curPlayers_ < maxPlayers_);
	curPlayers_++;
	
	// send create player packet to all clients
	PKT_S2C_CreatePlayer_s n;
	FillNetCreatePlayer(plr, n);
	p2pBroadcastToActive(NULL, &n, sizeof(n), true);
	
	if(curPlayers_ > simpleStats_.MaxPlayers)
		simpleStats_.MaxPlayers = curPlayers_;

	// since it's a new player, clear the domination check.
	for(int targetPlayerIndex = 0; targetPlayerIndex < MAX_NUM_PLAYERS; targetPlayerIndex++) 
	{
		plr->m_KillsPerPlayer[targetPlayerIndex] = 0;

		obj_ServerPlayer* targetPlr = gServerLogic.GetPlayer(targetPlayerIndex);
		if(targetPlr  == NULL )
			continue;
		targetPlr ->m_KillsPerPlayer[playerIdx] = 0;
	}

	return plr;
}

void ServerGameLogic::DeletePlayer(int playerIdx, obj_ServerPlayer* plr)
{
	r3d_assert(plr);

	r3d_assert(playerIdx == (plr->NetworkID - NETID_PLAYERS_START));
	r3dOutToLog("DeletePlayer: %s, playerIdx: %d\n", plr->userName, playerIdx);
	
	// kill UAV of player
	if(plr->uavId_ != invalidGameObjectID)
	{
		obj_ServerUAV* uav = (obj_ServerUAV*)GameWorld().GetObject(plr->uavId_);
		r3d_assert(uav);
		uav->DoDestroy(plr->NetworkID);
	}

	// kill all player's mines
	for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_Mine))
		{
			obj_ServerMine* mine = (obj_ServerMine*)obj;
			if(mine->ownerID == plr->GetSafeID())
			{
				mine->onExplode(true);
			}
		}
	}

	// send player disconnected message to all peers
	{
		PKT_S2C_DropPlayer_s n;
		n.reason    = 0;
		n.playerIdx = playerIdx;
		p2pBroadcastToActive(NULL, &n, sizeof(n));
	}

	// mark for deletion
	plr->setActiveFlag(0);
	plr->NetworkID = 0;

	r3d_assert(curPlayers_ > 0);
	curPlayers_--;

	return;
}

int ServerGameLogic::BuildUserList(int* num_users, GBUserInfo* users) const
{
	int nn = 0;
	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		if(plrToPeer_[i] == NULL)
			continue;
		const peerInfo_s& peer = *plrToPeer_[i];

		if(nn >= *num_users) {
			r3dError("BuildUserList num_users isn't enough %d vs %d", nn, num_users);
		}

		if(peer.player) {
			users[nn].teamId     = peer.player->TeamID;
			users[nn].CustomerID = peer.CustomerID;
			r3dscpy(users[nn].gbUserName, peer.player->userName);
		} else {
			users[nn].teamId     = peer.desiredTeam;
			users[nn].CustomerID = peer.CustomerID;
			r3dscpy(users[nn].gbUserName, "-");
		}
		nn++;
	}

	*num_users = nn;
	return 1;
}



void ServerGameLogic::SendCurrentGameState()
{ 
	// control points
	for(int i=0; i<gCPMgr.numControlPoints_; i++) 
	{
		gCPMgr.controlPoints_[i]->HostSendUpdate();
	}
	if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
	{
		for(int i=0; i<gSiegeObjMgr.numSiegeObj; ++i)
			gSiegeObjMgr.siegeObjs[i]->HostSendUpdate();
	}
	else if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
			gBombPlacementMgr.GetBombPlacement(i)->HostSendUpdate();
	}


	SendTicketUpdate();
}

void ServerGameLogic::SendTicketUpdate()
{
	if(ginfo_.mapType == GBGameInfo::MAPT_Conquest || ginfo_.mapType == GBGameInfo::MAPT_Deathmatch || ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		// send ticket update
		PKT_S2C_TicketsUpdate_s n;
		n.tickets[0] = WORD(int(tickets_[0]));
		n.tickets[1] = WORD(int(tickets_[1]));
		p2pBroadcastToActive(NULL, &n, sizeof(n));
	}
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_JoinGameReq)
{
	DWORD ip = net_->GetPeerIp(peerId);
	r3dOutToLog("peer%02d PKT_C2S_JoinGameReq: CID:%d, ip:%s\n", 
		peerId, n.CustomerID, inet_ntoa(*(in_addr*)&ip)); 
	CLOG_INDENT;

	// GetFreePlayerSlot
	int playerIdx = -1;
	for(int i=0; i<maxPlayers_; i++) 
	{
		if(plrToPeer_[i] == NULL) 
		{
			playerIdx = i;
			break;
		}
	}

	if(playerIdx == -1)
	{
		r3dOutToLog("game is full\n");
		PKT_S2C_JoinGameAns_s n;
		n.success   = 0;
		n.playerIdx = 0;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n));

		DisconnectPeer(peerId, true, "game is full");
		return;
	}

	{ // send answer to peer
		PKT_S2C_JoinGameAns_s n;
		n.success      = 1;
		n.playerIdx    = playerIdx;
		n.gameTimeLeft = gameEndTime_ - r3dGetTime();
		n.gameStartTime = r3dGetTime() - gameStartTime_;

		p2pSendToPeer(peerId, NULL, &n, sizeof(n));
	}
	
	{  // send game parameters to peer
		PKT_S2C_SetGamePlayParams_s n;
		n.GPP_Data = GPP_Data;
		n.GPP_Seed = GPP_Seed;
		// airstrike cooldowns
		for(int i=0; i<getNumAirstrikes(); ++i)
		{
			n.airstrike_cooldowns[(i*2)] = getAirstrike(i)->CurrentCooldown[0];
			n.airstrike_cooldowns[(i*2)+1] = getAirstrike(i)->CurrentCooldown[1];
		}
		n.airstrike_cooldowns[getNumAirstrikes()*2] = AIRSTRIKE_Team_Cooldown[0];
		n.airstrike_cooldowns[getNumAirstrikes()*2+1] = AIRSTRIKE_Team_Cooldown[1];

		p2pSendToPeer(peerId, NULL, &n, sizeof(n));
	}

	peerInfo_s& peer = GetPeer(peerId);
	r3d_assert(peer.player == NULL);
	peer.SetStatus(PEER_LOADING);
	peer.desiredTeam = 0; // temp, will set proper team after peer finished loading map
	peer.playerIdx    = playerIdx;
	peer.CustomerID   = n.CustomerID;
	peer.SessionID    = n.SessionID;

	// add to players table
	r3d_assert(plrToPeer_[playerIdx] == NULL);
	plrToPeer_[playerIdx] = &peer;
	
	// start thread for profile loading
	peer.haveProfile = 0;
	peer.getProfileH = CreateThread(NULL, 0, GetProfileDataThread, &peer, 0, NULL);

	simpleStats_.Joined++;
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_PlayerReady)
{
	if(m_requestGameStart)
		return;
	if(gameStartCountdown > 0)
		return;
	if(m_isGameHasStarted)
		return;

	peerInfo_s& peer = GetPeer(peerId);
	peer.isPlayerReady = !peer.isPlayerReady;
	peer.selectedLoadoutSlot = n.selectedLoadoutSlot;

	PKT_S2C_Bomb_PlayerReady_s pkt;
	pkt.peerID = peerId;
	pkt.isReady = peer.isPlayerReady;
	p2pBroadcastToActive(NULL, &pkt, sizeof(pkt), true);

	bool force_start = false;
	if(peer.CustomerID == creatorID_) // if creator saying start game, then force start it without waiting for others
		force_start = peer.isPlayerReady==1;

	// check how many players connected, if enough and everyone is ready, start countdown to start the game
	int numPlayers[2] = {0};
	bool everyoneIsReady = true;
	bool foundCreator = false;
	for(int i=0; i<MAX_PEERS_COUNT; ++i)
	{
		peerInfo_s& peerInfo = GetPeer(i);
		if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL && peerInfo.isPlayerSpectator==false)
		{
			if(peerInfo.CustomerID == creatorID_)
				foundCreator = true;
			numPlayers[peerInfo.desiredTeam]++;
			everyoneIsReady &= peerInfo.isPlayerReady;
		}
	}

	// min 4 players needed to start the game
	int minPlrNeeded = R3D_MAX(ginfo_.minPlayers/2, 1);
	if(numPlayers[0] >= minPlrNeeded && numPlayers[1] >= minPlrNeeded && (everyoneIsReady || force_start || ginfo_.permGameIdx>0))
	{
		// start the countdown
		r3dOutToLog("Game started: everyoneIsReady: %d, foundCreator: %d, force_start: %d, permGameIdx: %d\n", everyoneIsReady, foundCreator, force_start, ginfo_.permGameIdx);
		m_requestGameStart = true;
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_StartGameReq)
{
	peerInfo_s& peer = GetPeer(peerId);
	r3d_assert(peer.playerIdx != -1);
	r3d_assert(peer.desiredTeam != -1);
	r3d_assert(peer.player == NULL);
	r3d_assert(peer.status_ == PEER_LOADING);
	
	r3dOutToLog("peer%02d PKT_C2S_StartGameReq, haveProfile: %d, lastNetID: %d\n", peerId, peer.haveProfile, n.lastNetID);

	if(n.lastNetID != net_mapLoaded_LastNetID)
	{
		PKT_S2C_StartGameAns_s n;
		n.result      = PKT_S2C_StartGameAns_s::RES_UNSYNC;
		n.gameStarted = (BYTE)m_isGameHasStarted;
		n.autoplacedTeamId = 0;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n));
		DisconnectPeer(peerId, true, "netID doesn't match");
		return;
	}
	
	switch(peer.haveProfile)
	{
		default: r3d_assert(0);
		// we have profile, process
		case 1:
			break;
			
		// no profile loaded yet
		case 0:
		{
			// we give 60sec to finish getting profile per user
			if(r3dGetTime() > (peer.startTime + 60.0f))
			{
				PKT_S2C_StartGameAns_s n;
				n.result      = PKT_S2C_StartGameAns_s::RES_Failed;
				n.gameStarted = (BYTE)m_isGameHasStarted;
				n.autoplacedTeamId = 0;
				p2pSendToPeer(peerId, NULL, &n, sizeof(n));
				DisconnectPeer(peerId, true, "timeout getting profile data");
			}
			else
			{
				// still pending
				PKT_S2C_StartGameAns_s n;
				n.result      = PKT_S2C_StartGameAns_s::RES_Pending;
				n.gameStarted = (BYTE)m_isGameHasStarted;
				n.autoplacedTeamId = 0;
				p2pSendToPeer(peerId, NULL, &n, sizeof(n));
			}
			return;
		}
		
		// getting profile failed
		case 2:
		{
			PKT_S2C_StartGameAns_s n;
			n.result      = PKT_S2C_StartGameAns_s::RES_Failed;
			n.gameStarted = (BYTE)m_isGameHasStarted;
			n.autoplacedTeamId = 0;
			p2pSendToPeer(peerId, NULL, &n, sizeof(n));
			
			DisconnectPeer(peerId, true, "failed to get profile data");
			return;
		}
	}
	
	// we have player profile, put it in game
	r3d_assert(peer.haveProfile == 1);

	peer.isPlayerSpectator = n.requestSpectator;

	// send all current players to new client
	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(plr == NULL)
			continue;

		PKT_S2C_CreatePlayer_s n;			
		FillNetCreatePlayer(plr, n);
		p2pSendToPeer(peerId, NULL, &n, sizeof(n));

		// send UAV of that player
		obj_ServerUAV* uav = (obj_ServerUAV*)GameWorld().GetObject(plr->uavId_);
		if(uav)
		{
			PKT_S2C_CreateNetObject_s n1;
			uav->fillInSpawnData(n1);
			p2pSendToPeer(peerId, plr, &n1, sizeof(n1), true);
		}
	}

	SendScoreToPlayers(peerId);

	// sync
	for(GameObject* obj=GameWorld().GetFirstObject(); obj; obj=GameWorld().GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_Mine))
		{
			obj_ServerMine* mine = (obj_ServerMine*)obj;
			if(!mine->bDestroyed)
			{
				PKT_S2C_SpawnMine_s n1;
				mine->fillInSpawnData(n1);
				GameObject* mineOwner = GameWorld().GetObject(mine->ownerID);
				if(mineOwner != NULL)
					p2pSendToPeer(peerId, mineOwner, &n1, sizeof(n1), true);
			}
		}
		else if(obj->isObjType(OBJTYPE_GameplayItem))
		{
			if(obj->Class->Name == "obj_ServerMedKit")
			{
				obj_ServerMedKit* medkit = (obj_ServerMedKit*)obj;
				PKT_S2C_CreateNetObject_s n1;
				medkit->fillInSpawnData(n1);
				GameObject* medkitOwner = GameWorld().GetObject(medkit->ownerID);
				if(medkitOwner != NULL)
					p2pSendToPeer(peerId, medkitOwner, &n1, sizeof(n1), true);
			}
			else if(obj->Class->Name == "obj_ServerMotionSensor")
			{
				obj_ServerMotionSensor* sensor = (obj_ServerMotionSensor*)obj;
				PKT_S2C_CreateNetObject_s n1;
				sensor->fillInSpawnData(n1);
				GameObject* sensorOwner = GameWorld().GetObject(sensor->ownerID);
				if(sensorOwner != NULL)
					p2pSendToPeer(peerId, sensorOwner, &n1, sizeof(n1), true);
			}
			else if(obj->Class->Name == "obj_ServerRespawnBeacon")
			{
				obj_ServerRespawnBeacon* beacon = (obj_ServerRespawnBeacon*)obj;
				PKT_S2C_CreateNetObject_s n1;
				beacon->fillInSpawnData(n1);
				GameObject* beaconOwner = GameWorld().GetObject(beacon->ownerID);
				if(beaconOwner != NULL)
					p2pSendToPeer(peerId, beaconOwner, &n1, sizeof(n1), true);
			}
			else if(obj->Class->Name == "obj_ServerRiotShield")
			{
				obj_ServerRiotShield* shield = (obj_ServerRiotShield*)obj;
				PKT_S2C_CreateNetObject_s n1;
				shield->fillInSpawnData(n1);
				GameObject* shieldOwner = GameWorld().GetObject(shield->ownerID);
				if(shieldOwner != NULL)
					p2pSendToPeer(peerId, shieldOwner, &n1, sizeof(n1), true);
			}
			else if(obj->Class->Name == "obj_ServerAutoTurret")
			{
				obj_ServerAutoTurret* turret = (obj_ServerAutoTurret*)obj;
				PKT_S2C_CreateNetObject_s n1;
				turret->fillInSpawnData(n1);
				GameObject* turretOwner = GameWorld().GetObject(turret->ownerID);
				if(turretOwner != NULL)
					p2pSendToPeer(peerId, turretOwner, &n1, sizeof(n1), true);
			}
		}
		else if(obj->Class->Name == "obj_LightMesh")
		{
			obj_ServerLightMesh* lightM = (obj_ServerLightMesh*)obj;
			lightM->SyncLightStatus(peerId);
		}
		else if(obj->Class->Name == "obj_ServerVehicle" )
		{

			obj_ServerVehicle* targetVehicle = ( obj_ServerVehicle* ) obj;

			r3dOutToLog("vehicle sent over network.");
			PKT_S2C_CreateNetObject_s n1;
			targetVehicle->fillInSpawnData(n1);
			p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);

		}
		else if(obj->isObjType(OBJTYPE_WeaponDrop))
		{
			obj_ServerWeaponDrop* wpnDrop = (obj_ServerWeaponDrop*)obj;
			// send dropped weapons
			PKT_S2C_SpawnDroppedWeapon_s n;
			n.spawnID = toP2pNetId(wpnDrop->NetworkID);
			n.pos = wpnDrop->GetPosition();
			n.rot = wpnDrop->GetRotationVector();
			n.itemID = wpnDrop->itemID;
			n.numBullets = wpnDrop->numBullets;
			n.attms = wpnDrop->attms;
			p2pSendToPeer(peerId, NULL, &n, sizeof(n));
		}
		else if(obj->isObjType(OBJTYPE_LootBoxDrop))
		{
			obj_ServerLootBox* lootBox = (obj_ServerLootBox*)obj;
			
			PKT_S2C_SpawnDroppedLootBox_s n;
			n.spawnID = toP2pNetId(lootBox->NetworkID);
			n.pos = lootBox->GetPosition();
			n.rot = lootBox->GetRotationVector();
			n.itemID = lootBox->itemID;
			p2pSendToPeer(peerId, NULL, &n, sizeof(n));
		}
	}
	
	// send info about damage dummies
	for(int i=0; i<4; i++)
	{
		if(testDamageDummy_[i])
		{
			PKT_S2C_CreatePlayer_s n;
			FillNetCreatePlayer(testDamageDummy_[i], n);
			p2pSendToPeer(peerId, NULL, &n, sizeof(n));
		}
	}

	// find a team to which we should join this peer
	{
		int teamPlrs[2] = {0};
		for(int i=0; i<MAX_PEERS_COUNT; ++i)
		{
			peerInfo_s& peerInfo = GetPeer(i);
			if(peerInfo.status_ == PEER_PLAYING && !peerInfo.isPlayerSpectator)
			{
				if(peerInfo.player == NULL)
					teamPlrs[peerInfo.desiredTeam]++;
				else
					teamPlrs[peerInfo.player->TeamID]++;
			}
		}
		if(teamPlrs[0] >= teamPlrs[1])
			peer.desiredTeam = 1;
		else
			peer.desiredTeam = 0;
	}


	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb) 
	{
		if(!m_isGameHasStarted)// game is still in lobby
		{
			// special handling for bomb mode
			// 1. send all connected players to peer
			// 2. send new player to all other peers
			PKT_S2C_Bomb_ConnectedPlayer_s new_player_packet;
			new_player_packet.peerID = peerId;
			new_player_packet.teamID = peer.desiredTeam;
			new_player_packet.isReady = peer.isPlayerReady;
			new_player_packet.isSpectator = peer.isPlayerSpectator;
			new_player_packet.plrLevel = (BYTE)peer.temp_profile.ProfileData.Stats.getRankLevel();
			r3dscpy(new_player_packet.userName, peer.temp_profile.ScreenName);
			new_player_packet.isMaster = peer.temp_profile.CustomerID == creatorID_; // this info goes only to creator to unlock UI options
			// send this info to ourselves firstly
			p2pSendToPeer(peerId, NULL, &new_player_packet, sizeof(new_player_packet));

			new_player_packet.isMaster = 0; // reset

			for(int i=0; i<MAX_PEERS_COUNT; ++i)
			{
				peerInfo_s& peerInfo = GetPeer(i);
				if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL)
				{
					PKT_S2C_Bomb_ConnectedPlayer_s n;
					n.peerID = i;
					n.teamID = peerInfo.desiredTeam;
					n.isReady = peerInfo.isPlayerReady;
					n.isSpectator = peerInfo.isPlayerSpectator;
					n.plrLevel = (BYTE)peerInfo.temp_profile.ProfileData.Stats.getRankLevel();
					r3dscpy(n.userName, peerInfo.temp_profile.ScreenName);
					n.isMaster = 0;
					p2pSendToPeer(peerId, NULL, &n, sizeof(n)); // send to new player all other connected players

					// send new player to already connected players
					p2pSendToPeer(i, NULL, &new_player_packet, sizeof(new_player_packet));
				}
			}
		}
		else // playing game already, create player and let him respawn at next round
		{
			peer.selectedLoadoutSlot = 0;
		}
	}

	peer.SetStatus(PEER_PLAYING);
	// but no player created yet

	//@TODO: send update only to new peer
	SendCurrentGameState();
	
	// send current weapon info to player
	SendWeaponsInfoToPlayer(false, peerId);

	// send answer to start game
	{ 
		PKT_S2C_StartGameAns_s n;
		n.result      = PKT_S2C_StartGameAns_s::RES_Ok;
		n.gameStarted = (BYTE)m_isGameHasStarted;
		n.autoplacedTeamId = peer.desiredTeam;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n));
	}

	if(!m_isGameHasStarted && gameStartCountdown > 0)
	{
		// send packet to client informing them that game is about to start
		PKT_S2C_GameAboutToStart_s n;			
		n.gameTimeLeft = (float)ginfo_.timeLimit * 60.0f + gameStartCountdown;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n), true);

	}

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb && peer.isPlayerSpectator)
	{
		PKT_S2C_GameStarted_s n; // let spectator know that game has started
		n.teleport_pos = r3dPoint3D(0, 0, 0);
		n.dir = 0;
		p2pSendToPeer(peerId, NULL, &n, sizeof(n), true);
	}

	return;
}

void ServerGameLogic::GetNumPlayersInTeam(int tp[2])
{
	tp[0] = 0;
	tp[1] = 0;

	// get team players number only for *active* players
	// connecting players are not counting, 
	// because their team will be set in first spawn message
	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) 
			continue;
		tp[plr->TeamID]++;
	}
}

int ServerGameLogic::AutobalanceTeamId(int teamId)
{
	if(!ginfo_.autoBalance)
		return teamId;

	// on autobalance you should join team with less players
	int tp[2];
	GetNumPlayersInTeam(tp);
	if(tp[TEAM_BLUE] > tp[TEAM_RED])
		return TEAM_RED;
	else if(tp[TEAM_BLUE] < tp[TEAM_RED])
		return TEAM_BLUE;
	else
		return teamId;
}

int ServerGameLogic::AdjustStartSpawnId(int spawnId, int teamId)
{
	if(ginfo_.mapType == GBGameInfo::MAPT_Conquest) // if not conquest (deathmatch) then spawn new player on a base CP only
	{
		if(gCPMgr.GetCP(spawnId)->GetSpawnTeamId() == teamId) 
			return spawnId;
	}
	else if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
	{
		// find objective
		obj_ServerSiegeObjective* objective = gSiegeObjMgr.GetActiveObjective();
		r3d_assert(objective); // shouldn't be null!
		return gCPMgr.getCPIndex(objective->getCP(teamId));
	}
		
	return gCPMgr.GetBaseIndex(teamId);
}

void ServerGameLogic::HandleTeamSwitch(obj_ServerPlayer* plr, int teamId)
{
	if(plr->TeamID == teamId)
		return;

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb) // no team switch in bomb mode
		return;
		
	const float curTime = r3dGetTime();
	
	// if no autobalance, allow switching anytime	
	if(!ginfo_.autoBalance) {
		plr->SwitchTeam(teamId);
		return;
	}
	
	// Do not allow team switch more often than once in three minutes
	if(curTime < plr->m_LastTimeTeamSwitch + 180.0f) {
		r3dOutToLog("HandleTeamSwitch: %s can't switch, too early\n", plr->userName);
		PKT_S2C_TeamSwitchError_s n;
		n.errorType = PKT_S2C_TeamSwitchError_s::ERR_TOO_EARLY;
		p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
		return;
	}

	// do not allow team switch in the last 20% of game time
	if(curTime > (gameEndTime_ - (((float)ginfo_.timeLimit * 60) * 0.2f)))
	{
		r3dOutToLog("HandleTeamSwitch: %s can't switch, game is almost finished\n", plr->userName);
		PKT_S2C_TeamSwitchError_s n;
		n.errorType = PKT_S2C_TeamSwitchError_s::ERR_ALMOST_FINISHED;
		p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
		return;
	}

	// do not allow team switch if trying to join a winning team
	if(ginfo_.mapType == GBGameInfo::MAPT_Conquest || ginfo_.mapType == GBGameInfo::MAPT_Deathmatch)
	{
		if((tickets_[teamId] - tickets_[!teamId]) > 20)
		{
			r3dOutToLog("HandleTeamSwitch: %s can't switch, trying to join a winning team, %d vs %d\n", plr->userName, tickets_[0], tickets_[1]);
			PKT_S2C_TeamSwitchError_s n;
			n.errorType = PKT_S2C_TeamSwitchError_s::ERR_JOINING_WINNING_TEAM;
			p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
			return;
		}
	}
	
	// Do not allow team switch if trying to join a team that has more players
	int tp[2];
	GetNumPlayersInTeam(tp);
	if((tp[teamId] - tp[!teamId]) > 0) {
		r3dOutToLog("HandleTeamSwitch: %s can't switch, %d vs %d\n", plr->userName, tp[0], tp[1]);
		PKT_S2C_TeamSwitchError_s n;
		n.errorType = PKT_S2C_TeamSwitchError_s::ERR_JOINING_TEAM_WITH_MORE_PLAYERS;
		p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
		return;
	}

	plr->SwitchTeam(teamId);
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_SetRespawnData)
{
	peerInfo_s& peer = GetPeer(peerId);
	obj_ServerPlayer* plr = peer.player;

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		if(plr == NULL)
		{
			peer.selectedLoadoutSlot = n.slotNum;
			return; 
		}
		else
		{
			if(!plr->isDead)
				return; // lag?

			// set loadout slot
			plr->SetLoadoutSlot(n.slotNum);
		}
	}
	else
	{
		r3dOutToLog("peer%02d PKT_C2S_SetRespawnData %s\n", peerId, plr == NULL ? "new player" : ""); CLOG_INDENT;
		if(plr == NULL)
		{
			// no player yet, player just now joining game after respawn menu	
			// so adjust spawning parameters based on game mode
			int teamId = AutobalanceTeamId(n.teamId);
			int spawnId = AdjustStartSpawnId(n.spawnId, teamId);

			r3dPoint3D spawnPos; float dir;
			GetSpawnPosition(spawnId, spawnPos, dir, NULL);
			float spawnProtection = gCPMgr.GetCP(spawnId)->spawnProtectionTime;
			CreateNewPlayer(peerId, teamId, spawnPos, dir, n.slotNum, spawnProtection);
			return;
		}

		r3d_assert(plr);
		if(!plr->isDead) {
			r3dOutToLog("!!! %s received spawn while not dead\n", plr->Name.c_str());
			return;
		}

		// set loadout slot
		plr->SetLoadoutSlot(n.slotNum);

		// handle team switch
		HandleTeamSwitch(plr, n.teamId);

		// handle spawn node
		if(n.spawnId>=0)
		{
			// RespawnPlayer() will take care of checking for correct node
			plr->NextSpawnNode = n.spawnId;
		}

		RespawnPlayer(plr);
	}

	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_FallingDamage)
{
    if(!fromObj)
        return;
    obj_ServerPlayer* fromPlr = IsServerPlayer(fromObj);
    if(!fromPlr)
        return;

    r3dOutToLog("Falling damage to %s, damage=%.2f\n", fromObj->Name.c_str(), n.damage); CLOG_INDENT;
    ApplyDamage(fromObj, fromObj, fromObj->GetPosition(), n.damage, true, storecat_INVALID);
}


IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Temp_Damage)
{
    obj_ServerPlayer* fromPlr = IsServerPlayer(fromObj);
    if(!fromPlr)
	{
		//r3dOutToLog("PKT_C2S_Temp_Damage: fromPlr is NULL\n");
		return;
	}


	GameObject* target = GameWorld().GetNetworkObject(n.targetId);
	if(!target)
	{
		//r3dOutToLog("PKT_C2S_Temp_Damage: targetPlr is NULL\n");
		return;
	}

    if(n.wpnIdx <0 || n.wpnIdx >= NUM_WEAPONS_ON_PLAYER)
	{
		//r3dOutToLog("PKT_C2S_Temp_Damage: wpnIdx is out of range\n");
		return;
	}
    if(fromPlr->m_WeaponArray[n.wpnIdx]==NULL)
	{
		//r3dOutToLog("PKT_C2S_Temp_Damage: m_WeaponArray[n.wpnIdx] is NULL\n");
		return;
	}

    const WeaponConfig* wc = fromPlr->m_WeaponArray[n.wpnIdx]->getConfig();

    // check distance
    float dist = (n.explosion_pos-target->GetPosition()).Length();
    if(dist > wc->m_AmmoArea)
	{    
		//r3dOutToLog("PKT_C2S_Temp_Damage: dist is more than AmmoArea\n");
		return;
	}
	if ( n.damagePercentage > 100 || n.damagePercentage < 0 ) {
		
		r3dOutToLog("PKT_C2S_Temp_Damage: Damagepercentage was %d, which is incorrect, potentially a hack, disgarded.\n", n.damagePercentage);
		return;
	}

    float damage = wc->m_AmmoDamage*(1.0f-(dist/wc->m_AmmoArea));
    damage *= n.damagePercentage / 100.0f; // damage through wall

	r3dOutToLog("temp_damage from %s to %s, damage=%.2f\n", fromObj->Name.c_str(), target->Name.c_str(), damage); CLOG_INDENT;
	ApplyDamage(fromObj, target, n.explosion_pos, damage, true, wc->category);
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2C_ChatMessage)
{
	if(!IsNullTerminated(n.msg, sizeof(n.msg))) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_ChatMessage #1");
		return;
	}

	if(fromObj == NULL || !IsServerPlayer(fromObj)) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_ChatMessage #2");
		return;
	}
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	const float curTime = r3dGetTime();
	
	/*
	// chat log
	OpenChatLog();
	if(chatLogFile_)
	{
		fprintf(chatLogFile_, "%06d.%03d| [%d][%s] %s\n", 
			int(curTime), (int(curTime * 1000) % 1000), 
			plr->profile_.CustomerID,
			plr->profile_.ScreenName,
			n.msg);
		fflush(chatLogFile_);
	}
	*/
	
	// check for chat spamming
	const float CHAT_DELAY_BETWEEN_MSG = 1.0f;	// expected delay between message
	const int   CHAT_NUMBER_TO_SPAM    = 4;		// number of messages below delay time to be considered spam
	float diff = curTime - plr->lastChatTime_;

	if(diff > CHAT_DELAY_BETWEEN_MSG) 
	{
		plr->numChatMessages_ = 0;
		plr->lastChatTime_    = curTime;
	}
	else 
	{
		plr->numChatMessages_++;
		if(plr->numChatMessages_ >= CHAT_NUMBER_TO_SPAM)
		{
			DisconnectPeer(peerId, true, "invalid PKT_C2C_ChatMessage #3 - spam");
			return;
		}
	}

	// public
	switch( n.msgType ) 
	{
		case CHAT_MSGTYPE_GENERAL:
		{
			RelayPacket(peerId, &n, sizeof(n), true);
		}
		break;
		
		case CHAT_MSGTYPE_TEAM: 
		{
			for(int i=0; i<MAX_PEERS_COUNT; i++) {
				if(peers_[i].status_ >= PEER_PLAYING && i != peerId && peers_[i].player) {
					if(peers_[i].player->TeamID == plr->TeamID)
						net_->SendToPeer(&n, sizeof(n), i, true);
				}
			}
		}
		break;
		case CHAT_MSGTYPE_CLAN: 
			{
				for(int i=0; i<MAX_PEERS_COUNT; i++) {
					if(peers_[i].status_ >= PEER_PLAYING && i != peerId && peers_[i].player) {
						if(peers_[i].player->profile_.ProfileData.ClanID == plr->profile_.ProfileData.ClanID)
							net_->SendToPeer(&n, sizeof(n), i, true);
					}
				}
			}
			break;
		case CHAT_MSGTYPE_WHISPER:
		{
			GameObject* targetObj = GameWorld().GetNetworkObject(n.target);
			if(targetObj && IsServerPlayer(targetObj))
			{
				for(int i=0; i<MAX_PEERS_COUNT; i++) {
					if(peers_[i].status_ >= PEER_PLAYING && i != peerId && peers_[i].player) {
						if(peers_[i].player == targetObj)
							net_->SendToPeer(&n, sizeof(n), i, true);
					}
				}
			}
		}
		break;
		case CHAT_MSGTYPE_SUICIDE: 
		{
			GameObject* killerObj = plr;

			float timeToRewardKill = 15.0f;
			// can't kill a dead person.
			if( plr != NULL && plr->isDead == false )
			{
// ptumik: retarded idea, removing it to prevent any bugs in the future
//				// if the last person to shoot him hit him recently reward that player. 
// 				if( plr->m_ShotFromHistory[0].networkID != -1 && (r3dGetTime() - plr->m_ShotFromHistory[0].timestamp) < timeToRewardKill )
// 				{
// 					GameObject* killer = GameWorld().GetNetworkObject( plr->m_ShotFromHistory[0].networkID ); // might be null
// 					if(killer)
// 						killerObj = killer;
// 				}

				DoKillPlayer( killerObj, plr, storecat_INVALID, false);
			}
			
		}
		break;
		default:
		{
			DisconnectPeer(peerId, true, "invalid PKT_C2C_ChatMessage #4 - wrong msgType");
			return;
		}
		break;
		
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2C_VoiceCommand)
{
	if(n.id <0 || n.id > 29) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_VoiceCommand #1");
		return;
	}

	if(fromObj == NULL || !IsServerPlayer(fromObj)) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_VoiceCommand #2");
		return;
	}
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	// voice commands always send to team only
	for(int i=0; i<MAX_PEERS_COUNT; i++) 
	{
		if(peers_[i].status_ >= PEER_PLAYING && i != peerId && peers_[i].player && !peers_[i].player->isDead) 
		{
			if(peers_[i].player->TeamID == plr->TeamID && peers_[i].player != plr) // send to team only and do not send it back to sender
				net_->SendToPeer(&n, sizeof(n), i, true);
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2C_CommRoseCommand)
{
	if(n.id <0 || n.id > 7) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_CommRoseCommand #1");
		return;
	}

	if(fromObj == NULL || !IsServerPlayer(fromObj)) {
		DisconnectPeer(peerId, true, "invalid PKT_C2C_CommRoseCommand #2");
		return;
	}
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	// comm rose commands always send to team only
	for(int i=0; i<MAX_PEERS_COUNT; i++) 
	{
		if(peers_[i].status_ >= PEER_PLAYING && i != peerId && peers_[i].player && !peers_[i].player->isDead) 
		{
			if(peers_[i].player->TeamID == plr->TeamID && peers_[i].player != plr) // send to team only and do not send it back to sender
				net_->SendToPeer(&n, sizeof(n), i, true);
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_DataUpdateReq)
{
	r3dOutToLog("got PKT_C2S_DataUpdateReq\n");
	
	// relay that event to master server.
	gMasterServerLogic.RequestDataUpdate();
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_ResupplyRequest)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	r3dOutToLog("%s requested resupply\n", plr->userName);

	plr->m_RequestedResupply = true;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_RequestWeaponPickup)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	// find our weapon
	obj_ServerWeaponDrop* wpn = 0;
	for( GameObject * pObj = GameWorld().GetFirstObject(); pObj; pObj = GameWorld().GetNextObject( pObj ) )
	{
		if(pObj->isObjType(OBJTYPE_WeaponDrop) && pObj->NetworkID == n.spawnID)
		{
			obj_ServerWeaponDrop* wpnDrop = (obj_ServerWeaponDrop*)pObj;
			if(wpnDrop->spawnTime > 0)
			{
				wpn = wpnDrop;
				break;
			}
		}
	}

	if(wpn == NULL)
	{
		r3dOutToLog("%s requested weapon pickup that doesn't exist\n", plr->userName); // maybe weapon just timed out or was picked up by someone else
		return;
	}
	
	// check distance, allow little delta for pickup distance
	float maxDist = (float)PKT_C2S_RequestWeaponPickup_s::PICKUP_RADIUS * 2.5f;
	// check only XZ distance, as on client weapon can fall down.
	float distance = (r3dVector(plr->GetPosition().x, 0, plr->GetPosition().z) - r3dVector(wpn->GetPosition().x, 0, wpn->GetPosition().z)).Length();
	if(distance > maxDist) 
	{
		LogInfo(peerId, "RequestWeaponPickup", "dist %f vs %f", distance, maxDist);
		return;
	}

	plr->m_RequestedWeaponPickup = wpn->itemID;
	plr->m_RequestedWeaponPickupAttms = wpn->attms;
	plr->m_RequestedWeaponPickupNumBullets = wpn->numBullets;
	plr->m_RequestedWeaponPickUpPrevOwner = wpn->isPermanentDrop?invalidGameObjectID:wpn->prevOwner;
	plr->m_RequestWeaponPickupSecretCode = (BYTE)u_random(254)+1; //[1..255];
	if(wpn->isPermanentDrop)
		plr->m_DisableWeaponDrop = true; // hack: to prevent player from dropping permanent weapon drops

	r3dOutToLog("%s requested to pickup weapon %d, secret code:%d\n", plr->userName, wpn->itemID, plr->m_RequestWeaponPickupSecretCode);

	const WeaponConfig& wc = *gWeaponArmory.getWeaponConfig(wpn->itemID);

	PKT_S2C_WeaponPickedUp_s resp;
	resp.itemID = wpn->itemID;
	resp.secretCode = plr->m_RequestWeaponPickupSecretCode;
	resp.numBullets = wpn->numBullets;
	resp.attms = wpn->attms;
	wc.copyParametersTo(resp.wi);
	p2pSendToPeer(plr->peerId_, plr, &resp, sizeof(resp));
	
	// mark to destroy picked up weapon
	wpn->spawnTime = -500;

	if(wpn->isPermanentDrop)
	{
		obj_ServerPermWeaponDrop* weaponDrop = (obj_ServerPermWeaponDrop*)GameWorld().GetObject(wpn->prevOwner);
		if(weaponDrop)
		{
			weaponDrop->ReturnToPickupArea();
		}
		wpn->isPermanentDrop = false; // to allow update code to delete this weapon
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2C_ConfirmWeaponPickup)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	if(plr->m_RequestedWeaponPickup > 0 && plr->m_RequestWeaponPickupSecretCode > 0) // if player died those will be reseted and we might receive packet after death. denis: is it possible? if not, that it will be cheat attempt
	{
		if(n.secretCode != plr->m_RequestWeaponPickupSecretCode)
		{
			LogCheat(plr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_WeaponPickup, true, "WeapPick",
				"Secret code rcvd: %d, original: %d", 
				n.secretCode, plr->m_RequestWeaponPickupSecretCode
				);
			return;
		}
		if(n.itemID != plr->m_RequestedWeaponPickup)
		{
			LogCheat(plr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_WeaponPickup, true, "WeapPick",
				"item rcvd: %d, original: %d", 
				n.itemID, plr->m_RequestedWeaponPickup
				);
			return;
		}
		// change player weapon
		const WeaponConfig* wpn = gWeaponArmory.getWeaponConfig(plr->m_RequestedWeaponPickup);
		if(wpn == NULL)
		{
			LogCheat(plr->peerId_, PKT_S2C_CheatWarning_s::CHEAT_WeaponPickup, true, "WeapPick",
				"Unknown weapon %d received", 
				plr->m_RequestedWeaponPickup
				);
			return;
		}

		STORE_CATEGORIES cat = wpn->category;
		// primary slot
		if(cat == storecat_ASR || cat == storecat_SNP || cat == storecat_MG)
		{
			SAFE_DELETE(plr->m_WeaponArray[0]);
			plr->m_WeaponArray[0] = gWeaponArmory.createWeapon(plr->m_RequestedWeaponPickup, plr, false, false);
			plr->m_WeaponArray[0]->setWeaponAttachmentsByIDs(plr->m_RequestedWeaponPickupAttms.attachments);
			plr->m_WeaponArray[0]->m_NumBulletsLeft = plr->m_RequestedWeaponPickupNumBullets;
		}
		else if(cat == storecat_SUPPORT || cat == storecat_SHTG || cat == storecat_SMG)
		{
			SAFE_DELETE(plr->m_WeaponArray[1]);
			plr->m_WeaponArray[1] = gWeaponArmory.createWeapon(plr->m_RequestedWeaponPickup, plr, false, false);
			plr->m_WeaponArray[1]->setWeaponAttachmentsByIDs(plr->m_RequestedWeaponPickupAttms.attachments);
			plr->m_WeaponArray[1]->m_NumBulletsLeft = plr->m_RequestedWeaponPickupNumBullets;
		}
		else if(cat == storecat_HG)
		{
			SAFE_DELETE(plr->m_WeaponArray[2]);
			plr->m_WeaponArray[2] = gWeaponArmory.createWeapon(plr->m_RequestedWeaponPickup, plr, false, false);
			plr->m_WeaponArray[2]->setWeaponAttachmentsByIDs(plr->m_RequestedWeaponPickupAttms.attachments);
			plr->m_WeaponArray[2]->m_NumBulletsLeft = plr->m_RequestedWeaponPickupNumBullets;
		}

		//n.secretCode = 0; // remove secret code, send to everyone else
		RelayPacket(plr->peerId_, &n, sizeof(n), true);

		// reset data
		plr->m_RequestedWeaponPickup = 0;
		plr->m_RequestWeaponPickupSecretCode = 0;
		plr->m_RequestedWeaponPickupAttms.Reset();
		
		// do not reset prev owner, we will use it to give award for killing prev owner with his own gun. it will be reset on death though
		//plr->m_RequestedWeaponPickUpPrevOwner = 0;
	}
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Siege_Activate)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));

	if(!IsFairGame()) 
		return;

	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	GameObject* targetObj = GameWorld().GetNetworkObject(n.ObjectiveID);
	r3d_assert(targetObj);
	if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
	{
		r3d_assert(targetObj->Class->Name == "obj_SiegeObjective");

		r3dOutToLog("%s activated siege object\n", plr->userName);

		obj_ServerSiegeObjective* sobj = (obj_ServerSiegeObjective*)targetObj;
		sobj->handleActivateRequest(plr);
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_RequestBombPlacement)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));

	if(!IsFairGame()) 
		return;
		
	// late packet. //@TODO: change to packet sequence
	if(bomb_nextRoundStartTimer > 0)
		return;

	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;

	GameObject* targetObj = GameWorld().GetNetworkObject(n.bombID);
	r3d_assert(targetObj);
	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		r3d_assert(targetObj->Class->Name == "obj_BombPlacement");
		r3dOutToLog("%s activated bomb object\n", plr->userName);

		obj_ServerBombPlacement* bobj = (obj_ServerBombPlacement*)targetObj;
		bobj->handleActivateRequest(plr, n.pos);
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_RequestBombPickup)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));

	if(!IsFairGame()) 
		return;

	// late packet. //@TODO: change to packet sequence
	if(bomb_nextRoundStartTimer > 0)
		return;

	if(!bomb_isDropped)
		return;

	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;
	if(plr->TeamID == 0) // only terrist can pick up bomb
	{
		if((plr->GetPosition()-bomb_droppedPos).Length() < 1.0f) // client is checking for 0.5f
		{
			bomb_isDropped = false;
			PKT_S2C_Bomb_PlayerHasBomb_s pkt;
			pkt.playerID = toP2pNetId(plr->NetworkID);
			plr->m_carryingBomb = true;
			
			p2pBroadcastToActive(NULL, &pkt, sizeof(pkt));
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_RequestDrop)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));

	if(!IsFairGame()) 
		return;

	// late packet. //@TODO: change to packet sequence
	if(bomb_nextRoundStartTimer > 0)
		return;

	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;
	if(plr->TeamID == 0 && plr->m_carryingBomb) // only terrist can drop bomb
	{
		bomb_isDropped = true;
		bomb_droppedPos = plr->GetPosition();
		
		plr->m_carryingBomb = false;

		PKT_S2C_Bomb_Dropped_s n;
		n.pos = bomb_droppedPos;
		gServerLogic.p2pBroadcastToActive(NULL, &n, sizeof(n), true);
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_RequestPlayerKick)
{
	if(m_requestGameStart)
		return;
	if(gameStartCountdown > 0)
		return;
	if(m_isGameHasStarted)
		return;

	peerInfo_s& peer = GetPeer(peerId);
	if(peer.temp_profile.CustomerID != creatorID_)
	{
		// cheat attempt?
		return;
	}

	if(n.peerIDtoKick == peerId) // trying to kick itself??
		return;
		
	if(n.peerIDtoKick >= MAX_PEERS_COUNT) {
		DisconnectPeer(peerId, true, "mailformed n.peerIDtoKick = %d", n.peerIDtoKick);
		return;
	}

	peerInfo_s& peerToKick = GetPeer(n.peerIDtoKick);
	if(peerToKick.status_ == PEER_PLAYING)
		DisconnectPeer(n.peerIDtoKick, false, "Kicked by creator");	
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_RequestTeamChange)
{
	if(m_requestGameStart)
		return;
	if(gameStartCountdown > 0)
		return;
	if(m_isGameHasStarted)
		return;

	peerInfo_s& peer = GetPeer(peerId);
	if(n.requestSpectator)
		peer.isPlayerSpectator = !peer.isPlayerSpectator;
	else
	{
		peer.isPlayerSpectator = false;
		peer.desiredTeam = 1 - peer.desiredTeam;
	}
	
	peer.isPlayerReady = false;

	PKT_S2C_Bomb_ConnectedPlayer_s new_player_packet;
	new_player_packet.peerID = peerId;
	new_player_packet.teamID = peer.desiredTeam;
	new_player_packet.isReady = peer.isPlayerReady;
	new_player_packet.isSpectator = peer.isPlayerSpectator;
	new_player_packet.plrLevel = (BYTE)peer.temp_profile.ProfileData.Stats.getRankLevel();
	r3dscpy(new_player_packet.userName, peer.temp_profile.ScreenName);

	for(int i=0; i<MAX_PEERS_COUNT; ++i)
	{
		peerInfo_s& peerInfo = GetPeer(i);
		if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL)
		{
			new_player_packet.isMaster = 0;
			if(peerInfo.temp_profile.CustomerID == creatorID_)
				new_player_packet.isMaster = 1;

			p2pSendToPeer(i, NULL, &new_player_packet, sizeof(new_player_packet));
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Bomb_ChatMsg)
{
	peerInfo_s& peer = GetPeer(peerId);
	PKT_S2C_Bomb_ChatMsg_s pkt;
	pkt.isTeam = n.isTeam;
	r3dscpy(pkt.msg, n.msg);
	pkt.senderPeerId = peerId;

	for(int i=0; i<MAX_PEERS_COUNT; ++i)
	{
		peerInfo_s& peerInfo = GetPeer(i);
		if(peerInfo.status_ == PEER_PLAYING && peerInfo.player == NULL)
		{
			pkt.isAlly = peer.desiredTeam == peerInfo.desiredTeam;
			if(n.isTeam)
			{
				if(peerInfo.desiredTeam == peer.desiredTeam)
					p2pSendToPeer(i, NULL, &pkt, sizeof(pkt));
			}
			else
				p2pSendToPeer(i, NULL, &pkt, sizeof(pkt));
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_MarkTarget)
{

	// TODO: We may want to make it so people can't spam this on the server side. Otherwise I can cheat and mark everyone at once. 
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	const obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;

	// check that target ID exist
	obj_ServerPlayer* targetPlr = IsServerPlayer(GameWorld().GetNetworkObject(n.targetID));
	if(targetPlr==NULL)
		return;

	// we need to find a place.  First let's find out if we're in the list.
	bool recorded = false;
	gobjid_t const from_ID  = fromObj->GetSafeID();
	for ( int targetIndex = 0; targetIndex < targetPlr->numOfUAVHits; targetIndex++ ) 
	{
		if ( targetPlr->listOfUAVHits[targetIndex].UAVTargetedBy == from_ID ) 
		{
			targetPlr->listOfUAVHits[targetIndex].UAVTargetedByTime = r3dGetTime();			
			recorded = true;
		}
	} 
	
	// let's see if we have less than the max numbers.
	if ( recorded == false && targetPlr->numOfUAVHits < MAX_UAV_TARGETS )
	{
		targetPlr->listOfUAVHits[ targetPlr->numOfUAVHits ].UAVTargetedBy = from_ID;
		targetPlr->listOfUAVHits[ targetPlr->numOfUAVHits ].UAVTargetedByTime = r3dGetTime();
		targetPlr->numOfUAVHits++;
	}
	
	
	if ( recorded == false ) 
	{
		// ok we're in trouble now.  Can we remove any entry.  It's rare we'd even be here. 
		float curTime = r3dGetTime();
		for ( int targetIndex = 0; targetIndex < targetPlr->numOfUAVHits; targetIndex++ ) 
		{
			if ( curTime - targetPlr->listOfUAVHits[targetIndex].UAVTargetedByTime < 0  ) 
			{
				targetPlr->listOfUAVHits[targetIndex].UAVTargetedBy = from_ID;			
				targetPlr->listOfUAVHits[targetIndex].UAVTargetedByTime = curTime;
				recorded = true;
			}
		}

		if( recorded == false ) {
			// we have completely failed, perhaps we need to increase the number of MAX_UAV targets.  Give up for now. Don't extend the UAV life at this point. 
			r3dOutToLog("UAV can not find an entry for this UAV hit, we currently support %d.  Perhaps up that limit", MAX_UAV_TARGETS );
			return;
		}
	}
	
	PKT_S2C_TargetMarked_s pkt;
	pkt.time = 10.0f; 
	pkt.targetID = n.targetID;

	for(int i=0; i<MAX_NUM_PLAYERS; ++i)
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(plr && !plr->isDead && plr->TeamID == fromPlr->TeamID)
		{
			p2pSendToPeer(plr->peerId_, NULL, &pkt, sizeof(pkt), true);
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_Admin_PlayerKick)
{
	peerInfo_s& peer = GetPeer(peerId);

	// check if received from legitimate admin account
	if(peer.haveProfile && peer.temp_profile.ProfileData.isDevAccount)
	{
		// go through all peers and find a player with netID
		for(int i=0; i<MAX_PEERS_COUNT; ++i)
		{
			peerInfo_s& pr = GetPeer(i);
			if(pr.status_ == PEER_PLAYING && pr.player)
			{
				if(pr.player->NetworkID == n.netID) // found
				{
					DisconnectPeer(i, false, "Kicked from the game by admin: %s", peer.temp_profile.ScreenName);	
					break;
				}
			}
		}
	}
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_SecurityRep)
{
	const float curTime = r3dGetTime();
	peerInfo_s& peer = GetPeer(peerId);
	if(peer.player==NULL) // cheat??
		return;

	if(peer.secRepGameTime < 0)
	{
		// first call.
		peer.secRepRecvTime = curTime;
		peer.secRepGameTime = n.gameTime;
		//r3dOutToLog("peer%02d, CustomerID:%d SecRep started\n");
		return;
	}
	
	float delta1 = n.gameTime - peer.secRepGameTime;
	float delta2 = curTime    - peer.secRepRecvTime;

	//@ ignore small values for now, until we resolve how that can happens without cheating.
	if(delta2 > ((float)PKT_C2S_SecurityRep_s::REPORT_PERIOD - 0.3f) && delta2 < PKT_C2S_SecurityRep_s::REPORT_PERIOD)
		delta2 = PKT_C2S_SecurityRep_s::REPORT_PERIOD;

	// account for late packets
	peer.secRepRecvAccum -= (delta2 - PKT_C2S_SecurityRep_s::REPORT_PERIOD);

	float k = delta1 - delta2;
	bool isLag = (k > 1.0f || k < -1.0f);
	
	/*
	r3dOutToLog("peer%02d, CID:%d SecRep: %f %f %f %f %s\n", 
		peerId, peer.CustomerID, delta1, delta2, k, peer.secRepRecvAccum,
		isLag ? "net_lag" : "");*/

	// check for client timer
	if(fabs(delta1 - PKT_C2S_SecurityRep_s::REPORT_PERIOD) > 1.0f)
	{
		LogInfo(peerId,	"client_lag?", "%f, %f, %f", delta1, delta2, peer.secRepRecvAccum);
	}

	// check if client was sending packets faster that he should, 20% limit
	if(peer.secRepRecvAccum > ((float)PKT_C2S_SecurityRep_s::REPORT_PERIOD * 0.2f))
	{
		LogCheat(peerId, PKT_S2C_CheatWarning_s::CHEAT_SpeedHack, true,	"speedhack",
			"%f, %f, %f", delta1, delta2, peer.secRepRecvAccum
			);

		peer.secRepRecvAccum = 0;
	}

	// add check for d3d cheats
	if(n.detectedWireframeCheat)
	{
		LogCheat(peerId, PKT_S2C_CheatWarning_s::CHEAT_Wireframe, false, "wireframe cheat");
	}
	
	if((GPP_Data.GetCrc32() ^ GPP_Seed) != n.GPP_Crc32)
	{
		LogCheat(peerId, PKT_S2C_CheatWarning_s::CHEAT_GPP, true, "GPP cheat");
	}

	peer.secRepRecvTime = curTime;
	peer.secRepGameTime = n.gameTime;
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_TEST_SpawnDummyReq)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	const obj_ServerPlayer* fromPlr = (obj_ServerPlayer*)fromObj;
	r3dOutToLog("%s requested to spawn test dummy\n", fromPlr->userName);
	
	float playerRot = r3dGetAngle2D(r3dPoint3D(1, 0, 0), (n.pos - fromObj->GetPosition()).NormalizeTo()) - 90;
	for(int i=0; i<4; i++)
	{
		if(testDamageDummy_[i])
		{
			obj_ServerPlayer* plr = testDamageDummy_[i];
			
			PKT_S2C_DropPlayer_s n;
			n.playerIdx = BYTE(plr->NetworkID - NETID_PLAYERS_START);
			n.reason    = 0;
			p2pBroadcastToActive(NULL, &n, sizeof(n), true);
			
			GameWorld().DeleteObject(plr, true);
			testDamageDummy_[i] = NULL;
		}
	
		// spawn a test dummy...
		char name[128];
		sprintf(name, "@@TestDummy%d@@", i);
		
		// form a line of spawned dummies
		r3dPoint3D pos = n.pos;
		r3dPoint3D right = r3dPoint3D(0, 1, 0).Cross((n.pos - fromObj->GetPosition()).NormalizeTo());
		pos += right * (2.0f * (float)(i - 1));
		
		obj_ServerPlayer* plr = (obj_ServerPlayer*)srv_CreateGameObject("obj_ServerPlayer", name, pos);
		plr->peerId_      = 0;
		plr->TeamID       = !fromPlr->TeamID;
		plr->NetworkID    = NETID_DAMAGEDUMMY + i;
		plr->NetworkLocal = false;
		plr->m_PlayerRotation = playerRot;
		plr->isTargetDummy_ = true;
		plr->m_DisableWeaponDrop = true;
		r3dscpy(plr->userName, name);
		plr->profile_.ProfileData.ArmorySlots[0].LoadoutID = 1;
		switch(i)
		{
			case 1: // light armor
				plr->profile_.ProfileData.ArmorySlots[0].BodyMeshID = 20080; //Slickman 
				plr->profile_.ProfileData.ArmorySlots[0].BodyArmorID = 20060; //light urban 
				plr->profile_.ProfileData.ArmorySlots[0].BodyHeadGearID = 20087; //Jack o lantern
				break;

			case 2: // medium armor
				plr->profile_.ProfileData.ArmorySlots[0].BodyMeshID = 20080; //Slickman 
				plr->profile_.ProfileData.ArmorySlots[0].BodyArmorID = 20065; //night stalker vest 
				plr->profile_.ProfileData.ArmorySlots[0].BodyHeadGearID = 20043; //m9 helmet black 
				break;

			case 3: // heavy armor
				plr->profile_.ProfileData.ArmorySlots[0].BodyMeshID = 20080; //Slickman 
				plr->profile_.ProfileData.ArmorySlots[0].BodyArmorID = 20078; //slickman gear 
				plr->profile_.ProfileData.ArmorySlots[0].BodyHeadGearID = 20031; //JIGN urban camo
				break;
		}
		plr->SetLoadoutSlot(0);

		testDamageDummy_[i] = plr;
		{
			// send create player packet to all clients
			PKT_S2C_CreatePlayer_s n;
			FillNetCreatePlayer(testDamageDummy_[i], n);
			p2pBroadcastToActive(NULL, &n, sizeof(n), true);
		}
	}
	
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_CreateExplosion)
{
//	r3d_assert(fromObj);
//	r3d_assert(IsServerPlayer(fromObj));
//	ApplyExplosionDamage( fromObj, n.pos, n.radius, n.wpnIndex, n.forwVector, n.direction );
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_DBG_LogMessage)
{
	if(!fromObj || !IsServerPlayer(fromObj))
		return;

	obj_ServerPlayer* plr = IsServerPlayer(fromObj);
	// log that packet with temp cheat code
	LogCheat(plr->peerId_, 98, false, "clientlog",
		"%s", 
		n.msg
		);
	return;
}

IMPL_PACKET_FUNC(ServerGameLogic, PKT_C2S_RequestAirstrike)
{
	r3d_assert(fromObj);
	r3d_assert(IsServerPlayer(fromObj));
	obj_ServerPlayer* plr = (obj_ServerPlayer*)fromObj;
	
	bool canUseLLDR = 
		plr->GetLoadoutData().hasItem(WeaponConfig::ITEMID_LLDR) &&
		ginfo_.mapType != GBGameInfo::MAPT_Bomb && 
		(ginfo_.mapId == GBGameInfo::MAPID_WO_Crossroads16  ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Crossroads2  ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Grozny ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Torn ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Jungle02 ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Citadel_DM ||
		ginfo_.mapId == GBGameInfo::MAPID_WO_Nightfall_CQ ||
		ginfo_.mapId == GBGameInfo::MAPID_BurningSea);

	if(!canUseLLDR)
	{
		PKT_S2C_Airstrike_s n1;
		n1.itemID = 3;
		p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
		return;
	}

	AirstrikeDataset* AS = getAirstrikeByID(n.itemID);
	if(AS == NULL)
	{
		r3dOutToLog("Failed to find airstrike with id=%d\n", n.itemID);
		PKT_S2C_Airstrike_s n1;
		n1.itemID = 0;
		p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
		return;
	}
	
	// find store item to locate price
	int price = 0;
	for(uint32_t i=0; i<g_NumStoreItems; ++i)
	{
		if(g_StoreItems[i].itemID == AS->itemID)
		{
			price = g_StoreItems[i].price1day;
			break;
		}
	}
	if(price == 0)
	{
		r3dOutToLog("Price isn't set for airstrike (%d)\n", AS->itemID);
		PKT_S2C_Airstrike_s n1;
		n1.itemID = 0;
		p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
		return;
	}
	// check for enough GP
	if((plr->profile_.ProfileData.Stats.GamePoints - price) < 0)
	{
		// not enough money
		PKT_S2C_Airstrike_s n1;
		n1.itemID = 1;
		p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
		return;
	}

	// check for cooldowns
	if(AIRSTRIKE_Team_Cooldown[plr->TeamID] > 0 || AS->CurrentCooldown[plr->TeamID]>0)
	{
		PKT_S2C_Airstrike_s n1;
		n1.itemID = 2;
		p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
		return;
	}

	r3dOutToLog("%s requested airstrike (%d)\n", plr->userName, n.itemID);

	AIRSTRIKE_Team_Cooldown[plr->TeamID] = 30.0f;
	AS->CurrentCooldown[plr->TeamID] = AS->Cooldown;

	plr->profile_.ProfileData.Stats.GamePoints -= price;
	// issue a command to deduct price from player profile in async function
	PlayerBuyItem_s* rc = new PlayerBuyItem_s;
	rc->itemID    = AS->itemID;
	rc->CustomerID =  GetPeer(peerId).CustomerID;
	rc->SessionID = plr->profile_.SessionID;
	rc->BuyIdx = 1; // GP, 1 Day
	CreateThread(NULL, 0, PlayerBuyItemThread, rc, 0, NULL);

	PKT_S2C_Airstrike_s n1;
	n1.itemID = 100;
	p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
	plr->m_AirstrikeKills = 0;

	r3dVector targetPos = n.pos;
	for(int i=0; i<AS->NumStrikes; ++i)
	{
		r3dVector strikePos = targetPos;
		strikePos.y += 500;
		strikePos.x += u_GetRandom(-AS->StrikeRadius, AS->StrikeRadius);
		strikePos.z += u_GetRandom(-AS->StrikeRadius, AS->StrikeRadius);

		float heightOffset = u_GetRandom(-AS->BaseHeight*0.2f, AS->BaseHeight*0.2f);

		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->PhysXScene->raycastSingle(PxVec3(strikePos.x, strikePos.y, strikePos.z), PxVec3(0,-1,0), 2000, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			r3dVector hitPos(hit.impact.x, hit.impact.y, hit.impact.z);

			// add airstrike damage into queue
			bool found = false;
			for(int i=0; i<MAX_AIRSTRIKES_IN_FLIGHT; ++i)
			{
				if(g_AirstrikesInFlight[i].itemID == 0)
				{
					g_AirstrikesInFlight[i].itemID = n.itemID;
					g_AirstrikesInFlight[i].speed = AS->Speed;
					g_AirstrikesInFlight[i].hitPos = hitPos;
					g_AirstrikesInFlight[i].heightOffset = heightOffset;
					g_AirstrikesInFlight[i].Creator = plr->GetSafeID();
					g_AirstrikesInFlight[i].teamID = plr->TeamID;
					found = true;
					break;
				}
			}
			if(!found)
			{
				r3dOutToLog("!!! reached max airstrikes in flight!!!!!\n");
				PKT_S2C_Airstrike_s n1;
				n1.itemID = 0;
				p2pSendToPeer(peerId, NULL, &n1, sizeof(n1), true);
				return;
			}

			// send airstrike packet to everyone
			{
				PKT_S2C_Airstrike_s n1;
				n1.pos = hitPos;
				n1.itemID = n.itemID;
				n1.heightOffset = heightOffset;
				p2pBroadcastToActive(plr, &n1, sizeof(n1), true);
			}
		}
	}
}

void ServerGameLogic::OnPKT_C2S_ScreenshotData(DWORD peerId, const int size, const char* data)
{
	char	fname[MAX_PATH];

	const peerInfo_s& peer = GetPeer(peerId);
	if(peer.player == NULL) {
		return;
	} else {
		sprintf(fname, "logss\\GS_%I64x_%d_%x.jpg", cfg_sessionId, peer.player->profile_.CustomerID, GetTickCount());
	}
	
	FILE* f = fopen(fname, "wb");
	if(f == NULL) {
		LogInfo(peerId, "SaveScreenshot", "unable to save fname:%s", fname);
		return;
	}
	//r3dOutToLog("peer%02d received screenshot, fname:%s", peerId, fname);

	fwrite(data, 1, size, f);
	fclose(f);

	return;
}


int ServerGameLogic::ProcessWorldEvent(GameObject* fromObj, DWORD eventId, DWORD peerId, const void* packetData, int packetSize)
{
	// do version check and game join request
	peerInfo_s& peer = GetPeer(peerId);

	switch(peer.status_)
	{
		// check version in connected state
	case PEER_CONNECTED:
		switch(eventId)
		{
			DEFINE_PACKET_HANDLER(PKT_C2S_ValidateConnectingPeer);
		}
		DisconnectPeer(peerId, true, "bad packet ID in connected state");
		return TRUE;

		// process join request in validated state
	case PEER_VALIDATED1:
		switch(eventId)
		{
			DEFINE_PACKET_HANDLER(PKT_C2S_JoinGameReq);
		}
		DisconnectPeer(peerId, true, "bad packet ID in validated1 state");
		return TRUE;

	case PEER_LOADING:
		switch(eventId)
		{
			DEFINE_PACKET_HANDLER(PKT_C2S_StartGameReq);
		}
		DisconnectPeer(peerId, true, "bad packet ID in loading state");
		return TRUE;
	}

	r3d_assert(peer.status_ == PEER_PLAYING);

	// validation and relay client code
	switch(eventId) 
	{
		DEFINE_PACKET_HANDLER(PKT_C2S_SetRespawnData);
		DEFINE_PACKET_HANDLER(PKT_C2S_Temp_Damage);
		DEFINE_PACKET_HANDLER(PKT_C2S_FallingDamage);
		DEFINE_PACKET_HANDLER(PKT_C2S_ResupplyRequest);
		DEFINE_PACKET_HANDLER(PKT_C2S_Siege_Activate);
		DEFINE_PACKET_HANDLER(PKT_C2C_ChatMessage);
		DEFINE_PACKET_HANDLER(PKT_C2C_VoiceCommand);
		DEFINE_PACKET_HANDLER(PKT_C2C_CommRoseCommand);
		DEFINE_PACKET_HANDLER(PKT_C2S_DataUpdateReq);
		DEFINE_PACKET_HANDLER(PKT_C2S_RequestAirstrike);
		DEFINE_PACKET_HANDLER(PKT_C2S_RequestWeaponPickup);
		DEFINE_PACKET_HANDLER(PKT_C2C_ConfirmWeaponPickup);

		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_PlayerReady);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_RequestBombPlacement);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_RequestBombPickup);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_RequestDrop);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_RequestTeamChange);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_RequestPlayerKick);
		DEFINE_PACKET_HANDLER(PKT_C2S_Bomb_ChatMsg);

		DEFINE_PACKET_HANDLER(PKT_C2S_MarkTarget);

		DEFINE_PACKET_HANDLER(PKT_C2S_SecurityRep);
		DEFINE_PACKET_HANDLER(PKT_C2S_Admin_PlayerKick);
		DEFINE_PACKET_HANDLER(PKT_C2S_TEST_SpawnDummyReq);
		
		DEFINE_PACKET_HANDLER(PKT_C2S_CreateExplosion);
		DEFINE_PACKET_HANDLER(PKT_C2S_DBG_LogMessage);
		
		// special packet case with variable length
		case PKT_C2S_ScreenshotData:
		{
			const PKT_C2S_ScreenshotData_s& n = *(PKT_C2S_ScreenshotData_s*)packetData;
			if(packetSize < sizeof(n)) {
				LogInfo(peerId, "PKT_C2S_ScreenshotData", "packetSize %d < %d", packetSize, sizeof(n));
				return TRUE;
			}
			if(n.errorCode != 0)
			{
				LogInfo(peerId, "PKT_C2S_ScreenshotData", "screenshot grab failed: %d", n.errorCode);
				return TRUE;
			}
			
			if(packetSize != sizeof(n) + n.dataSize) {
				LogInfo(peerId, "PKT_C2S_ScreenshotData", "dataSize %d != %d+%d", packetSize, sizeof(n), n.dataSize);
				return TRUE;
			}
			
			OnPKT_C2S_ScreenshotData(peerId, n.dataSize, (char*)packetData + sizeof(n));
			return TRUE;
		}
	}

	return FALSE;
}

void ServerGameLogic::UpdateGameLogic_CheckBattleZone()
{
	const float curTime = r3dGetTime();

	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr || plr->isDead) 
			continue;

		if(g_BattleZone.IsInBattle(plr->GetPosition())) {
			if(plr->bzOutTime > 0) {
				r3dOutToLog("%s entered battle zone\n", plr->Name.c_str());
				plr->bzOutTime = -1;
			}
			continue;
		}

		if(plr->bzOutTime < 0) {
			plr->bzOutTime = curTime;
			r3dOutToLog("%s left battle zone\n", plr->Name.c_str());
			continue;
		}

		if(curTime > (plr->bzOutTime + _glm_bzOutKillTime)) {
			r3dOutToLog("%s killed outside battle zone\n", plr->Name.c_str());
			DoKillPlayer(plr, plr, storecat_INVALID);
			continue;
		}
	}

	return;
}

void ServerGameLogic::UpdateGameLogic_UpdateTickets()
{
	// GLM_Conquest logic
	if(r3dGetTime() < nextTicketUpdate_)
		return;
	nextTicketUpdate_ = r3dGetTime() + _glmConq_BleedDelay;

	// do not update points if we don't have enough players
	if(!IsFairGame())
		return;

	if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
		return;
	else if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		SendTicketUpdate(); // in bomb mode just send ticket updates
		return;
	}

	int blue, red;
	int totalCapturableCP = gCPMgr.GetControlledPoints(blue, red, 0);
	{
		// the more control points you have, the quicker opposite team bleeds points
		if(ginfo_.mapType == GBGameInfo::MAPT_Conquest)
		{
			// titov's request: team that holds more than 50% start to bleed other's team tickets at a rate of 1 ticket per 3 seconds
			if(blue > (totalCapturableCP/2))
				tickets_[1] -= 0.33f; 
			else if(red > (totalCapturableCP/2))
				tickets_[0] -= 0.33f;
		}

		if(tickets_[0] < 1.0f) tickets_[0] = 0;
		if(tickets_[1] < 1.0f) tickets_[1] = 0;

		SendTicketUpdate();
	}
	return;
}

void ServerGameLogic::AwardLevelUpBonus(obj_ServerPlayer* plr, int newLevel)
{
	r3dOutToLog("level up, player '%s', newlevel=%d\n", plr->userName, newLevel);

	return; // all bonuses are given in SQL procedure
}

void ServerGameLogic::CheckForLevelUp(obj_ServerPlayer* plr, int addedHp)
{
	wiStats stats1 = plr->profile_.ProfileData.Stats;
	wiStats stats2 = plr->profile_.ProfileData.Stats;
	stats2.HonorPoints += addedHp;

	plr->LevelUpMin = stats1.getRankLevel();
	plr->LevelUpMax = stats2.getRankLevel();
	for(int newLevel=plr->LevelUpMin+1; newLevel <= plr->LevelUpMax; newLevel++)
	{
		AwardLevelUpBonus(plr, newLevel);
	}
	
	// now test against each level highest level. 
	for( int AchievementIndex = ACHIEVEMENT_LEVEL2; AchievementIndex<= ACHIEVEMENT_LEVEL60; AchievementIndex++ )
	{
		plr->checkAchievementByValue( AchievementIndex, plr->LevelUpMax );
	}
}

void ServerGameLogic::RewardEndGame(int winTeam, bool isDraw)
{
	r3dOutToLog("RewardEndGame, teamWon:%d, draw:%d\n", winTeam, isDraw); CLOG_INDENT;
	const float curTime = r3dGetTime();

	// award winning players
	obj_ServerPlayer* plr_mostKills = NULL;
	int			maxKills = 0;
	obj_ServerPlayer* plkr_mostCaptureNeutrals = NULL;
	int			maxCaptureNeutralPoints = 0;

	obj_ServerPlayer* plr_bestInTeam0 = NULL;
	int			bestInTeam_Score0 = 0;
	obj_ServerPlayer* plr_bestInTeam1 = NULL;
	int			bestInTeam_Score1 = 0;
	obj_ServerPlayer* plr_bestOverall = NULL;
	int			bestOverall_Score = 0;


	if(curPlayers_>=6)
	{
		for(int i=0; i<maxPlayers_; i++) 
		{
			obj_ServerPlayer* plr = GetPlayer(i);
			if(!plr) continue;

			if(plr->RoundStats_.HonorPoints > bestOverall_Score)
			{
				bestOverall_Score = plr->RoundStats_.HonorPoints;
				plr_bestOverall = plr;
			}
			if(plr->RoundStats_.HonorPoints > bestInTeam_Score0 && plr->TeamID == 0)
			{
				bestInTeam_Score0 = plr->RoundStats_.HonorPoints;
				plr_bestInTeam0 = plr;
			}
			if(plr->RoundStats_.HonorPoints > bestInTeam_Score1 && plr->TeamID == 1)
			{
				bestInTeam_Score1 = plr->RoundStats_.HonorPoints;
				plr_bestInTeam1 = plr;
			}
		}
	}

	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) continue;

		if(!isDraw)
		{
			//int yours, enemes;
			//gCPMgr.GetControlledPoints(&yours, &enemes, plr->TeamID);

			if(plr->TeamID == winTeam)
			{
				AddPlayerReward(plr, RWD_Win);
				plr->RoundStats_.Wins++;

				switch (ginfo_.mapType) 
				{
				case GBGameInfo::MAPT_Deathmatch:
					{
						const peerInfo_s& peerInfo = GetPeer( m_LastKiller_PeerID ); 
						if( peerInfo.status_ == PEER_PLAYING && peerInfo.player == plr )
						{
							plr->markAchievementComplete( ACHIEVEMENT_DM_LAST_KILL );	
						}

						
						plr->markAchievementComplete( ACHIEVEMENT_DM_WON_MATCH);
					}
					break;
				case GBGameInfo::MAPT_Siege:
					{
						plr->markAchievementComplete( ACHIEVEMENT_WIN_MATCH_IN_SABOTAGE);
					}
					break;
				case GBGameInfo::MAPT_Conquest:
					{
						plr->markAchievementComplete( ACHIEVEMENT_CONQUEST_WIN_MATCH);
					}
					break;
				}
						
			}
			else
			{
				AddPlayerReward(plr, RWD_Loss);
				plr->RoundStats_.Losses++;

			}
		}

		if(plr->RoundStats_.Kills > maxKills)
		{
			maxKills = plr->RoundStats_.Kills;
			plr_mostKills = plr;
		}
		if(plr->RoundStats_.CaptureNeutralPoints > maxCaptureNeutralPoints)
		{
			maxCaptureNeutralPoints = plr->RoundStats_.CaptureNeutralPoints;
			plkr_mostCaptureNeutrals = plr;
		}

		if(ginfo_.mapType != GBGameInfo::MAPT_Bomb)
		{
			// kill/death ratio bonus (only if killed more that 5)
			if(plr->RoundStats_.Kills >= 5)
			{
				float kd = 999.0f;
				if(plr->RoundStats_.Deaths > 0)
					kd = (float)plr->RoundStats_.Kills / (float)plr->RoundStats_.Deaths;
                else
                    kd = (float)plr->RoundStats_.Kills;

				/*if(kd >= 5.0f)
					AddPlayerReward(plr, RWD_KDRatioMore5_0, "RWD_KDRatioMore5_0");*/
				/*else if(kd >= 3.0f)
				AddPlayerReward(plr, RWD_KDRatioMore3_0, "RWD_KDRatioMore3_0");
				else if(kd >= 1.2f)
				AddPlayerReward(plr, RWD_KDRatioMore1_2, "RWD_KDRatioMore1_2");*/
			}

			/*if(plr->RoundStats_.Kills >= 50)
				AddPlayerReward(plr, RWD_TotalKills50, "RWD_TotalKills50");
			else if(plr->RoundStats_.Kills >= 25)
				AddPlayerReward(plr, RWD_TotalKills25, "RWD_TotalKills25");
			else if(plr->RoundStats_.Kills >= 10)
				AddPlayerReward(plr, RWD_TotalKills10, "RWD_TotalKills10");*/

			// accuracy bonus
			if(plr->RoundStats_.ShotsHits >= 50)
			{
				/*float acc = plr->RoundStats_.ShotsHits / (float)plr->RoundStats_.ShotsFired;
				if(acc >= 0.75f)
					AddPlayerReward(plr, RWD_AccuracyMore0_75, "RWD_AccuracyMore0_75");*/
				/*else if(acc >= 0.5f)
				AddPlayerReward(plr, RWD_AccuracyMore0_5, "RWD_AccuracyMore0_5");
				else if(acc >= 0.3f)
				AddPlayerReward(plr, RWD_AccuracyMore0_3, "RWD_AccuracyMore0_3");*/
			}
		}
	}

	/*if(ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(plr_mostKills)
			AddPlayerReward(plr_mostKills, RWD_KillMostEnemies, "RWD_KillMostEnemies");
		if(plkr_mostCaptureNeutrals)
			AddPlayerReward(plkr_mostCaptureNeutrals, RWD_CaptureMostNeutralPoints, "RWD_CaptureMostNeutralPoints");
	}*/
	if(plr_bestOverall)
		AddPlayerReward(plr_bestOverall, RWD_BestPlayerOverall);
	if(plr_bestInTeam0)
		AddPlayerReward(plr_bestInTeam0, RWD_BestPlayerInTeam);
	if(plr_bestInTeam1)
		AddPlayerReward(plr_bestInTeam1, RWD_BestPlayerInTeam);

	// update player profiles
	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) continue;

		int timePlayed = (int)(curTime - plr->startPlayTime_);
		plr->RoundStats_.TimePlayed += timePlayed;
		
		// update current loadout play time
		if(plr->m_SelectedLoadout >= 0)
			plr->loadoutUsage_[plr->m_SelectedLoadout].AddPlayTime();

		// do now allow to subtract honor/game points
		if(plr->RoundStats_.HonorPoints < 0)
			plr->RoundStats_.HonorPoints = 0;
		if(plr->RoundStats_.GamePoints < 0)
			plr->RoundStats_.GamePoints = 0;
		if(plr->RoundStats_.GameDollars < 0)
			plr->RoundStats_.GameDollars = 0;

		// sergey's hack. For new users give them enough XP to make sure they level up on their first games no matter what
		int xpOld = plr->profile_.ProfileData.Stats.HonorPoints;
		int xpNew = xpOld + plr->RoundStats_.HonorPoints;
		int lvl1  = g_RanksPoints[0];
		int lvl2  = g_RanksPoints[1];
		int xpAdd = 0;
		if (xpOld < lvl1 && xpNew < lvl1) // level 0 user
			xpAdd = lvl1 - xpNew; // give enough XP to make sure level up will happen
		else if ((xpOld >= lvl1 && xpOld < lvl2) && xpNew < lvl2) // level 1 user
			xpAdd = (lvl2 - lvl1) / 2; // give half XP for level 2, to make sure that user will level up after two games
		plr->RoundStats_.HonorPoints += xpAdd;

		r3dOutToLog("GameReward: %s: %dhp, %dgp, %dgd (%s)\n", 
			plr->userName, plr->RoundStats_.HonorPoints, plr->RoundStats_.GamePoints, plr->RoundStats_.GameDollars,
			isDraw ? "DRAW" : (plr->TeamID == winTeam ? "WON" : "LOST"));

		CheckForLevelUp(plr, plr->RoundStats_.HonorPoints);

		// add round statistics to whole profile
		plr->profile_.ProfileData.Stats += plr->RoundStats_;
		plr->incrementAchievement( ACHIEVEMENT_EARNED10000WP, plr->RoundStats_.GameDollars );
	}
}

void ServerGameLogic::SendGameFinish(PKT_S2C_GameFinish_s& n, const char* why)
{
	simpleStats_.SetEndData(tickets_, why);
	
	RewardEndGame(n.winTeam, n.reason == PKT_S2C_GameFinish_s::REASON_Draw);

	// first send each player stats
	SendScoreToPlayers(0);
	SendRoundStatsToPlayers();
	
	// send finish packet
	p2pBroadcastToAll(&n, sizeof(n));
}

void ServerGameLogic::Bomb_ReportWinRound(int winTeamID, bool winByObjective)
{
	r3dOutToLog("Bomb_ReportWinRound(%d)\n", winTeamID);

	if(winByObjective)
	{
		// add win by objective
		for(int i=0; i<MAX_NUM_PLAYERS; ++i)
		{
			obj_ServerPlayer* plr = GetPlayer(i);
			if(plr && plr->TeamID==winTeamID)
			{
				if( winTeamID == 0 ) // if Terrorists Won, and their bomb went off. 
				{
					plr->incrementAchievement( ACHIEVEMENT_DETONATE_50_BOMBS, 1);
				}

				AddPlayerReward(plr, RWD_Bomb_WinRound);
			}
		}
		
	}
	else
	{
		// add win round by kills
		// look how many plr are dead and count them as kills for other team, not precise as will not work correctly with suicides and team kills, but will do for now :)
		int maxKills[2] = {0};
		for(int i=0; i<MAX_NUM_PLAYERS; ++i)
		{
			obj_ServerPlayer* plr = GetPlayer(i);
			if(plr)
				maxKills[1-plr->TeamID] += plr->isDead;
		}
		int maxKillsTeam = maxKills[0]>maxKills[1]?0:1;

		for(int i=0; i<MAX_NUM_PLAYERS; ++i)
		{
			obj_ServerPlayer* plr = GetPlayer(i);
			if(plr && plr->TeamID==maxKillsTeam)
				AddPlayerReward(plr, RWD_Bomb_WinRoundKills);
		}
	}

	tickets_[winTeamID]++;
	PKT_S2C_Bomb_WonRound_s n;
	n.teamID = winTeamID;
	p2pBroadcastToActive(NULL, &n, sizeof(n), true);

	// allow 3 seconds before starting new round
	bomb_nextRoundStartTimer = 3.0f;

	for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
		gBombPlacementMgr.GetBombPlacement(i)->Status = obj_ServerBombPlacement::SS_EMPTY;
}

bool ServerGameLogic::UpdateGameLogic_CheckEnd()
{
	if(ginfo_.permGameIdx > 0 && !m_isGameHasStarted) // if this is a permanent game and game has not started - do not finish it. there is no point in constantly restarting empty games
	{
		gameEndTime_   = r3dGetTime() + (float)ginfo_.timeLimit * 60.0f;
		return false;
	}

	if(gameNoPeersConnectedTime > 90.0f && ginfo_.permGameIdx == 0) // more than X seconds of inactivity - finish game. and it's not a permanent game
	{
		r3dOutToLog("Game: stopped because of inactivity\n");
		return true;
	}

	if(m_isTestingGame)
	{
		if(GetAsyncKeyState(VK_F12) & 0x8000)
			gameEndTime_ = r3dGetTime() - 1.0f;
	}

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb && m_isGameHasStarted)
	{
		int numRoundsPlayed = (int)tickets_[0] + (int)tickets_[1];
		if(numRoundsPlayed >= ginfo_.respawnDelay)
		{
			// finish game
			PKT_S2C_GameFinish_s n;
			n.reason  = PKT_S2C_GameFinish_s::REASON_Ticket;
			n.winTeam = int(tickets_[0]) > int(tickets_[1]) ? 0 : 1;
			SendGameFinish(n, "Ticket");
			return true;
		}

		bool no_bombs_destroyed = true;
		bool no_bombs_armed = true;
		for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
		{
			if(gBombPlacementMgr.GetBombPlacement(i)->Status == obj_ServerBombPlacement::SS_DESTROYED)
				no_bombs_destroyed = false;
			if(gBombPlacementMgr.GetBombPlacement(i)->Status == obj_ServerBombPlacement::SS_ARMED)
				no_bombs_armed = false;
		}
		if(!m_requestGameStart && gameStartCountdown<=0 && no_bombs_destroyed && bomb_nextRoundStartTimer<=0)
		{
			int numPlAlive[2] = {0};
			for(int i=0; i<MAX_NUM_PLAYERS; ++i)
			{
				obj_ServerPlayer* pl = GetPlayer(i);
				if(pl && !pl->isDead)
					numPlAlive[pl->TeamID]++;
			}
			if(numPlAlive[0] == 0 && no_bombs_armed)
			{
				Bomb_ReportWinRound(1, false);
				return false;
			}
			else if(numPlAlive[1] == 0)
			{
				Bomb_ReportWinRound(0, false);
				return false;
			}
		}
	}

	// check end by timeout
	if(r3dGetTime() >= gameEndTime_ && !m_requestGameStart && gameStartCountdown<=0 && bomb_nextRoundStartTimer<=0 && m_isGameHasStarted) 
	{
		if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
		{
			// if terrorist armed bomb, then wait until bomb removed or exploded
			bool active_bomb = false;
			for(int i=0; i<gBombPlacementMgr.NumBombPlacements(); ++i)
			{
				if(gBombPlacementMgr.GetBombPlacement(i)->Status == obj_ServerBombPlacement::SS_DESTROYED)
					active_bomb = true;
				if(gBombPlacementMgr.GetBombPlacement(i)->Status == obj_ServerBombPlacement::SS_ARMED)
					active_bomb = true;
			}
			if(active_bomb)
				return false;

			Bomb_ReportWinRound(1, false); // if timeout, counter terrorists won
			return false;
		}
		else
		{
			r3dOutToLog("Game: finish by timeout. tickets: %d vs %d\n", int(tickets_[0]), int(tickets_[1]));

			PKT_S2C_GameFinish_s n;
			if(ginfo_.mapType == GBGameInfo::MAPT_Conquest || ginfo_.mapType == GBGameInfo::MAPT_Deathmatch)
			{
				n.reason  = (int(tickets_[0]) == int(tickets_[1])) ? PKT_S2C_GameFinish_s::REASON_Draw : PKT_S2C_GameFinish_s::REASON_Timeout;
				n.winTeam = int(tickets_[0]) > int(tickets_[1]) ? 0 : 1;
			}
			else if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
			{
				n.reason  = PKT_S2C_GameFinish_s::REASON_Timeout;
				n.winTeam = 1; // team 0 in siege is always attacking team, so if timeout then defending team won
			}
			SendGameFinish(n, "Timeout");
			return true;
		}
	}

	// check end by tickets
	if(ginfo_.mapType == GBGameInfo::MAPT_Conquest || ginfo_.mapType == GBGameInfo::MAPT_Deathmatch)
	{
		if(tickets_[0] < 1.0f || tickets_[1] < 1.0f)
		{
			r3dOutToLog("Game: finish by tickets, %d vs %d\n", int(tickets_[0]), int(tickets_[1]));

			PKT_S2C_GameFinish_s n;
			n.reason  = PKT_S2C_GameFinish_s::REASON_Ticket;
			n.winTeam = int(tickets_[0]) > int(tickets_[1]) ? 0 : 1;
			SendGameFinish(n, "Tickets");
			return true;
		}
	}
	else if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
	{
		if(gSiegeObjMgr.GetActiveObjective()==NULL) // no more objectives
		{
			r3dOutToLog("Game: no more active siege objectives\n");

			PKT_S2C_GameFinish_s n;
			n.reason  = PKT_S2C_GameFinish_s::REASON_Ticket;
			n.winTeam = 0; // team 0 in siege is always attacking team, so if no more objectives, attacking team won
			SendGameFinish(n, "Tickets");
			return true;
		}
	}

	// check if one team left a game completely
	if(!m_isTestingGame && (m_isGameHasStarted && ginfo_.autoBalance)) // check that game already been started
	{
		int tp[2];
		GetNumPlayersInTeam(tp);
		if(tp[0]==0 || tp[1] == 0)
		{
			int numRoundsPlayed = (int)tickets_[0] + (int)tickets_[1]; // bomb mode only
			if(r3dGetTime() >= (gameEndTime_ - 90.0f) || (ginfo_.mapType == GBGameInfo::MAPT_Bomb && numRoundsPlayed > (ginfo_.respawnDelay/2))) // less than a minute and a half left (bomb mode only: and num rounds is more than half)
			{
				r3dOutToLog("Game finished. Opposite team left a game. End game is less than 1.5 minutes, remaining team won\n");

				PKT_S2C_GameFinish_s n;
				n.reason  = PKT_S2C_GameFinish_s::REASON_Ticket;
				n.winTeam = (tp[0]) > 0 ? 0 : 1;
				SendGameFinish(n, "Team left game");
				return true;
			}
			if(curPlayers_ < 2) // no one wins, game finished
			{
				r3dOutToLog("Game finished. Opposite team left a game. Less then 2 players left. Draw.\n");

				PKT_S2C_GameFinish_s n;
				n.reason  = PKT_S2C_GameFinish_s::REASON_Draw;
				n.winTeam = int(tickets_[0]) > int(tickets_[1]) ? 0 : 1;;
				SendGameFinish(n, "Team left game, less than 2 players left");
				return true;
			}
			else
			{
				// kill everybody and assign a new team
				int newTeamId = 0;
				for(int i=0; i<maxPlayers_; i++) 
				{
					obj_ServerPlayer* plr = GetPlayer(i);
					if(!plr || plr->isDead) 
						continue;

					// move to new team
					plr->SwitchTeam(newTeamId);
					newTeamId = !newTeamId;

					PKT_S2C_TeamSwitchError_s n;
					n.errorType = plr->TeamID ? PKT_S2C_TeamSwitchError_s::OK_MOVED_TEAM1 : PKT_S2C_TeamSwitchError_s::OK_MOVED_TEAM0;
					p2pSendToPeer(plr->peerId_, plr, &n, sizeof(n));
						
					DoKillPlayer(plr, plr, storecat_INVALID, true); // make sure that player will not lose score for that
				}
			}
		}
	}
	
	return false;
}

void ServerGameLogic::TrackWeaponUsage(uint32_t ItemID, int ShotsFired, int ShotsHits, int Kills)
{
	WeaponStats_s* ws = NULL;
	for(size_t i = 0, size = weaponStats_.size(); i < size; ++i)
	{
		if(weaponStats_[i].ItemID == ItemID)
		{
			ws = &weaponStats_[i];
			break;
		}
	}
	
	if(ws == NULL)
	{
		weaponStats_.push_back(WeaponStats_s());
		ws = &weaponStats_.back();
		ws->ItemID = ItemID;
	}
	
	r3d_assert(ws);
	ws->ShotsFired += ShotsFired;
	ws->ShotsHits  += ShotsHits;
	ws->Kills      += Kills;
	return;
}

void ServerGameLogic::UpdatePlayerProfiles()
{
	r3d_assert(gameFinished_);

	ProfileUpd_s* upds[MAX_NUM_PLAYERS];
	int numUpdates = 0;

	// start updating threads
	r3dOutToLog("started to UpdatePlayerProfiles()\n");
	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) 
			continue;
			
		// create update job and start thread for it
		ProfileUpd_s* upd = new ProfileUpd_s();
		upds[numUpdates++] = upd;
		upd->idx        = i;
		upd->status     = 0;
		upd->CustomerID = plr->profile_.CustomerID;
		upd->stats      = plr->RoundStats_;
		upd->plrLevelUpMin = plr->LevelUpMin;
		upd->plrLevelUpMax = plr->LevelUpMax;
		upd->MapID      = ginfo_.mapId;
		upd->MapType    = ginfo_.mapType;
		upd->TeamID     = plr->TeamID;

		// fill per-loadout stats
		for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; i++)
		{
			CLoadoutUsageTrack& lu = upd->loadoutUsage_[i];
			lu = plr->loadoutUsage_[i];
			lu.LoadoutID   = plr->profile_.ProfileData.ArmorySlots[i].LoadoutID;
			lu.HonorPoints = R3D_MAX(0, lu.HonorPoints);
		}
		
		// for achievements update
		upd->targetProfile = &plr->profile_;

		upd->hThread    = CreateThread(NULL, 0, UpdateProfileThread, upd, 0, NULL);
	}
	
	// update weapon stats in main thread because we'll be anyway waiting for profile threads
	AddWeaponStatsThread(NULL);
	
	if(numUpdates == 0)
		return;
	
	r3dOutToLog("waiting for threads to close\n");
	r3dStartFrame();
	while(1)
	{
		::Sleep(33);

		r3dEndFrame();
		r3dStartFrame();

		int threadsActive = 0;
		for(int i=0; i<numUpdates; i++) 
		{
			if(upds[i]->status == 0)
				threadsActive++;
		}
		
		if(threadsActive == 0)
			break;
	}

	r3dOutToLog("All profiles updated\n");
	return;
}

void ServerGameLogic::SendScoreToPlayers(DWORD peerId)
{
	// update players score on clients
	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) continue;
	
		PKT_S2C_SetPlayerScore_s n;
		n.score  = plr->DetailedReward_.getTotalHP();
		n.kills  = plr->RoundStats_.Kills;
		n.deaths = plr->RoundStats_.Deaths;
		if(peerId == 0) {
			p2pBroadcastToActive(plr, &n, sizeof(n));
		} else {
			p2pSendToPeer(peerId, plr, &n, sizeof(n));
		}
	}
}

void ServerGameLogic::SendRoundStatsToPlayers()
{
	// notify each player of his round statistics
	for(int i=0; i<maxPlayers_; i++) 
	{
		obj_ServerPlayer* plr = GetPlayer(i);
		if(!plr) continue;

		// send detailed round statistics to that player only
		PKT_S2C_RoundStats_s n;
		n.rscore = plr->DetailedReward_;
		n.rstat  = plr->RoundStats_;
		n.levelUpMin = plr->LevelUpMin;
		n.levelUpMax = plr->LevelUpMax;

		{
			// random reward
			float rnd = u_GetRandom(0.0f, 100.0f);
			// one time reward from server: 0-100WP, 1-LootBox1, 2-LootBox2, 3-LootBox3, 4-2xWP for 20min, 5-2xXP for 20min
			if(rnd>20.0f && rnd <= 30.0f)
			{
				n.oneTimeReward = 1;
				GiveItemToPlayer(plr, 301118);
			}
			else if(rnd>30.0f && rnd <= 40.0f)
			{
				n.oneTimeReward = 2;
				GiveItemToPlayer(plr, 301119);
			}
			else if(rnd>40.0f && rnd <= 50.0f)
			{
				n.oneTimeReward = 3;
				GiveItemToPlayer(plr, 301120);
			}
			else if(rnd>50.0f && rnd <= 75.0f) // wp
			{
				n.oneTimeReward = 4;
				GiveItemToPlayerInMinutes(plr, 301003, 20);
			}
			else if(rnd>75.0f && rnd <= 100.0f) // xp
			{
				n.oneTimeReward = 5;
				GiveItemToPlayerInMinutes(plr, 301001, 20);
			}
			else // if(rnd <= 20.0f)
			{
				n.oneTimeReward = 0;
				plr->RoundStats_.GameDollars += 100;
			}
		}
		p2pSendToPeer(plr->peerId_, NULL, &n, sizeof(n), true);
	}
}

void ServerGameLogic::SendGameClose()
{
	PKT_S2C_GameClose_s n;
	n.res = 0;
	p2pBroadcastToAll(&n, sizeof(n));
}


void ServerGameLogic::Tick()
{
	r3d_assert(maxPlayers_ > 0);
	net_->Update();
	
	const float curTime = r3dGetTime();

	// shutdown notify logic
	if(gMasterServerLogic.shuttingDown_)
	{
	  // send note every 1 sec
	  static float lastSent = 999999;
	  if(fabs(lastSent - gMasterServerLogic.shutdownLeft_) > 1.0f)
	  {
	    lastSent = gMasterServerLogic.shutdownLeft_;
	    r3dOutToLog("sent shutdown note\n");
	  
	    PKT_S2C_ShutdownNote_s n;
	    n.reason   = 0;
	    n.timeLeft = gMasterServerLogic.shutdownLeft_;
	    p2pBroadcastToAll(&n, sizeof(n), true);
	  }

	  // close game when shutdown
	  if(gMasterServerLogic.shutdownLeft_ < 0)
		throw "shutting down....";
	}

	CheckClientsSecurity();

	if(gameFinished_)
		return;

	CheckForRespawnPlayers();
	
	if(gMasterServerLogic.gotWeaponUpdate_)
	{
		gMasterServerLogic.gotWeaponUpdate_ = false;

		weaponDataUpdates_++;
		SendWeaponsInfoToPlayer(true, 0);
	}

	// check if game is fair, check only until game has started
	if(ginfo_.mapType != GBGameInfo::MAPT_Bomb)
	{
		if(!m_isGameHasStarted && gameStartCountdown<=0.0f)
			if(curPlayers_ >= ginfo_.minPlayers) // games starts when at least two players are in game
			{
				r3dOutToLog("Game Started: num players more than minimum required to start game\n");
				m_requestGameStart = true;
			}
	}

	if(bomb_nextRoundStartTimer > 0)
	{
		bomb_nextRoundStartTimer -= r3dGetFrameTime();
		if(bomb_nextRoundStartTimer <= 0)
		{
			// force kill everyone
			for(int i=0; i<maxPlayers_; i++) 
			{
				obj_ServerPlayer* plr = GetPlayer(i);
				if(!plr || plr->isDead) 
					continue;
				DoKillPlayer(plr, plr, storecat_INVALID, true);
			}	

			r3dOutToLog("Game Start: sabotage next round request\n");
			m_requestGameStart = true;
		}
	}

	if(m_requestGameStart)
	{

		PreGameCleanUp();
		
		m_requestGameStart = false;
		gameStartCountdown = 5.0f; // 5 seconds to start actual match
		if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
			gameStartCountdown = 8.0f;

		// send packets to clients informing them that game is about to start
		PKT_S2C_GameAboutToStart_s n;			
		n.gameTimeLeft = (float)ginfo_.timeLimit * 60.0f + gameStartCountdown;
		p2pBroadcastToActive(NULL, &n, sizeof(n), true);
	}
	
	if(gameStartCountdown > 0)
	{
		gameStartCountdown -= r3dGetFrameTime();
		if(gameStartCountdown<=0)
		{
			if ( m_isGameHasStarted == false ) {
				m_FirstKill = true;
			}

			StartGame(m_isGameHasStarted==false); // reset timers
			m_isGameHasStarted = true;
			if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
			{
				bomb_isDropped = false;
				// spawn everyone
				for(int i=0; i<MAX_PEERS_COUNT; ++i)
				{
					peerInfo_s& peerInfo = GetPeer(i);
					if(peerInfo.status_ == PEER_PLAYING)
					{
						if(!peerInfo.isPlayerSpectator)
						{
							if(peerInfo.player == NULL)
							{
								int spawnId = AdjustStartSpawnId(0, peerInfo.desiredTeam);
								r3dPoint3D spawnPos; float dir;
								GetSpawnPosition(spawnId, spawnPos, dir, NULL);
								float spawnProtection = gCPMgr.GetCP(spawnId)->spawnProtectionTime;
								CreateNewPlayer(i, peerInfo.desiredTeam, spawnPos, dir, peerInfo.selectedLoadoutSlot, spawnProtection);
							}
							else
							{
								peerInfo.player->SetLoadoutSlot(peerInfo.player->m_SelectedLoadout);
								RespawnPlayer(peerInfo.player);
							}
						}
						else
						{
							PKT_S2C_GameStarted_s n; // let spectator know that game has started
							n.teleport_pos = r3dPoint3D(0, 0, 0);
							n.dir = 0;
							p2pSendToPeer(i, NULL, &n, sizeof(n), true);

						}
					}
				}
				// select one of the terrorists as bomb holder
				int rndNum = (int)u_GetRandom(1.1f, 10.0f);
				while(rndNum)
				{
					for(int i=0; i<MAX_NUM_PLAYERS; ++i)
					{
						obj_ServerPlayer* plr = GetPlayer(i);
						if(plr)
						{
							if(plr->TeamID == 0)
								rndNum--;
							if(rndNum == 0)
							{
								plr->m_carryingBomb = true;

								PKT_S2C_Bomb_PlayerHasBomb_s n;
								n.playerID = toP2pNetId(plr->NetworkID);
								p2pBroadcastToActive(NULL, &n, sizeof(n));
								break;
							}
						}
					}
				}
			}
			else // not bomb mode, regular games
			{
				// send new pos for all players
				for(int i=0; i<MAX_PEERS_COUNT; i++) 
				{
					if(peers_[i].status_ >= PEER_PLAYING && peers_[i].player) 
					{
						obj_ServerPlayer* player = peers_[i].player;
						r3dPoint3D pos; float dir;
						GetSpawnPosition(gCPMgr.GetBaseIndex(player->TeamID), pos, dir, player);

						player->SetLatePacketsBarrier("PKT_S2C_GameStarted");
						player->TeleportPlayer(pos);
						player->m_PlayerRotation = dir;
						player->m_Health = player->getMaxHealth();
						player->SetLoadoutSlot(player->m_SelectedLoadout);

						// reset UAV if it was destroyed in pre-game
						if(player->uavRequested_ && player->uavId_!=invalidGameObjectID)
						{
							obj_ServerUAV* uav = (obj_ServerUAV*)GameWorld().GetObject(player->uavId_);
							if(uav)
							{
								if(uav->state_ != obj_ServerUAV::UAV_Killed)
								{
									uav->ResetHealth();
								}
								else // if killed, reset
								{
									player->uavRequested_ = false;
									player->uavId_ = invalidGameObjectID;
								}								
							}
						}

						// broadcast player position and game started event
						PKT_S2C_GameStarted_s n;
						n.teleport_pos = pos;
						n.dir = dir;
						p2pBroadcastToActive(player, &n, sizeof(n), true);
					}
				}
			}
		}
	}

	if(curPeersConnected==0)
		gameNoPeersConnectedTime += r3dGetFrameTime();
	else
		gameNoPeersConnectedTime = 0;

	// update score for players every 10 sec
	if(curTime > nextScoreUpdate_) 
	{
		nextScoreUpdate_ = curTime + 10.0f;
		SendScoreToPlayers(0);
	}

	//@@@ kill all players
	if(GetAsyncKeyState(VK_F11) & 0x8000) 
	{
		r3dOutToLog("trying to kill all players\n");
		for(int i=0; i<maxPlayers_; i++) {
			obj_ServerPlayer* plr = GetPlayer(i);
			if(!plr || plr->isDead) 
				continue;

			DoKillPlayer(plr, plr, storecat_INVALID, true);
		}
	}

	//@@@ finish game
	if(GetAsyncKeyState(VK_F12) & 0x8000) 
	{
		PKT_S2C_GameFinish_s n;
		n.reason  = PKT_S2C_GameFinish_s::REASON_Timeout;
		n.winTeam = 1;
		SendGameFinish(n, "Timeout");
		gameFinished_ = true;
	}

	UpdateGameLogic_UpdateTickets();
	UpdateGameLogic_CheckBattleZone();

	// update air strikes
	{
		// update cooldowns
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

		// check for airstrikes in flight
		for(int i=0; i<MAX_AIRSTRIKES_IN_FLIGHT; ++i)
		{
			if(g_AirstrikesInFlight[i].itemID != 0)
			{
				const AirstrikeDataset* AS = getAirstrikeByID(g_AirstrikesInFlight[i].itemID);
				if(AS)
				{
					float dist = g_AirstrikesInFlight[i].speed * r3dGetFrameTime();
					g_AirstrikesInFlight[i].distanceTraveled += dist;
					g_AirstrikesInFlight[i].speed += g_AirstrikesInFlight[i].speed * r3dGetFrameTime();

					if(g_AirstrikesInFlight[i].distanceTraveled >= (AS->BaseHeight + g_AirstrikesInFlight[i].heightOffset)) // trigger damage a little bit earlier on server, so that there is time for damage packer to arrive to client when explosion happens on client
					{
						// spawn explosion
						PKT_S2C_Airstrike_s n1;
						n1.pos = g_AirstrikesInFlight[i].hitPos;
						n1.itemID = g_AirstrikesInFlight[i].itemID;
						n1.heightOffset = 1001; 
						p2pBroadcastToActive(NULL, &n1, sizeof(n1), true);

						// spawn damage
						obj_ServerPlayer* creator = (obj_ServerPlayer*)GameWorld().GetObject(g_AirstrikesInFlight[i].Creator); // can be NULL!						
						if(creator) // if creator is NULL (player left game for example) then no damage can be dealt, as ApplyDamageToPlayer requires valid pointer!
						{
							for(int j = 0; j < ServerGameLogic::MAX_NUM_PLAYERS; j++) 
							{
								obj_ServerPlayer* plr = gServerLogic.GetPlayer(j);
								if(plr == NULL)
									continue;

								// check radius
								float dist = (plr->GetPosition() - g_AirstrikesInFlight[i].hitPos).Length();
								if(dist > AS->DamageRadius)
									continue;

								if((plr->TeamID == g_AirstrikesInFlight[i].teamID && ginfo_.friendlyFire) || plr->TeamID != g_AirstrikesInFlight[i].teamID) 
								{
									gServerLogic.ApplyDamageToPlayer(creator, plr, g_AirstrikesInFlight[i].hitPos, AS->DamageAmount, -1, 0, false, storecat_Airstrike ); // mark as support weapon for now
								}
							}
						}

						// reset
						g_AirstrikesInFlight[i].Reset();
					}
				}
				else
				{
					r3dOutToLog("Unknown airstrike ID detected (%d)\n", g_AirstrikesInFlight[i].itemID);
					g_AirstrikesInFlight[i].Reset();
				}
			}
		}
	}

	if(UpdateGameLogic_CheckEnd())
	{
		gameFinished_ = true;
	}

	static float nextDebugLog_ = 0;
	if(curTime > nextDebugLog_) 
	{
		nextDebugLog_ = curTime + 10.0f;
		r3dOutToLog("%.0f left, plrs:%d/%d, t: %d-%d\n", 
			gameEndTime_ - curTime, 
			curPlayers_, ginfo_.maxPlayers,
			int(tickets_[0]), int(tickets_[1]));
	}

	return;
}

void ServerGameLogic::DumpSimpleStats()
{
  const SimpleStats_s& ss = simpleStats_;
  
  char ltime[128];
  r3dscpy(ltime, ctime(&ss.StartTime));
  ltime[strlen(ltime)-1]=0; // remove '\n' at end

  char line[1024];
  sprintf(line, "%10s: LogId:%I64x, Joined:%2d, MaxPlayes:%2d Won:%s by %s\n", 
    ltime, 
    cfg_sessionId,
    ss.Joined,
    ss.MaxPlayers,
    ss.TeamWon,
    ss.EndReason);
    
  FILE* f = fopen("SimpleStats.txt", "a+b");
  if(f) { 
    fprintf(f, line);
    fclose(f);
  }
}

void ServerGameLogic::DumpPacketStatistics()
{
  __int64 totsent = 0;
  __int64 totrecv = 0;
  
  for(int i=0; i<R3D_ARRAYSIZE(netRecvPktSize); i++) {
    totsent += netSentPktSize[i];
    totrecv += netRecvPktSize[i];
  }

  r3dOutToLog("Packet Statistics: out:%I64d in:%I64d, k:%f\n", totsent, totrecv, (float)totsent/(float)totrecv);
  CLOG_INDENT;
  
  for(int i=0; i<R3D_ARRAYSIZE(netRecvPktSize); i++) {
    if(netSentPktSize[i] == 0 && netRecvPktSize[i] == 0)
      continue;
      
    r3dOutToLog("%3d: out:%10I64d in:%10I64d out%%:%.1f%%\n", 
      i, 
      netSentPktSize[i],
      netRecvPktSize[i],
      (float)netSentPktSize[i] * 100.0f / float(totsent));
  }
  
}

void ServerGameLogic::SendWeaponsInfoToPlayer(bool broadcast, DWORD peerId)
{
  r3dOutToLog("sending weapon info to %s: %d\n", broadcast ? "ALL" : "peer", peerId);
  
  const peerInfo_s& peer = GetPeer(peerId);

  // send weapons update
  for(int i=0; i<gWeaponArmory.getNumWeapons(); i++)
  {
    const WeaponConfig& wc = *gWeaponArmory.getWeaponConfigByIndex(i);
    
    PKT_S2C_UpdateWeaponData_s n;
    n.itemId = wc.m_itemID;
    wc.copyParametersTo(n.wi);
    
    if(broadcast) {
      // in broadcast (forced update) mode, send update for all items
      p2pBroadcastToActive(NULL, &n, sizeof(n), true);
      continue;
    }
    
    // send update only if user have that item (send AK74 update by default)
    r3d_assert(peer.haveProfile == 1);
    if(wc.m_itemID == 101001 || peer.temp_profile.getInventoryItemByID(wc.m_itemID) != -1) {
      p2pSendToPeer(peerId, NULL, &n, sizeof(n), true);
    }
  }

  // send gears update
  for(int i=0; i<gWeaponArmory.getNumGears(); i++)
  {
    const GearConfig& gc = *gWeaponArmory.getGearConfigByIndex(i);
    
    PKT_S2C_UpdateGearData_s n;
    n.itemId = gc.m_itemID;
    gc.copyParametersTo(n.gi);
    
    if(broadcast) {
      // in broadcast (forced update) mode, send update for all items
      p2pBroadcastToActive(NULL, &n, sizeof(n), true);
      continue;
    }
    
    // send update only if user have that item
    r3d_assert(peer.haveProfile == 1);
    if(peer.temp_profile.getInventoryItemByID(gc.m_itemID) != -1) {
      p2pSendToPeer(peerId, NULL, &n, sizeof(n), true);
    }
  }

  return;
}
/*
int ServerGameLogic::CheckTeamSkillAvailable(obj_ServerPlayer* plr, int skill)
{
	r3d_assert(plr);
	r3d_assert(skill >= 0 && skill < 10);

	int highestSkillAvalable = 0;
	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		if(plrToPeer_[i] == NULL)
			continue;

		obj_ServerPlayer* player = plrToPeer_[i]->player;
		if(player && player->TeamID == plr->TeamID)
		{
			float radius = 0;
			if(player->profile_.ProfileData.Skills[skill] == 4)
				radius = 15.0f;
			else if(player->profile_.ProfileData.Skills[skill] == 5)
				radius = 25.0f;
			if(radius>0)
			{
				if((player->GetPosition() - plr->GetPosition()).LengthSq() < (radius*radius))
					highestSkillAvalable = R3D_MAX(highestSkillAvalable, player->profile_.ProfileData.Skills[skill]); // save skill level
			}			
		}
	}
	return highestSkillAvalable;
}*/

bool ServerGameLogic::CheckTeamAbilityAvailable(obj_ServerPlayer* plr, int ability, float radius)
{
	r3d_assert(plr);

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
		return false;

	for(int i=0; i<MAX_NUM_PLAYERS; i++) 
	{
		if(plrToPeer_[i] == NULL)
			continue;

		obj_ServerPlayer* player = plrToPeer_[i]->player;
		if(player && player->TeamID == plr->TeamID && player!=plr)
		{
			if((player->GetPosition() - plr->GetPosition()).LengthSq() < (radius*radius))
				if(player->GetLoadoutData().hasItem(ability))
					return true;
		}
	}
	return false;
}

void ServerGameLogic::OpenChatLog()
{
	if(chatLogFile_ != NULL)
		return;
		
	// open chat file
	_mkdir("logchats");
	char fname[MAX_PATH];
	sprintf(fname, "logchats\\chat_%I64x.txt", cfg_sessionId);
	chatLogFile_ = fopen(fname, "wt");
	if(chatLogFile_)
	{
		static const BYTE UTF8_BOM[3] = { 0xEF, 0xBB, 0xBF };
		fwrite(UTF8_BOM, 1, 3, chatLogFile_);
	}
}

void ServerGameLogic::CloseChatLog()
{
	// close chat file
	if(chatLogFile_) {
		fclose(chatLogFile_);
		chatLogFile_ = NULL;
	}
}


// THIS FUNCTION IS NOT BEING USED AND HAS NOT BEEN FULLY TESTED DUE TO LACK OF PHYSICS ON THE SERVER.
// applies damage from explosion
// Note: Direction tells us if it's a directional attack as well as how much. Remember to half the arc you want. (180 degree arc, is 90 degrees)  
// 		ForwVector is the direction.  ForwVector needs to be normalized if used.
void ServerGameLogic::ApplyExplosionDamage( GameObject* fromObj, const r3dVector& explosionPos, float radius, int explosiveItemID, const r3dVector& forwVector /*= FORWARDVECTOR*/, float direction /*= 360*/ )
{
	obj_ServerPlayer* explosionOwner = IsServerPlayer(fromObj);

	// should be impossible. 
	if( explosionOwner == NULL ) {
		r3dOutToLog("Got an explosion with no owner, so no explosion.");
		return;
	}

	float radius_incr = 1.0f;
	int ExpRadiusSkill = explosionOwner->GetLoadoutData().getSkillLevel(CUserSkills::SPEC_ExplosiveRadius);
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
		if(obj->ObjTypeFlags & OBJTYPE_Human)
		{
			r3d_assert(obj->NetworkID);
			obj_ServerPlayer* objPlayer = (obj_ServerPlayer*)obj;

			// skip friendly fire, but deal damage to yourself to prevent obvious shooting RPG when enemy is right next to you
			if((ginfo_.friendlyFire == 0) && ( explosionOwner->TeamID == objPlayer->TeamID) && (objPlayer != explosionOwner))
				continue;

			float dist_to_obj = (obj->GetPosition() - explosionPos).Length();
			if(dist_to_obj < ( radius * radius_incr ) )
			{
				// raycast to make sure that player isn't behind a wall
				r3dPoint3D orig = r3dPoint3D(obj->GetPosition().x, obj->GetPosition().y+.5f, obj->GetPosition().z);
				r3dPoint3D dir = r3dPoint3D(explosionPos.x-obj->GetPosition().x, explosionPos.y-(obj->GetPosition().y + .5f), explosionPos.z - obj->GetPosition().z);
				float rayLen = dir.Length();
				dir.Normalize();
				bool isVisible = true;
				float minDotDirection = cos( R3D_DEG2RAD( direction ) );
				if( direction == FULL_AREA_EXPLOSION || dir.Dot( forwVector ) > minDotDirection ) {

					float damagePercentage = 1.0f;
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
								PhysicsCallbackObject* target;
								if( hit.shape && (target = static_cast<PhysicsCallbackObject*>(hit.shape->getActor().userData)))
								{
									// this currently only handles one piercable object between the player and explosion.  More complexity might be valid here. 
									GameObject* obj = target->isGameObject();
									if ( obj )
									{
										damagePercentage = obj->m_BulletPierceable / 100.0f;
									}
								}
							}
						}
					}

					const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(explosiveItemID);

					// check distance
					float dist = (explosionPos - objPlayer->GetPosition()).Length();
					if(dist > wc->m_AmmoArea)
					{    
						continue;
					}

					float damage = wc->m_AmmoDamage*(1.0f-(dist/wc->m_AmmoArea));
					damage *= damagePercentage; // damage through wall

					r3dOutToLog("Explosion from %s to %s, damage=%.2f\n", fromObj->Name.c_str(), objPlayer->Name.c_str(), damage); CLOG_INDENT;
					ApplyDamage(fromObj, objPlayer, explosionPos, damage, true, wc->category);
				}
			}
		}
	}
}

// Called at the beginning of game modes, and at the beginning of every round of Sabotage. 
void ServerGameLogic::PreGameCleanUp()
{
	// reward players used to capture
	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if( plr != NULL ) {
			// find all the players UAVs and kill them! Boom.
			if(plr->uavId_ != invalidGameObjectID)
			{
				obj_ServerUAV* uav = (obj_ServerUAV*)GameWorld().GetObject(plr->uavId_);
				r3d_assert(uav);
				uav->DoDestroy(plr->NetworkID);
			}
			
			plr->uavRequested_ = false;
		}
	}
	
}

// cpIdx is the control point you want to spawn off of.  If you have a specific location use the cpLocIdx. 
// If you just know which Control Point to use, use -1 on the cpLocIdx
void ServerGameLogic::FindRespawnPoint( int& cpIdx, int& cpLocIdx, obj_ServerPlayer* plr )
{
	cpIdx = plr->NextSpawnNode;
	cpLocIdx = -1;

	if(ginfo_.mapType == GBGameInfo::MAPT_Deathmatch) // in deathmatch, spawn at random neutral CP
	{
		if(gCPMgr.numNeutralControlPoints > 0)
		{
			bool validSpawn = false; 
			int attemptCount = 0;

			do{
				validSpawn = true;
				unsigned long index = u_random(gCPMgr.numNeutralControlPoints);
				cpIdx = gCPMgr.neutralControlPoints[index];

				// Test against all the enemies. 
				const BaseControlPoint* cp = gCPMgr.GetCP(cpIdx);

				assert( cp->m_NumSpawnPoints > 0 );

				index = u_random(cp->m_NumSpawnPoints);
				cpLocIdx = index;
				r3dPoint3D position;
				float direction;
				cp->getSpawnPointByIdx( cpLocIdx, position, direction);

				// the first ten attempts we want to try to find a good point. 
				if( attemptCount < 10 ) {
				
					attemptCount++;
					for( int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
					{
						obj_ServerPlayer* enemy = gServerLogic.GetPlayer(i);
						// skip non existant or dead enemies. 
						if( enemy == NULL || enemy->isDead)
						{
							continue;
						}

						// ignore your team mates. 
						if( plr->TeamID == enemy->TeamID )
						{
							continue;
						}
						
						r3dVector distance = position - enemy->GetPosition();
						if ( distance.LengthSq() < MIN_SPAWN_DISTANCE_TO_ENEMY * MIN_SPAWN_DISTANCE_TO_ENEMY ) {
							validSpawn = false;
							break;
						}
					}
				}
			}while( validSpawn == false );

			r3dOutToLog( "Found a valid respawn point after %d attempts\n", attemptCount );
		}
		else
		{
			// if no neutral, then always spawn at the base!
			cpIdx = gCPMgr.GetBaseIndex(plr->TeamID);
			cpLocIdx = -1;
		}
	}

	// check for valid spawn id
	if(cpIdx != -1 && (cpIdx < 0 || cpIdx >= gCPMgr.NumCP()))
	{
		r3dOutToLog("!!! tried to respawn at bad cpIdx %d\n", cpIdx);
		cpIdx = -1;
		cpLocIdx = -1;
	}

	// check if we can spawn on this point, or is it owned by the other team.
	if(cpIdx != -1 && gCPMgr.GetCP(cpIdx)->GetSpawnTeamId() != plr->TeamID && (ginfo_.mapType == GBGameInfo::MAPT_Conquest)) 
	{
		cpIdx = -1;
		cpLocIdx = -1;
	}

	if(cpIdx == -1)
	{
		cpIdx = GetClosestControlPoint(plr->TeamID, plr->GetPosition());
		cpLocIdx = -1;
	}

	if(ginfo_.mapType == GBGameInfo::MAPT_Siege)
	{
		// find objective
		obj_ServerSiegeObjective* objective = gSiegeObjMgr.GetActiveObjective();
		r3d_assert(objective); // shouldn't be null!
		cpIdx = gCPMgr.getCPIndex(objective->getCP(plr->TeamID));
		cpLocIdx = -1;
	}

	if(ginfo_.mapType == GBGameInfo::MAPT_Bomb)
	{
		cpIdx = gCPMgr.GetBaseIndex(plr->TeamID);
		cpLocIdx = -1; 
	}	
}

bool ServerGameLogic::canRecordAchievements()
{
	return m_isGameHasStarted;
}

void ServerGameLogic::CheckAchievementsOnKill( obj_ServerPlayer* fromPlr, obj_ServerPlayer* targetPlr, const STORE_CATEGORIES dmgCat )
{
	fromPlr->checkAchievementByValue( ACHIEVEMENT_30KILLS_IN_A_GAME, fromPlr->RoundStats_.Kills );

	if( ginfo_.mapType == GBGameInfo::MAPT_Deathmatch ) {
		fromPlr->incrementAchievement( ACHIEVEMENT_DM_KILL_1000_PLAYERS, 1 );
	}

	fromPlr->incrementAchievement( ACHIEVEMENT_KILL_10000_PLAYERS, 1 );
	fromPlr->checkAchievementByValue( ACHIEVEMENT_5KILLS_IN_A_ROW, fromPlr->numKillWithoutDying );
	fromPlr->checkAchievementByValue( ACHIEVEMENT_10KILLS_IN_A_ROW, fromPlr->numKillWithoutDying );
	fromPlr->checkAchievementByValue( ACHIEVEMENT_15KILLS_IN_A_ROW, fromPlr->numKillWithoutDying );
	fromPlr->checkAchievementByValue( ACHIEVEMENT_20KILLS_IN_A_ROW, fromPlr->numKillWithoutDying );

	int fromPlrIdx = GetPeer( fromPlr->peerId_ ).playerIdx;
	int targetPlrIdx = GetPeer( targetPlr->peerId_ ).playerIdx;

	fromPlr->m_KillsPerPlayer[ targetPlrIdx ]++;

	fromPlr->checkAchievementByValue( ACHIEVEMENT_DOMINATE, fromPlr->m_KillsPerPlayer[ targetPlrIdx ] );
	fromPlr->checkAchievementByValue( ACHIEVEMENT_KILLDOMINATOR, targetPlr->m_KillsPerPlayer[ fromPlrIdx ] );

	// domination removal. 
	targetPlr->m_KillsPerPlayer[ fromPlrIdx ] = 0;

	if(fromPlr->isDead && dmgCat == storecat_GRENADES)
	{
		fromPlr->markAchievementComplete( ACHIEVEMENT_GRENADE_KILL_AFTER_DEATH);
	}

	if( dmgCat == storecat_SUPPORT ) 
	{
		fromPlr->m_RocketKills++;
		fromPlr->checkAchievementByValue( ACHIEVEMENT_2KILLS_1ROCKET, fromPlr->m_RocketKills );
	}

	if( dmgCat == storecat_Airstrike )
	{
		fromPlr->m_AirstrikeKills++;
		fromPlr->checkAchievementByValue( ACHIEVEMENT_3ENEMIES_ONE_AIRSTRIKE, fromPlr->m_AirstrikeKills );
	}

	fromPlr->m_CampingKills++;;
	fromPlr->checkAchievementByValue( ACHIEVEMENT_3ENEMIES_WITHOUT_MOVING, fromPlr->m_CampingKills );

	if( dmgCat == storecat_SNP ) 
	{
		fromPlr->m_SniperKillsWithoutMissing++;
		fromPlr->checkAchievementByValue( ACHIEVEMENT_3ENEMIES_IN_A_ROW_WITH_SNIPER, fromPlr->m_SniperKillsWithoutMissing );
		if( fromPlr->m_LastSniperShotZoomed == false && (fromPlr->GetPosition()- targetPlr->GetPosition()).LengthSq() > 20.0f * 20.0f ) 
		{
			fromPlr->markAchievementComplete( ACHIEVEMENT_SNIPER_KILL_NO_ZOOM );
		}
	}
}
