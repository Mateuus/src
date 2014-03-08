#ifndef	__R3D_INPUT_H
#define	__R3D_INPUT_H
#include "xinput.h"

// defined classes
class r3dMouse;
class r3dKeyboard;
class r3dGamepad;

class r3dMouse
{
public:
	enum MouseButtons
	{
		mLeftButton		= 0,
		mRightButton,
		mCenterButton,
		mWheelButton,
		//	This value must be last in enumeration
		mTotalButtons
	};

public:
	int m_MouseMoveX; // updated in Input_Update every frame, use it instead of QueryMotionDistance, as two calls to QueryMotionDistance in the same frame will result in move 0,0 for the second call
	int m_MouseMoveY;

	r3dMouse();
	~r3dMouse();

	int		SetCapture();
	void		ReleaseCapture();

	bool   	IsPressed(int key);		// if key is pressed
	bool	WasPressed(int key);		// if key was pressed from last call to this func
	bool	WasReleased(int key);		// if key was released from last call to this func
	void	ClearPressed();			// clear pressed buffer

	WORD	GetRawMouseData();

	void            SetRange(int x1, int y1, int x2, int y2);
	void            SetRange(HWND Wnd);

	void            MoveTo(int X, int Y);
	void            GetXY(int &X, int &Y);
	void            QueryMotionDistance(int *iHoriz, int *iVert); // do not call this function, use m_MouseMoveX, m_MouseMoveY;
	void            QueryWheelMotion(int *iWheel);

	static	bool		GetMouseVisibility();
	static	void            Show(bool force = false);
	static	void            Hide(bool force = false);
};

//
//
//
//
//
class r3dKeyboard
{
  public:
			r3dKeyboard();
			~r3dKeyboard();

	int    		SetCapture();
	void   		ReleaseCapture();

	bool    IsPressed(int key);		// if key is pressed
	bool	WasPressed(int key);		// if key was pressed from last call to this func
	bool	WasReleased(int key);		// if key was released from last call to this func
	int		Reset();			// reset keyboard data
	int		ClearPressed();			// clear pressed buffer
};

