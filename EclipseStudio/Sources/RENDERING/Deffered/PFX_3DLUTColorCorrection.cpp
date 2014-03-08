#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "../../EclipseStudio/Sources/ObjectsCode/Nature/obj_LocalColorCorrection.h"
#include "../../EclipseStudio/Sources/ObjectsCode/ai/AI_Player.H"

#include "PostFXChief.h"

#include "PFX_3DLUTColorCorrection.h"
#include "HUDFilters.h"


//------------------------------------------------------------------------

PFX_3DLUTColorCorrection::PFX_3DLUTColorCorrection()
: Parent( this )
{

}

//------------------------------------------------------------------------

PFX_3DLUTColorCorrection::~PFX_3DLUTColorCorrection()
{

}

//------------------------------------------------------------------------
/*virtual*/

void
PFX_3DLUTColorCorrection::InitImpl() /*OVERRIDE*/
{
	m3DLUTPSID			= r3dRenderer->GetPixelShaderIdx( "PS_CC_LUT3D"				);
	m3DLUTWithLocalPSID = r3dRenderer->GetPixelShaderIdx( "PS_CC_LUT3D_WITH_LOCAL"	);

	mData.PixelShaderID = m3DLUTPSID;
}

//------------------------------------------------------------------------
/*virtual*/

void
PFX_3DLUTColorCorrection::CloseImpl() /*OVERRIDE*/
{

}

//------------------------------------------------------------------------
/*virtual*/

void
PFX_3DLUTColorCorrection::PrepareImpl( r3dScreenBuffer* /*dest*/, r3dScreenBuffer* /*src*/ ) /*OVERRIDE*/
{
	void ReloadCCLUT3DTexture( const char*, HUDFilters );

	HUDFilterSettings &hfs = gHUDFilterSettings[r_hud_filter_mode->GetInt()];
	r3dTexture* useThisTex = hfs.colorCorrectionTex ;

	if( hfs.colorCorrectionTex == NULL )
	{
		ReloadCCLUT3DTexture( "default.dds", static_cast<HUDFilters>(r_hud_filter_mode->GetInt()) );
	}
	useThisTex = hfs.colorCorrectionTex ;

	r3dTexture* pLocalCCTex = NULL;
	float fFadeCoef = 0;
	// PT: we are not using local color correction right now, so, if player needs to render damage hit color correction, use it instead of local color correction
	// if later we will start using local color corrections - add new filter for player color correction
	if(obj_AI_Player::s_damageHitColorCorrectionVal > 0.001f)
	{
		pLocalCCTex = obj_AI_Player::s_damageHitColorCorrectionTex;
		fFadeCoef = obj_AI_Player::s_damageHitColorCorrectionVal;
	}
	else
	{
		// find nearest local color correction
		obj_LocalColorCorrection* pNearestLocalCC = obj_LocalColorCorrection::FindNearestColorCorrection ();
		fFadeCoef = pNearestLocalCC ? R3D_CLAMP( ( 1.0f - ( ( gCam - pNearestLocalCC->GetPosition() ).Length () - pNearestLocalCC->RadiusMin() ) / (pNearestLocalCC->RadiusMax() - pNearestLocalCC->RadiusMin()) ), 0.0f, 1.0f ) : 0.0f;

		// reset local cc if not appropriate
		if ( fFadeCoef <= 0.0f ) 
			pNearestLocalCC = NULL;
		if ( pNearestLocalCC && !pNearestLocalCC->CCTexture () )
			pNearestLocalCC = NULL;

		if(pNearestLocalCC!=NULL)
			pLocalCCTex = pNearestLocalCC->CCTexture();
	}

	r3dRenderer->SetTex( useThisTex, PostFXChief::FREE_TEX_STAGE_START );

	if ( pLocalCCTex )
	{
		mData.PixelShaderID = m3DLUTWithLocalPSID;
		r3dRenderer->SetTex( pLocalCCTex, PostFXChief::FREE_TEX_STAGE_START + 1 );
		r3dRenderer->pd3ddev->SetPixelShaderConstantF ( 0, (float*)&D3DXVECTOR4 ( fFadeCoef, 0.0f, 0.0f, 0.0f ), 1 );
	}
	else
		mData.PixelShaderID = m3DLUTPSID;
}

//------------------------------------------------------------------------
/*virtual*/

void
PFX_3DLUTColorCorrection::FinishImpl()	/*OVERRIDE*/
{

}

