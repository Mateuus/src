#include "r3dPCH.h"
#include "r3d.h"

#include "GameObj.h"
#include "sceneBox.h"
#include "ObjManag.h"

#include "Rendering/Deffered/VisibilityGrid.h"

#include "../sf/RenderBuffer.h"

// debug only
int gDisableNodeRecycling = 0;

static const int MAX_OCCLUSION_DELAY = 3;
int OcclusionQueryLoad[ MAX_OCCLUSION_DELAY ];

extern r3dCameraAccelerometer gCameraAccelerometer;

SceneBox::SceneBox()
{
	m_CenterPos = r3dPoint3D(0,0,0);
	m_HalfSize = r3dPoint3D(0,0,0);
	m_ObjectCount = 0;
	m_bDirty = false;
	m_bEmpty = false;
	m_ID = m_Level = -1;
	memset(m_Objects,0,SCENEBOX_MAXOBJECTS*sizeof(GameObject*));
	query = NULL;
	visible = false;
	needQuery = false;
	lastVisited = 0;
	lastRendered = 0;
	counter = 0;
	deleteCounter = 0;
}

SceneBox::SceneBox(const r3dPoint3D& centerPos, const r3dVector& halfSize)
{
	m_CenterPos = centerPos;
	m_HalfSize = halfSize;
	m_ObjectCount = 0;
	m_bDirty = false;
	m_bEmpty = false;
	m_ID = m_Level = -1;
	memset(m_Objects,0,SCENEBOX_MAXOBJECTS*sizeof(GameObject*));
	query = NULL;
	visible = false;
	needQuery = false;
	lastVisited = 0;
	lastRendered = 0;
	counter = 0;
	deleteCounter = 0;
}

SceneBox::~SceneBox()
{
	
}

float gSceneBox_LevelBase = 10.0f;
unsigned int gSceneBox_MinObjCount = 1;
#define LEVEL_COUNT 5
int LevelMask[LEVEL_COUNT] =
{
	0xfff0fff0,
	0xfff8fff8,
	0xfffcfffc,
	0xfffefffe,
	0xffffffff,
};

int LevelFromSize(float radius)
{
	int level = (LEVEL_COUNT - (int)((2*radius)/gSceneBox_LevelBase));
	return R3D_MAX(1,level);
}

int IDFromPos(const r3dPoint3D &pos)
{
	return ((((int)(pos.x/gSceneBox_LevelBase))&0xffff)<<16) | (((int)(pos.z/gSceneBox_LevelBase))&0xffff);
}

void SceneBox::Update()
{
	if( counter ) --counter;

	SceneBox* child = GetChild();
	while(child)
	{
		child->Update();
		child = child->GetSibling();
	}

}

void SceneBox::Add(GameObject* obj, bool setVisible )
{
	int ID = IDFromPos(obj->GetPosition());
	InternalAdd( obj, ID, setVisible );
}

//------------------------------------------------------------------------

void SceneBox::InternalAdd( GameObject* obj, int ID, bool setVisible )
{
	R3DPROFILE_FUNCTION("SceneBox::InternalAdd");

	if(obj->m_SceneBox == this)
		return;

	if (m_Level < (LEVEL_COUNT-1) && LevelFromSize(obj->GetObjectsRadius())>m_Level)
	{
		// get a child id that matches our objects ID (position)
		int ChildID = ID&LevelMask[m_Level+1];
		SceneBox *pChild = GetChild();
		while (pChild)
		{
			if (pChild->m_ID==ChildID && pChild->m_ObjectCount<SCENEBOX_MAXOBJECTS)
				break;
			pChild = pChild->GetSibling();
		}
		if (!pChild)
		{
			unsigned int Count = 1;
			for (uint32_t i=0; i<m_ObjectCount; ++i)
			{
				if (LevelFromSize(m_Objects[i]->GetObjectsRadius())<(m_Level+1))
					continue;
				int ObjID2 = IDFromPos(m_Objects[i]->GetPosition());
				int ChildID2 = ObjID2&LevelMask[m_Level+1];
				if (ChildID==ChildID2)
					++Count;
			}
			if (Count>=gSceneBox_MinObjCount || m_Level==-1 || m_ObjectCount==SCENEBOX_MAXOBJECTS)
			{
				r3dVector center(0,0,0);
				r3dVector halfSize(10000.0f, 10000.0f, 10000.0f);
				pChild = new SceneBox(center, halfSize);
				pChild->m_ID = ChildID;
				pChild->m_Level = m_Level + 1;
				AddChild(pChild);

				for (uint32_t i=0; i<m_ObjectCount;)
				{
					if (LevelFromSize(m_Objects[i]->GetObjectsRadius())<(m_Level+1))
					{
						++i;
						continue;
					}
					int ObjID = IDFromPos(m_Objects[i]->GetPosition());
					int ChildID2 = ObjID&LevelMask[m_Level+1];
					if (ChildID==ChildID2)
					{
						uint32_t oldcount = m_ObjectCount;
						pChild->InternalAdd( m_Objects[i], ObjID, setVisible );
						r3d_assert(m_ObjectCount<oldcount);
						if (m_ObjectCount==oldcount)
							++i;
					}
					else
						++i;
				}
			}
		}
		if (pChild)
		{
			pChild->InternalAdd( obj, ID, setVisible );
			return;
		}
	}

	r3d_assert(m_ObjectCount<SCENEBOX_MAXOBJECTS);
	if(obj->m_SceneBox)
		obj->m_SceneBox->Remove(obj);
	
	m_Objects[m_ObjectCount] = obj;
	obj->m_SceneBox = this;

	if( setVisible )
	{
		visible		= setVisible;
		needQuery	= false;
	}

	++m_ObjectCount;
	MarkDirty();
}

