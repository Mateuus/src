#pragma once

#include "UserProfile.h"

class CWOBackendReq;

class CUserClans
{
  public:
	CRITICAL_SECTION csClans_;
	
	enum EClanResultCodes
	{
/*	
	20 - not enough slots in clan	
	21 - already joined clan
	22 - no gamertag
	23 - no permission
	24 - already have pending application to specified clan
	25 - have maximum pending applications
	27 - clan name already exist
	28 - clan tag already exist
*/
	};
	int		GetResultCode2(const char* apiName, CWOBackendReq& req);
	
	enum EClanEvents
	{
	  CLANEvent_Created = 1,

	  CLANEvent_SetRank = 3,
		//Var1 - Who
		//Var2 - member
		//Var3 - rank
		//Text1 - gamertag
		//Text2 - gamertag
   
	  CLANEvent_Joined = 4,
		//Var1 - Who
		//Text1 - gamertag

	  CLANEvent_Left   = 5,
		//Var1 - who
		//Text1 - gamertag

	  CLANEvent_Kick   = 6,
		//Var1 - Who
		//Var2 - member
		//Text1 - gamertag
		//Text2 - gamertag

	  CLANEvent_DonateToClanGP = 10,
		//Var1 - who
		//Var3 - amount
		//Text1 - gamertag

	  CLANEvent_DonateToMemberGP = 11,
		//Var1 - who
		//Var2 - member
		//Var3 - amount
		//Text1 - gamertag
		//Text2 - gamertag

	  CLANEvent_LevelUp = 12,
		//Var1 - level
		
	  ClanEvent_AddMaxMembers = 13,
		//Var1 - who
		//Var2 - added members
		//Text1 - gamertag
	};
	
	// clan log
	struct log_s
	{
	  time_t	EventDate;
	  int		EventType;	// of EClanEvents
	  
	  int		Var1;
	  int		Var2;
	  int		Var3;
	  int		Var4;
	  std::string	Text1;
	  std::string	Text2;
	  std::string	Text3;
	};
	std::list<log_s> log_;
	
  public:
	CUserClans();
	~CUserClans();
	
	//
	// clan creation
	//
	struct CreateParams_s
	{
	  char		ClanName[64*2]; // utf8
	  int		ClanNameColor;
	  char		ClanTag[16]; // utf8
	  int		ClanTagColor;
	  int		ClanEmblemID;
	  int		ClanEmblemColor;
	  
	  CreateParams_s()
	  {
	    memset(this, 0, sizeof(*this));
	  }
	};
	int		ApiClanCheckIfCreateNeedMoney(int* out_NeedMoney);
	int		ApiClanCreate(const CreateParams_s& params);
	
	//
	// clan members
	//
	struct ClanMember_s
	{
	  int		CustomerID;
	  char		gamertag[64]; // utf8
	  int		ClanRank;
	  int		ContributedXP;
	  int		ContributedGP;
	  wiStats	stats;
	};
	typedef std::list<ClanMember_s> TClanMemberList;
	void		ParseClanMember(pugi::xml_node& xmlNode, ClanMember_s& member);
	
	//
	// clan information
	//
	struct ClanInfo_s
	{
	  int		ClanID;
	  
	  char		OwnerGamertag[32*2]; // utf8
	  char		ClanName[64*2]; // utf8
	  int		ClanNameColor;
	  char		ClanTag[5*2]; // utf8
	  int		ClanTagColor;
	  int		ClanEmblemID;
	  int		ClanEmblemColor;
	  char		ClanLore[512*2]; // utf8
	  
	  int		ClanXP;
	  int		ClanLevel;
	  int		ClanGP;		// clan GP reserve
	  int		NumClanMembers;
	  int		MaxClanMembers;
	  
	  ClanInfo_s()
	  {
	    memset(this, 0, sizeof(*this));
	  }
	};
	void		ParseClanInfo(pugi::xml_node& xmlNode, ClanInfo_s& clan);

	// our current clan info and members
	ClanInfo_s	clanInfo_;
	TClanMemberList	clanMembers_;

	// retreive clan info. if out_members is set, then members structure will be filled as well
	int		ApiClanGetInfo(int ClanID, ClanInfo_s* out_info, TClanMemberList* out_members);
	
	// leaderboards - all clan info
	std::list<ClanInfo_s> leaderboard_;
	int		ApiClanGetLeaderboard();
	
	
	void		RemoveMember(int MemberID);
	ClanMember_s*	GetMember(int MemberID);
	
	//
	// generic clan functions
	//
	int		ApiClanLeave();
	int		ApiClanKick(int MemberID);
	int		ApiClanSetRank(int MemberID, int Rank);
	int		ApiClanSetLore(const char* lore);
	int		ApiClanDonateGPToClan(int GP);
	int		ApiClanDonateGPToMember(int MemberID, int GP);
	int		ApiClanBuyAddMembers(int AddMemberIdx); // AddMemberIdx- index in gUserProfile.ShopClanAddMembers_ 
	
	//
	// clan->player invites
	//
	int		ApiClanSendInvite(const char* gamertag);
	int		ApiClanAnswerInvite(int ClanInviteID, int Accept);
	
	//
	// player->clan applications
	//
	int		ApiClanApplyToJoin(int ClanID, const char* note);
	int		ApiClanApplyAnswer(int ClanApplID, bool Accept);
	
	//
	// pereodically updated data from LoginSessionPoller
	// NOTE: access to all vars inside must be guarded with csClans_
	//
	volatile bool	gotNewData;
	void		SetCurrentData(pugi::xml_node& xmlNode);

	// current clan data, should be used for join/leave/new members check
	struct ClanCurrentData_s
	{
	  int		ClanID;
	  int		ClanRank;
	  int		MaxClanMembers;
	  int		NumClanMembers;
	};
	ClanCurrentData_s clanCurData_;
	void		ParseClanCurrentData(pugi::xml_node& xmlNode);

	struct ClanApplication_s
	{
	  int		ClanApplID;
	  std::string	Gamertag;
	  wiStats	stats;
	  std::string	Note;
	};
	std::list<ClanApplication_s> clanApplications_;
	void		ParseClanApplications(pugi::xml_node& xmlNode);

	struct ClanInvite_s
	{
	  int		ClanInviteID;
	  std::string	Gamertag;	// gamertag of who invited you
	  
	  std::string	ClanName;
	  int		ClanLevel;
	  int		ClanEmblemID;
	  int		ClanEmblemColor;
	};
	std::vector<ClanInvite_s> clanInvites_;
	void		ParseClanInvites(pugi::xml_node& xmlNode);
};

