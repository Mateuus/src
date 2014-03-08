#ifndef __PHYSXWORLD_H__
#define __PHYSXWORLD_H__

#ifndef WO_SERVER
#include "VehicleManager.h"
#endif
//////////////////////////////////////////////////////////////////////////

class VehicleManager;
class PhysXWorld
{
    bool    m_needFetchResults;
public:
	physx::PxPhysics*	PhysXSDK;
	PxScene*		PhysXScene;
	PxControllerManager* CharacterManager; // create all characters through it for proper physics behavior
	PxCooking* Cooking; // for cooking meshes for PhysX
	PxMaterial* defaultMaterial;
	VehicleManager *m_VehicleManager;
	/**
	* This critical section should be used to protect concurrent SDK operation during simulate() and fetchResults() calls. 
	* @see GetConcurrencyGuard().
	*/
	CRITICAL_SECTION concurrencyGuard;

public:
	PhysXWorld();
	~PhysXWorld();

	void Init();
	void Destroy();

	bool CookMesh(const r3dMesh* mesh, const char* save_as=NULL); // will save a cooked mesh as meshname.mpx
	PxTriangleMesh* getCookedMesh(const char* filename); // cached
	bool CookConvexMesh(const r3dMesh* mesh, const char* save_as=NULL); // will save a cooked mesh as meshname.cpx
	PxConvexMesh* getConvexMesh(const char* filename); // cached
	bool HasCookedMesh(const r3dMesh* mesh);
	bool HasConvexMesh(const r3dMesh* mesh);

    // fixed version of raycast single. PhysX was written by idiots and that shit returns collision with terrain even when ray is not even close to terrain.
    bool raycastSingle(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
        PxSceneQueryFlags outputFlags,
        PxRaycastHit& hit,
        const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData());

	void StartSimulation();
	void EndSimulation();
	void DrawDebug();

	/**
	* During multithread object creation/destruction and scene modification, client code should guard physX operations
	* using this CS. This is because during simulate() and fetchResults() calls no scene and object modification is allowed.
	*/
	CRITICAL_SECTION & GetConcurrencyGuard() { return concurrencyGuard; }

	/**	Client should use this add actor call, because of multithread concurrency issues. */
	void AddActor(PxActor &actor);
	void RemoveActor(PxActor &actor);

	/**	Export whole scene into physx collection format. This function can be used for debug purposes, when you need to share scene with specific issue to other people/applications. */
#ifndef FINAL_BUILD
	bool ExportWholeScene(const char *filename) const;
#endif
};
extern PhysXWorld* g_pPhysicsWorld;

class MyPhysXAllocator : public PxAllocatorCallback 	
{ 	     
public:
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line);
	virtual void deallocate(void* ptr);

};

#endif //__PHYSXWORLD_H__