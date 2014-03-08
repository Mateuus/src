//
// simple GameObject
//
#include "r3dPCH.h"
#include "r3d.h"

#include "GameObjects/GameObj.h"
#include "GameObjects/EventTransport.h"

#include "GameCommon.h"

#include "sceneBox.h"

extern bool g_bEditMode;

IMPLEMENT_CLASS(GameObject, "GameObject", "Object");
AUTOREGISTER_CLASS(GameObject);


//
// GameObject
//
GameObject::GameObject()
	: vPos( 0, 0, 0 )
	, vScl( 1, 1, 1 )	
	, vRot( 0, 0, 0 )
	, vLoadTimePos(0 ,0, 0)
	, m_isActive(1)
	, ShadowExDirty(1)
{
	m_bEnablePhysics = true;
	m_isSerializable = true;
	InMainFrustum = false;
	FirstUpdate = 10;
	m_SceneBox  = 0;
	hashID		 = 0;
	ID           = invalidGameObjectID;
	ownerID      = invalidGameObjectID;
	CompoundID	 = 0;
	bPersistent	 = 1;
	
	pPrevObject  = NULL;
	pNextObject  = NULL;

	ObjTypeFlags = OBJTYPE_Generic;
	PrivateFlags = 0 ;
	ObjFlags     = 0;
	m_MinQualityLevel = OBJQ_Low;
	
	Name         = "GameObject";

	NetworkID    = 0;
	NetworkLocal = false;

	PhysicsObject = 0;
	RecreatePhysicsObject = false;
	Velocity     = r3dPoint3D(0, 0, 0);

	m_BulletPierceable = 0;
	

	time_LifeTime   = 0;

	DrawOrder	= OBJ_DRAWORDER_NORMAL;
	
	bbox_local.Org        = r3dPoint3D(0, 0, 0);
	bbox_local.Size       = r3dPoint3D(-1, -1, -1);
	bbox_world = bbox_local;
	bbox_radius = 0.0f;

	LastShadowExtrusion = 0.f;

	bLoaded = 0 ;

	return;
}

//-----------------------------------------------------------------------
GameObject::~GameObject()
//-----------------------------------------------------------------------
{
#ifndef FINAL_BUILD
	r3d_assert(pPrevObject == NULL);
	r3d_assert(pNextObject == NULL);
#endif

	if(PhysicsObject) delete PhysicsObject;
}

void MatrixGetYawPitchRoll ( const D3DXMATRIX & mat, float & fYaw, float & fPitch, float & fRoll );
void NormalizeYPR ( r3dVector & ypr );

BOOL GameObject::Update()
{
	// NOTE : IsSleeping perfomance is slow when scaled to 1000 objects,
	// that's why we use perma sleep flag ( most of these objects always sleep )
	if(
			!( ObjFlags & OBJFLAG_ForceSleep )
				&&
			PhysicsObject
				&& 
			(!PhysicsObject->IsSleeping() || FirstUpdate) )
	{
		if(FirstUpdate != 0)
			--FirstUpdate;
		// physics object is not sleeping, so let's update our position to match physics, and update transform
		r3dVector physPos = PhysicsObject->GetPosition();

		// sometimes physics returns QNAN position, not sure why
		if( r3d_float_isFinite(physPos.x) && r3d_float_isFinite(physPos.y) && r3d_float_isFinite(physPos.z) )
		{
			if( ( vPos - physPos ).LengthSq() > 32 * FLT_EPSILON )
				ShadowExDirty = true ;

			vPos = physPos;

			D3DXMATRIX mat = PhysicsObject->GetRotation();
			mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = 0.0f; mat.m[3][3] = 1.0f;
			r3dVector rot;
			MatrixGetYawPitchRoll ( mat, rot.x, rot.y, rot.z );
			NormalizeYPR ( rot );
			rot.x = R3D_RAD2DEG ( rot.x ); rot.y = R3D_RAD2DEG ( rot.y ); rot.z = R3D_RAD2DEG ( rot.z );
			if(!vRot.AlmostEqual(rot, 0.01f)) // do not change rotation if it didn't change
			{
				vRot = rot;
				ShadowExDirty = true ;
			}

			Velocity = PhysicsObject->GetVelocity();

			UpdateTransform();
		}
	}
	return TRUE;
}

