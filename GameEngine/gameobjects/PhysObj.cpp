#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "GameObj.h"
#include "PhysObj.h"

#include "PxController.h"
#include "PxCapsuleController.h"
#include "PxBoxController.h"
#include "extensions/PxRigidBodyExt.h"
#include "PhysX/PhysXCharacterKinematic/src/CctCapsuleController.h"
#include "PhysX/PhysXAPI/vehicle/PxVehicleUtils.h"

#include "../TrueNature/Terrain.h"

bool g_bAllowPhysObjCreation = true;

class ControllerCallback : public PxUserControllerHitReport
{
public:
	virtual void onShapeHit(const PxControllerShapeHit& hit)
	{
	}

	virtual void onControllerHit(const PxControllersHit& hit)
	{
	}
} myControllerCallback;

BasePhysicsObject* BasePhysicsObject::CreateCharacterController(const PhysicsObjectConfig& params, PhysicsCallbackObject* physCallbackObj, const r3dVector* optionalPos, const r3dVector* optionalSize, const r3dMesh* optionalMesh, const D3DXMATRIX* optionalRotation)
{
    r3d_assert(g_bAllowPhysObjCreation);

	R3DPROFILE_FUNCTION("BasePhysicsObject::CreateCharacterController");
	r3dPoint3D objectPos, objectSize;
	const r3dMesh* objectMesh = 0;
	D3DXMATRIX objectRotation;
	if(physCallbackObj==0 || optionalPos != 0)
	{
		r3d_assert(optionalPos);
		r3d_assert(optionalSize);
		r3d_assert(optionalMesh);
		r3d_assert(optionalRotation);
		objectPos = *optionalPos;
		objectSize = *optionalSize;
		objectMesh = optionalMesh;
		objectRotation = *optionalRotation;
	}
	else
	{
		r3d_assert(physCallbackObj->isGameObject());
		objectPos = physCallbackObj->isGameObject()->GetPosition();
		objectSize = physCallbackObj->isGameObject()->GetBBoxLocal().Size;
		objectMesh = physCallbackObj->isGameObject()->GetObjectMesh();
		objectRotation = physCallbackObj->isGameObject()->GetRotationMatrix();
	}

	if(!(r3d_float_isFinite(objectPos.x) && r3d_float_isFinite(objectPos.y) && r3d_float_isFinite(objectPos.z)))
	{
		r3dError("CreateCharacterController: objectPos is invalid!"); 
	}

#ifdef _DEBUG
	r3dOutToLog("Creating character controller obj at: %.2f, %.2f, %.2f\n", objectPos.x, objectPos.y, objectPos.z);
#endif
	
	if(!(r3d_float_isFinite(objectRotation._11) && r3d_float_isFinite(objectRotation._12) && r3d_float_isFinite(objectRotation._13) &&
		r3d_float_isFinite(objectRotation._21) && r3d_float_isFinite(objectRotation._22) && r3d_float_isFinite(objectRotation._23) &&
		r3d_float_isFinite(objectRotation._31) && r3d_float_isFinite(objectRotation._32) && r3d_float_isFinite(objectRotation._33)))
	{
		r3dError("CreateCharacterController: objectRotation is invalid!"); 
	}

	r3d_assert(params.type == PHYSICS_TYPE_CONTROLLER);

	PxCapsuleControllerDesc desc;
	desc.setToDefault();
	desc.radius			= 0.3f; 
	desc.height			= 1.1f; 
	desc.climbingMode	= PxCapsuleClimbingMode::eCONSTRAINED;

	desc.position = PxExtendedVec3(objectPos.x, objectPos.y, objectPos.z);
	desc.upDirection = PxCCTUpAxis::eY;
	desc.slopeLimit = cosf(R3D_DEG2RAD(45.0f));
	desc.invisibleWallHeight = 0.0f;
	desc.maxJumpHeight = 0.0f;
	desc.contactOffset = 0.01f;
	desc.stepOffset = 0.5f;
	desc.callback = &myControllerCallback;
	desc.interactionMode = PxCCTInteractionMode::eEXCLUDE;
	desc.material = g_pPhysicsWorld->PhysXSDK->createMaterial(0.5f, 0.5f, 0.1f);
	desc.userData = (void*)physCallbackObj;

	PxController* Controller = g_pPhysicsWorld->CharacterManager->createController(*g_pPhysicsWorld->PhysXSDK, g_pPhysicsWorld->PhysXScene, desc);
	r3d_assert(Controller);

	PxRigidDynamic* actor = Controller->getActor();
	PxShape* shape;
	actor->getShapes(&shape, 1);
	PxFilterData filterData(PHYSCOLL_CHARACTERCONTROLLER, 0, 0, 0);
	shape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<PHYSCOLL_CHARACTERCONTROLLER, 0, 0, 0);
	shape->setQueryFilterData(qfilterData);
	shape->userData = (void*)physCallbackObj;

	ControllerPhysObj* physObj = new ControllerPhysObj();
	physObj->SetController(Controller);

	return physObj;
}

