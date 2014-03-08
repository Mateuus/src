#ifndef ARTY_CLOUDS_LIGHTNING
#define ARTY_CLOUDS_LIGHTNING

#if 0

#include "SH.h"
#include "cloudsSystem.h"

struct Points
{
	Vector3 pt[2];
	int quantity;
	float length() const { return quantity == 2 ? magnitude(pt[1] - pt[0]) : 0.0f; }
};

struct LineSegment
{
	Vector3 start;
	Vector3 end;

	float length() const { return magnitude(end - start); }
};

void CalculateStaticLightning(const Vector3& lightPos, const Vector3* particles, float radius, float* lightning, int count, float density);
void CalculateLightSamples(const Vector3* particles, CloudSystem::ParticleLightSamples* samples, float radius, int count);
void CalculateSHLightning(const CloudSystem::ParticleLightSamples* samples, Vector4* lightning, int count, float density);
//void CalculateSHLightning(const Vector3* particles, float radius, Vector4* lightning, int count, float density);
void CalculateAmbientSH(Vector4& sh);

#endif

#endif

