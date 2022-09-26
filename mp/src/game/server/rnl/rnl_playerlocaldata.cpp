//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "rnl_playerlocaldata.h"
#include "rnl_player.h"
#include "mathlib/mathlib.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_SEND_TABLE_NOBASE( CRnLPlayerLocalData, DT_RnLLocal )
	SendPropFloat	( SENDINFO( m_flViewRollOffset ) ),
	SendPropBool	( SENDINFO( m_bIsSprinting ) ),
	SendPropFloat	( SENDINFO( m_flStamina )),
	SendPropFloat	( SENDINFO( m_flSprintTime ) ),
	SendPropFloat	( SENDINFO( m_flRecoverTime ) ),
	SendPropVector	( SENDINFO( m_vecMovementPos ),      -1,  SPROP_COORD),
	SendPropBool	( SENDINFO( m_bUseAutobolt ) ),
	SendPropInt		( SENDINFO( m_iMoraleLevel ) ),
	SendPropFloat	( SENDINFO( m_flDamageBasedSpeedModifierLegs ) ),
	SendPropFloat	( SENDINFO( m_flDamageBasedSpeedModifierArms ) ),
END_SEND_TABLE()

BEGIN_SIMPLE_DATADESC( CRnLPlayerLocalData )
	DEFINE_FIELD	( m_flViewRollOffset,	FIELD_FLOAT ),
	DEFINE_FIELD	( m_bIsSprinting,		FIELD_BOOLEAN ),
	DEFINE_FIELD	( m_flStamina,			FIELD_FLOAT ),
	DEFINE_FIELD	( m_flSprintTime,		FIELD_TIME ),
	DEFINE_FIELD	( m_flRecoverTime,		FIELD_TIME ),
	DEFINE_FIELD	( m_vecMovementPos,		FIELD_VECTOR ),
	// Ladder related stuff
	//DEFINE_FIELD( m_hLadder, FIELD_EHANDLE ),
	//DEFINE_EMBEDDED( m_LadderMove ),
END_DATADESC()

CRnLPlayerLocalData::CRnLPlayerLocalData()
{
	m_flViewRollOffset = 0;
	m_bIsSprinting = false;
	m_flStamina = 100;
	m_flSprintTime = 0;
	m_flRecoverTime = 0;
}
