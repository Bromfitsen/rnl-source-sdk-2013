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

//Should be enumed but linux doesn't like it
//#define standing 0
//#define ducking 1
//#define proning 2
//#define climbing 3

extern bool g_bMovementOptimizations;

struct surfacedata_t;

class CBasePlayer;

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

private:
	bool			bLeanHit;
};

// ---------------------------------------------------------------------------------------- //
// CRnLGameMovement.
// ---------------------------------------------------------------------------------------- //

CRnLGameMovement::CRnLGameMovement()
{
}

//-----------------------------------------------------------------------------
// Handle the entire stamina stuff
//-----------------------------------------------------------------------------
void CRnLGameMovement::StaminaThink()
{
	// First cast us a player
	CRnLPlayer *pPlayer = ToRnLPlayer( player );
	// Preferably a valid one, too
	if( !pPlayer )
		return;

	if( !pPlayer->IsAlive() )
		return;

	if( pPlayer->GetTeamNumber() != TEAM_AXIS && pPlayer->GetTeamNumber() != TEAM_ALLIES )
	{
		mv->m_flMaxSpeed  = 250;
		return;
	}

	if( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT || pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT )
		return;

	float speed = SPEED_RUN;
	float recoverAmt = 0.0f;
	float curStamina = pPlayer->GetStamina();
	float recovTime = ( pPlayer->GetRecoverTime() - gpGlobals->curtime );

	if( recovTime > 0.0f )
	{
		if( pPlayer->IsSprinting() )
		{
			pPlayer->StopSprinting();
		}

		StaminaRecoveringState( recovTime, recoverAmt, speed );
		
		if( curStamina < 40 )
		{
			if( pPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
			{
				int pos = 8 + (curStamina / 5);
						
				pPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pPlayer->PlayStaminaSound( pos ) ) );
			}
		}
	}
	else
	{
		StaminaNormalState( recoverAmt, speed );
	}

	CWeaponRnLBase *pWeapon = pPlayer->GetActiveRnLWeapon();

	if( curStamina < pPlayer->GetMaxStamina() || recoverAmt < 0.0 )
	{
		curStamina += recoverAmt;

		if( pWeapon && (pWeapon->GetWeaponID() == WEAPON_AXISFISTS || pWeapon->GetWeaponID() == WEAPON_ALLIEDFISTS) )
			curStamina += 0.0075f;

		CWeaponRnLBase *pWeapon = pPlayer->GetActiveRnLWeapon();

		if( pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM )
			curStamina = pPlayer->GetMaxStamina();

		if( curStamina > 0 )
		{
			pPlayer->SetStamina( curStamina );
		}
		else
		{
			float length = pPlayer->GetSprintTimer();
			DevMsg( "Sprinted for %f seconds\n", length );
			length += (gpGlobals->curtime + 2.5f );
			pPlayer->SetRecoverTime( length );
		}
	}

	if( (pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM) || (player->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT || player->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT) )
	{
		mv->m_flMaxSpeed = speed;
	}
	else
	{
		//pPlayer->SetMaxSpeed( speed );

		if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS )
		{
			if( !pPlayer->IsProne() )
				speed *= 0.85f;
		}
		else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
		{
			if( !pPlayer->IsProne() )
				speed *= 0.55f;
		}

		if( pPlayer->GetDamageBasedSpeedModifier( 1 ) > 0.0f && speed > 35.0f )
		{
			if ( mv->m_flForwardMove > 45 || mv->m_flSideMove > 45 )
				pPlayer->PlayMovingWoundedSound();

			speed *= 1.0f - ( pPlayer->GetDamageBasedSpeedModifier( 1 ) / 100.0f );
			speed = clamp( speed, SPEED_CROUCH_WALK, SPEED_SPRINT );
		}

		if( !pPlayer->GetLeaningOffset().IsZero( 1.0 ) )
		{
			speed *= 0.70f;
		}

		if( speed > pPlayer->MaxSpeed() )
		{
			speed = pPlayer->MaxSpeed();
		}

		if( pPlayer->GetWaterLevel() >= WL_Waist )
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
	CRnLPlayer* pPlayer = ToRnLPlayer( player );
	if( !pPlayer )
		return;

	float recovTimeDec = 0.0f;
	float flMoraleFactor = (pPlayer->GetMoraleLevel()/50.0f);

	if( flMoraleFactor < 0.01 )
	{
		flMoraleFactor = 0.01;
	}

	if( pPlayer->IsDeployed() )
	{
		//MG Movement
		//if( (pPlayer->m_nButtons & IN_SPEED) == 0 )
			flMoveSpeed = SPEED_NONE;
		//else
		//	flMoveSpeed = SPEED_DEPLOYED_ADJUST;
		flRecoverAmt = STAMINA_PRONE_WALK * flMoraleFactor;
	}
	else if ( pPlayer->IsDucked() )
	{
		if( mv->m_nButtons & IN_WALK )
		{
			flMoveSpeed = SPEED_CROUCH_WALK;
			flRecoverAmt = STAMINA_CROUCH_WALK * flMoraleFactor;
		}
		else
		{
			flMoveSpeed = SPEED_CROUCH_RUN;
			flRecoverAmt = STAMINA_CROUCH_RUN * flMoraleFactor;
		}
		recovTimeDec = 0.04f;
	}
	else if ( pPlayer->IsProne() )
	{
		if( mv->m_nButtons & IN_WALK )
		{
			flMoveSpeed = SPEED_PRONE_WALK;
			flRecoverAmt = STAMINA_PRONE_WALK * flMoraleFactor;
		}
		else
		{
			flMoveSpeed = SPEED_PRONE_RUN;
			flRecoverAmt = STAMINA_PRONE_RUN * flMoraleFactor;
		}
		recovTimeDec = 0.05f;
	}
	else if( mv->m_nButtons & IN_WALK )
	{
		recovTimeDec = 0.02f;
		flMoveSpeed = SPEED_WALK;
		flRecoverAmt = STAMINA_WALK * flMoraleFactor;
	}
	else
	{
		float timer = pPlayer->GetSprintTimer();

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
				pPlayer->SetSprintTimer( 0.0f );
			}
		}

		flRecoverAmt = STAMINA_RECOVER  * flMoraleFactor;
	}

	if( pPlayer->GetGroundEntity() == NULL )
		recovTimeDec = 0.0f;

	if( recovTimeDec > 0.0f  )
	{
		flRecovTime = pPlayer->GetRecoverTime() - recovTimeDec;

		if( flRecovTime < (gpGlobals->curtime + 0.2 ) )
		{
			pPlayer->SetSprintTimer( 0.0f );
		}
		
		pPlayer->SetRecoverTime( flRecovTime );
	}
}

