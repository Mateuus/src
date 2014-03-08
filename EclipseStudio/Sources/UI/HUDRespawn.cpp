#include "r3dPCH.h"
#include "r3d.h"

#include "HUDRespawn.h"
#include "HUDDisplay.h"

#include "ObjectsCode/Gameplay/obj_ControlPoint.h"
#include "multiplayer/P2PMessages.h"
#include "multiplayer/ClientGameLogic.h"
#include "ObjectsCode/ai/AI_Player.H"
#include "ObjectsCode/ai/AI_PlayerAnim.H"
#include "GameLevel.h"
#include "ObjectsCode/weapons/WeaponArmory.h"
#include "ObjectsCode/weapons/Weapon.h"
#include "ObjectsCode/weapons/DroppedRespawnBeacon.h"

#include "../rendering/Deffered/CommonPostFX.h"
#include "../rendering/Deffered/PostFXChief.h"

extern HUDDisplay* hudMain;

#define RESPAWN_TIME 0.4f


void HUDRespawn::onEnterBattle(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		PKT_C2S_Bomb_PlayerReady_s n;
		n.selectedLoadoutSlot = m_slotID;
		p2pSendToHost(NULL, &n, sizeof(n), true);
	}
	else
	{
		m_respawnDelay = r3dGetTime() + RESPAWN_TIME; // X seconds before entering game
		m_updateTimer = (int)RESPAWN_TIME;
	}
}

void HUDRespawn::onSpawnPointSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);
	m_spawnID = (int)args[0].GetNumber();
}

void HUDRespawn::onTeamSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	m_teamID = (int)args[0].GetNumber();
	SetNewCPBasedOnTeamChange();
}

void HUDRespawn::onSlotSelected(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);
	int slotIndex = (int)args[0].GetNumber();

	m_slotID = slotIndex;
	wiWeaponAttachments emptyAttm;
	m_RespawnPlayer->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[m_slotID], emptyAttm);

	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // because in bomb mode server will spawn you, so send new info to server while waiting
	{
		PKT_C2S_SetRespawnData_s n;
		n.teamId  = m_teamID;
		n.spawnId = m_spawnID;
		n.slotNum = m_slotID;
		p2pSendToHost(NULL, &n, sizeof(n), true);
	}
}

void HUDRespawn::eventRespawnBombModeRequestTeamChange(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // because in bomb mode server will spawn you, so send new info to server while waiting
	{
		PKT_C2S_Bomb_RequestTeamChange_s n;
		n.requestSpectator = 0;
		p2pSendToHost(NULL, &n, sizeof(n), true);
	}
}

void HUDRespawn::eventRespawnBombModeRequestSpectator(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // because in bomb mode server will spawn you, so send new info to server while waiting
	{
		PKT_C2S_Bomb_RequestTeamChange_s n;
		n.requestSpectator = 1;
		p2pSendToHost(NULL, &n, sizeof(n), true);
	}
}

void HUDRespawn::eventRespawnSelectMode(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// TODO
	r3d_assert(argCount == 1);
	int mode = (int)args[0].GetNumber();
	// 0 - TPS, 1 - FPS
	if(mode == 0)
		g_camera_mode->SetInt(g_tps_camera_mode->GetInt());
	else
		g_camera_mode->SetInt(2);
}

void HUDRespawn::eventRespawnRequestPlayerKick(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // because in bomb mode server will spawn you, so send new info to server while waiting
	{
		r3d_assert(args);
		r3d_assert(argCount == 1);
		const char* name = args[0].GetString();

		PKT_C2S_Bomb_RequestPlayerKick_s n;
		n.peerIDtoKick = -1;
		for(int i=0; i<ClientGameLogic::MAX_NUM_LOBBY_PLAYERS; ++i)
		{
			if(strcmp(gClientLogic().lobbyPlayers[i].userName, name)==0 && gClientLogic().lobbyPlayers[i].peerID != -1)
			{
				n.peerIDtoKick = gClientLogic().lobbyPlayers[i].peerID;
				break;
			}
		}		 

		if(n.peerIDtoKick != -1)
			p2pSendToHost(NULL, &n, sizeof(n), true);
	}
}

void HUDRespawn::eventEnteredChatMessage(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) // because in bomb mode server will spawn you, so send new info to server while waiting
	{
		r3d_assert(args);
		r3d_assert(argCount == 2);

		bool isString = args[0].IsString();
		bool isStringW = args[0].IsStringW();
		const char* msg = args[0].GetString();
		bool isTeamChat = args[1].GetBool();

		PKT_C2S_Bomb_ChatMsg_s n;
		n.isTeam = isTeamChat;
		r3dscpy(n.msg, msg);
		p2pSendToHost(NULL, &n, sizeof(n), true);
	}
}