BasePhysicsObject* BasePhysicsObject::CreateDynamicObject(const PhysicsObjectConfig& params, PhysicsCallbackObject* physCallbackObj, const r3dVector* optionalPos, const r3dVector* optionalSize, const r3dMesh* optionalMesh, const D3DXMATRIX* optionalRotation)
{
    r3d_assert(g_bAllowPhysObjCreation);

	R3DPROFILE_FUNCTION("BasePhysicsObject::CreateDynamicObject");
	r3dPoint3D objectPos, objectSize;
	const r3dMesh* objectMesh = 0;
	D3DXMATRIX objectRotation;
	if(physCallbackObj==0 || optionalPos != 0)
	{
		r3d_assert(optionalPos);
		r3d_assert(optionalSize);
		r3d_assert(optionalMesh);
		r3d_assert(optionalRotation);
		objectPos = *optionalPos;
		objectSize = *optionalSize;
		objectMesh = optionalMesh;
		objectRotation = *optionalRotation;
	}
	else
	{
		r3d_assert(physCallbackObj->isGameObject());
		objectPos = physCallbackObj->isGameObject()->GetPosition();
		objectSize = physCallbackObj->isGameObject()->GetBBoxLocal().Size;
		objectMesh = physCallbackObj->isGameObject()->GetObjectMesh();
		objectRotation = physCallbackObj->isGameObject()->GetRotationMatrix();
	}

	if(!(r3d_float_isFinite(objectPos.x) && r3d_float_isFinite(objectPos.y) && r3d_float_isFinite(objectPos.z)))
	{
		r3dError("CreateDynamicObject: objectPos is invalid!"); 
	}

#ifdef _DEBUG
	{
		GameObject* temp = physCallbackObj->isGameObject();
		if(temp)
			r3dOutToLog("Creating dynamic phys obj (%s) at: %.2f, %.2f, %.2f\n", temp->Class->Name.c_str(), objectPos.x, objectPos.y, objectPos.z);
		else
			r3dOutToLog("Creating dynamic phys obj (unknown) at: %.2f, %.2f, %.2f\n", objectPos.x, objectPos.y, objectPos.z);
	}
#endif

	if(!(r3d_float_isFinite(objectRotation._11) && r3d_float_isFinite(objectRotation._12) && r3d_float_isFinite(objectRotation._13) &&
		r3d_float_isFinite(objectRotation._21) && r3d_float_isFinite(objectRotation._22) && r3d_float_isFinite(objectRotation._23) &&
		r3d_float_isFinite(objectRotation._31) && r3d_float_isFinite(objectRotation._32) && r3d_float_isFinite(objectRotation._33)))
	{
		r3dError("CreateDynamicObject: objectRotation is invalid!"); 
	}

	r3d_assert(params.type != PHYSICS_TYPE_UNKNOWN);

	bool nowWhoIsTheDummy = false;

	PxTransform pose;

	r3dPoint3D positionDifference(0,0,0);
	if(objectMesh && (params.type == PHYSICS_TYPE_BOX || params.type == PHYSICS_TYPE_SPHERE))
		positionDifference = objectMesh->getCentralPoint() - objectMesh->getPivot();

	r3dPoint3D gameObjPos = objectPos + positionDifference;
	pose.p = PxVec3(gameObjPos.x, gameObjPos.y, gameObjPos.z);
	PxMat33 orientation(PxVec3(objectRotation._11, objectRotation._12, objectRotation._13),
						PxVec3(objectRotation._21, objectRotation._22, objectRotation._23),
						PxVec3(objectRotation._31, objectRotation._32, objectRotation._33));
	pose.q = PxQuat(orientation);

	PxRigidDynamic* actor = g_pPhysicsWorld->PhysXSDK->createRigidDynamic(pose);

	PxShape* shape = NULL;
	PxU32 filter = params.group;
	if(params.isTrigger)
		filter	= PHYSCOLL_TRIGGER;

	if(params.type == PHYSICS_TYPE_BOX)
	{
		shape = actor->createShape(PxBoxGeometry(objectSize.x*0.5f, objectSize.y*0.5f, objectSize.z*0.5f), *(g_pPhysicsWorld->defaultMaterial));
	}
	else if(params.type == PHYSICS_TYPE_SPHERE)
	{
		shape = actor->createShape(PxSphereGeometry(R3D_MAX(R3D_MAX(objectSize.x*0.5f, objectSize.y*0.5f), objectSize.z*0.5f)), *(g_pPhysicsWorld->defaultMaterial));
	}
	else if(params.type == PHYSICS_TYPE_MESH)
	{
		r3d_assert(objectMesh);
		char cookedMeshFilename[256];
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		int len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-3], "mpx");

		if(!r3dFileExists(cookedMeshFilename))
		{
			// auto bake it
			if(!g_pPhysicsWorld->CookMesh(objectMesh))
			{
				r3dArtBug("Failed to auto cook mesh: %s\n", objectMesh->FileName.c_str());
				return NULL;
			}
		}

		PxTriangleMeshGeometry geom(g_pPhysicsWorld->getCookedMesh(cookedMeshFilename));
		shape = actor->createShape(geom, *(g_pPhysicsWorld->defaultMaterial));

		// check for player only collision
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-4], "_playerOnly.mpx");

		char rawMeshFilenamePlayerOnly[256];
		r3dscpy(rawMeshFilenamePlayerOnly, objectMesh->FileName.c_str());
		len = strlen(rawMeshFilenamePlayerOnly);
		r3dscpy(&rawMeshFilenamePlayerOnly[len-4], "_playerOnly.sco");

		// auto cook
		if(!r3dFileExists(cookedMeshFilename) && r3dFileExists(rawMeshFilenamePlayerOnly))
		{
			r3dMesh* playerOnlyMesh = r3dGOBAddMesh(rawMeshFilenamePlayerOnly, false);
			// auto bake it
			if(!g_pPhysicsWorld->CookMesh(playerOnlyMesh))
			{
				r3dArtBug("Failed to auto cook player only mesh: %s\n", playerOnlyMesh->FileName.c_str());
				return NULL;
			}
			SAFE_DELETE(playerOnlyMesh);
		}

		if(r3dFileExists(cookedMeshFilename))
		{
			PxTriangleMeshGeometry geom_player(g_pPhysicsWorld->getCookedMesh(cookedMeshFilename));
			PxShape* playerShape = actor->createShape(geom_player, *(g_pPhysicsWorld->defaultMaterial));
			PxFilterData filterData(PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setSimulationFilterData(filterData);
			PxFilterData qfilterData(1<<PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setQueryFilterData(qfilterData);
			filter = PHYSCOLL_NON_PLAYER_GEOMETRY; // make original bullet only geometry
		}
	}
	else if(params.type == PHYSICS_TYPE_CONVEX)
	{
		r3d_assert(objectMesh);
		char cookedMeshFilename[256]; 
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		int len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-3], "cpx");

		if(!r3dFileExists(cookedMeshFilename))
		{
			// auto bake it
			if(!g_pPhysicsWorld->CookConvexMesh(objectMesh))
			{
				r3dArtBug("Failed to auto cook convex mesh: %s\n", objectMesh->FileName.c_str());
				return NULL;
			}
		}

		PxConvexMeshGeometry geom(g_pPhysicsWorld->getConvexMesh(cookedMeshFilename));
		shape = actor->createShape(geom, *(g_pPhysicsWorld->defaultMaterial));

		// check for player only collision
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-4], "_playerOnly.cpx");

		char rawMeshFilenamePlayerOnly[256];
		r3dscpy(rawMeshFilenamePlayerOnly, objectMesh->FileName.c_str());
		len = strlen(rawMeshFilenamePlayerOnly);
		r3dscpy(&rawMeshFilenamePlayerOnly[len-4], "_playerOnly.sco");

		// auto cook
		if(!r3dFileExists(cookedMeshFilename) && r3dFileExists(rawMeshFilenamePlayerOnly))
		{
			r3dMesh* playerOnlyMesh = r3dGOBAddMesh(rawMeshFilenamePlayerOnly, false);
			// auto bake it
			if(!g_pPhysicsWorld->CookConvexMesh(playerOnlyMesh))
			{
				r3dArtBug("Failed to auto cook player only convex mesh: %s\n", playerOnlyMesh->FileName.c_str());
				return NULL;
			}
			SAFE_DELETE(playerOnlyMesh);
		}


		if(r3dFileExists(cookedMeshFilename))
		{
			PxConvexMeshGeometry geom_player(g_pPhysicsWorld->getConvexMesh(cookedMeshFilename));
			PxShape* playerShape = actor->createShape(geom_player, *(g_pPhysicsWorld->defaultMaterial));
			PxFilterData filterData(PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setSimulationFilterData(filterData);
			PxFilterData qfilterData(1<<PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setQueryFilterData(qfilterData);
			filter = PHYSCOLL_NON_PLAYER_GEOMETRY; // make original bullet only geometry
		}
	}
	else if( params.type == PHYSICS_TYPE_RAYCAST_BOX )
	{
		shape = actor->createShape(PxBoxGeometry(objectSize.x*0.5f, objectSize.y*0.5f, objectSize.z*0.5f), *(g_pPhysicsWorld->defaultMaterial));

		//boxDesc.shapeFlags		|= NX_SF_VISUALIZATION | NX_SF_DISABLE_RESPONSE | NX_SF_DISABLE_COLLISION;
		//actorDesc.flags			|= NX_AF_DISABLE_COLLISION | NX_AF_DISABLE_RESPONSE;

		actor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);

		nowWhoIsTheDummy = true;
	}
	else
	{
		r3d_assert( false );
		return NULL; // unknown type???
	}

	PxU32 filter2 = 0;

	//if(params.isFastMoving)
	//	filter2 = PHYSCOLL2_FAST_MOVING_OBJECT;

    PxFilterData filterData(filter, filter2, 0, 0);
	shape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<filter, 0, 0, 0);
	shape->setQueryFilterData(qfilterData);
	shape->userData = (void*)physCallbackObj;

	//if(params.isFastMoving)
	//	shape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, true);

	if(params.isTrigger)
		shape->setFlags(PxShapeFlag::eTRIGGER_SHAPE);

	if(params.needBoxCollision)
	{
		actor->createShape(PxBoxGeometry(objectSize.x*0.5f, objectSize.y*0.5f, objectSize.z*0.5f), *(g_pPhysicsWorld->defaultMaterial));
	}

	if(params.isKinematic)
	{
		actor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
	}

	PxRigidBodyExt::setMassAndUpdateInertia(*actor, params.mass);
	actor->userData = (void*)physCallbackObj;	

	g_pPhysicsWorld->AddActor(*actor);

	PhysObj* physObj( NULL );
	if( nowWhoIsTheDummy )
	{
		RaycastDummyPhysObj* dummy = new RaycastDummyPhysObj;
		dummy->SetOffset( params.offset );
		physObj = dummy;
	}
	else
	{
		physObj = new PhysObj();
	}

	physObj->SetPositionCorrection(positionDifference);
	physObj->SetActor(actor);

	return physObj;
}

