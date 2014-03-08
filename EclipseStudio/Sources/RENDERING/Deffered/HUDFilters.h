//=========================================================================
//	Module: HUDFilters.h
//	Created by Roman E. Marchenko <roman.marchenko@gmail.com>
//	Copyright (C) 2011.
//=========================================================================

#pragma once
#include "PFX_ExtractBloom.h"
#include "PFX_FilmGrain.h"

#include "CommonPostFX.h"

//////////////////////////////////////////////////////////////////////////

enum HUDFilters
{
	HUDFilter_Default = 0,
	HUDFilter_NightVision,
	HUDFilter_CameraDrone,
	HUDFilter_UAV,
	HUDFilter_C130,
	HUDFilter_Laser,
	HUDFilter_Total
};

//////////////////////////////////////////////////////////////////////////

struct HUDFilterSettings
{
	PFX_ExtractBloom::Settings bloomSettings;
	PFX_FilmGrain::Settings filmGrainSettings;
	int bloomBlurPasses;
	BlurTaps bloomBlurTaps;
	float directionalStreaksOnOffCoef;
	int filmGrain;
	r3dTexture *colorCorrectionTex;
	char colorCorrectionTextureName[256];
	int enableColorCorrection;

	int overrideHDRControls ;
	float hdrExposureBias ;

	HUDFilterSettings()
	: bloomBlurPasses(0)
	, bloomBlurTaps(BT_17)
	, directionalStreaksOnOffCoef(0)
	, filmGrain(0)
	, colorCorrectionTex(0)
	, enableColorCorrection(0)
	, overrideHDRControls(0)
	, hdrExposureBias(0.f)
	{
		strcpy_s(colorCorrectionTextureName, _countof(colorCorrectionTextureName), "default.dds");
	}
};

//////////////////////////////////////////////////////////////////////////

extern HUDFilterSettings gHUDFilterSettings[HUDFilter_Total];

//////////////////////////////////////////////////////////////////////////

void InitHudFilters();