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
	#define CWeaponKarabiner98Scoped C_WeaponKarabiner98Scoped
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

class CWeaponKarabiner98Scoped : public CWeaponRnLBaseRifle
{
public:
	DECLARE_CLASS( CWeaponKarabiner98Scoped, CWeaponRnLBaseRifle );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef SERVER_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponKarabiner98Scoped();
	
	void PrimaryAttack();

	void ItemPostFrame();

	void StartCycleBolt();
	bool HandleCycleBolt();
	void CycleBolt();
	
	void HandleViewAnimation( int iAnim );

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_K98SCOPED; }

private:
	// K98 specific
	CNetworkVar( bool, m_bCycleBolt );
	//OH OH It's Sir Hacks-a-lot - this just tells us if the bolt cycle needs
	//	to go back to ironsights
	CNetworkVar( int, m_iCycleTransition );
	

	CWeaponKarabiner98Scoped( const CWeaponKarabiner98Scoped & );

	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponKarabiner98Scoped, DT_WeaponKarabiner98Scoped );

BEGIN_NETWORK_TABLE( CWeaponKarabiner98Scoped, DT_WeaponKarabiner98Scoped )
#if !defined( CLIENT_DLL )
	// K98 specific
	SendPropBool( SENDINFO( m_bCycleBolt ) ),
	SendPropInt( SENDINFO( m_iCycleTransition ) ),
#else
	RecvPropBool( RECVINFO( m_bCycleBolt ) ),
	RecvPropInt( RECVINFO( m_iCycleTransition ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponKarabiner98Scoped )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_bCycleBolt, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iCycleTransition, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

#ifdef SERVER_DLL
	BEGIN_DATADESC( CWeaponKarabiner98Scoped )
	END_DATADESC()
#endif

acttable_t CWeaponKarabiner98Scoped::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponKarabiner98Scoped);

LINK_ENTITY_TO_CLASS( weapon_k98scoped, CWeaponKarabiner98Scoped );
PRECACHE_WEAPON_REGISTER( weapon_k98scoped );

CWeaponKarabiner98Scoped::CWeaponKarabiner98Scoped()
{
	m_bCycleBolt = false;
}

void CWeaponKarabiner98Scoped::PrimaryAttack( void )
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
 
void CWeaponKarabiner98Scoped::ItemPostFrame()
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

		WeaponIdle();
	}
}

void CWeaponKarabiner98Scoped::CycleBolt()
{
	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	// No need to cycle when there's no bullets left
	if( bMagEmpty )
		return;

	HandleViewAnimation( WEAPON_ANIMATION_K98_BOLTCYCLE );

	// Reset
	m_bCycleBolt = false;
	
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

#ifdef GAME_DLL
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BOLTCYCLE );
#endif
}

void CWeaponKarabiner98Scoped::StartCycleBolt()
{
	m_iCycleTransition = 1;
}

bool CWeaponKarabiner98Scoped::HandleCycleBolt()
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

void CWeaponKarabiner98Scoped::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return; 
	int iPosture = pPlayer->GetWeaponPosture();

	SetAnimationState( iAnim );

	if( iAnim == WEAPON_ANIMATION_K98_BOLTCYCLE )
		SendWeaponAnim( ACT_VM_CYCLEBOLT );
	else if( m_bCycleBolt )
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_DRAW:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPDRAWOPEN );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERDRAWOPEN );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTDRAWOPEN );
			}
			break;

		case WEAPON_ANIMATION_HOLSTER:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPHOLSTEROPEN );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERHOLSTEROPEN );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTHOLSTEROPEN );
			}
			break;

		case WEAPON_ANIMATION_IDLE:
			if( GetPlayerOwner()  && GetPlayerOwner()->IsProne()
				&& GetPlayerOwner()->GetLocalVelocity().Length2D() > SPEED_PRONE_WALK  )
			{
				SendWeaponAnim( ACT_VM_PRONE_IDLE_OPEN );
				SetAnimationState( WEAPON_ANIMATION_PRONE_IDLE );
			}			
			else if( GetPlayerOwner() && GetPlayerOwner()->IsSprinting() )
			{
				SendWeaponAnim( ACT_VM_SPRINT_IDLE_OPEN );
				SetAnimationState( WEAPON_ANIMATION_SPRINT_IDLE );
			}
			else if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPIDLEOPEN );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERIDLEOPEN );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS  )
			{
				SendWeaponAnim( ACT_VM_SIGHTIDLEOPEN );
			}
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_IS:
			SendWeaponAnim( ACT_VM_SHOULDER2SIGHTOPEN );
			break;

		case WEAPON_ANIMATION_IS_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_SIGHT2SHOULDEROPEN );
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_HIP:
			SendWeaponAnim( ACT_VM_SHOULDER2HIPOPEN );
			break;

		case WEAPON_ANIMATION_HIP_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_HIP2SHOULDEROPEN );
			break;

		case WEAPON_ANIMATION_PRONE_ENTER:
			SendWeaponAnim( ACT_VM_PRONE_ENTER_OPEN );
			break;
		case WEAPON_ANIMATION_PRONE_LEAVE:
			SendWeaponAnim( ACT_VM_PRONE_LEAVE_OPEN );
			break;

		case WEAPON_ANIMATION_SPRINT_ENTER:
			SendWeaponAnim( ACT_VM_SPRINT_ENTER_OPEN );
			break;
		case WEAPON_ANIMATION_SPRINT_LEAVE:
			SendWeaponAnim( ACT_VM_SPRINT_LEAVE_OPEN );
			break;

		default:
			BaseClass::HandleViewAnimation( iAnim );
			break;
		}
	}
	else
	{
		BaseClass::HandleViewAnimation( iAnim );
	}
}