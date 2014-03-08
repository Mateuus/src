#pragma once

#include "TimeGradient.h"

class r3dAtmosphere
{
  public:
	float __CurTime;

	r3dTexture*			StaticSkyTex ;
	r3dMesh*			StaticSkyMesh ;

	r3dString			StaticSkyTexName ;
	r3dString			StaticSkyMeshName ;

	float				ParticleShadingCoef ;

	float				StaticTexGenScaleX ;
	float				StaticTexGenScaleY ;

	float				StaticTexGetOffsetX ;
	float				StaticTexGetOffsetY ;

	int					bStaticSkyEnable ;
	int					bCustomStaticMeshEnable ;
	int					bStaticSkyPlanarMapping ;

	int					SunLightOn ;

	r3dTimeGradient2	SunColor;
	r3dTimeGradient2	SkyColor;
	float				SunIntensity;
	r3dTimeGradient2	LambdaCol;

	r3dTimeGradient2	BacklightColor;
	float				BacklightIntensity;

	int					bVolumeFog;
	int					FogVersion;
	float				Fog_MaxHeight;
	r3dTimeGradient2 	Fog_Color;
	r3dTimeGradient2 	Fog_Distance;
	r3dTimeGradient2 	Fog_Height;
	r3dTimeGradient2 	Fog_Range;
	r3dTimeGradient2 	Fog_HeightFadeStart;
	r3dTimeGradient2 	Fog_HeightFadeRange;
	r3dTimeGradient2 	Fog_Density;

	float				Fog2_MaxHeight;

	r3dTimeGradient2 	Fog2_Distance;
	r3dTimeGradient2 	Fog2_HeightFadeStart;
	r3dTimeGradient2 	Fog2_HeightFadeRange;
	r3dTimeGradient2 	Fog2_Density;

	r3dTimeGradient2 	Aerial_Density;
	r3dTimeGradient2 	Aerial_Distance;

	float				Aerial_MipBias;

	float				SkyFog_Start;
	float				SkyFog_End;

	r3dTimeGradient2 	HGg;
	r3dTimeGradient2 	InscatteringMultiplier;
	r3dTimeGradient2 	BetaRayMultiplier;
	r3dTimeGradient2 	BetaMieMultiplier;
	r3dTimeGradient2 	SunIntensityCoef;
	r3dTimeGradient2 	Turbitity;
	float				SkyDomeWindFactor;
	float				SkyDomeCloudsScale;
	float				SkyDomeCloudsDensity;

	float				SkySunsetBumpness;
	float				SkySunsetOffset;
	float				SkySunsetPower;
	float				SkySunsetStartCoef;
	float				SkyCloudsFadeStart;
	float				SkyCloudsFadeEnd;

	float				CloudFadeOutStartDist;
	float				CloudFadeOutEndDist;
	float				SunElevationAngle;

	float				SSSDiffuse ;
	float				SSSSpecular ;
	float				SSSSpecPower;

	class GameObject*	RainParticleSystem ;
	char				RainParticleSystemName[ 64 ] ;

  public:
	r3dAtmosphere()
	: StaticSkyTex( 0 )
	, StaticSkyMesh( 0 )
	, RainParticleSystem( 0 )
	, FogVersion( 0 )
	{
		Reset();
	}


#ifdef WO_SERVER
	void Reset() {};
#else
	void Reset();
#endif


	~r3dAtmosphere() {};

  template < bool Write >
  int	SerializeXML( pugi::xml_node root );

  int	LoadFromXML( pugi::xml_node root );
  int	SaveToXML( pugi::xml_node root );

  void  ReloadTextures();
  void	Update();

  void	SetRainParticle( const char* Name ) ;
  void	ClearRainParticle();

  void	EnableStaticSky();
  void	DisableStaticSky();
  void	SetStaticSkyTexture( const r3dString& textureName ) ;
  void	SetStaticSkyMesh( const r3dString& meshName ) ;	
  
  void	ConvertFog_V0() ;
  void	ConvertFog_V1() ;
};