enum r3dKeyboardScanCodes_e {
  kbsEsc                = DIK_ESCAPE,
  kbs1                  = DIK_1,
  kbs2                  = DIK_2,
  kbs3                  = DIK_3,
  kbs4                  = DIK_4,
  kbs5                  = DIK_5,
  kbs6                  = DIK_6,
  kbs7                  = DIK_7,
  kbs8                  = DIK_8,
  kbs9                  = DIK_9,
  kbs0                  = DIK_0,
  kbsMinus              = DIK_MINUS,
  kbsEquals             = DIK_EQUALS,
  kbsBACK               = DIK_BACK,
  kbsTab                = DIK_TAB,
  kbsQ                  = DIK_Q,
  kbsW                  = DIK_W,
  kbsE                  = DIK_E,
  kbsR                  = DIK_R,
  kbsT                  = DIK_T,
  kbsY                  = DIK_Y,
  kbsU                  = DIK_U,
  kbsI                  = DIK_I,
  kbsO                  = DIK_O,
  kbsP                  = DIK_P,
  kbsLeftBracket        = DIK_LBRACKET,
  kbsRightBracket       = DIK_RBRACKET,
  kbsEnter              = DIK_RETURN,
  kbsLeftControl        = DIK_LCONTROL,
  kbsA                  = DIK_A,
  kbsS                  = DIK_S,
  kbsD                  = DIK_D,
  kbsF                  = DIK_F,
  kbsG                  = DIK_G,
  kbsH                  = DIK_H,
  kbsJ                  = DIK_J,
  kbsK                  = DIK_K,
  kbsL                  = DIK_L,
  kbsSemiColon          = DIK_SEMICOLON,
  kbsApostrophe         = DIK_APOSTROPHE,
  kbsGrave              = DIK_GRAVE,
  kbsLeftShift          = DIK_LSHIFT,
  kbsBackSlash          = DIK_BACKSLASH,
  kbsZ                  = DIK_Z,
  kbsX                  = DIK_X,
  kbsC                  = DIK_C,
  kbsV                  = DIK_V,
  kbsB                  = DIK_B,
  kbsN                  = DIK_N,
  kbsM                  = DIK_M,
  kbsComma              = DIK_COMMA,
  kbsPeriod             = DIK_PERIOD,
  kbsSlash              = DIK_SLASH,
  kbsRightShift         = DIK_RSHIFT,
  kbsPrtScr             = DIK_SYSRQ,
  kbsLeftAlt            = DIK_LMENU,
  kbsSpace              = DIK_SPACE,
  kbsCapsLock           = DIK_CAPITAL,
  kbsF1                 = DIK_F1,
  kbsF2                 = DIK_F2,
  kbsF3                 = DIK_F3,
  kbsF4                 = DIK_F4,
  kbsF5                 = DIK_F5,
  kbsF6                 = DIK_F6,
  kbsF7                 = DIK_F7,
  kbsF8                 = DIK_F8,
  kbsF9                 = DIK_F9,
  kbsF10                = DIK_F10,
  kbsF11                = DIK_F11,
  kbsF12                = DIK_F12,
  kbsNumLock            = DIK_NUMLOCK,
  kbsScrollLock         = DIK_SCROLL,
  kbsNumericHome        = DIK_NUMPAD7,
  kbsNumericUp          = DIK_NUMPAD8,
  kbsNumericPgUp        = DIK_NUMPAD9,
  kbsGrayMinus          = DIK_SUBTRACT,
  kbsNumericLeft        = DIK_NUMPAD4,
  kbsNumericFive        = DIK_NUMPAD5,
  kbsNumericRight       = DIK_NUMPAD6,
  kbsGrayPlus           = DIK_ADD,
  kbsNumericEnd         = DIK_NUMPAD1,
  kbsNumericDown        = DIK_NUMPAD2,
  kbsNumericPgDn        = DIK_NUMPAD3,
  kbsNumericIns         = DIK_NUMPAD0,
  kbsNumericDel         = DIK_DECIMAL,
  kbsNumericEnter       = DIK_NUMPADENTER,
  
  kbsRightAlt           = DIK_RMENU,
  kbsRightControl       = DIK_RCONTROL,
  kbsGraySlash          = DIK_DIVIDE,
  kbsGrayHome           = DIK_HOME,

  kbsUp                 = DIK_UP,
  kbsDown               = DIK_DOWN,
  kbsLeft               = DIK_LEFT,
  kbsRight              = DIK_RIGHT,
  kbsHome               = DIK_HOME,
  kbsEnd                = DIK_END,
  kbsPgUp               = DIK_PRIOR,
  kbsPgDn               = DIK_NEXT,
  kbsIns                = DIK_INSERT,
  kbsDel                = DIK_DELETE,
};


char *r3dGetKeyName(int KeyCode);

enum GamepadStates_e
{
	gpPadUP	= XINPUT_GAMEPAD_DPAD_UP,
	gpPadDown	= XINPUT_GAMEPAD_DPAD_DOWN,
	gpPadLeft	= XINPUT_GAMEPAD_DPAD_LEFT,
	gpPadRight	= XINPUT_GAMEPAD_DPAD_RIGHT,
	gpStart	= XINPUT_GAMEPAD_START,
	gpBack		= XINPUT_GAMEPAD_BACK,
	gpLeftThumb	= XINPUT_GAMEPAD_LEFT_THUMB,
	gpRightThumb	= XINPUT_GAMEPAD_RIGHT_THUMB,
	gpLeftShoulder	= XINPUT_GAMEPAD_LEFT_SHOULDER,
	gpRightShoulder= XINPUT_GAMEPAD_RIGHT_SHOULDER,
	gpA		= XINPUT_GAMEPAD_A,
	gpB		= XINPUT_GAMEPAD_B,
	gpX		= XINPUT_GAMEPAD_X,
	gpY		= XINPUT_GAMEPAD_Y,
	gpGreen	= XINPUT_GAMEPAD_A,
	gpRed		= XINPUT_GAMEPAD_B,
	gpBlue		= XINPUT_GAMEPAD_X,
	gpYellow	= XINPUT_GAMEPAD_Y,
};