const int MAX_NUM_NODES_DELETE_DEFERRED = 16384; // only 64KB
SceneBox* deleteNodesDeferred[MAX_NUM_NODES_DELETE_DEFERRED]; // cannot delete on the same frame, due to delayed queries
int numDeleteNodesDeferred = 0;
extern bool gDestroyingWorld;
void SceneBox::Remove(GameObject* obj)
{
	R3DPROFILE_FUNCTION("SceneBox::Remove");
	// find index of object
	// todo: we can save index inside of gameobject if this will be slow
	uint32_t index = -1;
	for(uint32_t i=0; i<m_ObjectCount; ++i)
	{
		if(m_Objects[i] == obj)
		{
			index = i;
			break;
		}
	}
	r3d_assert(index<m_ObjectCount);

	--m_ObjectCount;
	m_Objects[index] = m_Objects[m_ObjectCount];
	m_Objects[m_ObjectCount] = NULL;
	obj->m_SceneBox = NULL;

	if(m_ObjectCount)
		MarkDirty(); // recalc bbox

	// recycling
	if (m_ObjectCount==0 && GetChild()==NULL)
	{
		if (GetParent())
		{
			GetParent()->RemoveChild(this);
			if(gDestroyingWorld)
				delete this;
			else
			{
				if(!gDisableNodeRecycling)
				{
					deleteNodesDeferred[numDeleteNodesDeferred++] = this;
					r3d_assert(numDeleteNodesDeferred<MAX_NUM_NODES_DELETE_DEFERRED);
					this->deleteCounter = 3;
				}
			}
		}
	}
}

static volatile LONG InMove ;

void SceneBox::Move(GameObject* obj)
{
	R3DPROFILE_FUNCTION("SceneBox::Move");
	if (GetParent()!=NULL)
	{
		int id = IDFromPos(obj->GetPosition())&LevelMask[m_Level];
		if (m_ID!=id)
		{
			// NOTE : This is temporary protection intended to handle particle update parallelism
			for( ; InterlockedExchange( &InMove, 1 ) == 1 ; );

			SceneBox *pRoot = GetParent();
			while (pRoot->GetParent())
				pRoot = pRoot->GetParent();
			pRoot->Add( obj, visible );

			InMove = 0 ;

			return;
		}
	}
	// parent node, mark dirty to re adjust size
	MarkDirty();
}

void SceneBox::PrepareForRender()
{
	for(int i=0; i<numDeleteNodesDeferred; ++i)
	{
		if((--deleteNodesDeferred[i]->deleteCounter)<0)
		{
			delete deleteNodesDeferred[i];
			--numDeleteNodesDeferred;
			deleteNodesDeferred[i] = deleteNodesDeferred[numDeleteNodesDeferred];
		}
	}
	SetupBox();
}

void SceneBox::MarkDirty()
{
	m_bDirty = true;
	if (GetParent())
		GetParent()->MarkDirty();
}

bool SceneBox::SetupBox()
{
	if (!m_bDirty)
	{
		if(m_bEmpty)
			return false;
		else
			return true;
	}
	m_bDirty = false;
	R3DPROFILE_FUNCTION("SceneBox::SetupBox");

	if (!GetChild())
	{
		if (!GetParent())
			return true;
		SceneBox *pParent = GetParent();
		if (m_ObjectCount<gSceneBox_MinObjCount && m_Level>0 && (pParent->m_ObjectCount+m_ObjectCount)<=SCENEBOX_MAXOBJECTS)
		{
			GetParent()->RemoveChild(this);
			if (m_ObjectCount>0)
			{
				while (m_ObjectCount)
				{
					r3d_assert(m_Objects[0]);
					int ID = IDFromPos(m_Objects[0]->GetPosition());
					pParent->InternalAdd( m_Objects[0], ID, false );
				}
				r3d_assert(m_ObjectCount==0);
			}
			if(gDestroyingWorld)
				delete this;
			else
			{
				if(!gDisableNodeRecycling)
				{
					deleteNodesDeferred[numDeleteNodesDeferred++] = this;
					r3d_assert(numDeleteNodesDeferred<MAX_NUM_NODES_DELETE_DEFERRED);
					this->deleteCounter = 3;
				}
			}
			return false;
		}
	}

	bool bUseMinMax = false;
	r3dVector minPos(FLT_MAX, FLT_MAX, FLT_MAX);
	r3dVector maxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// go through child boxes
	for (SceneBox *pChild = GetChild(), *pNext; pChild; pChild = pNext)
	{
		pNext = pChild->GetSibling();
		if (!pChild->SetupBox())
			continue;
		r3dVector objMinPos = pChild->m_CenterPos - pChild->m_HalfSize;
		r3dVector objMaxPos = pChild->m_CenterPos + pChild->m_HalfSize;
		minPos = ElementalMinimum(minPos,objMinPos);
		maxPos = ElementalMaximum(maxPos,objMaxPos);
		bUseMinMax = true;
	}

	// go through our objects
	for (uint32_t i=0; i<m_ObjectCount; i++)
	{
		r3d_assert(m_Objects[i]);

		if( m_Objects[i]->ObjFlags & OBJFLAG_SkipOcclusionCheck && 
			!( m_Objects[i]->ObjFlags & OBJFLAG_ForceSceneBoxBBox ) )
			continue;

		const r3dBoundBox& bbox = m_Objects[i]->GetBBoxWorld() ;
		const r3dBoundBox& bbox_local = m_Objects[i]->GetBBoxLocal() ;

		// bbox marked as invalid(not updated yet) - ignore
		if( bbox_local.Size.x < 0 )
			continue ;

		r3dVector objMinPos = bbox.Center() - bbox.Size*0.5f;
		r3dVector objMaxPos = bbox.Center() + bbox.Size*0.5f;
		minPos = ElementalMinimum(minPos,objMinPos);
		maxPos = ElementalMaximum(maxPos,objMaxPos);
		bUseMinMax = true;
	}

	if(!bUseMinMax)
	{
		m_bEmpty = true;
		return false;
	}
	m_bEmpty = false;

	m_CenterPos = (minPos + maxPos) * 0.5f;
	m_HalfSize = (maxPos - minPos) * 0.5f;

	return true;
}

