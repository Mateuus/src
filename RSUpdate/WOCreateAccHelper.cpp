#include "r3dPCH.h"
#include "r3d.h"

#include "WOCreateAccHelper.h"
#include "WOBackendAPI.h"
#include "SteamHelper.h"

int CCreateAccHelper::DoCreateSteamAcc()
{
	r3d_assert(AccReferrer == REF_Steam);
	r3d_assert(createAccCode = CA_Processing);
	r3d_assert(gSteam.steamID);
	
	char strSteamId[64];
	sprintf(strSteamId, "%I64u", gSteam.steamID);
	
	CWOBackendReq req("api_SteamCreateAcc.aspx");
	req.AddParam("username", username);
	req.AddParam("password", passwd1);
	req.AddParam("email",    email);
	req.AddParam("steamId",  strSteamId);

	// note, createAccCode will be cleared in CUpdater::CreateAccThreadEntry
	req.Issue();
	return req.resultCode_;
}

int CCreateAccHelper::DoCreateG1Acc()
{
	r3d_assert(AccReferrer == REF_G1);
	r3d_assert(createAccCode = CA_Processing);
	r3d_assert(g1AccountId);
	
	CWOBackendReq req("api_Gamersfirst.aspx");
	req.AddParam("func",      "create");
	req.AddParam("username",  username);
	req.AddParam("password",  passwd1);
	req.AddParam("email",     email);
	req.AddParam("g1Id",      g1AccountId);
	req.AddParam("g1PayCode", g1PayCode);

	// note, createAccCode will be cleared in CUpdater::CreateAccThreadEntry
	req.Issue();
	return req.resultCode_;
}
