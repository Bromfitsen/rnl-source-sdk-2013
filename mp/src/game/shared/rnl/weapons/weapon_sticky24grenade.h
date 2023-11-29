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
	~CSticky24Grenade() override = default;

	bool CanHolster( void ) override;

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_STICKY24GRENADE; }

	bool ShouldDrop( void ) override { return m_iGrenadeState == GRENADE_STARTFUSE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll ) override;
	
#endif

	CSticky24Grenade( const CSticky24Grenade & ) {}
};


#endif // WEAPON_STICKY24GRENADE_H
