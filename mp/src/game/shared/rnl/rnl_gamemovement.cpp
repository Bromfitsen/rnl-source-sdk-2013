//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "rnl_gamerules.h"
#include "rnl_shareddefs.h"
#include "in_buttons.h"
#include "movevars_shared.h"
#include "weapons/weapon_rnlballisticbase.h"
#include "env_wind_shared.h"
#include "decals.h"

#ifdef CLIENT_DLL
	// For showing/hiding the map
	#include <game/client/iviewport.h>
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
#endif

#include "tier0/vprof.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/memdbgon.h"

extern bool g_bMovementOptimizations;

struct surfacedata_t;

class CBasePlayer;

#define TIME_TO_ROLL 2.5f

class CRnLGameMovement : public CGameMovement
{
public:
	DECLARE_CLASS( CRnLGameMovement, CGameMovement );

	CRnLGameMovement();

protected:
	void			PlayerMove(	void );

	// Handle MOVETYPE_WALK.
	void			FullWalkMove();

	void			StaminaThink();
	void			StaminaRecoveringState( float flRecovTime, float& flRecoverAmt, float& flMoveSpeed );
	void			StaminaNormalState( float& flRecoverAmt, float& flMoveSpeed );

	void			HandleLeaning();

	virtual void	FullLadderMove( void );
	//BB this is climbing mumbo-jumbo
	// Handle MOVETYPE_CLIMBING.
	// Can't be bothered using/creating a new MOVETYPE so custom is used
	virtual void	FullClimbMove( void );
	virtual	bool	ClimbingMove( void );
	bool			FindClimbEndPosition( bool &toDuck );

	bool			CanUnprone( bool bToStand );

protected:
	const Vector& GetPlayerViewOffset() const;
	void DuckAndProne(void);
	bool CanProne() const;
};

// ---------------------------------------------------------------------------------------- //
// CRnLGameMovement.
// ---------------------------------------------------------------------------------------- //

CRnLGameMovement::CRnLGameMovement()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : ducked - 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CRnLGameMovement::GetPlayerViewOffset() const
{
	// First cast us a player
	CRnLPlayer* pRnLPlayer = ToRnLPlayer(player);
	// Preferably a valid one, too
	if (!pRnLPlayer)
	{
		return CGameMovement::GetPlayerViewOffset(false);
	}

	if (pRnLPlayer->IsProne())
		return VEC_PRONE_VIEW;

	if (pRnLPlayer->IsDucked()) {

		if ((pRnLPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pRnLPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS)
			|| mv->m_vecVelocity.Length() > 0.5f)
		{
			return VEC_DUCK_VIEW_MOVING;
		}
		else
		{
			return VEC_DUCK_VIEW;
		}
	}

	return VEC_VIEW;
}

