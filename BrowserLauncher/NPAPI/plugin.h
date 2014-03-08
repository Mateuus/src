
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <windows.h>
#include "npapi.h"
#include "npruntime.h"
#include "../logic.h"

class CPlugin
{
private:
	std::wstring cmdLine;

public:
	LauncherLogic logic;
	HANDLE hNewTimer;
  HWND m_hWnd; 
  CPlugin(NPP pNPInstance, const wchar_t* cmdLine);
	void SetWindow(HWND hw);
  ~CPlugin();
};

#endif // __PLUGIN_H__