BasePhysicsObject* BasePhysicsObject::CreateStaticObject(const PhysicsObjectConfig& params, PhysicsCallbackObject* physCallbackObj, const r3dVector* optionalPos, const r3dVector* optionalSize, const r3dMesh* optionalMesh, const D3DXMATRIX* optionalRotation)
{
    r3d_assert(g_bAllowPhysObjCreation);

	R3DPROFILE_FUNCTION("BasePhysicsObject::CreateStaticObject");
	r3dPoint3D objectPos, objectSize;
	const r3dMesh* objectMesh = 0;
	D3DXMATRIX objectRotation;
	if(physCallbackObj==0 || optionalPos != 0)
	{
		r3d_assert(optionalPos);
		r3d_assert(optionalSize);
		r3d_assert(optionalMesh);
		r3d_assert(optionalRotation);
		objectPos = *optionalPos;
		objectSize = *optionalSize;
		objectMesh = optionalMesh;
		objectRotation = *optionalRotation;
	}
	else
	{
		r3d_assert(physCallbackObj->isGameObject());
		objectPos = physCallbackObj->isGameObject()->GetPosition();
		objectSize = physCallbackObj->isGameObject()->GetBBoxLocal().Size;
		objectMesh = physCallbackObj->isGameObject()->GetObjectMesh();
		objectRotation = physCallbackObj->isGameObject()->GetRotationMatrix();
	}

	if(!(r3d_float_isFinite(objectPos.x) && r3d_float_isFinite(objectPos.y) && r3d_float_isFinite(objectPos.z)))
	{
		r3dError("CreateStaticObject: objectPos is invalid!"); 
	}

#ifdef _DEBUG
	r3dOutToLog("Creating static phys obj at: %.2f, %.2f, %.2f\n", objectPos.x, objectPos.y, objectPos.z);
#endif

	if(!(r3d_float_isFinite(objectRotation._11) && r3d_float_isFinite(objectRotation._12) && r3d_float_isFinite(objectRotation._13) &&
		r3d_float_isFinite(objectRotation._21) && r3d_float_isFinite(objectRotation._22) && r3d_float_isFinite(objectRotation._23) &&
		r3d_float_isFinite(objectRotation._31) && r3d_float_isFinite(objectRotation._32) && r3d_float_isFinite(objectRotation._33)))
	{
		r3dError("CreateStaticObject: objectRotation is invalid!"); 
	}

	r3d_assert(params.type != PHYSICS_TYPE_UNKNOWN);

	PxTransform pose;

	r3dPoint3D positionDifference(0,0,0);
	if(objectMesh && (params.type == PHYSICS_TYPE_BOX || params.type == PHYSICS_TYPE_SPHERE))
		positionDifference = objectMesh->getCentralPoint() - objectMesh->getPivot();

	r3dPoint3D gameObjPos = objectPos + positionDifference;
	pose.p = PxVec3(gameObjPos.x, gameObjPos.y, gameObjPos.z);
	PxMat33 orientation(PxVec3(objectRotation._11, objectRotation._12, objectRotation._13),
						PxVec3(objectRotation._21, objectRotation._22, objectRotation._23),
						PxVec3(objectRotation._31, objectRotation._32, objectRotation._33));
	pose.q = PxQuat(orientation);

	PxRigidStatic* actor = g_pPhysicsWorld->PhysXSDK->createRigidStatic(pose);

	PxShape* shape = NULL;
	PxU32 filter = params.group;
	if(params.isTrigger)
		filter	= PHYSCOLL_TRIGGER;

	if(params.type == PHYSICS_TYPE_BOX)
	{
		shape = actor->createShape(PxBoxGeometry(objectSize.x*0.5f, objectSize.y*0.5f, objectSize.z*0.5f), *(g_pPhysicsWorld->defaultMaterial));
	}
	else if(params.type == PHYSICS_TYPE_SPHERE)
	{
		shape = actor->createShape(PxSphereGeometry(R3D_MAX(R3D_MAX(objectSize.x*0.5f, objectSize.y*0.5f), objectSize.z*0.5f)), *(g_pPhysicsWorld->defaultMaterial));
	}
	// workaround for a crash with CCD sweeping against static convex mesh, so convert them to use static meshes instead
	else if(params.type == PHYSICS_TYPE_MESH || (params.type == PHYSICS_TYPE_CONVEX && !params.isTrigger))
	{
		r3d_assert(objectMesh);
		char cookedMeshFilename[256];
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		int len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-3], "mpx");

		if(!r3dFileExists(cookedMeshFilename))
		{
			if(params.needExplicitCollisionMesh)
			{
				char MeshFilenameCollision[256];
				r3dscpy(MeshFilenameCollision, objectMesh->FileName.c_str());
				len = strlen(MeshFilenameCollision);
				r3dscpy(&MeshFilenameCollision[len-4], "_collision.sco");

				// auto cook
				if(r3dFileExists(MeshFilenameCollision))
				{
					r3dMesh* CollisionMesh = r3dGOBAddMesh(MeshFilenameCollision, false);
					// auto bake it
					if(!g_pPhysicsWorld->CookMesh(CollisionMesh, objectMesh->FileName.c_str()))
					{
						r3dArtBug("Failed to auto cook collision mesh: %s\n", CollisionMesh->FileName.c_str());
						return NULL;
					}
					SAFE_DELETE(CollisionMesh);
				}
				else
				{
					return NULL; // skip this object
				}
			}
			else
			{
				// auto bake it
				if(!g_pPhysicsWorld->CookMesh(objectMesh))
				{
					r3dArtBug("Failed to auto cook mesh: %s\n", objectMesh->FileName.c_str());
					return NULL;
				}
			}
		}

		PxTriangleMeshGeometry geom(g_pPhysicsWorld->getCookedMesh(cookedMeshFilename));
		shape = actor->createShape(geom, *(g_pPhysicsWorld->defaultMaterial));

		// check for player only collision
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-4], "_playerOnly.mpx");

		char rawMeshFilenamePlayerOnly[256];
		r3dscpy(rawMeshFilenamePlayerOnly, objectMesh->FileName.c_str());
		len = strlen(rawMeshFilenamePlayerOnly);
		r3dscpy(&rawMeshFilenamePlayerOnly[len-4], "_playerOnly.sco");

		// auto cook
		if(!r3dFileExists(cookedMeshFilename) && r3dFileExists(rawMeshFilenamePlayerOnly))
		{
			r3dMesh* playerOnlyMesh = r3dGOBAddMesh(rawMeshFilenamePlayerOnly, false);
			// auto bake it
			if(!g_pPhysicsWorld->CookMesh(playerOnlyMesh))
			{
				r3dArtBug("Failed to auto cook player only mesh: %s\n", playerOnlyMesh->FileName.c_str());
				return NULL;
			}
			SAFE_DELETE(playerOnlyMesh);
		}


		if(r3dFileExists(cookedMeshFilename))
		{
			PxTriangleMeshGeometry geom_player(g_pPhysicsWorld->getCookedMesh(cookedMeshFilename));
			PxShape* playerShape = actor->createShape(geom_player, *(g_pPhysicsWorld->defaultMaterial));
			PxFilterData filterData(PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setSimulationFilterData(filterData);
			PxFilterData qfilterData(1<<PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setQueryFilterData(qfilterData);
			filter = PHYSCOLL_NON_PLAYER_GEOMETRY; // make original bullet only geometry
		}
	}
	else if(params.type == PHYSICS_TYPE_CONVEX)
	{
		r3d_assert(objectMesh);
		char cookedMeshFilename[256]; 
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		int len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-3], "cpx");

		if(!r3dFileExists(cookedMeshFilename))
		{
			if(params.needExplicitCollisionMesh)
			{
				char MeshFilenameCollision[256];
				r3dscpy(MeshFilenameCollision, objectMesh->FileName.c_str());
				len = strlen(MeshFilenameCollision);
				r3dscpy(&MeshFilenameCollision[len-4], "_collision.sco");

				// auto cook
				if(r3dFileExists(MeshFilenameCollision))
				{
					r3dMesh* CollisionMesh = r3dGOBAddMesh(MeshFilenameCollision, false);
					// auto bake it
					if(!g_pPhysicsWorld->CookConvexMesh(CollisionMesh, objectMesh->FileName.c_str()))
					{
						r3dArtBug("Failed to auto cook collision convex mesh: %s\n", CollisionMesh->FileName.c_str());
						return NULL;
					}
					SAFE_DELETE(CollisionMesh);
				}
				else
				{
					return NULL; // skip this object
				}
			}
			else
			{
				// auto bake it
				if(!g_pPhysicsWorld->CookConvexMesh(objectMesh))
				{
					r3dArtBug("Failed to auto cook convex mesh: %s\n", objectMesh->FileName.c_str());
					return NULL;
				}
			}
		}

		PxConvexMeshGeometry geom(g_pPhysicsWorld->getConvexMesh(cookedMeshFilename));
		shape = actor->createShape(geom, *(g_pPhysicsWorld->defaultMaterial));

		// check for player only collision
		r3dscpy(cookedMeshFilename, objectMesh->FileName.c_str());
		len = strlen(cookedMeshFilename);
		r3dscpy(&cookedMeshFilename[len-4], "_playerOnly.cpx");

		char rawMeshFilenamePlayerOnly[256];
		r3dscpy(rawMeshFilenamePlayerOnly, objectMesh->FileName.c_str());
		len = strlen(rawMeshFilenamePlayerOnly);
		r3dscpy(&rawMeshFilenamePlayerOnly[len-4], "_playerOnly.sco");

		// auto cook
		if(!r3dFileExists(cookedMeshFilename) && r3dFileExists(rawMeshFilenamePlayerOnly))
		{
			r3dMesh* playerOnlyMesh = r3dGOBAddMesh(rawMeshFilenamePlayerOnly, false);
			// auto bake it
			if(!g_pPhysicsWorld->CookConvexMesh(playerOnlyMesh))
			{
				r3dArtBug("Failed to auto cook player only convex mesh: %s\n", playerOnlyMesh->FileName.c_str());
				return NULL;
			}
			SAFE_DELETE(playerOnlyMesh);
		}

		if(r3dFileExists(cookedMeshFilename))
		{
			PxConvexMeshGeometry geom_player(g_pPhysicsWorld->getConvexMesh(cookedMeshFilename));
			PxShape* playerShape = actor->createShape(geom_player, *(g_pPhysicsWorld->defaultMaterial));
			PxFilterData filterData(PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setSimulationFilterData(filterData);
			PxFilterData qfilterData(1<<PHYSCOLL_PLAYER_ONLY_GEOMETRY, 0, 0, 0); // new geometry is player only
			playerShape->setQueryFilterData(qfilterData);
			filter = PHYSCOLL_NON_PLAYER_GEOMETRY; // make original bullet only geometry
		}
	}
	else
	{
		r3d_assert( false );
		return NULL; // unknown type???
	}

	PxFilterData filterData(filter, 0, 0, 0);
	shape->setSimulationFilterData(filterData);
	PxFilterData qfilterData(1<<filter, 0, 0, 0);
