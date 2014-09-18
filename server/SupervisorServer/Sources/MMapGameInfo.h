#pragma once

__declspec(align(32))
struct MMapGameInfo_s
{
	// filled by Supervisor server
	volatile __int64 sessionId;
	volatile DWORD	gameId;

	// filled by game server
	volatile LONG	isStarted;
	volatile LONG	tickCount;
	volatile LONG	isCrashed;
	
	// various game info
	volatile LONG	numPlayers;
	
	void		Reset(DWORD in_gameId, __int64 in_sessionId)
	{
	  sessionId  = in_sessionId;
	  gameId     = in_gameId;
	  isStarted  = 0;
	  isCrashed  = 0;
	  tickCount  = 0;
	  numPlayers = 0;
	}
};
