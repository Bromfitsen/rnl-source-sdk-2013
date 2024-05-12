//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlballisticbase.h"
#include "gamemovement.h"

#if defined( CLIENT_DLL )
#	ifdef _DEBUG
#		include "prediction.h"
#	endif
#	include "c_rnl_player.h"
#else
#	include "rnl_player.h"
#	include "te_effect_dispatch.h"
#	include "eventqueue.h"
#	include "gameinterface.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBallisticBase, DT_WeaponRnLBallisticBase );

BEGIN_NETWORK_TABLE( CWeaponRnLBallisticBase, DT_WeaponRnLBallisticBase )
	PropInt(PROPINFO(m_iReloadState)),
	PropTime(PROPINFO(m_flIronsightsPressedTime)),
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC(CWeaponRnLBallisticBase)
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA( CWeaponRnLBallisticBase )
		DEFINE_PRED_FIELD( m_iReloadState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
		DEFINE_PRED_FIELD_TOL(m_flIronsightsPressedTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_baseballistic, CWeaponRnLBallisticBase );

CWeaponRnLBallisticBase::CWeaponRnLBallisticBase()
{
	m_iReloadState = 0;
}

CWeaponRnLBallisticBase::~CWeaponRnLBallisticBase()
{

}

//=============================================================================//
// CWeaponRnLBallisticBase::Deploy( )
// Purpose: Sets the starting value of the vars.
//
//=============================================================================//
bool CWeaponRnLBallisticBase::Deploy( )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if( pPlayer )
	{
		pPlayer->m_iShotsFired = 0;
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );
	}

	HandleViewAnimation( WEAPON_ANIMATION_DRAW );
	m_flIronsightsPressedTime = 0.0f;

#ifndef CLIENT_DLL
	SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_SHOULDER] );
	HandleFoVTransitions();
#endif

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRnLBallisticBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	m_flIronsightsPressedTime = 0.0f;

	// cancel any reload in progress.
	m_bInReload = false; 

	// kill any think functions
	SetThink(NULL);
	
	// TODO: Add an emum for the positions - Stefan
	// i dont want anyone to get confused and juans code is still a bit messy
	// i myself mixed them up, already ;)
	ReturnToDefaultPosture();
	HandleFoVTransitions();

	// Send holster animation
	HandleViewAnimation( WEAPON_ANIMATION_HOLSTER );

	// Some weapons don't have holster anims yet, so detect that
	// Ours do, so don't care about that. - Stefan
	float flSequenceDuration = 0;
	
	flSequenceDuration = SequenceDuration();

	CRnLPlayer *pOwner = GetPlayerOwner();
	if (pOwner)
	{
		pOwner->SetNextAttack( gpGlobals->curtime + flSequenceDuration );
		pOwner->DoAnimationEvent( PLAYERANIMEVENT_HOLSTER );
	}

#ifndef CLIENT_DLL
	// If we don't have a holster anim, hide immediately to avoid timing issues
	if ( !flSequenceDuration )
	{
		SetWeaponVisible( false );
	}
	else
	{
		// Hide the weapon when the holster animation's finished
		g_EventQueue.AddEvent( this, "HideWeapon", flSequenceDuration, NULL, NULL );
	}
#endif

	return true;
}

//=============================================================================//
// CWeaponRnLBallisticBase::Reload( )
// Purpose: Check certain things and call the proper reload animation
//
//=============================================================================//
bool CWeaponRnLBallisticBase::Reload()
{	
	// Make an instance of the local player to deal with
	CRnLPlayer *pPlayer = GetPlayerOwner();
	
	/*
	if( !CheckChamber() )
		return false;
	*/

	int iReloadAnimState = 0;
	// We always reload "loosing" the mag, meaning that the "old" bullets are removed and either stored
	// in the ammo pouches or lost - no magic clip filling reload - Stefan
	// Check whether the mag is empty or not...
	// Here it's full
	//if (m_iClip1 == GetMaxClip1() )
	//{
	//	// Do nothing...why should we anyway
	//	iReloadAnimState = RELOAD_FULL;
	//	return false;
	//}

	HandleViewAnimation( WEAPON_ANIMATION_RELOAD );

	//
	// Here it's not empty
	if (m_iClip1 > 0)
	{
		// and the player has got enough bullets left to reload
		if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) > GetMaxClip1())
		{
			m_iClip1 = GetMaxClip1();
			pPlayer->RemoveAmmo( GetMaxClip1(), m_iPrimaryAmmoType );
		}
		// There's not enough left
		else if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= GetMaxClip1())
		{
			m_iClip1 = pPlayer->GetAmmoCount( GetPrimaryAmmoType() );
			pPlayer->RemoveAmmo( pPlayer->GetAmmoCount( GetPrimaryAmmoType() ), m_iPrimaryAmmoType );
		}
		iReloadAnimState = RELOAD_MID;
	}
	//
	// The clip is empty
	else if (m_iClip1 == 0)
	{
		// But there's not enough ammo left
		if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= GetMaxClip1())
		{
			m_iClip1 = pPlayer->GetAmmoCount( GetPrimaryAmmoType() );
			pPlayer->RemoveAmmo( pPlayer->GetAmmoCount( GetPrimaryAmmoType() ), m_iPrimaryAmmoType );
		}
		else
		{
			m_iClip1 += GetMaxClip1();
			pPlayer->RemoveAmmo( GetMaxClip1(), m_iPrimaryAmmoType );
		}
		iReloadAnimState = RELOAD_EMPTY;
	}

