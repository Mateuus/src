//=========================================================================
//	Module: obj_Vehicle.cpp
//	Copyright (C) 2011. Online Warmongers Group Inc. All rights reserved
//=========================================================================

#include "r3dPCH.h"

#if VEHICLES_ENABLED

#include "r3d.h"
#include "PhysXWorld.h"
#include "ObjManag.h"
#include "GameCommon.h"

#include "obj_Vehicle.h"
#include "VehicleDescriptor.h"

#include "../../EclipseStudio/Sources/Editors/ObjectManipulator3d.h"
#include "../../EclipseStudio/Sources/ObjectsCode/Gameplay/obj_VehicleSpawn.h"

extern bool g_bEditMode;
extern ObjectManipulator3d g_Manipulator3d;
extern int CurHUDID;

//////////////////////////////////////////////////////////////////////////

namespace
{
	void QuaternionToEulerAngles(PxQuat &q, float &xRot, float &yRot, float &zRot)
	{
		q.normalize();

		void MatrixGetYawPitchRoll ( const D3DXMATRIX & mat, float & fYaw, float & fPitch, float & fRoll );
		PxMat33 mat(q);
		D3DXMATRIX res;
		D3DXMatrixIdentity(&res);
		res._11 = mat.column0.x;
		res._12 = mat.column0.y;
		res._13 = mat.column0.z;

		res._21 = mat.column1.x;
		res._22 = mat.column1.y;
		res._23 = mat.column1.z;

		res._31 = mat.column2.x;
		res._32 = mat.column2.y;
		res._33 = mat.column2.z;

		MatrixGetYawPitchRoll(res, xRot, yRot, zRot);
	}

//////////////////////////////////////////////////////////////////////////

	/** Helper constant transformation factors */
	struct UsefulTransforms 
	{
		PxQuat rotY_quat;
		D3DXMATRIX rotY_mat;

		UsefulTransforms()
		{
			D3DXQUATERNION rotY_D3D;
			D3DXQuaternionRotationYawPitchRoll(&rotY_D3D, D3DX_PI / 2, 0, 0);
			rotY_quat = PxQuat(rotY_D3D.x, rotY_D3D.y, rotY_D3D.z, rotY_D3D.w);
			D3DXMatrixRotationQuaternion(&rotY_mat, &rotY_D3D);
		}
	};
}

//////////////////////////////////////////////////////////////////////////

obj_Vehicle::obj_Vehicle()
: vd(0)
,netMover(this, 0.1f, (float)PKT_C2C_MoveSetCell_s::VEHICLE_CELL_RADIUS)
{
	m_bEnablePhysics = false;
	ObjTypeFlags |= OBJTYPE_Vehicle;
	spawner = NULL;
	m_isSerializable = false;
}

//////////////////////////////////////////////////////////////////////////

