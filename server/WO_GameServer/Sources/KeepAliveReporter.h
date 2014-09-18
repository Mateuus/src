#pragma once

#include "../../SupervisorServer/Sources/MMapGameInfo.h"

class CKeepAliveReporter
{
  private:
	HANDLE		hMapFile;
	MMapGameInfo_s* info;
	
  public:
	CKeepAliveReporter();
	~CKeepAliveReporter();
	
	void		Init(DWORD in_gameId);	
	void		Close();
	__int64		GetSessionId() const {
	  return info->sessionId;
	}
	
	void		Tick(int numPlayers)
	{
	  InterlockedIncrement(&info->tickCount);
	  info->numPlayers = numPlayers;
	}
	void		SetStarted(bool started)
	{
	  info->isStarted = started ? 1 : 0;
	}
	
	void		SetCrashed()
	{
	  if(info) InterlockedExchange(&info->isCrashed, 1);
	}
};

extern CKeepAliveReporter gKeepAliveReporter;
