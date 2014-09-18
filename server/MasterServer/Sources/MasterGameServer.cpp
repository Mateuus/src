#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"
#include <shellapi.h>
#include "process.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"

#pragma warning(disable: 4065)	// switch statement contains 'default' but no 'case' labels

#include "MasterGameServer.h"
#include "MasterServer.h"

#include "../../MasterServer/Sources/NetPacketsServerBrowser.h"
using namespace NetPacketsServerBrowser;

#include "ObjectsCode/weapons/WeaponConfig.h"
#include "ServerWeapons/MasterServerWeaponArmory.h"
#include "../../EclipseStudio/Sources/backend/WOBackendAPI.h"

#include "../EclipseStudio/Sources/GameLevel.h"
#include "../EclipseStudio/Sources/GameLevel.cpp"

	CMasterGameServer gMasterGameServer;
	r3dNetwork	serverNet;

extern int		gDomainPort;
extern bool		gDomainUseSSL;

CMasterGameServer::CMasterGameServer()
{
  curSuperId_ = 1;
  
  itemsDbUpdateThread_ = NULL;
  itemsDbUpdateFlag_   = ITEMSDBUPDATE_Unactive;
  itemsDbLastUpdate_   = 0;
  newWeaponArmory_     = NULL;
  
  shuttingDown_        = false;
  
  return;
}

CMasterGameServer::~CMasterGameServer()
{
}

void CMasterGameServer::Start(int port, int in_serverId)
{
  SetConsoleTitle("WO::Master");
  
  masterServerId_ = in_serverId;
  r3d_assert(masterServerId_ > 0 && masterServerId_ < 255);

#if 1  // PAX_BUILD
  DoFirstItemsDbUpdate();
#endif
  DoFirstUpdateCGK();
  
  serverNet.Initialize(this, "serverNet");
  if(!serverNet.CreateHost(port, MAX_PEERS_COUNT)) {
    r3dError("CreateHost failed\n");
  }

  r3dOutToLog("MasterGameServer started at port %d\n", port);
  nextLogTime_ = r3dGetTime();
  
  #if 0
  // register local temp supervisor
  {
    r3dOutToLog("@@@ registering local temp supervisor\n");
    SBPKT_S2M_RegisterMachine_s n;
    n.maxGames = 32;
    n.maxPlayers = n.maxGames * 32;
    n.serverGroup = 0;
    r3dscpy(n.serverName, "arktos01");
    n.portStart = SBNET_GAME_PORT;

    DWORD id = curSuperId_++;
    CServerS* super = new CServerS(id, 0x1, 0);
    super->Init(n);
    supers_.insert(TSupersList::value_type(0x12345, super));
  }
  #endif

  //@ShellExecute(NULL, "open", "SupervisorServer.exe", "", "", SW_SHOW);
  
  return;
}

void CMasterGameServer::GetPermanentGamesData(int pgslot, int* out_active, int* out_free)
{
  r3d_assert(pgslot < gServerConfig->numPermGames_);

  const float curTime = r3dGetTime();
  const int PLAYERS_TO_CONSIDER_FREE_GAME = 2;
    
  int active = 0;
  int free   = 0;
  
  // scan supervisors for active & free games
  for(TSupersList::iterator it = supers_.begin(); it != supers_.end(); ++it) 
  {
    const CServerS* super = it->second;
    
    // for each game slot with correct permanent game index
    for(int gslot=0; gslot<super->maxGames_; gslot++)
    {
      if(super->games_[gslot].info.ginfo.permGameIdx != (pgslot + 1))
        continue;
        
      // closing games is not active and not free
      if(curTime < super->games_[gslot].closeTime)
        continue;

      // check if slot is used
      if(super->games_[gslot].game == NULL &&
        (curTime > super->games_[gslot].createTime + CServerS::REGISTER_EXPIRE_TIME)) {
        continue;
      }
      
      const CServerG* game = super->games_[gslot].game;
      if(game == NULL) {
        // game not started yet (within registering time) - consider it as free AND active
        active++;
        free++;
        continue;
      }

      if(!game->canJoinGame())
        continue;

      // consider only games you can join as active game.      
      active++;
      //if(game->curPlayers_ + game->GetJoiningPlayers() < PLAYERS_TO_CONSIDER_FREE_GAME)
      if(game->curPlayers_ + game->GetJoiningPlayers() < (game->getGameInfo().maxPlayers))
        free++;
    }
  }
  
  *out_active = active;
  *out_free   = free;
  
  return;
}