void CRnLGameMovement::StaminaNormalState( float& flRecoverAmt, float& flMoveSpeed )
{
	CRnLPlayer* pPlayer = ToRnLPlayer( player );
	if( !pPlayer )
		return;

	bool shouldSprint = (( mv->m_flForwardMove > 0.0f ) && ( mv->m_nButtons & IN_SPEED ) 
		&& pPlayer->GetLeaningOffset().IsZero( 1.0 ) && pPlayer->GetWeaponPosture() != WEAPON_POSTURE_THROWING_GRENADE
		&& pPlayer->GetAbsVelocity().Length2D() > 2.0f 
		/* Michael Lebson
		Don't let a player sprint with their map open, or when they are swimming.
		*/
		&& !( mv->m_nButtons & IN_MAP ) && pPlayer->GetWaterLevel() < WL_Waist );

	float flMoraleFactor = (pPlayer->GetMoraleLevel() / 65.0f);

	if( flMoraleFactor < 0.01 )
	{
		flMoraleFactor = 0.01;
	}

	if( pPlayer->IsDeployed() )
	{
		shouldSprint = false;
		//MG Movement
		//if( (pPlayer->m_nButtons & IN_SPEED) == 0 )
			flMoveSpeed = SPEED_NONE;
		//else
		//	flMoveSpeed = SPEED_DEPLOYED_ADJUST;
		flRecoverAmt = STAMINA_PRONE_WALK * flMoraleFactor;
	}
	else if( player->IsDucked() )
	{
		//DevMsg( "Player is ducking or ducked.\n" );
		if( shouldSprint && pPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f && pPlayer->StartSprinting() )
		{
			flMoveSpeed = SPEED_CROUCH_SPRINT;
			flRecoverAmt = STAMINA_SPRINT / flMoraleFactor;
			pPlayer->SetSprintTimer( ( pPlayer->GetSprintTimer() + 0.01f ) );

			if( pPlayer->GetStamina() < 40.0f )
			{
				flMoveSpeed = SPEED_CROUCH_RUN + ( (SPEED_CROUCH_SPRINT - SPEED_CROUCH_RUN ) * (pPlayer->GetStamina() / 100) );
				if( pPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
				{
					int pos = pPlayer->GetStamina() / 5;
					
					pPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pPlayer->PlayStaminaSound( pos ) ) );
				}
			}
		}
		else
		{
			shouldSprint = false;
			flMoveSpeed = SPEED_CROUCH_RUN;
			flRecoverAmt = STAMINA_CROUCH_RUN * flMoraleFactor;
		}
	}
	else if( player->IsProne()
		|| (player->GetMovementPostureFrom() == MOVEMENT_POSTURE_PRONE && player->GetMovementPostureDuration() < 0.5f) )
	{
		//DevMsg( "Player is proning or prone.\n" );
		//if( shouldSprint && !player->IsProning() && pPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f /*&& pPlayer->StartSprinting()*/ )
		//{
		//	/* Going from prone right into sprint, didn't turn out the I want it to be, would need special animations, so I'll leave that out of the game
		//	pPlayer->SetMovementPosture( MOVEMENT_POSTURE_PRONE_TO_STAND );
		//	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_PRONEGETUP );
		//	pPlayer->ViewPunch( QAngle( 180, 200, 140 ) );*/

		//	flMoveSpeed = SPEED_PRONE_SPRINT;
		//	flRecoverAmt = STAMINA_PRONE_SPRINT  / flMoraleFactor;
		//	pPlayer->SetSprintTimer( ( pPlayer->GetSprintTimer() + 0.01f ) );

		//	if( pPlayer->GetStamina() < 40.0f )
		//	{
		//		if( pPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
		//		{
		//			int pos = pPlayer->GetStamina() / 5;
		//			
		//			pPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pPlayer->PlayStaminaSound( pos ) ) );
		//		}
		//	}
		//}
		//else
		{
			shouldSprint = false;
			flMoveSpeed = SPEED_PRONE_RUN;
			flRecoverAmt = STAMINA_PRONE_RUN * flMoraleFactor;
		}
	}
	else
	{
		if( shouldSprint && pPlayer->GetDamageBasedSpeedModifier( 1 ) < 50.0f && pPlayer->StartSprinting() )
		{
			flMoveSpeed = SPEED_SPRINT;
			flRecoverAmt = STAMINA_SPRINT / flMoraleFactor;
			pPlayer->SetSprintTimer( ( pPlayer->GetSprintTimer() + 0.01f ) );

			if( pPlayer->GetStamina() < 40.0f )
			{
				flMoveSpeed = SPEED_RUN + ( (SPEED_SPRINT - SPEED_RUN ) * (pPlayer->GetStamina() / 100) );
				if( pPlayer->GetNextSprintSoundTime() <= gpGlobals->curtime )
				{
					int pos = pPlayer->GetStamina() / 5;
					
					pPlayer->SetNextSprintSoundTime( (gpGlobals->curtime + pPlayer->PlayStaminaSound( pos ) ) );
				}
			}
		}
		else 
		{
			shouldSprint = false;

			if( mv->m_nButtons & IN_WALK )
			{
				flMoveSpeed = SPEED_WALK;
				flRecoverAmt = STAMINA_WALK;
			}
			else
			{
				flMoveSpeed = SPEED_RUN;
				flRecoverAmt = STAMINA_RUN;
			}
		}
	}

	if( pPlayer->IsSprinting() && !shouldSprint )
	{
		if( pPlayer->GetSprintTimer() > 0.0f )
		{
			CWeaponRnLBase *pWeapon = pPlayer->GetActiveRnLWeapon();

			if( pWeapon && pWeapon->GetWeaponID() == WEAPON_DEVCAM )
			{
				pPlayer->SetRecoverTime( 1.0f );
			}
			else
			{
				float length = (pPlayer->GetSprintTimer() / 4);
				length += (gpGlobals->curtime + 1.0f );
				if( length > 15.0f )
				{
					pPlayer->SetRecoverTime( length );
				}
				else
				{
					pPlayer->SetRecoverTime( (gpGlobals->curtime + 1.0f ) );
				}
			}
		}
	}
}

