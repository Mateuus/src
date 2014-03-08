#include "r3dPCH.h"
#if 0

#include "r3dConst.h"
#include "r3d.h"
#include "cloudsSystem.h"
#include "cloudBoxesGeneration.h"
#include "cloudsLightning.h"
#include "GameLevel.h"

#include "r3dDeviceQueue.h"

r3dScreenBuffer* gCloudShadow;
r3dScreenBuffer* rtTemp = 0;

float CloudSystem::viewDist = 10000.0f;	//1:50 to real world

short CloudSystem::cRemap[CloudSystem::nClouds*CloudSystem::nClouds], CloudSystem::cidx[CloudSystem::nClouds*CloudSystem::nClouds];	//for sorting clouds
short CloudSystem::pidx[CloudSystem::MaxParticlePerCloud];	//for sorting particles
float CloudSystem::dist2cam[ CloudSystem::MaxParticlePerCloud > CloudSystem::nClouds*CloudSystem::nClouds ?
	CloudSystem::MaxParticlePerCloud : CloudSystem::nClouds*CloudSystem::nClouds];	//for sorting particles and clouds

Vector3 CloudSystem::tileSize;
Vector3 CloudSystem::shift;
Base::Random CloudSystem::rnd;
CloudSystem::Plane CloudSystem::frustum[6];

Vector3 CloudSystem::ParticleLightSamples::directions[CloudSystem::lightningSamplesCount];
Vector3 CloudSystem::ParticleLightSamples::hemidirections[CloudSystem::lightningSamplesCount];


void CloudSystem::TQuickSorter::QSort(short* ArrayToSort, int uCount)
{
	if (uCount <= 0) return;
	QSortRecursive (ArrayToSort, 0, (uCount-1));
}

void CloudSystem::TQuickSorter::QSortRecursive(short* pArr, int d, int h)
{
 int i,j;
 short str;

begin:

	i = h;
	j = d;

	str = pArr[(d+h)>>1];

	do 
	{
		while (CloudSystem::dist2cam[ pArr[j] ] > CloudSystem::dist2cam[str]  && (j < h)) ++j;
		while (CloudSystem::dist2cam[str] > CloudSystem::dist2cam[ pArr[i] ] && (i > d)) --i;

		if ( i >= j )
		{
			if ( i != j )
			{
				short zal;
				zal = pArr[i];
				pArr[i] = pArr[j];
				pArr[j] = zal;
			}
			--i;
			++j;
		}
	} while (j <= i);

	if (d < i) QSortRecursive(pArr,d,i);

	if (j < h)
	{
		d = j;
		goto begin;
	}
}




//pack float2 -> float1
float pack2(float f0, float f1)
{
	int i0 = int(f0*4096.0f);
	int i1 = int(f1*4096.0f);
	return i0*4096.0f + i1;
}

CloudSystem::RefCloud::RefCloud()
{
	particlesPos = 0;
	lightningSH = 0;
	params = 0;
	lightSamples = 0;
	boxSize_ = D3DXVECTOR3(0,0,0);
	nParticles = 0;
	cloudNum = 0;
}

CloudSystem::RefCloud::~RefCloud()
{
	clear();
}

void CloudSystem::RefCloud::clear()
{
	if(lightSamples) delete [] lightSamples;
	if(particlesPos) delete [] particlesPos;
	if(lightningSH) delete [] lightningSH;
	if(params) delete [] params;

	particlesPos = 0;
	lightningSH = 0;
	params = 0;
	lightSamples = 0;
	nParticles = 0;
}

// void CloudSystem::setTexIdxToRef(LPDIRECT3DTEXTURE9 cloudRefTex, int refIdx, int texIdx)
// {
// 
// }

void CloudSystem::RefCloud::createData(float particlesDensity, const Vector3& size)
{
	clear();

	particlesPos = new Vector3[MaxParticlePerCloud];

	CloudBoxes::Box bbox;
	CloudBoxes::Box nbox(0.0f, 15.999f);
	nParticles = CloudBoxes::generatePoints(particlesDensity, MaxParticlePerCloud, size, particlesPos, nbox, bbox);
	bbox.size().store(&boxSize_.x);

	lightSamples = new ParticleLightSamples[nParticles];
	lightningSH  = new Vector4[nParticles];
	params		 = new Vector4[nParticles];

	boxSize_ *= 60.0f;
	for( int p = 0; p < nParticles; ++p )
	{
		particlesPos[p] *= 60.0f;
	}
}

void CloudSystem::RefCloud::setTexIdx(int texIdx)
{
	float tidx = (float)texIdx;
	if(tidx < 0) tidx = 0;
	for( int p = 0; p < nParticles; ++p )
	{
		/*
A: x,y
R: z. idx, size
G: prt[0], prt[1]
B: prt[2], prt[3]
		*/
		const Vector3& pa = particlesPos[p];
		params[p].x = pack2(pa.z/(16.0f*60.0f), tidx/63.0f);
	}
}

void CloudSystem::RefCloud::generate(int cn, int texIdx, float particlesDensity, float opticalDensity, const Vector3& size)
{
	clear();
	assert(cn >= 0 && cn < maxRefClouds);
	float tidx = (float)texIdx;

	cloudNum = cn;

	createData(particlesDensity, size);
				 	
	CalculateLightSamples(particlesPos, lightSamples, 1.0f, nParticles);
	CalculateSHLightning(lightSamples, lightningSH, nParticles, opticalDensity);

	//float tidx = 0.0f;
	for( int p = 0; p < nParticles; ++p )
	{
		/*
			A: x,y
			R: z. idx, size
			G: prt[0], prt[1]
			B: prt[2], prt[3]
		*/

		//avoid pack problems by shifting particle's coor to positive values
		const Vector3& pa = particlesPos[p];
		params[p].w = pack2(pa.x/(16.0f*60.0f), pa.y/(16.0f*60.0f));
		params[p].x = pack2(pa.z/(16.0f*60.0f), tidx/64.0f);
		params[p].y = pack2( (lightningSH[p].x+4.0f)/8.0f, (lightningSH[p].y+4.0f)/8.0f);
		params[p].z = pack2( (lightningSH[p].z+4.0f)/8.0f, (lightningSH[p].w+4.0f)/8.0f);
	}
}

void CloudSystem::RefCloud::calculateLightning(float opticalDensity)
{
	CalculateSHLightning(lightSamples, lightningSH, nParticles, opticalDensity);

	for( int p = 0; p < nParticles; ++p )
	{
		/*
A: x,y
R: z. idx, size
G: prt[0], prt[1]
B: prt[2], prt[3]
		*/
		params[p].y = pack2( (lightningSH[p].x+4.0f)/8.0f, (lightningSH[p].y+4.0f)/8.0f);
		params[p].z = pack2( (lightningSH[p].z+4.0f)/8.0f, (lightningSH[p].w+4.0f)/8.0f);
	}
}

void CloudSystem::RefCloud::save(FILE* file)
{
	fwrite(&cloudNum, sizeof(cloudNum), 1, file);
	fwrite(&nParticles, sizeof(nParticles), 1, file);
	fwrite(&boxSize_, sizeof(boxSize_), 1, file);

	fwrite(params, sizeof(Vector4) * nParticles, 1, file);
	fwrite(particlesPos, sizeof(Vector3)* nParticles, 1, file);
	fwrite(lightningSH, sizeof(Vector4)* nParticles, 1, file);
	fwrite(lightSamples, sizeof(ParticleLightSamples)* nParticles, 1, file);
	
}

