#include "r3dPCH.h"
#if 0

#include "r3d.h"
#include "obj_cloudsSimple.h"

#include "cloudsLightning.h"
#include "GameCommon.h"
#include "GameLevel.h"

#include "r3dDeviceQueue.h"

#include "Editors/ObjectManipulator3d.h"

//extern ObjectManipulator3d g_Manipulator3d;

IMPLEMENT_CLASS(CloudGameObjectProxy, "CloudGameObjectProxy", "Object");
AUTOREGISTER_CLASS(CloudGameObjectProxy);

CloudGameObjectProxy::CloudGameObjectProxy()
{
	m_isSerializable = false; // do not allow this object to save itself
}

BOOL CloudGameObjectProxy::OnPositionChanged()
{
	GameObject::OnPositionChanged();

	const r3dVector& pos = GetPosition();
	cloudEngine->PositionSelectedCloud(Vector3(pos.x, pos.y, pos.z));

	return true;
}

BOOL CloudGameObjectProxy::OnOrientationChanged()
{
	GameObject::OnOrientationChanged();

	const r3dVector& angles = GetRotationVector();	
	cloudEngine->RotateSelectedCloud(R3D_DEG2RAD(angles.x));

	return true;
}


//
//
//
D3DXVECTOR4 ash;

static CloudsSimpleEngineData currentData;

bool CloudsSimpleEngineData::CompareData(const CloudsSimpleEngineData& data) const
{
	for (int i = 0; i < CloudSystem::maxRefClouds; ++i)
	{
		if(sizes[i]			!= data.sizes[i] || 
		minHeight[i]		!= data.minHeight[i] || 
		randomHeight[i]		!= data.randomHeight[i] || 
		opticalDensity[i]	!= data.opticalDensity[i] || 
		particlesDensity[i]	!= data.particlesDensity[i] || 
		texIdx[i]			!= data.texIdx[i]) return false;
	}

	if(topColor != data.topColor ||	bottomColor != data.bottomColor || coverage != data.coverage) 
		return false;

	return true;
// 	;
// 	ambIntensity;
}

CloudsSimpleEngine::CloudsSimpleEngine( const r3dIntegrityGuardian& ig )
: r3dIResource( ig )
{

}

void CloudsSimpleEngine::Init()
{
	proxyObj = (CloudGameObjectProxy*)srv_CreateGameObject("CloudGameObjectProxy", "objCloudGameObjectProxy", r3dPoint3D(0,0,0));
	proxyObj->cloudEngine = this;

	CalculateAmbientSH(ash);

	cloudSys = new CloudSystem( r3dIntegrityGuardian() );

	CreateQueuedResource( this ) ;

	plCount = 0;
	dirIntensity.Reset(0.7f);
	ambIntensity.Reset(0.3f);

	coverage = 0.5f;

	selectedCloud = -1;

	topColor = r3dColor::white;
	bottomColor = r3dColor::white;
	rimArea = 2.0f;
	rimPower = 0.5f;

	userSunColor = r3dColor::white;
	userAmbientColor = r3dColor::white;
	useEngineColors = 1;

	for (int i = 0; i < CloudSystem::maxRefClouds; ++i)
	{
		minHeight[i] = 500;
		randomHeight[i] = 300;
		texIdx[i] = 2;
		opticalDensity[i] = 0.85f;
		particlesDensity[i] = 2.0f;
		sizes[i] = Vector3(16.0f, 7.0f, 16.0f);
	}
}

void CloudsSimpleEngine::Destroy()
{
	R3D_ENSURE_MAIN_THREAD();

	D3DReleaseResource();

	SAFE_DELETE( cloudSys );
}

int& CloudsSimpleEngine::GetRefCloudsCount() const
{
	return cloudSys->refCloudsCount;
}

void CloudsSimpleEngine::GenerateRefClouds()
{
	cloudSys->generate(cloudRefTex->AsTex2D(), texIdx, particlesDensity, opticalDensity, sizes);
}

bool CloudsSimpleEngine::RefCloudsExist() const
{
	return cloudSys->refCloudsCount > 0;
}

void CloudsSimpleEngine::CalculateLightning()
{
	cloudSys->calculateLightning(cloudRefTex->AsTex2D(), opticalDensity);
}

void CloudsSimpleEngine::GenerateCloudsMap()
{
	cloudSys->generateCloudsMap(coverage, minHeight, randomHeight, cloudPosTex->AsTex2D());
}