void HUDRespawn::showChatMsg(bool isTeam, const char* from, bool isAlly, const char* msg)
{
	Scaleform::GFx::Value var[5];
	var[0].SetBoolean(isTeam);
	var[1].SetString(from);
	var[2].SetString(isAlly?"blue":"red");
	var[3].SetString(msg);
	var[4].SetString("white");

	gfxMovie.Invoke("_root.api.addChatMessage", var, 5);
}

void HUDRespawn::eventRespawnReadyToDrawCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	readyToDrawCharacter = true;
}

HUDRespawn::HUDRespawn()
{
	isActive_ = false;
	m_spawnID = 0;
	m_teamID = 0;
	m_slotID = 0;
	m_CPCreated = false;
	m_respawnDelay = 0;
	m_updateTimer = 0;
	m_RespawnPlayer = 0;
	m_Initted = false;
	m_BattleTimer = 0;
	m_KeyboardCaptureStarted = false;
}

HUDRespawn::~HUDRespawn()
{
}

bool HUDRespawn::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\RespawnMenu.swf", true)) 
		return false;

	// set rank data
	{
		char tempB[64];
		Scaleform::GFx::Value var[2];
		for(int i=0; i<MAX_NUM_RANKS; ++i)
		{
			sprintf(tempB, "PlayerRank%d", i);
			var[0].SetStringW(gLangMngr.getString(tempB));
			if(i==0)
				var[1].SetNumber(0);
			else
				var[1].SetNumber(g_RanksPoints[i-1]);
			gfxMovie.Invoke("_root.api.m_RanksData.addRank", var, 2);
		}
	}


	isActive_ = false;
	m_CPCreated = false;
	m_WaitingForNextRoundBomb = false;
	m_nextCPScanTime = 0;

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	char sFullPath[512];
	char sFullPathImg[512];
	sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap.dds");
	sprintf(sFullPathImg, "$%s", sFullPath); // use '$' chas to indicate absolute path

	if(r3dFileExists(sFullPath))
	{		
		gfxMovie.Invoke("_root.api.setMinimapImage", sFullPathImg );
	}

	#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<HUDRespawn>(this, &HUDRespawn::FUNC)
	gfxMovie.RegisterEventHandler("eventRespawnOnSelectedLoadout", MAKE_CALLBACK(onSlotSelected));
	gfxMovie.RegisterEventHandler("eventRespawnOnTeamSelected", MAKE_CALLBACK(onTeamSelected));
	gfxMovie.RegisterEventHandler("eventRespawnOnSpawnPointSelected", MAKE_CALLBACK(onSpawnPointSelected));
	gfxMovie.RegisterEventHandler("eventRespawnEnterBattleClick", MAKE_CALLBACK(onEnterBattle));
	gfxMovie.RegisterEventHandler("eventRespawnReadyToDrawCharacter", MAKE_CALLBACK(eventRespawnReadyToDrawCharacter));
	gfxMovie.RegisterEventHandler("eventRespawnBombModeRequestTeamChange", MAKE_CALLBACK(eventRespawnBombModeRequestTeamChange));
	gfxMovie.RegisterEventHandler("eventEnteredChatMessage", MAKE_CALLBACK(eventEnteredChatMessage));
	gfxMovie.RegisterEventHandler("eventRespawnRequestPlayerKick", MAKE_CALLBACK(eventRespawnRequestPlayerKick));
	gfxMovie.RegisterEventHandler("eventRespawnBombModeRequestSpectator", MAKE_CALLBACK(eventRespawnBombModeRequestSpectator));
	gfxMovie.RegisterEventHandler("eventRespawnSelectMode", MAKE_CALLBACK(eventRespawnSelectMode));

	m_RespawnWorld.Init(100);
	// create player
	m_RespawnPlayer = (obj_AI_Player *)srv_CreateGameObject("obj_AI_Player", "RespawnPlayer", r3dPoint3D(0,0,0), OBJ_CREATE_LOCAL);
	m_RespawnPlayer->TeamID = TEAM_BLUE;
	m_RespawnPlayer->NetworkLocal = false;
	m_RespawnPlayer->PlayerState = PLAYER_IDLE;
	m_RespawnPlayer->bDead = 0;
	m_RespawnPlayer->LoadoutSlot = m_slotID;
	m_RespawnPlayer->CurLoadout = gUserProfile.ProfileData.ArmorySlots[m_slotID];
	m_RespawnPlayer->m_disablePhysSkeleton = true;

	m_RespawnWorld.AddObject(m_RespawnPlayer);
	m_RespawnWorld.Update(); // to make sure that OnCreate was called for player

	m_Initted = true;

	return true;
}

void HUDRespawn::ReleaseGameWorld()
{
	m_RespawnWorld.DeleteObject(m_RespawnPlayer);
	m_RespawnPlayer = 0;
	m_RespawnWorld.Destroy();
}

bool HUDRespawn::Unload()
{
	r3d_assert(m_RespawnPlayer == NULL);
	r3d_assert(m_RespawnWorld.bInited == 0);

	gfxMovie.Unload();

	isActive_ = false;
	m_Initted = false;

	return true;
}