/*virtual*/
BOOL
GameObject::UpdateLoading()
{
	return TRUE ;
}

void GameObject::SetPosition(const r3dPoint3D& pos)
{
	if(r3d_float_isFinite(pos.x) && r3d_float_isFinite(pos.y) && r3d_float_isFinite(pos.z)) // sanity check
	{
		if(PhysicsObject)
			PhysicsObject->SetPosition(pos);
		vPos    = pos;
		vLoadTimePos = pos;

		ShadowExDirty = true ;

		UpdateTransform();
	}
}

void GameObject::SetRotationVector(const r3dVector& Angles)
{
	if(vRot.AlmostEqual(Angles))
		return;

	if(PhysicsObject)
		PhysicsObject->SetRotation(Angles);
 	vRot = Angles;
	vLoadTimeRot = Angles;

	ShadowExDirty = true ;

	UpdateTransform();
}

void GameObject::SetVelocity(const r3dPoint3D& speed)
{
	if(PhysicsObject)
		PhysicsObject->SetVelocity(speed);
	Velocity    = speed;
}

const r3dPoint3D& GameObject::GetPosition() const
{
	return vPos;
}

const r3dVector& GameObject::GetRotationVector() const
{
	return vRot;
}

D3DXMATRIX GameObject::GetRotationMatrix() const
{
	D3DXMATRIX res;
	D3DXMatrixRotationYawPitchRoll(&res, R3D_DEG2RAD(vRot.X), R3D_DEG2RAD(vRot.Y), R3D_DEG2RAD(vRot.Z));
	return res;
}

const r3dPoint3D& GameObject::GetVelocity() const
{
	return Velocity;
}

void
GameObject::AppendSlicedShadowRenderables( RenderArray ( & render_arrays )[ rsCount ], int inMainFrustum, const r3dCamera& Cam )
{
	// find in which slice out object is
	// squared distances are not working in some edge cases, so using normal distances

	extern float gShadowSunMultiplier;

	r3dPoint3D pos = GetBBoxWorld().Center();

	D3DXVECTOR4 viewPos;

	D3DXVec3Transform( &viewPos, (D3DXVECTOR3*)&pos, &r3dRenderer->ViewMatrix );

	uint8_t shadow_slice = 0;
	float dist = (gCam - pos).Length();

	float objRadius =  this->GetObjectsRadius() + this->GetBBoxWorld().Size.y*gShadowSunMultiplier;

	float shadowSlice0 = ShadowSplitDistancesOpaque[0];
	float shadowSlice1 = ShadowSplitDistancesOpaque[1];
	float shadowSlice2 = ShadowSplitDistancesOpaque[2];
	float shadowSlice3 = ShadowSplitDistancesOpaque[3];

	extern float SunVectorXZProj ;
	float shadowDim = objRadius + LastShadowExtrusion * SunVectorXZProj ;

	float shadowsq_max = dist + shadowDim;
	float shadowsq_min = R3D_MAX(dist - shadowDim, shadowSlice0);
	
	const float sC1 = 0.9f; // some overlapping required
	const float sC2 = 1.0f;

	float maX = viewPos.x + objRadius ;
	float miX = viewPos.x - objRadius ;
	float maY = viewPos.y + objRadius ;
	float miY = viewPos.y - objRadius ;

	r3d_assert( r_active_shadow_slices->GetInt() <= 3 ) ;

	int num_slices = r_active_shadow_slices->GetInt() ;
	int cache_last = r_lfsm_wrap_mode->GetInt() ;

	if( (shadowsq_min >= shadowSlice0*sC1 && shadowsq_min < shadowSlice1*sC2) || (shadowsq_max > shadowSlice0*sC1 && shadowsq_max < shadowSlice1*sC2) )
	{
		if( inMainFrustum || ( num_slices == 1 && cache_last ) )
		{
			AppendShadowRenderables( render_arrays[ rsCreateSM + 0 ], Cam );
			AppendShadowOptimizations( &gShadowMapOptimizationDataOpaque[ 0 ], miX, maX, miY, maY );
		}
	}
	
	if( ( ( shadowsq_min >= shadowSlice1*sC1 || num_slices == 2 ) && shadowsq_min < shadowSlice2*sC2) || (shadowsq_max > shadowSlice1*sC1 && shadowsq_max < shadowSlice2*sC2) || (shadowsq_min <= shadowSlice1*sC1 && shadowsq_max > shadowSlice2*sC2) )
	{
		if( inMainFrustum || ( num_slices == 2 && cache_last ) )
		{
			AppendShadowRenderables( render_arrays[ rsCreateSM + 1 ], Cam );
			AppendShadowOptimizations( &gShadowMapOptimizationDataOpaque[ 1 ], miX, maX, miY, maY );
		}
	}

	if( ( ( shadowsq_min >= shadowSlice2*sC1 || num_slices == 3 ) && shadowsq_min < shadowSlice3*sC2) || (shadowsq_max > shadowSlice2*sC1 && shadowsq_max < shadowSlice3*sC2) || (shadowsq_min <= shadowSlice2*sC1 && shadowsq_max > shadowSlice3*sC2)  )
	{
		if( inMainFrustum || ( num_slices == 3 && cache_last ) )
		{
			AppendShadowRenderables( render_arrays[ rsCreateSM + 2 ], Cam );
			AppendShadowOptimizations( &gShadowMapOptimizationDataOpaque[ 2 ], miX, maX, miY, maY );
		}
	}

}

