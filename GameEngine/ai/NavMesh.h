//=========================================================================
//	Module: NavData.h
//	Copyright (C) 2012.
//=========================================================================

#pragma once
#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "RecastAndDetour/Recast.h"
#include "RecastAndDetour/DetourNavMesh.h"
#include "RecastAndDetour/DetourNavMeshQuery.h"

//////////////////////////////////////////////////////////////////////////

class NavDataDebugVisualizer;
class r3dNavConvexRegionsManager;

class r3dNavigationMesh
{
public:
	r3dNavigationMesh();
	~r3dNavigationMesh();

	/**	Build navigation mesh for currently loaded level. */
	bool BuildForCurrentLevel();

	/**	Visualization type. */
	enum DebugVisualizationType
	{
		//	Don't show anything
		dvtNone = 0,
		//	Render navigation mesh
		dvtMesh = 1,
		//	Render voxel grid
		dvtVoxels = 2,
		// Render regions border
		dvtRegions = 4
	};

	/**	Visualize navigation mesh. */
	void DebugDraw();

	/**	Compose visualization mask using DebugVisualizationType flags */
	void VisualizeNavMesh(int visFalgs);

	r3dNavConvexRegionsManager & GetRegionsMgr();

	/**	Save navigation data into a specified folder. */
	bool Save(const r3dString &levelHomeDir);

	/**	Save navigation data from a specified folder. */
	bool Load(const r3dString &levelHomeDir);

	/**	Recast build configuration. */
	rcConfig buildConfig;

private:
	friend class r3dNavMeshActorsManager;

	rcPolyMesh *polyMesh;
	rcPolyMeshDetail *polyMeshDetail;
	dtNavMesh *navMesh;
	dtNavMeshQuery *navQuery;
	rcContext ctx;

	NavDataDebugVisualizer *debugDraw;
	int visualizationFlags;

	r3dNavConvexRegionsManager *regionsMgr;

	/**	Submit navmesh triangles form different physics body. */
	bool SubmitNavmeshTriangles(rcHeightfield &hf, PxHeightFieldGeometry &g);
	bool SubmitNavmeshTriangles(rcHeightfield &hf, PxTriangleMeshGeometry &g, const PxTransform &globalPose);

	/**	Create detour navigation data. */
	bool CreateDetourNavData();

	/**	Clear all internal navmesh data. */
	void FreeInternalData();

	/**	Reject all triangles that lie outside of user defined nav region. */
	PxU32 FilterOutsideTriangles(const PxVec3 *verices, int *indices, PxU32 numIndices);

	/**	Save poly mesh into file. */
	bool SavePolyMesh(const r3dString &filePath);

	/**	Save poly mesh detail into file. */
	bool SavePolyMeshDetail(const r3dString &filePath);

	/**	Load poly mesh from file. */
	bool LoadPolyMesh(const r3dString &filePath);

	/**	Load poly mesh detail from file. */
	bool LoadPolyMeshDetail(const r3dString &filePath);

	/**	Save build parameters. */
	bool SaveBuildParams(const r3dString &filePath);

	/**	Load build parameters from xml. */
	bool LoadBuildParams(const r3dString &filePath);

	void InitBuildConfig();
	void CalcSceneBBox(r3dPoint3D &minP, r3dPoint3D &maxP);
};

extern r3dNavigationMesh gNavMesh;

//////////////////////////////////////////////////////////////////////////

class r3dNavConvexRegionsManager
{
	typedef std::vector<r3dPoint2D> Convex;
	typedef std::vector<Convex> Regions;

	Regions regions;
	uint32_t hightlightedRegion;

	bool IsPointInsideConvex(const r3dPoint2D &p, const Convex &polygon) const;
	void CalcBBoxForConvex(r3dPoint2D &minPt, r3dPoint2D &maxPt, const Convex &polygon) const;

public:
	r3dNavConvexRegionsManager();
	void CreateNewRegion();
	void DeleteRegion(uint32_t idx);
	uint32_t GetNumRegions() const { return regions.size(); }
	void AddPointToRegion(const r3dPoint3D &pt, uint32_t idx);
	void VisualizeRegions();
	void HightlightRegion(uint32_t idx) { hightlightedRegion = idx; }
	bool IsPointInsideRegions(const r3dPoint2D &p) const;
	void CalcBBox(r3dPoint2D &minPt, r3dPoint2D &maxPt) const;

	bool Save(pugi::xml_document& xmlDoc);
	bool Load(const pugi::xml_document &xmlDoc);
};

#endif // ENABLE_RECAST_NAVIGATION