// Send the reload event so the thirdperson animation works
#ifdef GAME_DLL
	if( iReloadAnimState == 1 )
		SendReloadEvents( true );
	else
		SendReloadEvents();
#endif

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	return true;
}

///

float CWeaponRnLBallisticBase::GetSpread( void )
{ 
	return GetRnLWpnData().m_flSpread;
}
//=============================================================================//
// CWeaponM1Garand::Primary attack( )
// Purpose:
//
//=============================================================================//
#ifdef _DEBUG
//	#define DEBUG_SHOT
#endif

#ifdef DEBUG_SHOT
	float gFlPrevShotTime = 0.0f;
	int	giDoubleUpCounter = 0;
#endif

void CWeaponRnLBallisticBase::PrimaryAttack( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return;

	const CRnLWeaponInfo &pWeaponInfo = GetRnLWpnData();

	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	if (pPlayer->GetWaterLevel() == 3 )
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		return;
	}

	if( !bMagEmpty )
	{
		bool bPrimaryMode = true;
		// Out of ammo?
		if( m_iClip1 == 1) // Fire to empty - Stefan
		{
			PlayLastBulletSound(); // Play the last shot sound for any weapon that has one.
		}
		m_iClip1--;
		// player "shoot" animation, here comes another combo switch-if
		pPlayer->SetAnimation( PLAYER_ATTACK1 );
		HandleViewAnimation( WEAPON_ANIMATION_ATTACK );
		// TODO: Client viewmodel does not align with server, fixable? - nuke
		/*Vector muzzleOrigin; QAngle muzzleAngle;
		pPlayer->GetViewModel()->GetAttachment( pPlayer->GetViewModel()->LookupAttachment("Muzzle"), muzzleOrigin, muzzleAngle );*/
#ifdef DEBUG_SHOT
		Msg( "<-\n" );
#	ifdef CLIENT_DLL
		if( prediction->InPrediction() )
			if( prediction->IsFirstTimePredicted() )
				Msg( "Client (Predicted 1st): PrimaryAttack Before{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
			else
				Msg( "Client (Predicted Nth): PrimaryAttack Before{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
		else
			Msg( "Client: PrimaryAttack Before{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
#	else
		Msg( "Server: PrimaryAttack Before{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
#	endif
#endif
		FX_FireBullets( pPlayer->entindex(), pPlayer->Weapon_ShootPosition(), (/*pPlayer->EyeAngles()*/pPlayer->GetWeaponAngle() + pPlayer->GetPunchAngle())/* + pPlayer->GetAimingOffset()*/, GetWeaponID(), bPrimaryMode?Primary_Mode:Secondary_Mode,	CBaseEntity::GetPredictionRandomSeed() & 255, GetSpread() );
		pPlayer->m_iShotsFired++;
		pPlayer->DoMuzzleFlash();
		AddViewKick( CBaseEntity::GetPredictionRandomSeed() & 255 );// JJAS: here we call the recoil system to work

#ifdef DEBUG_SHOT
		if( gFlPrevShotTime == m_flNextPrimaryAttack.Get() )
		{
			giDoubleUpCounter++;
			Assert( giDoubleUpCounter < 3 );

#	ifdef CLIENT_DLL
			if( prediction->InPrediction() )
				if( prediction->IsFirstTimePredicted() )
				{
					Msg( "Client (Predicted 1st): Shot Double up #%d!\n", giDoubleUpCounter);
				}
				else
				{
					Msg( "Client (Predicted Nth): Shot Double up #%d!\n", giDoubleUpCounter);
				}
			else
			{
				Msg( "Client: Shot Double up #%d!\n", giDoubleUpCounter);
			}
#	else
			Msg( "Server: Shot Double up!\n");
#	endif
		}
		else
			giDoubleUpCounter = 0;

		gFlPrevShotTime = m_flNextPrimaryAttack;
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;
#ifndef CLIENT_DLL
		IGameEvent * event = gameeventmanager->CreateEvent( "player_shoot", true );

		if( event )
		{
			event->SetInt("userid", pPlayer->GetUserID() );
			event->SetInt("mode", pPlayer->GetWeaponPosture() );

			const char *killer_weapon_name = GetClassname();

			// strip the NPC_* or weapon_* from the inflictor's classname
			if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
				killer_weapon_name += 7;
			else if ( strncmp( killer_weapon_name, "NPC_", 4 ) == 0 )
				killer_weapon_name += 4;
			else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
				killer_weapon_name += 5;
			
			event->SetString("weapon", killer_weapon_name );

			gameeventmanager->FireEvent( event, true );
		}
#endif

#ifdef DEBUG_SHOT
#	ifdef CLIENT_DLL
		if( prediction->InPrediction() )
		{
			if( prediction->IsFirstTimePredicted() )
			{
				Msg( "Client (Predicted 1st): Platform Time{ %f and %u MS }\n", Plat_FloatTime(), Plat_MSTime() );
				Msg( "Client (Predicted 1st): PrimaryAttack After{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
			}
			else
			{
				Msg( "Client (Predicted Nth): Platform Time{ %f and %u MS }\n", Plat_FloatTime(), Plat_MSTime() );
				Msg( "Client (Predicted Nth): PrimaryAttack After{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
			}
		}
		else
		{
			Msg( "Client: Platform Time{ %f and %u MS }\n", Plat_FloatTime(), Plat_MSTime() );
			Msg( "Client: PrimaryAttack After{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
		}
#	else
		Msg( "Server: Platform Time{ %f and %u MS }\n", Plat_FloatTime(), Plat_MSTime() );
		Msg( "Server: PrimaryAttack After{ Curtime: %f NextPrimaryAttack: %f }\n", gpGlobals->curtime, m_flNextPrimaryAttack.Get() );
#	endif
		Msg( "->\n" );
#endif
	}
}

//=====================================================================================//
// CWeaponRnLBallisticBase::AddViewKick()
// Purpose: Weapon recoil after each shot based on player's current posture
// Note: firing positions (hip shoulder ironsights still mising not coded yet)
//
//=====================================================================================//
void CWeaponRnLBallisticBase::AddViewKick( int iSeed )
{ 
	// Do we have a valid owner holding the weapon? 
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return; 
	
	float fRecoilPenalty = GetRecoil();

	RandomSeed( iSeed );

	// Now let's get those random numbers within the provided limits 
	// set based on the player's posture 
	float fRandomX = ((RandomFloat( -0.1f, -0.8f ))*(fRecoilPenalty)); 
	float fRandomY = ((RandomFloat( -0.5f, 0.5f ))*(fRecoilPenalty));
	
	// new function that modifies client and server view angles
	QAngle kickAngle( fRandomX * 2, fRandomY * 2, 0.0f);

#ifdef CLIENT_DLL
	/*
	static float g_flViewKickStamp = 0;
	if( g_flViewKickStamp >= m_flNextPrimaryAttack )
	{
#ifdef DEBUG_SHOT
		Msg( "Viewkick Rejected\n" );
#endif
		return;
	}

	g_flViewKickStamp = m_flNextPrimaryAttack;
	*/
	
	//Don't allow the camera to go above straight up in any situation
	//Or past the MG's recoil limit (could be bypassed by Free Look)
	QAngle angCurrentViewAngles = pPlayer->GetWeaponAngle();

	if ( (IsMachineGun() && angCurrentViewAngles[PITCH] < -10.0f) || angCurrentViewAngles[PITCH] < -85.0f )
		return;

	pPlayer->AdjustWeaponAngle( kickAngle );
	pPlayer->AdjustViewAngles( kickAngle * random->RandomFloat( 0.75f, 1.25f ) );
#endif
}

float CWeaponRnLBallisticBase::GetRecoil( void )
{
	// Do we have a valid owner holding the weapon? 
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return GetRnLWpnData().m_flRecoil[RECOIL_NORMAL]; 

	int iPosture = pPlayer->GetWeaponPosture();
	
	// integer containing the player's current posture 
	int flags = pPlayer->GetFlags(); 
	float fRecoil = GetRnLWpnData().m_flRecoil[RECOIL_NORMAL];
	// Because we don't want the recoil to be always the same 
	// for each posture we'll have 2 limits and then we'll use 
	// then to randomly get the final number 
	// Is the player on the ground? 
	if ( flags & FL_ONGROUND ) 
	{ 
		// Is the player's ducking? 
		if( pPlayer->IsDucked() )
		{
			// The player is ducking so we'll set some a small penalty 
			fRecoil = GetRnLWpnData().m_flRecoil[RECOIL_CROUCHED]; 
		}
		else if (pPlayer->IsProne() )
		{ 
			// The player is proning so we'll set a moderate penalty 
			fRecoil = GetRnLWpnData().m_flRecoil[RECOIL_PRONE];
		} 
	} 
	else
	{ 
		// The player is jumping so we'll set a large penalty 
		fRecoil = GetRnLWpnData().m_flRecoil[RECOIL_JUMPING];
	} 

	if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
	{
		fRecoil *= GetRnLWpnData().m_flRecoil[RECOIL_SIGHTED];
	}

	return fRecoil;
}

bool CWeaponRnLBallisticBase::CheckResting( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return false;

	float flCurrentSpeed = pPlayer->GetAbsVelocity().Length();
	if( flCurrentSpeed > 0.5 )
		return false;
	
	if( pPlayer->IsProne() )
		return false;

	QAngle angle = pPlayer->EyeAngles();

	if( angle[PITCH] > 340 || angle[PITCH] < 20 )
	{
		Vector forward;
		Vector start = EyePosition() + Vector( 0, 0, -12 );
		angle[PITCH] = 0;
		AngleVectors( angle, &forward );

		trace_t result;
		UTIL_TraceLine( start, start + (forward * 48), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &result );

		if( result.startsolid || result.DidHit() )
		{
			return true;
		}
	}

	return BaseClass::CheckResting();
}

void CWeaponRnLBallisticBase::HandleViewSway( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	//float flWeight = (float)(GetRnLWpnData().iWeight);
	//if( flWeight < 1.0f )
	//	return;

	if( pPlayer->GetMoveType() == MOVETYPE_NOCLIP )
		return;

	if( CheckResting() )
		return;

	if( pPlayer->IsDeployed() )
		return;

	if( pPlayer->IsSprinting() )
	{
		pPlayer->AdjustWeaponSway( QAngle( 0, 0, 0 ) );
		return;
	}

	const float flCurrentSpeed = pPlayer->GetAbsVelocity().Length();
	const int iPosture = pPlayer->GetWeaponPosture(); 

	const float flStaminaFactor = 2.0f - (pPlayer->GetStamina() / 100.0f);
	const float flMoraleFactor = (pPlayer->GetMoraleLevel() / 65.0f); // Max ~1.5f
	const bool isDucked = pPlayer->IsDucked();
	const bool isProne = pPlayer->IsProne();

	// get a percentage of current speed, 160 is max speed that still has weapon sway (running)
	const float flMovementPercent = clamp(flCurrentSpeed / (SPEED_RUN + 10.0f), 0.0f, 1.0f);

	float movementModifierMin;
	float movementModifierMax;

	// Don't care about prone, because can't IS/SS while in prone
	switch (iPosture)
	{
	case WEAPON_POSTURE_IRONSIGHTS:
		movementModifierMin = isDucked ? 0.020f : 0.035f;
		movementModifierMax = isDucked ? 0.055f : 0.075f;
		break;
	case WEAPON_POSTURE_SUPERSIGHTS:
		movementModifierMin = isDucked ? 0.02f : 0.03f;
		movementModifierMax = isDucked ? 0.15f : 0.25f;
		break;
	default:
		movementModifierMin = isDucked ? 0.05f : 0.1f;
		movementModifierMax = isDucked ? 0.15f : 0.2f;
	}

	const float flMovementModifier = Lerp(flMovementPercent, movementModifierMin, movementModifierMax);

	const float vertMove = 7.25f;
	const float horzMove = 8.75f;

	float vertScale = isProne ? 0.35f : 0.90f;
	float horzScale = isProne ? 0.15f : 0.90f;

	if (iPosture == WEAPON_POSTURE_IRONSIGHTS)
	{
		vertScale = 1.5f;
	}
	else if(iPosture == WEAPON_POSTURE_SUPERSIGHTS)
	{
		if( flCurrentSpeed < 20 )
			horzScale = 0.20f;
		else
			horzScale = 0.40f;

#ifdef CLIENT_DLL
		vertScale = clamp(0.25f + (pPlayer->GetWeaponPostureDuration() / 3.03f), 0.25f, 1.8f);
#else
		vertScale = 1.50f;
#endif	
	}

	vertScale *= (2.0f - flMoraleFactor) * flStaminaFactor;
	horzScale *= (2.0f - flMoraleFactor) * flStaminaFactor;

	if( IsBayonetDeployed() )
	{
		horzScale += 0.05f;
		vertScale += 0.05f;
	}

	// TODO_KORNEEL This causes a sudden jump. Make smoother
	const float speedModifier = flMoraleFactor < 0.75 ? 2.0f : 1.0f;
	const float flTime = pPlayer->GetTimeBase() * speedModifier;

	QAngle offset(0, 0, 0);
	offset[YAW] =	horzScale * (sin(flTime * 2.35f) * horzMove * flMovementModifier);
	offset[PITCH] =	vertScale * (sin(flTime * 0.85f) * vertMove * flMovementModifier);

	pPlayer->AdjustWeaponSway( offset );
}

#ifndef CLIENT_DLL
void CWeaponRnLBallisticBase::UpdatePostureFoV()
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;
	
	int iPosition = pPlayer->GetWeaponPosture();

	// Also: set a new fov
	SetNextFoV( GetRnLWpnData().m_flFieldOfView[iPosition] );
}
#endif

bool CWeaponRnLBallisticBase::StartSprinting( void )
{
	// Do we have a valid owner holding the weapon? 
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return false; 

	if( m_bInReload || m_iReloadState != 0 )
	{
		return false;
	}

	ReturnToDefaultPosture();

	return true;
}

bool CWeaponRnLBallisticBase::CheckChamber()
{
	// Make an instance of the local player to deal with
	CRnLPlayer *pPlayer = GetPlayerOwner();
	
	// This happens when the player hasnt got any ammo left
	// If this happens we dont need to play any animations at all
	if (pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0)
		return false;

	/*if( Clip1() == GetMaxClip1() )
		return false;*/

	// We shall only reload when the whole clip is spend..on..peace making
	//else if( m_iClip1 == 0 )
	//	return true;
	//else
	//	return false;
	return true;
}

// If we try to reload in other positions than shoulder, play some transitions first
// before we do the actual reload and move it back into it's old position
// using the nextpos and prev pos functions would mess the iPosture while we're in that loop
// and play the wrong animations - Stefan
void CWeaponRnLBallisticBase::ReloadTransition( int iState )
{
	bool bReloadSuccess = false;
	m_bInReload = true;
	bool bAnimPlay = false;

	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	int iPosture = pPlayer->GetWeaponPosture();

	switch ( iPosture )
	{
	case WEAPON_POSTURE_HIP: // This is the transition code for the hip
		{
		switch( iState )
			{		
				case 1:
				{
					if( !CheckChamber() )
					{
						// and reset the reload animstate
						m_iReloadState = 0;
						m_bInReload = false;
						return;
					}
					else
					{
#ifndef CLIENT_DLL
						// Switch back to shoulder fov first
						SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_SHOULDER] );
#endif
						HandleViewAnimation( WEAPON_ANIMATION_HIP_TO_SHOULDER );
						bAnimPlay = true;
						m_iReloadState = 2;
						pPlayer->SetMaxSpeed( 99.0f );
						
					}
					break;
				}
				case 2:
				{
					m_iReloadState = 3;
					bReloadSuccess = Reload();
					bAnimPlay = true;
					break;
				}
				case 3:
				{
					HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_HIP );
					bAnimPlay = true;
					pPlayer->SetWeaponPosture( WEAPON_POSTURE_HIP );
					m_iReloadState = 0;
#ifndef CLIENT_DLL
					UpdatePostureFoV();
#endif
					m_bInReload = false;
					pPlayer->SetMaxSpeed( 250.0f );
					break;
				}
			}
			break;
		}
	case WEAPON_POSTURE_SHOULDER: // No transition needed for shoulder
		{
			if( m_iReloadState == 2 )
			{
				bReloadSuccess = Reload();
				bAnimPlay = true;
				m_iReloadState++;
			}
			else if( m_iReloadState == 3 )
			{
				m_bInReload = false;
				m_iReloadState = 0;
				pPlayer->SetMaxSpeed( 250.0f );
			}
			else
			{
				if( !CheckChamber() )
				{
					// and reset the reload animstate
					m_iReloadState = 0;
					m_bInReload = false;
					return;
				}
				else
				{
					pPlayer->SetMaxSpeed( 99.0f );
					m_iReloadState++;
				}
			}
			break;
		}
	case WEAPON_POSTURE_IRONSIGHTS: // but for sight we need one again
		{
			switch( iState )
			{		
				case 1:
				{
					// Don't start anything if there's no reason to reload
					if( !CheckChamber() )
					{
						// and reset the reload animstate
						m_iReloadState = 0;
						m_bInReload = false;
						return;
					}
					else
					{
						ReturnToDefaultPosture();
						pPlayer->SetMaxSpeed( 99.0f );
					}
					break;
				}
				case 2:
				{
					m_iReloadState = 3;
					bReloadSuccess = Reload();
					break;
				}
				// The transition that moves the weapon back to IS doesnt need an open act since its been reloaded
				case 3:
				{
					HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
					bAnimPlay = true;
					m_iReloadState = 0;
#ifndef CLIENT_DLL
					// Switch back to IS fov
					UpdatePostureFoV();
#endif
					m_bInReload = false;
					pPlayer->SetMaxSpeed( 250.0f );
					break;
				}
			}
			break;
		}
	case WEAPON_POSTURE_SUPERSIGHTS: // but for sight we need one again
		{
			switch( iState )
			{		
				case 1:
				{
					// Don't start anything if there's no reason to reload
					if( !CheckChamber() )
					{
						// and reset the reload animstate
						m_iReloadState = 0;
						m_bInReload = false;
						return;
					}
					else
					{
						ReturnToDefaultPosture();
						pPlayer->SetMaxSpeed( 99.0f );
					}
					break;
				}
				case 2:
				{
					m_iReloadState = 3;
					bReloadSuccess = Reload();
					break;
				}
				// The transition that moves the weapon back to IS doesnt need an open act since its been reloaded
				case 3:
				{
					HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
					bAnimPlay = true;
					m_iReloadState = 0;
#ifndef CLIENT_DLL
					UpdatePostureFoV();
#endif
					m_bInReload = false;
					pPlayer->SetMaxSpeed( 250.0f );
					break;
				}
			}
			break;
		}
	}

	if( bReloadSuccess || bAnimPlay )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
}

void CWeaponRnLBallisticBase::WeaponIdle( void )
{
#ifdef CLIENT_DLL
	HandleViewSway();
#endif

	if ( IsViewModelSequenceFinished() )
	{
		HandleViewAnimation( WEAPON_ANIMATION_IDLE );
	}
}

void CWeaponRnLBallisticBase::HandleFoVTransitions( void )
{
#ifndef CLIENT_DLL
	if( m_iNextFov != m_iCurrentFov )
	{
		CRnLPlayer *pPlayer = GetPlayerOwner();
		if( pPlayer )
		{
			pPlayer->SetFOV( pPlayer, m_iNextFov, m_flNextPrimaryAttack - gpGlobals->curtime );
		}
		m_iCurrentFov = m_iNextFov;
	}
#endif
}

bool CWeaponRnLBallisticBase::HandleReloadTransitions( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return false;

	if( m_bInReload )
	{
		if ( pPlayer && gpGlobals->curtime >= m_flNextPrimaryAttack )
			ReloadTransition( m_iReloadState );

		return true;
	}

	return false;
}

bool CWeaponRnLBallisticBase::HandlePostureTransitions( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer ) 
		return false;

	if( m_flNextPrimaryAttack > gpGlobals->curtime )
		return false;

	if( !(pPlayer->m_nButtons & IN_IRONSIGHTS) )
		return OnIronsightsUnheld();
	else if( pPlayer->m_nButtons & IN_IRONSIGHTS )
		return OnIronsightsHeld();

	return false;
}

#define IRONSIGHTS_CLICK_TIME 0.25f

bool CWeaponRnLBallisticBase::OnIronsightsUnheld( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return false;

	int iPosture = pPlayer->GetWeaponPosture();
	if( iPosture == WEAPON_POSTURE_IRONSIGHTS && m_flIronsightsPressedTime != 0.0f )
	{
		m_flIronsightsPressedTime = 0.0f;
		PrevFirePos();
		return true;
	}
	else if( iPosture == WEAPON_POSTURE_SUPERSIGHTS )
	{
		m_flIronsightsPressedTime = 0.0f;
		PrevFirePos();
		return true;
	}
	return false;
}

bool CWeaponRnLBallisticBase::OnIronsightsHeld( void )
{
	if( m_flIronsightsPressedTime == 0.0f )
		m_flIronsightsPressedTime = gpGlobals->curtime;

	float heldTime = gpGlobals->curtime - m_flIronsightsPressedTime;

	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return false;

	int iPosture = pPlayer->GetWeaponPosture();
	if( iPosture == WEAPON_POSTURE_SHOULDER )
	{
		m_flIronsightsPressedTime = 0.0f;
		NextFirePos();
		return true;
	}
	else if( iPosture == WEAPON_POSTURE_IRONSIGHTS && heldTime > IRONSIGHTS_CLICK_TIME )
	{
		NextFirePos();
		return true;
	}
	return false;
}

// dont let it holster if in IS
bool CWeaponRnLBallisticBase::CanHolster( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return true;

	return BaseClass::CanHolster() && !m_bInReload;
}

void CWeaponRnLBallisticBase::GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if( !pPlayer )
		return;

	int iPosture = pPlayer->GetWeaponPosture();
	if( iPosture  == WEAPON_POSTURE_SHOULDER )
	{
		maxExtents.x = 16.0f;
		maxExtents.y = 10.0f;

		deadZone.x = 6.0f;
		deadZone.y = 3.0f;
	}
	else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS )
	{
		if( pPlayer->IsProne() )
		{
			lockToExtents = AIM_LOCK_SLOWDOWN;

			maxExtents.x = 16.0f;
			maxExtents.y = 15.0f;

			deadZone.x =  6.0f;
			deadZone.y = 4.5f;
		}
		else
		{
			maxExtents.x = 10.0f;
			maxExtents.y = 6.0f;

			deadZone.x =  3.0f;
			deadZone.y = 1.5f;
		}
	}
	else if( iPosture == WEAPON_POSTURE_SUPERSIGHTS )
	{
		if( pPlayer->IsProne() ) 
		{
			lockToExtents = AIM_LOCK_LIMIT;

			maxExtents.x = 40.0f;
			maxExtents.y = 35.0f;

			deadZone.x =  3.0f;
			deadZone.y = 1.5f;
		}
	}
}

