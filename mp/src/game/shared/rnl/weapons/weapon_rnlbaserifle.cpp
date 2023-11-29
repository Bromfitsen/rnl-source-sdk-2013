//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbaserifle.h"
#include "takedamageinfo.h"
#include "gamemovement.h"
#include "rnl_fx_shared.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM,-BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM,BLUDGEON_HULL_DIM);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBaseRifle, DT_WeaponRnLBaseRifle );

BEGIN_NETWORK_TABLE( CWeaponRnLBaseRifle, DT_WeaponRnLBaseRifle )
	PropBool( PROPINFO( m_bDeployed ) ),
	PropInt(PROPINFO( m_iBayoAnimState ) ),
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC(CWeaponRnLBaseRifle)
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA( CWeaponRnLBaseRifle )
		DEFINE_PRED_FIELD( m_bDeployed, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
		DEFINE_PRED_FIELD( m_iBayoAnimState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_baserifle, CWeaponRnLBaseRifle );

CWeaponRnLBaseRifle::CWeaponRnLBaseRifle()
{
}

CWeaponRnLBaseRifle::~CWeaponRnLBaseRifle()
{
}

void CWeaponRnLBaseRifle::SecondaryAttack( bool bIsSecondary ) // TODO_KORNEEL What is up with this non-override with weird param?
{
	if( !m_bDeployed )
		return;

	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer ) 
		return;

	WeaponSound( SPECIAL1 );
	SendWeaponAnim( ACT_VM_SECSHOULDERFIRE1 );

	trace_t traceHit;
	Vector forward, swingStart = pPlayer->Weapon_ShootPosition();

	pPlayer->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit );
	Activity nHitActivity = ACT_VM_HITCENTER;

#ifndef CLIENT_DLL
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( pPlayer, pPlayer, GARANDMELEEDAMAGE, DMG_CLUB );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin );
#endif

	if ( traceHit.fraction == 1.0 )
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pPlayer );
			}
		}
	}

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange();

		// See if we happened to hit water
		ImpactWaterMelee( swingStart, testEnd );
#ifdef CLIENT_DLL
		FX_WeaponSound( pPlayer->entindex(), MELEE_MISS, swingStart, (CRnLWeaponInfo *)GetFileWeaponInfoFromHandle( GetWeaponFileInfoHandle() ) );
#else
		WeaponSound( MELEE_MISS );
#endif
	}
	else
	{
		HitMelee( traceHit, nHitActivity );

#ifndef CLIENT_DLL
		// Play the correct softimpact sound for the material hit.
		surfacedata_t *pSurf = physprops->GetSurfaceData( traceHit.surface.surfaceProps );
		if( pSurf )
		{
			EmitSound_t params;
			char buffer[128];

			Q_snprintf( buffer, 128, "%s.bayonet", physprops->GetString( pSurf->sounds.impactSoft ) );
			params.m_pSoundName = buffer;
			params.m_bWarnOnDirectWaveReference = true;

			CPASAttenuationFilter filter( pPlayer, params.m_pSoundName );

			pPlayer->EmitSound( filter, pPlayer->entindex(), params );
		}
#endif
	}

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_SECONDARY );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponRnLBaseRifle::HitMelee( trace_t &traceHit, Activity nHitActivity )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	//Make sound for the AI
	//CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer );

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

		CTakeDamageInfo info( GetOwner(), GetOwner(), GARANDMELEEDAMAGE, DMG_CLUB );

		if( pPlayer && pHitEntity->IsNPC() )
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
#ifdef GAME_DLL
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );
#endif
	}

	// Apply an impact effect
	ImpactEffectMelee( traceHit );
}

bool CWeaponRnLBaseRifle::StartSprinting( void )
{
	if( m_iBayoAnimState != 0 )
	{
		return false;
	}

	return BaseClass::StartSprinting();
}

