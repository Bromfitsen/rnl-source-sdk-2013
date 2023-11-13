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

	void SecondaryAttack( bool bIsSecondary );
	virtual bool StartSprinting( void );

	bool HandleBayonetTransition( void );
	void BayonetTransition( int iState );
	bool IsBayonetDeployed() { return m_bDeployed; }

	virtual void HandleViewAnimation( int iAnim );

	void ImpactEffect( trace_t &traceHit );
	bool ImpactWater( const Vector &start, const Vector &end );
	void Hit( trace_t &traceHit, Activity nHitActivity );

	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
	virtual float	GetRange( void )					{	return	96.0f;	}

	int GetBayonetState( void ) { return m_iBayoAnimState; }

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }

protected:

	CWeaponRnLBaseRifle( const CWeaponRnLBaseRifle & );

	void Fire( float flSpread );

	CNetworkVar( bool, m_bDeployed );
	CNetworkVar( int, m_iBayoAnimState );
};

#endif //WEAPON_RNLBASERIFLE_H