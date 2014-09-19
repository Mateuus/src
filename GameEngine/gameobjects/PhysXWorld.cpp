#include "r3dPCH.h"
#include "r3d.h"

#include "PxAllocatorCallback.h"
#include "PxErrorCallback.h"
#include "extensions/PxExtensionsAPI.h"
#include "extensions/PxVisualDebuggerExt.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxDefaultSimulationFilterShader.h"
#include "cooking/PxTriangleMeshDesc.h"
#include "cooking/PxConvexMeshDesc.h"
#include "characterkinematic/PxControllerManager.h"

#include "PhysXWorld.h"
#include "GameObj.h"

#include "..\TrueNature\Terrain.h"

#include "ApexWorld.h"
#include "PhysXRepXHelpers.h"
#include "VehicleManager.h"

//#define PHYSX_DEBUG_LIBS

// libs
#ifdef _DEBUG
#pragma comment(lib, "PhysX3CookingCHECKED_x86.lib")
#pragma comment(lib, "PhysX3CHECKED_x86.lib")
#pragma comment(lib, "SceneQueryCHECKED.lib")
#pragma comment(lib, "SimulationControllerCHECKED.lib")
#pragma comment(lib, "PvdRuntimeCHECKED.lib")
#pragma comment(lib, "LowLevelCHECKED.lib")
#ifdef PHYSX_DEBUG_LIBS
	#pragma comment(lib, "FoundationDEBUG.lib")
	#pragma comment(lib, "PhysX3CommonDEBUG.lib")
	#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
	#pragma comment(lib, "PhysX3VehicleDEBUG.lib")
	#pragma comment(lib, "GeomUtilsDEBUG.lib")
	#pragma comment(lib, "RepX3DEBUG.lib")
	#pragma comment(lib, "RepXUpgrader3DEBUG.lib")
#else
	#pragma comment(lib, "FoundationCHECKED.lib")
	#pragma comment(lib, "PhysX3CommonCHECKED.lib")
	#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
	#pragma comment(lib, "PhysX3VehicleCHECKED.lib")
	#pragma comment(lib, "GeomUtilsCHECKED.lib")
	#pragma comment(lib, "RepX3CHECKED.lib")
	#pragma comment(lib, "RepXUpgrader3CHECKED.lib")
#endif
#elif defined(FINAL_BUILD)
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "PhysX3Common.lib")
#pragma comment(lib, "PhysX3Cooking_x86.lib")
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "LowLevel.lib")
#pragma comment(lib, "GeomUtils.lib")
#pragma comment(lib, "SceneQuery.lib")
#pragma comment(lib, "SimulationController.lib")
#pragma comment(lib, "PvdRuntime.lib")
#pragma comment(lib, "RepX3.lib")
#pragma comment(lib, "RepXUpgrader3.lib")
#pragma comment(lib, "PhysX3Vehicle.lib")
#else // RELEASE
#pragma comment(lib, "FoundationPROFILE.lib")
#pragma comment(lib, "PhysX3CommonPROFILE.lib")
#pragma comment(lib, "PhysX3CookingPROFILE_x86.lib")
#pragma comment(lib, "PhysX3PROFILE_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsPROFILE.lib")
#pragma comment(lib, "LowLevelPROFILE.lib")
#pragma comment(lib, "GeomUtilsPROFILE.lib")
#pragma comment(lib, "SceneQueryPROFILE.lib")
#pragma comment(lib, "SimulationControllerPROFILE.lib")
#pragma comment(lib, "PvdRuntimePROFILE.lib")
#pragma comment(lib, "RepX3PROFILE.lib")
#pragma comment(lib, "RepXUpgrader3PROFILE.lib")
#pragma comment(lib, "PhysX3VehiclePROFILE.lib")
#endif


PhysXWorld* g_pPhysicsWorld = 0;
// because we might re-cook meshes in physics editor, let's be able to disable cache
bool gPhysics_DisableCacheForEditor = false;

// regular mesh, can be only static
struct PhysicsMesh
{
	char filename[256];
	PxTriangleMesh* mesh;
};
int			gob_NumMeshesInPhysicsFactoryCache=0;
PhysicsMesh	*gob_PhysicsFactoryCache[1024];

