//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
//#include "player.h"

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasesubmachinegun.h"
#include "gamemovement.h"

#define BAR_ROF 0.15f // ( 400 RPM / 60 )^-1 = second interval for rof

#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#define CWeaponBAR C_WeaponBAR
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
#endif

ConVar bar_ironsightsx("bar_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar bar_ironsightsy("bar_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar bar_ironsightsz("bar_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar bar_shoulderx("bar_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar bar_shouldery("bar_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar bar_shoulderz("bar_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponBAR : public CWeaponRnLBaseSubMachineGun
{
public:
	DECLARE_CLASS( CWeaponBAR, CWeaponRnLBaseSubMachineGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponBAR();
	~CWeaponBAR() override = default;

	void ItemPostFrame() override;
	void HandleViewAnimation( int iAnim ) override;

	void ToggleFireMode(void);

	void StartFireModeToggle();
	bool HandleFireModeToggle();

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_BAR; }

#ifdef CLIENT_DLL
	Vector	GetIronsightsOffset() override { return Vector( bar_ironsightsx.GetFloat(), bar_ironsightsy.GetFloat(), bar_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset() override { return Vector( bar_shoulderx.GetFloat(), bar_shouldery.GetFloat(), bar_shoulderz.GetFloat() ); }
#endif

private:
	//OH OH It's Sir Hacks-a-lot - this just tells us if the ROF toggle needs
	//	to go back to ironsights
	CNetworkVar( int, m_iFireToggleTrasition );
	CNetworkVar( bool, m_bAutoFire );

	CWeaponBAR( const CWeaponBAR & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBAR, DT_WeaponBAR )

BEGIN_NETWORK_TABLE( CWeaponBAR, DT_WeaponBAR )
	PropBool( PROPINFO( m_bAutoFire ) ),
	PropInt(PROPINFO( m_iFireToggleTrasition ) ),
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponBAR )
	DEFINE_PRED_FIELD( m_bAutoFire, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iFireToggleTrasition, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

acttable_t CWeaponBAR::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_TOMMY,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_TOMMY,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_TOMMY,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_TOMMY,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_TOMMY,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_TOMMY,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_TOMMY,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_TOMMY,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_TOMMY,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_TOMMY,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_TOMMY,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_TOMMY,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_TOMMY,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_TOMMY,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_TOMMY, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_TOMMY,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_TOMMY,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_TOMMY,		false },
};

IMPLEMENT_ACTTABLE(CWeaponBAR);

LINK_ENTITY_TO_CLASS( weapon_bar, CWeaponBAR );

PRECACHE_WEAPON_REGISTER( weapon_bar );

CWeaponBAR::CWeaponBAR()
{
	m_bAutoFire = true;
	m_iFireToggleTrasition = 0;
}

//======================================================================================// 
//																						//
//							CWeaponBAR::ItempostFrame()							//
//	Purpose: Method called every frame. Check for buttons/keys pressed Checks reload	//
//							status and calls WeaponIdle()								//
//																						//
//======================================================================================//
void CWeaponBAR::ItemPostFrame()
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
	/*else if( HandleFireModeToggle() )
	{
		return;
	}*/

	if( !(pPlayer->m_nButtons & IN_ATTACK) && m_bAutoFire == false && m_flNextPrimaryAttack > gpGlobals->curtime &&  IsActivityFinished())
	{
		//if we're in single round then we've let go of sinor trigger
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime;
	}

	bool canDoActivity = ( m_flNextPrimaryAttack <= gpGlobals->curtime ) ? true : false;
 
	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
		{
			PrimaryAttack(); 
		}
		else if( m_bAutoFire == false )
		{
			//We haven't let go of the trigger yet so keep increasing the next attack till we do
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + BAR_ROF;
		}

	}
	else if (pPlayer->m_nButtons & IN_RELOAD && canDoActivity )
	{
		ReloadTransition( 1 );
	}
	/*else if (( pPlayer->m_nButtons & IN_ATTACK2 ) && canDoActivity )
	{
		StartFireModeToggle();
	}*/

	WeaponIdle();
}

void CWeaponBAR::StartFireModeToggle()
{
	m_iFireToggleTrasition = 1;
}

bool CWeaponBAR::HandleFireModeToggle()
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return false; 

	int iPosture = pPlayer->GetWeaponPosture();

	if( m_iFireToggleTrasition != 0 )
	{
		bool canDoActivity = (gpGlobals->curtime >= m_flNextPrimaryAttack) ? true : false;

		if( m_iFireToggleTrasition == 1 && canDoActivity )
		{
			if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			m_iFireToggleTrasition = 2;
		}
		else if( m_iFireToggleTrasition == 2 && canDoActivity )
		{
			ToggleFireMode();
			m_iFireToggleTrasition = 3;
		}
		else if( m_iFireToggleTrasition == 3 && canDoActivity )
		{
			if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			m_iFireToggleTrasition = 0;
		}

		return true;
	}

	return false;
}

void CWeaponBAR::ToggleFireMode( void )
{
	//bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	if ( m_bAutoFire )
	{
		/*if( bMagEmpty )
		{
			SendWeaponAnim ( ACT_VM_SPECIALONOPEN );
		}
		else*/
		{
			SendWeaponAnim ( ACT_VM_SPECIALON );
		}
		m_bAutoFire = false;
	}	
	else
	{
		/*if( bMagEmpty )
		{
			SendWeaponAnim ( ACT_VM_SPECIALOFFOPEN );
		}
		else*/
		{
			SendWeaponAnim ( ACT_VM_SPECIALOFF );
		}
		m_bAutoFire = true;
	}

	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
}

void CWeaponBAR::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return;

	int iPosture = pPlayer->GetWeaponPosture();
	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	SetAnimationState( iAnim );

	if( !m_bAutoFire )
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_ATTACK:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPFIRE1_DEPLOYED );	
			}
			else if( iPosture  == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERFIRE1_DEPLOYED );
			}
			else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTFIRE1_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_RELOAD:
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_RELOAD_DEPLOYED );
			}
			else
			{
				SendWeaponAnim( ACT_VM_RELOADMID_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_DRAW:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPDRAW_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERDRAW_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTDRAW_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_HOLSTER:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPHOLSTER_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERHOLSTER_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTHOLSTER_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_IDLE:
			if( GetPlayerOwner()  && GetPlayerOwner()->IsProne() 
				&& GetPlayerOwner()->GetLocalVelocity().Length2D() > SPEED_PRONE_WALK  )
			{
				SendWeaponAnim( ACT_VM_PRONE_IDLE_DEPLOYED );
				SetAnimationState( WEAPON_ANIMATION_PRONE_IDLE );
			}			
			else if( GetPlayerOwner() && GetPlayerOwner()->IsSprinting() )
			{
				SendWeaponAnim( ACT_VM_SPRINT_IDLE_DEPLOYED );
				SetAnimationState( WEAPON_ANIMATION_SPRINT_IDLE );
			}
			else if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPIDLE_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERIDLE_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTIDLE_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_IS:
			SendWeaponAnim( ACT_VM_SHOULDER2SIGHT_DEPLOYED );
			break;

		case WEAPON_ANIMATION_IS_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_SIGHT2SHOULDER_DEPLOYED );
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_HIP:
			SendWeaponAnim( ACT_VM_SHOULDER2HIP_DEPLOYED );
			break;

		case WEAPON_ANIMATION_HIP_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_HIP2SHOULDER_DEPLOYED );
			break;

		case WEAPON_ANIMATION_PRONE_ENTER:
			SendWeaponAnim( ACT_VM_PRONE_ENTER_DEPLOYED );
			break;
		case WEAPON_ANIMATION_PRONE_LEAVE:
			SendWeaponAnim( ACT_VM_PRONE_LEAVE_DEPLOYED );
			break;

		case WEAPON_ANIMATION_SPRINT_ENTER:
			SendWeaponAnim( ACT_VM_SPRINT_ENTER_DEPLOYED );
			break;
		case WEAPON_ANIMATION_SPRINT_LEAVE:
			SendWeaponAnim( ACT_VM_SPRINT_LEAVE_DEPLOYED );
			break;
		}
	}
	else
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_ATTACK:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPFIRE1 );	
			}
			else if( iPosture  == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERFIRE1 );
			}
			else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTFIRE1 );
			}
			break;

		case WEAPON_ANIMATION_RELOAD:
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_RELOAD );
			}
			else
			{
				SendWeaponAnim( ACT_VM_RELOADMID );
			}
			break;

		case WEAPON_ANIMATION_DRAW:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPDRAW );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERDRAW );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTDRAW );
			}
			break;

		case WEAPON_ANIMATION_HOLSTER:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPHOLSTER );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTHOLSTER );
			}
			break;

		case WEAPON_ANIMATION_IDLE:
			if( GetPlayerOwner()  && GetPlayerOwner()->IsProne() 
				&& GetPlayerOwner()->GetLocalVelocity().Length2D() > SPEED_PRONE_WALK  )
			{
				SendWeaponAnim( ACT_VM_PRONE_IDLE );
				SetAnimationState( WEAPON_ANIMATION_PRONE_IDLE );
			}			
			else if( GetPlayerOwner() && GetPlayerOwner()->IsSprinting() )
			{
				SendWeaponAnim( ACT_VM_SPRINT_IDLE );
				SetAnimationState( WEAPON_ANIMATION_SPRINT_IDLE );
			}
			else if( iPosture == WEAPON_POSTURE_HIP )
			{
				SendWeaponAnim( ACT_VM_HIPIDLE );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				SendWeaponAnim( ACT_VM_SHOULDERIDLE );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				SendWeaponAnim( ACT_VM_SIGHTIDLE );
			}
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_IS:
			SendWeaponAnim( ACT_VM_SHOULDER2SIGHT );
			break;

		case WEAPON_ANIMATION_IS_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_SIGHT2SHOULDER );
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_HIP:
			SendWeaponAnim( ACT_VM_SHOULDER2HIP );
			break;

		case WEAPON_ANIMATION_HIP_TO_SHOULDER:
			SendWeaponAnim( ACT_VM_HIP2SHOULDER );
			break;

		case WEAPON_ANIMATION_PRONE_ENTER:
			SendWeaponAnim( ACT_VM_PRONE_ENTER );
			break;
		case WEAPON_ANIMATION_PRONE_LEAVE:
			SendWeaponAnim( ACT_VM_PRONE_LEAVE );
			break;

		case WEAPON_ANIMATION_SPRINT_ENTER:
			SendWeaponAnim( ACT_VM_SPRINT_ENTER );
			break;
		case WEAPON_ANIMATION_SPRINT_LEAVE:
			SendWeaponAnim( ACT_VM_SPRINT_LEAVE );
			break;
		}
	}
}