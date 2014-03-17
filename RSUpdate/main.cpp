#include "r3dPCH.h"
#include "r3d.h"
#include "shellapi.h"
#include "shobjidl.h"

#ifndef __ITaskbarList3_FWD_DEFINED__
  #include "ITaskbarList3.h"
#endif
const IID r3dIID_ITaskbarList3 = { 0xEA1AFB91, 0x9E28, 0x4B86, { 0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF } };

#include "main.h"
#include "resource.h"

#include "UPDATER_CONFIG.h"
#include "Updater.h"
#include "GenerateUpdaterXML.h"
#include "SelfUpdateEXE.h"
#include "FirewallUtil.h"
#include "TexImage2D.h"
#include "EULAWnd.h"
#include "HWInfo.h"
#include "HWInfoPost.h"
#include "Win32Input.h"
#include "SteamHelper.h"

#ifdef USE_VMPROTECT
  #error "DO NOT compile updater with USE_VMPROTECT"
#endif

#pragma comment(lib, "Wbemuuid.lib")

extern	HANDLE		r3d_CurrentProcess;
extern	bool		g_bExit;
extern	int		_r3d_bTerminateOnZ;
extern	void		r3dWinStyleModify(HWND hWnd, int add, DWORD style);

	bool		g_isConsoleUpdater = false;
	extern void cmdLine_MainLoop(CUpdater& updater);
	extern void cmdLine_Init();
	
	ITaskbarList3*  g_taskbar = NULL;
	HANDLE		g_updaterEvt = NULL;

	r3dTexImage2D*	g_bkgLogin  = NULL;
	r3dTexImage2D*	g_bkgUpdate = NULL;
	r3dTexImage2D*	g_bkgRegister = NULL;
	r3dTexImage2D*	g_bkgStarting = NULL;
	r3dTexImage2D*	g_imPB[3] = {0};	// progress bar images
	r3dTexImage2D*	g_imBtnPg[3] = {0};
	r3dTexImage2D*	g_imBtnClose = {0};
	r3dTexImage2D*	g_imBtnLogin = {0};
	//r3dTexImage2D*	g_imBtnTopMenu[8] = {0};
	r3dTexImage2D*	g_imBtnRegister = {0};
	r3dTexImage2D*	g_imG1RegisterNote = {0};
	
	CD3DFont*	g_font1   = NULL;
	CD3DFont*	g_font2   = NULL;
	CD3DFont*	g_font3   = NULL;
	bool		g_bStartGame = false;
	
	int		g_mx = 0;
	int		g_my = 0;
	int		g_mb = 0;

	HCURSOR		g_hCursor = NULL;
	HCURSOR		gCursorArrow;
	HCURSOR		gCursorHand;
	
	wchar_t		g_RegionLockCIS[521] = L"";


// some crap to make eternity link
DWORD DriverUpdater(HWND hParentWnd, DWORD VendorId, DWORD v1, DWORD v2, DWORD v3, DWORD v4, DWORD hash) {return hash;}
void writeGameOptionsFile() {}
void r3dScaleformBeginFrame() {}
void r3dScaleformEndFrame() {}
void SetNewSimpleFogParams() {}
void SetVolumeFogParams() {}
class r3dScreenBuffer * DepthBuffer;
class r3dCamera gCam;
class r3dSun * Sun;
#include "../EclipseStudio/Sources/GameLevel.h"
#include "../EclipseStudio/Sources/GameLevel.cpp"
r3dLightSystem WorldLightSystem;
void r3dAtmosphere::Reset() {}

struct r3dRECT {
  float left, top, width, height;
};

static bool insideRect(const r3dRECT& r, int x, int y)
{
  return x >= r.left && x <= (r.left+r.width) && y >= r.top && y <= (r.top+r.height);
}

static bool checkIfAlreadyRunning()
{
  static const char* updaterName = "Global\\WarInc_Updater_001";
  static const char* gameName = "Global\\WarInc_Game_001";
  
  HANDLE h;
  if((h = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, gameName)) != NULL) {
    MessageBox(NULL, "Game is already running", "Warning", MB_OK | MB_ICONEXCLAMATION);
    CloseHandle(h);
    return true;
  }

  if((h = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, updaterName)) != NULL) {
    OutputDebugStringA("Updater is already running");
    SetEvent(h);
    CloseHandle(h);
    return true;
  }
  
  // create named event to signalize that updater is already running.
  // handle will be automatically closed on program termination
  g_updaterEvt = CreateEvent(NULL, FALSE, FALSE, updaterName);
  return false;
}

static bool testForCurrentDir()
{
  char exeDir[MAX_PATH];
  char curDir[MAX_PATH];

  char exeMain[MAX_PATH];
  GetModuleFileName(NULL, exeMain, sizeof(exeMain));
  
  char* fname;
  ::GetFullPathName(exeMain, sizeof(exeDir), exeDir, &fname);
  if(fname) *(fname - 1) = 0; // remove '\' at the end
  
  ::GetCurrentDirectory(sizeof(curDir), curDir);
  if(stricmp(exeDir, curDir) != 0) {
    char msg[2048];
    sprintf(msg, "Updater current directory (%s) is different from working directory (%s)", curDir, exeDir);
    MessageBox(NULL, msg, "Warning", MB_OK | MB_ICONEXCLAMATION);
  }
  //::SetCurrentDirectory(g_workDir);
  
  return true;
}

static bool testForWriteToDir()
{
  char buf[MAX_PATH];
  sprintf(buf, "%s", "test.bin");
  FILE* f  = fopen(buf, "wb");
  if(f == NULL) {
    return false;
  }
  fclose(f);
  _unlink(buf);
  
  return true;
}

static bool detectTerminalSession()
{
  if(strstr(__r3dCmdLine, "-cmdLine") != NULL)
    return true;
  
/*    
  int rs = GetSystemMetrics(SM_REMOTESESSION);
  if(rs > 0)
    return true;
*/    
    
  return false;
}

