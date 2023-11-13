//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: ]=0)
//
//=====================================================================================//
//#include "player.h"

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasesubmachinegun.h"

#if defined( CLIENT_DLL )
	#define CWeaponMP40 C_WeaponMP40
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
#endif

ConVar mp40_ironsightsx("mp40_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar mp40_ironsightsy("mp40_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar mp40_ironsightsz("mp40_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar mp40_shoulderx("mp40_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar mp40_shouldery("mp40_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar mp40_shoulderz("mp40_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponMP40 : public CWeaponRnLBaseSubMachineGun
{
public:
	DECLARE_CLASS( CWeaponMP40, CWeaponRnLBaseSubMachineGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponMP40();

	void ItemPostFrame();

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_MP40; }

#ifdef CLIENT_DLL
	Vector	GetIronsightsOffset(){ return Vector( mp40_ironsightsx.GetFloat(), mp40_ironsightsy.GetFloat(), mp40_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset(){ return Vector( mp40_shoulderx.GetFloat(), mp40_shouldery.GetFloat(), mp40_shoulderz.GetFloat() ); }
#endif

private:

	CWeaponMP40( const CWeaponMP40 & );

	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP40, DT_WeaponMP40 )

BEGIN_NETWORK_TABLE( CWeaponMP40, DT_WeaponMP40 )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponMP40 )
END_PREDICTION_DATA()
#endif

acttable_t CWeaponMP40::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_MP40,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_MP40,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_MP40,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_MP40,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_MP40,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_MP40,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_MP40,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_MP40,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_MP40,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_MP40,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_MP40,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_MP40,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_MP40,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_MP40,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_MP40, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_MP40,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_MP40,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_MP40,	false },
};

IMPLEMENT_ACTTABLE(CWeaponMP40);

LINK_ENTITY_TO_CLASS( weapon_mp40, CWeaponMP40 );
PRECACHE_WEAPON_REGISTER( weapon_mp40 );

CWeaponMP40::CWeaponMP40()
{

}

//======================================================================================// 
//																						//
//							CWeaponMP40::ItempostFrame()							//
//	Purpose: Method called every frame. Check for buttons/keys pressed Checks reload	//
//							status and calls WeaponIdle()								//
//																						//
//======================================================================================//
 void CWeaponMP40::ItemPostFrame()
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
	bool canDoActivity = (gpGlobals->curtime >= m_flNextPrimaryAttack) ? true : false;
	
	if ( pPlayer->m_nButtons & IN_ATTACK && canDoActivity )
	{
		PrimaryAttack();
	}
	// Reload
	else if (pPlayer->m_nButtons & IN_RELOAD && canDoActivity )
	{ 
		ReloadTransition( 1 );
	}

	WeaponIdle();
}