PxTriangleMesh *r3dGOBAddPhysicsMesh(const char* fname)
{
	if(gPhysics_DisableCacheForEditor)
	{
		return g_pPhysicsWorld->PhysXSDK->createTriangleMesh(UserStream(fname, true));
	}

	for (int i=0;i<gob_NumMeshesInPhysicsFactoryCache;i++)
		if (strcmp(gob_PhysicsFactoryCache[i]->filename, fname)==0) 
			return gob_PhysicsFactoryCache[i]->mesh;

	if (gob_NumMeshesInPhysicsFactoryCache > 1023)
	{
		r3dArtBug("r3dGOBAddPhysicsMesh:  Mesh Cache overflow !\n" );
		return NULL;
	}

	int i = gob_NumMeshesInPhysicsFactoryCache;
	gob_PhysicsFactoryCache[i] = new PhysicsMesh;
	gob_PhysicsFactoryCache[i]->mesh = g_pPhysicsWorld->PhysXSDK->createTriangleMesh(UserStream(fname, true));
	r3dscpy(gob_PhysicsFactoryCache[i]->filename, fname);
	if(!gob_PhysicsFactoryCache[i]->mesh) 
		return NULL;
	gob_NumMeshesInPhysicsFactoryCache++;
	return gob_PhysicsFactoryCache[gob_NumMeshesInPhysicsFactoryCache-1]->mesh;
}

void r3dFreePhysicsMeshes()
{
	for (int i=0;i<gob_NumMeshesInPhysicsFactoryCache;i++)
	{
		gob_PhysicsFactoryCache[i]->mesh->release();
		gob_PhysicsFactoryCache[i]->mesh = NULL;
		delete gob_PhysicsFactoryCache[i];
	}
	gob_NumMeshesInPhysicsFactoryCache = 0;
}

// convex mesh, fast, can be dynamic. has a limit of 256 faces!!!
struct PhysicsConvexMesh
{
	char filename[256];
	PxConvexMesh* mesh;
};
int			gob_NumConvexMeshesInPhysicsFactoryCache=0;
PhysicsConvexMesh	*gob_PhysicsConvexFactoryCache[1024];

PxConvexMesh *r3dGOBAddPhysicsConvexMesh(const char* fname)
{
	if(gPhysics_DisableCacheForEditor)
	{
		return g_pPhysicsWorld->PhysXSDK->createConvexMesh(UserStream(fname, true));
	}
	for (int i=0;i<gob_NumConvexMeshesInPhysicsFactoryCache;i++)
		if (strcmp(gob_PhysicsConvexFactoryCache[i]->filename, fname)==0) 
			return gob_PhysicsConvexFactoryCache[i]->mesh;

	if (gob_NumConvexMeshesInPhysicsFactoryCache > 1023)
	{
		r3dArtBug("r3dGOBAddPhysicsConvexMesh:  Mesh Cache overflow !\n" );
		return NULL;
	}

	int i = gob_NumConvexMeshesInPhysicsFactoryCache;
	gob_PhysicsConvexFactoryCache[i] = new PhysicsConvexMesh;
	gob_PhysicsConvexFactoryCache[i]->mesh = g_pPhysicsWorld->PhysXSDK->createConvexMesh(UserStream(fname, true));
	r3dscpy(gob_PhysicsConvexFactoryCache[i]->filename, fname);
	if(!gob_PhysicsConvexFactoryCache[i]->mesh) 
		return NULL;
	gob_NumConvexMeshesInPhysicsFactoryCache++;
	return gob_PhysicsConvexFactoryCache[gob_NumConvexMeshesInPhysicsFactoryCache-1]->mesh;
}

void r3dFreePhysicsConvexMeshes()
{
	for (int i=0;i<gob_NumConvexMeshesInPhysicsFactoryCache;i++)
	{
		gob_PhysicsConvexFactoryCache[i]->mesh->release();
		gob_PhysicsConvexFactoryCache[i]->mesh = NULL;
		delete gob_PhysicsConvexFactoryCache[i];
	}
	gob_NumConvexMeshesInPhysicsFactoryCache = 0;
}


