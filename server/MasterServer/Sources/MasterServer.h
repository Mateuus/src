#pragma once

#include "../../MasterServer/Sources/NetPacketsServerBrowser.h"
#include "GameSlotInfo.h"

class CServerBase
{
  public:
	enum stype_e {
	  tUnknown    = 0,
	  tSupervisor,
	  tGame,
	};
	stype_e		type_;
	
	DWORD		id_;
	DWORD		ip_;
	DWORD		peer_;

  public:
	CServerBase(stype_e type, DWORD id, DWORD ip, DWORD peer) :
	  type_(type),
	  id_(id),
	  ip_(ip),
	  peer_(peer)
	{
	  r3d_assert(ip_);
	}
	virtual ~CServerBase()
	{
	}
};

// game server entry
class CServerG : public CServerBase
{
  public:
	CMSGameData	info_;
	bool		finished_;
	
	float		startTime_;
	float		lateTime_;	// time when we should not allow connectin
	
	int		curPlayers_;
	GBUserInfo	users_[MAX_POSSIBLE_PLAYERS];
	
	// times when some players are expected to join.
	struct joiners_s
	{
	  float		timeExpected;
	  DWORD		CustomerID;
	}		joiners_[MAX_POSSIBLE_PLAYERS];
	int		maxJoiners_;
	
  public:
	CServerG(DWORD id, DWORD ip, DWORD peer);
	const char*	GetName() const;

	void		Init(const CMSGameData& in_info);
	void		SetCurrentData(float timeLeft, int curPlayers, const GBUserInfo* users);

	const GBGameInfo& getGameInfo() const {
	  return info_.ginfo;
	}
	
	bool		isValid() const {
	  return info_.port != 0;
	}
	bool		isFinished() const { 
	  return finished_;
	}
	bool		isFull() const {
	  return (curPlayers_ + GetJoiningPlayers()) >= getGameInfo().maxPlayers;
	}
	bool		isTooLate() const {
		return r3dGetTime() > lateTime_ && getGameInfo().mapType!=GBGameInfo::MAPT_Bomb; // in bomb mode do not check for late time, as we are using rounds system
	}
	bool		isPermanentGame() const {
	  return getGameInfo().permGameIdx > 0;
	}
	bool		isClosedToJoin() const {
	  return r3dGetTime() < info_.joinEnableTime;
	}
	bool		canJoinGame() const {
	  if(isFinished() || isFull() || isTooLate() || isClosedToJoin())
	    return false;
	  return true;
	}
	bool		isPassworded() const {
	  return info_.pwd[0] != 0;
	}
	
	// return number of possible joining players (ones that connecting right now)
	int		GetJoiningPlayers() const;
	void		AddJoiningPlayer(DWORD CustomerID);
};

// supervisor server entry
class CServerS : public CServerBase
{
  public:
	int		region_;
	std::string	serverName_;

	int		maxPlayers_;
	int		maxGames_;
	int		portStart_;
	int		startedGames_;
	
	static const int REGISTER_EXPIRE_TIME = 30;	// game create request will expire afer this <N> sec
	struct games_s
	{
	  float		createTime;	// time when create request was send
	  float		closeTime;	// time when slot will be considered free
	  
	  CMSGameData	info;		// expected game info
	  CServerG*	game;		// pointer to actual game. NOTE that this class DOES NOT own them (CMasterGameServer.games_ do)
	};
	games_s*	games_;

	__int64		CreateSessionId();
	DWORD		CreateGameId(int gameSlot) 
	{
	  r3d_assert(id_ < 0xFFFF);
	  r3d_assert(gameSlot < 0xFF);
	  DWORD id = (id_ << 16) | ((++startedGames_ & 0xFF) << 8) | gameSlot;
	  return id;
	}
	void		ParseGameId(DWORD gameId, DWORD* serverId, int* gameSlot) const
	{
	  *serverId = (gameId >> 16);
	  *gameSlot = gameId & 0xff;
	}

  public:
	CServerS(DWORD id, DWORD ip, DWORD peer) : CServerBase(tSupervisor, id, ip, peer) 
	{
	  region_      = GBNET_REGION_Unknown,
	  maxPlayers_  = 0;
	  maxGames_    = 0;
	  portStart_   = 0;
	  startedGames_= 0;
	  
	  games_       = NULL;
	}
	~CServerS() {
	  delete[] games_;
	}
	
	void		Init(const NetPacketsServerBrowser::SBPKT_S2M_RegisterMachine_s& n);
	const char*	GetName() const {
	  return serverName_.c_str();
	}
	
	bool		RegisterNewGameSlot(const CMSNewGameData& ngd, NetPacketsServerBrowser::SBPKT_M2S_StartGameReq_s& out_n);
	int		GetExpectedGames() const;	// return number of playing AND pending (requested to create) games
	int		GetExpectedPlayers() const;	// return number of users

	CServerG*	CreateGame(DWORD gameId, DWORD peerId);
	void		DeregisterGame(const CServerG* game);
};