void game::PreInit(void)  
{ 
  // set icon
  win::hWinIcon = ::LoadIcon(win::hInstance, MAKEINTRESOURCE(IDI_WARINC));

  #ifndef _DEBUG
  // do not close on debug terminate key
  _r3d_bTerminateOnZ = 0;
  #endif

  if(strstr(__r3dCmdLine, "-generate") != NULL) {
    GenerateUpdaterXML();
    TerminateProcess(GetCurrentProcess(), 0);
    return;
  }
  
  if(checkIfAlreadyRunning()) {
    TerminateProcess(GetCurrentProcess(), 0);
    return;
  }

  // see if we running from correct directory (just in case)
  testForCurrentDir();

  if(!testForWriteToDir()) {
    MessageBox(NULL, "Updater must be run from administrator", "UAC problem", MB_OK | MB_ICONINFORMATION);
    TerminateProcess(GetCurrentProcess(), 0);
    return;
  }
  
  selfUpd_TestIfUpdated();
  
  // win7 taskbar
  CoInitialize(NULL);
  if(FAILED(CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, r3dIID_ITaskbarList3, (void**)&g_taskbar))) {
    g_taskbar = NULL;
  }

  if(detectTerminalSession())
  {
    g_isConsoleUpdater = true;
  }

  gCursorArrow = LoadCursor(NULL, IDC_ARROW);
  gCursorHand  = LoadCursor(NULL, IDC_HAND);
  
  LoadStringW(NULL, IDS_STRING_REGIONLOCK_CIS, g_RegionLockCIS, sizeof(g_RegionLockCIS) / sizeof(g_RegionLockCIS[0]));
  
  return;
}

static int fake_SetMode(int XRes, int YRes)
{
  r3dRenderLayer& r = *r3dRenderer;

  HRESULT res;
  int DepthMode = D3DFMT_D24S8;
  r.DeviceType = D3DDEVTYPE_HAL;
  r.CurrentBPP = 32;

  D3DDISPLAYMODE mode;
  // get windowed display mode
  r.pd3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
  mode.Width  = XRes;
  mode.Height = YRes;
  r3d_assert(mode.Width && mode.Height);

  // adjust our window size
  RECT rc;
  SetRect(&rc, 0, 0, mode.Width, mode.Height);        
  AdjustWindowRect(&rc, GetWindowLong(r.HLibWin, GWL_STYLE), false);
  SetWindowPos(r.HLibWin, HWND_TOP, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOMOVE);

  extern float __r3dGlobalAspect;
  __r3dGlobalAspect = float(mode.Width) / float(mode.Height);

  // Set up the structure used to create the D3DDevice. Since we are now
  D3DPRESENT_PARAMETERS d3dpp;
  ZeroMemory(&d3dpp, sizeof(d3dpp) );
  d3dpp.Windowed               = TRUE;
  d3dpp.hDeviceWindow          = r.HLibWin;
  d3dpp.BackBufferWidth        = mode.Width;
  d3dpp.BackBufferHeight       = mode.Height;
  d3dpp.BackBufferFormat       = D3DFMT_A8R8G8B8;
  d3dpp.BackBufferCount        = 1;
  d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
  d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_FLIP ;
  d3dpp.Flags                  = 0 ;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = (D3DFORMAT)DepthMode;
  r.d3dpp = d3dpp;
  
  IDirect3DDevice9 *mDev = NULL;
  res = r.pd3d->CreateDevice(
    D3DADAPTER_DEFAULT,
    r.DeviceType,
    r.HLibWin,
    D3DCREATE_HARDWARE_VERTEXPROCESSING,
    &r.d3dpp,
    &mDev);
  if(FAILED(res)) 
  {
    MessageBox(NULL, "Our apologies, but you need video card with hardware vertex processing to run this game.", "Apologies", MB_OK);
    TerminateProcess(GetCurrentProcess(), 0);
  }
  r.SetD3DDev(mDev);

  if(r.Reset() == false)
  {
    TerminateProcess(GetCurrentProcess(), 0);
  }
  
  r.ResetStats();

  r3dIntegrityGuardian ig ;
  _r3dSystemFont = new CD3DFont(ig, "Verdana", 10, D3DFONT_BOLD | D3DFONT_FILTERED | D3DFONT_SKIPGLYPH);
  _r3dSystemFont->CreateSystemFont();
  
  extern int r3d_CD3DFont_Quality;
  r3d_CD3DFont_Quality = 5; // set to CLEARTYPE_QUALITY

  g_font1 = new CD3DFont(ig, "Tahoma", 11, D3DFONT_FILTERED | D3DFONT_SKIPGLYPH);
  g_font1->CreateSystemFont();
  g_font2 = new CD3DFont(ig, "Tahoma", 10, D3DFONT_FILTERED | D3DFONT_SKIPGLYPH);
  g_font2->CreateSystemFont();
  g_font3 = new CD3DFont(ig, "Tahoma", 9, D3DFONT_FILTERED | D3DFONT_SKIPGLYPH);
  g_font3->CreateSystemFont();

  r3dMaterialLibrary::Reset();
  
  // enable alpha blending for texture
  r3dRenderer->pd3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
  r3dRenderer->pd3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  r3dRenderer->pd3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

  return TRUE;
}

