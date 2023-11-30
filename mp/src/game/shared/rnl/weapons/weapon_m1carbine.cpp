//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbaserifle.h"

#if defined( CLIENT_DLL )
	#define CWeaponM1Carbine C_WeaponM1Carbine
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
	#include "smoke_trail.h"
	#include "explode.h"
#endif

class CWeaponM1Carbine : public CWeaponRnLBaseRifle
{
public:
	DECLARE_CLASS( CWeaponM1Carbine, CWeaponRnLBaseRifle );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponM1Carbine();
	~CWeaponM1Carbine() = default;

	void ItemPostFrame() OVERRIDE;
	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_M1CARBINE; }

private:

	CWeaponM1Carbine( const CWeaponM1Carbine & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM1Carbine, DT_WeaponM1Carbine )

BEGIN_NETWORK_TABLE( CWeaponM1Carbine, DT_WeaponM1Carbine )
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC( CWeaponM1Carbine )
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA(CWeaponM1Carbine)
	END_PREDICTION_DATA()
#endif

acttable_t CWeaponM1Carbine::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_RIFLE,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_RIFLE,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_RIFLE,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_RIFLE,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_RIFLE,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_RIFLE,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_RIFLE,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_RIFLE,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_RIFLE,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_RIFLE,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_RIFLE,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_RIFLE,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_RIFLE,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_RIFLE,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_RIFLE, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_RIFLE,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_RIFLE,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_RIFLE,	false },
};

IMPLEMENT_ACTTABLE(CWeaponM1Carbine);

LINK_ENTITY_TO_CLASS( weapon_carbine, CWeaponM1Carbine );
PRECACHE_WEAPON_REGISTER( weapon_carbine );

CWeaponM1Carbine::CWeaponM1Carbine()
{

}

//=====================================================================================// 
// CWeaponM1Carbine::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
 
void CWeaponM1Carbine::ItemPostFrame()
{
    // Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) return; 

	HandleFoVTransitions();

	if( HandleReloadTransitions() )
	{
		return;
	}
	else if( ShouldHandleWeaponLimitations() )
	{
		WeaponIdle();
		return;
	}
	else if( HandlePostureTransitions() )
	{
		WeaponIdle();
		return;
	}
	// if it's not, check what else has been requested
	else
	{
		//----------------------------------------------//
		// Did the player press the primary attack button in this frame?
		//----------------------------------------------//
		if ( pPlayer->m_afButtonPressed & IN_ATTACK )
		{
			if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				PrimaryAttack(); 
			}
		}

		// Reload
		if (pPlayer->m_nButtons & IN_RELOAD  )
		{
			if( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				ReloadTransition( 1 );
			}
		}

		WeaponIdle();
	}
}