//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		The NCO map
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "weapon_rnl_base.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
	#include "iinput.h"	
#else
	#include "rnl_gamerules.h"
	#include "rnl_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponDevCam C_WeaponDevCam
#endif

//-----------------------------------------------------------------------------
// CWeaponDevCam
//-----------------------------------------------------------------------------

class CWeaponDevCam : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CWeaponDevCam, CWeaponRnLBase );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponDevCam();

	bool		Deploy();
	bool		Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	
	void		ItemPostFrame( void );

	bool		StartSprinting( void ) { return m_bHeadBob; }

	bool		AllowsAutoSwitchTo() { return false; }

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_DEVCAM; }

	CNetworkVar( bool, m_bHeadBob );

	CWeaponDevCam( const CWeaponDevCam & );		
};

//-----------------------------------------------------------------------------
// CWeaponDevCam
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDevCam, DT_WeaponDevCam )

BEGIN_NETWORK_TABLE( CWeaponDevCam, DT_WeaponDevCam )
#if !defined( CLIENT_DLL )
	SendPropBool( SENDINFO( m_bHeadBob ) ),
#else
	RecvPropBool( RECVINFO( m_bHeadBob ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponDevCam )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_bHeadBob, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_devcam, CWeaponDevCam );
PRECACHE_WEAPON_REGISTER( weapon_devcam );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponDevCam::CWeaponDevCam( void )
{
	m_bHeadBob = false;
}

//-----------------------------------------------------------------------------
// Purpose: Deploy the map model fire the deploy anim and show the map when that's done
//-----------------------------------------------------------------------------
bool CWeaponDevCam::Deploy()
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

#ifndef CLIENT_DLL
	pPlayer->ShowViewModel( false );
#endif

	pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );

	bool returnVal = BaseClass::Deploy();

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.10f;

	return returnVal;
}

//-----------------------------------------------------------------------------
// Purpose: Holster the map model fire the holster anim
// Send a "false" over to the client, so the map panel hides
//-----------------------------------------------------------------------------
bool CWeaponDevCam::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
#ifndef CLIENT_DLL
		pPlayer->ShowViewModel( true );
		pPlayer->SetFOV( pPlayer, 0, 0 );
#endif
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
	}

	return BaseClass::Holster();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponDevCam::ItemPostFrame( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) return; 

#ifndef CLIENT_DLL
	float currentFoV = pPlayer->GetFOV();
#endif

	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
		{
#ifndef CLIENT_DLL
			if( currentFoV < 160 )
			{
				currentFoV += 5;
				pPlayer->SetFOV( pPlayer, currentFoV, 0 );
			}
#endif
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.10f;
		}
	}
	// Attach the bayonett for now - Stefan
	if ( pPlayer->m_nButtons & IN_ATTACK2 )
	{
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
		{
#ifndef CLIENT_DLL
			if( currentFoV > 5 )
			{
				currentFoV -= 5;
				pPlayer->SetFOV( pPlayer, currentFoV, 0 );
			}
#endif
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.10f;
		}
	}

	/*
	if( pPlayer->m_nButtons & IN_WATCH )
	{
		if ( gpGlobals->curtime >= m_flNextSecondaryAttack )
		{
			m_bHeadBob = !m_bHeadBob;
			if( m_bHeadBob )
			{
				ClientPrint( pPlayer, HUD_PRINTCENTER, "Using head bob." );
			}
			else 
			{
				ClientPrint( pPlayer, HUD_PRINTCENTER, "Not using head bob" );
			}
			m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
		}
	}
	*/

}