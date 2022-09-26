//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple model entity that randomly moves and changes direction
//			when activated.
//
//=============================================================================//

#include "cbase.h"
#include "rnl_air_support.h"
#ifndef CLIENT_DLL
	#include "explode.h"
#endif

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( rnl_air_support_vehicle, CRnLAirSupportVehicle );

IMPLEMENT_NETWORKCLASS_ALIASED(RnLAirSupportVehicle, DT_RnLAirSupportVehicle)
BEGIN_NETWORK_TABLE( CRnLAirSupportVehicle, DT_RnLAirSupportVehicle )
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	// Start of our data description for the class
	BEGIN_DATADESC( CRnLAirSupportVehicle )
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( rnl_air_support, CRnLAirSupport );

IMPLEMENT_NETWORKCLASS_ALIASED(RnLAirSupport, DT_RnLAirSupport)
BEGIN_NETWORK_TABLE( CRnLAirSupport, DT_RnLAirSupport )
#ifndef CLIENT_DLL
	SendPropInt(	SENDINFO( m_iSupportState ) ),
#else
	RecvPropInt(	RECVINFO( m_iSupportState ) ),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	// Start of our data description for the class
	BEGIN_DATADESC( CRnLAirSupport )
		DEFINE_KEYFIELD( m_iOwningTeam, FIELD_INTEGER, "owningteam" ),
		DEFINE_KEYFIELD( m_fSpeed, FIELD_FLOAT, "speed" ),
		DEFINE_KEYFIELD( m_fTravelTime, FIELD_FLOAT, "traveltime" ),
		DEFINE_KEYFIELD( m_fYawOffset, FIELD_FLOAT, "rotateoffset" ),
		DEFINE_KEYFIELD( m_fBombingRadius, FIELD_FLOAT, "bombingradius" ),
		DEFINE_KEYFIELD( m_flBombDelay, FIELD_FLOAT, "bombdelay" ),
		DEFINE_KEYFIELD( m_iSupportForceSize, FIELD_INTEGER, "supportforce" ),

		// Declare our think function
		DEFINE_THINKFUNC( BombingRunThink ),

		// Links our input name from Hammer to our input member function
		DEFINE_INPUTFUNC( FIELD_STRING, "BombTarget", InputBombTarget ),

		DEFINE_OUTPUT( m_OnBeginFlight,			"OnBeginFlight" ),
		DEFINE_OUTPUT( m_OnFinishFlight,		"OnEndFlight" ),
		DEFINE_OUTPUT( m_OnBeginBombingRun,		"OnBombingBegin" ),
		DEFINE_OUTPUT( m_OnFinishBombingRun,	"OnBombingEnd" ),

	END_DATADESC()
#endif

CRnLAirSupportVehicle::CRnLAirSupportVehicle()
{
}

void CRnLAirSupportVehicle::Precache( void )
{
	BaseClass::Precache();
}

void CRnLAirSupportVehicle::Spawn( void )
{
	BaseClass::Spawn();
}

