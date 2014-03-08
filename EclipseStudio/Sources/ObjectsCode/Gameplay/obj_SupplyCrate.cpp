#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "obj_SupplyCrate.h"
#include "gameobjects/obj_Mesh.h"

#include "multiplayer/ClientGameLogic.h"
#include "..\..\ui\HUDMinimap.h"
extern HUDMinimap*	hudMinimap;

IMPLEMENT_CLASS(obj_SupplyCrate, "obj_SupplyCrate", "Object");
AUTOREGISTER_CLASS(obj_SupplyCrate);

IMPLEMENT_CLASS(obj_BlackopsCrate, "obj_BlackopsCrate", "Object");
AUTOREGISTER_CLASS(obj_BlackopsCrate);

extern bool g_bEditMode;

SupplyCrateManager g_SupplyCrateMngr;

obj_SupplyCrate::obj_SupplyCrate()
{
	m_SupplyMeshPath = "Data\\ObjectsDepot\\Desert_Props\\Resupply_Crate_Group_01.sco";
	needToAddToMinimap = 0;
	SupplyType = 0;
}

obj_SupplyCrate::~obj_SupplyCrate()
{
}

BOOL obj_SupplyCrate::Load(const char *fname)
{
	if(!parent::Load(m_SupplyMeshPath)) 
		return FALSE;

	if(!g_bEditMode)
		ObjFlags |= OBJFLAG_SkipCastRay;

	return TRUE;
}

BOOL obj_SupplyCrate::OnCreate()
{
	parent::OnCreate();
	if(!g_bEditMode)
		needToAddToMinimap = 30;
	sprintf(SupplyCrateName, "SupplyCrate_%d", GetHashID());

	g_SupplyCrateMngr.RegisterSupplyCrate(this);

	return 1;
}


BOOL obj_SupplyCrate::OnDestroy()
{
	return parent::OnDestroy();
}

BOOL obj_SupplyCrate::Update()
{
	if(needToAddToMinimap && hudMinimap && hudMinimap->IsInited() && gClientLogic().localPlayer_)
	{
		--needToAddToMinimap;
		if(needToAddToMinimap == 0) // need to skip frames to make sure that map is loaded, otherwise it will not show up in minimap. weird flash
			hudMinimap->AddSupplyCrate(SupplyCrateName, SupplyType==0?"regular":"blackops", GetPosition());
	}
	return parent::Update();
}

// BLACK OPS CRATE
obj_BlackopsCrate::obj_BlackopsCrate()
{
	m_SupplyMeshPath = "Data\\ObjectsDepot\\Desert_Props\\Resupply_Drop_Crate_01.sco";
	SupplyType = 1;
}
