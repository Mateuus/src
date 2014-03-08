#include "r3dPCH.h"
#include "r3d.h"

#include "HUDMinimap.h"

#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/ai/AI_Player.H"
#include "GameLevel.h"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
HUDMinimap::HUDMinimap()
: m_bInited ( false )
, m_bSmall ( true )
{
}

HUDMinimap::~HUDMinimap()
{
}

bool HUDMinimap::Init()
{
	if(!gfxSmallMap.Load("Data\\Menu\\HUD_minimap_small.swf", false)) 
		return false;
	if(!gfxBigMap.Load("Data\\Menu\\HUD_minimap_big.swf", false)) 
		return false;

	Scaleform::GFx::Value vars[2];
	vars[0].SetNumber(r3dRenderer->ScreenW);
	vars[1].SetNumber(r3dRenderer->ScreenH);
	gfxSmallMap.Invoke("_global.arrangeMap", vars, 2);
	gfxBigMap.Invoke("_global.arrangeMap", vars, 2);

	Scaleform::GFx::Viewport vp;
	gfxSmallMap.GetMovie()->GetViewport(&vp);
	gfxSmallMap.GetMovie()->SetViewScaleMode(Scaleform::GFx::Movie::SM_ExactFit);
	float scale = (r3dRenderer->ScreenW/1920.0f);
	int bufWidth = int(float(vp.BufferWidth) * scale);
	int bufHeight = int(float(vp.BufferHeight) * scale);
	vp.Width = bufWidth;
	vp.Height = bufHeight;
	vp.Left = (int)r3dRenderer->ScreenW-bufWidth;
	vp.BufferWidth += vp.Left;
	gfxSmallMap.GetMovie()->SetViewport(vp);

	gfxBigMap.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
//	gfxSmallMap.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_TopRight);
	gfxBigMap.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_Center);


	gfxSmallMap.SetVariable("_global.MapX", 0.5f );
	gfxSmallMap.SetVariable("_global.MapY", 0.5f );
	gfxBigMap.SetVariable("_global.MapX", 0.5f );
	gfxBigMap.SetVariable("_global.MapY", 0.5f );

	m_bInited = true;

	UpdateMinimapPic();

	m_bSmall = true;

	return true;
}

bool HUDMinimap::Unload()
{
	if(m_bInited)
	{
		gfxSmallMap.Unload();
		gfxBigMap.Unload();
	}
	m_bInited = false;
	return true;
}

void HUDMinimap::Update()
{
	gfxBigMap.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
}

void HUDMinimap::Draw()
{
	float X, Y, W, H;
	r3dRenderer->GetBackBufferViewport( &X, &Y, &W, &H );
	float fY = r3dRenderer->ScreenH / H;
	float fX = r3dRenderer->ScreenW / W;
	r3dScaleformSetUserMatrix(fX, fY, 0, 0);

	gfxSmallMap.UpdateAndDraw(!m_bSmall);
	r3dScaleformSetUserMatrix();

	gfxBigMap.UpdateAndDraw(m_bSmall);
}

void HUDMinimap::UpdateMinimapPic ()
{ 
	if(m_bInited)
	{
		char sFullPath[512];
		char sFullPathImg[512];
		sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap.dds");
		sprintf(sFullPathImg, "$%s", sFullPath); // use '$' chas to indicate absolute path

		if(r3dFileExists(sFullPath))
		{		
			gfxSmallMap.Invoke("_global.ImagePath", sFullPathImg );
			gfxBigMap.Invoke("_global.ImagePath", sFullPathImg );
		}
	}
}

void HUDMinimap::SwitchMinimap()
{
// 	if(m_bSmall) // switch to big
// 	{
// 		gfxMovie.Invoke("_global.BigMapOn", "");
// 		gfxMovie.Invoke("_global.MiniMapOff", "");
// 	}
// 	else // switch to small
// 	{
// 		gfxMovie.Invoke("_global.BigMapOff", "");
// 		gfxMovie.Invoke("_global.MiniMapOn", "");
// 	}
	m_bSmall = !m_bSmall;
}

r3dPoint2D getMinimapPos(const r3dPoint3D& pos)
{
	r3dPoint3D worldOrigin = GameWorld().m_MinimapOrigin;
	r3dPoint3D worldSize = GameWorld().m_MinimapSize;
	float left_corner_x = worldOrigin.x;
	float bottom_corner_y = worldOrigin.z; 
	float x_size = worldSize.x;
	float y_size = worldSize.z;

	float x = R3D_CLAMP((pos.x-left_corner_x)/x_size, 0.0f, 1.0f);
	float y = 1.0f-R3D_CLAMP((pos.z-bottom_corner_y)/y_size, 0.0f, 1.0f);
	
	return r3dPoint2D(x, y);
}