PhysXWorld::PhysXWorld()
: m_VehicleManager( NULL )
{
	PhysXSDK = 0;
	PhysXScene = 0;
	CharacterManager = 0;
	Cooking = 0;
	defaultMaterial = NULL;
}

PhysXWorld::~PhysXWorld()
{
	Destroy();
}

void* MyPhysXAllocator::allocate(size_t size, const char* typeName, const char* filename, int line)
{	
	void* memPtr = _aligned_malloc(size, 16);
	if(memPtr == NULL)
	{
		r3dOutToLog("PhysX: allocate of %u bytes failed: typeName=%s, filename=%s, line=%d\n", size, typeName, filename, line);
		r3dError("PhysX: Out of memory when allocating %u bytes!", size );
	}
	return memPtr; 
}

void MyPhysXAllocator::deallocate(void* ptr)
{ 
	_aligned_free(ptr); 
}
MyPhysXAllocator myPhysXAllocator;

class MyErrorCallback : public PxErrorCallback
{
public:
	virtual ~MyErrorCallback() {};

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorCode = NULL;
		switch (code)
		{
		case PxErrorCode::eINVALID_PARAMETER:
			errorCode = "invalid parameter";
			break;
		case PxErrorCode::eINVALID_OPERATION:
			errorCode = "invalid operation";
			break;
		case PxErrorCode::eINTERNAL_ERROR:
			errorCode = "internal error";
			break;
		case PxErrorCode::eOUT_OF_MEMORY:
			errorCode = "out of memory";
			break;
		case PxErrorCode::eDEBUG_INFO:
			errorCode = "info";
			break;
		case PxErrorCode::eDEBUG_WARNING:
			errorCode = "warning";
			break;
		default:
			errorCode = "unknown error";
			break;
		}

		if (code > PxErrorCode::eDEBUG_WARNING && code != PxErrorCode::eINVALID_PARAMETER) // TEMP eINVALID_PARAMETER check until PhysX 3.1.1
			r3dError("PhysX Error (%s): '%s' at file %s, line %d\n", errorCode, message, file, line);
		else
			r3dOutToLog("PhysX Warning (%s): '%s' at file %s, line %d\n", errorCode, message, file, line);
	}
} myErrorCallback;

class MySimulationEventCallback : public PxSimulationEventCallback     
{        
public:
	virtual void onContact(PxContactPair& pair, PxU32 events)       
	{      
		if(events == PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			if(pair.isDeletedActor[0] || pair.isDeletedActor[1])
				return;

			PhysicsCallbackObject* obj1 = (PhysicsCallbackObject*)pair.actors[0]->userData;
			PhysicsCallbackObject* obj2 = (PhysicsCallbackObject*)pair.actors[1]->userData;
			
			// Iterate through contact points
			PxContactStreamIterator i(pair.stream);
			//user can call getNumPairs() here
			bool done = false;
			CollisionInfo collInfo;
			collInfo.Type = CLT_Vertex;
			while(i.goNextPair())
			{
				//user can also call getShape() and getNumPatches() here
				while(i.goNextPatch())
				{
					//user can also call getPatchNormal() and getNumPoints() here
					collInfo.Normal = *(r3dPoint3D*)&i.getPatchNormal();
					while(i.goNextPoint())
					{
						collInfo.NewPosition = *(r3dPoint3D*)&i.getPoint();			
						collInfo.Distance = i.getSeparation();
						if(!done)
						{
							collInfo.pObj = obj2;
							if(obj1)
								obj1->OnCollide(obj2, collInfo);

							collInfo.pObj = obj1;
							if(obj2)
								obj2->OnCollide(obj1, collInfo);

							done = true;
						}
					}
				}
			}
		}
	};

