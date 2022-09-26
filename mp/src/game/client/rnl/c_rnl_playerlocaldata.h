//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_PLAYERLOCALDATA_H
#define C_RNL_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif


#include "dt_recv.h"

EXTERN_RECV_TABLE( DT_RnLLocal );


class C_RnLPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_RnLPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_RnLPlayerLocalData();

	float	m_flViewRollOffset;
	bool	m_bIsSprinting;
	float	m_flStamina;
	float	m_flSprintTime;
	float	m_flRecoverTime;
	int		m_iMoraleLevel;
	float	m_flDamageBasedSpeedModifierLegs;
	float	m_flDamageBasedSpeedModifierArms;

	//Player toggled options
	bool	m_bUseAutobolt;

	CNetworkVector(	m_vecMovementPos );

	// Ladder related data
	//EHANDLE			m_hLadder;
	//LadderMove_t	m_LadderMove;
};


#endif // C_RNL_PLAYERLOCALDATA_H