#if VEHICLES_ENABLED
	PxSetupDrivableShapeQueryFilterData(&qfilterData);
#endif
	shape->setQueryFilterData(qfilterData);
	shape->userData = (void*)physCallbackObj;
	
	if(params.isTrigger)
		shape->setFlags(PxShapeFlag::eTRIGGER_SHAPE);

	if(params.needBoxCollision)
	{
		PxShape* boxCollisionShape = actor->createShape(PxBoxGeometry(objectSize.x*0.5f, objectSize.y*0.5f, objectSize.z*0.5f), *(g_pPhysicsWorld->defaultMaterial));
		
		PxFilterData filterData2(params.group, 0, 0, 0);
		boxCollisionShape->setSimulationFilterData(filterData2);
		PxFilterData qfilterData2(1<<params.group, 0, 0, 0);
		boxCollisionShape->setQueryFilterData(qfilterData2);
		boxCollisionShape->userData = (void*)physCallbackObj;
	}

	r3d_assert(params.isKinematic==false);

	actor->userData = (void*)physCallbackObj;	

	g_pPhysicsWorld->AddActor(*actor);

	PhysObj* physObj( NULL );

	physObj = new PhysObj();
	physObj->SetPositionCorrection(positionDifference);
	physObj->SetActor(actor);

	return physObj;
}

