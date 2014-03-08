//=========================================================================
//	Module: obj_ZombieSpawn.cpp
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"

#if ENABLE_ZOMBIES

#include "obj_ZombieSpawn.h"
#include "../../XMLHelpers.h"
#include "../../Editors/LevelEditor.h"

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(obj_ZombieSpawn, "obj_ZombieSpawn", "Object");
AUTOREGISTER_CLASS(obj_ZombieSpawn);

extern bool g_bEditMode;

/**	Sounds id's. */
int gSndZombieBreatheID = -1;
int gSndZombieGrowlID = -1;
int gSndZombieAttack1ID = -1;
int gSndZombieAttack2ID = -1;

//////////////////////////////////////////////////////////////////////////

namespace
{
	/**	Get random value in range [minV..maxV] */
	float GetRandomValueInRange(float minV, float maxV)
	{
		return rand() / static_cast<float>(RAND_MAX) * (maxV - minV) + minV;
	}

//////////////////////////////////////////////////////////////////////////

	struct ZombieSpawnCompositeRenderable: public Renderable
	{
		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw( Renderable* RThis, const r3dCamera& Cam )
		{
			ZombieSpawnCompositeRenderable *This = static_cast<ZombieSpawnCompositeRenderable*>(RThis);

			r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

			r3dDrawLine3D(This->Parent->GetPosition(), This->Parent->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor24::grey);
			r3dDrawCircle3D(This->Parent->GetPosition(), This->Parent->spawnRadius, Cam, 0.1f, r3dColor(0, 255, 0));

			r3dRenderer->Flush();
			r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		}

		obj_ZombieSpawn *Parent;	
	};

//////////////////////////////////////////////////////////////////////////

	void InitSoundIDs()
	{
		if (gSndZombieAttack1ID != -1)
			return;

		gSndZombieAttack1ID = SoundSys.GetEventIDByPath("Sounds/WarZ/Zombie sounds/Attack01");
		gSndZombieAttack2ID = SoundSys.GetEventIDByPath("Sounds/WarZ/Zombie sounds/Attack02");
		gSndZombieBreatheID= SoundSys.GetEventIDByPath("Sounds/WarZ/Zombie sounds/Z_breathing");
		gSndZombieGrowlID = SoundSys.GetEventIDByPath("Sounds/WarZ/Zombie sounds/Z_growling");
	}
}

//////////////////////////////////////////////////////////////////////////

obj_ZombieSpawn::obj_ZombieSpawn()
: timeStamp(r3dGetTime())
, timeSinceLastSpawn(0)
, spawnRadius(30.0f)
, zombieSpawnRate(0.5f)
, maxZombieCount(50)
, minDetectionRadius(1)
, maxDetectionRadius(3.0f)
, minSpeed(3.0f)
, maxSpeed(5.0f)
{
	m_bEnablePhysics = false;
}

//////////////////////////////////////////////////////////////////////////

obj_ZombieSpawn::~obj_ZombieSpawn()
{

}

//////////////////////////////////////////////////////////////////////////

