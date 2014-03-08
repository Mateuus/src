#pragma once

#include "hud_base.h"

class obj_AI_Player;

class ShootingGalleryHUD : public BaseHUD
{
public:
	static gobjid_t	targetPlayerId;

public:
	ShootingGalleryHUD() 
	{ 
		targetPlayerId = invalidGameObjectID;
		cameraRayLen = 20000.0f;
	}
	
	~ShootingGalleryHUD() { }
	

	virtual	void    SetCameraDir (r3dPoint3D vPos );
	virtual r3dPoint3D GetCameraDir () const;
	
	virtual void	Process();

	virtual void	Draw();
	virtual r3dPoint3D	GetCamOffset() const;
	virtual void		SetCamPos( const r3dPoint3D& pos );
	void TargetPlayer( obj_AI_Player* targetPlayer );

protected:
	virtual	void	OnHudSelected();
	virtual void	OnHudUnselected();
	virtual void	InitPure();
	virtual void	DestroyPure();
	virtual void	SetCameraPure (r3dCamera &Cam );
private:
	float cameraRayLen;


};