static void CreateResources()
{
  g_bkgLogin    = new r3dTexImage2D(IDR_IMAGE_BACK_LOGIN, 785, 650);
  g_bkgUpdate   = new r3dTexImage2D(IDR_IMAGE_BACK_UPDATE, 785, 650);
  g_bkgRegister = new r3dTexImage2D(IDR_IMAGE_BACK_REGISTER, 785, 650);
  g_bkgStarting = new r3dTexImage2D(IDR_IMAGE_BACK_STARTING, 785, 650);
  g_imPB[0] = new r3dTexImage2D(IDR_IMAGE_PB_LEFT, 6, 10);
  g_imPB[1] = new r3dTexImage2D(IDR_IMAGE_PB_CENTER, 4, 10);
  g_imPB[2] = new r3dTexImage2D(IDR_IMAGE_PB_RIGHT, 6, 10);
  g_imBtnClose = new r3dTexImage2D(IDR_IMAGE_BTN_CLOSE_ON, 16, 16);
  g_imBtnLogin = new r3dTexImage2D(IDR_IMAGE_BTN_LOGIN_HOVER, 76, 112);
  g_imBtnRegister = new r3dTexImage2D(IDR_IMAGE_BTN_REGISTER_HOVER, 256, 44);
/*  g_imBtnTopMenu[0] = new r3dTexImage2D(IDR_IMAGE_BTN_HOWTO_HOVER, 148, 32);
  g_imBtnTopMenu[1] = new r3dTexImage2D(IDR_IMAGE_BTN_MYACC_HOVER, 145, 32);
  g_imBtnTopMenu[2] = new r3dTexImage2D(IDR_IMAGE_BTN_FORUMS_HOVER, 108, 32);
  g_imBtnTopMenu[3] = new r3dTexImage2D(IDR_IMAGE_BTN_BLOG_HOVER, 92, 32);
  g_imBtnTopMenu[4] = new r3dTexImage2D(IDR_IMAGE_BTN_SUPPORT_HOVER, 111, 32);
  g_imBtnTopMenu[5] = new r3dTexImage2D(IDR_IMAGE_BTN_YOUTUBE_HOVER, 64, 32);
  g_imBtnTopMenu[6] = new r3dTexImage2D(IDR_IMAGE_BTN_FACEBOOK_HOVER, 32, 32);
  g_imBtnTopMenu[7] = new r3dTexImage2D(IDR_IMAGE_BTN_TWITTER_HOVER, 32, 32);*/
  g_imG1RegisterNote = new r3dTexImage2D(IDR_IMAGE_G1_REGISTER_NOTE, 331, 113);
  
  g_imBtnPg[0] = new r3dTexImage2D(IDR_IMAGE_BTN_PG_ON,    328, 128);
  g_imBtnPg[1] = new r3dTexImage2D(IDR_IMAGE_BTN_PG_HOVER, 328, 128);

  // sync steam_api.dll from resource
  gSteam.SyncDllFromResource();

  return;
}

