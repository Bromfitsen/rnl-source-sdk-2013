//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_baseexplosive.h"
#include "in_buttons.h"	
#include "rnl_fx_shared.h"
#include "rnl_gamerules.h"

#ifdef CLIENT_DLL

	#include "c_rnl_player.h"
	#include "iinput.h"

#else

	#include "rnl_player.h"
	#include "items.h"

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBaseExplosive, DT_WeaponRnLBaseExplosive )

BEGIN_NETWORK_TABLE(CWeaponRnLBaseExplosive, DT_WeaponRnLBaseExplosive)

#ifndef CLIENT_DLL
	SendPropBool( SENDINFO(m_bPlanting) ),
	SendPropEHandle( SENDINFO( m_pPlantingTarget ) ),
#else
	RecvPropBool( RECVINFO(m_bPlanting) ),
	RecvPropEHandle( RECVINFO( m_pPlantingTarget ) ),
#endif

END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CWeaponRnLBaseExplosive )
		DEFINE_FIELD( m_bPlanting, FIELD_BOOLEAN ),
	END_DATADESC()
#else
	BEGIN_PREDICTION_DATA( CWeaponRnLBaseExplosive )
		DEFINE_PRED_FIELD( m_bPlanting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_rnl_baseexplosive, CWeaponRnLBaseExplosive );


CWeaponRnLBaseExplosive::CWeaponRnLBaseExplosive()
{
	m_bPlanting = false;
	m_pPlantingTarget = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRnLBaseExplosive::Precache()
{
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRnLBaseExplosive::Deploy()
{
	m_bPlanting = false;
	m_pPlantingTarget = NULL;

	CRnLPlayer *pPlayer = GetPlayerOwner();
	if( pPlayer )
	{
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );
	}

	// Play our animation
	HandleViewAnimation( WEAPON_ANIMATION_DRAW );

	return true;
}

bool CWeaponRnLBaseExplosive::CanDeploy()
{
	if( !GetPlayerOwner() )
		return false;

	return Clip1() > 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponRnLBaseExplosive::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bPlanting = false;
	m_pPlantingTarget = NULL;

	// If they attempt to switch weapons before the throw animation is done, 
	// allow it, but kill the weapon if we have to.
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_HOLSTER );
	}

	// Send holster animation
	HandleViewAnimation( WEAPON_ANIMATION_HOLSTER );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRnLBaseExplosive::PrimaryAttack()
{
	if ( m_bPlanting )
		return;

	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if( Clip1() <= 0 )
		return;

#ifndef CLIENT_DLL
	Vector forward;
	Vector eyePos = pPlayer->EyePosition();
	pPlayer->EyeVectors( &forward );
	CRnLTaskDestructable* pEnt = (CRnLTaskDestructable*)(gEntList.FindEntityClassNearestFacing( eyePos, forward,  0.95, "rnl_task_destructable" ));
	if( pEnt )
	{
		if( eyePos.DistTo( pEnt->GetAbsOrigin() ) < 64.0f )
		{
			if( pEnt->CanAttachExplosive(pPlayer) )
			{
				//Hack hack it's not a reload but reuse recycle ;)
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );

				// The pull pin animation has to finish, then we wait until they aren't holding the primary
				// attack button, then throw the grenade.
				HandleViewAnimation( WEAPON_ANIMATION_ATTACK );

				m_bPlanting = true;
				m_pPlantingTarget = pEnt;
				
				// Don't let weapon idle interfere in the middle of a throw!
				SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

				m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

				return;
			}
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRnLBaseExplosive::ItemPostFrame()
{
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( ShouldHandleWeaponLimitations() )
		return;

	bool bCanDoNextPrimary = ( m_flNextPrimaryAttack <= gpGlobals->curtime ) ? true : false;

	if( m_bPlanting && bCanDoNextPrimary )
	{
		m_bPlanting = false;
		m_iClip1--;
		PlantExplosive();

#ifdef GAME_DLL
		if( (m_iClip1 <= 0) && RnLGameRules() )
		{
			if( RnLGameRules() )
				RnLGameRules()->SwitchToNextBestWeapon( pPlayer, this );
			else
			{
				pPlayer->Weapon_Drop( this, NULL, NULL );
				UTIL_Remove(this);
			}
		}
#endif
	}
	else if( m_bPlanting && !( pPlayer->m_nButtons & IN_ATTACK ) )
	{
		m_bPlanting = false;
		m_pPlantingTarget = NULL;
		WeaponIdle();

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;
	}
	else if ( !m_bPlanting && ( pPlayer->m_nButtons & IN_ATTACK ) && bCanDoNextPrimary ) 
	{
		PrimaryAttack();
	}
	else
		WeaponIdle();
}

void CWeaponRnLBaseExplosive::WeaponIdle( void )
{
	if ( HasWeaponIdleTimeElapsed() )
		HandleViewAnimation( WEAPON_ANIMATION_IDLE );
}

void CWeaponRnLBaseExplosive::HandleViewAnimation( int iAnim )
{
	DevMsg("%i\n", iAnim);
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	SetAnimationState( iAnim );

	switch( iAnim )
	{
	case WEAPON_ANIMATION_ATTACK:
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
		break;

	case WEAPON_ANIMATION_DRAW:
		SendWeaponAnim( ACT_VM_SHOULDERDRAW );
		break;

	case WEAPON_ANIMATION_HOLSTER:
		SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );
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

	case WEAPON_ANIMATION_IDLE:
	default:
		if( pPlayer->IsProne() )
			SendWeaponAnim( ACT_VM_PRONE_IDLE );
		else if( pPlayer->IsSprinting() )
			SendWeaponAnim( ACT_VM_SPRINT_IDLE );
		else
			SendWeaponAnim( ACT_VM_SHOULDERIDLE );
		break;
	}

	DevMsg("Sequence Time: %f\n", SequenceDuration());
}