extern int g_CCBlackWhite;
extern float g_fCCBlackWhitePwr;
void HUDRespawn::Update()
{
 	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	if(r3dGetTime() > m_nextCPScanTime)
	{
		m_nextCPScanTime = r3dGetTime() + 0.1f;
		UpdateControlPoints();
	}

	if(m_BattleTimer > 0)
	{
		int prevTimer = int(ceilf(m_BattleTimer));
		m_BattleTimer -= r3dGetFrameTime();
		if(m_BattleTimer < 0) m_BattleTimer = 0;
		int curTimer = int(ceilf(m_BattleTimer));
		if(curTimer != prevTimer)
		{
			gfxMovie.Invoke("_root.api.setBattleTimer", curTimer);
			// idiotic scaleform just doesn't send event from flash! have no idea why, especially that the same code works for conquest map. so, workaround:
			if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch && curTimer == 0)
			{
				m_respawnDelay = r3dGetTime() + RESPAWN_TIME; // X seconds before entering game
				m_updateTimer = (int)RESPAWN_TIME;
			}
		}
	}
	else // we need to bash this, because there's a glitch when the scaleform screen doesn't come up in a timely manner (manipulating the video quality level when dead causes this).
	{
		m_BattleTimer -= r3dGetFrameTime();
		if(m_BattleTimer < -1) 
		{ 
			m_BattleTimer = 0;
			gfxMovie.Invoke("_root.api.setBattleTimer", 0);
		}
	}

	if(m_respawnDelay>0)
	{
		float timeLeft = (m_respawnDelay - r3dGetTime());
		int intTimeLeft = (int)timeLeft;
		if(intTimeLeft != m_updateTimer && m_updateTimer>0)
		{
			m_updateTimer = intTimeLeft;
		}

		// restore full color before spawning
		g_fCCBlackWhitePwr = timeLeft/RESPAWN_TIME;

		if(timeLeft < 0)
		{
			PKT_C2S_SetRespawnData_s n;
			n.teamId  = m_teamID;
			n.spawnId = m_spawnID;
			n.slotNum = m_slotID;
			p2pSendToHost(NULL, &n, sizeof(n), true);

			m_respawnDelay = 999999999999999.0f; // basically make sure that we will not send another respawn packet until server will not actually respawn us

			g_fCCBlackWhitePwr = 0;
			g_CCBlackWhite = 0;
		}
	}
	else
	{
		// turn image into black and white
		g_CCBlackWhite = 1;
		g_fCCBlackWhitePwr = 1.0f;

	}
}

void GetInterfaceSize(int& width, int& height, int& y_shift, const r3dScaleformMovie &m);
float GetOptimalDist(const r3dPoint3D& boxSize, float halfFovInDegrees);

