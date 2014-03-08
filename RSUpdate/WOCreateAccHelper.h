#pragma once

class CCreateAccHelper
{
  public:
	enum {
	  CA_Unactive,
	  CA_Processing,
	};
	volatile DWORD	createAccCode;
	
	enum {
	  REF_None,
	  REF_Steam,
	  REF_G1,
	};
	int		AccReferrer;

	char		username[128];
	char		passwd1[128];
	char		passwd2[128];
	char		email[128];
	
	int		g1PayCode;
	int		g1AccountId;
	
  public:
	CCreateAccHelper()
	{
	  AccReferrer   = REF_None;
	  username[0]   = 0;
	  passwd1[0]    = 0;
	  passwd2[0]    = 0;
	  createAccCode = CA_Unactive;
	  g1PayCode     = 0;
	  g1AccountId   = 0;
	}
	
	void		StartCreateAcc()
	{
	  r3d_assert(AccReferrer != REF_None);
	  r3d_assert(createAccCode != CA_Processing);
	  r3d_assert(*username);
	  r3d_assert(*passwd1);
	  r3d_assert(*passwd2);
	  r3d_assert(*email);

	  createAccCode = CA_Processing;
	}
	int		DoCreateSteamAcc();
	int		DoCreateG1Acc();
};
