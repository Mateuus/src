#pragma once

#include "r3dProtect.h"
#include "r3dNetwork.h"
#include "multiplayer/P2PMessages.h"

class GameObject;
class obj_AI_Player;

#include "../../ServerNetPackets/NetPacketsGameInfo.h"

#define FULL_AREA_EXPLOSION  360.0f

class ClientGameLogic : public r3dNetCallback
{
  protected:
	r3dNetwork	g_net;
	// r3dNetCallback virtuals
virtual	void		OnNetPeerConnected(DWORD peerId);
virtual	void		OnNetPeerDisconnected(DWORD peerId);
virtual	void		OnNetData(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);

  public:
	enum { MAX_NUM_PLAYERS = 128 };

	// player struct for hiding actual player pointer
	struct {
		DWORD pad1;
		r3dSec_type<obj_AI_Player*, 0xFA643FB3> ptr;
		DWORD pad2;
		DWORD pad3;
	}		players2_[MAX_NUM_PLAYERS];
	obj_AI_Player*	GetPlayer(int idx) const;
	void		SetPlayerPtr(int idx, obj_AI_Player* ptr);
	int		CurMaxPlayerIdx;	// current max used index in players2_

	DWORD		net_lastFreeId; // !!!ONLY use it for assigning networkID during loading map!!!
	r3dSec_type<obj_AI_Player*, 0xA3BFBAB7> localPlayer_;
	float		localPlayerConnectedTime; // used for calculating how much time player was playing

	bool		requestToJoinAsSpectator;

	// BOMB MODE - LOBBY
	enum { MAX_NUM_LOBBY_PLAYERS = 64};
	struct LobbyPlayerDesc
	{
		char userName[64];
		DWORD peerID;
		BYTE desiredTeam;
		BYTE isReady;
		BYTE isSpectator;
		BYTE level;
		LobbyPlayerDesc() : peerID(-1) {}
	};
	LobbyPlayerDesc lobbyPlayers[MAX_NUM_LOBBY_PLAYERS];

	class obj_DroppedBomb* m_DroppedBomb;
	bool		m_isSpectator;

	bool		serverConnected_;
	volatile LONG	serverVersionStatus_;
	volatile bool	gameJoinAnswered_;
	int		localPlayerIdx_;
	volatile bool	gameStartAnswered_;
	volatile bool	m_gameHasStarted;
	int		gameStartResult_; // as PKT_S2C_StartGameAns_s::EResult
	GBGameInfo	m_gameInfo;
	
	__int64		m_sessionId;
	__int64		GetGameSessionID() {
		if(serverConnected_)
			return m_sessionId;
		else
			return 0;
	}

	float	m_highPingTimer;

	float	showCoolThingTimer;

	BYTE	m_onJoinServerAssignedTeamId;
	
	PKT_S2C_GameFinish_s finishData_;
	bool		gameFinished_;		// game is finished, we should receive 
	bool		gameClosed_;
	bool		gameShuttedDown_;
	
	wiStats		curRoundStat_;
	wiStatsTracking	curRoundScore_;
	BYTE		curRoundLevelUpMin;
	BYTE		curRoundLevelUpMax;
	BYTE		curRoundRewardIdx;

	bool		gameReadyToExit; // after we showed player end of round menu, we are ready to get back to main menu
	bool		gameReadyForNextRound;
	
	// cheat things
	bool		cheatAttemptReceived_;	// true if server detected cheating
	BYTE		cheatAttemptCheatId_;	// server reason
	DWORD		nextSecTimeReport_;	// time when we need send next PKT_C2S_SecurityRep_s
	DWORD		gppDataSeed_;		// seed for sending crc of game player parameters
	bool		d3dCheatSent_;		// if we sended d3d cheat screenshot yet
	
