//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team spawnpoint entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_OBJECTIVE_ORDER_FILTER_H
#define RNL_OBJECTIVE_ORDER_FILTER_H
#pragma once

#include "cbase.h"

#ifdef CLIENT_DLL
	#define CRnLObjectiveOrderFilter C_RnLObjectiveOrderFilter
	class C_RnLObjectiveBase;
#else
	class CRnLObjectiveBase;
#endif

#define MAX_OBJECTIVE_LINKS 16


//-----------------------------------------------------------------------------
// Purpose: points at which the player can spawn, restricted by team
//-----------------------------------------------------------------------------
class CRnLObjectiveOrderFilter : public CBaseEntity
{
public:
	DECLARE_CLASS( CRnLObjectiveOrderFilter, CBaseEntity );

	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual int UpdateTransmitState() { return SetTransmitState( FL_EDICT_ALWAYS ); }
#endif

	virtual void Activate();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual bool AreRequirementsMet( void );

public:
#ifndef CLIENT_DLL
	CUtlVector<CUtlString> m_vNodeNames;
#endif
	CNetworkVar( int, m_iGoalState );
#ifdef CLIENT_DLL
	CUtlVector< CHandle<C_RnLObjectiveBase> >	m_vNodes;
#else
	CUtlVector< CHandle<CRnLObjectiveBase> >	m_vNodes;
#endif
};

#endif // RNL_OBJECTIVE_ORDER_FILTER_H
