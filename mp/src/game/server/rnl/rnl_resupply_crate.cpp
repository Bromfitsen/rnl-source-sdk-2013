//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: RnL resupply crate
//
//=============================================================================//

#include "cbase.h"
#include "rnl_player.h"
#include "rnl_gamerules.h"
#include "weapon_rnlballisticbase.h"
#include "rnl_ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Ammo crate
class CRnLResupplyCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS( CRnLResupplyCrate, CBaseAnimating );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	InputNeutral( inputdata_t &data );
	void	InputAlliedCapture( inputdata_t &data );
	void	InputAxisCapture( inputdata_t &data );

	COutputEvent	m_OnUsed;

protected:

	int m_iInitialTeam;
	float m_fResupplyDelay;
	float m_fPlayerResupplyTime[MAX_PLAYERS+1];
};

LINK_ENTITY_TO_CLASS( rnl_resupply_crate, CRnLResupplyCrate );

BEGIN_DATADESC( CRnLResupplyCrate )

	DEFINE_KEYFIELD( m_iInitialTeam,	FIELD_INTEGER, "InitialTeam" ),	
	DEFINE_KEYFIELD( m_fResupplyDelay,  FIELD_FLOAT, "ResupplyDelay" ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),
	DEFINE_INPUTFUNC( FIELD_VOID, "NeutralControlled", InputNeutral ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AlliedControlled", InputAlliedCapture ),
	DEFINE_INPUTFUNC( FIELD_VOID, "AxisControlled", InputAxisCapture ),

END_DATADESC()

// This table encodes the CBaseEntity data.
IMPLEMENT_SERVERCLASS_ST(CRnLResupplyCrate, DT_RnLResupplyCrate)
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	int bodygroup = FindBodygroupByName( "team" );
	if( bodygroup > -1 )
	{
		if( m_iInitialTeam == TEAM_AXIS )
			SetBodygroup( bodygroup, 2 );
		else if( m_iInitialTeam == TEAM_ALLIES )
			SetBodygroup( bodygroup, 1 );
		else
		{
			SetBodygroup( bodygroup, 0 );
			m_iInitialTeam = TEAM_NONE;
		}
	}
	ChangeTeam( m_iInitialTeam );

	RemoveEffects( EF_NODRAW );

	//ResetSequence( ACT_IDLE );
	//SetCycle( 0 );
	UseClientSideAnimation();

	memset( m_fPlayerResupplyTime, 0, sizeof(float) * (MAX_PLAYERS+1) );

	m_takedamage = DAMAGE_NO;

}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CRnLResupplyCrate::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	if( pPlayer->GetTeamNumber() != GetTeamNumber() )
		return;

	if( pPlayer->IsDeployed() )
		return;

	if( m_fPlayerResupplyTime[pPlayer->entindex()] > gpGlobals->curtime )
		return;

	m_OnUsed.FireOutput( pActivator, this );

	m_fPlayerResupplyTime[pPlayer->entindex()] = gpGlobals->curtime + m_fResupplyDelay;

	CWeaponRnLBase* pWeapon = NULL;
	for( int i = 0; i < pPlayer->WeaponCount(); i++ )
	{
		pWeapon = dynamic_cast<CWeaponRnLBase*>(pPlayer->GetWeapon( i ));

		if( pWeapon && !pWeapon->IsGrenade() )
		{
			pPlayer->GiveAmmo( GetRnLAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType()) - pPlayer->GetAmmoCount( pWeapon->GetPrimaryAmmoType()), pWeapon->GetPrimaryAmmoType() );
			if( pWeapon == pPlayer->GetActiveRnLWeapon() )
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
				pWeapon->HandleViewAnimation( WEAPON_ANIMATION_RELOAD );
			}

			pWeapon->m_iClip1 = pWeapon->GetMaxClip1();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::InputAlliedCapture( inputdata_t &data )
{
	if( GetTeamNumber() != TEAM_ALLIES )
	{
		memset( m_fPlayerResupplyTime, 0, sizeof(float) * (MAX_PLAYERS+1) );
		ChangeTeam( TEAM_ALLIES );

		int bodygroup = FindBodygroupByName( "team" );
		if( bodygroup > -1 )
			SetBodygroup( bodygroup, 1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::InputAxisCapture( inputdata_t &data )
{
	if( GetTeamNumber() != TEAM_AXIS )
	{
		memset( m_fPlayerResupplyTime, 0, sizeof(float) * (MAX_PLAYERS+1) );
		ChangeTeam( TEAM_AXIS );
		int bodygroup = FindBodygroupByName( "team" );
		if( bodygroup > -1 )
			SetBodygroup( bodygroup, 2 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CRnLResupplyCrate::InputNeutral( inputdata_t &data )
{
	if( GetTeamNumber() != TEAM_NONE )
	{
		memset( m_fPlayerResupplyTime, 0, sizeof(float) * (MAX_PLAYERS+1) );
		ChangeTeam( TEAM_NONE );
		int bodygroup = FindBodygroupByName( "team" );
		if( bodygroup > -1 )
			SetBodygroup( bodygroup, 0 );
	}
}

