#pragma once

#include "r3dNetwork.h"
#include <list>
#include <hash_map>

#include "../../ServerNetPackets/NetPacketsGameInfo.h"
#include "GameSlotInfo.h"
#include "MasterServerConfig.h"

class MSWeaponArmory;

class CServerS;
class CServerG;

class CMasterGameServer : public r3dNetCallback
{
  public:
	int		masterServerId_;
	float		nextLogTime_;
	
	bool		shuttingDown_;
	float		shutdownLeft_;

	typedef stdext::hash_map<DWORD, CServerS*> TSupersList;
	typedef stdext::hash_map<DWORD, CServerG*> TGamesList;
	TSupersList	supers_;
	TGamesList	games_;
	DWORD		curSuperId_;
	
	CServerS*	GetLeastUsedServer(EGBGameRegion region);
	CServerS*	GetServerByGameId(DWORD gameId);
	CServerG*	GetGameByGameId(DWORD gameId);
	CServerG*	GetGameBySessionId(__int64 sessionId);
	CServerG*	GetQuickJoinGame(int gameMap, int gameMode, EGBGameRegion region, int playerLevel);
	bool		HaveGameByCustomerId(DWORD customerId);
	
	void		DeleteGame(CServerG* game);	// deregister and delete game
	
	bool		CreateNewGame(const CMSNewGameData& ngd, DWORD* out_ip, DWORD* out_port, __int64* out_sessionId);
	
	enum { MAX_PEERS_COUNT = 1024, };
	enum EPeerStatus
	{
	  PEER_Free,
	  PEER_Connected,
	  PEER_Validated,
	  PEER_GameServer,
	  PEER_SuperServer,
	};
	struct peer_s 
	{
	  int		status;
	  float		connectTime;
	  
	  peer_s() 
	  {
	    status = PEER_Free;
	  }
	};
	peer_s		peers_[MAX_PEERS_COUNT];

	void		DisconnectIdlePeers();
	void		DisconnectCheatPeer(DWORD peerId, const char* message);
	bool		DoValidatePeer(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize);
	
	void		OnNetPeerConnected(DWORD peerId);
	void		OnNetPeerDisconnected(DWORD peerId);
	void		OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);
	
	void		UpdatePermanentGames();
	void		 GetPermanentGamesData(int pgslot, int* out_active, int* out_free);
	
	//
	// weapons update
	//
	static DWORD __stdcall ItemsDbUpdateThread(LPVOID in);
	enum EItemsDbUpdate {
	  ITEMSDBUPDATE_Unactive = 0,
	  ITEMSDBUPDATE_Processing,
	  ITEMSDBUPDATE_Ok,
	  ITEMSDBUPDATE_Error,
	};
	DWORD		itemsDbUpdateFlag_;
	HANDLE		itemsDbUpdateThread_;
	bool		itemsDbUpdateForced_;
	float		itemsDbLastUpdate_;
	MSWeaponArmory*	newWeaponArmory_;
	void		StartItemsDbUpdate(bool forced);
	void		DoFirstItemsDbUpdate();
	void		UpdateItemsDb();
	
	void		SendArmoryInfoToGame(const CServerG* game);
	
	// create game key logic
	DWORD		createGameKey1_;
	DWORD		createGameKey2_;

	static DWORD __stdcall UpdateCGKThread(LPVOID in);
	float		nextCGKupdate_;
	HANDLE		updateCGKThread_;
	void		StartUpdateCGK();
	void		DoFirstUpdateCGK();

  public:
	CMasterGameServer();
	~CMasterGameServer();

	void		Start(int port, int in_serverId);
	void		Tick();
	void		Stop();
	
	void		RequestShutdown();
};

extern	CMasterGameServer gMasterGameServer;
