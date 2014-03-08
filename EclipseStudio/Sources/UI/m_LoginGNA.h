#pragma once

#include "UIMenu.h"

class CLoginGNAMenu : public UIMenu
{
public:
	enum {
	  RET_Unknown = 0,
	  RET_Exit,
	  RET_Logged,
	};
	
	CLoginGNAMenu(const char* movieName);
	virtual ~CLoginGNAMenu();

	virtual bool Initialize();
	virtual int Update();
	
	const wchar_t* m_ShowMessageIfAny;
private:
	static unsigned int WINAPI LoginGNAProcessThread(void* in_data);
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
	bool	loginStarted_;

	float	menuInitTime;	// time when menu was initialized
	
	void	ShowErrorMessage(const wchar_t* msg);

	void	eventLoginBoxAlertOkClick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventFadeOut(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
};
