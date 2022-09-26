//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_MK2GRENADE_H
#define WEAPON_MK2GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rnl_basegrenade.h"


#ifdef CLIENT_DLL
	
	#define CMK2Grenade C_MK2Grenade

#endif

//-----------------------------------------------------------------------------
// The American MK2 Frag Grenade
//-----------------------------------------------------------------------------
class CMK2Grenade : public CBaseRnLGrenade
{
public:
	DECLARE_CLASS( CMK2Grenade, CBaseRnLGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CMK2Grenade() {}

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_MK2GRENADE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float fDelay, bool roll );
	
#endif

	CMK2Grenade( const CMK2Grenade & ) {}
};


#endif // WEAPON_MK2GRENADE_H