obj_Vehicle::~obj_Vehicle()
{
	g_pPhysicsWorld->m_VehicleManager->DeleteCar(vd);
	if( spawner )
	{
		spawner->clearVehicle();
		spawner = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::UpdatePositionFromPhysx()
{
	if (!vd)
		return;

	PxRigidDynamicFlags f = vd->vehicle->mActor->getRigidDynamicFlags();
	if ( !(f & PxRigidDynamicFlag::eKINEMATIC) )
	{

		PxTransform t = vd->vehicle->mActor->getGlobalPose();
		SetPosition(r3dVector(t.p.x, t.p.y, t.p.z));

		r3dVector angles;
		QuaternionToEulerAngles(t.q, angles.x, angles.y, angles.z);
		//	To degrees
		angles = D3DXToDegree(angles);

		SetRotationVector(angles);
	}
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Vehicle::Update()
{

#ifndef FINAL_BUILD

	if ( g_bEditMode )
	{
		if( CurHUDID == 0 && d_drive_vehicles->GetBool() == false ) {
			if ( spawner != NULL && g_Manipulator3d.IsSelected( this ) )
			{
				spawner->SetPosition( GetPosition() );
				spawner->SetRotationVector( GetRotationVector() );
			}
		}
	}
#endif 

	if(NetworkLocal)
	{
		CNetCellMover::moveData_s md;
		md.pos       = GetPosition();
		md.turnAngle = GetRotationVector().x;
		md.bendAngle = 0;
		md.state     = 0;
		netMover.SendPosUpdate(md);
	}
	else 
	{
		UpdatePositionFromRemote();
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Vehicle::OnCreate()
{
	if (!parent::OnCreate())
		return FALSE;

	r3dMesh *m = MeshLOD[0];
	if (!m)
		return FALSE;

	vd = g_pPhysicsWorld->m_VehicleManager->CreateVehicle(m);
	if (vd)
	{
		//	Set position and orientation for car
		SwitchToDrivable(d_drive_vehicles->GetBool());
		SyncPhysicsPoseWithObjectPose();
		r3dBoundBox bb = GetBBoxLocal();
		std::swap(bb.Size.x, bb.Size.z);
		std::swap(bb.Org.x, bb.Org.z);
		SetBBoxLocal(bb);
		vd->owner = this;
	}

	netMover.Teleport(GetPosition());

	return vd != 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_Vehicle::OnDestroy()
{
	if (vd)
		vd->owner = 0;
	return parent::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::SetPosition(const r3dPoint3D& pos)
{
	parent::SetPosition(pos);
	SyncPhysicsPoseWithObjectPose();
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::SetRotationVector(const r3dVector& Angles)
{
	parent::SetRotationVector(Angles);
	SyncPhysicsPoseWithObjectPose();
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::SwitchToDrivable(bool doDrive)
{
	if (vd && vd->vehicle->mActor)
	{
		vd->vehicle->mActor->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, !doDrive);
		if (doDrive)
			g_pPhysicsWorld->m_VehicleManager->DriveCar(vd);
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::SyncPhysicsPoseWithObjectPose()
{
	if (!vd)
		return;

	PxRigidDynamicFlags f = vd->vehicle->mActor->getRigidDynamicFlags();
	if (!(f & PxRigidDynamicFlag::eKINEMATIC))
		return;

	r3dPoint3D pos(GetPosition());
	D3DXMATRIX rotM(GetRotationMatrix());
	D3DXQUATERNION q;
	D3DXQuaternionRotationMatrix(&q, &rotM);
	PxQuat quat(q.x, q.y, q.z, q.w);

	PxTransform carPose(PxVec3(pos.x, pos.y, pos.z), quat);
	vd->vehicle->mActor->setGlobalPose(carPose);
}

//////////////////////////////////////////////////////////////////////////

void obj_Vehicle::SetBoneMatrices()
{
	if (!vd)
		return;

	PxRigidDynamic *a = vd->vehicle->mActor;
	D3DXMATRIX boneTransform;

	static const UsefulTransforms T;

	//	Init with identities
	for (int i = 0; i < vd->skl->NumBones; ++i)
	{
		r3dBone &b = vd->skl->Bones[i];
		b.CurrentTM = T.rotY_mat;
	}

	//	Retrieve vehicle wheels positions
	PxShape *shapes[VEHICLE_PARTS_COUNT] = {0};
	PxU32 sn = a->getShapes(shapes, VEHICLE_PARTS_COUNT);
	for (PxU32 i = 0; i < sn; ++i)
	{
		PxShape *s = shapes[i];
		PxU32 boneIndex = reinterpret_cast<PxU32>(s->userData);
		r3dBone &b = vd->skl->Bones[boneIndex];
		PxTransform pose = s->getLocalPose();

		PxVec3 bonePos = PxVec3(b.vRelPlacement.x, b.vRelPlacement.y, b.vRelPlacement.z);
		D3DXMATRIX toOrigin, fromOrigin, suspensionOffset;
		D3DXMatrixTranslation(&toOrigin, -bonePos.x, -bonePos.y, -bonePos.z);
		
		PxVec3 bonePosNew = T.rotY_quat.rotate(bonePos);
		D3DXMatrixTranslation(&fromOrigin, pose.p.x, pose.p.y, pose.p.z);

		pose.q = pose.q * T.rotY_quat;

		D3DXQUATERNION q(pose.q.x, pose.q.y, pose.q.z, pose.q.w);
		D3DXMatrixRotationQuaternion(&boneTransform, &q);

		D3DXMatrixMultiply(&boneTransform, &toOrigin, &boneTransform);
		D3DXMatrixMultiply(&boneTransform, &boneTransform, &fromOrigin);

		b.CurrentTM = boneTransform;
	}
	vd->skl->SetShaderConstants();
}

void obj_Vehicle::UpdatePositionFromRemote()
{
	r3d_assert(!NetworkLocal);

	const float fTimePassed = r3dGetFrameTime();

	r3dVector currentRotation = GetRotationVector();
	float rotX      = currentRotation.x;
	float turnAngle = netMover.NetData().turnAngle;

	if(fabs(rotX - turnAngle) > 0.01f) 
	{
		extern float getMinimumAngleDistance(float from, float to);
		float f1 = getMinimumAngleDistance(rotX, turnAngle);
		rotX += ((f1 < 0) ? -1 : 1) * fTimePassed * 360;
		float f2 = getMinimumAngleDistance(rotX, turnAngle);
		if((f1 > 0 && f2 <= 0) || (f1 < 0 && f2 >= 0))
			rotX = turnAngle;

		currentRotation.x = rotX;
		SetRotationVector( currentRotation );
	}


	if(netVelocity.LengthSq() > 0.0001f)
	{
		SetPosition(GetPosition() + netVelocity * fTimePassed);

		// check if we overmoved to target position
		r3dPoint3D v = netMover.GetNetPos() - GetPosition();
		float d = netVelocity.Dot(v);
		if(d < 0) {
			SetPosition(netMover.GetNetPos());
			netVelocity = r3dPoint3D(0, 0, 0);
		}
	}

}
//////////////////////////////////////////////////////////////////////////

extern PxF32 &gVehicleTireGroundFriction;

#ifndef FINAL_BUILD
//#define EXTENDED_VEHICLE_CONFIG
float obj_Vehicle::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float y = scry + parent::DrawPropertyEditor(scrx, scry, scrw, scrh, startClass, selected );

	if( !IsParentOrEqual( &ClassData, startClass ) || !vd)
		return y;

	y += 10.0f;
	y += imgui_Static(scrx, y, "Vehicle physics configuration");
#ifdef EXTENDED_VEHICLE_CONFIG
	y += imgui_Value_Slider(scrx, y, "Friction", &gVehicleTireGroundFriction, 0.5f, 8.0f, "%-02.2f");

	y += 5.0f;
	y += imgui_Static(scrx, y, "Chassis:");
	y += imgui_Value_Slider(scrx, y, "Mass", &vd->chassisData.mMass, 100.0f, 5000.0f, "%-02.2f");
	y += imgui_Value_Slider(scrx, y, "Ackerman accuracy", &vd->ackermannData.mAccuracy, 0.0f, 1.0f, "%-02.2f");
#endif

	y += 5.0f;
	y += imgui_Static(scrx, y, "Engine:");
	y += imgui_Value_Slider(scrx, y, "Peak torque", &vd->engineData.mPeakTorque, 100.0f, 5000.0f, "%-02.f");
	float x = vd->engineData.mMaxOmega / 0.104719755f;
	y += imgui_Value_Slider(scrx, y, "Max RPM", &x, 0.0f, 15000.0f, "%-02.0f");
	vd->engineData.mMaxOmega = x * 0.104719755f;

#ifdef EXTENDED_VEHICLE_CONFIG
	y += 5.0f;
	y += imgui_Static(scrx, y, "Gears:");
	y += imgui_Value_Slider(scrx, y, "Switch time", &vd->gearsData.mSwitchTime, 0.0f, 3.0f, "%-02.2f");

	y += 5.0f;
	y += imgui_Static(scrx, y, "Differential:");
	y += imgui_Value_Slider(scrx, y, "Front-rear split", &vd->diffData.mFrontRearSplit, 0.0f, 1.0f, "%-02.3f");
#endif
	y += 10.0f;
	y += imgui_Static(scrx, y, "Select wheel:");

	static int currentWheel = 2;
	if (imgui_Button(scrx, y, 80.0f, 30.0f, "Front-Left", currentWheel == 2))
		currentWheel = 2;

	if (imgui_Button(scrx + 80, y, 80.0f, 30.0f, "Front-Right", currentWheel == 3))
		currentWheel = 3;

	if (imgui_Button(scrx + 160, y, 80.0f, 30.0f, "Rear-Left", currentWheel == 0))
		currentWheel = 0;

	if (imgui_Button(scrx + 240, y, 80.0f, 30.0f, "Rear-Right", currentWheel == 1))
		currentWheel = 1;

	y += 30.0f;

	VehicleDescriptor::WheelData &wd = vd->wheelsData[currentWheel];

	y += 5.0f;
#ifdef EXTENDED_VEHICLE_CONFIG
	y += imgui_Value_Slider(scrx, y, "Mass", &wd.wheelData.mMass, 1.0f, 100.0f, "%-02.3f");
	y += imgui_Value_Slider(scrx, y, "Spring max compression", &wd.suspensionData.mMaxCompression, 0.0f, 2.0f, "%-02.3f");
	y += imgui_Value_Slider(scrx, y, "Spring max droop", &wd.suspensionData.mMaxDroop, 0.0f, 2.0f, "%-02.3f");
#endif
	y += imgui_Value_Slider(scrx, y, "Break torque", &wd.wheelData.mMaxBrakeTorque, 0.0f, 25000.0f, "%-02.0f");
	float f = R3D_RAD2DEG(wd.wheelData.mMaxSteer);
	y += imgui_Value_Slider(scrx, y, "Steer angle", &f, 0.0f, 90.0f, "%-02.2f");
	wd.wheelData.mMaxSteer = R3D_DEG2RAD(f);
	if (currentWheel >= 2)
	{
		vd->wheelsData[(currentWheel + 1) % 2 + 2].wheelData.mMaxSteer = wd.wheelData.mMaxSteer;
	}
	float z = R3D_RAD2DEG(wd.wheelData.mToeAngle);
	wd.wheelData.mToeAngle = R3D_DEG2RAD(z);
#ifdef EXTENDED_VEHICLE_CONFIG
	y += imgui_Value_Slider(scrx, y, "Spring strength", &wd.suspensionData.mSpringStrength, 10000.0f, 50000.0f, "%-05.0f");
#endif
	y += imgui_Value_Slider(scrx, y, "Spring damper", &wd.suspensionData.mSpringDamperRate, 500.0f, 9000.0f, "%-02.0f");

	y += 10.0f;
	if (imgui_Button(scrx, y, 360.0f, 22.0f, "Save Vehicle Data"))
	{
		vd->Save(0);
	}
	y += 22.0f;

	vd->ApplyDynamicChanges();
	return y - scry;
}
#endif

//////////////////////////////////////////////////////////////////////////

const bool obj_Vehicle::getExitSpace( r3dVector& outVector, int exitIndex )
{
	return vd->GetExitIndex( outVector,exitIndex);
}



BOOL obj_Vehicle::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
	r3d_assert(!(ObjFlags & OBJFLAG_JustCreated)); // make sure that object was actually created before processing net commands

	switch(EventID)
	{
	default: return FALSE;

	case PKT_C2C_MoveSetCell:
		{
			const PKT_C2C_MoveSetCell_s& n = *(PKT_C2C_MoveSetCell_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			OnNetPacket(n);
			break;
		}
	case PKT_C2C_MoveRel:
		{
 			const PKT_C2C_MoveRel_s& n = *(PKT_C2C_MoveRel_s*)packetData;
			r3d_assert(packetSize == sizeof(n));

			OnNetPacket(n);
			break;
		}

	case PKT_S2C_UAVSetState:
		{
// 			const PKT_S2C_UAVSetState_s& n = *(PKT_S2C_UAVSetState_s*)packetData;
// 			r3d_assert(packetSize == sizeof(n));
// 
// 			OnNetPacket(n);
			break;
		}

	}

	return TRUE;
}

void obj_Vehicle::OnNetPacket(const PKT_C2C_MoveSetCell_s& n)
{

	// HACK, we have no knowledge if this is network local on the server yet. 
	if ( !NetworkLocal) 
	{
		netMover.SetCell(n);
	}
}

void obj_Vehicle::OnNetPacket(const PKT_C2C_MoveRel_s& n)
{

	// HACK, we have no knowledge if this is network local on the server yet. 
	if ( !NetworkLocal) 
	{
		const CNetCellMover::moveData_s& md = netMover.DecodeMove(n);

		// calc velocity to reach position on time for next update
		r3dPoint3D vel = netMover.GetVelocityToNetTarget(
			GetPosition(),
			GPP->UAV_FLY_SPEED_V * 1.5f,
			1.0f);

		netVelocity = vel;
	}
}


#endif // VEHICLES_ENABLED