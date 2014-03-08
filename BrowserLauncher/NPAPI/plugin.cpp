#include <windows.h>
#include <stdio.h>
#include <windowsx.h>

#include "plugin.h"
#include "npfunctions.h"
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#include <sstream>


CPlugin::CPlugin(NPP pNPInstance, const wchar_t* _cmdLine)
: m_hWnd(0)
, cmdLine(_cmdLine)
{
}


VOID CALLBACK Callback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	CPlugin* p = (CPlugin*)lpParameter;

	bool r = p->logic.tick();

	const wchar_t* msg = p->logic.getLastError();
	if(msg)	MessageBox(p->m_hWnd, msg, L"Error", MB_OK);

	if(r && !msg)	return;

	BOOL rd = DeleteTimerQueueTimer(NULL, p->hNewTimer, INVALID_HANDLE_VALUE);
	p->hNewTimer = 0;
}

void CPlugin::SetWindow(HWND hw)
{
	//pass only one set window
	if(m_hWnd)	return;

	m_hWnd = hw;

	bool s = logic.login(GetModuleHandle(L"npwiu.dll"), cmdLine.c_str());
	const wchar_t* msg = logic.getLastError();
	if(msg)	MessageBox(hw, msg, L"Error", MB_OK);

	if(!s)
	{
		BOOL r = CreateTimerQueueTimer(&hNewTimer, NULL, Callback, this, 1000, 1000, 0);
		if(!r)	MessageBox(hw, L"Can't CreateTimerQueueTimer", L"Error", MB_OK);
	}
}

CPlugin::~CPlugin()
{
	if(hNewTimer)
	{
		BOOL r = DeleteTimerQueueTimer(NULL, hNewTimer, INVALID_HANDLE_VALUE);
	}
}