void CloudSystem::RefCloud::load(r3dFile* file)
{
	clear();
	fread(&cloudNum, sizeof(cloudNum), 1, file);
	fread(&nParticles, sizeof(nParticles), 1, file);
	fread(&boxSize_, sizeof(boxSize_), 1, file);
	
	lightSamples = new ParticleLightSamples[nParticles];
	particlesPos = new Vector3[nParticles];
	lightningSH  = new Vector4[nParticles];
	params		 = new Vector4[nParticles];

	fread(params, sizeof(Vector4)* nParticles, 1, file);
	fread(particlesPos, sizeof(Vector3)* nParticles, 1, file);
	fread(lightningSH, sizeof(Vector4)* nParticles, 1, file);
	fread(lightSamples, sizeof(ParticleLightSamples)* nParticles, 1, file);
}

void CloudSystem::RefCloud::init(Color* ln)
{
	for( int p = 0; p < nParticles; ++p )
	{
		ln[p].a = params[p].w;
		ln[p].r = params[p].x;
		ln[p].g = params[p].y;
		ln[p].b = params[p].z;
	}
}

static Base::Random rnd;

Vector3 hemisphereUniform(const Vector3& normal, const Vector3& cX, const Vector3& cY)
{
	float phi = R3D_PI * 2.0f * rnd.urnd();
	float cos_phi = cosf(phi);
	float sin_phi = sinf(phi);
	float cos_theta = rnd.urnd();
	float sin_theta = sqrtf(1.0f - cos_theta*cos_theta);

	//theta = acosf(cos_theta);

	//rotate
	Vector3 dir;
	dir = cos_phi*cX + sin_phi*cY;
	dir = sin_theta*dir + cos_theta*normal;
	return dir;
}

void sphereUniformRand(Vector3& v)
{
	float z = rnd.srnd();
	float r = sqrtf(1 - z*z);
	float t = rnd.urnd() * R3D_PI * 2.0f;

	v.x = cosf(t) * r;
	v.y = sinf(t) * r;
	v.z = z;
}

void CloudSystem::ParticleLightSamples::precomputeDirections()
{
	static bool inited = false;
	if(inited) return;

	Vector3 normal(0.0, 1.0, 0.0);
	Vector3 cX(-1.0, 0.0, 0.0);
	Vector3 cY(0.0, 0.0, 1.0);
	for(int j = 0; j < lightningSamplesCount; ++j)
	{
		hemidirections[j] = hemisphereUniform(normal, cX, cY);
		directions[j] = hemisphereUniform(normal, cX, cY);
		//sphereUniformRand(directions[j]);
	}

	inited = true;
}

CloudSystem::Cloud::Cloud()
{
	scale = 0;
	particlesRotation = 0;
	vrtBuff = 0;
}

CloudSystem::Cloud::~Cloud()
{
	clear();
}

// void CloudSystem::Cloud::setPosition(const Vector3& p, const Vector3& tileSize)
// {
// 	realPos = pos = p;
// 	
// // 	pos.x = fmod(pos.x, tileSize.x);
// // 	pos.z = fmod(pos.z, tileSize.z);
// 
// 	while(pos.x < 0.0f) pos.x += tileSize.x;
// 	while(pos.z < 0.0f) pos.z += tileSize.z;
// }


void CloudSystem::Cloud::bbox(r3dBox3D& box) const
{
	box.Size = r3dPoint3D(cloud->boxSize_.x, cloud->boxSize_.y, cloud->boxSize_.z) * scale * 1.5f;
	box.Org = r3dPoint3D(pos.x, pos.y, pos.z) - box.Size * 0.5f;
}

bool CloudSystem::Cloud::intersect(const r3dPoint3D& vStart, const r3dPoint3D& vDir, float* dist) const
{
	r3dBox3D box; bbox(box);
	r3dPoint3D vDirN = vDir;
	vDirN.Normalize();

	float d;
	int res = box.ContainsRay(vStart, vDirN, 25000.0f, &d);
	if(res && dist)
	{
		*dist = d;
	}

	return res != 0;
}

void CloudSystem::Cloud::clear()
{
	if(particlesRotation) delete [] particlesRotation;
	if(vrtBuff) delete [] vrtBuff;	
	particlesRotation = 0;
	vrtBuff = 0;
}

void CloudSystem::Cloud::flushTo(Color& inst)
{
	Vector3 p = pos;

	// 	pos.x = fmod(pos.x, tileSize.x);
	// 	pos.z = fmod(pos.z, tileSize.z);

	while(p.x < 0.0f) p.x += tileSize.x;
	while(p.z < 0.0f) p.z += tileSize.z;

// 	inst.a = pos.x;
// 	inst.g = pos.y;
	inst.a = pack2(p.x/100000.0f, p.y/100000.0f);
	inst.g = pack2(scale/100.0f, bboxSize().y/(16.0f*60.0f));
	inst.b = pack2(p.z/100000.0f, rotate/(R3D_PI*2.0f));
	inst.r = r;
}

// void CloudSystem::Cloud::save(FILE* file)
// {
// 	fwrite(&posIdx, sizeof(posIdx), 1, file);
// 	fwrite(&refIdx, sizeof(refIdx), 1, file);
// 	fwrite(&pos, sizeof(pos), 1, file);
// 	fwrite(&wp, sizeof(wp), 1, file);
// 	fwrite(&r, sizeof(r), 1, file);
// }
// 
// void CloudSystem::Cloud::load(r3dFile* file, RefCloud* refClouds)
// {
// 	fread(&posIdx, sizeof(posIdx), 1, file);
// 	fread(&refIdx, sizeof(refIdx), 1, file);
// 	fread(&pos, sizeof(pos), 1, file);
// 	fread(&wp, sizeof(wp), 1, file);
// 	fread(&r, sizeof(r), 1, file);
// }

void CloudSystem::Cloud::init(const Cloud::InitData& data)
{
	//_CrtCheckMemory();
	clear();

	rotate = data.rotate;
	scale = data.scale;
	posIdx = data.posIdx;
	refIdx = data.refIdx;
	r = data.r;
	pos = wp = data.pos;
	cloud = data.ref;

	prevCamPos = Vector3(0,0,0);
	curLod = prevLod = 0.0f;
	particlesRotation = new float[cloud->nParticles];
	vrtBuff = new InstanceVertex[ cloud->nParticles ];
	for(int p=0; p<cloud->nParticles; p++)
	{
		particlesRotation[p] = rnd.urnd();	//rotation
	}

	//_CrtCheckMemory();

}

int CloudSystem::atlasSize() const
{
	return atlas.count;
}

