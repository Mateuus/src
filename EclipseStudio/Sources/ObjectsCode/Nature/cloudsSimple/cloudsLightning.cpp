#include "r3dPCH.h"

#if 0

#include "r3dConst.h"
#include "r3d.h"
#include "cloudsLightning.h"
#include "random.h"
#include "cloudsSystem.h"

inline float dot(const Vector3& v0, const Vector3& v1)
{
	return D3DXVec3Dot(&v0, &v1);
}

inline float sqr(float f)
{
	return f*f;
}

bool DistancePointLine( const Vector3& Point, const Vector3& LineStart, const Vector3& LineEnd, float& Distance )
{
	float LineMagSq = magnitudeSq(LineEnd - LineStart);
	float U = dot( (Point - LineStart), (LineEnd - LineStart)) / LineMagSq;

	if( U < 0.0f || U > 1.0f )
		return false;   // closest point does not fall within the line segment

	Vector3 Intersection = LineStart + U * ( LineEnd - LineStart );

	Distance = magnitude(Point - Intersection);

	return true;
}

struct Segment 
{
	Vector3 origin, dir;
	float extent;

	inline Segment(const Vector3& v0, const Vector3& v1)
	{
		origin = 0.5f * (v0 + v1);
		dir = v1 - v0;
		float m = magnitude(dir);
		extent = 0.5f * m;
		dir /= m;
	}
};


bool LineSphereTest(const Vector3& start, const Vector3& end, const Vector3& center, float radius, Points& points)
{
	int& count = points.quantity; 
	Vector3* point = points.pt;

	Segment segment(start, end);
	float segmentT[2];

	Vector3 diff = segment.origin - center;
	float a0 = dot(diff, diff) - sqr(radius);
	float a1 = dot(segment.dir, diff);
	float fDiscr = sqr(a1) - a0;
	if (fDiscr < 0.0f)
	{
		count = 0;
		return false;
	}

	float fTmp0 = sqr(segment.extent) + a0;
	float fTmp1 = (2.0f)*a1*segment.extent;
	float fQM = fTmp0 - fTmp1;
	float fQP = fTmp0 + fTmp1;
	float fRoot;
	if (fQM*fQP <= 0.0f)
	{
		fRoot = sqrtf(fDiscr);
		segmentT[0] = (fQM > 0.0f ? -a1 - fRoot : -a1 + fRoot);
		point[0] = segment.origin + segmentT[0] * segment.dir;
		count = 1;
		return true;
	}

	if (fQM > 0.0 && fabsf(a1) < segment.extent)
	{
		if (fDiscr >= 0.0001f)
		{
			fRoot = sqrtf(fDiscr);
			segmentT[0] = -a1 - fRoot;
			segmentT[1] = -a1 + fRoot;
			point[0] = segment.origin + segmentT[0] * segment.dir;
			point[1] = segment.origin + segmentT[1] * segment.dir;
			count = 2;
		}
		else
		{
			segmentT[0] = -a1;
			point[0] = segment.origin + segmentT[0] * segment.dir;
			count = 1;
		}
	}
	else
	{
		count = 0;
	}

	return count > 0;
}

bool LineSphereSimpleTest(const Vector3& start, const Vector3& end, const Vector3& center, float radius, Points& points)
{
	int& count = points.quantity; 
	Vector3* point = points.pt;

	Segment segment(start, end);
	float segmentT[2];

	Vector3 diff = segment.origin - center;
	float a0 = dot(diff, diff) - sqr(radius);
	float a1 = dot(segment.dir, diff);
	float fDiscr = sqr(a1) - a0;
	if (fDiscr < 0.0f)
	{
		count = 0;
		return false;
	}

	float fTmp0 = sqr(segment.extent) + a0;
	float fTmp1 = (2.0f)*a1*segment.extent;
	float fQM = fTmp0 - fTmp1;
	
	if (fQM > 0.0 && fabsf(a1) < segment.extent)
	{
		if (fDiscr >= 0.0001f)
		{
			float fRoot = sqrtf(fDiscr);
			segmentT[0] = -a1 - fRoot;
			segmentT[1] = -a1 + fRoot;
			point[0] = segment.origin + segmentT[0] * segment.dir;
			point[1] = segment.origin + segmentT[1] * segment.dir;
			count = 2;
		}
		else
		{
			count = 0;
		}
	}
	else
	{
		count = 0;
	}
	


	return count > 0;
}

void LineUniformSpheresTest(const LineSegment& s, const Vector3* centers, float radius, Points* result, int spheresCount)
{
	for(int i = 0; i < spheresCount; ++i)
	{
		//LineSphereTest(s.start, s.end,centers[i], radius, result[i]);
		LineSphereSimpleTest(s.start, s.end,centers[i], radius, result[i]);
	}
}

