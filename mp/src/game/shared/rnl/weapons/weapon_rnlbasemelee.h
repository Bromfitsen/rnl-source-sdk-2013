//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBASEMelee_H
#define WEAPON_RNLBASEMelee_H
#ifdef _WIN32
#pragma once
#endif

#include "in_buttons.h"
#include "weapon_rnlballisticbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBaseMelee C_WeaponRnLBaseMelee
#endif

class CWeaponRnLBaseMelee : public CWeaponRnLBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRnLBaseMelee, CWeaponRnLBallisticBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef SERVER_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBaseMelee();
	virtual ~CWeaponRnLBaseMelee();

	void PrimaryAttack( void );
	void SecondaryAttack( bool bIsSecondary ) { PrimaryAttack(); }

	virtual void HandleViewAnimation( int iAnim );

	void ImpactEffect( trace_t &traceHit );
	bool ImpactWater( const Vector &start, const Vector &end );
	void Hit( trace_t &traceHit, int iDamage );

	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
	virtual float	GetRange( void )					{	return	40.0f;	}

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }

protected:

	CWeaponRnLBaseMelee( const CWeaponRnLBaseMelee & );

	void Fire( float flSpread );

	CNetworkVar( bool, m_bRightHand );
};

#endif //WEAPON_RNLBASEMelee_H