//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasemelee.h"

#if defined( CLIENT_DLL )
	#define CWeaponAxisFists C_WeaponAxisFists
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
	#include "smoke_trail.h"
	#include "explode.h"
#endif

class CWeaponAxisFists : public CWeaponRnLBaseMelee
{
public:
	DECLARE_CLASS( CWeaponAxisFists, CWeaponRnLBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponAxisFists();
	~CWeaponAxisFists() = default;

	void ItemPostFrame() OVERRIDE;
	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_AXISFISTS; }

private:

	CWeaponAxisFists( const CWeaponAxisFists & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAxisFists, DT_WeaponAxisFists )

BEGIN_NETWORK_TABLE( CWeaponAxisFists, DT_WeaponAxisFists )
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC( CWeaponAxisFists )
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA(CWeaponAxisFists)
	END_PREDICTION_DATA()
#endif


acttable_t CWeaponAxisFists::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponAxisFists);

LINK_ENTITY_TO_CLASS( weapon_axisfists, CWeaponAxisFists );

PRECACHE_WEAPON_REGISTER( weapon_axisfists );

CWeaponAxisFists::CWeaponAxisFists()
{

}

//=====================================================================================// 
// CWeaponAxisFists::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
void CWeaponAxisFists::ItemPostFrame()
{
    // Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) return; 

	HandleFoVTransitions();

	if( ShouldHandleWeaponLimitations() )
	{
		WeaponIdle();
		return;
	}

	//----------------------------------------------//
	// Did the player press the primary attack button in this frame?
	//----------------------------------------------//
	if ( pPlayer->m_afButtonPressed & IN_ATTACK || pPlayer->m_nButtons & IN_ATTACK2 )
	{
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
		{
			PrimaryAttack(); 
		}
	}

	WeaponIdle();
}