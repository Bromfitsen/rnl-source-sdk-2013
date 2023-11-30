//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasemelee.h"
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

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBaseMelee, DT_WeaponRnLBaseMelee );

BEGIN_NETWORK_TABLE( CWeaponRnLBaseMelee, DT_WeaponRnLBaseMelee )
	PropBool( PROPINFO( m_bRightHand ) ),
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC(CWeaponRnLBaseMelee)
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA(CWeaponRnLBaseMelee)
		DEFINE_PRED_FIELD(m_bRightHand, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_basemelee, CWeaponRnLBaseMelee );

CWeaponRnLBaseMelee::CWeaponRnLBaseMelee()
{
}

CWeaponRnLBaseMelee::~CWeaponRnLBaseMelee()
{
}

void CWeaponRnLBaseMelee::PrimaryAttack()
{
	CRnLPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer ) 
		return;

	const char *weaponAlias = WeaponIDToAlias( GetWeaponID() );
	if ( !weaponAlias )
	{
		DevMsg("CWeaponRnLBaseMelee::PrimaryAttack(): weapon alias for ID %i not found\n", GetWeaponID() );
		return;
	}

	char wpnName[128];
	Q_snprintf( wpnName, sizeof( wpnName ), "weapon_%s", weaponAlias );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( wpnName );

	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		DevMsg("CWeaponRnLBaseMelee::PrimaryAttack(): LookupWeaponInfoSlot failed for weapon %s\n", wpnName );
		return;
	}

	CRnLWeaponInfo *pWeaponInfo = static_cast< CRnLWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );

	if( !pWeaponInfo )
	{
		DevMsg("CWeaponRnLBaseMelee::PrimaryAttack(): Weapon Info null for weapon %s\n", wpnName );
		return;
	}

	int iDamage = pWeaponInfo->m_iDamage;

	m_bRightHand.Set(random->RandomInt(0, 1) == 0);

	WeaponSound( SPECIAL1 );
	SendWeaponAnim( ACT_VM_SECSHOULDERFIRE1 );

	trace_t traceHit;
	Vector forward, swingStart = pPlayer->Weapon_ShootPosition();

	pPlayer->EyeVectors( &forward, NULL, NULL );

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit );

#ifndef CLIENT_DLL
	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( pPlayer, pPlayer, iDamage, DMG_CLUB );
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
				ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pPlayer );
			}
		}
	}

	//WeaponSound( SINGLE );
	HandleViewAnimation( WEAPON_ANIMATION_ATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_THROW_GRENADE );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
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
		HitMelee( traceHit, iDamage );

#ifndef CLIENT_DLL
		// Play the correct softimpact sound for the material hit.
		surfacedata_t *pSurf = physprops->GetSurfaceData( traceHit.surface.surfaceProps );
		if( pSurf )
		{
			EmitSound_t params;
			char buffer[128];

			Q_snprintf( buffer, 128, "%s.punch", physprops->GetString( pSurf->sounds.impactSoft ) );
			params.m_pSoundName = buffer;
			params.m_bWarnOnDirectWaveReference = true;

			CPASAttenuationFilter filter( pPlayer, params.m_pSoundName );

			pPlayer->EmitSound( filter, pPlayer->entindex(), params );
		}
#endif
	}
}

void CWeaponRnLBaseMelee::HitMelee( trace_t &traceHit, int iDamage )
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

		CTakeDamageInfo info( GetOwner(), GetOwner(), iDamage, DMG_CLUB );

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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRnLBaseMelee::ImpactEffectMelee( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWaterMelee( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_DecalTrace( &traceHit, "Impact.Concrete" );
	//UTIL_ImpactTrace( &traceHit, DMG_CLUB, "ManhackCut" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRnLBaseMelee::ImpactWaterMelee( const Vector &start, const Vector &end )
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

Activity CWeaponRnLBaseMelee::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
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

void CWeaponRnLBaseMelee::HandleViewAnimation( int iAnim )
{
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) 
		return;

	SetAnimationState( iAnim );

	switch( iAnim )
	{
	case WEAPON_ANIMATION_ATTACK:
		if( m_bRightHand )
			SendWeaponAnim( ACT_VM_SHOULDERFIRE1 );
		else
			SendWeaponAnim( ACT_VM_SHOULDERFIRE2 );
		break;

	case WEAPON_ANIMATION_DRAW:
		SendWeaponAnim( ACT_VM_SHOULDERDRAW );
		break;

	case WEAPON_ANIMATION_HOLSTER:
		SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );
		break;

	case WEAPON_ANIMATION_IDLE:
		if( GetPlayerOwner() && GetPlayerOwner()->IsProne()
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
			SendWeaponAnim( ACT_VM_SHOULDERIDLE );
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