void CMasterGameServer::UpdatePermanentGames()
{
  static float nextCheck = -1;
  const float curTime = r3dGetTime();
  if(curTime < nextCheck)
    return;
  nextCheck = curTime + 0.5f;
  
  // no new games if we shutting down
  if(shuttingDown_)
    return;
  
  // there always should be [this] amount of free games
  const int NUM_FREE_GAMES = 1;

  // don't do anything if there is no supervisors registered
  if(supers_.size() == 0)
    return;

  // scan each permanent game slot and spawn new game if there isn't enough of them
  for(int pgslot=0; pgslot<gServerConfig->numPermGames_; pgslot++)
  {
    CMasterServerConfig::permGame_s& pg = gServerConfig->permGames_[pgslot];
    
    // check if we have supervisors with correct region, if not - skip it
    bool haveSupers = false;
    for(TSupersList::iterator it = supers_.begin(); it != supers_.end(); ++it) {
      const CServerS* super = it->second;
      if(super->region_ == pg.ginfo.region) {
        haveSupers = true;
        break;
      }
    }
    // silently continue, no need to spam log about that
    if(!haveSupers) {
      //r3dOutToLog("permgame: %d, no supervisors in region %d\n", pgslot, pg.ginfo.region);
      continue;
    }
    
    int gactive;
    int gfree;
    GetPermanentGamesData(pgslot, &gactive, &gfree);
    //r3dOutToLog("permgame: %d, active:%d, free:%d\n", pgslot, gactive, gfree); CLOG_INDENT;
    
    // see how many games we need to spawn
    int numToSpawn = 0;
    if(gactive < pg.minGames)
      numToSpawn += (pg.minGames - gactive);
    if(gfree < NUM_FREE_GAMES)
      numToSpawn += (NUM_FREE_GAMES - gfree);

    if(numToSpawn == 0)
      continue;
    
    if(gactive + numToSpawn >= pg.maxGames) {
      r3dOutToLog("permgame: %d reached maxGames\n", pgslot);
      continue;
    }

    r3dOutToLog("permgame: %d spawning new, active:%d/%d, free:%d\n", 
      pgslot,
      gactive,
      pg.maxGames,
      gfree);
    CLOG_INDENT;

    // spawn new game
    // note: spawn only one game per tick, otherwise we can spam session servers
    CMSNewGameData ngd(pg.ginfo, "", 0);

    DWORD ip;
    DWORD port;
    __int64 sessionId;
    if(!CreateNewGame(ngd, &ip, &port, &sessionId)) {
      continue;
    }

/*
    r3dOutToLog("permgame: %d(%d out of %d) created at %s:%d\n", 
      pgslot,
      pg.curGames,
      pg.maxGames,
      inet_ntoa(*(in_addr*)&ip), 
      port);
*/      
  }
  
}

void CMasterGameServer::RequestShutdown()
{
  if(shuttingDown_)
    return;

  r3dOutToLog("--------- SHUTDOWN requested\n");

  shuttingDown_ = true;
  shutdownLeft_ = 30.0f;
    
  // notify each game about shutdown
  SBPKT_M2G_ShutdownNote_s n;
  n.reason   = 0;
  n.timeLeft = shutdownLeft_ - 5.0f; // make that game server shutdown 5 sec before us

  for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
  {
    const CServerG* game = it->second;
    net_->SendToPeer(&n, sizeof(n), game->peer_, true);
  }
    
}


