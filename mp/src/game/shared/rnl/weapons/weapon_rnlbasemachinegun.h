#ifndef WEAPON_RNLBASEMACHINEGUN_H
#define WEAPON_RNLBASEMACHINEGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_rnlballisticbase.h"


#if defined( CLIENT_DLL )
#define CWeaponRnLBaseMachineGun C_WeaponRnLBaseMachineGun
#endif


class CWeaponRnLBaseMachineGun : public CWeaponRnLBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRnLBaseMachineGun, CWeaponRnLBallisticBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponRnLBaseMachineGun();
	virtual ~CWeaponRnLBaseMachineGun();

	virtual bool			StartSprinting( void ) { return false; }
	virtual bool ShouldUseWeaponActivities( void ) { return true; }

	bool IsDeployed() { return m_bDeployed; }
	void ToggleDeploy();
	bool CheckUnDeploy( void );

	bool Deploy();

	void HandleShotDelay( void );
	float GetRecoil( void );

	void HandleViewAnimation( int iAnim );

	float GetLastShotTime( void ) { return m_flLastShotTime; }
	//HACK HACK
	void SetLastShotTime( float time, float penalty ) { m_flLastShotTime = time; m_flAccuracyPenalty += penalty; }

	//A NO NO if we are deployededed
	bool CanHolster( void ) override;

	virtual bool CanClimb( void ) { return !(m_bDeployed); }

	virtual float GetSpread( void );

	virtual bool	ShouldUseFreeAim() { return m_bDeployed; }
	virtual void	GetFreeAimBounds( Vector2D& maxExtents, Vector2D& deadZone, int& lockToExtents );

	virtual float		GetStandOffset() { return 10.0f; }
	virtual const char*	GetTracerType( void ) { return "Tracer"; }

	//Michael Lebson
	virtual bool			IsMachineGun() const { return true; }
	virtual bool			ShouldMakeTracer();

#ifndef CLIENT_DLL
	//virtual void HandleSuppression( void );
#endif

private:

	CWeaponRnLBaseMachineGun( const CWeaponRnLBaseMachineGun & );

	void Fire( float flSpread );

	CNetworkVar( bool, m_bDeployed );
	CNetworkVar( int, m_iBarrelAnimState );
	CNetworkVar( int, m_iTracerCount );
	CNetworkVar( float, m_flLastShotTime );
	CNetworkVar( float, m_flAccuracyPenalty );

#ifndef CLIENT_DLL
	CBaseEntity* m_pResetingEntity;
	char m_oldDamage;
	//float m_flNextSuppressionTime;
#endif
};

#endif //WEAPON_RNLBASEMACHINEGUN_H