//LOD and visibility computation
int CloudSystem::Cloud::updatePos(/*const Base::Frustum& frustum, */const Vector3& cameraPos, float fovH, float fovV)
{
	//_CrtCheckMemory();
	float invSinHalfFov = 1.0f / sinf( 0.5f*std::min(fovH, fovV) );
	float optimalDistance = invSinHalfFov * magnitude(bboxSize());

	wp = pos;
	wp += shift;
	wp.x += cameraPos.x - fmod(cameraPos.x, tileSize.x);
	wp.z += cameraPos.z - fmod(cameraPos.z, tileSize.z);

	Vector3 dx = cameraPos - wp;
	if(fabs(dx.x)>viewDist)	wp.x += dx.x < 0.0f ? -tileSize.x : tileSize.x;
	if(fabs(dx.z)>viewDist)	wp.z += dx.z < 0.0f ? -tileSize.z : tileSize.z;

	if( fabs(cameraPos.x - wp.x) > viewDist || fabs(cameraPos.z - wp.z) > viewDist )	return 0;

	//if earth curvature is required. Reflect this code into shader
	/*float earthRadius = 1000.0f;
	Vector3 earthCenter(cameraPos.x, -earthRadius, cameraPos.z);
	wp += (earthCenter - wp).GetNormalized()*( (wp-earthCenter).Magnitude() - earthRadius);
	wp.y += 30.0f;*/

	if( !frustumIsVisible(wp + bboxSize() * 0.5f, magnitude(bboxSize())) )	return 0;

	//[1..0)
	curLod = optimalDistance / magnitude(wp - cameraPos);

	//Eliminate fill-rate problem when a cloud is too near by computing LOD from integrating overdraw
	if(curLod>1.0f)
	{
		float sq = 0.0f;
		for(int p=0; p<cloud->nParticles; p++)
		{
			Vector3 pp = wp+cloud->particlesPos[p] * scale;
			if( frustumIsVisible(pp, 20.0f) == false)	continue;
			sq += optimalDistance / magnitude(pp - cameraPos);
			//sq += 1.0f;
		}
		if(sq>0.0f)
		{
			//LOG_MSG(sq);
			curLod = pow(4000.0f/sq, 0.5f);
		}
	}

	if(curLod>1.0f)	curLod = 1.0f;
	curLod = powf(curLod, 2.0f);

	//_CrtCheckMemory();
	activeParticles = int(float(cloud->nParticles)*curLod);
	return activeParticles;
}

inline float dot(const Vector3& v0, const Vector3& v1)
{
	return D3DXVec3Dot(&v0, &v1);
}
inline Vector3 normalize(const Vector3& v0)
{
	D3DXVECTOR3 v;
	D3DXVec3Normalize(&v, &v0);
	return v;
}

Vector3 CloudSystem::Cloud::bboxSize() const
{
	return cloud->boxSize_ * scale;
}

int CloudSystem::Cloud::sort(const Vector3& sunDir, float dt, InstanceVertex* vs, const Vector3& cameraPos)
{
	//_CrtCheckMemory();
	int np = activeParticles;//int(float(cloud->nParticles)*curLod);


	float r = float(int(rotate * 4096.0f))/4096.0f;
	float sangle = sinf(r);
	float cangle = cosf(r);

	float sc = float(int(scale/100.0f * 4096.0f))/4096.0f*100.0f;

	Vector3 ww = wp / 100000.0f * 4096.0f;
	ww.x = float(int(ww.x));
	ww.y = float(int(ww.y));
	ww.z = float(int(ww.z));
	ww = ww / 4096.0f * 100000.0f;



	Vector3 locCam = cameraPos - ww;
	bool cloudIsNear = magnitudeSq(locCam) < magnitudeSq(bboxSize())*3.0f;
	float cs = (locCam.x*prevCamPos.x + locCam.y*prevCamPos.y + locCam.z*prevCamPos.z) / sqrt ( magnitudeSq(locCam) * magnitudeSq(prevCamPos) );
	if(cloudIsNear==false && cs>0.995f && prevLod/curLod < 1.01f && curLod/prevLod < 1.01f)
	{
		int pp = int(float(cloud->nParticles)*prevLod);
		pp = std::min(activeParticles, pp);
		memcpy(vs, &vrtBuff[0], pp * sizeof(InstanceVertex));
		return pp;
	}
	prevLod = curLod;


	//skip rotation compensation for clouds not under/over camera
	if(cloudIsNear==false)
	{
		//_CrtCheckMemory();
		for(int p=0; p<np; p++)
		{
			Vector3 pos = cloud->particlesPos[p];
			float x = pos.x;
			float z = pos.z;

			pos.x = cangle * x + sangle * z;
			pos.z = -sangle * x + cangle * z;
   
			pidx[p] = p;
			dist2cam[p] = magnitudeSq( locCam - pos*scale );
		}
		//_CrtCheckMemory();
	}
	else
	{
		//np = 1;

		for(int p=0; p<np; p++)
		{
			Vector3 pos = cloud->particlesPos[p];

			pos = pos / (16.0f*60.0f)*4096.0f;
			pos.x = float(int(pos.x));
			pos.y = float(int(pos.y));
			pos.z = float(int(pos.z));
			pos = pos / 4096.0f*(16.0f*60.0f) * sc;

			Vector3 ppos;
			ppos.x = cangle * pos.x + sangle * pos.z;
			ppos.y = pos.y;
			ppos.z = -sangle * pos.x + cangle * pos.z;


			pidx[p] = p;
			dist2cam[p] = magnitudeSq( locCam - ppos );

			//ppos += ww;

			//compensate rotation of particles for camera movement
			Vector2 v0( prevCamPos.x - ppos.x, prevCamPos.z - ppos.z);
			Vector2 v1( locCam.x - ppos.x, locCam.z - ppos.z);
			float cs = (v0.x*v1.x + v0.y*v1.y) / sqrt ((v0.x*v0.x + v0.y*v0.y) * (v1.x*v1.x + v1.y*v1.y));
			if(cs<-1.0f)	cs = -1.0f;
			if(cs> 1.0f)	cs =  1.0f;
			float a = acosf( cs );
			if( v0.y*v1.x - v0.x*v1.y < 0.0f)	a = -a;

			Vector3 dir = ppos - locCam;
			float l = magnitude(dir);
			float da = -dir.y/l * a;
			if(l<2.0f)	da *= l*0.5f;

			//randomly rotate particles with random angle speed to make clouds "live"
			//do not rotate particles near to light dir to eliminate visual problems
			//da += float(15 - (p%30) )/100.0f*dt * powf(1.0f-dot(sunDir, normalize(ppos-locCam)), 0.5f);
			//da += float(15 - (p%30) )/100.0f*dt;	

			//TODO: optimize
			float ang = particlesRotation[p] + da/2.0f/R3D_PI;
			if(ang < 0.0f)	ang += 1.0f;
			if(ang >= 1.0f)	ang -= 1.0f;

			particlesRotation[p] = ang;
		}
		//_CrtCheckMemory();
	}
	prevCamPos = locCam;

 	//_CrtCheckMemory();
 
 	TQuickSorter Sorter;
 	Sorter.QSort(&pidx[0], np);
 
 
 	//_CrtCheckMemory();
 	for(int p=0; p<np; p++)
 	{
 		//vrtBuff[p].uv2<0>().u = pack2((float(pidx[p])+0.5f)/MaxParticlePerCloud, float(posIdx)/(nClouds*nClouds));
 		//particle index
 		vrtBuff[p].u = (float(pidx[p])+0.5f)/MaxParticlePerCloud;
 		//cloud reference
 		vrtBuff[p].s = float(posIdx)/(nClouds*nClouds);
 
 		//base alpha value is 0.25 and increases with the LOD
 		float opacity = 0.25f/powf(curLod, 0.25f);
 		if(float(pidx[p]+1)/np > 0.7f)	opacity *= (1.0f-float(pidx[p]+1)/np)/(1.0f-0.7f);
 
 		//vrtBuff[p].uv4<0>().v = pack2( particlesRotation[pidx[p]],  opacity / 16.0f);
 		vrtBuff[p].v = particlesRotation[pidx[p]];
 		vrtBuff[p].q = opacity;
 
 	}
 	memcpy(vs, &vrtBuff[0], np *sizeof(InstanceVertex));
 
 	//_CrtCheckMemory();

	return np;
}

