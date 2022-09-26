//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_FX_SHARED_H
#define RNL_FX_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets( 
	int	iPlayer,
	const Vector &vOrigin,
	const QAngle &vAngles,
	int	iWeaponID,
	int	iMode,
	int iSeed,
	float flSpread
	);

void FX_ThrowNadeSpoon( int iSpoonType, Vector vOrigin, QAngle vAngles );

#ifdef CLIENT_DLL
#include "weapon_rnl_base.h"

void FX_WeaponSound(
		int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		CRnLWeaponInfo *pWeaponInfo );
#endif

#endif // RNL_FX_SHARED_H
