#include "r3dPCH.h"
#include "r3d.h"

#include "UserClans.h"
#include "backend/WOBackendAPI.h"

CUserClans::CUserClans()
{
	InitializeCriticalSection(&csClans_);
	gotNewData = false;
}

CUserClans::~CUserClans()
{
	DeleteCriticalSection(&csClans_);
}

int CUserClans::GetResultCode2(const char* apiName, CWOBackendReq& req)
{
	// due to original design flaw, 2 digits return code is parsed here...
	if(req.resultCode_ == 2)
	{
		r3d_assert(req.bodyStr_[0] >= '0' && req.bodyStr_[0] <= '9');
		req.resultCode_ = req.resultCode_ * 10 + (req.bodyStr_[0] - '0');
	}
	r3dOutToLog("%s FAILED, code: %d\n", apiName, req.resultCode_);
	return req.resultCode_;
}

/*
// todo

clan applications

question about security
 - donation
 - invites
 - kicking

TODO: clean pending clan invites each day sometime
TODO: clean pending clan applications
*/

void CUserClans::RemoveMember(int MemberID)
{
	for(TClanMemberList::iterator it = clanMembers_.begin(); it != clanMembers_.end(); ++it)
	{
		if(it->CustomerID == MemberID)
		{
			clanMembers_.erase(it);
			return;
		}
	}

	r3d_assert(false && "member not in clan");
	return;
}

CUserClans::ClanMember_s* CUserClans::GetMember(int MemberID)
{
	for(TClanMemberList::iterator it = clanMembers_.begin(); it != clanMembers_.end(); ++it)
	{
		if(it->CustomerID == MemberID)
		{
			return &(*it);
		}
	}

	return NULL;
}

void CUserClans::ParseClanInfo(pugi::xml_node& xmlNode, ClanInfo_s& clan)
{
	clan.ClanID          = xmlNode.attribute("ClanID").as_int();
	clan.ClanNameColor   = xmlNode.attribute("ClanNameColor").as_int();
	clan.ClanTagColor    = xmlNode.attribute("ClanTagColor").as_int();
	clan.ClanEmblemID    = xmlNode.attribute("ClanEmblemID").as_int();
	clan.ClanEmblemColor = xmlNode.attribute("ClanEmblemColor").as_int();
	clan.ClanXP          = xmlNode.attribute("ClanXP").as_int();
	clan.ClanLevel       = xmlNode.attribute("ClanLevel").as_int();
	clan.ClanGP          = xmlNode.attribute("ClanGP").as_int();
	clan.NumClanMembers  = xmlNode.attribute("NumClanMembers").as_int();
	clan.MaxClanMembers  = xmlNode.attribute("MaxClanMembers").as_int();

	r3dscpy(clan.ClanName, xmlNode.attribute("ClanName").value());
	r3dscpy(clan.ClanTag,  xmlNode.attribute("ClanTag").value());
	r3dscpy(clan.ClanLore, xmlNode.attribute("ClanLore").value());
	r3dscpy(clan.OwnerGamertag, xmlNode.attribute("OwnerGamertag").value());
	
	return;
}

void CUserClans::ParseClanMember(pugi::xml_node& xmlNode, ClanMember_s& member)
{
	member.CustomerID        = xmlNode.attribute("id").as_int();
	member.ClanRank          = xmlNode.attribute("cr").as_uint();
	member.ContributedGP     = xmlNode.attribute("cgp").as_uint();
	member.ContributedXP     = xmlNode.attribute("cxp").as_uint();
	member.stats.HonorPoints = xmlNode.attribute("xp").as_uint();
	member.stats.Kills       = xmlNode.attribute("k").as_uint();
	member.stats.Deaths      = xmlNode.attribute("d").as_uint();
	member.stats.Wins        = xmlNode.attribute("w").as_uint();
	member.stats.Losses      = xmlNode.attribute("l").as_uint();
	member.stats.TimePlayed  = xmlNode.attribute("tp").as_uint();

	r3dscpy(member.gamertag, xmlNode.attribute("gt").value());
	
	return;
}