static bool dragActive = 0;
static POINT dragSavedPnt = {0};
static LRESULT CALLBACK updApp_WndFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg) 
  {
    case WM_CLOSE:
      ClipCursor(NULL);
      ReleaseCapture();
      g_bExit = true;
      return 0;
    
    case WM_LBUTTONDOWN:
    {
      int mx = LOWORD(lParam);
      int my = HIWORD(lParam);
      if(my < 20 && mx < 747) {
        dragActive = true;
        GetCursorPos(&dragSavedPnt);

        RECT wndRect;
        GetWindowRect(hWnd, &wndRect);
        dragSavedPnt.x -= wndRect.left;
        dragSavedPnt.y -= wndRect.top;

        SetCapture(hWnd);
        break;
      }

      break;
    }

    case WM_LBUTTONUP:
      if(dragActive) {
        dragActive = false;
        ReleaseCapture();
        break;
      }

      g_mb |= 1;
      break;

    case WM_MOUSEMOVE:
      g_mx = LOWORD(lParam);
      g_my = HIWORD(lParam);
      
      if(dragActive) {
        POINT pnt;
        GetCursorPos(&pnt);

        SetWindowPos(hWnd, NULL, pnt.x - dragSavedPnt.x, pnt.y - dragSavedPnt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        break;
      }
      break;
      
    case WM_CHAR:
      win32_OnWMChar(wParam);
      break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void game::Init(void)
{
  if(g_isConsoleUpdater)
  {
    cmdLine_Init();
    return;
  }
  
  gHwInfoPoster.Start(); //-- DISABLED FOR NOW

  // set small icon
  ::SendMessage(win::hWnd, WM_SETICON, FALSE, (LPARAM)::LoadIcon(win::hInstance, MAKEINTRESOURCE(IDI_WARINC_SMALL)));
  
  // make borderless window and subclass wndProc
  r3dWinStyleModify(win::hWnd, 0, WS_BORDER);
  r3dWinStyleModify(win::hWnd, 0, WS_CAPTION);
  SetWindowLong(win::hWnd, GWL_WNDPROC, (DWORD)&updApp_WndFunc);

  // ok, now r3drendered started using vars inside itself...
  RegisterAllVars();

  const int Width  = 785;
  const int Height = 650;

  HDC disp_dc  = CreateIC("DISPLAY", NULL, NULL, NULL);
  int curDispWidth  = GetDeviceCaps(disp_dc, HORZRES);
  int curDispHeight = GetDeviceCaps(disp_dc, VERTRES);
  int StartXPos = (curDispWidth - Width) / 2;
  int StartYPos = (curDispHeight - Height) / 2;
  SetWindowPos(win::hWnd, NULL, StartXPos, StartYPos, (int)Width, (int)Height, 0);

  r3dRenderer = new r3dRenderLayer;
  r3dRenderer->Init(win::hWnd, NULL);
  
  fake_SetMode(Width, Height);
  
  char title[512];
  sprintf(title, "WOUpdater %s (%s)", UPDATER_VERSION, UPDATER_BUILD);
  if(!UPDATER_UPDATER_ENABLED) strcat(title, "!!!!SELF_UPDATE_DISABLED!!!");
  ::SetWindowText(win::hWnd, title);

  CreateResources();
  
  ShowWindow(win::hWnd, SW_SHOW);
  
  ClipCursor(NULL);
  
  // release dinput classes becase we won't be using them
  Keyboard->ReleaseCapture();
  Mouse->ReleaseCapture();
}

void game::Shutdown(void)
{
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

static SIZE GetTextExtent(CD3DFont* fnt, const char* text)
{
  RECT r;
  r.left   = 0;
  r.top    = 0;
  r.right  = 99999;
  r.bottom = 99999;

  fnt->GetD3DFont()->DrawTextA(
    NULL,
    text,
    strlen(text), 
    &r,
    DT_CALCRECT,
    0xFFFFFFFF);
    
  SIZE sz = {r.right  - r.left, r.bottom - r.top};
  return sz;
}


static int editTextField(float x, float y, char* editString, bool isFocused, bool isPassword, unsigned int maxLength = 32)
{
  r3dColor clr(0, 86, 159);

  char stars[512];
  for(size_t i=0; i<strlen(editString); i++)
    stars[i] = '*';
  stars[strlen(editString)] = 0;
     
  char* msg = isPassword ? stars : editString;
  g_font1->PrintF(x, y, clr, "%s", msg);
  if(!isFocused)
    return 0;
    
  // blinking 
  static float blinkRate = (float)GetCaretBlinkTime() / 1000.0f;
  static float nextBlink = r3dGetTime();
  static int   caretOn   = 0;
  
  if(r3dGetTime() > nextBlink) {
    nextBlink += blinkRate;
    caretOn = !caretOn;
  }

  // by some weird reason GetTextExtent ignoring last spaces in string.
  char sizemsg[512];
  sprintf(sizemsg, "%s.", msg);
  SIZE ts1 = GetTextExtent(g_font1, sizemsg);
  SIZE ts2 = GetTextExtent(g_font1, ".");
  if(caretOn) {
    r3dDrawBox2D(x + (float)(ts1.cx - ts2.cx), y+2, 1, 16, r3dColor(0, 0, 0));
  }

  // do input
  int ch = win32_getch();
  if(ch == 0)
    return 0;
    
  //r3dOutToLog("KEY: %d\n", ch);
  
  // backspace
  if(ch == 8 && *editString != 0)
    editString[strlen(editString)-1] = 0;

  // check for max length
  if(strlen(editString) >= maxLength)
    return 0;
    
  // normal key
  if(ch >= 0x20 && ch < 0x80) {
    sprintf(editString + strlen(editString), "%c", ch);
  }
  
  return ch;
}



static void drawProgressBar(CUpdater& updater)
{
  float base_y = updater.status_ == CUpdater::STATUS_Updating ? 615.0f : 626.0f;
  
  // updater info & version
  g_font1->PrintF(21, base_y-24, r3dColor(153, 208, 237), "%s", updater.updMsg1_);
  g_font3->PrintF(21, base_y+11, r3dColor(82, 108, 129), "v%s%s", UPDATER_VERSION,UPDATER_VERSION_SUFFIX);
  
  if(!updater.showProgress_)
    return;

  r3dColor pbclr = r3dColor(255, 255, 255);
  float y    = base_y;
  float x    = 22;
  float w    = 762 - x;
  float perc = updater.prgTotal_.getCoef();
  float c    = (w * perc);	// length of center bar
  
  if(c > 19) {
    g_imPB[1]->draw(x, y, c, 10, pbclr);
    g_imPB[0]->draw(x, y, 6, 10, pbclr);
    g_imPB[2]->draw(x+c-6, y, 6, 10, pbclr);
  } else if(c >= 1) {
    g_imPB[0]->draw(x, y, 6, 10, pbclr);
    g_imPB[2]->draw(x+4, y, 6, 10, pbclr);
  } else {
    // draw nothing
  }


/*
  float mbLeft = ((float)updater.prgTotal_.total - updater.prgTotal_.cur) / 1024.0f / 1024.0f;
  if(mbLeft > 0.01f) {
     g_font1->PrintF(423, 224, r3dColor(153, 208, 237), "%.0f mb left\n", mbLeft);
  }
*/  
  
  if(g_taskbar) g_taskbar->SetProgressValue(win::hWnd, updater.prgTotal_.cur, updater.prgTotal_.total);
}

static bool drawUrl(float x, float y, const char* msg)
{
    // news text
    SIZE ts = GetTextExtent(g_font1, msg);
    r3dRECT r = {x, y, (float)ts.cx, (float)ts.cy};
    r3dColor clr = r3dColor(34, 152, 214);
    
    // selection and underline
    if(insideRect(r, g_mx, g_my)) {
      clr = r3dColor(255, 216, 0);
      r3dDrawBox2D(x, y + (float)ts.cy, (float)ts.cx, 1, clr);
      if(g_mb) {
        return true;
      }
    }

    g_font1->PrintF(x, y, clr, "%s", msg);
    return false;
}

static void drawCloseButton()
{
  static r3dRECT g_rCloseBtn = {759, 6, 20, 16};

  // close button
  r3dColor clr(255, 255, 255);
  if(insideRect(g_rCloseBtn, g_mx, g_my)) {
    g_imBtnClose->draw(761, 7, 16, 16, clr);
    if(g_mb) {
      g_bExit = true;
    }
  }
}

static void filterAccountName(char* acc)
{
  char filtered[128];
  char* out = filtered;

  const char* in = acc;
  
  // trim left
  while(*in == ' ') in++;
  
  for(; *in; in++) 
  {
    if(*in == ' ' || 
      (*in >= 'a' && *in <= 'z') ||
      (*in >= 'A' && *in <= 'Z') ||
      (*in >= '0' && *in <= '9')) {
      *out++ = *in;
    }
  }
  *out = 0;
  
  r3dscpy(acc, filtered);
}

static void drawRegisterButtons(CUpdater& updater)
{
  static r3dRECT g_rRegisterBtn  = {516, 530, 256, 44};
  static r3dRECT g_rEditFields[4] = {
    {462, 321, 289, 19},
    {462, 373, 289, 19},
    {462, 424, 289, 19},
    {462, 476, 289, 19}
  };
  // username, email, passwd1, passwd1
  static char gEditText[4][512] = {0};
  static int gInputLen[4] = {16, 64, 32, 32};
  
  static bool firstTime = true;
  if(firstTime) {
    if(gSteam.steamID > 0)
	r3dscpy(gEditText[0], gSteam.playerName);
    if(updater.gUserProfile.g1AccountId > 0)
	r3dscpy(gEditText[0], updater.gUserProfile.g1Username);
    
    filterAccountName(gEditText[0]);
    firstTime = false;
  }
  
  r3dColor clr(255, 255, 255);

  if(updater.gUserProfile.g1AccountId > 0)
    g_imG1RegisterNote->draw(441, 137, 331, 113, clr);
  
  // if login in process, draw only needed things
  if(updater.createAccHelper.createAccCode == CCreateAccHelper::CA_Processing) 
  {
    for(int i=0; i<4; i++) {
      r3dDrawBox2D(g_rEditFields[i].left-2, g_rEditFields[i].top-2, g_rEditFields[i].width+4, g_rEditFields[i].height+4, r3dColor(0, 0, 0, 100));
      editTextField(g_rEditFields[i].left, g_rEditFields[i].top, gEditText[i], false, i>=2);
    }
    return;
  }
  
  // register button. enabled if we have all data
  if(gEditText[0][0] && gEditText[1][0] && gEditText[2][0] && gEditText[3][0] &&
     insideRect(g_rRegisterBtn, g_mx, g_my)) 
  {
    g_imBtnRegister->draw(g_rRegisterBtn.left, g_rRegisterBtn.top, g_rRegisterBtn.width, g_rRegisterBtn.height, clr);
    if(g_mb) 
    {
      strcpy(updater.createAccHelper.username, gEditText[0]);
      strcpy(updater.createAccHelper.email,    gEditText[1]);
      strcpy(updater.createAccHelper.passwd1,  gEditText[2]);
      strcpy(updater.createAccHelper.passwd2,  gEditText[3]);
      updater.DoCreateAccount();
    }
  }
  
  // input fields logic
  static int focusId = 0;
  
  for(int i=0; i<4; i++) 
  {
    if(g_mb && insideRect(g_rEditFields[i], g_mx, g_my)) {
      focusId = i;
      win32_input_Flush();
    }
    
    int key;
    key = editTextField(
      g_rEditFields[i].left, 
      g_rEditFields[i].top, 
      gEditText[i], 
      focusId == i, 
      i>=2,
      gInputLen[i]);
    if(key == 9)  focusId = (GetKeyState(VK_SHIFT)&0x8000) ? focusId-1 : focusId+1;
    if(key == 13 && gEditText[i][0]) focusId++;
    
    if(focusId >= 4) focusId = 0;
    if(focusId < 0)  focusId = 3;
  }
  
  filterAccountName(gEditText[0]);

  return;
}

static void drawLoginButtons(CUpdater& updater)
{
  static r3dRECT g_rLoginBtn  = {687, 166, 76, 112};
  static r3dRECT g_rLoginEdit = {462, 194, 206, 19};
  static r3dRECT g_rPwdEdit   = {462, 246, 206, 19};
  r3dColor clr(255, 255, 255);
  
  // if login or update in process, draw only needed things
  if(updater.gUserProfile.loginAnswerCode == CLoginHelper::ANS_Processing 
     || updater.gUserProfile.loginAnswerCode == CLoginHelper::ANS_Logged)
  {
    r3dDrawBox2D(g_rLoginEdit.left-2, g_rLoginEdit.top-2, g_rLoginEdit.width+4, g_rLoginEdit.height+4, r3dColor(0, 0, 0, 100));
    r3dDrawBox2D(g_rPwdEdit.left-2, g_rPwdEdit.top-2, g_rPwdEdit.width+4, g_rPwdEdit.height+4, r3dColor(0, 0, 0, 100));
    editTextField(g_rLoginEdit.left, g_rLoginEdit.top, updater.gUserProfile.username, false, false);
    editTextField(g_rPwdEdit.left, g_rPwdEdit.top, updater.gUserProfile.passwd, false, true);

    if(updater.gUserProfile.loginAnswerCode == CLoginHelper::ANS_Logged) {
      updater.SwitchToUpdater();
    }

    return;
  }
  
  // login button. enabled if we have all data
  if(*updater.gUserProfile.username && *updater.gUserProfile.passwd && insideRect(g_rLoginBtn, g_mx, g_my)) {
    g_imBtnLogin->draw(g_rLoginBtn.left, g_rLoginBtn.top, g_rLoginBtn.width, g_rLoginBtn.height, clr);
    if(g_mb) {
      updater.DoLogin();
    }
  }
  
  if(drawUrl(446, 621, "Esqueceu a senha?")) {
    ShellExecute(NULL, "open", "https://account.thewarinc.com/pwdreset1.php", "", NULL, SW_SHOW);
  }

  if(drawUrl(640, 621, "Criar Nova Conta")) {
    ShellExecute(NULL, "open", "http://signup.thewarinc.com", "", NULL, SW_SHOW);
  }
  
  // login error
  if(updater.loginErrMsg_[0]) {
    r3dDrawBox2D(441, 293, 331, 22, r3dColor(0, 0, 0, 100));
    g_font1->DrawText(441, 293, 331, 22, r3dColor(255, 26, 26), updater.loginErrMsg_, D3DFONT_CENTERED);
  }
  
  // name/pwd logic
  static int focusId = updater.gUserProfile.username[0] ? 1 : 0;
  
  if(g_mb && insideRect(g_rLoginEdit, g_mx, g_my)) {
    focusId = 0;
    win32_input_Flush();
  }
  if(g_mb && insideRect(g_rPwdEdit, g_mx, g_my)) {
    focusId = 1;
    win32_input_Flush();
  }

  int key;
  key = editTextField(g_rLoginEdit.left, g_rLoginEdit.top, updater.gUserProfile.username, focusId == 0, false);
  if(key == 9)  focusId = 1;
  if(key == 13 && updater.gUserProfile.username[0]) focusId = 1;

  key = editTextField(g_rPwdEdit.left, g_rPwdEdit.top, updater.gUserProfile.passwd, focusId == 1, true);
  if(key == 9)  focusId = 0;
  if(key == 13 && updater.gUserProfile.passwd[0] && updater.gUserProfile.username[0])
    updater.DoLogin();
    
  return;
}

static void executeTopMenuButton(const CUpdater& updater, int btnIdx)
{
  char token[512];
  updater.gUserProfile.CreateAuthToken(token);

  switch(btnIdx)
  {
    case 0: // how to play
    {
     // ShellExecute(NULL, "open", "http://thewarinc.com/faq", "", NULL, SW_SHOW);
      break;
    }

    case 1: // my account
    {
      char url[1024];
      sprintf(url, "https://account.thewarinc.com/?WoLogin=%s", token);
      ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
      break;
    }
    
    case 2: // forums
    {
     // ShellExecute(NULL, "open", "http://forums.thewarinc.com", "", NULL, SW_SHOW);
      break;
    }
    
    case 3: // get gc
    {
      char url[1024];
      sprintf(url, "http://account.thewarinc.com/earn.php?WoLogin=%s", token);

      ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
      break;
    }
    
    case 4: // support
    {
     // ShellExecute(NULL, "open", "http://support.thewarinc.com", "", NULL, SW_SHOW);
      break;
    }
    
    case 5: // youtube
    {
    //  ShellExecute(NULL, "open", "http://www.youtube.com/user/WarIncBattlezone", "", NULL, SW_SHOW);
      break;
    }
    
    case 6: // facebook
    {
      ShellExecute(NULL, "open", "http://www.facebook.com/equipewarbrasil", "", NULL, SW_SHOW);
      break;
    }
    
    case 7: // twitter
    {
     // ShellExecute(NULL, "open", "http://twitter.com/#!/thewarinc", "", NULL, SW_SHOW);
      break;
    }
  }

}

static void drawUpdateButtons(CUpdater& updater)
{
  static r3dRECT g_rPlayBtn  = {499, 494, 275, 100};
  static r3dRECT g_rTopBtn[8] = {
    {  6, 30, 148, 32},
    {155, 30, 145, 32},
    {300, 30, 108, 32},
    {409, 30,  92, 32},
    {502, 30, 111, 32},
    {640, 30,  64, 32},
    {704, 30,  32, 32},
    {738, 30,  32, 32}
  };
    

  r3dColor clr = r3dColor(255, 255, 255);

  // play button. active only when updater finished updating.
  // and servers logically offline
  if(updater.result_ == CUpdater::RES_PLAY && updater.IsServerOnline())
  {
    if(insideRect(g_rPlayBtn, g_mx, g_my)) {
      g_imBtnPg[1]->draw(485, 500, 328, 128, clr);
    } else {
      g_imBtnPg[0]->draw(485, 500, 328, 128, clr);
    }

    if(insideRect(g_rPlayBtn, g_mx, g_my) && g_mb) {
      g_bStartGame = true;
    }
  }
  
 
  for(int btnIdx=0; btnIdx<8; btnIdx++)
  {
    if(insideRect(g_rTopBtn[btnIdx], g_mx, g_my)) 
    {
      /*g_imBtnTopMenu[btnIdx]->draw(
        g_rTopBtn[btnIdx].left, 
        g_rTopBtn[btnIdx].top,
        g_rTopBtn[btnIdx].width,
        g_rTopBtn[btnIdx].height,
        clr);*/
        
      if(g_mb) {
        executeTopMenuButton(updater, btnIdx);
      }
    }
  }
  
  return;
}

static void drawNewsSection(float x, float y, const std::vector<CUpdater::news_s>& news, size_t maxLines)
{
  float o = 624; // date text offset

  for(size_t i=0; i<news.size() && i < maxLines; i++) 
  {
    // news date
    g_font2->PrintF(x+o, y, r3dColor(243, 182, 86), news[i].date_.c_str());

    // news text
	const char* newsText = news[i].name_.c_str();
    SIZE ts = GetTextExtent(g_font2, newsText);
    r3dRECT r = {x, y, (float)ts.cx, (float)ts.cy};
    r3dColor clr = r3dColor(255, 255, 255);
    
    // selection and underline
    if(insideRect(r, g_mx, g_my)) {
      clr = r3dColor(255, 216, 0);
      r3dDrawBox2D(x, y + (float)ts.cy, (float)ts.cx, 1, clr);
      if(g_mb) {
        ShellExecute(NULL, "open", news[i].url_.c_str(), "", NULL, SW_SHOW);
      }
    }

    g_font2->PrintF(x, y, clr, "%s", newsText);
    y += 20;
  }
  
  //TODO: "View All"

  return;  
}

/*
static void drawAnnouncements(const std::vector<CUpdater::news_s>& news, size_t maxLines)
{
  float y  = 117;
  int   x1 = 465;
  int   x2 = 759;

  DWORD flags = DT_RIGHT | DT_TOP | DT_WORDBREAK;
  
  ID3DXFont* d3dfont = g_font2->GetD3DFont(); 
  for(size_t i=0; i<news.size() && i < maxLines; i++) 
  {
    // announcement title (in date_ field)
    RECT r1 = {x1, (int)y, x2, (int)y+30};
    
    d3dfont->DrawTextA(
      NULL, //fnt_sprite,
      news[i].date_.c_str(),
      news[i].date_.length(), 
      &r1,
      flags,
      r3dColor(191, 227, 255).GetPacked());

    // announcement text
    y += 20;
    RECT r2 = {x1, (int)y, x2, (int)y+50};

    d3dfont->DrawTextA(
      NULL, //fnt_sprite,
      news[i].name_.c_str(),
      news[i].name_.length(),
      &r2,
      flags,
      r3dColor(161, 161, 161).GetPacked());

    y += 50;
    if(drawUrl(718, y, "more...")) {
      ShellExecute(NULL, "open", news[i].url_.c_str(), "", NULL, SW_SHOW);
    }
    
    y += 35;
  }

}
*/

static void drawNews(CUpdater& updater)
{
  if(updater.newsStatus_ != 2)
    return;

  r3dCSHolder cs1(updater.csNews_);

  // check for survey
  static bool need_to_show_survey = true;
  if(!updater.surveyLinkIn_.empty() && need_to_show_survey) {
    need_to_show_survey = false;
    ShellExecute(NULL, "open", updater.surveyLinkIn_.c_str(), "", NULL, SW_SHOW);
  }

  drawNewsSection(43, 366, updater.newsData_, 5);

  return;
}

static void drawServerStatus(const CUpdater& updater)
{
  float y = 591;

  if(updater.newsStatus_ == 0) {
    g_font1->PrintF(607, y, r3dColor(153, 208, 237), "Retrieving Server Status");
    return;
  }

  if(updater.newsStatus_ == 1 || updater.serverStatus_.size() == 0) {
    g_font1->PrintF(607, y, r3dColor(255, 31, 31), "Server Status Unavailable");
    return;
  }
  
  g_font1->PrintF(610, y, r3dColor(153, 208, 237), "Server Status:");
  const char* status = updater.serverStatus_.c_str();
  if(stricmp(status, "ONLINE") == 0)
    g_font1->PrintF(710, y, r3dColor(31, 255, 57), "ONLINE");
  else if(stricmp(status, "OFFLINE") == 0)
    g_font1->PrintF(710, y, r3dColor(255, 31, 31), "OFFLINE");
  else
    g_font1->PrintF(710, y, r3dColor(255, 127, 39), status);
    
  return;
}

static void rotatorCreateTexture(CUpdater::rotator_s& rot)
{
  // create rotator texture if we downloaded data
  if(rot.imgStatus_ == 2) 
  {
    IDirect3DTexture9* d3dtex = NULL;
    HRESULT hr = D3DXCreateTextureFromFileInMemory(
      r3dRenderer->pd3ddev,
      rot.imgData_.getBytes(), 
      rot.imgData_.getSize(), 
      &d3dtex);

    if(hr == D3D_OK)
    {
      rot.tex_ = r3dRenderer->AllocateTexture();
      rot.tex_->Create(4, 4, D3DFMT_A8R8G8B8, 1);
      rot.tex_->SetNewD3DTexture(d3dtex);
    }
    else
    {
      r3dOutToLog("Rotator: failed to create texture from %s\n", rot.image_);
    }
    
    rot.imgStatus_ = 3;
  }
}

static void drawRotator(CUpdater& updater)
{
  static r3dRECT rRotImg = {21, 85, 745, 226};

  if(updater.rotatorStatus_ != 1)
    return;
  if(updater.rotatorData_.size() == 0)
    return;

  static float TIME_IN_IMAGE = 10.0f;
  static float TIME_TO_FADEIN = 1.0f;
  static float TIME_TO_SHOW_DESC = 1.0f;
    
  static int rotIdx = -1;
  static r3dTexture* prevTex = NULL;
  static float imgStartTime = -999;
  static float descShowTime = -999;
  
  if(r3dGetTime() > imgStartTime + TIME_IN_IMAGE) 
  {
    // check if next image is loaded
    size_t newIdx = (rotIdx + 1) % updater.rotatorData_.size();
    if(updater.rotatorData_[newIdx].imgStatus_ == 0)
      return;

    if(newIdx != rotIdx)
    {
      rotatorCreateTexture(updater.rotatorData_[newIdx]);
      
      // switch to new image
      if(rotIdx >= 0) 
        prevTex    = updater.rotatorData_[rotIdx].tex_;
      rotIdx       = newIdx;
      imgStartTime = r3dGetTime();
    }
  }
  
  r3d_assert(rotIdx >= 0 && rotIdx < (int)updater.rotatorData_.size());
  CUpdater::rotator_s& rot = updater.rotatorData_[rotIdx]; 
  
  if(rot.tex_ != NULL) 
  {
    if(prevTex) 
      r3dDrawBox2D(21, 85, 745, 226, r3dColor(255, 255, 255), prevTex);

    float a = 255.0f * R3D_MIN(1.0f, (r3dGetTime()-imgStartTime) / TIME_TO_FADEIN);
    r3dDrawBox2D(21, 85, 745, 226, r3dColor(255, 255, 255, (int)a), rot.tex_);
  } 
  else 
  {
    r3dDrawBox2D(21, 85, 745, 226, r3dColor(0, 0, 0, 100), NULL);
  }
  
  // process click
  if(g_mb && insideRect(rRotImg, g_mx, g_my)) 
  {
    char token[512];
    updater.gUserProfile.CreateAuthToken(token);

    char url[1024];
    if(strchr(rot.url_.c_str(), '?') == NULL) {
      sprintf(url, "%s?WoLogin=%s", rot.url_.c_str(), token);
    } else {
      sprintf(url, "%s&WoLogin=%s", rot.url_.c_str(), token);
    }
    
    ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
  }
  
  // show description
  if(insideRect(rRotImg, g_mx, g_my))
  {
    g_hCursor = gCursorHand;

    if(descShowTime < 0)
      descShowTime = r3dGetTime();

    if(r3dGetTime() > descShowTime + TIME_TO_SHOW_DESC)
    {
      g_font1->PrintF(25, 89, r3dColor(0, 0, 0), rot.desc_.c_str());
      g_font1->PrintF(23, 87, r3dColor(255, 255, 255), rot.desc_.c_str());
    }
  }
  else 
  {
    descShowTime = -1;
  }
}


static void showErrorMessageBox(const CUpdater& updater)
{
  char buf[2048];
  sprintf(buf, "There was a problem updating the game\n'%s'\n\nPlease retry later", updater.updErr1_);
  MessageBox(win::hWnd, updater.updErr1_, "Error", MB_OK | MB_ICONERROR);

  return;
}

static void startGame(const CUpdater& updater)
{
/*
  const char* UPDATE_FINISH_MSG = \
    "The game has been successfully updated to the latest version.\n"\
    "Do you want to start the game now?";
    
  if(updater.numUpdatedFiles_) {
    if(IDYES != MessageBox(win::hWnd, UPDATE_FINISH_MSG, GAME_TITLE, MB_YESNO | MB_ICONQUESTION))
      return;
  }
*/  

  // show EULA if we have updated something
  if(updater.numUpdatedFiles_) {
    if(eulaShowDialog() != IDOK) {
      return;
    }
  }

  r3dOutToLog("Starting game\n"); CLOG_INDENT;
  
  /* Because ShellExecute can delegate execution to Shell extensions 
     (data sources, context menu handlers, verb implementations) that are 
     activated using Component Object Model (COM), COM should be initialized 
     before ShellExecute is called. 
  */
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  
  extern bool r3dGetFileCrc32(const char* fname, DWORD* out_crc32, DWORD* out_size);
  DWORD crc32, size;
  if(!r3dGetFileCrc32(GAME_EXE_NAME, &crc32, &size)) {
    r3dOutToLog("failed1\n");
  } else {
    r3dOutToLog("%08x %d\n", crc32, size);
  }
  
  // pass special string to exe so it will know we updated successfully...
  char token[1024];
  updater.gUserProfile.CreateAuthToken(token);

  char GAME_START_PARAM[2048];
  sprintf(GAME_START_PARAM, "-WOUpdatedOk %s -WOLogin \"%s\"", __r3dCmdLine, token);
  if(updater.surveyLinkOut_.length() > 0) {
    sprintf(GAME_START_PARAM + strlen(GAME_START_PARAM), " -survey \"%s\"", updater.surveyLinkOut_.c_str());
  }
  
  const char* verb = getShellExecuteVerb();
  int err = (int)ShellExecute(NULL, verb, GAME_EXE_NAME, GAME_START_PARAM, NULL, SW_SHOW);
  if(err < 32) {
    r3dOutToLog("failed: %d\n", err);
  }
  
  return;
}

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
		case '\"':
			in_QM = TRUE;
			in_TEXT = TRUE;
			if(in_SPACE) {
				argv[argc] = _argv+j;
				argc++;
			}
			in_SPACE = FALSE;
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			if(in_TEXT) {
				_argv[j] = '\0';
				j++;
			}
			in_TEXT = FALSE;
			in_SPACE = TRUE;
			break;
		default:
			in_TEXT = TRUE;
			if(in_SPACE) {
				argv[argc] = _argv+j;
				argc++;
			}
			_argv[j] = a;
			j++;
			in_SPACE = FALSE;
			break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

void game::MainLoop(void)
{
  r3dOutToLog("Starting updater, v:%s, cmd:%s\n", UPDATER_VERSION, __r3dCmdLine);

  CUpdater updater;
  
  // parse command line
  int argc = 0;
  char** argv = CommandLineToArgvA(__r3dCmdLine, &argc);
  for(int i=0; i<argc; i++) 
  {
    if(strcmp(argv[i], "-steam") == 0 && (i + 0) < argc)
    {
      r3dOutToLog("Trying to init steam\n");
      gSteam.InitSteam();
      continue;
    }

    if(strncmp(argv[i], "-auth_token=", 12) == 0 && (i + 0) < argc)
    {
      r3dOutToLog("Gamersfirst auth_token=\n");
      r3dscpy(updater.gUserProfile.g1AuthToken, argv[i] + 12);
      continue;
    }
    
    if(strcmp(argv[i], "-auth_token") == 0 && (i + 1) < argc)
    {
      r3dOutToLog("Gamersfirst auth_token\n");
      r3dscpy(updater.gUserProfile.g1AuthToken, argv[++i]);
      continue;
    }
  }

  if(g_isConsoleUpdater)
  {
    cmdLine_MainLoop(updater);
    return;
  }
  
  updater.Start();

  win32_input_Flush();
  
  if(g_taskbar) g_taskbar->SetProgressState(win::hWnd, TBPF_NORMAL);
  
  r3dStartFrame();
  while(1)
  {
    r3dEndFrame();
    r3dStartFrame();

    g_mb = 0;
    g_hCursor = gCursorArrow;
    tempDoMsgLoop();
    ::Sleep(1);
    
    r3dRenderer->StartRender();
    r3dRenderer->StartFrame();
    r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
    r3dSetFiltering(R3D_POINT);
    
    r3dDrawBox2D(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor(64, 64, 64));
    
    switch(updater.status_) 
    {
      default: r3d_assert(0);
      
      case CUpdater::STATUS_Checking:
	g_bkgStarting->draw(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor(255, 255, 255));
	break;

      case CUpdater::STATUS_NeedLogin:
	g_bkgLogin->draw(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor(255, 255, 255));
	
	drawLoginButtons(updater);
	break;
    
      case CUpdater::STATUS_NeedRegister:
	g_bkgRegister->draw(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor(255, 255, 255));

	drawRegisterButtons(updater);
	break;

      case CUpdater::STATUS_Updating:
	g_bkgUpdate->draw(0, 0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, r3dColor(255, 255, 255));

	drawUpdateButtons(updater);
        drawServerStatus(updater);
	drawNews(updater);
	drawRotator(updater);
	break;
    }
    drawProgressBar(updater);
    drawCloseButton();

    r3dRenderer->EndFrame();
    r3dRenderer->EndRender(true);

    // if exit requested by control-f4
    if(g_bExit) {
      updater.RequestStop();
      break;
    }
    
    // activate updater window if other instance requested it
    if(WaitForSingleObject(g_updaterEvt, 0) == WAIT_OBJECT_0) {
      SwitchToThisWindow(win::hWnd, TRUE);
    }

    // try to remove mouse lock because of 
    // win::HandleActivate code that will lock mouse inside window
    ClipCursor(NULL);

    // if signalled to play game
    if(g_bStartGame)
      break;

    if(updater.result_ != CUpdater::RES_UPDATING && updater.result_ != CUpdater::RES_PLAY)
      break;
      
    // wndclass.hCursor must be NULL for this to work
    //SetCursor(g_hCursor);
  }
  
  updater.Stop();

  if(g_taskbar) g_taskbar->SetProgressState(win::hWnd, TBPF_NOPROGRESS);
  
  switch(updater.result_)
  {
    default: r3d_assert(0); 
    case CUpdater::RES_STOPPED:
      break;

    case CUpdater::RES_PLAY:
      startGame(updater);
      break;
      
    case CUpdater::RES_ERROR:
      showErrorMessageBox(updater);
      break;
  }
  
  gHwInfoPoster.Stop();
  TerminateProcess(GetCurrentProcess(), 0);
  
  return;
}
