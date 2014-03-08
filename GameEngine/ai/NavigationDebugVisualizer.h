//=========================================================================
//	Module: NavigationDebugVisualizer.h
//	Copyright (C) 2012.
//=========================================================================

#pragma once
#if ENABLE_RECAST_NAVIGATION

//////////////////////////////////////////////////////////////////////////

#include "RecastAndDetour/DebugDraw.h"

//////////////////////////////////////////////////////////////////////////

class NavDataDebugVisualizer: public duDebugDraw
{
	duDebugDrawPrimitives currPrimType;
	float currSize;

	/**	Vertex storage. */
	struct Vertex
	{
		float pos[3];
		r3dColor24 color;
		float uv[2];
	};

	/** Intermediate vertex store buffer. */
	r3dTL::TArray<Vertex> vtxBuffer;

public:
	NavDataDebugVisualizer();
	virtual ~NavDataDebugVisualizer(){}
	virtual void depthMask(bool state){}
	virtual void texture(bool state){}
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
};

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_RECAST_NAVIGATION