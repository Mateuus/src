//=========================================================================
//	Module: obj_Vehicle.hpp
//	Copyright (C) 2011. Online Warmongers Group Inc. All rights reserved
//=========================================================================

#pragma once

#include "GameObj.h"
#include "vehicle/PxVehicle.h"
#include "../../EclipseStudio/Sources/multiplayer/NetCellMover.h"
//////////////////////////////////////////////////////////////////////////

#if VEHICLES_ENABLED

class obj_VehicleSpawn;
struct VehicleDescriptor;

class obj_Vehicle: public MeshGameObject
{
	DECLARE_CLASS(obj_Vehicle, MeshGameObject)

	VehicleDescriptor *vd;

	CNetCellMover	netMover;
	r3dPoint3D	netVelocity;
	
	void SyncPhysicsPoseWithObjectPose();
	void SetBoneMatrices();

public:
	obj_Vehicle();
	~obj_Vehicle();
	virtual BOOL Update();
	virtual BOOL OnCreate();
	virtual BOOL OnDestroy();
	virtual void SetPosition(const r3dPoint3D& pos);
	virtual	void SetRotationVector(const r3dVector& Angles);
	virtual void OnPreRender() { SetBoneMatrices(); }

	void SwitchToDrivable(bool doDrive);
	const VehicleDescriptor* getVehicleDescriptor() { return vd; }
#ifndef FINAL_BUILD
	float DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected);
#endif
	const bool getExitSpace( r3dVector& outVector, int exitIndex );
	void setVehicleSpawner( obj_VehicleSpawn* targetSpawner) { spawner = targetSpawner;}
	void UpdatePositionFromRemote();
	void UpdatePositionFromPhysx();
	void OnNetPacket(const PKT_C2C_MoveSetCell_s& n);
	void OnNetPacket(const PKT_C2C_MoveRel_s& n);
	BOOL OnNetReceive(DWORD EventID, const void* packetData, int packetSize);
private:
	obj_VehicleSpawn* spawner;

};

#endif // VEHICLES_ENABLED