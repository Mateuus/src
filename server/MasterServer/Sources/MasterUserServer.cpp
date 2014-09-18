#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"
#include <shellapi.h>

#pragma warning(disable: 4065)	// switch statement contains 'default' but no 'case' labels

#include "MasterUserServer.h"
#include "MasterGameServer.h"

using namespace NetPacketsGameBrowser;

static	r3dNetwork	clientNet;
	CMasterUserServer gMasterUserServer;

const int GBPKT_C2M_CreateGame_s::timeLimitChoices[3]  = {20, 30, 40};
const int GBPKT_C2M_CreateGame_s::conquestStartTickets[3]  = {200, 300, 500};
const int GBPKT_C2M_CreateGame_s::timeLimitChoicesPractice[2]  = {20, 60};
const int GBPKT_C2M_CreateGame_s::timeLimitChoicesCybersport[4]  = {2, 3, 4, 5};
const int GBPKT_C2M_CreateGame_s::numRoundsChoices[3] = {9, 12, 15};

static bool IsNullTerminated(const char* data, int size)
{
  for(int i=0; i<size; i++) {
    if(data[i] == 0)
      return true;
  }

  return false;
}

CMasterUserServer::CMasterUserServer()
{
  return;
}

CMasterUserServer::~CMasterUserServer()
{
  SAFE_DELETE_ARRAY(peers_);
}

void CMasterUserServer::Start(int port, int in_maxPeerCount)
{
  r3d_assert(in_maxPeerCount);
  MAX_PEERS_COUNT = in_maxPeerCount;
  peers_          = new peer_s[MAX_PEERS_COUNT];

  numConnectedPeers_ = 0;
  maxConnectedPeers_ = 0;
  curPeerUniqueId_   = 0;

  clientNet.Initialize(this, "clientNet");
  if(!clientNet.CreateHost(port, MAX_PEERS_COUNT)) {
    r3dError("CreateHost failed\n");
  }
  
  r3dOutToLog("MasterUserServer started at port %d, %d CCU\n", port, MAX_PEERS_COUNT);
  
  return;
}

void CMasterUserServer::PrintStats()
{
  const float curTime = r3dGetTime();
  
  static float nextShutLog_ = 0;
  if(gMasterGameServer.shuttingDown_ && curTime > nextShutLog_) {
    nextShutLog_ = curTime + 1.0f;
    r3dOutToLog("SHUTDOWN in %.0f\n", gMasterGameServer.shutdownLeft_);
  }
  
  // dump some useful statistics
  static float nextDebugLog_ = 0;
  if(curTime < nextDebugLog_) 
    return;

  nextDebugLog_ = curTime + 10.0f;
    
  int numGames = 0;
  int numCCU   = 0;
  for(CMasterGameServer::TGamesList::const_iterator it = gMasterGameServer.games_.begin(); 
      it != gMasterGameServer.games_.end();
      ++it)
  {
    const CServerG* game = it->second;

    numCCU += game->curPlayers_;
    numGames++;
  }
    
  static int maxCCU = 0;
  if(numCCU > maxCCU) maxCCU = numCCU;

  FILE* f = fopen("MasterServer_ccu.txt", "wt");

  char buf[1024];    
  sprintf(buf, "MSINFO: %d (%d max) peers, %d CCU in %d games, MaxCCU: %d\n",
    numConnectedPeers_,
    maxConnectedPeers_,
    numCCU,
    numGames,
    maxCCU); 

  r3dOutToLog(buf); CLOG_INDENT;
  if(f) fprintf(f, buf);
  
  // log supervisors status
  for(CMasterGameServer::TSupersList::const_iterator it = gMasterGameServer.supers_.begin();
      it != gMasterGameServer.supers_.end();
      ++it)
  {
    const CServerS* super = it->second;
    sprintf(buf, "%s, games:%d/%d, players:%d/%d\n", 
      super->GetName(),
      super->GetExpectedGames(), super->maxGames_,
      super->GetExpectedPlayers(), super->maxPlayers_);
      
    r3dOutToLog(buf);
    if(f) fprintf(f, buf);
  }
  
  if(f) fclose(f);
  
  return;
}

void CMasterUserServer::Tick()
{
  const float curTime = r3dGetTime();
  
  net_->Update();
  
  DisconnectIdlePeers();
  CheckWaitBuckets();
  
  PrintStats();

  return;
}

void CMasterUserServer::Stop()
{
  if(net_)
    net_->Deinitialize();
}

