#ifndef __AIRSTRIKE_H__
#define __AIRSTRIKE_H__

#include "GameCommon.h"


struct AirstrikeDataset
{
	uint32_t	itemID; // from DB
	int		isAvailable;
	const char	*Name;

	float	Cooldown; // cooldown after using this airstrike
	float	CurrentCooldown[2]; // if this airstrike currently has a cooldown, per team
	
	float	DamageRadius;
	float   DamageRadiusProgrUI; // 0..1
	float	DamageAmount;
	float	DamageAmountProgrUI;
	
	float	Speed;

	float	TargetingTime;

	int		NumStrikes;
	float	StrikeRadius;
	float	StrikeRadiusProgrUI;
	float	BaseHeight;

	float	StrikeDelay;
	float	StrikeDelayProgrUI;

	const char* DescName;

	const char* icon;

	const char	*MeshName;
	const char	*TrailName;
	const char	*Explosion1;
	const char	*Explosion2;
	const char	*Explosion3;
	const char	*Explosion4;
	const char	*ExplosionSound;
};

#ifdef WO_SERVER
struct AirstrikeInFlight
{
	int itemID;
	float distanceTraveled;
	float heightOffset;
	float speed;
	r3dPoint3D hitPos;
	gobjid_t Creator; 
	int teamID;
	void Reset() { itemID = 0; distanceTraveled = 0; speed = 0; Creator=invalidGameObjectID; heightOffset = 0;}
	AirstrikeInFlight() {Reset();}
};
static const int MAX_AIRSTRIKES_IN_FLIGHT = 128;
extern AirstrikeInFlight g_AirstrikesInFlight[MAX_AIRSTRIKES_IN_FLIGHT];
#endif //WO_SERVER

#ifndef WO_SERVER
class obj_Airstrike : public GameObject
{
	DECLARE_CLASS(obj_Airstrike, GameObject)

public: // shared data

	const AirstrikeDataset	*ASData;

	r3dVector	m_FireDirection;
	float		m_CreationTime;
	r3dPoint3D	m_CreationPos;
	r3dPoint3D	m_AppliedVelocity;

	r3dPoint3D	m_HitDestination;

	r3dMesh* 	m_PrivateModel; // may be null
	class obj_ParticleSystem* m_ParticleTracer;


	r3dPoint3D	m_CollisionPoint;
	r3dPoint3D  	m_CollisionNormal;
	//bool		m_hasCollision;
	float		m_DistanceTraveled;
	float		m_DistanceUntilExplosion;

	r3dMesh* 	getModel();
	void 			unloadModel();

	void		onHit(const r3dPoint3D& pos, const r3dPoint3D& norm, const r3dPoint3D& fly_dir);
public:
	obj_Airstrike();

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();
	virtual	void		OnCollide(PhysicsCallbackObject *tobj, CollisionInfo &trace);
	virtual	BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  OVERRIDE;

	virtual r3dMesh* GetObjectMesh();
	virtual r3dMesh* GetObjectLodMesh() OVERRIDE;
};
extern void Airstrike_Fire(const r3dPoint3D& hitPos, int itemID, float heightOffset, int teamID);
extern void Airstrike_Spawn_Explosion(const r3dPoint3D& hitPos, int itemID);
#endif //WO_SERVER

extern int getNumAirstrikes();
extern AirstrikeDataset* getAirstrike(int i);
extern AirstrikeDataset* getAirstrikeByID(int itemID);
extern float AIRSTRIKE_Team_Cooldown[2];

#endif	// AIRSTRIKE
