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
#include "rnl_dt_shared.h"


#ifdef CLIENT_DLL

#define CRnLPlayerLocalData C_RnLPlayerLocalData

#endif

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
	DECLARE_PREDICTABLE();

	CRnLPlayerLocalData();

	CNetworkVar( float,	m_flViewRollOffset );
	CNetworkVar( bool,	m_bIsSprinting );
	CNetworkVar( float,	m_flStamina );
	CNetworkVar( float,	m_flSprintTime );
	CNetworkVar( float,	m_flRecoverTime );
	CNetworkVar(float, m_flDamageBasedSpeedModifierLegs);
	CNetworkVar(float, m_flDamageBasedSpeedModifierArms);

	//Player toggled options
	CNetworkVar( bool, m_bUseAutobolt );

	CNetworkVector(m_vecMovementPos);
	CNetworkVar(float, m_flMovementPostureEntranceTime);

	//where the climber will end
	CNetworkVector(m_vecClimbEndPos);

	//RnL : Andrew : Moved from RnL_Player
	//These have been moved since only local players need the info
	CNetworkVar(int, m_iMoraleLevel);
	CNetworkVector(m_vecDeployedOrigin);

	// This is the combined speed modifier of the players weapons and equipment
	CNetworkVar(int, m_iRunSpeedModifier);
	// This is the sprint speed modifier
	CNetworkVar(int, m_iSprintSpeedModifier);

	// for the ncomap
	CNetworkHandle(CBaseEntity, m_hCurrentSpawnArea);

	CNetworkVar(float, m_flMovementPostureOffset);
	CNetworkVar(float, m_flMovementPostureVelocity);
	CNetworkVar(float, m_flMovementPostureTarget);
};


EXTERN_PROP_TABLE(DT_RnLLocal);

#endif // RNL_PLAYERLOCALDATA_H
