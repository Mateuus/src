#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerControlPoint.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerPlayer.h"
#include "../EclipseStudio/Sources/ObjectsCode/weapons/WeaponConfig.h"

#include "ServerGameLogic.h"

//
//
// 	class for ControlPoint Object, as you may guess..
//
//

	CVAR_FLOAT(	_cp_ScanInterval,	0.5f,	"");
	CVAR_FLOAT(	_cp_InfluenceInc,	0.025f,	"per player"); // 0.025f - should take 20 seconds to capture from neutral to yours. 

IMPLEMENT_CLASS(obj_ServerControlPoint, "obj_ControlPoint", "Object");
AUTOREGISTER_CLASS(obj_ServerControlPoint);

obj_ServerControlPoint::obj_ServerControlPoint()
{
  nextScan_   = 0;
  last_send_status = 0;
}

obj_ServerControlPoint::~obj_ServerControlPoint()
{
}

BOOL obj_ServerControlPoint::Load(const char *fname)
{
  if(!parent::Load(fname)) 
    return FALSE;

  return TRUE;
}

BOOL obj_ServerControlPoint::OnCreate()
{
  parent::OnCreate();
  ObjFlags |= OBJFLAG_SkipCastRay;

  NetworkLocal = true;
  NetworkID    = gCPMgr.RegisterControlPoint(this) + NETID_CONTROLS_START;
  r3dOutToLog("CP %d at %f %f %f\n", NetworkID, GetPosition().x, GetPosition().y, GetPosition().z);

  return 1;
}


BOOL obj_ServerControlPoint::OnDestroy()
{
  return parent::OnDestroy();
}

void obj_ServerControlPoint::RewardCapturingPlayers(int teamId)
{
	r3dOutToLog("CP%d, rewarding for team:%d\n", NetworkID, teamId); CLOG_INDENT;

	// check for flag trading
	struct PlayerRecord
	{
		gobjid_t plrID;
		float prevCapture;
		int numCaptures;
		PlayerRecord():plrID(invalidGameObjectID),prevCapture(0), numCaptures(0) {}
	};

	// reward players used to capture
	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if(plr == NULL || plr->isDead)
			continue;

		if(plr->capturingPointId_ != NetworkID)
			continue;
		if(plr->TeamID != teamId)
			continue;

		// skip rewarding if we started to capture when it was already ours
		if(teamId == TEAM_BLUE && plr->capturingPointStart_ < -OWN_THRESHOLD) 
			continue;
		if(teamId == TEAM_RED && plr->capturingPointStart_ > OWN_THRESHOLD) 
			continue;

		// reward based on when capture was started, neutral or enemy
		bool wasEnemy = false;
		if(teamId == TEAM_BLUE) 
		{
			wasEnemy = (plr->capturingPointStart_ > OWN_THRESHOLD);
		}
		else
		{
			wasEnemy = (plr->capturingPointStart_ < -OWN_THRESHOLD);
		}

		r3dOutToLog("rewarding %s, from %f, wasEnemy:%d\n", plr->userName, plr->capturingPointStart_, (int)wasEnemy);

		// reset cooldown every 5 min
		float captureCD = r3dGetTime() - plr->capturingPointCooldown_;
		if(captureCD > 300) 
		{
			plr->capturingPointCooldown_ = r3dGetTime();
			plr->capturingPointStreak_ = 0;
		}
		plr->capturingPointStreak_++;
                        		
		wiStatsTracking rwd;
		if(wasEnemy)
		{
			plr->RoundStats_.CaptureEnemyPoints++;
			rwd = gServerLogic.GetRewardData(plr, RWD_CaptureEnemy);
		}
		else
		{
			plr->RoundStats_.CaptureNeutralPoints++;
			rwd = gServerLogic.GetRewardData(plr, RWD_CaptureNeutral);
			plr->incrementAchievement( ACHIEVEMENT_CAPTURE_5000_FLAGS, 1 );
		}

		// reset
		plr->capturingPointId_    = 0;
		plr->capturingPointStart_ = 0;

		
		// disabled for now as sergey is confused when he doesn't get reward for capturing flag
		/*switch(plr->capturingPointStreak_)
		{
			case 1:
				break;
			case 2:
				// 50% XP, 0 GD
				rwd.HP /= 2;
				rwd.GD = 0;
				break;
			case 3:
				// 25% XP, 0 GD
				rwd.HP /= 4;
				rwd.GD = 0;
				break;
			default:
				// 0
				rwd.HP = 0;
				rwd.GD = 0;
				break;
		}*/
			
		gServerLogic.AddDirectPlayerReward(plr, rwd, wasEnemy ? "RWD_CaptureEnemy" : "RWD_CaptureNeutral");
	}
}

static bool firstPlayerToStartCaptureNeutral[2] = {false, false};
static bool firstPlayerToStartCaptureEnemy[2] = {false, false};

