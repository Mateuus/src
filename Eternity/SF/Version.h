#pragma once


#ifdef FINAL_BUILD
	#define PROJECT_CONFIG_NAME		"final"
#else
	#ifdef _DEBUG
		#define PROJECT_CONFIG_NAME		"debug"
	#else 
		#define PROJECT_CONFIG_NAME		"release"
	#endif
#endif

extern const char * g_szApplicationName;

extern int32_t	g_nProjectVersionMajor;
extern int32_t	g_nProjectVersionMinor;

//--------------------------------------------------------------------------------------------------------
#define PROJECT_NAME					"%s v%d.%d  (build: %s %s) - %s"


//--------------------------------------------------------------------------------------------------------
__forceinline const char * GetBuildVersionString()
{
	return Va( PROJECT_NAME, g_szApplicationName, g_nProjectVersionMajor, g_nProjectVersionMinor, __DATE__, __TIME__, PROJECT_CONFIG_NAME );
}
