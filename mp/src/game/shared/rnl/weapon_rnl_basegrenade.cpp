//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_rnl_basegrenade.h"
#include "in_buttons.h"	
#include "rnl_fx_shared.h"
#include "rnl_gamerules.h"
#include "rnl_ammodef.h"

#ifdef CLIENT_DLL

	#include "c_rnl_player.h"
	#include "iinput.h"

#else

	#include "rnl_player.h"
	#include "items.h"

#endif

#ifndef CLIENT_DLL
ConVar	mp_melontastic	( "mp_melontastic", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Makes RnL Mellontastic bitches!" );
#endif

#define GRENADE_TIMER	1.5f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED( BaseRnLGrenade, DT_BaseRnLGrenade )

BEGIN_NETWORK_TABLE(CBaseRnLGrenade, DT_BaseRnLGrenade)
	PropBool( PROPINFO(m_bRedraw) ),
	PropInt(PROPINFO(m_iGrenadeState) ),
	PropTime(PROPINFO(m_flFuseEnd)),
	PropTime(PROPINFO( m_fThrowTime ) ),
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CBaseRnLGrenade )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iGrenadeState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL(m_flFuseEnd, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	DEFINE_PRED_FIELD_TOL(m_fThrowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_basernlgrenade, CBaseRnLGrenade );


CBaseRnLGrenade::CBaseRnLGrenade()
{
	m_bRedraw = false;
	m_iGrenadeState = GRENADE_DRAWN;
	m_flFuseEnd = 0.0f;
	m_fThrowTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseRnLGrenade::Precache()
{
	BaseClass::Precache();
}

void CBaseRnLGrenade::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

bool CBaseRnLGrenade::CanDeploy( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if( !pPlayer )
		return false;

	return HasAnyAmmo() || (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) > 0);
}

bool CBaseRnLGrenade::CanHolster( void )
{
	return BaseClass::CanHolster() && (m_iGrenadeState == GRENADE_DRAWN);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseRnLGrenade::Deploy()
{
	m_fThrowTime = 0;
	m_bRedraw = false;
	m_iGrenadeState = GRENADE_DRAWN;

	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime;

		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );
#ifndef CLIENT_DLL
		pPlayer->SetFOV( pPlayer, 0 );
#endif
	}

	bool ret_val = BaseClass::Deploy();

	//Definatly want to play OUR animation :)
	HandleViewAnimation( WEAPON_ANIMATION_DRAW );

	return ret_val;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseRnLGrenade::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_fThrowTime = 0;
	m_bRedraw = false;

	// Send holster animation
	HandleViewAnimation( WEAPON_ANIMATION_HOLSTER );

	m_iGrenadeState = GRENADE_DRAWN; // when this is holstered make sure the pin isn’t pulled.

	// If they attempt to switch weapons before the throw animation is done, 
	// allow it, but kill the weapon if we have to.
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_HOLSTER );
	}

#ifndef CLIENT_DLL
	if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
	{
		pPlayer->Weapon_Drop( this, NULL, NULL );
		UTIL_Remove(this);
	}
