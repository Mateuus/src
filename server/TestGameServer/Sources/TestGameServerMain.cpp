#include "r3dPCH.h"
#include "r3d.h"

#include "TestGameServer.h"

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

static void serverCreateTempD3DWindow()
{
static	char*		ClassName = "r3dGameServerWin";
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

//
// server entry
//
int main(int argc, char* argv[])
{
  extern int _r3d_bLogToConsole;
  _r3d_bLogToConsole = 1;
  
  win::hInstance = GetModuleHandle(NULL);
  
  try
  {
    serverCreateTempD3DWindow();
    ClipCursor(NULL);
    
    gGameServer.InitFromArgs(argc, argv);
    gGameServer.Start();

    while(1) 
    {
      MSG msg;
      while(PeekMessage(&msg, NULL,0,0,PM_NOREMOVE)) {
        if(!GetMessage (&msg, NULL, 0, 0)) 
          return 1;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      
      gGameServer.Tick();
      Sleep(1);
    }
  } 

  catch(const char* what)
  {
    //r3dOutToLog("!!! Exception: %s\n", what);
    what = what;
    HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
  }
  
  gGameServer.Stop();
  
  DestroyWindow(win::hWnd);
  HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);

  return 0;
}