r3dTL::TArray< r3dPoint3D > debug_SceneBox;
r3dTL::TArray< r3dColor24 > debug_SceneBoxColors;

r3dTL::TFixedArray< r3dColor, LEVEL_COUNT + 1 > debugColorLevels ;

struct FillDebugColors
{
	FillDebugColors()
	{
		int idx = 0 ;

		debugColorLevels[ idx ++ ] = r3dColor24(0, 0, 0) ;
		debugColorLevels[ idx ++ ] = r3dColor24(255, 255, 0) ;
		debugColorLevels[ idx ++ ] = r3dColor24(255, 255, 255) ;
		debugColorLevels[ idx ++ ] = r3dColor24(255, 0, 0) ;
		debugColorLevels[ idx ++ ] = r3dColor24(0, 0, 255) ;
		debugColorLevels[ idx ++ ] = r3dColor24(0, 255, 0) ;
	}
} static gFillDebugColors ;

void SceneBox::AppendDebugBoxes()
{
	r3dPoint3D v0( m_CenterPos + m_HalfSize );
	r3dPoint3D v1( m_CenterPos - m_HalfSize );

	debug_SceneBox.PushBack( r3dPoint3D( v0.x, v0.y, v0.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v1.x, v0.y, v0.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v0.x, v1.y, v0.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v1.x, v1.y, v0.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v0.x, v0.y, v1.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v1.x, v0.y, v1.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v0.x, v1.y, v1.z ) );
	debug_SceneBox.PushBack( r3dPoint3D( v1.x, v1.y, v1.z ) );

	debug_SceneBoxColors.PushBack( debugColorLevels[ m_Level + 1 ] );

	SceneBox* child = GetChild();

	while(child)
	{
		child->AppendDebugBoxes();

		child = child->GetSibling();
	}
}

uint8_t getShadowSliceBit(GameObject* userObject, const r3dCamera& Cam );

void SceneBox::TraverseDebug(const r3dCamera& Cam, int isFullyInside)
{
	R3DPROFILE_FUNCTION("SceneBox::TraverseDebug");

	if(m_ObjectCount>0)
	{
		r3dBoundBox mybbox; 
		mybbox.Org = m_CenterPos - m_HalfSize;
		mybbox.Size = m_HalfSize*2;
		r3dDrawBoundBox(mybbox, Cam, r3dColor24::green/*debugColorLevels[m_Level]*/, 0.2f);
	}

	SceneBox* child = GetChild();
	while(child)
	{
		r3dBoundBox bbox; 
		bbox.Org = child->m_CenterPos - child->m_HalfSize;
		bbox.Size = child->m_HalfSize*2;
		int frustum_check = r3dRenderer->IsBoxInsideFrustum(bbox);

		//If clipping is ok, render child
		if (frustum_check != 0)
			child->TraverseDebug(Cam, frustum_check);
		child = child->GetSibling();
	}
}

