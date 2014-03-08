#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "m_Login.h"
#include "GameCode\UserProfile.h"
#include "Backend\WOBackendAPI.h"
#include "r3dDebug.h"

static int LoginMenuExitFlag = 0;
static int LoginQuitRequested = 0;

	char	Login_PassedUser[256] = "";
	char	Login_PassedPwd[256] = "";
	char	Login_PassedAuth[256] = "";

unsigned int WINAPI CLoginMenu::LoginProcessThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	CLoginMenu* This = (CLoginMenu*)in_data;
	
	r3d_assert(This->loginAnswerCode == ANS_Unactive);
	This->loginAnswerCode = ANS_Processing;
	gUserProfile.CustomerID = 0;

	CWOBackendReq req("api_Login.aspx");
	req.AddParam("username", This->username);
	req.AddParam("password", This->password);

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

unsigned int WINAPI CLoginMenu::LoginAuthThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	CLoginMenu* This = (CLoginMenu*)in_data;
	
	r3d_assert(This->loginAnswerCode == ANS_Unactive);
	This->loginAnswerCode = ANS_Processing;
	r3d_assert(gUserProfile.CustomerID);
	r3d_assert(gUserProfile.SessionID);

	CWOBackendReq req(&gUserProfile, "api_CheckLoginSession.aspx");
	if(req.Issue() == true)
	{
		This->loginAnswerCode = ANS_Logged;
		return true;
	}
	
	gUserProfile.CustomerID    = 0;
	gUserProfile.SessionID     = 0;
	gUserProfile.AccountStatus = 0;

	r3dOutToLog("LoginAuth: %d\n", req.resultCode_);
	This->loginAnswerCode = ANS_BadPassword;
	return 0;
}

CLoginMenu::CLoginMenu(const char * movieName) : UIMenu(movieName), 
	m_ShowMessageIfAny(0)
{
	password[0] = 0;
	username[0] = 0;
	
	loginThread = NULL;
	loginAnswerCode = ANS_Unactive;
	m_pBackgroundTex = NULL;
	// this should actually be set in intialize. 
	hideLogin = false; 
}

CLoginMenu::~CLoginMenu()
{
	r3d_assert(loginThread == NULL);
	if(m_pBackgroundTex)
		r3dRenderer->DeleteTexture(m_pBackgroundTex);
}

void CLoginMenu::ShowErrorMessage(const wchar_t* msg) // we show that only in case if player couldn't connect to server, or was dropped from the server while playing
{
	//gfxMovie.Invoke("_root.api.showScreenLogin", "true");
	Scaleform::GFx::Value vars[2];
	vars[0].SetStringW(msg);
	vars[1].SetBoolean(true);
	gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);
}

void writeGameOptionsFile();
void CLoginMenu::eventLoginBoxLoginClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args != NULL);
	r3d_assert(pMovie != NULL);
	r3d_assert(argCount==2);

	Scaleform::GFx::Value vars[2];
	vars[0].SetStringW(gLangMngr.getString("LoginMenu_ValidatingLogin"));
	vars[1].SetBoolean(false);
	pMovie->Invoke("_root.api.showScreenAlert", vars, 2);

	r3dscpy(username, args[0].GetString());
	r3dscpy(password, args[1].GetString());
	if(g_user_save_login->GetBool())
		g_user_login_info->SetString(username);
	else
		g_user_login_info->SetString("");
	writeGameOptionsFile();

	r3d_assert(loginThread == NULL);
	loginThread = (HANDLE)_beginthreadex(NULL, 0, &LoginProcessThread, this, 0, NULL);
    if(loginThread == NULL)
        r3dError("Failed to begin thread");
}

void CLoginMenu::eventRememberLoginClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args != NULL);
	r3d_assert(pMovie != NULL);
	r3d_assert(argCount ==1);

	bool result = args[0].GetBool();
	g_user_save_login->SetBool(result);
}

void CLoginMenu::eventLoginBoxAlertOkClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	pMovie->Invoke("_root.api.showScreenLogin", "");
}

void CLoginMenu::eventLoginBoxQuitClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	pMovie->Invoke("_root.api.fadeOut", "");
	LoginQuitRequested = 1;
}

void CLoginMenu::eventFadeOut(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	LoginMenuExitFlag = 1;
}