PhysObj::PhysObj()
{
	Actor = 0;
	m_PositionDifference.Assign(0,0,0);
}

bool
BasePhysicsObject::IsStatic()
{
	if( PxActor* actor = getPhysicsActor() )
	{
		return actor->isRigidStatic() ? true : false ;
	}

	return false ;
}

PhysObj::~PhysObj()
{
	if(Actor)
	{
        r3d_assert(g_bAllowPhysObjCreation);
		Actor->userData = NULL; // pt: not sure, but I keep getting physics objects that have a pointers to dead game object
		Actor->release();
		Actor = NULL;
	}
}

void PhysObj::AddImpulseAtPos(const r3dPoint3D& impulse, const r3dPoint3D& pos)
{
	if(Actor && Actor->isRigidDynamic() && Actor->isRigidDynamic()->getRigidDynamicFlags()!=PxRigidDynamicFlag::eKINEMATIC)
	{
		PxRigidDynamic* dyn = Actor->isRigidDynamic();
		dyn->addForce(*(PxVec3*)&impulse, PxForceMode::eIMPULSE);
	}
}

void PhysObj::AddImpulseAtLocalPos(const r3dPoint3D& impulse, const r3dPoint3D& pos)
{
	if(Actor && Actor->isRigidDynamic() && Actor->isRigidDynamic()->getRigidDynamicFlags()!=PxRigidDynamicFlag::eKINEMATIC)
	{
		PxRigidDynamic* dyn = Actor->isRigidDynamic();
		dyn->addForce(*(PxVec3*)&impulse, PxForceMode::eIMPULSE);
	}
}



