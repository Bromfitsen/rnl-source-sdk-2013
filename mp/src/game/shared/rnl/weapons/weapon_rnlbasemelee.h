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
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBaseMelee();
	virtual ~CWeaponRnLBaseMelee();

	void PrimaryAttack( void ) override;
	void SecondaryAttack( bool bIsSecondary ) { PrimaryAttack(); } // TODO_KORNEEL What is up with this non-override with weird param?

	virtual void HandleViewAnimation( int iAnim );

	void ImpactEffectMelee( trace_t &traceHit );
	bool ImpactWaterMelee( const Vector &start, const Vector &end );
	void HitMelee( trace_t &traceHit, int iDamage );

	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
	virtual float	GetMeleeRange( void )					{	return	40.0f;	}

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }

	bool CanRearm(CRnLPlayer* pPlayer) override { return false; }

protected:

	CWeaponRnLBaseMelee( const CWeaponRnLBaseMelee & );

	void Fire( float flSpread );

	CNetworkVar( bool, m_bRightHand );
};

#endif //WEAPON_RNLBASEMelee_H