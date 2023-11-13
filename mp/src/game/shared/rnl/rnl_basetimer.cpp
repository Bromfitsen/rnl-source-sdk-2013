//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic Timer!
//
//			Simplified Version of Valves <^^,>
//
// $NoKeywords: $Big Titties!
//=============================================================================//

#include "cbase.h"
#include "rnl_basetimer.h"
#include "rnl_dt_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( rnl_timer, CRnLBaseTimer );

IMPLEMENT_NETWORKCLASS_ALIASED( RnLBaseTimer, DT_RnLBaseTimer )

BEGIN_NETWORK_TABLE_NOBASE( CRnLBaseTimer, DT_RnLBaseTimer )
	PropTime( PROPINFO( m_flTimerEndTime ) ),
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
BEGIN_DATADESC(CRnLBaseTimer)
	DEFINE_KEYFIELD( m_flTimerLength,	FIELD_FLOAT,	"timer_length" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER,	"SetTime",			InputSetTime ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,	"AddTime",			InputAddTime ),
	DEFINE_INPUTFUNC( FIELD_STRING,		"AddTeamTime",		InputAddTeamTime ),

END_DATADESC();
#endif

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CRnLBaseTimer::CRnLBaseTimer( void )
{
	m_flTimerEndTime = 0;
#ifndef CLIENT_DLL
	m_flTimerLength = 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
CRnLBaseTimer::~CRnLBaseTimer( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
void CRnLBaseTimer::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLBaseTimer::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLBaseTimer::Spawn( void )
{
	Precache();

#ifndef CLIENT_DLL
	m_flTimerEndTime = gpGlobals->curtime + (m_flTimerLength*60.0f);
#endif

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Gets the seconds left on the timer, paused or not.
//-----------------------------------------------------------------------------
float CRnLBaseTimer::GetTimeRemaining( void )
{
	float flSecondsRemaining = m_flTimerEndTime - gpGlobals->curtime;

	if ( flSecondsRemaining < 0 )
	{
		flSecondsRemaining = 0.0f;
	}

	return flSecondsRemaining;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: To set the initial timer duration
//-----------------------------------------------------------------------------
void CRnLBaseTimer::SetTimeRemaining( int iTimerSeconds )
{
	m_flTimerEndTime = gpGlobals->curtime + (float)iTimerSeconds;
}

//-----------------------------------------------------------------------------
// Purpose: Add seconds to the timer while it is running or paused
//-----------------------------------------------------------------------------
void CRnLBaseTimer::AddTimerSeconds( int iSecondsToAdd, int iTeamResponsible /* = TEAM_UNASSIGNED*/ )
{
	m_flTimerEndTime += (float)iSecondsToAdd;
}

//-----------------------------------------------------------------------------
// Purpose: The timer is always transmitted to clients
//-----------------------------------------------------------------------------
int CRnLBaseTimer::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLBaseTimer::InputSetTime( inputdata_t &input )
{
	int nSeconds = input.value.Int();
	SetTimeRemaining( nSeconds );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLBaseTimer::InputAddTime( inputdata_t &input )
{
	int nSeconds = input.value.Int();
	AddTimerSeconds( nSeconds );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLBaseTimer::InputAddTeamTime( inputdata_t &input )
{
	char		token[128];
	const char	*p = STRING( input.value.StringID() );
	int			nTeam = TEAM_UNASSIGNED;
	int			nSeconds = 0;

	// get the team
	p = nexttoken( token, p, ' ' );
	if ( token )
	{
		nTeam = Q_atoi( token );
	}

	// get the time
	p = nexttoken( token, p, ' ' );
	if ( token )
	{
		nSeconds = Q_atoi( token );
	}

	if ( nSeconds != 0 )
	{
		AddTimerSeconds( nSeconds, nTeam );
	}
}
#endif
