#pragma once

#include "../../ServerNetPackets/NetPacketsGameInfo.h"

class CMasterServerConfig
{
  public:
	int		masterPort_;
	int		clientPort_;
	int		serverId_;
	int		masterCCU_;	// max number of connected peers

	//
	// permanent games groups
	//
	struct permGame_s
	{
	  GBGameInfo	ginfo;
	  
	  int		minGames;
	  int		maxGames;
	  permGame_s()
	  {
	    minGames = 0;
	    maxGames = 0;
	  }
	};
	permGame_s	permGames_[256];
	int		numPermGames_;

	void		LoadPermGamesConfig();
	void		 ParsePermamentGame(const char* name, const char* map, const char* data);
	void		 AddPermanentGame(const GBGameInfo& ginfo, int minGames, int maxGames, EGBGameRegion region, int minLevel, int maxLevel);
	
	const GBGameInfo* GetMapForNextRound(int region, int mapType, int mapId, int playerLevel, int curPlayers);

  public:
	CMasterServerConfig();
};
extern CMasterServerConfig* gServerConfig;