void SceneBox::TraverseTree(const r3dCamera& Cam, struct draw_s* result, int& numObjects, int isFullyInside)
{
	R3DPROFILE_FUNCTION("SceneBox::TraverseTree");
	for (uint32_t i=0; i<m_ObjectCount; ++i)
	{
		if(!m_Objects[i]->isActive() || m_Objects[i]->ObjFlags & OBJFLAG_SkipDraw || m_Objects[i]->ObjFlags & OBJFLAG_JustCreated || m_Objects[i]->ObjFlags & OBJFLAG_Removed || !m_Objects[i]->isDetailedVisible())
			continue;

		if(!(m_Objects[i]->ObjFlags & (OBJFLAG_SkipOcclusionCheck | OBJFLAG_AlwaysDraw)))
			if(isFullyInside==2) // intersects with frustum
				if(!r3dRenderer->IsSphereInsideFrustum(m_Objects[i]->GetPosition(), m_Objects[i]->GetObjectsRadius()))
					continue;

		r3d_assert(numObjects < OBJECTMANAGER_MAXOBJECTS);
		result[numObjects].obj = m_Objects[i];
		result[numObjects].dist = (m_Objects[i]->GetPosition() - Cam).LengthSq();
		result[numObjects].shadow_slice = getShadowSliceBit( m_Objects[i], Cam );
		++numObjects;
	}

	SceneBox* child = GetChild();
	while(child)
	{
		r3dBoundBox bbox; 
		bbox.Org = child->m_CenterPos - child->m_HalfSize;
		bbox.Size = child->m_HalfSize*2;
		int frustum_check = r3dRenderer->IsBoxInsideFrustum(bbox);
		if (frustum_check != 0)
			child->TraverseTree(Cam, result, numObjects, frustum_check);
		child = child->GetSibling();
	}
}

void SceneBox::TraverseTree(const r3dBoundBox& box, GameObject** objects, int& numObjects)
{
	for (uint32_t i=0; i<m_ObjectCount; ++i)
	{
		if(!m_Objects[i]->isActive() || m_Objects[i]->ObjFlags & OBJFLAG_SkipDraw || m_Objects[i]->ObjFlags & OBJFLAG_JustCreated || m_Objects[i]->ObjFlags & OBJFLAG_Removed || !m_Objects[i]->isDetailedVisible())
			continue;

		r3d_assert(numObjects < OBJECTMANAGER_MAXOBJECTS);
		objects[numObjects] = m_Objects[i];
		++numObjects;
	}

	SceneBox* child = GetChild();
	while(child)
	{
		r3dBoundBox bbox; 
		bbox.Org = child->m_CenterPos - child->m_HalfSize;
		bbox.Size = child->m_HalfSize*2;

		bool res = box.Intersect(bbox) || box.ContainsBox(bbox);
		if (res != 0)
			child->TraverseTree(box, objects, numObjects);

		child = child->GetSibling();
	}
}

class mycomparison
{
public:
	bool operator() (const SceneBox* lhs, const SceneBox* rhs) const
	{
		return (lhs<rhs); 
	}
};

SceneBox* TraversalList[2048];
int numTraversalList = 0;

typedef std::list<SceneBox*> SceneBoxList ;
SceneBoxList	PrevFrameQueryList; // non critical queries from prev.frame
int				PrevFrameQueryStamp ;

extern r3dCamera gCam;
bool SceneBox::operator<(const SceneBox& r) const
{
	//float dist1 = (m_CenterPos-gCam).LengthSq();
	//float dist2 = (r.m_CenterPos-gCam).LengthSq();
	return distance < r.distance; //(dist1 < dist2);
}

bool isQueryResultAvailable(const SceneBox& node)
{
	R3DPROFILE_FUNCTION("isQueryResultAvailable");

	if( node.query )
	{
		int temp;

		HRESULT hr = D3DERR_DEVICELOST ;

		hr = node.query->GetData( &temp, sizeof(DWORD), D3DGETDATA_FLUSH );

		switch( hr )
		{
		case S_OK :
			return true ;

		case S_FALSE :
			return false ;

		case D3DERR_DEVICELOST :
			r3dRenderer->SetDeviceLost() ;
			return false ;

		default:
			r3dError( "node.query->GetData call returned error!" );
			return false ;
		}
	}
	return false;
}

int getQueryResult(const SceneBox& node)
{
	R3DPROFILE_FUNCTION("getQueryResult");

	const int DEFAULT_RESULT = 8192 ;

	if( node.query )
	{
		if( r3dRenderer->IsDeviceLost() )
			return DEFAULT_RESULT;

		int result ;

		// ptumik: on ATI card after device lost it got stuck in for loop returning S_FALSE forever, so I added simple timer to prevent that from happening
		float curTime = r3dGetTime();

		for( ; ; )
		{
			int ret = node.query->GetData(&result, sizeof(DWORD), D3DGETDATA_FLUSH);

			switch( ret )
			{
			case S_OK:
				return result ;

			case S_FALSE:
				{
					if((r3dGetTime() - curTime) > 0.3f) 
						return DEFAULT_RESULT;
					
					continue ;
				}
			case D3DERR_DEVICELOST:
				r3dRenderer->SetDeviceLost();
				return DEFAULT_RESULT ;
			default:
				r3dError( "node.query->GetData call returned error!" );
				return DEFAULT_RESULT ;
			}
		}
	}

	return DEFAULT_RESULT ;
}

void pullUpVisibility(SceneBox& node)
{
	SceneBox* pNode = &node;
	while(pNode && !pNode->visible)
	{
		pNode->visible = true;
		pNode = pNode->GetParent();
	}
}

bool isLeaf(const SceneBox& node)
{
	return /*node.getLevel()==4 ||*/ node.m_ObjectCount>0;
}

extern RenderArray	g_render_arrays[ rsCount ];
extern draw_s		draw[OBJECTMANAGER_MAXOBJECTS];
extern int			n_draw;
extern int gRenderFrameCounter;

