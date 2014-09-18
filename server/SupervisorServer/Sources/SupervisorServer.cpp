#include "r3dPCH.h"
#include "r3d.h"
#include "r3dNetwork.h"
#include <ShellAPI.h>

//#pragma warning(disable: 4065)	// switch statement contains 'default' but no 'case' labels

#include "SupervisorServer.h"
#include "../../MasterServer/Sources/SrvCrashHandler.h"

using namespace NetPacketsServerBrowser;

static	r3dNetwork	serverNet;
	CSupervisorServer gSupervisorServer;


CSupervisorServer::CSupervisorServer()
{
  disconnected_ = false;
  superId_      = 0;
  games_        = NULL;
  InitializeCriticalSection(&csGames_);

  nextMonitorUpdate_ = 0;
}

CSupervisorServer::~CSupervisorServer()
{
  if(games_) 
    delete[] games_;
    
  DeleteCriticalSection(&csGames_);
}

void CSupervisorServer::StartGame(const SBPKT_M2S_StartGameReq_s& n)
{
  r3dCSHolder cs1(csGames_);

  const int slot = n.port - gSupervisorConfig->portStart_;
  if(slot < 0 || slot >= gSupervisorConfig->maxGames_) {
    r3dOutToLog("!!!warning!!! invalid StartGame request port %d", n.port);
    return;
  }
  
  if(games_[slot].hProcess != 0) {
    r3dOutToLog("!!!warning!!! invalid StartGame request, slot %d already used\n", slot);

    // this may be because of previous instance crash, so terminate it
    TerminateProcess(games_[slot].hProcess, 0);
    games_[slot].Reset();
  }
  
  r3dOutToLog("StartGame %I64x slot:%d, users:%d, id:%08x, creatorID:%u\n", n.sessionId, slot, n.ginfo.maxPlayers, n.gameId, n.creatorID); CLOG_INDENT;

  games_[slot].Init(n.gameId, n.sessionId);
  
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  char arg1[128];
  char arg2[128];
  char arg3[128];
  sprintf(arg1, "%u %u %u", n.gameId, n.port, n.creatorID);
  n.ginfo.ToString(arg2);
  r3dscpy(arg3, gSupervisorConfig->masterIp_.c_str());
  
  char params[512];
  sprintf(params, "\"%s\" \"%s\" \"%s\" \"%s\" \"%d\"", 
    gSupervisorConfig->gameServerExe_.c_str(), 
    arg1, 
    arg2,
    arg3,
    gSupervisorConfig->uploadLogs_
    );
  
  r3dOutToLog("CreateProcess: %s\n", params);
  BOOL res = CreateProcess(
    NULL,
    params,
    NULL,
    NULL,
    FALSE,
    DETACHED_PROCESS, // no console by default, if needed game server will alloc it
    NULL,
    NULL,
    &si,
    &pi);
    
  if(res == 0) {
    r3dOutToLog("!!!warning!!! unable to spawn game servers, hr:%d\n", GetLastError());
    games_[slot].Reset();
    return;
  }
  
  // set process handle
  games_[slot].hProcess = pi.hProcess;
  
  return;
}

void CSupervisorServer::OnNetPeerConnected(DWORD peerId)
{
  r3dOutToLog("Supervisor: Connected to master as peer %d\n", peerId);
  return;
}

void CSupervisorServer::OnNetPeerDisconnected(DWORD peerId)
{
  r3dOutToLog("!!!!!!!! Supervisor: MasterServer disconnected\n");
  disconnected_ = true;
  return;
}

void CSupervisorServer::OnNetData(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize)
{
  switch(PacketData->EventID) 
  {
    default:
      r3dError("CSupervisorServer: invalid packetId %d", PacketData->EventID);
      break;

    case SBPKT_ValidateConnectingPeer:
    {
      const SBPKT_ValidateConnectingPeer_s& n = *(SBPKT_ValidateConnectingPeer_s*)PacketData;
      if(n.version != SBNET_VERSION) {
        r3dError("master server version is different (%d vs %d)", n.version, SBNET_VERSION);
        break;
      }
      break;
    }
    
    case SBPKT_M2S_RegisterAns:
    {
      const SBPKT_M2S_RegisterAns_s& n = *(SBPKT_M2S_RegisterAns_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);
      
      superId_ = n.id;
      break;
    }
    
    case SBPKT_M2S_StartGameReq:
    {
      const SBPKT_M2S_StartGameReq_s& n = *(SBPKT_M2S_StartGameReq_s*)PacketData;
      r3d_assert(sizeof(n) == PacketSize);
      
      StartGame(n);
      break;
    }
  }

  return;
}

int CSupervisorServer::WaitFunc(fn_wait fn, float timeout, const char* msg)
{
  const float endWait = r3dGetTime() + timeout;

  r3dOutToLog("waiting: %s, %.1f sec left\n", msg, endWait - r3dGetTime());
  
  while(1) 
  {
    r3dEndFrame();
    r3dStartFrame();
    
    net_->Update();
    
    if((this->*fn)())
      break;
      
    if(disconnected_)
      return 0;

    if(r3dGetTime() > endWait) {
      return 0;
    }
  }
  
  return 1;
}