//=============================================================================//
// CWeaponRnLBallisticBase::NextFirePos( )
// Purpose: Sets the next weapon position, and call its respective animation
//
//=============================================================================//
void CWeaponRnLBallisticBase::NextFirePos()
{
	// Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer )
		return; 

	int iPosition = pPlayer->GetWeaponPosture();

	switch(iPosition)
	{
	case WEAPON_POSTURE_HIP:
		iPosition = WEAPON_POSTURE_SHOULDER;
		HandleViewAnimation( WEAPON_ANIMATION_HIP_TO_SHOULDER );
		break;
	case WEAPON_POSTURE_SHOULDER:
		iPosition = WEAPON_POSTURE_IRONSIGHTS;
		HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
		break;
	case WEAPON_POSTURE_IRONSIGHTS:
		iPosition = WEAPON_POSTURE_SUPERSIGHTS; //Should be 0 later, when hip has been added
		break;
	case WEAPON_POSTURE_SUPERSIGHTS:
		iPosition = WEAPON_POSTURE_SHOULDER;
		HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );
		break;
	}
	
	pPlayer->SetWeaponPosture( iPosition );
	if( iPosition == WEAPON_POSTURE_SUPERSIGHTS )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
#ifndef CLIENT_DLL
	UpdatePostureFoV();
