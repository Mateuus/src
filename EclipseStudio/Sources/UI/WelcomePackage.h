#ifndef __WELCOME_PACKAGE_H__
#define __WELCOME_PACKAGE_H__

#include "UIMenu.h"

class WelcomePackageMenu : public UIMenu
{
private:
	friend void fsSelectPackage2(void* data, r3dScaleformMovie *pMovie, const char *arg);

public:
	enum {
		RET_Unknown = 0,
		RET_Exit,
		RET_Done,
	};

	WelcomePackageMenu(const char * movieName) : UIMenu(movieName) {}
	virtual ~WelcomePackageMenu() {};

	virtual bool Initialize();
	virtual int Update();

private:
	bool finished;
};

#endif  //__WELCOME_PACKAGE_H__