int CUserClans::ApiClanCheckIfCreateNeedMoney(int* out_NeedMoney)
{
	CWOBackendReq req(&gUserProfile, "api_ClanCreate.aspx");
	req.AddParam("func", "check1");
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanCheckIfCreateNeedMoney", req);
	}

	sscanf(req.bodyStr_, "%d", out_NeedMoney);
	return 0;
}

int CUserClans::ApiClanCreate(const CreateParams_s& params)
{
	r3d_assert(gUserProfile.ProfileData.ClanID == 0);
	r3d_assert(params.ClanName[0]!='\0');
	r3d_assert(params.ClanTag[0]!='\0');

	CWOBackendReq req(&gUserProfile, "api_ClanCreate.aspx");
	req.AddParam("func",            "create");
	req.AddParam("ClanName",        params.ClanName);
	req.AddParam("ClanNameColor",   params.ClanNameColor);
	req.AddParam("ClanTag",         params.ClanTag);
	req.AddParam("ClanTagColor",    params.ClanTagColor);
	req.AddParam("ClanEmblemID",    params.ClanEmblemID);
	req.AddParam("ClanEmblemColor", params.ClanEmblemColor);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanCreate", req);
	}
	
	int ClanID = 0;
	int newGPBalance = 0;
	int nargs = sscanf(req.bodyStr_, "%d %d", &ClanID, &newGPBalance);
	if(nargs != 2)
	{
		r3dError("wrong answer for ApiClanCreate");
		return 9;
	}
	
	gUserProfile.ProfileData.ClanID   = ClanID;
	gUserProfile.ProfileData.ClanRank = 0;
	gUserProfile.ProfileData.Stats.GamePoints = newGPBalance;
	
	// retreive new clan info
	return ApiClanGetInfo(ClanID, &clanInfo_, &clanMembers_);
}

int CUserClans::ApiClanGetInfo(int ClanID, ClanInfo_s* out_info, CUserClans::TClanMemberList* out_members)
{
	r3d_assert(out_info);
	CWOBackendReq req(&gUserProfile, "api_ClanGetInfo.aspx");
	req.AddParam("func",       "info");
	req.AddParam("ClanID",     ClanID);
	req.AddParam("GetMembers", out_members ? 1 : 0);
	if(!req.Issue())
	{
		return GetResultCode2("ApiGetClanInfo", req);
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlClan = xmlFile.child("clan_info").child("clan");
	ParseClanInfo(xmlClan, *out_info);
	
	if(out_members)
	{
		pugi::xml_node xmlMember = xmlFile.child("clan_info").child("members").first_child();
	
		out_members->clear();
		while(!xmlMember.empty())
		{
			ClanMember_s member;
			ParseClanMember(xmlMember, member);
			out_members->push_back(member);

			xmlMember = xmlMember.next_sibling();
		}
	}
	
	return 0;
}

int CUserClans::ApiClanGetLeaderboard()
{
	CWOBackendReq req(&gUserProfile, "api_ClanGetInfo.aspx");
	req.AddParam("func", "lb");
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanGetLeaderboard", req);
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);

	leaderboard_.clear();
	
	pugi::xml_node xmlClan = xmlFile.child("clans").first_child();
	while(!xmlClan.empty())
	{
		ClanInfo_s clan;
		ParseClanInfo(xmlClan, clan);
		leaderboard_.push_back(clan);

		xmlClan = xmlClan.next_sibling();
	}

	return 0;
}

int CUserClans::ApiClanLeave()
{
	r3d_assert(gUserProfile.ProfileData.ClanID);
	
	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func", "leave");
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanLeave", req);
	}
	
	// reset
	gUserProfile.ProfileData.ClanID = 0;
	clanInfo_ = ClanInfo_s();
	clanMembers_.clear();

	return 0;
}

