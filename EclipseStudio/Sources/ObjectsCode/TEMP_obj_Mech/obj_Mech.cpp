#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "obj_Mech.h"

#ifndef FINAL_BUILD

IMPLEMENT_CLASS(obj_Mech, "obj_Mech", "Object");
AUTOREGISTER_CLASS(obj_Mech);

CMechUberEquip::CMechUberEquip()
{
}

CMechUberEquip::~CMechUberEquip()
{
	Reset();
}

void CMechUberEquip::LoadSlot(int slotId, const char* fname)
{
	slot_s& sl = slots_[slotId];
	sl.mesh = NULL;
	
	if(strstr(fname, "@EMPTY") != NULL)
		return;

	sl.mesh = r3dGOBAddMesh(fname);
	sl.name = strrchr(fname, '\\') + 1;
}

D3DXMATRIX CMechUberEquip::getWeaponBone(int idx, const r3dSkeleton* skel, const D3DXMATRIX& offset)
{
	D3DXMATRIX mr1, world;
	D3DXMatrixRotationYawPitchRoll(&mr1, 0, R3D_PI/2, 0);
	if(idx == 1)      skel->GetBoneWorldTM("MechWeapon1", &world, offset);
	else if(idx == 2) skel->GetBoneWorldTM("MechWeapon2", &world, offset);
	else if(idx == 3) skel->GetBoneWorldTM("MechWeapon3", &world, offset);
	world = mr1 * world;
	return world;
}

void CMechUberEquip::Reset()
{
	for(int i=0; i<MSLOT_Max; ++i)
	{
		// do not delete model, cache will do it
		slots_[i].mesh = NULL;
	}
}

void CMechUberEquip::DrawSlot(int slotId, const D3DXMATRIX& world, DrawType dt, bool skin)
{
	r3dMesh* mesh = slots_[slotId].mesh;
	if(mesh == NULL)
		return;

	if(skin)
	{
		r3dMeshSetVSConsts(world, NULL, NULL);
	}
	else
	{
		mesh->SetVSConsts(world);

		// NOTE : needed for transparent camo only..
		// float4   WorldScale  		: register(c24);
		D3DXVECTOR4 scale(mesh->unpackScale.x, mesh->unpackScale.y, mesh->unpackScale.z, 0.f) ;
		D3D_V(r3dRenderer->pd3ddev->SetVertexShaderConstantF(24, (float*)&scale, 1)) ;
	}

	switch(dt)
	{
	case DT_DEFERRED:
		{
			r3dBoundBox worldBBox = mesh->localBBox;
			worldBBox.Transform(reinterpret_cast<const r3dMatrix *>(&world));
			// Vertex lights for forward transparent renderer.
			for (int i = 0; i < mesh->NumMatChunks; i++)
			{
				SetLightsIfTransparent(mesh->MatChunks[i].Mat, worldBBox);
			}

			mesh->DrawMeshDeferred(r3dColor::white, 0);
			break ;
		}

	case DT_DEPTH:
		if(mesh->IsSkeletal())
			r3dRenderer->SetVertexShader(VS_SKIN_DEPTH_ID) ;
		else
			r3dRenderer->SetVertexShader(VS_DEPTH_ID) ;

		// NOTE : no break on purpose

	case DT_AURA:
		mesh->DrawMeshWithoutMaterials();
		break ;

	case DT_SHADOWS:
		mesh->DrawMeshShadows();
		break ;
	}
}

void CMechUberEquip::Draw(const r3dSkeleton* skel, const D3DXMATRIX& CharMat, bool draw_weapon, DrawType dt)
{
	//todo: call extern void r3dMeshSetWorldMatrix(const D3DXMATRIX& world)
	// instead of mesh->SetWorldMatrix

	skel->SetShaderConstants();

	for(int i=0; i<=MSLOT_Mech4; i++)
	{
		DrawSlot(i, CharMat, dt, true);
	}

	if(dt != DT_AURA)
	{
		D3DXMATRIX world1 = getWeaponBone(1, skel, CharMat);
		D3DXMATRIX world2 = getWeaponBone(1, skel, CharMat);
		D3DXMATRIX world3 = getWeaponBone(1, skel, CharMat);

		DrawSlot(MSLOT_Weapon1, world1, dt, false);
		DrawSlot(MSLOT_Weapon2, world2, dt, false);
		DrawSlot(MSLOT_Weapon3, world3, dt, false);
	}
}

void CMechUberEquip::DrawSM(const r3dSkeleton* skel, const D3DXMATRIX& CharMat, bool draw_weapon)
{
	skel->SetShaderConstants();

	for(int i=0; i<=MSLOT_Mech4; i++) 
	{
		DrawSlot(i, CharMat, CMechUberEquip::DT_SHADOWS, true);
	}

	// not rigged parts
	D3DXMATRIX world1 = getWeaponBone(1, skel, CharMat);
	D3DXMATRIX world2 = getWeaponBone(1, skel, CharMat);
	D3DXMATRIX world3 = getWeaponBone(1, skel, CharMat);

	DrawSlot(MSLOT_Weapon1, world1, CMechUberEquip::DT_SHADOWS, false);
	DrawSlot(MSLOT_Weapon2, world2, CMechUberEquip::DT_SHADOWS, false);
	DrawSlot(MSLOT_Weapon3, world3, CMechUberEquip::DT_SHADOWS, false);
}

