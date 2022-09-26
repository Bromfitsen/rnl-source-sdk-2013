//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_M18GRENADE_H
#define WEAPON_M18GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rnl_basegrenade.h"


#ifdef CLIENT_DLL
	
	#define CM18Grenade C_M18Grenade
#endif

//-----------------------------------------------------------------------------
// The American M18 Smoke Grenade
//-----------------------------------------------------------------------------
class CM18Grenade : public CBaseRnLGrenade
{
public:
	DECLARE_CLASS( CM18Grenade, CBaseRnLGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CM18Grenade() {}

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_M18GRENADE; }
	virtual bool IsSmokeGrenade( void ) { return true; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll );
	
#endif

	CM18Grenade( const CM18Grenade & ) {}
};


#endif // WEAPON_M18GRENADE_H