void CMasterGameServer::Tick()
{
  const float curTime = r3dGetTime();
  
  net_->Update();
  
  DisconnectIdlePeers();
  
  UpdatePermanentGames();

#if 1  // PAX_BUILD  
  UpdateItemsDb();
#endif
  
  // periodically update key
  if(curTime > nextCGKupdate_)
    StartUpdateCGK();

  if(shuttingDown_)
    shutdownLeft_ -= r3dGetFrameTime();

  return;
}

void CMasterGameServer::Stop()
{
  if(net_)
    net_->Deinitialize();
}

void CMasterGameServer::DisconnectIdlePeers()
{
  const float IDLE_PEERS_CHECK = 0.2f;	// do checks every N seconds
  const float IDLE_PEERS_DELAY = 5.0f;	// peer have this N seconds to validate itself

  static float nextCheck = -1;
  const float curTime = r3dGetTime();
  if(curTime < nextCheck)
    return;
  nextCheck = curTime + IDLE_PEERS_CHECK;
  
  for(int i=0; i<MAX_PEERS_COUNT; i++) 
  {
    if(peers_[i].status != PEER_Connected)
      continue;
  
    if(curTime < peers_[i].connectTime + IDLE_PEERS_DELAY)
      continue;
      
    DisconnectCheatPeer(i, "validation time expired");
  }
  
  return;
}

void CMasterGameServer::DisconnectCheatPeer(DWORD peerId, const char* message)
{
  r3dOutToLog("cheat: master peer%d, reason: %s\n", peerId, message);
  net_->DisconnectPeer(peerId);
  
  // fire up disconnect event manually, enet might skip if if other peer disconnect as well
  OnNetPeerDisconnected(peerId);
}

void CMasterGameServer::OnNetPeerConnected(DWORD peerId)
{
  peer_s& peer = peers_[peerId];
  r3d_assert(peer.status == PEER_Free);
  
  peer.status = PEER_Connected;
  peer.connectTime = r3dGetTime();

  // report our version
  CREATE_PACKET(SBPKT_ValidateConnectingPeer, n);
  n.version = SBNET_VERSION;
  n.key1    = 0;
  net_->SendToPeer(&n, sizeof(n), true);

  return;
}

void CMasterGameServer::OnNetPeerDisconnected(DWORD peerId)
{
  peer_s& peer = peers_[peerId];
  switch(peer.status)
  {
    case PEER_Free:
      break;
    case PEER_Connected:
      break;
    case PEER_Validated:
      break;
    
    case PEER_GameServer:
    {
      TGamesList::iterator it = games_.find(peerId);
      // see if game was already closed successfully
      if(it == games_.end())
        break;

      CServerG* game = it->second;

      r3dOutToLog("game %s closed unexpectedly\n", game->GetName());
      DeleteGame(game);

      games_.erase(it);
      break;
    }
      
    case PEER_SuperServer:
    {
      TSupersList::iterator it = supers_.find(peerId);
      if(it == supers_.end()) {
        break;
      }
      
      CServerS* super = it->second;

      r3dOutToLog("master: super '%s' disconnected\n", super->GetName());
      //TODO? disconnect all games from there.
      delete super;
    
      supers_.erase(it);
      break;
    }
  }
  
  peer.status = PEER_Free;
  return;
}

bool CMasterGameServer::DoValidatePeer(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize)
{
  peer_s& peer = peers_[peerId];
  
  if(peer.status >= PEER_Validated)
    return true;

  // we still can receive packets after peer was force disconnected
  if(peer.status == PEER_Free)
    return false;

  r3d_assert(peer.status == PEER_Connected);
  if(PacketData->EventID != SBPKT_ValidateConnectingPeer) {
    DisconnectCheatPeer(peerId, "wrong validate packet id");
    return false;
  }

  const SBPKT_ValidateConnectingPeer_s& n = *(SBPKT_ValidateConnectingPeer_s*)PacketData;
  if(sizeof(n) != PacketSize) {
    DisconnectCheatPeer(peerId, "wrong validate packet size");
    return false;
  }
  if(n.version != SBNET_VERSION) {
    DisconnectCheatPeer(peerId, "wrong validate version");
    return false;
  }
  if(n.key1 != SBNET_KEY1) {
    DisconnectCheatPeer(peerId, "wrong validate key");
    return false;
  }
  
  // set peer to validated
  peer.status = PEER_Validated;
  return false;
}

