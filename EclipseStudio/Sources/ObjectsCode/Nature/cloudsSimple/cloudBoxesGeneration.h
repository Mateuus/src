#ifndef ARTY_CLOUD_BOXES_GENERATION
#define ARTY_CLOUD_BOXES_GENERATION

#include "ndmath.h"
#include "types.h"

namespace CloudBoxes
{
	typedef ndmath::NDElement<float, 3> Element;
	typedef ndmath::NDInterval<float, 3> Box;
	

	void generate(int count, const Vector3& size, Box* boxes);
	int generatePoints(float density, int maxPointsCount, const Vector3& size, Vector3* points, const Box& normalizationBox, Box& boundBox);
}

#endif