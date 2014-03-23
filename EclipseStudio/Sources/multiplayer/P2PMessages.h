#ifndef __P2PMESSAGES_H__
#define __P2PMESSAGES_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "r3dNetwork.h"
#include "GameObjects/EventTransport.h"
#include "GameCode/UserProfile.h"
#include "GameCode/UserSkills.h"
#include "../../../ServerNetPackets/NetPacketsGameInfo.h"
#include "../../../ServerNetPackets/NetPacketsWeaponInfo.h"
#include "Gameplay_Params.h"

class GameObject;

// all data packets should be as minimal as possible - so, no data aligning
#pragma pack(push)
#pragma pack(1)
                                 //dec 30
#define P2PNET_VERSION		(0x0000001E + GBWEAPINFO_VERSION + GBGAMEINFO_VERSION + GAMEPLAYPARAM_VERSION)

#define NETID_PLAYERS_START	1		// players [1--99]
#define NETID_CONTROLS_START	100		// control points [100-150]
#define NETID_SIEGEOBJ_START	151		// siege objective [151-199]
#define NETID_OBJECTS_START	200		// various spawned objects [200-0xffff]

static const int NUM_WEAPONS_ON_PLAYER = 7;

enum pkttype_e
{
  PKT_C2S_ValidateConnectingPeer = r3dNetwork::FIRST_FREE_PACKET_ID,

  PKT_C2C_PacketBarrier,		// server<->client per object packet indicating logical barrier for received packets

  PKT_S2C_LevelInfo,
  
  PKT_C2S_JoinGameReq,
  PKT_S2C_JoinGameAns,
  PKT_S2C_ShutdownNote,
  PKT_S2C_SetGamePlayParams,
  PKT_S2C_CreatePlayer,
  PKT_S2C_DropPlayer,
  
  PKT_C2S_StartGameReq,
  PKT_S2C_StartGameAns,

  PKT_S2C_GameAboutToStart,
  PKT_S2C_GameStarted,

  // bomb mode
  PKT_S2C_Bomb_ConnectedPlayer,
  PKT_S2C_Bomb_DisconnectedPlayer,
  PKT_C2S_Bomb_RequestPlayerKick,
  PKT_C2S_Bomb_PlayerReady,
  PKT_S2C_Bomb_PlayerReady,
  PKT_S2C_Bomb_PlayerHasBomb, 
  PKT_C2S_Bomb_RequestBombPlacement,
  PKT_S2C_Bomb_BombPlaced,
  PKT_S2C_Bomb_Exploded,
  PKT_S2C_Bomb_WonRound,
  PKT_S2C_Bomb_Dropped,
  PKT_C2S_Bomb_RequestDrop,
  PKT_C2S_Bomb_RequestBombPickup,
  PKT_C2S_Bomb_RequestTeamChange,
  PKT_C2S_Bomb_ChatMsg,
  PKT_S2C_Bomb_ChatMsg,
  
  // weapon fire packets
  PKT_C2C_PlayerFired,		// player fired event. play muzzle effect, etc. actual HIT event is separate event.
							// this will increment server side counter, and HIT packet will decrement it. So, need to make sure that for each FIRE packet
							// we send HIT packet to prevent bullet cheating

  PKT_C2C_PlayerHitNothing,
  PKT_C2C_PlayerHitStatic, // hit static geometry
  PKT_C2C_PlayerHitStaticPierced, // hit static geometry and pierced through it, will be followed up by another HIT event
  PKT_C2C_PlayerHitDynamic, // hit something that can be damaged (player, UAV, etc)

  PKT_C2C_PlayerSwitchWeapon,
  PKT_C2C_PlayerUseItem,
  PKT_S2C_CreateNetObject,
  PKT_S2C_DestroyNetObject,
  PKT_S2C_PlayerUsedItemAns, // this packet is sent for immediate action items, like bandages, or morphine shot

  PKT_C2C_PlayerReadyGrenade,
  PKT_C2S_CreateExplosion, // grenades as well as mines. 

  PKT_C2S_MarkTarget,
  PKT_S2C_TargetMarked,

  // movement packets
  PKT_S2C_MoveTeleport,
  PKT_C2C_MoveSetCell,			// set cell origin for PKT_C2C_MoveRel updates
  PKT_C2C_MoveRel,
  PKT_C2C_PlayerJump,
  
  PKT_C2S_Temp_Damage,			// temporary damage packet, client send them. used for rockets/explosions. This packet will always apply damage, even to friendlies!!!
  PKT_C2S_FallingDamage,	// player fell

  PKT_S2C_Damage,
  PKT_S2C_KillPlayer,
  