void
GameObject::AppendTransparentShadowRenderables( RenderArray & rarr, const r3dCamera& cam )
{
	(void)rarr, (void)cam;
}

void
GameObject::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& cam )
{
	(void)rarr, (void)cam;
}

void
GameObject::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& cam  )
{
	(void)render_arrays, (void)cam;
}

void GameObject::OnPickerMoved ()
{

}

void GameObject::OnPickerScaled ()
{

}

void GameObject::OnPickerRotated ()
{

}

void GameObject::OnPickerActionEnd()
{

}

BOOL GameObject::Load(const char* fname)
{
	char temp_name[512];
	r3dscpy(temp_name, fname);
	for(uint32_t i=0; i<strlen(temp_name); ++i)
	{
		if(temp_name[i] == '\\')
		{
			temp_name[i] = '/';
		}
	}	

	char* res = strrchr(temp_name, '/');
	if(res)
		Name = res;
	else 
		Name = temp_name;
	FileName = fname;

	// create a hash for this object, if we are serializing it - we will overwrite it with proper hash id right away during object construction
	// if we are creating it in editor - than we will have a proper hashID right away
	{
		// create a new hash for this object
		uint32_t counter = timeGetTime();
		time_t seconds = time(NULL);
		tm* date = localtime(&seconds);
		int rnd = rand();
		char hash_string[512];
		sprintf_s(hash_string, sizeof(hash_string), "%s_%d_%d%d%d_%d:%d:%d_%d", fname, counter, date->tm_year, date->tm_mon, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec, rnd);
		hashID = r3dHash::MakeHash(hash_string);
		if(hashID == 0x7FFFFFFF)
			__asm nop;
	}

	if( !( ObjFlags & OBJFLAG_AsyncLoading ) )
	{
		bLoaded = 1 ;
	}

	return TRUE;
}


BOOL GameObject::OnPositionChanged()
{
	if(m_SceneBox)
		m_SceneBox->Move(this);

	return TRUE;
};

BOOL GameObject::OnOrientationChanged()
{
	return TRUE;
}


BOOL GameObject::OnEvent(int eventId, void *data)
{
	return FALSE;
}



BOOL GameObject::OnCreate()
{
	R3DPROFILE_FUNCTION("GameObject::OnCreate");

	// we should only call it once
	r3d_assert((ObjFlags & OBJFLAG_JustCreated) && "multiple OnCreate calls");
	ObjFlags &= ~OBJFLAG_JustCreated;
	//r3dOutToLog("!!!!!!! ONCREATE %s\n", Name.c_str());

	if(!PhysicsConfig.ready)
		ReadPhysicsConfig(); // in case if we are creating object in editor, we need to load physics separately, as we are not calling ReadSerializedData
	CreatePhysicsData();

	extern void ObjectManager_Copy_OldState(GameObject *obj);
	ObjectManager_Copy_OldState(this);

	return TRUE;
}