	void onTrigger(PxTriggerPair *  pairs,  PxU32  count)
	{
		for(PxU32 i=0; i<count; ++i)
		{
            if(pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
            {
                r3d_assert(pairs[i].triggerShape);
                r3d_assert(pairs[i].otherShape);
                PhysicsCallbackObject* triggerObj = (PhysicsCallbackObject*)pairs[i].triggerShape->getActor().userData;
                r3d_assert(triggerObj);

                PhysicsCallbackObject* otherObj = (PhysicsCallbackObject*)pairs[i].otherShape->getActor().userData;

                triggerObj->OnTrigger(pairs[i].status, otherObj);
            }
		}
	}

} mySimulationEventCallback;

PxU32 groupCollisionFlags[32];

void setGroupCollisionFlag(PxU32 groups1, PxU32 groups2, bool enable)
{
	PX_ASSERT(groups1 < 32 && groups2 < 32);
	if (enable)
	{
		//be symmetric:
		groupCollisionFlags[groups1] |= (1 << groups2);
		groupCollisionFlags[groups2] |= (1 << groups1);
	}
	else
	{
		groupCollisionFlags[groups1] &= ~(1 << groups2);
		groupCollisionFlags[groups2] &= ~(1 << groups1);
	}
}

PxFilterFlags MyCollisionFilterShader(
						   PxFilterObjectAttributes attributes0, PxFilterData filterData0,
						   PxFilterObjectAttributes attributes1, PxFilterData filterData1,
						   PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through, and do any other prefiltering you need.
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		return PxFilterFlag::eDEFAULT;
	}

	PxU32 ShapeGroup0 = filterData0.word0 & 31;
	PxU32 ShapeGroup1 = filterData1.word0 & 31;
	PxU32 * groupCollisionFlags = (PxU32 *)constantBlock;

	if ((groupCollisionFlags[ShapeGroup0] & (1 << ShapeGroup1)) == 0)
		return PxFilterFlag::eSUPPRESS;

	//	Prevent multiobjects intersection of same type if configured
	if (filterData0.word0 == filterData1.word0 && filterData0.word3 != filterData1.word3)
		return PxFilterFlag::eSUPPRESS;

	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	// ptumik: disabled for now, crashing physx, no idea why :(
    /*{
		PxU32 ShapeGroup01 = filterData0.word1 & 31;
		PxU32 ShapeGroup11 = filterData1.word1 & 31;
		if(ShapeGroup01 == PHYSCOLL2_FAST_MOVING_OBJECT || ShapeGroup11 == PHYSCOLL2_FAST_MOVING_OBJECT)
			pairFlags |= PxPairFlag::eSWEPT_INTEGRATION_LINEAR;
	}*/

	return PxFilterFlag::eDEFAULT;
}

void PhysXWorld::Init()
{
    m_needFetchResults = false;

	InitializeCriticalSection(&concurrencyGuard);

	//init all group pairs to true:
	for (unsigned i = 0; i < 32; i ++)
		groupCollisionFlags[i] = 0xffffffff;


	PxTolerancesScale tolerancesScale;
	tolerancesScale.length = 1.0f;
	tolerancesScale.mass = 1000.0f;
	tolerancesScale.speed = 10.0f;

	bool recordMemoryAllocations = false;
#ifdef _DEBUG
	recordMemoryAllocations = true; // track memory allocations
#endif
	PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, myPhysXAllocator, myErrorCallback, tolerancesScale, recordMemoryAllocations);
	if(!PhysXSDK)
	{
		r3dError("Failed to init PhysX SDK");
	}

	if(!PxInitExtensions(*PhysXSDK))
	{
		r3dError("Failed to init PhysX Extensions");
	}

	PxCookingParams cookingParams;
	Cooking = PxCreateCooking(PX_PHYSICS_VERSION, &PhysXSDK->getFoundation(), cookingParams);
	if(!Cooking)
	{
		r3dError("Failed to init PhysX Cooking");
	}

#ifndef FINAL_BUILD
	PxExtensionVisualDebugger::connect(PhysXSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, true, PxGetDefaultDebuggerFlags());
#endif

	// set collision group BEFORE creating scene and cannot change after that
//#if !ENABLE_RAGDOLL
	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_LOCALPLAYER, false); // do not collide between local player and weapons that local player fires
	setGroupCollisionFlag(PHYSCOLL_NETWORKPLAYER, PHYSCOLL_NETWORKPLAYER, false);
