#ifndef CLoginMenu_h
#define CLoginMenu_h

#include "UIMenu.h"


class CLoginMenu : public UIMenu
{
public:
	enum {
	  RET_Unknown = 0,
	  RET_Exit,
	  RET_Logged,
	};
	
	CLoginMenu(const char* movieName);
	virtual ~CLoginMenu();

	virtual bool Initialize();
	virtual int Update();
	
	const wchar_t* m_ShowMessageIfAny;
private:
	static unsigned int WINAPI LoginProcessThread(void* in_data);
	static unsigned int WINAPI LoginAuthThread(void* in_data);
	HANDLE	loginThread;
	
	enum {
	  ANS_Unactive,
	  ANS_Processing,
	  ANS_Timeout,
	  ANS_Error,
	  
	  ANS_Logged,
	  ANS_BadPassword,
	  ANS_Frozen,
	};
	volatile DWORD loginAnswerCode;
	void	CheckAnswerCode();

	r3dTexture*	m_pBackgroundTex;

	char	password[128];
	char	username[128];

	float	menuInitTime;	// time when menu was initialized
	
	void	ShowErrorMessage(const wchar_t* msg);
	bool	DecodeAuthParams();

	void	eventLoginBoxLoginClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLoginBoxAlertOkClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLoginBoxQuitClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFadeOut(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRememberLoginClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

private:
	bool	hideLogin;
};


#endif  //CLoginMenu_h