bool CRnLGameMovement::CanProne() const
{
	int i;
	trace_t trace;
	Vector newOrigin;
	VectorCopy(mv->GetAbsOrigin(), newOrigin);

	if (player->GetGroundEntity() != NULL)
	{
		for (i = 0; i < 3; i++)
		{
			newOrigin[i] += (VEC_HULL_MIN[i] - VEC_PRONE_HULL_MIN[i]);
		}
	}

	Ray_t ray;
	ray.Init(mv->GetAbsOrigin(), newOrigin, VEC_PRONE_HULL_MIN, VEC_PRONE_HULL_MAX);
	UTIL_TraceRay(ray, MASK_PLAYERSOLID, mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

	if (trace.startsolid || (trace.fraction != 1.0f))
		return false;

	return true;
}

void CRnLGameMovement::DuckAndProne(void)
{
	// First cast us a player
	CRnLPlayer* pRnLPlayer = ToRnLPlayer(player);
	// Preferably a valid one, too
	if (!pRnLPlayer)
		return;

	if (!pRnLPlayer->IsAlive())
		return;

	if (pRnLPlayer->IsDeployed())
		return;

	// Determine new posture target
	RnLMovementPostures_t iPostureTarget = MOVEMENT_POSTURE_NONE;

	if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT)
	{
		if (pRnLPlayer->GetMovementPostureDuration() >= TIME_TO_ROLL)
			iPostureTarget = MOVEMENT_POSTURE_PRONE;
		else
		{
			mv->m_flForwardMove = 0.0;
			mv->m_flSideMove = -100;
		}
	}
	else if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT)
	{
		if (pRnLPlayer->GetMovementPostureDuration() >= TIME_TO_ROLL)
			iPostureTarget = MOVEMENT_POSTURE_PRONE;
		else
		{
			mv->m_flForwardMove = 0.0;
			mv->m_flSideMove = 100;
		}
	}
	else if (((mv->m_nButtons & IN_LEAN_LEFT) || (mv->m_nButtons & IN_LEAN_RIGHT)) && pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE && pRnLPlayer->GetMovementPostureDuration() > TIME_TO_ROLL && mv->m_vecVelocity.Length() < 0.5f)
	{
		CWeaponRnLBallisticBase* pBallisticWep = dynamic_cast<CWeaponRnLBallisticBase*>(player->GetActiveWeapon());

		if (!pBallisticWep || (pBallisticWep && !pBallisticWep->IsMachineGun()))
		{
			float curStamina = pRnLPlayer->GetStamina();
			if (mv->m_nButtons & IN_LEAN_LEFT)
			{
				if (curStamina > 10)
				{
					if (pRnLPlayer)
					{
						pRnLPlayer->SetStamina(curStamina - 10);
						pRnLPlayer->DoAnimationEvent(PLAYERANIMEVENT_ROLLLEFT);
					}

					pRnLPlayer->SetMovementPosture(MOVEMENT_POSTURE_PRONE_ROLL_LEFT);

					return;
				}
			}
			else if (mv->m_nButtons & IN_LEAN_RIGHT)
			{
				if (curStamina > 10)
				{
					if (pRnLPlayer)
					{
						pRnLPlayer->SetStamina(curStamina - 10);
						pRnLPlayer->DoAnimationEvent(PLAYERANIMEVENT_ROLLRIGHT);
					}

					pRnLPlayer->SetMovementPosture(MOVEMENT_POSTURE_PRONE_ROLL_RIGHT);

					return;
				}
			}
		}
	}
	else if (pRnLPlayer->GetGroundEntity() != NULL)
	{
		bool tryProne = (pRnLPlayer->m_afButtonPressed & IN_PRONE) ? (pRnLPlayer->IsDucked()) : (false);
		bool tryCrouch = (pRnLPlayer->m_afButtonPressed & IN_DUCK) ? (pRnLPlayer->IsStanding()) : ((pRnLPlayer->m_afButtonPressed & IN_JUMP) ? (pRnLPlayer->IsProne()) : (false));
		bool tryStand = (pRnLPlayer->m_afButtonPressed & IN_JUMP) ? (pRnLPlayer->IsDucked()) : (false);

		// Stand up
		if (tryStand)
		{
			if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE && CanUnprone(true))
				iPostureTarget = MOVEMENT_POSTURE_STAND;
			else if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CROUCH && CanUnduck())
				iPostureTarget = MOVEMENT_POSTURE_STAND;
		}
		// Prone, except stand if in or going to prone already
		else if (tryProne)
		{
			if (CanProne())
			{
				// Do a prone dive if we are in the right circumstances
				if (pRnLPlayer->GetAbsVelocity().Length() > SPEED_CROUCH_WALK)
				{
					Vector vecForward;
					AngleVectors(mv->m_vecViewAngles, &vecForward);
					vecForward.z = 0;
					VectorNormalize(vecForward);

					for (int iAxis = 0; iAxis < 2; ++iAxis)
						vecForward[iAxis] *= 5.0;

					VectorAdd(vecForward, mv->m_vecVelocity, mv->m_vecVelocity);
					SetGroundEntity(NULL);
					mv->m_vecVelocity[2] += 250;

					if (pRnLPlayer)
						pRnLPlayer->DoAnimationEvent(PLAYERANIMEVENT_PRONEGETDOWN);

					pRnLPlayer->SetMovementPosture(MOVEMENT_POSTURE_PRONE_DIVE);
					pRnLPlayer->ViewPunch(QAngle(25, 0, 0));

					return;
				}
				else
				{
					iPostureTarget = MOVEMENT_POSTURE_PRONE;
				}
			}
		}
		// Crouch, except stand if
		// - Not duck toggled and no longer holding the crouch button
		// OR
		// - Ducked toggled and in or going to crouch already
		// Also, if prone diving, change to prone stance
		else if (tryCrouch)
		{
			if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE && CanUnprone(false))
				iPostureTarget = MOVEMENT_POSTURE_CROUCH;
			else if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_STAND)
				iPostureTarget = MOVEMENT_POSTURE_CROUCH;
		}
	}

	int iLastPosture = pRnLPlayer->GetMovementPosture();

	// update posture targetiPostureTarget
	if (iPostureTarget != MOVEMENT_POSTURE_NONE)
		pRnLPlayer->SetMovementPosture(iPostureTarget);
	else
		iPostureTarget = pRnLPlayer->GetMovementPosture();

	// Handle animations
	if (iPostureTarget != iLastPosture)
	{
		if (iLastPosture == MOVEMENT_POSTURE_PRONE && pRnLPlayer->IsSequenceFinished())
			pRnLPlayer->DoAnimationEvent(PLAYERANIMEVENT_PRONECANCEL);
		else if (iLastPosture != MOVEMENT_POSTURE_PRONE_DIVE)
			pRnLPlayer->DoAnimationEvent(PLAYERANIMEVENT_PRONE);
	}

	// Target offset
	float flTarget = GetPlayerViewOffset().z;
	pRnLPlayer->SetMovementPostureTarget(flTarget);

	// Modify stance offset
	float flOffset = pRnLPlayer->GetMovementPostureOffset();
	float flCoefficient = 25.0f;
	static bool bSlowPosture = false;

	if (iPostureTarget == MOVEMENT_POSTURE_PRONE)
	{
		bSlowPosture = false;
		flCoefficient = 10.0f;
	}
	else if (iPostureTarget == MOVEMENT_POSTURE_STAND && (iLastPosture == MOVEMENT_POSTURE_PRONE || bSlowPosture))
	{
		bSlowPosture = true;
		flCoefficient = 6.5f;
	}
	else
		bSlowPosture = false;

	float flForce = flCoefficient * (flTarget - flOffset) - 6.0f * pRnLPlayer->GetMovementPostureVelocity();
	float flVelocity = pRnLPlayer->GetMovementPostureVelocity() + (flForce * TICK_INTERVAL);

	if ((fabsf(flVelocity) < 0.01f) && (fabsf(flTarget - flOffset) < 0.01f))
	{
		pRnLPlayer->SetMovementPostureVelocity(0.0f);
		pRnLPlayer->SetMovementPostureOffset(flTarget);

		return;
	}

	pRnLPlayer->SetMovementPostureVelocity(flVelocity);

	flOffset += flVelocity * TICK_INTERVAL;
	pRnLPlayer->SetMovementPostureOffset(flOffset);

	// Modify player view
	Vector vecView = pRnLPlayer->GetViewOffset();
	vecView.z = pRnLPlayer->GetMovementPostureOffset();

	if (vecView.z < 10)
		vecView.z = 10 - (10 - vecView.z) * 0.5f;
	if (vecView.z < 7)
		vecView.z = 7;

	pRnLPlayer->SetViewOffset(vecView);
}

