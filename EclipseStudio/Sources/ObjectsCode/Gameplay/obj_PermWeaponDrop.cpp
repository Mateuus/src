#include "r3dPCH.h"
#include "r3d.h"

#include "Gameplay_Params.h"
#include "GameCommon.h"
#include "obj_PermWeaponDrop.h"
#include "../ai/AI_Player.H"
#include "../WEAPONS/WeaponArmory.h"

#include "Editors/ObjectManipulator3d.h"

IMPLEMENT_CLASS(obj_PermWeaponDrop, "obj_PermWeaponDrop", "Object");
AUTOREGISTER_CLASS(obj_PermWeaponDrop);

extern bool g_bEditMode;

static r3dTexture *WeaponDropIcon = NULL;

obj_PermWeaponDrop::obj_PermWeaponDrop()
{
	m_weaponID = 0;
	attm0 = attm1 = attm2 = attm3 = attm4 = 0;
}

obj_PermWeaponDrop::~obj_PermWeaponDrop()
{
}

BOOL obj_PermWeaponDrop::Load(const char *fname)
{
	if(!parent::Load(fname)) return FALSE;

	if(g_bEditMode)
		if (!WeaponDropIcon) WeaponDropIcon = r3dRenderer->LoadTexture("Data\\Images\\WeaponDrop.dds");

	return TRUE;
}

BOOL obj_PermWeaponDrop::OnCreate()
{
	parent::OnCreate();

	DrawOrder	= OBJ_DRAWORDER_LAST;

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;
	SetBBoxLocal( bboxLocal ) ;
	UpdateTransform();

	if( m_weaponID > 0 )
	{
		const WeaponConfig* wcfg = gWeaponArmory.getWeaponConfig( m_weaponID ) ;

		if( wcfg )
			wcfg->aquireMesh( true ) ;
	}

	return 1;
}

/*virtual*/
BOOL
obj_PermWeaponDrop::OnDestroy()
{
	if( m_weaponID > 0 )
	{
		const WeaponConfig* wcfg = gWeaponArmory.getWeaponConfig( m_weaponID ) ;

		if( wcfg )
			wcfg -> releaseMesh () ;
	}

	return parent::OnDestroy() ;
}

BOOL obj_PermWeaponDrop::Update()
{
	return parent::Update();
}

void obj_PermWeaponDrop::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node myNode = node.child("PermWeaponDrop");
	m_weaponID = myNode.attribute("weaponID").as_uint();
	attm0 = myNode.attribute("attm0").as_uint();
	attm1 = myNode.attribute("attm1").as_uint();
	attm2 = myNode.attribute("attm2").as_uint();
	attm3 = myNode.attribute("attm3").as_uint();
	attm4 = myNode.attribute("attm4").as_uint();
}

void obj_PermWeaponDrop::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node myNode = node.append_child();
	myNode.set_name("PermWeaponDrop");
	myNode.append_attribute("weaponID") = m_weaponID;
	myNode.append_attribute("attm0") = attm0;
	myNode.append_attribute("attm1") = attm1;
	myNode.append_attribute("attm2") = attm2;
	myNode.append_attribute("attm3") = attm3;
	myNode.append_attribute("attm4") = attm4;
}

void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
struct PermWeaponDropRenderableHelper : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PermWeaponDropRenderableHelper *This = static_cast<PermWeaponDropRenderableHelper*>( RThis );

		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZC );

		if((This->Parent->GetPosition() - Cam).Length() < 100)
			r3dDrawIcon3D(This->Parent->GetPosition(), WeaponDropIcon, r3dColor(255,255,255), 32);
	}

	obj_PermWeaponDrop* Parent;	
};

void obj_PermWeaponDrop::DrawWeaponMesh(eRenderStageID DrawState)
{
	const WeaponConfig* wc = gWeaponArmory.getWeaponConfig(m_weaponID);
	if(wc)
	{
		r3dMesh* weaponMesh = wc->getMesh( false, false );
		r3d_assert(weaponMesh);
		weaponMesh->SetVSConsts(mTransform);

		switch( DrawState )
		{
		case rsFillGBuffer:
			weaponMesh->DrawMeshDeferred( r3dColor::white, 0 ) ;
			break ;

		case rsCreateSM:
			weaponMesh->DrawMeshShadows();
			break ;

		default:
			weaponMesh->DrawMeshSimple( 0 );
			break ;
		}
	}
}

struct PermWeaponDropShadowGBufferRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		PermWeaponDropShadowGBufferRenderable *This = static_cast<PermWeaponDropShadowGBufferRenderable*>( RThis );

		This->Parent->DrawWeaponMesh( This->DrawState );
	}

	obj_PermWeaponDrop*	Parent;
	eRenderStageID		DrawState;
};



#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_PermWeaponDrop::AppendShadowRenderables( RenderArray & rarr, const r3dCamera& Cam ) /*OVERRIDE*/
{
	if ( !g_bEditMode )
		return;

	PermWeaponDropShadowGBufferRenderable rend;

	rend.Init();
	rend.Parent		= this;
	rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
	rend.DrawState	= rsCreateSM;

	rarr.PushBack( rend );
}

void obj_PermWeaponDrop::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
#ifdef FINAL_BUILD
	return;
#else
	if ( !g_bEditMode )
		return;

	if(m_weaponID > 0)
	{
		// gbuffer
		{
			PermWeaponDropShadowGBufferRenderable rend;

			rend.Init();
			rend.Parent		= this;
			rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
			rend.DrawState	= rsFillGBuffer;

			render_arrays[ rsFillGBuffer ].PushBack( rend );
		}
	}

	if( r_hide_icons->GetInt() )
		return ;

	// helper
	extern int CurHUDID;
	if(CurHUDID == 0)
	{
		PermWeaponDropRenderableHelper rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}
#endif
}

