#include "r3dPCH.h"
#include "r3d.h"
#include "r3dDebug.h"

#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

extern void r3dCloseLogFile();

static	char	srv_dump_file[MAX_PATH] = "";

static BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, 
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

static LONG WINAPI CreateMiniDump(EXCEPTION_POINTERS* pep)
{
	r3dOutToLog("!!! crashed\n");
	r3dOutToLog("Creating minidump at %s\n", srv_dump_file);

	// Open the file 
		HANDLE hFile = CreateFile( srv_dump_file, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 
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

	r3dCloseLogFile(); 

	// stop search, just quit.
	LONG lRet = EXCEPTION_EXECUTE_HANDLER;
	return lRet ;
}

void SrvSetCrashHandler(const char* dumpPath)
{
	r3dscpy(srv_dump_file, dumpPath);
	SetUnhandledExceptionFilter(CreateMiniDump);
}
