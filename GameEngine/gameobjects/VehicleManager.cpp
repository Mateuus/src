//=========================================================================
//	Module: VehicleManager.cpp
//	Copyright (C) 2012.
//=========================================================================

#include "r3dPCH.h"

#if VEHICLES_ENABLED

#include "r3d.h"
#include "extensions/PxStringTableExt.h"
#include "PhysXWorld.h"
#include "PhysXRepXHelpers.h"
#include "ObjManag.h"
#include "vehicle/PxVehicle.h"
#include "geometry/PxConvexMeshGeometry.h"
#include "PxBatchQueryDesc.h"

#include "VehicleManager.h"
#include "obj_Vehicle.h"
#include "VehicleDescriptor.h"

//////////////////////////////////////////////////////////////////////////

namespace
{
	void ConvertCoordinateSystems(PxRigidDynamic *a);
	physx::PxVehicleDrivableSurfaceType PX_ALIGN(16, drivableSurfaceType);

	const int MAX_NUM_SURFACE_TYPES = 2;
	const int MAX_NUM_TYRE_TYPES = 2;
	PxF32 PX_ALIGN(16, gTyreFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TYRE_TYPES])=
	{
		2.0f, 1.5f, 1.5f, 1.5f
	};

//////////////////////////////////////////////////////////////////////////

	void DriveVehiclesChangeCallback(int oldI, float oldF)
	{
		ObjectManager& GW = GameWorld();
		for (GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
		{
			if(obj->isObjType(OBJTYPE_Vehicle))
			{
				obj_Vehicle * vh = static_cast<obj_Vehicle*>(obj);
				vh->SwitchToDrivable(d_drive_vehicles->GetBool());
			}
		}
	}

//////////////////////////////////////////////////////////////////////////

	PxVehicleKeySmoothingData gKeySmoothingData=
	{
		{
			3.0f,	//rise rate PX_VEHICLE_ANALOG_INPUT_ACCEL		
			3.0f,	//rise rate PX_VEHICLE_ANALOG_INPUT_BRAKE		
			10.0f,	//rise rate PX_VEHICLE_ANALOG_INPUT_HANDBRAKE	
			2.5f,	//rise rate PX_VEHICLE_ANALOG_INPUT_STEER_LEFT	
			2.5f	//rise rate PX_VEHICLE_ANALOG_INPUT_STEER_RIGHT	
		},
		{
			5.0f,	//fall rate PX_VEHICLE_ANALOG_INPUT_ACCEL		
			5.0f,	//fall rate PX_VEHICLE_ANALOG_INPUT_BRAKE		
			10.0f,	//fall rate PX_VEHICLE_ANALOG_INPUT_HANDBRAKE	
			5.0f,	//fall rate PX_VEHICLE_ANALOG_INPUT_STEER_LEFT	
			5.0f	//fall rate PX_VEHICLE_ANALOG_INPUT_STEER_RIGHT	
		}
	};

	PxF32 gSteerVsForwardSpeedData[2*8]=
	{
		0.0f,		0.75f,
		5.0f,		0.75f,
		30.0f,		0.125f,
		120.0f,		0.1f,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32
	};
	PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData,4);

//////////////////////////////////////////////////////////////////////////

	void ComputeWheelWidthsAndRadii(PxConvexMesh** wheelConvexMeshes, PxF32* wheelWidths, PxF32* wheelRadii, PxU32 numWheels)
	{
		for(PxU32 i=0;i<numWheels;i++)
		{
			const PxU32 numWheelVerts=wheelConvexMeshes[i]->getNbVertices();
			const PxVec3* wheelVerts=wheelConvexMeshes[i]->getVertices();
			PxVec3 wheelMin(PX_MAX_F32,PX_MAX_F32,PX_MAX_F32);
			PxVec3 wheelMax(-PX_MAX_F32,-PX_MAX_F32,-PX_MAX_F32);
			for(PxU32 j=0;j<numWheelVerts;j++)
			{
				wheelMin.x=PxMin(wheelMin.x,wheelVerts[j].x);
				wheelMin.y=PxMin(wheelMin.y,wheelVerts[j].y);
				wheelMin.z=PxMin(wheelMin.z,wheelVerts[j].z);
				wheelMax.x=PxMax(wheelMax.x,wheelVerts[j].x);
				wheelMax.y=PxMax(wheelMax.y,wheelVerts[j].y);
				wheelMax.z=PxMax(wheelMax.z,wheelVerts[j].z);
			}
			wheelWidths[i]=wheelMax.x-wheelMin.x;
			wheelRadii[i]=PxMax(wheelMax.y,wheelMax.z);//*0.975f;
		}
	}

