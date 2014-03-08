#pragma once

class CLoginHelper
{
  public:
	char		username[128];
	char		passwd[128];
	  
	DWORD		CustomerID;
	DWORD		SessionID;
	DWORD		AccountStatus;
	
	// Gamersfirst data
	char		g1AuthToken[2048];
	int		g1AccountId;
	char		g1Username[256];
	int		g1PayCode;
	
	enum {
	  ANS_Unactive,
	  ANS_Processing,
	  ANS_Timeout,
	  ANS_Error,
	  
	  ANS_Logged,
	  ANS_BadPassword,
	  ANS_Frozen,
	};
	volatile DWORD loginAnswerCode;

  public:
	CLoginHelper()
	{
	  username[0]   = 0;
	  passwd[0]     = 0;
	  CustomerID    = 0;
	  SessionID     = 0;
	  AccountStatus = 0;
	  loginAnswerCode = ANS_Unactive;
	  
	  g1AccountId   = 0;
	  g1PayCode     = 0;
	  g1AuthToken[0]= 0;
	  g1Username[0] = 0;
	  
	  LoadUserName();
	}
	
	void		StartLogin() 
	{
	  r3d_assert(loginAnswerCode != ANS_Processing);
	  r3d_assert(*username);
	  r3d_assert(*passwd);

	  loginAnswerCode = CLoginHelper::ANS_Processing;
	}
	void		DoLogin();
	
	bool		CheckSteamLogin();
	bool		CheckG1Login();

	void		SaveUserName();
	bool		LoadUserName();
	
	void		CreateAuthToken(char* token) const;
	void		CreateLoginToken(char* token) const;
};