void CMasterUserServer::DisconnectIdlePeers()
{
  const float IDLE_PEERS_CHECK = 0.2f;	// do checks every N seconds
  const float VALIDATE_DELAY   = 5.0f;	// peer have this N seconds to validate itself

  static float nextCheck = -1;
  const float curTime = r3dGetTime();
  if(curTime < nextCheck)
    return;
  nextCheck = curTime + IDLE_PEERS_CHECK;
  
  for(int i=0; i<MAX_PEERS_COUNT; i++) 
  {
    if(peers_[i].status == PEER_Connected && curTime > peers_[i].connectTime + VALIDATE_DELAY) {
      DisconnectCheatPeer(i, "validation time expired");
      continue;
    }
  }
  
  return;
}

void CMasterUserServer::DisconnectCheatPeer(DWORD peerId, const char* message, ...)
{
	char buf[2048] = {0};

  if(message)
  {
	  va_list ap;
	  va_start(ap, message);
	  StringCbVPrintfA(buf, sizeof(buf), message, ap);
	  va_end(ap);
  }

  DWORD ip = net_->GetPeerIp(peerId);
  if(message)
  {
    r3dOutToLog("!!! cheat: peer%d[%s], reason: %s\n", 
      peerId, 
      inet_ntoa(*(in_addr*)&ip),
      buf);
  }

  net_->DisconnectPeer(peerId);
  
  // fire up disconnect event manually, enet might skip if if other peer disconnect as well
  OnNetPeerDisconnected(peerId);
}

bool CMasterUserServer::DisconnectIfShutdown(DWORD peerId)
{
  if(!gMasterGameServer.shuttingDown_)
    return false;

  GBPKT_M2C_ShutdownNote_s n;
  n.reason = 0;
  net_->SendToPeer(&n, sizeof(n), peerId);
  
  DisconnectCheatPeer(peerId, NULL);
  return true;
}

//TODO: add logs to see what exactly happened
bool CMasterUserServer::Validate(const GBPKT_C2M_CreateGame_s& n)
{
  if(!IsNullTerminated(n.name, sizeof(n.name)))
    return false;

  if(!IsNullTerminated(n.pwd, sizeof(n.pwd)))
    return false;
    
  // validate max num players
  if(n.isBasicGame)
  {
	  switch(n.mapId)
	  {
	  case GBGameInfo::MAPID_WO_Crossroads16:
	  case GBGameInfo::MAPID_WO_Crossroads2:
	  case GBGameInfo::MAPID_BurningSea:
	  case GBGameInfo::MAPID_WO_Valley:
	  case GBGameInfo::MAPID_WO_Torn:
	  case GBGameInfo::MAPID_WO_Grozny:
		  if(n.maxPlayers != 32)
			  return false;
		  break;
	  case GBGameInfo::MAPID_WO_Nightfall_CQ:
		  if(n.maxPlayers != 24)
			  return false;
		  break;
	  case GBGameInfo::MAPID_WO_Shipyard:
	  case GBGameInfo::MAPID_WO_Jungle02:
	  case GBGameInfo::MAPID_WO_Citadel_DM:
	  case GBGameInfo::MAPID_WO_TornTown:
	  case GBGameInfo::MAPID_WO_Wasteland:
	  case GBGameInfo::MAPID_WO_EasternBunkerTDM:
		  if(n.maxPlayers != 16)
			  return false;
		  break;
		  // sabo games
	  case GBGameInfo::MAPID_WO_Torn_CT:
		  if(n.maxPlayers != 12)
			  return false;
		  break;
	  case GBGameInfo::MAPID_WO_Inferno:
		  if(n.maxPlayers != 16)
			  return false;
		  break;
	  default:
		  if(n.maxPlayers != 16)
			  return false;
		  break;
	  }
  }
  else // premium game
  {
	  switch(n.mapId)
	  {
		  // ptumik: let's just check for all valid number of players, but do not check each map for separate maxPlayers (makes it easier)
	  case GBGameInfo::MAPID_WO_Crossroads16:
	  case GBGameInfo::MAPID_WO_Crossroads2:
	  case GBGameInfo::MAPID_BurningSea:
	  case GBGameInfo::MAPID_WO_Valley:
	  case GBGameInfo::MAPID_WO_Nightfall_CQ:
	  case GBGameInfo::MAPID_WO_Torn:
	  case GBGameInfo::MAPID_WO_Grozny:
	  case GBGameInfo::MAPID_WO_Shipyard:
	  case GBGameInfo::MAPID_WO_Jungle02:
	  case GBGameInfo::MAPID_WO_Citadel_DM:
	  case GBGameInfo::MAPID_WO_TornTown:
	  case GBGameInfo::MAPID_WO_Wasteland:
	  case GBGameInfo::MAPID_WO_EasternBunkerTDM:
		  if(n.maxPlayers != 10 && n.maxPlayers != 16 && n.maxPlayers != 24 && n.maxPlayers != 32 && n.maxPlayers != 64)
			  return false;
		  break;
		  // sabo games
	  case GBGameInfo::MAPID_WO_Torn_CT:
		  if(n.maxPlayers != 10 && n.maxPlayers != 16)
			  return false;
		  break;
	  case GBGameInfo::MAPID_WO_Inferno:
		  if(n.maxPlayers != 10 && n.maxPlayers != 16 && n.maxPlayers != 32)
			  return false;
		  break;
	  default:
		  if(n.maxPlayers != 16)
			  return false;
		  break;
	  }
  }

  if(n.mapType == GBGameInfo::MAPT_Bomb)
  {
	  if(n.numRoundsVar >= R3D_ARRAYSIZE(GBPKT_C2M_CreateGame_s::numRoundsChoices))
		  return false;
	  if(n.timeLimitVar >= R3D_ARRAYSIZE(GBPKT_C2M_CreateGame_s::timeLimitChoicesCybersport))
		  return false;
  }
  else
  {
	  if(n.isBasicGame)
	  {
		  if(n.timeLimitVar >= R3D_ARRAYSIZE(GBPKT_C2M_CreateGame_s::timeLimitChoicesPractice))
			  return false;
	  }
	  else
	  {
		  if(n.timeLimitVar >= R3D_ARRAYSIZE(GBPKT_C2M_CreateGame_s::timeLimitChoices))
			  return false;
	  }
  }
    
  //TODO: maybe check for map availability?
  if(n.mapId >= GBGameInfo::MAPID_MAX_ID)
    return false;
  
  return true;    
}

