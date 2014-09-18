#include "r3dPCH.h"
#include "r3d.h"

#include "..\\ServerNetPackets\\NetPacketsServerBrowser.h"
using namespace NetPacketsServerBrowser;

#include "TestGameServer.h"

	CGameServer	gGameServer;
	r3dNetwork	serverNet;

void CGameServer::OnNetPeerConnected(DWORD peerId)
{
  r3dOutToLog("Game: Connected peer %d\n", peerId);
  connected_ = true;
  return;
}

void CGameServer::OnNetPeerDisconnected(DWORD peerId)
{
  connected_ = false;
  r3dError("MasterServer disconnected");
  return;
}

void CGameServer::OnNetData(DWORD peerId, const void* PacketData, int PacketSize)
{
  WORD pktId = *(WORD*)PacketData;
      
  switch(pktId) 
  {
    default:
      r3dError("sessionServer_OnReceive: invalid packetId %d", pktId);
      break;

    case SBPKT_ReportVersion:
      break;
  }

  return;
}



CGameServer::CGameServer()
{
  started_   = false;
  connected_ = false;
  
  port_      = 0;
}

CGameServer::~CGameServer()
{
}

void CGameServer::InitFromArgs(int argc, char* argv[])
{
  if(argc != 4) r3dError("invalid number of parameters (in:%d)", argc);
  
  port_       = atoi(argv[1]);
  maxPlayers_ = atoi(argv[2]);
  gameId_     = atoi(argv[3]);
  
  if(port_ == 0)        r3dError("invalid port");
  if(maxPlayers_ == 0)  r3dError("invalid maxPlayers");
  if(gameId_ == 0)      r3dError("invalid gameId");

  return;  
}

void CGameServer::LoadConfig()
{
  const char* configFile = "SupervisorServer.cfg";
  const char* group      = "SupervisorServer";

  if(_access(configFile, 0) != 0) {
    r3dError("can't open config file %s", configFile);
  }

  cfg_.masterPort_  = r3dReadCFG_I(configFile, group, "masterPort", 0);
  cfg_.masterIp_    = r3dReadCFG_S(configFile, group, "masterIp", "");
  
  #define CHECK_I(xx) if(xx == 0)  r3dError("missing %s value", #xx);
  #define CHECK_S(xx) if(xx == "") r3dError("missing %s value", #xx);
  CHECK_I(cfg_.masterPort_);
  CHECK_S(cfg_.masterIp_);
  #undef CHECK_I
  #undef CHECK_S
  
  return;
}

int CGameServer::ConnectToMasterServer()
{
  serverNet.Initialize(this, "serverNet");
  serverNet.CreateClient();
  
  const char* hostaddr = cfg_.masterIp_.c_str();
  int         hostport = cfg_.masterPort_;

  serverNet.Connect(hostaddr, hostport);
  
  r3dOutToLog("connecting to master server at %s:%d\n", hostaddr, hostport);

  float endWait = r3dGetTime() + 10.0f;
  while(1) 
  {
    serverNet.Update();
  
    if(serverNet.IsConnected()) {
      break;
    }
      
    if(r3dGetTime() > endWait) {
      r3dError("can't connect to %s:%d", hostaddr, hostport);
      return 0;
    }
  }
  
  r3dOutToLog("Connect to master server Ok\n");

  started_ = true;
  return 1;
}

void CGameServer::RegisterInMasterServer()
{
  // register in master server
  {
    CREATE_PACKET(SBPKT_G2M_RegisterGame, n);
    n.gameId      = gameId_;
    n.maxPlayers  = maxPlayers_;
    serverNet.SendToHost(&n, sizeof(n), true);
  }
  
  return;
}

void CGameServer::Start()
{
  //DWORD dwErrorMode = SetErrorMode(SetErrorMode(0));
  //SetErrorMode(dwErrorMode | SEM_NOGPFAULTERRORBOX);

  LoadConfig();

  SetConsoleTitle("WO::Game Server");
  char fname[MAX_PATH];
  sprintf(fname, "r3dlog_Game_%d.txt", port_);
  extern void r3dChangeLogFile(const char* fname);
  r3dChangeLogFile(fname);

  ConnectToMasterServer();
  RegisterInMasterServer();
  
  return;
}

void CGameServer::Tick()
{
  serverNet.Update();
  
  return;
}

void CGameServer::Stop()
{
  serverNet.Deinitialize();
}