void HUDRespawn::Draw()
{
	r3dPoint3D playerPos(0, -0.05f, 0);
	static float playerRot = -30;
	m_RespawnPlayer->SetPosition(playerPos);
	m_RespawnPlayer->m_fPlayerRotationTarget = m_RespawnPlayer->m_fPlayerRotation = playerRot;
	m_RespawnPlayer->UpdateTransform();
	r3dPoint3D size = m_RespawnPlayer->GetBBoxLocal().Size;

	float distance = GetOptimalDist(size, 35.0f)*1.2f;

	r3dPoint3D camPos(0, size.y * 0.5f, distance);
	gCam = camPos;
	gCam.vPointTo = r3dPoint3D(0, 0, -1);
	gCam.FOV = 60;
	gCam.SetPlanes(1.0f, 20.0f);

	gfxMovie.UpdateAndDraw();

	if(readyToDrawCharacter && !m_WaitingForNextRoundBomb)
	{
		m_RespawnWorld.StartFrame();
		r3dRenderer->SetCamera( gCam );
		m_RespawnWorld.Update();

		extern r3dScreenBuffer*	gBuffer_Color;	// rgb - albedo, a - gloss
		extern r3dScreenBuffer*	gBuffer_Normal;	// rgb - normal, a - reflectivity
		extern r3dScreenBuffer*	gBuffer_Depth;  // R32 - depth
		extern r3dScreenBuffer*	gBuffer_Aux;  // r - ssao channel, gb - motion blur velocities, a - self illum
		{
			gBuffer_Color->Activate();
			gBuffer_Normal->Activate(1);
			gBuffer_Depth->Activate(2);
			gBuffer_Aux->Activate(3);
			
			r3dRenderer->StartRender(1);


			{
				r3dRenderer->SetMaterial(NULL);
				r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

				void SetMRTClearShaders( bool depth_only );
				SetMRTClearShaders( false );

				D3DXVECTOR4 pconst0 = D3DXVECTOR4 ( gCam.NearClip, gCam.FarClip, 0.0f, 0.0f );
				r3dRenderer->pd3ddev->SetPixelShaderConstantF ( 0, (float*) pconst0, 1 );

				r3dColor Cl = r3dGameLevel::Environment.Fog_Color.GetColorValue(r3dGameLevel::Environment.__CurTime/24.0f);
				r3dDrawBoxFS( r3dRenderer->ScreenW, r3dRenderer->ScreenH, Cl);

				r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_ZC | R3D_BLEND_ZW );
			}

			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILENABLE, true );
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE );
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILREF, 1 );
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILMASK, 0xFFFFFFFF );
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xFFFFFFFF );

			r3dRenderer->SetVertexShader(VS_FILLGBUFFER_ID);
			FillbufferShaderKey key;

			SetLQ( &key ) ;

			SetFillGBufferPixelShader( key ) ;

			D3DXVECTOR4 CamVec = D3DXVECTOR4(gCam.x, gCam.y, gCam.z, 1);
			r3dRenderer->pd3ddev->SetPixelShaderConstantF(MC_CAMVEC, (float*)&CamVec, 1);
			for( int i = 0; i < 6; i ++ )
			{
				r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
				r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );

				r3dSetFiltering( R3D_ANISOTROPIC, i );
			}
			r_use_oq->SetInt( 0 );

			m_RespawnWorld.Prepare(gCam);
			m_RespawnWorld.Draw(rsFillGBuffer);

			r_use_oq->SetInt( 1 );

			r3dRenderer->SetVertexShader();
			r3dRenderer->SetPixelShader();
			r3dRenderer->SetFog(0);
			r3dRenderer->EndRender();

			gBuffer_Aux->Deactivate();
			gBuffer_Depth->Deactivate();
			gBuffer_Normal->Deactivate();
			gBuffer_Color->Deactivate();
			r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILENABLE, FALSE );

		}
		{
			g_pPostFXChief->ResetBuffers() ;

			g_pPostFXChief->GetBuffer(PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP)->Activate();
			r3dRenderer->StartRender(0);
			r3dColor Cl = r3dGameLevel::Environment.Fog_Color.GetColorValue(r3dGameLevel::Environment.__CurTime/24.0f);
			r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, Cl.GetPacked(), 1.0f, 0 );

			r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA);
			for (int i=0;i<8;i++)
			{
				r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSU,   D3DTADDRESS_CLAMP );
				r3dRenderer->pd3ddev->SetSamplerState( i, D3DSAMP_ADDRESSV,   D3DTADDRESS_CLAMP );
			}
			r3dRenderer->SetTex(gBuffer_Color->Tex);
			r3dRenderer->SetTex(gBuffer_Normal->Tex,1);
			r3dRenderer->SetTex(gBuffer_Depth->Tex,2);
			r3dRenderer->SetTex(gBuffer_Aux->Tex,3);
			r3dRenderer->SetTex(gBuffer_Aux->Tex,4);
			r3dRenderer->SetTex(__r3dShadeTexture[2],5); // white texture for shadows. no shadows needed here

			r3dSetFiltering( R3D_POINT );
			r3dSetFiltering( R3D_BILINEAR, 4 );
			r3dSetFiltering( R3D_BILINEAR, 8 );
			// sunlight must process some sky attributes including sun glow
			extern void Render_Deffered_Sunlight( bool ambient_only );
			int prevVal = r_half_scale_ssao->GetInt();
			r_half_scale_ssao->SetInt(0);
			Render_Deffered_Sunlight( false );
			r_half_scale_ssao->SetInt(prevVal);

			g_pPostFXChief->GetBuffer(PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP)->Deactivate();
			r3dRenderer->EndRender();

			PostFXChief::RTType stereoRTs[] = { PostFXChief::RTT_PINGPONG_LAST_AS_TEMP, PostFXChief::RTT_TEMP0_64BIT };
			{
				g_pPostFXChief->AddFX( gPFX_ConvertToLDR, PostFXChief::RTT_PINGPONG_LAST_AS_TEMP, PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP ) ;
				g_pPostFXChief->AddSwapBuffers() ;

				PFX_Fill::Settings fsts;

				fsts.ColorWriteMask = D3DCOLORWRITEENABLE_ALPHA;			

				gPFX_Fill.PushSettings( fsts );
				g_pPostFXChief->AddFX( gPFX_Fill, PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP, PostFXChief::RTT_DIFFUSE_32BIT );

				PFX_StencilToMask::Settings ssts;

				ssts.Value = float4( 0, 0, 0, 1 );

				gPFX_StencilToMask.PushSettings( ssts );
				g_pPostFXChief->AddFX( gPFX_StencilToMask, PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP, PostFXChief::RTT_DIFFUSE_32BIT );

				if (r3dRenderer->IsStereoActive())
				{
					AddSeparateEyesStereoReprojectionStack(PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP, stereoRTs[0], stereoRTs[1]);
				}
				else
				{
					stereoRTs[0] = PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP;
				}
				g_pPostFXChief->Execute( false, false );
			}

			r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_NZ | R3D_BLEND_PUSH );

			r3dRenderer->SetVertexShader();
			r3dRenderer->SetPixelShader();

			r3dTexture* Tex = g_pPostFXChief->GetBuffer( PostFXChief::RTT_PINGPONG_NEXT_AS_TEMP )->Tex;

			float x_start=0;
			float y_start=0;
			float x_width=0;
			float y_width=0;

			int width, height,y_shift;
			GetInterfaceSize(width, height, y_shift, gfxMovie);

			// get multipliers based on current resolution and default SWF size
			float xMultiplier = (width / 1920.0f);
			float yMultiplier = (height / 1080.0f);

			// get the smallest multiplier