bool CMasterUserServer::Validate(const GBPKT_C2M_JoinGameReq_s& n)
{
  if(!IsNullTerminated(n.pwd, sizeof(n.pwd)))
    return false;

  return true;    
}

bool CMasterUserServer::Validate(const GBPKT_C2M_JoinFriendGameReq_s& n)
{
  if(!IsNullTerminated(n.pwd, sizeof(n.pwd)))
    return false;

  return true;    
}

void CMasterUserServer::OnNetPeerConnected(DWORD peerId)
{
  peer_s& peer = peers_[peerId];
  r3d_assert(peer.status == PEER_Free);
  
  curPeerUniqueId_++;
  peer.peerUniqueId = (peerId << 16) | (curPeerUniqueId_ & 0xFFFF);
  peer.status       = PEER_Connected;
  peer.connectTime  = r3dGetTime();
  peer.lastReqTime  = r3dGetTime() - 1.0f; // minor hack to avoid check for 'too many requests'
  
  // send validate packet, so client can check version right now
  GBPKT_ValidateConnectingPeer_s n;
  n.version = GBNET_VERSION;
  n.key1    = 0;
  net_->SendToPeer(&n, sizeof(n), peerId, true);
  
  numConnectedPeers_++;
  maxConnectedPeers_ = R3D_MAX(maxConnectedPeers_, numConnectedPeers_);
  
  return;
}

void CMasterUserServer::OnNetPeerDisconnected(DWORD peerId)
{
  peer_s& peer = peers_[peerId];
  
  if(peer.status != PEER_Free)
    numConnectedPeers_--;
  
  //r3dOutToLog("master: client disconnected\n");
  peer.status       = PEER_Free;
  peer.peerUniqueId = 0;
  return;
}

bool CMasterUserServer::DoValidatePeer(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize)
{
  peer_s& peer = peers_[peerId];
  
  if(peer.status >= PEER_Validated)
    return true;

  // we still can receive packets after peer was force disconnected
  if(peer.status == PEER_Free)
    return false;

  r3d_assert(peer.status == PEER_Connected);
  if(PacketData->EventID != GBPKT_ValidateConnectingPeer) {
    DisconnectCheatPeer(peerId, "wrong validate packet id");
    return false;
  }

  const GBPKT_ValidateConnectingPeer_s& n = *(GBPKT_ValidateConnectingPeer_s*)PacketData;
  if(sizeof(n) != PacketSize) {
    DisconnectCheatPeer(peerId, "wrong validate packet size");
    return false;
  } 
  if(n.version != GBNET_VERSION) {
    DisconnectCheatPeer(peerId, "wrong validate version");
    return false;
  }
  if(n.key1 != GBNET_KEY1) {
    DisconnectCheatPeer(peerId, "wrong validate key");
    return false;
  }
  
  // send server info to client
  GBPKT_M2C_ServerInfo_s n1;
  n1.serverId = BYTE(gMasterGameServer.masterServerId_);
  net_->SendToPeer(&n1, sizeof(n1), peerId, true);
  
  // set peer to validated
  peer.status = PEER_Validated;
  return false;
}

