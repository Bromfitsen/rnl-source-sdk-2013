//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbasepistol.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBasePistol, DT_WeaponRnLBasePistol );

BEGIN_NETWORK_TABLE( CWeaponRnLBasePistol, DT_WeaponRnLBasePistol )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponRnLBasePistol )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_basepistol, CWeaponRnLBasePistol );

CWeaponRnLBasePistol::CWeaponRnLBasePistol()
{
}

CWeaponRnLBasePistol::~CWeaponRnLBasePistol()
{
}