void CRnLGameMovement::HandleLeaning()
{
	CRnLPlayer *pPlayer = ToRnLPlayer( player );
	if( !pPlayer )
		return;

	bool shouldLean = false;
	bool canLean = !( pPlayer->IsProne() || pPlayer->IsDeployed() || pPlayer->IsPostureChanging( 5.0, 2.0 ) );
	int iLeanDir = 0;


	if( canLean )
	{
		trace_t trace;
		UTIL_TraceHull(pPlayer->EyePosition(), pPlayer->EyePosition(), Vector( -10, -10, -10 ), Vector( 10, 10, 10 ),  MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace );

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
				float rollOffset = pPlayer->GetViewRollOffset();
				Vector vecOffset = pPlayer->GetLeaningOffset();
				
				vecOffset.z += 0.25f;

				if( vecOffset.z >= 0 )
				{
					pPlayer->SetViewRollOffset( 0 );
					pPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );
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

				pPlayer->SetViewRollOffset( rollOffset );
				pPlayer->SetLeaningOffset( vecOffset );
				
				if ( vecOffset.y > -1 && vecOffset.y < 1 )
				{
					pPlayer->SetViewRollOffset( 0 );
					pPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );
					return;
				}

				UTIL_TraceHull(pPlayer->EyePosition(), pPlayer->EyePosition(), Vector( -10, -10, -10 ), Vector( 10, 10, 10 ),  MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace );
			}
			return;
		}
	}

	if( !shouldLean )
	{
		bLeanHit = false;
		Vector vecOffset = pPlayer->GetLeaningOffset();

		float rollOffset = pPlayer->GetViewRollOffset();
		if( !vecOffset.IsZero() || ( rollOffset != 0 ))
		{
			if( vecOffset.y < 1 && vecOffset.y > -1 )
			{
				pPlayer->SetLeaningOffset( Vector( 0, 0, 0 ) );

				if( rollOffset != 0 )
				{
					pPlayer->SetViewRollOffset( 0 );
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
			Vector vecOffset = pPlayer->GetLeaningOffset();
			vecOffset.y -= iLeanDir;

			int imax = 14;
			int imin = -14;

			if( player->IsDucked() )
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
				float  rollOffset = pPlayer->GetViewRollOffset();
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

				pPlayer->SetViewRollOffset( rollOffset );
			}

			pPlayer->SetLeaningOffset( vecOffset );
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
	
	bool bIsClimbing = (player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING || player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );

	// clear output applied velocity
	mv->m_outWishVel.Init();
	mv->m_outJumpVel.Init();

	MoveHelper( )->ResetTouchList();                    // Assume we don't touch anything

	ReduceTimers();

	AngleVectors (mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp );  // Determine movement angles

	// Always try and unstick us unless we are a couple of the movement modes
	if ( CheckInterval( STUCK ) )
	{
		if ( player->GetMoveType() != MOVETYPE_NOCLIP && 
			player->GetMoveType() != MOVETYPE_NONE && 		 
			player->GetMoveType() != MOVETYPE_ISOMETRIC && 
			player->GetMoveType() != MOVETYPE_OBSERVER )
		{
			if ( CheckStuck() )
			{
				// Can't move, we're stuck
				return;  
			}
		}
	}

	// Now that we are "unstuck", see where we are (player->GetWaterLevel() and type, player->GetGroundEntity()).
	CategorizePosition();

	// Store off the starting water level
	m_nOldWaterLevel = player->GetWaterLevel();

	if( player->IsAlive() && (player->GetTeamNumber() == TEAM_AXIS || player->GetTeamNumber() == TEAM_ALLIES) )
	{
		// If we are not on ground, store off how fast we are moving down
		if ( player->GetGroundEntity() == NULL )
		{
			//Not Climbing fall
			if( !bIsClimbing )
			{
				if( player->GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
				{
					player->SetViewOffset( GetPlayerViewOffset() );
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

					mv->m_vecVelocity[ 2 ] = player->m_Local.m_flFallVelocity = min( (player->m_Local.m_flFallVelocity  - mv->m_vecVelocity[ 2 ]), MAX_PARASPEED );

					if( MAX_PARASPEED > player->m_Local.m_flFallVelocity )
					{
						player->m_Local.m_flFallVelocity = MAX_PARASPEED;
						mv->m_vecVelocity[ 2 ] = MAX_PARASPEED;
					}
				}
				else
				{
					player->m_Local.m_flFallVelocity = -mv->m_vecVelocity[ 2 ];
				}
			}
		}
		else if( player->GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
		{
			/* Michael Lebson
			If the player is on the ground but still parachuting,
			remove the parachute and make the player's weapon
			draw as if they are just pulling it out again.
			*/
		#ifndef CLIENT_DLL
			pRnLPlayer->RemoveEquipment( 0 );
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

	if( player->IsAlive() && (player->GetTeamNumber() == TEAM_AXIS || player->GetTeamNumber() == TEAM_ALLIES) )
	{
		if( player->GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING )
		{
			player->UpdateStepSound( player->GetSurfaceData(), mv->GetAbsOrigin(), mv->m_vecVelocity );

			DuckAndProne();

			// Don't run ladder code if dead or on a train
			if ( !player->pl.deadflag && !(player->GetFlags() & FL_ONTRAIN) )
			{
				// If was not on a ladder now, but was on one before, 
				//  get off of the ladder

				// TODO: this causes lots of weirdness.
				//bool bCheckLadder = CheckInterval( LADDER );
				//if ( bCheckLadder || player->GetMoveType() == MOVETYPE_LADDER )
				//{
					if( !LadderMove() )
					{
						if ( player->GetMoveType() == MOVETYPE_LADDER )
						{
							// Clear ladder stuff unless player is dead or riding a train
							// It will be reset immediately again next frame if necessary
							player->SetMoveType( MOVETYPE_WALK );
							player->SetMoveCollide( MOVECOLLIDE_DEFAULT );
							m_nOnLadder = 0;
						}

						CWeaponRnLBase* pActiveWeapon = pRnLPlayer->GetActiveRnLWeapon();

						if( pActiveWeapon && !pActiveWeapon->IsWeaponVisible() )
						{
							pActiveWeapon->SetWeaponVisible(true);
							pActiveWeapon->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
						}
					}

				//}

				//RnL : BB : Climbing Handling
				if ( !ClimbingMove() && ( player->GetMoveType() == MOVETYPE_CLIMBING ) )
				{
					// Clear climbing stuff unless player is dead or riding a train
					// It will be reset immediately again next frame if necessary
					player->SetMoveType( MOVETYPE_WALK );
					player->SetMoveCollide( MOVECOLLIDE_DEFAULT );
				}
			}
		}
	}

#ifndef CLIENT_DLL
	player->m_flForwardMove = mv->m_flForwardMove;
	player->m_flSideMove = mv->m_flSideMove;
#endif

	// Handle movement modes.
	switch (player->GetMoveType())
	{
		case MOVETYPE_NONE:
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

		//RnL : BB : Climbing
		case MOVETYPE_CLIMBING:
			FullClimbMove();
			break;

		default:
			DevMsg( 1, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", player->GetMoveType(), player->IsServer());
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
	CRnLPlayer* pPlayer = (CRnLPlayer*)player;

#ifdef CLIENT_DLL
	/*Michael Lebson
	Hide the map if they are on a ladder
	*/
	if( pPlayer && pPlayer->m_nButtons & IN_MAP )
		gViewPortInterface->ShowPanel( PANEL_MAP, false );
#endif

	/*Michael Lebson
	Holster the player's weapon while they are on the ladder,
	but only the first time.
	*/
	if( pPlayer && pPlayer->GetActiveRnLWeapon() && (m_nOnLadder == 0) )
		pPlayer->GetActiveRnLWeapon()->Holster();

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

	bool bIsClimbing = ( player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING || player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );

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
		if( mv->m_vecVelocity.IsZero() && player->IsStanding()  )
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
			player->SetMovementPosture( MOVEMENT_POSTURE_CLIMBING_TO_CROUCH );
		else
			player->SetMovementPosture( MOVEMENT_POSTURE_CLIMBING );

		// Get climb height
		pRnLPlayer->m_iClimbheight = pRnLPlayer->m_RnLLocal.m_vecMovementPos.GetZ() - player->GetAbsOrigin().z - 10;
		//DevMsg("Climb height: %i\n", pRnLPlayer->m_iClimbheight);
		
		// Holster for high climbs
		if( pWep && ( pRnLPlayer->GetClimbheight() >= 24 )/*LOWEST_CONTROLLED_CLIMB ) && pWep->CanHolster() */ )
		{
			pWep->HandleViewAnimation( WEAPON_ANIMATION_HOLSTER );
			//pWep->Holster();
		}

		pRnLPlayer->DoAnimationEvent( PLAYERANIMEVENT_CLIMB );

		player->SetMoveType( MOVETYPE_CLIMBING );
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
			if( player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH )
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
	
	if ( !(mv->m_nButtons & IN_JUMP) && player->GetCycle() < 0.75 )
	{
		// CANCEL CLIMB
		player->SetMovementPosture( MOVEMENT_POSTURE_STAND );

		if ( player->GetCycle() > 0.25 )
		{
			Vector playerPos = player->GetAbsOrigin();
			playerPos.z += abs((pRnLPlayer->m_RnLLocal.m_vecMovementPos.GetZ()-playerPos.z) * (player->GetCycle() - 0.25));
			mv->SetAbsOrigin( playerPos );
		}
		
		cancelClimbing = true;
	}

	if( doneClimbing )
	{
		mv->SetAbsOrigin( pRnLPlayer->m_RnLLocal.m_vecMovementPos );
		if( player->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING_TO_CROUCH )
			player->SetMovementPosture( MOVEMENT_POSTURE_CROUCH );
		else
			player->SetMovementPosture( MOVEMENT_POSTURE_STAND );
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
		QAngle ang = player->EyeAngles();
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
	CRnLPlayer* pPlayer = ToRnLPlayer( player );
	if( !pPlayer )
		return false;

	pPlayer->m_RnLLocal.m_vecMovementPos.Init();

	trace_t superTrace;

	Vector start = player->GetAbsOrigin();
	//Hull testing that is slightly larger than the 
	//player so we don't have them getting stuck
	Vector playerStandMins = Vector( -18, -18, -2 );
	Vector playerStandMax = Vector( 20, 20, 85 );

	Vector playerCrouchMins = Vector( -18, -18, -2 );
	Vector playerCrouchMax = Vector( 20, 20, 50 );

	Vector vForward;
	AngleVectors( player->GetLocalAngles(), &vForward );
	vForward.z = 0;
	VectorNormalize( vForward );
	
	//40 units just for good measure.
	Vector end = ( start + vForward * 32 );
	end.z += 10;

	Vector lineStart = start;
	Vector lineEnd = end - ( vForward * 12 );
	lineEnd.z += 4;

	UTIL_TraceLine( lineStart, lineEnd, MASK_SOLID, player, COLLISION_GROUP_NONE, &superTrace );

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
		UTIL_TraceHull( lineEnd, lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), MASK_SOLID, player, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			if ( sv_showclimbspot.GetBool() )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddBoxOverlay( lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), player->GetAbsAngles(), 255,0,0,127, 4 );
#else
				// draw blue server impact markers
				NDebugOverlay::BoxAngles( lineEnd, Vector( -8, -8, -4), Vector( 8, 8, 32), player->GetAbsAngles(), 0,0,255,127, 4 );
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
		UTIL_TraceHull( end, end, playerStandMins, playerStandMax, MASK_PLAYERSOLID, player, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			UTIL_TraceLine( end, ( end - vForward * 40 ), MASK_SOLID, player, COLLISION_GROUP_NONE, &superTrace );

			if( superTrace.DidHit() == false )
			{
				toDuck = false;
				pPlayer->m_RnLLocal.m_vecMovementPos = (end + vForward * 16 );
				return true;
			}
		}

		if( superTrace.m_pEnt && superTrace.m_pEnt->IsPlayer() )
		{
			return false;
		}

		// CROUCH CLIMB
		UTIL_TraceHull( end, end, playerCrouchMins, playerCrouchMax, MASK_PLAYERSOLID, player, COLLISION_GROUP_NONE, &superTrace );

		if( !(superTrace.DidHit()) )
		{
			UTIL_TraceLine( end, ( end - vForward * 40 ), MASK_SOLID, player, COLLISION_GROUP_NONE, &superTrace );

			if( superTrace.DidHit() == false )
			{
				toDuck = true;

				Vector newEnd = end + vForward * 16;
				UTIL_TraceHull( newEnd, newEnd, playerCrouchMins, playerCrouchMax, MASK_SOLID, player, COLLISION_GROUP_NONE, &superTrace );

				if( superTrace.DidHit() == false )
				{
					pPlayer->m_RnLLocal.m_vecMovementPos = (newEnd);
				}
				else
				{
					pPlayer->m_RnLLocal.m_vecMovementPos = (end);
				}

				return true;
			}
		}
	}

	return false;
}

bool CRnLGameMovement::CanUnprone( bool bToStand )
{
	CRnLPlayer* pPlayer = ToRnLPlayer( player );
	if( !pPlayer )
		return false;

	if ( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
		return false;

	return BaseClass::CanUnprone(bToStand);
}

// Expose our interface.
static CRnLGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );
