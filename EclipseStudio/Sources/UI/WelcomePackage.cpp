#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "WelcomePackage.h"
#include "GameCode\UserProfile.h"
#include "..\ObjectsCode\weapons\WeaponArmory.h"
#include "FrontEndShared.h"

void fsSelectPackage2(void* data, r3dScaleformMovie *pMovie, const char *arg)
{
	int specID;
	int nargs = sscanf(arg, "%d", &specID);
	if(nargs != 1) r3dError("fsSelectPackage2 wrong args: %s\n", arg);

	gUserProfile.WelcomePackageProcess(specID);

	WelcomePackageMenu* This = (WelcomePackageMenu*)data;
	This->finished = true;
}

bool WelcomePackageMenu::Initialize()
{
	gfxMovie.RegisterEventHandler("SELECTPACKAGE2", this, &fsSelectPackage2);

	extern int RUS_CLIENT;
	if(RUS_CLIENT)
		gfxMovie.Invoke("_root.Main.gotoAndPlay", "screen03");

	finished = false;

	return true;
}

bool IsNeedExit();
void ClearFullScreen_Menu();

int WelcomePackageMenu::Update()
{
	r3dProcessWindowMessages();

	if(IsNeedExit())
		return RET_Exit;

	r3dMouse::Show();
	r3dStartFrame();

	r3dRenderer->StartRender(1);
	r3dRenderer->StartFrame();

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
	ClearFullScreen_Menu();

	gfxMovie.UpdateAndDraw();

	r3dRenderer->Flush();  
	r3dRenderer->EndFrame();
	r3dRenderer->EndRender( true );

	r3dEndFrame();

	if(!finished)
		return 0;

	return RET_Done;
}