void CMasterGameServer::OnNetData(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize)
{
  if(PacketSize < sizeof(r3dNetPacketHeader)) {
    DisconnectCheatPeer(peerId, "too small packet");
    return;
  }

  // wait for validating packet
  if(!DoValidatePeer(peerId, PacketData, PacketSize))
    return;

  peer_s& peer = peers_[peerId];
  r3d_assert(peer.status >= PEER_Validated);
  
  switch(PacketData->EventID) 
  {
    default:
      r3dOutToLog("CMasterGameServer: invalid packetId %d", PacketData->EventID);
      DisconnectCheatPeer(peerId, "wrong packet id");
      return;
      
    case SBPKT_S2M_RegisterMachine:
    {
      const SBPKT_S2M_RegisterMachine_s& n = *(SBPKT_S2M_RegisterMachine_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);

      r3d_assert(supers_.find(peerId) == supers_.end());

      DWORD ip = net_->GetPeerIp(peerId);
      DWORD id = curSuperId_++;

      CServerS* super = new CServerS(id, ip, peerId);
      super->Init(n);
      supers_.insert(TSupersList::value_type(peerId, super));
      
      r3d_assert(peer.status == PEER_Validated);
      peer.status = PEER_SuperServer;
      
      // send registration answer
      {
        SBPKT_M2S_RegisterAns_s n;
        n.id = id;
        net_->SendToPeer(&n, sizeof(n), peerId);
      }
      
      break;
    }
    
    case SBPKT_G2M_RegisterGame:
    {
      const SBPKT_G2M_RegisterGame_s& n = *(SBPKT_G2M_RegisterGame_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);
      r3d_assert(peer.status == PEER_Validated);

      r3dOutToLog("game 0x%x connected\n", n.gameId);

      // register this game in supervisor
      CServerS* super = GetServerByGameId(n.gameId);
      if(super == NULL) {
        // this might happen when supervisor crashed between game start & registration
        r3dOutToLog("game 0x%x without supervisor\n", n.gameId);
        
        SBPKT_M2G_KillGame_s n1;
        net_->SendToPeer(&n1, sizeof(n1), peerId);
        net_->DisconnectPeer(peerId);
        break;
      }
      
      CServerG* game = super->CreateGame(n.gameId, peerId);
      r3d_assert(game);

      games_.insert(TGamesList::value_type(peerId, game));
      
      r3d_assert(peer.status == PEER_Validated);
      peer.status = PEER_GameServer;
      
#if 1  // PAX_BUILD
      SendArmoryInfoToGame(game);
#endif
      break;
    }

    case SBPKT_G2M_UpdateGame:
    {
      const SBPKT_G2M_UpdateGame_s& n = *(SBPKT_G2M_UpdateGame_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);

      TGamesList::iterator it = games_.find(peerId);
      r3d_assert(it != games_.end());
      CServerG* game = it->second;
      game->SetCurrentData(n.timeLeft, n.curPlayers, n.uinfo);
      break;
    }

    case SBPKT_G2M_FinishGame:
    {
      const SBPKT_G2M_FinishGame_s& n = *(SBPKT_G2M_FinishGame_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);

      TGamesList::iterator it = games_.find(peerId);
      r3d_assert(it != games_.end());
      CServerG* game = it->second;

      r3dOutToLog("game %s finished\n", game->GetName());
      game->finished_ = true;

      break;
    }

    case SBPKT_G2M_CloseGame:
    {
      const SBPKT_G2M_CloseGame_s& n = *(SBPKT_G2M_CloseGame_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);

      TGamesList::iterator it = games_.find(peerId);
      r3d_assert(it != games_.end());
      CServerG* game = it->second;

      r3dOutToLog("game %s closed\n", game->GetName());
      DeleteGame(game);

      games_.erase(it);

      break;
    }
    
    case SBPKT_G2M_DataUpdateReq:
    {
      const SBPKT_G2M_DataUpdateReq_s& n = *(SBPKT_G2M_DataUpdateReq_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);

      r3dOutToLog("got SBPKT_G2M_DataUpdateReq\n");
      StartItemsDbUpdate(true);
      break;
    }

  }

  return;
}

