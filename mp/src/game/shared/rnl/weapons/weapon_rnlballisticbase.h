//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
	~CWeaponRnLBallisticBase() override;

	void PrimaryAttack( void ) override;
	void WeaponIdle( void ) override;
	
	bool StartSprinting( void ) override;
	void HandleViewAnimation( int iAnim ) override;

	virtual void AddViewKick(int iSeed);

	bool Reload() override;

	bool Deploy() override;

	// dont let it holster if in IS
	bool CanHolster(void) override;
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL ) override;

	virtual void NextFirePos();
	virtual void PrevFirePos();

	virtual void HandleViewSway( void );
	virtual bool CheckResting( void );

	virtual int GetDefaultPosture() { return 1; }
	virtual void ReturnToDefaultPosture( void );

	void ReloadTransition( int iState );
	virtual bool CheckChamber();
	
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