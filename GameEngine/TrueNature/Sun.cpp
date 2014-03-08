#include "r3dPCH.h"
#include "r3d.h"

#include "sun.h"
#include "GameCommon.h"
#include "GameLevel.h"

#include "../SF/Console/Config.h"


void r3dSun :: Init()
{
 if (bLoaded) Unload();

 bLoaded = 1;

 SetLocation(0, 45);

 DawnTime = 4;
 DuskTime = 21;

 SunLight.SetType(R3D_DIRECT_LIGHT);
 //SunLight.SetColor(SunColor);
 SunLight.bAffectBump = 1;
 SunLight.bCastShadows = 0;
}




void r3dSun :: Unload()
{
 if ( !bLoaded ) return;

 bLoaded = 0;
}



void r3dSun :: SetLocation(float Angle1, float Angle2)
{
 if (!bLoaded) return;

 r3dVector V = r3dVector(0,0,1);

 V.RotateAroundX(-Angle2);
 V.RotateAroundY(Angle1);
 V.Normalize();
 SunDir = V;

 //SunLight.SetColor(SunColor);
 SunLight.Direction = -SunDir;
}


void r3dSun :: SetTime(float Hour)
{
 if (!bLoaded) return;

 Time = Hour;

 if (Time < 0 ) Time = 0;
 if (Time > 24 ) Time = 0;

 DawnTime = 0;
 DuskTime = 24;
 float ValD = (Time-DawnTime) / (24.0f-(24.0f-DuskTime-DawnTime));
 if (ValD <0 ) ValD = 0;
 if (ValD >1 ) ValD = 1;

 //V.RotateAroundX(-Angle2);
 //V.RotateAroundY(Angle1);
 //V.Normalize();

 float Angle = ValD*180.0f;

 r3dVector SunVec = r3dVector(1.0f, 0, 0);
 SunVec.RotateAroundY(r3dGameLevel::Environment.SunElevationAngle);

 SunVec.RotateAroundZ(Angle);
 SunVec.Normalize();


// 	if ( d_sun_rotate->GetBool() )
// 	{
// 		float fPhase = timeGetTime() * 0.001f;
// 		SunVec.x = cosf( fPhase );
// 		SunVec.y = 0.5f;
// 		SunVec.z = sinf( fPhase );
// 		SunVec.Normalize();
// 	}


 float Mult = -1.0f;

 SunDir = SunVec*Mult;
 SunLight.Direction = SunVec*Mult;

 r3dColor sunColor = r3dGameLevel::Environment.SunColor.GetColorValue(ValD);
 SunLight.SetColor(sunColor);
// r3dRenderer->AmbientColor = AmbientColorG.GetColorValue(ValD);

  #if 1
  // FOR TEST: setup external particle system sun color/direction
  extern r3dColor  gPartShadeColor;
  extern r3dVector gPartShadeDir;
  gPartShadeDir = SunLight.Direction;
  gPartShadeDir.Y = 0; gPartShadeDir.Normalize();	// in 2D

  gPartShadeColor = sunColor;
  //gPartShadeColor = r3dColor(255, 255, 0);
  #endif
}




void r3dSun :: DrawSun(const r3dCamera &Cam, int bReplicate)
{
}
