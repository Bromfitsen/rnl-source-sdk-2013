//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_SHAREDDEFS_H
#define RNL_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "shareddefs.h"
#include "rnl_dt_shared.h"

// After this many seconds, the grenade will be thrown instead of rolled
#define RNL_GRENADE_THROW_DELAY 0.25f

#define RNL_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )

#define LOWEST_CONTROLLED_CLIMB 40

#define RNL_DEFAULT_PLAYER_MODEL "models/gman.mdl"

const char *WeaponIDToAlias( int id );
int AliasToWeaponID( const char *alias );

const char *TeamNumberToName( int id );
int TeamNameToNumber( const char *name );

struct RnLBulletInfo_t
{
	Vector m_vecPosition;
	Vector m_vecDirShooting;
	Vector m_vecVelocity;
	
	float m_flPower;
	float m_flDistance;
	float m_flLifeTime;
	
	int m_iAmmoType;
	int m_iDamage;
	bool m_bDoEffects;

	CBaseEntity* m_pAttacker;
	CBaseEntity* m_pWeapon;
	CBaseEntity* m_pInflictor;
	int m_iPrevUpdate;
#ifndef CLIENT_DLL
	int m_iTickCount;
#endif
};

//RnL Damage types
#define DMG_RNL_TNT		(1<<30)

// These are the names of the ammo types that the weapon script files reference.
#define AMMO_GARAND				"30-06"
#define AMMO_CARBINE			"AMMO_CARBINE"
#define AMMO_BAR				"AMMO_BAR"
#define AMMO_BROWNING			"AMMO_BROWNING"
#define AMMO_THOMPSON			"45acp"
#define AMMO_COLT				"45acp_colt"
#define AMMO_GREASEGUN			"AMMO_GREASEGUN"

#define AMMO_MK2GREN			"mk2"
#define AMMO_M18GREN			"m18"

// german weapons
#define AMMO_K98K				"792"
#define AMMO_MP40				"9mm"
#define AMMO_MP44				"AMMO_MP44"
#define AMMO_G43				"AMMO_G43"
#define AMMO_WALTHER			"9mm_walther"
#define AMMO_MG42				"AMMO_MG42"
#define AMMO_MG34				"AMMO_MG34"

#define AMMO_STIEL24			"stiel24"
#define AMMO_STIEL39			"stiel39"

//--------------------------------------------------------------------------------------------------------
//
// Weapon IDs for all RnL Game weapons
//
typedef enum eRnLWeaponID
{
	WEAPON_NONE = 0,
	WEAPON_BAR,
	WEAPON_BROWNING,
	WEAPON_COLT,
	WEAPON_GREASEGUN,
	WEAPON_M1CARBINE,
	WEAPON_M1GARAND,
	WEAPON_SPRINGFIELD,
	WEAPON_THOMPSON,
	WEAPON_M3KNIFE,
	WEAPON_MK2GRENADE,
	WEAPON_M18GRENADE,
	WEAPON_STICKY24GRENADE,
	WEAPON_STICKY39GRENADE,
	WEAPON_STG44,
	WEAPON_WALTHER,
	WEAPON_K98K,
	WEAPON_MP40,
	WEAPON_MG34,
	WEAPON_MG42,
	WEAPON_TRENCHKNIFE,
	WEAPON_FG42,
	WEAPON_G43,
	WEAPON_K98SCOPED,
	WEAPON_NCOMAP,	// The map is being handled as weapon, so we can use some functions of the melee for it like range measurment - stefan
	WEAPON_DEVCAM,
	WEAPON_ALLIEDFISTS,
	WEAPON_AXISFISTS,
	WEAPON_ALLIEDEXPLOSIVES,
	WEAPON_AXISEXPLOSIVES,

	//Scripted Weapons
	WEAPON_SCRIPTED,

	WEAPON_MAX,		// number of weapons weapon index
} RnLWeaponID;

typedef enum
{
	Primary_Mode = 0,
	Secondary_Mode,
} RnLWeaponMode;

