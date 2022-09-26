//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "rnl_explosive.h"
#include "rnl_shareddefs.h"


#ifdef CLIENT_DLL
	#include "c_rnl_player.h"
#else
	#include "soundent.h"
	#include "spark.h"

	BEGIN_DATADESC( CRnLBaseExplosive )
		DEFINE_THINKFUNC( DangerSoundThink ),
	END_DATADESC()
#endif


IMPLEMENT_NETWORKCLASS_ALIASED( RnLBaseExplosive, DT_RnLBaseExplosive )

BEGIN_NETWORK_TABLE( CRnLBaseExplosive, DT_RnLBaseExplosive )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL

	void CRnLBaseExplosive::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );

		if( type == DATA_UPDATE_CREATED && (LookupAttachment("wick") > 0) )
		{
			SetSequence( LookupSequence( "w_tnt_wick" ) );
			SetPlaybackRate( 1.0f );
		}
	}

	int CRnLBaseExplosive::DrawModel( int flags )
	{
		return BaseClass::DrawModel( flags );
	}

	void CRnLBaseExplosive::Spawn()
	{
		BaseClass::Spawn();
	}

#else

	void CRnLBaseExplosive::Spawn( void )
	{
		BaseClass::Spawn();

		SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_NONE );
		SetSolid( SOLID_NONE );	// So it will collide with physics props!

		// smaller, cube bounding box so we rest on the ground
		SetSize( Vector ( -2, -2, -2 ), Vector ( 2, 2, 2 ) );
	}

	void CRnLBaseExplosive::DangerSoundThink( void )
	{
		if( gpGlobals->curtime > m_flDetonateTime )
		{
			Detonate();
			return;
		}

		StudioFrameAdvanceManual( 0.5 );

		if( LookupAttachment("wick") > 0 )
		{
			Vector sparkPos; QAngle sparkAngle;
			GetAttachment( LookupAttachment("wick"), sparkPos, sparkAngle );
			Vector sparkDir( 0.0, 1.0, 0.0 );

			DoSpark( this, sparkPos, 1, 1, false, sparkDir );
		}

		SetNextThink( gpGlobals->curtime + 0.5 );
	}

	//Sets the time at which the grenade will explode
	void CRnLBaseExplosive::SetDetonateTimerLength( float timer )
	{
		if( LookupAttachment("wick") > 0 )
		{
			UseClientSideAnimation();
			SetSequence( LookupSequence( "w_tnt_wick" ) );
			SetPlaybackRate( 1.0f );
		}

		m_flDetonateTime = gpGlobals->curtime + timer;
	}

	void CRnLBaseExplosive::Detonate( void )
	{
		trace_t		tr;
		Vector		vecSpot;// trace starts here!

		SetThink( NULL );

		vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
		UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);

		if( tr.startsolid )
		{
			// Since we blindly moved the explosion origin vertically, we may have inadvertently moved the explosion into a solid,
			// in which case nothing is going to be harmed by the grenade's explosion because all subsequent traces will startsolid.
			// If this is the case, we do the downward trace again from the actual origin of the grenade. (sjb) 3/8/2007  (for ep2_outland_09)
			UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );
		}

		Explode( &tr, DMG_BLAST | DMG_RNL_TNT );

		if ( GetShakeAmplitude() )
		{
			UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
		}
	}
#endif