void PhysObj::Move(const r3dPoint3D& move, float sharpness)
{
	if(Actor->isRigidActor())
	{
		PxRigidActor* dyn = Actor->isRigidActor();
		PxTransform trans = dyn->getGlobalPose();
		trans.p.x += move.x;
		trans.p.y += move.y;
		trans.p.z += move.z;
		dyn->setGlobalPose(trans);
	}
}

void PhysObj::SetPosition(const r3dPoint3D& pos)
{
	r3dPoint3D correctPos = pos + m_PositionDifference;
	if(Actor->isRigidActor())
	{
		PxRigidActor* dyn = Actor->isRigidActor();
		PxTransform trans = dyn->getGlobalPose();
		trans.p.x = correctPos.x;
		trans.p.y = correctPos.y;
		trans.p.z = correctPos.z;
		dyn->setGlobalPose(trans);
	}
}

void PhysObj::SetRotation(const r3dVector& Angles)
{
	D3DXMATRIX rotation;
	D3DXMatrixRotationYawPitchRoll(&rotation, R3D_DEG2RAD(Angles.X), R3D_DEG2RAD(Angles.Y), R3D_DEG2RAD(Angles.Z));
	PxMat33 orientation(PxVec3(rotation._11, rotation._12, rotation._13),
		PxVec3(rotation._21, rotation._22, rotation._23),
		PxVec3(rotation._31, rotation._32, rotation._33));

	if(Actor->isRigidActor())
	{
		PxRigidActor* dyn = Actor->isRigidActor();
		PxTransform trans = dyn->getGlobalPose();
		trans.q = PxQuat(orientation);
		dyn->setGlobalPose(trans);
	}
}

