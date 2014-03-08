#ifndef ARTY_CLOUD_SYSTEM
#define ARTY_CLOUD_SYSTEM

#if 0

#include "SH.h"
#include "random.h"
#include "types.h"

#include "AtlasDesc.h"

class CloudSystem : public r3dIResource
{
	friend void D3DCreateDecls( void* param );
public:

	int refCloudsCount;

	static const int nClouds=16;
	static const int maxRefClouds=1;
	static const int MaxParticlePerCloud = 1024;
	static const int lightningSamplesCount = 100;

	struct ParticleLightSamples 
	{
		static Vector3 directions[lightningSamplesCount];
		static Vector3 hemidirections[lightningSamplesCount];
		static void precomputeDirections();

		ParticleLightSamples()
		{
			for (int i = 0; i < lightningSamplesCount; ++i)
				lengthInMedia[i] = 1.0f;
		}

		float lengthInMedia[lightningSamplesCount];
	};

	struct Plane
	{
		Vector3 n;
		float d;
	};
	static Plane frustum[6];
	static bool frustumIsVisible(const Vector3& pos, float radius);
	static Vector3 shift;

#pragma pack(push,1)
	struct Color
	{
		float r,g,b,a;
	};
#pragma pack(pop)

private:
	static float viewDist;	//1:50 to real world
	static short cRemap[nClouds*nClouds], cidx[nClouds*nClouds];	//for sorting clouds
	static short pidx[MaxParticlePerCloud];	//for sorting particles
	static float dist2cam[ MaxParticlePerCloud > nClouds*nClouds ? MaxParticlePerCloud : nClouds*nClouds];	//for sorting particles and clouds

	static Vector3 tileSize;
	static Base::Random rnd;

#pragma pack(push,1)
	struct QuadVertex
	{
		float x,y,z;
		float u,v;
	};
	struct InstanceVertex
	{
		float u,v,s,q;
	};
#pragma pack(pop)

	IDirect3DVertexDeclaration9* vrtDecl;
	IDirect3DVertexDeclaration9* vrtDeclQuad;
	r3dD3DIndexBufferTunnel idxBuff;
	r3dD3DVertexBufferTunnel quadVrtBuff;
	r3dD3DVertexBufferTunnel instVrtBuff;
	int szInstBuff;

	int vsId, vsShadId, vsQuadId;
	int psId, psShadId, psQuadId;
	
	char textureFileName[256];
	AtlasDesc atlas;
	r3dTexture* cloudTex;

	class TQuickSorter
	{
	public:
		__forceinline void QSort(short* ArrayToSort, int uCount);
	protected:
		__forceinline void QSortRecursive(short* pArr, int d, int h);
	};

	struct RefCloud
	{
		ParticleLightSamples* lightSamples;
		Vector3* particlesPos;
		Vector4* lightningSH;
		Vector4* params;
		Vector3 boxSize_;
		int nParticles;
		int cloudNum;

		RefCloud();
		~RefCloud();
		void createData(float particlesDensity, const Vector3& size);
		void generate(int cn, int texIdx, float particlesDensity, float opticalDensity, const Vector3& size);
		void setTexIdx(int texIdx);
		void calculateLightning(float opticalDensity);
		void save(FILE* file);
		void load(r3dFile* file);
		void init(Color* ln);
		void clear();

	};
	RefCloud refClouds[maxRefClouds];
	

	struct Cloud
	{
		struct InitData 
		{
			Vector3 pos;
			int posIdx;
			int refIdx;
			float scale;
			float rotate;
			float r;
			RefCloud* ref;

			InitData() : posIdx(0), refIdx(0), ref(0), r(0), scale(1.0f), rotate(0) {}
		};

		RefCloud* cloud;
		int refIdx;
		float scale;
		float rotate;
		Vector3 pos, wp;
		Vector3 prevCamPos;
		float curLod, prevLod;
		float r;
		float* particlesRotation;
		int posIdx;
		InstanceVertex* vrtBuff;
		int activeParticles;

		Cloud();
		~Cloud();
		void clear();
		void init(const Cloud::InitData& data);
		void flushTo(Color& inst);
// 		void save(FILE* file);
// 		void load(r3dFile* file, RefCloud* refClouds);
		void finalize(RefCloud* refClouds);
//		void setPosition(const Vector3& pos, const Vector3& tileSize);

		Vector3 bboxSize() const;
		void bbox(r3dBox3D& box) const;
		bool intersect(const r3dPoint3D& vStart, const r3dPoint3D& vDir, float* dist = 0) const;

		int updatePos(const Vector3& cameraPos, float fovH, float fovV);
		int sort(const Vector3& sunDir, float dt, InstanceVertex* vs, const Vector3& cameraPos);

		int prepareShadow(const Vector3& cameraPos);
		int draw2Shadow(InstanceVertex* vs, const Vector3& cameraPos);
	};
	Cloud clouds[nClouds*nClouds];
	Cloud::InitData cloudsData[nClouds*nClouds];
	int cloudCursor;

	void seed(float cloudsInterval, float* minHeight, float* randomHeight, int cloudsNum, Cloud* clouds);
	void computeTileSize();
	void finalizeCloudsData();
	void reinitClouds();
	void loadTexture();
	
public:
	CloudSystem( const r3dIntegrityGuardian& ig );
	virtual ~CloudSystem();

	void init();
	void release();

	float getCloudScale(int idx) const;
	void setCloudScale(int idx, float scale);

	float getCloudRotate(int idx) const;
	void setCloudRotate(int idx, float scale);

	Vector3 getCloudPos(int idx) const;
	void setCloudPos(int idx, const Vector3& pos);

	int cloudsCount() const;

	int atlasSize() const;

	//generates type of clouds and cloud's tile
	void generateCloudsMap(float coverage, float* minHeight, float* randomHeight, LPDIRECT3DTEXTURE9 mapTex);
	void addCloud(int refIdx, const Vector3& pos);
	void deleteCloud(int idx);
	r3dBox3D getCloudBox(int idx) const;
	int pickCloud();
	void refreshCloudsMap(LPDIRECT3DTEXTURE9 cloudPosTex);
	void generate(LPDIRECT3DTEXTURE9 cloudPosTex, const int* texIdx, const float* particlesDensity, const float* opticalDensity, const Vector3* size);
	void refresh(LPDIRECT3DTEXTURE9 cloudRefTex);
	void setTexIdxToRef(LPDIRECT3DTEXTURE9 cloudRefTex, int refIdx, int texIdx);
	void calculateLightning(LPDIRECT3DTEXTURE9 cloudRefTex, const float* opticalDensity);
	void computeLight();	//spherical harmonics computation
	bool save(FILE* f);	//save generated clouds into file
	bool load(r3dFile* f);
	void setTextureFile(const char* fileName);

	void render(float dt, r3dTexture* cloudRefTex, r3dTexture* cloudPosTex, const Vector3& camPos, float lodK,
				const D3DXMATRIX& view, const D3DXMATRIX& proj,
				const Vector3& sunDir, 
				const Vector3& sunColor, 
				const Vector3& ambColor, 
				const Vector4& sunSh, 
				const Vector4& ambSh,
				const Vector4* pointPos, 
				const Vector4* pointColor,
				int pointCount, 
				float fovH, 
				float fovV,
				const Vector3& topColor, 
				const Vector3& bottomColor,
				float rimArea, float rimPower);

	virtual	void D3DCreateResource();
	virtual	void D3DReleaseResource();
};

#endif

#endif