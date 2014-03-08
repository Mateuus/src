#include "r3dPCH.h"

#if R3D_CALL_GRANNY

#include "r3d.h"

#include "GameCommon.h"

#include "gameobjects/obj_Mesh.h"

#include "r3dGrannyLoader.h"

#include "GrannyTestSkeleton.h"

//------------------------------------------------------------------------

GrannyTestSkeleton::GrannyTestSkeleton()
: mBoundObject( 0 )
, mLocalPose( 0 )
{

}

//------------------------------------------------------------------------

GrannyTestSkeleton::~GrannyTestSkeleton()
{
	for( FileMap::iterator i = mFileMap.begin(), e = mFileMap.end() ; i != e ; ++ i )
	{
		r3dCloseGranny( i->second ) ;
	}
}

//------------------------------------------------------------------------

void
GrannyTestSkeleton::AddAnimation( const char* fileName )
{
	FileMap::const_iterator found = mFileMap.find( fileName ) ;

	if( found == mFileMap.end() )
	{
		r3dGrannyFile* graFile = r3dOpenGranny( fileName ) ;

		if( graFile->fileInfo->AnimationCount >= 1 )
		{
			mAnimMap[ fileName ] = graFile->fileInfo->Animations[ 0 ] ;

			mFileMap[ fileName ] = graFile ;
		}
		else
		{
			r3dCloseGranny( graFile ) ;
		}
	}
}

//------------------------------------------------------------------------

void
GrannyTestSkeleton::BindToMeshObject( granny_model_instance* obj )
{
	mBoundObject = obj ;

	if( mBoundObject )
	{
		if( mLocalPose )
		{
			GrannyFreeLocalPose( mLocalPose );
			mLocalPose = 0 ;
		}

		mLocalPose = GrannyNewLocalPose( GrannyGetSourceSkeleton( obj )->BoneCount );
	}
}

//------------------------------------------------------------------------

void
GrannyTestSkeleton::Update()
{
	if( mBoundObject )
	{
		GrannySetModelClock( mBoundObject, r3dGetTime() ) ;
		GrannySampleModelAnimations( mBoundObject, 0, GrannyGetSourceSkeleton( mBoundObject )->BoneCount, mLocalPose );
	}
}

//------------------------------------------------------------------------

void
GrannyTestSkeleton::GetAnimationNames( stringlist_t* oStrings )
{
	oStrings->clear();

	char drive[ 8 ], dir[ 256 ], name[ 256 ], ext[ 64 ] ;

	for( AnimationMap::iterator i = mAnimMap.begin(), e = mAnimMap.end() ; i != e ; ++ i )
	{
		_splitpath( i->first.c_str(), drive, dir, name, ext ) ;

		oStrings->push_back( name ) ;
	}
}

//------------------------------------------------------------------------

struct granny_local_pose*
GrannyTestSkeleton::GetLocalPose() const
{
	return mLocalPose ;
}

//------------------------------------------------------------------------

void
GrannyTestSkeleton::AddAllAnimationsFrom( const char* Path )
{
	WIN32_FIND_DATA ffblk;
	HANDLE h;

	char fullPath[1024];

	strcpy( fullPath, Path ) ;
	strcat( fullPath, "/*.gr2" ) ;

	h = FindFirstFile( fullPath, &ffblk );
	if( h != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if ( !( ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
			{
				strcpy( fullPath, Path );
				strcat( fullPath, "/" ) ;
				strcat( fullPath, ffblk.cFileName );
				AddAnimation( fullPath );
			}
		}
		while( FindNextFile( h, &ffblk ) != 0 );

		FindClose( h );
	}
}

//------------------------------------------------------------------------

void GrannyTestSkeleton::SelectActiveAnimation( const char* Name )
{
	if( mBoundObject )
	{

		for( int i = 0, e = mActiveAnims.Count() ; i < e; i ++ )
		{
			GrannySetControlWeight( mActiveAnims[ i ], 0.f ) ;
			GrannySetControlActive(	mActiveAnims[ i ], false ) ;
		}

		mActiveAnims.Clear();

		for( AnimationMap::iterator i = mAnimMap.begin(), e = mAnimMap.end() ; i != e ; ++ i )
		{
			if( strstr( i->first.c_str(), Name ) )
			{
				granny_control* ctrl = GrannyPlayControlledAnimation( 0, i->second, mBoundObject ) ;

				mActiveAnims.PushBack( ctrl ) ;		

				GrannySetControlLoopCount(ctrl, 0);
				GrannyFreeControlOnceUnused(ctrl);



				break ;
			}
		}
	}
}

#endif