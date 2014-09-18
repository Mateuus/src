#pragma once

#include "MasterServer.h"
#include "MasterGameServer.h"

#include "../../ServerNetPackets/NetPacketsGameBrowser.h"
using namespace NetPacketsGameBrowser;

class CMasterUserServer : public r3dNetCallback
{
  public:
	// peer-status array for each peer
	enum EPeerStatus
	{
	  PEER_Free,
	  PEER_Connected,
	  PEER_Validated,
	};
	struct peer_s 
	{
	  EPeerStatus	status;
	  DWORD		peerUniqueId;	// index that is unique per each peer, 16bits: peerId, 16bits: curPeerUniqueId_

	  float		connectTime;
	  float		lastReqTime;

	  peer_s() 
	  {
	    status     = PEER_Free;
	  }
	};
	peer_s*		peers_;
	int		MAX_PEERS_COUNT;
	int		numConnectedPeers_;
	int		maxConnectedPeers_;
	DWORD		curPeerUniqueId_;	// counter for unique peer checking

	void		DisconnectIdlePeers();
	void		DisconnectCheatPeer(DWORD peerId, const char* message, ...);
	bool		DisconnectIfShutdown(DWORD peerId);
	bool		DoValidatePeer(DWORD peerId, const r3dNetPacketHeader* PacketData, int PacketSize);
	
	// callbacks from r3dNetwork
	void		OnNetPeerConnected(DWORD peerId);
	void		OnNetPeerDisconnected(DWORD peerId);
	void		OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);

	bool		Validate(const GBPKT_C2M_RefreshList_s& n) { return true; }
	bool		Validate(const GBPKT_C2M_CreateGame_s& n);
	bool		Validate(const GBPKT_C2M_JoinGameReq_s& n);
	bool		Validate(const GBPKT_C2M_JoinFriendGameReq_s& n);
	bool		Validate(const GBPKT_C2M_QuickGameReq_s& n) { return true; }
	bool		Validate(const GBPKT_C2M_NextRoundReq_s& n) { return true; }
	
	void		OnGBPKT_C2M_RefreshList(DWORD peerId, const GBPKT_C2M_RefreshList_s& n);
	void		OnGBPKT_C2M_CreateGame(DWORD peerId, const GBPKT_C2M_CreateGame_s& n);
	void		OnGBPKT_C2M_JoinGameReq(DWORD peerId, const GBPKT_C2M_JoinGameReq_s& n);
	void		OnGBPKT_C2M_JoinFriendGameReq(DWORD peerId, const GBPKT_C2M_JoinFriendGameReq_s& n);
	void		OnGBPKT_C2M_QuickGameReq(DWORD peerId, const GBPKT_C2M_QuickGameReq_s& n);
	void		OnGBPKT_C2M_NextRoundReq(DWORD peerId, const GBPKT_C2M_NextRoundReq_s& n);

	void		DoJoinGame(CServerG* game, DWORD CustomerID, const char* pwd, GBPKT_M2C_JoinGameAns_s& ans);
	void		CreateNewGame(const CMSNewGameData& ngd, GBPKT_M2C_JoinGameAns_s& ans);

	void		PrintStats();
	
	// delayed joins
	const static int NEXT_ROUND_WAIT_TIME = 10;	// delay for all players to register in new waiting list
	const static int NEXT_ROUND_JOIN_DELAY = 15;	// time for newly created games to be unavailable for quick join
	
	struct waiter_s
	{
	  DWORD		peerUniqueId;
	  DWORD		CustomerID;
	  
	  DWORD		GetPeerID() const {
	    return peerUniqueId >> 16;
	  }
	};
	
	struct bucket_s
	{
	  float		endTime;
	  
	  // info about the game
	  __int64	sessionId;
	  int		region;
	  int		mapId;
	  int		mapType;
	  int		playerLevel;
	  
	  // waiters by peer.peerUniqueId
	  std::vector<waiter_s> waiters;
	};
	
	std::list<bucket_s> waitBuckets_;
	bucket_s*	GetWaitBucket(const GBPKT_C2M_NextRoundReq_s& n);
	void		CheckWaitBuckets();
	CServerG*	 FindGameForBucket(const bucket_s& bck, const std::vector<waiter_s>& waiters);
	void		 RequestNewGameForBucket(const bucket_s& bck, const std::vector<waiter_s>& waiters);
	void		 JoinWaitPlayersToGame(CServerG* game, const std::vector<waiter_s>& waiters);
	

  public:
	CMasterUserServer();
	~CMasterUserServer();

	void		Start(int port, int in_maxPeerCount);
	void		Tick();
	void		Stop();
};

extern	CMasterUserServer gMasterUserServer;