  PKT_S2C_SetPlayerScore,		// broadcasting message used to set score for player
  PKT_S2C_AddScore,			// single player message to display hp/gp gain
  PKT_S2C_UnlockAchievement,

  PKT_C2S_SetRespawnData,
  PKT_S2C_RespawnPlayer,
  
  PKT_S2C_SpawnExplosion, // spawn visual effect of explosion only

  PKT_C2S_RequestAirstrike,
  PKT_S2C_Airstrike,

  PKT_S2C_ControlPointUpdate,

  PKT_C2S_Siege_Activate,
  PKT_S2C_SiegeUpdate,

  PKT_C2S_ResupplyRequest,
  PKT_S2C_ResupplyPlayer,
  
  PKT_S2C_GameAbort,			// might be used for master server game termination request
  PKT_S2C_GameFinish,			// game is finished
  PKT_S2C_RoundStats,			// per player packet containing current round statistics
  PKT_S2C_GameClose,			// actual game close packet, all profiles is updated at this moment

  PKT_C2C_ChatMessage,			// client sends to server chat message, server will relay it to everyone except for sender

  PKT_C2C_VoiceCommand,
  PKT_C2C_CommRoseCommand,
  
  // UAV packets
  PKT_S2C_UAVSetState,

  // Conquest logic packets
  PKT_S2C_TicketsUpdate,

  // team switch
  PKT_S2C_TeamSwitchError,

  // data update packets
  PKT_C2S_DataUpdateReq,
  PKT_S2C_UpdateWeaponData,
  PKT_S2C_UpdateGearData,
  
  // abilities
  PKT_S2C_BerserkerAbility,
  PKT_S2C_BigSurpriseAbility,

  // weapon drop
  PKT_S2C_SpawnDroppedWeapon,
  PKT_S2C_DestroyDroppedWeapon,
  PKT_C2S_RequestWeaponPickup, // request pick up
  PKT_S2C_WeaponPickedUp, // server replies this if you are able to pickup that weapon
  PKT_C2C_ConfirmWeaponPickup, // client has to confirm that weapon was picked up and let server and everyone else know that now it shoots with new weapon

  // loot drop
  PKT_S2C_SpawnDroppedLootBox,
  PKT_S2C_DestroyDroppedLootBox,
  PKT_S2C_LootBoxPickedUp, // informs client that it picked up loot box

  // mines
  PKT_C2S_PlayerCreateMine,
  PKT_S2C_SpawnMine,
  PKT_C2S_TriggerMine, // for now we have to trust clients to send us correct trigger :(
  PKT_S2C_ExplodeMine, // if mine exploded and destroyed, server will NOT send separate DestroyMine packet
  PKT_S2C_DestroyMine, // if mine silently destroyed, without explosion
  PKT_S2C_NewOwnerMine, // when mine switched its owner
  PKT_C2C_PlayerSwitchMine, // switch owner of the mine, used by The Fixer ability. if user able to do this, server will relay packet to everyone else

  // light meshes
  PKT_S2C_LightMeshStatus, // sends this when we need to turn off light in light mesh

  // some security
  PKT_C2S_SecurityRep,
  PKT_C2S_PlayerWeapDataRep,
  PKT_S2C_CheatWarning,
  PKT_C2S_ScreenshotData,
  
  // admin
  PKT_C2S_Admin_PlayerKick,

  // some test things
  PKT_C2S_TEST_SpawnDummyReq,
  PKT_C2S_TEST_PlayerSetWeapon,
  PKT_C2S_DBG_LogMessage,
  
  PKT_LAST_PACKET_ID,
};

#if PKT_LAST_PACKET_ID > 255
  #error Shit happens, more that 255 packet ids
#endif

