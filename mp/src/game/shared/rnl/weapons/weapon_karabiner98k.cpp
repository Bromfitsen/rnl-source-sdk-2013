//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbaserifle.h"
#include "gamemovement.h"

#define M1GARAND_ROF 0.05f // semi automatic, very small ROF

#if defined( CLIENT_DLL )
	#define CWeaponKarabiner98k C_WeaponKarabiner98k
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

ConVar k98_ironsightsx("k98_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar k98_ironsightsy("k98_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar k98_ironsightsz("k98_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar k98_shoulderx("k98_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar k98_shouldery("k98_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar k98_shoulderz("k98_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponKarabiner98k : public CWeaponRnLBaseRifle
{
public:
	DECLARE_CLASS( CWeaponKarabiner98k, CWeaponRnLBaseRifle );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef SERVER_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponKarabiner98k();
	
	void Precache( void );
	virtual int	GetWorldModelIndex( void );
	virtual const char *GetWorldModel( void );
	void PrimaryAttack();

	void ItemPostFrame();

	void StartCycleBolt();
	bool HandleCycleBolt();
	void CycleBolt();
	
	void HandleViewAnimation( int iAnim );

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_K98K; }

#ifdef CLIENT_DLL
	Vector	GetIronsightsOffset(){ return Vector( k98_ironsightsx.GetFloat(), k98_ironsightsy.GetFloat(), k98_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset(){ return Vector( k98_shoulderx.GetFloat(), k98_shouldery.GetFloat(), k98_shoulderz.GetFloat() ); }
#endif

private:
	// K98 specific
	CNetworkVar( bool, m_bCycleBolt );
	//OH OH It's Sir Hacks-a-lot - this just tells us if the bolt cycle needs
	//	to go back to ironsights
	CNetworkVar( int, m_iCycleTransition );
	

	CWeaponKarabiner98k( const CWeaponKarabiner98k & );
	CNetworkVar( int, m_iDeployedModelIndex );

	void Fire( float flSpread );
};

const char *pK98kWithBayonetModel = "models/weapons/w_k98_bayonet.mdl";

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponKarabiner98k, DT_WeaponKarabiner98k );

BEGIN_NETWORK_TABLE( CWeaponKarabiner98k, DT_WeaponKarabiner98k )
#ifndef CLIENT_DLL
	SendPropModelIndex( SENDINFO(m_iDeployedModelIndex) ),
	// K98 specific
	SendPropBool( SENDINFO( m_bCycleBolt ) ),
	SendPropInt( SENDINFO( m_iCycleTransition ) ),
#else
	RecvPropInt( RECVINFO(m_iDeployedModelIndex)),
	RecvPropBool( RECVINFO( m_bCycleBolt ) ),
	RecvPropInt( RECVINFO( m_iCycleTransition ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponKarabiner98k )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_iDeployedModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_bCycleBolt, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iCycleTransition, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

#ifdef SERVER_DLL
	BEGIN_DATADESC( CWeaponKarabiner98k )
	END_DATADESC()
#endif

acttable_t CWeaponKarabiner98k::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_BOLT,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_BOLT,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_BOLT,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_BOLT,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_BOLT,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_BOLT,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_BOLT,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_BOLT,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_BOLT,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_BOLT,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_BOLT,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_BOLT,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_BOLT,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_BOLT,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_BOLT, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_BOLT,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_BOLT,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_BOLT,	false },
};

IMPLEMENT_ACTTABLE(CWeaponKarabiner98k);

LINK_ENTITY_TO_CLASS( weapon_k98k, CWeaponKarabiner98k );
PRECACHE_WEAPON_REGISTER( weapon_k98k );

CWeaponKarabiner98k::CWeaponKarabiner98k()
{
	m_bCycleBolt = false;
}

void CWeaponKarabiner98k::Precache()
{
	BaseClass::Precache();

	m_iDeployedModelIndex	= CBaseEntity::PrecacheModel( pK98kWithBayonetModel );
}

int	CWeaponKarabiner98k::GetWorldModelIndex( void )
{
	if( IsBayonetDeployed() )
	{
		return m_iDeployedModelIndex;
	}
	else
	{
		return m_iWorldModelIndex;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CWeaponKarabiner98k::GetWorldModel( void )
{
	if( IsBayonetDeployed() )
	{
		return pK98kWithBayonetModel;
	}
	else
	{
		return BaseClass::GetWorldModel();
	}
}

void CWeaponKarabiner98k::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();

	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer && pPlayer->GetWaterLevel() == 3 )
	{
		return;
	}

	if( Clip1() > 0 )
	{
		m_bCycleBolt = true;
	}
}

//=====================================================================================// 
// CWeaponKarabiner98k::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
 
void CWeaponKarabiner98k::ItemPostFrame()
{
    // Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return; 

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
	else if( HandleBayonetTransition() )
	{
		return;
	}
	else if( HandleCycleBolt() )
	{
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
		if ( pPlayer->m_nButtons & IN_ATTACK )
		{
			if ( gpGlobals->curtime >= m_flNextPrimaryAttack && !m_bCycleBolt )
			{
				PrimaryAttack(); 
			}
			else if ( gpGlobals->curtime >= m_flNextPrimaryAttack && m_bCycleBolt )
			{
				StartCycleBolt();
			}
		}
		else if( m_bCycleBolt && pPlayer->m_RnLLocal.m_bUseAutobolt )
		{
			if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				StartCycleBolt();
			}
		}

		// Attach the bayonett for now - Stefan
		if ( pPlayer->m_nButtons & IN_ATTACK2 ) 
		{
			if ( gpGlobals->curtime >= m_flNextSecondaryAttack )
			{
				SecondaryAttack( true );
			}
		}

		// Reload
		if (pPlayer->m_nButtons & IN_RELOAD )
		{
			if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				if( Clip1() < 1 )
				{
					ReloadTransition( 1 );
				}
				else
				{
					if( !m_bCycleBolt )
						m_iClip1--;
					StartCycleBolt();
				}
			}
		}		
	
		// Attach the bayonett
		if (pPlayer->m_nButtons & IN_ATTACH )
		{
			if( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				BayonetTransition( 1 );
			}
		}

		WeaponIdle();
	}
}

