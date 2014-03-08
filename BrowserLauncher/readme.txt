Place WarInc_WebSetup.exe into this directory

Don't forget place into RC-file:
WIInstaller RCDATA "../WarInc_WebSetup.exe"

include "logic.cpp" into your project and declare:
bool startLauncher(HINSTANCE hInst, const wchar_t** msg);
bool tickLauncher(const wchar_t** msg);

enjoy Executable sample