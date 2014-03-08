#include "r3dPCH.h"
#include "r3d.h"

bool	UPDATER_UPDATER_ENABLED  = 1;
char	UPDATER_VERSION[512]     = "1.3.6";
char	UPDATER_VERSION_SUFFIX[512] = "";
char	UPDATER_BUILD[512]	 = __DATE__ " " __TIME__;

char	BASE_RESOURSE_NAME[512]  = "WO";
char	GAME_EXE_NAME[512]       = "WarInc.exe";
char	GAME_TITLE[512]          = "War Inc. Battlezone";

// updater (xml and exe) and game info on our server.
char	UPDATE_DATA_URL[512]     = "http://update.warinc.ph/wo/wo.xml";	// url for data update
char	UPDATE_UPDATER_URL[512]  = "http://update.warinc.ph/wo/updater/woupd.xml";
char	UPDATE_UPDATER_HOST[512] = "http://update.warinc.ph/wo/updater/";

char	EULA_URL[512]            = "https://api1.thewarinc.com/TOS.rtf";
char	GETSERVERINFO_URL[512]   = "http://update.warinc.ph/api_getserverinfo3.xml";

bool	UPDATER_STEAM_ENABLED	 = false;