#define DEFINE_PACKET_HANDLER_MUS(xxx) \
    case xxx: \
    { \
      const xxx##_s& n = *(xxx##_s*)PacketData; \
      if(sizeof(n) != PacketSize) { \
        DisconnectCheatPeer(peerId, "wrong %s size %d vs %d", #xxx, sizeof(n), PacketSize); \
        break; \
      } \
      if(!Validate(n)) { \
        DisconnectCheatPeer(peerId, "invalid %s", #xxx); \
        break; \
      } \
      On##xxx(peerId, n); \
      break; \
    }

void CMasterUserServer::OnNetData(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize)
{
  if(PacketSize < sizeof(r3dNetPacketHeader)) {
    DisconnectCheatPeer(peerId, "too small packet");
    return;
  }

  // wait for validating packet
  if(!DoValidatePeer(peerId, PacketData, PacketSize))
    return;
    
  if(DisconnectIfShutdown(peerId))
    return;

  peer_s& peer = peers_[peerId];
  r3d_assert(peer.status >= PEER_Validated);
  
  const float curTime = r3dGetTime();
  
  // check for valid request overflows
  if(curTime < peer.lastReqTime + 0.2f) {
    DisconnectCheatPeer(peerId, "too many requests per sec");
    return;
  }
  peer.lastReqTime = curTime;
      
  switch(PacketData->EventID) 
  {
    default:
      DisconnectCheatPeer(peerId, "invalid packet id");
      return;

    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_RefreshList)
    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_CreateGame);
    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_JoinGameReq);
    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_JoinFriendGameReq);
    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_QuickGameReq);
    DEFINE_PACKET_HANDLER_MUS(GBPKT_C2M_NextRoundReq);
  }
  
  return;
}

void CMasterUserServer::OnGBPKT_C2M_RefreshList(DWORD peerId, const GBPKT_C2M_RefreshList_s& n)
{
  //r3dOutToLog("sending session list to client%d\n", peerId);

  { // start list
    CREATE_PACKET(GBPKT_M2C_StartGamesList, n);
    net_->SendToPeer(&n, sizeof(n), peerId);
  }
  
  // send supervisors data
  for(CMasterGameServer::TSupersList::iterator it = gMasterGameServer.supers_.begin(); it != gMasterGameServer.supers_.end(); ++it)
  {
    const CServerS* super = it->second;
    
    GBPKT_M2C_SupervisorData_s n;
    n.ID     = BYTE(super->id_);
    n.ip     = super->ip_;
    n.region = super->region_;
    net_->SendToPeer(&n, sizeof(n), peerId);
  }

  // send games
  for(CMasterGameServer::TGamesList::iterator it = gMasterGameServer.games_.begin(); it != gMasterGameServer.games_.end(); ++it) 
  {
    const CServerG* game = it->second;
    if(game->isValid() == false)
      continue;
      
    if(game->isFinished())
      continue;
      
    BYTE gameStatus = 0;
    gameStatus |= game->isFull()?2:0;
    gameStatus |= game->isTooLate()?4:0;
    gameStatus |= game->isPassworded()?8:0;
    gameStatus |= game->isClosedToJoin()?16:0;

    CREATE_PACKET(GBPKT_M2C_GameData, n);
    n.gameId     = game->id_;
    n.info       = game->info_.ginfo;
    n.status     = gameStatus;
    n.curPlayers = (BYTE)game->curPlayers_;

    net_->SendToPeer(&n, sizeof(n), peerId);
  }

  { // end list
    CREATE_PACKET(GBPKT_M2C_EndGamesList, n);
    net_->SendToPeer(&n, sizeof(n), peerId);
  }
  
  
  return;
}