void _mech_AdjustBoneCallback(DWORD dwData, int boneId, D3DXMATRIX &mp, D3DXMATRIX &anim)
{
	obj_Mech* npc = (obj_Mech*)dwData;

	if(boneId == npc->boneId_MechPelvis) 
	{
		float xRot = npc->adjBody;
		float yRot = 0;

		D3DXMATRIX r1, r2;

		// rotate previos matrix, with saved position
		r3dPoint3D vv = r3dPoint3D(mp._41, mp._42, mp._43);
		D3DXMatrixRotationY(&r2, xRot);
		D3DXMatrixRotationX(&r1, -yRot);
		mp = mp * (r1 * r2);
		mp._41 = vv.x;
		mp._42 = vv.y;
		mp._43 = vv.z;
		return;
	}
	
	if(boneId == npc->boneId_Weapon1 || boneId == npc->boneId_Weapon2 || boneId == npc->boneId_Weapon3)
	{
		float yRot = 0;
		if(boneId == npc->boneId_Weapon1) yRot = npc->adjWeap1;
		if(boneId == npc->boneId_Weapon2) yRot = npc->adjWeap2;
		if(boneId == npc->boneId_Weapon3) yRot = npc->adjWeap3;

		// rotate animation
		D3DXMATRIX r1;
		D3DXMatrixRotationY(&r1, yRot);
		anim = r1 * anim;
		return;
	}
	return;    
}

obj_Mech::obj_Mech()
{
	m_BindSkeleton = NULL;
	m_Animation = NULL;

	sSkelSelected[0] = 0;
	fSkelListOffset = 0;

	sAnimSelected[0] = 0;
	fAnimListOffset = 0;

	adjBody = 0;
	adjWeap1 = 0;
	adjWeap2 = 0;
	adjWeap3 = 0;
	
	m_renderSkel = 1;
}

obj_Mech::~obj_Mech()
{
	SAFE_DELETE(m_BindSkeleton);
}

BOOL obj_Mech::Load(const char* fname)
{
	if(!parent::Load(fname))
		return FALSE;

	sDir = "data\\objectsdepot\\mech\\";
	
	r3dBoundBox bbox;
	bbox.Org = r3dPoint3D(-0.5f, 0, -0.f);
	bbox.Size = r3dPoint3D(1, 2, 1);
	SetBBoxLocal(bbox);

	return TRUE;
}

BOOL obj_Mech::Update()
{
	if(!m_BindSkeleton)
		return parent::Update();
		
	m_Animation->Update(r3dGetFrameTime(), r3dPoint3D(0, 0, 0), mTransform);
	m_Animation->GetCurrentSkeleton();

	return parent::Update();
}

void obj_Mech::ReloadSkeleton()
{
	uberEquip_.Reset();
	
	char buf[256];
	sprintf(buf, "%s%s", sDir.c_str(), sSkelSelected);

	SAFE_DELETE(m_BindSkeleton);
	m_BindSkeleton = new r3dSkeleton();
	m_BindSkeleton->LoadBinary(buf);
	extern void r3dDumpSkeleton(const r3dSkeleton* skel, int bone);
	r3dDumpSkeleton(m_BindSkeleton, -1);
	
	boneId_MechPelvis = m_BindSkeleton->GetBoneID("MechSpine");
	boneId_Weapon1    = m_BindSkeleton->GetBoneID("MechWeapon1");
	boneId_Weapon2    = m_BindSkeleton->GetBoneID("MechWeapon2");
	boneId_Weapon3    = m_BindSkeleton->GetBoneID("MechWeapon3");

	SAFE_DELETE(m_Animation);
	m_Animation = new r3dAnimation();
	m_Animation->Init(m_BindSkeleton, &m_AnimPool, _mech_AdjustBoneCallback, (DWORD)this);
}

