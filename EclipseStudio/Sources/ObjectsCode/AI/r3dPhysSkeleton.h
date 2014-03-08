#pragma once

class obj_AI_Player;

// physics skeleton
struct ActorBone
{
	PxRigidDynamic* actor;
	int		boneID;
	ActorBone():actor(0), boneID(-1) {}
	ActorBone(PxRigidDynamic* a, int b) : actor(a), boneID(b) {}
};

class r3dPhysSkeleton
{
	friend struct RepXItemAdder;
	ActorBone* m_Bones;
	int		m_NumBones;
	int		m_CurrentBone; // for loading
	bool	m_isRagdollMode;
	//	Used for background loading completion indication
	volatile LONG isLoaded;

	bool loadSkeleton(const char* fname);
public:
	r3dPhysSkeleton( const char* fname );
	virtual ~r3dPhysSkeleton();

	void linkParent(const r3dSkeleton *skel, const D3DXMATRIX &DrawFullMatrix, GameObject *parent, PHYSICS_COLLISION_GROUPS collisionGroup);
	void unlink() ;

	void syncAnimation(r3dSkeleton *skel, const D3DXMATRIX &DrawFullMatrix, r3dAnimation &anim);
	void SwitchToRagdoll(bool toRagdoll);
	void SwitchToRagdollWithForce(bool toRagdoll, int boneId, const r3dPoint3D& force);
	bool IsRagdollMode() const { return m_isRagdollMode; }
	r3dBoundBox getWorldBBox() const;
	void TogglePhysicsSimulation(bool on);
};

