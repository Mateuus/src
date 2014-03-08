//=========================================================================
//	Module: DecalProxyObject.cpp
//	Copyright (C) 2011.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"
#ifndef FINAL_BUILD

#include "DecalProxyObject.h"
#include "..\ObjectsCode\world\DecalChief.h"
#include "ObjectManipulator3d.h"

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(DecalGameObjectProxy, "DecalGameObjectProxy", "Object");
AUTOREGISTER_CLASS(DecalGameObjectProxy);

//-------------------------------------------------------------------------
//	class DecalGameObjectProxy
//-------------------------------------------------------------------------

DecalGameObjectProxy::DecalGameObjectProxy()
: selectedDecalIdx(INVALID_DECAL_ID)
, decalType(INVALID_DECAL_ID)
{
	ObjTypeFlags |= OBJTYPE_DecalProxy;
	m_isSerializable = false; // do not allow this object to save itself
}

//////////////////////////////////////////////////////////////////////////

BOOL DecalGameObjectProxy::OnPositionChanged()
{
	GameObject::OnPositionChanged();

	if (decalType == INVALID_DECAL_ID)
		return true;

	const r3dVector& pos = GetPosition();
	DecalParams p = *g_pDecalChief->GetStaticDecal(decalType, selectedDecalIdx);
	p.Pos = pos;
	g_pDecalChief->UpdateStaticDecal(p, selectedDecalIdx);

	return true;
}

//////////////////////////////////////////////////////////////////////////

BOOL DecalGameObjectProxy::OnOrientationChanged()
{
	GameObject::OnOrientationChanged();

	if (decalType == INVALID_DECAL_ID)
		return true;

	const r3dVector& angles = GetRotationVector();
	DecalParams p = *g_pDecalChief->GetStaticDecal(decalType, selectedDecalIdx);
	p.ZRot = R3D_DEG2RAD(angles.x);
	g_pDecalChief->UpdateStaticDecal(p, selectedDecalIdx);
	return true;
}

//////////////////////////////////////////////////////////////////////////

void DecalGameObjectProxy::SelectDecal(UINT typeIdx, UINT idx)
{
	selectedDecalIdx = idx;
	decalType = typeIdx;

	const DecalParams *p = g_pDecalChief->GetStaticDecal(decalType, selectedDecalIdx);

	SetPosition(r3dPoint3D(p->Pos.x, p->Pos.y, p->Pos.z));
	
	if (g_Manipulator3d.PickedObjectCount() == 0 || g_Manipulator3d.PickedObjectGet(0) != this)
	{
		r3dVector angles( R3D_RAD2DEG( p->ZRot ), 0.f, 0.f ) ;
		SetRotationVector( angles ) ;

		g_Manipulator3d.ScaleEnable();
		g_Manipulator3d.Enable();
		g_Manipulator3d.PickerResetPicked();
		g_Manipulator3d.PickerAddToPicked(this);
	}
}

//////////////////////////////////////////////////////////////////////////

void DecalGameObjectProxy::SetPosition(const r3dPoint3D& pos)
{
	GameObject::SetPosition(pos);
	OnPositionChanged();
}

//////////////////////////////////////////////////////////////////////////

void DecalGameObjectProxy::SetRotationVector(const r3dVector& Angles)
{
	GameObject::SetRotationVector(Angles);
	OnOrientationChanged();
}

//////////////////////////////////////////////////////////////////////////

#endif // FINAL BUILD