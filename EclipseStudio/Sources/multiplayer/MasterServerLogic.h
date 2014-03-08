#pragma once

#include "r3dNetwork.h"
#include "../../ServerNetPackets/NetPacketsGameBrowser.h"
using namespace NetPacketsGameBrowser;

class MasterServerLogic : public r3dNetCallback
{
  protected:
	CRITICAL_SECTION csNetwork;
	r3dNetwork	g_net;
	bool		isConnected_;

	// r3dNetCallback virtuals
virtual	void		OnNetPeerConnected(DWORD peerId);
virtual	void		OnNetPeerDisconnected(DWORD peerId);
virtual	void		OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);

  public:
	volatile int	masterServerId_;
  
	volatile bool	gameListReceived_;
	std::vector<GBPKT_M2C_SupervisorData_s> supers_;
	std::vector<GBPKT_M2C_GameData_s> games_;
	
	volatile bool	gameJoinAnswered_;
	GBPKT_M2C_JoinGameAns_s gameJoinAnswer_;
	
	volatile bool	versionChecked_;
	volatile bool	shuttingDown_;
	// this should not be reinited on connect()
	// bad version should be fatal error
	bool		badClientVersion_;

  protected:
	typedef bool (MasterServerLogic::*fn_wait)();
	int		WaitFunc(fn_wait fn, float timeout, const char* msg);
	
	// wait functions
	bool		wait_GameListReceived() {
	  return gameListReceived_;
	}
	bool		wait_GameJoinAsnwer() {
	  return gameJoinAnswered_;
	}
	
  public:
	MasterServerLogic();
	virtual ~MasterServerLogic();

	int		StartConnect(const char* host, int port);
	void		Disconnect();

	bool		IsConnected() { return isConnected_; }
	
	void		RequestGameList();
	int		WaitForGameList();

	void		SendJoinGame(int gameNum, const char* pwd = "");
	void		SendJoinQuickGame(const NetPacketsGameBrowser::GBPKT_C2M_QuickGameReq_s& n);
	void		SendFriendJoinGame(const NetPacketsGameBrowser::GBPKT_C2M_JoinFriendGameReq_s& n);
	void		SendNextRoundReq(const NetPacketsGameBrowser::GBPKT_C2M_NextRoundReq_s& n);
	void		SendCreateGame(const NetPacketsGameBrowser::GBPKT_C2M_CreateGame_s& n);
	int		WaitForGameJoin();
	void		GetJoinedGameServer(char* out_ip, int* out_port, __int64* out_sessionId);
	
	void		Tick();
};

extern	MasterServerLogic gMasterServerLogic;
