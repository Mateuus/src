#include "r3dPCH.h"
#include "r3d.h"

#include "../SF/CmdProcessor/CmdProcessor.h"
#include "../SF/CmdProcessor/CmdConsole.h"

#include "HUD_Base.h"


BaseHUD::BaseHUD () 
: bInited ( 0 )
{
}

BaseHUD::~BaseHUD() 
{
}

void BaseHUD::Init ()
{
	if ( !bInited )
	{
		bInited = 1;
		InitPure ();
	}
}

void BaseHUD::Destroy ()
{
	if ( bInited )
	{
		bInited = 0;
		DestroyPure ();
	}
}

void BaseHUD::SetCamera ( r3dCamera &Cam )
{
	SetCameraPure ( Cam );
}

void BaseHUD::HudSelected ()
{
	OnHudSelected ();
}

void BaseHUD::HudUnselected ()
{
	OnHudUnselected ();
}

/*virtual*/
r3dPoint3D
BaseHUD::GetCamOffset() const
{
	return r3dPoint3D( 0, 0, 0 );
}

//------------------------------------------------------------------------

/*virtual*/
void
BaseHUD::SetCamPos( const r3dPoint3D& pos )
{
	FPS_Position = pos - GetCamOffset();
}

//------------------------------------------------------------------------
#ifndef FINAL_BUILD

DECLARE_CMD( dumptex )
{
	void DumpTextures();
	DumpTextures();
}

DECLARE_CMD( flushmesh )
{
	void r3dFlushReleasedMeshes() ;
	r3dFlushReleasedMeshes() ;
}

DECLARE_CMD( testfb )
{
	void TestFlashBang() ;
	TestFlashBang() ;
}

DECLARE_CMD( terrafetch )
{
	void SwitchTerraFetch() ;
	SwitchTerraFetch() ;
}

DECLARE_CMD( rts )
{
	void DumpRTStats() ;
	DumpRTStats() ;
}

DECLARE_CMD( armory )
{
	void DumpArmoryStats() ;
	DumpArmoryStats() ;
}

DECLARE_CMD( playertex )
{
	void DumpPlayerTexes() ;
	DumpPlayerTexes() ;
}

DECLARE_CMD( vmem )
{
	void DumpVMemStats() ;
	DumpVMemStats() ;
}

DECLARE_CMD( phystats )
{
	void DumpPhysStats() ;
	DumpPhysStats() ;
}

DECLARE_CMD( setpos )
{
	if ( ev.NumArgs() != 4 )
	{
		ConPrint( "setpos {x} {y} {z}" );
		return;
	}

	extern r3dCamera gCam;

	r3dPoint3D Position;

	Position.x = ev.GetFloat( 1 );
	Position.y = ev.GetFloat( 2 );
	Position.z = ev.GetFloat( 3 );

	if( Hud )
		Hud->SetCamPos( Position );
	else
		gCam = Position;
}

//------------------------------------------------------------------------

DECLARE_CMD( cursor )
{
	if ( ev.NumArgs() != 2 )
	{
		ConPrint( "cursor {0/1}" );
		return;
	}

	if( ev.GetInteger( 1 ) )
	{
		g_cursor_mode->SetInt( 1 );
		r3dMouse::Show(true);
	}
	else
	{
		g_cursor_mode->SetInt( 0 );
		r3dMouse::Show(false);
	}
}

DECLARE_CMD( showripples )
{
	if ( ev.NumArgs() != 2 )
	{
		ConPrint( "showripples {0/1}" );
		return;
	}

	void DebugShowRipples();
	void DebugTextureOff();

	if( ev.GetInteger( 1 ) )
	{
		DebugShowRipples();
	}
	else
	{
		DebugTextureOff();
	}
}

DECLARE_CMD( aura )
{
	void DebugCyclePlayerAura();
	DebugCyclePlayerAura();
}

DECLARE_CMD( tcamo )
{
	void DebugTransparrentCamo( float alpha, int ToggleOrOn );

	if( ev.NumArgs() > 1 )
	{
		float alpha = ev.GetFloat( 1 );
		DebugTransparrentCamo( alpha, 1 );
	}
	else
	{
		DebugTransparrentCamo( 0.0f, 0 );
	}
}

DECLARE_CMD( die )
{
	void DebugPlayerDie();
	DebugPlayerDie();
}

DECLARE_CMD( ragdoll )
{
	void DebugPlayerRagdoll();
	DebugPlayerRagdoll();
}

DECLARE_CMD( export_physx_scene )
{
	void DebugExportPhysxScene();
	DebugExportPhysxScene();
}

//------------------------------------------------------------------------

void RegisterHUDCommands()
{
	REG_CCOMMAND( dumptex, 0, "Dump loaded textures" ) ;
	REG_CCOMMAND( flushmesh, 0, "Flush released meshes") ;
	REG_CCOMMAND( testfb, 0, "Test flash bang") ;
	REG_CCOMMAND( terrafetch, 0, "Switch terrain vfetch flag" ) ;
	REG_CCOMMAND( rts, 0, "Dump render target stats" ) ;
	REG_CCOMMAND( armory, 0, "Dump armory stats usage" ) ;
	REG_CCOMMAND( playertex, 0, "Dump player texture stats" ) ;
	REG_CCOMMAND( vmem, 0, "Dump video memory stats" ) ;
	REG_CCOMMAND( phystats, 0, "Dump physics stats" ) ;
	REG_CCOMMAND( setpos, 0, "Teleport observer to given coordinates" ) ;
	REG_CCOMMAND( cursor, 0, "Turn mouse cursor on/off" ) ;
	REG_CCOMMAND( showripples, 0, "Show ripples tex on/off" ) ;
	REG_CCOMMAND( aura, 0, "Cycle player aura state"  ) ;
	REG_CCOMMAND( tcamo, 0, "Turn on/off player transparent camouflage" ) ;
	REG_CCOMMAND( die, 0, "Make character die!" ) ;
	REG_CCOMMAND( ragdoll, 0, "Switch character to ragdoll" ) ;
	REG_CCOMMAND( export_physx_scene, 0, "Export whole physx scene into collection file" ) ;
}
#endif