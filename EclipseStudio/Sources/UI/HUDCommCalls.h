#pragma once

#include "APIScaleformGfx.h"

class HUDCommCalls
{
	struct CallCommand
	{
		int id;
		const wchar_t* txtCommand;
		int voiceCmd;
		CallCommand() :id(-1), txtCommand(0), voiceCmd(0) {}
		CallCommand(int i, const wchar_t* txt, int voice) : id(i), txtCommand(txt), voiceCmd(voice) {}
	};

	CallCommand m_CommandsZ[9];
	CallCommand m_CommandsX[9];
	CallCommand m_CommandsC[9];
public:
	r3dScaleformMovie gfxMovie;

	r3dScaleformMovie gfxCommRose;

public:
	HUDCommCalls();
	~HUDCommCalls();

	bool 	Init();
	bool	IsInited () const { return m_bInited; }
	bool 	Unload();

	void 	Update();
	void 	Draw();

	bool	isVisible() const { return m_bVisible; }
	bool	isCommRoseVisible() const { return m_bCommRoseVisible; }

	void	PlayMessage(class obj_AI_Player* from, int id); // play voice over, send message to chat

	void	showHUDIcon(class obj_AI_Player* from, int id, const r3dVector& pos);
private:
	void	eventPressButton(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);


	void	ShowMessages(int msgType);
	void	HideMessages();
	bool m_bInited;
	bool m_bVisible;
	int  m_CurrentMsgType;

	bool m_bCommRoseVisible;
};
