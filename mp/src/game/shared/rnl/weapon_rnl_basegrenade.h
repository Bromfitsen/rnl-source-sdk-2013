//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_BASERNLGRENADE_H
#define WEAPON_BASERNLGRENADE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_rnl_base.h"


#ifdef CLIENT_DLL
	
	#define CBaseRnLGrenade C_BaseRnLGrenade

#else

	extern ConVar mp_melontastic;
	#define MELON_MODEL "models/props_junk/watermelon01.mdl"

#endif

//Linux builds hate enums on network vars
enum eGrenadeStates
{
	GRENADE_DRAWN = 0,
	GRENADE_PREP,
	GRENADE_STARTFUSE,
	GRENADE_THROWING,
	GRENADE_ROLLING,
	GRENADE_THROWN,
};

class CBaseRnLGrenade : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CBaseRnLGrenade, CWeaponRnLBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CBaseRnLGrenade();

	virtual void	Precache();

	bool			Deploy();
	bool			Holster( CBaseCombatWeapon *pSwitchingTo );

	void			PrimaryAttack();

	bool			Reload();

	virtual void	ItemPostFrame();
	
	void			DecrementAmmo( CBaseCombatCharacter *pOwner );
	virtual void	ThrowGrenade();
	virtual void	RollGrenade();
	virtual void	DropGrenade();

	virtual void	WeaponIdle();
	virtual void	HandleViewAnimation( int iAnim );

	virtual void	Equip( CBaseCombatCharacter *pOwner );

	bool	CanDeploy( void ) override;
	bool	CanHolster( void ) override;
	bool	CanRearm( CRnLPlayer* pPlayer ) override { return false; }

	bool IsGrenadePrimed( ) { return m_iGrenadeState == GRENADE_STARTFUSE; };
	virtual bool IsSmokeGrenade( void ) { return false; }

	// Michael Lebson
	virtual bool	IsGrenade() const { return true; }

	virtual bool CanClimb( void );

	virtual Activity GetDrawActivity( void )
	{
		return ACT_VM_SHOULDERDRAW;
	}
	virtual int GetGrenadeState( void ) { return m_iGrenadeState; }

	//Default Any State but pin
	virtual bool ShouldDrop( void ) { return m_iGrenadeState != GRENADE_DRAWN; }

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual bool AllowsAutoSwitchFrom( void ) const;

	int		CapabilitiesGet();
	
	// Each derived grenade class implements this.
	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float m_fDelay, bool roll );
#endif

protected:
	CNetworkVar( bool, m_bRedraw );	// Draw the weapon again after throwing a grenade
	CNetworkVar( short, m_iGrenadeState );	// track current state of grenade, drawn, pinpulled and spoon released
	CNetworkVar( float, m_flFuseEnd ); // time the fuse will burn out and grenade explodes
	CNetworkVar( float, m_fThrowTime );
private:
	CBaseRnLGrenade( const CBaseRnLGrenade & ) {}
};

#endif // WEAPON_BASERNLGRENADE_H