#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_ZombieSpawn::AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam)
{
#ifdef FINAL_BUILD
	return;
#else
	if ( !g_bEditMode )
		return;

	ZombieSpawnCompositeRenderable rend;

	rend.Init();
	rend.Parent		= this;
	rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;

	render_arrays[ rsDrawDebugData ].PushBack( rend );
#endif
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_ZombieSpawn::OnCreate()
{
	parent::OnCreate();

	DrawOrder = OBJ_DRAWORDER_LAST;

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2, 2, 2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;
	SetBBoxLocal(bboxLocal) ;
	UpdateTransform();

	//	Hardcoded for now
	zombiePartNames[0].PushBack("Data\\ObjectsDepot\\Characters\\Hero_Girl_02.sco");
	zombiePartNames[0].PushBack("Data\\ObjectsDepot\\Characters\\Hero_Girl_01.sco");
	zombiePartNames[0].PushBack("Data\\ObjectsDepot\\Characters\\Hero_Man_01.sco");
	zombiePartNames[0].PushBack("Data\\ObjectsDepot\\Characters\\Hero_Man_02.sco");

	zombiePartNames[1].PushBack("Data\\ObjectsDepot\\Characters\\HGEAR_Beret.sco");
	zombiePartNames[1].PushBack("Data\\ObjectsDepot\\Characters\\HGEAR_Boonie.sco");
	zombiePartNames[1].PushBack("Data\\ObjectsDepot\\Characters\\HGEAR_Hood.sco");
	zombiePartNames[1].PushBack("Data\\ObjectsDepot\\Characters\\HGEAR_Maska_M.sco");

	zombiePartNames[2].PushBack("Data\\ObjectsDepot\\Characters\\ARMOR_Light_Forest.sco");
	zombiePartNames[2].PushBack("Data\\ObjectsDepot\\Characters\\ARMOR_Light_Urban.sco");
	zombiePartNames[2].PushBack("Data\\ObjectsDepot\\Characters\\ARMOR_Rebel_Heavy.sco");
	zombiePartNames[2].PushBack("Data\\ObjectsDepot\\Characters\\ARMOR_Shadow_Heavy.sco");

	InitSoundIDs();

	return 1;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_ZombieSpawn::Update()
{
	parent::Update();

	//	Spawn zombies.
	float curTime = r3dGetTime();
	float deltaTime = curTime - timeStamp;
	timeStamp = curTime;

	timeSinceLastSpawn += deltaTime;

	int numZombiesToSpawn = static_cast<int>(timeSinceLastSpawn * zombieSpawnRate);
	if (numZombiesToSpawn <= 0)
		return TRUE;

	timeSinceLastSpawn -= numZombiesToSpawn / zombieSpawnRate;

	int newMaxZombies = std::min<int>(maxZombieCount, numZombiesToSpawn + zombies.Count());
	int oldMaxZombies = zombies.Count();
	if (oldMaxZombies > newMaxZombies)
	{
		for (int i = newMaxZombies; i < oldMaxZombies; ++i)
		{
			zombies[i]->setActiveFlag(0);
		}
	}
	zombies.Resize(newMaxZombies);

	for (int i = oldMaxZombies; i < newMaxZombies; ++i)
	{
		r3dPoint3D pos(GetPosition());
		//	Generate random pos within spawn radius
		float rX = rand() / static_cast<float>(RAND_MAX);
		float rZ = rand() / static_cast<float>(RAND_MAX);

		r3dPoint3D offset(rX, 0, rZ);
		offset = (offset - 0.5f) * 2;
		offset.y = 0;
		offset.Normalize();
		offset *= rand() / static_cast<float>(RAND_MAX) * spawnRadius;
		pos += offset;

		const float rayCastDist = 20000.0f;
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlags(PxSceneQueryFilterFlag::eSTATIC));
		if (g_pPhysicsWorld->PhysXScene->raycastSingle(PxVec3(pos.x, rayCastDist, pos.z), PxVec3(0, -1, 0), rayCastDist * 2, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			pos.y = hit.impact.y;
		}

		obj_Zombie *z = static_cast<obj_Zombie*>(srv_CreateGameObject("obj_Zombie", "Data/ObjectsDepot/Characters/Hero_Girl_01.sco", pos));
		zombies[i] = z;
		z->SetSpawn(this);
		float randomRot = static_cast<float>(rand()) / RAND_MAX * 360.0f;
		z->SetRotationVector(r3dVector(randomRot, 0, 0));
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void obj_ZombieSpawn::DeleteZombie(obj_Zombie *z)
{
	uint32_t i = 0;
	for (; i < zombies.Count(); ++i)
	{
		if (zombies[i] == z)
			break;
	}

	if (i < zombies.Count())
	{
		z->setActiveFlag(0);
		zombies.Erase(i);
	}
};

//////////////////////////////////////////////////////////////////////////

BOOL obj_ZombieSpawn::OnDestroy()
{
	for (uint32_t i = 0; i < zombies.Count(); ++i)
	{
		zombies[i]->setActiveFlag(0);
	}
	zombies.Clear();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

extern int				ObjCatInit;
extern int				NumCategories;

typedef std::vector<CategoryStruct>  catlist;
extern catlist  ObjectCategories;
extern stringlist_t 	CatNames;
extern float			CatOffset;

#ifndef FINAL_BUILD
float obj_ZombieSpawn::SelectNewZombiePartElement(float y, int &arrIdx)
{
	const float DEFAULT_CONTROLS_WIDTH = 360.0f;
	const float DEFAULT_CONTROLS_HEIGHT = 22.0f;


	float SliderX = r3dRenderer->ScreenW-375;
	float SliderY = y;

	//
	static char CategoryName[64] = "";
	static char ClassName[64] = "";
	static char FileName[64] = "";

	char Str[256];

	InitObjCategories();

	SliderY += imgui_Static(SliderX, SliderY, CategoryName, 200);
	static int idx = -1;
	if (imgui_DrawList(SliderX, SliderY, 360, 200, CatNames, &CatOffset, &idx))
	{
		sprintf(CategoryName, "%s", CatNames.at(idx).c_str());

		sprintf(ClassName,"");
		sprintf(FileName, "");
		if(ObjectCategories.at(idx).ObjectsClasses.size() > 0)
		{
			sprintf(ClassName, "%s", ObjectCategories.at(idx).ObjectsClasses.at(0).c_str());
			sprintf(FileName, "%s", ObjectCategories.at(idx).ObjectsNames.at(0).c_str());
		}
	}

	SliderY += 210;

	SliderY += imgui_Static(SliderX, SliderY, FileName);

	static int idx1 = 0;
	if (idx != -1)
	{
		if (imgui_DrawList(SliderX, SliderY, 360, 200, ObjectCategories.at(idx).ObjectsNames , &ObjectCategories.at(idx).Offset, &idx1))
		{
			sprintf (ClassName,"%s", ObjectCategories.at(idx).ObjectsClasses.at(idx1).c_str());
			sprintf (FileName,"%s", ObjectCategories.at(idx).ObjectsNames.at(idx1).c_str());
		}
		SliderY += 205;

		if(ClassName[0] == 0 || FileName[0] == 0)
			return SliderY;

		sprintf (Str,"Data\\ObjectsDepot\\%s\\%s", CategoryName, FileName);
		std::string fileName = Str;

		sprintf (Str,"%s\\%s", CategoryName, FileName);
		std::string uiName = Str;

		if(imgui_Button(SliderX, SliderY, DEFAULT_CONTROLS_WIDTH, DEFAULT_CONTROLS_HEIGHT, "Add"))
		{
			zombiePartNames[arrIdx].PushBack(fileName);
			arrIdx = -1;
		}
		SliderY += DEFAULT_CONTROLS_HEIGHT;
	}
	return SliderY;
}
#endif

//////////////////////////////////////////////////////////////////////////

#ifndef FINAL_BUILD
float obj_ZombieSpawn::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float y = scry;
	static int addEl = -1;
	if (addEl >= 0)
	{
		y = SelectNewZombiePartElement(y, addEl);
		return y;
	}

	y += parent::DrawPropertyEditor(scrx, scry, scrw, scrh, startClass, selected);
	y += 5.0f;

	y += imgui_Static(scrx, y, "Spawn parameters:");
	int zc = maxZombieCount;
	y += imgui_Value_SliderI(scrx, y, "Max zombies", &zc, 0, 100.0f, "%d");
	maxZombieCount = zc;
	y += imgui_Value_Slider(scrx, y, "Spawn rate", &zombieSpawnRate, 0.0f, 3.0f, "%-02.3f");
	y += imgui_Value_Slider(scrx, y, "Spawn radius", &spawnRadius, 10.0f, 300.0f, "%-02.3f");

	y += 5.0f;
	y += imgui_Static(scrx, y, "Zombie senses parameters:");
	
	y += imgui_Value_Slider(scrx, y, "Min detect radius", &minDetectionRadius, 1.0f, 100.0f, "%-02.3f");
	maxDetectionRadius = std::max(minDetectionRadius, maxDetectionRadius);
	y += imgui_Value_Slider(scrx, y, "Max detect radius", &maxDetectionRadius, 1.0f, 100.0f, "%-02.3f");
	minDetectionRadius = std::min(minDetectionRadius, maxDetectionRadius);

	y += imgui_Value_Slider(scrx, y, "Min speed", &minSpeed, 3.0f, 100.0f, "%-02.3f");
	maxSpeed = std::max(minSpeed, maxSpeed);
	y += imgui_Value_Slider(scrx, y, "Max speed", &maxSpeed, 4.0f, 100.0f, "%-02.3f");
	minSpeed = std::min(minSpeed, maxSpeed);

	y += 5;
	if (imgui_Button(scrx, y, 360.0f, 22.0f, "Respawn All"))
	{
		OnDestroy();
	}
	y += 24.0f;

	stringlist_t sl;
	const uint32_t numPartNames = _countof(zombiePartNames);
	static float offsets[numPartNames] = {0};

	const float listH = 90.0f;
	for (uint32_t i = 0; i < numPartNames; ++i)
	{
		sl.clear();
		const Names &n = zombiePartNames[i];
		for (uint32_t k = 0; k < n.Count(); ++k)
		{
			sl.push_back(n[k]);
		}

		int sel = -1;
		imgui_DrawList(scrx, y, 360.0f, listH, sl, &offsets[i], &sel);
		y += listH;
		if (imgui_Button(scrx, y, 360.0f, 22.0f, "Add element"))
		{
			addEl = i;
		}
		y += 24.0f;
	}

	return y - scry;
}
#endif

//////////////////////////////////////////////////////////////////////////

float obj_ZombieSpawn::GetRandDetectionRadius() const
{
	return GetRandomValueInRange(minDetectionRadius, maxDetectionRadius);
}

//////////////////////////////////////////////////////////////////////////

float obj_ZombieSpawn::GetRandSpeed() const
{
	return GetRandomValueInRange(minSpeed, maxSpeed);
}

//////////////////////////////////////////////////////////////////////////

void obj_ZombieSpawn::WriteSerializedData(pugi::xml_node& node)
{
	parent::WriteSerializedData(node);
	pugi::xml_node zombieSpawnNode = node.append_child();
	zombieSpawnNode.set_name("spawn_parameters");
	SetXMLVal("spawn_radius", zombieSpawnNode, &spawnRadius);
	int mzc = maxZombieCount;
	SetXMLVal("max_zombies", zombieSpawnNode, &mzc);
	SetXMLVal("zombies_spawn_rate", zombieSpawnNode, &zombieSpawnRate);
	SetXMLVal("min_detection_radius", zombieSpawnNode, &minDetectionRadius);
	SetXMLVal("max_detection_radius", zombieSpawnNode, &maxDetectionRadius);
	SetXMLVal("min_speed", zombieSpawnNode, &minSpeed);
	SetXMLVal("max_speed", zombieSpawnNode, &maxSpeed);

	pugi::xml_node zombieBodyParts = node.append_child();
	zombieBodyParts.set_name("zombie_parts");

	char buf[16] = {0};
	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		sprintf(buf, "p%i", i);
		for (uint32_t k = 0; k < zombiePartNames[i].Count(); ++k)
		{
			SetXMLVal(buf, zombieBodyParts, &zombiePartNames[i][k]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_ZombieSpawn::ReadSerializedData(pugi::xml_node& node)
{
	parent::ReadSerializedData(node);
	pugi::xml_node zombieSpawnNode = node.child("spawn_parameters");
	GetXMLVal("spawn_radius", zombieSpawnNode, &spawnRadius);
	int mzc = maxZombieCount;
	GetXMLVal("max_zombies", zombieSpawnNode, &mzc);
	maxZombieCount = mzc;
	GetXMLVal("zombies_spawn_rate", zombieSpawnNode, &zombieSpawnRate);
	GetXMLVal("min_detection_radius", zombieSpawnNode, &minDetectionRadius);
	GetXMLVal("max_detection_radius", zombieSpawnNode, &maxDetectionRadius);
	GetXMLVal("min_speed", zombieSpawnNode, &minSpeed);
	GetXMLVal("max_speed", zombieSpawnNode, &maxSpeed);

	pugi::xml_node zombieBodyParts = node.child("zombie_parts");

	char buf[16] = {0};
	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		zombiePartNames[i].Clear();

		sprintf(buf, "p%i", i);
		pugi::xml_node partNode = zombieBodyParts.child(buf);
		while (!partNode.empty())
		{
			zombiePartNames[i].PushBack(partNode.attribute("val").value());
			partNode = partNode.next_sibling(buf);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void obj_ZombieSpawn::GetRandomBodyPartNames(std::string (&partNames)[ZOMBIE_BODY_PARTS_COUNT]) const
{
	for (int i = 0; i < ZOMBIE_BODY_PARTS_COUNT; ++i)
	{
		if (zombiePartNames[i].Count() > 0)
		{
			partNames[i] = zombiePartNames[i][rand() % zombiePartNames[i].Count()];
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#endif // ENABLE_ZOMBIES
