#pragma once

#include "r3dNetwork.h"
#include "multiplayer/P2PMessages.h"

#include "../../ServerNetPackets/NetPacketsGameInfo.h"
#include "Backend/ServerUserProfile.h"

class GameObject;
class obj_ServerPlayer;

#define FULL_AREA_EXPLOSION  360.0f

__forceinline obj_ServerPlayer* IsServerPlayer(const GameObject* obj)
{
	if(obj == NULL) return NULL;
	if(obj->Class->Name == "obj_ServerPlayer") return (obj_ServerPlayer*)obj;
	else return NULL;
}

class ServerGameLogic : public r3dNetCallback
{
  protected:
	r3dNetwork	g_net;
	// r3dNetCallback virtuals
virtual	void		OnNetPeerConnected(DWORD peerId);
virtual	void		OnNetPeerDisconnected(DWORD peerId);
virtual	void		OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);

  public:
	enum { MAX_NUM_PLAYERS = 128 };

	GBGameInfo	ginfo_;	// game info
	uint32_t	creatorID_; // customerID of player who created game, 0 - if permanent game
	
	enum { MAX_PEERS_COUNT = 256 }; 
	// peer-to-player array
	enum peerStatus_e 
	{
	  PEER_FREE,
	  PEER_CONNECTED,
	  PEER_VALIDATED1,	// version validated
	  PEER_LOADING,
	  PEER_PLAYING,		// 
	};

	struct peerInfo_s 
	{
	  peerStatus_e	status_;
	  float		startTime;	// status start time
	  void		SetStatus(peerStatus_e status) {
	    status_   = status;
	    startTime = r3dGetTime();
	  }
	  
	  int		playerIdx;
	  int		desiredTeam;
	  int		spawnNode;

	  // bomb mode
	  bool		isPlayerReady;
	  bool		isPlayerSpectator;
	  BYTE		selectedLoadoutSlot;
	  
	  // user id and it profile
	  DWORD		CustomerID;
	  DWORD		SessionID;
	  volatile DWORD haveProfile;
	  CServerUserProfile temp_profile;
	  HANDLE	getProfileH;

	  obj_ServerPlayer* player;
	  
	  // security report stuff (active when player is created)
	  float		secRepRecvTime;	// last time security report was received
	  float		secRepGameTime;	// last value of reported game time
	  float		secRepRecvAccum;
	  
	  float		lastPacketTime;
	  int		lastPacketId;
	  
	  void Clear()
	  {
	    status_     = PEER_FREE;
	    startTime   = r3dGetTime();
	    
	    desiredTeam = -1;
	    playerIdx   = -1;
	    spawnNode   = -1;
	    player      = NULL;
		isPlayerReady = false;
		isPlayerSpectator = false;
		selectedLoadoutSlot = 0;

	    haveProfile = 0;
	    getProfileH = NULL;
	    CustomerID  = 0;
	  }
	};

	peerInfo_s	peers_[MAX_PEERS_COUNT];	// peer to player table (double size to include temporary connects)
	peerInfo_s&	GetPeer(DWORD peerId)
	{
          r3d_assert(peerId < MAX_PEERS_COUNT);
          return peers_[peerId];
	}

	void		DisconnectPeer(DWORD peerId, bool cheat, const char* message, ...);

	DWORD		net_lastFreeId;
	DWORD		net_mapLoaded_LastNetID; // to make sure that client has identit map as server

	// actual player data - based on it ID
	peerInfo_s*	plrToPeer_[MAX_NUM_PLAYERS];	// index to players table
	int		curPlayers_;
	int		maxPlayers_;
	int		curPeersConnected;
	obj_ServerPlayer* CreateNewPlayer(DWORD peerId, int teamId, const r3dPoint3D& spawnPos, float dir,  int loadoutSlot, float spawnProtection);
	void		DeletePlayer(int playerIdx, obj_ServerPlayer* plr);
	void		FillNetCreatePlayer(const obj_ServerPlayer* plr, PKT_S2C_CreatePlayer_s& n);
	obj_ServerPlayer* GetPlayer(int playerIdx)
	{
	  r3d_assert(playerIdx < MAX_NUM_PLAYERS);
	  if(plrToPeer_[playerIdx] == NULL)
	    return NULL;

	  return plrToPeer_[playerIdx]->player;
	}
	
	int		BuildUserList(int* num_users, GBUserInfo* users) const;
	float		GetTimeToEnd() const { return gameEndTime_ - r3dGetTime(); }
	
	#define NETID_DAMAGEDUMMY 90 // must be > NETID_PLAYERS_START && less that next id
	obj_ServerPlayer*	testDamageDummy_[4];

	#define DEFINE_PACKET_FUNC(XX) \
	  void On##XX(const XX##_s& n, GameObject* fromObj, DWORD peerId, bool& needPassThru);
	#define IMPL_PACKET_FUNC(CLASS, XX) \
	  void CLASS::On##XX(const XX##_s& n, GameObject* fromObj, DWORD peerId, bool& needPassThru)

	int		ProcessWorldEvent(GameObject* fromObj, DWORD eventId, DWORD peerId, const void* packetData, int packetSize);
	 DEFINE_PACKET_FUNC(PKT_C2S_ValidateConnectingPeer);
	 DEFINE_PACKET_FUNC(PKT_C2S_JoinGameReq);
	 DEFINE_PACKET_FUNC(PKT_C2S_StartGameReq);
	 DEFINE_PACKET_FUNC(PKT_C2S_SetRespawnData);
	 DEFINE_PACKET_FUNC(PKT_C2S_Temp_Damage);
	 DEFINE_PACKET_FUNC(PKT_C2S_FallingDamage);
	 DEFINE_PACKET_FUNC(PKT_C2C_ChatMessage);
	 DEFINE_PACKET_FUNC(PKT_C2C_VoiceCommand);
	 DEFINE_PACKET_FUNC(PKT_C2C_CommRoseCommand);
	 DEFINE_PACKET_FUNC(PKT_C2S_DataUpdateReq);
	 DEFINE_PACKET_FUNC(PKT_C2S_RequestAirstrike);
	 DEFINE_PACKET_FUNC(PKT_C2S_ResupplyRequest);
	 DEFINE_PACKET_FUNC(PKT_C2S_Siege_Activate);
	 DEFINE_PACKET_FUNC(PKT_C2S_SecurityRep);
	 DEFINE_PACKET_FUNC(PKT_C2S_TEST_SpawnDummyReq);
	 DEFINE_PACKET_FUNC(PKT_C2S_RequestWeaponPickup);
	 DEFINE_PACKET_FUNC(PKT_C2C_ConfirmWeaponPickup);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_PlayerReady);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_RequestBombPlacement);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_RequestBombPickup);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_RequestDrop);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_RequestTeamChange);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_RequestPlayerKick);
	 DEFINE_PACKET_FUNC(PKT_C2S_Bomb_ChatMsg);
	 DEFINE_PACKET_FUNC(PKT_C2S_Admin_PlayerKick);
	 DEFINE_PACKET_FUNC(PKT_C2S_MarkTarget);
	 DEFINE_PACKET_FUNC(PKT_C2S_CreateExplosion);
	 DEFINE_PACKET_FUNC(PKT_C2S_DBG_LogMessage);
	 void		OnPKT_C2S_ScreenshotData(DWORD peerId, const int size, const char* data);
	 
	void		GetNumPlayersInTeam(int tp[2]);
	void		HandleTeamSwitch(obj_ServerPlayer* plr, int teamId);
	int		AutobalanceTeamId(int teamId);
	int		AdjustStartSpawnId(int spawnId, int teamId);
	
	void		ValidateMove(GameObject* fromObj, const void* packetData, int packetSize);

	void		RelayPacket(DWORD peerId, const DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered);
	void		p2pBroadcastToActive(const GameObject* from, DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered = true);
	void		p2pBroadcastToAll(DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered = true);
	void		p2pSendToPeer(DWORD peerId, const GameObject* from, DefaultPacket* packetData, int packetSize, bool guaranteedAndOrdered = true);

	void		SendCurrentGameState();
	
	bool		CanDamageThisObject(const GameObject* targetObj);
	void		ApplyDamage(GameObject* sourceObj, GameObject* targetObj, const r3dPoint3D& dmgPos, float damage, bool force_damage, STORE_CATEGORIES damageSource);
	bool		ApplyDamageToPlayer(GameObject* sourceObj, obj_ServerPlayer* targetPlr, const r3dPoint3D& dmgPos, float damage, int bodyBone, int bodyPart, bool force_damage, STORE_CATEGORIES damageSource, const int airState = 0 );  
	void		DoKillPlayer(GameObject* sourceObj, obj_ServerPlayer* targetPlr, STORE_CATEGORIES weaponCat, bool forced_by_server=false, bool fromPlayerInAir = false, bool targetPlayerInAir = false );
	void		 RewardKillPlayer(obj_ServerPlayer* sourcePlr, obj_ServerPlayer* targetPlr, STORE_CATEGORIES dmgCat);

	void		CheckAchievementsOnKill( obj_ServerPlayer* fromPlr,  obj_ServerPlayer* targetPlr, const STORE_CATEGORIES dmgCat );

	bool		RewardAssistedKills(obj_ServerPlayer* killedPlr);
	
	int		GetRandomControlPoint(int teamId);
	int		GetClosestControlPoint(int teamId, const r3dPoint3D& pos);
	void		GetSpawnPosition(int cpIdx, r3dPoint3D& pos, float& dir, obj_ServerPlayer* spawnedPlayer);
	
	void		CheckForRespawnPlayers();
	void		 RespawnPlayer(obj_ServerPlayer* plr);
	
	void		AddPlayerReward(obj_ServerPlayer* plr, EPlayerRewardID rewardID);
	wiStatsTracking	  GetRewardData(obj_ServerPlayer* plr, EPlayerRewardID rewardID);
	void		AddDirectPlayerReward(obj_ServerPlayer* plr, const wiStatsTracking& in_rwd, const char* rewardName);

	//
	// async funcs. TODO: make a pool of requests and process them every few seconds
	//

	// give item to player inventory on DB - not in game session
	struct PlayerGiveItem_s
	{
	  DWORD		CustomerID;
	  DWORD		SessionID;
	  uint32_t	itemID;
	};
	void		GiveItemToPlayer(obj_ServerPlayer* plr, uint32_t itemID);

	struct PlayerGiveItemInMin_s
	{
		DWORD		CustomerID;
		DWORD		SessionID;
		uint32_t	itemID;
		uint32_t	minutes;
	};
	void		GiveItemToPlayerInMinutes(obj_ServerPlayer* plr, uint32_t itemID, uint32_t numMin);

	struct PlayerRemoveItem_s
	{
		DWORD		CustomerID;
		DWORD		SessionID;
		uint32_t	itemID;
	};
	bool		RemoveItemFromPlayer(obj_ServerPlayer* plr, uint32_t itemID);

	struct PlayerBuyItem_s
	{
		DWORD	CustomerID;
		DWORD	SessionID;
		uint32_t	itemID;
		int			BuyIdx;
	};

	// security stuff
	struct LogInfo_s
	{
	  int	CheatID;	// or 0 for normal logging messages
	  DWORD	CustomerID;
	  DWORD IP;

	  char	Msg[2048];
	  char	Data[4096];
	};

	void		CheckClientsSecurity();
	void		LogInfo(DWORD peerId, const char* msg, const char* fmt = "", ...);
	void		LogCheat(DWORD peerId, int LogID, int disconnect, const char* msg, const char* fmt = "", ...);
	
	// GLM_Conquest data
	bool		m_isGameHasStarted; // set every update, also used in bomb mode to figure out if players in lobby or in game already
	bool		m_requestGameStart;
	bool		m_isTestingGame;
	bool		gameFinished_;
	float		gameEndTime_;
	float		gameStartTime_;
	float		gameNoPeersConnectedTime; // time counter when there are no peers on a map
	float		gameStartCountdown; // when enough players are in game, start counter and then start actual game
	float		respawnDelay_;
	float		tickets_[2];
	float		nextTicketUpdate_;
	float		nextScoreUpdate_;
	int		weaponDataUpdates_;	// number of times weapon data was updated

	// for achievement tracking. 
	bool m_FirstKill;
	
	r3dPoint3D	bomb_droppedPos;
	bool		bomb_isDropped;
	float		bomb_nextRoundStartTimer;

	void		UpdateGameLogic_CheckBattleZone();
	void		UpdateGameLogic_UpdateTickets();
	bool		UpdateGameLogic_CheckEnd();

	void		Bomb_ReportWinRound(int winTeamID, bool winByObjective);
	
	void		SendTicketUpdate();
	void		SendWeaponsInfoToPlayer(bool broadcast, DWORD peerId);
	
	// return if tickets update and control point conquering is enabled
	bool		IsFairGame() const { return m_isGameHasStarted; }
	
	FILE*		chatLogFile_;
	void		OpenChatLog();
	void		CloseChatLog();
	
	struct SimpleStats_s
	{
	  time_t	StartTime;
	  int		Joined;
	  int		MaxPlayers;
	  
	  const char*	EndReason;
	  const char*	TeamWon;
	  int		Tickets[2];
	  
	  SimpleStats_s()
	  {
	    memset(this, 0, sizeof(*this));
	    StartTime = time(NULL);
	    EndReason = "";
	    TeamWon   = "";
	  }
	  
	  void SetEndData(float* tickets, const char* reason)
	  {
	    EndReason  = reason;
	    Tickets[0] = (int)tickets[0];
	    Tickets[1] = (int)tickets[1];
	    if(tickets[0] == tickets[1]) {
	      TeamWon = "Draw";
	    } else if(tickets[0] > tickets[1]) {
	      TeamWon = "Blue";
	    } else {
	      TeamWon = "Red ";
	    }
	  }
	};
	SimpleStats_s	simpleStats_;
	void		DumpSimpleStats();

	// data size for each logical packet ids
	__int64		netRecvPktSize[256];
	__int64		netSentPktSize[256];
	void		DumpPacketStatistics();

	void		DropWeapon(uint32_t itemID, const r3dVector& pos, const r3dVector& rot, uint32_t numBullets, gobjid_t prevOwner, bool isPermDrop, const wiWeaponAttachment& attms);
	bool		DropLootBox(obj_ServerPlayer* killedPlr);
	
	void		SendGameFinish(PKT_S2C_GameFinish_s& n, const char* why);
	void		RewardEndGame(int winTeam, bool isDraw);
	void		CheckForLevelUp(obj_ServerPlayer* plr, int addedHp);
	void		AwardLevelUpBonus(obj_ServerPlayer* plr, int newLevel);
	void		SendRoundStatsToPlayers();
	void		SendScoreToPlayers(DWORD peerId);
	
  public:
	ServerGameLogic();
	virtual ~ServerGameLogic();

	void		Init(const GBGameInfo& ginfo, uint32_t creatorID);
	void		CreateHost(int port);
	void		Disconnect();

	void		Tick();

	void		StartGame(bool resetPlayerTime);
	void		SendGameClose();
	
	struct WeaponStats_s
	{
	  uint32_t ItemID;
	  int	ShotsFired;
	  int	ShotsHits;
	  int	Kills;
	  
	  WeaponStats_s()
	  {
		ItemID = 0;
		ShotsFired = 0;
		ShotsHits = 0;
		Kills = 0;
	  }
	};
	std::vector<WeaponStats_s> weaponStats_;
	
	DWORD m_LastKiller_PeerID; // this is used for an achivement. 
	
	void		TrackWeaponUsage(uint32_t ItemID, int ShotsFired, int ShotsHits, int Kills);

	struct ProfileUpd_s
	{
	  int idx;
	  volatile DWORD status;
	  HANDLE hThread;
	  
	  DWORD CustomerID;
	  wiStats stats;
	  CLoadoutUsageTrack loadoutUsage_[wiUserProfile::MAX_LOADOUT_SLOTS];
	  BYTE plrLevelUpMin;
	  BYTE plrLevelUpMax;
	  int MapID;
	  int MapType;
	  int TeamID;

	  CServerUserProfile *targetProfile;
	};

	struct AchievementThreadData
	{
		CServerUserProfile *targetProfile;
		wiAchievement* achievement;
		int achievementCount;
	};

	void		UpdatePlayerProfiles();

	//int	CheckTeamSkillAvailable(obj_ServerPlayer* plr, int skill);
	bool CheckTeamAbilityAvailable(obj_ServerPlayer* plr, int ability, float radius);
	void ApplyExplosionDamage( GameObject* fromObj, const r3dVector& pos, float radius, int explosiveItemID, const r3dVector& forwVector = R3D_ZERO_VECTOR, float direction = FULL_AREA_EXPLOSION );
	void PreGameCleanUp();
	void FindRespawnPoint( int& cpIdx, int& cpLocIdx, obj_ServerPlayer* plr );
	bool canRecordAchievements();
};

extern	ServerGameLogic	gServerLogic;