//#endif

	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_TINY_GEOMETRY, false); 
	setGroupCollisionFlag(PHYSCOLL_NETWORKPLAYER, PHYSCOLL_TINY_GEOMETRY, false);

	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_NON_PLAYER_GEOMETRY, false); 
	setGroupCollisionFlag(PHYSCOLL_NETWORKPLAYER, PHYSCOLL_NON_PLAYER_GEOMETRY, false);
	
	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_PROJECTILES, false); 

	setGroupCollisionFlag(PHYSCOLL_PROJECTILES, PHYSCOLL_LOCALPLAYER, false); 
	setGroupCollisionFlag(PHYSCOLL_PROJECTILES, PHYSCOLL_NETWORKPLAYER, false); 

	setGroupCollisionFlag(PHYSCOLL_PROJECTILES, PHYSCOLL_PLAYER_ONLY_GEOMETRY,   false); 
	setGroupCollisionFlag(PHYSCOLL_TINY_GEOMETRY, PHYSCOLL_PLAYER_ONLY_GEOMETRY, false); 
	setGroupCollisionFlag(PHYSCOLL_NETWORKPLAYER, PHYSCOLL_PLAYER_ONLY_GEOMETRY, false);

	setGroupCollisionFlag(PHYSCOLL_PROJECTILES, PHYSCOLL_CHARACTERCONTROLLER,   false); 
	setGroupCollisionFlag(PHYSCOLL_TINY_GEOMETRY, PHYSCOLL_CHARACTERCONTROLLER, false); 
	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_CHARACTERCONTROLLER,   false);
	setGroupCollisionFlag(PHYSCOLL_NETWORKPLAYER, PHYSCOLL_CHARACTERCONTROLLER, false);

	//	Prevent collision between player and character ragdoll
	setGroupCollisionFlag(PHYSCOLL_LOCALPLAYER, PHYSCOLL_NETWORKPLAYER, false);

#if VEHICLES_ENABLED
	//	Vehicle related collision detection
	setGroupCollisionFlag(PHYSCOLL_COLLISION_GEOMETRY, PHYSCOLL_VEHICLE_WHEEL, false);
	setGroupCollisionFlag(PHYSCOLL_STATIC_GEOMETRY, PHYSCOLL_VEHICLE_WHEEL, false);
#endif

	// create scene
	PxSceneDesc sceneDesc(PhysXSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -9.81f, 0);
	sceneDesc.filterShaderData = groupCollisionFlags;
	sceneDesc.filterShaderDataSize = 32 * sizeof(PxU32);

	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2, NULL);
	if(!sceneDesc.cpuDispatcher)
		r3dError("PhysX: Failed to create CPU dispatcher");
	
	sceneDesc.filterShader = MyCollisionFilterShader;

	sceneDesc.gpuDispatcher = NULL;

	//sceneDesc.flags |= PxSceneFlag::eENABLE_SWEPT_INTEGRATION;

	PhysXScene = PhysXSDK->createScene(sceneDesc);	
	if (!PhysXScene) 
		r3dError("PhysX: can't create physics scene!");

	PhysXScene->setSimulationEventCallback(&mySimulationEventCallback);

	defaultMaterial = PhysXSDK->createMaterial(0.8f, 0.8f, 0.05f);
	
	CharacterManager = PxCreateControllerManager(PhysXSDK->getFoundation());

#ifndef WO_SERVER
#if VEHICLES_ENABLED
	m_VehicleManager = new VehicleManager;
#endif
#endif
}

void PhysXWorld::Destroy()
{
	if(defaultMaterial)
	{
		defaultMaterial->release();
		defaultMaterial = NULL;
	}

#ifndef WO_SERVER
#if VEHICLES_ENABLED
	if (m_VehicleManager)
	{
		delete m_VehicleManager;
		m_VehicleManager = 0;
	}
#endif
#endif

	if(CharacterManager) {
		CharacterManager->release();
		CharacterManager = NULL;
	}
	
	r3dFreePhysicsMeshes();
	r3dFreePhysicsConvexMeshes();
	
	if(Cooking) {
		Cooking->release();
		Cooking = NULL;
	}

	PxCloseExtensions();
	
	if(PhysXScene)
	{
		PhysXScene->release();
		PhysXScene = NULL;
	}

	if(PhysXSDK) {
		PhysXSDK->release();
		PhysXSDK = NULL;
	}

	DeleteCriticalSection(&concurrencyGuard);
}

