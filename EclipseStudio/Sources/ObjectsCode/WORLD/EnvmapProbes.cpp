#include "r3dPCH.h"
#include "r3d.h"

#include "obj_EnvmapProbe.h"

#include "EnvmapProbes.h"

static r3dTexture* EnvmapProbes_GetClosestTexture(const r3dPoint3D& pos)
{
	return g_EnvmapProbes.GetClosestTexture(pos);
}

EnvmapProbes::EnvmapProbes():
mForceGlobal( false )
{
	// set callback in evenity
	extern r3dTexture* (*r3dMat_EnvmapProbes_GetClosestTexture)(const r3dPoint3D& pos);
	r3dMat_EnvmapProbes_GetClosestTexture = &EnvmapProbes_GetClosestTexture;
}

//------------------------------------------------------------------------

EnvmapProbes::~EnvmapProbes()
{

}

//------------------------------------------------------------------------

r3dTexture*
EnvmapProbes::GetClosestTexture( const r3dPoint3D& pos ) const
{
	float minDist	= FLT_MAX;
	int minIdx		= WRONG_IDX;

	if( !mForceGlobal )
	{
		for( uint32_t i = 0, e = mProbes.Count(); i < e; i ++ )
		{
			obj_EnvmapProbe* probe = mProbes[ i ];
			float newDist = (pos - probe->GetPosition()).LengthSq();

			if( newDist < minDist && ( probe->GetRadiusSq() > newDist || probe->IsInfinite() ) )
			{
				minDist	= newDist;
				minIdx	= i;
			}
		}
	}

	if( minIdx == WRONG_IDX )
	{
		return mGlobalEnvmap;
	}
	else
	{
		r3dTexture* tex = mProbes[ minIdx ]->GetTexture();
		return tex ? tex : mGlobalEnvmap;
	}
}

//------------------------------------------------------------------------

void
EnvmapProbes::Register( obj_EnvmapProbe* probe )
{
	r3d_assert( GetIdxOf( probe ) == WRONG_IDX );

	mProbes.PushBack( probe );
}

//------------------------------------------------------------------------

void
EnvmapProbes::Unregister( obj_EnvmapProbe* probe )
{
	int idx = GetIdxOf( probe );

	r3d_assert( idx != WRONG_IDX );

	mProbes.Erase( idx );
}

//------------------------------------------------------------------------

int
EnvmapProbes::GetCount() const
{
	return (int)mProbes.Count();
}

//------------------------------------------------------------------------

obj_EnvmapProbe*
EnvmapProbes::GetProbe( int idx ) const
{
	return mProbes[ idx ];
}

//------------------------------------------------------------------------

void
EnvmapProbes::DeselectAll()
{
	for( uint32_t i = 0, e = mProbes.Count(); i < e; i ++ )
	{
		mProbes[ i ]->SetSelected( false );
	}
}

//------------------------------------------------------------------------

void
EnvmapProbes::GenerateAll()
{
	obj_EnvmapProbe::CreateFolder( false );

	for( uint32_t i = 0, e = mProbes.Count(); i < e; i ++ )
	{
		mProbes[ i ]->Generate();
	}
}

//------------------------------------------------------------------------

void
EnvmapProbes::Generate( int id )
{
	for( uint32_t i = 0, e = mProbes.Count(); i < e; i ++ )
	{
		if( id - 1 == mProbes[ i ]->GetID() )
		{
			mProbes[ i ]->Generate();
		}
	}
}

//------------------------------------------------------------------------

void
EnvmapProbes::Init()
{
	mGlobalEnvmap = r3dRenderer->LoadTexture( "Data\\Shaders\\Texture\\Cubemap.dds" );
}

//------------------------------------------------------------------------

void
EnvmapProbes::Close()
{
	r3dRenderer->DeleteTexture( mGlobalEnvmap, 1 );
}

//------------------------------------------------------------------------

void
EnvmapProbes::SetForceGlobal( bool forceGlobal )
{
	mForceGlobal = forceGlobal;
}

//------------------------------------------------------------------------

int
EnvmapProbes::GetIdxOf( obj_EnvmapProbe* probe )
{
	for( uint32_t i = 0, e = mProbes.Count(); i < e; i ++ )
	{
		if( mProbes[ i ] == probe )
		{
			return (int)i;
		}
	}

	return -1;
}

//------------------------------------------------------------------------

EnvmapProbes g_EnvmapProbes;