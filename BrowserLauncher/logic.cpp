#include "logic.h"

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

//#define DEBUG_MESSAGES

LauncherLogic::LauncherLogic()
: errorMsg(0)
, isInstalled(false)
{
}


LauncherLogic::~LauncherLogic()
{
}

bool alreadyLaunched(const wchar_t* moduleName)
{
	DWORD pProcessIds[8192];
	DWORD cb;
	BOOL res = EnumProcesses(pProcessIds, sizeof(pProcessIds), &cb);
	if(!res)	return false;

	for(DWORD i=0; i<cb/sizeof(pProcessIds[0]); ++i)
	{
		wchar_t szProcessName[MAX_PATH];
		HANDLE hl = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, pProcessIds[i]);
		DWORD r = GetProcessImageFileName(hl, szProcessName, MAX_PATH);
		CloseHandle(hl);

		if(r>0 && wcsstr(szProcessName, moduleName))
			return true;
	}
	return false;
}

static bool updaterIsRunning()
{
	// firstly, check on event name
	static const char* updaterName = "Global\\WarInc_Updater_001";
  
	HANDLE h;
	if((h = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, updaterName)) != NULL) {
		OutputDebugStringA("Updater is already running");
		SetEvent(h);
		CloseHandle(h);
		return true;
	}
  
	// then check by process name
	return alreadyLaunched(L"RSUpdate");
}

bool LauncherLogic::login(HINSTANCE hInst, const wchar_t* _cmdLine)
{
	cmdLine = _cmdLine;
	hInstance = hInst;
	if(alreadyLaunched(L"WIInst"))
	{
		setNotification(L"installer already launched");
		return false;
	}

	if(!fillInstallPath())	
		return true;

	if(isInstalled)
	{
		launchGame();
		return true;
	}

	//Create temporary path
	wchar_t lpTempPathBuffer[MAX_PATH];
	DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		setError(L"GetTempPath failed");
		return false;
	}

	//Generates a temporary file name. 
	tempFileName << lpTempPathBuffer << L"\\WIInst" << GetTickCount() << L".exe";

	//Creates the new file to write
	HANDLE hTempFile = CreateFile(tempFileName.str().c_str(), GENERIC_WRITE, 0, NULL,                 // default security 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hTempFile == INVALID_HANDLE_VALUE) 
	{ 
		retss << L"Can't create temp-file " << tempFileName.str();
		setError();
		return false;
	}

	//write resource into the file

	HRSRC hRes = FindResource(hInstance, L"WIInstaller", RT_RCDATA);
	if (hRes == NULL)
	{
		setError(L"Can't locate resource in module");
		return false;
	}

	HGLOBAL hResLoad = LoadResource(hInstance, hRes);
	if (hResLoad == NULL)
	{
		setError(L"Can't load located resource");
		return false;
	}

	DWORD szRes = SizeofResource(hInstance, hRes);
	if(szRes==0)
	{
		setError(L"Error size of loaded resource");
		return false;
	}

	LPVOID ptr = LockResource(hResLoad);
	if(!ptr)
	{
		setError(L"Can't lock loaded resource");
		return false;
	}

	DWORD dwWritten;
	BOOL wr = WriteFile(hTempFile, ptr, szRes, &dwWritten, NULL);
	if(!wr || dwWritten != szRes)
	{
		if (!CloseHandle(hTempFile))
		{
			setError(L"WriteFile and CloseHandle error");
			return false;
		}
		setError(L"WriteFile error");
		return false;
	}

	if (!CloseHandle(hTempFile))
	{
		setError(L"CloseHandle hTempFile error");
		return false;
	}

	// launch installer. note that we can't use CreateProcess because of UAC elevation problem
	int err = (int)ShellExecuteW(NULL, L"open", tempFileName.str().c_str(), L"", NULL, SW_SHOW);
	if(err < 32) {
		retss << L"Can't launch " << tempFileName.str();
		setError();
		return false;
	}

	return false;
}

bool LauncherLogic::tick()
{
	//Try to delete running installer
	if( !DeleteFile(tempFileName.str().c_str()) )
		return true;

	if(!fillInstallPath())
		return false;
		
	//launch game
	if(isInstalled)
	{
		if(updaterIsRunning())
		{
			retss << InstallPath << L"\\RSUpdate.exe already launched";
			setNotification();
			return false;
		}
		launchGame();
		return false;
	}

	//keep ticking
	return true;
}



const wchar_t* LauncherLogic::getLastError() const
{
	return errorMsg;
}






void LauncherLogic::setError(const wchar_t* err)
{
	if(err)	retss << err;

	DWORD e = GetLastError();
	if(e)	retss << ", Error code: " << e;

	msg = retss.str();
	errorMsg = msg.c_str();
	retss.str(L"");
}


void LauncherLogic::setNotification(const wchar_t* err)
{
	if(err)	retss << err;
	msg = retss.str();
	retss.str(L"");

	OutputDebugStringW(L"LauncherLogic::setNotification");
	OutputDebugStringW(msg.c_str());

#ifdef DEBUG_MESSAGES
	errorMsg = msg.c_str();
#endif
}



bool LauncherLogic::launchGame()
{
	std::wstringstream exePath;
	exePath << InstallPath << L"\\RSUpdate.exe ";

	if(updaterIsRunning())
	{
		retss << exePath.str() << L" already launched";
		setNotification();
		return false;
	}

	std::wstringstream exdbg;
	exdbg << exePath.str() << cmdLine << std::endl;
	OutputDebugStringW(exdbg.str().c_str());

	//MessageBox(0, cmdLine.c_str(), L"", MB_OK);
	//launch updater, using ShellExecute because of UAC elevate problem with CreateProcess on vista+
	int err = (int)ShellExecuteW(NULL, L"open", exePath.str().c_str(), cmdLine.c_str(), InstallPath, SW_SHOW);
	if(err < 32) {
		retss << L"Can't launch " << exePath.str();
		setError();
		return false;
	}

	retss << L"Launch " << exePath.str();
	setNotification();
	return true;
}

bool LauncherLogic::fillInstallPath()
{
	//check installed and get installation path
	isInstalled = false;

	HKEY rKey;
	DWORD dwRet;

	// old installer path
	dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\War Inc Battlezone", 0, KEY_QUERY_VALUE, &rKey);
	if(dwRet==ERROR_SUCCESS)
	{
		DWORD szInstallPath = sizeof(InstallPath);
		dwRet = RegQueryValueEx(rKey, L"InstallLocation", NULL, NULL, (unsigned char*)InstallPath, &szInstallPath);
		RegCloseKey(rKey);
		if(dwRet==ERROR_SUCCESS)
		{
			isInstalled = true;
			return true;
		}
	}

	// new installer path
	dwRet = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Arktos Entertainment Group\\War Inc Battlezone", 0, KEY_QUERY_VALUE, &rKey);
	if(dwRet==ERROR_SUCCESS)
	{
		DWORD szInstallPath = sizeof(InstallPath);
		dwRet = RegQueryValueEx(rKey, L"InstallPath", NULL, NULL, (unsigned char*)InstallPath, &szInstallPath);
		RegCloseKey(rKey);
		if(dwRet==ERROR_SUCCESS)
		{
			isInstalled = true;
			return true;
		}
		setError(L"RegQueryValueEx error");
		return false;
	}

	return true;
}
