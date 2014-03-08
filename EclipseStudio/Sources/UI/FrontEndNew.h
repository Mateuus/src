#ifndef __FRONTENDNEW_H__
#define __FRONTENDNEW_H__

#include "UIMenu.h"
#include "../../ServerNetPackets/NetPacketsGameBrowser.h"

#include "CkHttp.h"

#include "FrontEndShared.h"


#define MAX_FRONTEND_ACHIEVEMENT_QUEUE 8

class FrontendUI : public UIMenu, public r3dIResource
{
public:
	static bool		frontendFirstTimeInit;
	static bool		frontendRequestShowBuyPack;

//	struct EventCalendar
//	{
//		int month;
//		int day;
//		char* text;
//		EventCalendar():month(-1), day(-1), text(0) {};
//		~EventCalendar() {if(text) free(text); }
//	};
	// this will be filled up in separate thread
//static	volatile bool gHasCalendar;
//static	EventCalendar m_CalendarEvents[32];
//static	int m_numCalendarEvents;

	virtual bool 	Unload();
private:
	r3dScaleformMovie gfxBuyPackMovie;
	bool		showBuyPackMovie;

	Scaleform::Render::D3D9::Texture* RTScaleformTexture;
	Scaleform::Render::D3D9::Texture* RTWelcomeBackScaleformTexture;
	bool		needReInitScaleformTexture;

	static FrontendUI*		This_;
	bool	showDisconnectErrorMessage;
	const wchar_t* disconnectErrorMsg;
	bool	exitRequested_;
	bool	showQuickJoin;
	bool	needUpdateSettings_;
	bool	needExitByGameJoin_;
	bool	needUpdateProfileOnActivate;
	bool	needUpdateProfile;
	DWORD		settingsChangeFlags_;

	bool showLoadoutWasFixedMsg;

	bool m_StartShootingGallery;
	// vars for passing info from callbacks to actual work functions
	NetPacketsGameBrowser::GBPKT_C2M_CreateGame_s createGameInfo_;
public:
	static NetPacketsGameBrowser::GBPKT_C2M_QuickGameReq_s quickGameInfo_; // end of round will use this info for next round
private:
	int		browseJoinSlot_;	// -1 for quick join, -2 for friends join
	float		lastServerReqTime_;

	uint32_t	mStore_buyWpnID;
	uint32_t	mStore_buyItemID;
	uint32_t	mStore_buyItemExp;
	uint32_t	mStore_buyItemPrice;
	uint32_t	mStore_buyItemPriceGD;
	uint32_t	mStorePremiumPrice;
	char		mStore_ChangeGamertag[32];
	int		StoreDetectBuyIdx();
	bool		UpdateInventoryWithBoughtItem();	// update inventory & UI with mStore_buyItem* vars

	uint32_t	mArmory_UnlockItemID;
	uint32_t	mArmory_UnlockSlotID;
	uint32_t	mArmory_UnlockUpgradeID;
	int			mArmory_UnlockIsGP;
	int			mArmory_UpgradeSuccess;
	uint32_t	mArmory_EquipWpnID;
	uint32_t	mArmory_EquipAttmID;
	uint32_t	mArmory_EquipAttmSlot;

	//bool	needScaleformReset;

	r3dTexture*	m_pBackgroundTex;
	//r3dTexture* m_pBackgroundPremiumTex;
	bool		m_hasPremiumAccount;
	float		m_premiumBackgroundFadeIn;

	void	addAvailableFPSUpgrades(int wpnID);
	