//-----------------------------------------------------------------------------
// Handle the entire stamina stuff
//-----------------------------------------------------------------------------
void CRnLGameMovement::StaminaThink()
{
	// First cast us a player
	CRnLPlayer *pRnLPlayer = ToRnLPlayer( player );
	// Preferably a valid one, too
	if( !pRnLPlayer )
		return;

	if( !pRnLPlayer->IsAlive() )
		return;

	if( pRnLPlayer->GetTeamNumber() != TEAM_AXIS && pRnLPlayer->GetTeamNumber() != TEAM_ALLIES )
	{
		mv->m_flMaxSpeed  = 250;
		return;
	}

	if( pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT || pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT )
		return;

	float speed = SPEED_RUN;
	float recoverAmt = 0.0f;
	float curStamina = pRnLPlayer->GetStamina();
	float recovTime = ( pRnLPlayer->GetRecoverTime() - gpGlobals->curtime );

	if( recovTime > 0.0f )
	{
		if( pRnLPlayer->IsSprinting() )
		{
			pRnLPlayer->StopSprinting();
		}

		StaminaRecoveringState( recovTime, recoverAmt, speed );
		
		if( curStamina < 40 )
		{
			if( pRnLPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
			{
				int pos = 8 + (curStamina / 5);
						
				pRnLPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pRnLPlayer->PlayStaminaSound( pos ) ) );
			}
		}
	}
	else
	{
		StaminaNormalState( recoverAmt, speed );
	}

	CWeaponRnLBase *pWeapon = pRnLPlayer->GetActiveRnLWeapon();

	if( curStamina < pRnLPlayer->GetMaxStamina() || recoverAmt < 0.0 )
	{
		curStamina += recoverAmt;

		if( pWeapon && (pWeapon->GetWeaponID() == WEAPON_AXISFISTS || pWeapon->GetWeaponID() == WEAPON_ALLIEDFISTS) )
			curStamina += 0.0075f;

		CWeaponRnLBase *pWeapon = pRnLPlayer->GetActiveRnLWeapon();

		if( pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM )
			curStamina = pRnLPlayer->GetMaxStamina();

		if( curStamina > 0 )
		{
			pRnLPlayer->SetStamina( curStamina );
		}
		else
		{
			float length = pRnLPlayer->GetSprintTimer();
			DevMsg( "Sprinted for %f seconds\n", length );
			length += (gpGlobals->curtime + 2.5f );
			pRnLPlayer->SetRecoverTime( length );
		}
	}

	if( (pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM) || (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT || pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT) )
	{
		mv->m_flMaxSpeed = speed;
	}
	else
	{
		//pRnLPlayer->SetMaxSpeed( speed );

		if( pRnLPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS )
		{
			if( !pRnLPlayer->IsProne() )
				speed *= 0.85f;
		}
		else if( pRnLPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( !pRnLPlayer->IsProne() )
				speed *= 0.55f;
		}

		if( pRnLPlayer->GetDamageBasedSpeedModifier( 1 ) > 0.0f && speed > 35.0f )
		{
			if ( mv->m_flForwardMove > 45 || mv->m_flSideMove > 45 )
				pRnLPlayer->PlayMovingWoundedSound();

			speed *= 1.0f - ( pRnLPlayer->GetDamageBasedSpeedModifier( 1 ) / 100.0f );
			speed = clamp( speed, SPEED_CROUCH_WALK, SPEED_SPRINT );
		}

		if( !pRnLPlayer->GetLeaningOffset().IsZero( 1.0 ) )
		{
			speed *= 0.70f;
		}

		if( speed > pRnLPlayer->MaxSpeed() )
		{
			speed = pRnLPlayer->MaxSpeed();
		}

		if( pRnLPlayer->GetWaterLevel() >= WL_Waist )
		{
			speed *= 0.45f;
			mv->m_flUpMove *= 0.65f;
			mv->m_flSideMove *= 0.65f;
		}

		if( mv->m_flSideMove != 0 ) // player half-strafing
		{
			speed *= 0.85f;
		}

		mv->m_flMaxSpeed = speed;
	}
}

void CRnLGameMovement::StaminaRecoveringState( float flRecovTime, float& flRecoverAmt, float& flMoveSpeed )
{
	CRnLPlayer* pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer )
		return;

	float recovTimeDec = 0.0f;
	float flMoraleFactor = (pRnLPlayer->GetMoraleLevel()/50.0f);

	if( flMoraleFactor < 0.01 )
	{
		flMoraleFactor = 0.01;
	}

	if( pRnLPlayer->IsDeployed() )
	{
		//MG Movement
		//if( (pRnLPlayer->m_nButtons & IN_SPEED) == 0 )
			flMoveSpeed = SPEED_NONE;
		//else
		//	flMoveSpeed = SPEED_DEPLOYED_ADJUST;
		flRecoverAmt = STAMINA_RECOVER_PRONE_WALK * flMoraleFactor;
	}
	else if ( pRnLPlayer->IsDucked() )
	{
		if( mv->m_nButtons & IN_WALK )
		{
			flMoveSpeed = SPEED_CROUCH_WALK;
			flRecoverAmt = STAMINA_RECOVER_CROUCH_WALK * flMoraleFactor;
		}
		else
		{
			flMoveSpeed = SPEED_CROUCH_RUN;
			flRecoverAmt = STAMINA_RECOVER_CROUCH_RUN * flMoraleFactor;
		}
		recovTimeDec = 0.04f;
	}
	else if ( pRnLPlayer->IsProne() )
	{
		if( mv->m_nButtons & IN_WALK )
		{
			flMoveSpeed = SPEED_PRONE_WALK;
			flRecoverAmt = STAMINA_RECOVER_PRONE_WALK * flMoraleFactor;
		}
		else
		{
			flMoveSpeed = SPEED_PRONE_RUN;
			flRecoverAmt = STAMINA_RECOVER_PRONE_RUN * flMoraleFactor;
		}
		recovTimeDec = 0.05f;
	}
	else if( mv->m_nButtons & IN_WALK )
	{
		recovTimeDec = 0.02f;
		flMoveSpeed = SPEED_WALK;
		flRecoverAmt = STAMINA_RECOVER_WALK * flMoraleFactor;
	}
	else
	{
		float timer = pRnLPlayer->GetSprintTimer();

		if( flRecovTime >= 0.5f && timer >= 1.0f )
		{
			if( flRecovTime > timer )
			{
				flMoveSpeed = SPEED_RECOVER; 
			}
			else
			{
				float factor = abs(( flRecovTime - timer )) / timer;

				if( factor < 0.01f )
				{
					factor = 0.01f;
				}
				else if( factor > 1.0f )
				{
					factor = 1.0f;
				}

				flMoveSpeed = SPEED_RECOVER  + ( SPEED_RUN - SPEED_RECOVER )* factor;

				if( flMoveSpeed > SPEED_RUN )
				{
					flMoveSpeed = SPEED_RUN;
				}
				else if( flMoveSpeed < SPEED_RECOVER )
				{
					flMoveSpeed = SPEED_RECOVER;
				}
			}
		}
		else
		{
			flMoveSpeed = SPEED_RUN;

			if( timer != 0.0f )
			{
				pRnLPlayer->SetSprintTimer( 0.0f );
			}
		}

		flRecoverAmt = STAMINA_RECOVER  * flMoraleFactor;
	}

	if( pRnLPlayer->GetGroundEntity() == NULL )
		recovTimeDec = 0.0f;

	if( recovTimeDec > 0.0f  )
	{
		flRecovTime = pRnLPlayer->GetRecoverTime() - recovTimeDec;

		if( flRecovTime < (gpGlobals->curtime + 0.2 ) )
		{
			pRnLPlayer->SetSprintTimer( 0.0f );
		}
		
		pRnLPlayer->SetRecoverTime( flRecovTime );
	}
}

