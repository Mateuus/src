//=========================================================================
//	Module: NavMesh.cpp
//	Copyright (C) 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"
#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "NavMesh.h"
#include "../TrueNature/Terrain.h"
#include "../gameobjects/ObjManag.h"
#include "RecastAndDetour/DetourNavMeshBuilder.h"
#include "RecastAndDetour/RecastDebugDraw.h"
#include "RecastAndDetour/RecastAlloc.h"
#include "NavigationDebugVisualizer.h"
#include "NavMeshActor.h"
#include "../../EclipseStudio/Sources/XMLHelpers.h"

//////////////////////////////////////////////////////////////////////////

r3dNavigationMesh gNavMesh;
extern r3dCamera gCam;
extern float g_NoTerrainLevelBaseHeight;

//////////////////////////////////////////////////////////////////////////

float gNavAgentHeight = 2.0f;
float gNavAgentRadius = 0.6f;
float gNavAgentMaxClimb = 0.9f;

namespace
{
	const float edgeMaxLen = 12.0f;
	const float edgeMaxError = 1.3f;
	const float regionMinSize = 8.0f;
	const float regionMergeSize = 20.0f;
	const float vertsPerPoly = 6.0f;
	const float detailSampleDist = 6.0f;
	const float detailSampleMaxError = 1.0f;

//////////////////////////////////////////////////////////////////////////

	const char * const NAV_DATA_DIR = "\\navigation";
	const char * const NAV_MESH_FILE_NAME = "polymesh.bin";
	const char * const NAV_DETAIL_MESH_FILE_NAME = "polymesh_detail.bin";
	const char * const NAV_PARAMS_FILE_NAME = "editor_nav_data.xml";

	const char NAV_MESH_HEADER[] = "RECASTPOLY1";
	const char NAV_DETAIL_MESH_HEADER[] = "RECASTPOLYDETAL1";

//////////////////////////////////////////////////////////////////////////

	template <typename F>
	struct AutoClose
	{
		explicit AutoClose(F *f): file(f) {}
		~AutoClose()
		{
			fclose( file );
		}

		F *file;
	};

} // unnamed namespace

//////////////////////////////////////////////////////////////////////////

using namespace r3dTL;

//////////////////////////////////////////////////////////////////////////

r3dNavigationMesh::r3dNavigationMesh()
: polyMesh(0)
, polyMeshDetail(0)
, navMesh(0)
, navQuery(dtAllocNavMeshQuery())
, debugDraw(0)
, visualizationFlags(0)
, regionsMgr(0)
{
	ZeroMemory(&buildConfig, sizeof(buildConfig));
	buildConfig.cs = 1.3f;
	buildConfig.ch = 0.2f;
	buildConfig.walkableSlopeAngle = 45.0f;
}

//////////////////////////////////////////////////////////////////////////

