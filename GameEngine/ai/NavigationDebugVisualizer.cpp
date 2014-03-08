//=========================================================================
//	Module: NavigationDebugVisualizer.cpp
//	Copyright (C) 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"
#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "NavigationDebugVisualizer.h"

//////////////////////////////////////////////////////////////////////////

extern r3dCamera gCam;

//////////////////////////////////////////////////////////////////////////

NavDataDebugVisualizer::NavDataDebugVisualizer()
: currPrimType(DU_DRAW_POINTS)
, currSize(1.0f)
{
	vtxBuffer.Reserve(0x100);
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::begin(duDebugDrawPrimitives prim, float size)
{
	currPrimType = prim;
	currSize = size;
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	Vertex vtx;
	vtx.pos[0] = x;
	vtx.pos[1] = y;
	vtx.pos[2] = z;
	vtx.color = r3dColor24(color);
	vtx.uv[0] = u;
	vtx.uv[1] = v;

	std::swap(vtx.color.B, vtx.color.R);

	vtxBuffer.PushBack(vtx);
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::vertex(const float* pos, unsigned int color, const float* uv)
{
	vertex(pos[0], pos[1], pos[2], color, uv[0], uv[1]);
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::vertex(const float x, const float y, const float z, unsigned int color)
{
	vertex(x, y, z, color, 0.0f, 0.0f);
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::vertex(const float* pos, unsigned int color)
{
	vertex(pos[0], pos[1], pos[2], color, 0.0f, 0.0f);
}

//////////////////////////////////////////////////////////////////////////

void NavDataDebugVisualizer::end()
{
	switch (currPrimType)
	{
	case DU_DRAW_POINTS:
		//	Unsupported yet
		break;

	case DU_DRAW_LINES:
		r3d_assert(vtxBuffer.Count() % 2 == 0);
		for (uint32_t i = 0; i < vtxBuffer.Count(); i += 2)
		{
			const Vertex &s = vtxBuffer[i];
			const Vertex &e = vtxBuffer[i + 1];

			r3dPoint3D start(s.pos[0], s.pos[1], s.pos[2]);
			r3dPoint3D end(e.pos[0], e.pos[1], e.pos[2]);

			r3dDrawLine3D(start, end, gCam, currSize * 0.03f, s.color);
		}
		break;

	case DU_DRAW_TRIS:
		r3d_assert(vtxBuffer.Count() % 3 == 0);
		for (uint32_t i = 0; i < vtxBuffer.Count(); i += 3)
		{
			const Vertex &v0_ = vtxBuffer[i];
			const Vertex &v1_ = vtxBuffer[i + 1];
			const Vertex &v2_ = vtxBuffer[i + 2];

			r3dPoint3D v0(v0_.pos[0], v0_.pos[1], v0_.pos[2]);
			r3dPoint3D v1(v1_.pos[0], v1_.pos[1], v1_.pos[2]);
			r3dPoint3D v2(v2_.pos[0], v2_.pos[1], v2_.pos[2]);
			r3dColor24 cl(v0_.color);

			r3dDrawTriangle3D(v0, v1, v2, gCam, v0_.color, 0, 0, true);
		}
		break;

	case DU_DRAW_QUADS:
		r3d_assert(vtxBuffer.Count() % 4 == 0);
		for (uint32_t i = 0; i < vtxBuffer.Count(); i += 4)
		{
			const Vertex &v0_ = vtxBuffer[i];
			const Vertex &v1_ = vtxBuffer[i + 1];
			const Vertex &v2_ = vtxBuffer[i + 2];
			const Vertex &v3_ = vtxBuffer[i + 3];

			r3dPoint3D v0(v0_.pos[0], v0_.pos[1], v0_.pos[2]);
			r3dPoint3D v1(v1_.pos[0], v1_.pos[1], v1_.pos[2]);
			r3dPoint3D v2(v2_.pos[0], v2_.pos[1], v2_.pos[2]);
			r3dPoint3D v3(v3_.pos[0], v3_.pos[1], v3_.pos[2]);
			r3dColor24 cl(v0_.color);

			r3dDrawTriangle3D(v0, v1, v2, gCam, v0_.color, 0, 0, true);
			r3dDrawTriangle3D(v0, v2, v3, gCam, v0_.color, 0, 0, true);
		}
	}

	vtxBuffer.Clear();
}

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_RECAST_NAVIGATION