void CRnLGameMovement::StaminaNormalState( float& flRecoverAmt, float& flMoveSpeed )
{
	CRnLPlayer* pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer )
		return;

	bool shouldSprint = (( mv->m_flForwardMove > 0.0f ) && ( mv->m_nButtons & IN_SPEED ) 
		&& pRnLPlayer->GetLeaningOffset().IsZero( 1.0 ) && pRnLPlayer->GetWeaponPosture() != WEAPON_POSTURE_THROWING_GRENADE
		&& pRnLPlayer->GetAbsVelocity().Length2D() > 2.0f 
		/* Michael Lebson
		Don't let a player sprint with their map open, or when they are swimming.
		*/
		&& !( mv->m_nButtons & IN_MAP ) && pRnLPlayer->GetWaterLevel() < WL_Waist );

	float flMoraleFactor = (pRnLPlayer->GetMoraleLevel() / 65.0f);

	if( flMoraleFactor < 0.01 )
	{
		flMoraleFactor = 0.01;
	}

	if( pRnLPlayer->IsDeployed() )
	{
		shouldSprint = false;
		//MG Movement
		//if( (pRnLPlayer->m_nButtons & IN_SPEED) == 0 )
			flMoveSpeed = SPEED_NONE;
		//else
		//	flMoveSpeed = SPEED_DEPLOYED_ADJUST;
		flRecoverAmt = STAMINA_RECOVER_PRONE_WALK * flMoraleFactor;
	}
	else if( pRnLPlayer->IsDucked() )
	{
		//DevMsg( "Player is ducking or ducked.\n" );
		if( shouldSprint && pRnLPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f && pRnLPlayer->StartSprinting() )
		{
			flMoveSpeed = SPEED_CROUCH_SPRINT;
			flRecoverAmt = STAMINA_CROUCH_SPRINT / flMoraleFactor;
			pRnLPlayer->SetSprintTimer( ( pRnLPlayer->GetSprintTimer() + 0.01f ) );

			if( pRnLPlayer->GetStamina() < 40.0f )
			{
				flMoveSpeed = SPEED_CROUCH_RUN + ( (SPEED_CROUCH_SPRINT - SPEED_CROUCH_RUN ) * (pRnLPlayer->GetStamina() / 100) );
				if( pRnLPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
				{
					int pos = pRnLPlayer->GetStamina() / 5;
					
					pRnLPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pRnLPlayer->PlayStaminaSound( pos ) ) );
				}
			}
		}
		else
		{
			shouldSprint = false;
			flMoveSpeed = SPEED_CROUCH_RUN;
			flRecoverAmt = STAMINA_RECOVER_CROUCH_RUN * flMoraleFactor;
		}
	}
	else if(pRnLPlayer->IsProne()
		|| (pRnLPlayer->GetMovementPostureFrom() == MOVEMENT_POSTURE_PRONE && pRnLPlayer->GetMovementPostureDuration() < 0.5f) )
	{
		//DevMsg( "Player is proning or prone.\n" );
		//if( shouldSprint && !player->IsProning() && pRnLPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f /*&& pRnLPlayer->StartSprinting()*/ )
		//{
		//	/* Going from prone right into sprint, didn't turn out the I want it to be, would need special animations, so I'll leave that out of the game
		//	pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_PRONE_TO_STAND );
		//	pRnLPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONEGETUP );
		//	pRnLPlayer->ViewPunch( QAngle( 180, 200, 140 ) );*/

		//	flMoveSpeed = SPEED_PRONE_SPRINT;
		//	flRecoverAmt = STAMINA_PRONE_SPRINT  / flMoraleFactor;
		//	pRnLPlayer->SetSprintTimer( ( pRnLPlayer->GetSprintTimer() + 0.01f ) );

		//	if( pRnLPlayer->GetStamina() < 40.0f )
		//	{
		//		if( pRnLPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
		//		{
		//			int pos = pRnLPlayer->GetStamina() / 5;
		//			
		//			pRnLPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pRnLPlayer->PlayStaminaSound( pos ) ) );
		//		}
		//	}
		//}
		//else
		{
			shouldSprint = false;
			flMoveSpeed = SPEED_PRONE_RUN;
			flRecoverAmt = STAMINA_RECOVER_PRONE_RUN * flMoraleFactor;
		}
	}
	else
	{
		if( shouldSprint && pRnLPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f && pRnLPlayer->StartSprinting() )
		{
			flMoveSpeed = SPEED_SPRINT;
			flRecoverAmt = STAMINA_SPRINT / flMoraleFactor;
			pRnLPlayer->SetSprintTimer( ( pRnLPlayer->GetSprintTimer() + 0.01f ) );

			if( pRnLPlayer->GetStamina() < 40.0f )
			{
				flMoveSpeed = SPEED_RUN + ( (SPEED_SPRINT - SPEED_RUN ) * (pRnLPlayer->GetStamina() / 100) );
				if( pRnLPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
				{
					int pos = pRnLPlayer->GetStamina() / 5;
					
					pRnLPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pRnLPlayer->PlayStaminaSound( pos ) ) );
				}
			}
		}
		else 
		{
			shouldSprint = false;

			if( mv->m_nButtons & IN_WALK )
			{
				flMoveSpeed = SPEED_WALK;
				flRecoverAmt = STAMINA_RECOVER_WALK;
			}
			else
			{
				flMoveSpeed = SPEED_RUN;
				flRecoverAmt = STAMINA_RECOVER;
			}
		}
	}

	if( pRnLPlayer->IsSprinting() && !shouldSprint )
	{
		if( pRnLPlayer->GetSprintTimer() > 0.0f )
		{
			CWeaponRnLBase *pWeapon = pRnLPlayer->GetActiveRnLWeapon();

			if( pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM )
			{
				pRnLPlayer->SetRecoverTime( 1.0f );
			}
			else
			{
				float length = (pRnLPlayer->GetSprintTimer() / 4);
				length += (gpGlobals->curtime + 1.0f );
				if( length > 15.0f )
				{
					pRnLPlayer->SetRecoverTime( length );
				}
				else
				{
					pRnLPlayer->SetRecoverTime( (gpGlobals->curtime + 1.0f ) );
				}
			}
		}
	}
}

