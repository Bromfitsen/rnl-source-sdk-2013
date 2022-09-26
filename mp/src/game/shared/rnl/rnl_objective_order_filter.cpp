//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "rnl_objective_base.h"
#include "rnl_objective_order_filter.h"

#ifndef CLIENT_DLL
#include "dt_utlvector_send.h"
#else
#include "dt_utlvector_recv.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(rnl_objective_order_filter,CRnLObjectiveOrderFilter);

IMPLEMENT_NETWORKCLASS_ALIASED(RnLObjectiveOrderFilter, DT_RnLObjectiveOrderFilter)
BEGIN_NETWORK_TABLE( CRnLObjectiveOrderFilter, DT_RnLObjectiveOrderFilter )
#ifndef CLIENT_DLL
	SendPropInt( SENDINFO( m_iGoalState ) ),

	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_vNodes ),
		MAX_OBJECTIVE_LINKS, // max elements
		SendPropEHandle( NULL, 0 ) ),
#else
	RecvPropInt( RECVINFO( m_iGoalState ) ),
	
	RecvPropUtlVector( 
		RECVINFO_UTLVECTOR( m_vNodes ), 
		MAX_OBJECTIVE_LINKS,
		RecvPropEHandle(NULL, 0, 0)),

#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLObjectiveOrderFilter )
		DEFINE_KEYFIELD( m_iGoalState, FIELD_INTEGER, "goal" ),
	END_DATADESC()
#endif

bool CRnLObjectiveOrderFilter::AreRequirementsMet( void )
{
	for( int i = 0; i < m_vNodes.Count(); i++ )
	{
		if( m_vNodes[i].Get()->GetObjectiveState() != m_iGoalState )
			return false;
	}
	return true;
}

void CRnLObjectiveOrderFilter::Activate( void )
{
#ifndef CLIENT_DLL
	for( int i = 0; i < m_vNodeNames.Count(); i++ )
	{
		CRnLObjectiveBase* pObj = dynamic_cast<CRnLObjectiveBase*>(gEntList.FindEntityByName( NULL, m_vNodeNames[i] ));
		if( pObj != NULL )
			m_vNodes.AddToTail( pObj );
	}
#endif
	BaseClass::Activate();
}

bool CRnLObjectiveOrderFilter::KeyValue( const char* szKeyName, const char* szValue )
{
	if ( !BaseClass::KeyValue( szKeyName, szValue ) )
	{
		if( Q_strnicmp( szKeyName, "RequiredNode", 12 ) == 0 )
		{
#ifndef CLIENT_DLL
			m_vNodeNames.AddToTail( szValue );
#endif
			return true;
		}
	}

	return false;
}