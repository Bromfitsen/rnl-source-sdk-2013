//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasesubmachinegun.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBaseSubMachineGun, DT_WeaponRnLBaseSubMachineGun );

BEGIN_NETWORK_TABLE( CWeaponRnLBaseSubMachineGun, DT_WeaponRnLBaseSubMachineGun )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
	BEGIN_PREDICTION_DATA( CWeaponRnLBaseSubMachineGun )
	END_PREDICTION_DATA()
#endif

#ifdef SERVER_DLL
	BEGIN_DATADESC( CWeaponRnLBaseSubMachineGun )
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( weapon_basesubmachinegun, CWeaponRnLBaseSubMachineGun );

CWeaponRnLBaseSubMachineGun::CWeaponRnLBaseSubMachineGun()
{
}

CWeaponRnLBaseSubMachineGun::~CWeaponRnLBaseSubMachineGun()
{
}

void CWeaponRnLBaseSubMachineGun::PrimaryAttack( void )
{
	// If there is ammo left in the clip
	if (m_iClip1 > 0 )
	{
		CRnLPlayer *pPlayer = GetPlayerOwner();

		// The weapon has a player, and they aren't in the water.
		// AND they don't already have less than 40 stamina, take 2 stamina away.
		if (pPlayer && pPlayer->GetWaterLevel() < 3 && pPlayer->GetStamina() >= 40)
		{
			pPlayer->SetStamina(pPlayer->GetStamina() - 2);
		}
	}

	BaseClass::PrimaryAttack();
}