int CUserClans::ApiClanKick(int MemberID)
{
	r3d_assert(GetMember(MemberID) != NULL);

	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func", "kick");
	req.AddParam("MemberID", MemberID);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanKick", req);
	}
	
	RemoveMember(MemberID);
	return 0;
}

int CUserClans::ApiClanSetRank(int MemberID, int Rank)
{
	r3d_assert(GetMember(MemberID) != NULL);

	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func",     "setrank");
	req.AddParam("MemberID", MemberID);
	req.AddParam("Rank",     Rank);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanSetRank", req);
	}
	
	ClanMember_s* m = GetMember(MemberID);
	m->ClanRank = Rank;

	return 0;
}

int CUserClans::ApiClanSetLore(const char* lore)
{
	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func", "setlore");
	req.AddParam("Lore", lore);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanSetLore", req);
	}
	
	r3dscpy(clanInfo_.ClanLore, lore);
	return 0;
}

int CUserClans::ApiClanDonateGPToClan(int GP)
{
	r3d_assert(GP > 0);
	r3d_assert(gUserProfile.ProfileData.Stats.GamePoints >= GP);
	
	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func", "gpclan");
	req.AddParam("GP",   GP);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanDonateGPToClan", req);
	}
	
	ClanMember_s* member = GetMember(gUserProfile.CustomerID);
	if(member)
		member->ContributedGP += GP;
	clanInfo_.ClanGP += GP;
	gUserProfile.ProfileData.Stats.GamePoints -= GP;
	return 0;
}

int CUserClans::ApiClanDonateGPToMember(int MemberID, int GP)
{
	r3d_assert(clanInfo_.ClanGP >= GP);

	CWOBackendReq req(&gUserProfile, "api_ClanMgr.aspx");
	req.AddParam("func",     "gpmember");
	req.AddParam("GP",       GP);
	req.AddParam("MemberID", MemberID);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanDonateGPToMember", req);
	}
	
	clanInfo_.ClanGP -= GP;
	return 0;
}

int CUserClans::ApiClanBuyAddMembers(int AddMemberIdx)
{
	r3d_assert(gUserProfile.ProfileData.ClanID > 0);
	r3d_assert(AddMemberIdx >= 0 && AddMemberIdx < 6);
	r3d_assert(gUserProfile.ShopClanAddMembers_GP[AddMemberIdx] > 0);
	r3d_assert(gUserProfile.ShopClanAddMembers_Num[AddMemberIdx] > 0);
	
	// buy "add clan item" for permantnt GP
	int rc = gUserProfile.ApiBuyItem(301144 + AddMemberIdx, 4);
	if(rc != 0) {
		return rc;
	}
	
	clanInfo_.MaxClanMembers += gUserProfile.ShopClanAddMembers_Num[AddMemberIdx];
	gUserProfile.ProfileData.Stats.GamePoints -= gUserProfile.ShopClanAddMembers_GP[AddMemberIdx];
	return 0;
}


int CUserClans::ApiClanSendInvite(const char* gamertag)
{
	CWOBackendReq req(&gUserProfile, "api_ClanInvites.aspx");
	req.AddParam("func",     "send");
	req.AddParam("Gamertag", gamertag);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanSendInvite", req);
	}

	return 0;
}

int CUserClans::ApiClanAnswerInvite(int ClanInviteID, int Accept)
{
	CWOBackendReq req(&gUserProfile, "api_ClanInvites.aspx");
	req.AddParam("func",     "answer");
	req.AddParam("InviteID", ClanInviteID);
	req.AddParam("Answer",   Accept ? 1 : 0);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanAnswerInvite", req);
	}

	int ClanID = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &ClanID);
	if(nargs != 1)
	{
		r3dError("wrong answer for ApiClanAnswerInvite");
		return 9;
	}
	
	if(Accept)
	{
		gUserProfile.ProfileData.ClanID   = ClanID;
		gUserProfile.ProfileData.ClanRank = 99;

		return ApiClanGetInfo(ClanID, &clanInfo_, &clanMembers_);
	}

	return 0;
}