const float vdScale = 0.25f;

int CloudSystem::Cloud::prepareShadow(const Vector3& cameraPos)
{
	if( fabs(cameraPos.x - wp.x) > viewDist * vdScale || fabs(cameraPos.z - wp.z) > viewDist * vdScale )	return 0;
	return int(float(cloud->nParticles)*0.1f);
}

int CloudSystem::Cloud::draw2Shadow(InstanceVertex* vs, const Vector3& cameraPos)
{
	if( fabs(cameraPos.x - wp.x) > viewDist * vdScale || fabs(cameraPos.z - wp.z) > viewDist * vdScale )	return 0;

	int np = int(float(cloud->nParticles)*0.1f);
	for(int p=0; p<np; p++)
	{
		vs[p].u = (float(p)+0.5f)/MaxParticlePerCloud;	//particle index
		vs[p].s = float(posIdx)/(nClouds*nClouds);	//cloud reference
	}
	return np;
}

bool CloudSystem::frustumIsVisible(const Vector3& pos, float radius)
{
	float negRadius = -radius;
	return !(	
		( dot(frustum[0].n, pos) + frustum[0].d < negRadius )  ||
		( dot(frustum[1].n, pos) + frustum[1].d < negRadius )  ||
		( dot(frustum[2].n, pos) + frustum[2].d < negRadius )  ||
		( dot(frustum[3].n, pos) + frustum[3].d < negRadius )  ||
		( dot(frustum[4].n, pos) + frustum[4].d < negRadius )  ||
		( dot(frustum[5].n, pos) + frustum[5].d < negRadius ));

}

void CloudSystem::finalizeCloudsData()
{
	for (int i = 0; i < cloudCursor; ++i)
	{
		Cloud::InitData& data = cloudsData[i];
		r3d_assert(data.refIdx>=0 && data.refIdx < maxRefClouds);
		data.ref = &refClouds[ data.refIdx ];
	}	
}

void CloudSystem::reinitClouds()
{
	for (int i = 0; i < cloudCursor; ++i)
	{
		clouds[i].init( cloudsData[i] );
	}
}

void CloudSystem::deleteCloud(int idx)
{
	int lastIdx = cloudCursor - 1;
	assert(lastIdx>=idx && idx >= 0);
	if(lastIdx != idx)
	{
		cloudsData[idx] = cloudsData[lastIdx];
	}

	--cloudCursor;	
	computeTileSize();
	reinitClouds();
}

r3dBox3D CloudSystem::getCloudBox(int idx) const
{
	r3dBox3D res;
	clouds[idx].bbox(res);
	return res;
}


int CloudSystem::cloudsCount() const
{
	return cloudCursor;
}

void CloudSystem::addCloud(int refIdx, const Vector3& pos)
{
	if(cloudCursor < nClouds*nClouds)
	{
		int idx = cloudCursor++;
		Cloud::InitData& cloud = cloudsData[idx];
		cloud.posIdx = idx;
		cloud.scale = u_GetRandom(0.5f, 2.0f);
		cloud.r = (float(idx)+0.5f)/refCloudsCount;
		cloud.refIdx = refIdx;
		cloud.ref = &refClouds[refIdx];

		setCloudPos(idx, pos);
		computeTileSize();
		reinitClouds();
	}	
}

void CloudSystem::computeTileSize()
{
	Vector3 minVal(FLT_MAX, FLT_MAX, FLT_MAX), maxVal(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < cloudCursor; ++i)
	{
		Cloud::InitData& cloud = cloudsData[i];
		
		if(minVal.x > cloud.pos.x) minVal.x = cloud.pos.x;
		if(minVal.y > cloud.pos.y) minVal.y = cloud.pos.y;
		if(minVal.z > cloud.pos.z) minVal.z = cloud.pos.z;

		if(maxVal.x < cloud.pos.x) maxVal.x = cloud.pos.x;
		if(maxVal.y < cloud.pos.y) maxVal.y = cloud.pos.y;
		if(maxVal.z < cloud.pos.z) maxVal.z = cloud.pos.z;		
	}

	float minTileSize = viewDist * 2.1f;

	tileSize = maxVal - minVal;
	tileSize.y = 0.0f;
	if(tileSize.x < minTileSize) tileSize.x = minTileSize;
	if(tileSize.z < minTileSize) tileSize.z = minTileSize;
}

void CloudSystem::generateCloudsMap(float coverage, float* minHeight, float* randomHeight, LPDIRECT3DTEXTURE9 cloudPosTex)
{
	seed(coverage, minHeight, randomHeight, nClouds, clouds);
	refreshCloudsMap(cloudPosTex);
}

void CloudSystem::refreshCloudsMap(LPDIRECT3DTEXTURE9 cloudPosTex)
{
	if(refCloudsCount > 0)
	{
		R3D_ENSURE_MAIN_THREAD();

		IDirect3DTexture9* sysTex;
		D3DLOCKED_RECT lr;
		//seeding clouds to the sky
		D3D_V( r3dRenderer->pd3ddev->CreateTexture(nClouds,nClouds,1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &sysTex, 0) );

		sysTex->LockRect(0, &lr, 0, 0);
		for(int c=0; c < cloudCursor; c++)
		{
			clouds[c].flushTo(((Color*)lr.pBits)[c]);
		}
		sysTex->UnlockRect(0);

		r3dRenderer->pd3ddev->UpdateTexture(sysTex, cloudPosTex);
		sysTex->Release();
	}
}

void CloudSystem::refresh(LPDIRECT3DTEXTURE9 cloudRefTex)
{
	if(refCloudsCount > 0)
	{
		R3D_ENSURE_MAIN_THREAD();

		IDirect3DTexture9* sysTex;
		D3DLOCKED_RECT lr;
		D3D_V( r3dRenderer->pd3ddev->CreateTexture(MaxParticlePerCloud, refCloudsCount, 1, 0, D3DFMT_A32B32G32R32F, D3DPOOL_SYSTEMMEM, &sysTex, 0) );
		sysTex->LockRect(0, &lr, 0, 0);
		for(int c=0; c<refCloudsCount; c++)
		{
			char* p = &((char*)lr.pBits)[c*lr.Pitch];
			refClouds[c].init((Color*)p);
		}
		sysTex->UnlockRect(0);

		r3dRenderer->pd3ddev->UpdateTexture(sysTex, cloudRefTex);
		sysTex->Release();
	}
}

