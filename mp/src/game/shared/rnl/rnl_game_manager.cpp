//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic Timer!
//
//			Simplified Version of Valves <^^,>
//
// $NoKeywords: $Big Titties!
//=============================================================================//

#include "cbase.h"
#include "rnl_gamerules.h"
#include "rnl_game_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( rnl_game_manager, CRnLGameManager );

IMPLEMENT_NETWORKCLASS_ALIASED( RnLGameManager, DT_RnLGameManager )

BEGIN_NETWORK_TABLE_NOBASE( CRnLGameManager, DT_RnLGameManager )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_fAlliedSpawnTime ) ),
	RecvPropTime( RECVINFO( m_fAxisSpawnTime ) ),

	RecvPropInt( RECVINFO( m_iAxisTickets ) ),
	RecvPropInt( RECVINFO( m_iAlliedTickets ) ),
#else
	SendPropTime( SENDINFO( m_fAlliedSpawnTime ) ),
	SendPropTime( SENDINFO( m_fAxisSpawnTime ) ),

	SendPropInt( SENDINFO( m_iAxisTickets ) ),
	SendPropInt( SENDINFO( m_iAlliedTickets ) ),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
BEGIN_DATADESC(CRnLGameManager)
	DEFINE_KEYFIELD( m_fHoldToWinDuration,		FIELD_FLOAT,	"hold_timer_length" ),
	DEFINE_KEYFIELD( m_fAlliedSpawnDelay,		FIELD_FLOAT,	"allied_spawn_length" ),
	DEFINE_KEYFIELD( m_fAxisSpawnDelay,			FIELD_FLOAT,	"axis_spawn_length" ),
	DEFINE_KEYFIELD( m_iAxisInitialTickets,		FIELD_INTEGER,	"axis_tickets" ),
	DEFINE_KEYFIELD( m_iAlliedInitialTickets,	FIELD_INTEGER,	"allied_tickets" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER,	"SetAxisTickets",	InputSetAxisTickets ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,	"SetAlliedTickets",	InputSetAlliedTickets ),
	DEFINE_INPUTFUNC( FIELD_STRING,		"SetTickets",		InputSetTickets ),

	DEFINE_INPUTFUNC( FIELD_INTEGER,	"AddAxisTickets",	InputAddAxisTickets ),
	DEFINE_INPUTFUNC( FIELD_INTEGER,	"AddAlliedTickets",	InputAddAlliedTickets ),
	DEFINE_INPUTFUNC( FIELD_STRING,		"AddTickets",		InputAddTickets ),

END_DATADESC();

	/*Michael Lebson
	Let server owners override the default axis and allied spawn timers and tickets*/
	void AlliedTickets_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue );
	void AxisTickets_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue );

	ConVar	mp_alliedspawntime( "mp_alliedspawntime", "-1", FCVAR_NOTIFY, "Override a map's allied spawn time. If set to less than one, one will be used. Set to -1 to use map's default." );
	ConVar	mp_axisspawntime( "mp_axisspawntime", "-1", FCVAR_NOTIFY, "Override a map's axis spawn time. If set to less than one, one will be used. Set to -1 to use map's default." );\
	ConVar	mp_alliedtickets( "mp_alliedtickets", "-1", FCVAR_NOTIFY, "Override a map's allied starting tickets count. If set to less than one, one will be used. If changed during round, remaining tickets will change by the difference. If changed from a vlue to -1, remaining tickets will be reset to map default. Set to -1 to use map's default.", AlliedTickets_ChangeCallback );
	ConVar	mp_axistickets( "mp_axistickets", "-1", FCVAR_NOTIFY, "Override a map's axis starting tickets count. If set to less than one, one will be used.  If changed during round, remaining tickets will change by the difference. If changed from a vlue to -1, remaining tickets will be reset to map default. Set to -1 to use map's default.", AxisTickets_ChangeCallback );

	void AlliedTickets_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue )
	{
		ConVarRef var( pConVar );

		// If this is set to anything < 1, but great then 0,
		// set it to 1 instead. If it is less than 0, leave it
		// alone and use map defaults.
		if( var.GetFloat() < 1.0 && var.GetFloat() >= 0.0 )
			var.SetValue( (float)1.0 );

		if( RnLGameRules() && RnLGameRules()->GetGameManager() )
		{
			// If this is less than 0, reset the tickets
			// to use the starting amount as set by
			// the mapper.
			if( var.GetFloat() < 0.0 )
				RnLGameRules()->GetGameManager()->SetAlliedTicketsRemaining( RnLGameRules()->GetGameManager()->GetAlliedInitialTickets() );
			else
			{
				// Get the difference between old ticket value and new,
				// but use the mapper's tickets if we are changing from -1
				float flValue = flOldValue > 0 ? flOldValue : RnLGameRules()->GetGameManager()->GetAlliedInitialTickets();

				RnLGameRules()->GetGameManager()->AddAlliedTickets( (int)(var.GetFloat() - flValue) );
			}
		}
	}

	void AxisTickets_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue )
	{
		ConVarRef var( pConVar );

		// If this is set to anything < 1, but great then 0,
		// set it to 1 instead. If it is less than 0, leave it
		// alone and use map defaults.
		if( var.GetFloat() < 1.0 && var.GetFloat() >= 0.0)
			var.SetValue( (float)1.0 );

		if( RnLGameRules() && RnLGameRules()->GetGameManager() )
		{
			// If this is less than 0, reset the tickets
			// to use the starting amount as set by
			// the mapper.
			if( var.GetFloat() < 0 )
				RnLGameRules()->GetGameManager()->SetAxisTicketsRemaining( RnLGameRules()->GetGameManager()->GetAxisInitialTickets() );
			else
			{
				// Get the difference between old ticket value and new,
				// but use the mapper's tickets if we are changing from -1
				float flValue = flOldValue > 0 ? flOldValue : RnLGameRules()->GetGameManager()->GetAxisInitialTickets();

				RnLGameRules()->GetGameManager()->AddAxisTickets( (int)(var.GetFloat() - flValue) );
			}
		}
	}