// for now gamepag supports only one controller.
class r3dGamepad
{
private:
	XINPUT_STATE 	State;
	XINPUT_STATE	PrevState;
	bool			isConnected;

	// thumb will be in [-1.0,1.0] range, trigger in [0,1]
	float			leftTrigger;
	float			rightTrigger;
	float			leftThumbX;
	float			leftThumbY;
	float			rightThumbX;
	float			rightThumbY;

	void		GetState();
public:
	r3dGamepad() { Reset(); }; // XInputEnable(1); }
	~r3dGamepad() {}; // XInputEnable(0); }

	void		Reset();
	void		Update();

	bool		IsConnected() const { return isConnected; }

	bool    IsPressed(int key);		// if key is pressed
	bool	WasPressed(int key);		// if key was pressed from last call to this func
	bool	WasReleased(int key);		// if key was released from last call to this func

	void		GetTrigger(float &Left, float &Right);
	void		GetLeftThumb(float &X, float &Y);
	void		GetRightThumb(float &X, float &Y);
};

// maps shortcuts to actual keyboard or mouse
class r3dInputMappingMngr
{
public:
	enum KeybordShortcuts
	{
		KS_PRIMARY_FIRE = 0,
		KS_AIM,

		KS_MOVE_FORWARD,
		KS_MOVE_BACKWARD,
		KS_MOVE_LEFT,
		KS_MOVE_RIGHT,
		KS_SPRINT,
		KS_JUMP,
		KS_CROUCH,
		KS_INTERACT,
		KS_RELOAD,
		KS_CHANGE_RATE_FIRE,
		KS_HOLD_BREATH, 

		KS_PRIMARY_WEAPON,
		KS_SECONDARY_WEAPON,
		KS_HANDGUN_WEAPON,
		KS_ITEM1,
		KS_ITEM2,

		KS_SWITCH_MINIMAP,

		KS_CHAT,
		KS_SHOW_SCORE,

		KS_QUICK_SWITCH_WEAPON,
		KS_THROW_GRENADE,

		KS_TOGGLE_NIGHTVISION,

		KS_DROP_BOMB,

		KS_UAV_TARGET,
		KS_UAV_UP,
		KS_UAV_DOWN,

		KS_COMMAND_Z,
		KS_COMMAND_X,
		KS_COMMAND_C,

		KS_COMMROSE,

		KS_LASER_VIEW_UP,
		KS_LASER_VIEW_DOWN,

		KS_ITEM3,
		KS_ITEM4,
		
		KS_NEXTITEM,

		// append new mappings ONLY at the end of file, to make it easier to read previous versions

		KS_NUM,
	};

	enum InputMapType
	{
		INPUTMAP_INVALID=-1,
		INPUTMAP_KEYBOARD = 0,
		INPUTMAP_MOUSE,
	};

public:
	r3dInputMappingMngr();
	~r3dInputMappingMngr();

	bool isPressed(KeybordShortcuts shortcut);
	bool wasPressed(KeybordShortcuts shortcut);
	bool wasReleased(KeybordShortcuts shortcut);

	// map shortcut to mouse/keyboard
	void mapKey(KeybordShortcuts shortcut, InputMapType type, int key);

	const char* getMapName(KeybordShortcuts shortcut);
	const char* getKeyName(KeybordShortcuts shortcut);

	// will query input devices for input and if any will remap and return true, otherwise false
	bool attemptRemapKey(KeybordShortcuts shortcut);

	void resetKeyMappingsToDefault();

	// save/load mapping
	void loadMapping(const char* path);
	void saveMapping(const char* path);
private:
	struct KeyboardMapping
	{
		InputMapType	type;
		int		default_key;
		int		key; // maps to actual keyboard or mouse key
		const char* name; // name this we will show in UI
		KeyboardMapping():type(INPUTMAP_INVALID), key(0), default_key(0), name(0) {}
		KeyboardMapping(InputMapType t, int k, const char* n):type(t), key(k), default_key(k), name(n) {}
	};

	KeyboardMapping	m_Mapping[KS_NUM];
};


#endif 	// __R3D_INPUT_H