void CRnLGameMovement::HandleLeaning()
{
	CRnLPlayer *pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer )
		return;

	bool shouldLean = false;
	bool canLean = !( pRnLPlayer->IsProne() || pRnLPlayer->IsDeployed() || pRnLPlayer->IsPostureChanging( 5.0, 2.0 ) );
	int iLeanDir = 0;
	bool bLeanHit = false;

	if( canLean )
	{
		trace_t trace;
		UTIL_TraceHull(pRnLPlayer->EyePosition(), pRnLPlayer->EyePosition(), Vector( -10, -10, -10 ), Vector( 10, 10, 10 ),  MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &trace );

		if( !trace.DidHit() )
		{
			if( ( (mv->m_nButtons & IN_LEAN_RIGHT) || ( mv->m_nButtons & IN_LEAN_LEFT ) ) && canLean)
			{
				if( mv->m_nButtons & IN_LEAN_LEFT )
				{
					shouldLean = true;
					iLeanDir = -1;
				}
				else if(mv->m_nButtons & IN_LEAN_RIGHT)
				{
					shouldLean = true;
					iLeanDir = 1;
				}
			}
		}
		else
		{
			// omg I hit a wall, save me from the clipping headache
			bLeanHit = true;
			while ( trace.DidHit() )
			{
				float rollOffset = pRnLPlayer->GetViewRollOffset();
				Vector vecOffset = pRnLPlayer->GetLeaningOffset();
				
				vecOffset.z += 0.25f;

				if( vecOffset.z >= 0 )
				{
					pRnLPlayer->SetViewRollOffset( 0 );
					pRnLPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );
					return;
				}

				if( mv->m_nButtons & IN_LEAN_LEFT )
				{
					rollOffset  += 1.0f;
					vecOffset.y -= 1;
				}
				else if(mv->m_nButtons & IN_LEAN_RIGHT)
				{
					rollOffset  -= 1.0f;
                    vecOffset.y += 1;
				}

				pRnLPlayer->SetViewRollOffset( rollOffset );
				pRnLPlayer->SetLeaningOffset( vecOffset );
				
				if ( vecOffset.y > -1 && vecOffset.y < 1 )
				{
					pRnLPlayer->SetViewRollOffset( 0 );
					pRnLPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );
					return;
				}

				UTIL_TraceHull(pRnLPlayer->EyePosition(), pRnLPlayer->EyePosition(), Vector( -10, -10, -10 ), Vector( 10, 10, 10 ),  MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &trace );
			}
			return;
		}
	}

	if( !shouldLean )
	{
		bLeanHit = false;
		Vector vecOffset = pRnLPlayer->GetLeaningOffset();

		float rollOffset = pRnLPlayer->GetViewRollOffset();
		if( !vecOffset.IsZero() || ( rollOffset != 0 ))
		{
			if( vecOffset.y < 1 && vecOffset.y > -1 )
			{
				pRnLPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );

				if( rollOffset != 0 )
				{
					pRnLPlayer->SetViewRollOffset( 0 );
				}
			}
			else
			{
				if(vecOffset.y > 0 )
				{
					shouldLean = true;
					iLeanDir = 1;
				}
				else 
				{
					shouldLean = true;
					iLeanDir = -1;
				}
			}
		}
	}

	if( shouldLean && !bLeanHit )
	{
		if( iLeanDir != 0 )
		{
			Vector vecOffset = pRnLPlayer->GetLeaningOffset();
			vecOffset.y -= iLeanDir;

			int imax = 14;
			int imin = -14;

			if( pRnLPlayer->IsDucked() )
			{
				imax = 8;
				imin = -8;
			}


			if( vecOffset.y > imax )
			{
				vecOffset.y = imax;
			}
			else if( vecOffset.y < imin )
			{
				vecOffset.y = imin;
			}
			else
			{
				float  rollOffset = pRnLPlayer->GetViewRollOffset();
				if( vecOffset.y < -1.0 )
				{
					if( iLeanDir > 0 )
					{
						vecOffset.z -= 0.25f;
						rollOffset += 1.0f;
					}
					else
					{
						vecOffset.z += 0.25f;
						rollOffset -= 1.0f;
					}
				}
				else if( vecOffset.y > 1.0 )
				{
					if( iLeanDir > 0 )
					{
						vecOffset.z += 0.25f;
						rollOffset += 1.0f;
					}
					else
					{
						vecOffset.z -= 0.25f;
						rollOffset -= 1.0f;
					}
				}
				else
				{
					vecOffset.z = 0;
					rollOffset = 0.0f;
				}

				if ( rollOffset < 0.0 )
					rollOffset += 360.0;
				else if ( rollOffset > 360.0 )
					rollOffset -= 360.0;

				pRnLPlayer->SetViewRollOffset( rollOffset );
			}

			pRnLPlayer->SetLeaningOffset( vecOffset );
		}
	}
}

