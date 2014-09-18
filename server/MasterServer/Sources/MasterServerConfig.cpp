#include "r3dPCH.h"
#include "r3d.h"

#include "MasterServerConfig.h"

	CMasterServerConfig* gServerConfig = NULL;

static const char* configFile = "MasterServer.cfg";

CMasterServerConfig::CMasterServerConfig()
{
  const char* group      = "MasterServer";

  if(_access(configFile, 0) != 0) {
    r3dError("can't open config file %s\n", configFile);
  }

  masterPort_  = r3dReadCFG_I(configFile, group, "masterPort", SBNET_MASTER_PORT);
  clientPort_  = r3dReadCFG_I(configFile, group, "clientPort", GBNET_CLIENT_PORT);
  masterCCU_   = r3dReadCFG_I(configFile, group, "masterCCU",  3000);

  #define CHECK_I(xx) if(xx == 0)  r3dError("missing %s value in %s", #xx, configFile);
  #define CHECK_S(xx) if(xx == "") r3dError("missing %s value in %s", #xx, configFile);
  CHECK_I(masterPort_);
  CHECK_I(clientPort_);
  #undef CHECK_I
  #undef CHECK_S

  serverId_    = r3dReadCFG_I(configFile, group, "serverId", 0);
  if(serverId_ == 0)
  {
	MessageBox(NULL, "you must define serverId in MasterServer.cfg", "", MB_OK);
	r3dError("no serverId");
  }
  if(serverId_ > 255 || serverId_ < 1)
  {
	MessageBox(NULL, "bad serverId", "", MB_OK);
	r3dError("bad serverId");
  }
  
  LoadPermGamesConfig();
  
  return;
}

void CMasterServerConfig::LoadPermGamesConfig()
{
  numPermGames_ = 0;

//#ifdef _DEBUG
//  r3dOutToLog("Permanet games disabled in DEBUG");
//  return;
//#endif
  
  for(int i=0; i<250; i++)
  {
    char group[128];
    sprintf(group, "PermGame%d", i+1);

    char map[512] = "";
    char data[512] = "";
    r3dscpy(map,  r3dReadCFG_S(configFile, group, "map", ""));
    r3dscpy(data, r3dReadCFG_S(configFile, group, "data", ""));

    if(*map == 0)
      continue;
    
    ParsePermamentGame(group, map, data);
  }

  return;  
}

static int StringToGBMapID(char* str)
{
  if(stricmp(str, "MAPID_WO_Crossroads16") == 0)
    return GBGameInfo::MAPID_WO_Crossroads16;
  if(stricmp(str, "MAPID_WO_Crossroads2") == 0)
	  return GBGameInfo::MAPID_WO_Crossroads2;
  if(stricmp(str, "MAPID_WO_Grozny") == 0)
    return GBGameInfo::MAPID_WO_Grozny;
  if(stricmp(str, "MAPID_WO_Torn") == 0)
    return GBGameInfo::MAPID_WO_Torn;
  if(stricmp(str, "MAPID_BurningSea") == 0)
    return GBGameInfo::MAPID_BurningSea;
  if(stricmp(str, "MAPID_WO_Shipyard") == 0)
    return GBGameInfo::MAPID_WO_Shipyard;
  if(stricmp(str, "MAPID_WO_Nightfall_CQ") == 0)
    return GBGameInfo::MAPID_WO_Nightfall_CQ;
  if(stricmp(str, "MAPID_WO_Torn_CT") == 0)
    return GBGameInfo::MAPID_WO_Torn_CT;
  if(stricmp(str, "MAPID_WO_Inferno") == 0)
	  return GBGameInfo::MAPID_WO_Inferno;
  if(stricmp(str, "MAPID_WO_Wasteland") == 0)
	  return GBGameInfo::MAPID_WO_Wasteland;
  if(stricmp(str, "MAPID_WO_EasternBunkerTDM") == 0)
	  return GBGameInfo::MAPID_WO_EasternBunkerTDM;
  if(stricmp(str, "MAPID_WO_Jungle02") == 0)
	  return GBGameInfo::MAPID_WO_Jungle02;
  if(stricmp(str, "MAPID_WO_Citadel_DM") == 0)
	  return GBGameInfo::MAPID_WO_Citadel_DM;
  if(stricmp(str, "MAPID_WO_NightfallPAX") == 0)
	  return GBGameInfo::MAPID_WO_NightfallPAX;

  if(stricmp(str, "MAPID_Editor_Particles") == 0)
    return GBGameInfo::MAPID_Editor_Particles;
    
  r3dError("bad GBMapID %s\n", str);
  return 0;
}

