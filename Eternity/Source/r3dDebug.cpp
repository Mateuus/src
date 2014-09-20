#include "r3dPCH.h"
#include "r3d.h"
#include "r3dDebug.h"

#include <windows.h>
#include "dbghelp/include/dbghelp.h"
#ifndef DISABLE_CRASHRPT
#pragma comment(lib, "../External/CrashRpt/Lib/CrashRptLIB.lib")
#include "CrashRpt/include/CrashRpt.h" 
#endif
//#include "errorrep.h"


extern void r3dCloseLogFile();

bool CreateConfigPath(char* dest);
bool CreateWorkPath(char* dest);

static BOOL CALLBACK MyMiniDumpCallback(PVOID                            pParam, 
								 const PMINIDUMP_CALLBACK_INPUT   pInput, 
								 PMINIDUMP_CALLBACK_OUTPUT        pOutput 
								 ) 
{
	BOOL bRet = FALSE; 

	// Check parameters 
	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 

	// Process the callbacks 
	switch( pInput->CallbackType ) 
	{
	case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 
	case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 
	case ModuleCallback: 
		{
			// Does the module have ModuleReferencedByMemory flag set ? 
			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
			{
				// No, it does not - exclude it 
				//r3dOutToLog( "Excluding module: %s \n", pInput->Module.FullPath ); 
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}
			bRet = TRUE; 
		}
		break; 
	case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 
	case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 
	case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 
	case CancelCallback: 
		break; 
	}
	return bRet; 
}

static TCHAR szPath[MAX_PATH+1];

static LONG WINAPI CreateMiniDump( EXCEPTION_POINTERS* pep ) 
{
	r3dOutToLog("Creating minidump!!\n");

	// Open the file 
	char miniDumpPath[1024];
	if(CreateConfigPath(miniDumpPath))
	{
		strcat( miniDumpPath, "MiniDump.dmp" );
		r3dOutToLog("Minidump path: %s\n", miniDumpPath);

		HANDLE hFile = CreateFile( miniDumpPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 
		if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
		{
			// Create the minidump 
			MINIDUMP_EXCEPTION_INFORMATION mdei; 
			mdei.ThreadId           = GetCurrentThreadId(); 
			mdei.ExceptionPointers  = pep; 
			mdei.ClientPointers     = FALSE; 

			MINIDUMP_CALLBACK_INFORMATION mci; 
			mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
			mci.CallbackParam       = 0; 

			MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory); 

			BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
				hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

			if( !rv ) 
				r3dOutToLog( "MiniDumpWriteDump failed. Error: %u \n", GetLastError() ); 
			else 
				r3dOutToLog( "Minidump created.\n" ); 

			// Close the file 
			CloseHandle( hFile ); 
		}
		else 
		{
			r3dOutToLog( "CreateFile failed. Error: %u \n", GetLastError() ); 
		}

		r3dOutToLog("\n!!!Crash!!!\nPlease send '%s' to support@thewarinc.com\nThank you.", miniDumpPath);
		// hide window, hopefully will work in fullscreen
		ShowWindow(win::hWnd, SW_FORCEMINIMIZE);
		
		// show message box to user
		char tempStr[2048];
		sprintf(tempStr, "Application crashed.\nPlease send '%s' and r3dLog.txt (in install folder of the game) to support@thewarinc.com along with description of what you were doing at the time of crash.\nThank you and sorry for inconvenience.", miniDumpPath);
		MessageBox(0, tempStr, "Crash", MB_OK);
	}
	r3dCloseLogFile(); 

	// call WINDOWS ERROR REPORTING, in case if user will not send us crashdump
	LONG lRet = EXCEPTION_CONTINUE_SEARCH;
	lRet = EXCEPTION_EXECUTE_HANDLER;
	return lRet ;
}

// The following function returns TRUE if the correct CrashRpt DLL was loaded,
// otherwise it returns FALSE
/*BOOL CheckCrashRptVersion()
{
	BOOL bExitCode = FALSE;
	TCHAR szModuleName[_MAX_PATH] = _T("");
	HMODULE hModule = NULL;
	DWORD dwBuffSize = 0;
	LPBYTE pBuff = NULL;
	VS_FIXEDFILEINFO* fi = NULL;
	UINT uLen = 0;

	// Get handle to loaded CrashRpt.dll module
	hModule = GetModuleHandle(_T("CrashRpt.dll"));
	if(hModule==NULL)
		goto cleanup; // No "CrashRpt.dll" module loaded

	// Get module file name
	GetModuleFileName(hModule, szModuleName, _MAX_PATH);

	// Get module version 
	dwBuffSize = GetFileVersionInfoSize(szModuleName, 0);
	if(dwBuffSize==0)
		goto cleanup; // Invalid buffer size

	pBuff = (LPBYTE)GlobalAlloc(GPTR, dwBuffSize);
	if(pBuff==NULL)
		goto cleanup;

	if(0==GetFileVersionInfo(szModuleName, 0, dwBuffSize, pBuff))
		goto cleanup; // No version info found

	VerQueryValue(pBuff, _T("\\"), (LPVOID*)&fi, &uLen);

	WORD dwVerMajor = HIWORD(fi->dwProductVersionMS);
	WORD dwVerMinor = LOWORD(fi->dwProductVersionMS);  
	WORD dwVerBuild = LOWORD(fi->dwProductVersionLS);

	DWORD dwModuleVersion = dwVerMajor*1000+dwVerMinor*100+dwVerBuild;

	if(CRASHRPT_VER==dwModuleVersion)
		bExitCode = TRUE; // Version match!

cleanup:

	if(pBuff)
	{
		// Free buffer
		GlobalFree((HGLOBAL)pBuff);
		pBuff = NULL;
	}

	return bExitCode;
}
*/
BOOL CALLBACK r3dCrashRptCallback(__reserved LPVOID lpvState)
{
	r3dCloseLogFile();
	return TRUE;
}

