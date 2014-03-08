#ifndef ARTY_CLOUDS_ENGINE
#define ARTY_CLOUDS_ENGINE

#if 0

#include "GameObjects\GameObj.h"
#include "GameObjects\ObjManag.h"

#include "TimeGradient.h"
#include "cloudsSystem.h"

#include "types.h"

class CloudsSimpleEngine;
class CloudGameObjectProxy : public GameObject
{
	DECLARE_CLASS(CloudGameObjectProxy, GameObject)
public:
	CloudGameObjectProxy();

	CloudsSimpleEngine* cloudEngine;

	virtual BOOL		OnPositionChanged();			// ObjMan: when position changed
	virtual	BOOL		OnOrientationChanged();			// ObjMan: when direction changed
};

struct CloudsSimpleEngineData
{
	Vector3 sizes[CloudSystem::maxRefClouds];
	float minHeight[CloudSystem::maxRefClouds];
	float randomHeight[CloudSystem::maxRefClouds];
	float opticalDensity[CloudSystem::maxRefClouds];
	float particlesDensity[CloudSystem::maxRefClouds];
	int texIdx[CloudSystem::maxRefClouds];

	r3dColor topColor;
	r3dColor bottomColor;
	float coverage, rimArea, rimPower;

	r3dTimeGradient2 	dirIntensity;
	r3dTimeGradient2 	ambIntensity;

	bool CompareData(const CloudsSimpleEngineData& data) const;
	bool NeedSave() const;
};

class CloudsSimpleEngine : public CloudsSimpleEngineData, public r3dIResource
{
public:
	CloudsSimpleEngine( const r3dIntegrityGuardian& ig );
	virtual ~CloudsSimpleEngine() {};

	void Init();
	void Destroy();
	void GenerateRefClouds();
	void GenerateCloudsMap();
	void CalculateLightning();
	void Draw(const r3dCamera& Cam);
	bool RefCloudsExist() const;
	void SetTextureFile(const char* file) const;
	void AddCloud(int refIdx, const r3dPoint3D& pos);
	void SetCloudTex(int refIdx, int texIdx);
	int GetAtlasSize() const;
	void DeleteCloudInFocus();
	int& GetRefCloudsCount() const;
	
	void StartDrag();
	void EndDrag();
	void DragCloudInFocus(const r3dPoint3D& pos);
	void SelectCloudInFocus();
	bool HasSelectedCloud() const;

	void DrawSelectedCloud();

	void ScaleSelectedCloud(float scale);
	float GetSelectedCloudScale() const;

	void RotateSelectedCloud(float scale);
	float GetSelectedCloudRotate() const;

	void PositionSelectedCloud(const Vector3& pos);
	Vector3 GetSelectedCloudPosition() const;

	virtual	void D3DCreateResource();
	virtual	void D3DReleaseResource();

	void Save( const char* targetDir );
	void Load( const char* targetDir );

	void RefreshCloudTextures();

	r3dColor userSunColor;
	r3dColor userAmbientColor;
	int useEngineColors;

private:

	static const int MAX_POINT_LIGHTS = 4;

	void CollectLights(const r3dCamera& Cam);

	CloudSystem* cloudSys;
	r3dTexture* cloudPosTex;
	r3dTexture* cloudRefTex;

	Vector4 plPos[MAX_POINT_LIGHTS];
	Vector4 plColor_Power[MAX_POINT_LIGHTS];
	int plCount;

	mutable int selectedCloud;
	bool dragInProgress;

	CloudGameObjectProxy* proxyObj;
};

#endif

#endif