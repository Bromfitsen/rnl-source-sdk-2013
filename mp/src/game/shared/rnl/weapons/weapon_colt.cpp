//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasepistol.h"

#if defined( CLIENT_DLL )
	#define CWeaponColt C_WeaponColt
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

ConVar colt_ironsightsx("colt_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar colt_ironsightsy("colt_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar colt_ironsightsz("colt_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar colt_shoulderx("colt_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar colt_shouldery("colt_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar colt_shoulderz("colt_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponColt : public CWeaponRnLBasePistol
{
public:
	DECLARE_CLASS( CWeaponColt, CWeaponRnLBasePistol );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponColt();
	void Precache();
	bool PlayLastBulletSound();

	void ItemPostFrame();
	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_COLT; }
#ifdef CLIENT_DLL
	Vector	GetIronsightsOffset(){ return Vector( colt_ironsightsx.GetFloat(), colt_ironsightsy.GetFloat(), colt_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset(){ return Vector( colt_shoulderx.GetFloat(), colt_shouldery.GetFloat(), colt_shoulderz.GetFloat() ); }
#endif

private:

	CWeaponColt( const CWeaponColt & );

	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponColt, DT_WeaponColt )

BEGIN_NETWORK_TABLE( CWeaponColt, DT_WeaponColt )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponColt )
END_PREDICTION_DATA()

acttable_t CWeaponColt::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_PISTOL,			false },

	{ ACT_IDLE,				ACT_DOD_STAND_AIM_PISTOL,			false },
	{ ACT_CROUCHIDLE,		ACT_DOD_CROUCH_AIM_PISTOL,			false },
	{ ACT_RUN_CROUCH,		ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_WALK_CROUCH,		ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_PISTOL,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_PISTOL,			false },
	{ ACT_RUN,				ACT_DOD_RUN_AIM_PISTOL,				false },
	{ ACT_WALK,				ACT_DOD_WALK_AIM_PISTOL,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_PISTOL,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_PISTOL,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_PISTOL,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_PISTOL,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_PISTOL,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_PISTOL,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_PISTOL,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_PISTOL,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_PISTOL,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_PISTOL,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_PISTOL,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_PISTOL,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_PISTOL,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_PISTOL, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_PISTOL,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_PISTOL,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_PISTOL,	false },

};

IMPLEMENT_ACTTABLE(CWeaponColt);

LINK_ENTITY_TO_CLASS( weapon_colt, CWeaponColt );

PRECACHE_WEAPON_REGISTER( weapon_colt );

CWeaponColt::CWeaponColt()
{

}

void CWeaponColt::Precache()
{
	PrecacheScriptSound( "Weapon_Colt1911.FireLast" );

	BaseClass::Precache();
}

bool CWeaponColt::PlayLastBulletSound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Weapon_Colt1911.FireLast" );

	return 0;
}

//=====================================================================================// 
// CWeaponColt::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
 
void CWeaponColt::ItemPostFrame()
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
		if (pPlayer->m_afButtonPressed & IN_RELOAD  )
		{
			if( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				ReloadTransition( 1 );
			}
		}

		WeaponIdle();
	}
}