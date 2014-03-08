#include "r3dPCH.h"
#include "r3d.h"


#include "GameCommon.h"
#include "AI_Player.H"

#include "r3dPhysSkeleton.h"

#include "extensions/PxRigidBodyExt.h"

#include "PhysX\RepX\include\RepX.h"
#include "PhysX\RepX\include\RepXUtility.h"
#include "PhysX\PhysXAPI\extensions\PxStringTableExt.h"
#include "PhysX\PxFoundation\internal\PxIOStream\public\PxFileBuf.h"
#include "../../../../GameEngine/gameobjects/PhysXRepXHelpers.h"
#include "r3dBackgroundTaskDispatcher.h"
#include "../../multiplayer/ClientGameLogic.h"

extern MyPhysXAllocator myPhysXAllocator;

static class physx::repx::RepXCollection* m_sCollection = NULL;
static class PxStringTable* m_sStringTable = NULL;
static int m_sCollectionRef = 0;

namespace
{
	bool gBonesSkipUpdateFlagsArr[0x100] = {0};
} // unnamed namespace

//////////////////////////////////////////////////////////////////////////

struct RepXItemAdder
{
	PxScene* mScene;
	r3dPhysSkeleton* m_Skeleton;

	RepXItemAdder( PxScene* inScene, r3dPhysSkeleton* skel)
	: mScene( inScene ), m_Skeleton(skel)
	{
	}
	void operator()( const void* inId, PxConvexMesh* ) {}
	void operator()( const void* inId, PxTriangleMesh* ) {}
	void operator()( const void* inId, PxHeightField* ) {}
	void operator()( const void* inId, PxClothFabric* ) {}
	void operator()( const void* inId, PxMaterial* ) {}
	void operator()( const void* inId, PxRigidStatic* inActor ) 
	{
		r3d_assert(false); // rag doll should be dynamic only, if you have static than you are trying to import static object
		//mScene->addActor( *inActor ); 
	}

	void operator()( const void* inId, PxRigidDynamic* inActor ) 
	{ 
		if (!inActor)
			return;

		// pre notify
		PxShape* shapes[64] = {0};
		inActor->getShapes(&shapes[0], 64);

		D3DXQUATERNION test;
		D3DXQuaternionRotationYawPitchRoll(&test, 0, D3DX_PI / 2, 0); // rotX
		PxQuat rotQ(test.x, test.y, test.z, test.w);

		for (uint32_t i = 0; i < inActor->getNbShapes(); ++i)
		{
			PxTransform pose = shapes[i]->getLocalPose();
			pose.q = rotQ * pose.q;
			std::swap(pose.p.y, pose.p.z);

			shapes[i]->setLocalPose(pose);
			shapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		}

		PxTransform actorMassPose = inActor->getCMassLocalPose();
		std::swap(actorMassPose.p.y, actorMassPose.p.z);
		inActor->setCMassLocalPose(actorMassPose);

		m_Skeleton->m_Bones[m_Skeleton->m_CurrentBone] = ActorBone(inActor, -1);
		m_Skeleton->m_CurrentBone++;

		// on add
		r3d_assert(m_Skeleton->m_CurrentBone<=m_Skeleton->m_NumBones);

		// not sure if that is too much mass, or if it should be set from Max, but otherwise PhysX outputs a lot of warnings
		//PxRigidBodyExt::setMassAndUpdateInertia(*inActor, 10.0f);

		inActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
		inActor->setSleepThreshold(0.1f);

		mScene->addActor(*inActor);
	}

