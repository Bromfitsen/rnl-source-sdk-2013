//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBASERIFLE_H
#define WEAPON_RNLBASERIFLE_H
#ifdef _WIN32
#pragma once
#endif

#include "in_buttons.h"
#include "weapon_rnlballisticbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBaseRifle C_WeaponRnLBaseRifle
#endif

class CWeaponRnLBaseRifle : public CWeaponRnLBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRnLBaseRifle, CWeaponRnLBallisticBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBaseRifle();
	virtual ~CWeaponRnLBaseRifle();

	void SecondaryAttack( void ) OVERRIDE; // TODO_KORNEEL What is up with this near-override with weird param?
	bool StartSprinting( void ) OVERRIDE;

	bool HandleBayonetTransition( void );
	void BayonetTransition( int iState );
	bool IsBayonetDeployed() const { return m_bDeployed; }
	int GetBayonetState(void) const { return m_iBayoAnimState; }

	void HandleViewAnimation( int iAnim ) OVERRIDE;

	void ImpactEffectMelee( trace_t &traceHit );
	bool ImpactWaterMelee( const Vector &start, const Vector &end );
	void HitMelee( trace_t &traceHit, Activity nHitActivity );

	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
	virtual float GetMeleeRange( void ) const {	return	96.0f;	}

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_NONE; }

protected:

	CWeaponRnLBaseRifle( const CWeaponRnLBaseRifle & );

	CNetworkVar( bool, m_bDeployed );
	CNetworkVar( int, m_iBayoAnimState );
};

#endif //WEAPON_RNLBASERIFLE_H