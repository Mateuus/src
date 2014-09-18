#pragma once
#include "r3dNetwork.h"

class CGameServer : public r3dNetCallback
{
  public:
	bool		started_;
	bool		connected_;

	struct config_s
	{
	  std::string	masterIp_;
	  int		masterPort_;
	};
	config_s	cfg_;
	void		LoadConfig();

	int		maxPlayers_;
	DWORD		gameId_;
	int		port_;
	
	int		ConnectToMasterServer();
	void		RegisterInMasterServer();
	
	// callbacks from r3dNetwork
	void		OnNetPeerConnected(DWORD peerId);
	void		OnNetPeerDisconnected(DWORD peerId);
	void		OnNetData(DWORD peerId, const void* packetData, int packetSize);

  public:
	CGameServer();
	~CGameServer();
	
	void		InitFromArgs(int argc, char* argv[]);

	void		Start();
	void		Tick();
	void		Stop();
};

extern	CGameServer	gGameServer;
