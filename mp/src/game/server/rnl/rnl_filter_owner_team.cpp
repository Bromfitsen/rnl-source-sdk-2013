//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "filters.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ###################################################################
//	> FilterOwnerTeam
// ###################################################################
class CRnLFilterOwnerTeam : public CBaseFilter
{
	DECLARE_CLASS( CRnLFilterOwnerTeam, CBaseFilter );
	DECLARE_DATADESC();

public:
	int	m_iTeamValue;

	void Spawn() OVERRIDE
	{
		BaseClass::Spawn();
		m_iTeamValue = TEAM_UNASSIGNED;
		if ( m_target != NULL_STRING )
		{
			SetThink( &CRnLFilterOwnerTeam::TeamCheckThink );
			SetNextThink( gpGlobals->curtime + 0.1 );
		}
	}

	void TeamCheckThink()
	{
		if ( m_target == NULL_STRING )
			return;

		CBaseEntity *pEntity = GetNextTarget();
		if ( pEntity )
		{
			if( pEntity->GetTeamNumber() != m_iTeamValue )
			{
				m_iTeamValue = pEntity->GetTeamNumber();
				m_OnTeamChange.FireOutput( this, pEntity );
			}
		}

		SetNextThink( gpGlobals->curtime + 0.1 );
	}

	bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity ) OVERRIDE
	{
	 	return ( pEntity->GetTeamNumber() == m_iTeamValue );
	}

	COutputEvent m_OnTeamChange;
};

LINK_ENTITY_TO_CLASS( rnl_filter_owner_team, CRnLFilterOwnerTeam );

BEGIN_DATADESC( CRnLFilterOwnerTeam )
	DEFINE_THINKFUNC( TeamCheckThink ),

	DEFINE_OUTPUT( m_OnTeamChange,			"OnTeamChange" ),
END_DATADESC()