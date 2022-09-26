//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_STICKY24GRENADE_H
#define WEAPON_STICKY24GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rnl_basegrenade.h"


#ifdef CLIENT_DLL
	#define CSticky24Grenade C_Sticky24Grenade
#endif

//-----------------------------------------------------------------------------
// The German Sticky24 Grenade
//-----------------------------------------------------------------------------
class CSticky24Grenade : public CBaseRnLGrenade
{
public:
	DECLARE_CLASS( CSticky24Grenade, CBaseRnLGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CSticky24Grenade() {}
	bool CanHolster( void );

	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_STICKY24GRENADE; }

	virtual bool ShouldDrop( void ) { return m_iGrenadeState == GRENADE_STARTFUSE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll );
	
#endif

	CSticky24Grenade( const CSticky24Grenade & ) {}
};


#endif // WEAPON_STICKY24GRENADE_H