// Garand melee damage
#define GARANDMELEEDAMAGE 55.0f


enum recoilStates_s
{
	RECOIL_NORMAL,
	RECOIL_CROUCHED,
	RECOIL_PRONE,
	RECOIL_JUMPING,
	RECOIL_SIGHTED,
	RECOIL_MAX
};

enum reloadstate_s
{
	RELOAD_FULL = 0,	//Stores the reload state
	RELOAD_MID,
	RELOAD_EMPTY,
	RELOAD_INVALID
};

enum grenadestates_s
{
	GREN_IDLE = 0,	//Stores the grenade state, like pin pulled, let the spoon go and so on
	GREN_PIN_PULLED,
	GREN_SPOON,
	GREN_THROWN,
	GREN_INVALID
};

enum eWeaponPostures_t
{
	WEAPON_POSTURE_HIP = 0,
	WEAPON_POSTURE_SHOULDER,
	WEAPON_POSTURE_IRONSIGHTS,
	WEAPON_POSTURE_SUPERSIGHTS,
	WEAPON_POSTURE_THROWING_GRENADE,
	WEAPON_POSTURE_MAX,
};

enum eAimLockStates_t
{
	AIM_LOCK_NONE = 0,
	AIM_LOCK_SLOWDOWN,
	AIM_LOCK_LIMIT,
	AIM_LOCK_MAX,
};

enum weaponAnimations_s
{
	//Bog Standard Stuffs!
	WEAPON_ANIMATION_ATTACK = 0,
	WEAPON_ANIMATION_ATTACK2,
	WEAPON_ANIMATION_RELOAD,
	WEAPON_ANIMATION_DRAW,
	WEAPON_ANIMATION_HOLSTER,
	WEAPON_ANIMATION_IDLE,
	WEAPON_ANIMATION_SHOULDER_TO_IS,
	WEAPON_ANIMATION_IS_TO_SHOULDER,
	WEAPON_ANIMATION_SHOULDER_TO_HIP,
	WEAPON_ANIMATION_HIP_TO_SHOULDER,
	
	//Sprinting
	WEAPON_ANIMATION_SPRINT_ENTER,
	WEAPON_ANIMATION_SPRINT_IDLE,
	WEAPON_ANIMATION_SPRINT_LEAVE,

	//Proning
	WEAPON_ANIMATION_PRONE_ENTER,
	WEAPON_ANIMATION_PRONE_IDLE,
	WEAPON_ANIMATION_PRONE_LEAVE,

	//Rifles with Bayos
	WEAPON_ANIMATION_ATTACH_BAYO,
	WEAPON_ANIMATION_DETACH_BAYO,

	//K98
	WEAPON_ANIMATION_K98_BOLTCYCLE,

	//Machine Guns
	WEAPON_ANIMATION_DEPLOY,
	WEAPON_ANIMATION_UNDEPLOY,
};

//Class stuff
// All our current classes
enum eRnLClass
{
	RNL_CLASS_INVALID = 0,
	RNL_CLASS_INFANTRY,
	RNL_CLASS_SUPPORT,
	RNL_CLASS_MAX,
};

#define RNL_KIT_INVALID -1
#define RNL_SQUAD_INVALID -1

#define RNL_KITS_MAX 10
#define RNL_SQUADS_MAX 8
#define RNL_KITS_PER_SQUAD_MAX 5

#define KIT_DESC_TITLE_LEN 32
#define KIT_DESC_MODEL_LEN 64

class CRnLLoadoutModelBodyGroupInfo
{
public:
	DECLARE_CLASS_NOBASE(CRnLLoadoutModelBodyGroupInfo);
	DECLARE_EMBEDDED_NETWORKVAR();

	CRnLLoadoutModelBodyGroupInfo(){ iVal = 0;}

	CNetworkString(groupName, KIT_DESC_MODEL_LEN);
	CNetworkVar(int, iVal);
};


