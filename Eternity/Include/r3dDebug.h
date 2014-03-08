#pragma once
/*
DWORD		r3dCountMemUsage(); // return active heap of the process
void		r3dCreateCrashDump();
void		r3dDebugDumpCallStack(bool selectMainThread = false);
void		r3dDebugRemoveAllDumps();
const char*	r3dDebugGetComputerName();*/

typedef	void	(*threadEntry_fn)(DWORD data);
void		r3dThreadEntryHelper(threadEntry_fn fn, DWORD in);

class r3dThreadAutoInstallCrashHelper
{
public:
	r3dThreadAutoInstallCrashHelper(DWORD dwFlags=0);
	~r3dThreadAutoInstallCrashHelper();
	int m_nInstallStatus;
};