// call the prone function by attaching ourselves to the playermove baseclass function
void CRnLGameMovement::PlayerMove()
{
	VPROF( "CGameMovement::PlayerMove" );
	CRnLPlayer* pRnLPlayer = ToRnLPlayer( player );

	if( !pRnLPlayer )
		return;
	
	CheckParameters();
	
	bool bIsClimbing = (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING || pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );
	m_nOnLadder = (pRnLPlayer->GetMoveType() == MOVETYPE_LADDER) ? 1 : 0;

	// clear output applied velocity
	mv->m_outWishVel.Init();
	mv->m_outJumpVel.Init();

	MoveHelper( )->ResetTouchList();                    // Assume we don't touch anything

	ReduceTimers();

	AngleVectors (mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp );  // Determine movement angles

	// Always try and unstick us unless we are a couple of the movement modes
	if ( CheckInterval( STUCK ) )
	{
		if (pRnLPlayer->GetMoveType() != MOVETYPE_NOCLIP &&
			pRnLPlayer->GetMoveType() != MOVETYPE_NONE &&
			pRnLPlayer->GetMoveType() != MOVETYPE_ISOMETRIC &&
			pRnLPlayer->GetMoveType() != MOVETYPE_OBSERVER )
		{
			if ( CheckStuck() )
			{
				// Can't move, we're stuck
				return;  
			}
		}
	}

	// Now that we are "unstuck", see where we are (pRnLPlayer->GetWaterLevel() and type, pRnLPlayer->GetGroundEntity()).
	CategorizePosition();

	// Store off the starting water level
	m_nOldWaterLevel = pRnLPlayer->GetWaterLevel();

	if(pRnLPlayer->IsAlive() && (pRnLPlayer->GetTeamNumber() == TEAM_AXIS || pRnLPlayer->GetTeamNumber() == TEAM_ALLIES) )
	{
		// If we are not on ground, store off how fast we are moving down
		if ( pRnLPlayer->GetGroundEntity() == NULL )
		{
			//Not Climbing fall
			if( !bIsClimbing )
			{
				if(pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
				{
					pRnLPlayer->SetViewOffset( GetPlayerViewOffset() );
					mv->m_flSideMove = 0.0f;
					mv->m_flForwardMove = 0.0f;
					mv->m_flUpMove = 0.0f;

					Vector wind;
					GetWindspeedAtTime( gpGlobals->curtime, wind );

					if( wind[0] != 0 && wind[1] != 0 )
					{
						mv->m_vecVelocity[0] = Approach( wind[0],mv->m_vecVelocity[0], gpGlobals->frametime * 25.0f );
						mv->m_vecVelocity[1] = Approach( wind[1],mv->m_vecVelocity[1], gpGlobals->frametime * 25.0f );
					}
					else
					{
						float goalX = random->RandomFloat( -100.0, 100.0 );
						float goalY = random->RandomFloat( -100.0, 100.0 );
						
						if ((mv->m_vecVelocity[0] < 0 && goalX > 0) || (mv->m_vecVelocity[0] > 0 && goalX < 0))
							goalX *= -1;
						if ((mv->m_vecVelocity[1] < 0 && goalY > 0) || (mv->m_vecVelocity[1] > 0 && goalY < 0))
							goalY *= -1;

						mv->m_vecVelocity[0] = Approach( goalX, mv->m_vecVelocity[0], gpGlobals->frametime * 25.0f );
						mv->m_vecVelocity[1] = Approach( goalY, mv->m_vecVelocity[1], gpGlobals->frametime * 25.0f );
					}

					mv->m_vecVelocity[ 2 ] = pRnLPlayer->m_Local.m_flFallVelocity = min( (pRnLPlayer->m_Local.m_flFallVelocity  - mv->m_vecVelocity[ 2 ]), SPEED_PARA );

					if( SPEED_PARA > pRnLPlayer->m_Local.m_flFallVelocity )
					{
						pRnLPlayer->m_Local.m_flFallVelocity = SPEED_PARA;
						mv->m_vecVelocity[ 2 ] = SPEED_PARA;
					}
				}
				else
				{
					pRnLPlayer->m_Local.m_flFallVelocity = -mv->m_vecVelocity[ 2 ];
				}
			}
		}
		else if( pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
		{
			/* Michael Lebson
			If the player is on the ground but still parachuting,
			remove the parachute and make the player's weapon
			draw as if they are just pulling it out again.
			*/
		#ifndef CLIENT_DLL
			pRnLPlayer->RemoveEquipment(RNL_EQUIPMENT_PARACHUTE);
		#endif

			pRnLPlayer->GetActiveRnLWeapon()->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
		}
	}

	// added by nuke for test
	/*if( ( m_flPlantNextFootPrint < 0.0f ) && mv->m_vecVelocity.Length() > 60 )
	{
		PlantFootprint();
		m_flPlantNextFootPrint = mv->m_vecVelocity.Length() * 2.5f;
	}*/

	if( pRnLPlayer->IsAlive() && (pRnLPlayer->GetTeamNumber() == TEAM_AXIS || pRnLPlayer->GetTeamNumber() == TEAM_ALLIES) )
	{
		if( pRnLPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING )
		{
			pRnLPlayer->UpdateStepSound( pRnLPlayer->GetSurfaceData(), mv->GetAbsOrigin(), mv->m_vecVelocity );

			DuckAndProne();

			// Don't run ladder code if dead or on a train
			if ( !pRnLPlayer->pl.deadflag && !(pRnLPlayer->GetFlags() & FL_ONTRAIN) )
			{
				// If was not on a ladder now, but was on one before, 
				//  get off of the ladder

				// TODO: this causes lots of weirdness.
				//bool bCheckLadder = CheckInterval( LADDER );
				//if ( bCheckLadder || pRnLPlayer->GetMoveType() == MOVETYPE_LADDER )
				//{
					if( !LadderMove() )
					{
						if ( pRnLPlayer->GetMoveType() == MOVETYPE_LADDER )
						{
							// Clear ladder stuff unless player is dead or riding a train
							// It will be reset immediately again next frame if necessary
							pRnLPlayer->SetMoveType( MOVETYPE_WALK );
							pRnLPlayer->SetMoveCollide( MOVECOLLIDE_DEFAULT );
						}

						CWeaponRnLBase* pActiveWeapon = pRnLPlayer->GetActiveRnLWeapon();

						if( pActiveWeapon && !pActiveWeapon->IsWeaponVisible() )
						{
							pActiveWeapon->SetWeaponVisible(true);
							pActiveWeapon->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
						}

						m_nOnLadder = 0;
					}

				//}

				//RnL : BB : Climbing Handling
				if ( !ClimbingMove() && ( pRnLPlayer->GetMoveType() == MOVETYPE_CLIMBING ) )
				{
					// Clear climbing stuff unless player is dead or riding a train
					// It will be reset immediately again next frame if necessary
					pRnLPlayer->SetMoveType( MOVETYPE_WALK );
					pRnLPlayer->SetMoveCollide( MOVECOLLIDE_DEFAULT );
				}
			}
		}
	}

#ifndef CLIENT_DLL
	pRnLPlayer->m_flForwardMove = mv->m_flForwardMove;
	pRnLPlayer->m_flSideMove = mv->m_flSideMove;
#endif

	// Handle movement modes.
	switch (pRnLPlayer->GetMoveType())
	{
		case MOVETYPE_NONE:
			//RnL : BB : Climbing
			if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING ||
				pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH)
			{
				FullClimbMove();
			}
			break;

		case MOVETYPE_NOCLIP:
			FullNoClipMove( sv_noclipspeed.GetFloat(), sv_noclipaccelerate.GetFloat() );
			break;

		case MOVETYPE_FLY:
		case MOVETYPE_FLYGRAVITY:
			FullTossMove();
			break;

		case MOVETYPE_LADDER:
			FullLadderMove();
			break;

		case MOVETYPE_WALK:
			FullWalkMove();
			break;

		case MOVETYPE_ISOMETRIC:
			//IsometricMove();
			// Could also try:  FullTossMove();
			FullWalkMove();
			break;
			
		case MOVETYPE_OBSERVER:
			FullObserverMove(); // clips against world&players
			break;

		default:
			DevMsg( 1, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pRnLPlayer->GetMoveType(), pRnLPlayer->IsServer());
			break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Overwritten so we can get our Stamina stuff and Leaning :) 
//-----------------------------------------------------------------------------
void CRnLGameMovement::FullWalkMove( )
{
	HandleLeaning();
	StaminaThink();
	BaseClass::FullWalkMove();
}
	

void CRnLGameMovement::FullLadderMove()
{
	CRnLPlayer* pRnLPlayer = (CRnLPlayer*)player;

#ifdef CLIENT_DLL
	/*Michael Lebson
	Hide the map if they are on a ladder
	*/
	if( pRnLPlayer && pRnLPlayer->m_nButtons & IN_MAP )
		gViewPortInterface->ShowPanel( PANEL_MAP, false );
#endif

	/*Michael Lebson
	Holster the player's weapon while they are on the ladder,
	but only the first time.
	*/
	if( pRnLPlayer && pRnLPlayer->GetActiveRnLWeapon() && (m_nOnLadder == 0) )
		pRnLPlayer->GetActiveRnLWeapon()->Holster();

	// Mark that we are on a ladder so it doesn't try to holster
	// the weapon forever.
	m_nOnLadder = 1;

	BaseClass::FullLadderMove();
}


ConVar sv_showclimbspot("sv_showclimbspot", "0", FCVAR_REPLICATED, "Shows end position for climbing" );

//RnL : BB : Climbing
//Climbing yay
//This checks to see if we want to climb,
//If we do then we'll see if we CAN climb.
//If not then to bad, if we can then strap on the climbing shoes
bool CRnLGameMovement::ClimbingMove( void )
{
	CRnLPlayer *pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer )
		return false;

	CWeaponRnLBase* pWep = pRnLPlayer->GetActiveRnLWeapon();
	if( pWep && (pWep->GetAnimationState() == WEAPON_ANIMATION_RELOAD) )
		return false;

	bool bIsClimbing = (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING || pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );

	if( !bIsClimbing && !(mv->m_nButtons & IN_JUMP ) )
		return false;

	//Andrew; Removed.. what's the point in not allowing an MG to climb?
	//Climbing with a damn machine gun.
	//if( pRnLPlayer->Weapon_OwnsThisType( "weapon_mg42" ) )
	//	return false;

	if( !bIsClimbing )
	{
		bool climbToDuck = false;

		// Cancel if weapon too heavy
		if( pWep && !(pWep->CanClimb() ) )
			return false;

		// Cancel if not standing still
		if( mv->m_vecVelocity.IsZero() && pRnLPlayer->IsStanding()  )
		{ 
			// Cancel if no valid target position
			if( FindClimbEndPosition( climbToDuck ) == false )
				return false;
		}
		else
		{
			return false;
		}

		// Start climb
		if( climbToDuck )
			pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );
		else
			pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_CLIMBING );

		// Get climb height
		pRnLPlayer->m_iClimbheight = pRnLPlayer->m_RnLLocal.m_vecMovementPos.GetZ() - pRnLPlayer->GetAbsOrigin().z - 10;
		//DevMsg("Climb height: %i\n", pRnLPlayer->m_iClimbheight);
		
		// Holster for high climbs
		if( pWep && ( pRnLPlayer->GetClimbheight() >= 24 )/*LOWEST_CONTROLLED_CLIMB ) && pWep->CanHolster() */ )
		{
			pWep->HandleViewAnimation( WEAPON_ANIMATION_HOLSTER );
			//pWep->Holster();
		}

		pRnLPlayer->DoAnimationEvent( PLAYERANIMEVENT_CLIMB );
		pRnLPlayer->SetMoveType(MOVETYPE_NONE); // MOVETYPE_CLIMBING );
	}

	return true;
}

