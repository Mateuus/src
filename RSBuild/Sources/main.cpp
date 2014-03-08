#include "r3dPCH.h"
#include "r3d.h"

#include "GameLevel.h"
#include "main.h"
#include "r3dFSBuilder.h"
#include "BuilderConfig.h"

extern	HANDLE		r3d_CurrentProcess;
extern	r3dFSBuilder	builder("");

class r3dSun * Sun;
r3dCamera gCam;
r3dScreenBuffer * DepthBuffer;
r3dLightSystem WorldLightSystem;

void r3dScaleformBeginFrame() {}
void r3dScaleformEndFrame() {}
unsigned long DriverUpdater(struct HWND__ *,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long) {return 0; }
void SetNewSimpleFogParams() {};
void SetVolumeFogParams() {};

void CreateArchive()
{
  BuilderConfig config(builder);
  config.ParseConfig("build\\RSBuild.xml");

  if(!builder.CheckTextures()) {
    r3dOutToLog("There is bad .dds files, exiting\n");
    return;
  }
  
  builder.ReconvertAllSCO();
  builder.BuildFileList();
  builder.CreateArchive("wo");
}

void TestArchive()
{
  r3dFileSystem fs;
  fs.OpenArchive("build\\lzo\\wo");
  r3dOutToLog("bobobo\n");
  
  float t1 = r3dGetTime();
  for(size_t i=0; i<fs.fl_.files_.size(); i++)
  {
    fs.ExtractFile(fs.fl_.files_[i], "build\\1\\");
  }
  r3dOutToLog("Extracted: %f\n", r3dGetTime() - t1);
  
  return;
}

void DoSomeWork()
{
  CreateArchive();
  //TestArchive();
}


void game::Shutdown(void) { r3dError("not a gfx app"); }
void game::MainLoop(void) { r3dError("not a gfx app"); }
void game::Init(void)     { r3dError("not a gfx app"); }
void game::PreInit(void)  { r3dError("not a gfx app"); }

/*
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
static	char*		ClassName = "RSBuildWin";
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
*/

static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
  switch(dwCtrlType) 
  {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
      r3dOutToLog("Control-c ...\n");
      builder.DeleteVolumes();
      //HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);
      break;
  }

  return FALSE;
}

void tempDoMsgLoop()
{
  MSG msg;
  while(PeekMessage(&msg, NULL,0,0,PM_NOREMOVE)) {
    if(!GetMessage (&msg, NULL, 0, 0)) 
      return;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return;
}

static void ParseArgs(int argc, char* argv[])
{
  r3dOutToLog("cmd: %d\n", argc);
  for(int i=1; i<argc; i++) r3dOutToLog("%d: %s\n", i, argv[i]);

  return;
}

//
// server entry
//
int main(int argc, char* argv[])
{
  extern int _r3d_bLogToConsole;
  _r3d_bLogToConsole = 1;
  
  // do not close on debug terminate key
  extern int _r3d_bTerminateOnZ;
  _r3d_bTerminateOnZ = 0;
  
  extern int _r3d_MatLib_SkipAllMaterials;
  _r3d_MatLib_SkipAllMaterials = 1;
  r3dMaterialLibrary::Reset();
  
  /*
  // skip filling mesh buffers
  extern int r3dMeshObject_SkipFillBuffers;
  r3dMeshObject_SkipFillBuffers = 1;
  
  // do not use textures
  extern int r3dTexture_UseEmpty;
  r3dTexture_UseEmpty = 1;
  */
  
  r3d_CurrentProcess = GetCurrentProcess();
  SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
  
  win::hInstance = GetModuleHandle(NULL);

  RegisterAllVars();
  
  try
  {
    ParseArgs(argc, argv);

/*
    serverCreateTempD3DWindow();
    r3dRenderer = new r3dRenderLayer;
    r3dRenderer->Init(win::hWnd, NULL);
    r3dRenderer->SetMode(32, 32, 32, R3DSetMode_Windowed, 0);
*/    

    DoSomeWork();
    
    ClipCursor(NULL);
  } 
  catch(const char* what)
  {
    r3dOutToLog("!!! Exception: %s\n", what);
    // fall thru
  }

  UnregisterAllVars();
  
  DestroyWindow(win::hWnd);
  HRESULT res = TerminateProcess(r3d_CurrentProcess, 0);

  return 0;
}
