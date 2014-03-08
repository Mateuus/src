#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "GameLevel.h"
#include "obj_DBFMasterControl.h"

#include "Editors/ObjectManipulator3d.h"

IMPLEMENT_CLASS(obj_DBFMasterControl, "obj_DBFMasterControl", "Object");
AUTOREGISTER_CLASS(obj_DBFMasterControl);


obj_DBFMasterControl::obj_DBFMasterControl ()
{
	ObjFlags |= OBJFLAG_SkipOcclusionCheck
					|
				OBJFLAG_DisableShadows
					|
				OBJFLAG_ForceSleep
					;
}

//------------------------------------------------------------------------

obj_DBFMasterControl::~obj_DBFMasterControl ()
{
}

//------------------------------------------------------------------------
/*virtual*/
BOOL
obj_DBFMasterControl::OnCreate()
{
	return GameObject::OnCreate();
}

//------------------------------------------------------------------------
/*virtual*/
BOOL
obj_DBFMasterControl::OnDestroy()
{
	return GameObject::OnDestroy();
}


//------------------------------------------------------------------------

BOOL obj_DBFMasterControl::Update()
{
	float f = 2.f;

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D( f, f, f );
	bboxLocal.Org = -bboxLocal.Size / 2;

	SetBBoxLocal( bboxLocal ) ;
	UpdateTransform();
	return TRUE;
}

//------------------------------------------------------------------------
void obj_DBFMasterControl::ReadSerializedData(pugi::xml_node& node)
{
/*
	GameObject::ReadSerializedData(node);
	pugi::xml_node envmapNode = node.child("envmapProbe");
	float fMaxRadius;
	int id ( WRONG_ID );
	int isInfinite;
	fMaxRadius = envmapNode.attribute("radius").as_float();
	isInfinite = envmapNode.attribute("infRadius").as_int();
*/
}
//------------------------------------------------------------------------
void obj_DBFMasterControl::WriteSerializedData(pugi::xml_node& node)
{
/*
	GameObject::WriteSerializedData(node);
	pugi::xml_node envmapNode = node.append_child();
	envmapNode.set_name("envmapProbe");
	envmapNode.append_attribute("radius") = mRadius;
	envmapNode.append_attribute("infRadius") = mInfiniteRadius;
	envmapNode.append_attribute("id") = mID;
*/
}


//------------------------------------------------------------------------
/*virtual*/
#ifndef FINAL_BUILD
float
obj_DBFMasterControl::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		float ystart = scry;

		//ystart += parent::DrawPropertyEditor(scrx, scry, scrw,scrh);
		ystart += imgui_Static ( scrx, ystart, "Global Parameters" );
		static int mInfiniteRadius = 0;
		ystart += imgui_Checkbox( scrx, ystart, "EnableSpawn", &mInfiniteRadius, 1 );
		ystart += 11;

		ystart += imgui_Static ( scrx, ystart, "Particles" );


		ystart += 11;

		ystart += imgui_Static ( scrx, ystart, "Events" );

			//Wrong Code - need to be individual buttons that can be ON/OFF independently
			const static char* listbattleedit[] = { "1", "2", "3", "4", "5" };
			static int CMode = 0;
			ystart += imgui_Toolbar(scrx, ystart, 360, 35, &CMode, 0, listbattleedit, 5);

	//	if( !mInfiniteRadius )
	//	{
	//		ystart += imgui_Value_Slider ( scrx, ystart, "Radius", &mRadius, 15, 10000, "%-02.02f" );
	//	}


		return ystart - scry ;
	}
	else
	{
		return 0 ;
	}
}
#endif

static obj_DBFMasterControl  *BFMasterControlObj = NULL;


#ifndef FINAL_BUILD
void ProcessBattlefieldEditor ()
{
	float SliderX = r3dRenderer->ScreenW-375;
	float SliderY = 85;

	BFMasterControlObj = (obj_DBFMasterControl  *)GameWorld().GetObject("objDynamicBattlefieldControl");

	g_pDesktopManager->Begin( "ed_env1" );

	SliderY += imgui_Static( SliderX, SliderY, "DYNAMIC BATTLEFIELD EDITOR" );

	if (!BFMasterControlObj)
	{
		SliderY += imgui_Static( SliderX, SliderY, "MODULE NOT PRESENT" );
		if (imgui_Button(SliderX,  SliderY, 350, 20, "Create Battlefield", 0, true))
		{
			BFMasterControlObj = (obj_DBFMasterControl  *)srv_CreateGameObject("obj_DBFMasterControl", "objDynamicBattlefieldControl", r3dPoint3D(0,0,0));
		}
	}
	else
	{
		if (imgui_Button(SliderX, SliderY, 150, 20, "Delete Master Control", 0, true))
		{
	//		GameWorld().DeleteObject(BFMasterControlObj);
	//		BFMasterControlObj = NULL;
		}
	}

	if (BFMasterControlObj)
	{
	//	SliderY += imgui_Value_Slider(SliderX, SliderY, "Brush Radius", &g_fBattleZoneBrushRadius, 0.1f, 500.0f, "%-02.2f");
		GameObjects selected ;

		g_Manipulator3d.GetPickedObjects( &selected ) ;
		BFMasterControlObj->DrawPropertyEditor(SliderX, SliderY, 350, 400, &obj_DBFMasterControl::ClassData, selected );

	}
	SliderY+= 10;

   g_pDesktopManager->End( );
}


#endif