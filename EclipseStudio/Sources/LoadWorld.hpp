#include "ObjectsCode\World\Lamp.h"

r3dColor BloomColor = r3dColor(155, 125, 105);
r3dColor BloomColor1 = r3dColor(255,255,255);
r3dColor BloomColor2 = r3dColor(0,0,0);
r3dColor TintColor = r3dColor(0,0,0,0);
float 	RenderDistance = 123000;






void LoadWorldLights(const char* Name);



static void LoadWorld(const char* Name, int maxPlayers )
{
 char 	TempStr1[128];

 SetLoadingProgress( 0.f );

 //**********************************************************
 //*
 //* Load sound and material properties mapper for current level
 //*
 //**********************************************************
 sprintf(TempStr1,"%s\\soundmap.ini", Name);
 //pwSoundScheme.Load(TempStr1);

 sprintf(TempStr1,"%s\\Sounds", Name);
// snd_LoadSoundEffects(TempStr1, "Sound.Dat");

 //**********************************************************
 //*
 //* Initialize environment ( fog, effects, sun )
 //*
 //**********************************************************
 Sun = NULL;

 sprintf(TempStr1,"%s\\sun.ini", Name);
 Sun = new r3dSun;
 Sun->Init();

 if (Sun->bLoaded)
 {
  SunVector = Sun->SunLight.Direction;
  SunVector.Normalize();

  Sun->SunLight.bCastShadows = 1;

//  Sun->SunLight.LightFunction = R3D_LIGHTFUNC_THUNDER;
  Sun->SunLight.LightFunctionParams[1] = 5;
  Sun->SunLight.LightFunctionParams[2] = 3;
  Sun->SunLight.LightFunctionParams[6] = 0.5f;

  WorldLightSystem.Add(&Sun->SunLight);
 }

 //**********************************************************
 //*
 //* Initialize dynamic lights
 //*
 //**********************************************************
 // LoadWorldLights(Name);

 SetLoadingPhase( "Loading" );
 SetLoadingProgress( PROGRESS_LOAD_LEVEL_START * 0.33F );

 SkyDome = new r3dSkyDome( r3dIntegrityGuardian() );
 SkyDome->Load(Name);

 SetLoadingProgress( PROGRESS_LOAD_LEVEL_START * 0.66F );

 LightPreset::LoadFromScript( FNAME_LIGHT_PRESETS );

 SetLoadingProgress( PROGRESS_LOAD_LEVEL_START );


 //**********************************************************
 //*
 //* Load level geometry data - gameworld, terrain, sky, etc
 //*
 //**********************************************************

 extern  int LoadLevel();

 LoadLevel();

 // volume fog params for skydome
 r3dColor	fogColor  = r3dGameLevel::Environment.Fog_Color.GetColorValue(r3dGameLevel::Environment.__CurTime/24.0f);
 float		fogHeight = r3dGameLevel::Environment.Fog_HeightFadeStart.GetFloatValue(r3dGameLevel::Environment.__CurTime/24.0f);	
 float		fWindFactor = r3dGameLevel::Environment.SkyDomeWindFactor;

 SkyDome->SetParams(fogHeight, 0.0f, fogColor,fWindFactor);

	GameObject* obj = NULL;

	r3dPoint3D pos = r3dPoint3D(0,0,0);

  obj = srv_CreateGameObject("obj_Tree", "Tree1", pos);

  SetLoadingProgress( PROGRESS_LOAD_LEVEL_END );

  InitializePhysSkeletonCache( maxPlayers, PROGRESS_LOAD_LEVEL_END, 1.0f ) ;

 return;  
}

