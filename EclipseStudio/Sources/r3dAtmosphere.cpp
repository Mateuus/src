#include "r3dPCH.h"
#include "r3d.h"

#include "r3dAtmosphere.h"
#include "GameLevel.h"

#include "Editors/LevelEditor.h"

#include "..\eternity/SF/script.h"
#include "TrueNature/SkyDome.h"

#include "GameObjects/ObjManag.h"
#include "ObjectsCode/effects/obj_ParticleSystem.h"
#include "ObjectsCode/Nature/wind.h"

#include "XMLHelpers.h"

void WriteTimeCurve( FILE* fout, r3dTimeGradient2& curve, const char * szName );
void ReadTimeCurveNew( Script_c &script, r3dTimeGradient2& curve );

extern r3dCamera gCam ;

#define PT_EMPTY_STR "<empty>"

int GetAtmoDownScale()
{
	return r_texture_quality->GetInt() == 1 ? 2 : 1 ;
}

static void atmoReloadTexture( r3dTexture*& tex )
{
	if( !tex )
		return;

	char FileName[ 1024 ];
	r3dscpy( FileName, tex->getFileLoc().FileName );
	FileName[ 1023 ] = 0;

	tex->Unload();
	tex->Load( FileName, D3DFMT_FROM_FILE, GetAtmoDownScale() );
}


void r3dAtmosphere :: Reset()
{
	if( RainParticleSystem )
	{
		GameWorld().DeleteObject( RainParticleSystem ) ;
		RainParticleSystem = 0 ;
	}
	RainParticleSystemName[ 0 ] = 0;

	StaticSkyTexName = "" ;
	bStaticSkyEnable = false ;
	bCustomStaticMeshEnable = false ;
	bStaticSkyPlanarMapping = false ;

	SunLightOn = 1 ;

    ParticleShadingCoef = 0.7f;

	__CurTime = 16;
	bVolumeFog = 0;

	BacklightIntensity = 0.1f;
	BacklightColor.Reset(255.0f);
	
	SunIntensity = 1.0f;
	SunColor.Reset(255.0f);
	SkyColor.Reset(55.0f);
	LambdaCol.Reset(255.0f);
	Fog_HeightFadeStart.Reset(100);
	Fog_Density.Reset(0);
	Fog_HeightFadeRange.Reset(1.0f);
	Fog_Distance.Reset(1.0f);
	Fog_MaxHeight = 1500.0f;
	Fog2_MaxHeight = 1024.0f ;

	Aerial_Density.Reset(0.3f);
	Aerial_Distance.Reset(0.9f);

	Aerial_MipBias = 0.f ;

	SkyFog_Start = 5.0f;
	SkyFog_End = 15.0f;

	HGg.Reset(0.9993f);
	InscatteringMultiplier.Reset(1.0f);
	BetaRayMultiplier.Reset(4.0f);
	BetaMieMultiplier.Reset(0.0006f);
	SunIntensityCoef.Reset(1.0f);
	Turbitity.Reset(1.0f);
	SkyDomeWindFactor = 1.0f;
	SkyDomeCloudsScale = 1.0f;
	SkyDomeCloudsDensity = 0.5f;
	CloudFadeOutStartDist = 4000.0f;
	CloudFadeOutEndDist = 7000.0f;

	SkySunsetBumpness = 11.47f;
	SkySunsetOffset = -0.0074f;
	SkySunsetPower = 3.17f;
	SkySunsetStartCoef = 0.3f;
	SkyCloudsFadeStart = 2000;
	SkyCloudsFadeEnd = 1000;

	SunElevationAngle = -5.8f;

	SSSDiffuse = 0.0f;
	SSSSpecular = 0.0f;
	SSSSpecPower = 100.0f;

	StaticTexGenScaleX = 0.00001f;
	StaticTexGenScaleY = 0.00001f;

	StaticTexGetOffsetX = 0.5f;
	StaticTexGetOffsetY = 0.5f;

}

