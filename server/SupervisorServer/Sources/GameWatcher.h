#pragma once
#include "MMapGameInfo.h"

class CGameWatcher
{
  public:
	DWORD		gameId;
	__int64		sessionId;
	HANDLE		hProcess;
	  
	HANDLE		hMapFile;
	MMapGameInfo_s* info;

	bool		isStarted;	// if game is activated it main loop
	DWORD		tickCount;	// copy of last tickCount from MMapGameInfo_s
	float		lastTick;	// last time when tickCount changed
	
	void		OnGameCrash();

  public:
	CGameWatcher();
	~CGameWatcher();

	void		Init(DWORD in_GameId, __int64 in_sessionId);
	void		Reset();
	
	void		CheckProcess();
	void		Terminate();
};