#ifndef FINAL_BUILD
float obj_PermWeaponDrop::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor( scrx, scry, scrw, scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		starty += imgui_Static ( scrx, starty, "Permanent Weapon Drop" );

		stringlist_t weaponDB_nameList;
		uint32_t* weaponDB_idList = 0;
		stringlist_t attachmentDB_muzzle_nameList;
		uint32_t* attachmentDB_muzzle_idList = 0;
		stringlist_t attachmentDB_upperRail_nameList;
		uint32_t* attachmentDB_upperRail_idList = 0;
		stringlist_t attachmentDB_leftRail_nameList;
		uint32_t* attachmentDB_leftRail_idList = 0;
		stringlist_t attachmentDB_bottomRail_nameList;
		uint32_t* attachmentDB_bottomRail_idList = 0;
		stringlist_t attachmentDB_clip_nameList;
		uint32_t* attachmentDB_clip_idList = 0;

		if(weaponDB_nameList.empty()) // fill in list
		{
			attachmentDB_muzzle_nameList.push_back("empty");
			attachmentDB_upperRail_nameList.push_back("empty");
			attachmentDB_leftRail_nameList.push_back("empty");
			attachmentDB_bottomRail_nameList.push_back("empty");
			attachmentDB_clip_nameList.push_back("empty");

			int numItemsInWeaponList = gWeaponArmory.getNumWeapons();
			weaponDB_idList = new uint32_t[numItemsInWeaponList];
			for(int i=0; i<numItemsInWeaponList; ++i)
			{
				const WeaponConfig* config = gWeaponArmory.getWeaponConfigByIndex(i);
				weaponDB_nameList.push_back(config->m_StoreName);
				weaponDB_idList[i] = config->m_itemID;
			}

			int numItemsInAttachmentList = gWeaponArmory.getNumAttachments() + 1;
			attachmentDB_muzzle_idList = new uint32_t[numItemsInAttachmentList]; int numMuzzles = 0;
			attachmentDB_muzzle_idList[0] = 0;
			attachmentDB_upperRail_idList = new uint32_t[numItemsInAttachmentList]; int numUpperRails = 0;
			attachmentDB_upperRail_idList[0] = 0;
			attachmentDB_leftRail_idList = new uint32_t[numItemsInAttachmentList]; int numLeftRails = 0;
			attachmentDB_leftRail_idList[0] = 0;
			attachmentDB_bottomRail_idList = new uint32_t[numItemsInAttachmentList]; int numBottomRails = 0;
			attachmentDB_bottomRail_idList[0] = 0;
			attachmentDB_clip_idList = new uint32_t[numItemsInAttachmentList]; int numClips = 0;
			attachmentDB_clip_idList[0] = 0;
			for(int i=1; i<numItemsInAttachmentList; ++i)
			{
				const WeaponAttachmentConfig* config = gWeaponArmory.getAttachmentConfigByIndex(i-1);
				switch(config->m_type)
				{
				case WPN_ATTM_MUZZLE:
					attachmentDB_muzzle_nameList.push_back(config->m_StoreName);
					attachmentDB_muzzle_idList[++numMuzzles] = config->m_itemID;
					break;
				case WPN_ATTM_UPPER_RAIL:
					attachmentDB_upperRail_nameList.push_back(config->m_StoreName);
					attachmentDB_upperRail_idList[++numUpperRails] = config->m_itemID;
					break;
				case WPN_ATTM_LEFT_RAIL:
					attachmentDB_leftRail_nameList.push_back(config->m_StoreName);
					attachmentDB_leftRail_idList[++numLeftRails] = config->m_itemID;
					break;
				case WPN_ATTM_BOTTOM_RAIL:
					attachmentDB_bottomRail_nameList.push_back(config->m_StoreName);
					attachmentDB_bottomRail_idList[++numBottomRails] = config->m_itemID;
					break;
				case WPN_ATTM_CLIP:
					attachmentDB_clip_nameList.push_back(config->m_StoreName);
					attachmentDB_clip_idList[++numClips] = config->m_itemID;
					break;
				default:
					break;
				}
			}
		}

		static const int width = 250;
		static const int shift = 25;

		static float primWeaponOffset = 0;
		static int selectedPrimaryWeaponIndex = 0;
		if(m_weaponID != weaponDB_idList[selectedPrimaryWeaponIndex]) // restore selected item after loading level
		{

			for(uint32_t i=0; i<weaponDB_nameList.size(); ++i)
			{
				if(weaponDB_idList[i]==m_weaponID)
				{
					selectedPrimaryWeaponIndex = i;
					break;
				}
			}
		}
		imgui_DrawList(scrx, starty, (float)width, 150.0f, weaponDB_nameList, &primWeaponOffset, &selectedPrimaryWeaponIndex);
		starty += 150;

		static int selectedPrimaryWeapon_Muzzle = 0;
		static int selectedPrimaryWeapon_UpperRail = 0;
		static int selectedPrimaryWeapon_LeftRail = 0;
		static int selectedPrimaryWeapon_BottomRail= 0;
		static int selectedPrimaryWeapon_Clip = 0;

		{
			static float primWeaponAttachmentMuzzleOffset = 0;
			if(attm0 != attachmentDB_muzzle_idList[selectedPrimaryWeapon_Muzzle])
			{
				for(uint32_t i=0; i<attachmentDB_muzzle_nameList.size(); ++i)
				{
					if(attachmentDB_muzzle_idList[i]==attm0)
					{
						selectedPrimaryWeapon_Muzzle = i;
						break;
					}
				}
			}
			imgui_Static(scrx, starty, "Muzzle", width); starty += 25;
			imgui_DrawList(scrx, starty, (float)width, (float)100.0f, attachmentDB_muzzle_nameList, &primWeaponAttachmentMuzzleOffset, &selectedPrimaryWeapon_Muzzle);
			starty += 100;

			static float primWeaponAttachmentUpperRailOffset = 0;
			if(attm1 != attachmentDB_upperRail_idList[selectedPrimaryWeapon_UpperRail])
			{
				for(uint32_t i=0; i<attachmentDB_upperRail_nameList.size(); ++i)
				{
					if(attachmentDB_upperRail_idList[i]==attm1)
					{
						selectedPrimaryWeapon_UpperRail = i;
						break;
					}
				}
			}
			imgui_Static(scrx, starty, "Upper Rail", width); starty += 25;
			imgui_DrawList(scrx, starty, (float)width, (float)100.0f, attachmentDB_upperRail_nameList, &primWeaponAttachmentUpperRailOffset, &selectedPrimaryWeapon_UpperRail);
			starty += 100;

			static float primWeaponAttachmentLeftRailOffset = 0;
			if(attm2 != attachmentDB_leftRail_idList[selectedPrimaryWeapon_LeftRail])
			{
				for(uint32_t i=0; i<attachmentDB_leftRail_nameList.size(); ++i)
				{
					if(attachmentDB_leftRail_idList[i]==attm2)
					{
						selectedPrimaryWeapon_LeftRail = i;
						break;
					}
				}
			}
			imgui_Static(scrx, starty, "Left Rail", width); starty += 25;
			imgui_DrawList(scrx, starty, (float)width, (float)100.0f, attachmentDB_leftRail_nameList, &primWeaponAttachmentLeftRailOffset, &selectedPrimaryWeapon_LeftRail);
			starty += 100;

			static float primWeaponAttachmentBottomRailOffset = 0;
			if(attm3 != attachmentDB_bottomRail_idList[selectedPrimaryWeapon_BottomRail])
			{
				for(uint32_t i=0; i<attachmentDB_bottomRail_nameList.size(); ++i)
				{
					if(attachmentDB_bottomRail_idList[i]==attm3)
					{
						selectedPrimaryWeapon_BottomRail = i;
						break;
					}
				}
			}
			imgui_Static(scrx, starty, "Bottom Rail", width); starty += 25;
			imgui_DrawList(scrx, starty, (float)width, (float)100.0f, attachmentDB_bottomRail_nameList, &primWeaponAttachmentBottomRailOffset, &selectedPrimaryWeapon_BottomRail);
			starty += 100;

			static float primWeaponAttachmentClipOffset = 0;
			if(attm4 != attachmentDB_clip_idList[selectedPrimaryWeapon_Clip])
			{
				for(uint32_t i=0; i<attachmentDB_clip_nameList.size(); ++i)
				{
					if(attachmentDB_clip_idList[i]==attm4)
					{
						selectedPrimaryWeapon_Clip = i;
						break;
					}
				}
			}
			imgui_Static(scrx, starty, "Clip", width); starty += 25;
			imgui_DrawList(scrx, starty, (float)width, (float)100.0f, attachmentDB_clip_nameList, &primWeaponAttachmentClipOffset, &selectedPrimaryWeapon_Clip);
			starty += 100;
		}

		uint32_t id = 0;

		id = attachmentDB_muzzle_idList[selectedPrimaryWeapon_Muzzle]; PropagateChange( id, &obj_PermWeaponDrop::attm0, this, selected );
		id = attachmentDB_upperRail_idList[selectedPrimaryWeapon_UpperRail]; PropagateChange( id, &obj_PermWeaponDrop::attm1, this, selected );
		id = attachmentDB_leftRail_idList[selectedPrimaryWeapon_LeftRail]; PropagateChange( id, &obj_PermWeaponDrop::attm2, this, selected );
		id = attachmentDB_bottomRail_idList[selectedPrimaryWeapon_BottomRail]; PropagateChange( id, &obj_PermWeaponDrop::attm3, this, selected );
		id = attachmentDB_clip_idList[selectedPrimaryWeapon_Clip]; PropagateChange( id, &obj_PermWeaponDrop::attm4, this, selected );

		id = weaponDB_idList[selectedPrimaryWeaponIndex];

		bool reaquireMesh = id != m_weaponID;
		if(reaquireMesh)
		{
			const WeaponConfig* wcfg = gWeaponArmory.getWeaponConfig( m_weaponID );
			if( wcfg )
				wcfg->releaseMesh();

			wcfg = gWeaponArmory.getWeaponConfig( id );
			if( wcfg )
				wcfg->aquireMesh(true);
		}

		PropagateChange( id, &obj_PermWeaponDrop::m_weaponID, this, selected );

		delete [] weaponDB_idList;
		delete [] attachmentDB_muzzle_idList;
		delete [] attachmentDB_upperRail_idList;
		delete [] attachmentDB_leftRail_idList;
		delete [] attachmentDB_bottomRail_idList;
		delete [] attachmentDB_clip_idList;
	}

	return starty-scry;
}
#endif
