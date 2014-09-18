#include "r3dPCH.h"
#include "r3d.h"
#include <conio.h>

#include "MasterGameServer.h"
#include "MasterUserServer.h"
#include "MasterUdpMonitor.h"

#include "SrvCrashHandler.h"

extern	HANDLE		r3d_CurrentProcess;

void game::Shutdown(void) { r3dError("not a gfx app"); }
void game::MainLoop(void) { r3dError("not a gfx app"); }
void game::Init(void)     { r3dError("not a gfx app"); }
void game::PreInit(void)  { r3dError("not a gfx app"); }

static LRESULT CALLBACK serverWndFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) 
  {
    case WM_CLOSE:
    {
      r3dOutToLog("alt-f4 pressed\n");
      r3dOutToLog("...terminating application\n");

      HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
      break;
    }
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
  switch(dwCtrlType) 
  {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
      r3dOutToLog("Control-c ...\n");
      HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
      break;
  }

  return FALSE;
}

static void serverCreateTempD3DWindow()
{
static	char*		ClassName = "r3dMasterServerWin";
	WNDCLASS    	wndclass;

  wndclass.style         = CS_DBLCLKS | CS_GLOBALCLASS;
  wndclass.lpfnWndProc   = serverWndFunc;		// window function
  wndclass.cbClsExtra    = 0;				// no extra count of bytes
  wndclass.cbWndExtra    = 0;				// no extra count of bytes
  wndclass.hInstance     = GetModuleHandle(NULL);	// this instance
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = ClassName;
  RegisterClass(&wndclass);

  win::hWnd = CreateWindowEx(
    0,
    ClassName,             			// window class name
    "temp d3d window",				// window caption
    WS_OVERLAPPEDWINDOW, 			// window style
    0,						// initial x position
    0,						// initial y position
    16,						// initial x size
    16,						// initial y size
    NULL,                  			// parent window handle
    NULL,                  			// window menu handle
    GetModuleHandle(NULL), 			// program instance handle
    NULL);                 			// creation parameters

  if(!win::hWnd) {
    r3dError("unable to create window");
    return;
  }

  ShowWindow(win::hWnd, FALSE);
  return;
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
    10,
    curDispHeight - (rect.bottom - rect.top) - 60,
    rect.right,
    rect.bottom,
    SWP_NOSIZE);
}

//
// server entry
//
int main(int argc, char* argv[])
{
  extern int _r3d_bLogToConsole;
  _r3d_bLogToConsole = 1;
  
  extern int _r3d_bSilent_r3dError;
  _r3d_bSilent_r3dError = 1;
  
  extern int _r3d_Network_DoLog;
  _r3d_Network_DoLog = 0;

  r3d_CurrentProcess = GetCurrentProcess();
  SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
  
  win::hInstance = GetModuleHandle(NULL);

  // change log file now and install handler, so we'll record r3dErrors
  {  
    _mkdir("logms");
    time_t t1;
    time(&t1);
    char fname[MAX_PATH];
    sprintf(fname, "logms\\MS_%x.txt", (DWORD)t1);
    extern void r3dChangeLogFile(const char* fname);
    r3dChangeLogFile(fname);

    sprintf(fname, "logms\\MS_%x.dmp", (DWORD)t1);
    SrvSetCrashHandler(fname);
  }
  
  try
  {
    serverCreateTempD3DWindow();
    ClipCursor(NULL);
    
    moveWindowToCorner();

    gServerConfig = new CMasterServerConfig();

    // from SF config.cpp, bah.
    extern void RegisterAllVars();
    RegisterAllVars();
    r3dOutToLog("API: %s\n", g_api_ip->GetString());
    
    gMasterGameServer.Start(gServerConfig->masterPort_, gServerConfig->serverId_);
    gMasterUserServer.Start(gServerConfig->clientPort_, gServerConfig->masterCCU_);
    gUdpMonitor.Start(SBNET_MASTER_WATCH_PORT);

    r3dStartFrame();
    while(1) 
    {
      r3dEndFrame();
      r3dStartFrame();
      
      Sleep(1);

      gMasterGameServer.Tick();
      gMasterUserServer.Tick();
      gUdpMonitor.Tick();


      if(_kbhit()) {
        // weird way to check for F4 (two symbols - 0, 62)
        if(_getch() == 0 && _kbhit()) {
          if(_getch() == 62) {
            gMasterGameServer.RequestShutdown();
          }
        }
      }

      // gracefully shutdown if requested
      if(gMasterGameServer.shuttingDown_ && gMasterGameServer.shutdownLeft_ < 0) {
        r3dOutToLog("Shutting down...\n");
        break;
      }

    }
  } 
  catch(const char* what)
  {
    r3dOutToLog("!!! Exception: %s\n", what);
    what = what;
    HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
  }
  
  gMasterUserServer.Stop();
  gMasterGameServer.Stop();
  gUdpMonitor.Stop();
  
  DestroyWindow(win::hWnd);
  HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);

  return 0;
}