BOOL GameObject::OnDestroy()
{
	return TRUE;
}

GameObject * GameObject::Clone ()
{
	return NULL;
}

BOOL GameObject::GetObjStat ( char * sStr, int iLen )
{
	sprintf_s (sStr, iLen, "Obj type \'%s\'\nObj name \'%s\'\nPOS \'[%f,%f,%f]\'\n", Class->Name.c_str (), Name.c_str(),GetPosition().x,GetPosition().y,GetPosition().z);
	return TRUE;
}


/*BOOL GameObject::CheckCollision(GameObject *obj, CollisionMove &move, CollisionInfo &trace)
{
	return FALSE;
}
*/
BOOL GameObject::CheckCastRay(const r3dPoint3D& vStart, const r3dPoint3D& vRay, float fRaytLen, float *fDist, int *facen)
{
	return FALSE;
}

void GameObject::SetColor(r3dColor24 &clr)
{
}

void GameObject::ReadSerializedData(pugi::xml_node& node)
{
	vLoadTimePos = vPos;
	// position is already set when object was created
	r3dVector A;

	pugi::xml_node gameObjNode = node.child("gameObject");
	hashID = gameObjNode.attribute("hash").as_uint();
	if(hashID == 0x7FFFFFFF)
		hashID = 0;
	if(!gameObjNode.attribute("PhysEnable").empty())
		m_bEnablePhysics = gameObjNode.attribute("PhysEnable").as_bool();
	if(!gameObjNode.attribute("MinQuality").empty())
		m_MinQualityLevel = (EGameObjQualityLevel)gameObjNode.attribute("MinQuality").as_int();
	if(!gameObjNode.attribute("BulletPierceable").empty())
		m_BulletPierceable = gameObjNode.attribute("BulletPierceable").as_int();

	if (!gameObjNode.attribute("DisableShadows").empty() && gameObjNode.attribute("DisableShadows").as_bool())
		ObjFlags |= OBJFLAG_DisableShadows;
	if (!gameObjNode.attribute("CollisionPlayerOnly").empty() && gameObjNode.attribute("CollisionPlayerOnly").as_bool())
	{
		ObjFlags |= OBJFLAG_PlayerCollisionOnly | OBJFLAG_SkipOcclusionCheck;
		if(!g_bEditMode)
			ObjFlags |= OBJFLAG_SkipDraw;
	}
	if (!gameObjNode.attribute("DetailObject").empty() && gameObjNode.attribute("DetailObject").as_bool())
	{
		ObjFlags |= OBJFLAG_Detailed;
	}
		
	pugi::xml_node rotNode = gameObjNode.child("rotation");
	if(!rotNode.empty())
	{
		A.x = rotNode.attribute("x").as_float();
		A.y = rotNode.attribute("y").as_float();
		A.z = rotNode.attribute("z").as_float();
		// fixes a case when there is a trash in rotation. not sure where it's coming from though
		if(A.x < -100000 || A.x > 100000) A.x = 0;
		if(A.y < -100000 || A.y > 100000) A.y = 0;
		if(A.z < -100000 || A.z > 100000) A.z = 0;

		// fixup orientation
		while ( A.X < 0.0f ) 
			A.X += 360.0f;
		while ( A.X > 360.0f ) 
			A.X -= 360.0f;
		while ( A.Y < 0.0f ) 
			A.Y += 360.0f;
		while ( A.Y > 360.0f ) 
			A.Y -= 360.0f;
		while ( A.Z < 0.0f ) 
			A.Z += 360.0f;
		while ( A.Z > 360.0f ) 
			A.Z -= 360.0f;
		SetRotationVector(A);
	}

	pugi::xml_node scaleNode = gameObjNode.child("scale");
	if(!scaleNode.empty())
		SetScale(r3dPoint3D(scaleNode.attribute("x").as_float(),scaleNode.attribute("y").as_float(),scaleNode.attribute("z").as_float()));

	ReadPhysicsConfig();
}

