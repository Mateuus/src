#pragma once

class obj_EnvmapProbe;

class EnvmapProbes
{
	// types
public:
	typedef r3dTL::TArray< obj_EnvmapProbe* > ProbeArr;
	static const int WRONG_IDX = -1;

	// construction/ destruction
public:
	EnvmapProbes();
	~EnvmapProbes();

	// manipulation/ access
public:
	r3dTexture*	GetClosestTexture( const r3dPoint3D& pos ) const;
	void		Register( obj_EnvmapProbe* probe );
	void		Unregister( obj_EnvmapProbe* probe );

	int					GetCount() const;
	obj_EnvmapProbe*	GetProbe( int idx ) const;

	void		DeselectAll();
	void		GenerateAll();
	void		Generate( int id ) ;

	void		Init();
	void		Close();

	void		SetForceGlobal( bool forceGlobal );

	// helpers
private:
	int			GetIdxOf( obj_EnvmapProbe* probe );

	// data
private:
	 ProbeArr		mProbes;
	 r3dTexture*	mGlobalEnvmap;

	 bool			mForceGlobal;
} extern g_EnvmapProbes;