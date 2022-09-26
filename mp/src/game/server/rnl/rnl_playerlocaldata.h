//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_PLAYERLOCALDATA_H
#define RNL_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"


//-----------------------------------------------------------------------------
// Purpose: Player specific data for RnL ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CRnLPlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	DECLARE_CLASS_NOBASE( CRnLPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CRnLPlayerLocalData();

	CNetworkVar( float,	m_flViewRollOffset );
	CNetworkVar( bool,	m_bIsSprinting );
	CNetworkVar( float,	m_flStamina );
	CNetworkVar( float,	m_flSprintTime );
	CNetworkVar( float,	m_flRecoverTime );
	CNetworkVector(	m_vecMovementPos );
	CNetworkVar( int, m_iMoraleLevel );

	//Player toggled options
	CNetworkVar( bool, m_bUseAutobolt );

	CNetworkVar( float, m_flDamageBasedSpeedModifierLegs );
	CNetworkVar( float, m_flDamageBasedSpeedModifierArms );

	//RnL : TODO : Add this back in from uber ladder climby climbing?
	// Ladder related data
	//CNetworkVar( EHANDLE, m_hLadder );
	//LadderMove_t			m_LadderMove;
};

EXTERN_SEND_TABLE(DT_RnLLocal);


#endif // RNL_PLAYERLOCALDATA_H