void GameObject::WriteSerializedData(pugi::xml_node& node)
{
	pugi::xml_node gameObjNode = node.append_child();
	gameObjNode.set_name("gameObject");
	gameObjNode.append_attribute("hash") = hashID;
	gameObjNode.append_attribute("PhysEnable") = m_bEnablePhysics;
	gameObjNode.append_attribute("MinQuality") = (int)m_MinQualityLevel;
	gameObjNode.append_attribute("BulletPierceable") = m_BulletPierceable;
	r3dPoint3D rot = vRot;
	if(PhysicsConfig.isDynamic)
		rot = vLoadTimeRot;
	if(!rot.AlmostEqual(r3dPoint3D(0.0f,0.0f,0.0f)))
	{
//		if(!vLoadTimeRot.AlmostEqual(rot))
//			__asm nop;

		pugi::xml_node rotNode = gameObjNode.append_child();
		rotNode.set_name("rotation");
		char temp_str[128];
		// save floats at %.4f to make merging easier
		sprintf(temp_str, "%.4f", rot.x);
		rotNode.append_attribute("x") = temp_str;
		sprintf(temp_str, "%.4f", rot.y);
		rotNode.append_attribute("y") = temp_str;
		sprintf(temp_str, "%.4f", rot.z);
		rotNode.append_attribute("z") = temp_str;
	}

	if(!vScl.AlmostEqual(r3dPoint3D(1.0f,1.0f,1.0f)))
	{
		pugi::xml_node scaleNode = gameObjNode.append_child();
		scaleNode.set_name("scale");
		char temp_str[128];
		// save floats at %.4f to make merging easier
		sprintf(temp_str, "%.4f", vScl.x);
		scaleNode.append_attribute("x") = temp_str;
		sprintf(temp_str, "%.4f", vScl.y);
		scaleNode.append_attribute("y") = temp_str;
		sprintf(temp_str, "%.4f", vScl.z);
		scaleNode.append_attribute("z") = temp_str;
	}

	if (ObjFlags & OBJFLAG_DisableShadows)
		gameObjNode.append_attribute("DisableShadows") = true;
	if (ObjFlags & OBJFLAG_PlayerCollisionOnly)
		gameObjNode.append_attribute("CollisionPlayerOnly") = true;
	if (ObjFlags & OBJFLAG_Detailed)
		gameObjNode.append_attribute("DetailObject") = true;
}


BOOL GameObject::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
  return FALSE;
}

void GameObject::SavePhysicsData()
{
	r3dMesh* mesh = GetObjectMesh();
	r3d_assert(mesh);
	char physicsFilename[256];
	r3dscpy(physicsFilename, mesh->FileName.c_str());
	int len = strlen(physicsFilename);
	r3dscpy(&physicsFilename[len-3], "phx");
	pugi::xml_document xmlFile;
	xmlFile.append_child(pugi::node_comment).set_value("physics data");
	pugi::xml_node xmlPhysics = xmlFile.append_child();
	xmlPhysics.set_name("physics");
	xmlPhysics.append_attribute("collision_group") = PhysicsConfig.group;
	xmlPhysics.append_attribute("type") = PhysicsConfig.type;
	xmlPhysics.append_attribute("mass") = PhysicsConfig.mass;
	xmlPhysics.append_attribute("dynamic") = PhysicsConfig.isDynamic;
	xmlPhysics.append_attribute("explMesh") = PhysicsConfig.needExplicitCollisionMesh;

	xmlFile.save_file(physicsFilename);
}