#endif
}

//=============================================================================//
// CWeaponRnLBallisticBase::PrevFirePos( )
// Purpose: Sets the next weapon position, and call its respective animation
//
//=============================================================================//
void CWeaponRnLBallisticBase::PrevFirePos()
{
// Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	int iPosition = pPlayer->GetWeaponPosture();

	switch(iPosition)
	{
	case WEAPON_POSTURE_HIP:
	//	iPosition = 1;
	//	HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );
		return;
		break;
	case WEAPON_POSTURE_SHOULDER:
		iPosition = WEAPON_POSTURE_SUPERSIGHTS;
		HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
		break;
	case WEAPON_POSTURE_IRONSIGHTS:
		iPosition = WEAPON_POSTURE_SHOULDER;
		HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );
		break;
	case WEAPON_POSTURE_SUPERSIGHTS:
		iPosition = WEAPON_POSTURE_IRONSIGHTS;
		break;
	}

	pPlayer->SetWeaponPosture( iPosition );

	if( iPosition != WEAPON_POSTURE_IRONSIGHTS )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;
	}
#ifndef CLIENT_DLL
	UpdatePostureFoV();
#endif
}

void CWeaponRnLBallisticBase::ReturnToDefaultPosture( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;
	int iPosture = pPlayer->GetWeaponPosture();

	if( iPosture > GetDefaultPosture() )
	{
		for( int i = 0; i < WEAPON_POSTURE_MAX && ( iPosture > GetDefaultPosture() ); i++ )
		{
			PrevFirePos();
			iPosture = pPlayer->GetWeaponPosture();
		}
	}
	else if( iPosture < GetDefaultPosture() )
	{
		for( int i = 0; i < WEAPON_POSTURE_MAX && ( iPosture < GetDefaultPosture() ); i++ )
		{
			NextFirePos();
			iPosture = pPlayer->GetWeaponPosture();
		}
	}
}