	#define DEFINE_PACKET_FUNC(XX) \
	  void On##XX(const XX##_s& n, GameObject* fromObj, DWORD peerId, bool& needPassThru);
	#define IMPL_PACKET_FUNC(CLASS, XX) \
	  void CLASS::On##XX(const XX##_s& n, GameObject* fromObj, DWORD peerId, bool& needPassThru)
	//  
	int		ProcessWorldEvent(GameObject* fromObj, DWORD eventId, DWORD peerId, const void* packetData, int packetSize);
	 DEFINE_PACKET_FUNC(PKT_C2S_ValidateConnectingPeer);
	 DEFINE_PACKET_FUNC(PKT_C2C_PacketBarrier);
	 DEFINE_PACKET_FUNC(PKT_S2C_LevelInfo);
	 DEFINE_PACKET_FUNC(PKT_S2C_JoinGameAns);
	 DEFINE_PACKET_FUNC(PKT_S2C_ShutdownNote);
	 DEFINE_PACKET_FUNC(PKT_S2C_SetGamePlayParams);
	 DEFINE_PACKET_FUNC(PKT_S2C_StartGameAns);
	 DEFINE_PACKET_FUNC(PKT_S2C_CreatePlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_DropPlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_Damage);
	 DEFINE_PACKET_FUNC(PKT_S2C_KillPlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_RespawnPlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_TicketsUpdate);
	 DEFINE_PACKET_FUNC(PKT_S2C_GameAbort);
	 DEFINE_PACKET_FUNC(PKT_S2C_GameFinish);
	 DEFINE_PACKET_FUNC(PKT_S2C_GameClose);
	 DEFINE_PACKET_FUNC(PKT_S2C_RoundStats);
	 DEFINE_PACKET_FUNC(PKT_S2C_GameAboutToStart);
	 DEFINE_PACKET_FUNC(PKT_S2C_GameStarted);
	 DEFINE_PACKET_FUNC(PKT_C2C_ChatMessage);
	 DEFINE_PACKET_FUNC(PKT_C2C_VoiceCommand);
	 DEFINE_PACKET_FUNC(PKT_C2C_CommRoseCommand);
	 DEFINE_PACKET_FUNC(PKT_S2C_Airstrike);
	 DEFINE_PACKET_FUNC(PKT_S2C_UpdateWeaponData);
	 DEFINE_PACKET_FUNC(PKT_S2C_UpdateGearData);
	 DEFINE_PACKET_FUNC(PKT_S2C_CreateNetObject);
	 DEFINE_PACKET_FUNC(PKT_S2C_DestroyNetObject);
	 DEFINE_PACKET_FUNC(PKT_S2C_CheatWarning);
	 DEFINE_PACKET_FUNC(PKT_S2C_SpawnDroppedWeapon);
	 DEFINE_PACKET_FUNC(PKT_S2C_DestroyDroppedWeapon);
	 DEFINE_PACKET_FUNC(PKT_S2C_SpawnDroppedLootBox);
	 DEFINE_PACKET_FUNC(PKT_S2C_DestroyDroppedLootBox);
	 DEFINE_PACKET_FUNC(PKT_S2C_SpawnMine);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_ConnectedPlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_DisconnectedPlayer);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_PlayerReady);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_PlayerHasBomb);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_WonRound);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_Dropped);
	 DEFINE_PACKET_FUNC(PKT_S2C_Bomb_ChatMsg);
	 DEFINE_PACKET_FUNC(PKT_S2C_TargetMarked);
	 
	// GLM_Conquest
	int		tickets_[2];
	float		gameTimeEnd_;		// expected end of game round
	float		m_gameStartTime; // time when game has started
	float		m_gameLocalStartTime; // just a time when game has started on a client
	
	r3dPoint3D	AdjustSpawnPositionToGround(const r3dPoint3D& pos);

  protected:
	typedef bool (ClientGameLogic::*fn_wait)();
	int		WaitFunc(fn_wait fn, float timeout, const char* msg);
	
	// wait functions
	bool		wait_IsConnected() {
	  return net_->IsConnected();
	}
	bool		wait_ValidateVersion() {
          return serverVersionStatus_ != 0;
        }
	bool		wait_GetLevelName() {
	  return m_gameInfo.mapId != 0xFF;
	}
	bool		wait_GameJoin() {
	  return gameJoinAnswered_;
	}
	bool		wait_GameStart();

  public:
	bool		wait_GameClosed() {  return gameClosed_; }

  private:	
	// make copy constructor and assignment operator inaccessible
	ClientGameLogic(const ClientGameLogic& rhs);
	ClientGameLogic& operator=(const ClientGameLogic& rhs);

  private: // this is singleton, can't create directly.
	ClientGameLogic();
	virtual ~ClientGameLogic();
	
  public:
	static void CreateInstance();
	static void DeleteInstance();
	static ClientGameLogic* GetInstance();

	void		Reset();

	bool	Connect(const char* host, int port);
	void	Disconnect();

	int		RequestToJoinGame();
	int		RequestToStartGame();
	int		ValidateServerVersion(__int64 sessionId);
	int		WaitForLevelName();

	void	ApplyExplosionDamage(const r3dVector& pos, float radius, int wpnIdx, const r3dVector& forwVector = R3D_ZERO_VECTOR, float direction = FULL_AREA_EXPLOSION );
	
	void	Tick();
	void		SendScreenshot();
	void		 SendScreenshotFailed(int code);

	//int	CheckTeamSkillAvailable(obj_AI_Player* plr, int skill);
	bool CheckTeamAbilityAvailable(const obj_AI_Player* teamPlayer, int ability, float radius) const;
};

__forceinline ClientGameLogic& gClientLogic() {
	return *ClientGameLogic::GetInstance();
}

