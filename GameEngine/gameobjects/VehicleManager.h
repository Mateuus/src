//=========================================================================
//	Module: VehicleManager.h
//	Copyright (C) 2012.
//=========================================================================

#pragma once

class VehicleManager;

#if VEHICLES_ENABLED

#include "PxBatchQuery.h"
#include "vehicle/PxVehicle.h"
#include "vehicle/PxVehicleUtils.h"

#include "VehicleDescriptor.h"

//////////////////////////////////////////////////////////////////////////

const uint32_t VEHICLE_DRIVABLE_SURFACE = 0xffff0000;
const uint32_t VEHICLE_NONDRIVABLE_SURFACE = 0xffff;

//Set up query filter data so that vehicles can drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void VehicleSetupDrivableShapeQueryFilterData(PxFilterData &qryFilterData);

//Set up query filter data so that vehicles cannot drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void VehicleSetupNonDrivableShapeQueryFilterData(PxFilterData &qryFilterData);

//////////////////////////////////////////////////////////////////////////

class obj_Vehicle;

//////////////////////////////////////////////////////////////////////////

class VehicleCameraController
{
	PxVec3 mCameraPos;
	PxVec3 mCameraTargetPos;
	PxVec3 mLastCarPos;
	PxVec3 mLastCarVelocity;
	bool mCameraInit;
	PxTransform mLastFocusVehTransform;
	float mCameraRotateAngleY;
	float mCameraRotateAngleZ;

	typedef r3dTL::TFixedArray<float, 10> FloatHistory;
	FloatHistory mAccelZHistory;
	FloatHistory mAccelXHistory;
	int mCounter;

	PxRigidDynamic *actor;

	void ResetHistory(FloatHistory &fh, float v);
	float HistoryAverage(const FloatHistory &fh) const;

public:
	VehicleCameraController();

	/** Update camera params for current driven vehicle */
	void Update(float dtime);

	/**	Configure camera position and orientation for current driven vehicle. */
	bool ConfigureCamera(r3dCamera &cam);
	void SetDrivenVehicle(PxRigidDynamic *a);
};

//////////////////////////////////////////////////////////////////////////

class VehicleManager
{
	r3dTL::TArray<VehicleDescriptor*> vehicles;
	r3dTL::TArray<PxVehicle4W*> physxVehs;
	//	One result for each wheel in each car.
	r3dTL::TArray<PxRaycastQueryResult> batchQueryResults;
	//	One hit for each wheel in each car.
	r3dTL::TArray<PxRaycastHit> batchHits;

	//	Suspension raycasts batched query
	physx::PxBatchQuery * batchSuspensionRaycasts;

	//	Tire-surface friction table;
	//PxVehicleDrivableSurfaceToTireFrictionPairs *frictionData;

	void ConfigureSuspensionRaycasts(const VehicleDescriptor &car);
	void ClearSuspensionRaycatsQuery();

	physx::PxBatchQuery * SetUpBatchedSceneQuery();
	void IssueSuspensionRaycasts();

	PxVehicleRawInputData carControlData;
	bool mAtRestUnderBraking;
	float mTimeElapsedSinceAtRestUnderBraking;
	bool mInReverseMode;

	bool clearInputData;
	void DoUserCarControl(float timeStep);
	bool ProcessAutoReverse(float timestep);

	VehicleDescriptor *drivableCar;
	VehicleCameraController cameraContoller;

public:
	VehicleManager();
	~VehicleManager();
	void Update(float timeStep);
	VehicleDescriptor * CreateVehicle(const r3dMesh *m);

	void DeleteCar(VehicleDescriptor *car);
	void DriveCar(VehicleDescriptor *car);
	const VehicleDescriptor * GetDrivenCar() const { return drivableCar; }
	void UpdateInput();
	obj_Vehicle* getRealDrivenVehicle();
	/** Update vehicle poses from physx. */
	void UpdateVehiclePoses();
	bool ConfigureCamera(r3dCamera &cam);
};

//////////////////////////////////////////////////////////////////////////

#endif // VEHICLES_ENABLED