int draw_Comparator(void const *b1, void const *b2);

int nodeTraverseID =0;


void renderNode( SceneBox& node, const r3dCamera& Cam )
{
	if(node.lastRendered == gRenderFrameCounter)
		return;
	node.lastRendered = gRenderFrameCounter;

	if(node.m_ObjectCount == 0)
		return;

//	int prev_draw = n_draw;
	for (uint32_t i=0; i<node.m_ObjectCount; ++i)
	{
		if(!node.m_Objects[i]->isActive() || node.m_Objects[i]->ObjFlags & OBJFLAG_SkipOcclusionCheck || node.m_Objects[i]->ObjFlags & OBJFLAG_SkipDraw || node.m_Objects[i]->ObjFlags & OBJFLAG_JustCreated || node.m_Objects[i]->ObjFlags & OBJFLAG_Removed || !node.m_Objects[i]->isDetailedVisible())
			continue;

		r3d_assert(n_draw < OBJECTMANAGER_MAXOBJECTS);
		draw[n_draw].obj = node.m_Objects[i];
		draw[n_draw].dist = (node.m_Objects[i]->GetPosition() - Cam).LengthSq();
		draw[n_draw].shadow_slice = getShadowSliceBit( node.m_Objects[i], Cam );
		++n_draw;

	}

}

static int curVisGridCellX ;
static int curVisGridCellY ;
static int curVisGridCellZ ;

static int enableVisGrid ;

void traverseNode(SceneBox& node)
{
	node.distance = (gCam-node.m_CenterPos).Length();

	if(node.m_ObjectCount>0)
	{
		TraversalList[numTraversalList++] = &node;		
	}
	r3d_assert(numTraversalList < 2048);
	SceneBox* child = node.GetChild();
	while(child)
	{
		r3dBoundBox bbox; 
		bbox.Org = child->m_CenterPos - child->m_HalfSize;
		bbox.Size = child->m_HalfSize*2;
		int frustum_check = r3dRenderer->IsBoxInsideFrustum(bbox);

		int visgrid_check = 1 ; 
		
#ifndef WO_SERVER
#ifndef FINAL_BUILD
		if( enableVisGrid )
		{
			visgrid_check = g_pVisibilityGrid->IsBBoxVisibleFrom( curVisGridCellX, curVisGridCellY, curVisGridCellZ, bbox ) ; 
		}
#endif
#endif

		if ( frustum_check && visgrid_check )
		{
			traverseNode(*child);
		}
		child = child->GetSibling();
	}
}

namespace
{
	bool isInitedOQ = false;

	int vertexShaderID;
	int pixelShaderID;

	const uint32_t VERTS_PER_BOX = 8;
	const uint32_t INDICES_PER_BOX = 36;

	uint16_t indexBufferData[ SCENEBOX_MAXOBJECTS * INDICES_PER_BOX ];
	r3dPoint3D vertexBufferData[ SCENEBOX_MAXOBJECTS * VERTS_PER_BOX ];

	r3dIndexBuffer * oqIndexBuffer;
	r3dVertexBuffer * oqVertexBuffer;

	uint32_t oqVertexBufferOffset;

	
}

uint16_t* g_temproraritetness;

void InitOcclusionQuerySystem()
{
	// it is also used in some debug routines
	g_temproraritetness = indexBufferData;

	isInitedOQ = true;
	
	vertexShaderID = r3dRenderer->GetVertexShaderIdx( "VS_OCCLUSION_QUERY" );
	pixelShaderID = r3dRenderer->GetPixelShaderIdx( "PS_OCCLUSION_QUERY" );

	for(int i=0; i<SCENEBOX_MAXOBJECTS; ++i)
	{
		int j = 0;

		int v = i * VERTS_PER_BOX;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 0;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 1;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 2;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 3;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 2;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 1;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 4;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 0;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 1;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 0;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 4;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 7;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 4;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 0;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 2;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 0;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 1;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 3;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 7;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 3;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 5;

		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 2;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 3;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 7;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 6;
		indexBufferData[ i * INDICES_PER_BOX + j ++ ] = v + 3;

		r3d_assert( j == INDICES_PER_BOX );
	}

	oqIndexBuffer = new r3dIndexBuffer( sizeof( indexBufferData ) / sizeof ( indexBufferData[ 0 ] ) );

	memcpy( oqIndexBuffer->Lock(), indexBufferData, sizeof indexBufferData );

	oqIndexBuffer->Unlock();

	oqVertexBuffer = new r3dVertexBuffer( SCENEBOX_MAXOBJECTS * VERTS_PER_BOX * 64, sizeof ( r3dPoint3D ), 0, true );

	// create decl
	R3D_POS_VERTEX::getDecl();
}

void CloseOcclusionQuerySystem()
{
	delete oqVertexBuffer;
	delete oqIndexBuffer;
}