#define DEFINE_PACKET_HANDLER(xxx) \
  case xxx: { \
    const xxx##_s&n = *(xxx##_s*)packetData; \
    if(packetSize != sizeof(n)) { \
      r3dOutToLog("!!!!errror!!!! %s packetSize %d != %d\n", #xxx, packetSize, sizeof(n)); \
      return TRUE; \
    } \
    bool needPassThru = false; \
    On##xxx(n, fromObj, peerId, needPassThru); \
    return needPassThru ? FALSE : TRUE; \
  }

struct PKT_C2S_ValidateConnectingPeer_s : public DefaultPacketMixin<PKT_C2S_ValidateConnectingPeer>
{
	DWORD		protocolVersion;
	// must be set by client to correctly connect to game server
	__int64		sessionId;
};

struct PKT_C2C_PacketBarrier_s : public DefaultPacketMixin<PKT_C2C_PacketBarrier>
{
};

struct PKT_S2C_LevelInfo_s : public DefaultPacketMixin<PKT_S2C_LevelInfo>
{
	GBGameInfo	gameInfo;
};

struct PKT_C2S_JoinGameReq_s : public DefaultPacketMixin<PKT_C2S_JoinGameReq>
{
	DWORD		CustomerID;
	DWORD		SessionID;
};

struct PKT_S2C_JoinGameAns_s : public DefaultPacketMixin<PKT_S2C_JoinGameAns>
{
	BYTE		success;
	BYTE		playerIdx;
	float		gameTimeLeft;
	float		gameStartTime;
};

struct PKT_S2C_ShutdownNote_s : public DefaultPacketMixin<PKT_S2C_ShutdownNote>
{
	BYTE		reason;
	float		timeLeft;
};

struct PKT_S2C_SetGamePlayParams_s : public DefaultPacketMixin<PKT_S2C_SetGamePlayParams>
{
	DWORD		GPP_Seed;	// per-session value used to xor crc of gpp
	CGamePlayParams	GPP_Data;
	float		airstrike_cooldowns[10]; // from AirstrikeShared.cpp: g_NumAirstrikesInArray*2 + 2 for team cooldowns
};

struct PKT_S2C_CreatePlayer_s : public DefaultPacketMixin<PKT_S2C_CreatePlayer>
{
	BYTE		playerIdx;
	BYTE		teamId;
	BYTE		bDying;
	BYTE		isPremium;
	BYTE		slotNum;	// index of loadout slot
	char		userName[64];
	r3dPoint3D	spawnPos;
	float		spawnDir;
	float		spawnProteciton;
	r3dPoint3D	moveCell;	// cell position from PKT_C2C_MoveSetCell
	int		weapIndex; // index of equipped weapon (-1 for default)
	wiLoadoutSlot	lslot;
	wiWeaponAttachments attms;
	int		score; // for kill tag and scoreboard
	int		wins;
	int		losses;
	int		ClanID;
	char		ClanTag[5*2]; // utf8
	int		ClanTagColor;
};

struct PKT_S2C_DropPlayer_s : public DefaultPacketMixin<PKT_S2C_DropPlayer>
{
	BYTE		playerIdx;
	BYTE		reason;
};

struct PKT_C2S_StartGameReq_s : public DefaultPacketMixin<PKT_C2S_StartGameReq>
{
	DWORD lastNetID; // to check sync
	bool  requestSpectator;
};

struct PKT_S2C_StartGameAns_s : public DefaultPacketMixin<PKT_S2C_StartGameAns>
{
	enum EResult {
	  RES_Ok = 0,
	  RES_Pending = 1,		// server still getting your profile
	  RES_Failed = 2,		// server was unable to get your profile
	  RES_UNSYNC = 3,
	};
	BYTE		result;		// status of joining
	BYTE		gameStarted;
	BYTE		autoplacedTeamId; // to which team server assigned this player on joining
};

struct PKT_S2C_GameAboutToStart_s : public DefaultPacketMixin<PKT_S2C_GameAboutToStart>
{
	float		gameTimeLeft;
};

struct PKT_S2C_GameStarted_s : public DefaultPacketMixin<PKT_S2C_GameStarted>
{
	r3dPoint3D	teleport_pos;
	float		dir;
};

struct PKT_S2C_Bomb_ConnectedPlayer_s : public DefaultPacketMixin<PKT_S2C_Bomb_ConnectedPlayer>
{
	DWORD peerID; // to ID players
	char  userName[64];
	BYTE  teamID;
	BYTE  isReady;
	BYTE  plrLevel;
	BYTE  isMaster; // set to true only for player who created game, he can start game and kick other players
	BYTE  isSpectator;
};

struct PKT_C2S_Bomb_PlayerReady_s : public DefaultPacketMixin<PKT_C2S_Bomb_PlayerReady>
{
	BYTE selectedLoadoutSlot;
};

struct PKT_S2C_Bomb_PlayerReady_s : public DefaultPacketMixin<PKT_S2C_Bomb_PlayerReady>
{
	DWORD peerID;
	BYTE isReady;
};

struct PKT_S2C_Bomb_PlayerHasBomb_s : public DefaultPacketMixin<PKT_S2C_Bomb_PlayerHasBomb>
{
	gp2pnetid_t playerID; 
};

struct PKT_C2S_Bomb_RequestBombPlacement_s : public DefaultPacketMixin<PKT_C2S_Bomb_RequestBombPlacement>
{
	gp2pnetid_t bombID;
	r3dPoint3D pos;
};

struct PKT_S2C_Bomb_BombPlaced_s : public DefaultPacketMixin<PKT_S2C_Bomb_BombPlaced>
{
	r3dPoint3D pos;
};

struct PKT_S2C_Bomb_Exploded_s : public DefaultPacketMixin<PKT_S2C_Bomb_Exploded>
{
};

struct PKT_S2C_Bomb_Dropped_s : public DefaultPacketMixin<PKT_S2C_Bomb_Dropped>
{
	r3dPoint3D pos;
};

struct PKT_C2S_Bomb_RequestDrop_s : public DefaultPacketMixin<PKT_C2S_Bomb_RequestDrop>
{
};

struct PKT_C2S_Bomb_RequestBombPickup_s : public DefaultPacketMixin<PKT_C2S_Bomb_RequestBombPickup>
{
};

struct PKT_C2S_Bomb_RequestTeamChange_s : public DefaultPacketMixin<PKT_C2S_Bomb_RequestTeamChange>
{
	BYTE requestSpectator;
};

// separate chat packets for lobby, as we don't have players in lobby, only server knows who is connected
// they are quite heavy, but hopefully ppl will not be spending too much time in lobby
struct PKT_C2S_Bomb_ChatMsg_s : public DefaultPacketMixin<PKT_C2S_Bomb_ChatMsg>
{
	bool isTeam;
	char msg[128]; // actual text
};

struct PKT_S2C_Bomb_ChatMsg_s : public DefaultPacketMixin<PKT_S2C_Bomb_ChatMsg>
{
	bool isTeam; // if this is a team only chat message (for UI purposes)
	bool isAlly;
	int senderPeerId;
	char msg[128];
};

struct PKT_S2C_Bomb_WonRound_s : public DefaultPacketMixin<PKT_S2C_Bomb_WonRound>
{
	BYTE teamID; // which team won this round
};

struct PKT_S2C_Bomb_DisconnectedPlayer_s : public DefaultPacketMixin<PKT_S2C_Bomb_DisconnectedPlayer>
{
	DWORD peerID;
};

struct PKT_C2S_Bomb_RequestPlayerKick_s : public DefaultPacketMixin<PKT_C2S_Bomb_RequestPlayerKick>
{
	DWORD peerIDtoKick;
};

struct PKT_C2C_PlayerFired_s : public DefaultPacketMixin<PKT_C2C_PlayerFired>
{
	r3dPoint3D	fire_from; // position of character when he is firing
	r3dPoint3D	fire_to;
	float		holding_delay; // if any, used for grenades. THIS IS TEMP until grenades are moved to server(!!)
	BYTE		debug_wid;	// weapon index for debug
	BYTE		wasAiming; // if player was aiming when shot. needed for some achievements
};

struct PKT_C2C_PlayerHitNothing_s : public DefaultPacketMixin<PKT_C2C_PlayerHitNothing>
{
	// empty
};

struct PKT_C2C_PlayerHitStatic_s : public DefaultPacketMixin<PKT_C2C_PlayerHitStatic>
{
	r3dPoint3D	hit_pos;
	r3dPoint3D	hit_norm;
	uint32_t	hash_obj;
	BYTE		decalIdx;
	BYTE		particleIdx;
};

//IMPORTANT: This packet should be equal to PKT_C2C_PlayerHitStatic_s!!!
struct PKT_C2C_PlayerHitStaticPierced_s : public DefaultPacketMixin<PKT_C2C_PlayerHitStaticPierced>
{
	r3dPoint3D	hit_pos;
	r3dPoint3D	hit_norm;
	uint32_t	hash_obj;
	BYTE		decalIdx;
	BYTE		particleIdx;
};

struct PKT_C2C_PlayerHitDynamic_s : public DefaultPacketMixin<PKT_C2C_PlayerHitDynamic>
{
	r3dPoint3D	hit_pos; // where your bullet hit
	gp2pnetid_t	targetId; // what you hit
	BYTE		hit_body_bone; // which bone we hit (for ragdoll)
	BYTE		state; // [0] - from player in air [1] target player in air
	
	// NEEDED FOR SERVER ONLY. TEMP. WILL BE REFACTORED AND MOVED TO SERVER.
	BYTE		hit_body_part;// where we hit player (head, body, etc)
	BYTE		damageFromPiercing; // 0 - no reduction, 100 - no damage at all
};

struct PKT_S2C_MoveTeleport_s : public DefaultPacketMixin<PKT_S2C_MoveTeleport>
{
	r3dPoint3D	teleport_pos; // don't forget to PKT_C2C_PacketBarrier ir
};

struct PKT_C2C_MoveSetCell_s : public DefaultPacketMixin<PKT_C2C_MoveSetCell>
{
	// define radius where relative packets will be sent
	const static int PLAYER_CELL_RADIUS = 5;  // packed difference: 0.04m
	const static int UAV_CELL_RADIUS    = 10;
	const static int VEHICLE_CELL_RADIUS = 10;

	r3dPoint3D	pos;
};

struct PKT_C2C_MoveRel_s : public DefaultPacketMixin<PKT_C2C_MoveRel>
{
	// (CELL_RADIUS*2)/[0-255] offset from previously received absolute position
	BYTE		rel_x;
	BYTE		rel_y;
	BYTE		rel_z;

	BYTE		turnAngle;	// [0..360] packed to [0..255]
	BYTE		bendAngle;	// [-PI/2..PI/2] packet to [0..255]
	BYTE		state;		// reflected PlayerState. [0..3] bits - state, [4,7] - dir
};

struct PKT_C2C_PlayerJump_s : public DefaultPacketMixin<PKT_C2C_PlayerJump>
{
};

struct PKT_C2C_PlayerSwitchWeapon_s : public DefaultPacketMixin<PKT_C2C_PlayerSwitchWeapon>
{
	BYTE		wid;
};

struct PKT_C2C_PlayerUseItem_s : public DefaultPacketMixin<PKT_C2C_PlayerUseItem>
{
	DWORD		itemId;
	r3dPoint3D	pos;
	// various parameters for items
	float		var1;
	float		var2;
	float		var3;
	float		var4;
};

struct PKT_S2C_PlayerUsedItemAns_s : public DefaultPacketMixin<PKT_S2C_PlayerUsedItemAns>
{
	DWORD		itemId;
	// various parameters for items
	float		var1;
	float		var2;
	float		var3;
	float		var4;
};

//NOTE: packet must be sent from player object
struct PKT_S2C_CreateNetObject_s : public DefaultPacketMixin<PKT_S2C_CreateNetObject>
{
	gp2pnetid_t	spawnID;
	DWORD		itemID;

	r3dPoint3D	pos;
	// various parameters for items
	float		var1;
	float		var2;
	float		var3;
	float		var4;
	float		var5;
};

struct PKT_S2C_DestroyNetObject_s : public DefaultPacketMixin<PKT_S2C_DestroyNetObject>
{
	gp2pnetid_t spawnID;
};

struct PKT_C2C_PlayerReadyGrenade_s : public DefaultPacketMixin<PKT_C2C_PlayerReadyGrenade>
{
	BYTE		wid;
};

struct PKT_C2S_MarkTarget_s : public DefaultPacketMixin<PKT_C2S_MarkTarget>
{
	gp2pnetid_t targetID;
};

struct PKT_S2C_TargetMarked_s : public DefaultPacketMixin<PKT_S2C_TargetMarked>
{
	gp2pnetid_t targetID;
	float time; // how long it will stay marked
};


//This packet will always apply damage, even to friendlies!!!
struct PKT_C2S_Temp_Damage_s : public DefaultPacketMixin<PKT_C2S_Temp_Damage>
{
	gp2pnetid_t	targetId;
    BYTE        wpnIdx;
    BYTE        damagePercentage;
	r3dPoint3D  explosion_pos;
};

struct PKT_C2S_FallingDamage_s : public DefaultPacketMixin<PKT_C2S_FallingDamage>
{
    float damage;
};

struct PKT_S2C_Damage_s : public DefaultPacketMixin<PKT_S2C_Damage>
{
	r3dPoint3D	dmgPos; // position of the damage. for bullets: player position. for grenades\mines\rpg - position of explosion
	gp2pnetid_t	targetId;
	BYTE		damage;
	BYTE		dmgType;
	BYTE		bodyBone;
};

struct PKT_S2C_KillPlayer_s : public DefaultPacketMixin<PKT_S2C_KillPlayer>
{
	// FromID in packet header will be a killer object
	gp2pnetid_t	targetId;
	BYTE		killerWeaponCat; // with what weapon player was killed
	bool		forced_by_server;
};

struct PKT_S2C_SetPlayerScore_s : public DefaultPacketMixin<PKT_S2C_SetPlayerScore>
{
	int		score;
	WORD		kills;
	WORD		deaths;
};

struct PKT_S2C_AddScore_s : public DefaultPacketMixin<PKT_S2C_AddScore>
{
	PKT_S2C_AddScore_s& operator= (const PKT_S2C_AddScore_s& rhs) {
	  memcpy(this, &rhs, sizeof(*this));
	  return *this;
	}

	WORD		ID;	// id of reward (defined in UserRewards.h for now)
	signed short	HP;	// honor points
	WORD		GD;	// game dollars
};

struct PKT_S2C_UnlockAchievement_s : public DefaultPacketMixin<PKT_S2C_UnlockAchievement>
{
	WORD		achiIdx;	// index of achievment
};

struct PKT_C2S_SetRespawnData_s : public DefaultPacketMixin<PKT_C2S_SetRespawnData>
{
	DWORD		spawnId;	// spawnID can be NetwordID for respawn beacons usage
	BYTE		teamId;
	BYTE		slotNum;
};

struct PKT_S2C_RespawnPlayer_s : public DefaultPacketMixin<PKT_S2C_RespawnPlayer>
{
	r3dPoint3D	pos;
	float		dir;
	float		spawnProtection;
	BYTE		teamId;
	BYTE		slotNum;
	wiLoadoutSlot	lslot;
	wiWeaponAttachments attms;
};

struct PKT_S2C_ControlPointUpdate_s : public DefaultPacketMixin<PKT_S2C_ControlPointUpdate>
{
	BYTE		packet_status;		// -1: red team, 0

	void		pack(float status) {
	  status = R3D_CLAMP(status, -1.0f, 1.0f);
	  status = ((status + 1.0f) * 127);
	  packet_status = BYTE(status);
	}
	float		unpack() const {
	  float status = float(packet_status);
	  status /= 127;
	  status -= 1.0f;
	  status = R3D_CLAMP(status, -1.0f, 1.0f);
	  return status;
	}
};

struct PKT_S2C_ResupplyPlayer_s : public DefaultPacketMixin<PKT_S2C_ResupplyPlayer>
{
	int result; // 0-fail, 1-success, 2-timer, 3-radius, 4-key
	float timeLeft; // time left until next resupply allowed
};

struct PKT_C2S_ResupplyRequest_s : public DefaultPacketMixin<PKT_C2S_ResupplyRequest>
{
};

struct PKT_C2S_Siege_Activate_s : public DefaultPacketMixin<PKT_C2S_Siege_Activate>
{
	gp2pnetid_t ObjectiveID;
};

struct PKT_S2C_SiegeUpdate_s : public DefaultPacketMixin<PKT_S2C_SiegeUpdate>
{
	BYTE status;
	float destruction_timer;
};

struct PKT_S2C_GameAbort_s : public DefaultPacketMixin<PKT_S2C_GameAbort>
{
};

struct PKT_S2C_RoundStats_s : public DefaultPacketMixin<PKT_S2C_RoundStats>
{
	wiStats		rstat;
	wiStatsTracking	rscore;
	BYTE levelUpMin; // 0 - no levelup
	BYTE levelUpMax;
	BYTE oneTimeReward; // one time reward from server: 0-100WP, 1-LootBox1, 2-LootBox2, 3-LootBox3, 4-2xWP for 20min, 5-2xXP for 20min
};

struct PKT_S2C_GameFinish_s : public DefaultPacketMixin<PKT_S2C_GameFinish>
{
	PKT_S2C_GameFinish_s& operator= (const PKT_S2C_GameFinish_s& rhs) {
	  memcpy(this, &rhs, sizeof(*this));
	  return *this;
	}

	enum EReasons {
	  REASON_Timeout,
	  REASON_Ticket,
	  REASON_Draw,	// noone win
	};
	BYTE		reason;
	BYTE		winTeam;
};

struct PKT_S2C_GameClose_s : public DefaultPacketMixin<PKT_S2C_GameClose>
{
	BYTE		res;
};

struct PKT_S2C_TicketsUpdate_s : public DefaultPacketMixin<PKT_S2C_TicketsUpdate>
{
	WORD		tickets[2];
};

struct PKT_S2C_TeamSwitchError_s : public DefaultPacketMixin<PKT_S2C_TeamSwitchError>
{
	enum ERROR_TYPE
	{
		ERR_TOO_EARLY=0,
		ERR_ALMOST_FINISHED,
		ERR_JOINING_WINNING_TEAM,
		ERR_JOINING_TEAM_WITH_MORE_PLAYERS,
		
		OK_MOVED_TEAM0 = 0x20,	// special case when server forcibly switched team
		OK_MOVED_TEAM1 = 0x21,	// special case when server forcibly switched team
	};
	BYTE		errorType; 
};

struct PKT_C2C_ChatMessage_s : public DefaultPacketMixin<PKT_C2C_ChatMessage>
{
#define CHAT_MSGTYPE_GENERAL 0
#define CHAT_MSGTYPE_TEAM 1
#define CHAT_MSGTYPE_WHISPER 2
#define CHAT_MSGTYPE_CLAN 3
#define CHAT_MSGTYPE_SUICIDE 129

	BYTE		msgType;  // 0 - public, 1-team, 2-whisper 129 - Suicide
	gp2pnetid_t	target;   // in case if it is a private message, target will be to whom this message is addressed
	char		msg[128]; // actual text
};

struct PKT_C2C_VoiceCommand_s : public DefaultPacketMixin<PKT_C2C_VoiceCommand>
{
	BYTE id;
};

struct PKT_C2C_CommRoseCommand_s : public DefaultPacketMixin<PKT_C2C_CommRoseCommand>
{
	BYTE id;
	r3dVector pos;
};

struct PKT_S2C_UAVSetState_s : public DefaultPacketMixin<PKT_S2C_UAVSetState>
{
	BYTE		state;		// 0 - active, 1 - damaged, 2 - killed
	gp2pnetid_t	killerId;
};

struct PKT_C2S_DataUpdateReq_s : public DefaultPacketMixin<PKT_C2S_DataUpdateReq>
{
};

struct PKT_S2C_UpdateWeaponData_s : public DefaultPacketMixin<PKT_S2C_UpdateWeaponData>
{
	DWORD		itemId;
	GBWeaponInfo	wi;
};

struct PKT_S2C_UpdateGearData_s : public DefaultPacketMixin<PKT_S2C_UpdateGearData>
{
	DWORD		itemId;
	GBGearInfo	gi;
};

struct PKT_S2C_BerserkerAbility_s : public DefaultPacketMixin<PKT_S2C_BerserkerAbility>
{
	float time;
};

struct PKT_S2C_BigSurpriseAbility_s : public DefaultPacketMixin<PKT_S2C_BigSurpriseAbility>
{
};

struct PKT_S2C_SpawnExplosion_s : public DefaultPacketMixin<PKT_S2C_SpawnExplosion>
{
	r3dPoint3D pos;
    float radius;
};

struct PKT_C2S_RequestAirstrike_s : public DefaultPacketMixin<PKT_C2S_RequestAirstrike>
{
	r3dPoint3D pos; // pos of where airstrike should hit (todo: that should be calculated on server after physics will be on server)
	int itemID; // itemID of the airstrike
};

// server sends confirmed airstrike to everyone, and then damage packets after that
struct PKT_S2C_Airstrike_s : public DefaultPacketMixin<PKT_S2C_Airstrike>
{
	r3dPoint3D pos;
	int itemID; // if itemID == 0 - unknown error, itemID=1 - GP, itemID=2-cooldown, itemID=3-not authorized 100 - success
	float heightOffset; // >1000 - spawn explosion
};

struct PKT_C2S_SecurityRep_s : public DefaultPacketMixin<PKT_C2S_SecurityRep>
{
	static const int REPORT_PERIOD = 15; // we should send report every <this> sec

	float		gameTime;
	bool		detectedWireframeCheat;
	DWORD		GPP_Crc32;
};

struct PKT_C2S_PlayerWeapDataRep_s : public DefaultPacketMixin<PKT_C2S_PlayerWeapDataRep>
{
	static const int REPORT_PERIOD = 15; // we should send report every <this> sec
	
	DWORD		weaponsDataHash[NUM_WEAPONS_ON_PLAYER];
	DWORD		debug_wid[NUM_WEAPONS_ON_PLAYER];
	GBWeaponInfo	debug_winfo[NUM_WEAPONS_ON_PLAYER];
};

struct PKT_S2C_CheatWarning_s : public DefaultPacketMixin<PKT_S2C_CheatWarning>
{
	enum ECheatId {
		CHEAT_SpeedHack = 1,
		CHEAT_NumShots,
		CHEAT_BadWeapData,
		CHEAT_Wireframe,
		CHEAT_WeaponPickup,
		CHEAT_MineSwitch,
		CHEAT_GPP,	// game player parameters mismatch
		CHEAT_HighKDRatio,
		CHEAT_ShootDistance,
		CHEAT_FastMove,
		CHEAT_LootboxPickup,
		CHEAT_AFK,
		CHEAT_UseItem,
	};

	BYTE		cheatId;
};

struct PKT_S2C_SpawnDroppedWeapon_s : public DefaultPacketMixin<PKT_S2C_SpawnDroppedWeapon>
{
	gp2pnetid_t spawnID;
	r3dVector	pos;
	r3dVector	rot;
	uint32_t	itemID;
	WORD		numBullets; // num bullets in the gun that left
	wiWeaponAttachment attms;
};

struct PKT_S2C_DestroyDroppedWeapon_s : public DefaultPacketMixin<PKT_S2C_DestroyDroppedWeapon>
{
	gp2pnetid_t spawnID;
};

struct PKT_C2S_RequestWeaponPickup_s : public DefaultPacketMixin<PKT_C2S_RequestWeaponPickup>
{
	static const int PICKUP_RADIUS = 3; // radius in meters to allow pickup
	gp2pnetid_t spawnID; // what weapon we are trying to pick up
};

struct PKT_S2C_WeaponPickedUp_s : public DefaultPacketMixin<PKT_S2C_WeaponPickedUp>
{
	uint32_t itemID; // itemID of the weapon. send it directly in case if it will timeout right after this packet
	WORD	numBullets; // num bullets in the gun that left
	BYTE secretCode; // client has to confirm pickup with this code
	GBWeaponInfo	wi;	// updated weapon info
	wiWeaponAttachment attms;
};

struct PKT_C2C_ConfirmWeaponPickup_s : public DefaultPacketMixin<PKT_C2C_ConfirmWeaponPickup>
{
	uint32_t itemID;
	BYTE secretCode; // server will check this code and remove it when resending it to everyone else
};

struct PKT_S2C_SpawnDroppedLootBox_s : public DefaultPacketMixin<PKT_S2C_SpawnDroppedLootBox>
{
	gp2pnetid_t spawnID;
	r3dVector	pos;
	r3dVector	rot;
	uint32_t	itemID;
};

struct PKT_S2C_DestroyDroppedLootBox_s : public DefaultPacketMixin<PKT_S2C_DestroyDroppedLootBox>
{
	gp2pnetid_t spawnID;
};

struct PKT_S2C_LootBoxPickedUp_s : public DefaultPacketMixin<PKT_S2C_LootBoxPickedUp>
{
	uint32_t itemID; // itemID of the box
};

struct PKT_C2S_ScreenshotData_s : public DefaultPacketMixin<PKT_C2S_ScreenshotData>
{
	BYTE		errorCode;	// 0 if success
	WORD		dataSize;
	// this packet will have attached screenshot data after header
	// char		data[0x7FFF];
};

struct PKT_C2S_Admin_PlayerKick_s : public DefaultPacketMixin<PKT_C2S_Admin_PlayerKick>
{
	gp2pnetid_t netID; // netID of who to kick
};

struct PKT_C2S_TEST_SpawnDummyReq_s : public DefaultPacketMixin<PKT_C2S_TEST_SpawnDummyReq>
{
	r3dPoint3D	pos;
};

struct PKT_C2S_TEST_PlayerSetWeapon_s : public DefaultPacketMixin<PKT_C2S_TEST_PlayerSetWeapon>
{
	DWORD		weapId;
};

// mines
struct PKT_C2S_PlayerCreateMine_s : public DefaultPacketMixin<PKT_C2S_PlayerCreateMine>
{
	BYTE wid;
	r3dVector pos;
	r3dVector rot;
};

struct PKT_S2C_SpawnMine_s : public DefaultPacketMixin<PKT_S2C_SpawnMine>
{
	gp2pnetid_t spawnID;
	uint32_t itemID;
	r3dVector pos;
	r3dVector rot;
};

struct PKT_C2S_TriggerMine_s : public DefaultPacketMixin<PKT_C2S_TriggerMine>
{
};

struct PKT_S2C_ExplodeMine_s : public DefaultPacketMixin<PKT_S2C_ExplodeMine>
{
};

struct PKT_S2C_DestroyMine_s : public DefaultPacketMixin<PKT_S2C_DestroyMine>
{
};

struct PKT_S2C_NewOwnerMine_s : public DefaultPacketMixin<PKT_S2C_NewOwnerMine>
{
	gp2pnetid_t newOwnerID;
};

struct PKT_C2C_PlayerSwitchMine_s : public DefaultPacketMixin<PKT_C2C_PlayerSwitchMine>
{
	gp2pnetid_t mineID;
};

struct PKT_S2C_LightMeshStatus_s : public DefaultPacketMixin<PKT_S2C_LightMeshStatus>
{
};

struct PKT_C2S_CreateExplosion_s: public DefaultPacketMixin<PKT_C2S_CreateExplosion>
{
	r3dVector pos;
	float radius;
	int wpnIndex;
	// for claymores. 
	r3dVector forwVector;
	float direction;
};

struct PKT_C2S_DBG_LogMessage_s : public DefaultPacketMixin<PKT_C2S_DBG_LogMessage>
{
	char	msg[128];
};

#pragma pack(pop)

#endif	// __P2PMESSAGES_H__

