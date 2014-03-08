#pragma once

class GrannyTestSkeleton
{
public:

	typedef std::string string ;
	typedef std::vector<std::string> stringlist_t;

	typedef std::map< string, struct granny_animation* > AnimationMap ;
	typedef std::map< string, struct r3dGrannyFile* > FileMap ;
	typedef r3dTL::TArray< string > Strings ;

	typedef r3dTL::TArray< struct granny_control* > ActiveAnimations ;

public:
	GrannyTestSkeleton();
	~GrannyTestSkeleton();

	void AddAnimation( const char* fileName ) ;
	void BindToMeshObject( struct granny_model_instance* obj ) ;
	void Update();

	void GetAnimationNames( stringlist_t* oStrings );

	struct granny_local_pose* GetLocalPose() const ;

	void AddAllAnimationsFrom( const char* Path );
	void SelectActiveAnimation( const char* Name );

	struct granny_model_instance* GetBoundObject() const { return mBoundObject ; }

private:

	AnimationMap		mAnimMap ;
	ActiveAnimations	mActiveAnims ;
	FileMap				mFileMap ;

	struct granny_model_instance*	mBoundObject ;

	struct granny_local_pose*		mLocalPose ;
};