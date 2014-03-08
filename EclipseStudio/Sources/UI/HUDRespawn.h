#pragma once

#include "APIScaleformGfx.h"
#include "GameCommon.h"

class HUDRespawn
{
	void	onSlotSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	onTeamSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	onSpawnPointSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	onEnterBattle(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnReadyToDrawCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnBombModeRequestTeamChange(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventEnteredChatMessage(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnRequestPlayerKick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnBombModeRequestSpectator(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnSelectMode(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	bool	isActive_;	
	bool m_Initted;
	int m_teamID;
	int m_spawnID;
	int m_slotID;
	bool readyToDrawCharacter;

	bool m_CPCreated;
	float	m_nextCPScanTime;
	float	m_respawnDelay;
	int		m_updateTimer;
	r3dScaleformMovie gfxMovie;
	bool	m_WaitingForNextRoundBomb;

	float	m_BattleTimer;

	void	SetNewCPBasedOnTeamChange();
	void	SetControlPoints();
	void	UpdateControlPoints();

	// for character rendering
	ObjectManager m_RespawnWorld;
	class obj_AI_Player* m_RespawnPlayer;
public:
	HUDRespawn();
	~HUDRespawn();

	bool 	Init();
	bool 	Unload();
	bool	isInit() const { return m_Initted; }

	void	ReleaseGameWorld();

	int		getSelectedTeamID() const { return m_teamID; }
	bool	isWaitingForNextRound() const { return m_WaitingForNextRoundBomb; }

	void	onRespawnBeaconDestroy(int spawnBeaconNetID);
	void	onRespawnBeaconCreate(int spawnBeaconNetID);

	void 	Update();
	void 	Draw();

	bool	isActive() const { return isActive_; }
	void	Activate(int teamId, int slotNum, bool gameStartedEvent=false);
	void	Deactivate();

	int		getSpawnID() const { return m_spawnID; }
	bool	isSpawning() const { return m_respawnDelay > 0; }

	// bomb mode lobby
	void	UpdateLobbyPlayerList();
	void	Lobby_SetPlayerReady(const char* name, bool isReady);
	void	Lobby_UnlockCreatorOptions();
	void	StartLobbyCountdown();
	void	showChatMsg(bool isTeam, const char* from, bool isAlly, const char* msg);

private: 
	bool    m_KeyboardCaptureStarted;

};
