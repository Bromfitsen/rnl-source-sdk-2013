//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "prediction.h"
#include "c_rnl_player.h"
#include "rnl_gamemovement.h"

static CRnLMoveData g_MoveData;
CMoveData* g_pMoveData = &g_MoveData;

class CRnLPrediction : public CPrediction
{
DECLARE_CLASS( CRnLPrediction, CPrediction );

public:
	virtual void	SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	FinishMove( C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPrediction::SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, 
	CMoveData *move )
{
	// Added by Toni
	player->AvoidPhysicsProps( ucmd );
	// --

	// Call the default SetupMove code.
	BaseClass::SetupMove( player, ucmd, pHelper, move );

	CRnLPlayer* pPlayer = ToRnLPlayer(player);
	CRnLMoveData* pRnLMove = static_cast<CRnLMoveData*>(move);
	if (pPlayer)
	{
		if (ucmd->mod_data.Count() > 0)
		{
			bf_read readBuffer(ucmd->mod_data.Base(), ucmd->mod_data.Count());
			if (readBuffer.ReadOneBit() > 0)
			{
				QAngle weaponAngle;
				readBuffer.ReadBitAngles(weaponAngle);
				pRnLMove->m_vecWeaponAngles = weaponAngle;
			}
			else
			{
				pRnLMove->m_vecWeaponAngles = ucmd->viewangles;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPrediction::FinishMove( C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	// Call the default FinishMove code.
	BaseClass::FinishMove( player, ucmd, move );
}


// Expose interface to engine
// Expose interface to engine
static CRnLPrediction g_Prediction;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CRnLPrediction, IPrediction, VCLIENT_PREDICTION_INTERFACE_VERSION, g_Prediction );

CPrediction *prediction = &g_Prediction;

