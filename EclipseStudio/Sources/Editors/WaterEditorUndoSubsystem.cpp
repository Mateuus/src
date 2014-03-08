//=========================================================================
//	Module: WaterEditorUndoSubsystem.cpp
//	Copyright (C) 2011.
//=========================================================================

#include "r3dPCH.h"

#ifndef FINAL_BUILD

#include "r3d.h"
#include "WaterEditorUndoSubsystem.h"
#include "EditedValueTracker.h"

//-------------------------------------------------------------------------
//	Value tracker list
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//	Particle system values
//-------------------------------------------------------------------------

ValueTracker<r3dColor> ShallowColorT;
ValueTracker<r3dColor> DeepColorT;
ValueTracker<r3dColor> AttenColorT;
ValueTracker<float> ShallowDepthT;
ValueTracker<float> AttenDistT;
ValueTracker<float> RefractionIndexT;
ValueTracker<float> BumpnessT;
ValueTracker<float> TileSizeT;
ValueTracker<float> SpecularCosinePowerT;
ValueTracker<float> SpecularTilingT;
ValueTracker<float> CausticStrengthT;
ValueTracker<float> CausticDepthT;
ValueTracker<float> CausticTilingT;

ValueTracker<float> WaterColorTileT;
ValueTracker<float> WaterColorBlendT;
ValueTracker<float> RefractionPerturbationT;
ValueTracker<float> FoamTileT;
ValueTracker<float> FoamPerturbationT;
ValueTracker<float> FoamSpeedT;
ValueTracker<float> FoamIntensityT;
ValueTracker<float> SurfTileT;
ValueTracker<float> SurfPeriodT;
ValueTracker<float> SurfWidthT;
ValueTracker<float> SurfRangeT;
ValueTracker<float> SurfPerturbationT;
ValueTracker<float> SurfIntensityT;
ValueTracker<float> Surf2CoastT;

ValueTracker<float> URefractionIndexT;
ValueTracker<float> URefractionPerturbationT;
ValueTracker<float> UAttenDistT;
ValueTracker<r3dColor> UShallowColorT;
ValueTracker<r3dColor> UDeepColorT;
ValueTracker<r3dColor> UAttenColorT;

ValueTracker<float> PlaneOffsetXT;
ValueTracker<float> PlaneOffsetZT;

extern float g_fWaterPlaneBrushRadius;

ValueTracker<float> WaterPlaneBrushRadiusT;
ValueTracker<float> CellGridSizeT;
ValueTracker<bool> WaterPlaneFollowTerrainT;
ValueTracker<float> WaterPlaneHeightOnTerrainT;
ValueTracker<int> CoastSmoothLevelsT;
ValueTracker<float> WaterPlaneHeightT;

//////////////////////////////////////////////////////////////////////////

void UpdateWaterPlaneCallbackI(int &v, ValueTracker<int> &vt)
{
	obj_WaterPlane *o = reinterpret_cast<obj_WaterPlane*>(vt.tag);
	if (o)
	{
		o->UpdateWaterPlane ();
	}
}

//////////////////////////////////////////////////////////////////////////

void CellSizeChangeCallback(float &v, ValueTracker<float> &vt)
{
	obj_WaterPlane *o = reinterpret_cast<obj_WaterPlane*>(vt.tag);
	if (o)
	{
		vt.GetTrackedValue() -= 1.0f;
		o->SetCellGridSize(v);
	}
}

//////////////////////////////////////////////////////////////////////////

void UpdateWaterPlaneCallbackF(float &v, ValueTracker<float> &vt)
{
	obj_WaterPlane *o = reinterpret_cast<obj_WaterPlane*>(vt.tag);
	if (o)
	{
		o->UpdateWaterPlane ();
	}
}

//////////////////////////////////////////////////////////////////////////