	void operator()( const void* inId, PxArticulation* inArticulation ) { }
	void operator()( const void* inId, PxCloth* inData ) {}
	void operator()( const void* inId, PxJoint* inJoint ) 
	{
		PxTransform p0 = inJoint->getLocalPose(PxJointActorIndex::eACTOR0);
		PxTransform p1 = inJoint->getLocalPose(PxJointActorIndex::eACTOR1);

		std::swap(p0.q.y, p0.q.z);
		std::swap(p1.q.y, p1.q.z);
		p0.q.y *= -1;
		p0.q.z *= -1;
		p0.q.x *= -1;
		p1.q.y *= -1;
		p1.q.z *= -1;
		p1.q.x *= -1;

		std::swap(p0.p.y, p0.p.z);
		std::swap(p1.p.y, p1.p.z);

		inJoint->setLocalPose(PxJointActorIndex::eACTOR0, p0);
		inJoint->setLocalPose(PxJointActorIndex::eACTOR1, p1);

		PxD6Joint *j = inJoint->is<PxD6Joint>();
		if (j)
		{
			PxJointLimitCone lc = j->getSwingLimit();
			std::swap(lc.yAngle, lc.zAngle);
			j->setSwingLimit(lc);

			PxD6Motion::Enum s1 = j->getMotion(PxD6Axis::eSWING1);
			PxD6Motion::Enum s2 = j->getMotion(PxD6Axis::eSWING2);
			j->setMotion(PxD6Axis::eSWING1, s2);
			j->setMotion(PxD6Axis::eSWING2, s1);
		}
	}

};


//////////////////////////////////////////////////////////////////////////

r3dPhysSkeleton::r3dPhysSkeleton( const char* fname )
: m_isRagdollMode(false)
, isLoaded(0)
{
	m_Bones = 0;
	m_NumBones = 0;
	m_CurrentBone = 0;

	loadSkeleton(fname);
}

r3dPhysSkeleton::~r3dPhysSkeleton()
{
	r3d_assert(m_sCollectionRef>0);
	--m_sCollectionRef;
	if(m_sCollectionRef == 0)
	{
		m_sCollection->destroy();
		m_sCollection = NULL;

		m_sStringTable->release();
		m_sStringTable = NULL;
	}

	for(int i=0; i<m_NumBones; ++i)
	{
		m_Bones[i].actor->userData = NULL;
		m_Bones[i].actor->release();
	}
	SAFE_DELETE_ARRAY(m_Bones);
}

bool r3dPhysSkeleton::loadSkeleton(const char* fname)
{
	R3DPROFILE_FUNCTION("r3dPhysSkeleton::loadSkeleton");

	r3dCSHolder block( g_pPhysicsWorld->GetConcurrencyGuard() ) ;

	if(m_sCollectionRef == 0)
	{
		m_sStringTable = &PxStringTableExt::createStringTable( myPhysXAllocator );
		m_sCollection = loadCollection( fname, g_pPhysicsWorld->PhysXSDK->getFoundation().getAllocator() );
	}
	++m_sCollectionRef;

	int numActors = 0;
	for(const physx::repx::RepXCollectionItem* iter=m_sCollection->begin(); iter!=m_sCollection->end(); ++iter)
	{
		if(strcmp(iter->mLiveObject.mTypeName, "PxRigidDynamic")==0)
			++numActors;
	}

	m_CurrentBone = 0;
	m_NumBones = numActors;
	m_Bones = new ActorBone[m_NumBones];
	
	RepXItemAdder itemAdder(g_pPhysicsWorld->PhysXScene, this);

	//R3D_LOG_TIMESPAN_START(instantiateCollection);
	instantiateCollection( m_sCollection, g_pPhysicsWorld->PhysXSDK, g_pPhysicsWorld->Cooking, m_sStringTable, true, itemAdder );
	//R3D_LOG_TIMESPAN_END(instantiateCollection);

	unlink() ;

	isLoaded = 1 ;

	return true;
}

//------------------------------------------------------------------------

