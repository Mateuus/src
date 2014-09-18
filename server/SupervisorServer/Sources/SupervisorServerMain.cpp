#include "r3dPCH.h"
#include "r3d.h"
#include <conio.h>

#include "SupervisorConfig.h"
#include "SupervisorServer.h"
#include "LogUploader.h"
#include "../../MasterServer/Sources/MasterUdpMonitor.h"

#include "../../EclipseStudio/Sources/GameLevel.h"
#include "../../EclipseStudio/Sources/GameLevel.cpp"

extern	HANDLE		r3d_CurrentProcess;

void game::Shutdown(void) { r3dError("not a gfx app"); }
void game::MainLoop(void) { r3dError("not a gfx app"); }
void game::Init(void)     { r3dError("not a gfx app"); }
void game::PreInit(void)  { r3dError("not a gfx app"); }

// Eternity compile fix
DWORD DriverUpdater(HWND hParentWnd, DWORD VendorId, DWORD v1, DWORD v2, DWORD v3, DWORD v4, DWORD hash) { return 0; }
void r3dScaleformBeginFrame() {}
void r3dScaleformEndFrame() {}
void SetNewSimpleFogParams() {}
void SetAdvancedFogParams() {}
void SetVolumeFogParams() {}
r3dCamera gCam ;
class r3dSun * Sun;
r3dScreenBuffer * DepthBuffer;
r3dLightSystem WorldLightSystem;

static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
  switch(dwCtrlType) 
  {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
      r3dOutToLog("Control-c ...\n");

      // do not do any network deinitialization here
      // because this handler is invoked from other thread
      
      gSupervisorServer.TerminateAllGames();
      HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
      break;
  }

  return FALSE;
}

// move console window to specified corner
static void moveWindowToCorner()
{
  HDC disp_dc  = CreateIC("DISPLAY", NULL, NULL, NULL);
  int curDispWidth  = GetDeviceCaps(disp_dc, HORZRES);
  int curDispHeight = GetDeviceCaps(disp_dc, VERTRES);
  DeleteDC(disp_dc);

  HWND cwhdn = GetConsoleWindow();  

  RECT rect;
  GetWindowRect(cwhdn, &rect);
  
  SetWindowPos(cwhdn, 
    0, 
    curDispWidth - (rect.right - rect.left) - 10, 
    curDispHeight - (rect.bottom - rect.top) - 60,
    rect.right,
    rect.bottom,
    SWP_NOSIZE);
}

static void supervisorServerLoop()
{
  r3dResetFrameTime();

  if(!gSupervisorServer.Start())
    return;
    
  gUdpMonitor.Start(SBNET_SUPER_WATCH_PORT);
  
  // start log uploader
  if(gSupervisorConfig->uploadLogs_) {
    gLogUploader.Start();
  } else {
    r3dOutToLog("LogUploader is disabled\n");
  }
  

  r3dResetFrameTime();
  while(1) 
  {
    r3dEndFrame();
    r3dStartFrame();

    Sleep(1);

    gSupervisorServer.Tick();
    gUdpMonitor.Tick();

    if(gSupervisorServer.IsMasterDisconnected()) {
      r3dOutToLog("Master Server disconnected, exiting\n");
      break;
    }
  }

  gUdpMonitor.Stop();
  
  return;
}

volatile float sum = 0;
void DoWork()
{
	int cnt = (int)u_GetRandom(1000000, 10000000);
	for(int i=0; i<cnt; i++) {
	  sum += sinf(u_GetRandom());
	}
}

void TestTimer()
{
	float   b1 = r3dGetTime();
	DWORD   b2 = GetTickCount();

	float   t1 = r3dGetTime();
	DWORD   t2 = GetTickCount();
	while(1)
	{
		::Sleep(1); //(DWORD)u_GetRandom(10, 500));
		DoWork();

		float c1 = r3dGetTime();
		DWORD c2 = GetTickCount();

		// base delta
		float d1 = (c1 - b1) - (float)(c2 - b2)/1000.0f;
		// tick delta
		float d2 = (c1 - t1) - (float)(c2 - t2)/1000.0f;

		t1 = c1;
		t2 = c2;
		
		LARGE_INTEGER qwTicksPerSec;
		QueryPerformanceFrequency(&qwTicksPerSec );
		r3dOutToLog("delta: %f, %f, %I64d\n", d1, d2, qwTicksPerSec );
	}
}


//
// server entry
//
int main(int argc, char* argv[])
{
  extern int _r3d_bLogToConsole;
  extern int _r3d_bLogToDebug;
  _r3d_bLogToConsole = 1;
  _r3d_bLogToDebug   = 0;
  
  r3d_CurrentProcess = GetCurrentProcess();
  win::hInstance = GetModuleHandle(NULL);
  SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
  
  try
  {
    if(argc > 1 && stricmp(argv[1], "-timer") == 0)
    {
	TestTimer();
    }
  
    moveWindowToCorner();

    // from SF config.cpp, bah.
    extern void RegisterAllVars();
    RegisterAllVars();
    r3dOutToLog("API: %s\n", g_api_ip->GetString());

    // sleep 2 sec, waiting for master to start
    if(stricmp(g_api_ip->GetString(), "localhost") == 0)
	::Sleep(2000);
    
    gSupervisorConfig = new CSupervisorConfig();
    
    supervisorServerLoop();
  } 
  catch(const char* what)
  {
    r3dOutToLog("!!! Exception: %s\n", what);
    gSupervisorServer.Stop();

    //TODO: email exception to devteam
    //NOTE: supervisor will not restart itself for now.
    _getch();
    HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
  }
  
  gSupervisorServer.Stop();
  gSupervisorServer.TerminateAllGames();
  
  gLogUploader.Stop(true);
  
  DestroyWindow(win::hWnd);
  ExitProcess(0);

  return 0;
}
