//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_PLAYERANIMSTATE_H
#define RNL_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "iplayeranimstate.h"
#include "base_playeranimstate.h"


#ifdef CLIENT_DLL
	class C_BaseAnimatingOverlay;
	class C_WeaponRnLBase;
	#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
	#define CWeaponRnLBase C_WeaponRnLBase
	#define CRnLPlayer C_RnLPlayer
#else
	class CBaseAnimatingOverlay;
	class CWeaponRnLBase; 
	class CRnLPlayer;
#endif


// When moving this fast, he plays run anim.
#define ARBITRARY_RUN_SPEED		80.0f
// When moving this fast, he plays sprint animaations
#define ARBITRARY_SPRINT_SPEED	170.0f
//When moving this fast a crouched player is running
#define ARBITRARY_CROUCH_RUN_SPEED		70.0f


enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY=0,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
	PLAYERANIMEVENT_THROW_GRENADE,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	// new rnl events
	PLAYERANIMEVENT_MIDRELOAD,
	PLAYERANIMEVENT_BOLTCYCLE,
	PLAYERANIMEVENT_PRONE,
	PLAYERANIMEVENT_PRONECANCEL,
	PLAYERANIMEVENT_PRONEGETDOWN,
	//Andrew : Climbing Mumbo Jumbo
	PLAYERANIMEVENT_CLIMB,
	PLAYERANIMEVENT_STARTPARACHUTING,
	PLAYERANIMEVENT_ENDPARACHUTING,
	PLAYERANIMEVENT_DEPLOY,
	PLAYERANIMEVENT_DRAW,
	PLAYERANIMEVENT_HOLSTER,

	PLAYERANIMEVENT_BAYONET_ON,
	PLAYERANIMEVENT_BAYONET_OFF,

	PLAYERANIMEVENT_ROLLLEFT,
	PLAYERANIMEVENT_ROLLRIGHT,

	PLAYERANIMEVENT_COUNT
};


class IRnLPlayerAnimState : virtual public IPlayerAnimState
{
public:
	// This is called by both the client and the server in the same way to trigger events for
	// players firing, jumping, throwing grenades, etc.
	virtual void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 ) = 0;
	
	// Returns true if we're playing the grenade prime or throw animation.
	virtual bool IsThrowingGrenade() = 0;
};


// This abstracts the differences between RnL players and hostages.
class IRnLPlayerAnimStateHelpers
{
public:
	virtual CWeaponRnLBase* RnLAnim_GetActiveWeapon() = 0;
	virtual bool CanMove() = 0;
};


IRnLPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IRnLPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

// If this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;


#endif // RnL_PLAYERANIMSTATE_H