	void	updateInfoMsgText(wchar_t* msg);
private:
	void	eventBuyPackContinue(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBuyPackSteam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBuyPackNonSteam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void	eventArmorySelectLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmorySaveLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryModifyOnEquip(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryCurrentWeaponSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryRequestCharacterRender(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryRequestUnlockLoadoutSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBrowseGamesRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBrowseGamesUpdateFilter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBrowseRegionRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBrowseGamesJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBrowseGamesEnterPassword(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventCreateGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventJoinQuickGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStoreBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void	CheckOnBuyAchievements();

	void	eventArmoryOnOpenFPSUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryOnCloseFPSUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryOnEquipAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStoreBuyAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void	eventClansShowClans(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansShowClanMembers(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansCreateClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansJoinClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansLeaveClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansDonateGCToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansDonateGCToMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansPromoteMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansDemoteMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansKickMember(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansShowApplications(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansApplicationAnswer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansApplicationInvitePlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClanPopUpClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansInviteAnswer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClansBuyClanSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	//
	// clan asyncs & functions
	//
	enum EClanPopup
	{
		CLPopup_JoinedClan,
		CLPopup_LeftClan,
		CLPopup_NewInvites,
		CLPopup_NewApplications,
	};
	struct ClanPopup_s 
	{
		int		type;
		int		ID;
		int		clanEmblemID;
		int		clanEmblemColor;
		int		clanLevel;
		char	clanName[64]; // utf8
		ClanPopup_s(int in_type, int in_Id, int in_emblemID, int in_emblemColor, int in_level, const char* in_clanName)
		{
			type        = in_type;
			ID			= in_Id;
			clanEmblemID= in_emblemID;
			clanEmblemColor=in_emblemColor;
			clanLevel = in_level;
			r3dscpy(clanName, in_clanName);
		}
	};
	void	updateMyClanInfoUI();
	void	ProcessClanAPIResult(int apiCode);
	void	updateClanData();
	bool		m_clanPopupActive;
	std::deque<ClanPopup_s> m_clanPopupQuery;
	void		 UpdateClanPopups();

	void	eventUnlockPremiumAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventExit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsControlsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsControlsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsLanguageSelection(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLearnSkill(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventResetSkillTree(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOptionsControlsRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRenewItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventWelcomeBackBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventConfirmHighLevelGameJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStoreChangeGamertag(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStorePurchaseGP(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStorePurchaseGP_Steam(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStoreEntered(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventStoreRequestPackageDetails(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventInventorySellLootBox(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryOnOpenUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryUpgradeUnlockSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventArmoryUpgradeSlot(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventClaimDailyReward(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventWelcomeBackClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventSwitchToShootingRange(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	// friend events
	void	eventAddFriend(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRemoveFriend(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFriendJoinGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFriendRequestAccept(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFriendRequestDecline(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFriendPopupClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	

	// callbacks
	/*
	friend void eventStatsAbilUnlockClick(void* data, r3dScaleformMovie *pMovie, const char *arg);
	friend void fsAbilityUnlock(void* data, r3dScaleformMovie *pMovie, const char *arg);
	friend void fsLearnSkill(void* data, r3dScaleformMovie *pMovie, const char *arg);
	*/

	void	initFrontend();
	void	setBaseStats();
	void	setAdditionalStats();

	void	updateServerTime();
	float	prevServerTimeUpdate;
	time_t  m_serverTime;
	int		minToAddToServerTime;

	void	updateInventoryAndSkillItems();
	void	addSkillItemToInventory(uint32_t itemID);
	void	updateAllAttachments();

	void UpdateSettings();

	// frontend async worker functions and other stuff
	typedef void (FrontendUI::*fn_finish)();
	typedef unsigned int (WINAPI *fn_thread)(void*);

	float		masterConnectTime_;
	void		DelayServerRequest();
	bool		ConnectToMasterServer();
	void		ParseGameJoinAnswer();
	bool		CheckCybersportLimits();

	int		superPings_[2048]; // big enough to hold all possiblesupervisors id
	void		ProcessSupervisorPings();
	int		 GetSupervisorPing(DWORD ip);
	int		GetGamePing(DWORD gameId);

	struct FilterGames
	{
		bool conquest;
		bool deathmatch;
		bool sabotage;

		int regionSelected;
		int levelBracket;

		FilterGames() 
		{ 
			conquest = true;
			deathmatch = true;
			sabotage = true;

			regionSelected = 0;
			levelBracket = 0;
		}
	};
	static FilterGames	m_FilterGames;

	char		m_GameJoin_Password[64];
	static unsigned int WINAPI as_BrowseGamesThread(void* in_data);
	void		processNewGameList();
	void		OnGameListReceived();
	void		OnGameListReceived_ServerBrowse();
	static unsigned int WINAPI as_CreateGameThread(void* in_data);
	static unsigned int WINAPI as_JoinGameThread(void* in_data);
	static unsigned int WINAPI as_BuyItemThread(void* in_data);
	static unsigned int WINAPI as_BuyAttachmentThread(void* in_data);
	static unsigned int WINAPI as_ChangeGamertagThread(void* in_data);
	static unsigned int WINAPI as_EquipAttachmentThread(void* in_data);
	
	static unsigned int WINAPI as_GetMysteryBoxDetailsThread(void* in_data);
	void						OnGetMysteryBoxSuccess();
	int							m_MysteryBoxItemIdRequested;
	const void*					m_RequestedMysteryBox; // retarded C++!! Cannot forward declare it here as CClientUserProfile::MysteryBox_s, so will leave it as void. :(

	void		OnEquipAttachmentSuccess();
	void		OnBuyAttachmentSuccess();
	void		OnBuyItemSuccess();

	void updateDefaultAttachments( bool isNewItem, uint32_t itemID );
	void		OnBuyPremiumAccountSuccess();
	static unsigned int WINAPI as_BuyLootBoxThread(void* in_data);
	void		OnBuyLootBoxSuccess();
	static unsigned int WINAPI as_SellLootBoxThread(void* in_data);
	void		OnSellLootBoxSuccess();
	static unsigned int WINAPI as_UpdateLoadout(void* in_data);
	void		OnUpdateLoadoutSuccess();
	static unsigned int WINAPI as_GNAGetBalanceThread(void* in_data);
	void		OnGNAGetBalanceSuccess();

	static unsigned int WINAPI as_ClaimDailyRewardThread(void* in_data);
	void		OnClaimDailyRewardSuccess();

	int		m_steamGpItemId;
	static unsigned int WINAPI as_SteamBuyGPThread(void* in_data);
	void		OnSteamBuyGPSuccess();
	void		OnSteamBuyPackSuccess();

	//
	// friends asyncs & functions
	//
	enum EFriendPopup
	{
	  FRPopup_Online,
	  FRPopup_Offline,
	  FRPopup_GameJoined,
	  FRPopup_AddReq,
	  FRPopup_FriendAdded,
	  FRPopup_FriendRemoved,
	};
	struct FriendPopup_s 
	{
	  int		type;
	  DWORD		friendId;
	  char		gamerTag[64];
	  int		HonorPoints;
	  char		clanName[64];
	  FriendPopup_s(int in_type, DWORD in_friendId, const char* in_gamerTag, int in_HonorPoints, const char* in_clanName)
	  {
		type        = in_type;
		friendId    = in_friendId;
		r3dscpy(gamerTag, in_gamerTag);
		HonorPoints = in_HonorPoints;
		r3dscpy(clanName, in_clanName);
	  }
	};
	bool		m_friendPopupActive;
	std::deque<FriendPopup_s> m_friendPopupQuery;
	int		m_numFriendsUpdates;

	bool		WaitForFriendsUpdate();	// will wait for async thread to fetch new friend update
	void		SyncFriendsDataToUI();
	void		UpdateFriendData();
	void		 UpdateFriendPopups();
	void		 SetUIFriendStatus(DWORD friendId, const char* status);

	// friend adding
	char		m_friendAddName[64];
	int		m_friendAddStatus;
	static unsigned int WINAPI as_AddFriendThread(void* in_data);
	void		OnAddFriendSuccess();
	// friend remove
	DWORD		m_friendRemoveId;
	static unsigned int WINAPI as_RemoveFriendThread(void* in_data);
	void		OnRemoveFriendSuccess();
	// friend adding answer
	DWORD		m_friendAckId;
	bool		m_friendAckAllow;
	static unsigned int WINAPI as_AddFriendAckThread(void* in_data);
	void		OnAddFriendAckSuccess();
	// friend game joining (using as_JoinGameThread
	NetPacketsGameBrowser::GBPKT_C2M_JoinFriendGameReq_s m_friendJoinPacket;
	bool		m_friendJoinGameGotPwd;
	bool		m_friendJoinGameOKHighLevelGame;
	void		OnJoinFriendGameSuccess();
	// added friends statistics grab
	std::vector<DWORD> m_friendsAddedIds;
	static unsigned int WINAPI as_GetFriendStatsThread(void* in_data);
	void		OnGetFriendStatsSuccess();
	
	// leaderboard
	int		m_lbCurPos[4];
	int		m_lbTableID;
	static unsigned int WINAPI as_GetLeaderboardThread(void* in_data);
	void		OnGetLeaderboardSuccess();
	void		eventOnEnterLeaderboard(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void		eventOnLeaderboardButton(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	
	//
	//
	//

	fn_finish	asyncFinish_;
	HANDLE		asyncThread_;
	wchar_t		asyncErr_[512];
	void		StartAsyncOperation(fn_thread threadFn, fn_finish finishFn = NULL);
	void		SetAsyncError(int apiCode, const wchar_t* msg);
	void		ProcessAsyncOperation();
	//

	int			m_waitingForKeyRemap; // if -1 - then not waiting, otherwise it is a key remap index that we are about to change

	class obj_AI_Player* m_Player;
	class UIWeaponModel* m_WeaponModel;
public:	
	void SetLoadedThings(obj_AI_Player* plr, UIWeaponModel* wpn)
	{
		r3d_assert(m_Player == NULL);
		r3d_assert(m_WeaponModel == NULL);
		m_Player = plr;
		m_WeaponModel = wpn;
	}
private:
	
	int		m_needPlayerRenderingRequest; // 0-disable, 1-armory, 2-armory_modify,3-home,4-zoom in armory modify, 5-weapon
	int		m_CurrentModifySlot;
	bool	m_ArmoryModify_RequireExit;
	void	addGameMap(int mapID, const wchar_t* name, const char* icon, const wchar_t* desc, const char* options);
	void	addGameMap_numBasicPlayers(int mapID, int numPl);
	void	addGameMap_numPremiumPlayers(int mapID, int numPl);

	// achievement functions. 
	
	bool CheckForPermenentWeapons();
	void RequeustAchievement( int achievementID );
	void ProcessAchievementQueue();
	bool MarkAchievementComplete( int whichAchievement );
	bool IncrementAchievement( int whichAchievement, int value );
	void ShowAchievementRibbon( int whichAchievement );

	// achievement data
	int		m_NumAchievementsRibbonsInQueue;
	int		m_AchievementRibbonQueue[MAX_FRONTEND_ACHIEVEMENT_QUEUE];
	float	m_LastAchievementShownTime;
	
public:
	FrontendUI(const char * movieName, EGameResult gameResult);
	virtual ~FrontendUI();

	void SetNeedUpdateSettings();
	void AddSettingsChangeFlag( DWORD flag );
	void SyncGraphicsUI();

	virtual bool Initialize();

	virtual int Update();

	void bindRTsToScaleForm() ;

	void drawPlayer( int numStereoPasses, int toTemp ) ;

	void initItems();
	void  addStore();
	void UpdateSkillUI();
	void UpdateLoadoutUI();

	//	r3dIResource, callbacks
	//	Need to rebind texture to scaleform after device lost
	virtual	void D3DCreateResource();
	virtual	void D3DReleaseResource(){};
	void RunShootingRange();
	bool  IsObjectVisibleInShooting(const GameObject* obj);
	void SwitchShootingRange(bool set);
};

#endif //__FRONTENDNEW_H__