float obj_Mech::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	//@ starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( ! IsParentOrEqual( &ClassData, startClass ) )	
		return starty - scry ;

	starty += imgui_Static(scrx, starty, "obj_Mech properties");

	// skeleton change
	starty += imgui_Static(scrx, starty, " Select Skeleton");
	{
		std::string sDirFind = sDir + "*.skl";
		if(imgui_FileList(scrx, starty, 360, 60, sDirFind.c_str(), sSkelSelected, &fSkelListOffset, true))
		{
			ReloadSkeleton();
		}
		starty += 60;
	}
	
	if(m_BindSkeleton == NULL)
		return starty - scry ;
	
	// animation
	starty += imgui_Static(scrx, starty, " Select Animation");
	{
		std::string sDirFind = sDir + "Animations\\*.anm";
		if(imgui_FileList(scrx, starty, 360, 100, sDirFind.c_str(), sAnimSelected, &fAnimListOffset, true))
		{
			char buf[512];
			sprintf(buf, "%sAnimations\\%s", sDir.c_str(), sAnimSelected);
			int aid = m_AnimPool.Add(sAnimSelected, buf);
			m_Animation->StartAnimation(aid, ANIMFLAG_Looped, 1.0f, 1.0f, 0.0f);
			
		}
		starty += 100;
	}
	
	starty += imgui_Checkbox(scrx, starty, "Show Skeleton", &m_renderSkel, 1);

	starty += imgui_Value_Slider(scrx, starty, "bodyRot", &adjBody, -1.5, 1.5, "%.2f");
	starty += imgui_Value_Slider(scrx, starty, "weap1Rot", &adjWeap1, -1.5, 1.5, "%.2f");
	starty += imgui_Value_Slider(scrx, starty, "weap2Rot", &adjWeap2, -1.5, 1.5, "%.2f");
	starty += imgui_Value_Slider(scrx, starty, "weap3Rot", &adjWeap3, -1.5, 1.5, "%.2f");
	
	static int showEquip = 1;
	starty += imgui_Checkbox(scrx, starty, "Show Equip Mgr", &showEquip, 1);
	
	if(!showEquip)
		return starty - scry ;

	float eqx = 10;
	float eqh = 140;
	float eqy = r3dRenderer->ScreenH - 360;
	float eqw = 120;
	for(int i=0; i<MSLOT_Max; i++) 
	{
		char buf[265];
		sprintf(buf, "%s%d : %s", 
			i < MSLOT_Weapon1 ? "Slot" : "Wpn",
			i < MSLOT_Weapon1 ? (i + 1) : (i - MSLOT_Weapon1 + 1),
			uberEquip_.slots_[i].mesh == NULL ? "empty" : uberEquip_.slots_[i].name.c_str());
		starty += imgui_Static(scrx, starty, buf);
			
		static float fEqOffsets[20] = {0};
		static int   iEqIdx[20] = {0};
		char sconame[256];

		//imgui_Static(left + width * 4, top, "Side Weapon", width);
		Desktop().Deactivate();
		std::string sDirFind = sDir + "*.sco";
		if(imgui_FileList(eqx, eqy, eqw, eqh, sDirFind.c_str(), sconame, &fEqOffsets[i], false ))
		{
			sprintf(buf, "%s%s", sDir.c_str(), sconame);
			uberEquip_.LoadSlot(i, buf);
		}
		Desktop().Activate();
		
		eqx += eqw + 3;
	}

	return starty - scry ;
}

BOOL obj_Mech::OnCreate()
{
	parent::OnCreate();
	
	return 1;
}

struct obj_MechDeferredRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		obj_MechDeferredRenderable* This = static_cast<obj_MechDeferredRenderable*>( RThis );
		if(This->Parent->m_BindSkeleton == NULL)
			return;

		float vCData[ 4 ] = { 1, 1, 1, 0 } ;
		D3D_V( r3dRenderer->pd3ddev->SetPixelShaderConstantF( MC_MASKCOLOR, vCData, 1 ) );

		int oldVsId = r3dRenderer->GetCurrentVertexShaderIdx();
		This->Parent->uberEquip_.Draw(This->Parent->m_Animation->GetCurrentSkeleton(), This->Parent->GetMtx(), true, CMechUberEquip::DT_DEFERRED);
		r3dRenderer->SetVertexShader(oldVsId);
	}

	obj_Mech* Parent;
};

struct obj_MechShadowRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		obj_MechShadowRenderable* This = static_cast<obj_MechShadowRenderable*>( RThis );
		if(This->Parent->m_BindSkeleton == NULL)
			return;
		int oldVsId = r3dRenderer->GetCurrentVertexShaderIdx();
		This->Parent->uberEquip_.DrawSM(This->Parent->m_Animation->GetCurrentSkeleton(), This->Parent->GetMtx(), true);		
		r3dRenderer->SetVertexShader(oldVsId);
	}

	obj_Mech* Parent;
};

struct obj_MechDebugRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		obj_MechDebugRenderable* This = static_cast<obj_MechDebugRenderable*>( RThis );
		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		r3dDrawBoundBox(This->Parent->GetBBoxWorld(), Cam, r3dColor(0, 0, 255), 0.2f);
		if(This->Parent->m_BindSkeleton)
		{
			This->Parent->m_Animation->GetCurrentSkeleton()->DrawSkeleton(Cam, This->Parent->GetPosition());
		}
	}

	obj_Mech* Parent;
};

#define	RENDERABLE_BUILDING_SORT_VALUE (31*RENDERABLE_USER_SORT_VALUE)

void obj_Mech::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam )
{
	obj_MechShadowRenderable rend;
	rend.Init();
	rend.Parent	= this;
	rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

	rarr.PushBack( rend );
}

void obj_Mech::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	// deferred
	{
		obj_MechDeferredRenderable rend;
		rend.Init();
		rend.Parent	= this;
		rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

		render_arrays[ rsFillGBuffer ].PushBack( rend );
	}

	// skeleton draw
	if(m_renderSkel)
	{
		obj_MechDebugRenderable rend;
		rend.Init();
		rend.Parent	= this;
		rend.SortValue	= RENDERABLE_BUILDING_SORT_VALUE;

		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}
}

#endif
