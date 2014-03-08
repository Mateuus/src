#pragma once

#include "r3dAtmosphere.h"

extern_nspace(r3dGameLevel,	float		StartLevelTime);

extern_nspace(r3dGameLevel,	r3dAtmosphere	Environment);

extern_nspace(r3dGameLevel,	const char*	GetSaveDir());
extern_nspace(r3dGameLevel,	const char*	GetHomeDir());
extern_nspace(r3dGameLevel,	void		SetSaveDir(const char* dir));
extern_nspace(r3dGameLevel,	void		SetHomeDir(const char* MapName));
extern_nspace(r3dGameLevel,	void		SetStartGameTime(float Time));


extern	int	LoadLevel_Objects( float BarRange );
extern  int LoadLevel_Groups ();
extern	int	LoadLevel_MatLibs();