void r3dPhysSkeleton::linkParent(const r3dSkeleton *skel, const D3DXMATRIX &DrawFullMatrix, GameObject *parent, PHYSICS_COLLISION_GROUPS collisionGroup)
{
	for( int i = 0, e = m_NumBones ; i < e; i ++ )
	{
		PxRigidDynamic* inActor = m_Bones[ i ].actor ;

		char* name = strdup(inActor->getName());
		int boneID = skel->GetBoneID(name);
		
		if(boneID==-1)
			r3dError("Ragdoll: cannot find bone '%s' in skeleton\n", name);

		m_Bones[ i ].boneID = boneID ;

		D3DXMATRIX mat;
		D3DXMATRIX offset = DrawFullMatrix;

		skel->GetBoneWorldTM(boneID, &mat, offset);

		PxTransform actorPose = inActor->getGlobalPose();
		actorPose.p = PxVec3(mat._41, mat._42, mat._43);
		PxMat33 orientation(
							PxVec3(mat._11, mat._12, mat._13),
							PxVec3(mat._21, mat._22, mat._23),
							PxVec3(mat._31, mat._32, mat._33)
							);

		actorPose.q = PxQuat(orientation);
		inActor->setGlobalPose(actorPose);

		inActor->userData = parent; // for collision report

		free(name);

		PxShape* shapes[64] = {0};
		inActor->getShapes(&shapes[0], 64);

		for (uint32_t i = 0, e = inActor->getNbShapes() ; i < e ; ++ i )
		{
			PxFilterData filterData(collisionGroup, 0, 0, reinterpret_cast<PxU32>(parent));
			shapes[i]->setSimulationFilterData(filterData);
			PxFilterData qfilterData(1 << collisionGroup, 0, 0, 0);
			shapes[i]->setQueryFilterData(qfilterData);
			shapes[i]->userData = parent;
		}
	}

	for( int i = 0, e = m_NumBones ; i < e; i ++ )
	{
		m_Bones[ i ].actor->enableInScene() ;
	}
}