PhysicsObjectConfig GameObject::LoadPhysicsConfig(r3dMesh* mesh)
{
	r3d_assert(mesh);

	PhysicsObjectConfig PhysicsConfig;

	// not sure about this, it might be slow to do that for each game object
	char physicsFilename[256];
	r3dscpy(physicsFilename, mesh->FileName.c_str());
	int len = strlen(physicsFilename);
	r3dscpy(&physicsFilename[len-3], "phx");
	if(r3d_access(physicsFilename, 0) == 0)
	{
		r3dFile* f = r3d_open(physicsFilename, "rb");
		r3d_assert(f);
		char* fileBuffer = new char[f->size + 1];
		fread(fileBuffer, f->size, 1, f);
		fileBuffer[f->size] = 0;
		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(fileBuffer, f->size);
		fclose(f);
		if(!parseResult)
			r3dError("Failed to parse XML, error: %s", parseResult.description());
		pugi::xml_node xmlPhysics = xmlFile.child("physics");
		if(xmlPhysics.attribute("collision_group").empty())
		{
			PhysicsConfig.group = PHYSCOLL_COLLISION_GEOMETRY;
			int group = xmlPhysics.attribute("group").as_uint();
			if(group == 1)
				PhysicsConfig.isDynamic = true;
		}
		else
		{
			PhysicsConfig.group = (PHYSICS_COLLISION_GROUPS)xmlPhysics.attribute("collision_group").as_uint();
			PhysicsConfig.isDynamic = xmlPhysics.attribute("dynamic").as_bool();
		}
		if(!xmlPhysics.attribute("explMesh").empty())
			PhysicsConfig.needExplicitCollisionMesh = xmlPhysics.attribute("explMesh").as_bool();
		
		PhysicsConfig.type = (PHYSICS_TYPE)xmlPhysics.attribute("type").as_uint();
		PhysicsConfig.mass = xmlPhysics.attribute("mass").as_float();
		PhysicsConfig.ready = true;

		delete [] fileBuffer;
	}

	return PhysicsConfig;
}

void GameObject::ReadPhysicsConfig()
{
	r3dMesh* mesh = GetObjectMesh();
	if(mesh && PhysicsObject == NULL)
		PhysicsConfig = LoadPhysicsConfig(mesh);

	if(ObjFlags & OBJFLAG_PlayerCollisionOnly)
		PhysicsConfig.group = PHYSCOLL_PLAYER_ONLY_GEOMETRY;
}

void GameObject::CreatePhysicsData()
{
	if(PhysicsConfig.ready && PhysicsObject == 0 && m_bEnablePhysics)
	{
		if(PhysicsConfig.isDynamic || PhysicsConfig.isKinematic)
		{
			PhysicsObject = BasePhysicsObject::CreateDynamicObject(PhysicsConfig, this);
			ObjFlags &= ~OBJFLAG_ForceSleep ;
		}
		else
		{
			PhysicsObject = BasePhysicsObject::CreateStaticObject(PhysicsConfig, this);
			ObjFlags |= OBJFLAG_ForceSleep ;
		}
	}	
}

bool GameObject::isDetailedVisible() const
{
	if (ObjFlags & OBJFLAG_Detailed)
	{
		r3dVector objVec = gCam - GetPosition();
		float distSquared = objVec.LengthSq();
		float detailRadiusSquared = r_level_detail_radius->GetFloat();
		detailRadiusSquared *= detailRadiusSquared;
		return distSquared < detailRadiusSquared;
	}
	return true;
}

#ifndef FINAL_BUILD

void GameObject::UpdatePhysicsEnabled( const int& physicsEnabled )
{
	if(physicsEnabled != (int)m_bEnablePhysics)
	{
		m_bEnablePhysics = physicsEnabled?true:false;
		if(!m_bEnablePhysics)
		{
			SAFE_DELETE(PhysicsObject);
		}
		else
			CreatePhysicsData();
	}
}

void GameObject::UpdateCollisionOnly( const int& newCollisionOnly )
{
	int collisionOnly = (ObjFlags & OBJFLAG_PlayerCollisionOnly)?1:0;

	if(collisionOnly != newCollisionOnly)
	{
		if(newCollisionOnly)
			ObjFlags |= OBJFLAG_PlayerCollisionOnly | OBJFLAG_SkipOcclusionCheck;
		else
			ObjFlags &= ~(OBJFLAG_PlayerCollisionOnly | OBJFLAG_SkipOcclusionCheck);

		// now we also need to recreate our physics data to update collision
		if(m_bEnablePhysics)
		{
			ReadPhysicsConfig();
			SAFE_DELETE(PhysicsObject);
			CreatePhysicsData();
		}

	}
}

