//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_rnl_playerlocaldata.h"
#include "dt_recv.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar s_cl_autobolt("cl_autobolt", "1", FCVAR_USERINFO|FCVAR_ARCHIVE, "Autobolting of weapons");

BEGIN_RECV_TABLE_NOBASE( C_RnLPlayerLocalData, DT_RnLLocal )
	RecvPropFloat	(RECVINFO( m_flViewRollOffset ) ),
	RecvPropBool	(RECVINFO( m_bIsSprinting ) ),
	RecvPropFloat	(RECVINFO( m_flStamina ) ),
	RecvPropFloat	(RECVINFO( m_flSprintTime ) ),
	RecvPropFloat	(RECVINFO( m_flRecoverTime ) ),
	RecvPropVector	(RECVINFO( m_vecMovementPos )),
	RecvPropBool	(RECVINFO( m_bUseAutobolt ) ),
	RecvPropInt		(RECVINFO( m_iMoraleLevel )),
	RecvPropFloat	(RECVINFO( m_flDamageBasedSpeedModifierLegs ) ),
	RecvPropFloat	(RECVINFO( m_flDamageBasedSpeedModifierArms ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA_NO_BASE( C_RnLPlayerLocalData )
	DEFINE_PRED_FIELD( m_flViewRollOffset,	FIELD_FLOAT,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bIsSprinting,		FIELD_BOOLEAN,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flStamina,			FIELD_FLOAT,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flSprintTime,		FIELD_FLOAT,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flRecoverTime,		FIELD_FLOAT,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecMovementPos,	FIELD_VECTOR,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iMoraleLevel,		FIELD_INTEGER,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDamageBasedSpeedModifierLegs, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flDamageBasedSpeedModifierArms, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	//DEFINE_PRED_FIELD( m_hLadder, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

C_RnLPlayerLocalData::C_RnLPlayerLocalData()
{
	m_flViewRollOffset = 0;
	m_bIsSprinting = false;
	m_flStamina = 100;
	m_flSprintTime = 0;
	m_flRecoverTime = 0;
}