void CMasterUserServer::OnGBPKT_C2M_CreateGame(DWORD peerId, const GBPKT_C2M_CreateGame_s& n)
{
  // see if user supplied correct create game key
  {
    if(n.createGameKey != gMasterGameServer.createGameKey1_ && 
       n.createGameKey != gMasterGameServer.createGameKey2_)
    {
      GBPKT_M2C_JoinGameAns_s ans;
      ans.result = GBPKT_M2C_JoinGameAns_s::rWrongCreateGameKey;
      net_->SendToPeer(&ans, sizeof(ans), peerId, true);

      r3dOutToLog("!!! CGK: %d vs %d %d\n", n.createGameKey, gMasterGameServer.createGameKey1_, gMasterGameServer.createGameKey2_);
      DisconnectCheatPeer(peerId, "bad createGameKey in GBPKT_C2M_CreateGame_s");
      return;
    }
  }
      
  // create game info
  GBGameInfo ginfo;
  r3dscpy(ginfo.name, n.name);
  ginfo.mapId        = n.mapId;
  ginfo.mapType      = n.mapType;
  ginfo.region       = n.region;
  ginfo.friendlyFire = n.friendlyFire;
  ginfo.autoBalance  = n.autoBalance;
  ginfo.maxPlayers   = n.maxPlayers;
  if(ginfo.mapType == GBGameInfo::MAPT_Bomb)
  {
	  if(n.isBasicGame)
	  {
		ginfo.respawnDelay = 9;
		ginfo.timeLimit = 3;
	  }
	  else
	  {
		  ginfo.respawnDelay = GBPKT_C2M_CreateGame_s::numRoundsChoices[n.numRoundsVar];
		  ginfo.timeLimit    = GBPKT_C2M_CreateGame_s::timeLimitChoicesCybersport[n.timeLimitVar];
	  }
  }
  else
  {
    if(n.isBasicGame)
    {
      ginfo.timeLimit    = GBPKT_C2M_CreateGame_s::timeLimitChoicesPractice[n.timeLimitVar];
    }
    else
    {
      ginfo.timeLimit    = GBPKT_C2M_CreateGame_s::timeLimitChoices[n.timeLimitVar];
	  if(ginfo.mapType == GBGameInfo::MAPT_Conquest)
	  {
		  ginfo.startTickets = GBPKT_C2M_CreateGame_s::conquestStartTickets[n.timeLimitVar];
	  }
      ginfo.autoBalance  = 1;
//    ginfo.friendlyFire = 0;
    }
    ginfo.respawnDelay = 3; // TODO: this should be loaded from level data, as designers want to control that crap
  }
      
  ginfo.minLevel     = n.minPlayerLevel;
  ginfo.maxLevel     = n.maxPlayerLevel;

  ginfo.minPlayers   = 2;
  switch(ginfo.mapId)
  {
  case GBGameInfo::MAPID_WO_Crossroads16:
  case GBGameInfo::MAPID_WO_Crossroads2:
  case GBGameInfo::MAPID_BurningSea:
  case GBGameInfo::MAPID_WO_Torn:
  case GBGameInfo::MAPID_WO_Grozny:
  case GBGameInfo::MAPID_WO_Nightfall_CQ:
  case GBGameInfo::MAPID_WO_TornTown:
	  ginfo.minPlayers = 6;
	  break;
  case GBGameInfo::MAPID_WO_Jungle02:
  case GBGameInfo::MAPID_WO_Citadel_DM:
  case GBGameInfo::MAPID_WO_Wasteland:
  case GBGameInfo::MAPID_WO_EasternBunkerTDM:
  case GBGameInfo::MAPID_WO_Shipyard:
	  ginfo.minPlayers = 4;
	  break;
  case GBGameInfo::MAPID_WO_Inferno:
  case GBGameInfo::MAPID_WO_Torn_CT:
	  ginfo.minPlayers = 4;
	  break;
  }

  const char* pwd = n.pwd;
  if(n.isBasicGame) // do not allow passwords in non premium games
	  pwd = "";

  // set new game data info
  ginfo.region = n.region;
  CMSNewGameData ngd(ginfo, pwd, n.CustomerID); // no password in ranked game
      
  GBPKT_M2C_JoinGameAns_s ans;
  do 
  {
    r3dOutToLog("requested new game, %d players, pwd:%s\n", ngd.ginfo.maxPlayers, ngd.pwd);
  
    // user can have only one game
    if(gMasterGameServer.HaveGameByCustomerId(ngd.CustomerID))
    {
      ans.result = GBPKT_M2C_JoinGameAns_s::rHaveCreatedGame;
      break;
    }
    
    CreateNewGame(ngd, ans);
    break;
  } while (0);
      
  r3d_assert(ans.result != GBPKT_M2C_JoinGameAns_s::rUnknown);
  net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  return;
}


void CMasterUserServer::OnGBPKT_C2M_JoinGameReq(DWORD peerId, const GBPKT_C2M_JoinGameReq_s& n)
{
  GBPKT_M2C_JoinGameAns_s ans;
  do 
  {
    CServerG* game = gMasterGameServer.GetGameByGameId(n.gameId);
    if(!game) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rGameNotFound;
      break;
    }
    
    // can't join game - server is waiting for previous round players
    if(game->isClosedToJoin()) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rJoinDelayActive;
      break;
    }
    
    //NOTE: level restriction is enforced in client for this packet.
    //if(n.playerLevel...
  
    DoJoinGame(game, n.CustomerID, n.pwd, ans);
    break;
  } while (0);

  r3d_assert(ans.result != GBPKT_M2C_JoinGameAns_s::rUnknown);
  net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  return;
}