void PhysObj::SetVelocity(const r3dPoint3D& vel)
{
	if(Actor && Actor->isRigidBody())
	{
		PxRigidBody* dyn = Actor->isRigidBody();
		dyn->setLinearVelocity(*(PxVec3*)&vel);
	}
}

r3dPoint3D PhysObj::GetPosition() const
{
	if(Actor->isRigidActor())
	{
		const PxRigidActor* dyn = Actor->isRigidActor();
		PxTransform trans = dyn->getGlobalPose();
		r3dPoint3D correctPos = r3dPoint3D(trans.p.x, trans.p.y, trans.p.z);
		correctPos -= m_PositionDifference;
		return correctPos;
	}
	r3d_assert(false);
	return r3dPoint3D(0,0,0);
}

D3DXMATRIX PhysObj::GetRotation() const
{
	D3DXMATRIX res; D3DXMatrixIdentity(&res);
	if(Actor->isRigidActor())
	{
		const PxRigidActor* dyn = Actor->isRigidActor();
		PxTransform trans = dyn->getGlobalPose();
		PxMat33 mat(trans.q);

		res._11 = mat.column0.x;
		res._12 = mat.column0.y;
		res._13 = mat.column0.z;

		res._21 = mat.column1.x;
		res._22 = mat.column1.y;
		res._23 = mat.column1.z;

		res._31 = mat.column2.x;
		res._32 = mat.column2.y;
		res._33 = mat.column2.z;

		return res;
	}
	r3d_assert(false);
	return res;
}

r3dPoint3D PhysObj::GetVelocity() const
{
	if(Actor && Actor->isRigidBody())
	{
		const PxRigidBody* dyn = Actor->isRigidBody();
		PxVec3 vel = dyn->getLinearVelocity();
		return r3dPoint3D(vel.x,vel.y,vel.z);
	}
	else
		return r3dPoint3D(0,0,0);
}

bool PhysObj::IsSleeping() 
{ 
	if(Actor && Actor->isRigidDynamic())
		return Actor->isRigidDynamic()->isSleeping(); 

	return true; // static
}

void PhysObj::addSmoothVelocity(const r3dVector& vel)
{
	if(Actor && Actor->isRigidDynamic() && Actor->isRigidDynamic()->getRigidDynamicFlags()!=PxRigidDynamicFlag::eKINEMATIC)
	{
		PxRigidDynamic* dyn = Actor->isRigidDynamic();
		dyn->addForce(*(PxVec3*)&vel, PxForceMode::eIMPULSE);
	}
}

void PhysObj::addImpulse(const r3dVector& impulse)
{
	if(Actor && Actor->isRigidDynamic() && Actor->isRigidDynamic()->getRigidDynamicFlags()!=PxRigidDynamicFlag::eKINEMATIC)
	{
		PxRigidDynamic* dyn = Actor->isRigidDynamic();
		dyn->addForce(*(PxVec3*)&impulse, PxForceMode::eIMPULSE);
	}
}

//------------------------------------------------------------------------

RaycastDummyPhysObj::RaycastDummyPhysObj()
: Offset( 0.f, 0.f, 0.f )
{

}

//------------------------------------------------------------------------
/*virtual*/

RaycastDummyPhysObj::~RaycastDummyPhysObj()
{
}

//------------------------------------------------------------------------

void
RaycastDummyPhysObj::SetOffset( const r3dPoint3D& offset )
{
	Offset = offset;
}

//------------------------------------------------------------------------
/*virtual*/

void
RaycastDummyPhysObj::SetPosition( const r3dPoint3D& pos )
{
	PhysObj::SetPosition( pos + Offset );
}

//------------------------------------------------------------------------
/*virtual*/

r3dPoint3D
RaycastDummyPhysObj::GetPosition() const
{
	return PhysObj::GetPosition() - Offset;
}


//------------------------------------------------------------------------
/*virtual*/

void
RaycastDummyPhysObj::SetVelocity(const r3dPoint3D& /*vel*/ )
{
	// do nothing
}

//------------------------------------------------------------------------
/*virtual*/
bool
RaycastDummyPhysObj::IsSleeping()
{
	// this one sleeps forever
	return true;
}

//------------------------------------------------------------------------
/*virtual*/

void
RaycastDummyPhysObj::AddImpulseAtPos(const r3dPoint3D& /*impulse*/, const r3dPoint3D& /*pos*/)
{
	// ignore
}

//------------------------------------------------------------------------

ControllerPhysObj::ControllerPhysObj()
{
	Controller = 0;
	m_HeightOffset = 0.85f;
}

ControllerPhysObj::~ControllerPhysObj()
{
	if(Controller)
	{
		Controller->release();
		Controller = NULL;
	}
}

