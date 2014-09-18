#include "r3dPCH.h"
#include "r3d.h"

#include "MasterUdpMonitor.h"

CMasterUdpMonitor	gUdpMonitor;

CMasterUdpMonitor::CMasterUdpMonitor()
{
  s_ = INVALID_SOCKET ;
}


CMasterUdpMonitor::~CMasterUdpMonitor()
{
  Stop();
}

void CMasterUdpMonitor::Start(int port)
{
  r3d_assert(s_ == INVALID_SOCKET );

  s_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(s_ == INVALID_SOCKET)
    r3dError("can't create socket: %d\n", WSAGetLastError());

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int res = bind(s_, (SOCKADDR *)&addr, sizeof(addr));
  if(res == SOCKET_ERROR)
    r3dError("can't bind socket to port %d: %d\n", port, WSAGetLastError());
  
  // put to non blocking mode
  DWORD nonBlocking = 1;
  res = ioctlsocket(s_, FIONBIO, &nonBlocking);
  
  return;
}

void CMasterUdpMonitor::Stop()
{
  closesocket(s_);
  s_ = INVALID_SOCKET;
}

void CMasterUdpMonitor::Tick()
{
  if(s_ == INVALID_SOCKET)
    return;

  char data[1024];
  sockaddr_in addr;
  int alen = sizeof(addr);

  int res = recvfrom(s_, data, 1024, 0, (sockaddr*)&addr, &alen);
  if(res == SOCKET_ERROR || res == 0)
    return;

  if(memcmp(data, "WPING", 5) == 0) 
  {
    //r3dOutToLog("got WPING req from %s:%d\n", inet_ntoa(addr.sin_addr), addr.sin_port);
    sendto(s_, "WPONG", 6, 0, (sockaddr*)&addr, alen);
    return;
  }
  
  return;
}