//			float multiplier = R3D_MIN(xMultiplier, yMultiplier);

			// get real values based on multiplier
			x_start = xMultiplier * 771;//401;
			y_start = yMultiplier * 130;//208;//110;
			x_width = xMultiplier * 360;//380;
			y_width = yMultiplier * 720;//650;//432;

			float minX, minY, maxX, maxY;

			float texelX = 1.0f / (float)width; 
			float texelY = 1.0f / (float)height;
			
			minX = 0.5f - texelX * (x_width) / 1.7f;
			minY = 0.5f - texelY * (y_width) / 1.7f;
			maxX = 0.5f + texelX * (x_width) / 1.7f;
			maxY = 0.5f + texelY * (y_width) / 1.7f;

			float TC[16] = {minX, minY,
				maxX, minY,
				maxX, maxY,
				minX, maxY,
				minX, minY,
				maxX, minY,
				maxX, maxY,
				minX, maxY};

			// 		r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
			// 		r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );
			r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSU,   D3DTADDRESS_BORDER );
			r3dRenderer->pd3ddev->SetSamplerState( 0, D3DSAMP_ADDRESSV,   D3DTADDRESS_BORDER );
			uint32_t numPasses = (r3dRenderer->GetPresentEye() == R3D_STEREO_EYE_MONO ? 1 : 2);
			for (uint32_t i = 0; i < numPasses; ++i)
			{
				r3dTexture* Tex = g_pPostFXChief->GetBuffer( stereoRTs[i] )->Tex;;
				if (numPasses > 1)
					r3dRenderer->SetEye(i == 0 ? R3D_STEREO_EYE_LEFT : R3D_STEREO_EYE_RIGHT);
				r3dDrawBox2D( x_start, y_start, x_width, y_width, r3dColor(255,255,255), Tex, TC);
			}
			r3dRenderer->SetEye(R3D_STEREO_EYE_MONO);
			r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
		}

		D3D_V( r3dRenderer->pd3ddev->SetRenderState( D3DRS_STENCILENABLE, false )			);

		m_RespawnWorld.EndFrame();		
	}
}

void HUDRespawn::Deactivate()
{
	isActive_ = false;
	g_CCBlackWhite = 0;
	g_fCCBlackWhitePwr = 0;

	if( !g_cursor_mode->GetInt() )
	{
		r3dMouse::Hide();
	}
	
	if ( m_KeyboardCaptureStarted )
	{
		hudMain->gfxHUD.SetKeyboardCapture();	
		m_KeyboardCaptureStarted = false;
	}

}

void HUDRespawn::SetNewCPBasedOnTeamChange()
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
		return;

    int newSpawnID = 0;
    if(gClientLogic().localPlayer_ == NULL) // choose a point that is furthest away from the base, trying to estimate where the action might be going on
    {
        r3dPoint3D basePos = gCPMgr.GetBase(m_teamID)->GetPosition();
        float distance = -1;
        for(int i=0; i<gCPMgr.NumCP(); ++i)
        {
            if(gCPMgr.GetCP(i)->GetSpawnTeamId() == m_teamID)
            {
                float dst = (basePos - gCPMgr.GetCP(i)->GetPosition()).Length();
                if(dst > distance) 
                {
                    newSpawnID = i;
                    distance = dst;
                }
            }
        }
    }
    else // choose the closest spawn point to where player died
    {
        r3dPoint3D basePos = gClientLogic().localPlayer_->PosOfDeath;
        float distance = 9999999999.f;
        for(int i=0; i<gCPMgr.NumCP(); ++i)
        {
            if(gCPMgr.GetCP(i)->GetSpawnTeamId() == m_teamID)
            {
                float dst = (basePos - gCPMgr.GetCP(i)->GetPosition()).Length();
                if(dst < distance) 
                {
                    newSpawnID = i;
                    distance = dst;
                }
            }
        }
    }
	m_spawnID = newSpawnID;
	gfxMovie.Invoke("_root.api.setSpawnPointActive", m_spawnID);
}

