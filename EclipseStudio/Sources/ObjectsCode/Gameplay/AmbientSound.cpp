#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "GameObjects\GameObj.h"
#include "GameObjects\ObjManag.h"
#include "../../Editors/ObjectManipulator3d.h"

#include "rendering/Deffered/DeferredHelpers.h"

#include "AmbientSound.h"

IMPLEMENT_CLASS(obj_AmbientSound, "obj_AmbientSound", "Object");
AUTOREGISTER_CLASS(obj_AmbientSound);

static r3dTexture *SoundIcon = NULL;

obj_AmbientSound::obj_AmbientSound()
: sndEvent(0)
, sndID(-1)
{
	memset(SoundFilename, 0, sizeof(SoundFilename));
	ObjTypeFlags |= OBJTYPE_Sound;
}

BOOL obj_AmbientSound::Load(const char *fname)
{
	if (!parent::Load(fname)) return FALSE;

	if (!SoundIcon) SoundIcon = r3dRenderer->LoadTexture("Data\\Images\\Sound.dds");

	return TRUE;
}

BOOL obj_AmbientSound::OnCreate()
{
	parent::OnCreate();

	DrawOrder	= OBJ_DRAWORDER_LAST;
	ObjFlags	|=	OBJFLAG_SkipOcclusionCheck
						|
					OBJFLAG_DisableShadows 
						|
					OBJFLAG_ForceSleep
					;

	r3dBoundBox bboxLocal ;

	bboxLocal.Size = r3dPoint3D(2,2,2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;

	SetBBoxLocal( bboxLocal ) ;

	UpdateTransform();

	return 1;
}

void obj_AmbientSound::LoadNewSound(int sndID, bool paused)
{
	if (sndEvent)
	{
		SoundSys.Stop(sndEvent);
		SoundSys.Release(sndEvent);
		sndEvent = 0;
	}

	sndEvent = SoundSys.Play(sndID, GetPosition());
}
#ifndef FINAL_BUILD

void
obj_AmbientSound::UpdateSound( const std::string& soundPath )
{
	strcpy_s(SoundFilename, _countof(SoundFilename), soundPath.c_str());
	sndID = SoundSys.GetEventIDByPath(soundPath.c_str());
	SoundSys.Release(sndEvent);
}

float	obj_AmbientSound::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{

	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{
		starty += imgui_Static ( scrx, starty, "Sound Properties" );
		starty += imgui_Static ( scrx, starty, "Ambient Sound Source:" );
		starty += imgui_Static ( scrx, starty, SoundFilename );
		
		const stringlist_t & sl = SoundSys.GetSoundsList();
		static float offset = 0;
		static int selectedSoundIndex = 0;
		int prevIdx = selectedSoundIndex;
		imgui_DrawList(scrx, starty, 360.0f, 122.0f, sl, &offset, &selectedSoundIndex);
		starty += 122.0f;
		if (prevIdx != selectedSoundIndex)
		{
			const std::string &newPath = sl[selectedSoundIndex].c_str();
			PropagateChange(newPath, &obj_AmbientSound::UpdateSound, selected ) ;
		}
	}
	
	return starty-scry;
}
#endif

BOOL obj_AmbientSound::OnDestroy()
{
	if (sndEvent)
	{
		SoundSys.Stop(sndEvent);
		SoundSys.Release(sndEvent);
		sndEvent = 0;
	}
	return parent::OnDestroy();
}

BOOL obj_AmbientSound::Update()
{
	const r3dPoint3D &pos = GetPosition();
	if (!SoundSys.IsHandleValid(sndEvent))
	{
		SoundSys.Release(sndEvent);
		sndEvent = SoundSys.Play(sndID, pos);
	}
	else
	{
		if (!SoundSys.IsAudible(sndEvent, pos))
		{
			SoundSys.Release(sndEvent);
		}
		else
		{
			SoundSys.Set3DAttributes(sndEvent, &pos, 0, 0);
		}
	}
	return 1;
}

struct AmbientSoundRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		AmbientSoundRenderable *This = static_cast<AmbientSoundRenderable*>( RThis );

		This->Parent->DoDraw();
	}

	obj_AmbientSound* Parent;
};

void r3dDrawIcon3D(const r3dPoint3D& pos, r3dTexture *icon, const r3dColor &Col, float size);
struct SoundHelperRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		SoundHelperRenderable *This = static_cast<SoundHelperRenderable*>( RThis );

		r3dRenderer->SetTex(NULL);
		r3dRenderer->SetMaterial(NULL);
		r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );

		if((This->Parent->GetPosition() - Cam).Length() < 100)
			r3dDrawIcon3D(This->Parent->GetPosition(), SoundIcon, r3dColor(255,255,255), 32);

		r3dRenderer->SetRenderingMode( R3D_BLEND_NOALPHA | R3D_BLEND_ZC );
	}

	obj_AmbientSound* Parent;	
};

#define RENDERABLE_SOUND_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_AmbientSound::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )  /*OVERRIDE*/
{
#ifdef FINAL_BUILD
	return;
#else
	// don't draw debug info if we're not in editor mode
	extern bool g_bEditMode;
	if ( !g_bEditMode )
		return;

	if( r_hide_icons->GetInt() )
		return ;

	{
		AmbientSoundRenderable rend;
		rend.Init();
		rend.SortValue = RENDERABLE_SOUND_USER_SORT_VALUE;
		rend.Parent = this;
		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}

	// helper
	extern int CurHUDID;
	if(CurHUDID == 0)
	{
		SoundHelperRenderable rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_SOUND_USER_SORT_VALUE;
		render_arrays[ rsDrawComposite1 ].PushBack( rend );
	}
#endif
}

int g_bEnableSoundRadiusDraw = 0;
extern ObjectManipulator3d g_Manipulator3d;
void obj_AmbientSound::DoDraw()
{
#ifndef FINAL_BUILD
	if(g_bEnableSoundRadiusDraw || g_Manipulator3d.IsSelected(this))
	{
		r3dColor clr = r3dColor::yellow;
		clr.A = 128;
		r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_ZC );

		float maxDist = SoundSys.GetSoundMaxDistance(sndID);

		r3dDrawUniformSphere ( GetPosition(), maxDist, gCam, clr );
		r3dRenderer->Flush();
	}
#endif
}

void obj_AmbientSound::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);
	pugi::xml_node ambientSoundNode = node.child("ambientSound");
	const char *fn = ambientSoundNode.attribute("Sound").value();
	strcpy_s(SoundFilename, _countof(SoundFilename), fn);
	sndID = SoundSys.GetEventIDByPath(SoundFilename);
}

void obj_AmbientSound::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);
	pugi::xml_node ambientSoundNode = node.append_child();
	ambientSoundNode.set_name("ambientSound");
	ambientSoundNode.append_attribute("Sound") = SoundFilename;
}