void CMasterUserServer::OnGBPKT_C2M_JoinFriendGameReq(DWORD peerId, const GBPKT_C2M_JoinFriendGameReq_s& n)
{
  GBPKT_M2C_JoinGameAns_s ans;
  
  do 
  {
    CServerG* game = gMasterGameServer.GetGameBySessionId(n.sessionId);
    if(!game) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rGameNotFound;
      break;
    }

    // even if you're friend of player, you still can't join game - server is waiting for previous round players
    if(game->isClosedToJoin()) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rJoinDelayActive;
      break;
    }
  
    // check level restrictions
    if(n.playerLevel < game->getGameInfo().minLevel && !n.playerOKToJoinHighLevelGame) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rLevelTooLow;
      break;
    }
    if(n.playerLevel > game->getGameInfo().maxLevel) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rLevelTooHigh;
      break;
    }

    DoJoinGame(game, n.CustomerID, n.pwd, ans);
    break;
  } while (0);

  r3d_assert(ans.result != GBPKT_M2C_JoinGameAns_s::rUnknown);
  net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  return;
}

void CMasterUserServer::OnGBPKT_C2M_QuickGameReq(DWORD peerId, const GBPKT_C2M_QuickGameReq_s& n)
{
  GBPKT_M2C_JoinGameAns_s ans;
  
  CServerG* game = gMasterGameServer.GetQuickJoinGame(n.gameMap, n.gameMode, (EGBGameRegion)n.region, n.playerLevel);
  // in case some region game wasn't available, repeat search without specifying filter
  if(game == NULL && n.region != GBNET_REGION_Unknown)
  {
    CLOG_INDENT;
    game = gMasterGameServer.GetQuickJoinGame(n.gameMap, n.gameMode, GBNET_REGION_Unknown, n.playerLevel);
  }
  
  if(!game) {
    ans.result = GBPKT_M2C_JoinGameAns_s::rGameNotFound;
    net_->SendToPeer(&ans, sizeof(ans), peerId, true);
    return;
  }

  game->AddJoiningPlayer(n.CustomerID);
  
  ans.result    = GBPKT_M2C_JoinGameAns_s::rOk;
  ans.ip        = game->ip_;
  ans.port      = game->info_.port;
  ans.sessionId = game->info_.sessionId;
  net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  return;
}

void CMasterUserServer::OnGBPKT_C2M_NextRoundReq(DWORD peerId, const GBPKT_C2M_NextRoundReq_s& n)
{
  r3dOutToLog("NextRoundReq %d\n", n.CustomerID); CLOG_INDENT;
  
  bucket_s* bck = GetWaitBucket(n);
  r3d_assert(bck);
  
  // register that user in waiting list.
  waiter_s w;
  w.CustomerID   = n.CustomerID;
  w.peerUniqueId = peers_[peerId].peerUniqueId;
  bck->waiters.push_back(w);
  
  return;
}

void CMasterUserServer::DoJoinGame(CServerG* game, DWORD CustomerID, const char* pwd, GBPKT_M2C_JoinGameAns_s& ans)
{
  r3d_assert(game);
  
  if(game->isFull()) {
    ans.result = GBPKT_M2C_JoinGameAns_s::rGameFull;
    return;
  }
  if(game->isFinished() || game->isTooLate()) {
    ans.result = GBPKT_M2C_JoinGameAns_s::rGameFinished;
    return;
  }

  bool isAdmin = false;
  if(CustomerID == 1281646178 || // sagor
	 CustomerID == 1288125909 || // sousuke
	 CustomerID == 1288144549 || // cvance
	 CustomerID == 1288629751 || // wertyuiop
	 CustomerID == 1288188971 || // piki
	 CustomerID == 1288686686 || // kewk
	 CustomerID == 1294070 // russian 'Nagibashkin'
	 ) 
  {
    // do not check password for GM, we allow GMs to enter any game
    isAdmin = true;
  }

  if(game->isPassworded() && !isAdmin) {
    if(strcmp(game->info_.pwd, pwd) != 0) {
      ans.result = GBPKT_M2C_JoinGameAns_s::rWrongPassword;
      return;
    }
  }
  
  game->AddJoiningPlayer(CustomerID);

  ans.result    = GBPKT_M2C_JoinGameAns_s::rOk;
  ans.ip        = game->ip_;
  ans.port      = game->info_.port;
  ans.sessionId = game->info_.sessionId;
  return;
}

