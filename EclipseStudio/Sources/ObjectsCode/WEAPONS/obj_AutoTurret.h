#pragma  once

#include "GameCommon.h"

class obj_AutoTurret: public GameObject
{
	DECLARE_CLASS(obj_AutoTurret, GameObject)
	friend struct AutoTurretDeferredRenderable;
public:
	obj_AutoTurret();
	virtual ~obj_AutoTurret();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();
	virtual	BOOL		OnNetReceive(DWORD EventID, const void* packetData, int packetSize);

	virtual void		AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) OVERRIDE;
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh*	GetObjectMesh();
	virtual r3dMesh*	GetObjectLodMesh() OVERRIDE;

	uint32_t			m_ItemID;
	float				m_RotX;

protected:
	r3dSkeleton* m_Skeleton;
	r3dMesh* m_PrivateModel;
private:
	int			m_TurretBoneID;

	const WeaponConfig* m_WpnConfig;

	class obj_ParticleSystem* m_MuzzleParticle;
	r3dLight	m_MuzzleLight;

	void*	m_sndNewFire;

	r3dPoint3D muzzlerPoint;
	r3dPoint3D muzzlerDir;

	float		_nextScan;

	// gameplay params
	float m_Spread;
	float m_AngleMax;
	float m_AngleSpeed;
	float m_Range;
	float m_FireDelay;

	float m_LastFireTime;

	// 
	float m_CurAngle;
	float m_CurAngleTarget;
	gobjid_t m_TargetID;
};