void r3dPhysSkeleton::unlink()
{
	for( int i = 0, e = m_NumBones ; i < e; i ++ )
	{
		PxRigidDynamic* actor = m_Bones[ i ].actor ;

		actor->disableInScene() ;
		actor->userData = 0 ;

		PxShape* shapes[64] = {0};
		actor->getShapes(&shapes[0], 64);

		for (uint32_t i = 0, e = actor->getNbShapes() ; i < e ; ++ i )
		{
			shapes[i]->userData = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dPhysSkeleton::syncAnimation(r3dSkeleton *skel, const D3DXMATRIX &DrawFullMatrix, r3dAnimation &anim)
{
	R3DPROFILE_FUNCTION("r3dPhysSkeleton::syncAnimation");

	if (!skel || !m_Bones || isLoaded == 0)
		return;

	if (!m_isRagdollMode)
	{

		for(int i=0; i<m_NumBones; ++i)
		{
			D3DXMATRIX mat;
			skel->GetBoneWorldTM(m_Bones[i].boneID, &mat, DrawFullMatrix);
			PxTransform actorPose;
			actorPose.p = PxVec3(mat._41, mat._42, mat._43);
			PxMat33 orientation(PxVec3(mat._11, mat._12, mat._13),
				PxVec3(mat._21, mat._22, mat._23),
				PxVec3(mat._31, mat._32, mat._33));
			actorPose.q = PxQuat(orientation);

			m_Bones[i].actor->moveKinematic(actorPose); // bones are kinematic, need to call moveGlobalPose for them
		}
	}
	else
	{
		D3DXMATRIX m(DrawFullMatrix);
		D3DXMATRIX rot;
		D3DXMatrixRotationX(&rot, D3DX_PI / 2);
		D3DXMatrixInverse(&m, 0, &m);

		r3d_assert(_countof(gBonesSkipUpdateFlagsArr) > skel->GetNumBones());
		::ZeroMemory(&gBonesSkipUpdateFlagsArr[0], sizeof(gBonesSkipUpdateFlagsArr));

		for (int i = 0; i < m_NumBones; ++i)
		{
			D3DXMATRIX res; D3DXMatrixIdentity(&res);
			ActorBone &b = m_Bones[i];
			PxTransform pose = b.actor->getGlobalPose();
			PxMat33 mat(pose.q);
			res._11 = mat.column0.x;
			res._12 = mat.column0.y;
			res._13 = mat.column0.z;

			res._21 = mat.column1.x;
			res._22 = mat.column1.y;
			res._23 = mat.column1.z;

			res._31 = mat.column2.x;
			res._32 = mat.column2.y;
			res._33 = mat.column2.z;

			res._41 = pose.p.x;
			res._42 = pose.p.y;
			res._43 = pose.p.z;

			D3DXMatrixMultiply(&res, &res, &m);

			const char* nnn = b.actor->getName();

			int boneIdx = b.boneID;

			skel->SetBoneWorldTM(boneIdx, &res);
			gBonesSkipUpdateFlagsArr[boneIdx] = true;
		}
		D3DXMATRIX basePose;
		D3DXMatrixIdentity(&basePose);
		anim.CalcBasePose(basePose);
		skel->Recalc(&basePose, 0, gBonesSkipUpdateFlagsArr);
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dPhysSkeleton::SwitchToRagdoll(bool toRagdoll)
{
#if ENABLE_RAGDOLL
	if (isLoaded == 0)
		return;

	m_isRagdollMode = toRagdoll;
	const PxTransform identity(PxTransform::createIdentity());
	for (int i = 0; i < m_NumBones; ++i)
	{
		PxRigidDynamic *a = m_Bones[i].actor;
		if (!a) continue;

		if (m_isRagdollMode)
			a->moveKinematic(identity);

		a->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, !m_isRagdollMode);

		if(m_isRagdollMode)
		{
			PxVec3 zero(0, 0, 0);
			a->setAngularVelocity(zero);
 			a->setLinearVelocity(zero);
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////

void r3dPhysSkeleton::SwitchToRagdollWithForce(bool toRagdoll, int boneId, const r3dPoint3D& force)
{
#if ENABLE_RAGDOLL
	if (isLoaded == 0)
		return;

	SwitchToRagdoll(toRagdoll);

	if (m_isRagdollMode)
	{
		if(boneId >= m_NumBones)
			boneId = random(m_NumBones-1);
		PxRigidDynamic *ab = m_Bones[boneId].actor;
		if(ab)
		{
			ab->addForce(PxVec3(force.x, force.y, force.z), PxForceMode::eVELOCITY_CHANGE);
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////

void r3dPhysSkeleton::TogglePhysicsSimulation(bool on)
{
	for (int i = 0; i < m_NumBones; ++i)
	{
		PxRigidDynamic *a = m_Bones[i].actor;
		if (a)
		{
			on ? g_pPhysicsWorld->AddActor(*a) : g_pPhysicsWorld->RemoveActor(*a);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

r3dBoundBox r3dPhysSkeleton::getWorldBBox() const
{
	PxBounds3 bbox(PxBounds3::empty());

	if (isLoaded)
	{
		for (int i = 0; i < m_NumBones; ++i)
		{
			PxRigidDynamic *a = m_Bones[i].actor;
			if (!a) continue;

			PxShape* shapes[16] = {0};
			a->getShapes(&shapes[0], 16);
			PxU32 numShapes = a->getNbShapes();
			for (PxU32 j = 0; j < numShapes; ++j)
			{
				PxShape *s = shapes[j];
				PxBounds3 shapeBox = s->getWorldBounds();
				bbox.include(shapeBox);
			}
		}
	}
	r3dBoundBox rv;
	rv.Org = r3dPoint3D(bbox.minimum.x, bbox.minimum.y, bbox.minimum.z);
	rv.Size = r3dPoint3D(bbox.maximum.x - bbox.minimum.x, bbox.maximum.y - bbox.minimum.y, bbox.maximum.z - bbox.minimum.z);
	return rv;
}