template < bool Write >
int r3dAtmosphere :: SerializeXML( pugi::xml_node root )
{
	const bool W = Write ;

	if( W )
	{
		FogVersion = 2 ;
	}
	else
	{
		FogVersion = 0 ;
	}

	if( pugi::xml_node atmoNode = SerializeXMLNode<W>( root, "atmosphere" ) )
	{
		SerializeXMLVal<W>( "cur_time", atmoNode, &__CurTime );
	
		SerializeXMLVal<W>( "sun_backlightintensity"					, atmoNode, &BacklightIntensity				);
		SerializeXMLCurve3f<W>( "sun_backlightcolor_curve"				, atmoNode, &BacklightColor					);
	
		SerializeXMLCurve3f<W>( "sun_color_curve"				, atmoNode, &SunColor					);
		SerializeXMLCurve3f<W>( "sky_color_curve"				, atmoNode, &SkyColor					);
		SerializeXMLCurve3f<W>( "lambda_col_curve"				, atmoNode, &LambdaCol					);
		SerializeXMLCurve3f<W>( "hg_g_curve"					, atmoNode, &HGg						);
		SerializeXMLCurve3f<W>( "inscattering_multiplier_curve"	, atmoNode, &InscatteringMultiplier	);
		SerializeXMLCurve3f<W>( "beta_ray_multiplier_curve"		, atmoNode, &BetaRayMultiplier		);
		SerializeXMLCurve3f<W>( "beta_mie_multiplier_curve"		, atmoNode, &BetaMieMultiplier		);
		SerializeXMLCurve3f<W>( "sun_intensity_curve"			, atmoNode, &SunIntensityCoef			);
		SerializeXMLCurve3f<W>( "turbitity_curve"				, atmoNode, &Turbitity					);

		SerializeXMLVal<W>( "fog_version"						, atmoNode, &FogVersion			);

		SerializeXMLCurve3f<W>( "fog_color_curve"				, atmoNode, &Fog_Color					);
		SerializeXMLCurve3f<W>( "fog_new_range_curve"			, atmoNode, &Fog_Range					);
		SerializeXMLCurve3f<W>( "fog_distance_curve"			, atmoNode, &Fog_Distance				);
		SerializeXMLCurve3f<W>( "fog_new_density_curve"			, atmoNode, &Fog_Density				);
		SerializeXMLCurve3f<W>( "fog_height_curve"				, atmoNode, &Fog_Height					);
		SerializeXMLCurve3f<W>( "aerial_density_curve"			, atmoNode, &Aerial_Density				);
		SerializeXMLCurve3f<W>( "aerial_distance_curve"			, atmoNode, &Aerial_Distance			);

		SerializeXMLVal<W>( "skyfog_start"						, atmoNode, &SkyDomeWindFactor		);
		SerializeXMLVal<W>( "skyfog_end"						, atmoNode, &SkyDomeCloudsScale		);
		SerializeXMLVal<W>( "skydome_wind_factor"				, atmoNode, &SkyDomeWindFactor		);
		SerializeXMLVal<W>( "skydome_clouds_scale"				, atmoNode, &SkyDomeCloudsScale		);
		SerializeXMLVal<W>( "skydome_clouds_density"			, atmoNode, &SkyDomeCloudsDensity	);
		SerializeXMLVal<W>( "cloudfade_outstart_dist"			, atmoNode, &CloudFadeOutStartDist	);
		SerializeXMLVal<W>( "cloudfade_out_enddist"				, atmoNode, &CloudFadeOutEndDist	);
		SerializeXMLVal<W>( "sky_sunset_bumpness"				, atmoNode, &SkySunsetBumpness		);
		SerializeXMLVal<W>( "sky_sunset_offset"					, atmoNode, &SkySunsetOffset		);
		SerializeXMLVal<W>( "sky_sunset_power"					, atmoNode, &SkySunsetPower			);
		SerializeXMLVal<W>( "sky_sunset_startcoef"				, atmoNode, &SkySunsetStartCoef		);
		SerializeXMLVal<W>( "sky_clouds_fadestart"				, atmoNode, &SkyCloudsFadeStart		);
		SerializeXMLVal<W>( "sky_clouds_fadeend"				, atmoNode, &SkyCloudsFadeEnd		);
		SerializeXMLVal<W>( "sun_elevation_angle"				, atmoNode, &SunElevationAngle		);
		SerializeXMLVal<W>( "sss_diffuse"						, atmoNode, &SSSDiffuse				);
		SerializeXMLVal<W>( "sss_specular"						, atmoNode, &SSSSpecular				);
		SerializeXMLVal<W>( "sss_specpower"						, atmoNode, &SSSSpecPower				);
		SerializeXMLVal<W>( "have_new_simple_fog"				, atmoNode, &bVolumeFog			);
		SerializeXMLVal<W>( "fog_max_height"					, atmoNode, &Fog_MaxHeight				);
		SerializeXMLVal<W>( "aerial_mip_bias"					, atmoNode, &Aerial_MipBias				);
		SerializeXMLVal<W>( "sun_intensity"						, atmoNode, &SunIntensity				);
		SerializeXMLVal<W>( "static_sky_tex_name"				, atmoNode, &StaticSkyTexName			);
		SerializeXMLVal<W>( "static_sky_mesh_name"				, atmoNode, &StaticSkyMeshName			);
		SerializeXMLVal<W>( "static_sky_enable"					, atmoNode, &bStaticSkyEnable			);
		SerializeXMLVal<W>( "static_sky_custom_mesh_enable"		, atmoNode, &bCustomStaticMeshEnable	);
		SerializeXMLVal<W>( "static_sky_planar_mapping"			, atmoNode, &bStaticSkyPlanarMapping	);

		SerializeXMLVal<W>( "static_texgenscale_x"				, atmoNode, &StaticTexGenScaleX			);
		SerializeXMLVal<W>( "static_texgenscale_y"				, atmoNode, &StaticTexGenScaleY			);
		SerializeXMLVal<W>( "static_texgetoffset_x"				, atmoNode, &StaticTexGetOffsetX		);
		SerializeXMLVal<W>( "static_texgetoffset_y"				, atmoNode, &StaticTexGetOffsetY		);

		SerializeXMLVal<W>( "rain_particles"					, atmoNode, RainParticleSystemName		);
		SerializeXMLVal<W>( "sunlight"							, atmoNode, &SunLightOn					);

		SerializeXMLCmdVarF<W>("sky_intensity"					, atmoNode, r_sky_intensity				);

		SerializeXMLVal<W>("particle_shading_coef"				, atmoNode, &ParticleShadingCoef		);
		
		if( g_pWind )
		{
			r3dWind::Settings windSts = g_pWind->GetSettings() ;

			SerializeXMLVal<W>( "wind_dir_x",			atmoNode, &windSts.DirX ) ;
			SerializeXMLVal<W>( "wind_dir_z",			atmoNode, &windSts.DirZ ) ;
			SerializeXMLVal<W>( "wind_pattern_scale",	atmoNode, &windSts.WindPatternScale ) ;
			SerializeXMLVal<W>( "wind_pattern_speed",	atmoNode, &windSts.WindPatternSpeed ) ;

			if( !W )
			{
				g_pWind->SetSettings( windSts ) ;
			}
		}

		return 1 ;
	}
	else
	{
		return 0 ;
	}
}