void CloudsSimpleEngine::Draw( const r3dCamera& Cam)
{
	if(cloudSys == 0)
		return;

	R3DPROFILE_FUNCTION("CloudsSimpleEngine::Draw");

	D3DPERF_BeginEvent(0, L"CloudsSimpleEngine::Draw");

	float di = dirIntensity.GetFloatValue( r3dGameLevel::Environment.__CurTime/24.0f );
	float ai = ambIntensity.GetFloatValue( r3dGameLevel::Environment.__CurTime/24.0f );

	Vector3 sunDir(1.0, 1.0, 1.0);
	sunDir = -*Sun->SunDir.d3dx();

	float ci = 1.0f / 255.0f;
	Vector3 sc, ac;
	if(useEngineColors)
	{
		sc = Vector3(Sun->SunLight.R, Sun->SunLight.G, Sun->SunLight.B);
		ac = Vector3(r3dRenderer->AmbientColor.R, r3dRenderer->AmbientColor.G, r3dRenderer->AmbientColor.B);
	}
	else
	{
		sc = Vector3(userSunColor.R, userSunColor.G, userSunColor.B);
		ac = Vector3(userAmbientColor.R, userAmbientColor.G, userAmbientColor.B);
	}
			
	//D3DXVec3Normalize(&sc, &sc);
	//D3DXVec3Normalize(&ac, &ac);
	sc *= di * ci;
	ac *= ai * ci;

	Vector3 top(topColor.R, topColor.G, topColor.B);
	Vector3 bottom(bottomColor.R, bottomColor.G, bottomColor.B);
	top *= ci;
	bottom *= ci;

	Light::SH::SHSample<1> lightSH(sunDir);
	D3DXVECTOR4 sh;
	lightSH.extract(&sh.x, 4);
	CollectLights(Cam);
	cloudSys->render(r3dGetFrameTime(), cloudRefTex, cloudPosTex, *Cam.d3dx(), 1.0f, r3dRenderer->ViewMatrix, r3dRenderer->ProjMatrix, sunDir, sc, ac, sh, ash, plPos, plColor_Power, plCount, gCam.Aspect*R3D_DEG2RAD(gCam.FOV), R3D_DEG2RAD(gCam.FOV), top, bottom, rimArea, rimPower);
	D3DPERF_EndEvent();
}

void CloudsSimpleEngine::CollectLights(const r3dCamera& Cam)
{
	plCount = 0;

	if(MAX_POINT_LIGHTS > 0)
	{
		int iLightsProcessed = 0;

		struct LightEntry
		{
			r3dLight* light;
			float distSq;

			inline LightEntry():light(0), distSq(FLT_MAX){}
		};

		LightEntry lights[MAX_POINT_LIGHTS];

		for (uint32_t i = 0, i_end = WorldLightSystem.Lights.Count(); i != i_end; ++i)
		{
			r3dLight *pTmp = WorldLightSystem.Lights[i];
			if ( pTmp && pTmp->Type == R3D_OMNI_LIGHT )
			{
				float lenSq = (Cam - *pTmp).LengthSq();
			
				int maxIdx = 0;
				for(int i = maxIdx + 1; i < MAX_POINT_LIGHTS; ++i)
				{
					if( lights[maxIdx].distSq > lights[i].distSq )
					{
						maxIdx = i;
					}
				}

				
				if(lenSq < lights[maxIdx].distSq)
				{
					lights[maxIdx].distSq = lenSq;
					lights[maxIdx].light = pTmp;
				}
			}
		}	

		float cn = 1.0f / 255.0f;
		int iNumLights = 0;
		for(int i = 0; i < MAX_POINT_LIGHTS; ++i)
		{
			if(lights[i].light)
			{
				r3dLight& l = *lights[i].light;

				Vector4& light = plPos[iNumLights];
				Vector4& color = plColor_Power[iNumLights];
				light = Vector4(l.x, l.y, l.z, 0.0f);
				color = Vector4(l.R * cn, l.G * cn, l.B * cn, 10000.0f);
				++iNumLights;
			}
		}

		plCount = iNumLights;
	}	

	


// 	plPos[0] = Vector4(Cam.x, Cam.y, Cam.z, 0.0f);
// 	plColor_Power[0] = Vector4(0, 1, 0, 100000.0f);
// 	plCount = 4;

}

int formatTag = -57;