void GameObject::UpdateDetailObject( const int& detailObject )
{
	if (detailObject)
		ObjFlags |= OBJFLAG_Detailed;
	else
		ObjFlags &= ~OBJFLAG_Detailed;
}

void GameObject::UpdateScale( const r3dVector& scale )
{
	SetScale( scale ) ;
}

void GameObject::UpdateRotation( const r3dVector& vRot )
{
	SetRotationVector ( vRot );
}

void GameObject::UpdatePosition( const r3dPoint3D &vPos )
{
	SetPosition( vPos );
}

float GameObject::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected )
{
	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		float starty = scry;

		r3dVector vRotation = vRot;
		if ( vRotation.x < 0.0f )
			vRotation.x += 360.0f;
		if ( vRotation.y < 0.0f )
			vRotation.y += 360.0f;
		if ( vRotation.z < 0.0f )
			vRotation.z += 360.0f;

		r3dVector scale = vScl;

		starty += imgui_Static ( scrx, starty, "Base Parameters" );
		int physicsEnabled = m_bEnablePhysics;
		starty += imgui_Checkbox(scrx, starty, "Enable Physics", &physicsEnabled, 1);

		if( physicsEnabled != (int)m_bEnablePhysics )
		{
			PropagateChange( physicsEnabled, &GameObject::UpdatePhysicsEnabled, selected ) ;
		}

		int collisionOnly = (ObjFlags & OBJFLAG_PlayerCollisionOnly)?1:0;
		int newCollisionOnly = collisionOnly;
		starty += imgui_Checkbox(scrx, starty, "Player only collision", &newCollisionOnly, 1);

		if( newCollisionOnly != collisionOnly )
		{
			PropagateChange( newCollisionOnly, &GameObject::UpdateCollisionOnly, selected ) ;
		}

		int BulletPierceable = m_BulletPierceable;
		starty += imgui_Value_SliderI(scrx, starty, "Bullet Pierceable", &BulletPierceable,	0.0f, 100.0f, "%d");
		if ( (BulletPierceable-m_BulletPierceable) != 0)
			PropagateChange( BulletPierceable, &GameObject::m_BulletPierceable, this, selected ) ;

		const char* quality_level_str[] = {"Low", "Medium", "High", "Ultra"};
		if(imgui_Button(scrx+150, starty, 30, 25, "<"))
		{
			int curQ = (int)m_MinQualityLevel;
			curQ--;
			if(curQ<(int)OBJQ_Low)
				curQ = (int)OBJQ_Ultra;

			PropagateChange( (EGameObjQualityLevel)curQ, &GameObject::m_MinQualityLevel, this, selected );
		}
		if(imgui_Button(scrx+180, starty, 30, 25, ">"))
		{
			int curQ = (int)m_MinQualityLevel;
			curQ++;
			if(curQ>(int)OBJQ_Ultra)
				curQ = (int)OBJQ_Low;

			PropagateChange( (EGameObjQualityLevel)curQ, &GameObject::m_MinQualityLevel, this, selected );
		}

		imgui_Static(scrx, starty, "Minimum Quality: ", 100);
		starty += imgui_Static(scrx+100, starty, quality_level_str[(int)m_MinQualityLevel-1], 50);

		if(PhysicsObject) // no scale on physics objects!!! PhysX doesn't support scale
		{
			scale.Assign(1.0f, 1.0f, 1.0f);
		}
		else
		{
			starty += imgui_Value_Slider(scrx, starty, "Scale X", &scale.x,	0.001f, 200.0f,	"%3.2f", 1);
			starty += imgui_Value_Slider(scrx, starty, "Scale Y", &scale.y,	0.001f, 200.0f,	"%3.2f", 1);
			starty += imgui_Value_Slider(scrx, starty, "Scale Z", &scale.z,	0.001f, 200.0f,	"%3.2f", 1);
		}

		starty += imgui_Value_Slider(scrx, starty, "Rotation X", &vRotation.x,	0.0f, 360.0f,	"%3.2f", 1);
		starty += imgui_Value_Slider(scrx, starty, "Rotation Y", &vRotation.y,	0.0f, 360.0f,	"%3.2f", 1);
		starty += imgui_Value_Slider(scrx, starty, "Rotation Z", &vRotation.z,	0.0f, 360.0f,	"%3.2f", 1);
		starty += 1;

		r3dPoint3D vPos = GetPosition();
		starty += imgui_Value_Slider(scrx, starty, "Position X", &vPos.x, -FLT_MAX, FLT_MAX,	"%3.2f", 0);
		starty += imgui_Value_Slider(scrx, starty, "Position Y", &vPos.y, -FLT_MAX, FLT_MAX,	"%3.2f", 0);
		starty += imgui_Value_Slider(scrx, starty, "Position Z", &vPos.z, -FLT_MAX, FLT_MAX,	"%3.2f", 0);
		starty += 1;

		if ( fabsf( scale.x - vScl.x ) > 0.00001f ||
			 fabsf( scale.y - vScl.y ) > 0.00001f ||
			 fabsf( scale.z - vScl.z ) > 0.00001f )
		{
			PropagateChange( scale, &GameObject::UpdateScale, selected ) ;
		}

		if ( fabsf( vRotation.x - vRot.x ) > 0.00001f ||
			 fabsf( vRotation.y - vRot.y ) > 0.00001f ||
			 fabsf( vRotation.z - vRot.z ) > 0.00001f )
		{
			PropagateChange( vRotation, &GameObject::UpdateRotation, selected ) ;
		}

		r3dPoint3D vPosOrig = GetPosition();
		if ( fabsf( vPosOrig.x - vPos.x ) > 0.00001f ||
			 fabsf( vPosOrig.y - vPos.y ) > 0.00001f ||
			 fabsf( vPosOrig.z - vPos.z ) > 0.00001f )
		{
			PropagateChange( vPos, &GameObject::UpdatePosition, selected ) ;
		}

		int detObj = ObjFlags & OBJFLAG_Detailed ? 1 : 0;
		int detObjOld = detObj ;
		starty += imgui_Checkbox(scrx, starty, "Detail object", &detObj, 1);

		if( !!detObj != !!detObjOld )
		{
			PropagateChange(detObj, &GameObject::UpdateDetailObject, selected);
		}

		return starty - scry;
	}
	else
	{
		return 0 ;
	}
}