void buildBBox(r3dPoint3D* buffer, const r3dPoint3D& center, const r3dPoint3D& size )
{
	for(int i=0, e = VERTS_PER_BOX; i<e; ++i)
		buffer[i]  = center;

	float hw = size.x * 0.5f;
	float hh = size.y * 0.5f;
	float hd = size.z * 0.5f;

	// face 0
	buffer[0]   += r3dPoint3D( -hw, -hh, -hd ); 
	buffer[1]   += r3dPoint3D( -hw, +hh, -hd ); 
	buffer[2]   += r3dPoint3D( +hw, -hh, -hd ); 
	buffer[3]   += r3dPoint3D( +hw, +hh, -hd ); 

	// face 1
	buffer[4]   += r3dPoint3D( -hw, -hh, +hd ); 
	buffer[5]   += r3dPoint3D( -hw, +hh, +hd );
	buffer[6]   += r3dPoint3D( +hw, -hh, +hd ); 
	buffer[7]   += r3dPoint3D( +hw, +hh, +hd );
}

void buildOBBox( r3dPoint3D* buffer, GameObject* object )
{
	R3DPROFILE_FUNCTION("buildOBBox");

	const D3DXMATRIX& trans = object->GetTransformMatrix();

	r3dPoint3D XA		= r3dPoint3D( trans._11, trans._12, trans._13 );
	r3dPoint3D YA		= r3dPoint3D( trans._21, trans._22, trans._23 );
	r3dPoint3D ZA		= r3dPoint3D( trans._31, trans._32, trans._33 );

	const r3dBoundBox& bbox = object->GetBBoxLocal();

	r3dPoint3D dsp = bbox.Org + bbox.Size * 0.5f;

	D3DXVECTOR4 tdsp( dsp.x, dsp.y, dsp.z, 1.f );

	D3DXVec4Transform( &tdsp, &tdsp, &trans );

	r3dPoint3D Pos( tdsp.x, tdsp.y, tdsp.z );

	float coef = ( 1.f + r_occ_bbox_expand_coef->GetFloat() ) * 0.5f;

	const float EXTRA_DELTA = r_occ_bbox_add_coef->GetFloat();

	// need extra scale for it to stay on top of the object with "guarantee"
	float hw = bbox.Size.x * coef + EXTRA_DELTA;
	float hh = bbox.Size.y * coef + EXTRA_DELTA;
	float hd = bbox.Size.z * coef + EXTRA_DELTA;

	buffer[0]   = Pos - hw * XA - hh * YA - hd * ZA; 
	buffer[1]   = Pos - hw * XA + hh * YA - hd * ZA; 
	buffer[2]   = Pos + hw * XA - hh * YA - hd * ZA; 
	buffer[3]   = Pos + hw * XA + hh * YA - hd * ZA; 

	buffer[4]   = Pos - hw * XA - hh * YA + hd * ZA; 
	buffer[5]   = Pos - hw * XA + hh * YA + hd * ZA;
	buffer[6]   = Pos + hw * XA - hh * YA + hd * ZA; 
	buffer[7]   = Pos + hw * XA + hh * YA + hd * ZA;
}

extern int	g_OcclusionQueryCounter;
extern IDirect3DQuery9* g_pOcclusionQueries[1000];

void startConsequtiveOcclusionQueries()
{
	r3d_assert( isInitedOQ );

	r3dRenderer->Flush();

	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE1, 0);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE2, 0);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE3, 0);

	oqVertexBuffer->Set( 0 );
	oqIndexBuffer->Set();

	r3dRenderer->SetRenderingMode(R3D_BLEND_ZC | R3D_BLEND_NOALPHA | R3D_BLEND_PUSH );
	r3dRenderer->SetVertexShader(vertexShaderID);
	r3dRenderer->SetPixelShader(pixelShaderID);

	d3dc._SetDecl( R3D_POS_VERTEX::getDecl() );

	// temp, just do not render bbox that are inside of camera
	r3dRenderer->SetCullMode( D3DCULL_CCW );
	r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILENABLE, FALSE );

	D3DXMATRIX matWVP;
	D3DXMatrixTranspose( &matWVP, &r3dRenderer->ViewProjMatrix );
	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&matWVP,  4 );
}

void endConsequitiveOcclusionQueries()
{
	r3dRenderer->SetRenderingMode( R3D_BLEND_POP );

	DWORD enableAllColors =	D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | 
							D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA;

	r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILENABLE, TRUE );

	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE, enableAllColors );
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE1, enableAllColors );
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE2, enableAllColors );
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_COLORWRITEENABLE3, enableAllColors );
	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();	
}

void makeQueryD3DCalls( SceneBox& node, uint32_t numVertices )
{
	R3DPROFILE_FUNCTION("makeQueryD3DCalls");

	D3D_V( node.query->Issue( D3DISSUE_BEGIN ) ) ;
	r3dRenderer->DrawIndexed( D3DPT_TRIANGLELIST, oqVertexBufferOffset, 0, numVertices, 0, numVertices / VERTS_PER_BOX * INDICES_PER_BOX / 3 );
	D3D_V( node.query->Issue( D3DISSUE_END) ); 
}

