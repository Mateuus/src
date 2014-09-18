#pragma once

// for GBGameInfo
#include "../../ServerNetPackets/NetPacketsGameInfo.h"

// info needed to start new game
struct CMSNewGameData
{
	GBGameInfo	ginfo;
	char		pwd[16];
	DWORD		CustomerID;
	float		joinEnableTime; // time when this game will be available to public join requests

	CMSNewGameData(const GBGameInfo& in_ginfo, const char* in_pwd, DWORD in_CustomerID)
	{
		ginfo      = in_ginfo;
		r3dscpy(pwd, in_pwd);
		CustomerID = in_CustomerID;
		joinEnableTime = -1;
	}
};

// actual game data
struct CMSGameData : public CMSNewGameData
{
	// filled parameters by RegisterNewGameSlot()
	DWORD		gameId;
	WORD		port;
	__int64		sessionId;
	
	CMSGameData() : CMSNewGameData(GBGameInfo(), "", 0)
	{
		gameId     = 0;
		port       = 0;
		sessionId  = 0;
	}

	CMSGameData& operator=(const CMSNewGameData &rhs)
	{
	  *static_cast<CMSNewGameData*>(this) = rhs;
	  return *this;
	}
};