//////////////////////////////////////////////////////////////////////////

	PxVec3 ComputeChassisAABBDimensions(const PxConvexMesh* chassisConvexMesh)
	{
		const PxU32 numChassisVerts=chassisConvexMesh->getNbVertices();
		const PxVec3* chassisVerts=chassisConvexMesh->getVertices();
		PxVec3 chassisMin(PX_MAX_F32,PX_MAX_F32,PX_MAX_F32);
		PxVec3 chassisMax(-PX_MAX_F32,-PX_MAX_F32,-PX_MAX_F32);
		for(PxU32 i=0;i<numChassisVerts;i++)
		{
			chassisMin.x=PxMin(chassisMin.x,chassisVerts[i].x);
			chassisMin.y=PxMin(chassisMin.y,chassisVerts[i].y);
			chassisMin.z=PxMin(chassisMin.z,chassisVerts[i].z);
			chassisMax.x=PxMax(chassisMax.x,chassisVerts[i].x);
			chassisMax.y=PxMax(chassisMax.y,chassisVerts[i].y);
			chassisMax.z=PxMax(chassisMax.z,chassisVerts[i].z);
		}
		const PxVec3 chassisDims=chassisMax-chassisMin;
		return chassisDims;
	}

//////////////////////////////////////////////////////////////////////////

	void SetVehicleGeometricData
	(
		const PxVec3 &chassisDims,
		const PxVec3* const wheelCentreOffsets,
		const PxF32* const wheelWidths,
		const PxF32* const wheelRadii,
		PxVehicle4WSimpleSetup& simpleSetup
	)
	{
		//Chassis descriptor.
		simpleSetup.mChassisDims = chassisDims;
		simpleSetup.mChassisCMOffset=PxVec3(0.0f,-chassisDims.y*0.5f+0.65f,0.25f);

		//Wheels descriptor.
		for(PxU32 i=0;i<PxVehicle4WSimulationData::eNUM_WHEELS;i++)
		{
			simpleSetup.mWheelCentreCMOffsets[i]=wheelCentreOffsets[i]-simpleSetup.mChassisCMOffset;
		}
		simpleSetup.mFrontWheelWidth=wheelWidths[0];
		simpleSetup.mFrontWheelRadius=wheelRadii[0];
		simpleSetup.mRearWheelWidth=wheelWidths[2];
		simpleSetup.mRearWheelRadius=wheelRadii[2];

		//Suspension descriptor.
		simpleSetup.mFrontSuspensionTravelDir=PxVec3(0.0f,-1.0f,0.0f);
		simpleSetup.mRearSuspensionTravelDir=PxVec3(0.0f,-1.0f,0.0f);
	}

//////////////////////////////////////////////////////////////////////////
	
	PxVec3 ComputeChassisMOI(const PxVec3& chassisDims, const PxF32 chassisMass)
	{
		//We can approximately work out the chassis moment of inertia from the aabb.
		PxVec3 chassisMOI
			(chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z,
			chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z,
			chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y);
		chassisMOI*=chassisMass*0.0833f;
		//Well, the AABB moi gives rather sluggish behaviour so lets let the car turn a bit quicker.
		chassisMOI.y*=0.8f;//
		return chassisMOI;
	}

//////////////////////////////////////////////////////////////////////////

	void SetOptionalVehicleData(PxVehicle4WSimpleSetup& simpleSetup)
	{
		//Tyres descriptor.
		simpleSetup.mFrontTyreType=0;
		simpleSetup.mRearTyreType=0;

		//Diff descriptor.
		simpleSetup.mDiffType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
	}

//////////////////////////////////////////////////////////////////////////

	void SetVehicleMassData
	(
		const PxF32 chassisMass, const PxVec3& chassisMOI,
		PxVehicle4WSimpleSetup& simpleSetup
	)
	{
		simpleSetup.mChassisMass=chassisMass;
		simpleSetup.mChassisMOI=chassisMOI;
		simpleSetup.mFrontWheelMass=20.0f;
		simpleSetup.mRearWheelMass=20.0f;
	}

//////////////////////////////////////////////////////////////////////////

	PxSceneQueryHitType::Enum VehicleWheelRaycastPreFilter
	(	
		PxFilterData filterData0, 
		PxFilterData filterData1,
		const void* constantBlock,
		PxU32 constantBlockSize,
		PxSceneQueryFilterFlags& filterFlags
	)
	{
		//filterData0 is the vehicle suspension raycast.
		//filterData1 is the shape potentially hit by the raycast.
		PX_UNUSED(filterFlags);
		PX_UNUSED(constantBlockSize);
		PX_UNUSED(constantBlock);
		PX_UNUSED(filterData0);
		PxSceneQueryHitType::Enum ht = ((0 == (filterData1.word3 & VEHICLE_DRIVABLE_SURFACE)) ? PxSceneQueryHitType::eNONE : PxSceneQueryHitType::eBLOCK);
		return ht;
	}