//RnL : BB : Climbing
//Handle the actual climbing
void CRnLGameMovement::FullClimbMove( void )
{
	CRnLPlayer *pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer )
		return;

#ifdef CLIENT_DLL
	/*Michael Lebson
	Hide the map if they are climbing
	*/
	if( pRnLPlayer->m_nButtons & IN_MAP )
		gViewPortInterface->ShowPanel( PANEL_MAP, false );
#endif

	// DEBUG VISUALIZATION	
	if ( sv_showclimbspot.GetBool() )
	{
		static float nextDraw = gpGlobals->curtime;
		if( nextDraw <= gpGlobals->curtime )
		{
			if(pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddBoxOverlay( pRnLPlayer->m_RnLLocal.m_vecMovementPos, Vector(-16,-16,0), Vector(16,16,45), QAngle( 0, 0, 0), 255,0,0,127, 4 );

#else
				// draw blue server impact markers
				NDebugOverlay::Box( pRnLPlayer->m_RnLLocal.m_vecMovementPos, Vector(-16,-16,0), Vector(16,16,45), 0,0,255,127, 4 );

#endif
			}
			else
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddBoxOverlay( pRnLPlayer->m_RnLLocal.m_vecMovementPos, Vector(-16,-16,0), Vector(16,16,72), QAngle( 0, 0, 0), 255,0,0,127, 4 );

#else
				// draw blue server impact markers
				NDebugOverlay::Box( pRnLPlayer->m_RnLLocal.m_vecMovementPos, Vector(-16,-16,0), Vector(16,16,72), 0,0,255,127, 4 );

#endif
			}

			nextDraw += 5;
		}
	}

	int height = pRnLPlayer->GetClimbheight();
	bool doneClimbing = false;
	bool cancelClimbing = false;

	// If the player has a gun, reset it to default posture
	// to cancel IS or SS.
	if( pRnLPlayer->GetActiveRnLWeapon() )
		pRnLPlayer->GetActiveRnLWeapon()->ReturnToDefaultPosture();

	doneClimbing = pRnLPlayer->GetCycle() >= 0.96;
	/*
	if( height < 24 )
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB16_TIME )
			doneClimbing = true;
	}
	else if( height >= 24 && height < 40 )
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB32_TIME )
			doneClimbing = true;
	}
	else if( height >= 40 && height < 56 )
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB48_TIME )
			doneClimbing = true;
	}
	else if( height >= 56 && height < 72 )
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB64_TIME )
			doneClimbing = true;
	}
	else if( height >= 72 && height < 88 )
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB80_TIME )
			doneClimbing = true;
	}
	else
	{
		if( pRnLPlayer->GetMovementPostureDuration() > GAMEMOVEMENT_CLIMB96_TIME )
			doneClimbing = true;
	}
	*/
	
	if ( !(mv->m_nButtons & IN_JUMP) && pRnLPlayer->GetCycle() < 0.75 )
	{
		// CANCEL CLIMB
		pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_STAND );

		if (pRnLPlayer->GetCycle() > 0.25 )
		{
			Vector playerPos = pRnLPlayer->GetAbsOrigin();
			playerPos.z += abs((pRnLPlayer->m_RnLLocal.m_vecMovementPos.GetZ()-playerPos.z) * (player->GetCycle() - 0.25));
			mv->SetAbsOrigin( playerPos );
		}
		
		cancelClimbing = true;
	}

	if( doneClimbing )
	{
		mv->SetAbsOrigin( pRnLPlayer->m_RnLLocal.m_vecMovementPos );
		if(pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH )
			pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_CROUCH );
		else
			pRnLPlayer->SetMovementPosture( MOVEMENT_POSTURE_STAND );
	}
	
	if( doneClimbing || cancelClimbing )
	{
		//if( height >= LOWEST_CONTROLLED_CLIMB )
		if( height >= 24 )
		{
			CWeaponRnLBase* pWep = pRnLPlayer->GetActiveRnLWeapon();
			if ( pWep /*&& pWep->CanDeploy()*/ )
			{
				pWep->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
				//pWep->Deploy();
			}
		}

		pRnLPlayer->DoAnimationEvent( PLAYERANIMEVENT_CLIMB ); // call this only to reset the climb cycle, for whatever reason the client would lag behind at the next climb otherwise
#ifdef CLIENT_DLL
		QAngle ang = pRnLPlayer->EyeAngles();
		ang[PITCH] = 0;
		ang[ROLL] = 0;
		pRnLPlayer->ResetPlayerView(ang);
#endif
	}
}