void CWeaponRnLBallisticBase::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;
	int iPosture = pPlayer->GetWeaponPosture();
	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;
	int randomNumber = (rand() % 3 );

	SetAnimationState( iAnim );

	switch( iAnim )
	{
	case WEAPON_ANIMATION_ATTACK:
		if( iPosture == WEAPON_POSTURE_HIP )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_HIPFIREOPEN );
			}
			else
			{
				if( randomNumber == 2 )
				{
					SendWeaponAnim( ACT_VM_HIPFIRE3 );
				}
				else if( randomNumber == 1 )
				{
					SendWeaponAnim( ACT_VM_HIPFIRE2 );
				}
				else
				{
					SendWeaponAnim( ACT_VM_HIPFIRE1 );
				}
			}
		}
		else if( iPosture  == WEAPON_POSTURE_SHOULDER )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SHOULDERFIREOPEN );
			}
			else
			{
				if( randomNumber == 2 )
				{
					SendWeaponAnim( ACT_VM_SHOULDERFIRE3 );
				}
				else if( randomNumber == 1 )
				{
					SendWeaponAnim( ACT_VM_SHOULDERFIRE2 );
				}
				else
				{
					SendWeaponAnim( ACT_VM_SHOULDERFIRE1 );
				}
			}
		}
		else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SIGHTFIREOPEN );
			}
			else
			{
				if( randomNumber == 2 )
				{
					SendWeaponAnim( ACT_VM_SIGHTFIRE3 );
				}
				else if( randomNumber == 1 )
				{
					SendWeaponAnim( ACT_VM_SIGHTFIRE2 );
				}
				else
				{
					SendWeaponAnim( ACT_VM_SIGHTFIRE1 );
				}
			}
		}
		break;

	case WEAPON_ANIMATION_ATTACK2:
		if( iPosture == WEAPON_POSTURE_HIP )
		{
			SendWeaponAnim( ACT_VM_SECSHOULDERFIRE1 );	
		}
		else if( iPosture  == WEAPON_POSTURE_SHOULDER )
		{
			SendWeaponAnim( ACT_VM_SECSHOULDERFIRE1 );
		}
		else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
		{
			SendWeaponAnim( ACT_VM_SECSHOULDERFIRE1 );
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
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_HIPDRAWOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_HIPDRAW );
			}
		}
		else if( iPosture == WEAPON_POSTURE_SHOULDER )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SHOULDERDRAWOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SHOULDERDRAW );
			}
		}
		else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SIGHTDRAWOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SIGHTDRAW );
			}
		}
		break;

	case WEAPON_ANIMATION_HOLSTER:
		if( iPosture == WEAPON_POSTURE_HIP )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_HIPHOLSTEROPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_HIPHOLSTER );
			}
		}
		else if( iPosture == WEAPON_POSTURE_SHOULDER )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SHOULDERHOLSTEROPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );
			}
		}
		else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SIGHTHOLSTEROPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SIGHTHOLSTER );
			}
		}
		break;

	case WEAPON_ANIMATION_IDLE:
		if( GetPlayerOwner()  && GetPlayerOwner()->IsProne()
			&& GetPlayerOwner()->GetLocalVelocity().Length2D() > SPEED_PRONE_WALK )
		{
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_PRONE_IDLE_OPEN );
			else
				SendWeaponAnim( ACT_VM_PRONE_IDLE );

			SetAnimationState( WEAPON_ANIMATION_PRONE_IDLE );
		}			
		else if( GetPlayerOwner() && GetPlayerOwner()->IsSprinting() )
		{
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SPRINT_IDLE_OPEN );
			else
				SendWeaponAnim( ACT_VM_SPRINT_IDLE );

			SetAnimationState( WEAPON_ANIMATION_SPRINT_IDLE );
		}
		else if( iPosture == WEAPON_POSTURE_HIP )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_HIPIDLEOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_HIPIDLE );
			}
		}
		else if( iPosture == WEAPON_POSTURE_SHOULDER )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SHOULDERIDLEOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SHOULDERIDLE );
			}
		}
		else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( bMagEmpty )
			{
				SendWeaponAnim( ACT_VM_SIGHTIDLEOPEN );
			}
			else
			{
				SendWeaponAnim( ACT_VM_SIGHTIDLE );
			}
		}
		break;

	case WEAPON_ANIMATION_SHOULDER_TO_IS:
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_SHOULDER2SIGHTOPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_SHOULDER2SIGHT );
		}
		break;

	case WEAPON_ANIMATION_IS_TO_SHOULDER:
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_SIGHT2SHOULDEROPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_SIGHT2SHOULDER );
		}
		break;

	case WEAPON_ANIMATION_SHOULDER_TO_HIP:
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_SHOULDER2HIPOPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_SHOULDER2HIP );
		}
		break;

	case WEAPON_ANIMATION_HIP_TO_SHOULDER:
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_HIP2SHOULDEROPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_HIP2SHOULDER );
		}
		break;

	case WEAPON_ANIMATION_PRONE_ENTER:
		if( bMagEmpty )
			SendWeaponAnim( ACT_VM_PRONE_ENTER_OPEN );
		else
			SendWeaponAnim( ACT_VM_PRONE_ENTER );
		break;
	case WEAPON_ANIMATION_PRONE_LEAVE:
		if( bMagEmpty )
			SendWeaponAnim( ACT_VM_PRONE_LEAVE_OPEN );
		else
			SendWeaponAnim( ACT_VM_PRONE_LEAVE );
		break;

	case WEAPON_ANIMATION_SPRINT_ENTER:
		if( bMagEmpty )
			SendWeaponAnim( ACT_VM_SPRINT_ENTER_OPEN );
		else
			SendWeaponAnim( ACT_VM_SPRINT_ENTER );
		break;
	case WEAPON_ANIMATION_SPRINT_LEAVE:
		if( bMagEmpty )
			SendWeaponAnim( ACT_VM_SPRINT_LEAVE_OPEN );
		else
			SendWeaponAnim( ACT_VM_SPRINT_LEAVE );
		break;
	}
}