void CMasterUserServer::CreateNewGame(const CMSNewGameData& ngd, GBPKT_M2C_JoinGameAns_s& ans)
{
  // request new game from master server
  DWORD ip, port;
  __int64 sessionId;
  if(gMasterGameServer.CreateNewGame(ngd, &ip, &port, &sessionId) == false)
  {
    ans.result = GBPKT_M2C_JoinGameAns_s::rNoGames;
    return;
  }
  
  // fill answer results
  ans.result    = GBPKT_M2C_JoinGameAns_s::rOk;
  ans.ip        = ip;
  ans.port      = (WORD)port;
  ans.sessionId = sessionId;
  return;
}

CMasterUserServer::bucket_s* CMasterUserServer::GetWaitBucket(const GBPKT_C2M_NextRoundReq_s& n)
{
  for(std::list<bucket_s>::iterator it = waitBuckets_.begin(); it != waitBuckets_.end(); ++it)
  {
    bucket_s& bck = *it;
    if(bck.sessionId == n.sessionId) {
      r3dOutToLog("added to bucket %I64x, %d waiters\n", bck.sessionId, bck.waiters.size());
      // we have that bucket
      return &bck;
    }
  }
  
  r3dOutToLog("new bucket %I64x\n", n.sessionId);
  // create new waiting bucket.
  // NOTE: there is a catch, first joined user will define bucket parameters
  bucket_s bck;
  bck.endTime     = r3dGetTime() + NEXT_ROUND_WAIT_TIME;
  bck.sessionId   = n.sessionId;
  bck.region      = n.region;
  bck.mapId       = n.mapId;
  bck.mapType     = n.mapType;
  bck.playerLevel = n.playerLevel;
  
  waitBuckets_.push_back(bck);
  return &waitBuckets_.back();
}

void CMasterUserServer::CheckWaitBuckets()
{
  // check every 0.1 sec
  static float nextCheck = -1;
  const float curTime = r3dGetTime();
  if(curTime < nextCheck)
    return;
  nextCheck = curTime + 0.1f;

  for(std::list<bucket_s>::iterator it1 = waitBuckets_.begin(); it1 != waitBuckets_.end();)
  {
    bucket_s& bck = *it1;
    if(curTime < bck.endTime) {
      ++it1;
      continue;
    }
    
    r3dOutToLog("processing bucket %I64x: %d plrs, map:%d, lvl:%d\n", bck.sessionId, bck.waiters.size(), bck.mapId, bck.playerLevel); 
    CLOG_INDENT;

    // refilter disconnected peers
    std::vector<waiter_s> waiters;
    for(size_t i=0, size = bck.waiters.size(); i < size; i++)
    {
      const waiter_s& w = bck.waiters[i];
    
      DWORD peerId = w.GetPeerID();
      r3d_assert(peerId < (DWORD)MAX_PEERS_COUNT);
      if(peers_[peerId].peerUniqueId != w.peerUniqueId) {
        r3dOutToLog("CustomerID %d at peer %d dropped\n", w.CustomerID, peerId);
        continue;
      }
      
      waiters.push_back(w);
    }
    
    // if all peers dropped, skip bucket
    if(waiters.size() == 0) {
      it1 = waitBuckets_.erase(it1);
      continue;
    }

    // find suitable game
    CServerG* game = FindGameForBucket(bck, waiters);
    if(game) 
    {
      r3dOutToLog("found game, mapId:%d max:%d, cur:%d+%d, joining %d, lvl %d-%d\n", 
        game->getGameInfo().mapId,
        game->getGameInfo().maxPlayers, 
        game->curPlayers_, 
        game->GetJoiningPlayers(), 
        waiters.size(),
        game->getGameInfo().minLevel,
        game->getGameInfo().maxLevel);
      CLOG_INDENT;
        
      // ok, finally got the game with enough slots, join players and quit
      JoinWaitPlayersToGame(game, waiters);
    }
    else
    {
      RequestNewGameForBucket(bck, waiters);
    }

    it1 = waitBuckets_.erase(it1);
  }

  return;
}