// Play the proper transition animations before we attach or detach the bayo
void CWeaponRnLBaseRifle::BayonetTransition( int iState )
{
	CRnLPlayer *pPlayer = dynamic_cast< CRnLPlayer* >( GetOwner() );
	if ( !pPlayer )
		return;

	int iPosture = pPlayer->GetWeaponPosture();

	switch ( iPosture )
	{
	case WEAPON_POSTURE_HIP:
		{
		switch( iState )
			{		
				case 1:
				{
					HandleViewAnimation( WEAPON_ANIMATION_HIP_TO_SHOULDER );
					m_iBayoAnimState = 2;
					break;
				}
				case 2:
				{
					m_iBayoAnimState = 3;
					if( !m_bDeployed )
					{
						pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_ON );
						HandleViewAnimation( WEAPON_ANIMATION_ATTACH_BAYO );
						m_bDeployed = true;
					}
					else
					{
						pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_OFF );
						HandleViewAnimation( WEAPON_ANIMATION_DETACH_BAYO );
						m_bDeployed = false;
					}
					break;
				}
				case 3:
				{
					HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_HIP );
					m_iBayoAnimState = 0;
					break;
				}
			}
			break;
		}
	case WEAPON_POSTURE_SHOULDER: // No transition needed for shoulder
		{
			m_iBayoAnimState = 0;
			if( !m_bDeployed )
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_ON );
				HandleViewAnimation( WEAPON_ANIMATION_ATTACH_BAYO );
				m_bDeployed = true;
			}
			else
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_OFF );
				HandleViewAnimation( WEAPON_ANIMATION_DETACH_BAYO );
				m_bDeployed = false;
			}
			break;
		}
	case WEAPON_POSTURE_IRONSIGHTS:
	case WEAPON_POSTURE_SUPERSIGHTS:
		{
			switch( iState )
			{		
				case 1:
				{
#ifndef CLIENT_DLL
					// Switch back to shoulder fov first
					SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_SHOULDER] );
#endif
					HandleViewAnimation( WEAPON_ANIMATION_IS_TO_SHOULDER );
					m_iBayoAnimState = 2;
					break;
				}
				case 2:
				{
					m_iBayoAnimState = 3;
					if( !m_bDeployed )
					{
						pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_ON );
						HandleViewAnimation( WEAPON_ANIMATION_ATTACH_BAYO );
						m_bDeployed = true;
					}
					else
					{
						pPlayer->DoAnimationEvent( PLAYERANIMEVENT_BAYONET_OFF );
						HandleViewAnimation( WEAPON_ANIMATION_DETACH_BAYO );
						m_bDeployed = false;
					}
					break;
				}
				case 3:
				{
					HandleViewAnimation( WEAPON_ANIMATION_SHOULDER_TO_IS );
					m_iBayoAnimState = 0;
#ifndef CLIENT_DLL
					// Now move back
					SetNextFoV( GetRnLWpnData().m_flFieldOfView[WEAPON_POSTURE_SHOULDER] );
#endif
				}
			}
			break;
		}
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRnLBaseRifle::ImpactEffectMelee( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWaterMelee( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_DecalTrace( &traceHit, "ManhackCut" );
	//UTIL_ImpactTrace( &traceHit, DMG_CLUB, "ManhackCut" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRnLBaseRifle::ImpactWaterMelee( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if ( UTIL_PointContents( start ) & (CONTENTS_WATER|CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if ( !(UTIL_PointContents( end ) & (CONTENTS_WATER|CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, (CONTENTS_WATER|CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
#ifndef CLIENT_DLL
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
#endif
	}

	return true;
}

Activity CWeaponRnLBaseRifle::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = {mins.Base(), maxs.Base()};
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}

bool CWeaponRnLBaseRifle::HandleBayonetTransition( void )
{
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer && ( m_iBayoAnimState != 0 ) )
	{
		// Has the animations stopped playing yet? if so, play the next one
		if ( gpGlobals->curtime >= m_flNextPrimaryAttack ) /*the time that the actual animations takes*/
			BayonetTransition( m_iBayoAnimState );

		return true;
	}

	return false;
}

