#include "stdafx.h"
#include "Async_h.h"
#include "ATLAsync.h"


LRESULT CWarIncLaunch::OnCreate(UINT, WPARAM , LPARAM , BOOL& )
{
	m_uTimerID = 0;

	return 0;
}

LRESULT CWarIncLaunch::OnTimer(UINT, WPARAM, LPARAM, BOOL&)
{
	// Eat spurious WM_TIMER messages
	MSG wmsg;
	while(::PeekMessage(&wmsg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
	
	// there might be queued timer messages
	if(m_uTimerID == 0)
		return 0;

	bool r = logic.tick();

	const wchar_t* msg = logic.getLastError();
	if(msg)	MessageBox(msg, MB_OK);

	if(r && !msg)	
		return 0;

	KillTimer(m_uTimerID);
	m_uTimerID = 0;

	return 0;
}




LRESULT CWarIncLaunch::OnDestroy(UINT, WPARAM , LPARAM , BOOL& )
{
	if(m_uTimerID)	KillTimer(m_uTimerID);
	return 0;
}

HRESULT CWarIncLaunch::OnDraw(ATL_DRAWINFO&)
{
	return S_OK;
}



LRESULT CWarIncLaunch::OnLButtonDown(UINT, WPARAM , LPARAM , BOOL& )
{
	return 0;
}


STDMETHODIMP CWarIncLaunch::put_cmdLine(BSTR cmdLine)
{
	m_cmdLine = cmdLine;

	if(wcslen(m_cmdLine)>0)
	{
		extern HINSTANCE g_hInstance;
		bool s = logic.login(g_hInstance, m_cmdLine);
		const wchar_t* msg = logic.getLastError();
		if(msg)	MessageBox(msg, MB_OK);

		if(!s)	m_uTimerID = (UINT)(SetTimer(0x451,330));
	}

	return S_OK;
}