class CRnLLoadoutModelInfo
{
public:
	DECLARE_CLASS_NOBASE(CRnLLoadoutModelInfo);
	DECLARE_EMBEDDED_NETWORKVAR();

	CRnLLoadoutModelInfo(){ iSkin = 0;}

	CNetworkString(file, KIT_DESC_MODEL_LEN);
	CNetworkVar(int, iSkin);
	CUtlVector<CRnLLoadoutModelBodyGroupInfo> vecBodyGroups;
};

class RnLLoadoutKitInfo
{
public:
	DECLARE_CLASS_NOBASE(RnLLoadoutKitInfo);
	DECLARE_EMBEDDED_NETWORKVAR();

	RnLLoadoutKitInfo() { iKitId = -1; }

	CNetworkVar(int, iKitId);
	CNetworkVar(int, iClass);
	CNetworkString(title, KIT_DESC_TITLE_LEN);
	CNetworkString(name, KIT_DESC_TITLE_LEN);
	// TODO: Remove when squad leader voting is finished
	CNetworkVar(bool, bSquadLeader);
	CNetworkVarEmbedded(CRnLLoadoutModelInfo, model);
	CUtlVector<int> weapons;
};

EXTERN_PROP_TABLE(DT_RnLLoadoutKitInfo);

//defs for the allied loadout panel
enum {
	CLASSLOADOUT_RIFLEMAN = 0,
	CLASSLOADOUT_MACHINEGUNNER,
	CLASSLOADOUT_RADIOMAN,
	CLASSLOADOUT_MEDIC,
	CLASSLOADOUT_NCO,
	CLASSLOADOUT_MAX,
};

//highest attachment suffix
#define MAX_EQUIPMENT_WAIST 3
#define MAX_EQUIPMENT_BELT 0
#define MAX_EQUIPMENT_HEAD 1

// ---------------------------
//  Hit Group standards
// ---------------------------
enum eRnLHitGroups
{
	HITGROUP_RNL_GENERIC = (HITGROUP_GEAR+1),
	HITGROUP_RNL_HEAD,
	HITGROUP_RNL_NECK,
	HITGROUP_RNL_UPPER_ARM,
	HITGROUP_RNL_LOWER_ARM,
	HITGROUP_RNL_HAND,
	HITGROUP_RNL_CHEST,
	HITGROUP_RNL_STOMACH,
	HITGROUP_RNL_WAIST,
	HITGROUP_RNL_UPPER_LEG,
	HITGROUP_RNL_LOWER_LEG,
	HITGROUP_RNL_FOOT,

	HITGROUP_RNL_EQUIPMENT,

	HITGROUP_RNL_MAX,
};

typedef struct sprintSoundInfo_t
{
	float fTime;
	char* szName;
}SprintSoundInfo;

static SprintSoundInfo g_sSprintSoundInfo[] = 
{
	{ 2.5, "Player.Sprint_Stage0" }, { 2.5, "Player.Sprint_Stage1" },
	{ 2.5, "Player.Sprint_Stage2" }, { 2.5, "Player.Sprint_Stage3" },
	{ 2.5, "Player.Sprint_Stage4" }, { 2.5, "Player.Sprint_Stage5" },
	{ 2.5, "Player.Sprint_Stage6" }, { 2.5, "Player.Sprint_Stage7" },
	{ 3.5, "Player.Sprint.OutofBreath_Stage0" }, { 2.5, "Player.Sprint.OutofBreath_Stage1" },
	{ 2.5, "Player.Sprint.OutofBreath_Stage2" }, { 3.5, "Player.Sprint.OutofBreath_Stage3" },
	{ 3.5, "Player.Sprint.OutofBreath_Stage4" }, { 3.5, "Player.Sprint.OutofBreath_Stage5" },
	{ 3.5, "Player.HeavyBreathing" }
};

enum
{
	HL2COLLISION_GROUP_CROW = LAST_SHARED_COLLISION_GROUP,
};

// RnL Specific team numbering.
enum RnLTeams
{
	TEAM_ALLIES = FIRST_GAME_TEAM,
	TEAM_AXIS,

