#ifndef	__PWAR_OBJMANAG_H
#define	__PWAR_OBJMANAG_H

#include "GameObj.h"

enum SnapType_t
{
	eSnapType_Pivot,
	eSnapType_Vertex,

	eSnapType_Count
};

struct SnapInfo_t
{
	SnapType_t	eType;
	float		fRadius;	
};


struct SnapPointResult_t
{
	GameObject *	pObj;
	r3dVector		vPos;
};

struct draw_s {
	GameObject	*obj;
	float		dist;
	uint8_t		shadow_slice; // 1,2,3 bit flag
};
#define OBJECTMANAGER_MAXOBJECTS 10000

#include "sceneBox.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class ObjectManagerResourceHelper : public r3dIResource
{
public:

	ObjectManagerResourceHelper();
	virtual ~ObjectManagerResourceHelper();

	virtual	void		D3DCreateResource() {};
	virtual	void		D3DReleaseResource();
};

class ObjectManager
{
private:
	typedef void (*GameObjEvent_fn)(GameObject * pObj);

	GameObjEvent_fn pObjectAddEvent;
	GameObjEvent_fn pObjectDeleteEvent;

	int				m_FrameId ;
	SceneBox*		m_pRootBox;
	ObjectManagerResourceHelper* m_ResourceHelper;

	CRITICAL_SECTION m_CS ;

public:
	bool			GetSnapPoint		( const r3dPoint2D &vCursor, const SnapInfo_t &tInfo, SnapPointResult_t &tRes );
	void			DrawDebug(const r3dCamera& Cam);
	void			AppendDebugBoxes();

	int				GetFrameId() ;

	int		bInited;

	r3dSec_type<GameObject*, 0x0F9FD85A> pFirstObject;
	r3dSec_type<int, 0x1FDC2DE9> MaxObjects;
	r3dSec_type<GameObject*, 0xF83A8CD2> pLastObject;
	r3dSec_type<int, 0xF4F1CD3F> NumObjects;
	r3dSec_type<int, 0xF22EB5C0> CurObjID;
	r3dSec_type<int, 0xDA4CE36C> LastFreeObject;
	r3dSec_type<GameObject**, 0x30D6ADFC> pObjectArray;

	// prolly no need to protect these - they're unimportant - used for particle
	// shadow casting only
	r3dTL::TFixedArray< GameObject*, 512 >	TransparentShadowCasters ;
	int										TransparentShadowCasterCount ;

	r3dCamera	PrepCam;
	r3dCamera	PrepCamInterm;

	r3dPoint3D		m_MinimapOrigin;
	r3dPoint3D		m_MinimapSize;

#ifndef WO_SERVER
	class BulletShellMngr* m_BulletMngr;
#endif

  public:
	ObjectManager();
	~ObjectManager();

	int	    	Init(int MaxObjects);
	int	    	Destroy();
	
	void		OnResetDevice();

	int			GetNumObjects() const { return NumObjects; }

	void		SetAddObjectEvent (GameObjEvent_fn pEvent);
	void		SetDeleteObjectEvent (GameObjEvent_fn pEvent);

	int	    	AddObject(GameObject *obj);
	int	    	DeleteObject(GameObject *obj, bool call_delete=true);

	void		LinkObject		( GameObject* obj );
	void		UnlinkObject	( GameObject* obj );

	int			SetDrawingOrder(GameObject *obj, int order);

	GameObject*	GetObject(gobjid_t ID);
	GameObject*	GetObject(const char* name);
	GameObject*	GetObjectByHash(uint32_t hash);
	GameObject*	GetFirstObject();
	GameObject*	GetNextObject(const GameObject* obj);
	GameObject*	GetNetworkObject(DWORD netID);

	void		GetObjectsInCube(const r3dBoundBox& box, GameObject**& result, int& objectsCount);

	void		StartFrame();
	void		Update();
	void		EndFrame();

	void		DumpObjects();

	void		PrepareSlicedShadowsInterm( const r3dCamera& Cam, D3DXPLANE (&mainFrustumPlanes)[ 6 ] );
	void		PrepareShadowsInterm( const r3dCamera& Cam );
	void		PrepareTransparentShadowsInterm( const r3dCamera& Cam );
	void		Prepare( const r3dCamera& Cam );

	// to force managed resources into vmem
	void		WarmUp();

	void		IssueOcclusionQueries();

	void		Draw( eRenderStageID DrawState );
	void		DrawIntermediate( eRenderStageID DrawState );

	void		ResetObjFlags();

	void		FlushInstancedMeshes(eRenderStageID DrawState);

	GameObject*	CastRay(const r3dPoint3D& pos, const r3dPoint3D& vRay, float RayLen, CollisionInfo *cInfo, int bboxonly = false);
	GameObject*	CastMeshRay(const r3dPoint3D& pos, const r3dPoint3D& vRay, float RayLen, CollisionInfo *cInfo);
	GameObject*	CastQuickRay(const r3dPoint3D& pos, const r3dPoint3D& vRay, float RayLen, CollisionInfo *cInfo);
	GameObject*	CastBBoxRay(const r3dPoint3D& pos, const r3dPoint3D& vRay, float RayLen, CollisionInfo *cInfo);

	int			SendEvent_to_All(int event, void *data);
	int			SendEvent_to_ObjClass(const char* name, int event, void *data);
	int			SendEvent_to_ObjName(const char* name, int event, void *data);
	void		RecalcIntermObjectMatrices();
	void		RecalcObjectMatrices();

	SceneBox*	GetRoot() const ;

	void		UpdateTransparentShadowCaster( GameObject* obj ) ;

	void		OnGameEnded();
private:
	void		DoPreparedDraw( const r3dCamera& Cam, eRenderStageID DrawState );

	void		AddToTransparentShadowCasters( GameObject* obj ) ;
	void		RemoveFromTransparentShadowCasters( GameObject* obj ) ;
};

// gameworld creation/destroying/getting
extern void GameWorld_Create();
extern void GameWorld_Destroy();
extern ObjectManager& GameWorld();

	// create game object either by ID or by name
	//  flags:
#define OBJ_CREATE_LOCAL	(1<<0)			// object will NOT be added to world
#define OBJ_CREATE_DYNAMIC	(1<<1)			// object will be added to dynamic list - no collision, etc.
#define OBJ_CREATE_SKIP_LOAD	(1<<5)			// loadname invalid.
#define OBJ_CREATE_SKIP_POS	(1<<6)			// Pos invalid - do not call setpos

	//  *data will be used later for passing object data to creation functions
extern	GameObject* 	srv_CreateGameObject(int class_type, const char* load_name, const r3dPoint3D& Pos, long flags = 0, void *data = NULL);
extern	GameObject*	srv_CreateGameObject(const char* class_name, const char* load_name, const r3dPoint3D &Pos, long flags = 0, void *data = NULL);

bool		DoesShadowCullNeedRecalc() ;
void		PrecalculateWorldMatrices(void* Data, size_t ItemStart, size_t ItemCount);

extern void (*gInstance_Compute_Visibility) ( bool shadows, bool directionalSM );

#endif	//__PWAR_OBJMANAG_H
