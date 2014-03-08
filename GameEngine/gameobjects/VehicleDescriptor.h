//=========================================================================
//	Module: VehicleDescriptor.h
//	Copyright (C) 2012.
//=========================================================================

#pragma once

#if VEHICLES_ENABLED

#include "vehicle/PxVehicle.h"

//////////////////////////////////////////////////////////////////////////

const char * const VEHICLE_PART_NAMES[] =
{
	"Bone_Root",
	"Bone_Wheel_002",
	"Bone_Wheel_001",
	"Bone_Wheel_004",
	"Bone_Wheel_003",
// 	"Bone_Wheel_005",
// 	"Bone_Wheel_006",
// 	"Bone_Wheel_007",
// 	"Bone_Wheel_008",
// 	"Bone_Wheel_009",
// 	"Bone_Wheel_010",
// 	"Bone_Wheel_011",
// 	"Bone_Wheel_012",
// 	"Bone_Wheel_013",
// 	"Bone_Wheel_014",
// 	"Bone_Wheel_015",
// 	"Bone_Wheel_016"
};

const char * const VEHICLE_EXIT_NAMES[] =
{
	"ExitPos0",
	"ExitPos1",
	"ExitPos2",
	"ExitPos3",
};

const uint32_t VEHICLE_PARTS_COUNT = _countof(VEHICLE_PART_NAMES);
const uint32_t MAX_HULL_PARTS_COUNT = 1;
const uint32_t MAX_WHEELS_COUNT = VEHICLE_PARTS_COUNT - MAX_HULL_PARTS_COUNT;
const uint32_t MAX_EXITS_COUNT = 4;

const uint32_t INCORRECT_INDEX = 0xffffffff;

class obj_Vehicle;

//////////////////////////////////////////////////////////////////////////

struct VehicleDescriptor
{
	physx::PxVehicle4W *vehicle;
	r3dSkeleton *skl;
	std::string driveFileDefinitionPath;

	PxVehicleAckermannGeometryData ackermannData;
	PxVehicleChassisData chassisData;
	PxVehicleEngineData engineData;
	PxVehicleGearsData gearsData;
	PxVehicleClutchData clutchData;
	PxVehicleDifferential4WData diffData;
	struct WheelData
	{
		PxVehicleWheelData wheelData;
		PxVehicleSuspensionData suspensionData;
		PxVehicleTyreData tireData;
		PxVec3 suspensionTravelDir;
		WheelData()
		: suspensionTravelDir(0, -1, 0)
		{
			wheelData.mRadius = 1.0f;
 			suspensionData.mSpringStrength = 35000.0f;
 			suspensionData.mSpringDamperRate = 4500.0f;
			wheelData.mMOI = 1.0f;
		}
	};
	r3dTL::TArray<WheelData> wheelsData;

	uint32_t numWheels;
	uint32_t numHullParts;

	/**
	* Wheel remap indices. Bone indices in order corresponding strings in VEHICLE_PART_NAMES array
	*/
	uint32_t wheelBonesRemapIndices[MAX_WHEELS_COUNT];
	uint32_t hullBonesRemapIndices[MAX_HULL_PARTS_COUNT];

	PxVec3 wheelCenterOffsets[MAX_WHEELS_COUNT];
	PxVec3 hullCenterOffsets[MAX_HULL_PARTS_COUNT];
	PxVec3 exitCenterOffsets[MAX_EXITS_COUNT];

	obj_Vehicle *owner;

	VehicleDescriptor();
	~VehicleDescriptor();
	bool Init(const r3dMesh *m);

	/** Get wheel index corresponding given bone. If requested bone not a wheel bone, return INCORRECT_INDEX.  */
	uint32_t GetWheelIndex(uint32_t boneId) const;
	
	/** Get hull index corresponding given bone. If requested bone not a hull bone, return INCORRECT_INDEX.  */
	uint32_t GetHullIndex(uint32_t boneId) const;

	void ApplyDynamicChanges();

	/** Save vehicle parameters to xml file. */
	bool Save(const char *fileName);

	// true if found, false if not found. 
	bool GetExitIndex( r3dVector& outVector, uint32_t exitIndex) const;

private:
	uint32_t GetIndex(const uint32_t *arr, uint32_t arrCount, uint32_t boneIndex) const;
	bool Load(const r3dMesh *m);
	bool InitSkeleton(const r3dSkeleton *s);
	void InitToDefault();
};

#endif VEHICLES_ENABLED