void CMasterGameServer::DeleteGame(CServerG* game)
{
  CServerS* super = GetServerByGameId(game->id_);

  if(super) {
    super->DeregisterGame(game);
  } else {
    r3dOutToLog("master: game %s from UNKNOWN supervisor disconnected\n", game->GetName());
  }

  delete game;
}

CServerS* CMasterGameServer::GetLeastUsedServer(EGBGameRegion region)
{
  CServerS* selected = NULL;
  int       maxDiff = -1;
  
  // search for server with maximum available users
  for(TSupersList::iterator it = supers_.begin(); it != supers_.end(); ++it) 
  {
    CServerS* super = it->second;
    
    // filter our supervisors if region is specified
    if(region != GBNET_REGION_Unknown && super->region_ != region)
      continue;
    
    int diff = super->maxPlayers_ - super->GetExpectedPlayers();
    // diff can be negative here, because of unexpected joiners
    if(diff > 0 && diff > maxDiff) {
      maxDiff  = diff;
      selected = super;
    }
  }
  
  return selected;
}


CServerS* CMasterGameServer::GetServerByGameId(DWORD gameId)
{
  DWORD sid = gameId >> 16;
    
  for(TSupersList::iterator it = supers_.begin(); it != supers_.end(); ++it) {
    CServerS* super = it->second;
    if(sid == super->id_)
      return super;
  }
  
  return NULL;
}

CServerG* CMasterGameServer::GetGameByGameId(DWORD gameId)
{
  for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
  {
    CServerG* game = it->second;
    if(gameId == game->id_)
      return game;
  }
  
  return NULL;
}

CServerG* CMasterGameServer::GetGameBySessionId(__int64 sessionId)
{
  for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
  {
    CServerG* game = it->second;
    if(sessionId == game->info_.sessionId)
      return game;
  }
  
  return NULL;
}

CServerG* CMasterGameServer::GetQuickJoinGame(int gameMap, int gameMode, EGBGameRegion region, int playerLevel)
{
  r3dOutToLog("GetQuickJoinGame: %d %d region:%d, level:%d\n", gameMap, gameMode, region, playerLevel); CLOG_INDENT;
  
  // find most populated game
  int       foundMax  = -1;
  CServerG* foundGame = NULL;
  
  for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
  {
    CServerG* game = it->second;
    if(!game->canJoinGame())
      continue;
    if(game->isPassworded())
      continue;
      
    const GBGameInfo& gi = game->getGameInfo();

    // filter not-server created games
    if(!game->isPermanentGame())
      continue;

    // filter out region
    if(region != GBNET_REGION_Unknown && gi.region != region) {
      continue;
    }
      
    // [Sambikin Idea] some overrided behavior
    if(gi.region == GBNET_REGION_Russia)
    {
      // filter our sabotage game if it wasn't exactly specified
      if(gi.mapType == GBGameInfo::MAPT_Bomb && gameMode != GBGameInfo::MAPT_Bomb)
        continue;
      // filtered maps
      if(gi.mapId == GBGameInfo::MAPID_WO_Shipyard)
        continue;
      if(gi.mapId == GBGameInfo::MAPID_WO_Grozny) // eastern fall
        continue;
    }

    // filter out specified map/mode (0xFF mean any game/mode)
    if(gameMap < 0xFF && gameMap != gi.mapId)
      continue;
    if(gameMode < 0xFF && gameMode != gi.mapType)
      continue;
      
    // filter out games by level (bound inclusive)
    if(playerLevel < gi.minLevel || playerLevel > gi.maxLevel)
      continue;
      
    int numPlayers = game->curPlayers_ + game->GetJoiningPlayers();
    if(numPlayers >= gi.maxPlayers)
      continue;
      
    if(numPlayers > foundMax) {
      foundMax  = numPlayers;
      foundGame = game;
    }
  }
  
  CServerG* game = foundGame;
  if(game == NULL) {
    r3dOutToLog("no free games\n");
    return NULL;
  }
  
  r3dOutToLog("%s, %d(+%d) of %d players, lvl %d-%d\n", 
    game->GetName(), 
    game->curPlayers_, 
    game->GetJoiningPlayers(), 
    game->getGameInfo().maxPlayers,
    game->getGameInfo().minLevel,
    game->getGameInfo().maxLevel);
    
  return game;
}

