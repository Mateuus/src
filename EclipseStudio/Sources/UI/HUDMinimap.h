#pragma once

#include "APIScaleformGfx.h"

class HUDMinimap
{
private:
	r3dScaleformMovie gfxSmallMap;
	r3dScaleformMovie gfxBigMap;

public:
	HUDMinimap();
	~HUDMinimap();

	bool 	Init();
	bool	IsInited () const { return m_bInited; }
	bool 	Unload();

	bool	isShowingBigMap() const { return !m_bSmall; }

	void 	Update();
	void 	Draw();

	void	UpdateMinimapPic();

	void	SwitchMinimap();

	void	SetCameraPosition(const r3dPoint3D& pos, const r3dPoint3D& dir);
	void	AddUnit(const char* name, bool enemy, bool isNemesis, const r3dPoint3D& pos);
	void	AddControlPoint(const char* name, const r3dPoint3D& pos);
	void	AddSupplyCrate(const char* name, const char* type, const r3dPoint3D& pos);
	void	AddBomb(const char* name, const r3dPoint3D& pos);
	void	SetControlPointStatus(const char* name, const char* status, const char* tag);
	void	MoveUnit(const char* name, const r3dPoint3D& pos, bool visible=true);
	void	RotateUnit(const char* name, float angle); // 0 < angle < 1
	void	EraseUnit(const char* name);

private:
	bool m_bInited;
	bool m_bSmall;
};