static int StringToGBMapType(const char* str)
{
  if(stricmp(str, "MAPT_Conquest") == 0)
    return GBGameInfo::MAPT_Conquest;
  if(stricmp(str, "MAPT_Deathmatch") == 0)
    return GBGameInfo::MAPT_Deathmatch;
  if(stricmp(str, "MAPT_Siege") == 0)
    return GBGameInfo::MAPT_Siege;
  if(stricmp(str, "MAPT_Bomb") == 0)
    return GBGameInfo::MAPT_Bomb;
    
  r3dError("bad GBMapType %s\n", str);
  return 0;
}

static EGBGameRegion StringToGBRegion(const char* str)
{
  if(stricmp(str, "GBNET_REGION_US_West") == 0)
    return GBNET_REGION_US_West;
  if(stricmp(str, "GBNET_REGION_US_East") == 0)
    return GBNET_REGION_US_East;
  if(stricmp(str, "GBNET_REGION_Europe") == 0)
    return GBNET_REGION_Europe;
  if(stricmp(str, "GBNET_REGION_Russia") == 0)
    return GBNET_REGION_Russia;
    
  r3dError("bad GBGameRegion %s\n", str);
  return GBNET_REGION_Unknown;
}

void CMasterServerConfig::ParsePermamentGame(const char* name, const char* map, const char* data)
{
  char mapid[128];
  char maptype[128];
  char region[128];
  int minGames;
  int maxGames;
  if(5 != sscanf(map, "%s %s %s %d %d", mapid, maptype, region, &minGames, &maxGames)) {
    r3dError("bad map format: %s\n", map);
  }

  int maxPlayers;
  int minPlayers;
  int timeLimit;
  int respawnDelay;
  int minLevel;
  int maxLevel;
  int tickets;
  if(7 != sscanf(data, "%d %d %d %d %d %d %d", &maxPlayers, &minPlayers, &timeLimit, &respawnDelay, &minLevel, &maxLevel, &tickets)) {
    r3dError("bad data format: %s\n", data);
  }

  GBGameInfo ginfo;
  ginfo.mapId        = StringToGBMapID(mapid);
  ginfo.mapType      = StringToGBMapType(maptype);
  ginfo.maxPlayers   = maxPlayers;
  ginfo.minPlayers	 = minPlayers;
  ginfo.timeLimit    = timeLimit;
  ginfo.respawnDelay = respawnDelay;
  ginfo.friendlyFire = 0;
  ginfo.autoBalance  = 1;
  ginfo.startTickets = tickets;
  r3dscpy(ginfo.name, name);

  r3dOutToLog("permgame: %s, %s at %s, %d->%d, maxp:%d, minp:%d, tl:%d, rd:%d\n", 
    name, mapid, region, minGames, maxGames, maxPlayers, minPlayers, timeLimit, respawnDelay);
  
  EGBGameRegion eregion = StringToGBRegion(region);
  
  if(eregion == GBNET_REGION_US_West)
  {
	  AddPermanentGame(ginfo, minGames, maxGames, GBNET_REGION_US_West, minLevel, maxLevel);
	  AddPermanentGame(ginfo, minGames, maxGames, GBNET_REGION_Europe, minLevel, maxLevel);
  }
  else
	AddPermanentGame(ginfo, minGames, maxGames, eregion, minLevel, maxLevel);
}

void CMasterServerConfig::AddPermanentGame(const GBGameInfo& ginfo, int minGames, int maxGames, EGBGameRegion region, int minLevel, int maxLevel)
{
  r3d_assert(numPermGames_ < R3D_ARRAYSIZE(permGames_));
  permGame_s& pg = permGames_[numPermGames_++];

  pg.ginfo     = ginfo;
  pg.ginfo.permGameIdx = numPermGames_;
  pg.ginfo.minLevel    = minLevel;
  pg.ginfo.maxLevel    = maxLevel;
  pg.ginfo.region      = region;
  pg.minGames  = minGames;
  pg.maxGames  = maxGames;
  
  return;
}

const GBGameInfo* CMasterServerConfig::GetMapForNextRound(int region, int mapType, int mapId, int playerLevel, int curPlayers)
{
  const GBGameInfo* found = NULL;
  
  // scan for all possible games, and return next in list.
  for(int i=0; i<numPermGames_; i++)
  {
    const GBGameInfo& gi = permGames_[i].ginfo;
    
    if(region != GBNET_REGION_Unknown && gi.region != region)
      continue;
      
    if(gi.mapType != mapType)
      continue;
      
    if(playerLevel < gi.minLevel || playerLevel > gi.maxLevel)
      continue;
      
    if(curPlayers > gi.maxPlayers)
      continue;

    // set first in list
    if(gi.mapId <= mapId && !found)
      found = &gi;
      
    // found next in list
    if(gi.mapId > mapId) {
      found = &gi;
      break;
    }
  }
  
  return found;
}