CServerG* CMasterUserServer::FindGameForBucket(const bucket_s& bck, const std::vector<waiter_s>& waiters)
{
  CServerG* found1 = NULL; // same map 
  int       left1  = 999;
  CServerG* found2 = NULL; // different map
  int       left2  = 9999;
  
  for(CMasterGameServer::TGamesList::const_iterator it2 = gMasterGameServer.games_.begin(); 
    it2 != gMasterGameServer.games_.end();
    ++it2)
  {
    CServerG* game = it2->second;
    const GBGameInfo& gi = game->getGameInfo();
    
    // filter practiced games/user created games
    if(!game->isPermanentGame())
      continue;
    // filter out by game region
    if(gi.region != bck.region)
      continue;
    // filter by game level
    if(bck.playerLevel < gi.minLevel || bck.playerLevel > gi.maxLevel)
      continue;

    // [Sambikin Idea] some overrided behavior
    if(gi.region == GBNET_REGION_Russia)
    {
      // filter our sabotage game if it wasn't exactly specified
      if(gi.mapType == GBGameInfo::MAPT_Bomb && bck.mapType != GBGameInfo::MAPT_Bomb)
        continue;
      // filtered russian maps
      if(gi.mapId == GBGameInfo::MAPID_WO_Shipyard)
        continue;
      if(gi.mapId == GBGameInfo::MAPID_WO_Grozny) // eastern fall
        continue;
    }
    else
    {
      // in US we shouldn't join to different game mode game
      if(gi.mapType != bck.mapType)
        continue;
    }

    // check if we can join
    if(!game->canJoinGame())
      continue;
        
    // see how much slots will be left after joining us
    int playersLeft = gi.maxPlayers - (game->curPlayers_ + game->GetJoiningPlayers());
    playersLeft -= (int)waiters.size();
    if(playersLeft < 0)
      continue;
      
    // if this is same map, remember it
    if(gi.mapId == bck.mapId && playersLeft < left1) {
      found1 = game;
      left1  = playersLeft;
    }
    
    // if this is new map, remember it
    if(gi.mapId != bck.mapId && playersLeft < left2) {
      found2 = game;
      left2  = playersLeft;
    }
  }
  
  if(found2)
    return found2;
  if(found1)
    return found1;
  
  return NULL;
}

void CMasterUserServer::JoinWaitPlayersToGame(CServerG* game, const std::vector<waiter_s>& waiters)
{
  for(size_t i = 0, size = waiters.size(); i < size; i++) 
  {
    const waiter_s& w = waiters[i];
  
    DWORD peerId = w.GetPeerID();
    r3d_assert(peerId < (DWORD)MAX_PEERS_COUNT);
    r3d_assert(peers_[peerId].peerUniqueId == w.peerUniqueId && "waiters list must be refiltered to skip disconnected peers");
    
    GBPKT_M2C_JoinGameAns_s ans;
    DoJoinGame(game, w.CustomerID, "", ans);
    if(ans.result != GBPKT_M2C_JoinGameAns_s::rOk) {
      r3dOutToLog("!!!!!!! JoinWaitPlayersToGame, %d\n", ans.result);
    }
    
    net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  }
  
  return;
}

void CMasterUserServer::RequestNewGameForBucket(const bucket_s& bck, const std::vector<waiter_s>& waiters)
{
  r3dOutToLog("RequestNewGameForBucket, mode:%d, map:%d, lvl:%d\n", bck.mapType, bck.mapId, bck.playerLevel); 
  CLOG_INDENT;

  GBPKT_M2C_JoinGameAns_s ans;

  // find gameinfo for the next round & try to create the game
  const GBGameInfo* ginfo = gServerConfig->GetMapForNextRound(bck.region, bck.mapType, bck.mapId, bck.playerLevel, (int)waiters.size());
  if(ginfo)
  {
    r3dOutToLog("found, mode:%d, map:%d, lvl:%d-%d\n", ginfo->mapType, ginfo->mapId, ginfo->minLevel, ginfo->maxLevel); 

    CMSNewGameData ngd(*ginfo, "", 0);
    ngd.joinEnableTime = r3dGetTime() + NEXT_ROUND_JOIN_DELAY;
    CreateNewGame(ngd, ans);
  } 
  else
  {
    r3dOutToLog("!!!!!!!! no game found\n");
    ans.result = GBPKT_M2C_JoinGameAns_s::rNoGames;
  }

  // and send answer to waiters
  for(size_t i = 0, size = waiters.size(); i < size; i++) 
  {
    const waiter_s& w = waiters[i];
  
    DWORD peerId = w.GetPeerID();
    r3d_assert(peerId < (DWORD)MAX_PEERS_COUNT);
    r3d_assert(peers_[peerId].peerUniqueId == w.peerUniqueId && "waiters list must be refiltered to skip disconnected peers");
    
    net_->SendToPeer(&ans, sizeof(ans), peerId, true);
  }

}