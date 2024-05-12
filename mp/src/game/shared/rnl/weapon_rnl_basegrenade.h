//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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

	void	Precache() override;

	bool	CanDeploy(void) override;
	bool	CanHolster(void) override;
	bool	CanClimb(void) override;
	bool	CanRearm( CRnLPlayer* pPlayer ) override { return true; }

	bool	Deploy() override;
	bool	Holster( CBaseCombatWeapon *pSwitchingTo ) override;
	void	Equip(CBaseCombatCharacter* pOwner) override;

	void	PrimaryAttack() override;

	bool	Reload() override;

	void	ItemPostFrame() override;

	void	WeaponIdle() override;
	void	HandleViewAnimation(int iAnim) override;

	// Michael Lebson
	bool IsGrenade() const override { return true; }

	Activity GetDrawActivity(void) override { return ACT_VM_SHOULDERDRAW; }
	
	
	virtual void	ThrowGrenade();
	virtual void	RollGrenade();
	virtual void	DropGrenade();

	virtual bool IsSmokeGrenade(void) { return false; }
	bool IsGrenadePrimed( ) { return m_iGrenadeState == GRENADE_STARTFUSE; };
	
	virtual int GetGrenadeState( void ) { return m_iGrenadeState; }

	//Default Any State but pin
	virtual bool ShouldDrop( void ) { return m_iGrenadeState != GRENADE_DRAWN; }

	void	DecrementAmmo(CBaseCombatCharacter* pOwner);

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