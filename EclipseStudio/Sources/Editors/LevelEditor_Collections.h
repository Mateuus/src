#ifndef LEVEL_COLLECTIONS_H
#define LEVEL_COLLECTIONS_H

#include "r3dPCH.h"
#include "r3d.h"
#include "r3dLight.h"
#include "r3dUtils.h"
#include "d3dfont.h"
#include "GameCommon.h"
//#include "Editors\Tool_AxisControl.h"
#include "LevelEditor.h"
    
  

//#define NUM_INSTANCE_GRID		100
#define INST_GMAP_SIZE		500
#define INST_GMAP_SIZE_NEW		320
#define MAX_INSTANCE_DBASE			250000
#define CHAR_ANGLE_TO_FLOAT		(R3D_PI * 2.0f * (1.0f/256.0f))
#define MAX_HWINSTANCE_TYPES		20
#define MAX_INSTANCE_TYPE	100

#define MAX_INSTANCE_GROUP		20
#define MAX_GROUND_TYPES		5

union FC { float f; unsigned char c[4];int i; } ;
union USC { unsigned short s; unsigned char c[2]; } ;


struct CInstanceData_InFile{

	// this is min for a object.. also will need one more char for the height..fuck
	unsigned short	mX;
	unsigned short	mZ;
	float			mY;							// center Y point 4
	unsigned char	mAngle;								// 1 bytes
	unsigned char	mWhich;								// which tree goes here.	// 1 bytes
	unsigned char	mScale;								// which tree goes here.	// 1 bytes
	
	union{
		struct CInstanceData *mPrev;
		int		mGridX;
	};
	union{
		struct CInstanceData *mNext;
		int mGridZ;
	};
	bool mUsed; // for allocation info.

	CInstanceData_InFile()
	{
		mX = 0;
		mZ = 0;
		mY = 0;
		mAngle = 0;
		mWhich = 0;
		mScale = 0;
		mGridX = 0;
		mGridZ = 0;
		mUsed = 0;
	}
};


class TreePhysicsCallbackObject : public PhysicsCallbackObject
{
public:
	TreePhysicsCallbackObject() : treeMesh(NULL) {}

	virtual	void OnCollide(PhysicsCallbackObject *obj, CollisionInfo &trace) {}; // do nothing
	virtual GameObject* isGameObject() { return NULL; }
	virtual r3dMesh* hasMesh() { return treeMesh; }

	r3dMesh* treeMesh;
};

struct CInstanceData : public CInstanceData_InFile
{
	PhysicsObjectConfig	PhysicsConfig ;
	ShadowExtrusionData	ShadowExData ;

	BasePhysicsObject* PhysObj;
	TreePhysicsCallbackObject* PhysCallbackObj;

	float			bendSpeed ;
	float			bendVal ;
	float			windPower ;

	unsigned char	mRandomColor ;						// randomize color
	bool			ShadowExDataDirty;
	bool			wasVisible ;

	CInstanceData();
};

extern CInstanceData gEditInstanceDbase[MAX_INSTANCE_DBASE];

CInstanceData *Get_New_Instance();
void Free_Instance(CInstanceData *insatnce); // does it take out of link list??? think soo...
void Instance_Init_Physics (CInstanceData & data, int x, int z );
void Instance_Done_Physics (CInstanceData & data );


// OLD GRID-- JUST USED FOR EDITOR~!!
struct CInstanceInfo{

	int mNum;
	CInstanceData *mList; // pointer to head

	void Add_Instance(CInstanceData *instance);
	void Del_Instance(CInstanceData *instance);

	CInstanceData *Get_Instance(int which) const;
	
	int Get_Num() const { return mNum; }			// for now fake it.


};

#if 0
struct CInstanceInfoData{
	char *mMeshName;
	char *mTexName;
	r3dVector mScale; 
	char *mBBMeshName; // null means use rect
	char *mBBTexName;
	r3dVector mScale2; 
};
#else

struct CInstanceInfoData{
	char mMeshName[64];

	r3dVector mScale; 
	float mRenderDist; // max render dist

	float mLOD1Dist; // dist to swap to lod1
	float mLOD2Dist; // dist to swap to lod2
	int	 mLOD1Offset; // need these fore PS3 only, but PC could use !
	int	 mLOD2Offset;
	int bPhysicsEnable;
	int hasAnimation;
	bool hasLOD1;
	bool hasLOD2;	

	CInstanceInfoData()
	{
		memset(this, 0, sizeof(*this));
	}
};

bool operator == (const CInstanceInfoData &l, const CInstanceInfoData &r);
bool operator != (const CInstanceInfoData &l, const CInstanceInfoData &r);

#endif


// NEW GRID: USED FOR PS3
// center of the grid
struct CInstanceInfoNew
{
	unsigned short	CenterY;														// 2 bytes
	unsigned short  NumInstance;													// 2 byte
	unsigned int	Offset; // offset in database to start of this instance		// 4 bytes
};  

struct CInstanceInfoDBase{
	unsigned char	mX;
	unsigned char	mZ;
	unsigned char	mAngle;
	unsigned char	mWhich;
	unsigned char	mScale; 
	unsigned char	mTint;
	unsigned short	mY;
};



// for collections editor
struct CollectionCircle{
	r3dPoint3D Pos;
	float		Radius;
};

struct CollectionType
{
//	float		Offset;
	stringlist_t	DataDirectory;
};




extern Editor_Level	LevelEditor;
extern	r3dPoint3D	UI_TargetPos;		// snapped to object position (object center)
extern	r3dPoint3D  UI_TerraTargetPos;
extern	gobjid_t	UI_TargetObjID;
extern	r3dMaterial	*UI_TargetMaterial;
extern CInstanceInfo gInstanceGridMap[INST_GMAP_SIZE][INST_GMAP_SIZE];
extern int gDoInstanceTint[MAX_HWINSTANCE_TYPES];
extern int gInstanceSaved;
extern float gGridSize; // 40!!!
extern CInstanceInfoData gInstanceInfoData[];
extern r3dMesh *gTreeTest[MAX_INSTANCE_TYPE];
extern r3dMesh *gTreeTestLOD[MAX_INSTANCE_TYPE];
extern r3dMesh *gTreeTestLOD2[MAX_INSTANCE_TYPE];

//extern r3dTexture *gTreeTex[MAX_INSTANCE_TYPE];
//extern r3dTexture *gTreeTexLOD[MAX_INSTANCE_TYPE];
//extern r3dTexture *gTreeTexBillboards[MAX_INSTANCE_TYPE];

extern CInstanceInfoNew gInstanceGridMapNew[INST_GMAP_SIZE_NEW][INST_GMAP_SIZE_NEW];
// this should be allocated!!
extern CInstanceInfoDBase gInstanceDatabase[MAX_INSTANCE_DBASE];

 
struct InstanceGroupInfo
{
	int		index;
	float	Density;	
	float	Scale;		
	float	Rotate;	
	float	Spacing;	
	int		Visible;
	int		Slope;

	void Reset();
};

bool operator == (const InstanceGroupInfo &l, const InstanceGroupInfo &r);
bool operator != (const InstanceGroupInfo &l, const InstanceGroupInfo &r);

extern InstanceGroupInfo gInstanceGroup[MAX_INSTANCE_GROUP];


struct InstanceMeshParams
{
	InstanceMeshParams() ;

	int UIVisible;

	enum { MAX_ANIM_LAYERS = 3 } ;

	struct AnimLayer
	{
		float Scale ;
		float Freq ;
		float Speed ;
	} AnimLayers [ MAX_ANIM_LAYERS ] ;

	float Stiffness ;
	float Mass ;
	float BendPow ;
	float LeafMotionRandomness ;
	float Dissipation ;
	float BorderDissipation ;
	float BorderDissipationStrength ;

} extern gInstanceMeshParams [ MAX_INSTANCE_TYPE ];

extern int gNumInstanceGroup;
extern float gInstanceDensity;

extern float BrushRadius;
extern int gCurrentLayer;

void SetInstanceViewRefPos( const r3dPoint3D& pos );

// grid
void CheckedCollectionsSave();
void Save_Instance_Map( bool isAutoSave );
void Init_Instance_Map();
float terra_GetH(const r3dPoint3D &vPos);
 
// collections.
void Load_Collections();
void SaveCollections ( bool saveMeshParams );
void r3dDrawCircle3DT(const r3dPoint3D& P1, const float Radius, const r3dCamera &Cam, float Width, const r3dColor24& clr, r3dTexture *Tex=NULL);
void r3dDrawCircle3DT_GrassPlanes(const r3dPoint3D& P1, const float Radius, const r3dCamera &Cam, float Width, const r3dColor24& clr, r3dTexture *Tex=NULL);
void DrawCollectionBrush();

void CloseCollections();

// collecions_misc
void Get_Pos(int x,int z,const CInstanceData *data,r3dPoint3D *pos);
int Is_Slope(int x,int z);

// collections_brush_logic
int To_Close(int x,int z,CInstanceData *data);
int Pick_Spot(int x,int z, CInstanceData *data,r3dPoint3D &pos);
int Too_Dense(int x,int z);
void Clear_Grid_Radius();
int Get_Current_Tree(int x,int z);

r3dMaterial* Get_Material_By_Ray(const r3dPoint3D& vStart, const r3dPoint3D& vRay);

enum InstanceDrawModeEnum
{
	R3D_IDME_NORMAL,
	R3D_IDME_SHADOW,
	R3D_IDME_DEPTH
};

void Draw_Instance_Map( InstanceDrawModeEnum drawMode ) ;

// in GUI code
int imgui_Checkbox_Small(float x, float y,int wid, const char* name, int* edit_val, const DWORD flag, bool bUseDesctop = true );

#endif // LEVEL_COLLECTIONS_H




