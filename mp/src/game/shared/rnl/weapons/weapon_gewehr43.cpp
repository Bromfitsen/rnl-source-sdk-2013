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
	#define CWeaponGewehr43 C_WeaponGewehr43
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
	#include "smoke_trail.h"
	#include "explode.h"
#endif

class CWeaponGewehr43 : public CWeaponRnLBaseRifle
{
public:
	DECLARE_CLASS( CWeaponGewehr43, CWeaponRnLBaseRifle );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponGewehr43();
	void Precache();
	bool PlayLastBulletSound();

	void ItemPostFrame();
	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_G43; }

private:

	CWeaponGewehr43( const CWeaponGewehr43 & );

	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGewehr43, DT_WeaponGewehr43 )

BEGIN_NETWORK_TABLE( CWeaponGewehr43, DT_WeaponGewehr43 )
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC( CWeaponGewehr43 )
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA(CWeaponGewehr43)
	END_PREDICTION_DATA()
#endif

acttable_t CWeaponGewehr43::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponGewehr43);

LINK_ENTITY_TO_CLASS( weapon_gewehr43, CWeaponGewehr43 );
PRECACHE_WEAPON_REGISTER( weapon_gewehr43 );

CWeaponGewehr43::CWeaponGewehr43()
{

}

void CWeaponGewehr43::Precache()
{
	PrecacheScriptSound( "Weapon_G43.FireLast" );

	BaseClass::Precache();
}

bool CWeaponGewehr43::PlayLastBulletSound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Weapon_G43.FireLast" );

	return 0;
}

//=====================================================================================// 
// CWeaponGewehr43::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
 
void CWeaponGewehr43::ItemPostFrame()
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