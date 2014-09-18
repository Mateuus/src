#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_ServerSupplyCrate.h"

#include "multiplayer/P2PMessages.h"

#include "ObjectsCode/obj_ServerSupplyCrate.h"
#include "ObjectsCode/obj_ServerPlayer.h"

#include "ServerGameLogic.h"

//
//
// 	class for Control Crates Objects
//
//

CVAR_FLOAT(	_rc_ResupplyRadius,	3.0f,	"resupply crate radius");
CVAR_FLOAT(	_rc_ResupplyInterval,	60.0f,	"resupply crate interval");

IMPLEMENT_CLASS(obj_ServerSupplyCrate, "obj_SupplyCrate", "Object");
AUTOREGISTER_CLASS(obj_ServerSupplyCrate);

IMPLEMENT_CLASS(obj_ServerBlackopsCrate, "obj_BlackopsCrate", "Object");
AUTOREGISTER_CLASS(obj_ServerBlackopsCrate);

obj_ServerSupplyCrate::obj_ServerSupplyCrate()
{
	nextScan_   = 0;
	isBlackops_ = false;
}

obj_ServerSupplyCrate::~obj_ServerSupplyCrate()
{
}

BOOL obj_ServerSupplyCrate::Load(const char *fname)
{
	return parent::Load(fname);
}

BOOL obj_ServerSupplyCrate::OnCreate()
{
	parent::OnCreate();
	ObjFlags |= OBJFLAG_SkipCastRay;

	return 1;
}

BOOL obj_ServerSupplyCrate::OnDestroy()
{
	return parent::OnDestroy();
}

bool obj_ServerBlackopsCrate::CanResupply(obj_ServerPlayer* plr)
{
	return plr->m_CanUseBlackopsCrate;
}

void obj_ServerSupplyCrate::ScanForPlayers()
{
	const float curTime = r3dGetTime();

	//r3dOutToLog("resupply crate: scanning for players\n");
	for(int i = 0; i < ServerGameLogic::MAX_NUM_PLAYERS; i++) 
	{
		obj_ServerPlayer* plr = gServerLogic.GetPlayer(i);
		if(plr == NULL)
			continue;
		if(!plr->m_RequestedResupply)
			continue;

		// check timer
		if((curTime - plr->m_LastTimeResupplied) < _rc_ResupplyInterval)
		{
			//r3dOutToLog("RC: %s too early\n", plr->userName);
			PKT_S2C_ResupplyPlayer_s n;
			n.result = 2;
			n.timeLeft = _rc_ResupplyInterval-(curTime - plr->m_LastTimeResupplied);
			gServerLogic.p2pBroadcastToActive(plr, &n, sizeof(n));

			// reset flag
			plr->m_RequestedResupply = false;
			continue;
		}

		// check radius
		float distSq = (plr->GetPosition() - GetPosition()).LengthSq();
		if(distSq > (_rc_ResupplyRadius * _rc_ResupplyRadius))
		{
			//r3dOutToLog("RC: %s too far\n", plr->userName);
			//PKT_S2C_ResupplyPlayer_s n;
			//n.result = 3;
			//n.timeLeft = 0;
			//gServerLogic.p2pBroadcastToActive(plr, &n, sizeof(n));
			// player too far away from this crate, just skip it
			continue;
		}

		// reset flag, as it this point it is our crate
		plr->m_RequestedResupply = false;

		// check if we can resupply from this crate
		if(!CanResupply(plr))
		{
			//r3dOutToLog("RC: %s can not resupply\n", plr->userName);
			PKT_S2C_ResupplyPlayer_s n;
			n.result = 4;
			n.timeLeft = 0;
			gServerLogic.p2pBroadcastToActive(plr, &n, sizeof(n));
			continue;
		}

		r3dOutToLog("RC: %s resupplied\n", plr->userName);

		// do resupply
		plr->DoResupply();

		PKT_S2C_ResupplyPlayer_s n;
		n.result = 1;
		n.timeLeft = 0;
		gServerLogic.p2pBroadcastToActive(plr, &n, sizeof(n));

		// reward for using blackop crate
		if(isBlackops_)
		{
			gServerLogic.AddPlayerReward(plr, RWD_BlackopResupply);
		}
	}

	return;
}

BOOL obj_ServerSupplyCrate::Update()
{
	const float curTime = r3dGetTime();
	if(curTime > nextScan_) {
		nextScan_ = curTime + .25f;
		ScanForPlayers();
	}

	return TRUE;
}