// not a nice solution, but PhysX right now crashing on loading WO_Torn map on server. Have no idea why, as we are not even creating any physx objects. Have a very bad feeling of some memory corruption somewhere...
//#ifdef WO_SERVER
//int DisablePhysXSimulation = 1;
//#else
int DisablePhysXSimulation = 0;
//#endif

#ifndef FINAL_BUILD
int DrawPhysicsDebug = 0;
#endif

extern bool g_bAllowPhysObjCreation;
void PhysXWorld::StartSimulation()
{
    g_bAllowPhysObjCreation = false;

#ifndef WO_SERVER
#if VEHICLES_ENABLED
	m_VehicleManager->UpdateInput();
#endif
#endif
#ifndef FINAL_BUILD
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, DrawPhysicsDebug?1.0f:0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES,	1.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eBODY_MASS_AXES,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_NORMAL,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES,	0.0f);
	PhysXScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS,	0.0f);
#endif

#if !APEX_ENABLED
	if(!DisablePhysXSimulation)
	{
		float elapsedTime = r3dGetFrameTime();
        static const float substepSize = 1.0f/60.0f;
        static const int numMaxSubsteps = 16;
        
        static float accumulator = 0.0f;
        accumulator += elapsedTime;
        if(accumulator > substepSize*numMaxSubsteps)
            accumulator = substepSize*numMaxSubsteps;
        
        int numStepsReq = (int)floorf(accumulator/substepSize);
        if(numStepsReq > 1)
        {
            for(int i=0; i<numStepsReq-1; ++i)
            {
				r3dCSHolder block(concurrencyGuard) ;
                accumulator -= substepSize;
#ifndef WO_SERVER
#if VEHICLES_ENABLED
				m_VehicleManager->Update(substepSize);
#endif
#endif
                PhysXScene->simulate(substepSize);
                PhysXScene->fetchResults(true);
            }
        }
        if(accumulator >= substepSize)
        {
			r3dCSHolder block(concurrencyGuard) ;
            accumulator -=substepSize;
#ifndef WO_SERVER
#if VEHICLES_ENABLED
			m_VehicleManager->Update(substepSize);
#endif
#endif
		    PhysXScene->simulate(substepSize); 
            m_needFetchResults = true;
        }
	}
#endif
}

void PhysXWorld::EndSimulation()
{
	if(!DisablePhysXSimulation)
	{
#if !APEX_ENABLED
        if(m_needFetchResults)
		{
			r3dCSHolder block(concurrencyGuard) ;
		    PhysXScene->fetchResults(true);
		}
#endif
		CharacterManager->updateControllers();
	}
    g_bAllowPhysObjCreation = true;
}

bool PhysXWorld::raycastSingle(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, PxSceneQueryFlags outputFlags, PxRaycastHit& hit, const PxSceneQueryFilterData& filterData /* = PxSceneQueryFilterData */)
{
    PxRaycastHit hits[32];
    bool blockingHit;
    // some weird PhysX shit. Sometimes raycastSingle will just return distance 0 and show collision with terrain!!! even though there is no even close terrain to the ray
    outputFlags |= PxSceneQueryFlag::eDISTANCE; // always add, as we need distance to check for physX bug
    int numHits = g_pPhysicsWorld->PhysXScene->raycastMultiple(origin, unitDir, distance, outputFlags, hits, 32, blockingHit, filterData);
    bool foundProperHit = false;
    float closestHit = 99999999.0f;
    if(numHits)
    {
        for(int i=0; i<numHits; ++i)
        {
            if(hits[i].distance > 0)
            {
                if(hits[i].distance < closestHit)
                {
                    hit = hits[i];
                    closestHit = hits[i].distance;
                    foundProperHit = true;
                }
            }					
        }
    }

    return foundProperHit;
}

