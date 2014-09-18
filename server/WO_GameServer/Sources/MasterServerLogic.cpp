#include "r3dPCH.h"
#include "r3d.h"

#include "MasterServerLogic.h"

	MasterServerLogic gMasterServerLogic;

#include "../../MasterServer/Sources/NetPacketsServerBrowser.h"
using namespace NetPacketsServerBrowser;

#include "ObjectsCode/weapons/WeaponArmory.h"

MasterServerLogic::MasterServerLogic()
{
  disconnected_    = false;
  gotWeaponUpdate_ = false;
  shuttingDown_    = false;
}

MasterServerLogic::~MasterServerLogic()
{
}

void MasterServerLogic::OnNetPeerConnected(DWORD peerId)
{
  return;
}

void MasterServerLogic::OnNetPeerDisconnected(DWORD peerId)
{
  r3dOutToLog("!!! master server disconnected\n");
  disconnected_ = true;
  return;
}

void MasterServerLogic::OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize)
{
  switch(packetData->EventID) 
  {
    default:
      r3dError("MasterServerLogic: unknown packetId %d", packetData->EventID);
      return;

    case SBPKT_ValidateConnectingPeer:
    {
      const SBPKT_ValidateConnectingPeer_s& n = *(SBPKT_ValidateConnectingPeer_s*)packetData;
      if(n.version != SBNET_VERSION) {
        r3dError("master server version is different (%d vs %d)", n.version, SBNET_VERSION);
        break;
      }
      break;
    }
    
    case SBPKT_M2G_KillGame:
    {
      r3dOutToLog("Master server requested game kill\n");
      net_->DisconnectPeer(peerId);
      break;
    }
    
    case SBPKT_M2G_ShutdownNote:
    {
      const SBPKT_M2G_ShutdownNote_s& n = *(SBPKT_M2G_ShutdownNote_s*)packetData;
      r3d_assert(sizeof(n) == packetSize);
      
      r3dOutToLog("---- Master server is shutting down now\n");
      shuttingDown_ = true;
      shutdownLeft_ = n.timeLeft;
      break;
    }
    
    case SBPKT_M2G_UpdateWeaponData:
    {
      const SBPKT_M2G_UpdateWeaponData_s& n = *(SBPKT_M2G_UpdateWeaponData_s*)packetData;
      r3d_assert(sizeof(n) == packetSize);

      WeaponConfig* wc = const_cast<WeaponConfig*>(gWeaponArmory.getWeaponConfig(n.itemId));
      if(wc == NULL) {
        r3dOutToLog("!!! got update for not existing weapon %d\n", n.itemId);
        return;
      }

      wc->copyParametersFrom(n.wi);
      //r3dOutToLog("got update for weapon %s\n", wc->m_StoreName);
      break;
    }

    case SBPKT_M2G_UpdateGearData:
    {
      const SBPKT_M2G_UpdateGearData_s& n = *(SBPKT_M2G_UpdateGearData_s*)packetData;
      r3d_assert(sizeof(n) == packetSize);

      GearConfig* gc = const_cast<GearConfig*>(gWeaponArmory.getGearConfig(n.itemId));
      if(gc == NULL) {
        r3dOutToLog("!!! got update for not existing gear %d\n", n.itemId);
        return;
      }

      gc->copyParametersFrom(n.gi);
      //r3dOutToLog("got update for gear %s\n", gc->m_StoreName);
      break;
    }

    case SBPKT_M2G_UpdateItemData:
    {
      const SBPKT_M2G_UpdateItemData_s& n = *(SBPKT_M2G_UpdateItemData_s*)packetData;
      r3d_assert(sizeof(n) == packetSize);

      ItemConfig* ic = const_cast<ItemConfig*>(gWeaponArmory.getItemConfig(n.itemId));
      if(ic == NULL) {
        r3dOutToLog("!!! got update for not existing item %d\n", n.itemId);
        return;
      }

      ic->m_LevelRequired = n.LevelRequired;
      r3dOutToLog("got update for item %s\n", ic->m_StoreName);
      break;
    }

    case SBPKT_M2G_UpdateDataEnd:
    {
      const SBPKT_M2G_UpdateDataEnd_s& n = *(SBPKT_M2G_UpdateDataEnd_s*)packetData;
      r3d_assert(sizeof(n) == packetSize);

      r3dOutToLog("got SBPKT_M2G_UpdateDataEnd\n");
      gotWeaponUpdate_ = true;
      
      break;
    }
  }

  return;
}

int MasterServerLogic::WaitFunc(fn_wait fn, float timeout, const char* msg)
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

    if(r3dGetTime() > endWait) {
      return 0;
    }
  }
  
  return 1;
}

void MasterServerLogic::Init(DWORD gameId)
{
  gameId_ = gameId;
}

int MasterServerLogic::Connect(const char* host, int port)
{
  r3dOutToLog("Connecting to master server at %s:%d\n", host, port); CLOG_INDENT;
  
  g_net.Initialize(this, "master server");
  g_net.CreateClient();
  g_net.Connect(host, port);

  if(!WaitFunc(&MasterServerLogic::wait_IsConnected, 10, "connecting"))
    throw "can't connect to master server";

  // send validation packet immediately
  CREATE_PACKET(SBPKT_ValidateConnectingPeer, n);
  n.version = SBNET_VERSION;
  n.key1    = SBNET_KEY1;
  net_->SendToHost(&n, sizeof(n), true);
    
  return 1;
}

void MasterServerLogic::Disconnect()
{
  g_net.Deinitialize();
}

void MasterServerLogic::Tick()
{
  if(net_ == NULL)
    return;

  net_->Update();
  
  if(shuttingDown_)
    shutdownLeft_ -= r3dGetFrameTime();
}

void MasterServerLogic::RegisterGame()
{
  if(net_ == NULL)
    return;
    
  CREATE_PACKET(SBPKT_G2M_RegisterGame, n);
  n.gameId = gameId_;
  net_->SendToHost(&n, sizeof(n), true);

  return;
}

void MasterServerLogic::FinishGame()
{
  if(net_ == NULL)
    return;

  CREATE_PACKET(SBPKT_G2M_FinishGame, n);
  net_->SendToHost(&n, sizeof(n), true);
  
  return;
}

void MasterServerLogic::CloseGame()
{
  if(net_ == NULL || IsMasterDisconnected())
    return;

  CREATE_PACKET(SBPKT_G2M_CloseGame, n);
  net_->SendToHost(&n, sizeof(n), true);
  
  return;
}

void MasterServerLogic::SendGameUpdate(float timeLeft, int num_users, const GBUserInfo* users)
{
  if(net_ == NULL)
    return;

  r3d_assert(num_users < MAX_POSSIBLE_PLAYERS);

  CREATE_PACKET(SBPKT_G2M_UpdateGame, n);
  n.timeLeft   = timeLeft;
  n.curPlayers = num_users;
  for(int i=0; i<num_users; i++) {
    n.uinfo[i] = users[i];
  }
  net_->SendToHost(&n, sizeof(n), true);

  return;  
}

void MasterServerLogic::RequestDataUpdate()
{
  SBPKT_G2M_DataUpdateReq_s n;
  net_->SendToHost(&n, sizeof(n), true);
}