int	r3dAtmosphere :: LoadFromXML( pugi::xml_node root )
{
	strcpy( RainParticleSystemName, PT_EMPTY_STR ) ;

	int res = SerializeXML<false>( root );

	if( FogVersion == 1 )
	{
		ConvertFog_V1() ;
	}

	if( bStaticSkyEnable )
	{
		EnableStaticSky();
	}
	else
	{
		DisableStaticSky();
	}

	SetRainParticle( RainParticleSystemName ) ;

	return res ;
}

int	r3dAtmosphere :: SaveToXML( pugi::xml_node root )
{
	return SerializeXML<true>( root );
}


void
r3dAtmosphere::EnableStaticSky()
{
	if( StaticSkyTex )
	{
		r3dRenderer->DeleteTexture( StaticSkyTex ) ;
	}

	StaticSkyTex = r3dRenderer->LoadTexture( StaticSkyTexName.c_str(), D3DFMT_UNKNOWN, false, GetAtmoDownScale() ) ;

	SAFE_DELETE( StaticSkyMesh )  ;

	if( bCustomStaticMeshEnable )
	{
		StaticSkyMesh = new r3dMesh( 0 );
		StaticSkyMesh->Load( StaticSkyMeshName.c_str(), true, true );
		if( !StaticSkyMesh->IsLoaded() )
		{
			SAFE_DELETE( StaticSkyMesh );
		}
		else
		{
			StaticSkyMesh->FillBuffers();
		}
	}

	bStaticSkyEnable = 1 ;
}

void
r3dAtmosphere::DisableStaticSky()
{
	if( StaticSkyTex )
	{
		r3dRenderer->DeleteTexture( StaticSkyTex ) ;
		StaticSkyTex = 0 ;
	}

	if( StaticSkyMesh )
	{
		SAFE_DELETE( StaticSkyMesh ) ;
	}

	extern r3dSkyDome *SkyDome;
	if( SkyDome )
	{
		StaticSkySettings sts ;
		SkyDome->SetStaticSkyParams( sts ) ;
	}

	bStaticSkyEnable = 0 ;
}

void
r3dAtmosphere::SetStaticSkyTexture( const r3dString& textureName )
{
	StaticSkyTexName = textureName ;

	if( bStaticSkyEnable )
	{
		// reloads stuff
		DisableStaticSky();
		EnableStaticSky();
	}
}