int CloudSystem::pickCloud()
{
	int imx, imy;
	Mouse->GetXY(imx,imy);

	float vx, vy, vw, vh;
	r3dRenderer->GetBackBufferViewport( &vx, &vy, &vw, &vh );

	float mx = imx - vx;
	float my = imy - vy;

	r3dPoint3D dir;
	r3dScreenTo3D(mx, my, &dir);

	int res = -1;
	float dist = FLT_MAX;
	for (int i = 0; i < cloudCursor; ++i)
	{
		extern r3dCamera gCam;
		float d = FLT_MAX;
		clouds[i].intersect(gCam, dir, &d);

		if(d < dist)
		{
			dist = d;
			res = i;
		}
	}

	return res;
}

float CloudSystem::getCloudScale(int idx) const
{
	return clouds[idx].scale;
}

void CloudSystem::setCloudScale(int idx, float scale)
{
	clouds[idx].scale = scale;
	cloudsData[idx].scale = scale;
}

float CloudSystem::getCloudRotate(int idx) const
{
	return clouds[idx].rotate;
}

void CloudSystem::setCloudRotate(int idx, float rotate)
{
	clouds[idx].rotate = rotate;
	cloudsData[idx].rotate = rotate;
}

Vector3 CloudSystem::getCloudPos(int idx) const
{
	return clouds[idx].pos;
}

void CloudSystem::setCloudPos(int idx, const Vector3& pos)
{
	clouds[idx].pos = pos;
	cloudsData[idx].pos = pos;
	
	computeTileSize();
	//clouds[idx].setPosition(pos, tileSize);
	reinitClouds();
}

void CloudSystem::generate(LPDIRECT3DTEXTURE9 cloudRefTex, const int* texIdx, const float* particlesDensity, const float* opticalDensity, const Vector3* sizes)
{
	for(int c=0; c < refCloudsCount; c++)
	{
		refClouds[c].generate(c, texIdx[c], particlesDensity[c], opticalDensity[c], sizes[c]);
	}
	refresh(cloudRefTex);

	computeTileSize();
	finalizeCloudsData();
	reinitClouds();
}

void CloudSystem::setTexIdxToRef(LPDIRECT3DTEXTURE9 cloudRefTex, int refIdx, int texIdx)
{
	refClouds[refIdx].setTexIdx(texIdx);
	refresh(cloudRefTex);
}

// void CloudSystem::setTex(LPDIRECT3DTEXTURE9 cloudRefTex, const int* texIdx, const float* particlesDensity, const float* opticalDensity, const Vector3* sizes)
// {
// 	for(int c=0; c < refCloudsCount; c++)
// 	{
// 		refClouds[c].generate(c, texIdx[c], particlesDensity[c], opticalDensity[c], sizes[c]);
// 	}
// 	refresh(cloudRefTex);
// 
// 	computeTileSize();
// 	finalizeCloudsData();
// 	reinitClouds();
// }

// void CloudSystem::refreshRefClouds(LPDIRECT3DTEXTURE9 cloudRefTex)
// {
// 	refresh(cloudRefTex);
// 
// 	computeTileSize();
// 	finalizeCloudsData();
// 	reinitClouds();
// }

void CloudSystem::calculateLightning(LPDIRECT3DTEXTURE9 cloudRefTex, const float* opticalDensity)
{
	for(int c=0; c<refCloudsCount; c++)
	{
		refClouds[c].calculateLightning(opticalDensity[c]);
	}
	refresh(cloudRefTex);
}

extern r3dScreenBuffer*	gBuffer_Depth;  // R32 - depth

void SetD3DResourcePrivateData(LPDIRECT3DRESOURCE9 res, const char* FName);