void ControllerPhysObj::Move(const r3dPoint3D& move, float sharpness)
{
	//r3dOutToLog("move: %.2f, %.2f, %.2f\n", move.x, move.y, move.z);
	static PxVec3 prevFramePos;

	PxU32 collisionFlags;
	PxVec3 d(move.x, move.y, move.z);

	PxVec3 old_pos;
	PxVec3 new_pos;
	old_pos.x = (float)Controller->getPosition().x;
	old_pos.y = (float)Controller->getPosition().y;
	old_pos.z = (float)Controller->getPosition().z;
	
	// iterate movement by 0.2meter steps
	const int steps = int(move.Length() / 0.2f) + 1;
	PxVec3 dstep = d / (float)steps;
	for(int cur_step=0; cur_step<steps; cur_step++) 
	{
		{
			PxFilterData filterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK, 0, 0, 0); //COLLIDABLE_PLAYER_COLLIDABLE_MASK
			Controller->move(dstep, 0, 0.001f, collisionFlags, sharpness, &filterData);
		}
	
		// SPECIAL CODE TO PREVENT PLAYER FROM GOING THROUGH WALLS
		new_pos.x = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().x; //getDebugPosition will return new updated position from Controller->move. getPosition() returns previous frame position
		new_pos.y = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().y;
		new_pos.z = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().z;

		// check if for any reason we fell through geometry (that will check terrain too)
		float heighOffset = 0.8f;
		if(((PxCapsuleController*)Controller)->getHeight()<0.5f)
			heighOffset = 0.4f;
		PxBoxGeometry bbox(0.05f, heighOffset, 0.05f);
		PxTransform pose(PxVec3(new_pos.x, new_pos.y, new_pos.z), PxQuat(0,0,0,1));
		PxShape* hit = NULL;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK,0,0,0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
		if(!g_pPhysicsWorld->PhysXScene->overlapAny(bbox, pose, hit, filter)) 
		{
			continue;
		}
			
		// player somehow ended up in geometry, move him back
		Controller->setPosition(PxExtendedVec3(old_pos.x, old_pos.y, old_pos.z));

		// after returning him back to his previous position, try to add some gravity to prevent player being stuck to geometry, but check that previously we have any horizontal movement
		PxVec3 dd = d;
		dd.y = 0;
		if(dd.magnitudeSquared() > 0)
		{
			dd = PxVec3(0.0f, -9.81f*r3dGetFrameTime(), 0.0f);
			Controller->move(dd, COLLIDABLE_PLAYER_COLLIDABLE_MASK, 0.001f, collisionFlags, 1.0f);
		}
		break;
	}

	new_pos.x = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().x; //getDebugPosition()
	new_pos.y = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().y;
	new_pos.z = (float)((Cct::CapsuleController*)Controller)->getDebugPosition().z;

	// check if player fell through world and don't allow him to do this
	{
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_PLAYER_COLLIDABLE_MASK,0,0,0), PxSceneQueryFilterFlag::eSTATIC);
		PxSceneQueryHit hit;
		if(!g_pPhysicsWorld->PhysXScene->raycastAny(PxVec3(new_pos.x, new_pos.y, new_pos.z), PxVec3(0,-1,0), 1000.0f, hit, filter)) // nothing below our feet?
		{
			r3dOutToLog("Player falling through world. Auto correcting. Current pos: %.2f, %.2f, %.2f, Corrected Pos: %.2f, %.2f, %2.f\n", new_pos.x, new_pos.y, new_pos.z, prevFramePos.x, prevFramePos.y, prevFramePos.z);
			Controller->setPosition(PxExtendedVec3(prevFramePos.x, prevFramePos.y, prevFramePos.z));
		}
	}

	prevFramePos = old_pos;
}

void ControllerPhysObj::SetPosition(const r3dPoint3D& pos)
{
	// teleport controller in this position
	Controller->setPosition(PxExtendedVec3(pos.x, pos.y+m_HeightOffset, pos.z));
}

void ControllerPhysObj::SetRotation(const r3dVector& Angles)
{
}
void ControllerPhysObj::SetVelocity(const r3dPoint3D& vel)
{
}

r3dPoint3D ControllerPhysObj::GetPosition() const
{
	PxExtendedVec3 pos = Controller->getPosition();
	return r3dPoint3D((float)pos.x, (float)pos.y-m_HeightOffset, (float)pos.z);  // -1.0f - because physics position is in center of capsule, while game expects position to be at the bottom of capsule
}

D3DXMATRIX ControllerPhysObj::GetRotation() const
{
	D3DXMATRIX res; D3DXMatrixIdentity(&res);
	return res;
}

r3dPoint3D ControllerPhysObj::GetVelocity() const
{
	return r3dPoint3D(0,0,0);
}

bool ControllerPhysObj::IsSleeping()
{
	return Controller->getActor()->isSleeping(); 
}

void ControllerPhysObj::AdjustControllerSize(float new_radius, float new_height, float new_offset)
{
	((PxCapsuleController*)Controller)->setRadius(new_radius);
	((PxCapsuleController*)Controller)->setHeight(new_height);
//	((NxBoxController*)Controller)->setExtents(NxVec3(new_radius, new_height, new_radius));
	m_HeightOffset = new_offset;
}

PxActor* ControllerPhysObj::getPhysicsActor() 
{
	return Controller->getActor(); 
}