void
r3dAtmosphere::SetStaticSkyMesh( const r3dString& meshName )
{
	StaticSkyMeshName = meshName ;

	if( bStaticSkyEnable )
	{
		// reloads stuff
		DisableStaticSky();
		EnableStaticSky();
	}
}

void
r3dAtmosphere::ConvertFog_V0()
{
	r3dTimeGradient2 heightMax = Fog_HeightFadeStart ;
	r3dTimeGradient2 thicknessOfFadeZone = Fog_HeightFadeRange ;

	Fog_HeightFadeStart.Reset( 
					heightMax.GetFloatValue( 0 ) * thicknessOfFadeZone.GetFloatValue( 0 ) * 0.01f
						,
					heightMax.GetFloatValue( 1 ) * thicknessOfFadeZone.GetFloatValue( 1 ) * 0.01f
					) ;

	for( int i = 1, e = heightMax.NumValues - 1 ; i < e ; i ++ )
	{
		float t = heightMax.Values[ i ].time ;

		Fog_HeightFadeStart.AddValue( 
				t, 
				heightMax.Values[ i ].val[ 0 ] * thicknessOfFadeZone.GetFloatValue( t ) * 0.01f ) ;
	}

	Fog_HeightFadeRange.Reset(
			heightMax.GetFloatValue( 0 ) - Fog_HeightFadeStart.GetFloatValue( 0 ),
			heightMax.GetFloatValue( 1 ) - Fog_HeightFadeStart.GetFloatValue( 1 )
		) ;

	for( int i = 1, e = Fog_HeightFadeStart.NumValues - 1 ; i < e ; i ++ )
	{
		float t = Fog_HeightFadeStart.Values[ i ].time ;

		Fog_HeightFadeRange.AddValue( 
				t,
				heightMax.Values[ i ].val[ 0 ] - Fog_HeightFadeStart.Values[ i ].val[ 0 ]			
			) ;
	}
}

void
r3dAtmosphere::ConvertFog_V1()
{
	r3dTimeGradient2 heightStart	= Fog_Height ;
	r3dTimeGradient2 fogFadeRange	= Fog_Range ;

	Fog_Height.Reset(
					heightStart.GetFloatValue( 0 ) + fogFadeRange.GetFloatValue( 0 )
						,
					heightStart.GetFloatValue( 1 ) + fogFadeRange.GetFloatValue( 1 ) 
					) ;

	for( int i = 1, e = heightStart.NumValues - 1 ; i < e ; i ++ )
	{
		float t = heightStart.Values[ i ].time ;

		Fog_Height.AddValue( 
				t, 
				heightStart.Values[ i ].val[ 0 ] + fogFadeRange.GetFloatValue( t ) ) ;
	}

	Fog_Range.Reset(
			( 1.0f - heightStart.GetFloatValue( 0 ) / Fog_Height.GetFloatValue( 0 ) ) * 100.f,
			( 1.0f - heightStart.GetFloatValue( 1 ) / Fog_Height.GetFloatValue( 1 ) ) * 100.f 
		) ;

	for( int i = 1, e = heightStart.NumValues - 1 ; i < e ; i ++ )
	{
		float t = heightStart.Values[ i ].time ;

		Fog_Range.AddValue( 
				t,
				( 1.0f - heightStart.Values[ i ].val[ 0 ] / Fog_Height.Values[ i ].val[ 0 ]	) * 100.f
			) ;
	}
}

void r3dAtmosphere::ReloadTextures()
{
	atmoReloadTexture( StaticSkyTex  ) ;
}

void r3dAtmosphere::Update()
{
	if( RainParticleSystem )
	{
		RainParticleSystem->SetPosition( gCam ) ;
	}
}

void r3dAtmosphere::SetRainParticle( const char* Name ) 
{
	if( RainParticleSystem )
	{
		GameWorld().DeleteObject( RainParticleSystem ) ;
		RainParticleSystem = 0 ;
	}

	if( RainParticleSystemName != Name )
	{
		strcpy( RainParticleSystemName, Name ) ;
	}

	if( !stricmp( RainParticleSystemName, PT_EMPTY_STR ) || !strlen( RainParticleSystemName ) )
	{
		return ;
	}

	RainParticleSystem = srv_CreateGameObject( "obj_ParticleSystem", Name, gCam ) ;

	// have to load/save separately
	RainParticleSystem->bPersistent = false ;

	obj_ParticleSystem* gem = static_cast<obj_ParticleSystem*>( RainParticleSystem ) ;
	gem->bKeepAlive		= true ;
	gem->bKillDelayed	= false ;
	gem->bKill			= false ;
}

void r3dAtmosphere::ClearRainParticle()
{
	SetRainParticle( PT_EMPTY_STR ) ;
}