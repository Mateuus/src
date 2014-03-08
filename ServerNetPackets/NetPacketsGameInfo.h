#pragma once

#pragma pack(push)
#pragma pack(1)

#define MAX_POSSIBLE_PLAYERS	128

#define SBNET_MASTER_PORT	33000	// default port for master server
#define GBNET_CLIENT_PORT	33001	// default port for game browser master server (client requests)
#define SBNET_MASTER_WATCH_PORT	33005	// watch port for master server
#define SBNET_SUPER_WATCH_PORT	33006	// watch port for supervisor server
#define SBNET_GAME_PORT		33010

enum EGBGameRegion
{
	GBNET_REGION_Unknown   = 0,
	GBNET_REGION_US_West   = 1,
	GBNET_REGION_US_East   = 2,
	GBNET_REGION_Europe    = 10,
	GBNET_REGION_Russia    = 20,
};

// MAKE SURE to increase GBGAMEINFO_VERSION after changing following structs
#define GBGAMEINFO_VERSION 0x00000008

struct GBGameInfo
{
	enum EMapId
	{
	  MAPID_Editor_Particles = 0,
	  MAPID_WO_Crossroads, // not used
	  MAPID_WO_Crossroads16,
	  MAPID_WO_Grozny,
	  MAPID_WO_Torn,
	  MAPID_WO_Jungle02,
	  MAPID_WO_Shipyard,
	  MAPID_WO_TestConquest,
	  MAPID_WO_TestDeathmatch,
	  MAPID_WO_Nightfall_CQ,
	  MAPID_BurningSea,
	  MAPID_WO_Torn_CT,
	  MAPID_WO_Grozny_CQ,
	  MAPID_WO_CrossroadsRedux_NOTUSED, // not used
	  MAPID_WO_Inferno,
	  MAPID_WO_Wasteland,
	  MAPID_WO_EasternBunkerTDM, 
	  MAPID_WO_Crossroads2, 
	  MAPID_WO_NightfallPAX, // not used
	  MAPID_WO_TornTown,
	  MAPID_WO_Valley, // not used
	  MAPID_WO_TestSabotage,
	  MAPID_WO_Citadel_DM,
	  // NOTE: do *NOT* add maps inside current IDs, add ONLY at the end
	  // otherwise current map statistics in DB will be broken
	  MAPID_MAX_ID,
	};

	enum EMapType
	{
		MAPT_Conquest=0,
		MAPT_Deathmatch,
		MAPT_Siege,
		MAPT_Bomb, 
	};

	char	name[16];
	BYTE	mapId;
	BYTE	mapType;
	BYTE	maxPlayers;
	BYTE	minPlayers; // min players to start the game
	WORD	startTickets;
	WORD	timeLimit;
	WORD	respawnDelay; // numRounds in bomb mode
	BYTE	friendlyFire;
	BYTE	autoBalance;
	BYTE	permGameIdx;

	// min/max player level to join this game. not need to pass via command line
	BYTE	minLevel;
	BYTE	maxLevel;

	// game region
	BYTE	region;
	
	GBGameInfo()
	{
	  sprintf(name, "g%08X", this);
	  mapId = 0xFF;
	  mapType = 0xFF;
	  maxPlayers = 0;
	  minPlayers = 0;
	  startTickets = 100;
	  timeLimit = 0;
	  respawnDelay = 0;
	  friendlyFire = 0;
	  autoBalance = 0;
	  permGameIdx = 0;
	  minLevel = 0;
	  maxLevel = 99;
	  region = GBNET_REGION_Unknown;
	}
	
	bool IsValid() const
	{
	  if(mapId == 0xFF) return false;
	  if(mapType == 0xFF) return false;
	  if(maxPlayers == 0) return false;
	  if(minPlayers == 0) return false;
	  if(timeLimit == 0) return false;
	  if(respawnDelay == 0) return false;
	  return true;
	}
	
	bool FromString(const char* arg) 
	{
	  int v[13];
	  int args = sscanf(arg, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
	    &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
	  if(args != 13) return false;
	  
	  mapId         = (BYTE)v[0];
	  maxPlayers    = (BYTE)v[1];
	  timeLimit     = (WORD)v[2];
	  respawnDelay  = (WORD)v[3];
	  friendlyFire  = (BYTE)v[4];
	  autoBalance   = (BYTE)v[5];
	  permGameIdx   = (BYTE)v[6];
	  mapType       = (BYTE)v[7];
	  minPlayers    = (BYTE)v[8];
	  region        = (BYTE)v[9];
	  minLevel      = (BYTE)v[10];
	  maxLevel      = (BYTE)v[11];
	  startTickets =  (WORD)v[12];
	  return true;
	}
	
	void ToString(char* arg) const
	{
	  sprintf(arg, "%d %d %d %d %d %d %d %d %d %d %d %d %d", 
	    mapId,
	    maxPlayers,
	    timeLimit,
	    respawnDelay,
	    friendlyFire,
	    autoBalance,
	    permGameIdx,
	    mapType,
	    minPlayers,
	    region,
	    minLevel,
	    maxLevel,
		startTickets);
	}
};

// MAKE SURE to increase GBGAMEINFO_VERSION
struct GBUserInfo
{
	BYTE	teamId;
	DWORD	CustomerID;
	char	gbUserName[16];
	
	GBUserInfo()
	{
	  gbUserName[0] = 0;
	}
};

#pragma pack(pop)