#endif

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseRnLGrenade::CanClimb( void ) 
{ 
	return ( m_iGrenadeState == GRENADE_DRAWN ); 
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseRnLGrenade::PrimaryAttack()
{
	if ( m_bRedraw || m_iGrenadeState > 0 )
		return;

	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer || pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 || pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
		return;

	//Hack hack it's not a reload but reuse recycle ;)
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );

	// The pull pin animation has to finish, then we wait until they aren't holding the primary
	// attack button, then throw the grenade.
	HandleViewAnimation( WEAPON_ANIMATION_ATTACK );

	m_iGrenadeState = GRENADE_PREP;
	
	// Don't let weapon idle interfere in the middle of a throw!
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseRnLGrenade::Reload()
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if ( pPlayer && m_bRedraw && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		HandleViewAnimation( WEAPON_ANIMATION_DRAW );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );

		m_bRedraw = false;
		m_iGrenadeState = GRENADE_DRAWN;

		//Update our times
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		SetWeaponIdleTime( m_flNextPrimaryAttack );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseRnLGrenade::ItemPostFrame()
{
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	CBaseViewModel *vm = pPlayer->GetViewModel( m_nViewModelIndex );
	if ( !vm )
		return;

	bool bCanDoNextPrimary = ( m_flNextPrimaryAttack <= gpGlobals->curtime ) ? true : false;

	// the player has pulled the PIN and has pressed attack again
	// start the fuse burning
	if ( m_iGrenadeState == GRENADE_PREP && ( pPlayer->m_afButtonPressed & IN_ATTACK ) && bCanDoNextPrimary ) 
	{
		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_IRONSIGHTS );

		m_iGrenadeState = GRENADE_STARTFUSE;
		HandleViewAnimation( WEAPON_ANIMATION_ATTACK );

		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

		// seed the predicted seed so same result on server/client
		RandomSeed( CBaseEntity::GetPredictionRandomSeed() & 255 );

		// put a little random factor in there, one second difference
		m_flFuseEnd = gpGlobals->curtime + RandomFloat( 4.5f, 5.0f );

		if( GetWeaponID() == WEAPON_MK2GRENADE || GetWeaponID() == WEAPON_M18GRENADE )
		{
			QAngle angThrow = pPlayer->GetWeaponAngle();
			angThrow[YAW] += 90;
			Vector vForward, vRight, vUp;

			if (angThrow.x < 90 )
				angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
			else
			{
				angThrow.x = 360.0f - angThrow.x;
				angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
			}

			AngleVectors( angThrow, &vForward, &vRight, &vUp );

			Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

			// move out, right and up a little (to where the hand would be)
			vecSrc += vRight * 16;
			FX_ThrowNadeSpoon( 0, vecSrc, angThrow );
		}
	}
	// throw the actual grenade
	else if( m_iGrenadeState == GRENADE_STARTFUSE && !(pPlayer->m_nButtons & IN_ATTACK) && bCanDoNextPrimary )
	{
		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );

		if( (m_flNextPrimaryAttack + RNL_GRENADE_THROW_DELAY) > gpGlobals->curtime )
			m_iGrenadeState = GRENADE_ROLLING;
		else
			m_iGrenadeState = GRENADE_THROWING;

		DecrementAmmo( pPlayer );

		m_fThrowTime = gpGlobals->curtime;
		HandleViewAnimation( WEAPON_ANIMATION_ATTACK2 );

		pPlayer->SetAbsVelocity( vec3_origin );
		pPlayer->SetMaxSpeed( 55.0f );

		SetWeaponIdleTime( gpGlobals->curtime + 0.25);//SequenceDuration() );

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
	else if( m_iGrenadeState == GRENADE_THROWING || m_iGrenadeState == GRENADE_ROLLING )
	{
		if(m_fThrowTime < gpGlobals->curtime)
		{
			if( m_iGrenadeState == GRENADE_ROLLING )
				RollGrenade();
			else
				ThrowGrenade();

			//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
			m_flTimeWeaponIdle = gpGlobals->curtime + 0.50f;
		}
	}
	// player has held the grenade for entire length of fuse.. you know what that means!
	else if( m_iGrenadeState == GRENADE_STARTFUSE && ( m_flFuseEnd < gpGlobals->curtime ))
	{
		m_iGrenadeState = GRENADE_THROWING;
		DecrementAmmo( pPlayer );
		DropGrenade();
	}
	else if( m_bRedraw )
	{
		// Has the throw animation finished playing
		if( m_flTimeWeaponIdle < gpGlobals->curtime )
		{
			pPlayer->SetMaxSpeed( 250.0f );
#ifndef CLIENT_DLL
			// if we're officially out of grenades, ditch this weapon
			if( pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 )
			{
				pPlayer->Weapon_Drop( this, NULL, NULL );
				UTIL_Remove(this);
			}
			else
				Deploy();
#endif
			return;
		}
	}
	else
	{
		// go to primary attack and reload etc
		BaseClass::ItemPostFrame();
	}

	int iBodyGroup = -1;

	if( IsSmokeGrenade() )
		iBodyGroup = pPlayer->FindBodygroupByName("smokes");
	else
		iBodyGroup = pPlayer->FindBodygroupByName("frags");

	if( iBodyGroup > -1 && (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) != pPlayer->GetBodygroup( iBodyGroup )) )
		pPlayer->SetBodygroup( iBodyGroup, pPlayer->GetAmmoCount( m_iPrimaryAmmoType ));
}

