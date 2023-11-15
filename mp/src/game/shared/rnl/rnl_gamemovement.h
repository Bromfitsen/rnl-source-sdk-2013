//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_GAMEMOVEMENT_H
#define RNL_GAMEMOVEMENT_H
#pragma once

#include "igamemovement.h"

class CRnLMoveData : public CMoveData
{
public:
	QAngle			m_vecWeaponAngles;		// direction the player is aiming.
};

#endif //RNL_GAMEMOVEMENT_H