void HUDMinimap::SetCameraPosition(const r3dPoint3D& pos, const r3dPoint3D& dir)
{
	r3dPoint2D mapPos = getMinimapPos(pos);
	gfxSmallMap.SetVariable("_global.MapX", mapPos.x );
	gfxSmallMap.SetVariable("_global.MapY", mapPos.y );
	gfxBigMap.SetVariable("_global.MapX", mapPos.x );
	gfxBigMap.SetVariable("_global.MapY", mapPos.y );
	gfxSmallMap.SetVariable("_global.PlayerPositionBigMapX", mapPos.x );
	gfxSmallMap.SetVariable("_global.PlayerPositionBigMapY", mapPos.y );
	gfxBigMap.SetVariable("_global.PlayerPositionBigMapX", mapPos.x );
	gfxBigMap.SetVariable("_global.PlayerPositionBigMapY", mapPos.y );

	// calculate rotation around Y axis
	r3dPoint3D d = dir;
	d.y = 0;
	d.Normalize();
	float dot1 = d.Dot(r3dPoint3D(0,0,1)); // north
	float dot2 = d.Dot(r3dPoint3D(1,0,0));
	float deg = acosf(dot1);
	deg = R3D_RAD2DEG(deg);
	if(dot2<0) 
		deg = 360 - deg;
	deg = R3D_CLAMP(deg, 0.0f, 360.0f);
	//deg = deg/360.0f;
	gfxSmallMap.SetVariable("_global.PlayerRotation", deg);
	gfxBigMap.SetVariable("_global.PlayerRotation", deg);
}

void HUDMinimap::AddUnit(const char* name, bool enemy, bool isNemesis, const r3dPoint3D& pos)
{
	Scaleform::GFx::Value args[3];
	args[0].SetString(name);
	if(enemy)
		args[1].SetString("red");
	else
		args[1].SetString("blue");
	args[2].SetBoolean(isNemesis);

	gfxSmallMap.Invoke("_global.AddUnit", args, 3);
	gfxBigMap.Invoke("_global.AddUnit", args, 3);
	
	MoveUnit(name, pos);
}

void HUDMinimap::AddControlPoint(const char* name, const r3dPoint3D& pos)
{
	gfxSmallMap.Invoke("_global.AddControlPoint", name);
	gfxBigMap.Invoke("_global.AddControlPoint", name);
	MoveUnit(name, pos);
}

void HUDMinimap::AddBomb(const char* name, const r3dPoint3D& pos)
{
	gfxSmallMap.Invoke("_global.AddBomb", name);
	gfxBigMap.Invoke("_global.AddBomb", name);
	MoveUnit(name, pos);
}

void HUDMinimap::AddSupplyCrate(const char* name, const char* type, const r3dPoint3D& pos)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(name);
	var[1].SetString(type);
	gfxSmallMap.Invoke("_global.AddSupplyCrate", var, 2);
	gfxBigMap.Invoke("_global.AddSupplyCrate", var, 2);
	MoveUnit(name, pos);
}

void HUDMinimap::SetControlPointStatus(const char* name, const char* status, const char* tag)
{
	Scaleform::GFx::Value args[3];
	args[0].SetString(name);
	args[1].SetString(status);
	args[2].SetString(tag==NULL?"":tag);
	gfxSmallMap.Invoke("_global.ControlPointStatus", args, 3);
	gfxBigMap.Invoke("_global.ControlPointStatus", args, 3);
}

void HUDMinimap::MoveUnit(const char* name, const r3dPoint3D& pos, bool visible)
{
	r3dPoint2D mapPos = getMinimapPos(pos);
	Scaleform::GFx::Value args[4];
	args[0].SetString(name);
	args[1].SetNumber(mapPos.x);
	args[2].SetNumber(mapPos.y);
	args[3].SetBoolean(visible);
	gfxSmallMap.Invoke("_global.UnitMove", args, 4);
	gfxBigMap.Invoke("_global.UnitMove", args, 4);
}

void HUDMinimap::RotateUnit(const char* name, float angle)
{
	Scaleform::GFx::Value args[2];
	args[0].SetString(name);
	args[1].SetNumber(angle);
	gfxSmallMap.Invoke("_global.UnitRotate", args, 2);
	gfxBigMap.Invoke("_global.UnitRotate", args, 2);
}

void HUDMinimap::EraseUnit(const char* name)
{
	gfxSmallMap.Invoke("_global.UnitErase", name);
	gfxBigMap.Invoke("_global.UnitErase", name);
}