void HUDRespawn::UpdateLobbyPlayerList()
{
	if(!isActive_)
		return;

	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.clearPlayerList"), "");
	for(int i=0; i<ClientGameLogic::MAX_NUM_LOBBY_PLAYERS; ++i)
	{
		if(gClientLogic().lobbyPlayers[i].peerID != -1 && !gClientLogic().lobbyPlayers[i].isSpectator)
		{
			Scaleform::GFx::Value var[5];
			var[0].SetNumber(gClientLogic().lobbyPlayers[i].desiredTeam);
			var[1].SetNumber(gClientLogic().lobbyPlayers[i].level);
			var[2].SetString(gClientLogic().lobbyPlayers[i].userName);
			var[3].SetBoolean(gClientLogic().lobbyPlayers[i].isReady==1);
			var[4].SetBoolean(false); // todo: add bool to check if player is local
			gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.addPlayer"), var, 5);
		}
	}
	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.updatePlayerList"), "");
}

void HUDRespawn::Lobby_SetPlayerReady(const char* name, bool isReady)
{
	if(!isActive_)
		return;

	Scaleform::GFx::Value var[2];
	var[0].SetString(name);
	var[1].SetBoolean(isReady);
	gfxMovie.Invoke(VMPROTECT_DecryptStringA("_root.api.setPlayerReady"), var, 2);
}

void HUDRespawn::Lobby_UnlockCreatorOptions()
{
	gfxMovie.Invoke("_root.api.showKickOption", 1);
}

void HUDRespawn::StartLobbyCountdown()
{
	m_BattleTimer = 8.0f;
	gfxMovie.Invoke("_root.api.setBattleTimer", m_BattleTimer);
	gfxMovie.Invoke("_root.api.startCountdown", "");
}

