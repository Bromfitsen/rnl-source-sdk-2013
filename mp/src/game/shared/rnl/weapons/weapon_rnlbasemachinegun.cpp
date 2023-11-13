//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasemachinegun.h"
#include "gamemovement.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
	#include "iinput.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

#include "engine/ivdebugoverlay.h"

//#define DEPLOY_DISTANCE GetStandOffset()
#define DEPLOY_DISTANCE(player) ((GetStandOffset() > (player->GetPlayerMaxs().x) ) ? GetStandOffset() : (player->GetPlayerMaxs().x + 1.0f ))
#define MAXIMUM_SPREAD_PENALTY 10.0f
#define ROUNDS_PER_TRACER 5

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBaseMachineGun, DT_WeaponRnLBaseMachineGun )

BEGIN_NETWORK_TABLE( CWeaponRnLBaseMachineGun, DT_WeaponRnLBaseMachineGun )
	PropBool(PROPINFO(m_bDeployed)),
	PropInt(PROPINFO(m_iBarrelAnimState)),
	PropInt(PROPINFO(m_iTracerCount)),
	PropFloat(PROPINFO(m_flLastShotTime)),
	PropFloat(PROPINFO(m_flAccuracyPenalty)),
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponRnLBaseMachineGun )
	DEFINE_PRED_FIELD( m_bDeployed, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iBarrelAnimState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iTracerCount, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL(m_flLastShotTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	DEFINE_PRED_FIELD( m_flAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

CWeaponRnLBaseMachineGun::CWeaponRnLBaseMachineGun()
{
	m_bDeployed = false;
	m_iBarrelAnimState = 0;
	m_iTracerCount = 0;
	m_flLastShotTime = 0;
	m_flAccuracyPenalty = 0.0f;
#ifndef CLIENT_DLL
	//m_flNextSuppressionTime = 0.0f;
#endif
}

CWeaponRnLBaseMachineGun::~CWeaponRnLBaseMachineGun()
{
}

bool CWeaponRnLBaseMachineGun::Deploy( void )
{
	m_bDeployed = false;
	
	return BaseClass::Deploy();
}

bool CWeaponRnLBaseMachineGun::CanHolster( void )
{
	if( m_bDeployed )
	{
		return false;
	}

	return BaseClass::CanHolster();
}

float CWeaponRnLBaseMachineGun::GetSpread( void )
{		
	float base = BaseClass::GetSpread();
	float max = GetRnLWpnData().m_flMaxSpread;
	float cur = min(m_flAccuracyPenalty,MAXIMUM_SPREAD_PENALTY) / MAXIMUM_SPREAD_PENALTY;
	return (base + ((max-base) * cur));
}

void CWeaponRnLBaseMachineGun::HandleShotDelay( void )
{
	if( ( m_flLastShotTime + 1.0 ) < gpGlobals->curtime  )
	{
		CRnLPlayer *pPlayer = GetPlayerOwner(); 
		if ( pPlayer )
		{
			pPlayer->m_iShotsFired = 0;
		}
		m_flAccuracyPenalty = 0.0f;
	}

	if( m_flAccuracyPenalty > 0.0f )
	{
		m_flAccuracyPenalty -= 0.1f;
		if( m_flAccuracyPenalty < 0.0f )
			m_flAccuracyPenalty = 0.0f;
	}
}

float CWeaponRnLBaseMachineGun::GetRecoil( void )
{
	return (min(m_flAccuracyPenalty,MAXIMUM_SPREAD_PENALTY) / MAXIMUM_SPREAD_PENALTY) * GetRnLWpnData().m_flRecoil[RECOIL_NORMAL];
}

bool CWeaponRnLBaseMachineGun::CheckUnDeploy( void )
{
	if( !m_bDeployed )
		return false;

	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer == NULL )
		return false;

	if( pPlayer->IsProne() )
		return false;

	if( pPlayer->GetAbsVelocity().IsZero() )
		return false;

	if( (pPlayer->m_nButtons & IN_SPEED) != 0 )
	{
		if( pPlayer->GetWeaponAngle() != pPlayer->GetMovementPostureAngle() )
			pPlayer->SetMovementPostureAngle( pPlayer->GetWeaponAngle() );
	}

	trace_t tr;
	Vector src = pPlayer->GetAbsOrigin() + pPlayer->m_RnLLocal.m_vecMovementPos - Vector( 0, 0, 4);
	UTIL_TraceLine( src, src + Vector( 0, 0, -72 ) , MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
	if( developer.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddLineOverlay( tr.startpos, tr.endpos, 255,0,0,127, 4 );
#else
		// draw blue server impact markers
		NDebugOverlay::Line( tr.startpos, tr.endpos, 0,0,255,127, 4 );
#endif
	}

	if( tr.DidHit() )
	{
		if( tr.endpos.DistTo( src ) < 1.5f )
			return false;
		else if( (tr.endpos.z-pPlayer->GetAbsOrigin().z) > 23.0f )
		{
			pPlayer->Deploy( pPlayer->GetMovementPostureAngle(), tr.endpos );
			return false;
		}
	}
	else if( tr.allsolid )
	{
		Vector start = src;
		start.z = pPlayer->GetAbsOrigin().z + VEC_VIEW.z;
		UTIL_TraceLine( start + Vector( 0, 0, 24 ), src, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
		if( !tr.allsolid && tr.DidHit() )
		{
			pPlayer->Deploy( pPlayer->GetMovementPostureAngle(), tr.endpos );
			return false;
		}
	}

	ToggleDeploy();
	return true;
}

void CWeaponRnLBaseMachineGun::ToggleDeploy( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer == NULL )
		return;

	if( m_bDeployed )
	{
		HandleViewAnimation( WEAPON_ANIMATION_UNDEPLOY );
		m_bDeployed = false;

#ifndef CLIENT_DLL
		// Switch back to shoulder fov first
		SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_SHOULDER] );
#endif
		//TODO : Make the player aware of our undeployment
		pPlayer->UnDeploy();
		
#ifndef CLIENT_DLL
		if( m_pResetingEntity )
		{
			IPhysicsObject *pPhysics = m_pResetingEntity->VPhysicsGetObject();
			if ( pPhysics )
			{
				pPhysics->EnableMotion( true );
			}
			m_pResetingEntity->m_takedamage = m_oldDamage;
			m_oldDamage = DAMAGE_NO;
			m_pResetingEntity = NULL;
		}
#endif

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	}
	else
	{
		Vector vecDir;
		QAngle playerAngles = pPlayer->EyeAngles();
		//playerAngles[PITCH] = 0;
		AngleVectors( playerAngles, &vecDir );

		float flDeployDistance = DEPLOY_DISTANCE(pPlayer);

		if( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE )
		{
			//DevMsg( "Deployed in prone.\n" );
			HandleViewAnimation( WEAPON_ANIMATION_DEPLOY );
			m_bDeployed = true;
#ifndef CLIENT_DLL
			// Switch back to shoulder fov first
			SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_IRONSIGHTS] );
#endif
			//TODO : Make the player aware of our deployment
			pPlayer->Deploy( playerAngles, pPlayer->Weapon_ShootPosition() + (vecDir * flDeployDistance) );

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		}
		else if( ( pPlayer->GetFlags() & FL_ONGROUND ) && pPlayer->GetAbsVelocity().Length() < 65 )
		{
			Vector vecSrc = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();
			if ( pPlayer->IsDucked() )
				vecSrc.z += 8; // temp crouch height hack
			Vector vecEnd = pPlayer->GetAbsOrigin() + (vecDir * flDeployDistance) + Vector( 0, 0, 24 );

			trace_t tr;
			UTIL_TraceLine( vecSrc, vecSrc + (vecDir * flDeployDistance) , MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

			if( developer.GetBool() )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddLineOverlay( tr.startpos, tr.endpos, 255,0,0,127, 4 );
#else
				// draw blue server impact markers
				NDebugOverlay::Line( tr.startpos, tr.endpos, 0,0,255,127, 4 );
#endif
			}

			if ( tr.fraction != 1.0f )
			{
				DevMsg( "Area infront of the player is not clear.\n" );
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.1;
				return;
			}
			vecSrc = tr.endpos;

			//Another trance this time start out towards the players bounding box
			UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

			if( developer.GetBool() )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddLineOverlay( tr.startpos, tr.endpos, 255,0,0,127, 4 );
#else
				// draw blue server impact markers
				NDebugOverlay::Line( tr.startpos, tr.endpos, 0,0,255,127, 4 );
#endif
			}

			int highestTrace = 0;
			Vector startPos[2];
			startPos[0] =  (pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset()) + (vecDir * (flDeployDistance/2.0f));
			startPos[1] =  (pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset()) + (vecDir * (flDeployDistance));
			trace_t traces[10];
			float traceIncrement = flDeployDistance * 0.08f;
			float traceDist = flDeployDistance * 0.7f;
			bool hit = false;

			for( int i = 0; i < 8; i++ )
			{
				vecEnd = pPlayer->GetAbsOrigin() + (vecDir * traceDist) + Vector( 0, 0, 24 );
				UTIL_TraceLine( startPos[(i/4)], vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &(traces[i]) );
				
				traceDist += traceIncrement;

				if( traces[i].DidHit() )
					hit = true;

				if( highestTrace != i )
					if( traces[i].DidHit() && traces[i].endpos.z > traces[highestTrace].endpos.z )
						if( !traces[i].m_pEnt || traces[i].m_pEnt->IsPlayer() == false )
							highestTrace = i;

				if( developer.GetBool() )
				{
#ifdef CLIENT_DLL
					// draw red client impact markers
					debugoverlay->AddLineOverlay( traces[i].startpos, traces[i].endpos, 255,0,0,127, 8.0f );
#else
					// draw blue server impact markers
					NDebugOverlay::Line( traces[i].startpos, traces[i].endpos, 0,0,255,127, 8.0f );
#endif
				}
			}

			if ( hit == false )
			{
				DevMsg( "Nothing To rest on.\n" );
				m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + 0.1;
				return;
			}
#ifndef CLIENT_DLL
			if( tr.DidHitNonWorldEntity() && tr.m_pEnt )
			{
				m_pResetingEntity = tr.m_pEnt;
				m_oldDamage = m_pResetingEntity->m_takedamage;
				m_pResetingEntity->m_takedamage = DAMAGE_NO;
				IPhysicsObject *pPhysics = tr.m_pEnt->VPhysicsGetObject();
				if ( pPhysics && pPhysics->IsMoveable() )
				{
					pPhysics->EnableMotion( false );
				}
			}
			// Switch back to shoulder fov first
			SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_IRONSIGHTS] );
