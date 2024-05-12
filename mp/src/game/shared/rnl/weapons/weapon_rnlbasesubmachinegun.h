//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBASESUBMACHINEGUN_H
#define WEAPON_RNLBASESUBMACHINEGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "in_buttons.h"
#include "weapon_rnlballisticbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBaseSubMachineGun C_WeaponRnLBaseSubMachineGun
#endif

class CWeaponRnLBaseSubMachineGun : public CWeaponRnLBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRnLBaseSubMachineGun, CWeaponRnLBallisticBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBaseSubMachineGun();
	~CWeaponRnLBaseSubMachineGun() override;

	void PrimaryAttack( void ) override;

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_NONE; }

private:
	CWeaponRnLBaseSubMachineGun( const CWeaponRnLBaseSubMachineGun & );
};

#endif //WEAPON_RNLBASESUBMACHINEGUN_H