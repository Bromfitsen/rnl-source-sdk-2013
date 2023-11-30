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
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponRnLBallisticBase();
	~CWeaponRnLBallisticBase();

	void PrimaryAttack( void ) OVERRIDE;
	void WeaponIdle( void ) OVERRIDE;
	
	bool StartSprinting( void ) OVERRIDE;
	void HandleViewAnimation( int iAnim ) OVERRIDE;

	virtual void AddViewKick(int iSeed);

	bool Reload() OVERRIDE;

	bool Deploy() OVERRIDE;

	// dont let it holster if in IS
	bool CanHolster(void) OVERRIDE;
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL ) OVERRIDE;

	void NextFirePos() OVERRIDE;
	void PrevFirePos() OVERRIDE;

	void HandleViewSway( void ) OVERRIDE;
	bool CheckResting( void ) OVERRIDE;

	virtual int GetDefaultPosture() { return 1; }
	void ReturnToDefaultPosture( void ) OVERRIDE;

	void ReloadTransition( int iState );
	virtual bool CheckChamber();
	
	void	GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents ) OVERRIDE;

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

	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_NONE; }
	void GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir );
	void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles ) OVERRIDE;
	float	CalcViewmodelBob( void ) OVERRIDE;

protected:
	void HandleFoVTransitions(void);
	bool HandleReloadTransitions(void);
	bool HandlePostureTransitions(void);

	bool OnIronsightsUnheld( void );
	bool OnIronsightsHeld( void );

	CNetworkVar( float, m_flIronsightsPressedTime );

private:

	CWeaponRnLBallisticBase( const CWeaponRnLBallisticBase & );

	CNetworkVar( int, m_iReloadState );

#ifndef CLIENT_DLL
	int m_iNextFov;
	int m_iCurrentFov;
#endif

};

#endif //WEAPON_RNLBALLISTICBASE_H