// Helper function to get viewmodel attachments

#ifdef CLIENT_DLL
extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );
#endif

void CWeaponRnLBallisticBase::GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir /*= NULL*/ )
{
	QAngle	angles;
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

#ifdef CLIENT_DLL
	if ( IsCarriedByLocalPlayer() )
	{
		if ( pOwner != NULL )
		{
			pOwner->GetViewModel()->GetAttachment( attachmentId, outVector, angles );
			::FormatViewModelAttachment( outVector, true );
		}
	}
	else
	{
		BaseClass::GetAttachment( attachmentId, outVector, angles );
	}
#else
	if ( pOwner != NULL )
		pOwner->GetActiveWeapon()->GetAttachment( attachmentId, outVector, angles );
#endif

	// Supply the direction, if requested
	if ( dir != NULL )
	{
		AngleVectors( angles, dir, NULL, NULL );
	}
}

float	g_lateralBob;

#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB_UP		0.5f

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CWeaponRnLBallisticBase::CalcViewmodelBob( void )
{
	static	float flBobTime;
	static	float flLastBobTime;
	float	flCycle;
	
	CRnLPlayer *pPlayer = ToRnLPlayer( GetOwner() );

	if ( ( !gpGlobals->frametime ) || ( pPlayer == NULL ) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the flSpeed of the player
	float flSpeed = clamp( pPlayer->GetLocalVelocity().Length2D(), 30, 320 );
	float flBobOffset = RemapVal( flSpeed, 0, 320, 0.0f, 1.0f );
	
	flBobTime += ( gpGlobals->curtime - flLastBobTime ) * flBobOffset;
	flLastBobTime = gpGlobals->curtime;

	//Calculate the lateral bob
	flCycle = flBobTime - (int)(flBobTime/HL2_BOB_CYCLE_MAX*2)*HL2_BOB_CYCLE_MAX*2;
	flCycle /= HL2_BOB_CYCLE_MAX*2;

	if ( flCycle < HL2_BOB_UP )
		flCycle = M_PI * flCycle / HL2_BOB_UP;
	else
		flCycle = M_PI + M_PI*(flCycle-HL2_BOB_UP)/(1.0 - HL2_BOB_UP);

	if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
		g_lateralBob = flSpeed*0.00075f;
	else
		g_lateralBob = flSpeed*0.0035f;

	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(flCycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );
	
	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CWeaponRnLBallisticBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector right;
	AngleVectors( angles, NULL, &right, NULL );

	CalcViewmodelBob();

	angles[YAW] -= g_lateralBob * 0.3f;

	VectorMA( origin, g_lateralBob * 0.65f, right, origin );
}