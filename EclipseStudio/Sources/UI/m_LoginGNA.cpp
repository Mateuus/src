#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "m_LoginGNA.h"
#include "GameCode\UserProfile.h"
#include "Backend\WOBackendAPI.h"
#include "r3dDebug.h"

static int LoginMenuExitFlag = 0;
static int LoginQuitRequested = 0;

	char	Login_GNA_userid[256] = "";
	char	Login_GNA_appkey[256] = "";
	char	Login_GNA_token[256] = "";

unsigned int WINAPI CLoginGNAMenu::LoginGNAProcessThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	CLoginGNAMenu* This = (CLoginGNAMenu*)in_data;
	
	r3d_assert(This->loginAnswerCode == ANS_Unactive);
	This->loginAnswerCode = ANS_Processing;
	gUserProfile.CustomerID = 0;

	CWOBackendReq req("api_GNALogin.aspx");
	req.AddParam("userId", Login_GNA_userid);
	req.AddParam("appKey", Login_GNA_appkey);
	req.AddParam("token",  Login_GNA_token);

	if(!req.Issue())
	{
		r3dOutToLog("Login FAILED, code: %d\n", req.resultCode_);
		This->loginAnswerCode = req.resultCode_ == 8 ? ANS_Timeout : ANS_Error;
		return 0;
	}

	int n = sscanf(req.bodyStr_, "%d %d %d", 
		&gUserProfile.CustomerID, 
		&gUserProfile.SessionID,
		&gUserProfile.AccountStatus);
	if(n != 3)
	{
		r3dOutToLog("Login: bad answer\n");
		This->loginAnswerCode = ANS_Error;
		return 0;
	}
	//r3dOutToLog("CustomerID: %d\n",gUserProfile.CustomerID);

	if(gUserProfile.CustomerID == 0)
		This->loginAnswerCode = ANS_BadPassword;
	else if(gUserProfile.AccountStatus >= 200)
		This->loginAnswerCode = ANS_Frozen;
	else
		This->loginAnswerCode = ANS_Logged;
		
	return 0;
}

CLoginGNAMenu::CLoginGNAMenu(const char * movieName) : UIMenu(movieName), 
	m_ShowMessageIfAny(0)
{
	loginThread = NULL;
	loginAnswerCode = ANS_Unactive;
	m_pBackgroundTex = NULL;
	loginStarted_ = false;
}

CLoginGNAMenu::~CLoginGNAMenu()
{
	r3d_assert(loginThread == NULL);
	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
}

void CLoginGNAMenu::ShowErrorMessage(const wchar_t* msg) // we show that only in case if player couldn't connect to server, or was dropped from the server while playing
{
	Scaleform::GFx::Value vars[2];
	vars[0].SetStringW(msg);
	vars[1].SetBoolean(true);
	gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
}

void CLoginGNAMenu::eventLoginBoxAlertOkClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	pMovie->Invoke("_root.api.fadeOut", "");
	LoginQuitRequested = 1;
}

void CLoginGNAMenu::eventFadeOut(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	LoginMenuExitFlag = 1;
}

bool CLoginGNAMenu::Initialize()
{
	m_pBackgroundTex = r3dRenderer->LoadTexture("Data/Menu/Background_rus.dds");
	r3d_assert(m_pBackgroundTex);

	Mouse->SetRange( r3dRenderer->HLibWin );

	LoginMenuExitFlag = 0;

	#define MAKE_CALLBACK(FUNC) \
		new r3dScaleformMovie::TGFxEICallback<CLoginGNAMenu>(this, &CLoginGNAMenu::FUNC)
	
	gfxMovie.RegisterEventHandler("eventLoginBoxAlertOkClick", MAKE_CALLBACK(eventLoginBoxAlertOkClick));
	gfxMovie.RegisterEventHandler("eventFadeOut", MAKE_CALLBACK(eventFadeOut));
	
	#undef MAKE_CALLBACK

	// skip everything
	gfxMovie.Invoke("_root.api.showScreenLogin", "true");
	gfxMovie.SetVariable("_root.api.forceHideLogin", 1);

	if(m_ShowMessageIfAny)
	{
		// show error message and skip login sequence
		ShowErrorMessage(m_ShowMessageIfAny);
	}
	else
	{
		loginStarted_ = true;

		Scaleform::GFx::Value vars[2];
		vars[0].SetStringW(gLangMngr.getString("LoginMenu_ValidatingLogin"));
		vars[1].SetBoolean(false);
		gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);

		r3d_assert(loginThread == NULL);
		loginThread = (HANDLE)_beginthreadex(NULL, 0, &LoginGNAProcessThread, this, 0, NULL);
        if(loginThread == NULL)
            r3dError("Failed to begin thread");
	}

	menuInitTime = r3dGetTime();
	
	return true;
}
bool IsNeedExit();