void CloudSystem::render(float dt, r3dTexture* cloudRefTex, r3dTexture* cloudPosTex, const Vector3& cameraPos, float lodK, const D3DXMATRIX& view, const D3DXMATRIX& proj, const Vector3& lightDir, const Vector3& sunColor, const Vector3& ambColor, const D3DXVECTOR4& sunSh, const D3DXVECTOR4& ambSh, const Vector4* pointPos, const Vector4* pointColor, int pointCount, float fovH, float fovV, const Vector3& topColor, const Vector3& bottomColor, float rimArea, float rimPower)
{	
	if(cloudCursor <= 0 || refCloudsCount <= 0 || !r3dRenderer->SupportsVertexTextureFetch ) 
		return;

	R3DPROFILE_FUNCTION("CloudSystem::render");

	//move clouds to the desired direction
	shift.x += dt*10.0f;
	shift.z += dt*10.0f;

	shift.x = 0;
	shift.z = 0;



	//frustum culling clouds
	D3DXMATRIX mtx;
	D3DXMatrixMultiply(&mtx, &view, &proj);
	frustum[0].n.x = mtx(0,3)+mtx(0,0);	frustum[0].n.y = mtx(1,3)+mtx(1,0);	frustum[0].n.z = mtx(2,3)+mtx(2,0);	frustum[0].d = mtx(3,3)+mtx(3,0);
	frustum[1].n.x = mtx(0,3)-mtx(0,0);	frustum[1].n.y = mtx(1,3)-mtx(1,0);	frustum[1].n.z = mtx(2,3)-mtx(2,0);	frustum[1].d = mtx(3,3)-mtx(3,0);
	frustum[2].n.x = mtx(0,3)+mtx(0,1);	frustum[2].n.y = mtx(1,3)+mtx(1,1);	frustum[2].n.z = mtx(2,3)+mtx(2,1);	frustum[2].d = mtx(3,3)+mtx(3,1);
	frustum[3].n.x = mtx(0,3)-mtx(0,1);	frustum[3].n.y = mtx(1,3)-mtx(1,1);	frustum[3].n.z = mtx(2,3)-mtx(2,1);	frustum[3].d = mtx(3,3)-mtx(3,1);
	frustum[4].n.x = mtx(0,2);	frustum[4].n.y = mtx(1,2);	frustum[4].n.z = mtx(2,2);	frustum[4].d = mtx(3,2);
	frustum[5].n.x = mtx(0,3)-mtx(0,2);	frustum[5].n.y = mtx(1,3)-mtx(1,2);	frustum[5].n.z = mtx(2,3)-mtx(2,2);	frustum[5].d = mtx(3,3)-mtx(3,2);
	for(int i = 0; i < 6; ++i)
	{
		float l = 1.0f/magnitude(frustum[i].n);
		frustum[i].n *= l;
		frustum[i].d *= l;
	}

	//compute size of instance buffer required and sort clouds back-to-front
	int ci=0;
	int curPoint = 0;	//num particles to draw
	for(int c=0; c < cloudCursor; c++)
	{
		int np = clouds[c].updatePos(cameraPos,fovH, fovV);
		if(np==0)	continue;

		curPoint += np;

		cRemap[ci] = c;
		cidx[ci] = ci;
		dist2cam[ci] = magnitudeSq( cameraPos - clouds[c].wp );
		ci++;
	}
	TQuickSorter Sorter;
	Sorter.QSort(&cidx[0], ci);

	//resize instance buffer to larger size
	if(curPoint > szInstBuff)
	{
		if(szInstBuff>0)	instVrtBuff.ReleaseAndReset();
		r3dRenderer->Stats.BufferMem -= szInstBuff;

		r3dDeviceTunnel::CreateVertexBuffer(sizeof(InstanceVertex)*curPoint, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &instVrtBuff );
		szInstBuff = curPoint;
		r3dRenderer->Stats.BufferMem += szInstBuff;
	}




	//remember previous settings of device
	LPDIRECT3DVERTEXDECLARATION9 decl;
	r3dRenderer->pd3ddev->GetVertexDeclaration(&decl);

	//setup streams
	d3dc._SetIndices(idxBuff.Get());
	d3dc._SetStreamSource(0, quadVrtBuff.Get(), 0, sizeof(QuadVertex));

	r3dRenderer->SetTex( cloudTex, 0 );
	r3dRenderer->SetTex( gBuffer_Depth->Tex, 1 );
	r3dRenderer->SetTex(cloudRefTex, D3DVERTEXTEXTURESAMPLER2);
	r3dRenderer->SetTex(cloudPosTex, D3DVERTEXTEXTURESAMPLER3);

	for (unsigned int i = 0; i < 2; ++i)
	{
		r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		r3dRenderer->pd3ddev->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	}

	DWORD alphaTest;
	DWORD alphaBlend;
	DWORD srcBlend;
	DWORD dstBlend;
	DWORD blendOp;
	DWORD blendOpAlpha;

	r3dRenderer->pd3ddev->GetRenderState(D3DRS_ALPHATESTENABLE, &alphaTest);
	r3dRenderer->pd3ddev->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
	r3dRenderer->pd3ddev->GetRenderState(D3DRS_SRCBLEND, &srcBlend);
	r3dRenderer->pd3ddev->GetRenderState(D3DRS_DESTBLEND, &dstBlend);
	r3dRenderer->pd3ddev->GetRenderState(D3DRS_BLENDOP, &blendOp);
	r3dRenderer->pd3ddev->GetRenderState(D3DRS_BLENDOPALPHA, &blendOpAlpha);

	r3dRenderer->SetCullMode( D3DCULL_NONE );

	r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);



	//set consts to be shared in rendering of shadow and main view
	D3DXVECTOR4 cp(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);
	r3dRenderer->pd3ddev->SetVertexShaderConstantF(8, &cp.x, 1);

	D3DXVECTOR4 ts(tileSize.x, viewDist, tileSize.z, float(nClouds));
	r3dRenderer->pd3ddev->SetVertexShaderConstantF(12, &ts.x, 1);

	D3DXVECTOR4 shift1(shift.x, 0.0f, shift.z, 0.0f);
	r3dRenderer->pd3ddev->SetVertexShaderConstantF(14, &shift1.x, 1);

	IDirect3DSurface9* prevRT[4];
	for(int i=0; i<2; i++)	r3dRenderer->GetRT(i, &prevRT[i]);
	D3DVIEWPORT9 prevVP;
	r3dRenderer->pd3ddev->GetViewport(&prevVP);


	//if any clouds needs to be rendered into the main camera's view
	if(curPoint > 0)
	{
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

 		r3dRenderer->SetPixelShader(psId);
 		r3dRenderer->SetVertexShader(vsId);

// 		int dbgCounter = 0;
// 		for(int c=0; c<ci; c++)
// 		{
// 			Cloud& cld = clouds[ cRemap[cidx[c]] ];
// 			dbgCounter += cld.activeParticles;
// 		}
// 
// 		assert(curPoint == dbgCounter);

		InstanceVertex* vrtData;
		instVrtBuff->Lock(0, sizeof(vrtData[0])*curPoint, (void**)&vrtData, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		int totalParticles = curPoint;
		curPoint=0;
		for(int c=0; c<ci; c++)
		{
			Cloud& cld = clouds[ cRemap[cidx[c]] ];
			curPoint += cld.sort(lightDir, dt, &vrtData[curPoint], cameraPos);
		}

		assert(curPoint <= totalParticles);

		instVrtBuff->Unlock();

		d3dc._SetDecl(vrtDecl);
		d3dc._SetStreamSource(1, instVrtBuff.Get(), 0, sizeof(InstanceVertex));
		r3dRenderer->pd3ddev->SetStreamSourceFreq( 1, ( D3DSTREAMSOURCE_INSTANCEDATA | 1 ));
 		r3dRenderer->pd3ddev->SetStreamSourceFreq( 0, ( D3DSTREAMSOURCE_INDEXEDDATA | ( UINT )curPoint ));
	 

		D3DXMATRIX v = view;
		D3DXMatrixTranspose(&v, &v);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(0, (float*)&v, 4);

		D3DXMATRIX p = proj;
		D3DXMatrixTranspose(&p, &p);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(4, (float*)&p, 4);

		D3DXVECTOR4 ac(ambColor.x, ambColor.y, ambColor.z, 0.0f);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(13, &ac.x, 1);

		D3DXVECTOR4 ld(lightDir.x, lightDir.y, lightDir.z, 0.0f);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(9, &ld.x, 1);

		D3DXVECTOR4 lc(sunColor.x, sunColor.y, sunColor.z, 0.0f);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(11, &lc.x, 1);

		r3dRenderer->pd3ddev->SetPixelShaderConstantF(9, &ambSh.x, 1);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(10, &sunSh.x, 1);

		
		float fpc = (float)pointCount;
		float pc[4] = {fpc, fpc, fpc, fpc}; 
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(25, pc, 1);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(17, (float*)pointPos, pointCount);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(21, (float*)pointColor, pointCount);

		//fog
		float DepthZ = r3dRenderer->FarClip * 0.9375f;
		D3DXVECTOR4 DepthVector(cameraPos.x, cameraPos.y, cameraPos.z,1.f/DepthZ);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(  15, (float *)&DepthVector,  1 );


		//gradient colors
		D3DXVECTOR4 topc(topColor.x, topColor.y, topColor.z, 0.0f);
		D3DXVECTOR4 botc(bottomColor.x, bottomColor.y, bottomColor.z, 0.0f);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(  26, (float *)&botc,  1 );
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(  27, (float *)&topc,  1 );

		r3dRenderer->pd3ddev->SetVertexShaderConstantF(  28, (float *)atlas.rects,  atlas.count );

		//create temp rendertarget
		if(rtTemp==0)
		{
			D3DSURFACE_DESC desc;
			prevRT[0]->GetDesc(&desc);
			rtTemp = r3dScreenBuffer::CreateClass("CloudSystem: temp render target", (float)desc.Width, (float)desc.Height, desc.Format);
			rtTemp->SetDebugD3DComment("CloudSystem: temp render target");
		}
		rtTemp->Activate();
		for(int i=1; i<4; i++)	r3dRenderer->SetRT(i, 0);
		r3dRenderer->pd3ddev->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
 		r3dRenderer->DrawIndexed(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
		
		//render offscreen surface with clouds into the main rendertarget
		for(int i=0; i<2; i++)	r3dRenderer->SetRT(i, prevRT[i]);
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3dc._SetDecl( vrtDeclQuad );
		r3dRenderer->pd3ddev->SetStreamSourceFreq( 0, 1 );
		r3dRenderer->pd3ddev->SetStreamSourceFreq( 1, 1 );
		d3dc._SetStreamSource(1, 0, 0, 0);
 		r3dRenderer->SetPixelShader(psQuadId);
 		r3dRenderer->SetVertexShader(vsQuadId);
		r3dRenderer->SetTex(rtTemp->Tex, 5);


		//transform sun direction into camera space
		D3DXMATRIX invView;
		FLOAT det;
		D3DXMatrixInverse(&invView, &det, &view);
		D3DXMatrixTranspose(&invView, &invView);
		D3DXVECTOR3 sunDir;
		D3DXVec3TransformNormal(&sunDir, &lightDir, &invView);

		float su = -10.0f, sv = -10.0f;
		if(sunDir.z>0.0f)
		{
			su = 0.5f + sunDir.x*tanf(fovV)*0.5f/sunDir.z;
			sv = 0.5f - sunDir.y*tanf(fovV)*0.5f/sunDir.z;
		}
		D3DXVECTOR4 sp(su, sv, r3dRenderer->ViewW/r3dRenderer->ViewH, 0.0f);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(20, &sp.x, 1);

		D3DXVECTOR4 par(rimArea, rimPower, 0.0f, 0.0f);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(21, &par.x, 1);

		r3dRenderer->DrawIndexed(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	}

/*
	if(0)
	{
		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);

 		r3dRenderer->SetPixelShader(psShadId);
 		r3dRenderer->SetVertexShader(vsShadId);

		//render shadow texture
		//IDirect3DSurface9* surf;
		//GetSurfaceLevel(0, &surf);
		//r3dRenderer->pd3ddev->SetRenderTarget(0, surf);
		//for(int i=1; i<4; i++)	r3dRenderer->pd3ddev->SetRenderTarget(i, 0);
		//r3dRenderer->pd3ddev->SetTexture(9, 0);
		gCloudShadow->Activate();

		D3DVIEWPORT9 viewPort;
		viewPort.Height = viewPort.Width = 1024;
		viewPort.X = viewPort.Y = 0;
		viewPort.MinZ = 0.0f;
		viewPort.MaxZ = 1.0f;
		r3dRenderer->pd3ddev->SetViewport(&viewPort);

		r3dRenderer->pd3ddev->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		D3DXMATRIX shView;
		Vector3 shCamPos(cameraPos.x, 10000.0f, cameraPos.z);
		Vector3 up = Vector3(0.0f, 0.0f, 1.0f);
		Vector3 dir = shCamPos + Vector3( 0.0f,-1.0f, 0.0f);
		D3DXMatrixLookAtLH( &shView, (D3DXVECTOR3*)&shCamPos, (D3DXVECTOR3*)&dir, (D3DXVECTOR3*)&up );
		D3DXMatrixTranspose(&shView, &shView);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(0, (float*)&shView, 4);

		D3DXMATRIX shProj;
		D3DXMatrixPerspectiveFovLH(&shProj, 3.1415926f*0.12f, 1.0f, 10.0f, 11000.0f);
		D3DXMatrixTranspose(&shProj, &shProj);
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(4, (float*)&shProj, 4);

		//all other consts was set earlier


		int curPoint=0;
		for(int c=0; c<nClouds*nClouds; c++)
			curPoint += clouds[c].prepareShadow(cameraPos);
		if(curPoint > szInstBuff)
		{
			if(szInstBuff>0)	instVrtBuff->Release();
			r3dRenderer->Stats.BufferMem -= szInstBuff;
			D3D_V( r3dRenderer->pd3ddev->CreateVertexBuffer(sizeof(InstanceVertex)*curPoint, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &instVrtBuff, 0) ) ;
			szInstBuff = curPoint;
			r3dRenderer->Stats.BufferMem += szInstBuff;
		}

		InstanceVertex* vrtData;
		instVrtBuff->Lock(0, sizeof(vrtData[0])*curPoint, (void**)&vrtData, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		curPoint=0;
		for(int c=0; c<nClouds*nClouds; c++)
			curPoint += clouds[c].draw2Shadow(&vrtData[curPoint], cameraPos);
		instVrtBuff->Unlock();
		d3dc._SetDecl(vrtDecl);
		d3dc._SetStreamSource(1, instVrtBuff, 0, sizeof(InstanceVertex));
		r3dRenderer->pd3ddev->SetStreamSourceFreq( 1, ( D3DSTREAMSOURCE_INSTANCEDATA | 1 ));
 		r3dRenderer->pd3ddev->SetStreamSourceFreq( 0, ( D3DSTREAMSOURCE_INDEXEDDATA | ( UINT )curPoint ));


		r3dRenderer->DrawIndexed(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

		//surf->Release();
		if(gCloudShadow)
		{
			r3dRenderer->SetTex(gCloudShadow->Tex, 9);
		}
	}
*/

	for(int i=0; i<2; i++)	r3dRenderer->SetRT(i, prevRT[i]);
	r3dRenderer->pd3ddev->SetViewport(&prevVP);


	r3dRenderer->pd3ddev->SetStreamSourceFreq( 0, 1 );
	r3dRenderer->pd3ddev->SetStreamSourceFreq( 1, 1 );
	d3dc._SetStreamSource(1, 0, 0, 0);

	d3dc._SetDecl( decl );
	decl->Release();
	for(int i=0; i<2; i++)	prevRT[i]->Release();

	r3dRenderer->RestoreCullMode();

	r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTest);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_SRCBLEND, srcBlend);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_DESTBLEND, dstBlend);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_BLENDOP, blendOp);
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_BLENDOPALPHA, blendOpAlpha);
}