	TEAMS_COUNT
};

// Remap some of the existing inputs that aren't being used,
//   to do things WE want to do instead :)
#define IN_IRONSIGHTS	IN_ZOOM
#define IN_LEAN_RIGHT	IN_ALT1
#define IN_LEAN_LEFT	IN_ALT2
#define IN_PRONE		IN_DUCK
#define IN_MAP			IN_GRENADE2

enum RnLMoveType
{
	MOVETYPE_CLIMBING = MOVETYPE_LAST + 1,
};

enum RnLEquipmentTypes_t
{
	RNL_EQUIPMENT_ANY = -1,
	RNL_EQUIPMENT_PARACHUTE,
	RNL_EQUIPMENT_RADIO,
};


enum RnLMovementPostures_t
{
	//RnL : MovementMod : These define all the movement types
	MOVEMENT_POSTURE_NONE = 0,
	//Base Movements
	MOVEMENT_POSTURE_STAND,
	MOVEMENT_POSTURE_CROUCH,
	MOVEMENT_POSTURE_CROUCH_TOGGLE,
	MOVEMENT_POSTURE_PRONE,
	//Transitions
	MOVEMENT_POSTURE_STAND_TO_CROUCH,
	MOVEMENT_POSTURE_STAND_TO_CROUCH_TOGGLE,
	MOVEMENT_POSTURE_STAND_TO_PRONE,
	MOVEMENT_POSTURE_CROUCH_TO_STAND,
	MOVEMENT_POSTURE_CROUCH_TO_PRONE,
	MOVEMENT_POSTURE_PRONE_TO_CROUCH,
	MOVEMENT_POSTURE_PRONE_TO_CROUCH_TOGGLE,
	MOVEMENT_POSTURE_PRONE_TO_STAND,
	MOVEMENT_POSTURE_PRONE_DIVE,
	MOVEMENT_POSTURE_LAND_TO_CROUCH,
	MOVEMENT_POSTURE_LAND_TO_PRONE,

	//Others
	MOVEMENT_POSTURE_PARACHUTING,
	MOVEMENT_POSTURE_CLIMBING,
	MOVEMENT_POSTURE_CLIMBING_TO_CROUCH,
	MOVEMENT_POSTURE_DEPLOYED,
	MOVEMENT_POSTURE_DEPLOYED_CROUCH,
	MOVEMENT_POSTURE_DEPLOYED_PRONE,

	MOVEMENT_POSTURE_PRONE_ROLL_LEFT,
	MOVEMENT_POSTURE_PRONE_ROLL_RIGHT,

	// Wounded states
	MOVEMENT_POSTURE_KNOCKDOWN,
	MOVEMENT_POSTURE_MORTAL_WOUND,

	MOVEMENT_POSTURE_MAX,
	//RnL : MovementMod : Done
};

//RnL : Andrew : Unused decal
#define CHAR_TEX_HEDGEROW		'R'

extern ConVar	sv_speed_walk;
extern ConVar	sv_speed_run;
extern ConVar	sv_speed_sprint;
extern ConVar	sv_speed_crouchwalk;
extern ConVar	sv_speed_crouchrun;
extern ConVar	sv_speed_crouchsprint;
extern ConVar	sv_speed_pronewalk;
extern ConVar	sv_speed_pronerun;
extern ConVar	sv_speed_recovery;
extern ConVar	sv_speed_para;

#define SPEED_NONE 0
#define SPEED_WALK sv_speed_walk.GetFloat()
#define SPEED_RUN sv_speed_run.GetFloat()
#define SPEED_SPRINT sv_speed_sprint.GetFloat()
#define SPEED_CROUCH_WALK sv_speed_crouchwalk.GetFloat()
#define SPEED_CROUCH_RUN sv_speed_crouchrun.GetFloat()
#define SPEED_CROUCH_SPRINT sv_speed_crouchsprint.GetFloat()
#define SPEED_PRONE_WALK sv_speed_pronewalk.GetFloat()
#define SPEED_PRONE_RUN sv_speed_pronerun.GetFloat()
#define SPEED_RECOVER sv_speed_recovery.GetFloat()
#define SPEED_PARA sv_speed_para.GetFloat()