void InitWaterValueTrackers(obj_WaterPlane *o)
{
	if (!o)
		return;
	uint32_t tag = reinterpret_cast<uint32_t>(o);

	ShallowColorT.Init(o->shallowColor);
	ShallowColorT.tag = tag;
	DeepColorT.Init(o->deepColor);
	DeepColorT.tag = tag;
	AttenColorT.Init(o->attenColor);
	AttenColorT.tag = tag;
	ShallowDepthT.Init(o->shallowDepth);
	ShallowDepthT.tag = tag;
	AttenDistT.Init(o->attenDist);
	AttenDistT.tag = tag;
	RefractionIndexT.Init(o->refractionIndex);
	RefractionIndexT.tag = tag;
	BumpnessT.Init(o->bumpness);
	BumpnessT.tag = tag;
	TileSizeT.Init(o->tileSize);
	TileSizeT.tag = tag;
	SpecularCosinePowerT.Init(o->specularCosinePower);
	SpecularCosinePowerT.tag = tag;
	SpecularTilingT.Init(o->specularTiling);
	SpecularTilingT.tag = tag;
	CausticStrengthT.Init(o->causticStrength);
	CausticStrengthT.tag = tag;
	CausticDepthT.Init(o->causticDepth);
	CausticDepthT.tag = tag;
	CausticTilingT.Init(o->causticTiling);
	CausticTilingT.tag = tag;

	WaterColorTileT.Init(o->waterColorTile);
	WaterColorTileT.tag = tag;
	WaterColorBlendT.Init(o->waterColorBlend);
	WaterColorBlendT.tag = tag;
	RefractionPerturbationT.Init(o->refractionPerturbation);
	RefractionPerturbationT.tag = tag;
	FoamTileT.Init(o->foamTile);
	FoamTileT.tag = tag;
	FoamPerturbationT.Init(o->foamPerturbation);
	FoamPerturbationT.tag = tag;
	FoamSpeedT.Init(o->foamSpeed);
	FoamSpeedT.tag = tag;
	FoamIntensityT.Init(o->foamIntensity);
	FoamIntensityT.tag = tag;
	SurfTileT.Init(o->surfTile);
	SurfTileT.tag = tag;
	SurfPeriodT.Init(o->surfPeriod);
	SurfPeriodT.tag = tag;
	SurfWidthT.Init(o->surfWidth);
	SurfWidthT.tag = tag;
	SurfRangeT.Init(o->surfRange);
	SurfRangeT.tag = tag;
	SurfPerturbationT.Init(o->surfPerturbation);
	SurfPerturbationT.tag = tag;
	SurfIntensityT.Init(o->surfIntensity);
	SurfIntensityT.tag = tag;
	Surf2CoastT.Init(o->surf2Coast);
	Surf2CoastT.tag = tag;

	URefractionIndexT.Init(o->uRefractionIndex);
	URefractionIndexT.tag = tag;
	URefractionPerturbationT.Init(o->uRefractionPerturbation);
	URefractionPerturbationT.tag = tag;
	UAttenDistT.Init(o->uAttenDist);
	UAttenDistT.tag = tag;
	UShallowColorT.Init(o->uAttenColor);
	UShallowColorT.tag = tag;
	UDeepColorT.Init(o->uDeepColor);
	UDeepColorT.tag = tag;
	UAttenColorT.Init(o->uAttenColor);
	UAttenColorT.tag = tag;

	PlaneOffsetXT.Init(o->PlaneOffsetX);
	PlaneOffsetXT.tag = tag;
	PlaneOffsetZT.Init(o->PlaneOffsetZ);
	PlaneOffsetZT.tag = tag;

	WaterPlaneBrushRadiusT.Init(g_fWaterPlaneBrushRadius);

	CellGridSizeT.Init(o->m_fCellGridSize);
	CellGridSizeT.tag = tag;
	CellGridSizeT.SetOnUndoCallback(&CellSizeChangeCallback);
	WaterPlaneFollowTerrainT.Init(o->m_bWaterPlaneFollowTerrain);
	WaterPlaneFollowTerrainT.tag = tag;
	WaterPlaneHeightOnTerrainT.Init(o->m_fWaterPlaneHeightOnTerrain);
	WaterPlaneHeightOnTerrainT.tag = tag;
	WaterPlaneHeightOnTerrainT.SetOnUndoCallback(&UpdateWaterPlaneCallbackF);
	CoastSmoothLevelsT.Init(o->m_iCoastSmoothLevels);
	CoastSmoothLevelsT.tag = tag;
	CoastSmoothLevelsT.SetOnUndoCallback(&UpdateWaterPlaneCallbackI);
	WaterPlaneHeightT.Init(o->m_fWaterPlaneHeight);
	WaterPlaneHeightT.tag = tag;
}

//////////////////////////////////////////////////////////////////////////

obj_WaterPlane::GridVecType gSavedGridCellsState;

void WaterCellsTrackUndoEnd(obj_WaterPlane *o)
{
	if (!o) return;
	if (gSavedGridCellsState != o->GetGrid())
	{
		WaterGridChangeUndo *undoItem = static_cast<WaterGridChangeUndo*>(g_pUndoHistory->CreateUndoItem(UA_WATER_GRID_CHANGE, 0));
		undoItem->SetWaterObject(o, gSavedGridCellsState);
	}
}

//////////////////////////////////////////////////////////////////////////

void WaterCellsTrackUndoStart(obj_WaterPlane *o)
{
	if (!o) return;
	gSavedGridCellsState = o->GetGrid();
}
//-------------------------------------------------------------------------
//	class WaterGridChangeUndo
//-------------------------------------------------------------------------

void WaterGridChangeUndo::Undo()
{
	if (obj)
	{
		obj_WaterPlane::GridVecType tmp = obj->GetGrid();
		obj->SetGrid(savedVal);
		savedVal = tmp;
	}
}

//////////////////////////////////////////////////////////////////////////

void WaterGridChangeUndo::Redo()
{
	Undo();
}

//////////////////////////////////////////////////////////////////////////
#endif // FINAL_BUILD