void issueConsequtiveOcclusionQuery( SceneBox& node, const r3dCamera& Cam, float nearPlaneDoubleRadius )
{
	R3DPROFILE_FUNCTION("issueConsequtiveOcclusionQuery");

	r3d_assert( node.m_ObjectCount <= SCENEBOX_MAXOBJECTS );

	if( node.m_ObjectCount ) // check actual geometry
	{
#ifndef WO_SERVER
		extern int g_UseOBBox;
#else 
		int g_UseOBBox = 0;
#endif

		int numVertices = 0;

		if( g_UseOBBox )
		{
			for( unsigned int i=0; i<node.m_ObjectCount; ++i, numVertices += VERTS_PER_BOX )
			{
				r3dBoundBox BBox = node.m_Objects[ i ]->GetBBoxWorld();

				BBox.GrowByCoef( r_occ_bbox_expand_coef->GetFloat() );
				BBox.Grow( nearPlaneDoubleRadius );

				if( BBox.ContainsPoint( Cam ) )
				{
					node.visible = true;
					node.needQuery = false;
					return;
				}

				buildOBBox( &vertexBufferData[numVertices], node.m_Objects[i] );
			}
		}
		else
		{
			for( unsigned int i=0; i<node.m_ObjectCount; ++i, numVertices += VERTS_PER_BOX )
				buildBBox(&vertexBufferData[numVertices], node.m_Objects[i]->GetBBoxWorld().Center(), node.m_Objects[i]->GetBBoxWorld().Size);
		}

		r3d_assert( numVertices < oqVertexBuffer->GetItemCount() );

		if( oqVertexBufferOffset + numVertices > (uint32_t)oqVertexBuffer->GetItemCount() )
		{
			oqVertexBufferOffset = 0;
		}

		void* locked = oqVertexBuffer->Lock( oqVertexBufferOffset, numVertices );
		memcpy( locked, vertexBufferData, sizeof vertexBufferData[ 0 ] * numVertices );
		oqVertexBuffer->Unlock();

		node.query = g_pOcclusionQueries[g_OcclusionQueryCounter++];
		g_OcclusionQueryCounter = g_OcclusionQueryCounter%1000;

		makeQueryD3DCalls( node, numVertices );

		oqVertexBufferOffset += numVertices;

		r3dRenderer->Stats.AddNumOcclusionQueries( 1 ) ;


	}
	else 
	{
		r3d_assert(false); // shouldn't happen!
	}
}

//------------------------------------------------------------------------

int gVisibilityTreshold = 0;

static int countRef;

struct SceneBoxNodeCompare
{
	bool operator() ( SceneBox* b1, SceneBox* b2 )
	{
		float dist1 = b1 ->distance ;
		float dist2 = b2 ->distance ;

		return  dist2 < dist1 ;
	}
};

void
SceneBox::TraverseAndPrepareForOcclusionQueries( const r3dCamera& Cam, int frameId )
{
	R3DPROFILE_FUNCTION("SceneBox::TraverseAndPrepareOQ");

#ifndef WO_SERVER
#ifndef FINAL_BUILD
	if( g_pVisibilityGrid && r_scenebox_visgrid->GetInt() )
	{
		enableVisGrid = 1 ;
		g_pVisibilityGrid->GetCellCoords( gCam, &curVisGridCellX, &curVisGridCellY, &curVisGridCellZ ) ;
	}
	else
	{
		enableVisGrid = 0 ;
	}
#endif
#endif

	// check non critical queries from prev.frame
	// are these queries really from previous frame?
	if( frameId == PrevFrameQueryStamp + 1 )
	{
		R3DPROFILE_START("PrevFrameQueries");

		while(!PrevFrameQueryList.empty())
		{
			R3DPROFILE_START("numQueries");
			SceneBox& node = *PrevFrameQueryList.front(); 
			PrevFrameQueryList.pop_front();

			int visiblePixels = node.needQuery ? getQueryResult(node) : gVisibilityTreshold + 1;

			node.needQuery = false;

			if(visiblePixels > gVisibilityTreshold)
			{
				pullUpVisibility( node );
			}
			else
			{
				node.visible = false;
			}
			R3DPROFILE_END("numQueries");
		}
		R3DPROFILE_END("PrevFrameQueries");
	}
	else
	{
		// pointers too old - objects could be deleted.
		PrevFrameQueryList.clear() ;
#ifndef FINAL_BUILD
		r3dOutToLog( "Warning: skipped frames between GameWorld().Update() and SceneBox::TraverseAndPrepareForOcclusionQueries\n" ) ;
#endif
	}


	PrevFrameQueryStamp = frameId ;

	countRef ++;

	traverseNode(*this);

	// this sort works faster than qsort here
	std::sort( TraversalList, TraversalList + numTraversalList, SceneBoxNodeCompare() );

	nodeTraverseID = 0;

	int numTL = numTraversalList;

	// part 2: hierarchical traversal
	R3DPROFILE_START("part2");

	while(numTL)
	{
		SceneBox& node = *TraversalList[numTL-1]; --numTL;

		node.UpdateHeuristicScreenSpaceVisibilty();

		// identify previously visible nodes
		bool wasVisible = node.visible;

		// identify nodes that we cannot skip queries for
		bool leafOrWasInvisible = !wasVisible || isLeaf(node);

		if(node.counter == 0)
		{
			node.lastVisited	= gRenderFrameCounter;

			// forget about visible stuff for longer periods of time
			if( r_allow_delayed_queries->GetInt() )
			{
				int minIdx = 0;
				int minVal = OcclusionQueryLoad[ 0 ];

				for( int i = 1, e = R3D_ARRAYSIZE( OcclusionQueryLoad ); i < e; i ++ )
				{
					int curVal = OcclusionQueryLoad[ i ];

					if( curVal < minVal )
					{
						minIdx = i;
						minVal = curVal;
					}						
				}

				OcclusionQueryLoad[ minIdx ] ++ ;
				node.counter = 1 + minIdx ;
			}
			else
				node.counter		= 1;
		}
		else
		{
			leafOrWasInvisible = false; // do not issue query
		}

		// skip testing previously visible interior nodes
		if ( leafOrWasInvisible ) 
		{
			node.needQuery = true;
			PrevFrameQueryList.push_back(&node);
		}

		// always traverse a node if it was visible
		if ( wasVisible )
		{
			renderNode( node, Cam );
		}
	}

	R3DPROFILE_END("part2");
}