extern ConVar	sv_stamina_recover;
extern ConVar	sv_stamina_recover_walk;
extern ConVar	sv_stamina_recover_crouchwalk;
extern ConVar	sv_stamina_recover_crouchrun;
extern ConVar	sv_stamina_recover_pronewalk;
extern ConVar	sv_stamina_recover_pronerun;

#define STAMINA_RECOVER sv_stamina_recover.GetFloat()
#define STAMINA_RECOVER_WALK sv_stamina_recover_walk.GetFloat()
#define STAMINA_RECOVER_CROUCH_WALK sv_stamina_recover_crouchwalk.GetFloat()
#define STAMINA_RECOVER_CROUCH_RUN sv_stamina_recover_crouchrun.GetFloat()
#define STAMINA_RECOVER_PRONE_WALK sv_stamina_recover_pronewalk.GetFloat()
#define STAMINA_RECOVER_PRONE_RUN sv_stamina_recover_pronerun.GetFloat()

extern ConVar	sv_stamina_sprint;
extern ConVar	sv_stamina_crouchsprint;
extern ConVar	sv_stamina_pronesprint;

#define STAMINA_SPRINT sv_stamina_sprint.GetFloat()
#define STAMINA_CROUCH_SPRINT sv_stamina_crouchsprint.GetFloat()
#define STAMINA_PRONE_SPRINT sv_stamina_pronesprint.GetFloat()


// Each mod defines these for itself.
class CRnLViewVectors : public CViewVectors
{
public:
	CRnLViewVectors();

	CRnLViewVectors(

		Vector vView,
		Vector vHullMin,
		Vector vHullMax,

		Vector vDuckView,
		Vector vDuckWalkView,
		Vector vDuckHullMin,
		Vector vDuckHullMax,

		Vector vProneView,
		Vector vProneHullMin,
		Vector vProneHullMax,

		Vector vObsViewHeight,
		Vector vObsHullMin,
		Vector vObsHullMax
	);

	// Height above entity position where the viewer's eye is.
	Vector m_vDuckWalkView;

	Vector m_vProneView;
	Vector m_vProneHullMin;
	Vector m_vProneHullMax;
};

#define VEC_DUCK_VIEW_MOVING ((const CRnLViewVectors*)(g_pGameRules->GetViewVectors()))->m_vDuckWalkView

#define VEC_PRONE_VIEW		((const CRnLViewVectors*)(g_pGameRules->GetViewVectors()))->m_vProneView
#define VEC_PRONE_HULL_MIN	((const CRnLViewVectors*)(g_pGameRules->GetViewVectors()))->m_vProneHullMin
#define VEC_PRONE_HULL_MAX	((const CRnLViewVectors*)(g_pGameRules->GetViewVectors()))->m_vProneHullMax

#define PANEL_SQUAD_ALLIES	"squad_allies"
#define PANEL_SQUAD_AXIS	"squad_axis"
#define PANEL_SQUAD_SELECT	"squad_select"	// cjd @add
#define PANEL_LOADOUT		"loadout"
#define PANEL_LOADOUT_ROOM	"loadoutroom"	// cjd @add
#define PANEL_APPEARANCE	"appearance"	// cjd @add
#define PANEL_MAP			"map"
#define PANEL_RADIAL		"radial"

//RnL; Andrew
enum RnLShellTypes
{
	RNL_SHELL = 0,
	RNL_SHELL_PISTOL,
	RNL_SHELL_RIFLE,
	RNL_SHELL_MG,
};
//RnL;

#define COLOR_DARKRED	Color(136, 5, 5, 255)
#define COLOR_DARKGREEN	Color(0, 80, 0, 255)

#endif // RNL_SHAREDDEFS_H
