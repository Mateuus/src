//=========================================================================
//	Module: HUDFilters.cpp
//	Created by Roman E. Marchenko <roman.marchenko@gmail.com>
//	Copyright (C) 2011.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"
#include "HUDFilters.h"

//////////////////////////////////////////////////////////////////////////

HUDFilterSettings gHUDFilterSettings[HUDFilter_Total];

//////////////////////////////////////////////////////////////////////////

void InitHudFilters()
{
	HUDFilterSettings &hfsCameraDrone = gHUDFilterSettings[HUDFilter_CameraDrone];

	hfsCameraDrone.bloomBlurPasses = 3;
	hfsCameraDrone.filmGrain = 1;

	hfsCameraDrone.filmGrainSettings.GrainScale = 10.0f;
	hfsCameraDrone.filmGrainSettings.Strength = 0.5f;
	hfsCameraDrone.filmGrainSettings.FPS = 24.0f; ///////
	
	strcpy_s(hfsCameraDrone.colorCorrectionTextureName, _countof(hfsCameraDrone.colorCorrectionTextureName), "cameradrone.dds");
	hfsCameraDrone.directionalStreaksOnOffCoef = 1.0f;
	hfsCameraDrone.enableColorCorrection = true;

	gHUDFilterSettings[ HUDFilter_Default ].directionalStreaksOnOffCoef = 1.0f ;

	gHUDFilterSettings[HUDFilter_NightVision].enableColorCorrection = true;
	gHUDFilterSettings[HUDFilter_NightVision].filmGrain = 1 ;
}