void CBaseRnLGrenade::WeaponIdle( void )
{
	if ( IsViewModelSequenceFinished() )
		HandleViewAnimation( WEAPON_ANIMATION_IDLE );
}

void CBaseRnLGrenade::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	SetAnimationState( iAnim );

	switch( iAnim )
	{
	case WEAPON_ANIMATION_ATTACK:
		if( m_iGrenadeState == GRENADE_STARTFUSE )
			SendWeaponAnim( ACT_VM_STARTFUSE );
		else
			SendWeaponAnim( ACT_VM_PREP );
		break;

	case WEAPON_ANIMATION_ATTACK2:
		if( m_iGrenadeState == GRENADE_ROLLING )
			SendWeaponAnim( ACT_VM_THROWLOWER );
		else
			SendWeaponAnim( ACT_VM_THROW );
		break;

	case WEAPON_ANIMATION_DRAW:
		SendWeaponAnim( GetDrawActivity() );
		break;

	case WEAPON_ANIMATION_HOLSTER:
		if( ((GetWeaponID() == WEAPON_STICKY39GRENADE) || (GetWeaponID() == WEAPON_STICKY24GRENADE)) && (m_iGrenadeState == GRENADE_PREP) )
			SendWeaponAnim( ACT_VM_SHOULDERHOLSTEROPEN );
		else
			SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );
		break;

	case WEAPON_ANIMATION_IDLE:
		if( m_iGrenadeState == GRENADE_STARTFUSE )
			SendWeaponAnim( ACT_VM_PREP_IDLE );
		else if( pPlayer->IsSprinting() )
			SendWeaponAnim( ACT_VM_SPRINT_IDLE );
		else
			SendWeaponAnim( ACT_VM_IDLE );
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

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CBaseRnLGrenade::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

#ifdef CLIENT_DLL

	void CBaseRnLGrenade::DropGrenade()
	{
		m_bRedraw = true;
		m_iGrenadeState = GRENADE_THROWN;
		m_flFuseEnd = 0.0f; // reset the timer
	}

	void CBaseRnLGrenade::RollGrenade()
	{
		m_iGrenadeState = GRENADE_THROWN;
		m_bRedraw = true;
		m_flFuseEnd = 0.0f; // reset the timer

		CRnLPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return;

		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
	}

	void CBaseRnLGrenade::ThrowGrenade()
	{
		m_iGrenadeState = GRENADE_THROWN;
		m_bRedraw = true;
		m_flFuseEnd = 0.0f; // reset the timer

		CRnLPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return;

		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
	}