bool CLoginMenu::Initialize()
{
	m_pBackgroundTex = r3dRenderer->LoadTexture("Data/Menu/Background.dds");
	r3d_assert(m_pBackgroundTex);

	Mouse->SetRange( r3dRenderer->HLibWin );

	LoginMenuExitFlag = 0;

	#define MAKE_CALLBACK(FUNC) \
		new r3dScaleformMovie::TGFxEICallback<CLoginMenu>(this, &CLoginMenu::FUNC)
	
	gfxMovie.RegisterEventHandler("eventLoginBoxLoginClick", MAKE_CALLBACK(eventLoginBoxLoginClick));
	gfxMovie.RegisterEventHandler("eventRememberLoginClick", MAKE_CALLBACK(eventRememberLoginClick));

	gfxMovie.RegisterEventHandler("eventLoginBoxAlertOkClick", MAKE_CALLBACK(eventLoginBoxAlertOkClick));

	gfxMovie.RegisterEventHandler("eventLoginBoxQuitClick", MAKE_CALLBACK(eventLoginBoxQuitClick));
	gfxMovie.RegisterEventHandler("eventFadeOut", MAKE_CALLBACK(eventFadeOut));
	
	#undef MAKE_CALLBACK

	if(g_user_login_info->GetString() && (g_user_login_info->GetString()[0]!='\0'))
		gfxMovie.Invoke("_root.api.setUsername", g_user_login_info->GetString());
	gfxMovie.Invoke("_root.api.setRememberLogin", g_user_save_login->GetInt());
	
	// decode passed username & passwords from RSUpdate
	if(Login_PassedUser[0] == '@') {
		CkString s;
		s = Login_PassedUser + 1;
		s.base64Decode("utf-8");
		r3dscpy(Login_PassedUser, s.getUtf8());
	} 

	if ( Login_PassedAuth[0] != '\0' )
	{
		hideLogin = true;
	}
	else 
	{
		hideLogin = false;
	}

	if(Login_PassedPwd[0] == '@') {
		CkString s;
		s = Login_PassedPwd + 1;
		s.base64Decode("utf-8");
		r3dscpy(Login_PassedPwd, s.getUtf8());
	}

	if(Login_PassedUser[0])
		gfxMovie.Invoke("_root.api.setUsername", Login_PassedUser);

	// skip everything
	gfxMovie.Invoke("_root.api.showScreenLogin", "true");

	if(m_ShowMessageIfAny)
		ShowErrorMessage(m_ShowMessageIfAny);

	menuInitTime = r3dGetTime();
	
	return true;
}
bool IsNeedExit();

void ClearFullScreen_Menu();

void CLoginMenu::CheckAnswerCode()
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
		
	// If something bad happens, start displaying the login again.   
	// This should never happen through normal use. 
	if ( loginAnswerCode != ANS_Logged )
	{
		hideLogin = false;
	}
	

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

		if ( hideLogin )
		{
			// just a quick exit here. 
			LoginMenuExitFlag = 1; 
		} 
		else 
		{
			gfxMovie.Invoke("_root.api.fadeOut", "");
		}
		
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

bool CLoginMenu::DecodeAuthParams()
{
	r3d_assert(Login_PassedAuth[0]);

	CkString s1;
	s1 = Login_PassedAuth;
	s1.base64Decode("utf-8");

	char* authToken = (char*)s1.getAnsi();
	for(size_t i=0; i<strlen(authToken); i++)
		authToken[i] = authToken[i] ^ 0x64;

	DWORD CustomerID = 0;
	DWORD SessionID = 0;
	DWORD AccountStatus = 0;
	int n = sscanf(authToken, "%d:%d:%d", &CustomerID, &SessionID, &AccountStatus);
	if(n != 3)
		return false;

	gUserProfile.CustomerID    = CustomerID;
	gUserProfile.SessionID     = SessionID;
	gUserProfile.AccountStatus = AccountStatus;
	return true;
}

int CLoginMenu::Update()
{
	{
		r3dPoint3D soundPos(0,0,0), soundDir(0,0,1), soundUp(0,1,0);
		SoundSys.Update(soundPos, soundDir, soundUp);
	}

	r3dProcessWindowMessages();

	// if we have supplied username & password, start login sequence once
	if(Login_PassedUser[0] && Login_PassedPwd[0] && r3dGetTime() > menuInitTime + 0.5f )
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(Login_PassedUser);
		args[1].SetString(Login_PassedPwd);
		eventLoginBoxLoginClick(&gfxMovie, args, 2);

		Login_PassedUser[0] = 0;
	}

	// if we have encoded login session information
	if(Login_PassedAuth[0] && r3dGetTime() > menuInitTime + 0.5f)
	{
		if(DecodeAuthParams())
		{
			Scaleform::GFx::Value vars[2];
			vars[0].SetStringW(gLangMngr.getString("LoginMenu_ValidatingLogin"));
			vars[1].SetBoolean(false);
			gfxMovie.Invoke("_root.api.showScreenAlert", vars, 2);

			r3d_assert(loginThread == NULL);
			loginThread = (HANDLE)_beginthreadex(NULL, 0, &LoginAuthThread, this, 0, NULL);
            if(loginThread == NULL)
                r3dError("Failed to begin thread");

		}
		Login_PassedAuth[0] = 0;
	}

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
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, oldScissor);
		r3dRenderer->SetViewport( (float)oldVp.X, (float)oldVp.Y, (float)oldVp.Width, (float)oldVp.Height );

		if( hideLogin == false )
		{
			gfxMovie.UpdateAndDraw();
		}

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
