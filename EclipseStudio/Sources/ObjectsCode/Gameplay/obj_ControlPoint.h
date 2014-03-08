#pragma once

#include "GameCommon.h"
#include "BaseControlPoint.h"

#include "APIScaleformGfx.h"

class obj_ControlPoint : public BaseControlPoint
{
	DECLARE_CLASS(obj_ControlPoint, BaseControlPoint)
private:
	class r3dParticleSystem*	Torch;
	r3dMesh*	flagMesh_[3];
	r3dSkeleton*	flagSkels_[3];
	int				flagSkelShared_[3];
	r3dAnimation	flagAnimations_[3];
	r3dAnimPool		animPool_;

	Scaleform::GFx::Value m_CPIcon;
    char ControlName[32];
	const char* UITagName;
	const char* UITagNameFull;
	const wchar_t* UITagNameLong;
	int		needToAddToMinimap;

	float	showCaptureProgressTimer;
	bool	CaptureProgressVisible;

	int		m_SelectedSpawnPoint;

	bool	m_HUDCPVisible;
	float	m_HUDCPDist;


public:
	void	SetControlPointStatusOnMinimap();

	const char* getUITagName() const { return UITagName; }

	obj_ControlPoint();
	virtual ~obj_ControlPoint();

	inline r3dParticleSystem* GetTorch() const { return Torch; }

	void DoDrawComposite( const r3dCamera& Cam );

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

	void				DrawFlagMesh(eRenderStageID  DrawState);

	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	int					GetFlagIdx();
};
