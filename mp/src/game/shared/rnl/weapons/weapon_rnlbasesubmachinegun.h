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
#ifdef SERVER_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBaseSubMachineGun();
	virtual ~CWeaponRnLBaseSubMachineGun();
	void PrimaryAttack( void );

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }

private:
	CWeaponRnLBaseSubMachineGun( const CWeaponRnLBaseSubMachineGun & );

	void Fire( float flSpread );
};

#endif //WEAPON_RNLBASESUBMACHINEGUN_H