int CUserClans::ApiClanApplyAnswer(int ClanApplID, bool Accept)
{
	r3d_assert(gUserProfile.ProfileData.ClanID);
	
	CWOBackendReq req(&gUserProfile, "api_ClanApply.aspx");
	req.AddParam("func",   "answer");
	req.AddParam("ApplID", ClanApplID);
	req.AddParam("Answer", Accept ? 1 : 0);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanApplyAnswer", req);
	}
	
	//TODO? reread user list

	return 0;
}

int CUserClans::ApiClanApplyToJoin(int ClanID, const char* note)
{
	r3d_assert(gUserProfile.ProfileData.ClanID == 0);
	
	CWOBackendReq req(&gUserProfile, "api_ClanApply.aspx");
	req.AddParam("func",   "apply");
	req.AddParam("ClanID", ClanID);
	req.AddParam("Note",   note);
	if(!req.Issue())
	{
		return GetResultCode2("ApiClanApplyToJoin", req);
	}
	
	return 0;
}

void CUserClans::SetCurrentData(pugi::xml_node& xmlNode)
{
	r3dCSHolder cs1(csClans_);

	pugi::xml_node xmlClans = xmlNode.child("clan");

	ParseClanCurrentData(xmlClans.child("cldata"));
	ParseClanInvites(xmlClans.child("clinvites"));
	ParseClanApplications(xmlClans.child("clapps"));
	gotNewData = true;
}

void CUserClans::ParseClanCurrentData(pugi::xml_node& xmlNode)
{
	clanCurData_.ClanID         = xmlNode.attribute("ID").as_int();
	clanCurData_.ClanRank       = xmlNode.attribute("rank").as_int();
	clanCurData_.NumClanMembers = xmlNode.attribute("cm1").as_int();
	clanCurData_.MaxClanMembers = xmlNode.attribute("cm2").as_int();
}

void CUserClans::ParseClanApplications(pugi::xml_node& xmlNode)
{
	clanApplications_.clear();
	pugi::xml_node xmlApply = xmlNode.first_child();
	while(!xmlApply.empty())
	{
		ClanApplication_s capply;
		capply.ClanApplID = xmlApply.attribute("id").as_int();
		capply.Gamertag = xmlApply.attribute("gt").value();
		capply.Note = xmlApply.attribute("note").value();
		capply.stats.HonorPoints= xmlApply.attribute("xp").as_int();
		capply.stats.Kills= xmlApply.attribute("k").as_int();
		capply.stats.Deaths= xmlApply.attribute("d").as_int();
		capply.stats.Wins= xmlApply.attribute("w").as_int();
		capply.stats.Losses= xmlApply.attribute("l").as_int();
		capply.stats.TimePlayed= xmlApply.attribute("tp").as_int();

		clanApplications_.push_back(capply);

		xmlApply = xmlApply.next_sibling();
	}
}

void CUserClans::ParseClanInvites(pugi::xml_node& xmlNode)
{
	clanInvites_.clear();
	pugi::xml_node xmlInvites = xmlNode.first_child();
	while(!xmlInvites.empty())
	{
		ClanInvite_s cinv;
		cinv.ClanInviteID = xmlInvites.attribute("id").as_int();
		cinv.Gamertag = xmlInvites.attribute("gt").value();
		cinv.ClanName = xmlInvites.attribute("cname").value();
		cinv.ClanLevel = xmlInvites.attribute("cl").as_int();
		cinv.ClanEmblemID = xmlInvites.attribute("cem").as_int();
		cinv.ClanEmblemColor = xmlInvites.attribute("cemc").as_int();
		clanInvites_.push_back(cinv);

		xmlInvites = xmlInvites.next_sibling();
	}
}
