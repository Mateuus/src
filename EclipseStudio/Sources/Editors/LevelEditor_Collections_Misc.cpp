#include "r3dPCH.h"
#include "LevelEditor_Collections.h"

 

int Is_Slope(int x,int z)
{

	r3dPoint3D pos;
	pos.x = x * gGridSize;
	pos.z = z * gGridSize; 
	float y = terra_GetH(pos);

	pos.x = (x+1) * gGridSize; 
	pos.z = z * gGridSize;
	float yr = terra_GetH(pos);

	pos.x = (x-1) * gGridSize;
	pos.z = z * gGridSize;
	float yl = terra_GetH(pos);

	pos.x = (x) * gGridSize;
	pos.z = (z+1) * gGridSize;
	float yu = terra_GetH(pos);

	pos.x = (x) * gGridSize;
	pos.z = (z-1) * gGridSize;
	float yd = terra_GetH(pos);


	float max = 20;

	if(fabs(y - yl) > max){
		return true;
	}
	if(fabs(y - yr) > max){
		return true;
	}
	if(fabs(y - yu) > max){
		return true;
	}
	if(fabs(y - yd) > max){
		return true;
	}

	return false;

}


void Get_Pos(int x,int z,const CInstanceData *data,r3dPoint3D *pos)
{
	r3d_assert(data);
	r3d_assert(pos);
	pos->x = x * gGridSize;
	pos->z = z * gGridSize;
	float cxx = data->mX;
	float czz = data->mZ;
	float range = gGridSize;
	float xx = ((cxx * (range)) / 65536.0f) - (range * 0.5f);
	float zz = ((czz * (range)) / 65536.0f) - (range * 0.5f);
	pos->x += xx; 
	pos->z += zz; 
	pos->y = data->mY;
}


CInstanceData gEditInstanceDbase[MAX_INSTANCE_DBASE];

void Instance_Init_Physics (CInstanceData & data, int x, int z )
{
	data.PhysicsConfig.group = PHYSCOLL_STATIC_GEOMETRY;
	data.PhysicsConfig.mass = 100;
	data.PhysicsConfig.offset = r3dVector ( 0, 0, 0 );
	data.PhysicsConfig.type = PHYSICS_TYPE_MESH;
	data.PhysicsConfig.ready = true;
	data.PhysicsConfig.needExplicitCollisionMesh = true;

	r3dPoint3D pos;
	Get_Pos(x,z,&data,&pos);

	r3dPoint3D size(5,5,5);
	const float iangle = ((float)data.mAngle) * CHAR_ANGLE_TO_FLOAT;
	D3DXMATRIX rotation; D3DXMatrixRotationY(&rotation, iangle);

	extern int gNumInstanceInfoData;
	if(gTreeTest[data.mWhich] && data.mWhich< gNumInstanceInfoData && gInstanceInfoData[data.mWhich].bPhysicsEnable)
	{
		if(!data.PhysObj)
		{
			data.PhysCallbackObj = new TreePhysicsCallbackObject();
			data.PhysCallbackObj->treeMesh = gTreeTest[data.mWhich];
			data.PhysObj = BasePhysicsObject::CreateStaticObject(data.PhysicsConfig, data.PhysCallbackObj, &pos, &size, gTreeTest[data.mWhich], &rotation );
		}
	}
}

void Instance_Done_Physics (CInstanceData & data )
{
	SAFE_DELETE(data.PhysObj);
	SAFE_DELETE(data.PhysCallbackObj);
}

CInstanceData *Get_New_Instance()
{
	for(int t = 0; t < MAX_INSTANCE_DBASE; t++){
		if(!gEditInstanceDbase[t].mUsed){
			gEditInstanceDbase[t].mUsed = true;
			gEditInstanceDbase[t].PhysObj = NULL;
			gEditInstanceDbase[t].PhysCallbackObj = NULL;
			return &gEditInstanceDbase[t];
		}
	}

	r3d_assert( false );

	return NULL;
}

void Free_Instance(CInstanceData *instance)
{
	if(instance->mUsed)
		Instance_Done_Physics(*instance);
	instance->mUsed = false;
}