void HUDRespawn::Activate(int teamId, int slotNum, bool gameStartedEvent)
{
	r3d_assert(!isActive_);
	isActive_ = true;

	const ClientGameLogic& CGL = gClientLogic();
	
	if( CGL.m_gameInfo.mapType != GBGameInfo::MAPT_Bomb || CGL.m_gameHasStarted == false )
	{
		r3dMouse::Show();
	}
	
	// if we're on the sabotage starting screen. 
	if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb && CGL.m_gameHasStarted == false )
	{
		gfxMovie.SetKeyboardCapture();
		m_KeyboardCaptureStarted = true;
	}

	m_respawnDelay = 0.0f;
	m_updateTimer = 0;
	m_BattleTimer = 0;
	readyToDrawCharacter = false;
	m_nextCPScanTime = 0;

	if(g_num_matches_played->GetInt()==1 && g_num_game_executed2->GetInt()==1 && CGL.localPlayer_ == NULL) // auto jump into match
	{
		m_respawnDelay = r3dGetTime();
	}

	m_CPCreated = false;
	{
		gfxMovie.Invoke("_root.api.clearSpawnPoints", NULL, 0);
	}

	{
		Scaleform::GFx::Value var[4];
		var[0].SetString(gUserProfile.ScreenName);
		var[1].SetNumber(gUserProfile.ProfileData.Stats.HonorPoints);
		var[2].SetNumber(gUserProfile.ProfileData.Stats.GamePoints);
		var[3].SetNumber(gUserProfile.ProfileData.Stats.GameDollars);
		gfxMovie.Invoke("_root.api.setUserStats", var, 4);
	}

	{
		Scaleform::GFx::Value var[2];
		if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
		{
			var[0].SetString("ATTACK");
			var[1].SetString("DEFENSE");
		}
		else if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
		{
			var[0].SetStringW(gLangMngr.getString("HUD_Respawn_Terrorist"));
			var[1].SetStringW(gLangMngr.getString("HUD_Respawn_CounterTerrorist"));
		}
		else
		{
			var[0].SetStringW(gLangMngr.getString("HUD_Respawn_Team1"));
			var[1].SetStringW(gLangMngr.getString("HUD_Respawn_Team2"));
		}
		gfxMovie.Invoke("_root.api.setTeamNames", var, 2);
	}

	{
		gfxMovie.Invoke("_root.api.setActiveMode", g_camera_mode->GetInt()==2?1:0);
	}

	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
	{
		Scaleform::GFx::Value values[3];
		values[0].SetNumber(i);
		values[1].SetBoolean(gUserProfile.ProfileData.ArmorySlots[i].LoadoutID?true:false);
		gfxMovie.Invoke("_root.api.setLoadoutSlotAvailable", values, 2);
		if(gUserProfile.ProfileData.ArmorySlots[i].LoadoutID)
		{
			Scaleform::GFx::Value vars[18];

			vars[0].SetNumber(i);

			const static char* classes[] = {
				"$FR_AssaultSpec",
				"$FR_SpecialistSpec",
				"$FR_ReconSpec",
				"$FR_MedicSpec",
			};
			int loadoutLevel = (gUserProfile.ProfileData.ArmorySlots[i].HonorPoints / 20000)+1;
			int loadoutClass = gUserProfile.ProfileData.ArmorySlots[i].Class;
			char timePlayed[32] = {0};
			int tPlayed = gUserProfile.ProfileData.ArmorySlots[i].TimePlayed;
			int hours = tPlayed/3600;
			int minutes = (tPlayed-hours*3600)/60;
			int sec = tPlayed%60;
			sprintf(timePlayed, "%02d:%02d:%02d", hours, minutes, sec);
			vars[1].SetString(classes[loadoutClass]);
			vars[2].SetNumber(loadoutLevel);
			vars[3].SetString(timePlayed);

			{
				const WeaponConfig* wpnConfig = 0;
				wpnConfig = gWeaponArmory.getWeaponConfig(gUserProfile.ProfileData.ArmorySlots[i].PrimaryWeaponID);
				//SABOTAGE_NoRpg rule: replace RPG with ak74
				if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb) {
					if(wpnConfig && wpnConfig->category == storecat_SUPPORT)
						wpnConfig = gWeaponArmory.getWeaponConfig(101001);
				}
				vars[4].SetString(wpnConfig?wpnConfig->m_StoreName:"");
				vars[5].SetString(wpnConfig?wpnConfig->m_StoreIcon:"");
			}
			vars[6].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].SecondaryWeaponID, true));
			vars[7].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].SecondaryWeaponID, true));
			
			vars[8].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].SidearmWeaponID, true));
			vars[9].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].SidearmWeaponID, true));
			
			vars[10].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item1, true));
			vars[11].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item1, true));

			vars[12].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item2, true));
			vars[13].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item2, true));

			vars[14].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item3, true));
			vars[15].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item3, true));

			vars[16].SetString(gWeaponArmory.getNameByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item4, true));
			vars[17].SetString(gWeaponArmory.getIconByItemId(gUserProfile.ProfileData.ArmorySlots[i].Item4, true));

			gfxMovie.Invoke("_root.api.setLoadoutSlotParams", vars, 18);
		}	
	}

	m_teamID = teamId;
	m_slotID = slotNum;
	wiWeaponAttachments emptyAttm;
	m_RespawnPlayer->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[m_slotID], emptyAttm);
	gfxMovie.Invoke("_root.api.setLoadoutSlotActive", m_slotID);
	gfxMovie.Invoke("_root.api.setActiveTeam", m_teamID);

	UpdateControlPoints();
	SetNewCPBasedOnTeamChange();

	if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Bomb)
	{
		if( CGL.localPlayer_ == NULL && CGL.m_gameHasStarted == false)
		{
			UpdateLobbyPlayerList();
			gfxMovie.Invoke("_root.api.setRespawnView", "bomb");
		}
		else
		{
			if( CGL.localPlayer_ == NULL && !gameStartedEvent)
			{
				m_WaitingForNextRoundBomb = true;
				gfxMovie.Invoke("_root.api.setBombWaitingForNextRound", 1);
			}
			else
			{
				m_WaitingForNextRoundBomb = false;
				m_BattleTimer = 8.0f;
				gfxMovie.Invoke("_root.api.setBombWaitingForNextRound", 0);
				gfxMovie.Invoke("_root.api.setBattleTimer", m_BattleTimer);

				r3dMouse::Show(); // confirm that the mouse is showing.

			}
			gfxMovie.Invoke("_root.api.setRespawnView", "conquest");
			// disable map
			gfxMovie.SetVariable("_root.api.view.Main.Map._visible", false);
			gfxMovie.SetVariable("_root.api.view.Main.MapBack._visible", false);
			gfxMovie.SetVariable("_root.api.view.Main.AddMenu._visible", false);
		}
	}
	else
	{
		if( CGL.localPlayer_ == NULL) // if just joined, always show enter battle button
			gfxMovie.Invoke("_root.api.setBattleTimer", -1); // X seconds for auto entering battle
		else
		{
			m_BattleTimer = CGL.m_gameInfo.respawnDelay;
			gfxMovie.Invoke("_root.api.setBattleTimer", m_BattleTimer); // X seconds for auto entering battle
		}

		if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Conquest)
			gfxMovie.Invoke("_root.api.setRespawnView", "conquest");
		else if( CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch || CGL.m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
		{
			gfxMovie.Invoke("_root.api.setRespawnView", "conquest");
		}
		else
			r3dError("Unknown type of map in respawn screen, type=%d\n", CGL.m_gameInfo.mapType);
	}
}