#endif

			HandleViewAnimation( WEAPON_ANIMATION_DEPLOY );
			m_bDeployed = true;
			tr.endpos.z = traces[highestTrace].endpos.z;
			pPlayer->Deploy( playerAngles, tr.endpos );

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		}
		else
		{
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime;
		}
	}
}

void CWeaponRnLBaseMachineGun::GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents )
{
	BaseClass::GetFreeAimBounds( maxExtents, deadZone, lockToExtents );

	if( m_bDeployed )
	{
		//CRnLPlayer *pPlayer = GetPlayerOwner();
		//if( !pPlayer && (pPlayer->m_nButtons & IN_SPEED) != 0 )		
		//	lockToExtents = AIM_LOCK_SLOWDOWN;
		//else
			lockToExtents = AIM_LOCK_LIMIT;
		maxExtents[0] = 22.0f;
		maxExtents[1] = 12.0f;
	}
}

void CWeaponRnLBaseMachineGun::HandleViewAnimation( int iAnim )
{
	SetAnimationState( iAnim );

	if( iAnim == WEAPON_ANIMATION_DEPLOY )
	{
		SendWeaponAnim( ACT_VM_DEPLOY );
	}
	else if( iAnim == WEAPON_ANIMATION_UNDEPLOY )
	{	
		SendWeaponAnim( ACT_VM_UNDEPLOY );
	}
	else if( IsDeployed() )
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_ATTACK:
			SendWeaponAnim( ACT_VM_PRIMARYATTACK_DEPLOYED );
			break;

		case WEAPON_ANIMATION_RELOAD:
			/* Michael Lebson
			Each box of rounds should reset the tracer count.*/
			m_iTracerCount = 0;

			if( Clip1() > 0 )
				SendWeaponAnim( ACT_VM_RELOADMID );
			else
				SendWeaponAnim( ACT_VM_RELOAD );
			break;

		case WEAPON_ANIMATION_IDLE:
			SendWeaponAnim( ACT_VM_IDLE_DEPLOYED );
			break;
		}
	}
	else
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_ATTACK:
			//SendWeaponAnim( ACT_VM_PRIMARYATTACK );
			break;

		case WEAPON_ANIMATION_RELOAD:
			/*
			 Michael Lebson
			Each box of rounds should reset the tracer count.
			m_iTracerCount = 0;

			if( Clip1() > 0 )
				SendWeaponAnim( ACT_VM_RELOADMID );
			else
				SendWeaponAnim( ACT_VM_RELOAD );
			*/
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
			else 
				SendWeaponAnim( ACT_VM_IDLE );
			break;

		case WEAPON_ANIMATION_DRAW:
			SendWeaponAnim( ACT_VM_DRAW );
			break;

		case WEAPON_ANIMATION_HOLSTER:
			SendWeaponAnim( ACT_VM_HOLSTER );
			break;

		case WEAPON_ANIMATION_PRONE_ENTER:
			SendWeaponAnim( ACT_VM_PRONE_ENTER );
			break;
		case WEAPON_ANIMATION_PRONE_LEAVE:
			SendWeaponAnim( ACT_VM_PRONE_LEAVE );
			break;

		case WEAPON_ANIMATION_SPRINT_ENTER:
			//SendWeaponAnim( ACT_VM_SPRINT_ENTER );
			break;
		case WEAPON_ANIMATION_SPRINT_LEAVE:
			//SendWeaponAnim( ACT_VM_SPRINT_LEAVE );
			break;

		}
	}
}

/* Michael Lebson
Make a tracer every ROUNDS_PER_TRACER number of bullets fired*/
bool CWeaponRnLBaseMachineGun::ShouldMakeTracer()
{
	m_iTracerCount++;

	if( m_iTracerCount >= ROUNDS_PER_TRACER )
		m_iTracerCount = 0;
	else if( m_iTracerCount == 1 )
		return true;

	return false;
}