//////////////////////////////////////////////////////////////////////////

	PxConvexMesh* CreateConvexMesh(const PxVec3* verts, const PxU32 numVerts)
	{
		PxPhysics& physics = *g_pPhysicsWorld->PhysXSDK;
		PxCooking& cooking = *g_pPhysicsWorld->Cooking;
		// Create descriptor for convex mesh
		PxConvexMeshDesc convexDesc;
		convexDesc.points.count			= numVerts;
		convexDesc.points.stride		= sizeof(PxVec3);
		convexDesc.points.data			= verts;
		convexDesc.flags				= PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eINFLATE_CONVEX;

		PxConvexMesh* convexMesh = NULL;
		MemoryWriteBuffer buf;
		if(cooking.cookConvexMesh(convexDesc, buf))
		{
			convexMesh = physics.createConvexMesh(MemoryReadBuffer(buf.data));
		}

		return convexMesh;
	}

//////////////////////////////////////////////////////////////////////////

	PxConvexMesh* CreateCylinderConvexMesh(const PxF32 width, const PxF32 radius, const PxU32 numCirclePoints)
	{
		#define  MAX_NUM_VERTS_IN_CIRCLE 16

		PxPhysics& physics = *g_pPhysicsWorld->PhysXSDK;
		PxCooking& cooking = *g_pPhysicsWorld->Cooking;

		r3d_assert(numCirclePoints<MAX_NUM_VERTS_IN_CIRCLE);
		PxVec3 verts[2*MAX_NUM_VERTS_IN_CIRCLE];
		PxU32 numVerts=2*numCirclePoints;
		const PxF32 dtheta=2*PxPi/(1.0f*numCirclePoints);
		for(PxU32 i=0;i<MAX_NUM_VERTS_IN_CIRCLE;i++)
		{
			const PxF32 theta=dtheta*i;
			const PxF32 cosTheta=radius*PxCos(theta);
			const PxF32 sinTheta=radius*PxSin(theta);
			verts[2*i+0]=PxVec3(-0.5f*width, cosTheta, sinTheta);
			verts[2*i+1]=PxVec3(+0.5f*width, cosTheta, sinTheta);
		}

		return CreateConvexMesh(verts,numVerts);
	}

//////////////////////////////////////////////////////////////////////////

	PxConvexMesh* CreateWheelConvexMesh(const PxVec3* verts, const PxU32 numVerts)
	{
		//Extract the wheel radius and width from the aabb of the wheel convex mesh.
		PxVec3 wheelMin(PX_MAX_F32,PX_MAX_F32,PX_MAX_F32);
		PxVec3 wheelMax(-PX_MAX_F32,-PX_MAX_F32,-PX_MAX_F32);
		for(PxU32 i=0;i<numVerts;i++)
		{
			wheelMin.x=PxMin(wheelMin.x,verts[i].x);
			wheelMin.y=PxMin(wheelMin.y,verts[i].y);
			wheelMin.z=PxMin(wheelMin.z,verts[i].z);
			wheelMax.x=PxMax(wheelMax.x,verts[i].x);
			wheelMax.y=PxMax(wheelMax.y,verts[i].y);
			wheelMax.z=PxMax(wheelMax.z,verts[i].z);
		}
		const PxF32 wheelWidth=wheelMax.x-wheelMin.x;
		const PxF32 wheelRadius=PxMax(wheelMax.y,wheelMax.z);

		return CreateCylinderConvexMesh(wheelWidth,wheelRadius,8);
	}

//////////////////////////////////////////////////////////////////////////

	void SetupShapesUserData(const VehicleDescriptor &vd)
	{
		PxRigidDynamic *a = vd.vehicle->mActor;
		if (!a)
			return;

		PxU32 numShapes = a->getNbShapes();
		r3d_assert(numShapes <= vd.numWheels + vd.numHullParts);
		
		for (PxU32 i = 0; i < numShapes; ++i)
		{
			PxShape *s = 0;
			a->getShapes(&s, 1, i);
			if (!s)
				continue;

			if (i < vd.numWheels)
			{
				s->userData = reinterpret_cast<void*>(vd.wheelBonesRemapIndices[i]);
			}
			else
			{
				s->userData = reinterpret_cast<void*>(vd.hullBonesRemapIndices[i - vd.numWheels]);
			}
		}
	}

//////////////////////////////////////////////////////////////////////////

	void dampVec3(const PxVec3& oldPosition, PxVec3& newPosition, PxF32 timestep)
	{
		PxF32 t = 0.7f * timestep * 8.0f;
		t = PxMin(t, 1.0f);
		newPosition = oldPosition * (1 - t) + newPosition * t;
	}

} // unnamed namespace

PxF32 &gVehicleTireGroundFriction = gTyreFrictionMultipliers[0][0];

//////////////////////////////////////////////////////////////////////////