extern	r3dCamera	gCam;
extern r3dScreenBuffer* ScreenBuffer;
void PhysXWorld::DrawDebug()
{
#ifndef FINAL_BUILD
	if(!DrawPhysicsDebug)
		return;

	R3DPROFILE_FUNCTION("PhysXWorld::DrawDebug");
	R3DPROFILE_START("getDebugRenderable");
	const PxRenderBuffer& dbgRenderable=PhysXScene->getRenderBuffer();
	R3DPROFILE_END("getDebugRenderable");
	r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA|R3D_BLEND_ZC);
	// restore depth buffer, as at this point it fucked up by someone
	r3dRenderer->SetDSS( ScreenBuffer->ZBuf.Get() );
	
	//Render lines       
	{
		PxU32 NbLines = dbgRenderable.getNbLines();
		const PxDebugLine* Lines = dbgRenderable.getLines();
		while(NbLines--)
		{        
			r3dDrawUniformLine3D(*(r3dPoint3D*)&Lines->pos0, *(r3dPoint3D*)&Lines->pos1, gCam, r3dColor(Lines->color0));
			Lines++;            
		}            
	}
	// Render triangles       
	{           
		PxU32 NbTris = dbgRenderable.getNbTriangles();  
		const PxDebugTriangle* Triangles = dbgRenderable.getTriangles(); 
		while(NbTris--)           
		{   
			r3dDrawTriangle3D(*(r3dPoint3D*)&Triangles->pos0, *(r3dPoint3D*)&Triangles->pos1, *(r3dPoint3D*)&Triangles->pos2, gCam, r3dColor(Triangles->color0));
			Triangles++;          
		}          
	}

	// render terrain
	// only enable for debug. VERY SLOW code!
// 	if(Terrain1 && Terrain1->physicsTerrain)
// 	{
// 		const NxHeightFieldShape * hfs = (const NxHeightFieldShape*)Terrain1->physicsTerrain->getShapes()[0];
// 		for(NxU32 row = 0; row < hfs->getHeightField().getNbRows() - 1; row++)
// 		{
// 			for(NxU32 column = 0; column < hfs->getHeightField().getNbColumns() - 1; column++)
// 			{
// 				NxTriangle tri;
// 				NxU32 triangleIndex = 2 * (row * hfs->getHeightField().getNbColumns() + column);
// 
// 				if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
// 				{
// 					NxVec3 n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
// 					n.normalize();
// 					float l = 1.0f;//n.dot(NxVec3(0,1,0));
// 					//r3dDrawTriangle3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[1], *(r3dPoint3D*)&tri.verts[2], gCam, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 					r3dDrawLine3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[1], gCam, 0.05f, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 					r3dDrawLine3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[2], gCam, 0.05f, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 					r3dDrawLine3D(*(r3dPoint3D*)&tri.verts[1], *(r3dPoint3D*)&tri.verts[2], gCam, 0.05f, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 				}
// 				triangleIndex++;
// 				if (hfs->getTriangle(tri, NULL, NULL, triangleIndex, true))
// 				{
// 					NxVec3 n = (tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
// 					n.normalize();
// 					float l = 1.0f;//n.dot(NxVec3(0,1,0));
// 					r3dDrawLine3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[1], gCam, 0.05f, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 					r3dDrawLine3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[2], gCam, 0.05f, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// //					r3dDrawTriangle3D(*(r3dPoint3D*)&tri.verts[0], *(r3dPoint3D*)&tri.verts[1], *(r3dPoint3D*)&tri.verts[2], gCam, r3dColor(0.0f, 255.0f*l, 0.0f, 255.0f));
// 				}
// 			}
// 		}
// 	}

	r3dRenderer->Flush();

#endif
}

bool PhysXWorld::CookMesh(const r3dMesh* orig_mesh, const char* save_as)
{
#ifndef FINAL_BUILD
	r3d_assert(orig_mesh);
	const r3dMesh* mesh = orig_mesh;

	r3d_assert(mesh->NumVertices >0 && mesh->NumIndices>0);

	//Build physical model 
	PxTriangleMeshDesc meshDesc;    
	meshDesc.points.count = mesh->NumVertices;    
	meshDesc.points.stride = sizeof(r3dPoint3D);   
	meshDesc.points.data = mesh->GetVertexPositions();  
	meshDesc.triangles.count = mesh->NumIndices/3;    
	meshDesc.triangles.stride = 3*sizeof(uint32_t);   
	meshDesc.triangles.data = mesh->GetIndices();

	r3d_assert( meshDesc.points.data && meshDesc.triangles.data ) ;

	char cookedMeshFilename[256]; memset(cookedMeshFilename, 0, 256);
	if(save_as)
		r3dscpy(cookedMeshFilename, save_as);
	else
		r3dscpy(cookedMeshFilename, orig_mesh->FileName.c_str());
	int len = strlen(cookedMeshFilename);
	r3dscpy(&cookedMeshFilename[len-3], "mpx");
	bool res = Cooking->cookTriangleMesh(meshDesc, UserStream(cookedMeshFilename, false));

	return res;
#else
	return false;
#endif
}