bool CMasterGameServer::HaveGameByCustomerId(DWORD customerId)
{
  for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
  {
    const CServerG* game = it->second;
    if(game->info_.CustomerID == customerId)
      return true;
  }
  
  return false;
}

bool CMasterGameServer::CreateNewGame(const CMSNewGameData& ngd, DWORD* out_ip, DWORD* out_port, __int64* out_sessionId)
{
  CServerS* super = GetLeastUsedServer((EGBGameRegion)ngd.ginfo.region);
  if(super == NULL)
  {
    r3dOutToLog("there is no free game servers at region:%d\n", ngd.ginfo.region);
    return false;
  }
  
  CREATE_PACKET(SBPKT_M2S_StartGameReq, n);
  if(super->RegisterNewGameSlot(ngd, n) == false)
  {
    r3dOutToLog("request for new game failed at %s\n", super->GetName());
    return false;
  }

#if 1
  r3dOutToLog("request for new game send to %s, creator:%d, players:%d, id:%x, port:%d\n", super->GetName(), ngd.CustomerID, ngd.ginfo.maxPlayers, n.gameId, n.port);
  net_->SendToPeer(&n, sizeof(n), super->peer_, true);
#else
  char strginfo[256];
  ginfo.ToString(strginfo);

  char cmd[512];
  sprintf(cmd, "\"%u %u %u\" \"%s\"", n.gameId, n.port, ngd.CustomerID, strginfo);
  const char* exe = "WO_GameServer.exe";
  int err;
  if(err = (int)ShellExecute(NULL, "open", exe, cmd, "", SW_SHOW) < 32) {
    r3dOutToLog("!!! unable to run %s: %d\n", exe, err);
  }
#endif
  
  *out_ip        = super->ip_;
  *out_port      = n.port;
  *out_sessionId = n.sessionId;

  return true;
}

DWORD __stdcall CMasterGameServer::ItemsDbUpdateThread(LPVOID in)
{
  CMasterGameServer* This = (CMasterGameServer*)in;

  MSWeaponArmory* wa   = NULL;
  CkHttpResponse* resp = NULL;

  try
  {
    r3dOutToLog("ItemsDBUpdateThread: started\n");

    CkHttp http;
    int success = http.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
    if(success != 1) 
      r3dError("Internal error");

    CkHttpRequest req;
    req.UsePost();
    req.put_Path("/php/api_getItemsDB.php");
    req.AddParam("serverkey", "CfFkqQWjfgksYG56893GDhjfjZ20");

    resp = http.SynchronousRequest(g_api_ip->GetString(), gDomainPort, gDomainUseSSL, req);
    if(!resp)
      throw "no response";
      
    wa = new MSWeaponArmory();
    char* data = (char*)resp->bodyStr();
    if(!wa->loadItemsDB(data, strlen(data))) 
      throw "failed to load itemsdb";
    
    r3dOutToLog("ItemsDBUpdateThread: updated, %d weapons\n", wa->m_NumWeaponsLoaded);
    
    delete resp;

    This->newWeaponArmory_   = wa;
    This->itemsDbUpdateFlag_ = ITEMSDBUPDATE_Ok;
    return This->itemsDbUpdateFlag_;
  }
  catch(const char* msg)
  {
    r3dOutToLog("!!!! ItemsDBUpdateThread failed: %s\n", msg);
  }
  
  SAFE_DELETE(wa);
  SAFE_DELETE(resp);

  This->itemsDbUpdateFlag_ = ITEMSDBUPDATE_Error;
  return This->itemsDbUpdateFlag_;
}