void CloudsSimpleEngine::Save( const char* targetDir )
{
	char path[256];
	sprintf(path, "%s%s", targetDir, "/clouds.bin");

	FILE* f = fopen(path, "wb");
	if(f)
	{
		fwrite(&formatTag, sizeof(formatTag), 1, f);

		cloudSys->save(f);

		fwrite(&useEngineColors, sizeof(useEngineColors), 1, f);
		fwrite(&userAmbientColor, sizeof(userAmbientColor), 1, f);
		fwrite(&userSunColor, sizeof(userSunColor), 1, f);
		fwrite(&coverage, sizeof(coverage), 1, f);
		fwrite(&minHeight, sizeof(minHeight), 1, f);
		fwrite(&randomHeight, sizeof(randomHeight), 1, f);
		fwrite(&topColor, sizeof(topColor), 1, f);
		fwrite(&bottomColor, sizeof(bottomColor), 1, f);
		fwrite(&rimArea, sizeof(rimArea), 1, f);
		fwrite(&rimPower, sizeof(rimPower), 1, f);
		

		fwrite(&texIdx, sizeof(texIdx), 1, f);
		fwrite(&opticalDensity, sizeof(opticalDensity), 1, f);
		fwrite(&particlesDensity, sizeof(particlesDensity), 1, f);
		fwrite(&sizes, sizeof(sizes), 1, f);

		fclose(f);
	}
}

bool CloudsSimpleEngineData::NeedSave() const
{
	return !CompareData( currentData );
}

void CloudsSimpleEngine::ScaleSelectedCloud(float scale)
{
	if(selectedCloud >=0)
	{
		float s = cloudSys->getCloudScale(selectedCloud);
		if(fabsf(s - scale) > 0.01f)
		{
			cloudSys->setCloudScale(selectedCloud, scale);
			cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
		}		
	}
}

float CloudsSimpleEngine::GetSelectedCloudScale() const
{
	if(selectedCloud >=0)
	{
		return cloudSys->getCloudScale(selectedCloud);
	}

	return -1.0f;
}


void CloudsSimpleEngine::RotateSelectedCloud(float rotate)
{
	if(selectedCloud >=0)
	{
		float r = cloudSys->getCloudRotate(selectedCloud);
		if(fabsf(r - rotate) > 0.01f)
		{
			cloudSys->setCloudRotate(selectedCloud, rotate);
			cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
		}
	}
}

float CloudsSimpleEngine::GetSelectedCloudRotate() const
{
	if(selectedCloud >=0)
	{
		return cloudSys->getCloudRotate(selectedCloud);
	}

	return -1.0f;
}

void CloudsSimpleEngine::PositionSelectedCloud(const Vector3& pos)
{
	if(selectedCloud >=0)
	{
		Vector3 p = cloudSys->getCloudPos(selectedCloud);
		if(magnitude(p - pos) > 0.01f)
		{
			cloudSys->setCloudPos(selectedCloud, pos);
			cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
		}
	}	
}

