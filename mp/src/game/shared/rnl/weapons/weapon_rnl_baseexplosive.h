//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_RNL_BASEEXPLOSIVE_H
#define WEAPON_RNL_BASEEXPLOSIVE_H
#ifdef _WIN32
#pragma once
#endif


#include "rnl_task_destructable.h"
#include "weapon_rnl_base.h"


#ifdef CLIENT_DLL
	
	#define CWeaponRnLBaseExplosive C_WeaponRnLBaseExplosive

#else

	extern ConVar mp_melontastic;
	#define TNT_MELON_MODEL "models/props_junk/watermelon01.mdl"

#endif

class CWeaponRnLBaseExplosive : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CWeaponRnLBaseExplosive, CWeaponRnLBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponRnLBaseExplosive();

	void Precache() override;

	bool Deploy() override;
	bool CanDeploy() override;
	bool Holster( CBaseCombatWeapon *pSwitchingTo ) override;

	void PrimaryAttack() override;
	void ItemPostFrame() override ;

	void HandleViewAnimation(int iAnim) override;
	void WeaponIdle(void) override;

	bool ShouldUseWeaponActivities(void) override { return true; }

	virtual void PlantExplosive( void ) { }

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

protected:
	CNetworkVar( bool, m_bPlanting );
	CNetworkHandle( CRnLTaskDestructable, m_pPlantingTarget );
private:
	CWeaponRnLBaseExplosive( const CWeaponRnLBaseExplosive & ) {}
};

#endif // WEAPON_RNL_BASEEXPLOSIVE_H