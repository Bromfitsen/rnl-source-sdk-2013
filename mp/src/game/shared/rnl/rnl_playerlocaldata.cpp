//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "datamap.h"
#include "predictable_entity.h"
#include "rnl_playerlocaldata.h"
#include "rnl_dt_shared.h"
#include "mathlib/mathlib.h"

#ifdef CLIENT_DLL
#include "c_rnl_player.h"
#else
#include "rnl_player.h"
#include "entitylist.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
static ConVar s_cl_autobolt("cl_autobolt", "1", FCVAR_USERINFO | FCVAR_ARCHIVE, "Autobolting of weapons");
#endif

BEGIN_NETWORK_TABLE_NOBASE(CRnLPlayerLocalData, DT_RnLLocal)
	PropFloat(PROPINFO(m_flViewRollOffset)),
	PropBool(PROPINFO(m_bIsSprinting)),
	PropFloat(PROPINFO(m_flStamina)),
	PropFloat(PROPINFO(m_flSprintTime)),
	PropFloat(PROPINFO(m_flRecoverTime)),
	PropFloat(PROPINFO(m_flDamageBasedSpeedModifierLegs)),
	PropFloat(PROPINFO(m_flDamageBasedSpeedModifierArms)),
	PropBool(PROPINFO(m_bUseAutobolt)),
	PropVector(PROPINFO(m_vecMovementPos), -1, SPROP_COORD),
	PropFloat(PROPINFO(m_flMovementPostureEntranceTime)),
	PropVector(PROPINFO(m_vecClimbEndPos), -1, SPROP_COORD),
	PropInt(PROPINFO(m_iMoraleLevel)),
	PropVector(PROPINFO(m_vecDeployedOrigin)),

	//RnL : Andrew : IMportant info for stamina
	PropInt(PROPINFO(m_iRunSpeedModifier)),
	PropInt(PROPINFO(m_iSprintSpeedModifier)),

	PropEHandle(PROPINFO(m_hCurrentSpawnArea)),

	PropFloat(PROPINFO(m_flMovementPostureOffset)),
	PropFloat(PROPINFO(m_flMovementPostureVelocity)),
	PropFloat(PROPINFO(m_flMovementPostureTarget)),
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
	BEGIN_PREDICTION_DATA_NO_BASE(CRnLPlayerLocalData)
		DEFINE_PRED_FIELD(m_flViewRollOffset, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_bIsSprinting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_flStamina, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD_TOL(m_flSprintTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
		DEFINE_PRED_FIELD_TOL(m_flRecoverTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
		DEFINE_PRED_FIELD(m_vecMovementPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_iMoraleLevel, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_flDamageBasedSpeedModifierLegs, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_flDamageBasedSpeedModifierArms, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
		//DEFINE_PRED_FIELD( m_hLadder, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
		DEFINE_PRED_FIELD_TOL(m_flMovementPostureEntranceTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
		DEFINE_PRED_FIELD(m_vecClimbEndPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
		/*DEFINE_PRED_FIELD( m_angAimingOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),*/
		DEFINE_PRED_FIELD(m_vecDeployedOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
		//DEFINE_PRED_FIELD( m_flStaminaThink, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
		//DEFINE_PRED_FIELD( m_flRatio, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
		DEFINE_PRED_FIELD(m_iRunSpeedModifier, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
		DEFINE_PRED_FIELD(m_iSprintSpeedModifier, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),

		DEFINE_PRED_FIELD_TOL(m_flMovementPostureOffset, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
		DEFINE_PRED_FIELD_TOL(m_flMovementPostureVelocity, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
		DEFINE_PRED_FIELD_TOL(m_flMovementPostureTarget, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	END_PREDICTION_DATA()
#else
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
#endif

CRnLPlayerLocalData::CRnLPlayerLocalData()
{
	m_flViewRollOffset = 0;
	m_bIsSprinting = false;
	m_flStamina = 100;
	m_flSprintTime = 0;
	m_flRecoverTime = 0;

	m_flMovementPostureEntranceTime = 0;
	m_flMovementPostureOffset = 0;
	m_flMovementPostureVelocity = 0;
	m_flMovementPostureTarget = 0;
}
