#include <windows.h>
#include <sstream>


class LauncherLogic
{
public:
	LauncherLogic();
	~LauncherLogic();

	//return true if trying to launch the game
	//false, if it was an attempt to install
	//if getLastError return 0, then all succeded
	bool login(HINSTANCE hInst, const wchar_t* cmdLine);


	bool tick();


	const wchar_t* getLastError() const;	//live until class instance lives

protected:
	std::wstring msg, cmdLine;
	std::wstringstream retss;
	std::wstringstream tempFileName;
	const wchar_t* errorMsg;
	HINSTANCE hInstance;

	void setError(const wchar_t* err=0);
	void setNotification(const wchar_t* err=0);

	bool isInstalled;
	wchar_t InstallPath[MAX_PATH];
	bool fillInstallPath();	//false if error occured, fills isInstalled and InstallPath if installed


	bool launchGame();

};