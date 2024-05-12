//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBASEPISTOL_H
#define WEAPON_RNLBASEPISTOL_H
#ifdef _WIN32
#pragma once
#endif

#include "in_buttons.h"
#include "weapon_rnlballisticbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBasePistol C_WeaponRnLBasePistol
#endif

class CWeaponRnLBasePistol : public CWeaponRnLBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRnLBasePistol, CWeaponRnLBallisticBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponRnLBasePistol();
	~CWeaponRnLBasePistol() override;

	bool ShouldUseWeaponActivities( void ) override { return true; }

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_NONE; }

private:

	CWeaponRnLBasePistol( const CWeaponRnLBasePistol & );
};

#endif //WEAPON_RNLBASERIFLE_H