#endif
#ifndef FINAL_BUILD
/*virtual*/
void
GameObject::DrawSelected( const r3dCamera& Cam, eRenderStageID DrawState )
{
	(void)Cam, (void)DrawState;
}
#endif

//////////////////////////////////////////////////////////////////////////

void GameObject::PrecalculateMatrices()
{
	r3dMesh *m = GetObjectLodMesh() ;
	r3dPoint3D *scale ;
	r3dPoint2D *texcScale ;
		
	if( m )
	{
		scale = m->IsSkeletal() ? 0 : &m->unpackScale;
		texcScale = &m->texcUnpackScale ;
	}
	else
	{
		scale = 0 ;
		texcScale = 0 ;
	}

	r3dPrepareMeshVSConsts(preparedVSConsts, GetTransformMatrix(), scale, texcScale );
}

void
GameObject::SetTransparentShadowCasting( bool enabled )
{
	if( enabled )
	{
		// first set flag, then provoke adding
		PrivateFlags |= PRIVFLAG_TransparentShadowCaster ;
		GameWorld().UpdateTransparentShadowCaster( this ) ;
	}
	else
	{
		// first provoke removal, then clear flag
		GameWorld().UpdateTransparentShadowCaster( this ) ;
		PrivateFlags &= ~PRIVFLAG_TransparentShadowCaster ;
	}
}

ShadowExtrusionData::ShadowExtrusionData()
: MinX		( 0.f )
, MinY		( 0.f )
, MinZ		( 0.f )
, MaxX		( 0.f )
, MaxY		( 0.f )
, Extrude	( 0.f )
{
	D3DXMatrixIdentity( &ToExtrusionBox );
}

int IsParentOrEqual( const AClass* This,  const AClass* SuspectedChild )
{
	if( This == SuspectedChild )
		return 1 ;

	for( const AClass* expert = SuspectedChild ; expert != &AObject::ClassData ; expert = expert->BaseClass )
	{
		if( expert == This )
			return 1 ;
	}

	return 0 ;

}