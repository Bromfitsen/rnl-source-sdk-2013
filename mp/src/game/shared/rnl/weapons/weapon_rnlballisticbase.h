//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNLBALLISTICBASE_H
#define WEAPON_RNLBALLISTICBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "in_buttons.h"
#include "weapon_rnl_base.h"

#if defined( CLIENT_DLL )
	#define CWeaponRnLBallisticBase C_WeaponRnLBallisticBase
#endif

class CWeaponRnLBallisticBase : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CWeaponRnLBallisticBase, CWeaponRnLBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef SERVER_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBallisticBase();

	void PrimaryAttack( void );
	void WeaponIdle( void );
	void AddViewKick( int iSeed );
	virtual bool StartSprinting( void );
	virtual void HandleViewAnimation( int iAnim );
	virtual void HandleFoVTransitions( void );
	virtual bool HandleReloadTransitions( void );
	virtual bool HandlePostureTransitions( void );
	bool Reload();

	bool Deploy();
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	virtual void NextFirePos();
	virtual void PrevFirePos();

	virtual void HandleViewSway( void );
	virtual bool CheckResting( void );

	virtual int GetDefaultPosture() { return 1; }
	virtual void ReturnToDefaultPosture( void );

	void ReloadTransition( int iState );
	virtual bool CheckChamber();

	// dont let it holster if in IS
	bool CanHolster( void );
	
	virtual void	GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents );

	virtual float	GetSpread( void );
	virtual float	GetRecoil( void );

	int	GetReloadState( void ) { return m_iReloadState; }
	void SetReloadState( int iState ) { m_iReloadState = iState; }

#ifndef CLIENT_DLL
	int GetNextFoV( void ){ return m_iNextFov; }
	void SetNextFoV( int iFoV ) { m_iNextFov = iFoV; }

	int GetCurrentFoV( void ) { return m_iCurrentFov; }
	void SetCurrentFoV( int iFoV ) { m_iCurrentFov = iFoV; }

	virtual void UpdatePostureFoV();
#endif

	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
	void GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir );
	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float	CalcViewmodelBob( void );

protected:
	virtual bool OnIronsightsUnheld( void );
	virtual bool OnIronsightsHeld( void );

	CNetworkVar( float, m_flIronsightsPressedTime );

private:

	CWeaponRnLBallisticBase( const CWeaponRnLBallisticBase & );

	void Fire( float flSpread );

	CNetworkVar( int, m_iReloadState );

#ifndef CLIENT_DLL
	int m_iNextFov;
	int m_iCurrentFov;
#endif

};

#endif //WEAPON_RNLBALLISTICBASE_H