r3dPoint2D getMinimapPos(const r3dPoint3D& pos);
void HUDRespawn::UpdateControlPoints()
{
	if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Siege)
		return;

	if(!m_CPCreated)
		SetControlPoints();

	for(int i=0; i<gCPMgr.NumCP(); i++) 
	{
		obj_ControlPoint* cp = (obj_ControlPoint*)gCPMgr.GetCP(i);
		int teamId = cp->GetSpawnTeamId();

		const char* spawnColor = "grey";
		if(teamId == 0)
			spawnColor = "blue";
		if(teamId == 1)
			spawnColor = "red";

		const char* tagName = cp->getUITagName();
		if(tagName == NULL) tagName = "";
		Scaleform::GFx::Value args[3];
		args[0].SetNumber(i);
		args[1].SetString(spawnColor);
		args[2].SetString(tagName);
		gfxMovie.Invoke("_root.api.setSpawnPointStatus", args, 3);
	}

	// set beacons
	obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(localPlayer) // respawn beacons are active ONLY if you already spawned on the map
	{
		for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
		{
			if(obj && obj->isObjType(OBJTYPE_GameplayItem) && obj->Class->Name == "DroppedRespawnBeacon")
			{
				DroppedRespawnBeacon* beacon = (DroppedRespawnBeacon*)obj;
				obj_AI_Player* beaconOwner = (obj_AI_Player*)GameWorld().GetObject(beacon->ownerID);
				if(beaconOwner->TeamID == localPlayer->TeamID)
				{
					Scaleform::GFx::Value args[3];
					const char* tagName;
					if(beacon->m_ItemID == WeaponConfig::ITEMID_RespawnBeacon || beaconOwner == localPlayer) 
					{
						if(beacon->m_ItemID == WeaponConfig::ITEMID_RespawnBeacon)// whole team sees
							tagName = "beacon2";
						else
							tagName = "beacon1"; // your personal beacon
						Scaleform::GFx::Value args[3];
						args[0].SetNumber(beacon->NetworkID);
						args[1].SetString(localPlayer->TeamID==0?"blue":"red");
						args[2].SetString(tagName);
						gfxMovie.Invoke("_root.api.setSpawnPointStatus", args, 3);
					}
				}
			}
		}
	}

	gfxMovie.Invoke("_root.api.updateSpawnPointsOnRequest", NULL, 0);
}

void HUDRespawn::onRespawnBeaconDestroy(int spawnBeaconNetID)
{
	m_CPCreated = false;
	gfxMovie.Invoke("_root.api.clearSpawnPoints", NULL, 0);
	UpdateControlPoints();
	if(m_spawnID == spawnBeaconNetID)
		SetNewCPBasedOnTeamChange();
}

void HUDRespawn::onRespawnBeaconCreate(int spawnBeaconNetID)
{
	DroppedRespawnBeacon* beacon = (DroppedRespawnBeacon*)GameWorld().GetNetworkObject(spawnBeaconNetID);
	obj_AI_Player* beaconOwner = (obj_AI_Player*)GameWorld().GetObject(beacon->ownerID);
	obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(localPlayer && beaconOwner->TeamID == localPlayer->TeamID)
	{
		Scaleform::GFx::Value args[3];
		r3dPoint3D pos = beacon->GetPosition();
		r3dPoint2D mappos = getMinimapPos(pos);

		if(beacon->m_ItemID == WeaponConfig::ITEMID_RespawnBeacon || beaconOwner == localPlayer)
		{
			args[0].SetNumber(beacon->NetworkID);
			args[1].SetNumber(mappos.x);
			args[2].SetNumber(mappos.y);
			gfxMovie.Invoke("_root.api.addSpawnPoint", args, 3);

			UpdateControlPoints();
		}
	}
}

void HUDRespawn::SetControlPoints()
{
	for(int i=0; i<gCPMgr.NumCP(); i++) 
	{
		if(gClientLogic().m_gameInfo.mapType == GBGameInfo::MAPT_Deathmatch)
			if(gCPMgr.GetCP(i)->GetSpawnTeamId() == 2) // do not add grey CP in deathmatch mode
				continue;

		r3dPoint3D pos = gCPMgr.GetCP(i)->GetPosition();
		r3dPoint2D mappos = getMinimapPos(pos);

		Scaleform::GFx::Value args[3];
		args[0].SetNumber(i);
		args[1].SetNumber(mappos.x);
		args[2].SetNumber(mappos.y);
		gfxMovie.Invoke("_root.api.addSpawnPoint", args, 3);
	}
	// add respawn beacons
	obj_AI_Player* localPlayer = gClientLogic().localPlayer_;
	if(localPlayer) // respawn beacons are active ONLY if you already spawned on the map
	{
		for( GameObject* obj = GameWorld().GetFirstObject(); obj; obj = GameWorld().GetNextObject(obj) )
		{
			if(obj && obj->isObjType(OBJTYPE_GameplayItem) && obj->Class->Name == "DroppedRespawnBeacon")
			{
				DroppedRespawnBeacon* beacon = (DroppedRespawnBeacon*)obj;
				obj_AI_Player* beaconOwner = (obj_AI_Player*)GameWorld().GetObject(beacon->ownerID);
				if(beaconOwner->TeamID == localPlayer->TeamID)
				{
					Scaleform::GFx::Value args[3];
					r3dPoint3D pos = beacon->GetPosition();
					r3dPoint2D mappos = getMinimapPos(pos);

					if(beacon->m_ItemID == WeaponConfig::ITEMID_RespawnBeacon || beaconOwner == localPlayer)
					{
						args[0].SetNumber(beacon->NetworkID);
						args[1].SetNumber(mappos.x);
						args[2].SetNumber(mappos.y);
						gfxMovie.Invoke("_root.api.addSpawnPoint", args, 3);
					}
				}
			}
		}
	}
	m_CPCreated = true;
}
