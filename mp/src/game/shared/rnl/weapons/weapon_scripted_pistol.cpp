//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasepistol.h"
#include "rnl_weapon_parse.h"

#if defined( CLIENT_DLL )
	#define CWeaponColt C_WeaponColt
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
	#define CWeaponScriptedPistol C_WeaponScriptedPistol
#endif

class CWeaponScriptedPistol : public CWeaponRnLBasePistol
{
public:
	DECLARE_CLASS( CWeaponScriptedPistol, CWeaponRnLBasePistol );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponScriptedPistol();

	void ItemPostFrame();
	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_SCRIPTED; }

private:

	CWeaponScriptedPistol( const CWeaponScriptedPistol & );
};

acttable_t CWeaponScriptedPistol::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_PISTOL,		false },

	{ ACT_IDLE,				ACT_DOD_STAND_AIM_PISTOL,		false },
	{ ACT_CROUCHIDLE,		ACT_DOD_CROUCH_AIM_PISTOL,		false },
	{ ACT_RUN_CROUCH,		ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_WALK_CROUCH,		ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_PISTOL,	false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_PISTOL,		false },
	{ ACT_RUN,				ACT_DOD_RUN_AIM_PISTOL,			false },
	{ ACT_WALK,				ACT_DOD_WALK_AIM_PISTOL,		false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_PISTOL,		false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_PISTOL,		false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_PISTOL,		false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_PISTOL,		false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_PISTOL,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_PISTOL,	false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_PISTOL,		false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_PISTOL,			false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_PISTOL,		false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_PISTOL,		false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_PISTOL,		false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_PISTOL,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_PISTOL, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_PISTOL,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_PISTOL,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_PISTOL,	false },
};

IMPLEMENT_ACTTABLE(CWeaponScriptedPistol);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponScriptedPistol, DT_WeaponScriptedPistol )

BEGIN_NETWORK_TABLE( CWeaponScriptedPistol, DT_WeaponScriptedPistol )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponScriptedPistol )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_scripted_pistol, CWeaponScriptedPistol );
//PRECACHE_WEAPON_REGISTER( weapon_colt );
LINK_SCRIPTED_WEAPON_TO_TYPE( weapon_scripted_pistol, SCRIPTED_WEAPON_PISTOL );

CWeaponScriptedPistol::CWeaponScriptedPistol()
{
}

//=====================================================================================// 
// CWeaponScriptedPistol::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
void CWeaponScriptedPistol::ItemPostFrame()
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
	
	//----------------------------------------------//
	// Did the player press the primary attack button in this frame?
	//----------------------------------------------//
	if ( pPlayer->m_afButtonPressed & IN_ATTACK )
	{
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
				PrimaryAttack();
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