VehicleManager::VehicleManager()
: batchSuspensionRaycasts(0)
, drivableCar(0)
, clearInputData(true)
, mAtRestUnderBraking(true)
, mTimeElapsedSinceAtRestUnderBraking(0.0f)
, mInReverseMode(false)
{
	r3d_assert(g_pPhysicsWorld);
	r3d_assert(g_pPhysicsWorld->PhysXSDK);
	r3d_assert(g_pPhysicsWorld->PhysXScene);

	d_drive_vehicles->SetChangeCallback(&DriveVehiclesChangeCallback);

	drivableSurfaceType.mType = 0;
	g_pPhysicsWorld->defaultMaterial->userData = &drivableSurfaceType;
}

//////////////////////////////////////////////////////////////////////////

VehicleManager::~VehicleManager()
{
	ClearSuspensionRaycatsQuery();

	while (vehicles.Count() > 0)
	{
		DeleteCar(vehicles.GetFirst());
	}
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::Update(float timeStep)
{
	if (vehicles.Count() == 0 || !d_drive_vehicles->GetBool())
		return;

	if (!batchSuspensionRaycasts)
	{
		batchSuspensionRaycasts = SetUpBatchedSceneQuery();
	}

	physxVehs.Resize(vehicles.Count());
	for (uint32_t i = 0, i_end = physxVehs.Count(); i < i_end; ++i)
	{
		physxVehs[i] = vehicles[i]->vehicle;
	}

	DoUserCarControl(timeStep);	

	PxVehicle4WSuspensionRaycasts(batchSuspensionRaycasts, vehicles.Count(), &batchQueryResults.GetFirst(), &physxVehs[0]);

	PxVehicleDrivableSurfaceToTyreFrictionPairs surfaceTyrePairs;
	surfaceTyrePairs.mPairs = &gTyreFrictionMultipliers[0][0];
	surfaceTyrePairs.mNumSurfaceTypes=MAX_NUM_SURFACE_TYPES;
	surfaceTyrePairs.mNumTyreTypes=MAX_NUM_TYRE_TYPES;
	PxVehicle4WUpdate(timeStep,g_pPhysicsWorld->PhysXScene->getGravity(),surfaceTyrePairs,vehicles.Count(),&physxVehs[0]);

	cameraContoller.Update(timeStep);
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::DoUserCarControl(float timeStep)
{
	if (!drivableCar)
		return;

	PxVehicle4W &car = *drivableCar->vehicle;

	//Work out if the car is to flip from reverse to forward gear or from forward gear to reverse.
	bool toggleAutoReverse = false;
	if(PxVehicle4WGetUseAutoGears(car))
	{
		toggleAutoReverse = ProcessAutoReverse(timeStep);
	}

	//If the car is to flip gear direction then switch gear as appropriate.
	if(toggleAutoReverse)
	{
		mInReverseMode = !mInReverseMode;

		if(mInReverseMode)
		{
			PxVehicle4WForceGearChange(PxVehicleGearsData::eREVERSE, car);
		}
		else
		{
			PxVehicle4WForceGearChange(PxVehicleGearsData::eFIRST, car);
		}
	}

	//If in reverse mode then swap the accel and brake.
	if(mInReverseMode)
	{
		const bool accel = carControlData.getDigitalAccel();
		const bool brake = carControlData.getDigitalBrake();
		carControlData.setDigitalAccel(brake);
		carControlData.setDigitalBrake(accel);
	}

	PxVehicle4WSmoothDigitalRawInputsAndSetAnalogInputs(gKeySmoothingData, gSteerVsForwardSpeedTable, carControlData, timeStep, car);
	clearInputData = true;
}

//////////////////////////////////////////////////////////////////////////

#define THRESHOLD_FORWARD_SPEED (5e-2f)
#define THRESHOLD_SIDEWAYS_SPEED (1e-2f)

bool VehicleManager::ProcessAutoReverse(float timestep)
{
	if (!drivableCar || !drivableCar->vehicle)
		return false;

	PxVehicle4W &car = *drivableCar->vehicle;

	bool accelRaw, brakeRaw;
	accelRaw = carControlData.getDigitalAccel();
	brakeRaw = carControlData.getDigitalBrake();

	const bool accel = mInReverseMode ? brakeRaw : accelRaw;
	const bool brake = mInReverseMode ? accelRaw : brakeRaw;

	//If the car has been brought to rest by pressing the brake then raise a flag.
	bool justRaisedFlag = false;
	if(brake && !mAtRestUnderBraking)
	{
		bool isInAir = PxVehicle4WIsInAir(car);
		PxF32 forwardSpeed = PxAbs(PxVehicle4WComputeForwardSpeed(car));
		PxF32 sidewaysSpeed = PxAbs(PxVehicle4WComputeSidewaysSpeed(car));

		if(!isInAir && forwardSpeed < THRESHOLD_FORWARD_SPEED && sidewaysSpeed < THRESHOLD_SIDEWAYS_SPEED)
		{
			//justRaisedFlag = true;
			mAtRestUnderBraking = true;
			mTimeElapsedSinceAtRestUnderBraking = 0.0f;
		}
	}

	//If the flag is raised and the player pressed accelerate then lower the flag.
	if(mAtRestUnderBraking && accel)
	{	
		mAtRestUnderBraking = false;
		mTimeElapsedSinceAtRestUnderBraking = 0.0f;
	}

	//If the flag is raised and the player doesn't press brake then increment the timer.
	if(mAtRestUnderBraking && !justRaisedFlag)
	{
		mTimeElapsedSinceAtRestUnderBraking += timestep;
	}

	//If the flag is raised and the player pressed brake again then switch auto-reverse.
	if(mAtRestUnderBraking && !justRaisedFlag && mTimeElapsedSinceAtRestUnderBraking > 0.0f)
	{
		mAtRestUnderBraking = false;
		mTimeElapsedSinceAtRestUnderBraking = 0.0f;
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::UpdateInput()
{
	if (clearInputData)
	{
		carControlData.setDigitalAccel(false);
		carControlData.setDigitalBrake(false);
		carControlData.setDigitalSteerLeft(false);
		carControlData.setDigitalSteerRight(false);
		clearInputData = false;
	}
	carControlData.setDigitalAccel(carControlData.getDigitalAccel() || Keyboard->IsPressed(kbsUp));
	carControlData.setDigitalBrake(carControlData.getDigitalBrake() || Keyboard->IsPressed(kbsDown));
	carControlData.setDigitalSteerLeft(carControlData.getDigitalSteerLeft() || Keyboard->IsPressed(kbsLeft));
	carControlData.setDigitalSteerRight(carControlData.getDigitalSteerRight() || Keyboard->IsPressed(kbsRight));
}

//////////////////////////////////////////////////////////////////////////

VehicleDescriptor * VehicleManager::CreateVehicle(const r3dMesh *m)
{
	if (!m)
		return 0;

	typedef r3dTL::TArray<PxVec3> Vertices;
	typedef r3dTL::TArray<Vertices> Meshes;

	std::auto_ptr<VehicleDescriptor> vd(new VehicleDescriptor);
	if (!vd->Init(m))
	{
		return 0;
	}

	Meshes wheelMeshes;
	wheelMeshes.Resize(vd->numWheels);
	Meshes hullMeshes;
	hullMeshes.Resize(vd->numHullParts);

	D3DXQUATERNION test;
	D3DXQuaternionRotationYawPitchRoll(&test, D3DX_PI / 2, 0, 0); // rotY
	PxQuat rotQ(test.x, test.y, test.z, test.w);

	for (int i = 0; i < m->NumVertices; ++i)
	{
		const r3dMesh::r3dWeight &w = m->pWeights[i];
		BYTE boneId = w.BoneID[0];
		const r3dBone &bone = vd->skl->Bones[boneId];
		Meshes *meshArr = 0;
		uint32_t meshIdx = INVALID_INDEX;
		uint32_t wheelIdx = vd->GetWheelIndex(boneId);
		uint32_t hullIdx = vd->GetHullIndex(boneId);
		
		if (wheelIdx != INCORRECT_INDEX)
		{
			meshArr = &wheelMeshes;
			meshIdx = wheelIdx;
		}
		else if (hullIdx != INCORRECT_INDEX)
		{
			meshArr = &hullMeshes;
			meshIdx = hullIdx;
		}
		else
		{
			continue;
		}

		Vertices &v = (*meshArr)[meshIdx];
		r3dPoint3D tmp = m->VertexPositions[i];
		PxVec3 bonePos(bone.vRelPlacement.x, bone.vRelPlacement.y, bone.vRelPlacement.z);
		PxVec3 pt(tmp.x, tmp.y, tmp.z);
		pt -= bonePos;
		pt = rotQ.rotate(pt);
		v.PushBack(pt);
	}

	// Hull mesh
	PxConvexMesh *hull = CreateConvexMesh(&hullMeshes[0][0], hullMeshes[0].Count());

	// Wheel meshes
	PxConvexMesh **wheels = reinterpret_cast<PxConvexMesh**>(_alloca(sizeof(PxConvexMesh*) * wheelMeshes.Count()));
	for (uint32_t i = 0; i < wheelMeshes.Count(); ++i)
	{
		Vertices &v = wheelMeshes[i];
		wheels[i] = CreateWheelConvexMesh(&v[0], v.Count());
	}

	const PxVec3 chasisDims = ComputeChassisAABBDimensions(hull);
	const PxVec3 chassisMOI = ComputeChassisMOI(chasisDims, vd->chassisData.mMass);

	PxVehicle4WSimpleSetup simpleSetup;
	PxF32 wheelWidths[MAX_WHEELS_COUNT];
	PxF32 wheelRadii[MAX_WHEELS_COUNT];
	ComputeWheelWidthsAndRadii(wheels,wheelWidths,wheelRadii, vd->numWheels);
	SetVehicleGeometricData(chasisDims, vd->wheelCenterOffsets, wheelWidths, wheelRadii, simpleSetup);
	SetVehicleMassData(vd->chassisData.mMass, chassisMOI, simpleSetup);
	SetOptionalVehicleData(simpleSetup);

	PxVehicle4WSimulationData vehSimulationData=PxCreateVehicle4WSimulationData(simpleSetup);

	r3d_assert(vd->wheelsData.Count() <= PxVehicle4WSimulationData::eNUM_WHEELS);
	for (uint32_t i = 0; i < vd->wheelsData.Count(); ++i)
	{
		VehicleDescriptor::WheelData &wd = vd->wheelsData[i];
		wd.wheelData.mWidth = wheelWidths[i];
		wd.wheelData.mRadius = wheelRadii[i];
		wd.wheelData.mMOI = vehSimulationData.getWheelData(i).mMOI;
		wd.suspensionData.mSprungMass = vehSimulationData.getSuspensionData(i).mSprungMass;
	}

	vd->chassisData.mCMOffset += vehSimulationData.getChassisData().mCMOffset;
	vd->chassisData.mMOI = chassisMOI;

	//We need a rigid body actor for the vehicle.
	//Don't forget to add the actor the scene after setting up the associated vehicle.
	PxRigidDynamic* vehActor = g_pPhysicsWorld->PhysXSDK->createRigidDynamic(PxTransform::createIdentity());

	//We need to add wheel collision shapes, a material for the wheels, and a simulation filter for the wheels.
	PxConvexMeshGeometry frontLeftWheelGeom(wheels[0]);
	PxConvexMeshGeometry frontRightWheelGeom(wheels[1]);
	PxConvexMeshGeometry rearLeftWheelGeom(wheels[2]);
	PxConvexMeshGeometry rearRightWheelGeom(wheels[3]);
	PxMaterial& wheelMaterial=*g_pPhysicsWorld->defaultMaterial;
	PxFilterData wheelCollFilterData;
	wheelCollFilterData.word0=PHYSCOLL_VEHICLE_WHEEL;

	//We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
	PxConvexMeshGeometry chassisConvexGeom(hull);
	PxGeometry* chassisGeoms[1]={&chassisConvexGeom};

	//We need to specify the local poses of the chassis composite shapes.
	PxTransform chassisLocalPoses[1]={PxTransform::createIdentity()};

	PxMaterial& chassisMaterial=*g_pPhysicsWorld->defaultMaterial;
	PxFilterData chassisCollFilterData;
	chassisCollFilterData.word0=PHYSCOLL_STATIC_GEOMETRY;

	//Create a query filter data for the car to ensure that cars
	//do not attempt to drive on themselves.
	const PxU32 vehIndex = vehicles.Count() + 1;
	PxFilterData vehQryFilterData;
	PxSetupVehicleShapeQueryFilterData(vehIndex,&vehQryFilterData);

	//Create a car.
	vd->vehicle = new PxVehicle4W;
	PxVehicle4WSetup(
		vehSimulationData,
		vehQryFilterData,vehActor,
		frontLeftWheelGeom,frontRightWheelGeom,rearLeftWheelGeom,rearRightWheelGeom,&wheelMaterial,wheelCollFilterData,
		chassisGeoms,chassisLocalPoses,1,&chassisMaterial,chassisCollFilterData,
		g_pPhysicsWorld->PhysXSDK,
		vd->vehicle);

	//Don't forget to add the actor to the scene.
	g_pPhysicsWorld->PhysXScene->addActor(*vehActor);

	PxVehicle4WSetUseAutoGears(true, *vd->vehicle);
	ConfigureSuspensionRaycasts(*vd);

	SetupShapesUserData(*vd);

	PxVehicle4WSetToRestState(*vd->vehicle);

	vd->ApplyDynamicChanges();

	VehicleDescriptor *rv = vd.release();
	vehicles.PushBack(rv);

	return rv;
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::ConfigureSuspensionRaycasts(const VehicleDescriptor &car)
{
	//	Increase suspension raycasts buffers
	batchHits.Resize(batchHits.Count() + car.numWheels);
	batchQueryResults.Resize(batchQueryResults.Count() + car.numWheels);

	//	Clear current batch query, it will be recreated in next update with new data
	ClearSuspensionRaycatsQuery();
}

//////////////////////////////////////////////////////////////////////////

PxBatchQuery * VehicleManager::SetUpBatchedSceneQuery()
{
	PxBatchQueryDesc bqd;
	bqd.userRaycastHitBuffer = &batchHits.GetFirst();
	bqd.userRaycastResultBuffer = &batchQueryResults.GetFirst();
	bqd.raycastHitBufferSize = batchHits.Count();
	bqd.preFilterShader = &VehicleWheelRaycastPreFilter;
	return g_pPhysicsWorld->PhysXScene->createBatchQuery(bqd);
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::DeleteCar(VehicleDescriptor *car)
{
	if (!car)
		return;

	for (uint32_t i = 0; i < vehicles.Count(); ++i)
	{
		if (car == vehicles[i])
		{
			vehicles.Erase(i);
			PxU32 numWheels = car->numWheels;
			r3d_assert(batchHits.Count() >= numWheels);
			r3d_assert(batchQueryResults.Count() >= numWheels);
			r3d_assert(batchQueryResults.Count() == batchHits.Count());
			batchHits.Resize(batchHits.Count() - numWheels);
			batchQueryResults.Resize(batchQueryResults.Count() - numWheels);
			ClearSuspensionRaycatsQuery();
			PxRigidDynamic *a = car->vehicle->mActor;
			g_pPhysicsWorld->PhysXScene->removeActor(*a);
			a->release();

			//	Clear drivable car deleted car, clear pointer either
			if (drivableCar == car)
				drivableCar = 0;

			/**	This is intended memory leak, because SDK release method is crashing if we delete PxVehicle4W*. PhysX 3.2 does not have this error, so waiting for upgrade. */
			//delete car;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::ClearSuspensionRaycatsQuery()
{
	if (batchSuspensionRaycasts)
	{
		batchSuspensionRaycasts->release();
		batchSuspensionRaycasts = 0;
	}
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::DriveCar(VehicleDescriptor *car)
{
	drivableCar = car;
	if (car)
		cameraContoller.SetDrivenVehicle(car->vehicle->mActor);
	else
		cameraContoller.SetDrivenVehicle(0);
}

//////////////////////////////////////////////////////////////////////////

void VehicleManager::UpdateVehiclePoses()
{
	for (uint32_t i = 0; i < vehicles.Count(); ++i)
	{
		VehicleDescriptor *vd = vehicles[i];
		if (vd && vd->owner)
		{
			vd->owner->UpdatePositionFromPhysx();
		}
	}
}

//////////////////////////////////////////////////////////////////////////

obj_Vehicle* VehicleManager::getRealDrivenVehicle()
{
	ObjectManager& GW = GameWorld();
	for (GameObject *obj = GW.GetFirstObject(); obj; obj = GW.GetNextObject(obj))
	{
		if(obj->isObjType(OBJTYPE_Vehicle))
		{
			obj_Vehicle * vh = static_cast<obj_Vehicle*>(obj);
			if ( vh->getVehicleDescriptor() == drivableCar ){
				return vh;
			}
			
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

bool VehicleManager::ConfigureCamera(r3dCamera &cam)
{
	return cameraContoller.ConfigureCamera(cam);
}

//-------------------------------------------------------------------------
//	Vehicle camera controller
//-------------------------------------------------------------------------

VehicleCameraController::VehicleCameraController()
: mCameraPos(0, 0, 0)
, mCameraTargetPos(0, 0, 0)
, mLastCarPos(0, 0, 0)
, mLastCarVelocity(0, 0, 0)
, mCameraInit(false)
, mLastFocusVehTransform(PxTransform::createIdentity())
, mCameraRotateAngleY(0)
, mCameraRotateAngleZ(0.38f)
, mCounter(0)
{
	ResetHistory(mAccelXHistory, 0);
	ResetHistory(mAccelZHistory, 0);
}

//////////////////////////////////////////////////////////////////////////

bool VehicleCameraController::ConfigureCamera(r3dCamera &Cam)
{
	if (!actor)
		return false;

	Cam.SetPosition(r3dVector(mCameraPos.x, mCameraPos.y, mCameraPos.z));
	Cam.PointTo(r3dVector(mCameraTargetPos.x, mCameraTargetPos.y, mCameraTargetPos.z));
	Cam.vUP = r3dPoint3D(0, 1, 0);
	return true;
}

//////////////////////////////////////////////////////////////////////////

void VehicleCameraController::SetDrivenVehicle(PxRigidDynamic *a)
{
	actor = a;
	mCameraInit = false;
	mLastCarPos = a->getGlobalPose().p;
}

//////////////////////////////////////////////////////////////////////////

void VehicleCameraController::Update(float dtime)
{
	if (!actor)
		return;

	PxTransform carChassisTransfm(actor->getGlobalPose());

	PxF32 camDist = 10.0f;
	PxF32 cameraYRotExtra = 0.0f;

	PxVec3 velocity = mLastCarPos - carChassisTransfm.p;

	int idx = mCounter % FloatHistory::COUNT;
	mCounter++;
	if (mCameraInit)
	{
		//Work out the forward and sideways directions.
		PxVec3 unitZ(0,0,1);
		PxVec3 carDirection = carChassisTransfm.q.rotate(unitZ);
		PxVec3 unitX(1,0,0);
		PxVec3 carSideDirection = carChassisTransfm.q.rotate(unitX);

		//Acceleration (note that is not scaled by time).
		PxVec3 acclVec = mLastCarVelocity - velocity;

		//Higher forward accelerations allow the car to speed away from the camera.
		PxF32 acclZ = carDirection.dot(acclVec);
		mAccelZHistory[idx] = acclZ;
		acclZ = HistoryAverage(mAccelZHistory);
		camDist = PxMax(camDist+acclZ*400.0f, 5.0f);

		//Higher sideways accelerations allow the car's rotation to speed away from the camera's rotation.
		PxF32 acclX = carSideDirection.dot(acclVec);
		mAccelXHistory[idx] = acclX;
		acclX = HistoryAverage(mAccelXHistory);
		cameraYRotExtra = -acclX * 20.0f;

		//At very small sideways speeds the camera greatly amplifies any numeric error in the body and leads to a slight jitter.
		//Scale cameraYRotExtra by a value in range (0,1) for side speeds in range (0.1,1.0) and by zero for side speeds less than 0.1.
		PxFixedSizeLookupTable<4> table;
		table.addPair(0.0f, 0.0f);
		table.addPair(0.1f * dtime, 0);
		table.addPair(1.0f * dtime, 1);
		PxF32 velX = carSideDirection.dot(velocity);
		cameraYRotExtra *= table.getYVal(PxAbs(velX));
	}

	mCameraRotateAngleY=physx::intrinsics::fsel(mCameraRotateAngleY-10*PxPi, mCameraRotateAngleY-10*PxPi, physx::intrinsics::fsel(-mCameraRotateAngleY-10*PxPi, mCameraRotateAngleY + 10*PxPi, mCameraRotateAngleY));
	mCameraRotateAngleZ=PxClamp(mCameraRotateAngleZ,-PxPi*0.05f,PxPi*0.45f);

	PxVec3 cameraDir=PxVec3(0,0,1)*PxCos(mCameraRotateAngleY+cameraYRotExtra) + PxVec3(1,0,0) * PxSin(cameraYRotExtra);

	cameraDir=cameraDir*PxCos(mCameraRotateAngleZ)-PxVec3(0,1,0)*PxSin(mCameraRotateAngleZ);

	const PxVec3 direction = carChassisTransfm.q.rotate(cameraDir);
	PxVec3 target = carChassisTransfm.p;
	target.y += 0.5f;

	camDist = PxMax(5.0f, PxMin(camDist, 50.0f));

	PxVec3 position = target-direction*camDist;

	if (mCameraInit)
	{
		dampVec3(mCameraPos, position, dtime);
		dampVec3(mCameraTargetPos, target, dtime);
	}

	mCameraPos = position;
	mCameraTargetPos = target;
	mCameraInit = true;

	mLastCarVelocity = velocity;
	mLastCarPos = carChassisTransfm.p;
}

//////////////////////////////////////////////////////////////////////////

float VehicleCameraController::HistoryAverage(const FloatHistory &fh) const
{
	float rv = 0;
	for (int i = 0; i < FloatHistory::COUNT; ++i)
	{
		rv += fh[i];
	}
	rv /= FloatHistory::COUNT;
	return rv;
}

//////////////////////////////////////////////////////////////////////////

void VehicleCameraController::ResetHistory(FloatHistory &fh, float v)
{
	for (size_t i = 0; i < FloatHistory::COUNT; ++i)
	{
		fh[i] = v;
	}
}

//-------------------------------------------------------------------------
//	Standalone helper functions
//-------------------------------------------------------------------------

void VehicleSetupDrivableShapeQueryFilterData(PxFilterData &qryFilterData)
{
	r3d_assert(qryFilterData.word3 == 0 || qryFilterData.word3 == VEHICLE_DRIVABLE_SURFACE || qryFilterData.word3 == VEHICLE_NONDRIVABLE_SURFACE);
	qryFilterData.word3 = VEHICLE_DRIVABLE_SURFACE;
}

//////////////////////////////////////////////////////////////////////////

void VehicleSetupNonDrivableShapeQueryFilterData(PxFilterData &qryFilterData)
{
	r3d_assert(qryFilterData.word3 == 0 || qryFilterData.word3 == VEHICLE_NONDRIVABLE_SURFACE || qryFilterData.word3 == VEHICLE_NONDRIVABLE_SURFACE);
	qryFilterData.word3 = VEHICLE_NONDRIVABLE_SURFACE;
}

#endif // VEHICLES_ENABLED