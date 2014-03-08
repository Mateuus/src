#ifndef __R3DSUN_H
#define __R3DSUN_H


#include "TimeGradient.h"

class r3dSun
{
 public: 

	r3dVector		SunDir;
	int				bLoaded;
	
	r3dLight		SunLight;
	
	float			DawnTime;
	float			DuskTime;
	float			Time;			


	r3dSun() { bLoaded = 0; }
	~r3dSun() { Unload(); }


	void	Init();
	void	Unload();
	
	void	SetLocation(float Angle1, float Angle2);
	void	SetTime(float Hour);

	void	DrawSun(const r3dCamera &Cam, int bReplicate = 1);
};

#endif // R3DSUN