#endif

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CRnLGameManager::CRnLGameManager( void )
{
	m_iAlliedTickets = 0;
	m_iAxisTickets = 0;
#ifndef CLIENT_DLL 
	m_fHoldToWinDuration = 0;

	m_fAlliedSpawnDelay = 0;
	m_fAxisSpawnDelay = 0;

	m_iAlliedInitialTickets = 0;
	m_iAxisInitialTickets = 0;
#endif 
}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
CRnLGameManager::~CRnLGameManager( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: destructor
//-----------------------------------------------------------------------------
void CRnLGameManager::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::Spawn( void )
{
	Precache();

#ifndef CLIENT_DLL 
	m_iAlliedTickets = m_iAlliedInitialTickets;
	m_iAxisTickets = m_iAxisInitialTickets;

	m_fAxisSpawnTime = 0;
	m_fAlliedSpawnTime = 0;
#endif

	BaseClass::Spawn();
}

#ifndef CLIENT_DLL 
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	int CRnLGameManager::UpdateTransmitState()
	{
		// ALWAYS transmit to all clients.
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	#define SPAWN_TIMER_DELAY -0.2f
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameManager::Update()
	{
		if(GetAxisSpawnTimer() < SPAWN_TIMER_DELAY)
		{
			float flSpawnDelay = m_fAxisSpawnDelay;

			/*Michael Lebson
			Let server owners override the default axis spawn timer.*/
			if( mp_axisspawntime.GetFloat() >= 0 )
				flSpawnDelay = mp_axisspawntime.GetFloat();

			if( flSpawnDelay < 1 )
				flSpawnDelay = 1;

			if( RnLGameRules() )
			{
				float flAlliesCount = (float)RnLGameRules()->GetTeamCount( TEAM_ALLIES );

				if( flAlliesCount < 1 )
					flAlliesCount = 1;

				float flScale = (RnLGameRules()->GetTeamCount( TEAM_AXIS ) / flAlliesCount);
				
				if( flScale < 1.0 )
					flScale = 1.0;

				flSpawnDelay *= flScale;
			}

			m_fAxisSpawnTime = gpGlobals->curtime + flSpawnDelay;
		}
		if(GetAlliedSpawnTimer() < SPAWN_TIMER_DELAY)
		{
			float flSpawnDelay = m_fAlliedSpawnDelay;

			/*Michael Lebson
			Let server owners override the default allied spawn timer.*/
			if( mp_alliedspawntime.GetFloat() >= 0 )
				flSpawnDelay = mp_alliedspawntime.GetFloat();

			if( flSpawnDelay < 1 )
				flSpawnDelay = 1;
			
			if( RnLGameRules() )
			{
				float flAxisCount = (float)RnLGameRules()->GetTeamCount( TEAM_AXIS );

				if( flAxisCount < 1 )
					flAxisCount = 1;

				float flScale = (RnLGameRules()->GetTeamCount( TEAM_AXIS ) / flAxisCount);

				if( flScale < 1.0 )
					flScale = 1.0;

				flSpawnDelay *= flScale;
			}

			m_fAlliedSpawnTime = gpGlobals->curtime + flSpawnDelay;
		}
	}

	float CRnLGameManager::GetHoldToWinTime( void )
	{
		return m_fHoldToWinDuration;
	}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CRnLGameManager::GetSpawnTimer( int iTeam )
{
	if( iTeam == TEAM_AXIS )
	{
		return GetAxisSpawnTimer();
	}
	else if( iTeam == TEAM_ALLIES )
	{
		return GetAlliedSpawnTimer();
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CRnLGameManager::GetAxisSpawnTimer( void )
{
	return (m_fAxisSpawnTime - gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CRnLGameManager::GetAlliedSpawnTimer( void )
{
	return (m_fAlliedSpawnTime - gpGlobals->curtime);
}

#ifndef CLIENT_DLL
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameManager::SetSpawnTimer( int iTeam, float flDuration )
	{
		if( iTeam == TEAM_AXIS )
		{
			SetAxisSpawnTimer(flDuration);
		}
		else if( iTeam == TEAM_ALLIES )
		{
			SetAlliedSpawnTimer(flDuration);
		}
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameManager::SetAxisSpawnTimer( float flDuration )
	{
		m_fAxisSpawnDelay = flDuration;
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameManager::SetAlliedSpawnTimer( float flDuration )
	{
		m_fAlliedSpawnDelay = flDuration;
	}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameManager::GetTicketsRemaining( int iTeam )
{
	if( iTeam == TEAM_AXIS )
	{
		return m_iAxisTickets;
	}
	else if( iTeam == TEAM_ALLIES )
	{
		return m_iAlliedTickets;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameManager::GetAxisTicketsRemaining( void )
{
	return m_iAxisTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameManager::GetAlliedTicketsRemaining( void )
{
	return m_iAlliedTickets;
}

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameManager::GetAxisInitialTickets( void )
{
	return m_iAxisInitialTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameManager::GetAlliedInitialTickets( void )
{
	return m_iAlliedInitialTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::SetTicketsRemaining( int iTeam, int iTickets )
{
	if( iTeam == TEAM_AXIS )
	{
		m_iAxisTickets = iTickets;
	}
	else if( iTeam == TEAM_ALLIES )
	{
		m_iAlliedTickets = iTickets;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::SetAxisTicketsRemaining( int iTickets )
{
	m_iAxisTickets = iTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::SetAlliedTicketsRemaining( int iTickets )
{
	m_iAlliedTickets = iTickets;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::AddTickets( int iTeam, int iTickets )
{
	if( iTeam == TEAM_AXIS )
	{
		m_iAxisTickets += iTickets;
	}
	else if( iTeam == TEAM_ALLIES )
	{
		m_iAlliedTickets += iTickets;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::AddAxisTickets( int iTickets )
{
	m_iAxisTickets += iTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::AddAlliedTickets( int iTickets )
{
	m_iAlliedTickets += iTickets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputSetTickets( inputdata_t &input )
{
	char		token[128];
	const char	*p = STRING( input.value.StringID() );
	int			nTeam = TEAM_UNASSIGNED;
	int			nCount = 0;

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
		nCount = Q_atoi( token );
	}

	if ( nCount != 0 )
	{
		SetTicketsRemaining( nTeam, nCount );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputSetAxisTickets( inputdata_t &input )
{
	int nTicks = input.value.Int();
	SetAxisTicketsRemaining( nTicks );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputSetAlliedTickets( inputdata_t &input )
{
	int nTicks = input.value.Int();
	SetAlliedTicketsRemaining( nTicks );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputAddTickets( inputdata_t &input )
{
	char		token[128];
	const char	*p = STRING( input.value.StringID() );
	int			nTeam = TEAM_UNASSIGNED;
	int			nCount = 0;

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
		nCount = Q_atoi( token );
	}

	if ( nCount != 0 )
	{
		AddTickets( nTeam, nCount );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputAddAxisTickets( inputdata_t &input )
{
	int nTicks = input.value.Int();
	AddAxisTickets( nTicks );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLGameManager::InputAddAlliedTickets( inputdata_t &input )
{
	int nTicks = input.value.Int();
	AddAlliedTickets( nTicks );
}
#endif