#include "r3dPCH.h"
#include "r3d.h"

#include "WOLoginHelper.h"
#include "WOBackendAPI.h"
#include "SteamHelper.h"

void CLoginHelper::DoLogin()
{
	SaveUserName();

	CWOBackendReq req("api_Login.aspx");
	req.AddParam("username", username);
	req.AddParam("password", passwd);

	if(!req.Issue())
	{
		r3dOutToLog("Login FAILED, code: %d\n", req.resultCode_);
		loginAnswerCode = ANS_Error;
		return;
	}

	int n = sscanf(req.bodyStr_, "%d %d %d", 
		&CustomerID, 
		&SessionID,
		&AccountStatus);
	if(n != 3) {
		r3dError("Login: bad answer\n");
		return;
	}

	if(CustomerID == 0)
		loginAnswerCode = ANS_BadPassword;
	else if(AccountStatus >= 200)
		loginAnswerCode = ANS_Frozen;
	else
		loginAnswerCode = ANS_Logged;
		
	return;
}

bool CLoginHelper::CheckSteamLogin()
{
	r3d_assert(gSteam.steamID);
	r3d_assert(gSteam.authToken.getSize() > 0);

	CkString ticket;
	gSteam.authToken.encode("hex", ticket);

	CWOBackendReq req("api_SteamLogin.aspx");
	req.AddParam("ticket", ticket.getAnsi());
	if(!req.Issue()) {
		r3dOutToLog("CheckSteamLogin: failed %d\n", req.resultCode_);
		return false;
	}

	int n = sscanf(req.bodyStr_, "%d %d %d", 
		&CustomerID, 
		&SessionID,
		&AccountStatus);
	if(n != 3) {
		r3dOutToLog("CheckSteamLogin: bad answer %s\n", req.bodyStr_);
		return false;
	}
	
	return true;
}

bool CLoginHelper::CheckG1Login()
{
	r3d_assert(g1AuthToken[0]);

	CWOBackendReq req("api_Gamersfirst.aspx");
	req.AddParam("func",  "login");
	req.AddParam("token", g1AuthToken);
	if(!req.Issue()) {
		r3dOutToLog("CheckG1Login: failed %d\n", req.resultCode_);
		return false;
	}

	int n = sscanf(req.bodyStr_, "%d %d %d %d %d", 
		&CustomerID, 
		&SessionID,
		&AccountStatus,
		&g1AccountId, 
		&g1PayCode);
	if(n != 5) {
		r3dOutToLog("CheckG1Login: bad answer %s\n", req.bodyStr_);
		return false;
	}
	
	// something wrong happens
	if(g1AccountId == 0)
		return false;
		
	// get username from parsestring after :
	const char* delim = strchr(req.bodyStr_, ':');
	if(delim)
		r3dscpy(g1Username, delim + 1);

	return true;
}

void CLoginHelper::SaveUserName()
{
	HKEY hKey;
	int hr;
	hr = RegCreateKeyEx(HKEY_CURRENT_USER, 
		"Software\\Arktos Entertainment Group\\War Inc Battlezone", 
		0, 
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		NULL);
	if(hr == ERROR_SUCCESS)
	{
		DWORD size = strlen(username) + 1;
		
		hr = RegSetValueEx(hKey, "username", NULL, REG_SZ, (BYTE*)username, size);
		RegCloseKey(hKey);
	}
}

bool CLoginHelper::LoadUserName()
{
	// query for game registry node
	HKEY hKey;
	int hr;
	hr = RegOpenKeyEx(HKEY_CURRENT_USER, 
		"Software\\Arktos Entertainment Group\\War Inc Battlezone", 
		0, 
		KEY_ALL_ACCESS, 
		&hKey);
	if(hr != ERROR_SUCCESS)
		return true;

	DWORD size = sizeof(username);
	hr = RegQueryValueEx(hKey, "username", NULL, NULL, (BYTE*)username, &size);
	RegCloseKey(hKey);
	
	return true;
}

void CLoginHelper::CreateAuthToken(char* token) const
{
	char sessionInfo[512];
	sprintf(sessionInfo, "%d:%d:%d", CustomerID, SessionID, AccountStatus);

	for(size_t i=0; i<strlen(sessionInfo); ++i)
		sessionInfo[i] = sessionInfo[i] ^ 0x64;
        
	CkString encoded;
	encoded = sessionInfo;
	encoded.base64Encode("utf-8");
      
	strcpy(token, encoded.getAnsi());
	return;
}

void CLoginHelper::CreateLoginToken(char* token) const
{
	char crlogin[128];
	strcpy(crlogin, username);
	
	CkString s1;
	s1 = username;
	s1.base64Encode("utf-8");

	CkString s2;
	s2 = passwd;
	s2.base64Encode("utf-8");
      
	sprintf(token, "-login \"@%s\" -pwd \"@%s\"", s1.getAnsi(), s2.getAnsi());
	return;
}