void ClearFullScreen_Menu();

void CLoginGNAMenu::CheckAnswerCode()
{
	if(loginAnswerCode == ANS_Unactive)
		return;
		
	if(loginAnswerCode == ANS_Processing)
		return;
		
	// wait for thread to finish
	if(::WaitForSingleObject(loginThread, 1000) == WAIT_TIMEOUT)
		r3d_assert(0);
		
    CloseHandle(loginThread);
	loginThread = NULL;
		
	Scaleform::GFx::Value vars[2];
	switch(loginAnswerCode)
	{
	case ANS_Timeout:
		vars[0].SetStringW(gLangMngr.getString("LoginMenu_CommError"));
		vars[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
		break;
	case ANS_Error:
		vars[0].SetStringW(gLangMngr.getString("LoginMenu_WrongLoginAnswer"));
		vars[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
		break;

	case ANS_Logged:
		gfxMovie.Invoke("_root.api.fadeOut", "");
		break;

	case ANS_BadPassword:
		vars[0].SetStringW(gLangMngr.getString("LoginMenu_LoginFailed"));
		vars[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
		break;

	case ANS_Frozen:
		vars[0].SetStringW(gLangMngr.getString("LoginMenu_AccountFrozen"));
		vars[1].SetBoolean(true);
		gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
		break;
	}

	loginAnswerCode = ANS_Unactive;
}

int CLoginGNAMenu::Update()
{
	{
		r3dPoint3D soundPos(0,0,0), soundDir(0,0,1), soundUp(0,1,0);
		SoundSys.Update(soundPos, soundDir, soundUp);
	}

	r3dProcessWindowMessages();

	r3dMouse::Show();
	r3dStartFrame();

	if( r3dRenderer->DeviceAvailable )
	{
		r3dRenderer->StartRender(1);
		r3dRenderer->StartFrame();
	}

	if( r3dRenderer->DeviceAvailable )
	{
		// for now just draw a static picture in background, later on will be a video
		r3d_assert(m_pBackgroundTex);
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		float x, y, w, h;
		r3dRenderer->GetBackBufferViewport(&x, &y, &w, &h);
		D3DVIEWPORT9 oldVp, newVp;

		r3dRenderer->DoGetViewport(&oldVp);
		newVp = oldVp;
		newVp.X = 0;
		newVp.Y = 0;
		newVp.Width = r3dRenderer->d3dpp.BackBufferWidth;
		newVp.Height = r3dRenderer->d3dpp.BackBufferHeight;
		DWORD oldScissor = 0;
		r3dRenderer->pd3ddev->GetRenderState(D3DRS_SCISSORTESTENABLE, &oldScissor);
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		r3dRenderer->SetViewport( (float)newVp.X, (float)newVp.Y, (float)newVp.Width, (float)newVp.Height );
		r3dDrawBox2D(x, y, w, h, r3dColor24::white, m_pBackgroundTex);
		r3dRenderer->SetViewport( (float)oldVp.X, (float)oldVp.Y, (float)oldVp.Width, (float)oldVp.Height );
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, oldScissor);

		gfxMovie.UpdateAndDraw();

		r3dRenderer->Flush();  
		r3dRenderer->EndFrame();
	}

	r3dRenderer->EndRender( true );

	r3dEndFrame();

	CheckAnswerCode();

	if(loginThread == NULL)
	{

		// sorry, must have available(but possibly lost!) device here ( resource are soon created )
		r3dEnsureDeviceAvailable();

		if(IsNeedExit())
			return RET_Exit;

		if(LoginQuitRequested)
			return RET_Exit;

		if(LoginMenuExitFlag) 
			return RET_Logged;
	}
	
	return 0;
}