//seeding clouds to the sky. clouds coverage depends on interval between clouds
void CloudSystem::seed(float coverage, float* minHeight, float* randomHeight, int cloudsNum, Cloud* clouds)
{
	if(refCloudsCount <= 0)
		return;

	float cloudsInterval = R3D_LERP(5000.0f, 200.0f, coverage);

  	float summ = 0.0f;
  	for(int i=0; i < refCloudsCount; ++i)
  	{
  		summ += magnitude(refClouds[i].boxSize_);
  	}
  
  	summ /= refCloudsCount;

	
  
//  	tileSize.x = tileSize.z = nClouds*(summ + cloudsInterval);
//  	tileSize.y = 0.0f;
//  
//  	const int tryCount = 1000000;
// 	Vector3 pos(0,minHeight,0);
//  	for(int c = 0; c < cloudsNum*cloudsNum; c++)
//  	{
//  		int cloudRef = int(rnd.urnd()*maxRefClouds);
//  		int i;
//  		for(i=0; i<tryCount; i++)
//  		{
//  			//pos = Vector3(rnd.urnd()*tileSize.x, minHeight + rnd.urnd()*randomHeight, rnd.urnd()*tileSize.z);
// 			pos += Vector3(100, 0, 100);
//  
//  			//clouds can't intersect due to sorting problem
//  			int c1;
//  			for(c1=0; c1<c; c1++)
//  				if( magnitude(pos - clouds[c1].pos) < 
//  					cloudsInterval + (magnitude(cloudsData[c1].ref->boxSize) + magnitude(refClouds[cloudRef].boxSize))*0.5f)	break;
//  			if(c1==c)	break;
//  		}
//  
//  		if(i==tryCount)
//  		{
//  			tileSize *= 1.1f;
//  			c = 0;
// 			pos = Vector3(0,minHeight,0);
//  			continue;
//  		}
//  
//  		float r = (float(cloudRef)+0.5f)/maxRefClouds;
//  
//  		Cloud::InitData& data = cloudsData[c];
//  		data.pos = pos;
//  		data.posIdx = c;
//  		data.r = r;
//  		data.refIdx = cloudRef;
//  		data.ref = &refClouds[cloudRef];		
//  	}

	cloudCursor = 0;
 	int c = 0;
 	for(int x = 0; x < cloudsNum; ++x)
 	{
 		for(int y = 0; y < cloudsNum; ++y)
 		{
 			int cloudRef = rand() % refCloudsCount;
 			Vector3 pos = Vector3(cloudsInterval * x, minHeight[cloudRef], cloudsInterval * y);
			addCloud(cloudRef, pos);
 		}
 	}

	reinitClouds();
}