float CalculateLengthInMedia(const Vector3& point, const Vector3* centers, float radius, const Points* points, int spheresCount)
{
	float res = 0.0f;
	for(int i = 0; i < spheresCount; ++i)
	{
		for (int j = 0; j < points[i].quantity; ++j)
		{
			float mSq = magnitudeSq(point - points[i].pt[j]);
			if(res < mSq)
				res = mSq;
		}
	}

	return sqrtf(res);
}

void CalculateStaticLightning(const Vector3& lightPos, const Vector3* particles, float radius, float* lightning, int count, float density)
{
	Points* points = (Points*)malloc(count * sizeof(Points));
	memset(points, 0, count * sizeof(Points));
	for(int i = 0; i < count; ++i)
	{
		LineSegment s = {lightPos, particles[i]};
		LineUniformSpheresTest(s, particles, radius, points, count);
		float len = CalculateLengthInMedia(particles[i], particles, radius, points, count);
		lightning[i] = powf(density, len);
	}

	free(points);
}


// static const int numSamples = CloudSystem::lightningSamplesCount;
// Vector3 directions[numSamples];
// void precomputeDirections()
// {
// 	static bool inited = false;
// 	if(inited) return;
// 
// 	Vector3 normal(0.0, 1.0, 0.0);
// 	Vector3 cX(-1.0, 0.0, 0.0);
// 	Vector3 cY(0.0, 0.0, 1.0);
// 	for(int j = 0; j < numSamples; ++j)
// 	{
// 		//directions[j] = hemisphereUniform(normal, cX, cY);
// 		sphereUniformRand(directions[j]);
// 	}
// 
// 	inited = true;
// }

/*
void CalculateSHLightning(const Vector3* particles, float radius, Vector4* lightning, int count, float density)
{
	//PROFILE_THIS_SCOPE_MSG;
	Points* points = (Points*)malloc(count * sizeof(Points));
	memset(points, 0, count * sizeof(Points));

	CloudSystem::ParticleLightSamples::precomputeDirections();
	for(int i = 0; i < count; ++i)
	{
		Light::SH::SHSample<1> sh;
		for(int j = 0; j < numSamples; ++j)
		{
			Vector3& lightDir = directions[j];
			Vector3 lightPos = particles[i] + lightDir*1000.0f;
			LineSegment s = {lightPos, particles[i]};
			LineUniformSpheresTest(s, particles, radius, points, count);
			float len = CalculateLengthInMedia(particles[i], particles, radius, points, count);
			float energy = powf(density, len);
			sh.add(lightDir, energy);
		}
		sh.extract(&lightning[i].x, 4);
		lightning[i] *= (4 * R3D_PI) / numSamples;
	}

	free(points);
}*/


void CalculateLightSamples(const Vector3* particles, CloudSystem::ParticleLightSamples* samples, float radius, int count)
{
	Points* points = (Points*)malloc(count * sizeof(Points));
	memset(points, 0, count * sizeof(Points));

	CloudSystem::ParticleLightSamples::precomputeDirections();
	for(int i = 0; i < count; ++i)
	{
		for(int j = 0; j < CloudSystem::lightningSamplesCount; ++j)
		{
			Vector3& lightDir = CloudSystem::ParticleLightSamples::directions[j];
			Vector3 lightPos = particles[i] + lightDir*1000.0f;
			LineSegment s = {lightPos, particles[i]};
			LineUniformSpheresTest(s, particles, radius, points, count);
			samples[i].lengthInMedia[j] = CalculateLengthInMedia(particles[i], particles, radius, points, count);
		}
	}

	free(points);	
}

void CalculateSHLightning(const CloudSystem::ParticleLightSamples* samples, Vector4* lightning, int count, float density)
{
	for(int i = 0; i < count; ++i)
	{
		Light::SH::SHSample<1> sh;
		for(int j = 0; j < CloudSystem::lightningSamplesCount; ++j)
		{
			Vector3& lightDir = CloudSystem::ParticleLightSamples::directions[j];
			sh.add(lightDir, powf(density, samples[i].lengthInMedia[j]));
		}
		sh.extract(&lightning[i].x, 4);
		lightning[i] *= (4 * R3D_PI) / CloudSystem::lightningSamplesCount;
	}
}

void CalculateAmbientSH(Vector4& sh_)
{

 	Light::SH::SHSample<1> sh;
	sh.add( Vector3(0, 1, 0) );
// 	for(int j = 0; j < CloudSystem::lightningSamplesCount; ++j)
// 	{
// 		sh.add(CloudSystem::ParticleLightSamples::hemidirections[j]);
// 	}
// 
 	sh.extract(&sh_.x, 4);
 	//sh_ *= (4 * R3D_PI);// / CloudSystem::lightningSamplesCount; 
}

#endif