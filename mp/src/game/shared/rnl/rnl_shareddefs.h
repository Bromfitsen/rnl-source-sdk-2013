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

// After this many seconds, the grenade will be thrown instead of rolled
#define RNL_GRENADE_THROW_DELAY 0.25f

#define SDK_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )

#define LOWEST_CONTROLLED_CLIMB 40

#define RNL_SQUAD_SLOTS_MAX 10

#define RNL_VOICE_DISTANCE	1000.0f	// cjd @add

#define RNL_DEFAULT_PLAYER_MODEL "models/gman.mdl"

const char *WeaponIDToAlias( int id );
int AliasToWeaponID( const char *alias );

const char *TeamNumberToName( int id );
int TeamNameToNumber( const char *name );

typedef struct RnLBulletInfo_t
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
}RnLBulletInfo;

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

#define RNL_SQUAD_SLOTS_MAX 10

#define KIT_DESC_TITLE_LEN 32
#define KIT_DESC_MODEL_LEN 64

typedef struct bodygroupInfo_t
{
	bodygroupInfo_t(){ iVal = 0;}

	CUtlString groupName;
	int	iVal;
}RnLBodyGroupInfo;

typedef struct modelInfo_t
{
	modelInfo_t(){ iSkin = 0;}

	char		file[KIT_DESC_MODEL_LEN];
	int			iSkin;
	CUtlVector<RnLBodyGroupInfo> vecBodyGroups;
}RnLModelInfo;

typedef struct kitDescription_t
{
	kitDescription_t(){ iClass = RNL_CLASS_INVALID; }

	char		title[KIT_DESC_TITLE_LEN];
	char		name[KIT_DESC_TITLE_LEN];
	int			iClass;
	// TODO: Remove when squad leader voting is finished
	bool		bSquadLeader;
	RnLModelInfo model;
	CUtlVector<CUtlString> weapons;

}RnLKitDescription;

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

#endif // RNL_SHAREDDEFS_H