void CWeaponRnLBaseRifle::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	int iPosture = pPlayer->GetWeaponPosture();
	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;

	SetAnimationState( iAnim );

	if( iAnim == WEAPON_ANIMATION_ATTACH_BAYO )
	{
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_SPECIALONOPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_SPECIALON );
		}
	}
	else if( iAnim == WEAPON_ANIMATION_DETACH_BAYO )
	{
		if( bMagEmpty )
		{
			SendWeaponAnim( ACT_VM_SPECIALOFFOPEN );
		}
		else
		{
			SendWeaponAnim( ACT_VM_SPECIALOFF );
		}
	}
	else if( m_bDeployed )
	{
		switch( iAnim )
		{
		case WEAPON_ANIMATION_ATTACK:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_HIPFIREOPEN_DEPLOYED );	
				else
					SendWeaponAnim( ACT_VM_HIPFIRE1_DEPLOYED );

			}
			else if( iPosture  == WEAPON_POSTURE_SHOULDER )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SHOULDERFIREOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SHOULDERFIRE1_DEPLOYED );
			}
			else if( iPosture  == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SIGHTFIREOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SIGHTFIRE1_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_RELOAD:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_RELOAD_DEPLOYED );
			else
				SendWeaponAnim( ACT_VM_RELOADMID_DEPLOYED );
			break;

		case WEAPON_ANIMATION_DRAW:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_HIPDRAWOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_HIPDRAW_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SHOULDERDRAWOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SHOULDERDRAW_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SIGHTDRAWOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SIGHTDRAW_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_HOLSTER:
			if( iPosture == WEAPON_POSTURE_HIP )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_HIPHOLSTEROPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_HIPHOLSTER_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SHOULDERHOLSTEROPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SHOULDERHOLSTER_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SIGHTHOLSTEROPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SIGHTHOLSTER_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_IDLE:
			if( GetPlayerOwner()  && GetPlayerOwner()->IsProne()
				&& GetPlayerOwner()->GetLocalVelocity().Length2D() > SPEED_PRONE_WALK  )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_PRONE_IDLE_DEPLOYED_OPEN );
				else
					SendWeaponAnim( ACT_VM_PRONE_IDLE_DEPLOYED );

				SetAnimationState( WEAPON_ANIMATION_PRONE_IDLE );
			}
			else if( GetPlayerOwner() && GetPlayerOwner()->IsSprinting() )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SPRINT_IDLE_DEPLOYED_OPEN );
				else
					SendWeaponAnim( ACT_VM_SPRINT_IDLE_DEPLOYED );

				SetAnimationState( WEAPON_ANIMATION_SPRINT_IDLE );
			}
			else if( iPosture == WEAPON_POSTURE_HIP )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_HIPIDLEOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_HIPIDLE_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_SHOULDER )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SHOULDERIDLEOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SHOULDERIDLE_DEPLOYED );
			}
			else if( iPosture == WEAPON_POSTURE_IRONSIGHTS || iPosture == WEAPON_POSTURE_SUPERSIGHTS )
			{
				if( bMagEmpty )
					SendWeaponAnim( ACT_VM_SIGHTIDLEOPEN_DEPLOYED );
				else
					SendWeaponAnim( ACT_VM_SIGHTIDLE_DEPLOYED );
			}
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_IS:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SHOULDER2SIGHTOPEN_DEPLOYED );
			else
				SendWeaponAnim( ACT_VM_SHOULDER2SIGHT_DEPLOYED );
			break;

		case WEAPON_ANIMATION_IS_TO_SHOULDER:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SIGHT2SHOULDEROPEN_DEPLOYED );
			else
				SendWeaponAnim( ACT_VM_SIGHT2SHOULDER_DEPLOYED );
			break;

		case WEAPON_ANIMATION_SHOULDER_TO_HIP:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SHOULDER2HIPOPEN_DEPLOYED );
			else
				SendWeaponAnim( ACT_VM_SHOULDER2HIP_DEPLOYED );
			break;

		case WEAPON_ANIMATION_HIP_TO_SHOULDER:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_HIP2SHOULDEROPEN_DEPLOYED );
			else
				SendWeaponAnim( ACT_VM_HIP2SHOULDER_DEPLOYED );
			break;

		case WEAPON_ANIMATION_PRONE_ENTER:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_PRONE_ENTER_DEPLOYED_OPEN );
			else
				SendWeaponAnim( ACT_VM_PRONE_ENTER_DEPLOYED );
			break;
		case WEAPON_ANIMATION_PRONE_LEAVE:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_PRONE_LEAVE_DEPLOYED_OPEN );
			else
				SendWeaponAnim( ACT_VM_PRONE_LEAVE_DEPLOYED );
			break;

		case WEAPON_ANIMATION_SPRINT_ENTER:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SPRINT_ENTER_DEPLOYED_OPEN );
			else
				SendWeaponAnim( ACT_VM_SPRINT_ENTER_DEPLOYED );
			break;
		case WEAPON_ANIMATION_SPRINT_LEAVE:
			if( bMagEmpty )
				SendWeaponAnim( ACT_VM_SPRINT_LEAVE_DEPLOYED_OPEN );
			else
				SendWeaponAnim( ACT_VM_SPRINT_LEAVE_DEPLOYED );
			break;
		}
	}
	else
	{
		BaseClass::HandleViewAnimation( iAnim );
	}
}
