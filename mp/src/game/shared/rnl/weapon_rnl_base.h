//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBASE_H
#define WEAPON_RNLBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "rnl_playeranimstate.h"
#include "rnl_weapon_parse.h"
#include "rnl_shareddefs.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBase C_WeaponRnLBase
#endif

class CRnLPlayer;

class CWeaponRnLBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponRnLBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

							CWeaponRnLBase();
	virtual 				~CWeaponRnLBase();

	virtual void			Precache( void );

#ifdef GAME_DLL
	DECLARE_DATADESC();
#else
	virtual bool ShouldPredict();
#endif

	// TF Sprinting functions
	virtual bool StartSprinting( void ) { return true; };
	virtual bool StopSprinting( void ) { return true; };

	virtual void NextFirePos( void ) {}
	virtual void PrevFirePos( void ) {}

	virtual void HandleViewSway( void ) {}
	virtual bool CheckResting( void ) { return false; }

	virtual bool CanPickup( CRnLPlayer* pPlayer );
	virtual bool CanHolster( void );

	CNetworkVar( int, m_iWeaponAnimationState );
	virtual void	SetAnimationState( int iState ) { m_iWeaponAnimationState = iState; }
	virtual int		GetAnimationState( void )		{ return m_iWeaponAnimationState; }

	virtual bool ShouldUseWeaponActivities( void ) { return false; }

	bool ShouldHandleWeaponLimitations( void );
	virtual void HandleViewAnimation( int iAnim ) { };
	virtual void ReturnToDefaultPosture() {}

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const;
	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
	
	// Get RnL weapon specific weapon data.
	CRnLWeaponInfo const	&GetRnLWpnData() const;

	// Get a pointer to the player that owns this weapon
	CRnLPlayer* GetPlayerOwner() const;

	// overwrite drop here
	void Drop( const Vector &vecVelocity );

	virtual bool	ShouldUseFreeAim();
	virtual void	GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents ) {}
	virtual bool	CanClimb( void ) { return true; }

	// override to play custom empty sounds
	virtual bool	PlayEmptySound();
	virtual bool	PlayLastBulletSound();
	void	SetNewFov( int iNewFov );

	// Michael Lebson
	virtual bool			IsGrenade() const { return false; }
	virtual bool			IsMachineGun() const { return false; }
	virtual bool			IsBayonetDeployed() const { return false; }

#ifdef GAME_DLL
	virtual void	SendReloadEvents( bool bMidReload = false );
#else
	virtual Vector	GetIronsightsOffset(){ return Vector( 0, 0, 0 ); }
	virtual Vector	GetShoulderOffset(){ return Vector( 0, 0, 0 ); }
#endif

private:
	CWeaponRnLBase( const CWeaponRnLBase & );
};


#endif // WEAPON_RNLBASE_H