r3dNavigationMesh::~r3dNavigationMesh()
{
	dtFreeNavMeshQuery(navQuery);
	FreeInternalData();
	delete debugDraw;
	delete regionsMgr;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavigationMesh::FreeInternalData()
{
	rcFreePolyMesh(polyMesh);
	polyMesh = 0;

	rcFreePolyMeshDetail(polyMeshDetail);
	polyMeshDetail = 0;

	dtFreeNavMesh(navMesh);
	navMesh = 0;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavigationMesh::InitBuildConfig()
{
	//	Most of settings hardcoded for now
	buildConfig.walkableHeight = static_cast<int>(ceilf(gNavAgentHeight / buildConfig.ch));
	buildConfig.walkableClimb = static_cast<int>(floorf(gNavAgentMaxClimb / buildConfig.ch));
	buildConfig.walkableRadius = static_cast<int>(ceilf(gNavAgentRadius / buildConfig.cs));
	buildConfig.maxEdgeLen = static_cast<int>(edgeMaxLen / buildConfig.cs);
	buildConfig.maxSimplificationError = edgeMaxError;
	buildConfig.minRegionArea = static_cast<int>(rcSqr(regionMinSize));		// Note: area = size*size
	buildConfig.mergeRegionArea = static_cast<int>(rcSqr(regionMergeSize));	// Note: area = size*size
	buildConfig.maxVertsPerPoly = static_cast<int>(vertsPerPoly);
	buildConfig.detailSampleDist = detailSampleDist < 0.9f ? 0 : buildConfig.cs * detailSampleDist;
	buildConfig.detailSampleMaxError = buildConfig.ch * detailSampleMaxError;

	r3dPoint3D minPt, maxPt;
	CalcSceneBBox(minPt, maxPt);
	buildConfig.bmax[0] = maxPt.x;
	buildConfig.bmax[1] = maxPt.y;
	buildConfig.bmax[2] = maxPt.z;
	buildConfig.bmin[0] = minPt.x;
	buildConfig.bmin[1] = minPt.y;
	buildConfig.bmin[2] = minPt.z;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavigationMesh::CalcSceneBBox(r3dPoint3D &minP, r3dPoint3D &maxP)
{
	r3dPoint2D minPt, maxPt;
	GetRegionsMgr().CalcBBox(minPt, maxPt);
	maxP.x = maxPt.x;
	maxP.z = maxPt.y;
	minP.x = minPt.x;
	minP.z = minPt.y;

	if (Terrain)
	{
		const r3dTerrainDesc &td = Terrain->GetDesc();;
		minP.y = td.MinHeight;
		maxP.y = td.MaxHeight;
	}
	else
	{
		GameObject *o = GameWorld().GetFirstObject();
		if (o)
		{
			r3dBoundBox bb = o->GetBBoxWorld();
			for (GameObject *obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj))
			{
				if (!obj->hasMesh())
					continue;

				bb.ExpandTo(obj->GetBBoxWorld());
			}

			minP.y = bb.Org.y;
			maxP.y = bb.Org.y + bb.Size.y;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::BuildForCurrentLevel()
{
	FreeInternalData();

	InitBuildConfig();

	rcCalcGridSize(buildConfig.bmin, buildConfig.bmax, buildConfig.cs, &buildConfig.width, &buildConfig.height);

	rcHeightfield *heightfield = rcAllocHeightfield();
	r3d_assert(heightfield);

	if (!rcCreateHeightfield(&ctx, *heightfield, buildConfig.width, buildConfig.height, buildConfig.bmin, buildConfig.bmax, buildConfig.cs, buildConfig.ch))
	{
		r3dOutToLog("buildNavigation: Could not create solid heightfield.");
		return false;
	}

	//	Submit terrain triangles
	if (Terrain)
	{
		SubmitNavmeshTriangles(*heightfield, Terrain->GetHFShape());
	}

	//	Submit level objects triangles
	GameObject *obj = 0;
	for (obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj))
	{
		if (!obj->isActive() || obj->ObjFlags & OBJFLAG_SkipCastRay)
		{
			continue;
		}

		// if it's a mesh objects
		if (!obj->isObjType(OBJTYPE_Mesh) || !obj->PhysicsObject)
		{
			continue;
		}

		PxActor *a = obj->PhysicsObject->getPhysicsActor();
		PxRigidActor *ra = a->isRigidActor();
		if (!ra)
		{
			continue;
		}

		PxU32 nbShapes = ra->getNbShapes();
		for (PxU32 i = 0; i < nbShapes; ++i)
		{
			PxShape *s = 0;
			ra->getShapes(&s, 1, i);
			PxGeometryType::Enum gt = s->getGeometryType();
			switch(gt)
			{
			case PxGeometryType::eTRIANGLEMESH:
				{
					PxTriangleMeshGeometry g;
					s->getTriangleMeshGeometry(g);
					PxTransform t = s->getLocalPose().transform(ra->getGlobalPose());
					SubmitNavmeshTriangles(*heightfield, g, t);
					break;
				}
			default:
				r3dOutToLog("buildNavigation: Unsupported physx mesh type");
			}
		}
	}

	//	Filter walkable areas
	rcFilterLowHangingWalkableObstacles(&ctx, buildConfig.walkableClimb, *heightfield);
	rcFilterLedgeSpans(&ctx, buildConfig.walkableHeight, buildConfig.walkableClimb, *heightfield);
	rcFilterWalkableLowHeightSpans(&ctx, buildConfig.walkableHeight, *heightfield);

	//	Create compact heightfield
	rcCompactHeightfield *compactHeightfield = rcAllocCompactHeightfield();
	r3d_assert(compactHeightfield);

	if (!rcBuildCompactHeightfield(&ctx, buildConfig.walkableHeight, buildConfig.walkableClimb, *heightfield, *compactHeightfield))
	{
		r3dOutToLog("buildNavigation: Could not build compact data.");
		return false;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&ctx, buildConfig.walkableRadius, *compactHeightfield))
	{
		r3dOutToLog("buildNavigation: Could not erode.");
		return false;
	}

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(&ctx, *compactHeightfield))
	{
		r3dOutToLog("buildNavigation: Could not build distance field.");
		return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(&ctx, *compactHeightfield, 0, buildConfig.minRegionArea, buildConfig.mergeRegionArea))
	{
		r3dOutToLog("buildNavigation: Could not build regions.");
		return false;
	}

	//	Trace and simplify region contours
	rcContourSet *contourSet = rcAllocContourSet();
	r3d_assert(contourSet);

	if (!rcBuildContours(&ctx, *compactHeightfield, buildConfig.maxSimplificationError, buildConfig.maxEdgeLen, *contourSet))
	{
		r3dOutToLog("buildNavigation: Could not create contours.");
		return false;
	}

	//	Build polygons mesh from contours.
	polyMesh = rcAllocPolyMesh();
	r3d_assert(polyMesh);

	if (!rcBuildPolyMesh(&ctx, *contourSet, buildConfig.maxVertsPerPoly, *polyMesh))
	{
		r3dOutToLog("buildNavigation: Could not triangulate contours.");
		return false;
	}

	//	Create detail mesh which allows to access approximate height on each polygon.
	polyMeshDetail = rcAllocPolyMeshDetail();
	r3d_assert(polyMeshDetail);

	if (!rcBuildPolyMeshDetail(&ctx, *polyMesh, *compactHeightfield, buildConfig.detailSampleDist, buildConfig.detailSampleMaxError, *polyMeshDetail))
	{
		r3dOutToLog("buildNavigation: Could not build detail mesh.");
		return false;
	}

	CreateDetourNavData();

	rcFreeHeightField(heightfield);
	rcFreeCompactHeightfield(compactHeightfield);
	rcFreeContourSet(contourSet);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::CreateDetourNavData()
{
	unsigned char* navData = 0;
	int navDataSize = 0;

	// Update poly flags from areas.
	for (int i = 0; i < polyMesh->npolys; ++i)
	{
		polyMesh->flags[i] = 0xffff;
	}

	dtNavMeshCreateParams params;
	memset(&params, 0, sizeof(params));
	params.verts = polyMesh->verts;
	params.vertCount = polyMesh->nverts;
	params.polys = polyMesh->polys;
	params.polyAreas = polyMesh->areas;
	params.polyFlags = polyMesh->flags;
	params.polyCount = polyMesh->npolys;
	params.nvp = polyMesh->nvp;
	params.detailMeshes = polyMeshDetail->meshes;
	params.detailVerts = polyMeshDetail->verts;
	params.detailVertsCount = polyMeshDetail->nverts;
	params.detailTris = polyMeshDetail->tris;
	params.detailTriCount = polyMeshDetail->ntris;
// 	params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
// 	params.offMeshConRad = m_geom->getOffMeshConnectionRads();
// 	params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
// 	params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
// 	params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
// 	params.offMeshConUserID = m_geom->getOffMeshConnectionId();
// 	params.offMeshConCount = m_geom->getOffMeshConnectionCount();
	params.walkableHeight = gNavAgentHeight;
	params.walkableRadius = gNavAgentRadius;
	params.walkableClimb = gNavAgentMaxClimb;
	rcVcopy(params.bmin, polyMesh->bmin);
	rcVcopy(params.bmax, polyMesh->bmax);
	params.cs = buildConfig.cs;
	params.ch = buildConfig.ch;
	params.buildBvTree = true;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
	{
		r3dOutToLog("buildNavigation: Could not build Detour navmesh.");
		return false;
	}

	navMesh = dtAllocNavMesh();
	r3d_assert(navMesh);

	dtStatus status;

	status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status))
	{
		dtFree(navData);
		r3dOutToLog("buildNavigation: Could not init Detour navmesh");
		return false;
	}

	status = navQuery->init(navMesh, 2048);
	if (dtStatusFailed(status))
	{
		r3dOutToLog("buildNavigation: Could not init Detour navmesh query");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::SubmitNavmeshTriangles(rcHeightfield &hf, PxHeightFieldGeometry &g)
{
	const PxReal rs = g.rowScale;
	const PxReal hs = g.heightScale;
	const PxReal cs = g.columnScale;

	PxHeightField*	physHF = g.heightField;
	const PxU32 nbCols = physHF->getNbColumns();
	const PxU32	nbRows = physHF->getNbRows();
	const PxU32	nbVerts = nbRows * nbCols;	
	      PxU32 nbFaces = (nbCols - 1) * (nbRows - 1) * 2;

	PxHeightFieldSample* sampleBuffer = new PxHeightFieldSample[nbVerts];
	physHF->saveCells(sampleBuffer, nbVerts * sizeof(PxHeightFieldSample));

	PxVec3* vertexes = new PxVec3[nbVerts];
	for(PxU32 i = 0; i < nbRows; i++) 
	{
		for(PxU32 j = 0; j < nbCols; j++) 
		{
			vertexes[i * nbCols + j] = PxVec3(PxReal(i) * rs, PxReal(sampleBuffer[j + (i*nbCols)].height) * hs, PxReal(j) * cs);
		}
	}
	
	PxU32 numIndices = nbFaces * 3;
	int *indices = new int[numIndices];
	for(PxU32 i = 0; i < (nbCols - 1); ++i) 
	{
		for(PxU32 j = 0; j < (nbRows - 1); ++j) 
		{
			// first triangle
			indices[6 * (i * (nbRows - 1) + j) + 0] = (i + 1) * nbRows + j; 
			indices[6 * (i * (nbRows - 1) + j) + 1] = i * nbRows + j;
			indices[6 * (i * (nbRows - 1) + j) + 2] = i * nbRows + j + 1;
			// second triangle
			indices[6 * (i * (nbRows - 1) + j) + 3] = (i + 1) * nbRows + j + 1;
			indices[6 * (i * (nbRows - 1) + j) + 4] = (i + 1) * nbRows + j;
			indices[6 * (i * (nbRows - 1) + j) + 5] = i * nbRows + j + 1;
		}
	}

	//	Reject out-of-regions triangles
	numIndices = FilterOutsideTriangles(vertexes, indices, numIndices);
	r3d_assert(numIndices % 3 == 0);
	nbFaces = numIndices / 3;

	if (nbFaces > 0)
	{
		//	Submit triangles to navmesh builder
		unsigned char *triAreas = new unsigned char[nbFaces];
		memset(triAreas, 0, sizeof(*triAreas) * nbFaces);
		rcMarkWalkableTriangles(&ctx, buildConfig.walkableSlopeAngle, &vertexes[0].x, nbVerts, indices, nbFaces, triAreas);
		rcRasterizeTriangles(&ctx, &vertexes[0].x, nbVerts, indices, triAreas, nbFaces, hf, buildConfig.walkableClimb);
		delete [] triAreas;
	}

	delete [] indices;
	delete [] sampleBuffer;
	delete [] vertexes;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::SubmitNavmeshTriangles(rcHeightfield &hf, PxTriangleMeshGeometry &g, const PxTransform &globalPose)
{
	PxTriangleMesh *tm = g.triangleMesh;

	const PxU32 nbVerts = tm->getNbVertices();
	const PxVec3* verts = tm->getVertices();
	      PxU32 nbTris = tm->getNbTriangles();
	const void* typelessTris = tm->getTriangles();

	PxU32 numIndices = nbTris * 3;
	int* tris = new int[numIndices];

	if (tm->has16BitTriangleIndices())
	{
		//	Convert indices to 32 bit
		const short *i16 = reinterpret_cast<const short*>(typelessTris);

		for (PxU32 i = 0; i < numIndices; ++i)
		{
			tris[i] = i16[i];
		}
	}
	else
	{
		int size = numIndices * sizeof(*tris);
		memcpy_s(tris, size, typelessTris, size);
	}

	//	Transform vertices to world space
	PxVec3 *transformedVerts = new PxVec3[nbVerts];
	for (PxU32 i = 0; i < nbVerts; ++i)
	{
		transformedVerts[i] = globalPose.transform(verts[i]);
	}
	
	//	Reject out-of-regions triangles
	numIndices = FilterOutsideTriangles(transformedVerts, tris, numIndices);
	r3d_assert(numIndices % 3 == 0);
	nbTris = numIndices / 3;

	if (nbTris > 0)
	{
		unsigned char *triAreas = new unsigned char[nbTris];
		ZeroMemory(triAreas, sizeof(*triAreas) * nbTris);
		rcMarkWalkableTriangles(&ctx, buildConfig.walkableSlopeAngle, &transformedVerts[0].x, nbVerts, tris, nbTris, triAreas);
		rcRasterizeTriangles(&ctx, &transformedVerts[0].x, nbVerts, tris, triAreas, nbTris, hf, buildConfig.walkableClimb);
		delete [] triAreas;
	}

	delete [] transformedVerts;
	delete [] tris;

	return true;
}

//////////////////////////////////////////////////////////////////////////

PxU32 r3dNavigationMesh::FilterOutsideTriangles(const PxVec3 *verices, int *indices, PxU32 numIndices)
{
	r3d_assert(numIndices % 3 == 0);

	int *indEnd = indices + numIndices;
	int *indCur = indices;
	PxU32 resultIndices = numIndices;

	while (indCur < indEnd)
	{
		const PxVec3 &v0 = verices[indCur[0]];
		const PxVec3 &v1 = verices[indCur[1]];
		const PxVec3 &v2 = verices[indCur[2]];

		r3dPoint2D pt0(v0.x, v0.z);
		r3dPoint2D pt1(v1.x, v1.z);
		r3dPoint2D pt2(v2.x, v2.z);

		bool inside =
			GetRegionsMgr().IsPointInsideRegions(pt0) ||
			GetRegionsMgr().IsPointInsideRegions(pt1) ||
			GetRegionsMgr().IsPointInsideRegions(pt2);

		//	If all vertices are outside, delete this indices
		if (!inside)
		{
			indCur[0] = indEnd[-3];
			indCur[1] = indEnd[-2];
			indCur[2] = indEnd[-1];
			indEnd -= 3;
			resultIndices -= 3;
		}
		else
		{
			indCur += 3;
		}
	}
	return resultIndices;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavigationMesh::DebugDraw()
{
	if (visualizationFlags == dvtNone)
		return;

	D3DPERF_BeginEvent(0, L"r3dNavigationMesh::DebugDraw");

	r3dRenderer->SetCullMode(D3DCULL_NONE);
	int psIdx = r3dRenderer->GetCurrentPixelShaderIdx();
	int vsIdx = r3dRenderer->GetCurrentVertexShaderIdx();

	r3dRenderer->SetPixelShader();
	r3dRenderer->SetVertexShader();

	int rm = r3dRenderer->GetRenderingMode();
	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_ZC);

	r3dRenderer->SetTex(0);

	if (regionsMgr && visualizationFlags & dvtRegions)
	{
		regionsMgr->VisualizeRegions();
	}

	if (polyMesh)
	{
		if (!debugDraw)
			debugDraw = new NavDataDebugVisualizer;

		if (visualizationFlags & dvtMesh)
			duDebugDrawPolyMesh(debugDraw, *polyMesh);

//		if (visualizationFlags & dvtVoxels)
			//duDebugDrawHeightfieldSolid(debugDraw, *heightfield);
	}

	r3dRenderer->Flush();
	r3dRenderer->SetRenderingMode(rm);

	r3dRenderer->SetPixelShader(psIdx);
	r3dRenderer->SetVertexShader(vsIdx);

	r3dRenderer->RestoreCullMode();

	gNavMeshActorsManager.DebugDrawAgents();

	D3DPERF_EndEvent();
}

//////////////////////////////////////////////////////////////////////////

void r3dNavigationMesh::VisualizeNavMesh(int visFalgs)
{
	visualizationFlags = visFalgs;
}

//////////////////////////////////////////////////////////////////////////

r3dNavConvexRegionsManager & r3dNavigationMesh::GetRegionsMgr()
{
	if (!regionsMgr)
		regionsMgr = new r3dNavConvexRegionsManager;
	return *regionsMgr;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::Save(const r3dString &levelHomeDir)
{
	r3dString navPath = levelHomeDir + NAV_DATA_DIR;

	// try creating just in case
	mkdir(navPath.c_str());

	navPath += "\\";

	bool rv = SavePolyMesh(navPath + NAV_MESH_FILE_NAME);
	rv &= SavePolyMeshDetail(navPath + NAV_DETAIL_MESH_FILE_NAME);
	rv &= SaveBuildParams(navPath + NAV_PARAMS_FILE_NAME);
	return rv;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::Load(const r3dString &levelHomeDir)
{
	r3dString navPath = levelHomeDir + NAV_DATA_DIR + "\\";

	FreeInternalData();

	bool rv = LoadPolyMesh(navPath + NAV_MESH_FILE_NAME);
	rv &= LoadPolyMeshDetail(navPath + NAV_DETAIL_MESH_FILE_NAME);
	rv &= LoadBuildParams(navPath + NAV_PARAMS_FILE_NAME);

	if (rv)
	{
		rv &= CreateDetourNavData();
		gNavMeshActorsManager.Init(*this);
	}

	return rv;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::SavePolyMesh(const r3dString &filePath)
{
	if (!polyMesh)
		return false;

	FILE* fout = fopen(filePath.c_str(), "wb");

	if (!fout)
	{
		r3dOutToLog("r3dNavigationMesh::SavePolyMesh: Couldn't open poly mesh file %s for writing!", filePath.c_str());
		return false;
	}

	AutoClose<FILE> acf(fout);

	fwrite(NAV_MESH_HEADER, sizeof(NAV_MESH_HEADER), 1, fout);

	fwrite_be(polyMesh->nverts, fout);
	fwrite_be(polyMesh->npolys, fout);
	fwrite_be(polyMesh->maxpolys, fout);
	fwrite_be(polyMesh->nvp, fout);
	fwrite_be(polyMesh->bmin[0], fout);
	fwrite_be(polyMesh->bmin[1], fout);
	fwrite_be(polyMesh->bmin[2], fout);
	fwrite_be(polyMesh->bmax[0], fout);
	fwrite_be(polyMesh->bmax[1], fout);
	fwrite_be(polyMesh->bmax[2], fout);
	fwrite_be(polyMesh->cs, fout);
	fwrite_be(polyMesh->ch, fout);
	fwrite_be(polyMesh->borderSize, fout);

//	polyMesh->verts = reinterpret_cast<unsigned short *>(rcAlloc(sizeof(unsigned short) * polyMesh->nverts, RC_ALLOC_PERM));
	for (int i = 0; i < polyMesh->nverts * 3; ++i)
	{
		fwrite_be(polyMesh->verts[i], fout);
	}

	for (int i = 0; i < polyMesh->maxpolys * 2 * polyMesh->nvp; ++i)
	{
		fwrite_be(polyMesh->polys[i], fout);
	}

	for (int i = 0; i < polyMesh->maxpolys; ++i)
	{
		fwrite_be(polyMesh->regs[i], fout);
	}

	for (int i = 0; i < polyMesh->maxpolys; ++i)
	{
		fwrite_be(polyMesh->flags[i], fout);
	}

	fwrite(polyMesh->areas, polyMesh->maxpolys, 1, fout);
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::SavePolyMeshDetail(const r3dString &filePath)
{
	if (!polyMeshDetail)
		return false;

	FILE* fout = fopen(filePath.c_str(), "wb");

	if (!fout)
	{
		r3dOutToLog("r3dNavigationMesh::SavePolyMesh: Couldn't open poly mesh file %s for writing!", filePath.c_str());
		return false;
	}

	AutoClose<FILE> acf(fout);

	fwrite(NAV_DETAIL_MESH_HEADER, sizeof(NAV_DETAIL_MESH_HEADER), 1, fout);

	fwrite_be(polyMeshDetail->nmeshes, fout);
	fwrite_be(polyMeshDetail->nverts, fout);
	fwrite_be(polyMeshDetail->ntris, fout);

	for (int i = 0; i < polyMeshDetail->nmeshes * 4; ++i)
	{
		fwrite_be(polyMeshDetail->meshes[i], fout);
	}

	for (int i = 0; i < polyMeshDetail->nverts * 3; ++i)
	{
		fwrite_be(polyMeshDetail->verts[i], fout);
	}

	fwrite(polyMeshDetail->tris, polyMeshDetail->ntris * 4, 1, fout);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::LoadPolyMesh(const r3dString &filePath)
{
	r3dFile * fin = r3d_open(filePath.c_str(), "rb");

	if (!fin) return false;

	AutoClose<r3dFile> afc(fin);

	char header[sizeof(NAV_MESH_HEADER)] = {0};

	if (fread(header, sizeof(header), 1, fin) != 1)
		return false;

	if (strcmp(NAV_MESH_HEADER, header) != 0)
	{
		r3dArtBug("r3dNavigationMesh::LoadPolyMesh: Navigation polymesh file is corrupted!");
		return false;
	}

	rcFreePolyMesh(polyMesh);
	polyMesh = rcAllocPolyMesh();

	fread_be(polyMesh->nverts, fin);
	fread_be(polyMesh->npolys, fin);
	fread_be(polyMesh->maxpolys, fin);
	fread_be(polyMesh->nvp, fin);
	fread_be(polyMesh->bmin[0], fin);
	fread_be(polyMesh->bmin[1], fin);
	fread_be(polyMesh->bmin[2], fin);
	fread_be(polyMesh->bmax[0], fin);
	fread_be(polyMesh->bmax[1], fin);
	fread_be(polyMesh->bmax[2], fin);
	fread_be(polyMesh->cs, fin);
	fread_be(polyMesh->ch, fin);
	fread_be(polyMesh->borderSize, fin);

	polyMesh->verts = reinterpret_cast<unsigned short *>(rcAlloc(sizeof(unsigned short) * polyMesh->nverts * 3, RC_ALLOC_PERM));
	for (int i = 0; i < polyMesh->nverts * 3; ++i)
	{
		fread_be(polyMesh->verts[i], fin);
	}

	polyMesh->polys = reinterpret_cast<unsigned short *>(rcAlloc(sizeof(unsigned short) * polyMesh->maxpolys * 2 * polyMesh->nvp, RC_ALLOC_PERM));
	for (int i = 0; i < polyMesh->maxpolys * 2 * polyMesh->nvp; ++i)
	{
		fread_be(polyMesh->polys[i], fin);
	}

	polyMesh->regs = reinterpret_cast<unsigned short *>(rcAlloc(sizeof(unsigned short) * polyMesh->maxpolys, RC_ALLOC_PERM));
	for (int i = 0; i < polyMesh->maxpolys; ++i)
	{
		fread_be(polyMesh->regs[i], fin);
	}

	polyMesh->flags = reinterpret_cast<unsigned short *>(rcAlloc(sizeof(unsigned short) * polyMesh->maxpolys, RC_ALLOC_PERM));
	for (int i = 0; i < polyMesh->maxpolys; ++i)
	{
		fread_be(polyMesh->flags[i], fin);
	}

	polyMesh->areas = reinterpret_cast<unsigned char *>(rcAlloc(sizeof(unsigned char) * polyMesh->maxpolys, RC_ALLOC_PERM));
	fread(polyMesh->areas, polyMesh->maxpolys, 1, fin);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::LoadPolyMeshDetail(const r3dString &filePath)
{
	r3dFile * fin = r3d_open(filePath.c_str(), "rb");

	if (!fin) return false;

	AutoClose<r3dFile> afc(fin);

	char header[sizeof(NAV_DETAIL_MESH_HEADER)] = {0};

	if (fread(header, sizeof(header), 1, fin) != 1)
		return false;

	if (strcmp(NAV_DETAIL_MESH_HEADER, header) != 0)
	{
		r3dArtBug("r3dNavigationMesh::LoadPolyMeshDetail: Navigation detail polymesh file is corrupted!");
		return false;
	}

	rcFreePolyMeshDetail(polyMeshDetail);
	polyMeshDetail = rcAllocPolyMeshDetail();

	fread_be(polyMeshDetail->nmeshes, fin);
	fread_be(polyMeshDetail->nverts, fin);
	fread_be(polyMeshDetail->ntris, fin);

	polyMeshDetail->meshes = reinterpret_cast<unsigned int *>(rcAlloc(sizeof(unsigned int) * polyMeshDetail->nmeshes * 4, RC_ALLOC_PERM));
	for (int i = 0; i < polyMeshDetail->nmeshes * 4; ++i)
	{
		fread_be(polyMeshDetail->meshes[i], fin);
	}

	polyMeshDetail->verts = reinterpret_cast<float *>(rcAlloc(sizeof(float) * polyMeshDetail->nverts * 3, RC_ALLOC_PERM));
	for (int i = 0; i < polyMeshDetail->nverts * 3; ++i)
	{
		fread_be(polyMeshDetail->verts[i], fin);
	}

	polyMeshDetail->tris = reinterpret_cast<unsigned char *>(rcAlloc(sizeof(unsigned char) * polyMeshDetail->ntris * 4, RC_ALLOC_PERM));
	fread(polyMeshDetail->tris, polyMeshDetail->ntris * 4, 1, fin);

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::SaveBuildParams(const r3dString &filePath)
{
	pugi::xml_document xmlDoc;

	pugi::xml_node bp = xmlDoc.append_child();
	bp.set_name("build_config");

	SetXMLVal("cell_size", bp, &buildConfig.cs);
	SetXMLVal("cell_height", bp, &buildConfig.ch);
	SetXMLVal("walkable_slope_angle", bp, &buildConfig.walkableSlopeAngle);
	SetXMLVal("agent_height", bp, &gNavAgentHeight);
	SetXMLVal("agent_radius", bp, &gNavAgentRadius);
	SetXMLVal("agent_max_climb", bp, &gNavAgentMaxClimb);

	bool rv = GetRegionsMgr().Save(xmlDoc);

	xmlDoc.save_file(filePath.c_str());
	return rv;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavigationMesh::LoadBuildParams(const r3dString &filePath)
{
	r3dFile* f = r3d_open(filePath.c_str(), "r");
	if (!f) return false;

	r3dTL::TArray<char> fileBuffer(f->size + 1);

	fread(&fileBuffer[0], f->size, 1, f);
	fileBuffer[f->size] = 0;

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result parseResult = xmlDoc.load_buffer_inplace(&fileBuffer[0], f->size);
	fclose(f);
	if (!parseResult) return false;

	pugi::xml_node bp = xmlDoc.child("build_config");
	GetXMLVal("cell_size", bp, &buildConfig.cs);
	GetXMLVal("cell_height", bp, &buildConfig.ch);
	GetXMLVal("walkable_slope_angle", bp, &buildConfig.walkableSlopeAngle);
	GetXMLVal("agent_height", bp, &gNavAgentHeight);
	GetXMLVal("agent_radius", bp, &gNavAgentRadius);
	GetXMLVal("agent_max_climb", bp, &gNavAgentMaxClimb);

	bool rv = GetRegionsMgr().Load(xmlDoc);

	InitBuildConfig();
	return rv;
}

//-------------------------------------------------------------------------
//	NavConvexRegionsManager
//-------------------------------------------------------------------------

namespace
{
	typedef float VECT2[2];
	#define SWAP(i,j) { int Tmp = Ind[i]; Ind[i] = Ind[j]; Ind[j] = Tmp; }
	static const VECT2 *Sort_Pts;

	//////////////////////////////////////////////////////////////////////////

	static float Angle(const VECT2 P0, const VECT2 P1, const VECT2 P2)
	{
		// "Angle" between P0->P1 and P0->P2.
		// actually returns: ||(P1-P0) ^ (P2-P0)||
		float dx21 = P2[0] - P1[0];
		float dy21 = P2[1] - P1[1];
		float dx10 = P1[0] - P0[0];
		float dy10 = P1[1] - P0[1];
		return (dy21*dx10 - dx21*dy10);
	}

	//////////////////////////////////////////////////////////////////////////

	static int Compare(const VECT2 A, const VECT2 B) {
		if      (A[0]<B[0]) return  1;
		else if (A[0]>B[0]) return -1;
		else return 0;
	}

	//////////////////////////////////////////////////////////////////////////

	static int Cmp_Lo(const void *a, const void *b) {
		return Compare( Sort_Pts[*(int*)a], Sort_Pts[*(int*)b] );
	}

	//////////////////////////////////////////////////////////////////////////

	int QHull_Internal(int N, const VECT2 *Pts, int *Ind,
		int iA, int iB)
	{
		int n, nn, n2, m, Split1, Split2, iMid;
		int Mid_Pt;
		double dMax;

		if (N<=2) return N;
		// As middle point, search the farthest away from line [A-->B]
		Mid_Pt = -1;
		dMax = 0.0;
		for(n=1; n<N; ++n) {
			double d = fabs( Angle( Pts[Ind[n]], Pts[iA], Pts[iB] ) );
			if (d>=dMax) { dMax = d; Mid_Pt = n; }
		}
		// Partition against midpoint
		iMid = Ind[Mid_Pt];
		Ind[Mid_Pt] = Ind[N-1];
		//Ind = [A|...........|xxx] (B)
		//  n = [0|1.......N-2|N-1|

		nn = N-2;
		n  = 1;
		while(n<=nn) {
			float d = Angle( Pts[Ind[n]], Pts[iA], Pts[iMid] );
			if (d>=0.0) { SWAP(n,nn); nn--; }
			else n++;
		}
		Ind[N-1] = Ind[n];
		Ind[n] = iMid;
		Split1 = n++;
		// Partition Status:
		//Ind = (A) [..(+)..| M |...........] (B)
		//  n =     [1......|Sp1|........N-1]
		nn = N-1;
		while(n<=nn) {
			float d = Angle( Pts[Ind[n]], Pts[iMid], Pts[iB] );
			if (d>=0.0) { SWAP(n,nn); nn--; }
			else n++;
		}
		Split2 = n;
		// Partition Status:
		//Ind = (A) [....(+)...| M |....(-)....|(trash)......] (B)
		//  N =     [1.........|Sp1|...........|Sp2.......N-1]

		// Recurse each sub-partition

		n  = QHull_Internal(Split1,        Pts, Ind       , iA, iMid);
		n2 = QHull_Internal(Split2-Split1, Pts, Ind+Split1, iMid, iB);
		m = Split1;
		while(n2-->0) {
			SWAP(n,m);
			m++; n++;
		}

		return n;
	}

	//////////////////////////////////////////////////////////////////////////

	int QuickHullAlgo(int Nb, const VECT2 *Pts, int *Ind)
	{
		int n, nn, m, iA, iB;

		if (Nb<=2) return Nb;

		Sort_Pts = Pts; // nasty. Only to overcome qsort()'s API
		qsort(Ind, Nb, sizeof(int), Cmp_Lo);
		// first partitioning: classify points with respect to
		// the line joining the extreme points #0 and #Nb-1
		iA = Ind[0];
		iB = Ind[Nb-1];

		m = Nb-2;
		n = 1;
		while(n<=m) {
			float d = Angle( Pts[Ind[n]], Pts[iA], Pts[iB] );
			if (d>=0.0) { SWAP(n,m); m--; }
			else n++;
		}
		Ind[Nb-1] = Ind[n];
		Ind[n] = iB;
		// Partition is now:
		//  Ind = [ A|...(+)....[B]...(-)...|A ]
		//   n  = [0 |1.........[n].........|Nb]
		// We now process the two halves [A..(+)..B] and [B..(-)..A]
		m  = QHull_Internal(   n, Pts, Ind  , iA, iB); // 1st half [A..(+)..B]
		nn = QHull_Internal(Nb-n, Pts, Ind+n, iB, iA); // 2nd half [B..(-)..A]

		while(nn-->0) {
			SWAP(m,n);
			m++; n++;
		}
		return m;
	}
}

//////////////////////////////////////////////////////////////////////////

r3dNavConvexRegionsManager::r3dNavConvexRegionsManager()
: hightlightedRegion(0)
{

}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::AddPointToRegion(const r3dPoint3D &pt, uint32_t idx)
{
	if (regions.size() == 0 || idx >= regions.size())
		return;

	Convex &c = regions[idx];

	r3dPoint2D pt2D(pt.x, pt.z);

	c.push_back(pt2D);

	VECT2 *pts = reinterpret_cast<VECT2*>(&c[0].x);
	int *indices = new int[c.size()];

	for (int i = 0; i < static_cast<int>(c.size()); ++i)
	{
		indices[i] = i;
	}

	int n = QuickHullAlgo(c.size(), pts, indices);

	Convex newC;
	for (int i = 0; i < n; ++i)
	{
		newC.push_back(c[indices[i]]);
	}
	c = newC;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::CreateNewRegion()
{
	regions.resize(regions.size() + 1);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::DeleteRegion(uint32_t idx)
{
	if (regions.size() <= idx)
		return;

	regions.erase(regions.begin() + idx);
}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::VisualizeRegions()
{
	const float fenceHeight = 5.0f;

	const r3dColor fenceCl(0, 255, 0, 90);
	const r3dColor brightfenceCl(255, 255, 0, 90);

	float baseFenceHeight = g_NoTerrainLevelBaseHeight;

	for (size_t i = 0; i < regions.size(); ++i)
	{
		Convex &c = regions[i];

		const r3dColor &modCl = (i == hightlightedRegion ? brightfenceCl : fenceCl);
		
		for (size_t j = 0; j < c.size(); ++j)
		{
			r3dPoint2D &p2D0 = c[j];
			r3dPoint2D &p2D1 = c[(j + 1) % c.size()];

			if (Terrain)
				baseFenceHeight = Terrain->GetHeight(r3dPoint3D(p2D0.x, 0, p2D0.y));

			r3dPoint3D p0(p2D0.x, baseFenceHeight, p2D0.y);
			r3dPoint3D p2(p2D0.x, fenceHeight + baseFenceHeight, p2D0.y);

			if (Terrain)
				baseFenceHeight = Terrain->GetHeight(r3dPoint3D(p2D1.x, 0, p2D1.y));

			r3dPoint3D p1(p2D1.x, baseFenceHeight, p2D1.y);
			r3dPoint3D p3(p2D1.x, fenceHeight + baseFenceHeight, p2D1.y);

			r3dDrawTriangle3D(p0, p1, p2, gCam, modCl, 0, 0, true);
			r3dDrawTriangle3D(p1, p3, p2, gCam, modCl, 0, 0, true);

			r3dDrawTriangle3D(p0, p2, p1, gCam, modCl, 0, 0, true);
			r3dDrawTriangle3D(p2, p3, p1, gCam, modCl, 0, 0, true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavConvexRegionsManager::IsPointInsideConvex(const r3dPoint2D &p, const Convex &polygon) const
{
	if (polygon.empty())
		return false;

	int counter = 0;
	float xinters;
	r3dPoint2D p1, p2;

	int N = static_cast<int>(polygon.size());

	p1 = polygon[0];
	for (int i = 1; i <= N; i++)
	{
		p2 = polygon[i % N];
		if (p.y > std::min(p1.y,p2.y))
		{
			if (p.y <= std::max(p1.y,p2.y))
			{
				if (p.x <= std::max(p1.x,p2.x))
				{
					if (p1.y != p2.y)
					{
						xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
					}
				}
			}
		}
		p1 = p2;
	}

	return counter % 2 != 0;
}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::CalcBBox(r3dPoint2D &minPt, r3dPoint2D &maxPt) const
{
	for (size_t j = 0; j < regions.size(); ++j)
	{
		const Convex &c = regions[j];

		r3dPoint2D polyMin(0, 0), polyMax(0, 0);
		CalcBBoxForConvex(polyMin, polyMax, c);

		if (j == 0)
		{
			maxPt = polyMax;
			minPt = polyMin;
		}
		else
		{
			maxPt.x = std::max(maxPt.x, polyMax.x);
			maxPt.y = std::max(maxPt.y, polyMax.y);
			minPt.x = std::min(minPt.x, polyMin.x);
			minPt.y = std::min(minPt.y, polyMin.y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void r3dNavConvexRegionsManager::CalcBBoxForConvex(r3dPoint2D &minPt, r3dPoint2D &maxPt, const Convex &polygon) const
{
	if (polygon.empty())
		return;

	minPt = polygon[0];
	maxPt = polygon[0];
	for (size_t i = 1; i < polygon.size(); ++i)
	{
		minPt.x = std::min(polygon[i].x, minPt.x);
		minPt.y = std::min(polygon[i].y, minPt.y);

		maxPt.x = std::max(polygon[i].x, maxPt.x);
		maxPt.y = std::max(polygon[i].y, maxPt.y);
	}
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavConvexRegionsManager::IsPointInsideRegions(const r3dPoint2D &p) const
{
	for (size_t j = 0; j < regions.size(); ++j)
	{
		const Convex &c = regions[j];
		if (c.empty())
			continue;

		//	Calc AABB for quick rejection
		r3dPoint2D minPt, maxPt;
		CalcBBoxForConvex(minPt, maxPt, c);

		//	Quick AABB reject
		if (minPt.x > p.x || minPt.y > p.y || maxPt.x < p.x || maxPt.y < p.y)
			continue;

		if (IsPointInsideConvex(p, c))
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavConvexRegionsManager::Load(const pugi::xml_document &xmlDoc)
{
	regions.clear();
	pugi::xml_node n = xmlDoc.child("convex");
	while (!n.empty())
	{
		Convex c;
		pugi::xml_node pts = n.child("point");
		while (!pts.empty())
		{
			r3dPoint2D pt;
			pt.x = pts.attribute("x").as_float();
			pt.y = pts.attribute("y").as_float();
			c.push_back(pt);
			pts = pts.next_sibling("point");
		}
		regions.push_back(c);
		n = n.next_sibling("convex");
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool r3dNavConvexRegionsManager::Save(pugi::xml_document& xmlDoc)
{
	for (uint32_t i = 0; i < regions.size(); ++i)
	{
		Convex &c = regions[i];
		if (c.empty())
			continue;

		pugi::xml_node n = xmlDoc.append_child();
		n.set_name("convex");

		for (uint32_t j = 0; j < c.size(); ++j)
		{
			SetXMLVal("point", n, &c[j]);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_RECAST_NAVIGATION