static void D3DCreateDecls( void* param )
{
	R3D_ENSURE_MAIN_THREAD();

	CloudSystem* cs = (CloudSystem*) param ;

	D3DVERTEXELEMENT9 decl[] = {
		{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{1,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		D3DDECL_END()
	};
	( r3dDeviceTunnel::CreateVertexDeclaration(decl, &cs->vrtDecl) );

	D3DVERTEXELEMENT9 decl1[] = {
		{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	( r3dDeviceTunnel::CreateVertexDeclaration(decl1, &cs->vrtDeclQuad) ) ;
}

void CloudSystem::init()
{
	if( r3dRenderer->SupportsVertexTextureFetch )
	{
		vsId = r3dRenderer->GetVertexShaderIdx( "VS_CLOUD_SIMPLE" );
		psId = r3dRenderer->GetPixelShaderIdx( "PS_CLOUD_SIMPLE" );
		vsShadId = r3dRenderer->GetVertexShaderIdx( "VS_CLOUD_SIMPLE_SH" );
		psShadId = r3dRenderer->GetPixelShaderIdx( "PS_CLOUD_SIMPLE_SH" );
	}

	vsQuadId = r3dRenderer->GetVertexShaderIdx( "VS_CLOUD_SIMPLE_QD" );
	psQuadId = r3dRenderer->GetPixelShaderIdx( "PS_CLOUD_SIMPLE_QD" );

	szInstBuff = 0;

	ProcessCustomDeviceQueueItem( D3DCreateDecls, this ) ;
	CreateQueuedResource( this );
}

void CloudSystem::release()
{
// 	cloudRefTex->Release();
// 	cloudPosTex->Release();

	R3D_ENSURE_MAIN_THREAD();

	D3DReleaseResource();
}

bool CloudSystem::save(FILE* f)
{
	if(f)
	{
		fwrite(textureFileName, sizeof(textureFileName), 1, f);
		fwrite(&ParticleLightSamples::directions, sizeof(ParticleLightSamples::directions), 1, f);
		fwrite(&refCloudsCount, sizeof(refCloudsCount), 1, f);
		for (int i = 0; i < refCloudsCount; ++i)
		{
			refClouds[i].save(f);
		}

		fwrite(&cloudCursor, sizeof(cloudCursor), 1, f);
		fwrite(&tileSize, sizeof(tileSize), 1, f);
		fwrite(cloudsData, sizeof(cloudsData), 1, f);
		
		return true;
	}

	return false;	
}

bool CloudSystem::load(r3dFile* f)
{
	if(f)
	{
		fread(textureFileName, sizeof(textureFileName), 1, f);
		loadTexture();

		fread(&ParticleLightSamples::directions, sizeof(ParticleLightSamples::directions), 1, f);
		fread(&refCloudsCount, sizeof(refCloudsCount), 1, f);
		assert(refCloudsCount <= maxRefClouds);
		for (int i = 0; i < refCloudsCount; ++i)
		{
			refClouds[i].load(f);
		}

		fread(&cloudCursor, sizeof(cloudCursor), 1, f);
		fread(&tileSize, sizeof(tileSize), 1, f);
		fread(cloudsData, sizeof(cloudsData), 1, f);

		if(refCloudsCount)
		{
			finalizeCloudsData();
			reinitClouds();	
		}	
		
		return true;
	}

	return false;
}

CloudSystem::CloudSystem(const r3dIntegrityGuardian& ig ) 
: r3dIResource( ig ),
refCloudsCount(0), cloudTex(0), cloudCursor(0), 
vsId ( -1 ), psId( -1 ), vsShadId( -1 ), 
psShadId( -1 ), vsQuadId ( -1 ), psQuadId ( -1 )

{	
	textureFileName[0] = 0;
	init();
}

CloudSystem::~CloudSystem()
{
	release();	
}

void CloudSystem::setTextureFile(const char* fileName)
{
	sprintf(textureFileName, "%s", fileName);
	loadTexture();
}

void CloudSystem::loadTexture()
{
	r3dRenderer->DeleteTexture(cloudTex);

	cloudTex = r3dRenderer->LoadTexture(textureFileName);
	char atlasFileName[256];
	sprintf(atlasFileName, "%s%s", textureFileName, ".atlas");
	atlas.load(atlasFileName);
}

void CloudSystem::D3DCreateResource()
{	
	QuadVertex* vrtData;

	r3dDeviceTunnel::CreateVertexBuffer(sizeof(QuadVertex)*4, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &quadVrtBuff );

	quadVrtBuff.Lock(0, 0, (void**)&vrtData, D3DLOCK_NOSYSLOCK);
	vrtData[0].x = -0.5f;	vrtData[0].y = -0.5f;	vrtData[0].z = 0.0f;	vrtData[0].u = 0.0f;	vrtData[0].v = 1.0f;
	vrtData[1].x =  0.5f;	vrtData[1].y = -0.5f;	vrtData[1].z = 0.0f;	vrtData[1].u = 1.0f;	vrtData[1].v = 1.0f;
	vrtData[2].x =  0.5f;	vrtData[2].y =  0.5f;	vrtData[2].z = 0.0f;	vrtData[2].u = 1.0f;	vrtData[2].v = 0.0f;
	vrtData[3].x = -0.5f;	vrtData[3].y =  0.5f;	vrtData[3].z = 0.0f;	vrtData[3].u = 0.0f;	vrtData[3].v = 0.0f;
	quadVrtBuff.Unlock();

	r3dDeviceTunnel::CreateIndexBuffer(sizeof(short)*6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &idxBuff );
	short* idxData;
	idxBuff.Lock(0, 0, (void**)&idxData, D3DLOCK_NOSYSLOCK);
	idxData[0] = 0;	idxData[1] = 1;	idxData[2] = 2;
	idxData[3] = 2;	idxData[4] = 3;	idxData[5] = 0;
	idxBuff.Unlock();

	r3dRenderer->Stats.BufferMem += sizeof(QuadVertex)*4 + sizeof(short)*6;
	
}

void CloudSystem::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();

	if(instVrtBuff.Valid()) instVrtBuff.ReleaseAndReset();
	szInstBuff = 0; 

	// Need all these if's cause otherwise it crashes if you load/unload then change resolution (cloud system doesn't get fully shut down on unload?)
	quadVrtBuff.ReleaseAndReset();
	idxBuff.ReleaseAndReset();

	r3dRenderer->Stats.BufferMem -= sizeof(QuadVertex)*4 + sizeof(short)*6;
}


#endif