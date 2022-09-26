#include "cbase.h"
#include "rnl_task_perimeter.h"
#include "rnl_gamerules.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(RnLTaskCapturePerimeter, DT_RnLTaskCapturePerimeter)
BEGIN_NETWORK_TABLE( CRnLTaskCapturePerimeter, DT_RnLTaskCapturePerimeter )
#ifndef CLIENT_DLL
	SendPropInt(	SENDINFO( m_iCaptureRadius ) ),
#else
	RecvPropInt(	RECVINFO( m_iCaptureRadius ) ),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLTaskCapturePerimeter )
		DEFINE_KEYFIELD( m_flCaptureDuration, FIELD_FLOAT, "time" ),
		DEFINE_KEYFIELD( m_iCaptureRadius, FIELD_INTEGER, "radius" ),
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( rnl_task_perimeter_capture, CRnLTaskCapturePerimeter );

CRnLTaskCapturePerimeter::CRnLTaskCapturePerimeter()
{
#ifndef CLIENT_DLL
	//m_iCapturePointName = 0;
	m_flCaptureDuration = 10.0f;
	m_flCaptureTimeRemaining = 0.0;
#endif
	m_iCaptureRadius = 300;
}

#ifndef CLIENT_DLL
	void CRnLTaskCapturePerimeter::Spawn( void )
	{
		BaseClass::Spawn();
		SetNextThink( gpGlobals->curtime + 1.0);

		m_vTaskExtentMin = GetAbsOrigin() - Vector( m_iCaptureRadius, m_iCaptureRadius, m_iCaptureRadius );
		m_vTaskExtentMax = GetAbsOrigin() + Vector( m_iCaptureRadius, m_iCaptureRadius, m_iCaptureRadius );
	}

	void CRnLTaskCapturePerimeter::Think()
	{
		CBaseEntity *pEntity = NULL;
		CBasePlayer *pPlayer = NULL;
		int iAxisCount = 0;
		int iAlliesCount = 0;
		int iDominantTeam = TEAM_NONE;

		for ( CEntitySphereQuery sphere( GetAbsOrigin(), m_iCaptureRadius, FL_CLIENT | FL_FAKECLIENT ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( pEntity == NULL )
				continue;

			if( pEntity->IsPlayer() == false )
				continue;

			pPlayer = ToBasePlayer( pEntity );

			if( pPlayer == NULL )
				continue;

			if( pPlayer->IsDead() || pPlayer->IsObserver() )
				continue;

			if( pPlayer->GetTeamNumber() == TEAM_AXIS )
				iAxisCount++;
			else if( pPlayer->GetTeamNumber() == TEAM_ALLIES )
				iAlliesCount++;
		}

		// Set the dominant team, but only if they are allowed to cap the point.
		if( iAxisCount > iAlliesCount && AreRequirementsMet( TEAM_AXIS ) )
		{
			if( m_iCapturableTeam == -1 || m_iCapturableTeam == TEAM_AXIS )
				iDominantTeam = TEAM_AXIS;
		}
		else if( iAlliesCount > iAxisCount && AreRequirementsMet( TEAM_ALLIES ) )
		{
			if( m_iCapturableTeam == -1 || m_iCapturableTeam == TEAM_ALLIES )
				iDominantTeam = TEAM_ALLIES;
		}

		if( iAxisCount != GetAxisCount() )
			SetAxisCount( iAxisCount );

		if( iAlliesCount != GetAlliesCount() )
			SetAlliesCount( iAlliesCount );


		switch( GetTaskState() )
		{
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
			if( iAlliesCount < GetRequiredAlliesCount() || iAlliesCount < iAxisCount )
			{
				SetCapturePercent( 0 );
				SetTaskState( RNL_TASK_NEUTRAL );
			}
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
			if( iAlliesCount < GetRequiredAlliesCount() || iAlliesCount < iAxisCount )
			{
				SetCapturePercent( 0 );
				SetTaskState( RNL_TASK_AXIS_CONTROLLED );
			}
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
			if( iAxisCount < GetRequiredAxisCount() || iAlliesCount > iAxisCount )
			{
				SetCapturePercent( 0 );
				SetTaskState(  RNL_TASK_NEUTRAL );
			}
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
			if( iAxisCount < GetRequiredAxisCount() || iAlliesCount > iAxisCount  )
			{
				SetCapturePercent( 0 );
				SetTaskState(  RNL_TASK_ALLIED_CONTROLLED );
			}
			break;
		case RNL_TASK_NEUTRAL:
			if( iAxisCount >= GetRequiredAxisCount() && iDominantTeam == TEAM_AXIS )
			{
				SetCapturePercent( 0 );
				SetTaskState( RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL );
				m_flCaptureTimeRemaining = m_flCaptureDuration;
			}
			else if( iAlliesCount >= GetRequiredAlliesCount() && iDominantTeam == TEAM_ALLIES )
			{
				SetCapturePercent( 0 );
				SetTaskState(  RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL );
				m_flCaptureTimeRemaining = m_flCaptureDuration;
			}
			break;
		case RNL_TASK_ALLIED_CONTROLLED:
			if( iAxisCount >= GetRequiredAxisCount() && iDominantTeam == TEAM_AXIS )
			{
				SetCapturePercent( 0 );
				SetTaskState( RNL_TASK_AXIS_CAPTURING_FROM_ALLIED );
				m_flCaptureTimeRemaining = m_flCaptureDuration;
			}
			break;
		case RNL_TASK_AXIS_CONTROLLED:
			if( iAlliesCount >= GetRequiredAlliesCount() && iDominantTeam == TEAM_ALLIES )
			{
				SetCapturePercent( 0 );
				SetTaskState( RNL_TASK_ALLIED_CAPTURING_FROM_AXIS );
				m_flCaptureTimeRemaining = m_flCaptureDuration;
			}
			break;
		}

		// Capture Transition
		if( GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_AXIS ||  GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL
			|| GetTaskState() == RNL_TASK_AXIS_CAPTURING_FROM_ALLIED || GetTaskState() ==  RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL )
		{
			//deduct our refresh from the capture time
			m_flCaptureTimeRemaining -= 1.0;

			if( GetCapturePercent() >= 100 )
			{
				SetCapturePercent( 100 );
				
				if( GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL || GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_AXIS )
					SetTaskState( RNL_TASK_ALLIED_CONTROLLED );
				else if( GetTaskState() == RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL || GetTaskState() == RNL_TASK_AXIS_CAPTURING_FROM_ALLIED )
					SetTaskState( RNL_TASK_AXIS_CONTROLLED );
			}
			else
			{
				float flCountInCap = 0.0, flCapturingCount = 0.0, flOtherCount = 0.0, flTeamBonus = 0.0;
				float flCurCapturePercent = (float)GetCapturePercent();

				if( GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_AXIS || GetTaskState() == RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL )
				{
					flCountInCap = (float)GetAlliesCount();
					if( RnLGameRules() )
					{
						flCapturingCount = (float)RnLGameRules()->GetTeamCount( TEAM_ALLIES );
						flOtherCount = (float)RnLGameRules()->GetTeamCount( TEAM_AXIS );
					}
				}
				else if ( GetTaskState() == RNL_TASK_AXIS_CAPTURING_FROM_ALLIED || GetTaskState() ==  RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL )
				{
					flCountInCap = (float)GetAxisCount();
					if( RnLGameRules() )
					{
						flCapturingCount = (float)RnLGameRules()->GetTeamCount( TEAM_AXIS );
						flOtherCount = (float)RnLGameRules()->GetTeamCount( TEAM_ALLIES );
					}
				}

				if( flCapturingCount < 1 )
					flCapturingCount = 1;
				if( flOtherCount < 1 )
					flOtherCount = 1;

				flTeamBonus = (flOtherCount / flCapturingCount);

				if( flTeamBonus > 3.0 )
					flTeamBonus = 3.0;

				float flCapturePercent = ((m_flCaptureDuration - m_flCaptureTimeRemaining) / m_flCaptureDuration) * 100.0f;
				flCapturePercent += flCountInCap;
				flCapturePercent *= flTeamBonus;

				SetCapturePercent( flCurCapturePercent + (flCapturePercent - flCurCapturePercent));
			}
		}

		if ( developer.GetBool() )
		{
			if( GetTaskState() > RNL_TASK_ALLIED_STATES_MIN && GetTaskState() < RNL_TASK_ALLIED_STATES_MAX )
			{
				NDebugOverlay::Sphere( GetAbsOrigin(), QAngle( 0, 0, 0 ), m_iCaptureRadius, 0, 150, 0, 255, 0, 1.2 );
			}
			else if( GetTaskState() > RNL_TASK_AXIS_STATES_MIN && GetTaskState() < RNL_TASK_AXIS_STATES_MAX )
			{
				NDebugOverlay::Sphere( GetAbsOrigin(), QAngle( 0, 0, 0 ), m_iCaptureRadius, 200, 0, 0, 255, 0, 1.2 );
			}
			else
			{
				NDebugOverlay::Sphere( GetAbsOrigin(), QAngle( 0, 0, 0 ), m_iCaptureRadius, 127, 127, 127, 255, 0, 1.2 );
			}
		}
		SetNextThink( gpGlobals->curtime + 1.0f );
	}
#endif