#else

	BEGIN_DATADESC( CBaseRnLGrenade )
		DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	END_DATADESC()

	int CBaseRnLGrenade::CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1; 
	}

	void CBaseRnLGrenade::ThrowGrenade()
	{
		CRnLPlayer *pPlayer = ToRnLPlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		float fuseLeft = m_flFuseEnd - gpGlobals->curtime;

		QAngle angThrow = pPlayer->GetWeaponAngle();

		Vector vForward, vRight, vUp;

		if (angThrow.x < 90 )
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);
		else
		{
			angThrow.x = 360.0f - angThrow.x;
			angThrow.x = -10 + angThrow.x * -((90 - 10) / 90.0);
		}

		float flVel = 675;

		//RnL : BB
		//Careful to make sure we don't have a zero
		//since it is the denominator in some division
		if(fuseLeft < 0.1f)
			fuseLeft = 0.1f;

		// throws held longer have a greater velocity
		float flBonus = ( 5.0f - fuseLeft ) * 200.0f;

		AngleVectors( angThrow, &vForward, &vRight, &vUp );

		Vector vecSrc = pPlayer->EyePosition();

		// move out, right and up a little (to where the hand would be)
		vecSrc += vForward * 16;

		vecSrc += vRight * 24;
		vecSrc += vUp * 16;
		
		flVel += flBonus;

		if( pPlayer->IsProne() )
			flVel *= 0.65;
		else if( pPlayer->IsDucked() )
			flVel *= 0.85;

		Vector vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();
		DevMsg( "Grenade thrown! Velocity bonus: %f, Velocity: %f Fuse left: %f\n", flBonus, flVel, fuseLeft );

		EmitGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(0,500,0), pPlayer, fuseLeft, false );


		m_bRedraw = true;
		m_flFuseEnd = 0.0f;
		m_iGrenadeState = GRENADE_THROWN;

		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
	}

	void CBaseRnLGrenade::RollGrenade()
	{
		CRnLPlayer *pPlayer = ToRnLPlayer( GetOwner() );
		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		float fuseLeft = m_flFuseEnd - gpGlobals->curtime;

		QAngle angThrow = pPlayer->GetWeaponAngle();
		angThrow[PITCH] = 0;

		float flVel = 750;

		//RnL : BB
		//Careful to make sure we don't have a zero
		//since it is the denominator in some division
		if(fuseLeft < 0.1f)
			fuseLeft = 0.1f;


		Vector vForward, vRight, vUp;
		AngleVectors( angThrow, &vForward, &vRight, &vUp );

		Vector vecSrc = pPlayer->EyePosition();

		// move out, right and down a little (to where the hand would be)
		vecSrc += vForward * 6;

		vecSrc += vRight * 12;
		vecSrc -= vUp * 6;
		
		Vector vecThrow = vForward * flVel + pPlayer->GetAbsVelocity();

		if( pPlayer->IsProne() )
			flVel *= 0.5;
		else if( pPlayer->IsDucked() )
			flVel *= 0.75;

		DevMsg( "Grenade rolled! Velocity: %f Fuse left: %f\n", flVel, fuseLeft );

		EmitGrenade( vecSrc, vec3_angle, vecThrow, AngularImpulse(0,500,0), pPlayer, fuseLeft, true );


		m_bRedraw = true;
		m_flFuseEnd = 0.0f;
		m_iGrenadeState = GRENADE_THROWN;

		//pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
	}

	void CBaseRnLGrenade::DropGrenade()
	{
		CRnLPlayer *pPlayer = ToRnLPlayer( GetOwner() );

		if ( !pPlayer )
		{
			Assert( false );
			return;
		}

		Vector vForward;
		pPlayer->EyeVectors( &vForward );
		Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset() + vForward * 16; 

		Vector vecVel = pPlayer->GetAbsVelocity();

		if( m_iGrenadeState != GRENADE_STARTFUSE && m_iGrenadeState != GRENADE_THROWING )
		{
			// put a little random factor in there, one second difference
			m_flFuseEnd = gpGlobals->curtime + RandomFloat( 4.0f, 5.0f );
		}

		float fuseLeft = m_flFuseEnd - gpGlobals->curtime;

		EmitGrenade( vecSrc, vec3_angle, vecVel, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, fuseLeft, true );

		m_bRedraw = true;
		m_flFuseEnd = 0.0f;
		m_iGrenadeState = GRENADE_THROWN;

		DevMsg( "Grenade dropped!\n" );
	}

	void CBaseRnLGrenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float m_fDelay, bool roll )
	{
		Assert( 0 && "CBaseRnLGrenade::EmitGrenade should not be called. Make sure to implement this in your subclass!\n" );
	}

	bool CBaseRnLGrenade::AllowsAutoSwitchFrom( void ) const
	{
		return ( m_iGrenadeState == GRENADE_DRAWN );
	}
	

#endif

