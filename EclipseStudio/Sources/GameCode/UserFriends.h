#pragma once

#include "UserProfile.h"
class CUserFriends
{
  public:
	CRITICAL_SECTION csFriends_;

	/* friend statuses
	  0 - not added
	  1 - request sent
	  2 - added
	  3 - request denied
	*/

	// friends data
	struct Friend_s
	{
	  DWORD		friendID;
	  char		gamerTag[64];
	  bool		isOnline;
	  __int64	gameSessionId;
	  char		clanName[64];
	};
	std::vector<Friend_s> friendsPrev_;
	std::vector<Friend_s> friendsCur_;
	Friend_s	GetFriendData(DWORD friendId);
	Friend_s*	internalGetFriendData(DWORD friendId);
	const Friend_s*	GetPrevFriendData(DWORD friendId);
	
	struct Stats_s
	{
	  DWORD		friendID;
	  wiStats	stats;
	};
	std::vector<Stats_s> friendStats_;
	wiStats		GetFriendStats(DWORD friendId);
	wiStats*	internalGetFriendStats(DWORD friendId);
	
	// currently pending add reqs
	struct AddReq_s
	{
	  DWORD		friendID;
	  char		gamerTag[64];
	  DWORD		HonorPoints;
	  char		clanName[64];
	};
	std::vector<AddReq_s> addReqs_;
	std::vector<DWORD>    savedAddReqs_;  //
	
	volatile bool	gotNewData;
	void		ClearNewDataFlag();

	void		SetCurrentData(pugi::xml_node& xmlNode);
	void		 ParseAddReqs(pugi::xml_node xmlPending);
	void		 ParseFriendStatus(pugi::xml_node xmlFriends);
	void		SetCurrentStats(pugi::xml_document& xmlFile);
	
	AddReq_s	GetPendingFriendReq();
	void		ClearSavedFriendReq(DWORD friendId);	// used if AddFriendAns api failed
	
  public:
	CUserFriends();
	~CUserFriends();
};