PxTriangleMesh* PhysXWorld::getCookedMesh(const char* filename)
{
	return r3dGOBAddPhysicsMesh(filename);
}

bool PhysXWorld::HasCookedMesh(const r3dMesh* mesh)
{
	char cookedMeshFilename[256]; memset(cookedMeshFilename, 0, 256);
	r3dscpy(cookedMeshFilename, mesh->FileName.c_str());
	int len = strlen(cookedMeshFilename);
	r3dscpy(&cookedMeshFilename[len-3], "mpx");
	return r3d_access(cookedMeshFilename, 0)==0;
}

bool PhysXWorld::HasConvexMesh(const r3dMesh* mesh)
{
	char cookedMeshFilename[256]; memset(cookedMeshFilename, 0, 256);
	r3dscpy(cookedMeshFilename, mesh->FileName.c_str());
	int len = strlen(cookedMeshFilename);
	r3dscpy(&cookedMeshFilename[len-3], "cpx");
	return r3d_access(cookedMeshFilename, 0)==0;
}

bool PhysXWorld::CookConvexMesh(const r3dMesh* orig_mesh, const char* save_as)
{
#ifndef FINAL_BUILD
	r3d_assert(orig_mesh);
	const r3dMesh* mesh = orig_mesh;

	r3d_assert(mesh->NumVertices >0 && mesh->NumIndices>0);

	//Build physical model 
	PxConvexMeshDesc meshDesc;    
	meshDesc.points.count = mesh->NumVertices;    
	meshDesc.points.stride = sizeof(r3dPoint3D);   
	meshDesc.points.data = mesh->GetVertexPositions();  
	meshDesc.triangles.count = mesh->NumIndices/3;    
	meshDesc.triangles.stride = 3*sizeof(uint32_t);   
	meshDesc.triangles.data = mesh->GetIndices();   

	r3d_assert( meshDesc.points.data && meshDesc.triangles.data ) ;

	meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX; // let physX compute convex mesh. even if we are providing it a convex mesh, maybe it can optimize it better. also in that case if don't have to worry about 256 faces limit

	char cookedMeshFilename[256]; memset(cookedMeshFilename, 0, 256);
	if(save_as)
		r3dscpy(cookedMeshFilename, save_as);
	else
		r3dscpy(cookedMeshFilename, orig_mesh->FileName.c_str());
	int len = strlen(cookedMeshFilename);
	r3dscpy(&cookedMeshFilename[len-3], "cpx");
	bool res = Cooking->cookConvexMesh(meshDesc, UserStream(cookedMeshFilename, false));

	return res;
#else
	return false;
#endif
}

PxConvexMesh* PhysXWorld::getConvexMesh(const char* filename)
{
	return r3dGOBAddPhysicsConvexMesh(filename);
}

//////////////////////////////////////////////////////////////////////////

void PhysXWorld::AddActor(PxActor &actor)
{
	r3dCSHolder block(concurrencyGuard) ;
	PhysXScene->addActor(actor);
}

//////////////////////////////////////////////////////////////////////////

void PhysXWorld::RemoveActor(PxActor &actor)
{
	r3dCSHolder block(concurrencyGuard) ;
	PhysXScene->removeActor(actor);
}

//////////////////////////////////////////////////////////////////////////

#ifndef FINAL_BUILD
bool PhysXWorld::ExportWholeScene(const char *filename) const
{
	if (!PhysXSDK || !PhysXScene)
		return false;

	PxCollection *cl = PhysXSDK->createCollection();
	if (!cl)
		return false;

	PxCollectForExportSDK(*PhysXSDK, *cl);
	PxCollectForExportScene(*PhysXScene, *cl);

	FileSerialStream fs(filename);
	cl->serialize(fs);

	PhysXSDK->releaseCollection(cl);

	return true;
}
#endif