r3dThreadAutoInstallCrashHelper::r3dThreadAutoInstallCrashHelper(DWORD dwFlags)
{
#ifndef DISABLE_CRASHRPT
	m_nInstallStatus = crInstallToCurrentThread2(dwFlags);    
#endif
}

r3dThreadAutoInstallCrashHelper::~r3dThreadAutoInstallCrashHelper()
{
#ifndef DISABLE_CRASHRPT
	crUninstallFromCurrentThread();
#endif
}

static const wchar_t* crashRpgGetLangFile()
{
	static const wchar_t* en = L"crashrpt_lang.ini";
	static const wchar_t* ru = L"crashrpt_lang_ru.ini";

	if(_waccess(ru, 0) != 0)
		return en;

	// russia & ukraine using 1251 codepage
	if(GetACP() == 1251)
		return ru;
		
	return en;
}

void r3dThreadEntryHelper(threadEntry_fn fn, DWORD in)
{
	/*if(!CheckCrashRptVersion())
	{
		// An invalid CrashRpt.dll loaded!
		MessageBox(NULL, "The version of CrashRpt.dll is invalid.", "CRASH RPT ERROR", MB_OK);
		return;
	}*/

	if(!IsDebuggerPresent())
	{
#ifdef DISABLE_CRASHRPT
		SetUnhandledExceptionFilter(CreateMiniDump);
#else
		// detect language file
		wchar_t curDir[MAX_PATH];
		wchar_t langFile[MAX_PATH];
		GetCurrentDirectoryW(sizeof(curDir), curDir);
		swprintf(langFile, MAX_PATH, L"%s\\%s", curDir, crashRpgGetLangFile());

		// use wide versino of structure, as pszLangFilePath *require* full path by some reasons
		CR_INSTALL_INFOW info;
		memset(&info, 0, sizeof(CR_INSTALL_INFOW));  
		info.cb = sizeof(CR_INSTALL_INFOW);
#ifdef FINAL_BUILD
		info.pszAppName = L"WarIncBattlezone";
#else
		info.pszAppName = L"Studio";
#endif
		info.pszAppVersion = L"0.2";
		info.pszEmailTo = NULL;
		
		info.pszUrl = L"https://167.114.32.66/warbrasil/api/php/api_CrashRpt.php";
		
		info.pszCrashSenderPath = NULL;
		info.pfnCrashCallback = &r3dCrashRptCallback;
		info.uPriorities[CR_HTTP] = 1;
		info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY; // skip it
		info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; // skip it
		info.dwFlags |= CR_INST_ALL_EXCEPTION_HANDLERS;
		info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;
		info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
		//we should not restart app, as GNA using command line to pass login info
		//info.dwFlags |= CR_INST_APP_RESTART;
		//info.pszRestartCmdLine   = __r3dCmdLine; 
		info.pszPrivacyPolicyURL = L"https://167.114.32.66/warbrasil/PrivacyPolicy.htm";
		info.pszLangFilePath     = langFile;

		int res = crInstallW(&info);
		if(res !=0)
		{
			// Something goes wrong. Get error message.
			TCHAR szErrorMsg[512] = _T("");        
			crGetLastErrorMsg(szErrorMsg, 512);    
			r3dOutToLog(("%s\n"), szErrorMsg);    
			//return 1;
		}

		// add files to crash report
		char filePath[1024];
		res = crAddFile2(_T("r3dlog.txt"), NULL, _T("Log file"), CR_AF_MAKE_FILE_COPY|CR_AF_MISSING_FILE_OK);
		CreateConfigPath(filePath);
		strcat(filePath, "gameSettings.ini");
		res = crAddFile2(filePath, NULL, _T("Game Settings file"), CR_AF_MAKE_FILE_COPY|CR_AF_MISSING_FILE_OK);
		CreateConfigPath(filePath);
		strcat(filePath, "GPU.txt");
		res = crAddFile2(filePath, NULL, _T("GPU information file"), CR_AF_MAKE_FILE_COPY|CR_AF_MISSING_FILE_OK);
#endif
	}

	//crEmulateCrash(CR_CPP_NEW_OPERATOR_ERROR);
	
	fn(in);

#ifndef DISABLE_CRASHRPT
	if(!IsDebuggerPresent())
	{
		crUninstall();
	}
#endif
}