void obj_ServerControlPoint::ScanForPlayers()
{
  //r3dOutToLog("scanning for players\n");
  
  if(!gServerLogic.IsFairGame()) 
    return;

  if(gServerLogic.ginfo_.mapType != GBGameInfo::MAPT_Conquest)
	  return;
  
  float redMod = 1.0f;
  float blueMod = 1.0f;
  int numRed = 0;
  int numBlue = 0;

  float team0Status = 0.0f;
  float team1Status = 0.0f;
  
  for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
  {
    obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
    if(plr == NULL || plr->isDead)
      continue;

	float FlagHogAbility = 1.0f;
	float FastCapSkill = 1.0f;
	{
		if(plr->GetLoadoutData().hasItem(AbilityConfig::AB_FlagHog))
			FlagHogAbility = 1.1f; // +10% to the speed of flag capturing

		int FastCap = R3D_MAX(plr->GetLoadoutData().getSkillLevel(CUserSkills::ASSAULT_FastCap), plr->GetLoadoutData().getSkillLevel(CUserSkills::RECON_FastCap));
		switch(FastCap)
		{
		case 1: FastCapSkill = 1.03f; break;
		case 2:	FastCapSkill = 1.06f; break;
		case 3:	FastCapSkill = 1.09f; break;
		case 4:	FastCapSkill = 1.12f; break;
		case 5:	FastCapSkill = 1.15f; break;
		default:break;
		}
	}
      
    r3dPoint3D p = plr->GetPosition() - GetPosition();
    float len2d = sqrtf(p.x*p.x + p.z*p.z);
    //r3dOutToLog(" %s: %f\n", obj->Name.c_str(), len2d);
    if(len2d > capture_radius) 
    {
      // if player left capturing point, reset his attempt
      if(plr->capturingPointId_ == NetworkID) {
        plr->capturingPointId_ = 0;
      }
      continue;
    }
      
    if(plr->capturingPointId_ != NetworkID) {
      // set that we started to capture this point from not yet updated status
      plr->capturingPointId_    = NetworkID;
      plr->capturingPointStart_ = status_;

	  if(status_ == 0.0f) // first to start capturing neutral
	  {
		  if(firstPlayerToStartCaptureNeutral[plr->TeamID] == false)
		  {
			  firstPlayerToStartCaptureNeutral[plr->TeamID] = true;
			  gServerLogic.AddPlayerReward(plr, RWD_FirstCaptureNeutral);  
			  plr->markAchievementComplete( ACHIEVEMENT_FIRST_PLAYER_TO_START_CAPTURING );
		  }
	  }
	  else if(R3D_ABS(status_) > 0.8f) // first to capture enemy
	  {
		  if(plr->TeamID && status_ < -0.8f && firstPlayerToStartCaptureEnemy[plr->TeamID]==false)
		  {
			  firstPlayerToStartCaptureEnemy[plr->TeamID] = true;
			  gServerLogic.AddPlayerReward(plr, RWD_FirstCaptureEnemy);    
		  } 
		  else if(plr->TeamID==0 && status_ > 0.8f && firstPlayerToStartCaptureEnemy[plr->TeamID]==false)
		  {
			  firstPlayerToStartCaptureEnemy[plr->TeamID] = true;
			  gServerLogic.AddPlayerReward(plr, RWD_FirstCaptureEnemy);    
		  }
	  }
	}
      
    if(plr->TeamID)
    {
      if(numRed<4) // if more than 4 players - it will not affect time of capture
      {
        team1Status += _cp_InfluenceInc*redMod*FlagHogAbility*FastCapSkill;
        redMod = 0.25f; // each new team player adds only a quarter
        ++numRed;
      }
    }
    else 
    {
      if(numBlue<4)
      {
        team0Status -= _cp_InfluenceInc*blueMod*FlagHogAbility*FastCapSkill;
        blueMod = 0.25f;
        ++numBlue;
      }
    }
  }
  
  if(fabs(team0Status) < 0.001f && fabs(team1Status) < 0.001f )
    return;

  //r3dOutToLog("cp%d capture: %f, %f, %d, %d\n", NetworkID, team0Status, team1Status, numRed, numBlue);
  status_ = status_ + team0Status + team1Status;
  status_ = R3D_CLAMP(status_, -1.0f, 1.0f);
  
  //r3dOutToLog("cp%d status: %f\n", NetworkID, status_);
  
  HostSendUpdate();

  // make sure that we reward at 100 & -100
  if((int)(status_ * 100.0f) == 100)
    RewardCapturingPlayers(TEAM_RED);
  if((int)(status_ * 100.0f) == -100)
    RewardCapturingPlayers(TEAM_BLUE);

  return;
}

BOOL obj_ServerControlPoint::Update()
{
  float TimePassed = r3dGetFrameTime();

  // only free control points can be captured
  if(spawnType_ == SPAWN_FREE && r3dGetTime() > nextScan_) {
    nextScan_ = r3dGetTime() + _cp_ScanInterval;
    ScanForPlayers();
  }

  return TRUE;
}

void obj_ServerControlPoint::HostSendUpdate()
{
	// send update only when it is greater than precision packing or when status is 0, 1 or -1
	if(R3D_ABS(last_send_status - status_) > (2.0f/255.0f) || (status_ == 0.0f || status_ == 1.0f || status_ == -1.0f))
	{
		last_send_status = status_;

		PKT_S2C_ControlPointUpdate_s n;
		n.pack(status_);

		gServerLogic.p2pBroadcastToActive(this, &n, sizeof(n));

	}
}

BOOL obj_ServerControlPoint::OnNetReceive(DWORD EventID, const void* packetData, int packetSize)
{
  /*
  switch(EventID)
  {
    default: return FALSE;
  }
  */

  return TRUE;
}