void CMasterGameServer::StartItemsDbUpdate(bool forced)
{
  if(itemsDbUpdateFlag_ != ITEMSDBUPDATE_Unactive) 
  {
    r3dOutToLog("items db update already in process\n");
    return;
  }
  
  itemsDbUpdateFlag_   = ITEMSDBUPDATE_Processing;
  itemsDbUpdateForced_ = forced;

  // create items update thread thread
  itemsDbUpdateThread_ = CreateThread(NULL, 0, &ItemsDbUpdateThread, this, 0, NULL);
  itemsDbLastUpdate_   = r3dGetTime();
  
  return;
}

void CMasterGameServer::DoFirstItemsDbUpdate()
{
  // minor hack: if we're running in local test mode, skip items updating
  extern int gDomainPort;
  if(gDomainPort == 55016 || stricmp(g_api_ip->GetString(), "localhost") == 0)
  {
    itemsDbUpdateFlag_ = ITEMSDBUPDATE_Processing; // put it in permanent wait state
    return;
  }

  r3dOutToLog("reading new items db\n");
  StartItemsDbUpdate(false);
  DWORD rr = ::WaitForSingleObject(itemsDbUpdateThread_, 30000);
  if(rr != WAIT_OBJECT_0) {
    r3dError("failed to download items db - timeout\n");
    return;
  }
  if(itemsDbUpdateFlag_ != ITEMSDBUPDATE_Ok) {
    r3dError("failed to download items db - error\n");
    return;
  }

  // swap current weapon armory with new one
  r3d_assert(gMSWeaponArmory == NULL);
  gMSWeaponArmory    = newWeaponArmory_;
  newWeaponArmory_   = NULL;
  itemsDbUpdateFlag_ = ITEMSDBUPDATE_Unactive;
  return;
}

void CMasterGameServer::UpdateItemsDb()
{
  if(itemsDbUpdateFlag_ == ITEMSDBUPDATE_Processing)
    return;
    
  if(itemsDbUpdateFlag_ == ITEMSDBUPDATE_Unactive)
  {
    // do update every 10 min
    if(r3dGetTime() > itemsDbLastUpdate_ + 600.0f)
    {
      r3dOutToLog("Starting periodic itemsdb update\n");
      StartItemsDbUpdate(false);
    }
    return;
  }
    
  if(itemsDbUpdateFlag_ == ITEMSDBUPDATE_Error)
  {
    r3dOutToLog("failed to get items db update\n");
    itemsDbUpdateFlag_ = ITEMSDBUPDATE_Unactive;
    return;
  }
  
  r3d_assert(itemsDbUpdateFlag_ == ITEMSDBUPDATE_Ok);
  r3dOutToLog("got new weapon info, sending to %d games\n", games_.size());

  // swap current weapon armory with new one
  SAFE_DELETE(gMSWeaponArmory);
  r3d_assert(gMSWeaponArmory == NULL);
  gMSWeaponArmory    = newWeaponArmory_;
  newWeaponArmory_   = NULL;
  itemsDbUpdateFlag_ = ITEMSDBUPDATE_Unactive;
  
  // send new data to games
  if(itemsDbUpdateForced_)
  {
    for(TGamesList::iterator it = games_.begin(); it != games_.end(); ++it) 
    {
      const CServerG* game = it->second;
#if 1  // PAX_BUILD
      SendArmoryInfoToGame(game);
#endif
    }
  }
  
  return;
}

void CMasterGameServer::SendArmoryInfoToGame(const CServerG* game)
{
  if(gMSWeaponArmory == NULL) {
    r3dOutToLog("gMSWeaponArmory isn't loaded\n");
    return;
  }

  r3d_assert(gMSWeaponArmory);
  
  // send all weapons to game server
  for(uint32_t i=0; i<gMSWeaponArmory->m_NumWeaponsLoaded; i++)
  {
    const WeaponConfig& wc = *gMSWeaponArmory->m_WeaponArray[i];
    
    SBPKT_M2G_UpdateWeaponData_s n;
    n.itemId = wc.m_itemID;
    wc.copyParametersTo(n.wi);
    
    net_->SendToPeer(&n, sizeof(n), game->peer_, true);
  }

  // send all gears to game server
  for(uint32_t i=0; i<gMSWeaponArmory->m_NumGearLoaded; i++)
  {
    const GearConfig& gc = *gMSWeaponArmory->m_GearArray[i];
    
    SBPKT_M2G_UpdateGearData_s n;
    n.itemId = gc.m_itemID;
    gc.copyParametersTo(n.gi);
    
    net_->SendToPeer(&n, sizeof(n), game->peer_, true);
  }
  
  // send lootboxes to game server
  for(uint32_t i=0; i<gMSWeaponArmory->m_NumItemLoaded; i++)
  {
    const ItemConfig& ic = *gMSWeaponArmory->m_ItemArray[i];
    if(ic.category != storecat_LootBox)
	continue;
    
    SBPKT_M2G_UpdateItemData_s n;
    n.itemId = ic.m_itemID;
    n.LevelRequired = ic.m_LevelRequired;
    
    net_->SendToPeer(&n, sizeof(n), game->peer_, true);
  }

  // send end update event
  SBPKT_M2G_UpdateDataEnd_s n;
  net_->SendToPeer(&n, sizeof(n), game->peer_, true);
  
  return;
}

//
//
// updating "Create Game Key"
//
//

DWORD __stdcall CMasterGameServer::UpdateCGKThread(LPVOID in)
{
  r3dRandInitInTread rand_in_thread;

  CMasterGameServer* This = (CMasterGameServer*)in;
  
  try
  {
    // make new key (it can not be zero)
    DWORD createGameKey = u_random(0xFFFFFFFF);
    if(createGameKey == 0) createGameKey = 1;
    
    // set new key and store previous
    This->createGameKey2_ = This->createGameKey1_;
    This->createGameKey1_ = createGameKey;
    r3dOutToLog("Create Game Key changed to %d\n", createGameKey);
    
    CWOBackendReq req("api_SrvSetCreateGameKey2.aspx");
    req.AddParam("skey1", "CfFkqQWjfgksYG56893GDhjfjZ20");
    req.AddParam("CreateGameKey", createGameKey);
    req.AddParam("ServerID", gMasterGameServer.masterServerId_);
    if(!req.Issue())
    {
	r3dOutToLog("!!!! api_SrvSetCreateGameKey failed: %d\n", req.resultCode_);
	return 0;
    }
  }
  catch(const char* msg)
  {
    r3dOutToLog("!!!! CMasterGameServer_UpdateCGKThread failed: %s\n", msg);
  }
  
  return 0;
}

void CMasterGameServer::StartUpdateCGK()
{
  if(updateCGKThread_ != NULL) 
  {
    // wait until therad finish
    if(::WaitForSingleObject(updateCGKThread_, 0) != WAIT_OBJECT_0)
      return;

    updateCGKThread_ = NULL;
  }
    
  updateCGKThread_ = CreateThread(NULL, 0, &UpdateCGKThread, this, 0, NULL);

  // next update every 5 min
  nextCGKupdate_ = r3dGetTime() + 300.0f;
  
  return;
}

void CMasterGameServer::DoFirstUpdateCGK()
{
  // init vars here, because CMasterGameServer is static and we can't call rand() there
  updateCGKThread_ = NULL;
  createGameKey1_  = u_random(0xFFFFFFFF);
  createGameKey2_  = u_random(0xFFFFFFFF);
  if(createGameKey1_ == 0) createGameKey1_++;
  if(createGameKey2_ == 0) createGameKey2_++;
 
  StartUpdateCGK();
  DWORD rr = ::WaitForSingleObject(updateCGKThread_, 30000);
  if(rr != WAIT_OBJECT_0) {
    r3dError("failed to updateCGK - timeout\n");
    return;
  }
  
  return;
}