void CWeaponKarabiner98k::CycleBolt()
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return;

	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	// No need to cycle when there's no bullets left
	if( bMagEmpty )
		return;

	HandleViewAnimation( WEAPON_ANIMATION_K98_BOLTCYCLE );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	// Reset
	m_bCycleBolt = false;

#ifdef GAME_DLL
	//CRnLPlayer *pPlayer = dynamic_cast< CRnLPlayer* >( GetOwner() );
	//if ( !pPlayer )
	//	return;

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BOLTCYCLE );
#endif
}

void CWeaponKarabiner98k::StartCycleBolt()
{
	m_iCycleTransition = 1;
}

bool CWeaponKarabiner98k::HandleCycleBolt()
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return false;

	int iPosture = pPlayer->GetWeaponPosture();

	if( m_iCycleTransition != 0 )
	{
		bool canDoActivity = (gpGlobals->curtime >= m_flNextPrimaryAttack) ? true : false;

		if( m_iCycleTransition == 1 && canDoActivity )
		{
			if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				if( iPosture == WEAPON_POSTURE_SUPERSIGHTS )
					PrevFirePos();

				HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			m_iCycleTransition = 2;
		}
		else if( m_iCycleTransition == 2 && canDoActivity )
		{
			CycleBolt();
			m_iCycleTransition = 3;
		}
		else if( m_iCycleTransition == 3 && canDoActivity )
		{
			if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			m_iCycleTransition = 0;
		}

		return true;
	}

	return false;
}

void CWeaponKarabiner98k::HandleViewAnimation( int iAnim )
{
	if( iAnim == WEAPON_ANIMATION_K98_BOLTCYCLE )
	{
		SetAnimationState( iAnim );

		if( IsBayonetDeployed() )
			SendWeaponAnim( ACT_VM_CYCLEBOLT_DEPLOYED );
		else
			SendWeaponAnim( ACT_VM_CYCLEBOLT );
	}
	else
		BaseClass::HandleViewAnimation( iAnim );
}