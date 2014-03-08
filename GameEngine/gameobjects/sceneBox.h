#ifndef __SCENEBOX_H__
#define __SCENEBOX_H__

#include "r3dTreeNode.h"

#define SCENEBOX_MAXOBJECTS 25

class GameObject;
class SceneBox : public r3dTreeNode<SceneBox>
{
public:
	SceneBox();
	SceneBox(const r3dPoint3D& centerPos, const r3dVector& halfSize);
	virtual ~SceneBox();

	void Update();

	void Add( GameObject* obj, bool setVisible );
	void Remove(GameObject* obj);

	void Move(GameObject* obj);

	void PrepareForRender();

	void TraverseTree(const r3dBoundBox& box, GameObject** objects, int& numObjects);
	void TraverseTree(const r3dCamera& Cam, struct draw_s* result, int& numObjects, int isFullyInside = 0);

	void IsVisibleFrom();

	void TraverseAndPrepareForOcclusionQueries( const r3dCamera& Cam, int frameId );
	void DoOcclusionQueries( const r3dCamera& Cam );
	void TraverseDebug(const r3dCamera& Cam, int isFullyInside = 0);

	void MarkDirty();
	void onResetDevice();

	bool SetupBox();

	void AppendDebugBoxes();

	r3dPoint3D GetCenterPos() const { return m_CenterPos; }
	r3dVector  GetHalfSize() const { return m_HalfSize; }

	//temp, test
	LPDIRECT3DQUERY9 query;
	bool visible;
	int lastVisited;
	int lastRendered;

	float distance;
	
	int counter;

	int needQuery;

	int deleteCounter;

	int getLevel() const { return m_Level; }

	bool operator<(const SceneBox& r) const;

//private:
public: // test!!!
	void InternalAdd(GameObject* obj, int id, bool setVisible );
	void UpdateHeuristicScreenSpaceVisibilty();
	float CalculateScreenSpaceArea();

	r3dPoint3D m_CenterPos;
	r3dVector m_HalfSize;

	GameObject* m_Objects[SCENEBOX_MAXOBJECTS];
	uint32_t m_ObjectCount;

	bool m_bDirty;
	bool m_bEmpty;

	int m_ID;
	int m_Level;
};

void InitOcclusionQuerySystem();
void CloseOcclusionQuerySystem();

void AdvanceQueryBalancer();

#endif //__SCENEBOX_H__