int CSupervisorServer::ConnectToMasterServer()
{
  const char* hostaddr = gSupervisorConfig->masterIp_.c_str();
  int         hostport = gSupervisorConfig->masterPort_;
  r3dOutToLog("Connecting to master server at %s:%d\n", hostaddr, hostport); CLOG_INDENT;
  
  disconnected_ = false;
  superId_      = 0;
  net_->Connect(hostaddr, hostport);
  if(!WaitFunc(&CSupervisorServer::wait_IsConnected, 20.0f, "connecting"))
    return 2;

  // send validation packet immediately
  CREATE_PACKET(SBPKT_ValidateConnectingPeer, n);
  n.version = SBNET_VERSION;
  n.key1    = SBNET_KEY1;
  net_->SendToHost(&n, sizeof(n), true);
  
  // report caps
  {
    CREATE_PACKET(SBPKT_S2M_RegisterMachine, n);
    n.region         = (BYTE)gSupervisorConfig->serverGroup_;
    r3dscpy(n.serverName, gSupervisorConfig->serverName_.c_str());
    n.maxGames       = gSupervisorConfig->maxGames_;
    n.maxPlayers     = gSupervisorConfig->maxPlayers_;
    n.portStart      = gSupervisorConfig->portStart_;
    n.externalIpAddr = gSupervisorConfig->externalIpAddr_;
    net_->SendToHost(&n, sizeof(n), true);
  }

  if(!WaitFunc(&CSupervisorServer::wait_IsRegistered, 10.0f, "wait for register")) 
  {
    if(disconnected_)
      r3dOutToLog("Supervisor was disconnected from master server\n");
    else
      r3dOutToLog("Supervisor was unable to register at master server\n");
    return 3;
  }
  
  r3dOutToLog("registered, id: %d\n", superId_);
  return 1;
}

bool CSupervisorServer::Start()
{
  SetConsoleTitle("WO::Supervisor");

  _mkdir("logsv");
  time_t t1;
  time(&t1);
  char fname[MAX_PATH];
  sprintf(fname, "logsv\\SV_%x.txt", (DWORD)t1);
  extern void r3dChangeLogFile(const char* fname);
  r3dChangeLogFile(fname);

  sprintf(fname, "logsv\\SV_%x.dmp", (DWORD)t1);
  SrvSetCrashHandler(fname);

  games_ = new CGameWatcher[gSupervisorConfig->maxGames_];

  serverNet.Initialize(this, "serverNet");
  serverNet.CreateClient();

  // try to connect for max 60sec, then exit.
  const float maxConnectTime = r3dGetTime() + 60.0f;
  while(1)
  {
    r3dStartFrame();
    r3dEndFrame();

    switch(ConnectToMasterServer())
    {
      default: 
        r3d_assert(0);
        break;
        
      case 1: // ok
        return true;
        
      case 2: // timeout
        r3dOutToLog("retrying in 2 sec\n");
        ::Sleep(2000);
        break;
      
      case 3: // disconnect
        r3dOutToLog("failed to connect to master server\n");
        return false;
    }
       
    if(r3dGetTime() > maxConnectTime) {
      r3dOutToLog("Timeout while connecting to master server\n");
      return false;
    }
  }
  
  r3d_assert(0);
}

void CSupervisorServer::MonitorProcesses()
{
  if(r3dGetTime() < nextMonitorUpdate_)
    return;
  nextMonitorUpdate_ = r3dGetTime() + 0.1f;

  r3dCSHolder cs1(csGames_);
  for(int slot=0; slot<gSupervisorConfig->maxGames_; slot++) {
    games_[slot].CheckProcess();
  }

  return;
}

void CSupervisorServer::TerminateAllGames()
{
  r3dCSHolder cs1(csGames_);

  if(games_ == NULL)
    return;
    
  for(int slot=0; slot<gSupervisorConfig->maxGames_; slot++) {
    games_[slot].Terminate();
  }
}

bool CSupervisorServer::IsActiveSession(__int64 gameSessionId)
{
  r3dCSHolder cs1(csGames_);

  if(games_ == NULL)
    return false;

  for(int slot=0; slot<gSupervisorConfig->maxGames_; slot++) {
    if(games_[slot].info && games_[slot].info->sessionId == gameSessionId)
      return true;
  }
  
  return false;
}

void CSupervisorServer::Tick()
{
  net_->Update();
  
  MonitorProcesses();
  
  // output some info
  static float nextLog = 0;
  if(r3dGetTime() > nextLog) 
  {
    nextLog = r3dGetTime() + 2.0f;
    
    int numGames = 0;
    for(int slot=0; slot<gSupervisorConfig->maxGames_; slot++) {
      if(games_[slot].hProcess != NULL) 
        numGames++;
    }

    char buf[1024];
    sprintf(buf, "WO::Supervisor, %d games", numGames);
    SetConsoleTitle(buf);
  }
  
  return;
}

void CSupervisorServer::Stop()
{
  if(net_)
    net_->Deinitialize();
}