//------------------------------------------------------------------------

void
SceneBox::DoOcclusionQueries( const r3dCamera& Cam )
{
	R3DPROFILE_FUNCTION("SceneBox::DoOcclusionQueries");

	startConsequtiveOcclusionQueries();

	float a = Cam.NearClip * tanf( 0.5f * Cam.FOV * R3D_PI / 180.f);
	float b = a * Cam.Aspect;

	float nearPlaneDoubleRadius = 2.2f * sqrtf( a * a + b * b + Cam.NearClip * Cam.NearClip );

	// complete evil
	nearPlaneDoubleRadius += r_occ_bbox_add_coef->GetFloat() * 2.f;

	while( numTraversalList )
	{
		SceneBox& node = *TraversalList[numTraversalList-1]; --numTraversalList;

		// skip testing previously visible interior nodes
		if ( node.needQuery ) 
		{
			issueConsequtiveOcclusionQuery( node, Cam, nearPlaneDoubleRadius );
		}
	}

	endConsequitiveOcclusionQueries();
}

void SceneBox::onResetDevice()
{
	if(GetParent()==0)
		PrevFrameQueryList.clear();
	query = 0;
	SceneBox* child = GetChild();
	while(child)
	{
		child->onResetDevice();
		child = child->GetSibling();
	}
}

//////////////////////////////////////////////////////////////////////////

float SceneBox::CalculateScreenSpaceArea()
{
	const D3DXMATRIXA16 &vm = r3dRenderer->ViewMatrix;
	const D3DXMATRIXA16 &pm = r3dRenderer->ProjMatrix;

	r3dVector viewSpaceCenterPos(m_CenterPos);
	D3DXVECTOR3 *pv = reinterpret_cast<D3DXVECTOR3*>(&viewSpaceCenterPos);
	D3DXVec3TransformCoord(pv, pv, &vm);

	float maxExtent = R3D_MAX(m_HalfSize.x, R3D_MAX(m_HalfSize.y, m_HalfSize.z));
	r3dVector extent(maxExtent, maxExtent, maxExtent);

	//	Get 2 opposite points on bounding sphere
	r3dVector p1 = viewSpaceCenterPos + maxExtent;
	r3dVector p2 = viewSpaceCenterPos - maxExtent;
	D3DXVECTOR3 *pp1 = reinterpret_cast<D3DXVECTOR3*>(&p1);
	D3DXVECTOR3 *pp2 = reinterpret_cast<D3DXVECTOR3*>(&p2);

	//	Calculate bounding sphere diameter in NDC
	D3DXVec3TransformCoord(pp1, pp1, &pm);
	D3DXVec3TransformCoord(pp2, pp2, &pm);

	//	We don't care about z, but it will prevent us from correct screen space vector length calculation, so zero it out
	p1.z = 0;
	p2.z = 0;

	//	Clamp to screen bounds
	p1.x = r3dTL::Clamp(p1.x, -1.0f, 1.0f);
	p1.y = r3dTL::Clamp(p1.y, -1.0f, 1.0f);
	p2.x = r3dTL::Clamp(p2.x, -1.0f, 1.0f);
	p2.y = r3dTL::Clamp(p2.y, -1.0f, 1.0f);

	//	Calc radius of screen space bounding sphere
	float r = (p1 - p2).Length() / 2;

	//	Return circle area
	return D3DX_PI * r * r;
}

//////////////////////////////////////////////////////////////////////////

void SceneBox::UpdateHeuristicScreenSpaceVisibilty()
{
	if (!r_occlusion_vis_helper->GetBool())
		return;

	if (!isLeaf(*this))
		return;

	//	If we are already visible, skip area calculations
	if (visible)
		return;

	float accA = gCameraAccelerometer.GetMaxAngularAcceleration();
	float accL = gCameraAccelerometer.GetMaxLinearAcceleration();
	if (accA < 2.0f && accL < 2.0f)
		return;

	//	Update visibility only for big objects
	float r = CalculateScreenSpaceArea();

	visible = r > 0.1f;
}

//////////////////////////////////////////////////////////////////////////


void AdvanceQueryBalancer()
{
	int i = 0, e = R3D_ARRAYSIZE(OcclusionQueryLoad) - 1;
	for( ; i < e; i ++ )
	{
		OcclusionQueryLoad[ i ] = OcclusionQueryLoad[ i + 1 ];
	}

	OcclusionQueryLoad[ i ] = 0;
}