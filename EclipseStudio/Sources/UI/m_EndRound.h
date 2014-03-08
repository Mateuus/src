#ifndef CEndRoundMenu_h
#define CEndRoundMenu_h

#include "APIScaleformGfx.h"
#include "GameCommon.h"
#include "../../ServerNetPackets/NetPacketsGameBrowser.h"

#define MAX_END_ROUND_ACHIEVEMENT_QUEUE 10

struct EndRound_PlayerData
{
	char name[64];
	int score;
	int kills;
	int deaths;
};

class HUDEndRound
{
private:
	r3dScaleformMovie gfxMovie;
	r3dScaleformMovie gfxHUDScreen;
	bool	isActive_;	
	float	timeActivated;
	bool	m_movieInitComplete;
	void	initMovieData();
	int		curLevel;
	bool	handledOnNextRound;

	bool	ConnectToMasterServer();

	void	onExit(bool forceAsyncThreadDeath);
	
	static HUDEndRound*		This_;
	
	NetPacketsGameBrowser::GBPKT_C2M_NextRoundReq_s m_nextRoundReq;
	static unsigned int WINAPI as_NextRoundThread(void* in_data);

	void onNextRound();

public:
	HUDEndRound();
	virtual ~HUDEndRound();

	bool	Init();
	bool 	Unload();

	typedef unsigned int (WINAPI *fn_thread)(void*);
	HANDLE		asyncThread_;
	wchar_t		asyncErr_[512];
	void StartAsyncOperation(fn_thread threadFn);
	void		SetAsyncError(int apiCode, const wchar_t* msg);
	void		ProcessAsyncOperation();

	void Update();
	void Draw();

	bool	isActive() const { return isActive_; }
	void	Activate();

	void	eventEndOfRoundContinueClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	levelUpCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void	ShowAchievementRibbon( int whichAchievement );
};


#endif  //CEndRoundMenu_h
