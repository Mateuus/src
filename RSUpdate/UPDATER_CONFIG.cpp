#include "r3dPCH.h"
#include "r3d.h"

bool	UPDATER_UPDATER_ENABLED  = 1;
char	UPDATER_VERSION[512]     = "1.3.6";
char	UPDATER_VERSION_SUFFIX[512] = "";
char	UPDATER_BUILD[512]	 = __DATE__ " " __TIME__;

char	BASE_RESOURSE_NAME[512]  = "WO";
char	GAME_EXE_NAME[512]       = "WarBrasil.exe";
char	GAME_TITLE[512]          = "War Brasil";

// updater (xml and exe) and game info on our server.
char	UPDATE_DATA_URL[512]     = "https://198.50.173.40/wo/wo.xml";	// url for data update
char	UPDATE_UPDATER_URL[512]  = "https://198.50.173.40/wo/updater/woupd.xml";

// HIGHWIND CDN
char	UPDATE_UPDATER_HOST[512] = "http://198.50.173.40/wo/updater/";
// PANDO CDN
//char	UPDATE_UPDATER_HOST[512] = "http://arktos.pandonetworks.com/Arktos/wo/updater/";

// LOCAL TESTING
//http://arktos.pandonetworks.com/Arktos
//char	UPDATE_DATA_URL[512]     = "http://localhost/wo/wo.xml";	// url for data update
//char	UPDATE_UPDATER_HOST[512] = "http://localhost/wo/updater/";	// url for updater .xml


char	EULA_URL[512]            = "https://198.50.173.40/TOS.rtf";
char	GETSERVERINFO_URL[512]   = "https://198.50.173.40/api_getserverinfo3.xml";

bool	UPDATER_STEAM_ENABLED	 = true;
