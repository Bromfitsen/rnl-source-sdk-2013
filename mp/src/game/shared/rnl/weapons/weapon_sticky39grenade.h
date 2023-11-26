//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_STICKY39GRENADE_H
#define WEAPON_STICKY39GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rnl_basegrenade.h"


#ifdef CLIENT_DLL
	#define CSticky39Grenade C_Sticky39Grenade
#endif

//-----------------------------------------------------------------------------
// The American Sticky39 Grenade
//-----------------------------------------------------------------------------
class CSticky39Grenade : public CBaseRnLGrenade
{
public:
	DECLARE_CLASS( CSticky39Grenade, CBaseRnLGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CSticky39Grenade() {}
	bool CanHolster( void ) override;

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_STICKY39GRENADE; }
	virtual bool IsSmokeGrenade( void ) { return true; }

	virtual bool ShouldDrop( void ) { return m_iGrenadeState == GRENADE_STARTFUSE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll );
	
#endif

	CSticky39Grenade( const CSticky39Grenade & ) {}
};


#endif // WEAPON_STICKY39GRENADE_H