//RnL : BB : Climbing
//This finds us a valid spot to climb too
//if it doesnt find one then it'll damn well tell you so.
bool CRnLGameMovement::FindClimbEndPosition( bool &toDuck )
{
	CRnLPlayer* pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer)
		return false;

	pRnLPlayer->m_RnLLocal.m_vecMovementPos.Init();

	trace_t superTrace;

	Vector start = pRnLPlayer->GetAbsOrigin();
	//Hull testing that is slightly larger than the 
	//player so we don't have them getting stuck
	Vector playerStandMins = Vector( -18, -18, -2 );
	Vector playerStandMax = Vector( 20, 20, 85 );

	Vector playerCrouchMins = Vector( -18, -18, -2 );
	Vector playerCrouchMax = Vector( 20, 20, 50 );

	Vector vForward;
	AngleVectors(pRnLPlayer->GetLocalAngles(), &vForward );
	vForward.z = 0;
	VectorNormalize( vForward );
	
	//40 units just for good measure.
	Vector end = ( start + vForward * 32 );
	end.z += 10;

	Vector lineStart = start;
	Vector lineEnd = end - ( vForward * 12 );
	lineEnd.z += 4;

	UTIL_TraceLine( lineStart, lineEnd, MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

	// DEBUG STUFF
	if ( sv_showclimbspot.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddLineOverlay( lineStart, lineEnd, 255,0,0,127, 4 );
#else
		// draw blue server impact markers
		NDebugOverlay::Line( lineStart, lineEnd, 0,0,255,127, 4 );
#endif
	}
		
	if( !( superTrace.DidHit() ) )
	{
		//Could be a fence
		UTIL_TraceHull( lineEnd, lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			if ( sv_showclimbspot.GetBool() )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddBoxOverlay( lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), pRnLPlayer->GetAbsAngles(), 255,0,0,127, 4 );
#else
				// draw blue server impact markers
				NDebugOverlay::BoxAngles( lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), pRnLPlayer->GetAbsAngles(), 0,0,255,127, 4 );
#endif
			}
			return false;
		}
	}

	// IGNORE FOLIAGE AND DOORS
	if( superTrace.m_pEnt )
	{
		const char *pModelName = STRING(superTrace.m_pEnt->GetModelName());

		if( pModelName && Q_stristr( pModelName, "door" ) )
		{
			return false;
		}
	}

	surfacedata_t *phit = physprops->GetSurfaceData( superTrace.surface.surfaceProps );
	if( phit )
	{
		char tex = phit->game.material;
		if ( tex == CHAR_TEX_HEDGEROW || tex == CHAR_TEX_FOLIAGE )
		{
			return false;
		}
	}

	//GET POSSIBLE CLIMBING END POSITION (MAX 96 UNITS)
	for( int i = 16; i < 98; i+=16 )
	{
		end.z += 16;

		// NORMAL CLIMB
		UTIL_TraceHull( end, end, playerStandMins, playerStandMax, MASK_PLAYERSOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			UTIL_TraceLine( end, ( end - vForward * 40 ), MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

			if( superTrace.DidHit() == false )
			{
				toDuck = false;
				pRnLPlayer->m_RnLLocal.m_vecMovementPos = (end + vForward * 16 );
				return true;
			}
		}

		if( superTrace.m_pEnt && superTrace.m_pEnt->IsPlayer() )
		{
			return false;
		}

		// CROUCH CLIMB
		UTIL_TraceHull( end, end, playerCrouchMins, playerCrouchMax, MASK_PLAYERSOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			UTIL_TraceLine( end, ( end - vForward * 40 ), MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

			if( superTrace.DidHit() == false )
			{
				toDuck = true;

				Vector newEnd = end + vForward * 16;
				UTIL_TraceHull( newEnd, newEnd, playerCrouchMins, playerCrouchMax, MASK_SOLID, pRnLPlayer, COLLISION_GROUP_NONE, &superTrace );

				if( superTrace.DidHit() == false )
				{
					pRnLPlayer->m_RnLLocal.m_vecMovementPos = (newEnd);
				}
				else
				{
					pRnLPlayer->m_RnLLocal.m_vecMovementPos = (end);
				}

				return true;
			}
		}
	}

	return false;
}

bool CRnLGameMovement::CanUnprone( bool bToStand )
{
	CRnLPlayer* pRnLPlayer = ToRnLPlayer( player );
	if( !pRnLPlayer)
		return false;

	if (pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
		return false;

	int i;
	trace_t trace;
	Vector newOrigin;

	VectorCopy(mv->GetAbsOrigin(), newOrigin);

	if (player->GetGroundEntity() == NULL)
		return false;

	for (i = 0; i < 3; i++)
	{
		if (bToStand)
			newOrigin[i] += (VEC_PRONE_HULL_MIN[i] - VEC_HULL_MIN[i]);
		else
			newOrigin[i] += (VEC_PRONE_HULL_MIN[i] - VEC_DUCK_HULL_MIN[i]);
	}

	//TracePlayerBBox( mv->m_vecAbsOrigin, newOrigin, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace );
	Ray_t ray;
	if (bToStand)
		ray.Init(mv->GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX);
	else
		ray.Init(mv->GetAbsOrigin(), newOrigin, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);

	UTIL_TraceRay(ray, PlayerSolidMask(), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

	if (trace.startsolid || (trace.fraction != 1.0f))
		return false;

	return true;
}

// Expose our interface.
static CRnLGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );
