#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"

#include "MasterServer.h"
using namespace NetPacketsServerBrowser;

// Eternity compile fix
DWORD DriverUpdater(HWND hParentWnd, DWORD VendorId, DWORD v1, DWORD v2, DWORD v3, DWORD v4, DWORD hash) { return 0; }
void r3dScaleformBeginFrame() {}
void r3dScaleformEndFrame() {}
void SetNewSimpleFogParams() {}
void SetAdvancedFogParams() {}
void SetVolumeFogParams() {}
r3dCamera gCam ;
class r3dSun * Sun;
r3dScreenBuffer * DepthBuffer;
r3dLightSystem WorldLightSystem;

CServerG::CServerG(DWORD id, DWORD ip, DWORD peer) : CServerBase(tGame, id, ip, peer) 
{
  curPlayers_ = 0;
  finished_   = false;
  
  for(int i=0; i<MAX_POSSIBLE_PLAYERS; i++) {
    joiners_[i].timeExpected = -1;
  }
  maxJoiners_ = 0;
}

void CServerG::Init(const CMSGameData& in_info)
{
  info_       = in_info;

  curPlayers_ = 0;
  startTime_  = r3dGetTime();

  // we should disable connects at the last 20% of game time
  lateTime_   = startTime_ + ((float)getGameInfo().timeLimit * 60) * 0.8f;
}

const char* CServerG::GetName() const
{
  static char name[64];
  sprintf(name, "0x%x", id_);
  return name;
}

void CServerG::SetCurrentData(float timeLeft, int curPlayers, const GBUserInfo* users)
{
  const float curTime = r3dGetTime();
  // update lateTime
  lateTime_	  = curTime + timeLeft - (((float)getGameInfo().timeLimit * 60) * 0.2f); // -20% of time limit
  
  curPlayers_ = curPlayers;
  for(int i=0; i<curPlayers_; i++) 
  {
    users_[i] = users[i];
    
    // filter our joiners.
    for(int j=0; j<maxJoiners_; j++) {
      if(joiners_[j].CustomerID == users[i].CustomerID) {
        joiners_[j].CustomerID   = 0;
        joiners_[j].timeExpected = -1;
      }
    }
  }

  
  return;
}

void CServerG::AddJoiningPlayer(DWORD CustomerID)
{
  // give some delay for player to actually join
  static const float JOIN_DELAY = 10;
  const float curTime = r3dGetTime();

  for(int i=0; i<MAX_POSSIBLE_PLAYERS; i++) 
  {
    if(curTime < joiners_[i].timeExpected)
      continue;

    joiners_[i].timeExpected = curTime + JOIN_DELAY;
    joiners_[i].CustomerID   = CustomerID;

    maxJoiners_ = R3D_MAX(maxJoiners_, i + 1);
    return;
  }
  
  return;
}

int CServerG::GetJoiningPlayers() const
{
  const float curTime = r3dGetTime();
  
  int users = 0;
  for(int i=0; i<MAX_POSSIBLE_PLAYERS; i++) {
    if(curTime < joiners_[i].timeExpected) {
      users++;
    }
  }

  return users;
}

void CServerS::Init(const SBPKT_S2M_RegisterMachine_s& n)
{
  r3d_assert(games_ == NULL);
  
  // if server specified external ip - replace it
  if(n.externalIpAddr) 
    ip_ = n.externalIpAddr;

  region_      = n.region;
  maxPlayers_  = n.maxPlayers;
  maxGames_    = n.maxGames;
  portStart_   = n.portStart;
  serverName_  = n.serverName;
  games_       = new games_s[maxGames_];
  for(int i=0; i<maxGames_; i++) {
    games_[i].game       = NULL;
    games_[i].createTime = -99;
    games_[i].closeTime  = -99;
  }
      
  r3dOutToLog("master: super '%s' (ip:%s, id:%d) registered. max %d players, %d sessions\n", 
    GetName(),
    inet_ntoa(*(in_addr*)&ip_),
    id_, 
    maxPlayers_, 
    maxGames_);
    
  return;
}

bool CServerS::RegisterNewGameSlot(const CMSNewGameData& ngd, SBPKT_M2S_StartGameReq_s& out_n)
{
  r3d_assert(games_ != NULL);

  const float curTime = r3dGetTime();
  int gameSlot = -1;
  
  for(int i=0; i<maxGames_; i++) 
  {
    if(games_[i].game != NULL)
      continue;
      
    // filter out pending games
    if(curTime < games_[i].createTime + REGISTER_EXPIRE_TIME) 
      continue;
    
    // filter out closing games
    if(curTime < games_[i].closeTime)
      continue;
      
    gameSlot = i;
    break;
  }
  
  if(gameSlot == -1)
  {
    r3dOutToLog("CServerS::RegisterNewGameSlot: no free slots\n");
    return false;
  }
  
  // set game params
  DWORD gameId      = CreateGameId(gameSlot);
  __int64 sessionId = CreateSessionId();
  WORD gamePort     = portStart_ + gameSlot;

  // create answering packet
  out_n.gameId    = gameId;
  out_n.sessionId = sessionId;
  out_n.ginfo     = ngd.ginfo;
  out_n.port      = gamePort;
  out_n.creatorID = ngd.CustomerID;
  
  // reserved this gameslot
  games_s& slot = games_[gameSlot];
  r3d_assert(slot.game == NULL);
  slot.createTime     = curTime;
  slot.info           = ngd;
  slot.info.gameId    = gameId;
  slot.info.port      = gamePort;
  slot.info.sessionId = sessionId;

  r3dOutToLog("NewGame: registered new game in slot %d, name '%s', pwd '%s', port %d\n", 
	gameSlot, 
	slot.info.ginfo.name, 
	slot.info.pwd, 
	out_n.port);
  
  return true;
}

__int64 CServerS::CreateSessionId()
{
  static FILETIME ft2;

  // GetSystemTimeAsFileTime resolution is not enough if two games spawned at once
  // so wait for next system time update
  FILETIME ft;
  do 
  {
    GetSystemTimeAsFileTime(&ft);
    if(ft.dwLowDateTime != ft2.dwLowDateTime) 
      break;
    
    Sleep(0);
  } while(1);
  ft2 = ft;

  // Do not cast a pointer to a FILETIME structure to either a LARGE_INTEGER* 
  // or __int64* value because it can cause alignment faults on 64-bit Windows.
  LARGE_INTEGER li;
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  
  // strip milliseconds part as it's always same
  li.QuadPart = li.QuadPart - (li.QuadPart % 10000);

  // make unique log id in milliseconds part. 
  // including random value and supervisor id (0-10000) - format will be "0RvId"

  // random value in 0XX00
  int randVal = (u_random(99) * 100);
  // supervisor id in 000XX (NOTE: it will overflow after 100 supervisors :)
  randVal += (id_ % 100);

  r3d_assert(id_ < 100);
  r3d_assert(randVal < 10000);
  li.QuadPart += randVal;
  
  return li.QuadPart;
}

int CServerS::GetExpectedGames() const
{
  r3d_assert(games_ != NULL);

  const float curTime = r3dGetTime();
  int num = 0;

  for(int i=0; i<maxGames_; i++) 
  {
    if(games_[i].game || (curTime < games_[i].createTime + REGISTER_EXPIRE_TIME)) {
      num++;
    }
  }
  
  return num;
}

int CServerS::GetExpectedPlayers() const
{
  r3d_assert(games_ != NULL);

  int users = 0;
  for(int i=0; i<maxGames_; i++) 
  {
    const CServerG* game = games_[i].game;
    if(!game) 
      continue;

    users += game->curPlayers_ + game->GetJoiningPlayers();
  }
  
  return users;
}

CServerG* CServerS::CreateGame(DWORD gameId, DWORD peerId)
{
  int gameSlot;
  DWORD serverId;
  ParseGameId(gameId, &serverId, &gameSlot);
  r3d_assert(serverId == id_);

  r3d_assert(gameSlot < maxGames_);
  games_s& slot = games_[gameSlot];

  r3d_assert(slot.info.gameId == gameId);
  r3d_assert(slot.game == NULL);

  CServerG* game = new CServerG(gameId, this->ip_, peerId);
  game->Init(slot.info);

  slot.game = game;

  //r3dOutToLog("game %x registered in slot %d in %s\n", gameId, gameSlot, GetName());
  return game;
}

void CServerS::DeregisterGame(const CServerG* game)
{
  r3d_assert(games_ != NULL);

  int gameSlot;
  DWORD serverId;
  ParseGameId(game->id_, &serverId, &gameSlot);
  r3d_assert(serverId == id_);

  r3d_assert(gameSlot < maxGames_);
  games_s& slot = games_[gameSlot];
  
  r3d_assert(slot.game == game);
  slot.game = NULL;
  slot.createTime = -99;
  slot.closeTime  = r3dGetTime() + 2.0f; // give few sec to allow game server executable to finish

  r3dOutToLog("game stopped at slot %d in %s\n", gameSlot, GetName());

  return;
}