CRnLAirSupport::CRnLAirSupport()
{
#ifndef CLIENT_DLL
	m_fYawOffset = 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CRnLAirSupport::Precache( void )
{
	PrecacheModel( STRING(GetModelName()) );
#ifndef CLIENT_DLL
	m_iBombModelIndex = PrecacheModel( "models/props_vehicles/german/stuka01_bomb.mdl" );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CRnLAirSupport::Spawn( void )
{
	BaseClass::Spawn();

#ifndef CLIENT_DLL
	Precache();

	if( m_iSupportForceSize < 1 )
		m_iSupportForceSize = 1;
	else if( m_iSupportForceSize > 5 )
		m_iSupportForceSize = 5;

	Vector offset( 0.0f, 0.0f, 0.0f );
	Vector boundingSize;
	for( int i = 0; i < m_iSupportForceSize; i++ )
	{
		int index = m_hSupportVehicles.AddToTail();
		m_hSupportVehicles[index] = (CRnLAirSupportVehicle*)CreateEntityByName( "rnl_air_support_vehicle" );
		if( m_hSupportVehicles[index] != NULL )
		{
			m_hSupportVehicles[index]->SetModel( STRING(GetModelName()) );
			m_hSupportVehicles[index]->SetMoveType( MOVETYPE_NOCLIP );
			m_hSupportVehicles[index]->SetSolid( SOLID_NONE );	// So it will collide with physics props!

			m_hSupportVehicles[index]->UseClientSideAnimation();
			m_hSupportVehicles[index]->ResetSequence( ACT_IDLE );

			m_hSupportVehicles[index]->m_takedamage = DAMAGE_NO;
			m_hSupportVehicles[index]->ChangeTeam( m_iOwningTeam );
			m_hSupportVehicles[index]->AddEffects( EF_NODRAW );

			if( i > 0 )
			{
				if( (i % 2) != 0 )
				{
					if( i == 1 )
					{
						m_hSupportVehicles[index]->SetAbsOrigin( m_hSupportVehicles[index-1]->GetAbsOrigin() + offset );
						m_hSupportVehicles[index]->SetAbsAngles( m_hSupportVehicles[index-1]->GetAbsAngles() );
						m_hSupportVehicles[index]->SetParent( m_hSupportVehicles[index-1] );
					}
					else
					{
						m_hSupportVehicles[index]->SetAbsOrigin( m_hSupportVehicles[index-2]->GetAbsOrigin() + offset );
						m_hSupportVehicles[index]->SetAbsAngles( m_hSupportVehicles[index-2]->GetAbsAngles() );
						m_hSupportVehicles[index]->SetParent( m_hSupportVehicles[index-2] );
					}
				}
				else
				{
					m_hSupportVehicles[index]->SetAbsOrigin( m_hSupportVehicles[index-2]->GetAbsOrigin() + Vector( -offset.x, offset.y, 0 ) );
					m_hSupportVehicles[index]->SetAbsAngles( m_hSupportVehicles[index-2]->GetAbsAngles() );
					m_hSupportVehicles[index]->SetParent( m_hSupportVehicles[index-2] );

					offset.x += boundingSize.x * 0.75;
					offset.y += boundingSize.y * 0.75;
				}
			}
			else
			{
				boundingSize = m_hSupportVehicles[index]->WorldAlignMaxs() - m_hSupportVehicles[index]->WorldAlignMins();

				offset.x = boundingSize.x;
				offset.y = boundingSize.y / 2;
			}
		}
	}

	m_takedamage = DAMAGE_NO;
	ChangeTeam( m_iOwningTeam );

	m_hEntryPoint = (CPointEntity*)gEntList.FindEntityByName( NULL, m_target );
#endif

	m_iSupportState = RNL_AIRSUPPORT_STATE_NONE;
}

#ifndef CLIENT_DLL
	#define PLANE_TRAVEL_SPEED 512.0f
	#define	PLANE_START_DISTANCE ( -2.0f * PLANE_TRAVEL_SPEED )
	#define	PLANE_END_DISTANCE ( 5.0f * PLANE_TRAVEL_SPEED )
	#define	PLANE_EXIT_DISTANCE 32.0f
	#define	PLANE_EXIT_DISTANCE_SQR (PLANE_EXIT_DISTANCE * PLANE_EXIT_DISTANCE)
	#define PLANE_ATTACK_DISTANCE 1024.0f
	#define PLANE_ATTACK_DISTANCE_SQR (PLANE_ATTACK_DISTANCE * PLANE_ATTACK_DISTANCE)

	extern short	g_sModelIndexWExplosion;
	void CRnLAirSupport::BombingRunThink( void )
	{
		if( m_iSupportState == RNL_AIRSUPPORT_BOMBING_INITIALISED )
		{
			m_iSupportState = RNL_AIRSUPPORT_BOMBING_APPROACHING;
			m_OnBeginFlight.FireOutput( this, this );
		}
		else if( m_iSupportState == RNL_AIRSUPPORT_BOMBING_APPROACHING )
		{
			if( m_vPositionTarget.DistToSqr( m_hSupportVehicles[0]->GetAbsOrigin() ) < (m_fBombingRadius*m_fBombingRadius) )
			{
				m_iSupportState = RNL_AIRSUPPORT_BOMBING_IN_PROGRESS;
				m_OnBeginBombingRun.FireOutput( this, this );
			}
		}
		else if( m_iSupportState == RNL_AIRSUPPORT_BOMBING_IN_PROGRESS )
		{
			if( m_vPositionTarget.DistToSqr( m_hSupportVehicles[0]->GetAbsOrigin() ) > (m_fBombingRadius*m_fBombingRadius) )
			{
				m_iSupportState = RNL_AIRSUPPORT_BOMBING_DEPARTING;
				m_OnFinishBombingRun.FireOutput( this, this );
			}
			else if( m_flNextBombTime < gpGlobals->curtime )
			{
				m_iBomber++;
				if( m_iBomber >= m_hSupportVehicles.Count() )
					m_iBomber = 0;

				CPASFilter filter( m_hSupportVehicles[m_iBomber]->GetAbsOrigin() );
				Vector origin = m_hSupportVehicles[m_iBomber]->GetAbsOrigin() - Vector( 0, 0, 50 );
				Vector velocity = m_hSupportVehicles[m_iBomber]->GetAbsVelocity() + Vector( 0, 0, -500 );
				Vector dir = velocity;
				dir.NormalizeInPlace();

				trace_t tr;
				UTIL_TraceLine( origin, origin + (dir * 5000), MASK_SHOT, m_hSupportVehicles[m_iBomber], COLLISION_GROUP_NONE, &tr );

				int indx = m_vDelayedEvents.AddToTail();
				m_vDelayedEvents[indx].time = (10.0f * tr.fraction) + gpGlobals->curtime;
				m_vDelayedEvents[indx].position = tr.endpos;

				te->ClientProjectile( filter, 0.0,
					&origin, &velocity, m_iBombModelIndex, (10.0f * tr.fraction), m_hSupportVehicles[m_iBomber] );
	
				if( developer.GetBool() )
					NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 255, 255, 255, 4 );

				m_flNextBombTime = gpGlobals->curtime + m_flBombDelay;
			}
		}
		else if( m_iSupportState == RNL_AIRSUPPORT_BOMBING_DEPARTING )
		{
			if( m_vExitTarget.DistToSqr( m_hSupportVehicles[0]->GetAbsOrigin() ) < PLANE_EXIT_DISTANCE_SQR )
			{
				m_iSupportState = RNL_AIRSUPPORT_STATE_NONE;
				m_OnFinishFlight.FireOutput( this, this );
				m_hSupportVehicles[0]->SetAbsVelocity( vec3_origin );
				for( int i = 0; i < m_hSupportVehicles.Count(); i++ )
					m_hSupportVehicles[i]->AddEffects( EF_NODRAW );
				SetThink( NULL );
				return;
			}
		}

		for( int i = 0; i < m_vDelayedEvents.Count(); i++ )
		{
			if( m_vDelayedEvents[i].time < gpGlobals->curtime )
			{
				CDisablePredictionFiltering disabler;
				Vector vecAbsOrigin =  m_vDelayedEvents[i].position + Vector( 0, 0, 5 );
				UTIL_ScreenShake( vecAbsOrigin, 45.0f, 150.0, 1.0, 1024, SHAKE_START );
				int contents = UTIL_PointContents ( vecAbsOrigin );

				CPASFilter filter( vecAbsOrigin );
				te->Explosion( filter, -1.0, // don't apply cl_interp delay
							&vecAbsOrigin, 
							!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
							5.0f, 
							25,
							TE_EXPLFLAG_NONE,
							512,
							150 );

				CTakeDamageInfo info( this, this, vec3_origin, GetAbsOrigin(), 150, DMG_BLAST, 0, &vecAbsOrigin );
				RadiusDamage( info, vecAbsOrigin, 512, CLASS_NONE, NULL );
				EmitSound( "BaseGrenade.Explode" );
				m_vDelayedEvents.FastRemove( i );
				i--;

			}
		}

		SetNextThink( gpGlobals->curtime + 0.05f );
	}

	void CRnLAirSupport::BombingInitialise( Vector vecTarget )
	{
		Vector entryPoint = m_hEntryPoint->GetAbsOrigin();
		vecTarget.z = entryPoint.z;

		Vector dirToTarget = vecTarget - entryPoint;
		dirToTarget.NormalizeInPlace();

		m_hSupportVehicles[0]->SetAbsOrigin( entryPoint - (dirToTarget * ( m_fTravelTime * m_fSpeed ) ) );
		m_hSupportVehicles[0]->SetAbsVelocity( dirToTarget * m_fSpeed );

		// Snap our facing to where we're heading
		QAngle angFacing;
		VectorAngles( dirToTarget, angFacing );
		angFacing[YAW] += m_fYawOffset;
		m_hSupportVehicles[0]->SetAbsAngles( angFacing );

		m_vPositionTarget = vecTarget;
		m_vExitTarget = vecTarget + (dirToTarget * m_fBombingRadius) + (dirToTarget * ( m_fTravelTime * m_fSpeed ) );

		m_iSupportState = RNL_AIRSUPPORT_BOMBING_INITIALISED;
		
		for( int i = 0; i < m_hSupportVehicles.Count(); i++ )
			m_hSupportVehicles[i]->RemoveEffects( EF_NODRAW );

		m_vDelayedEvents.Purge();
		m_iBomber = 0;
		m_flNextBombTime = gpGlobals->curtime;

		// Start thinking
		SetThink( &CRnLAirSupport::BombingRunThink );
		SetNextThink( gpGlobals->curtime + 0.05f );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Toggle the movement of the entity
	//-----------------------------------------------------------------------------
	void CRnLAirSupport::InputBombTarget( inputdata_t &inputData )
	{
		if( m_iSupportState != RNL_AIRSUPPORT_STATE_NONE )
			return;

		if( m_hEntryPoint.Get() == NULL )
			return;

		const char *pszEntityToForget = inputData.value.String();
		CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, pszEntityToForget );
		if ( pEntity )
			BombingInitialise( pEntity->GetAbsOrigin() );
	
	}
#endif