Vector3 CloudsSimpleEngine::GetSelectedCloudPosition() const
{
	if(selectedCloud >=0)
	{
		return cloudSys->getCloudPos(selectedCloud);
	}

	return Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

bool CloudsSimpleEngine::HasSelectedCloud() const
{
	return selectedCloud >= 0;
}

int CloudsSimpleEngine::GetAtlasSize() const
{
	return cloudSys->atlasSize();
}

void CloudsSimpleEngine::SetCloudTex(int refIdx, int texIdx)
{
	cloudSys->setTexIdxToRef(cloudRefTex->AsTex2D(), refIdx, texIdx);
}

void CloudsSimpleEngine::AddCloud(int refIdx, const r3dPoint3D& pos)
{
	cloudSys->addCloud(refIdx, Vector3(pos.x, pos.y, pos.z));
	cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
	selectedCloud = cloudSys->cloudsCount() - 1;
}

void CloudsSimpleEngine::DeleteCloudInFocus()
{
	int cloud = cloudSys->pickCloud();
	if(cloud >= 0)
	{
		cloudSys->deleteCloud(cloud);		
		cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
		selectedCloud = -1;
	}
}

void CloudsSimpleEngine::DragCloudInFocus(const r3dPoint3D& pos)
{
	if(selectedCloud >=0 && dragInProgress)
	{
		float y = cloudSys->getCloudPos(selectedCloud).y;
		cloudSys->setCloudPos(selectedCloud, Vector3(pos.x, y, pos.z));
		cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
	}
}

void CloudsSimpleEngine::DrawSelectedCloud()
{
	if(selectedCloud >=0)
	{
		r3dBox3D box = cloudSys->getCloudBox(selectedCloud);
		r3dDrawBoundBox(box, gCam, r3dColor::green, 10.0f);
	}
}

void CloudsSimpleEngine::StartDrag()
{
	if(!dragInProgress)
	{
		int cloud = cloudSys->pickCloud();
		dragInProgress = (cloud >= 0 && cloud == selectedCloud);
	}	
}

void CloudsSimpleEngine::EndDrag()
{
	dragInProgress = false;
}

void CloudsSimpleEngine::SelectCloudInFocus()
{
	selectedCloud = cloudSys->pickCloud();

	Vector3 v;
	v = cloudSys->getCloudPos(selectedCloud);

	proxyObj->SetPosition( r3dPoint3D(v.x, v.y, v.z) );

	g_Manipulator3d.ScaleEnable();
	g_Manipulator3d.Enable();
	g_Manipulator3d.PickerResetPicked();
	g_Manipulator3d.PickerAddToPicked(proxyObj);
}


void CloudsSimpleEngine::SetTextureFile(const char* file) const
{
	cloudSys->setTextureFile(file);
}

static void RefreshCloudTextures( void* param )
{
	CloudsSimpleEngine* cse = (CloudsSimpleEngine*) param ;

	cse->RefreshCloudTextures();
}

void CloudsSimpleEngine::Load( const char* targetDir )
{
	char path[256];
	sprintf(path, "%s%s", targetDir, "/clouds.bin");
	if(r3dFileExists(path))
	{
		r3dFile* f = r3d_open(path, "rb");

		int ft;
		fread(&ft, sizeof(ft), 1, f);
		if(ft != formatTag)
		{
			fclose(f);
			return;
		}

 		if( cloudSys->load(f) )	
		{
			fread(&useEngineColors, sizeof(useEngineColors), 1, f);
			fread(&userAmbientColor, sizeof(userAmbientColor), 1, f);
			fread(&userSunColor, sizeof(userSunColor), 1, f);
			fread(&coverage, sizeof(coverage), 1, f);
			fread(&minHeight, sizeof(minHeight), 1, f);
			fread(&randomHeight, sizeof(randomHeight), 1, f);
			fread(&topColor, sizeof(topColor), 1, f);
			fread(&bottomColor, sizeof(bottomColor), 1, f);
			fread(&rimArea, sizeof(rimArea), 1, f);
			fread(&rimPower, sizeof(rimPower), 1, f);

			fread(&texIdx, sizeof(texIdx), 1, f);
			fread(&opticalDensity, sizeof(opticalDensity), 1, f);
			fread(&particlesDensity, sizeof(particlesDensity), 1, f);
			fread(&sizes, sizeof(sizes), 1, f);

			ProcessCustomDeviceQueueItem( ::RefreshCloudTextures, this ) ;

		}

		fclose(f);
	}

	currentData = *this;
}

void CloudsSimpleEngine::RefreshCloudTextures()
{
	R3D_ENSURE_MAIN_THREAD();

	cloudSys->refresh(cloudRefTex->AsTex2D());
	cloudSys->refreshCloudsMap(cloudPosTex->AsTex2D());
}

// void CloudsSimpleEngine::SetPointLight(const Vector3& pos, const Vector3& color, float power)
// {
// 	plPos = Vector4(pos.x, pos.y, pos.z, 0);
// 	plColor_Power =  Vector4(color.x, color.y, color.z, power);
// }

void CloudsSimpleEngine::D3DCreateResource()
{
	R3D_ENSURE_MAIN_THREAD();
 
  	cloudPosTex = r3dRenderer->AllocateTexture();
  	cloudRefTex = r3dRenderer->AllocateTexture();
  	extern D3DPOOL r3dDefaultTexturePool;
  	D3DPOOL defaultTexturePool = r3dDefaultTexturePool;
  	r3dDefaultTexturePool = D3DPOOL_DEFAULT;
  	cloudPosTex->Create(CloudSystem::nClouds, CloudSystem::nClouds, D3DFMT_A32B32G32R32F, 1);
  	cloudRefTex->Create(CloudSystem::MaxParticlePerCloud, CloudSystem::maxRefClouds, D3DFMT_A32B32G32R32F, 1);
  	r3dDefaultTexturePool = defaultTexturePool;

	RefreshCloudTextures();
}

void CloudsSimpleEngine::D3DReleaseResource()
{
	R3D_ENSURE_MAIN_THREAD();

  	r3dRenderer->DeleteTexture(cloudPosTex);
  	r3dRenderer->DeleteTexture(cloudRefTex);
}

#endif